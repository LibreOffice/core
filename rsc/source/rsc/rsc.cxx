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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#ifdef UNX
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#else
#include <io.h>
#include <process.h>
#include <direct.h>
#endif

#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <tools/stream.hxx>
#include <rscerror.h>
#include <rsctop.hxx>
#include <rscdb.hxx>
#include <rscpar.hxx>
#include <rscrsc.hxx>
#include <rschash.hxx>

#include <osl/file.h>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/strbuf.hxx>
#include <rtl/tencinfo.h>
#include <rtl/textenc.h>
#include <comphelper/string.hxx>

#include <vector>
#include <algorithm>

using comphelper::string::getToken;
using comphelper::string::getTokenCount;

OString*  pStdParType  = NULL;
OString*  pStdPar1     = NULL;
OString*  pStdPar2     = NULL;
OString*  pWinParType  = NULL;
OString*  pWinPar1     = NULL;
OString*  pWinPar2     = NULL;
sal_uInt32      nRefDeep     = 10;
AtomContainer*  pHS          = NULL;


void RscCmdLine::Init()
{
    nCommands       = 0;
    nByteOrder      = RSC_BIGENDIAN;

    aPath = OString(".");
    m_aOutputFiles.clear();
    m_aOutputFiles.push_back( OutputFile() );
}

RscCmdLine::RscCmdLine( int argc, char ** argv, RscError * pEH )
{
    char *          pStr;
    char **         ppStr;
    RscPtrPtr       aCmdLine;
    sal_uInt32      i;
    bool            bOutputSrsIsSet = false;

    Init();

    pStr = ::ResponseFile( &aCmdLine, argv, argc );
    if( pStr )
        pEH->FatalError( ERR_OPENFILE, RscId(), pStr );

    /* check the inputted switches       */
    ppStr  = (char **)aCmdLine.GetBlock();
    ppStr++;
    i = 1;
    while( ppStr && i < (aCmdLine.GetCount() -1) )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "CmdLineArg: \"%s\"\n", *ppStr );
