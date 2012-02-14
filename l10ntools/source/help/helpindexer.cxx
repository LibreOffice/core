#include <CLucene/StdHeader.h>
#include <CLucene.h>
#ifdef TODO
#include <CLucene/analysis/LanguageBasedAnalyzer.h>
#endif

#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <iostream>
#include <algorithm>
#include <set>

// I assume that TCHAR is defined as wchar_t throughout

using namespace lucene::document;

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
	bool helpDocument(std::string const & fileName, Document *doc);

	/**
	 * Create a reader for the given file, and create an "empty" reader in case the file doesn't exist.
	 */
	lucene::util::Reader *helpFileReader(std::string const & path);

	std::wstring string2wstring(std::string const &source);
};

HelpIndexer::HelpIndexer(std::string const &lang, std::string const &module,
	std::string const &captionDir, std::string const &contentDir, std::string const &indexDir) :
d_lang(lang), d_module(module), d_captionDir(captionDir), d_contentDir(contentDir), d_indexDir(indexDir), d_error(""), d_files() {}

bool HelpIndexer::indexDocuments() {
	if (!scanForFiles()) {
		return false;
	}

#ifdef TODO
	// Construct the analyzer appropriate for the given language
	lucene::analysis::Analyzer *analyzer = (
		d_lang.compare("ja") == 0 ?
		(lucene::analysis::Analyzer*)new lucene::analysis::LanguageBasedAnalyzer(L"cjk") :
		(lucene::analysis::Analyzer*)new lucene::analysis::standard::StandardAnalyzer());
#else
	lucene::analysis::Analyzer *analyzer = (
		(lucene::analysis::Analyzer*)new lucene::analysis::standard::StandardAnalyzer());
#endif

	lucene::index::IndexWriter writer(d_indexDir.c_str(), analyzer, true);

	// Index the identified help files
	Document doc;
	for (std::set<std::string>::iterator i = d_files.begin(); i != d_files.end(); ++i) {
		doc.clear();
		if (!helpDocument(*i, &doc)) {
			delete analyzer;
			return false;
		}
		writer.addDocument(&doc);
	}

	// Optimize the index
	writer.optimize();

	delete analyzer;
	return true;
}

std::string const & HelpIndexer::getErrorMessage() {
	return d_error;
}

bool HelpIndexer::scanForFiles() {
	if (!scanForFiles(d_contentDir)) {
		return false;
	}
	if (!scanForFiles(d_captionDir)) {
		return false;
	}
	return true;
}

bool HelpIndexer::scanForFiles(std::string const & path) {
	DIR *dir = opendir(path.c_str());
	if (dir == 0) {
		d_error = "Error reading directory " + path + strerror(errno);
		return true;
	}

	struct dirent *ent;
	struct stat info;
	while ((ent = readdir(dir)) != 0) {
		if (stat((path + "/" + ent->d_name).c_str(), &info) == 0 && S_ISREG(info.st_mode)) {
			d_files.insert(ent->d_name);
		}
	}

	closedir(dir);

	return true;
}

bool HelpIndexer::helpDocument(std::string const & fileName, Document *doc) {
	// Add the help path as an indexed, untokenized field.
	std::wstring path(L"#HLP#" + string2wstring(d_module) + L"/" + string2wstring(fileName));
	doc->add(*new Field(_T("path"), path.c_str(), Field::STORE_YES | Field::INDEX_UNTOKENIZED));

	// Add the caption as a field.
	std::string captionPath = d_captionDir + "/" + fileName;
	doc->add(*new Field(_T("caption"), helpFileReader(captionPath), Field::STORE_NO | Field::INDEX_TOKENIZED));
	// FIXME: does the Document take responsibility for the FileReader or should I free it somewhere?

	// Add the content as a field.
	std::string contentPath = d_contentDir + "/" + fileName;
	doc->add(*new Field(_T("content"), helpFileReader(contentPath), Field::STORE_NO | Field::INDEX_TOKENIZED));
	// FIXME: does the Document take responsibility for the FileReader or should I free it somewhere?

	return true;
}

lucene::util::Reader *HelpIndexer::helpFileReader(std::string const & path) {
	if (access(path.c_str(), R_OK) == 0) {
		return new lucene::util::FileReader(path.c_str(), "UTF-8");
	} else {
		return new lucene::util::StringReader(L"");
	}
}

std::wstring HelpIndexer::string2wstring(std::string const &source) {
	std::wstring target(source.length(), L' ');
	std::copy(source.begin(), source.end(), target.begin());
	return target;
}

int main(int argc, char **argv) {
	const std::string pLang("-lang");
	const std::string pModule("-mod");
	const std::string pOutDir("-zipdir");
	const std::string pSrcDir("-srcdir");

	std::string lang;
	std::string module;
	std::string srcDir;
	std::string outDir;

	bool error = false;
	for (int i = 1; i < argc; ++i) {
		if (pLang.compare(argv[i]) == 0) {
			if (i + 1 < argc) {
				lang = argv[++i];
			} else {
				error = true;
			}
		} else if (pModule.compare(argv[i]) == 0) {
			if (i + 1 < argc) {
				module = argv[++i];
			} else {
				error = true;
			}
		} else if (pOutDir.compare(argv[i]) == 0) {
			if (i + 1 < argc) {
				outDir = argv[++i];
			} else {
				error = true;
			}
		} else if (pSrcDir.compare(argv[i]) == 0) {
			if (i + 1 < argc) {
				srcDir = argv[++i];
			} else {
				error = true;
			}
		} else {
			error = true;
		}
	}

	if (error) {
		std::cerr << "Error parsing command-line arguments" << std::endl;
	}

	if (error || lang.empty() || module.empty() || srcDir.empty() || outDir.empty()) {
		std::cerr << "Usage: HelpIndexer -lang ISOLangCode -mod HelpModule -srcdir SourceDir -zipdir OutputDir" << std::endl;
		return 1;
	}

	std::string captionDir(srcDir + "/caption");
	std::string contentDir(srcDir + "/content");
	std::string indexDir(outDir + "/" + module + ".idxl");
	HelpIndexer indexer(lang, module, captionDir, contentDir, indexDir);
	if (!indexer.indexDocuments()) {
		std::cerr << indexer.getErrorMessage() << std::endl;
		return 2;
	}
	return 0;
}
