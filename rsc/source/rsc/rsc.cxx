/*************************************************************************
 *
 *  $RCSfile: rsc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:56 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/rsc/rsc.cxx,v 1.1.1.1 2000-09-18 16:42:56 hr Exp $

**************************************************************************/
/****************************************************************/
/*                  Include File                                */
/****************************************************************/
#ifdef MAC
#include "mac_start.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#ifdef UNX
#include <unistd.h>
#include <sys/wait.h>
#else
#ifndef MAC
#include <io.h>
#include <process.h>
#include <direct.h>
#endif
#endif
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef MAC
#ifdef MAC_UNIVERSAL
#ifndef _UNISTD
  #include <unistd.h>
#endif
#endif
#include "mac_end.h"
#endif

#if defined( PM2 ) && defined( ZTC )
#include <svpm.h>
#ifndef unlink
#define unlink( p ) DosDelete( (PSZ)(const char*)p )
#endif
#endif

#include <tools/fsys.hxx>

#ifndef _RSCERROR_H
#include <rscerror.h>
#endif
#ifndef _RSCTOP_HXX
#include <rsctop.hxx>
#endif
#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif
#ifndef _RSCPAR_HXX
#include <rscpar.hxx>
#endif
#ifndef _RSCRSC_HXX
#include <rscrsc.hxx>
#endif

#include <rtl/tencinfo.h>
#include <rtl/textenc.h>
/*************** F o r w a r d s *****************************************/
/*************** G l o b a l e   V a r i a b l e n **********************/
HashString*     pHS          = NULL;
ByteString*     pStdParType  = NULL;
ByteString*     pStdPar1     = NULL;
ByteString*     pStdPar2     = NULL;
ByteString*     pWinParType  = NULL;
ByteString*     pWinPar1     = NULL;
ByteString*     pWinPar2     = NULL;
USHORT          nRefDeep     = 10;

/*************** R s c C m d L i n e ************************************/
/*************************************************************************
|*
|*    RscCmdLine::Init()
|*
|*    Beschreibung      Kommandozeile interpretierten
|*    Ersterstellung    MM 03.05.91
|*    Letzte Aenderung  MM 03.05.91
|*
*************************************************************************/
void RscCmdLine::Init()
{
    nSourceCharSet  = RTL_TEXTENCODING_MS_1252;
    nCommands       = 0;
    nLangTypeId     = LANGUAGE_DONTKNOW;
    nByteOrder      = RSC_BIGENDIAN;

    DirEntry aEntry;
    aPath = ByteString( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US ); //Immer im Aktuellen Pfad suchen
}