#endif
        if( '-' == **ppStr )
        {
            if( !rsc_stricmp( (*ppStr) + 1, "h" )
              || !strcmp( (*ppStr) + 1, "?" ) )
            { // Write help to standard output
                nCommands |= HELP_FLAG;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "p" ) )
            { // No preprocessor
                nCommands |= NOPREPRO_FLAG;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "s" ) )
            { // Syntax analysis, creates .srs file
                nCommands |= NOLINK_FLAG;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "l" ) )
            { // Linken, keine Syntax und kein Prepro
                nCommands |= NOPREPRO_FLAG;
                nCommands |= NOSYNTAX_FLAG;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "r" ) )
            { // erzeugt kein .res-file
                nCommands |= NORESFILE_FLAG;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "sub", 3 ) )
            {
                const char* pEqual;
                for( pEqual = (*ppStr)+4; *pEqual && *pEqual != '='; ++pEqual )
                    ;
                if( *pEqual )
                {
                    m_aReplacements.push_back( std::pair< OString, OString >( OString( (*ppStr)+4, pEqual - *ppStr - 4 ),
                        OString( pEqual + 1 ) ) );
                }
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "PreLoad" ) )
            { // Alle Resourcen mit Preload
                nCommands |= PRELOAD_FLAG;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "LITTLEENDIAN" ) )
            { // Byte Ordnung beim Schreiben
                nByteOrder = RSC_LITTLEENDIAN;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "BIGENDIAN" ) )
            { // Byte Ordnung beim Schreiben
                nByteOrder = RSC_BIGENDIAN;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "d", 1 ) )
            { // Symbole definieren
                nCommands |= DEFINE_FLAG;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "i", 1 ) )
            { // define include path
                nCommands |= INCLUDE_FLAG;
                OStringBuffer aBuffer(aPath);
                if (!aBuffer.isEmpty())
                    aBuffer.append(SAL_PATHSEPARATOR);
                aBuffer.append((*ppStr) + 2);
                aPath = aBuffer.makeStringAndClear();
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fs=", 3 ) )
            { // define name of .res file
                if( m_aOutputFiles.back().aOutputRc.getLength() )
                    m_aOutputFiles.push_back( OutputFile() );
                m_aOutputFiles.back().aOutputRc = (*ppStr) + 4;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "lip=", 4 ) )
            {  // additional language specific include for system dependent files
                const OString aSysSearchDir( (*ppStr)+5 );

                // ignore empty -lip= arguments that we get lots of these days
                if (!aSysSearchDir.isEmpty())
                {
                    m_aOutputFiles.back().aSysSearchDirs.push_back(aSysSearchDir);
                    OString aLangSearchPath = m_aOutputFiles.back().aLangSearchPath;
                    if( !aLangSearchPath.isEmpty() )
                    {
                        aLangSearchPath = aLangSearchPath + OString( SAL_PATHSEPARATOR );
                    }
                    aLangSearchPath = aLangSearchPath + aSysSearchDir;

                    m_aOutputFiles.back().aLangSearchPath = aLangSearchPath;
                }
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fp=", 3 ) )
            { // define name of .srs file
                aOutputSrs = (*ppStr) + 4;
                bOutputSrsIsSet = true;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "oil=", 4 ) )
            {
                aILDir = (*ppStr) + 5;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "NoSysResTest" ) )
            { // Bitmap, Pointers, Icons nicht ueberpruefen
                nCommands |= NOSYSRESTEST_FLAG;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "SrsDefault" ) )
            { // Only write one language to srs file
                nCommands |= SRSDEFAULT_FLAG;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "lg" ) )
            {
                m_aOutputFiles.back().aLangName = OString();
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "lg", 2 ) )
            {
                if( !m_aOutputFiles.back().aLangName.isEmpty() )
                    m_aOutputFiles.push_back( OutputFile() );
                m_aOutputFiles.back().aLangName = OString((*ppStr)+3);
            }
            else
                pEH->FatalError( ERR_UNKNOWNSW, RscId(), *ppStr );
        }
        else
        {
            // Eingabedatei
            aInputList.push_back( new OString(*ppStr) );
        }
        ppStr++;
        i++;
    }

    if( nCommands & HELP_FLAG )
        pEH->FatalError( ERR_USAGE, RscId() );
    // was an inputted file specified
    else if( !aInputList.empty() )
    {
        ::std::list<OutputFile>::iterator it;
        for( it = m_aOutputFiles.begin(); it != m_aOutputFiles.end(); ++it )
        {
            if( it->aOutputRc.isEmpty() )
                it->aOutputRc  = ::OutputFile( *aInputList.front(), "rc"  );
        }
        if( ! bOutputSrsIsSet )
            aOutputSrs = ::OutputFile( *aInputList.front(), "srs" );
    }
    else
        pEH->FatalError( ERR_NOINPUT, RscId() );
}

RscCmdLine::~RscCmdLine()
{
    for ( size_t i = 0, n = aInputList.size(); i < n; ++i )
        delete aInputList[ i ];
    aInputList.clear();
}

OString RscCmdLine::substitutePaths( const OString& rIn )
{
    // prepare return value
    OStringBuffer aRet( 256 );
    std::list< std::pair< OString, OString > >::const_iterator last_match = m_aReplacements.end();

    // search for longest replacement match
    for( std::list< std::pair< OString, OString > >::const_iterator repl = m_aReplacements.begin(); repl != m_aReplacements.end(); ++repl )
    {
        if( rIn.startsWith( repl->second ) ) // path matches
        {
            if( last_match == m_aReplacements.end() || last_match->second.getLength() < repl->second.getLength() )
                last_match = repl;
        }
    }

    // copy replacement found and rest of rIn
    sal_Int32 nIndex = 0;
    if( last_match != m_aReplacements.end() )
    {
        aRet.append( "%" );
        aRet.append( last_match->first );
        aRet.append( "%" );
        nIndex = last_match->second.getLength();
    }

    if( rIn.match( "/", nIndex ) )
        aRet.append( rIn.copy( nIndex ) );
    else
        aRet.append( rIn.copy( nIndex - 1 ) );

    return aRet.makeStringAndClear();
}

RscCompiler::RscCompiler( RscCmdLine * pLine, RscTypCont * pTypCont )
{
    fListing      = NULL;
    fExitFile     = NULL;

    //Set Command Line, set Type Container
    pCL = pLine;
    pTC = pTypCont;
}

RscCompiler::~RscCompiler()
{
    pTC->pEH->SetListFile( NULL );

    if( fListing )
        fclose( fListing );

    if( fExitFile )
        fclose( fExitFile );
}

