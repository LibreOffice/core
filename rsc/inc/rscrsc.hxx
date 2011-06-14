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
    ByteString          aPath;          // Liste der Pfade
    RSCBYTEORDER_TYPE   nByteOrder;
    unsigned short      nCommands;      // Steuerbits
    ByteString          aOutputLst;     // Name der List-Ausgabedatei
    ByteString          aOutputSrs;     // Name der Srs-Ausgabedatei
    ByteString          aOutputSrc;     // Name der Src-Ausgabedatei
    ByteString          aOutputRcCtor;  // Name der Ctor-Ausgabedatei
    ByteString          aOutputCxx;     // Name der Cxx-Ausgabedatei
    ByteString          aOutputHxx;     // Name der Hxx-Ausgabedatei
    ByteString          aTouchFile;     // create this file when done in rsc2
    ByteString          aILDir;

    struct OutputFile
    {
        ByteString                   aLangName;         // language name
        ByteString                   aOutputRc;         // target file
        ByteString                   aLangSearchPath;   // language specific search path
        ::std::list< ByteString >    aSysSearchDirs;    // pathes to search for images

        OutputFile() {}
    };

    std::list<OutputFile>                                   m_aOutputFiles;
    std::list< std::pair< rtl::OString, rtl::OString > >    m_aReplacements;

                    RscCmdLine( int argc, char ** argv, RscError * pEH );
                    RscCmdLine();

                    ~RscCmdLine();

  ::rtl::OString     substitutePaths( const ::rtl::OString& rIn );
};
/****************** R s c ************************************************/

struct WriteRcContext;

class RscCompiler
{
private:
    ByteString      aTmpOutputHxx;  // Name der TempHxx-Ausgabedatei
    ByteString      aTmpOutputCxx;  // Name der TempCxx-Ausgabedatei
    ByteString      aTmpOutputRcCtor; // Name der Temp Ctor-Ausgabedatei
    ByteString      aTmpOutputSrc;  // Name der TempSrc-Ausgabedatei

    void            CreateResFile( const char * pRc );
    void            Append( const ByteString& rOutputSrs, const ByteString& rTmpFile );
    void            OpenInput( const ByteString& rInput );

    bool            GetImageFilePath( const RscCmdLine::OutputFile& rOutputFile,
                                       const WriteRcContext& rContext,
                                    const ByteString& rBaseFileName,
                                    ByteString& rImagePath,
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
    ByteString      GetTmpFileName();   // Die Dateien werden geloescht

                    // Include Statements lesen
    ERRTYPE         IncludeParser( sal_uLong lFileKey );
    ERRTYPE         ParseOneFile( sal_uLong lFileKey, const RscCmdLine::OutputFile* pOutputFile, const WriteRcContext* pContext );
    ERRTYPE         Link();
    void            EndCompile();
};

#endif //_RSCRSC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
