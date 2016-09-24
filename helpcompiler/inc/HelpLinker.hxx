/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_HELPCOMPILER_INC_HELPLINKER_HXX
#define INCLUDED_HELPCOMPILER_INC_HELPLINKER_HXX

#include <helpcompiler/dllapi.h>
#include <libxslt/transform.h>

#ifdef AIX
#    undef _THREAD_SAFE
#endif

class L10N_DLLPUBLIC IndexerPreProcessor
{
private:
    fs::path          m_fsCaptionFilesDirName;
    fs::path          m_fsContentFilesDirName;

    xsltStylesheetPtr m_xsltStylesheetPtrCaption;
    xsltStylesheetPtr m_xsltStylesheetPtrContent;

public:
    IndexerPreProcessor( const fs::path& fsIndexBaseDir,
         const fs::path& idxCaptionStylesheet, const fs::path& idxContentStylesheet );
    ~IndexerPreProcessor();

    void processDocument( xmlDocPtr doc, const std::string& EncodedDocPath );
};

class L10N_DLLPUBLIC HelpLinker
{
public:
    void main(std::vector<std::string> &args,
              std::string* pExtensionPath = nullptr,
              std::string* pDestination = nullptr,
              const OUString* pOfficeHelpPath = nullptr )
            throw( HelpProcessingException, std::exception );

    HelpLinker()
        : bExtensionMode(false)
        , m_pIndexerPreProcessor(nullptr)
        , m_bUseLangRoot(true)
        , m_bCreateIndex(true)
    {}
    ~HelpLinker()
    {
        delete m_pIndexerPreProcessor;
    }

private:
    Stringtable additionalFiles;
    HashSet helpFiles;
    fs::path sourceRoot;
    fs::path compactStylesheet;
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
    fs::path indexDirParentName;
    IndexerPreProcessor* m_pIndexerPreProcessor;
    bool m_bUseLangRoot;
    bool m_bCreateIndex;
    void initIndexerPreProcessor();
    void link() throw(HelpProcessingException, BasicCodeTagger::TaggerException, std::exception);
    static void addBookmark( FILE* pFile_DBHelp, std::string thishid,
        const std::string& fileB, const std::string& anchorB,
        const std::string& jarfileB, const std::string& titleB );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