ERRTYPE RscCompiler::Start()
{
    ERRTYPE         aError;
    RscFile*        pFName;

    if( pCL->aInputList.empty() )
        pTC->pEH->FatalError( ERR_NOINPUT, RscId() );

    for( size_t i = 0, n = pCL->aInputList.size(); i < n; ++i )
        pTC->aFileTab.NewCodeFile( *pCL->aInputList[ i ] );

    if( !(pCL->nCommands & NOSYNTAX_FLAG) )
    {
        if( pCL->nCommands & NOPREPRO_FLAG )
        {

            pTC->pEH->SetListFile( NULL );

            sal_uIntPtr aIndex = pTC->aFileTab.FirstIndex();
            while( aIndex != UNIQUEINDEX_ENTRY_NOTFOUND && aError.IsOk() )
            {
                pFName = pTC->aFileTab.Get( aIndex );
                if( !pFName->bScanned && !pFName->IsIncFile() )
                {
                    aError = IncludeParser( aIndex );
                    // Currentzeiger richtig setzen
                    aIndex = pTC->aFileTab.GetIndexOf( pFName );
                };
                aIndex = pTC->aFileTab.NextIndex( aIndex );
            };

            pTC->pEH->SetListFile( fListing );
        }
    };

    if ( pTC->pEH->GetVerbosity() >= RscVerbosityVerbose )
    {
        pTC->pEH->StdOut( "Files: " );
        sal_uIntPtr aIndex = pTC->aFileTab.FirstIndex();
        while( aIndex != UNIQUEINDEX_ENTRY_NOTFOUND )
        {
            pFName = pTC->aFileTab.Get( aIndex );
            pTC->pEH->StdOut( pFName->aFileName.getStr() );
            pTC->pEH->StdOut( " " );
            aIndex = pTC->aFileTab.NextIndex( aIndex );
        };
        pTC->pEH->StdOut( "\n" );
    }

    if( aError.IsOk() )
        aError = Link();

    if( aError.IsOk() )
        EndCompile();

    if( aError.IsError() )
        pTC->pEH->Error( ERR_ERROR, NULL, RscId() );

    return aError;
}

void RscCompiler::EndCompile()
{
    if( !pCL->aOutputSrs.isEmpty() && (pCL->nCommands & NOLINK_FLAG) )
    {
        pTC->pEH->StdOut( "Writing file ", RscVerbosityVerbose );
        pTC->pEH->StdOut( pCL->aOutputSrs.getStr(), RscVerbosityVerbose );
        pTC->pEH->StdOut( ".\n", RscVerbosityVerbose );

        // kopiere von TMP auf richtigen Namen
        unlink( pCL->aOutputSrs.getStr() );   // Zieldatei loeschen
        if( !(pCL->nCommands & NOSYNTAX_FLAG) )
        {
            FILE        * foutput;
            RscFile     * pFN;

            if( NULL == (foutput = fopen( pCL->aOutputSrs.getStr(), "w" )) )
                pTC->pEH->FatalError( ERR_OPENFILE, RscId(), pCL->aOutputSrs.getStr() );
            else
            {
                // Schreibe Datei
                sal_uIntPtr aIndex = pTC->aFileTab.FirstIndex();
                while( aIndex != UNIQUEINDEX_ENTRY_NOTFOUND )
                {
                    pFN = pTC->aFileTab.Get( aIndex );
                    if( !pFN->IsIncFile() )
                    {
                        pTC->WriteSrc( foutput, NOFILE_INDEX, false );
                        break; // ?T 281091MM nur eine Src-Datei
                    }
                };

                fclose( foutput );
            };
        };
    }
}

