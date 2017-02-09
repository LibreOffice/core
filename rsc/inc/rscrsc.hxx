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
#ifndef INCLUDED_RSC_INC_RSCRSC_HXX
#define INCLUDED_RSC_INC_RSCRSC_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rsctools.hxx>

#include <list>

class RscTypCont;


/****************** R s c C m d L i n e **********************************/
class RscCmdLine
{
    void        Init();

public:

    RscStrList     aInputList;     // source file list
    OString        aPath;          // path list
    RSCBYTEORDER_TYPE   nByteOrder;
    CommandFlags   nCommands;      // command bits
    OString        aOutputSrs;     // Srs output file name
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
    static bool     GetImageFilePath( const RscCmdLine::OutputFile& rOutputFile,
                                      const WriteRcContext& rContext,
                                      const OString& rBaseFileName,
                                      OString& rImagePath,
                                      FILE* pSysListFile );
    void            PreprocessSrsFile( const RscCmdLine::OutputFile& rOutputFile,
                                       const WriteRcContext& rContext,
                                       const OUString& rSrsInPath,
                                       const OUString& rSrsOutPath );

public:
    RscTypCont*     pTC;        // string and id manager
    RscCmdLine*     pCL;        // command line
    FILE *          fExitFile;  // when deconstructed, this file must be deleted

                    RscCompiler( RscCmdLine *, RscTypCont * );
                    ~RscCompiler();

    ERRTYPE         Start();

                    // read include statements
    ERRTYPE         IncludeParser( RscFileTab::Index lFileKey );
    ERRTYPE         ParseOneFile( RscFileTab::Index lFileKey, const RscCmdLine::OutputFile* pOutputFile, const WriteRcContext* pContext );
    ERRTYPE         Link();
    void            EndCompile();
};

#endif // INCLUDED_RSC_INC_RSCRSC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
