/*************************************************************************
 *
 *  $RCSfile: bmp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: pluby $ $Date: 2001-02-21 00:32:30 $
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <tools/stream.hxx>
#include <tools/fsys.hxx>
#include <tools/new.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/menu.hxx>
#include <vcl/config.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bmpacc.hxx>
#include <solar.hrc>
#include <filedlg.hxx>
#include "bmp.hrc"

// -----------
// - Defines -
// -----------

#define SOLAR_PATH          "s:\\solenv\\res"
#define LANG_ALL            "Alle Sprachen"
#define FILETEST(FileEntry) ((FileEntry).Exists())

// --------------
// - Exit codes -
// --------------

#define EXIT_NOERROR            0
#define EXIT_MISSING_BITMAP     1
#define EXIT_NOSRSFILE          2
#define EXIT_NOIMGLIST          3
#define EXIT_DIMENSIONERROR     4
#define EXIT_IOERROR            5
#define EXIT_COMMONERROR        6
#define EXIT_MISSING_RESOURCE   7
#define EXIT_COLORDEPTHERROR    8

// -----------
// - Globals -
// -----------

char    LangDir[][ 9 ] = {  "deut", "enus", "fren", "ital", "dtch", "span", "dan", "swed", "port", "japn", "chinsim", "chintrad", "russ", "pol", "arab", "greek", "korean" , "turk" };
USHORT  LangNum[] = { 49, 1, 33, 39, 31, 34, 45, 46, 03, 81, 86, 88, 07, 48, 96, 30, 82, 90};

// ------------
// - LangInfo -
// ------------

struct LangInfo
{
    String  maLangDir;
    USHORT  mnLangNum;

            LangInfo( const String& rLangDir, USHORT nLangNum ) : maLangDir( rLangDir ), mnLangNum( nLangNum ) {}
};

// ----------
// - BmpApp -
// ----------

class BmpWin;

class BmpApp : public Application
{
private:

    BmpWin* pBmpWin;
    String  aOutputFileName;
    BYTE    cExitCode;

            DECL_LINK( Select, Menu* pMenu );

    BOOL    GetCommandOption( const String& rSwitch, String& rSwitchParam );

public:

    void    Main( );
    void    SetExitCode( BYTE cExit )
            {
                if( ( EXIT_NOERROR == cExitCode ) || ( cExit != EXIT_NOERROR ) )
                    cExitCode = cExit;
            }
};

// -----------------------------------------------------------------------

BmpApp aBmpApp;

// ----------
// - Pathes -
// ----------

class Pathes : public ModalDialog
{
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    GroupBox        aGrpPath;
    FixedText       aFtSrs;
    FixedText       aFtRes;
    FixedText       aFtOut;
    Edit            aEdtSrs;
    PushButton      aBtnSrs;
    Edit            aEdtRes;
    PushButton      aBtnRes;
    Edit            aEdtOut;
    PushButton      aBtnOut;
    GroupBox        aGrpLang;
    ListBox         aLbLang;

    DECL_LINK( ClickSrsHdl, void* );
    DECL_LINK( ClickResHdl, void* );
    DECL_LINK( ClickOutHdl, void* );


public:

            Pathes( Window* pParent,
                    const String& rSrsPath,
                    const String& rResPath,
                    const String& rOutPath,
                    const USHORT nLang );

    String  GetSrsPath() const { return aEdtSrs.GetText(); }
    String  GetResPath() const { return aEdtRes.GetText(); }
    String  GetOutPath() const { return aEdtOut.GetText(); }

    USHORT  GetLangNum() const { return aLbLang.GetSelectEntryPos(); }
};

// ----------
// - BmpWin -
// ----------

class BmpWin : public WorkWindow
{
    VirtualDevice   aDev;
    ListBox         aLB;
    Bitmap          aOutBmp;
    Size            aOneSize;
    Size            aTotSize;
    Point           aPos;
    SvFileStream*   pSRS;
    ULONG           nTotCount;
    USHORT          nPos;

    void            DoIt( SvStream& rStm, DirEntry&, DirEntry&, String&, String&, const LangInfo& rLang );

public:

                    BmpWin( Window* pParent, WinBits nWinStyle );

    void            Resize();
    void            StartCreation( const String& rSRSName, const String& rInName,
                                   const String& rOutName, const LangInfo& rLang );

    void            SetMode( BOOL bMode ) { aLB.SetUpdateMode( bMode ); }
    void            ClearInfo();
    void            WriteInfo( const String& rText, BYTE cExitCode = EXIT_NOERROR );
};

// ----------
// - BmpApp -
// ----------

BOOL BmpApp::GetCommandOption( const String& rSwitch, String& rFollowingParam )
{
    BOOL bRet = FALSE;

    for( long i = 0, nCount = GetCommandLineParamCount(); ( i < nCount ) && !bRet; i++ )
    {
        String  aTestStr( '-' );

        for( long n = 0; ( n < 2 ) && !bRet; n++ )
        {
            aTestStr += rSwitch;

            if( aTestStr.CompareIgnoreCaseToAscii( GetCommandLineParam( i ) ) == COMPARE_EQUAL )
            {
                bRet = TRUE;

                if( i < ( nCount - 1 ) )
                    rFollowingParam = GetCommandLineParam( i + 1 );
                else
                    rFollowingParam = String();
            }

            if( 0 == n )
                aTestStr = '/';
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void BmpApp::Main( )
{
    ResMgr* pAppResMgr = NULL;
    String  aLangDir;
    String  aOutName;
    int     nCmdCount = GetCommandLineParamCount();
    USHORT  nCurCmd = 0;

    cExitCode = EXIT_NOERROR;

    // Param1: filename of *.srs input file with image lists
    // Param2: path to local bitmaps
    // Param3: output directory
    // Param4: Langugage dependent directory

    if( nCmdCount >= 3 )
    {
        const String aSrsName( GetCommandLineParam( nCurCmd++ ) );
        const String aInName( GetCommandLineParam( nCurCmd++ ) );
        aOutName = GetCommandLineParam( nCurCmd++ );

        pBmpWin = new BmpWin( NULL, WB_APP | WB_STDWORK );
        pBmpWin->ClearInfo();

        if( nCurCmd < nCmdCount )
        {
#if 0
            GetCommandOption( 'l', aLangDir );
#else
            aLangDir = GetCommandLineParam( nCurCmd++ );
#endif
            GetCommandOption( 'f', aOutputFileName );

            if( aLangDir.Len() && ( aLangDir.CompareIgnoreCaseToAscii( "all" ) == COMPARE_EQUAL ) )
            {
                for ( USHORT i = 0, nCount = ( sizeof( LangNum ) / sizeof( LangNum[ 0 ] ) ); i < nCount; i++ )
                    pBmpWin->StartCreation( aSrsName, aInName, aOutName, LangInfo( String( LangDir[ i ], RTL_TEXTENCODING_UTF8 ), LangNum[ i ] ) );
            }
            else
            {
                pBmpWin->StartCreation( aSrsName, aInName, aOutName, LangInfo( aLangDir, DirEntry( aOutName ).GetName().ToInt32() ) );
            }
        }
        else
            pBmpWin->StartCreation( aSrsName, aInName, aOutName, LangInfo( String( LangDir[ 0 ], RTL_TEXTENCODING_UTF8 ), LangNum[ 0 ] ) );
    }
    else
    {
        ResMgr*     pAppResMgr = new ResMgr( String( RTL_CONSTASCII_USTRINGPARAM( "bmp.res" ) ), NULL, NULL );
        MenuBar*    pMB = new MenuBar();
        PopupMenu*  pPB = new PopupMenu();

        Resource::SetResManager( pAppResMgr );

        pMB->InsertItem( 1, String( RTL_CONSTASCII_USTRINGPARAM( "~ImageList" ) ) );
            pPB->InsertItem( 1, String( RTL_CONSTASCII_USTRINGPARAM( "~Erstellen..." ) ) );
            pPB->InsertSeparator();
            pPB->InsertItem( 2, String( RTL_CONSTASCII_USTRINGPARAM( "~Beenden" ) ) );

        pMB->SetPopupMenu( 1, pPB );
        pMB->SetSelectHdl( LINK( this, BmpApp, Select ) );

        pBmpWin = new BmpWin( NULL, WB_APP | WB_STDWORK );
        pBmpWin->SetMenuBar( pMB );
        pBmpWin->Show();

        Execute();
        delete pPB;
        delete pMB;
        delete pBmpWin;
    }

    Resource::SetResManager( NULL );
    delete pAppResMgr;

    if( ( EXIT_NOERROR == cExitCode ) && aOutputFileName.Len() && aOutName.Len() )
    {
        SvFileStream    aOStm( aOutputFileName, STREAM_WRITE | STREAM_TRUNC );
        ByteString      aStr( "Successfully generated ImageList(s) in: " );

        aOStm.WriteLine( aStr.Append( ByteString( aOutName, RTL_TEXTENCODING_UTF8 ) ) );
        aOStm.Close();
    }

    if ( EXIT_NOERROR != cExitCode )
        raise( SIGABRT );
    return;
}

// -----------------------------------------------------------------------

IMPL_LINK( BmpApp, Select, Menu*, pMenu )
{
    if ( pMenu->GetCurItemId() == 1 )
    {
        Config  aCfg;
        aCfg.SetGroup( "Path-Entries" );

        String  aSrsPath( String::CreateFromAscii( aCfg.ReadKey( "SRS", ByteString() ).GetBuffer() ) );
        String  aResPath( String::CreateFromAscii( aCfg.ReadKey( "RES", ByteString() ).GetBuffer() ) );
        String  aOutPath( String::CreateFromAscii( aCfg.ReadKey( "OUT", ByteString() ).GetBuffer() ) );
        USHORT  nLanguage = (USHORT) aCfg.ReadKey( "LNG", '1' ).ToInt32();
        Pathes  aPathDlg( pBmpWin, aSrsPath, aResPath, aOutPath, nLanguage );

        if ( aPathDlg.Execute() == RET_OK )
        {
            aCfg.WriteKey( "SRS", ByteString( aSrsPath = String( aPathDlg.GetSrsPath() ), RTL_TEXTENCODING_UTF8 ) );
            aCfg.WriteKey( "RES", ByteString( aResPath = String( aPathDlg.GetResPath() ), RTL_TEXTENCODING_UTF8 ) );
            aCfg.WriteKey( "OUT", ByteString( aOutPath = String( aPathDlg.GetOutPath() ), RTL_TEXTENCODING_UTF8 ) );
            aCfg.WriteKey( "LNG", ByteString::CreateFromInt32( nLanguage = aPathDlg.GetLangNum() ) );

            pBmpWin->ClearInfo();

            if ( !nLanguage )
            {
                for ( USHORT i = 0, nCount = ( sizeof( LangNum ) / sizeof( LangNum[ 0 ] ) ); i < nCount; i++ )
                    pBmpWin->StartCreation( aSrsPath, aResPath, aOutPath, LangInfo( String( RTL_CONSTASCII_USTRINGPARAM( "deut" ) ), 49 ) );
            }
            else
                pBmpWin->StartCreation( aSrsPath, aResPath, aOutPath, LangInfo( String( RTL_CONSTASCII_USTRINGPARAM( LangDir[ nLanguage - 1 ] ) ), LangNum[ nLanguage - 1 ] ) );

            pBmpWin->WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "DONE!" ) ) );
        }
    }
    else if( pMenu->GetCurItemId() == 2 )
        Quit();

    return 0;
}

// ----------
// - Pathes -
// ----------

Pathes::Pathes( Window* pParent,
                const String& rSrsPath,
                const String& rResPath,
                const String& rOutPath,
                const USHORT nLang ) :
            ModalDialog ( pParent, ResId( DLG_PATH ) ),
            aGrpPath    ( this, ResId( GRP_PATH ) ),
            aFtSrs      ( this, ResId( FT_SRS ) ),
            aFtRes      ( this, ResId( FT_RES ) ),
            aFtOut      ( this, ResId( FT_OUT ) ),
            aEdtSrs     ( this, ResId( EDT_SRS ) ),
            aBtnSrs     ( this, ResId( BTN_SRS ) ),
            aEdtRes     ( this, ResId( EDT_RES ) ),
            aBtnRes     ( this, ResId( BTN_RES ) ),
            aEdtOut     ( this, ResId( EDT_OUT ) ),
            aBtnOut     ( this, ResId( BTN_OUT ) ),
            aGrpLang    ( this, ResId( GRP_LANG ) ),
            aLbLang     ( this, ResId( LB_LANG ) ),
            aBtnOk      ( this, ResId( BTN_OK ) ),
            aBtnCancel  ( this, ResId( BTN_CANCEL ) )
{
    FreeResource();

    aBtnSrs.SetClickHdl( LINK( this, Pathes, ClickSrsHdl ) );
    aBtnRes.SetClickHdl( LINK( this, Pathes, ClickResHdl ) );
    aBtnOut.SetClickHdl( LINK( this, Pathes, ClickOutHdl ) );

    aEdtSrs.SetText( rSrsPath );
    aEdtRes.SetText( rResPath );
    aEdtOut.SetText( rOutPath );
    aEdtSrs.GrabFocus();

    aLbLang.InsertEntry( String( RTL_CONSTASCII_USTRINGPARAM( LANG_ALL ) ), 0 );
    for( USHORT i = 1, nCount = ( sizeof( LangNum ) / sizeof( LangNum[ 0 ] ) ); i <= nCount; i++ )
        aLbLang.InsertEntry( String::CreateFromInt32( LangNum[ i - 1 ] ), i );

    aLbLang.SelectEntryPos( nLang );
}

// -----------------------------------------------------------------------------

IMPL_LINK( Pathes, ClickSrsHdl, void*, p )
{
    FileDialog  aFileDlg( this, WB_OPEN | WB_3DLOOK );
    DirEntry    aPath( aEdtSrs.GetText() );

    aFileDlg.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "SRS-Datei mit Imagelisten laden" ) ) );
    aPath += DirEntry( String( RTL_CONSTASCII_USTRINGPARAM( "*.srs" ) ) );
    aFileDlg.SetPath( aPath.GetFull() );
    if ( aFileDlg.Execute() == RET_OK )
        aEdtSrs.SetText( aFileDlg.GetPath() );

    return 0L;
}

// -----------------------------------------------------------------------------

IMPL_LINK( Pathes, ClickResHdl, void*, p )
{
    PathDialog aPathDlg( this, WB_OPEN | WB_3DLOOK );

    aPathDlg.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Quellpfad BMP-Dateien" ) ) );
    if ( aPathDlg.Execute() == RET_OK )
        aEdtRes.SetText( aPathDlg.GetPath() );

    return 0L;
}

// -----------------------------------------------------------------------------

IMPL_LINK( Pathes, ClickOutHdl, void*, p )
{
    PathDialog aPathDlg( this, WB_OPEN | WB_3DLOOK );

    aPathDlg.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Zielpfad BMP-Dateien" ) ) );
    if ( aPathDlg.Execute() == RET_OK )
        aEdtOut.SetText( aPathDlg.GetPath() );

    return 0L;
}

// ----------
// - BmpWin -
// ----------

BmpWin::BmpWin(Window* pParent, WinBits aWinStyle) :
    WorkWindow  ( pParent, aWinStyle ),
    aLB         ( this, WB_HSCROLL | WB_VSCROLL ),
    nPos        ( 0 )
{
    SetText( String( RTL_CONSTASCII_USTRINGPARAM( "SVX - BmpMaker" ) ) );
    aLB.SetUpdateMode( TRUE );
    aLB.Show();
}

// -----------------------------------------------------------------------

void BmpWin::StartCreation( const String& rSRSName, const String& rInName,
                            const String& rOutName, const LangInfo& rLang )
{
    DirEntry    aFileName( rSRSName );
    DirEntry    aInDir( rInName );
    DirEntry    aOutDir( rOutName );
    BOOL        bDone = FALSE;

    aFileName.ToAbs();
    aInDir.ToAbs();
    aOutDir.ToAbs();

    pSRS = new SvFileStream ( aFileName.GetFull(), STREAM_STD_READ );

    if( pSRS->GetError() )
        WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Kein SRS file!" ) ), EXIT_NOSRSFILE );
    else
    {
        String      aText;
        ByteString  aByteText;
        BOOL        bLangDep = FALSE;

        do
        {
            do
            {
                if (!pSRS->ReadLine(aByteText))
                    break;
            }
            while ( aByteText.Search( "ImageList" ) == STRING_NOTFOUND );

            do
            {
                if (!pSRS->ReadLine( aByteText ) )
                    break;
            }
            while ( aByteText.Search( "File" ) == STRING_NOTFOUND );
            aText = String::CreateFromAscii( aByteText.GetBuffer() );

            USHORT nStart = aText.Search('"') + 1;
            USHORT nEnd = aText.Search( '"', nStart+1 );
            String aName( aText, nStart, nEnd-nStart );
            String aPrefix( aName, 0, 2 );

            do
            {
                if( !bLangDep &&
                    aByteText.Search( "File" ) != STRING_NOTFOUND &&
                    aByteText.Search( '[' ) != STRING_NOTFOUND &&
                    aByteText.Search( ']' ) != STRING_NOTFOUND )
                {
                    bLangDep = TRUE;
                }

                if (!pSRS->ReadLine(aByteText))
                     break;
            }
            while (aByteText.Search( "IdList" ) == STRING_NOTFOUND );
            aText = String::CreateFromAscii( aByteText.GetBuffer() );

            // if image list is not language dependent,
            // don't do anything for languages except german
            if( aText.Len() )
            {
                bDone = TRUE;
                DoIt( *pSRS, aInDir, aOutDir, aPrefix, aName, rLang );
            }
            else if( ( rLang.mnLangNum != 49 ) && !bLangDep )
            {
                WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "INFO: ImageList is not language dependent! Nothing to do for this language." ) ) );
                bDone = TRUE;
            }
        }
        while ( aText.Len() );
    }

    if( !bDone )
        WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: No ImageList found in SRS file!" ) ), EXIT_NOIMGLIST );

    delete pSRS;
}

// -----------------------------------------------------------------------

void BmpWin::DoIt( SvStream& rStm, DirEntry& rIn, DirEntry& rOut, String& rPrefix,
                   String& rName, const LangInfo& rLang )
{
    String      aResPath;
    const char* pResPath = getenv( "SOLARSRC" );

    if( pResPath )
    {
        String aString( String::CreateFromAscii( ByteString( pResPath ).GetBuffer() ) );
        aResPath = ( DirEntry( aString ) += DirEntry( String( RTL_CONSTASCII_USTRINGPARAM( "res" ) ) ) ).GetFull();
    }
    else
    {
        aResPath.Assign( String( RTL_CONSTASCII_USTRINGPARAM( "o:\\" ) ) );
        aResPath.Append( String::CreateFromInt32( SOLARUPD ) );
        aResPath.Append( String( RTL_CONSTASCII_USTRINGPARAM( "\\res" ) ) );
    }

    SvFileStream        aOutStream;
    Bitmap              aTotalBmp;
    Bitmap              aBmp;
    Size                aSize;
    String              aText;
    String              aString;
    String              aName( rName );
    String              aFileName;
    ULONG               nSRSPos;
    long                nBmpPos = 0L;
    USHORT              nId;
    USHORT              nN = 1;
    DirEntry            aInPath( rIn + DirEntry( String( RTL_CONSTASCII_USTRINGPARAM( "x.bmp" ) ) ) );
    DirEntry            aOutFile( rOut );
    DirEntry            SolarPath1( aResPath );
    DirEntry            SolarPath2( aResPath );
    String              aDefaultName( rPrefix ); aDefaultName.Append( String( RTL_CONSTASCII_USTRINGPARAM( "00000.bmp" ) ) );
    BOOL                bInserted = FALSE;
    BOOL                bFirst = TRUE;

    // Falls nicht deutsch, noch die Vorwahlnummer hintenran
    if( rLang.mnLangNum != 49 )
    {
        String aNumStr( String::CreateFromInt32( rLang.mnLangNum ) );

        if( aNumStr.Len() == 1 )
            aNumStr.Insert( '0', 0 );

        aName = DirEntry( aName ).GetBase();
        aName += aNumStr;
        aName += String( RTL_CONSTASCII_USTRINGPARAM( ".bmp" ) );
        SolarPath1 += rLang.maLangDir;
    }

    aOutFile += DirEntry( aName );

    // Die Namen werden spaeter ersetzt
    SolarPath1 += DirEntry( String( RTL_CONSTASCII_USTRINGPARAM( "x.bmp" ) ) );
    SolarPath2 += DirEntry( String( RTL_CONSTASCII_USTRINGPARAM( "x.bmp" ) ) );

    // Anzahl der Bitmaps bestimmen
    for ( nTotCount = 0UL, nSRSPos = pSRS->Tell(); aText.Search( '}' ) == STRING_NOTFOUND; )
    {
        ByteString aTmp;

        if ( !pSRS->ReadLine( aTmp ) )
            break;

        aText.Assign( String::CreateFromAscii( aTmp.GetBuffer() ) );
        aText.EraseLeadingChars( ' ' );
        aText.EraseLeadingChars( '\t' );
        aText.EraseAllChars( ';' );

        if ( ByteString( aText, RTL_TEXTENCODING_UTF8 ).IsNumericAscii() )
            nTotCount++;
    }

    if( !nTotCount )
        WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "WARNING: No imagelist resource found: " ) ).Append( aString ), EXIT_MISSING_RESOURCE );

    // Wieder an Anfang und los gehts
    aText = String();
    pSRS->Seek( nSRSPos );

    // write info
    String aInfo( RTL_CONSTASCII_USTRINGPARAM( "CREATING ImageList for language: " ) );
    aInfo += rLang.maLangDir;
    aInfo += String( RTL_CONSTASCII_USTRINGPARAM( " [ " ) );
    aInfo += aInPath.GetPath().GetFull();
    aInfo += String( RTL_CONSTASCII_USTRINGPARAM( "; " ) );
    aInfo += SolarPath1.GetPath().GetFull();
    if( SolarPath2 != SolarPath1 )
    {
        aInfo += String( RTL_CONSTASCII_USTRINGPARAM( "; " ) );
        aInfo += SolarPath2.GetPath().GetFull();
    }
    aInfo += String( RTL_CONSTASCII_USTRINGPARAM( " ]" ) );
    WriteInfo( aInfo );

    for ( ; aText.Search( '}' ) == STRING_NOTFOUND; )
    {
        ByteString aTmp;

        if ( !pSRS->ReadLine( aTmp ) )
            break;

        aText.Assign( String::CreateFromAscii( aTmp.GetBuffer() ) );
        aText.EraseLeadingChars( ' ' );
        aText.EraseLeadingChars( '\t' );
        aText.EraseAllChars( ';' );

        aTmp = ByteString( aText, RTL_TEXTENCODING_UTF8 );

        if( aTmp.IsNumericAscii() )
        {
            nId = atoi( aTmp.GetBuffer() );

            if ( nId < 10000 )
            {
                const String aTmp( aText );

                aText.Assign( String::CreateFromInt32( 0 ) );
                aText.Append( aTmp );
            }

            aString.Assign( rPrefix );
            aString.Append( aText );
            aString.Append( String( RTL_CONSTASCII_USTRINGPARAM( ".bmp" ) ) );
            aInPath.SetName( aString );
        }
        else
            continue;

        if( !FILETEST( aInPath ) )
        {
            // Falls nicht deutsch, suchen wir zuerst im jeweiligen Sprach-Unterverz.
            if( rLang.mnLangNum != 49 )
            {
                SolarPath1.SetName( aString );

                if ( !FILETEST( SolarPath1 ) )
                {
                    SolarPath2.SetName( aString );

                    if( !FILETEST( SolarPath2 ) )
                        aBmp = Bitmap();
                    else
                    {
                        SvFileStream aIStm( aFileName = SolarPath2.GetFull(), STREAM_READ );
                        aIStm >> aBmp;
                    }
                }
                else
                {
                    SvFileStream aIStm( aFileName = SolarPath1.GetFull(), STREAM_READ );
                    aIStm >> aBmp;
                }
            }
            else
            {
                SolarPath2.SetName( aString );

                if( !FILETEST( SolarPath2 ) )
                    aBmp = Bitmap();
                else
                {
                    SvFileStream aIStm( aFileName = SolarPath2.GetFull(), STREAM_READ );
                    aIStm >> aBmp;
                }
            }
        }
        else
        {
            SvFileStream aIStm( aFileName = aInPath.GetFull(), STREAM_READ );
            aIStm >> aBmp;
        }

        aSize = aBmp.GetSizePixel();

        // falls Bitmap defekt ist, malen wir ein rotes Kreuz
        if( !aSize.Width() || !aSize.Height() )
        {
            WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "WARNING: Bitmap is missing: " ) ).Append( aString ), EXIT_MISSING_BITMAP );

            aSize = aOneSize;

            if( aSize.Width() && aSize.Height() )
            {
                aBmp = Bitmap( aSize, !!aTotalBmp ? aTotalBmp.GetBitCount() : 4 );
                aBmp.Erase( COL_WHITE );

                BitmapWriteAccess* pAcc = aBmp.AcquireWriteAccess();

                if( pAcc )
                {
                    Point aPoint;
                    const Rectangle aRect( aPoint, aOneSize );

                    pAcc->SetLineColor( Color( COL_LIGHTRED ) );
                    pAcc->DrawRect( aRect );
                    pAcc->DrawLine( aRect.TopLeft(), aRect.BottomRight() );
                    pAcc->DrawLine( aRect.TopRight(), aRect.BottomLeft() );
                    aBmp.ReleaseAccess( pAcc );
                }
            }
        }

        // Beim ersten Mal Zugriffs-Bitmap mit der
        // richtigen Groesse, 4Bit und der Standardpalette anlegen
        if( bFirst && aSize.Width() && aSize.Height() )
        {
            aTotSize = aOneSize = aSize;
            aTotSize.Width() *= nTotCount;
            aTotalBmp = Bitmap( aTotSize, 4 );
            bFirst = FALSE;
        }

        if( ( aSize.Width() > aOneSize.Width() ) || ( aSize.Height() > aOneSize.Height() ) )
             WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Different dimensions in file: " ) ).Append( aString ), EXIT_DIMENSIONERROR );
        else if( aBmp.GetBitCount() != aTotalBmp.GetBitCount() )
             WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Different color depth in file: ") ).Append( aString ), EXIT_COLORDEPTHERROR );
        else
        {
            const Rectangle aDst( Point( nBmpPos * aOneSize.Width(), 0L ), aSize );
            Point aPoint;
            const Rectangle aSrc( aPoint, aSize );

            if( !!aTotalBmp && !!aBmp && !aDst.IsEmpty() && !aSrc.IsEmpty() )
                aTotalBmp.CopyPixel( aDst, aSrc, &aBmp );
        }

        nBmpPos++;

        GetpApp()->Reschedule();
    }

    if ( !!aTotalBmp && aTotSize.Width() && aTotSize.Height() )
    {
        const String aFile( aOutFile.GetFull() );

        aOutStream.Open( aFile, STREAM_WRITE | STREAM_TRUNC );

        if( !aOutStream.IsOpen() )
            WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Could not open output file: " ) ).Append( aFile ), EXIT_IOERROR );
        else
        {
            aOutStream << aTotalBmp;

            if( aOutStream.GetError() )
                WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Could not write to output file: " ) ).Append( aFile ), EXIT_IOERROR );
            else
                WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "Successfully generated ImageList " ) ).Append( aFile ) );

            aOutStream.Close();
        }
    }
    else
        WriteInfo( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Could not generate  " ) ).Append( aOutFile.GetFull() ), EXIT_COMMONERROR );

    WriteInfo( ' ' );
}

// -----------------------------------------------------------------------

void BmpWin::Resize()
{
    aLB.SetPosSizePixel( Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void BmpWin::WriteInfo( const String& rText, BYTE cExitCode )
{
    aLB.InsertEntry( rText, nPos );
    aLB.SelectEntryPos( nPos++ );

    if( EXIT_NOERROR != cExitCode )
        aBmpApp.SetExitCode( cExitCode );

    ByteString aText( rText, RTL_TEXTENCODING_UTF8 );
#ifdef UNX
    aText.Append( "\r\n" );
#else
    aText.Append( "\n" );
#endif
    fprintf( stderr, aText.GetBuffer() );

/*
#ifdef WNT
    if( AllocConsole() )
    {
        HANDLE hHdl = GetStdHandle( STD_OUTPUT_HANDLE );

        if( hHdl )
        {
            CONSOLE_SCREEN_BUFFER_INFO aInfo;

            if( GetConsoleScreenBufferInfo( hHdl, &aInfo ) )
            {
                long nTest = 0;
            }
       }
    }
#endif
*/
}

// -----------------------------------------------------------------------

void BmpWin::ClearInfo()
{
    aLB.Clear();
    Invalidate();
    nPos = 0;
}


