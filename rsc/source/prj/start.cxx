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
#if defined (WNT) && defined (tcpp)
#define _spawnvp spawnvp
#define _P_WAIT P_WAIT
#endif

#ifdef UNX
#include <unistd.h>
#include <sys/wait.h>
#else // UNX

#include <io.h>
#include <process.h>
#if defined ( OS2 ) && !defined ( GCC )
#include <direct.h>
#endif
#if !defined ( CSET ) && !defined ( OS2 )
#include <dos.h>
#endif

#endif // UNX
#include <rsctools.hxx>
#include <rscerror.h>
#include <sal/main.h>
#include <tools/fsys.hxx>

/*************** C O D E ************************************************/
/****************************************************************/
/*                                                              */
/*  Function    :   fuer Ansi kompatibilitaet                   */
/*                                                              */
/****************************************************************/
#ifdef UNX
#define P_WAIT 0
    int spawnvp( int, const char * cmdname, char *const*  argv ){
        int rc(0);

        switch( fork() ){
            case -1:
                return( -1 );
            case 0:
                if( execvp( cmdname, argv ) == -1 )
                    // an error occurs
                    return( -1 );
                break;
            default:
                if( -1 == wait( &rc ) )
                    return( -1 );
        }
        return( WEXITSTATUS( rc ) );
    }
#endif

