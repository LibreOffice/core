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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"

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
#include <rsctools.hxx>
#include <rscerror.h>
#include <sal/main.h>
#include <tools/fsys.hxx>

/*************** C O D E ************************************************/

// Entry point declaration for modules rscpp and rsc2
extern "C"
{
    int rscpp_main(int, char**);
}
int rsc2_main(int, char**);

/*************************************************************************
|*    CallPrePro()
|*
|*    Beschreibung
*************************************************************************/
static sal_Bool CallPrePro( const ByteString& rInput,
                        const ByteString& rOutput,
                        RscPtrPtr * pCmdLine,
                        sal_Bool bResponse )
{
    RscPtrPtr       aNewCmdL;   // Kommandozeile
    RscPtrPtr       aRespCmdL;   // Kommandozeile
    RscPtrPtr *     pCmdL = &aNewCmdL;
    int             i, nRet;
    FILE*           fRspFile = NULL;
    ByteString      aRspFileName;

    if( bResponse )
    {
        aRspFileName = ::GetTmpFileName();
        fRspFile = fopen( aRspFileName.GetBuffer(), "w" );
    }

    if( !fRspFile )
        aNewCmdL.Append( rsc_strdup( "rscpp" ) );

    bool bVerbose = false;
    for( i = 1; i < int(pCmdLine->GetCount() -1); i++ )
    {
        if ( 0 == rsc_stricmp( (char *)pCmdLine->GetEntry( i ), "-verbose" ) )
        {
            bVerbose = true;
            continue;
        }
        if  (   !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-u", 2 )
            ||  !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-i", 2 )
            ||  !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-d", 2 )
            )
        {
            aNewCmdL.Append( rsc_strdup( (char *)pCmdLine->GetEntry( i ) ) );
        }
    }

    aNewCmdL.Append( rsc_strdup( rInput.GetBuffer() ) );
    aNewCmdL.Append( rsc_strdup( rOutput.GetBuffer() ) );
    aNewCmdL.Append( (void *)0 );

    if ( bVerbose )
    {
        printf( "Preprocessor commandline: " );
        for( i = 0; i < (int)(pCmdL->GetCount() -1); i++ )
        {
            printf( " " );
            printf( "%s", (const char *)pCmdL->GetEntry( i ) );
        }
        printf( "\n" );
    }

    if( fRspFile )
    {
        aRespCmdL.Append( rsc_strdup( "rscpp" ) );
        ByteString aTmpStr( '@' );
        aTmpStr += aRspFileName;
        aRespCmdL.Append( rsc_strdup( aTmpStr.GetBuffer() ) );
        aRespCmdL.Append( (void *)0 );

        pCmdL = &aRespCmdL;
        for( i = 0; i < (int)(aNewCmdL.GetCount() -1); i++ )
        {
            fprintf( fRspFile, "%s ", (const char *)aNewCmdL.GetEntry( i ) );
        }
        fclose( fRspFile );

        if ( bVerbose )
        {
            printf( "Preprocessor startline: " );
            for( i = 0; i < (int)(pCmdL->GetCount() -1); i++ )
            {
                printf( " " );
                printf( "%s", (const char *)pCmdL->GetEntry( i ) );
            }
            printf( "\n" );
        }
    }

    nRet = rscpp_main( pCmdL->GetCount()-1, (char**)pCmdL->GetBlock() );

    if ( fRspFile )
        #if OSL_DEBUG_LEVEL > 5
        fprintf( stderr, "leaving response file %s\n", aRspFileName.GetBuffer() );
        #else
        unlink( aRspFileName.GetBuffer() );
        #endif
    if ( nRet )
        return sal_False;

    return sal_True;
}


