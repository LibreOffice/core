/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Gert van Valkenhoef <g.h.m.van.valkenhoef@rug.nl>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <helpcompiler/HelpIndexer.hxx>

#include <rtl/string.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>

#include "LuceneHelper.hxx"

using namespace lucene::document;

HelpIndexer::HelpIndexer(OUString const &lang, OUString const &module,
    OUString const &srcDir, OUString const &outDir)
    : d_lang(lang), d_module(module)
{
    d_indexDir = OUStringBuffer(outDir).append('/').
        append(module).appendAscii(RTL_CONSTASCII_STRINGPARAM(".idxl")).toString();
    d_captionDir = srcDir + OUString("/caption");
    d_contentDir = srcDir + OUString("/content");
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
        boost::scoped_ptr<lucene::analysis::Analyzer> analyzer;
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
        for (std::set<OUString>::iterator i = d_files.begin(); i != d_files.end(); ++i) {
            helpDocument(*i, &doc);
            writer.addDocument(&doc);
            doc.clear();
        }
        writer.optimize();

        // Optimize the index
        writer.optimize();
    }
    catch (CLuceneError &e)
    {
        d_error = OUString::createFromAscii(e.what());
        return false;
    }

    return true;
}

OUString const & HelpIndexer::getErrorMessage() {
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

bool HelpIndexer::scanForFiles(OUString const & path) {

    osl::Directory dir(path);
    if (osl::FileBase::E_None != dir.open()) {
        d_error = OUString("Error reading directory ") + path;
        return true;
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

bool HelpIndexer::helpDocument(OUString const & fileName, Document *doc) {
    // Add the help path as an indexed, untokenized field.

    OUString path = OUString("#HLP#") +
        d_module + OUString("/") + fileName;
    std::vector<TCHAR> aPath(OUStringToTCHARVec(path));
    doc->add(*_CLNEW Field(_T("path"), &aPath[0], Field::STORE_YES | Field::INDEX_UNTOKENIZED));

    OUString sEscapedFileName =
        rtl::Uri::encode(fileName,
        rtl_UriCharClassUric, rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8);

    // Add the caption as a field.
    OUString captionPath = d_captionDir + OUString("/") + sEscapedFileName;
    doc->add(*_CLNEW Field(_T("caption"), helpFileReader(captionPath), Field::STORE_NO | Field::INDEX_TOKENIZED));

    // Add the content as a field.
    OUString contentPath = d_contentDir + OUString("/") + sEscapedFileName;
    doc->add(*_CLNEW Field(_T("content"), helpFileReader(contentPath), Field::STORE_NO | Field::INDEX_TOKENIZED));

    return true;
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