/*************************************************************************
|*    CallPrePro()
|*
|*    Beschreibung
*************************************************************************/
static sal_Bool CallPrePro( const ByteString& rPrePro,
                        const ByteString& rInput,
                        const ByteString& rOutput,
                        RscPtrPtr * pCmdLine,
                        sal_Bool bResponse )
{
    RscPtrPtr       aNewCmdL;   // Kommandozeile
    RscPtrPtr       aRespCmdL;   // Kommandozeile
    RscPtrPtr *     pCmdL = &aNewCmdL;
    int             i, nExit;
    FILE*           fRspFile = NULL;
    ByteString      aRspFileName;

    if( bResponse )
    {
        aRspFileName = ::GetTmpFileName();
        fRspFile = fopen( aRspFileName.GetBuffer(), "w" );
    }

    if( !fRspFile )
        aNewCmdL.Append( rsc_strdup( rPrePro.GetBuffer() ) );

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
        aRespCmdL.Append( rsc_strdup( rPrePro.GetBuffer() ) );
        ByteString aTmpStr( '@' );
        aTmpStr += aRspFileName;
        aRespCmdL.Append( rsc_strdup( aTmpStr.GetBuffer() ) );
        aRespCmdL.Append( (void *)0 );

        pCmdL = &aRespCmdL;
        for( i = 0; i < (int)(aNewCmdL.GetCount() -1); i++ )
        {
#ifdef OS2
            fprintf( fRspFile, "%s\n", (const char *)aNewCmdL.GetEntry( i ) );
#else
            fprintf( fRspFile, "%s ", (const char *)aNewCmdL.GetEntry( i ) );
#endif
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

#if ((defined OS2 || defined WNT) && (defined TCPP || defined tcpp)) || defined UNX || defined OS2
    nExit = spawnvp( P_WAIT, rPrePro.GetBuffer(), (char* const*)pCmdL->GetBlock() );
#elif defined CSET
    nExit = spawnvp( P_WAIT, (char*)rPrePro.GetBuffer(), (const char**)pCmdL->GetBlock() );
#elif defined WTC
    nExit = spawnvp( P_WAIT, (char*)rPrePro.GetBuffer(), (const char* const*)pCmdL->GetBlock() );
#elif defined MTW
    nExit = spawnvp( P_WAIT, (char*)rPrePro.GetBuffer(), (char**)pCmdL->GetBlock() );
#else
    nExit = spawnvp( P_WAIT, (char*)rPrePro.GetBuffer(), (const char**)pCmdL->GetBlock() );
#endif

    if ( fRspFile )
        #if OSL_DEBUG_LEVEL > 5
        fprintf( stderr, "leaving response file %s\n", aRspFileName.GetBuffer() );
        #else
        unlink( aRspFileName.GetBuffer() );
        #endif
    if ( nExit )
        return sal_False;

    return sal_True;
}


/*************************************************************************
|*    CallRsc2
|*
|*    Beschreibung
*************************************************************************/
static sal_Bool CallRsc2( ByteString aRsc2Name,
                      RscStrList * pInputList,
                      ByteString aSrsName,
                      RscPtrPtr * pCmdLine )
{
    int             i, nExit;
    ByteString*     pString;
    ByteString      aRspFileName;   // Response-Datei
    FILE *          fRspFile;       // Response-Datei

    aRspFileName = ::GetTmpFileName();
    fRspFile = fopen( aRspFileName.GetBuffer(), "w" );

    RscVerbosity eVerbosity = RscVerbosityNormal;
    if( fRspFile )
    {
        for( i = 1; i < (int)(pCmdLine->GetCount() -1); i++ )
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
              || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-pp=", 4 )
              || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-rsc2=", 6 )
              || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-presponse", 9 )
              || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-rc", 3 )
              || !rsc_stricmp( (char *)pCmdLine->GetEntry( i ), "-+" )
              || !rsc_stricmp( (char *)pCmdLine->GetEntry( i ), "-br" )
              || !rsc_stricmp( (char *)pCmdLine->GetEntry( i ), "-bz" )
              || !rsc_stricmp( (char *)pCmdLine->GetEntry( i ), "-r" )
              // Am I the only one that thinks the following line inludes all the tests before?
              || ( '-' != *(char *)pCmdLine->GetEntry( i ) ) )
            {
            }
            else
#ifdef OS2
                fprintf( fRspFile, "%s\n",
#else
                fprintf( fRspFile, "%s ",
#endif
                         (const char *)pCmdLine->GetEntry( i ) );
        };

#ifdef OS2
        fprintf( fRspFile, "%s\n", aSrsName.GetBuffer() );
#else
        fprintf( fRspFile, aSrsName.GetBuffer() );
#endif

        pString = pInputList->First();
        while( pString )
        {
#ifdef OS2
            fprintf( fRspFile, "%s\n", pString->GetBuffer() );
#else
            fprintf( fRspFile, " %s", pString->GetBuffer() );
#endif
            pString = pInputList->Next();
        };

        fclose( fRspFile );
    };

    RscPtrPtr       aNewCmdL;       // Kommandozeile
    aNewCmdL.Append( rsc_strdup( aRsc2Name.GetBuffer() ) );
    ByteString aTmpStr( '@' );
    aTmpStr += aRspFileName;
    aNewCmdL.Append( rsc_strdup( aTmpStr.GetBuffer() ) );
    aNewCmdL.Append( (void *)0 );

    if ( eVerbosity >= RscVerbosityVerbose )
    {
        printf( "Rsc2 commandline: " );
        printf( "%s", (const char *)aNewCmdL.GetEntry( 0 ) );
        printf( " " );
        printf( "%s", (const char *)aNewCmdL.GetEntry( 1 ) );
        printf( "\n" );
    }

#if ((defined OS2 || defined WNT) && (defined TCPP || defined tcpp)) || defined UNX || defined OS2
    nExit = spawnvp( P_WAIT, aRsc2Name.GetBuffer(), (char* const*)aNewCmdL.GetBlock() );
#elif defined CSET
    nExit = spawnvp( P_WAIT, (char*)aRsc2Name.GetBuffer(), (char **)(const char**)aNewCmdL.GetBlock() );
#elif defined WTC
    nExit = spawnvp( P_WAIT, (char*)aRsc2Name.GetBuffer(), (const char* const*)aNewCmdL.GetBlock() );
#elif defined MTW
    nExit = spawnvp( P_WAIT, (char*)aRsc2Name.GetBuffer(), (char**)aNewCmdL.GetBlock() );
#else
    nExit = spawnvp( P_WAIT, (char*)aRsc2Name.GetBuffer(), (const char**)aNewCmdL.GetBlock() );
#endif

    if( fRspFile )
        #if OSL_DEBUG_LEVEL > 5
        fprintf( stderr, "leaving response file %s\n", aRspFileName.GetBuffer() );
        #else
        unlink( aRspFileName.GetBuffer() );
        #endif
    if( nExit )
        return( sal_False );
    return( sal_True );
}

