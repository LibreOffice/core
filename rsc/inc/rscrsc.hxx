/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _RSCRSC_HXX
#define _RSCRSC_HXX

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
        ::std::list< ByteString >    aSysSearchDirs;    // paths to search for images

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
    ERRTYPE         IncludeParser( sal_uLong lFileKey );
    ERRTYPE         ParseOneFile( sal_uLong lFileKey, const RscCmdLine::OutputFile* pOutputFile, const WriteRcContext* pContext );
    ERRTYPE         Link();
    void            EndCompile();
};

#endif //_RSCRSC_HXX