/*************************************************************************
|*
|*    RscCmdLine::RscCmdLine()
|*
|*    Beschreibung      Kommandozeile interpretierten
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
RscCmdLine::RscCmdLine()
{
    Init();
}

/*************************************************************************
|*
|*    RscCmdLine::RscCmdLine()
|*
|*    Beschreibung      Kommandozeile interpretierten
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
RscCmdLine::RscCmdLine( short argc, char ** argv, RscError * pEH )
{
    char *          pStr;
    char **         ppStr;
    RscPtrPtr       aCmdLine;       // Kommandozeile
    ByteString      aString;
    USHORT          i;
    BOOL            bOutputSrsIsSet = FALSE;

    Init(); // Defaults setzen

    pStr = ::ResponseFile( &aCmdLine, argv, argc );
    if( pStr )
        pEH->FatalError( ERR_OPENFILE, RscId(), pStr );

    /* check the inputted switches       */
    ppStr  = (char **)aCmdLine.GetBlock();
    ppStr++;
    i = 1;
    while( ppStr && i < (USHORT)(aCmdLine.GetCount() -1) )
    {
        if( '-' == **ppStr )
        {
            if( !rsc_stricmp( (*ppStr) + 1, "h" )
              || !strcmp( (*ppStr) + 1, "?" ) )
            { // Hilfe
                nCommands |= HELP_FLAG;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "syntax" ) )
            { // Hilfe
                nCommands |= PRINTSYNTAX_FLAG;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "RefDeep", 7 ) )
            { // maximale Aufloesungtiefe fuer Referenzen
                nRefDeep = (short)(ByteString( (*ppStr) +1 + strlen( "RefDeep" ) ).ToInt32());
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "p" ) )
            { // kein Preprozessor
                nCommands |= NOPREPRO_FLAG;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "s" ) )
            { // nicht linken
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
            else if( !rsc_stricmp( (*ppStr) + 1, "PreLoad" ) )
            { // Alle Ressourcen mit Preload
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
            else if( !rsc_stricmp( (*ppStr) + 1, "SMART" ) )
            { // Byte Ordnung beim Schreiben
                nCommands |= SMART_FLAG;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "d", 1 ) )
            { // Symbole definieren
                nCommands |= DEFINE_FLAG;
                aSymbolList.Insert( new ByteString( (*ppStr) + 2 ), 0xFFFF );
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "i", 1 ) )
            { // Include-Pfade definieren
                nCommands |= INCLUDE_FLAG;
                if( aPath.Len() )
                    aPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_ASCII_US );
                aPath += (*ppStr) + 2;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fs", 2 ) )
            { // anderer Name fuer .rc-file
                aOutputRc = (*ppStr) + 3;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fp", 2 ) )
            { // anderer Name fuer .srs-file
                aOutputSrs = (*ppStr) + 3;
                bOutputSrsIsSet = TRUE;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fl", 2 ) )
            { // Name fuer listing-file
                aOutputLst = (*ppStr) + 3;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fh", 2 ) )
            { // Name fuer .hxx-file
                aOutputHxx = (*ppStr) + 3;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fc", 2 ) )
            { // Name fuer .cxx-file
                aOutputCxx = (*ppStr) + 3;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fr", 2 ) )
            { // Name fuer .cxx-file der Ressource Konstruktoren
                aOutputRcCtor = (*ppStr) + 3;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fx", 2 ) )
            { // Name fuer .src-file
                aOutputSrc = (*ppStr) + 3;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "NoSysResTest" ) )
            { // Bitmap, Pointers, Icons nicht ueberpruefen
                nCommands |= NOSYSRESTEST_FLAG;
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "SrsDefault" ) )
            { // Bitmap, Pointers, Icons nicht ueberpruefen
                nCommands |= SRSDEFAULT_FLAG;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "CHARSET_", 8 ) )
            {
                if( !rsc_stricmp( (*ppStr) + 9, "ANSI" ) )
                     nSourceCharSet = RTL_TEXTENCODING_MS_1252;
                else if( !rsc_stricmp( (*ppStr) + 9, "MAC" ) )
                     nSourceCharSet = RTL_TEXTENCODING_APPLE_ROMAN;
                else if( !rsc_stricmp( (*ppStr) + 9, "IBMPC_437" ) )
                     nSourceCharSet = RTL_TEXTENCODING_IBM_437;
                else if( !rsc_stricmp( (*ppStr) + 9, "IBMPC_850" ) )
                     nSourceCharSet = RTL_TEXTENCODING_IBM_850;
                else if( !rsc_stricmp( (*ppStr) + 9, "IBMPC_860" ) )
                     nSourceCharSet = RTL_TEXTENCODING_IBM_860;
                else if( !rsc_stricmp( (*ppStr) + 9, "IBMPC_861" ) )
                     nSourceCharSet = RTL_TEXTENCODING_IBM_861;
                else if( !rsc_stricmp( (*ppStr) + 9, "IBMPC_863" ) )
                     nSourceCharSet = RTL_TEXTENCODING_IBM_863;
                else if( !rsc_stricmp( (*ppStr) + 9, "IBMPC_865" ) )
                     nSourceCharSet = RTL_TEXTENCODING_IBM_865;
                else if( !rsc_stricmp( (*ppStr) + 9, "IBMPC" ) )
                     nSourceCharSet = RTL_TEXTENCODING_IBM_850;
                else if( !rsc_stricmp( (*ppStr) + 9, "DONTKNOW" ) )
                     nSourceCharSet = RTL_TEXTENCODING_DONTKNOW;
                else
                {
                    nSourceCharSet = rtl_getTextEncodingFromUnixCharset( (*ppStr) + 9 );
                    if( nSourceCharSet == 0 )
                    {
                        nSourceCharSet = RTL_TEXTENCODING_MS_1252;
                        pEH->FatalError( ERR_UNKNOWNSW, RscId(), *ppStr );
                        break;
                    }
                }
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "lg" ) )
            {
                nLangTypeId = LANGUAGE_DONTKNOW;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "lg", 2 ) )
            {
                nLangTypeId = LANGUAGE_DONTKNOW;
#define LT(Name)                                            \
                if( !rsc_stricmp( (*ppStr) + 3, #Name ) )   \
                    nLangTypeId = LANGUAGE_##Name
                LT( SYSTEM              );
#include <rsclang.c>
                LT( USER1               );
                LT( USER2               );
                LT( USER3               );
                LT( USER4               );
                LT( USER5               );
                LT( USER6               );
                LT( USER7               );
                LT( USER8               );
                LT( USER9               );
                if( nLangTypeId == LANGUAGE_DONTKNOW )
                    pEH->FatalError( ERR_UNKNOWNSW, RscId(), *ppStr );
            }
            else
                pEH->FatalError( ERR_UNKNOWNSW, RscId(), *ppStr );
        }
        else
        {
            // Eingabedatei
            aInputList.Insert( new ByteString( *ppStr ), 0xFFFF );
        }
        ppStr++;
        i++;
    }

    if( nCommands & HELP_FLAG )
        pEH->FatalError( ERR_USAGE, RscId() );
    // was an inputted file specified
    else if( aInputList.Count() )
    {
        if( ! aOutputRc.Len() )
            aOutputRc  = OutputFile( *aInputList.First(), "rc"  );
        if( ! bOutputSrsIsSet )
            aOutputSrs = OutputFile( *aInputList.First(), "srs" );
    }
    else if( !(nCommands & PRINTSYNTAX_FLAG) )
        pEH->FatalError( ERR_NOINPUT, RscId() );
}

