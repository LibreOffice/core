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
#include <o3tl/string_view.hxx>
#include <memory>
#include <utility>

#include "LuceneHelper.hxx"
#include <CLucene.h>
#include <CLucene/analysis/LanguageBasedAnalyzer.h>

#if defined _WIN32
#include <o3tl/char16_t2wchar_t.hxx>
#include <prewin.h>
#include <postwin.h>
#include <systools/win32/extended_max_path.hxx>
#endif

using namespace lucene::document;

HelpIndexer::HelpIndexer(OUString lang, OUString module,
    std::u16string_view srcDir, std::u16string_view outDir)
    : d_lang(std::move(lang)), d_module(std::move(module))
{
    d_indexDir = outDir + OUStringChar('/') + d_module + ".idxl";
    osl_getAbsoluteFileURL(nullptr, d_indexDir.pData, &d_indexDir.pData);
    d_captionDir = OUString::Concat(srcDir) + "/caption";
    osl_getAbsoluteFileURL(nullptr, d_captionDir.pData, &d_captionDir.pData);
    d_contentDir = OUString::Concat(srcDir) + "/content";
    osl_getAbsoluteFileURL(nullptr, d_contentDir.pData, &d_contentDir.pData);
}

#if defined _WIN32
namespace
{
template <class Constructor>
auto TryWithUnicodePathWorkaround(const OUString& ustrPath, const Constructor& constructor)
{
    const rtl_TextEncoding eThreadEncoding = osl_getThreadTextEncoding();
    OString sPath = OUStringToOString(ustrPath, eThreadEncoding);
    try
    {
        // First try path in thread encoding (ACP in case of Windows).
        return constructor(sPath);
    }
    catch (const CLuceneError&)
    {
        // Maybe the path contains characters not representable in ACP. There's no API in lucene
        // that takes Unicode strings (they take 8-bit strings, and pass them to CRT library
        // functions without conversion).

        // For a workaround, try short name, which should only contain ASCII characters. Would
        // not help (i.e., would return original long name) if short (8.3) file name creation is
        // disabled in OS or volume settings.
        wchar_t buf[EXTENDED_MAX_PATH];
        if (GetShortPathNameW(o3tl::toW(ustrPath.getStr()), buf, std::size(buf)) == 0)
            throw;
        sPath = OUStringToOString(o3tl::toU(buf), eThreadEncoding);
        return constructor(sPath);
    }
}
}
#endif

bool HelpIndexer::indexDocuments()
{
    if (!scanForFiles())
        return false;

    try
    {
        std::u16string_view sLang = o3tl::getToken(d_lang, 0, '-');
        bool bUseCJK = sLang == u"ja" || sLang == u"ko" || sLang == u"zh";

        // Construct the analyzer appropriate for the given language
        std::unique_ptr<lucene::analysis::Analyzer> analyzer;
        if (bUseCJK)
            analyzer.reset(new lucene::analysis::LanguageBasedAnalyzer(L"cjk"));
        else
            analyzer.reset(new lucene::analysis::standard::StandardAnalyzer());

        OUString ustrSystemPath;
        osl::File::getSystemPathFromFileURL(d_indexDir, ustrSystemPath);

#if defined _WIN32
        // Make sure the path exists, or GetShortPathNameW (if attempted) will fail.
        osl::Directory::createPath(d_indexDir);
        auto writer = TryWithUnicodePathWorkaround(ustrSystemPath, [&analyzer](const OString& s) {
            return std::make_unique<lucene::index::IndexWriter>(s.getStr(), analyzer.get(), true);
        });
#else
        OString indexDirStr = OUStringToOString(ustrSystemPath, osl_getThreadTextEncoding());
        auto writer = std::make_unique<lucene::index::IndexWriter>(indexDirStr.getStr(),
                                                                   analyzer.get(), true);
#endif

#ifndef SYSTEM_CLUCENE
        // avoid random values in index file, making help indices reproducible
        writer->setSegmentInfoStartVersion(0);
#endif

        //Double limit of tokens allowed, otherwise we'll get a too-many-tokens
        //exception for ja help. Could alternative ignore the exception and get
        //truncated results as per java-Lucene apparently
        writer->setMaxFieldLength(lucene::index::IndexWriter::DEFAULT_MAX_FIELD_LENGTH*2);

        // Index the identified help files
        Document doc;
        for (auto const& elem : d_files)
        {
            helpDocument(elem, &doc);
            writer->addDocument(&doc);
            doc.clear();
        }

        // Optimize the index
        writer->optimize();
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
#if defined _WIN32
        return TryWithUnicodePathWorkaround(ustrSystemPath, [](const OString& s) {
            return _CLNEW lucene::util::FileReader(s.getStr(), "UTF-8");
        });
#else
        OString pathStr = OUStringToOString(ustrSystemPath, osl_getThreadTextEncoding());
        return _CLNEW lucene::util::FileReader(pathStr.getStr(), "UTF-8");
#endif
    } else {
        return _CLNEW lucene::util::StringReader(L"");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
