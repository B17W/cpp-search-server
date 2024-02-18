#include <algorithm>
#include <cmath>
#include <map>
#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        vector<string> words = SplitIntoWordsNoStop(document);

        map<string, double> tf;
        for (const string& word : words) {
            tf[word] += 1.0 / words.size();
        }

        for (const auto& [word, n] : tf) {
            word_to_document_freqs_[word][document_id] = n;
        }

        document_count_++;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    map<string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;
    int document_count_ = 0;

    vector<string> SplitIntoWords(const string& text) const {
        vector<string> words;
        string word;
        for (const char c : text) {
            if (c == ' ') {
                if (!word.empty()) {
                    words.push_back(word);
                    word.clear();
                }
            }
            else {
                word += c;
            }
        }
        if (!word.empty()) {
            words.push_back(word);
        }

        return words;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (stop_words_.count(word) == 0) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                query.minus_words.insert(word.substr(1));
            }
            else {
                query.plus_words.insert(word);
            }
        }
        return query;
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        map<int, double> relevance_map;

        for (const auto& plus_word : query.plus_words) {
            if (word_to_document_freqs_.count(plus_word) > 0) {
                double idf = log(document_count_ / static_cast<double>(word_to_document_freqs_.at(plus_word).size()));

                for (const auto& [document_id, tf] : word_to_document_freqs_.at(plus_word)) {
                    relevance_map[document_id] += tf * idf;
                }
            }
        }

        for (const auto& minus_word : query.minus_words) {
            if (word_to_document_freqs_.count(minus_word) > 0) {
                for (const auto& [document_id, _] : word_to_document_freqs_.at(minus_word)) {
                    relevance_map.erase(document_id);
                }
            }
        }

        vector<Document> matched_documents;
        for (const auto& [document_id, relevance] : relevance_map) {
            matched_documents.push_back({ document_id, relevance });
        }

        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
};

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
            << "relevance = "s << relevance << " }"s << endl;
    }

    return 0;
}