/*************************************************************************
|*
|*    RscCmdLine::SetInputFile()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.04.91
|*    Letzte Aenderung  MM 22.04.91
|*
*************************************************************************/
void RscCmdLine::SetInputFile( const ByteString & rInputName )
{
    ByteString * pString;

    while( NULL != (pString = aInputList.Remove( (ULONG)0 )) )
        delete pString;
    aInputList.Insert( new ByteString( rInputName ) );
}

/*************************************************************************
|*
|*    RscCmdLine::~RscCmdLine()
|*
|*    Beschreibung      dtor
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
RscCmdLine::~RscCmdLine()
{
    ByteString  *pString;

    while( NULL != (pString = aInputList.Remove( (ULONG)0 )) )
        delete pString;
    while( NULL != (pString = aSymbolList.Remove( (ULONG)0 )) )
        delete pString;
}

/*************** R s c C o m p i l e r **********************************/
/****************************************************************/
/*                                                              */
/*  RscCompiler :: RscCompiler(int argc, char **argv)           */
/*                                                              */
/*  Parameters  :   argc - number of parameters on command line */
/*                  argv - arry of pointers to input parameters */
/*                                                              */
/*  Description :   main calling routine. Calls functions to    */
/*  check and assign the input parameters. It then builds the   */
/*  command line to call the Glockenspiel preprocessor          */
/****************************************************************/

RscCompiler::RscCompiler( RscCmdLine * pLine, RscTypCont * pTypCont )
{
    fListing      = NULL;
    fExitFile     = NULL;

    //Kommandozeile setzen, TypContainer setzen
    pCL = pLine;
    pTC = pTypCont;

    if( pCL->aOutputLst.Len() )
    {
        if ( NULL == (fListing = fopen( pCL->aOutputLst.GetBuffer(), "w" )) )
            pTC->pEH->FatalError( ERR_OPENFILE, RscId(), pCL->aOutputLst.GetBuffer() );
        pTC->pEH->SetListFile( fListing );
    }
}

/*************************************************************************
|*
|*    RscCompiler :: RscCompiler()
|*
|*    Beschreibung
|*    Ersterstellung    MM 07.02.91
|*    Letzte Aenderung  MM 07.02.91
|*
*************************************************************************/
RscCompiler::~RscCompiler()
{
    ByteString* pString;

    // Dateien loeschen
    pString = aTmpFileList.First();
    while( pString )
    {
        unlink( pString->GetBuffer() );
        delete pString;
        pString = aTmpFileList.Next();
    }

    pTC->pEH->SetListFile( NULL );

    if( fListing )
        fclose( fListing );

    if( fExitFile )
        fclose( fExitFile );
    if( aTmpOutputRc.Len() )
        unlink( aTmpOutputRc.GetBuffer() );
    if( aTmpOutputHxx.Len() )
        unlink( aTmpOutputHxx.GetBuffer() );
    if( aTmpOutputCxx.Len() )
        unlink( aTmpOutputCxx.GetBuffer() );
    if( aTmpOutputRcCtor.Len() )
        unlink( aTmpOutputRcCtor.GetBuffer() );
    if( aTmpOutputSrc.Len() )
        unlink( aTmpOutputSrc.GetBuffer() );
}