ERRTYPE RscCompiler :: IncludeParser( sal_uLong lFileKey )
{
    FILE            * finput;
    RscFile         * pFName;
    ERRTYPE           aError;

    pFName = pTC->aFileTab.Get( lFileKey );
    if( !pFName )
        aError = ERR_ERROR;
    else if( !pFName->bScanned )
    {
        finput = fopen( pFName->aPathName.getStr(), "r" );
        if( !finput )
        {
            aError = ERR_OPENFILE;
            pTC->pEH->Error( aError, NULL, RscId(),
                             pFName->aPathName.getStr() );
        }
        else
        {
            RscFile         * pFNTmp;
            RscDepend       * pDep;
            RscFileInst       aFileInst( pTC, lFileKey, lFileKey, finput );

            pFName->bScanned = true;
            ::IncludeParser( &aFileInst );
            fclose( finput );

            // Include-Pfad durchsuchen
            for ( size_t i = 0, n = pFName->aDepLst.size(); i < n; ++i )
            {
                pDep = pFName->aDepLst[ i ];
                pFNTmp = pTC->aFileTab.GetFile( pDep->GetFileKey() );
            }

            for ( size_t i = 0, n = pFName->aDepLst.size(); i < n; ++i )
            {
                pDep = pFName->aDepLst[ i ];
                pFNTmp = pTC->aFileTab.GetFile( pDep->GetFileKey() );
                // Kein Pfad und Include Datei
                if( pFNTmp && !pFNTmp->bLoaded )
                {
                   pFNTmp->aPathName = pFNTmp->aFileName;
                }
            };
        };
    };

    return aError;
}

ERRTYPE RscCompiler :: ParseOneFile( sal_uLong lFileKey,
                                     const RscCmdLine::OutputFile* pOutputFile,
                                     const WriteRcContext* pContext )
{
    FILE *     finput = NULL;
    ERRTYPE    aError;
    RscFile *  pFName;

    pFName = pTC->aFileTab.Get( lFileKey );
    if( !pFName )
        aError = ERR_ERROR;
    else if( !pFName->bLoaded )
    {
        RscDepend * pDep;

        //Include-Dateien vorher lesen
        pFName->bLoaded = true; //Endlos Rekursion vermeiden

        for ( size_t i = 0; i < pFName->aDepLst.size() && aError.IsOk(); ++i )
        {
            pDep = pFName->aDepLst[ i ];
            aError = ParseOneFile( pDep->GetFileKey(), pOutputFile, pContext );
        }

        if( aError.IsError() )
            pFName->bLoaded = false; //bei Fehler nicht geladenen
        else
        {
            OUString aTmpPath;
            OUString aSrsPath = OStringToOUString( pFName->aPathName, RTL_TEXTENCODING_ASCII_US );

            osl::FileBase::createTempFile( 0, 0, &aTmpPath );
            osl::FileBase::getFileURLFromSystemPath( aSrsPath, aSrsPath );

            if( pContext && pOutputFile )
                PreprocessSrsFile( *pOutputFile, *pContext, aSrsPath, aTmpPath );
            else
                osl::File::copy( aSrsPath, aTmpPath );

            OUString aParseFile;
            osl::FileBase::getSystemPathFromFileURL( aTmpPath, aParseFile );
            finput = fopen(OUStringToOString(aParseFile, RTL_TEXTENCODING_ASCII_US).getStr(), "r");

            if( !finput )
            {
                pTC->pEH->Error( ERR_OPENFILE, NULL, RscId(), pFName->aPathName.getStr() );
                aError = ERR_OPENFILE;
            }
            else
            {
                RscFileInst aFileInst( pTC, lFileKey, lFileKey, finput );

                pTC->pEH->StdOut( "reading file ", RscVerbosityVerbose );
                pTC->pEH->StdOut( OUStringToOString(aParseFile, RTL_TEXTENCODING_ASCII_US).getStr(), RscVerbosityVerbose );
                pTC->pEH->StdOut( " ", RscVerbosityVerbose );

                aError = ::parser( &aFileInst );
                if( aError.IsError() )
                    pTC->Delete( lFileKey );//Resourceobjekte loeschen
                pTC->pEH->StdOut( "\n", RscVerbosityVerbose );
                fclose( finput );
            };

            osl::File::remove( aTmpPath );
        };
    };

    return aError;
}

namespace
{
    using namespace ::osl;
    class RscIoError { };

    static inline OUString lcl_getAbsoluteUrl(const OUString& i_sBaseUrl, const OString& i_sPath)
    {
        OUString sRelUrl, sAbsUrl;
        if(FileBase::getFileURLFromSystemPath(OStringToOUString(i_sPath, RTL_TEXTENCODING_MS_1252), sRelUrl) != FileBase::E_None)
            throw RscIoError();
        if(FileBase::getAbsoluteFileURL(i_sBaseUrl, sRelUrl, sAbsUrl) != FileBase::E_None)
            throw RscIoError();
        return sAbsUrl;
    };

