#include "HelpIndexer.hxx"

#include <string>
#include <iostream>

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
	HelpIndexer indexer(
		rtl::OUString::createFromAscii(lang.c_str()),
		rtl::OUString::createFromAscii(module.c_str()),
		rtl::OUString::createFromAscii(captionDir.c_str()),
		rtl::OUString::createFromAscii(contentDir.c_str()),
		rtl::OUString::createFromAscii(indexDir.c_str()));
	if (!indexer.indexDocuments()) {
		std::wcerr << indexer.getErrorMessage().getStr() << std::endl;
		return 2;
	}
	return 0;
}
