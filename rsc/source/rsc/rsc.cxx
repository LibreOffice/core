/*************************************************************************
 *
 *  $RCSfile: rsc.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 15:54:44 $
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
#include <sys/stat.h>
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
#include <tools/intn.hxx>
#include <tools/isolang.hxx>
#include <tools/stream.hxx>

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
#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif

#include <osl/file.h>
#include <osl/process.h>
#include <rtl/strbuf.hxx>
#include <rtl/tencinfo.h>
#include <rtl/textenc.h>

#include <vector>


using namespace rtl;

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
    nCommands       = 0;
    nByteOrder      = RSC_BIGENDIAN;

    DirEntry aEntry;
    aPath = ByteString( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US ); //Immer im Aktuellen Pfad suchen
    m_aOutputFiles.clear();
    m_aOutputFiles.push_back( OutputFile() );
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
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "CmdLineArg: \"%s\"\n", *ppStr );
#endif
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
            else if( !rsc_strnicmp( (*ppStr) + 1, "sub", 3 ) )
            {
                const char* pEqual;
                for( pEqual = (*ppStr)+4; *pEqual && *pEqual != '='; ++pEqual )
                    ;
                if( *pEqual )
                {
                    const ByteString    aPath( pEqual + 1 );
                    DirEntry            aDir( String( aPath, RTL_TEXTENCODING_ASCII_US ) );

                    m_aReplacements.push_back( std::pair< OString, OString >( OString( (*ppStr)+4, pEqual - *ppStr - 4 ),
                                                                              ByteString( aDir.GetFull(), RTL_TEXTENCODING_ASCII_US ) ) );
                }
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
            else if( !rsc_strnicmp( (*ppStr) + 1, "fs=", 3 ) )
            { // anderer Name fuer .rc-file
                if( m_aOutputFiles.back().aOutputRc.Len() )
                    m_aOutputFiles.push_back( OutputFile() );
                m_aOutputFiles.back().aOutputRc = (*ppStr) + 4;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "lip=", 4 ) )
            {  // additional language specific include for system dependent files
                const ByteString    aSysSearchDir( (*ppStr)+5 );
                DirEntry            aSysDir( String( aSysSearchDir, RTL_TEXTENCODING_ASCII_US ) );

                m_aOutputFiles.back().aSysSearchDirs.push_back( ByteString( aSysDir.GetFull(), RTL_TEXTENCODING_ASCII_US ) );

                if( m_aOutputFiles.back().aLangSearchPath.Len() )
                    m_aOutputFiles.back().aLangSearchPath.Append( ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_ASCII_US ) );

                m_aOutputFiles.back().aLangSearchPath.Append( aSysSearchDir );
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fp=", 3 ) )
            { // anderer Name fuer .srs-file
                aOutputSrs = (*ppStr) + 4;
                bOutputSrsIsSet = TRUE;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fl=", 3 ) )
            { // Name fuer listing-file
                aOutputLst = (*ppStr) + 4;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fh=", 3 ) )
            { // Name fuer .hxx-file
                aOutputHxx = (*ppStr) + 4;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fc=", 3 ) )
            { // Name fuer .cxx-file
                aOutputCxx = (*ppStr) + 4;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fr=", 3 ) )
            { // Name fuer .cxx-file der Ressource Konstruktoren
                aOutputRcCtor = (*ppStr) + 4;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "fx=", 3 ) )
            { // Name fuer .src-file
                aOutputSrc = (*ppStr) + 4;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "ft=", 3 ) )
            { // touch file
                aTouchFile = (*ppStr) + 4;
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
            { // Bitmap, Pointers, Icons nicht ueberpruefen
                nCommands |= SRSDEFAULT_FLAG;
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "CHARSET_", 8 ) )
            {
                // ignore (was an option once)
            }
            else if( !rsc_stricmp( (*ppStr) + 1, "lg" ) )
            {
                m_aOutputFiles.back().aLangName = ByteString();
            }
            else if( !rsc_strnicmp( (*ppStr) + 1, "lg", 2 ) )
            {
                if( m_aOutputFiles.back().aLangName.Len() )
                    m_aOutputFiles.push_back( OutputFile() );
                m_aOutputFiles.back().aLangName = ByteString( (*ppStr)+3 );
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
        ::std::list<OutputFile>::iterator it;
        for( it = m_aOutputFiles.begin(); it != m_aOutputFiles.end(); ++it )
        {
            if( ! it->aOutputRc.Len() )
                it->aOutputRc  = ::OutputFile( *aInputList.First(), "rc"  );
        }
        if( ! bOutputSrsIsSet )
            aOutputSrs = ::OutputFile( *aInputList.First(), "srs" );
    }
    else if( !(nCommands & PRINTSYNTAX_FLAG) )
        pEH->FatalError( ERR_NOINPUT, RscId() );
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

/*************************************************************************
|*
|*    RscCmdLine::substitutePaths()
|*
*************************************************************************/

