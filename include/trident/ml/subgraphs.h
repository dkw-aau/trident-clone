#ifndef _SUBGRAPHS_H
#define _SUBGRAPHS_H

#include <trident/ml/embeddings.h>
#include <trident/kb/querier.h>

#include <inttypes.h>
#include <vector>

template<typename K>
class Subgraphs {
    public:
        typedef enum { PO, SP } TYPE;

    private:
        struct Metadata {
            uint64_t id;
            TYPE t;
            uint64_t ent, rel;
        };

        std::vector<Metadata> subgraphs;

    public:
        void addSubgraph(const TYPE t, uint64_t ent, uint64_t rel) {
            Metadata m;
            m.id = subgraphs.size();
            m.t = t;
            m.ent = ent;
            m.rel = rel;
            subgraphs.push_back(m);
        }

        virtual void calculateEmbeddings(Querier *q,
                std::shared_ptr<Embeddings<K>> E,
                std::shared_ptr<Embeddings<K>> R) = 0;

        virtual void loadFromFile(string file) = 0;
};

template<typename K>
class CIKMSubgraphs : public Subgraphs<K> {
    private:
        std::vector<K> params;
        uint16_t dim;

    public:
        void loadFromFile(string file) {
            std::ifstream ifs;
            ifs.open(file, std::ifstream::in);
            boost::iostreams::filtering_stream<boost::iostreams::input> in;
            in.push(ifs);
            in.push(boost::iostreams::gzip_decompressor());
            char bdim[2];
            in.read(bdim, 2);
            dim = *(uint16_t*) bdim;
            const uint16_t sizeline = 17 + dim * 8;
            std::unique_ptr<char> buffer = std::unique_ptr<char>(new char[sizeline]);
            while (true) {
                in.read(buffer.get(), sizeline);
                if (in.eof()) {
                    break;
                }
                //Parse the subgraph
                uint64_t ent = *(uint64_t*)(buffer.get() + 1);
                uint64_t rel = *(uint64_t*)(buffer.get() + 9);
                if (buffer.get()[0]) {
                    this->addSubgraph(Subgraphs<K>::TYPE::SP, ent, rel);
                } else {
                    this->addSubgraph(Subgraphs<K>::TYPE::PO, ent, rel);
                }
                //Parse the features vector
                const uint8_t lenParam = sizeof(K);
                for(uint16_t i = 0; i < dim; ++i) {
                    K param = *(K*) (buffer.get() + 17 + i * lenParam); 
                    params.push_back(param);
                }
            }
        }

        void calculateEmbeddings(Querier *q,
                std::shared_ptr<Embeddings<K>> E,
                std::shared_ptr<Embeddings<K>> R) {}
};

template<typename K>
class GaussianSubgraphs : public Subgraphs<K> {
    private:
        std::vector<double> mu;
        std::vector<double> sigma;

    public:
        void calculateEmbeddings(Querier *q,
                std::shared_ptr<Embeddings<K>> E,
                std::shared_ptr<Embeddings<K>> R) {
            //TODO
        }
};

#endif