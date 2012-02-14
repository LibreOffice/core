#ifndef HELPINDEXER_HXX
#define HELPINDEXER_HXX

#include <CLucene/StdHeader.h>
#include <CLucene.h>

#include <rtl/ustring.hxx>
#include <set>

// I assume that TCHAR is defined as wchar_t throughout

class HelpIndexer {
	private:
		rtl::OUString d_lang;
		rtl::OUString d_module;
		rtl::OUString d_captionDir;
		rtl::OUString d_contentDir;
		rtl::OUString d_indexDir;
		rtl::OUString d_error;
		std::set<rtl::OUString> d_files;

	public:

	/**
	 * @param lang Help files language.
	 * @param module The module of the helpfiles.
	 * @param captionDir The directory to scan for caption files.
	 * @param contentDir The directory to scan for content files.
	 * @param indexDir The directory to write the index to.
	 */
	HelpIndexer(rtl::OUString const &lang, rtl::OUString const &module,
		rtl::OUString const &captionDir, rtl::OUString const &contentDir,
		rtl::OUString const &indexDir);

	/**
	 * Run the indexer.
	 * @return true if index successfully generated.
	 */
	bool indexDocuments();

	/**
	 * Get the error string (empty if no error occurred).
	 */
	rtl::OUString const & getErrorMessage();

	private:

	/**
	 * Scan the caption & contents directories for help files.
	 */
	bool scanForFiles();

	/**
	 * Scan for files in the given directory.
	 */
	bool scanForFiles(rtl::OUString const &path);

	/**
	 * Fill the Document with information on the given help file.
	 */
	bool helpDocument(rtl::OUString const & fileName, lucene::document::Document *doc);

	/**
	 * Create a reader for the given file, and create an "empty" reader in case the file doesn't exist.
	 */
	lucene::util::Reader *helpFileReader(rtl::OUString const & path);
};

#endif
