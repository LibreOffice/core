#include <l10ntools/HelpSearch.hxx>
#include "LuceneHelper.hxx"

#include <iostream>

HelpSearch::HelpSearch(rtl::OUString const &lang, rtl::OUString const &indexDir) :
d_lang(lang), d_indexDir(indexDir) {}

bool HelpSearch::query(rtl::OUString const &queryStr, bool captionOnly,
		std::vector<rtl::OUString> &rDocuments, std::vector<float> &rScores) {
	rtl::OString pathStr;
	d_indexDir.convertToString(&pathStr, RTL_TEXTENCODING_ASCII_US, 0);
	lucene::index::IndexReader *reader = lucene::index::IndexReader::open(pathStr.getStr());
	lucene::search::IndexSearcher searcher(reader);

	TCHAR captionField[] = L"caption";
	TCHAR contentField[] = L"content";
	TCHAR *field = captionOnly ? captionField : contentField;

	bool isWildcard = queryStr[queryStr.getLength() - 1] == L'*';
	std::vector<TCHAR> aQueryStr(OUStringToTCHARVec(queryStr));
	lucene::search::Query *aQuery = (isWildcard ?
		(lucene::search::Query*)new lucene::search::WildcardQuery(new lucene::index::Term(field, &aQueryStr[0])) :
		(lucene::search::Query*)new lucene::search::TermQuery(new lucene::index::Term(field, &aQueryStr[0])));
	// FIXME: who is responsible for the Term*?

	lucene::search::Hits *hits = searcher.search(aQuery);
	for (unsigned i = 0; i < hits->length(); ++i) {
		lucene::document::Document &doc = hits->doc(i); // Document* belongs to Hits.
		wchar_t const *path = doc.get(L"path");
		rDocuments.push_back(TCHARArrayToOUString(path != 0 ? path : L""));
		rScores.push_back(hits->score(i));
	}

	delete hits;
	delete aQuery;

	reader->close();
	return true;
}
