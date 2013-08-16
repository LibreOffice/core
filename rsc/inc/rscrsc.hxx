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
#ifndef _RSCRSC_HXX
#define _RSCRSC_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rsctools.hxx>

#include <list>

class RscTypCont;

/****************** T Y P E S ********************************************/

#define MAX_INPUTFILES  100
#define MAX_SYMBOLS     10
#define MAX_INCLUDES    10

/****************** R s c C m d L i n e **********************************/
class RscCmdLine
{
    void        Init();

public:

    RscStrList          aInputList;     // Liste der Quelldateien
    OString        aPath;          // Liste der Pfade
    RSCBYTEORDER_TYPE   nByteOrder;
    unsigned short      nCommands;      // Steuerbits
    OString        aOutputSrs;     // Name der Srs-Ausgabedatei
    OString        aILDir;

    struct OutputFile
    {
        OString aLangName;         // language name
        OString aOutputRc;         // target file
        OString aLangSearchPath;   // language specific search path
        ::std::list< OString >    aSysSearchDirs;    // paths to search for images

        OutputFile() {}
    };

    std::list<OutputFile>                                   m_aOutputFiles;
    std::list< std::pair< OString, OString > >    m_aReplacements;

    RscCmdLine( int argc, char ** argv, RscError * pEH );
    ~RscCmdLine();

    OString     substitutePaths( const OString& rIn );
};
/****************** R s c ************************************************/

struct WriteRcContext;

class RscCompiler
{
private:
    void            CreateResFile( const char * pRc );

    bool            GetImageFilePath( const RscCmdLine::OutputFile& rOutputFile,
                                      const WriteRcContext& rContext,
                                      const OString& rBaseFileName,
                                      OString& rImagePath,
                                      FILE* pSysListFile );
    void            PreprocessSrsFile( const RscCmdLine::OutputFile& rOutputFile,
                                       const WriteRcContext& rContext,
                                       const OUString& rSrsInPath,
                                       const OUString& rSrsOutPath );

public:
    RscTypCont*     pTC;        // String und Id-Verwalter
    RscCmdLine*     pCL;        // Kommandozeile
    FILE *          fListing;   // Ausgabedatei fuer Listings
    FILE *          fExitFile;  // bei Abbruch muss diese Datei geschlossen werden

                    RscCompiler( RscCmdLine *, RscTypCont * );
                    ~RscCompiler();

    ERRTYPE         Start();

                    // Include Statements lesen
    ERRTYPE         IncludeParser( sal_uLong lFileKey );
    ERRTYPE         ParseOneFile( sal_uLong lFileKey, const RscCmdLine::OutputFile* pOutputFile, const WriteRcContext* pContext );
    ERRTYPE         Link();
    void            EndCompile();
};

#endif //_RSCRSC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
