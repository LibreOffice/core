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
#include <string.h>

#ifdef UNX
#include <unistd.h>
#include <sys/wait.h>
#else // UNX

#include <io.h>
#include <process.h>
#include <dos.h>

#endif // UNX

#include <gui.hxx>
#include <rsctools.hxx>
#include <rscerror.h>
#include <sal/main.h>
#include <rtl/strbuf.hxx>


static bool CallPrePro( const OString& rInput,
                        const OString& rOutput, RscPtrPtr * pCmdLine,
                        bool bResponse )
{
    RscPtrPtr       aNewCmdL;   // Kommandozeile
    RscPtrPtr       aRespCmdL;   // Kommandozeile
    RscPtrPtr *     pCmdL = &aNewCmdL;
    int             i, nRet;
    FILE*           fRspFile = nullptr;
    OString    aRspFileName;

    if( bResponse )
    {
        aRspFileName = ::GetTmpFileName();
        fRspFile = fopen( aRspFileName.getStr(), "w" );
    }

    if( !fRspFile )
        aNewCmdL.Append( rsc_strdup( "rscpp" ) );

    bool bVerbose = false;
    for( i = 1; i < int(pCmdLine->GetCount() -1); i++ )
    {
        if ( 0 == rsc_stricmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-verbose" ) )
        {
            bVerbose = true;
            continue;
        }
        if (strcmp(static_cast<char *>(pCmdLine->GetEntry(i)), "-isystem") == 0)
        {
            // ignore "-isystem" and following arg
            if (i < int(pCmdLine->GetCount()) - 1)
            {
                ++i;
            }
            continue;
        }
        if (strncmp(
                static_cast<char *>(pCmdLine->GetEntry(i)), "-isystem",
                strlen("-isystem"))
            == 0)
        {
            // ignore args starting with "-isystem"
            continue;
        }
        if ( !rsc_strnicmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-u", 2 ) ||
             !rsc_strnicmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-i", 2 ) ||
             !rsc_strnicmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-d", 2 ))
        {
            aNewCmdL.Append( rsc_strdup( static_cast<char *>(pCmdLine->GetEntry( i )) ) );
        }
    }

    aNewCmdL.Append( rsc_strdup( rInput.getStr() ) );
    aNewCmdL.Append( rsc_strdup( rOutput.getStr() ) );
    aNewCmdL.Append( static_cast<void *>(nullptr) );

    if ( bVerbose )
    {
        printf( "Preprocessor commandline: " );
        for( i = 0; i < (int)(pCmdL->GetCount() -1); i++ )
        {
            printf( " " );
            printf( "%s", static_cast<const char *>(pCmdL->GetEntry( i )) );
        }
        printf( "\n" );
    }

    if( fRspFile )
    {
        aRespCmdL.Append( rsc_strdup( "rscpp" ) );
        OStringBuffer aTmpStr;
        aTmpStr.append('@').append(aRspFileName);
        aRespCmdL.Append( rsc_strdup( aTmpStr.getStr() ) );
        aRespCmdL.Append( static_cast<void *>(nullptr) );

        pCmdL = &aRespCmdL;
        for( i = 0; i < (int)(aNewCmdL.GetCount() -1); i++ )
        {
            fprintf( fRspFile, "%s ", static_cast<const char *>(aNewCmdL.GetEntry( i )) );
        }
        fclose( fRspFile );

        if ( bVerbose )
        {
            printf( "Preprocessor startline: " );
            for( i = 0; i < (int)(pCmdL->GetCount() -1); i++ )
            {
                printf( " " );
                printf( "%s", static_cast<const char *>(pCmdL->GetEntry( i )) );
            }
            printf( "\n" );
        }
    }

    nRet = rscpp_main( pCmdL->GetCount()-1, reinterpret_cast<char**>(pCmdL->GetBlock()) );

    if ( fRspFile )
    {
        #if OSL_DEBUG_LEVEL > 5
        fprintf( stderr, "leaving response file %s\n", aRspFileName.getStr() );
        #else
        unlink( aRspFileName.getStr() );
        #endif
    }
    if ( nRet )
        return false;

    return true;
}


static bool CallRsc2( RscStrList * pInputList,
                      const OString &rSrsName, RscPtrPtr * pCmdLine )
{
    int nRet;
    RscVerbosity eVerbosity = RscVerbosityNormal;

    RscPtrPtr aNewCmdL;
    aNewCmdL.Append( rsc_strdup( "rsc2" ) );

    for (int i = 1; i < (int)(pCmdLine->GetCount() -1); ++i)
    {
        if ( !rsc_stricmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-verbose" ) )
        {
            eVerbosity = RscVerbosityVerbose;
            continue;
        }
        if ( !rsc_stricmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-quiet" ) )
        {
            eVerbosity = RscVerbositySilent;
            continue;
        }
        if( !rsc_strnicmp( static_cast<char *>(pCmdLine->GetEntry( i )),  "-fp=", 4 ) ||
            !rsc_strnicmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-fo=", 4 ) ||
            !rsc_strnicmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-presponse", 10 ) ||
            !rsc_strnicmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-rc", 3 ) ||
            !rsc_stricmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-+" ) ||
            !rsc_stricmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-br" ) ||
            !rsc_stricmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-bz" ) ||
            !rsc_stricmp( static_cast<char *>(pCmdLine->GetEntry( i )), "-r" ) ||
            ( '-' != *static_cast<char *>(pCmdLine->GetEntry( i )) ) )
        {
        }
        else
        {
            aNewCmdL.Append( rsc_strdup( static_cast<char *>(pCmdLine->GetEntry( i )) ) );
        }
    }

    aNewCmdL.Append( rsc_strdup( rSrsName.getStr() ) );

    for ( size_t i = 0, n = pInputList->size(); i < n; ++i )
    {
        OString* pString = (*pInputList)[ i ];
        aNewCmdL.Append( rsc_strdup( pString->getStr() ) );
    }

    if ( eVerbosity >= RscVerbosityVerbose )
    {
        printf( "Rsc2 commandline: " );
        for( size_t i = 0; i < (unsigned int)(aNewCmdL.GetCount() -1); i++ )
        {
            printf( " %s", static_cast<const char *>(aNewCmdL.GetEntry( i )) );
        }
        printf( "\n" );
    }

    nRet = rsc2_main( aNewCmdL.GetCount(), reinterpret_cast<char**>(aNewCmdL.GetBlock()) );

    if( nRet )
        return false;
    return true;
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    bool        bPrePro  = true;
    bool        bHelp    = false;
    bool        bError   = false;
    bool        bResponse = false;
    OString     aSrsName;
    OString     aResName;
    RscStrList  aInputList;
    RscStrList  aTmpList;
    char *      pStr;
    char **     ppStr;
    RscPtrPtr   aCmdLine;       // Kommandozeile
    sal_uInt32  i;

    pStr = ::ResponseFile( &aCmdLine, argv, argc );
    if( pStr )
    {
        printf( "Cannot open response file <%s>\n", pStr );
        return 1;
    };

    ppStr  = reinterpret_cast<char **>(aCmdLine.GetBlock());
    ppStr++;
    i = 1;
    bool bSetSrs = false;
    while( ppStr && i < (aCmdLine.GetCount() -1) )
    {
        if (strcmp(*ppStr, "-isystem") == 0)
        {
            // ignore "-isystem" and following arg
            if (i < aCmdLine.GetCount() - 1)
            {
                ++ppStr;
                ++i;
            }
        }
        else if (strncmp(*ppStr, "-isystem", strlen("-isystem")) == 0)
        {
            // ignore args starting with "-isystem"
        }
        else if( '-' == **ppStr )
        {
            if( !rsc_stricmp( (*ppStr) + 1, "p" )
              || !rsc_stricmp( (*ppStr) + 1, "l" ) )
            { // kein Preprozessor
                bPrePro = false;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "h" ) )
            { // Hilfe anzeigen
                bHelp = true;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "presponse", 9 ) )
            { // whether to use response file when parameterising preprocessor
                bResponse = true;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fo=", 3 ) )
            { // anderer Name fuer .res-file
                aResName = (*ppStr) + 4;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fp=", 3 ) )
            { // anderer Name fuer .srs-file
                bSetSrs  = true;
                aSrsName = (*ppStr);
            }
        }
        else
        {
            // Eingabedatei
            aInputList.push_back( new OString(*ppStr) );
        }
        ppStr++;
        i++;
    }

    if( !aInputList.empty() )
    {
        /* build the output file names          */
        if (!aResName.getLength())
            aResName = OutputFile( *aInputList[ 0 ], "res" );
        if( ! bSetSrs )
        {
            aSrsName = OStringBuffer("-fp=").append(OutputFile(*aInputList[0], "srs")).
                makeStringAndClear();
        }
    };

    if( bHelp )
        bPrePro = false;

    if( bPrePro && !aInputList.empty() )
    {
        OString aTmpName;

        for ( size_t k = 0, n = aInputList.size(); k < n; ++k )
        {
            OString* pString = aInputList[ k ];
            aTmpName = ::GetTmpFileName();
            if( !CallPrePro( *pString, aTmpName, &aCmdLine, bResponse ) )
            {
                printf( "Error starting preprocessor\n" );
                bError = true;
                break;
            }
            aTmpList.push_back( new OString(aTmpName) );
        };
    };

    if( !bError )
    {
        if( !CallRsc2( bPrePro ? &aTmpList : &aInputList, aSrsName, &aCmdLine ) )
        {
            if( !bHelp )
            {
                printf( "Error starting rsc2 compiler\n" );
                bError = true;
            }
        };
    };

    for ( size_t k = 0, n = aTmpList.size(); k < n; ++k )
        unlink( aTmpList[ k ]->getStr() );

    return bError ? EXIT_FAILURE : EXIT_SUCCESS;
}

void RscExit( sal_uInt32 nExit )
{
    if( nExit )
        printf( "Program exit is %d\n", (int)nExit );
    exit( nExit );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
