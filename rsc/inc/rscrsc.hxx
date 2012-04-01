/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _RSCRSC_HXX
#define _RSCRSC_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rsctools.hxx>

#include <list>

class RscTypCont;
class DirEntry;

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
    rtl::OString        aPath;          // Liste der Pfade
    RSCBYTEORDER_TYPE   nByteOrder;
    unsigned short      nCommands;      // Steuerbits
    rtl::OString        aOutputLst;     // Name der List-Ausgabedatei
    rtl::OString        aOutputSrs;     // Name der Srs-Ausgabedatei
    rtl::OString        aOutputSrc;     // Name der Src-Ausgabedatei
    rtl::OString        aOutputRcCtor;  // Name der Ctor-Ausgabedatei
    rtl::OString        aOutputCxx;     // Name der Cxx-Ausgabedatei
    rtl::OString        aOutputHxx;     // Name der Hxx-Ausgabedatei
    rtl::OString        aTouchFile;     // create this file when done in rsc2
    rtl::OString        aILDir;

    struct OutputFile
    {
        rtl::OString aLangName;         // language name
        rtl::OString aOutputRc;         // target file
        rtl::OString aLangSearchPath;   // language specific search path
        ::std::list< rtl::OString >    aSysSearchDirs;    // paths to search for images

        OutputFile() {}
    };

    std::list<OutputFile>                                   m_aOutputFiles;
    std::list< std::pair< rtl::OString, rtl::OString > >    m_aReplacements;

    RscCmdLine( int argc, char ** argv, RscError * pEH );
    ~RscCmdLine();

    ::rtl::OString     substitutePaths( const ::rtl::OString& rIn );
};
/****************** R s c ************************************************/

struct WriteRcContext;

class RscCompiler
{
private:
    rtl::OString    aTmpOutputHxx;  // Name der TempHxx-Ausgabedatei
    rtl::OString    aTmpOutputCxx;  // Name der TempCxx-Ausgabedatei
    rtl::OString    aTmpOutputRcCtor; // Name der Temp Ctor-Ausgabedatei
    rtl::OString    aTmpOutputSrc;  // Name der TempSrc-Ausgabedatei

    void            CreateResFile( const char * pRc );
    void            Append( const rtl::OString& rOutputSrs, const rtl::OString& rTmpFile );

    bool            GetImageFilePath( const RscCmdLine::OutputFile& rOutputFile,
                                      const WriteRcContext& rContext,
                                      const rtl::OString& rBaseFileName,
                                      rtl::OString& rImagePath,
                                      FILE* pSysListFile );
    void            PreprocessSrsFile( const RscCmdLine::OutputFile& rOutputFile,
                                       const WriteRcContext& rContext,
                                       const DirEntry& rSrsInPath,
                                       const DirEntry& rSrsOutPath );

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