/*************************************************************************
|*
|*    RscCompiler::Start()
|*
|*    Beschreibung      Datei in Kommandozeile aendern
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
ERRTYPE RscCompiler::Start()
{
    ERRTYPE         aError;
    ByteString*     pString;
    RscFile*        pFName;

    if( PRINTSYNTAX_FLAG & pCL->nCommands )
    {
#ifndef W30
        pTC->WriteSyntax( stdout );
printf( "khg\n" );
#endif
        return ERR_OK;
    }

    // Kein Parameter, dann Hilfe
    pString = pCL->aInputList.First();
    if( !pString )
        pTC->pEH->FatalError( ERR_NOINPUT, RscId() );

    while( pString )
    {
        pTC->aFileTab.NewCodeFile( *pString );
        pString = pCL->aInputList.Next();
    }

    if( !(pCL->nCommands & NOSYNTAX_FLAG) )
    {
        if( pCL->nCommands & NOPREPRO_FLAG )
        {

            pTC->pEH->SetListFile( NULL );

            pFName = pTC->aFileTab.First();
            while( pFName && aError.IsOk() )
            {
                if( !pFName->bScanned && !pFName->IsIncFile() )
                {
                    aError = IncludeParser(
                                 pTC->aFileTab.GetIndex( pFName )
                             );
                    // Currentzeiger richtig setzen
                    pTC->aFileTab.Seek( pFName );
                };
                pFName = pTC->aFileTab.Next();
            };

            pTC->pEH->SetListFile( fListing );
        }
    };

    pTC->pEH->StdOut( "Files: " );
    pFName = pTC->aFileTab.First();
    while( pFName )
    {
        pTC->pEH->StdOut( pFName->aFileName.GetBuffer() );
        pTC->pEH->StdOut( " " );
        pFName = pTC->aFileTab.Next();
    };
    pTC->pEH->StdOut( "\n" );

    if( aError.IsOk() )
        aError = CheckSyntax();

    if( aError.IsOk() )
        aError = Link();

    if( aError.IsOk() )
        EndCompile();

    if( aError.IsError() )
        pTC->pEH->Error( ERR_ERROR, NULL, RscId() );

    return( aError );
}
/*************************************************************************
|*
|*    RscCmdLine::EndCompile()
|*
|*    Beschreibung      Datei in Kommandozeile aendern
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
void RscCompiler::EndCompile()
{
    if( pCL->aOutputSrs.Len() && (pCL->nCommands & NOLINK_FLAG) )
    {
        pTC->pEH->StdOut( "Writing file " );
        pTC->pEH->StdOut( pCL->aOutputSrs.GetBuffer() );
        pTC->pEH->StdOut( ".\n" );

        // kopiere von TMP auf richtigen Namen
        unlink( pCL->aOutputSrs.GetBuffer() );   // Zieldatei loeschen
        if( !(pCL->nCommands & NOSYNTAX_FLAG) )
        {
            FILE        * foutput;
            RscFile     * pFN;

            if( NULL == (foutput = fopen( pCL->aOutputSrs.GetBuffer(), "w" )) )
                pTC->pEH->FatalError( ERR_OPENFILE, RscId(), pCL->aOutputSrs.GetBuffer() );
            else
            {
                // Schreibe Datei
                pFN = pTC->aFileTab.First();
                while( pFN )
                {
                    if( !pFN->IsIncFile() )
                    {
                        pTC->WriteSrc( foutput, NOFILE_INDEX,
                                       RTL_TEXTENCODING_UNICODE, FALSE );
                        break; // ?T 281091MM nur eine Src-Datei
                    }
                };

                fclose( foutput );
            };
        };
    }

    if ( aTmpOutputHxx.Len() )
    {
        pTC->pEH->StdOut( "Writing file " );
        pTC->pEH->StdOut( pCL->aOutputHxx.GetBuffer() );
        pTC->pEH->StdOut( ".\n" );

        // kopiere von TMP auf richtigen Namen
        unlink( pCL->aOutputHxx.GetBuffer() );   // Zieldatei loeschen
        Append( pCL->aOutputHxx, aTmpOutputHxx );
        unlink( aTmpOutputHxx.GetBuffer() );// TempDatei  loeschen
        aTmpOutputHxx = ByteString();
    }

    if( aTmpOutputCxx.Len() )
    {
        pTC->pEH->StdOut( "Writing file " );
        pTC->pEH->StdOut( pCL->aOutputCxx.GetBuffer() );
        pTC->pEH->StdOut( ".\n" );

        // kopiere von TMP auf richtigen Namen
        unlink( pCL->aOutputCxx.GetBuffer() );   // Zieldatei loeschen
        Append( pCL->aOutputCxx, aTmpOutputCxx );
        unlink( aTmpOutputCxx.GetBuffer() );// TempDatei  loeschen
        aTmpOutputCxx = ByteString();
    }

    if( aTmpOutputRcCtor.Len() )
    {
        pTC->pEH->StdOut( "Writing file " );
        pTC->pEH->StdOut( pCL->aOutputRcCtor.GetBuffer() );
        pTC->pEH->StdOut( ".\n" );

        // kopiere von TMP auf richtigen Namen
        unlink( pCL->aOutputRcCtor.GetBuffer() );   // Zieldatei loeschen
        Append( pCL->aOutputRcCtor, aTmpOutputRcCtor );
        unlink( aTmpOutputRcCtor.GetBuffer() );// TempDatei  loeschen
        aTmpOutputRcCtor = ByteString();
    }

    if( aTmpOutputSrc.Len() )
    {
        // kopiere von TMP auf richtigen Namen
        unlink( pCL->aOutputSrc.GetBuffer() );   // Zieldatei loeschen
        Append( pCL->aOutputSrc, aTmpOutputSrc );
        unlink( aTmpOutputSrc.GetBuffer() );// TempDatei  loeschen
        aTmpOutputSrc = ByteString();
    }

    if( aTmpOutputRc.Len() )
    {
        // kopiere von TMP auf richtigen Namen
        unlink( pCL->aOutputRc.GetBuffer() );    // Zieldatei loeschen
        Append( pCL->aOutputRc, aTmpOutputRc );
        unlink( aTmpOutputRc.GetBuffer() ); // TempDatei  loeschen
        aTmpOutputRc = ByteString();
    }
}

/*************************************************************************
|*
|*    RscCompiler::IncludeParser()
|*
|*    Beschreibung
|*    Ersterstellung    MM 21.06.91
|*    Letzte Aenderung  MM 21.06.91
|*
*************************************************************************/
ERRTYPE RscCompiler :: IncludeParser( ULONG lFileKey )
{
    FILE            * finput;
    RscFile         * pFName;
    ERRTYPE           aError;

    pFName = pTC->aFileTab.Get( lFileKey );
    if( !pFName )
        aError = ERR_ERROR;
    else if( !pFName->bScanned )
    {
        finput = fopen( pFName->aPathName.GetBuffer(), "r" );
        if( !finput )
        {
            aError = ERR_OPENFILE;
            pTC->pEH->Error( aError, NULL, RscId(),
                             pFName->aPathName.GetBuffer() );
        }
        else
        {
            RscFile         * pFNTmp;
            ByteString        aPathName;
            RscDepend       * pDep;
            RscFileInst       aFileInst( pTC, lFileKey, lFileKey,
                                         finput, pCL->nSourceCharSet );

            pFName->bScanned = TRUE;
            ::IncludeParser( &aFileInst );
            fclose( finput );

            // Include-Pfad durchsuchen
            pDep = pFName->First();
            while( pDep )
            {
                pFNTmp = pTC->aFileTab.GetFile( pDep->GetFileKey() );
                pDep = pFName->Next();
            }

            pDep = pFName->First();
            while( pDep )
            {
                pFNTmp = pTC->aFileTab.GetFile( pDep->GetFileKey() );
                // Kein Pfad und Include Datei
                if( pFNTmp && !pFNTmp->bLoaded )
                {
                    UniString aUniFileName( pFNTmp->aFileName, RTL_TEXTENCODING_ASCII_US );
                    DirEntry aFullName( aUniFileName );
                    if ( aFullName.Find( UniString( pCL->aPath, RTL_TEXTENCODING_ASCII_US ) ) )
                        pFNTmp->aPathName = ByteString( aFullName.GetFull(), RTL_TEXTENCODING_ASCII_US );
                    else
                        aError = ERR_OPENFILE;
                }
                pDep = pFName->Next();
            };
        };
    };

    return aError;
}

