/*************************************************************************
 *
 *  $RCSfile: start.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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
#else
#ifndef MAC
#include <io.h>
#include <process.h>
#if defined ( OS2 ) && !defined ( GCC )
#include <direct.h>
#endif
#ifndef CSET
#include <dos.h>
#endif
#endif
#endif

#ifndef _RSCTOOLS_HXX
#include <rsctools.hxx>
#endif
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
        int rc;
        /*
        union wait rc;
        rc.w_status = 0;
        */

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
static BOOL CallPrePro( const ByteString& rPrePro,
                        const ByteString& rInput,
                        const ByteString& rOutput,
                        RscPtrPtr * pCmdLine,
                        BOOL bResponse )
{
    RscPtrPtr       aNewCmdL;   // Kommandozeile
    RscPtrPtr       aRespCmdL;   // Kommandozeile
    RscPtrPtr *     pCmdL = &aNewCmdL;
    short           i, nExit;
    FILE*           fRspFile = NULL;
    ByteString      aRspFileName;

    if( bResponse )
    {
        aRspFileName = ::GetTmpFileName();
        fRspFile = fopen( aRspFileName.GetBuffer(), "w" );
    }

    if( !fRspFile )
        aNewCmdL.Append( RscMem::Assignsw( rPrePro.GetBuffer(), 0 ) );
    for( i = 1; i < (short)(pCmdLine->GetCount() -1); i++ ){
        if( !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-u", 2 )
          || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-i", 2 )
          || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-d", 2 ) )
        {
            aNewCmdL.Append(
                RscMem::Assignsw( (char *)pCmdLine->GetEntry( i ), 0 ) );
        }
    };
    aNewCmdL.Append( RscMem::Assignsw( rInput.GetBuffer(), 0 ) );
    aNewCmdL.Append( RscMem::Assignsw( rOutput.GetBuffer(), 0 ) );
    aNewCmdL.Append( (void *)0 );

    printf( "Preprocessor commandline: " );
    for( i = 0; i < (short)(pCmdL->GetCount() -1); i++ )
    {
        printf( " " );
        printf( "%s", (const char *)pCmdL->GetEntry( i ) );
    }
    printf( "\n" );

    if( fRspFile )
    {
        aRespCmdL.Append( RscMem::Assignsw( rPrePro.GetBuffer(), 0 ) );
        ByteString aTmpStr( '@' );
        aTmpStr += aRspFileName;
        aRespCmdL.Append( RscMem::Assignsw( aTmpStr.GetBuffer(), 0 ) );
        aRespCmdL.Append( (void *)0 );

        pCmdL = &aRespCmdL;
        for( i = 0; i < (short)(aNewCmdL.GetCount() -1); i++ )
        {
            fprintf( fRspFile, "%s ", (const char *)aNewCmdL.GetEntry( i ) );
        }
        fclose( fRspFile );

        printf( "Preprocessor startline: " );
        for( i = 0; i < (short)(pCmdL->GetCount() -1); i++ )
        {
            printf( " " );
            printf( "%s", (const char *)pCmdL->GetEntry( i ) );
        }
        printf( "\n" );
    }

#if ((defined PM2 || defined WNT) && (defined TCPP || defined tcpp)) || defined UNX
    nExit = spawnvp( P_WAIT, rPrePro.GetBuffer(), (char* const*)pCmdL->GetBlock() );
#elif defined CSET
    nExit = spawnvp( P_WAIT, (char*)rPrePro.GetBuffer(), char **) (const char**)pCmdL->GetBlock() );
#elif defined WTC
    nExit = spawnvp( P_WAIT, (char*)rPrePro.GetBuffer(), (const char* const*)pCmdL->GetBlock() );
#elif defined MTW
    nExit = spawnvp( P_WAIT, (char*)rPrePro.GetBuffer(), (char**)pCmdL->GetBlock() );
#else
    nExit = spawnvp( P_WAIT, (char*)rPrePro.GetBuffer(), (const char**)pCmdL->GetBlock() );
#endif

    if ( fRspFile )
        unlink( aRspFileName.GetBuffer() );
    if ( nExit )
        return FALSE;

    return TRUE;
}


/*************************************************************************
|*    CallRsc2
|*
|*    Beschreibung
*************************************************************************/
static BOOL CallRsc2( ByteString aRsc2Name,
                      RscStrList * pInputList,
                      ByteString aSrsName,
                      ByteString aRcName,
                      RscPtrPtr * pCmdLine )
{
    RscPtrPtr       aNewCmdL;       // Kommandozeile
    short           i, nExit;
    ByteString*     pString;
    ByteString      aRspFileName;   // Response-Datei
    FILE *          fRspFile;       // Response-Datei

    aRspFileName = ::GetTmpFileName();
    fRspFile = fopen( aRspFileName.GetBuffer(), "w" );

    printf( "Rsc2 commandline: " );
    aNewCmdL.Append( RscMem::Assignsw( aRsc2Name.GetBuffer(), 0 ) );
    printf( "%s", (const char *)aNewCmdL.GetEntry( aNewCmdL.GetCount() -1 ) );
    printf( " " );
    ByteString aTmpStr( '@' );
    aTmpStr += aRspFileName;
    aNewCmdL.Append( RscMem::Assignsw( aTmpStr.GetBuffer(), 0 ) );
    printf( "%s", (const char *)aNewCmdL.GetEntry( aNewCmdL.GetCount() -1 ) );
    aNewCmdL.Append( (void *)0 );
    printf( "\n" );

    if( fRspFile )
    {
        for( i = 1; i < (short)(pCmdLine->GetCount() -1); i++ )
        {
            if( !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ),  "-fp", 3 )
              || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-fo", 3 )
              || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-fs", 3 )
              || !rsc_strnicmp( (char *)pCmdLine->GetEntry( i ), "-pp", 3 )
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
                fprintf( fRspFile, "%s ",
                         (const char *)pCmdLine->GetEntry( i ) );
        };

        fprintf( fRspFile, "%s -fs%s",
                 aSrsName.GetBuffer(), aRcName.GetBuffer() );

        pString = pInputList->First();
        while( pString )
        {
            fprintf( fRspFile, " %s", pString->GetBuffer() );
            pString = pInputList->Next();
        };

        fclose( fRspFile );
    };

