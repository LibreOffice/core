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

#ifndef HELPLINKER_HXX
#define HELPLINKER_HXX

#include <l10ntools/dllapi.h>

#include <rtl/ustring.hxx>
#include <set>

#include <string.h>
#include <limits.h>

#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/functions.h>
#include <libxslt/extensions.h>

#include <sal/main.h>
#include <sal/types.h>
#include <osl/time.h>
#include <rtl/bootstrap.hxx>


// I assume that TCHAR is defined as wchar_t throughout

namespace lucene
{
namespace document
{
class Document;
}
namespace util
{
class Reader;
}
}

#define DBHELP_ONLY

class L10N_DLLPUBLIC IndexerPreProcessor
{
private:
    std::string       m_aModuleName;
    fs::path          m_fsIndexBaseDir;
    fs::path          m_fsCaptionFilesDirName;
    fs::path          m_fsContentFilesDirName;

    xsltStylesheetPtr m_xsltStylesheetPtrCaption;
    xsltStylesheetPtr m_xsltStylesheetPtrContent;

public:
    IndexerPreProcessor( const std::string& aModuleName, const fs::path& fsIndexBaseDir,
         const fs::path& idxCaptionStylesheet, const fs::path& idxContentStylesheet );
    ~IndexerPreProcessor();

    void processDocument( xmlDocPtr doc, const std::string& EncodedDocPath );
};

class L10N_DLLPUBLIC HelpLinker
{
public:
    void main(std::vector<std::string> &args,
              std::string* pExtensionPath = NULL,
              std::string* pDestination = NULL,
              const rtl::OUString* pOfficeHelpPath = NULL )

            throw( HelpProcessingException );

    HelpLinker()
        : init(true)
        , m_pIndexerPreProcessor(NULL)
    {}
    ~HelpLinker()
        { delete m_pIndexerPreProcessor; }

private:
    int locCount, totCount;
    Stringtable additionalFiles;
    HashSet helpFiles;
    fs::path sourceRoot;
    fs::path embeddStylesheet;
    fs::path idxCaptionStylesheet;
    fs::path idxContentStylesheet;
    fs::path zipdir;
    fs::path outputFile;
    std::string extsource;
    std::string extdestination;
    std::string module;
    std::string lang;
    std::string extensionPath;
    std::string extensionDestination;
    bool bExtensionMode;
    fs::path indexDirName;
    fs::path indexDirParentName;
    bool init;
    IndexerPreProcessor* m_pIndexerPreProcessor;
    void initIndexerPreProcessor();
    void link() throw( HelpProcessingException );
    void addBookmark( DB* dbBase, FILE* pFile_DBHelp, std::string thishid,
        const std::string& fileB, const std::string& anchorB,
        const std::string& jarfileB, const std::string& titleB );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