/*************************************************************************
|*
|*    RscCompiler :: ParseOneFile()
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.06.91
|*    Letzte Aenderung  MM 26.06.91
|*
*************************************************************************/
ERRTYPE RscCompiler :: ParseOneFile( ULONG lFileKey )
{
    FILE *              finput = NULL;
    ERRTYPE             aError;
    RscFile           * pFName;

    pFName = pTC->aFileTab.Get( lFileKey );
    if( !pFName )
        aError = ERR_ERROR;
    else if( !pFName->bLoaded )
    {
        RscDepend  * pDep;

        //Include-Dateien vorher lesen
        pFName->bLoaded = TRUE; //Endlos Rekursion vermeiden
        pDep = pFName->First();
        while( pDep && aError.IsOk() )
        {
            aError = ParseOneFile( pDep->GetFileKey() );
            pFName->Seek( pDep );
            pDep = pFName->Next();
        }

        if( aError.IsError() )
            pFName->bLoaded = FALSE; //bei Fehler nicht geladenen
        else
        {
            finput = fopen( pFName->aPathName.GetBuffer(), "r" );
            if( !finput )
            {
                pTC->pEH->Error( ERR_OPENFILE, NULL, RscId(),
                                 pFName->aPathName.GetBuffer() );
                aError = ERR_OPENFILE;
            }
            else
            {
                RscFileInst aFileInst( pTC, lFileKey, lFileKey, finput,
                                       pCL->nSourceCharSet );
                // Parser schreibt Punkte fuer jedes Objekt auf
                // unterster Ebene nach stdout
                pTC->pEH->StdOut( "reading file " );
                pTC->pEH->StdOut( pFName->aPathName.GetBuffer() );
                pTC->pEH->StdOut( " " );

                aError = ::parser( &aFileInst );
                if( aError.IsError() )
                    pTC->Delete( lFileKey );//Resourceobjekte loeschen
                pTC->pEH->StdOut( "\n" );
                fclose( finput );
            };
        };
    };

    return( aError );
}