#if ((defined PM2 || defined WNT) && (defined TCPP || defined tcpp)) || defined UNX
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
        unlink( aRspFileName.GetBuffer() );
    if( nExit )
        return( FALSE );
    return( TRUE );
}

/*************************************************************************
|*    CallRes
|*
|*    Beschreibung
*************************************************************************/
static BOOL CallRes( ByteString aRcName, ByteString aResName )
{
    short       nExit;

    unlink( aResName.GetBuffer() );   // Zieldatei loeschen
    printf( "Copy %s to %s\n", aRcName.GetBuffer(), aResName.GetBuffer() );
    if( !Append( aResName, aRcName ) )
    {
        printf( "Cannot open file <%s or %s>\n",
                aResName.GetBuffer(), aRcName.GetBuffer() );
        nExit = 1;
    }
    else
        nExit = 0;
    return( nExit == 0 );
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
#if defined UNX || (defined PM2 && (defined CSET || defined GCC )) || defined WTC || defined MTW || defined ICC
int main ( int argc, char ** argv)
{
#else
int cdecl main ( int argc, char ** argv)
{
#endif

    BOOL            bPrePro  = TRUE;
    BOOL            bResFile = TRUE;
    BOOL            bHelp    = FALSE;
    BOOL            bError   = FALSE;
    BOOL            bResponse = FALSE;
    ByteString      aPrePro( "rscpp" );
    ByteString      aRsc2Name( "rsc2" );
    ByteString      aSrsName;
    ByteString      aRcName;
    ByteString      aResName;
    RscStrList      aInputList;
    RscStrList      aTmpList;
    char *          pStr;
    char **         ppStr;
    RscPtrPtr       aCmdLine;       // Kommandozeile
    USHORT          i;
    ByteString*     pString;

    printf( "VCL Resource Compiler 3.0\n" );

    pStr = ::ResponseFile( &aCmdLine, argv, argc );
    if( pStr )
    {
        printf( "Cannot open response file <%s>\n", pStr );
        return( 1 );
    };

    ppStr  = (char **)aCmdLine.GetBlock();
    ppStr++;
    i = 1;
    BOOL bSetSrs = FALSE;
    while( ppStr && i < (USHORT)(aCmdLine.GetCount() -1) )
    {
        if( '-' == **ppStr )
        {
            if( !rsc_stricmp( (*ppStr) + 1, "p" )
              || !rsc_stricmp( (*ppStr) + 1, "l" ) )
            { // kein Preprozessor
                bPrePro = FALSE;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "r" )
              || !rsc_stricmp( (*ppStr) + 1, "s" ) )
            { // erzeugt kein .res-file
                bResFile = FALSE;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "h" ) )
            { // Hilfe anzeigen
                bHelp = TRUE;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "presponse", 9 ) )
            { // anderer Name fuer den Preprozessor
                bResponse = TRUE;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "pp", 2 ) )
            { // anderer Name fuer den Preprozessor
                aPrePro = (*ppStr) + 3;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fo", 2 ) )
            { // anderer Name fuer .res-file
                aResName = (*ppStr) + 3;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fs", 2 ) )
            { // anderer Name fuer .rc-file
                aRcName = (*ppStr) + 3;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fp", 2 ) )
            { // anderer Name fuer .srs-file
                bSetSrs  = TRUE;
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
        if( ! aRcName.Len() )
            aRcName  = OutputFile( *aInputList.First(), "rc"  );
        if( ! bSetSrs )
        {
            aSrsName = "-fp";
            aSrsName += OutputFile( *aInputList.First(), "srs" );
        }
    };

    if( bHelp )
    {
        bPrePro = FALSE;
        bResFile = FALSE;
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
                bError = TRUE;
                break;
            }
            aTmpList.Insert( new ByteString( aTmpName ), CONTAINER_APPEND );
            pString = aInputList.Next();
        };
    };

    if( !bError )
    {
        if( !CallRsc2( aRsc2Name, bPrePro ? &aTmpList : &aInputList,
                       aSrsName, aRcName, &aCmdLine ) )
        {
            if( !bHelp )
            {
                printf( "Error starting rsc2 compiler\n" );
                bError = TRUE;
            }
        };
    };

    pString = aTmpList.First();
    while( pString )
    {
        unlink( pString->GetBuffer() );
        pString = aTmpList.Next();
    };

    if( !bError && bResFile )
    {
        if( !CallRes( aRcName, aResName ) )
        {
            printf( "Error in system resource compiler\n" );
            bError = TRUE;
        }
        else
            unlink( aRcName.GetBuffer() );
    };

    return( bError );
}

void RscExit( USHORT nExit )
{
    if( nExit )
        printf( "Program exit is %d\n", nExit );
    exit( nExit );
}
