/*************************************************************************
 *
 *  $RCSfile: rscrsc.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-26 20:24:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _RSCRSC_HXX
#define _RSCRSC_HXX

#ifndef _LANG_HXX //autogen
#include <tools/lang.hxx>
#endif


#ifndef RSCALL_H
#include <rscall.h>
#endif
#ifndef RSCERROR_H
#include <rscerror.h>
#endif
#ifndef RSCTOOLS_HXX
#include <rsctools.hxx>
#endif

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
    RscStrList          aSymbolList;    // Liste der Symbole
    ByteString          aPath;          // Liste der Pfade
    RSCBYTEORDER_TYPE   nByteOrder;
    short               nCommands;      // Steuerbits
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

                    RscCmdLine( short argc, char ** argv, RscError * pEH );
                    RscCmdLine();

                    ~RscCmdLine();

  ::rtl::OString     substitutePaths( const ::rtl::OString& rIn );
};
/****************** R s c ************************************************/

class WriteRcContext;

class RscCompiler
{
private:
    RscStrList      aTmpFileList;   // Liste der Tmp-Dateien
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
    ERRTYPE         IncludeParser( ULONG lFileKey );
    ERRTYPE         ParseOneFile( ULONG lFileKey, const RscCmdLine::OutputFile* pOutputFile, const WriteRcContext* pContext );
    ERRTYPE         Link();
    void            EndCompile();
};

#endif //_RSCRSC_HXX