OString RscCmdLine::substitutePaths( const OString& rIn )
{
    // prepare return value
    OStringBuffer aRet( 256 );
    std::list< std::pair< OString, OString > >::const_iterator last_match = m_aReplacements.end();

    // search for longest replacement match
    for( std::list< std::pair< OString, OString > >::const_iterator repl = m_aReplacements.begin(); repl != m_aReplacements.end(); ++repl )
    {
        if( rIn.compareTo( repl->second, repl->second.getLength() ) == 0 ) // path matches
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
    aRet.append( rIn.copy( nIndex ) );

    return aRet.makeStringAndClear();
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

    if( pCL->aTouchFile.Len() )
    {
        FILE* fp = fopen( pCL->aTouchFile.GetBuffer(), "w" );
        if( fp )
        {
            fprintf( fp, "Done\n" );
            fclose( fp );
        }
        else
            pTC->pEH->FatalError( ERR_OPENFILE, RscId(), pCL->aTouchFile.GetBuffer() );
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
            RscFileInst       aFileInst( pTC, lFileKey, lFileKey, finput );

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
ERRTYPE RscCompiler :: ParseOneFile( ULONG lFileKey,
                                     const RscCmdLine::OutputFile* pOutputFile,
                                      const WriteRcContext* pContext )
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
            aError = ParseOneFile( pDep->GetFileKey(), pOutputFile, pContext );
            pFName->Seek( pDep );
            pDep = pFName->Next();
        }

        if( aError.IsError() )
            pFName->bLoaded = FALSE; //bei Fehler nicht geladenen
        else
        {
            String      aTmpName( ::GetTmpFileName(), RTL_TEXTENCODING_ASCII_US );
            DirEntry    aTmpPath( aTmpName ), aSrsPath( String( pFName->aPathName.GetBuffer(), RTL_TEXTENCODING_ASCII_US ) );

            aTmpPath.ToAbs();
            aSrsPath.ToAbs();

            if( pContext && pOutputFile )
                PreprocessSrsFile( *pOutputFile, *pContext, aSrsPath, aTmpPath );
            else
                aSrsPath.CopyTo( aTmpPath, FSYS_ACTION_COPYFILE );

            ByteString aParseFile( aTmpPath.GetFull(), RTL_TEXTENCODING_ASCII_US );
            finput = fopen( aParseFile.GetBuffer(), "r" );

            if( !finput )
            {
                pTC->pEH->Error( ERR_OPENFILE, NULL, RscId(), pFName->aPathName.GetBuffer() );
                aError = ERR_OPENFILE;
            }
            else
            {
                RscFileInst aFileInst( pTC, lFileKey, lFileKey, finput );

                pTC->pEH->StdOut( "reading file " );
                pTC->pEH->StdOut( aParseFile.GetBuffer() );
                pTC->pEH->StdOut( " " );

                aError = ::parser( &aFileInst );
                if( aError.IsError() )
                    pTC->Delete( lFileKey );//Resourceobjekte loeschen
                pTC->pEH->StdOut( "\n" );
                fclose( finput );
            };

            aTmpPath.Kill();
        };
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

static OString do_prefix( const char* pPrefix, const OUString& rFile )
{
    OStringBuffer aBuf(256);
    aBuf.append( pPrefix );
    aBuf.append( ":" );
    aBuf.append( OUStringToOString( rFile, RTL_TEXTENCODING_MS_1252 ) );
    return aBuf.makeStringAndClear();
}

ERRTYPE RscCompiler::Link()
{
    FILE *      foutput;
    ERRTYPE     aError;
    RscFile*    pFName;

#ifdef UNX
#define PATHSEP '/'
#else
#define PATHSEP '\\'
#endif

    if( !(pCL->nCommands & NOLINK_FLAG) )
    {
        ::std::list<RscCmdLine::OutputFile>::const_iterator it;

        for( it = pCL->m_aOutputFiles.begin(); it != pCL->m_aOutputFiles.end(); ++it )
        {
            // cleanup nodes
            for( pFName = pTC->aFileTab.First(); pFName && aError.IsOk(); pFName = pTC->aFileTab.Next() )
            {
                if( !pFName->IsIncFile() )
                {
                    pTC->Delete( pTC->aFileTab.GetIndex( pFName ) );
                    pTC->aFileTab.Seek( pFName );
                    pFName->bLoaded = FALSE;
                }
            }

            // rc-Datei schreiben
            ByteString aDir( it->aOutputRc );
            aDir.SetToken( aDir.GetTokenCount( PATHSEP )-1, PATHSEP, ByteString() );
            if( ! aDir.Len() )
            {
                char aBuf[1024];
                if( getcwd( aBuf, sizeof( aBuf ) ) )
                {
                    aDir = aBuf;
                    aDir.Append( PATHSEP );
                }
            }
            // work dir for absolute Urls
            OUString aCWD, aTmpUrl;
            osl_getProcessWorkingDir( &aCWD.pData );

            // get two temp file urls
            OString aRcTmp, aSysListTmp, aSysList;
            OUString aSysPath, aUrlDir;
            aSysPath = OStringToOUString( aDir, RTL_TEXTENCODING_MS_1252 );
            if( osl_getFileURLFromSystemPath( aSysPath.pData, &aUrlDir.pData ) != osl_File_E_None )
                pTC->pEH->FatalError( ERR_OPENFILE, RscId(), do_prefix( "url conversion", aUrlDir ) );

            if( osl_getAbsoluteFileURL( aCWD.pData, aUrlDir.pData, &aTmpUrl.pData ) != osl_File_E_None )
                pTC->pEH->FatalError( ERR_OPENFILE, RscId(), do_prefix( "absolute url", aUrlDir ) );
            aUrlDir = aTmpUrl;

            // create temp file for rc target
            if( osl_createTempFile( aUrlDir.pData, NULL, &aTmpUrl.pData ) != osl_File_E_None )
                pTC->pEH->FatalError( ERR_OPENFILE, RscId(), do_prefix( "temp file creation", aUrlDir ) );

            if( osl_getSystemPathFromFileURL( aTmpUrl.pData, &aSysPath.pData ) != osl_File_E_None )
                pTC->pEH->FatalError( ERR_OPENFILE, RscId(), do_prefix( "sys path conversion", aTmpUrl ) );
            aRcTmp = OUStringToOString( aSysPath, RTL_TEXTENCODING_MS_1252 );

            if ( NULL == (fExitFile = foutput = fopen( aRcTmp.getStr(), "wb" )) )
                pTC->pEH->FatalError( ERR_OPENFILE, RscId(), aRcTmp.getStr() );

            // make absolute path from IL dir (-oil switch)
            // if no -oil was given, use the same dir as for rc file
            if( pCL->aILDir.Len() )
            {
                aSysPath = OStringToOUString( pCL->aILDir, RTL_TEXTENCODING_MS_1252 );
                if( osl_getFileURLFromSystemPath( aSysPath.pData, &aTmpUrl.pData ) != osl_File_E_None )
                    pTC->pEH->FatalError( ERR_OPENFILE, RscId(), do_prefix( "url conversion", aSysPath ) );
                if( osl_getAbsoluteFileURL( aCWD.pData, aTmpUrl.pData, &aUrlDir.pData ) != osl_File_E_None )
                    pTC->pEH->FatalError( ERR_OPENFILE, RscId(), do_prefix( "absolute url", aTmpUrl ) );
            }

            if( osl_getSystemPathFromFileURL( aUrlDir.pData, &aSysPath.pData ) != osl_File_E_None )
                pTC->pEH->FatalError( ERR_OPENFILE, RscId(), do_prefix( "sys path conversion", aUrlDir ) );

            aSysList = OUStringToOString( aSysPath, RTL_TEXTENCODING_MS_1252 );
            aSysList = aSysList + "/";
            xub_StrLen nLastSep = it->aOutputRc.SearchBackward( PATHSEP );
            if( nLastSep == STRING_NOTFOUND )
                nLastSep = 0;
            xub_StrLen nLastPt = it->aOutputRc.Search( '.', nLastSep );
            if( nLastPt == STRING_NOTFOUND )
                nLastPt = it->aOutputRc.Len()+1;
            aSysList = aSysList + it->aOutputRc.Copy( nLastSep+1, nLastPt - nLastSep-1 );
            aSysList = aSysList + ".ilst";
            // create temp file for sys list target
            if( osl_createTempFile( aUrlDir.pData, NULL, &aTmpUrl.pData ) != osl_File_E_None )
                pTC->pEH->FatalError( ERR_OPENFILE, RscId(), do_prefix( "temp file creation", aUrlDir ) );

            if( osl_getSystemPathFromFileURL( aTmpUrl.pData, &aSysPath.pData ) != osl_File_E_None )
                pTC->pEH->FatalError( ERR_OPENFILE, RscId(), do_prefix( "sys path conversion", aTmpUrl ) );
            aSysListTmp = OUStringToOString( aSysPath, RTL_TEXTENCODING_MS_1252 );

            pTC->pEH->StdOut( "Generating .rc file\n" );

            rtl_TextEncoding aEnc = RTL_TEXTENCODING_UTF8;
            if( it->aLangName.CompareIgnoreCaseToAscii( "de", 2 ) == COMPARE_EQUAL )
                aEnc = RTL_TEXTENCODING_MS_1252;

            // Schreibe Datei
            sal_Char cSearchDelim = ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_ASCII_US ).GetChar( 0 );
            sal_Char cAccessDelim = ByteString( DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US ).GetChar( 0 );
            pTC->ChangeLanguage( it->aLangName );
            pTC->SetSourceCharSet( aEnc );
            pTC->ClearSysNames();
            ByteString aSysSearchPath( it->aLangSearchPath );
            xub_StrLen nIndex = 0;
            ByteString aSearchPath = pTC->GetSearchPath();
            while( nIndex != STRING_NOTFOUND )
            {
                ByteString aToken = aSearchPath.GetToken( 0, cSearchDelim, nIndex );
                if( aSysSearchPath.Len() )
                    aSysSearchPath.Append( cSearchDelim );
                aSysSearchPath.Append( aToken );
                aSysSearchPath.Append( cAccessDelim );
                aSysSearchPath.Append( it->aLangName );
                aSysSearchPath.Append( cSearchDelim );
                aSysSearchPath.Append( aToken );
            }
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "setting search path for language %s: %s\n", it->aLangName.GetBuffer(), aSysSearchPath.GetBuffer() );
#endif
            pTC->SetSysSearchPath( aSysSearchPath );

            WriteRcContext  aContext;

            aContext.fOutput = foutput;
            aContext.aOutputRc = it->aOutputRc;
            aContext.aOutputSysList = aSysListTmp;
            aContext.pCmdLine = pCL;

            // create empty sys list
            if( aContext.aOutputSysList.getLength() )
            {
                FILE* pSysListFile = fopen( aContext.aOutputSysList.getStr(), "wb" );

                if( !pSysListFile )
                    pTC->pEH->FatalError( ERR_OPENFILE, RscId(), aContext.aOutputSysList.getStr() );
                else
                    fclose( pSysListFile );
            }

            // parse files for specific language
            for( pFName = pTC->aFileTab.First(); pFName && aError.IsOk(); pFName = pTC->aFileTab.Next() )
            {
                if( !pFName->IsIncFile() )
                {
                    aError = ParseOneFile( pTC->aFileTab.GetIndex( pFName ), &*it, &aContext );
                    pTC->aFileTab.Seek( pFName );
                }
            };

            aError = pTC->WriteRc( aContext );

            fclose( foutput );
            fExitFile = NULL;
            unlink( it->aOutputRc.GetBuffer() );
            if( rename( aRcTmp.getStr(), it->aOutputRc.GetBuffer() ) )
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
                chmod( it->aOutputRc.GetBuffer(), S_IRWXU | S_IRWXG | S_IROTH );
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
        for( pFName = pTC->aFileTab.First(); pFName && aError.IsOk(); pFName = pTC->aFileTab.Next() )
        {
            if( !pFName->IsIncFile() )
            {
                aError = ParseOneFile( pTC->aFileTab.GetIndex( pFName ), NULL, NULL );
                pTC->aFileTab.Seek( pFName );
            }
        };
    }

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

/*************************************************************************
|*
|*    GetImageFilePath()
|*
|*************************************************************************/

bool RscCompiler::GetImageFilePath( const RscCmdLine::OutputFile& rOutputFile,
                                       const WriteRcContext& rContext,
                                    const ByteString& rBaseFileName,
                                    ByteString& rImagePath,
                                    FILE* pSysListFile )
{
    ::std::list< ByteString >                   aFileNames;
    ByteString*                                 pPath;
    bool                                        bFound = false;

    ByteString  aFileName( rBaseFileName );
    aFileNames.push_back( aFileName += ".png" );

    aFileName = rBaseFileName;
    aFileNames.push_back( aFileName += ".bmp" );

    ::std::list< ByteString >::iterator aFileIter( aFileNames.begin() );

    while( ( aFileIter != aFileNames.end() ) && !bFound )
    {
        ::std::list< ByteString >::const_iterator aDirIter( rOutputFile.aSysSearchDirs.begin() );

        while( ( aDirIter != rOutputFile.aSysSearchDirs.end() ) && !bFound )
        {
            const DirEntry  aPath( String( *aDirIter, RTL_TEXTENCODING_ASCII_US ) );
            DirEntry        aRelPath( aPath );
            DirEntry        aAbsPath( aRelPath += DirEntry( String( *aFileIter, RTL_TEXTENCODING_ASCII_US ) ) );

            aAbsPath.ToAbs();
            const FileStat aFS( aAbsPath.GetFull() );

#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "Searching image: %s\n", ByteString( aRelPath.GetFull(), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
#endif

            if( aFS.IsKind( FSYS_KIND_FILE ) )
            {
                std::list< std::pair< OString, OString > >::const_iterator  aReplIter( rContext.pCmdLine->m_aReplacements.begin() );
                String                                                      aStr( aRelPath.GetFull() );
                OString                                                     aRelPathStr( aStr.GetBuffer(), aStr.Len(), RTL_TEXTENCODING_ASCII_US );

                while( ( aReplIter != rContext.pCmdLine->m_aReplacements.end() ) && !bFound )
                {
                    if( ByteString( aRelPathStr ).ToLowerAscii().Search( ByteString( aReplIter->second ).ToLowerAscii() ) == 0 )
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
                    DirEntry    aSysPath( String( *aDirIter, RTL_TEXTENCODING_ASCII_US ) );
                    String      aSysPathFull( ( aSysPath += DirEntry( String( *aFileIter, RTL_TEXTENCODING_ASCII_US ) ) ).GetFull() );
                    OString     aSysPathStr( aSysPathFull.GetBuffer(), aSysPathFull.Len(), RTL_TEXTENCODING_ASCII_US );

                    fprintf( pSysListFile, "%s\n", rContext.pCmdLine->substitutePaths( aSysPathStr ).getStr() );
                }

#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "ImagePath to add: %s\n", rImagePath.GetBuffer() );
#endif
            }

            ++aDirIter;
        }

        ++aFileIter;
    }

    return bFound;
}

// ------------------------------------------------------------------------------

void RscCompiler::PreprocessSrsFile( const RscCmdLine::OutputFile& rOutputFile,
                                        const WriteRcContext& rContext,
                                       const DirEntry& rSrsInPath,
                                       const DirEntry& rSrsOutPath )
{
    SvFileStream                aIStm( rSrsInPath.GetFull(), STREAM_READ );
    SvFileStream                aOStm( rSrsOutPath.GetFull(), STREAM_WRITE | STREAM_TRUNC );
    ::std::vector< ByteString > aMissingImages;
    FILE*                       pSysListFile = rContext.aOutputSysList.getLength() ? fopen( rContext.aOutputSysList.getStr(), "ab" ) : NULL;
    bool                        bRet = true;

    if( !aIStm.GetError() && !aOStm.GetError() )
    {
        ByteString aLine, aFilePath;

        while( aIStm.ReadLine( aLine ) )
        {
            if( ( aLine.GetTokenCount( '=' ) == 2 ) &&
                ( aLine.GetToken( 0, '=' ).Search( "File" ) != STRING_NOTFOUND ) )
            {
                ByteString aBaseFileName( aLine.GetToken( 1, '"' ).GetToken( 0, '.' ) );

                if( GetImageFilePath( rOutputFile, rContext, aBaseFileName, aFilePath, pSysListFile ) )
                    ( ( aLine = "File = \"" ) += aFilePath ) += "\";";
                else
                    aMissingImages.push_back( aBaseFileName );

                aOStm.WriteLine( aLine );
            }
            else if( aLine.Search( "ImageList" ) != STRING_NOTFOUND )
            {
                ::std::vector< ::std::pair< ByteString, sal_Int32 > > aEntryVector;

                aOStm.WriteLine( aLine );

                if( aLine.Search( ';' ) == STRING_NOTFOUND )
                {
                    const sal_uInt32 nImgListStartPos = aIStm.Tell();

                    do
                    {
                        if( !aIStm.ReadLine( aLine ) )
                            break;
                    }
                    while( aLine.Search( "Prefix" ) == STRING_NOTFOUND );

                    const ByteString aPrefix( aLine.GetToken( 1, '"' ) );
                    aIStm.Seek( nImgListStartPos );

                    do
                    {
                        if (!aIStm.ReadLine( aLine ) )
                            break;
                    }
                    while( aLine.Search( "IdList" ) == STRING_NOTFOUND );

                    // scan all ids and collect images
                    while( aLine.Search( '}' ) == STRING_NOTFOUND )
                    {
                        if( !aIStm.ReadLine( aLine ) )
                            break;

                        aLine.EraseLeadingChars( ' ' );
                        aLine.EraseLeadingChars( '\t' );
                        aLine.EraseAllChars( ';' );

                        if( aLine.IsNumericAscii() )
                        {
                            ByteString  aBaseFileName( aPrefix );
                            sal_Int32   nNumber = atoi( aLine.GetBuffer() );

                            if( nNumber < 10000 )
                                aBaseFileName += ByteString::CreateFromInt32( 0 );

                            if( GetImageFilePath( rOutputFile, rContext, aBaseFileName += aLine , aFilePath, pSysListFile ) )
                                aEntryVector.push_back( ::std::make_pair< ByteString, sal_Int32 >( aFilePath, nNumber ) );
                            else
                                aMissingImages.push_back( aBaseFileName );
                        }
                    }

                    const sal_uInt32 nImgListEndPos = aIStm.Tell();
                    aIStm.Seek( nImgListStartPos );
                    while( aIStm.Tell() < nImgListEndPos )
                    {
                        aIStm.ReadLine( aLine );

                        if( aLine.Search( "IdList" ) != STRING_NOTFOUND )
                        {
                            while( aLine.Search( '}' ) == STRING_NOTFOUND )
                                aIStm.ReadLine( aLine );
                        }
                        else
                            aOStm.WriteLine( aLine );
                    }

                    aOStm.WriteLine( "FileList = {" );

                    for( sal_uInt32 i = 0; i < aEntryVector.size(); ++i )
                    {
                        ByteString aEntryString( "< \"" );

                        aEntryString += aEntryVector[ i ].first;
                        aEntryString += "\"; ";
                        aEntryString += ByteString::CreateFromInt32( aEntryVector[ i ].second );
                        aEntryString += "; >;";

                        aOStm.WriteLine( aEntryString );
                    }

                    aOStm.WriteLine( "};" );
                   }
                else
                    aOStm.WriteLine( aLine );
            }
            else
                aOStm.WriteLine( aLine );
        }
    }
    else
        bRet = false;

    if( aMissingImages.size() > 0 )
    {
        ByteString aImagesStr;

        for( sal_uInt32 i = 0; i < aMissingImages.size(); ++i )
        {
            if( i )
                aImagesStr += ' ';

            aImagesStr += aMissingImages[ i ];
        }

        pTC->pEH->FatalError( ERR_NOIMAGE, RscId(), aImagesStr.GetBuffer() );
    }

    if( pSysListFile )
        fclose( pSysListFile );
}