    static inline OString lcl_getSystemPath(const OUString& i_sUrl)
    {
        OUString sSys;
        if(FileBase::getSystemPathFromFileURL(i_sUrl, sSys) != FileBase::E_None)
            throw RscIoError();
        OSL_TRACE("temporary file: %s", OUStringToOString(sSys, RTL_TEXTENCODING_UTF8).getStr());
        return OUStringToOString(sSys, RTL_TEXTENCODING_MS_1252);
    };

    static inline OString lcl_getTempFile(OUString& sTempDirUrl)
    {
        // get a temp file name for the rc file
        OUString sTempUrl;
        if(FileBase::createTempFile(&sTempDirUrl, NULL, &sTempUrl) != FileBase::E_None)
            throw RscIoError();
        OSL_TRACE("temporary url: %s", OUStringToOString(sTempUrl, RTL_TEXTENCODING_UTF8).getStr());
        return lcl_getSystemPath(sTempUrl);
    };
}

ERRTYPE RscCompiler::Link()
{
    FILE *      foutput;
    ERRTYPE     aError;
    RscFile*    pFName;

    if( !(pCL->nCommands & NOLINK_FLAG) )
    {
        ::std::list<RscCmdLine::OutputFile>::const_iterator it;

        for( it = pCL->m_aOutputFiles.begin(); it != pCL->m_aOutputFiles.end(); ++it )
        {
            // cleanup nodes
            for( sal_uIntPtr aIndex = pTC->aFileTab.FirstIndex();
                 aIndex != UNIQUEINDEX_ENTRY_NOTFOUND && aError.IsOk();
                 aIndex = pTC->aFileTab.NextIndex( aIndex ) )
            {
                pFName = pTC->aFileTab.Get( aIndex );
                if( !pFName->IsIncFile() )
                {
                    pTC->Delete( aIndex );
                    aIndex = pTC->aFileTab.GetIndexOf( pFName );
                    pFName->bLoaded = false;
                }
            }


            // get two temp file urls
            OString aRcTmp, aSysListTmp, aSysList;
            try
            {
                OUString sPwdUrl;
                osl_getProcessWorkingDir( &sPwdUrl.pData );
                OUString sRcUrl = lcl_getAbsoluteUrl(sPwdUrl, it->aOutputRc);
                // TempDir is either the directory where the rc file is located or pwd
                OUString sTempDirUrl = sRcUrl.copy(0,sRcUrl.lastIndexOf('/'));
                OSL_TRACE("rc directory URL: %s", OUStringToOString(sTempDirUrl, RTL_TEXTENCODING_UTF8).getStr());

                aRcTmp = lcl_getTempFile(sTempDirUrl);
                OSL_TRACE("temporary rc file: %s", aRcTmp.getStr());

                OUString sOilDirUrl;
                if(!pCL->aILDir.isEmpty())
                    sOilDirUrl = lcl_getAbsoluteUrl(sPwdUrl, pCL->aILDir);
                else
                    sOilDirUrl = sTempDirUrl;
                OSL_TRACE("ilst directory URL: %s", OUStringToOString(sOilDirUrl, RTL_TEXTENCODING_UTF8).getStr());

                aSysListTmp = lcl_getTempFile(sOilDirUrl);
                OSL_TRACE("temporary ilst file: %s", aSysListTmp.getStr());

                OUString sIlstUrl;
                sIlstUrl = sRcUrl.copy(sRcUrl.lastIndexOf('/')+1);
                sIlstUrl = sIlstUrl.copy(0,sIlstUrl.lastIndexOf('.'));
                sIlstUrl += ".ilst";
                sIlstUrl = lcl_getAbsoluteUrl(sOilDirUrl, OUStringToOString(sIlstUrl, RTL_TEXTENCODING_UTF8));

                aSysList = lcl_getSystemPath(sIlstUrl);
                OSL_TRACE("ilst file: %s", aSysList.getStr());
            }
            catch (RscIoError&)
            {
                OString sMsg("Error with paths:\n");
                sMsg += "temporary rc file: " + aRcTmp + "\n";
                sMsg += "temporary ilst file: " + aSysListTmp + "\n";
                sMsg += "ilst file: " + aSysList + "\n";
                pTC->pEH->FatalError(ERR_OPENFILE, RscId(), sMsg.getStr());
            }
            if ( NULL == (fExitFile = foutput = fopen( aRcTmp.getStr(), "wb" )) )
                pTC->pEH->FatalError( ERR_OPENFILE, RscId(), aRcTmp.getStr() );

            // Schreibe Datei
            sal_Char cSearchDelim = SAL_PATHSEPARATOR;
            sal_Char cAccessDelim = SAL_PATHDELIMITER;
            pTC->ChangeLanguage( it->aLangName );
            pTC->SetSourceCharSet( RTL_TEXTENCODING_UTF8 );
            pTC->ClearSysNames();
            OStringBuffer aSysSearchPath(it->aLangSearchPath);
            sal_Int32 nIndex = 0;
            OString aSearchPath = pTC->GetSearchPath();
            do
            {
                OString aToken = aSearchPath.getToken( 0, cSearchDelim, nIndex );
                if (!aSysSearchPath.isEmpty())
                    aSysSearchPath.append(cSearchDelim);
                aSysSearchPath.append(aToken);
                aSysSearchPath.append(cAccessDelim);
                aSysSearchPath.append(it->aLangName);
                aSysSearchPath.append(cSearchDelim);
                aSysSearchPath.append(aToken);
            }
            while ( nIndex >= 0 );
            OSL_TRACE( "setting search path for language %s: %s", it->aLangName.getStr(), aSysSearchPath.getStr() );
            pTC->SetSysSearchPath(aSysSearchPath.makeStringAndClear());

            WriteRcContext  aContext;

            aContext.fOutput = foutput;
            aContext.aOutputRc = it->aOutputRc;
            aContext.aOutputSysList = aSysListTmp;
            aContext.pCmdLine = pCL;

            // create empty sys list
            if( !aContext.aOutputSysList.isEmpty() )
            {
                FILE* pSysListFile = fopen( aContext.aOutputSysList.getStr(), "wb" );

                if( !pSysListFile )
                    pTC->pEH->FatalError( ERR_OPENFILE, RscId(), aContext.aOutputSysList.getStr() );
                else
                    fclose( pSysListFile );
            }

            // parse files for specific language
            for( sal_uIntPtr aIndex = pTC->aFileTab.FirstIndex();
                 aIndex != UNIQUEINDEX_ENTRY_NOTFOUND && aError.IsOk();
                 aIndex = pTC->aFileTab.NextIndex( aIndex ) )
            {
                pFName = pTC->aFileTab.Get( aIndex );
                if( !pFName->IsIncFile() )
                {
                    aError = ParseOneFile( aIndex, &*it, &aContext );
                    aIndex = pTC->aFileTab.GetIndexOf( pFName );
                }
            };

            aError = pTC->WriteRc( aContext );

            fclose( foutput );
            fExitFile = NULL;
            unlink( it->aOutputRc.getStr() );
            if( rename( aRcTmp.getStr(), it->aOutputRc.getStr() ) )
            {
                OStringBuffer aBuf;
                aBuf.append( aRcTmp );
                aBuf.append( " -> " );
                aBuf.append( it->aOutputRc );
                pTC->pEH->FatalError( ERR_RENAMEFILE, RscId(), aBuf.getStr() );
            }
            else
            {
#ifdef UNX
                chmod( it->aOutputRc.getStr(), S_IRWXU | S_IRWXG | S_IROTH );
#endif
            }

            unlink( aSysList.getStr() );
            if( rename( aSysListTmp.getStr(), aSysList.getStr() ) )
            {
                OStringBuffer aBuf;
                aBuf.append( aSysListTmp );
                aBuf.append( " -> " );
                aBuf.append( aSysList );
                pTC->pEH->FatalError( ERR_RENAMEFILE, RscId(), aBuf.getStr() );
            }
            else
            {
#ifdef UNX
                chmod( aSysList.getStr(), S_IRWXU | S_IRWXG | S_IROTH );
#endif
            }
        }
    }
    else
    {
        // parse files
        for( sal_uIntPtr aIndex = pTC->aFileTab.FirstIndex();
             aIndex != UNIQUEINDEX_ENTRY_NOTFOUND && aError.IsOk();
             aIndex = pTC->aFileTab.NextIndex( aIndex ) )
        {
            pFName = pTC->aFileTab.Get( aIndex );
            if( !pFName->IsIncFile() )
            {
                aError = ParseOneFile( aIndex, NULL, NULL );
                aIndex = pTC->aFileTab.GetIndexOf( pFName );
            }
        };
    }

    return aError;
}

