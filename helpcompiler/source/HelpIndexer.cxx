/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <helpcompiler/HelpIndexer.hxx>

#include <rtl/string.hxx>
#include <rtl/uri.hxx>
#include <o3tl/runtimetooustring.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <memory>

#include "LuceneHelper.hxx"
#include <CLucene.h>
#include <CLucene/analysis/LanguageBasedAnalyzer.h>

using namespace lucene::document;

HelpIndexer::HelpIndexer(OUString const &lang, OUString const &module,
    std::u16string_view srcDir, std::u16string_view outDir)
    : d_lang(lang), d_module(module)
{
    d_indexDir = outDir + OUStringChar('/') + module + ".idxl";
    d_captionDir = OUString::Concat(srcDir) + "/caption";
    d_contentDir = OUString::Concat(srcDir) + "/content";
}

bool HelpIndexer::indexDocuments()
{
    if (!scanForFiles())
        return false;

    try
    {
        OUString sLang = d_lang.getToken(0, '-');
        bool bUseCJK = sLang == "ja" || sLang == "ko" || sLang == "zh";

        // Construct the analyzer appropriate for the given language
        std::unique_ptr<lucene::analysis::Analyzer> analyzer;
        if (bUseCJK)
            analyzer.reset(new lucene::analysis::LanguageBasedAnalyzer(L"cjk"));
        else
            analyzer.reset(new lucene::analysis::standard::StandardAnalyzer());

        OUString ustrSystemPath;
        osl::File::getSystemPathFromFileURL(d_indexDir, ustrSystemPath);

        OString indexDirStr = OUStringToOString(ustrSystemPath, osl_getThreadTextEncoding());
        lucene::index::IndexWriter writer(indexDirStr.getStr(), analyzer.get(), true);
        //Double limit of tokens allowed, otherwise we'll get a too-many-tokens
        //exception for ja help. Could alternative ignore the exception and get
        //truncated results as per java-Lucene apparently
        writer.setMaxFieldLength(lucene::index::IndexWriter::DEFAULT_MAX_FIELD_LENGTH*2);

        // Index the identified help files
        Document doc;
        for (auto const& elem : d_files)
        {
            helpDocument(elem, &doc);
            writer.addDocument(&doc);
            doc.clear();
        }
        writer.optimize();

        // Optimize the index
        writer.optimize();
    }
    catch (CLuceneError &e)
    {
        d_error = o3tl::runtimeToOUString(e.what());
        return false;
    }

    return true;
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

bool HelpIndexer::scanForFiles(OUString const & path) {

    osl::Directory dir(path);
    if (osl::FileBase::E_None != dir.open()) {
        d_error = "Error reading directory " + path;
        return false;
    }

    osl::DirectoryItem item;
    osl::FileStatus fileStatus(osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_Type);
    while (dir.getNextItem(item) == osl::FileBase::E_None) {
        item.getFileStatus(fileStatus);
        if (fileStatus.getFileType() == osl::FileStatus::Regular) {
            d_files.insert(fileStatus.getFileName());
        }
    }

    return true;
}

void HelpIndexer::helpDocument(OUString const & fileName, Document *doc) const {
    // Add the help path as an indexed, untokenized field.

    OUString path = "#HLP#" + d_module + "/" + fileName;
    std::vector<TCHAR> aPath(OUStringToTCHARVec(path));
    doc->add(*_CLNEW Field(_T("path"), aPath.data(), int(Field::STORE_YES) | int(Field::INDEX_UNTOKENIZED)));

    OUString sEscapedFileName =
        rtl::Uri::encode(fileName,
        rtl_UriCharClassUric, rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8);

    // Add the caption as a field.
    OUString captionPath = d_captionDir + "/" + sEscapedFileName;
    doc->add(*_CLNEW Field(_T("caption"), helpFileReader(captionPath), int(Field::STORE_NO) | int(Field::INDEX_TOKENIZED)));

    // Add the content as a field.
    OUString contentPath = d_contentDir + "/" + sEscapedFileName;
    doc->add(*_CLNEW Field(_T("content"), helpFileReader(contentPath), int(Field::STORE_NO) | int(Field::INDEX_TOKENIZED)));
}

lucene::util::Reader *HelpIndexer::helpFileReader(OUString const & path) {
    osl::File file(path);
    if (osl::FileBase::E_None == file.open(osl_File_OpenFlag_Read)) {
        file.close();
        OUString ustrSystemPath;
        osl::File::getSystemPathFromFileURL(path, ustrSystemPath);
        OString pathStr = OUStringToOString(ustrSystemPath, osl_getThreadTextEncoding());
        return _CLNEW lucene::util::FileReader(pathStr.getStr(), "UTF-8");
    } else {
        return _CLNEW lucene::util::StringReader(L"");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