/*************************************************************************
|*    CallRsc2
|*
|*    Beschreibung
*************************************************************************/
static sal_Bool CallRsc2( RscStrList * pInputList,
                      ByteString aSrsName,
                      RscPtrPtr * pCmdLine )
{
    int nRet;
    ByteString*  pString;
    RscVerbosity eVerbosity = RscVerbosityNormal;

    RscPtrPtr aNewCmdL;
    aNewCmdL.Append( rsc_strdup( "rsc2" ) );

    for (int i = 1; i < (int)(pCmdLine->GetCount() -1); ++i)
    {
        if ( !rsc_stricmp( (char *)pCmdLine->GetEntry( i ), "-verbose" ) )
        {
            eVerbosity = RscVerbosityVerbose;
            continue;
        }
        if ( !rsc_stricmp( (char *)pCmdLine->GetEntry( i ), "-quiet" ) )
        {
            eVerbosity = RscVerbositySilent;
            continue;
        }
        if( !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ),  "-fp=", 4 )
          || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-fo=", 4 )
          || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-presponse", 9 )
          || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-rc", 3 )
          || !rsc_stricmp( (char *)pCmdLine->GetEntry( i ), "-+" )
          || !rsc_stricmp( (char *)pCmdLine->GetEntry( i ), "-br" )
          || !rsc_stricmp( (char *)pCmdLine->GetEntry( i ), "-bz" )
          || !rsc_stricmp( (char *)pCmdLine->GetEntry( i ), "-r" )
          || ( '-' != *(char *)pCmdLine->GetEntry( i ) ) )
        {
        }
        else
            aNewCmdL.Append( rsc_strdup( (char *)pCmdLine->GetEntry( i ) ) );
    };

    aNewCmdL.Append( rsc_strdup( aSrsName.GetBuffer() ) );

    for ( size_t i = 0, n = pInputList->size(); i < n; ++i )
    {
        pString = (*pInputList)[ i ];
        aNewCmdL.Append( rsc_strdup( pString->GetBuffer() ) );
    };

    if ( eVerbosity >= RscVerbosityVerbose )
    {
        printf( "Rsc2 commandline: " );
        for( size_t i = 0; i < (unsigned int)(aNewCmdL.GetCount() -1); i++ )
        {
            printf( " %s", (const char *)aNewCmdL.GetEntry( i ) );
        }
        printf( "\n" );
    }

    nRet = rsc2_main( aNewCmdL.GetCount(), (char**)aNewCmdL.GetBlock() );

    if( nRet )
        return( sal_False );
    return( sal_True );
}

/*************************************************************************
|*
|*    main()
|*
*************************************************************************/
SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    sal_Bool            bPrePro  = sal_True;
    sal_Bool            bHelp    = sal_False;
    sal_Bool            bError   = sal_False;
    sal_Bool            bResponse = sal_False;
    ByteString      aSrsName;
    ByteString      aResName;
    RscStrList      aInputList;
    RscStrList      aTmpList;
    char *          pStr;
    char **         ppStr;
    RscPtrPtr       aCmdLine;       // Kommandozeile
    sal_uInt32      i;
    ByteString*     pString;

    pStr = ::ResponseFile( &aCmdLine, argv, argc );
    if( pStr )
    {
        printf( "Cannot open response file <%s>\n", pStr );
        return( 1 );
    };

    ppStr  = (char **)aCmdLine.GetBlock();
    ppStr++;
    i = 1;
    sal_Bool bSetSrs = sal_False;
    while( ppStr && i < (aCmdLine.GetCount() -1) )
    {
        if( '-' == **ppStr )
        {
            if( !rsc_stricmp( (*ppStr) + 1, "p" )
              || !rsc_stricmp( (*ppStr) + 1, "l" ) )
            { // kein Preprozessor
                bPrePro = sal_False;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "h" ) )
            { // Hilfe anzeigen
                bHelp = sal_True;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "presponse", 9 ) )
            { // whether to use response file when parameterising preprocessor
                bResponse = sal_True;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fo=", 3 ) )
            { // anderer Name fuer .res-file
                aResName = (*ppStr) + 4;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fp=", 3 ) )
            { // anderer Name fuer .srs-file
                bSetSrs  = sal_True;
                aSrsName = (*ppStr);
            }
        }
        else
        {
            // Eingabedatei
            aInputList.push_back( new ByteString( *ppStr ) );
        }
        ppStr++;
        i++;
    }

    if( !aInputList.empty() )
    {
        /* build the output file names          */
        if( ! aResName.Len() )
            aResName = OutputFile( *aInputList[ 0 ], "res" );
        if( ! bSetSrs )
        {
            aSrsName = "-fp=";
            aSrsName += OutputFile( *aInputList[ 0 ], "srs" );
        }
    };

    if( bHelp )
        bPrePro = sal_False;
    if( bPrePro && !aInputList.empty() )
    {
        ByteString aTmpName;

        for ( size_t k = 0, n = aInputList.size(); k < n; ++k )
        {
            pString = aInputList[ k ];
            aTmpName = ::GetTmpFileName();
            if( !CallPrePro( *pString, aTmpName, &aCmdLine, bResponse ) )
            {
                printf( "Error starting preprocessor\n" );
                bError = sal_True;
                break;
            }
            aTmpList.push_back( new ByteString( aTmpName ) );
        };
    };

    if( !bError )
    {
        if( !CallRsc2( bPrePro ? &aTmpList : &aInputList, aSrsName, &aCmdLine ) )
        {
            if( !bHelp )
            {
                printf( "Error starting rsc2 compiler\n" );
                bError = sal_True;
            }
        };
    };

    for ( size_t k = 0, n = aTmpList.size(); k < n; ++k )
        unlink( aTmpList[ k ]->GetBuffer() );

    return( bError );
}

void RscExit( sal_uInt32 nExit )
{
    if( nExit )
        printf( "Program exit is %d\n", (int)nExit );
    exit( nExit );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