bool RscCompiler::GetImageFilePath( const RscCmdLine::OutputFile& rOutputFile,
                                    const WriteRcContext& rContext,
                                    const OString& rBaseFileName,
                                    OString& rImagePath,
                                    FILE* pSysListFile )
{
    ::std::list< OString >  aFileNames;
    bool bFound = false;

    aFileNames.push_back( rBaseFileName + OString(".png") );
    aFileNames.push_back( rBaseFileName + OString(".bmp") );

    ::std::list< OString >::iterator aFileIter( aFileNames.begin() );

    while( ( aFileIter != aFileNames.end() ) && !bFound )
    {
        ::std::list< OString >::const_iterator aDirIter( rOutputFile.aSysSearchDirs.begin() );

        while( ( aDirIter != rOutputFile.aSysSearchDirs.end() ) && !bFound )
        {
            const OString aSysPath = *aDirIter + "/" + *aFileIter;
            OUString aAbsPath = OStringToOUString( aSysPath, RTL_TEXTENCODING_ASCII_US );


            osl::FileBase::getFileURLFromSystemPath( aAbsPath, aAbsPath );
            osl::DirectoryItem aDirectoryItem;
            bool bFile = false;
            if (osl::DirectoryItem::E_None == osl::DirectoryItem::get( aAbsPath, aDirectoryItem ))
            {
                osl::FileStatus aFS(osl_FileStatus_Mask_Type);
                if (osl::DirectoryItem::E_None == aDirectoryItem.getFileStatus( aFS ))
                    bFile = aFS.isRegular();
            }

#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "Searching image: %s\n", aSysPath.getStr() );
#endif

            if( bFile )
            {
                std::list< std::pair< OString, OString > >::const_iterator  aReplIter( rContext.pCmdLine->m_aReplacements.begin() );
                OString aRelPathStr( aSysPath );

                while( ( aReplIter != rContext.pCmdLine->m_aReplacements.end() ) && !bFound )
                {
                    OString aSearch(aReplIter->second.toAsciiLowerCase());
                    OString aSearchIn(aRelPathStr.toAsciiLowerCase());
                    if( aSearchIn.indexOf(aSearch) == 0 )
                    {
                        sal_Int32       nCopyPos = aReplIter->second.getLength(), nLength = aRelPathStr.getLength();
                        const sal_Char* pChars = aRelPathStr.getStr();

                        while( ( nCopyPos < nLength ) && ( pChars[ nCopyPos ] == '/' || pChars[ nCopyPos ] == '\\' || pChars[ nCopyPos ] == ':' ) )
                        {
                            ++nCopyPos;
                        }

                        if( nCopyPos < nLength )
                            rImagePath = aRelPathStr.copy( nCopyPos ).replace( '\\', '/' );

                        bFound = true;
                    }

                    ++aReplIter;
                }

                if( bFound && pSysListFile )
                {
                    fprintf( pSysListFile, "%s\n", rContext.pCmdLine->substitutePaths( aSysPath ).getStr() );
                }

#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "ImagePath to add: %s\n", rImagePath.getStr() );
#endif
            }

            ++aDirIter;
        }

        ++aFileIter;
    }

    return bFound;
}

