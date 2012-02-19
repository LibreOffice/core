#include <l10ntools/HelpIndexer.hxx>
#include "LuceneHelper.hxx"

#define TODO

#ifdef TODO
#include <CLucene/analysis/LanguageBasedAnalyzer.h>
#endif

#include <rtl/string.hxx>
#include <osl/file.hxx>

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
	osl::Directory dir(path);
	if (osl::FileBase::E_None != dir.open()) {
		d_error = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Error reading directory ")) + path;
		return true;
	}

	osl::DirectoryItem item;
	osl::FileStatus fileStatus(osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_Type);
	while (dir.getNextItem(item) == osl::FileBase::E_None) {
		if (fileStatus.getFileType() == osl::FileStatus::Regular) {
			d_files.insert(fileStatus.getFileName());
		}
	}

	return true;
}

bool HelpIndexer::helpDocument(rtl::OUString const & fileName, Document *doc) {
	// Add the help path as an indexed, untokenized field.
	rtl::OUString path = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("#HLP#")) + d_module + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + fileName;
	std::vector<TCHAR> aPath(OUStringToTCHARVec(path));
	doc->add(*new Field(_T("path"), &aPath[0], Field::STORE_YES | Field::INDEX_UNTOKENIZED));

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
	osl::File file(path);
	if (osl::FileBase::E_None == file.open(osl_File_OpenFlag_Read)) {
		file.close();
		rtl::OString pathStr;
		path.convertToString(&pathStr, RTL_TEXTENCODING_ASCII_US, 0); // FIXME: path encoding?
		return new lucene::util::FileReader(pathStr.getStr(), "UTF-8");
	} else {
		return new lucene::util::StringReader(L"");
	}
}
