/*************************************************************************
 *
 *  $RCSfile: bmpgui.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 13:41:12 $
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
#include <signal.h>

#include <sal/main.h>

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

#include "solar.hrc"
#include "filedlg.hxx"
#include "bmpcore.hxx"
#include "bmp.hrc"

// -----------
// - statics -
// -----------

static LangInfo aLangEntries[] =
{
    { "deut", 49 },
    { "enus", 1 },
    { "fren", 33 },
    { "ital", 39 },
    { "dtch", 31 },
    { "span", 34 },
    { "dan", 45 },
    { "swed", 46 },
    { "port", 03 },
    { "japn", 81 },
    { "chinsim", 86  },
    { "chintrad", 88 },
    { "russ", 07 },
    { "pol", 48 },
    { "arab",96 },
    { "greek", 30 },
    { "korean", 82 },
    { "turk", 90 },
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

public:

    void    Main();
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

// -----------------------------------------------------------------------------

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

    aLbLang.InsertEntry( String( RTL_CONSTASCII_USTRINGPARAM( "all" ) ), 0 );
    for( USHORT i = 1, nCount = ( sizeof( aLangEntries ) / sizeof( aLangEntries[ 0 ] ) ); i <= nCount; i++ )
        aLbLang.InsertEntry( String::CreateFromInt32( aLangEntries[ i - 1 ].mnLangNum ), i );

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

class BmpWin : public WorkWindow, public BmpCreator
{
private:

    ListBox         aLB;
    USHORT          nPos;

protected:

    virtual void    Message( const String& rText, BYTE cExitCode = EXIT_NOERROR );

public:

                    BmpWin();

    void            Resize();
    void            SetMode( BOOL bMode ) { aLB.SetUpdateMode( bMode ); }
    void            ClearInfo();
};

// -----------------------------------------------------------------------------

BmpWin::BmpWin() :
    WorkWindow( NULL, WB_APP | WB_STDWORK ),
    aLB( this, WB_HSCROLL | WB_VSCROLL ),
    nPos( 0 )
{
    SetText( String( RTL_CONSTASCII_USTRINGPARAM( "SVX - BmpMaker" ) ) );
    aLB.SetUpdateMode( TRUE );
    aLB.Show();
}

// -----------------------------------------------------------------------

void BmpWin::Resize()
{
    aLB.SetPosSizePixel( Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void BmpWin::Message( const String& rText, BYTE cExitCode )
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
}

// -----------------------------------------------------------------------

void BmpWin::ClearInfo()
{
    aLB.Clear();
    Invalidate();
    nPos = 0;
}

// ----------
// - BmpApp -
// ----------

void BmpApp::Main( )
{
    // Param1: filename of *.srs input file with image lists
    // Param2: path to local bitmaps
    // Param3: output directory
    // Param4: Langugage dependent directory

    ResMgr*     pAppResMgr = new ResMgr( String( RTL_CONSTASCII_USTRINGPARAM( "bmp.res" ) ), NULL, NULL );
    MenuBar*    pMB = new MenuBar();
    PopupMenu*  pPB = new PopupMenu();
    String      aLangDir;
    String      aOutName;
    int         nCmdCount = GetCommandLineParamCount();
    USHORT      nCurCmd = 0;

    Resource::SetResManager( pAppResMgr );

    cExitCode = EXIT_NOERROR;

    pMB->InsertItem( 1, String( RTL_CONSTASCII_USTRINGPARAM( "~ImageList" ) ) );
        pPB->InsertItem( 1, String( RTL_CONSTASCII_USTRINGPARAM( "~Erstellen..." ) ) );
        pPB->InsertSeparator();
        pPB->InsertItem( 2, String( RTL_CONSTASCII_USTRINGPARAM( "~Beenden" ) ) );

    pMB->SetPopupMenu( 1, pPB );
    pMB->SetSelectHdl( LINK( this, BmpApp, Select ) );

    pBmpWin = new BmpWin;
    pBmpWin->SetMenuBar( pMB );
    pBmpWin->Show();

    Execute();

    delete pPB;
    delete pMB;
    delete pBmpWin;

    Resource::SetResManager( NULL );
    delete pAppResMgr;

    if( ( EXIT_NOERROR == cExitCode ) && aOutputFileName.Len() && aOutName.Len() )
    {
        SvFileStream    aOStm( aOutputFileName, STREAM_WRITE | STREAM_TRUNC );
        ByteString      aStr( "Successfully generated ImageList(s) in: " );

        aOStm.WriteLine( aStr.Append( ByteString( aOutName, RTL_TEXTENCODING_UTF8 ) ) );
        aOStm.Close();
    }

    if( EXIT_NOERROR != cExitCode )
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
            ::std::vector< String > aInDirs;

            aCfg.WriteKey( "SRS", ByteString( aSrsPath = String( aPathDlg.GetSrsPath() ), RTL_TEXTENCODING_UTF8 ) );
            aCfg.WriteKey( "RES", ByteString( aResPath = String( aPathDlg.GetResPath() ), RTL_TEXTENCODING_UTF8 ) );
            aCfg.WriteKey( "OUT", ByteString( aOutPath = String( aPathDlg.GetOutPath() ), RTL_TEXTENCODING_UTF8 ) );
            aCfg.WriteKey( "LNG", ByteString::CreateFromInt32( nLanguage = aPathDlg.GetLangNum() ) );

            pBmpWin->ClearInfo();

            aInDirs.push_back( aResPath );

            if( getenv( "SOLARSRC" ) )
                aInDirs.push_back( ::rtl::OUString::createFromAscii( getenv( "SOLARSRC" ) ) );

            if( !nLanguage )
            {
                for ( USHORT i = 0, nCount = ( sizeof( aLangEntries ) / sizeof( aLangEntries[ 0 ] ) ); i < nCount; i++ )
                    pBmpWin->Create( aSrsPath, aInDirs, aOutPath, aLangEntries[ i ] );
            }
            else
                pBmpWin->Create( aSrsPath, aInDirs, aOutPath, aLangEntries[ nLanguage - 1 ] );
        }
    }
    else if( pMenu->GetCurItemId() == 2 )
        Quit();

    return 0;
}

BOOL SVMain();

SAL_IMPLEMENT_MAIN()
{
    SVMain();
    return 0;
}
