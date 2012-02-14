#ifndef HELPINDEXER_HXX
#define HELPINDEXER_HXX

#include <CLucene/StdHeader.h>
#include <CLucene.h>

#include <string>
#include <set>

// I assume that TCHAR is defined as wchar_t throughout

class HelpIndexer {
	private:
		std::string d_lang;
		std::string d_module;
		std::string d_captionDir;
		std::string d_contentDir;
		std::string d_indexDir;
		std::string d_error;
		std::set<std::string> d_files;

	public:

	/**
	 * @param lang Help files language.
	 * @param module The module of the helpfiles.
	 * @param captionDir The directory to scan for caption files.
	 * @param contentDir The directory to scan for content files.
	 * @param indexDir The directory to write the index to.
	 */
	HelpIndexer(std::string const &lang, std::string const &module,
		std::string const &captionDir, std::string const &contentDir,
		std::string const &indexDir);

	/**
	 * Run the indexer.
	 * @return true if index successfully generated.
	 */
	bool indexDocuments();

	/**
	 * Get the error string (empty if no error occurred).
	 */
	std::string const & getErrorMessage();

	private:

	/**
	 * Scan the caption & contents directories for help files.
	 */
	bool scanForFiles();

	/**
	 * Scan for files in the given directory.
	 */
	bool scanForFiles(std::string const &path);

	/**
	 * Fill the Document with information on the given help file.
	 */
	bool helpDocument(std::string const & fileName, lucene::document::Document *doc);

	/**
	 * Create a reader for the given file, and create an "empty" reader in case the file doesn't exist.
	 */
	lucene::util::Reader *helpFileReader(std::string const & path);

	std::wstring string2wstring(std::string const &source);
};

#endif
