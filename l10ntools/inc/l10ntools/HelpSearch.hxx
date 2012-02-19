#ifndef HELPSEARCH_HXX
#define HELPSEARCH_HXX

#include <l10ntools/dllapi.h>

#include <CLucene/StdHeader.h>
#include <CLucene.h>

#include <rtl/ustring.hxx>
#include <vector>

class L10N_DLLPUBLIC HelpSearch {
	private:
		rtl::OUString d_lang;
		rtl::OUString d_indexDir;

	public:

	/**
	 * @param lang Help files language.
	 * @param indexDir The directory where the index files are stored.
	 */
	HelpSearch(rtl::OUString const &lang, rtl::OUString const &indexDir);

	/**
	 * Query the index for a certain query string.
	 * @param queryStr The query.
	 * @param captionOnly Set to true to search in the caption, not the content.
	 * @param rDocuments Vector to write the paths of the found documents.
	 * @param rScores Vector to write the scores to.
	 */
	bool query(rtl::OUString const &queryStr, bool captionOnly,
		std::vector<rtl::OUString> &rDocuments, std::vector<float> &rScores);
};

#endif
