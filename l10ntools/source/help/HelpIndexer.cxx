#include "HelpIndexer.hxx"

#define TODO

#ifdef TODO
#include <CLucene/analysis/LanguageBasedAnalyzer.h>
#endif

#include <rtl/string.hxx>

#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <algorithm>

using namespace lucene::document;

HelpIndexer::HelpIndexer(rtl::OUString const &lang, rtl::OUString const &module,
	rtl::OUString const &captionDir, rtl::OUString const &contentDir, rtl::OUString const &indexDir) :
d_lang(lang), d_module(module), d_captionDir(captionDir), d_contentDir(contentDir), d_indexDir(indexDir),
d_error(), d_files() {}

bool HelpIndexer::indexDocuments() {
	if (!scanForFiles()) {
		return false;
	}

#ifdef TODO
	// Construct the analyzer appropriate for the given language
	lucene::analysis::Analyzer *analyzer = (
		d_lang.compareToAscii("ja") == 0 ?
		(lucene::analysis::Analyzer*)new lucene::analysis::LanguageBasedAnalyzer(L"cjk") :
		(lucene::analysis::Analyzer*)new lucene::analysis::standard::StandardAnalyzer());
#else
	lucene::analysis::Analyzer *analyzer = (
		(lucene::analysis::Analyzer*)new lucene::analysis::standard::StandardAnalyzer());
#endif

	rtl::OString indexDirStr;
	d_indexDir.convertToString(&indexDirStr, RTL_TEXTENCODING_ASCII_US, 0);
	lucene::index::IndexWriter writer(indexDirStr.getStr(), analyzer, true);

	// Index the identified help files
	Document doc;
	for (std::set<rtl::OUString>::iterator i = d_files.begin(); i != d_files.end(); ++i) {
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

rtl::OUString const & HelpIndexer::getErrorMessage() {
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

bool HelpIndexer::scanForFiles(rtl::OUString const & path) {
	rtl::OString pathStr;
	path.convertToString(&pathStr, RTL_TEXTENCODING_ASCII_US, 0);
	DIR *dir = opendir(pathStr.getStr());
	if (dir == 0) {
		d_error = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Error reading directory ")) + path +
			 rtl::OUString::createFromAscii(strerror(errno));
		return true;
	}

	struct dirent *ent;
	struct stat info;
	while ((ent = readdir(dir)) != 0) {
		rtl::OString entPath(pathStr);
		entPath += rtl::OString(RTL_CONSTASCII_STRINGPARAM("/")) + rtl::OString(ent->d_name);
		if (stat(entPath.getStr(), &info) == 0 && S_ISREG(info.st_mode)) {
			d_files.insert(rtl::OUString::createFromAscii(ent->d_name));
		}
	}

	closedir(dir);

	return true;
}

bool HelpIndexer::helpDocument(rtl::OUString const & fileName, Document *doc) {
	// Add the help path as an indexed, untokenized field.
	rtl::OUString path = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("#HLP#")) + d_module + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + fileName;
	// FIXME: the (TCHAR*) cast is a problem, because TCHAR does not match sal_Unicode
	doc->add(*new Field(_T("path"), (TCHAR*)path.getStr(), Field::STORE_YES | Field::INDEX_UNTOKENIZED));

	// Add the caption as a field.
	rtl::OUString captionPath = d_captionDir + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + fileName;
	doc->add(*new Field(_T("caption"), helpFileReader(captionPath), Field::STORE_NO | Field::INDEX_TOKENIZED));
	// FIXME: does the Document take responsibility for the FileReader or should I free it somewhere?

	// Add the content as a field.
	rtl::OUString contentPath = d_contentDir + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + fileName;
	doc->add(*new Field(_T("content"), helpFileReader(contentPath), Field::STORE_NO | Field::INDEX_TOKENIZED));
	// FIXME: does the Document take responsibility for the FileReader or should I free it somewhere?

	return true;
}

lucene::util::Reader *HelpIndexer::helpFileReader(rtl::OUString const & path) {
	rtl::OString pathStr;
	path.convertToString(&pathStr, RTL_TEXTENCODING_ASCII_US, 0);
	if (access(pathStr.getStr(), R_OK) == 0) {
		return new lucene::util::FileReader(pathStr.getStr(), "UTF-8");
	} else {
		return new lucene::util::StringReader(L"");
	}
}