void RscCompiler::PreprocessSrsFile( const RscCmdLine::OutputFile& rOutputFile,
                                     const WriteRcContext& rContext,
                                     const OUString& rSrsInPath,
                                     const OUString& rSrsOutPath )
{
    SvFileStream                aIStm( rSrsInPath, STREAM_READ );
    SvFileStream                aOStm( rSrsOutPath, STREAM_WRITE | STREAM_TRUNC );
    ::std::vector< OString > aMissingImages;
    FILE*                       pSysListFile = rContext.aOutputSysList.isEmpty() ? NULL : fopen( rContext.aOutputSysList.getStr(), "ab" );

    if( !aIStm.GetError() && !aOStm.GetError() )
    {
        OString aLine;
        OString aFilePath;

        while( aIStm.ReadLine( aLine ) )
        {
            if( ( getTokenCount(aLine, '=') == 2 ) &&
                ( getToken(aLine, 0, '=').indexOf("File") != -1 ) )
            {
                OString aBaseFileName( getToken(getToken(aLine, 1, '"'), 0, '.') );

                if( GetImageFilePath( rOutputFile, rContext, aBaseFileName, aFilePath, pSysListFile ) )
                {
                    aLine = OStringBuffer(RTL_CONSTASCII_STRINGPARAM("File = \"")).
                        append(aFilePath).append(RTL_CONSTASCII_STRINGPARAM("\";")).
                        makeStringAndClear();
                }
                else
                    aMissingImages.push_back( aBaseFileName );

                aOStm.WriteLine(aLine);
            }
            else if (aLine.indexOfL(RTL_CONSTASCII_STRINGPARAM("ImageList")) != -1)
            {
                ::std::vector< ::std::pair< OString, sal_Int32 > > aEntryVector;

                aOStm.WriteLine(aLine);

                if (aLine.indexOf(';') == -1)
                {
                    const sal_uInt32 nImgListStartPos = aIStm.Tell();

                    do
                    {
                        if( !aIStm.ReadLine(aLine) )
                            break;
                    }
                    while (aLine.indexOfL(RTL_CONSTASCII_STRINGPARAM("Prefix")) == -1);

                    const OString aPrefix( getToken(aLine, 1, '"') );
                    aIStm.Seek( nImgListStartPos );

                    do
                    {
                        if (!aIStm.ReadLine(aLine) )
                            break;
                    }
                    while (aLine.indexOfL(RTL_CONSTASCII_STRINGPARAM("IdList")) == -1);

                    // scan all ids and collect images
                    while (aLine.indexOf('}') == -1)
                    {
                        if( !aIStm.ReadLine(aLine) )
                            break;

                        aLine = comphelper::string::stripStart(aLine, ' ');
                        aLine = comphelper::string::stripStart(aLine, '\t');
                        aLine = comphelper::string::remove(aLine, ';');

                        if (comphelper::string::isdigitAsciiString(aLine))
                        {
                            sal_Int32 nNumber = atoi(aLine.getStr());

                            OStringBuffer aBuf(aPrefix);
                            if( nNumber < 10000 )
                                aBuf.append('0');
                            aBuf.append(aLine);
                            OString aBaseFileName = aBuf.makeStringAndClear();

                            if( GetImageFilePath( rOutputFile, rContext, aBaseFileName, aFilePath, pSysListFile ) )
                                aEntryVector.push_back( ::std::pair< OString, sal_Int32 >( aFilePath, nNumber ) );
                            else
                                aMissingImages.push_back( aBaseFileName );
                        }
                    }

                    const sal_uInt32 nImgListEndPos = aIStm.Tell();
                    aIStm.Seek( nImgListStartPos );
                    while( aIStm.Tell() < nImgListEndPos )
                    {
                        aIStm.ReadLine( aLine );

                        if (aLine.indexOfL(RTL_CONSTASCII_STRINGPARAM("IdList")) != -1)
                        {
                            while (aLine.indexOf('}') == -1)
                                aIStm.ReadLine(aLine);
                        }
                        else
                            aOStm.WriteLine(aLine);
                    }

                    aOStm.WriteLine(OString(RTL_CONSTASCII_STRINGPARAM("FileList = {")));

                    for( sal_uInt32 i = 0; i < aEntryVector.size(); ++i )
                    {
                        OStringBuffer aEntryString(
                            RTL_CONSTASCII_STRINGPARAM("< \""));

                        aEntryString.append(aEntryVector[i].first);
                        aEntryString.append(RTL_CONSTASCII_STRINGPARAM("\"; "));
                        aEntryString.append(static_cast<sal_Int32>(aEntryVector[ i ].second));
                        aEntryString.append(RTL_CONSTASCII_STRINGPARAM("; >;"));

                        aOStm.WriteLine(aEntryString.makeStringAndClear());
                    }

                    aOStm.WriteLine(OString(RTL_CONSTASCII_STRINGPARAM("};")));
                }
                else
                    aOStm.WriteLine(aLine);
            }
            else
                aOStm.WriteLine(aLine);
        }
    }

    if( aMissingImages.size() > 0 )
    {
        OStringBuffer aImagesStr;

        for( sal_uInt32 i = 0; i < aMissingImages.size(); ++i )
        {
            if( i )
                aImagesStr.append(' ');

            aImagesStr.append(aMissingImages[i]);
        }

        pTC->pEH->FatalError( ERR_NOIMAGE, RscId(), aImagesStr.getStr() );
    }

    if( pSysListFile )
        fclose( pSysListFile );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