/*************************************************************************
|*
|*    RscCompiler :: CheckSyntax()
|*
|*    Beschreibung
|*    Ersterstellung    MM 07.02.91
|*    Letzte Aenderung  MM 07.02.91
|*
*************************************************************************/
ERRTYPE RscCompiler::CheckSyntax(){
    ERRTYPE             aError;
    RscFile        *    pFName;

    // Parser schreibt Punkte fuer jedes Objekt auf
    // unterster Ebene nach stdout

    //Eingabedateien liegen als Liste vor
    pFName = pTC->aFileTab.First();
    while( pFName && aError.IsOk() ){
        if( !pFName->IsIncFile() ){
            aError = ParseOneFile( pTC->aFileTab.GetIndex( pFName ) );
            // Currentzeiger richtig setzen
            pTC->aFileTab.Seek( pFName );
        };
        pFName = pTC->aFileTab.Next();
    };

    return( aError );
}

/*************************************************************************
|*
|*    RscCompiler :: Link()
|*
|*    Beschreibung
|*    Ersterstellung    MM 07.02.91
|*    Letzte Aenderung  MM 07.02.91
|*
*************************************************************************/
ERRTYPE RscCompiler::Link()
{
    FILE *  foutput;
    ERRTYPE aError;

    if( !(pCL->nCommands & NOLINK_FLAG) )
    {
        // rc-Datei schreiben
        aTmpOutputRc = ::GetTmpFileName();
        if ( NULL == (fExitFile = foutput = fopen( aTmpOutputRc.GetBuffer(), "wb" )) )
            pTC->pEH->FatalError( ERR_OPENFILE, RscId(), aTmpOutputRc.GetBuffer() );

        pTC->pEH->StdOut( "Generating .rc file\n" );

        // Schreibe Datei
        aError = pTC->WriteRc( foutput );

        fclose( foutput );
        fExitFile = NULL;
    };

    // hxx-Datei schreiben
    if( pCL->aOutputHxx.Len() && aError.IsOk() )
    {
        aTmpOutputHxx = ::GetTmpFileName();
        if ( NULL == (fExitFile = foutput = fopen( aTmpOutputHxx.GetBuffer(), "w" )) )
            pTC->pEH->FatalError( ERR_OPENFILE, RscId(), aTmpOutputHxx.GetBuffer() );

        pTC->pEH->StdOut( "Generating .hxx file\n" );

        // Schreibe Datei
        aError = pTC->WriteHxx( foutput, NOFILE_INDEX );

        fclose( foutput );
        fExitFile = NULL;
    }

    // cxx-Datei schreiben
    if( pCL->aOutputCxx.Len() && aError.IsOk() )
    {
        aTmpOutputCxx = ::GetTmpFileName();
        if ( NULL == (fExitFile = foutput = fopen( aTmpOutputCxx.GetBuffer(), "w" )) )
            pTC->pEH->FatalError( ERR_OPENFILE, RscId(), aTmpOutputCxx.GetBuffer() );

        pTC->pEH->StdOut( "Generating .cxx file\n" );

        ByteString aHxx = pCL->aOutputHxx;
        if( !aHxx.Len() )
        {
            UniString aUniOutputCxx( pCL->aOutputCxx, RTL_TEXTENCODING_ASCII_US );
            aHxx = ByteString( DirEntry( aUniOutputCxx ).GetBase(), RTL_TEXTENCODING_ASCII_US );
            aHxx += ".hxx";
        }

        // Schreibe Datei
        aError = pTC->WriteCxx( foutput, NOFILE_INDEX, aHxx );

        fclose( foutput );
        fExitFile = NULL;
    }

    // RcCtor-Datei schreiben
    if( pCL->aOutputRcCtor.Len() && aError.IsOk() )
    {
        aTmpOutputRcCtor = ::GetTmpFileName();
        if ( NULL == (fExitFile = foutput = fopen( aTmpOutputRcCtor.GetBuffer(), "w" )) )
            pTC->pEH->FatalError( ERR_OPENFILE, RscId(), aTmpOutputRcCtor.GetBuffer() );

        pTC->pEH->StdOut( "Generating .cxx ressource constructor file\n" );

        // Schreibe Datei
        pTC->WriteRcCtor( foutput );

        fclose( foutput );
        fExitFile = NULL;
    }

    // src-Datei schreiben
    if( pCL->aOutputSrc.Len() && aError.IsOk() )
    {
        aTmpOutputSrc = ::GetTmpFileName();
        if ( NULL == (fExitFile = foutput = fopen( aTmpOutputSrc.GetBuffer(), "w" )) )
            pTC->pEH->FatalError( ERR_OPENFILE, RscId(), aTmpOutputSrc.GetBuffer() );

        // Schreibe Datei
        pTC->WriteSrc( foutput, NOFILE_INDEX, RTL_TEXTENCODING_UNICODE );

        fclose( foutput );
        fExitFile = NULL;
    };

    return( aError );
}