/*************************************************************************
|*
|*    main()
|*
|*    Beschreibung
|*    Ersterstellung    MM 05.09.91
|*    Letzte Aenderung  MM 05.09.91
|*
*************************************************************************/
SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    sal_Bool            bPrePro  = sal_True;
    sal_Bool            bResFile = sal_True;
    sal_Bool            bHelp    = sal_False;
    sal_Bool            bError   = sal_False;
    sal_Bool            bResponse = sal_False;
    ByteString      aSolarbin(getenv("SOLARBINDIR"));
    ByteString      aDelim("/");
    ByteString      aPrePro; //( aSolarbin + aDelim + ByteString("rscpp"));
    ByteString      aRsc2Name; //(  aSolarbin + aDelim + ByteString("rsc2"));
    ByteString      aSrsName;
    ByteString      aResName;
    RscStrList      aInputList;
    RscStrList      aTmpList;
    char *          pStr;
    char **         ppStr;
    RscPtrPtr       aCmdLine;       // Kommandozeile
    sal_uInt32      i;
    ByteString*     pString;

    aPrePro = aSolarbin;
    aPrePro += aDelim;
    aPrePro += ByteString("rscpp");

    aRsc2Name = aSolarbin;
    aRsc2Name += aDelim;
    aRsc2Name += ByteString("rsc2");

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
            else if( !rsc_stricmp( (*ppStr) + 1, "r" )
              || !rsc_stricmp( (*ppStr) + 1, "s" ) )
            { // erzeugt kein .res-file
                bResFile = sal_False;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "h" ) )
            { // Hilfe anzeigen
                bHelp = sal_True;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "presponse", 9 ) )
            { // anderer Name fuer den Preprozessor
                bResponse = sal_True;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "pp=", 3 ) )
            { // anderer Name fuer den Preprozessor
                aPrePro = (*ppStr) + 4;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "rsc2=", 5 ) )
            { // Accept alternate name for the rsc2 compiler
                aRsc2Name = (*ppStr) + 6;
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
            aInputList.Insert( new ByteString( *ppStr ), CONTAINER_APPEND );
        }
        ppStr++;
        i++;
    }

    if( aInputList.Count() )
    {
        /* build the output file names          */
        if( ! aResName.Len() )
            aResName = OutputFile( *aInputList.First(), "res" );
        if( ! bSetSrs )
        {
            aSrsName = "-fp=";
            aSrsName += OutputFile( *aInputList.First(), "srs" );
        }
    };

    if( bHelp )
    {
        bPrePro = sal_False;
        bResFile = sal_False;
    };
    if( bPrePro && aInputList.Count() )
    {
        ByteString aTmpName;

        pString = aInputList.First();
        while( pString )
        {
            aTmpName = ::GetTmpFileName();
            if( !CallPrePro( aPrePro, *pString, aTmpName, &aCmdLine, bResponse ) )
            {
                printf( "Error starting preprocessor\n" );
                bError = sal_True;
                break;
            }
            aTmpList.Insert( new ByteString( aTmpName ), CONTAINER_APPEND );
            pString = aInputList.Next();
        };
    };

    if( !bError )
    {
        if( !CallRsc2( aRsc2Name, bPrePro ? &aTmpList : &aInputList,
                       aSrsName, &aCmdLine ) )
        {
            if( !bHelp )
            {
                printf( "Error starting rsc2 compiler\n" );
                bError = sal_True;
            }
        };
    };

    pString = aTmpList.First();
    while( pString )
    {
        #if OSL_DEBUG_LEVEL > 5
        fprintf( stderr, "leaving temp file %s\n", pString->GetBuffer() );
        #else
        unlink( pString->GetBuffer() );
        #endif
        pString = aTmpList.Next();
    };

    return( bError );
}

void RscExit( sal_uInt32 nExit )
{
    if( nExit )
        printf( "Program exit is %d\n", (int)nExit );
    exit( nExit );
}