/********************************************************************/
/*                                                                  */
/*  Function    :   Append( )                                       */
/*                                                                  */
/*  Parameters  :   psw     - pointer to a preprocessor switch      */
/*                                                                  */
/*  Description :   appends text files                              */
/********************************************************************/
void RscCompiler::Append( const ByteString& rOutputSrs,
                          const ByteString& rTmpFile )
{
    if( !::Append( rOutputSrs, rTmpFile ) )
    {
        ByteString aTemp = rOutputSrs;
        aTemp += " or ";
        aTemp += rTmpFile;
        pTC->pEH->FatalError( ERR_OPENFILE, RscId(), aTemp.GetBuffer() );
    }
}

/********************************************************************/
/*                                                                  */
/*  Function    :   GetTmpFileName()                                */
/*                                                                  */
/*  Description :   Packt einen Dateinamen in Tmp-Dateiliste.       */
/*                                                                  */
/********************************************************************/
ByteString RscCompiler::GetTmpFileName()
{
    ByteString aFileName;

    aFileName = ::GetTmpFileName();
    aTmpFileList.Insert( new ByteString( aFileName ) );
    return( aFileName );
}

/********************************************************************/
/*                                                                  */
/*  Function    :   BOOL openinput()                                */
/*                                                                  */
/*  Description :   Check to see if the input file exists and can   */
/*  be opened for reading.                                          */
/********************************************************************/

void RscCompiler::OpenInput( const ByteString& rInput )
{
    FILE *fp;
                        /* try to open the input file               */
    if( NULL == (fp = fopen( rInput.GetBuffer(), "r")))
        pTC->pEH->FatalError( ERR_OPENFILE, RscId(), rInput.GetBuffer() );

    fclose( fp );
}

