/*************************************************************************
 *
 *  $RCSfile: mgrtest.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:12 $
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

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
#ifndef _BASRDLL_HXX //autogen
#include <basrdll.hxx>
#endif

//#include <sv.hxx>
//#include <basic.hxx>
//#include <sostor.hxx>

// Defines fuer ToolBox-Id's
#define TB_NEW              1
#define TB_OPENSTORAGE      2
#define TB_SAVESTORAGE      3
#define TB_ORG              4
#define TB_CREATELIB1       10
#define TB_CREATELIB2       11
#define TB_CREATELIB3       12
#define TB_LOADLIB1         20
#define TB_LOADLIB2         21
#define TB_LOADLIB3         22
#define TB_STORELIBX        30
#define TB_UNLOADX          31
#define TB_LOADX            32
#define TB_EXECX            33
#define TB_REMOVEX          34
#define TB_REMOVEDELX       35

#define TB_LIB0             40
#define TB_LIB1             41
#define TB_LIB2             42
#define TB_LIB3             43

const char* pLib1Str = "Lib1";
const char* pLib2Str = "Lib2";
const char* pLib3Str = "Lib3";

// Test-Applikation
class TestApp : public Application
{
public:
    virtual void Main( void );
    virtual void Main( int, char*[] );
};

// Test-Fenster mit ToolBox zur Auswahl eines Tests
// und den typischerweise verwendeten virtuellen Methoden
class TestWindow : public WorkWindow
{
private:
    ToolBox         aToolBox;
    BasicManager*   pBasMgr;

    void            CheckError();
    USHORT          nLibX;
    DECL_LINK( BasicErrorHdl, StarBASIC * );


public:
                    TestWindow();
                    ~TestWindow();

    virtual void    Paint( const Rectangle& );
    virtual void    Resize();
    virtual void    KeyInput( const KeyEvent& rKeyEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );

    DECL_LINK( TBSelect, ToolBox * );
    void            UpdateToolBox();
    void            ShowInfo();
};

TestWindow::~TestWindow()
{
}

TestWindow::TestWindow() :
    WorkWindow( NULL, WB_APP | WB_STDWORK | WB_SVLOOK | WB_CLIPCHILDREN ) ,
    aToolBox( this, WinBits( WB_BORDER | WB_SVLOOK | WB_SCROLL | WB_LINESPACING ) )
{
    nLibX = 0;

    aToolBox.SetButtonType( BUTTON_TEXT );
    aToolBox.SetLineCount( 2 );
    aToolBox.SetPosPixel( Point( 0, 0 ) );
    aToolBox.SetSelectHdl( LINK( this, TestWindow, TBSelect ) );

    Font aFont;
    aFont.SetName( "Helv" );
    aFont.SetSize( Size( 0, 6 ) );
    aFont.SetPitch( PITCH_VARIABLE );
    aFont.SetFamily( FAMILY_SWISS );
    aFont.SetTransparent( TRUE );
    aFont.SetAlign( ALIGN_TOP );
    aToolBox.SetFont( aFont );
    SetFont( aFont );

    aToolBox.InsertItem( TB_NEW, "New" );
    aToolBox.SetHelpText( TB_NEW, "New BasicManager" );
    aToolBox.InsertItem( TB_OPENSTORAGE, "Load" );
    aToolBox.SetHelpText( TB_OPENSTORAGE, "Load Storage D:\\MYSTORE.SVS" );
    aToolBox.InsertItem( TB_SAVESTORAGE, "Save" );
    aToolBox.SetHelpText( TB_SAVESTORAGE, "Save Storage D:\\MYSTORE.SVS" );

    aToolBox.InsertSeparator();

    aToolBox.InsertItem( TB_ORG, "Verwalten" );
    aToolBox.SetHelpText( TB_ORG, "Libaries verwalten" );

    aToolBox.InsertSeparator();

    aToolBox.InsertItem( TB_LIB0, "0" );
    aToolBox.SetHelpText( TB_LIB0, "Aktuelle Lib: STANDARD" );
    aToolBox.InsertItem( TB_LIB1, "1" );
    aToolBox.SetHelpText( TB_LIB1, "Aktuelle Lib: 1" );
    aToolBox.InsertItem( TB_LIB2, "2" );
    aToolBox.SetHelpText( TB_LIB2, "Aktuelle Lib: 2" );
    aToolBox.InsertItem( TB_LIB3, "3" );
    aToolBox.SetHelpText( TB_LIB3, "Aktuelle Lib: 3" );

    aToolBox.InsertBreak();
    aToolBox.InsertItem( TB_CREATELIB1, "CreateLib1" );
    aToolBox.SetHelpText( TB_CREATELIB1, "Create Libary LIB1" );
    aToolBox.InsertItem( TB_CREATELIB2, "CreateLib2" );
    aToolBox.SetHelpText( TB_CREATELIB2, "Create Libary LIB2" );
    aToolBox.InsertItem( TB_CREATELIB3, "CreateLib3" );
    aToolBox.SetHelpText( TB_CREATELIB3, "Create Libary LIB3" );

    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_LOADLIB1, "LoadLib1" );
    aToolBox.SetHelpText( TB_LOADLIB1, "Load Libary LIB1" );
    aToolBox.InsertItem( TB_LOADLIB2, "LoadLib2" );
    aToolBox.SetHelpText( TB_LOADLIB2, "Load Libary LIB2" );
    aToolBox.InsertItem( TB_LOADLIB3, "LoadLib3" );
    aToolBox.SetHelpText( TB_LOADLIB3, "Load Libary LIB3" );

    aToolBox.InsertSeparator();
    aToolBox.InsertItem( TB_STORELIBX, "StoreLibX" );
    aToolBox.SetHelpText( TB_STORELIBX, "Store Libary LIBX" );
    aToolBox.InsertItem( TB_UNLOADX, "UnloadX" );
    aToolBox.SetHelpText( TB_UNLOADX, "Unload Libary LIBX" );
    aToolBox.InsertItem( TB_LOADX, "LoadX" );
    aToolBox.SetHelpText( TB_LOADX, "Load Libary LIBX" );
    aToolBox.InsertItem( TB_EXECX, "ExecX" );
    aToolBox.SetHelpText( TB_EXECX, "Execute 'Libary' LIBX" );
    aToolBox.InsertItem( TB_REMOVEX, "RemoveX" );
    aToolBox.SetHelpText( TB_REMOVEX, "Remove Libary LIBX" );
    aToolBox.InsertItem( TB_REMOVEDELX, "RemDelX" );
    aToolBox.SetHelpText( TB_REMOVEDELX, "Remove and delete Libary LIBX" );

    pBasMgr = 0;

    Show();
    UpdateToolBox();
    aToolBox.Show();
}
void TestWindow::ShowInfo()
{
    Invalidate();
    Update();
    long nH = GetTextSize( "X" ).Height();
    if ( pBasMgr )
    {
        Point aPos( 10, aToolBox.GetSizePixel().Height()+5 );
        for ( USHORT nLib = 0; nLib < pBasMgr->GetLibCount(); nLib++ )
        {
            String aOutStr( nLib );
            aOutStr +=": ";
            StarBASIC* pL = pBasMgr->GetLib( nLib );
            aOutStr += '[';
            aOutStr += pBasMgr->GetLibName( nLib );
            aOutStr += "]<";
            if ( pL )
                aOutStr += pL->GetName();
            else
                aOutStr += "NoLoaded";
            aOutStr += ">, Storage='";
            aOutStr += pBasMgr->GetLibStorageName( nLib );
            aOutStr += "', bLoaded=";
            aOutStr += (USHORT)pBasMgr->IsLibLoaded( nLib );
            DrawText( aPos, aOutStr );
            aPos.Y() += nH;
        }
    }
}

void TestWindow::UpdateToolBox()
{
    // Darstellung bestimmter Buttons als gecheckt oder disabled,
    // falls fuer Test gewuenscht
    aToolBox.EnableItem( TB_ORG, (BOOL)(ULONG)pBasMgr );

    aToolBox.EnableItem( TB_CREATELIB1, (BOOL)(ULONG)pBasMgr );
    aToolBox.EnableItem( TB_CREATELIB2, (BOOL)(ULONG)pBasMgr );
    aToolBox.EnableItem( TB_CREATELIB3, (BOOL)(ULONG)pBasMgr );

    aToolBox.EnableItem( TB_LOADLIB1, (BOOL)(ULONG)pBasMgr );
    aToolBox.EnableItem( TB_LOADLIB2, (BOOL)(ULONG)pBasMgr );
    aToolBox.EnableItem( TB_LOADLIB3, (BOOL)(ULONG)pBasMgr );

    aToolBox.EnableItem( TB_STORELIBX, (BOOL)(ULONG)pBasMgr );
    aToolBox.EnableItem( TB_EXECX, (BOOL)(ULONG)pBasMgr );
    aToolBox.EnableItem( TB_UNLOADX, (BOOL)(ULONG)pBasMgr );
    aToolBox.EnableItem( TB_LOADX, (BOOL)(ULONG)pBasMgr );
    aToolBox.EnableItem( TB_REMOVEX, (BOOL)(ULONG)pBasMgr );
    aToolBox.EnableItem( TB_REMOVEDELX, (BOOL)(ULONG)pBasMgr );

    aToolBox.CheckItem( TB_LIB0, nLibX == 0 );
    aToolBox.CheckItem( TB_LIB1, nLibX == 1 );
    aToolBox.CheckItem( TB_LIB2, nLibX == 2 );
    aToolBox.CheckItem( TB_LIB3, nLibX == 3 );
}

IMPL_LINK( TestWindow, TBSelect, ToolBox *, p )
{
    USHORT nId = aToolBox.GetCurItemId();
    BOOL bChecked = aToolBox.IsItemChecked( nId );
    switch ( nId )
    {
        case TB_NEW:
        {
            delete pBasMgr;
            pBasMgr = new BasicManager( new StarBASIC );
            pBasMgr->SetStorageName( "d:\\mystore.svs" );
        }
        break;
        case TB_OPENSTORAGE:
        {
            delete pBasMgr;
            SvStorageRef xStorage = new SvStorage( "d:\\mystore.svs", STREAM_READ | STREAM_SHARE_DENYWRITE );
            DBG_ASSERT( xStorage.Is(), "Kein Storage!" );
            pBasMgr = new BasicManager( *xStorage );
        }
        break;
        case TB_SAVESTORAGE:
        {
            if ( pBasMgr)
            {
                SvStorageRef xStorage = new SvStorage( "d:\\mystore.svs" );
                DBG_ASSERT( xStorage.Is(), "Kein Storage!" );
                pBasMgr->Store( *xStorage );
            }
        }
        break;
        case TB_ORG:
        {
            if ( pBasMgr)
            {
                InfoBox( 0, "Organisieren..." ).Execute();
            }
        }
        break;
        case TB_CREATELIB1:
        {
            if ( pBasMgr )
            {
                USHORT nLib = pBasMgr->GetLibId( pBasMgr->CreateLib( pLib1Str ) );
                if ( nLib != LIB_NOTFOUND )
                {
                    pBasMgr->SetLibStorageName( nLib, "d:\\mystore.svs" );
                    StarBASIC* pLib = pBasMgr->GetLib( pLib1Str );
                    DBG_ASSERT( pLib, "Lib?!" );
                    String aSource( "Sub SubInLib1Mod1\nprint\"XXX\"\nEnd Sub");
                    SbModule* pM = pLib->MakeModule( "ModLib1", aSource  );
                    DBG_ASSERT( pM, "Modul?" );
                    pLib->Compile( pM );
                }
                else
                    InfoBox( 0, "CreateLibary fehlgeschlagen..." ).Execute();
            }
        }
        break;
        case TB_CREATELIB2:
        {
            if ( pBasMgr )
            {
                USHORT nLib = pBasMgr->GetLibId( pBasMgr->CreateLib( pLib2Str ) );
                if ( nLib != LIB_NOTFOUND )
                {
                    pBasMgr->SetLibStorageName( nLib, "d:\\mystore.svs" );
                    StarBASIC* pLib = pBasMgr->GetLib( pLib2Str );
                    DBG_ASSERT( pLib, "Lib?!" );
                    SbModule* pM = pLib->MakeModule( "ModuleLib2", "Sub SubInLib2\n print \"Tralala\" \nEnd Sub\n" );
                    pLib->Compile( pM );
                }
                else
                    InfoBox( 0, "CreateLibary fehlgeschlagen..." ).Execute();
            }
        }
        break;
        case TB_CREATELIB3:
        {
            if ( pBasMgr )
            {
                // liegt in einem anderen Storage !!!
                USHORT nLib = pBasMgr->GetLibId( pBasMgr->CreateLib( pLib3Str ) );
                if ( nLib != LIB_NOTFOUND )
                {
                    pBasMgr->SetLibStorageName( nLib, "d:\\mystore2.svs" );
                    StarBASIC* pLib = pBasMgr->GetLib( pLib3Str );
                    DBG_ASSERT( pLib, "Lib?!" );
                    SbModule* pM = pLib->MakeModule( "ModuleLib2", "Sub XYZInLib3\n print \"?!\" \nEnd Sub\n" );
                    pLib->Compile( pM );
                }
                else
                    InfoBox( 0, "CreateLibary fehlgeschlagen..." ).Execute();
            }
        }
        break;
        case TB_LOADLIB1:
        {
            if ( pBasMgr )
            {
                SvStorageRef xStorage = new SvStorage( "d:\\mystore.svs" );
                if ( !pBasMgr->AddLib( *xStorage, pLib1Str, FALSE ) )
                    Sound::Beep();
            }
        }
        break;
        case TB_LOADLIB2:
        {
            if ( pBasMgr )
            {
                SvStorageRef xStorage = new SvStorage( "d:\\mystore.svs" );
                if ( !pBasMgr->AddLib( *xStorage, pLib2Str, FALSE ) )
                    Sound::Beep();
            }
        }
        break;
        case TB_LOADLIB3:
        {
            if ( pBasMgr )
            {
                // liegt in einem anderen Storage !!!
                SvStorageRef xStorage = new SvStorage( "d:\\mystore2.svs" );
                if ( !pBasMgr->AddLib( *xStorage, pLib3Str, FALSE ) )
                    Sound::Beep();
            }
        }
        break;
        case TB_STORELIBX:
        {
            if ( pBasMgr )
                pBasMgr->StoreLib( nLibX );
        }
        break;
        case TB_UNLOADX:
        {
            if ( pBasMgr )
                pBasMgr->UnloadLib( nLibX );
        }
        break;
        case TB_LOADX:
        {
            if ( pBasMgr )
                pBasMgr->LoadLib( nLibX );
        }
        break;
        case TB_REMOVEX:
        {
            if ( pBasMgr )
                pBasMgr->RemoveLib( nLibX, FALSE );
        }
        break;
        case TB_REMOVEDELX:
        {
            if ( pBasMgr )
                pBasMgr->RemoveLib( nLibX, TRUE );
        }
        break;
        case TB_EXECX:
        {
            if ( pBasMgr )
            {
                StarBASIC* pBasic = pBasMgr->GetLib( nLibX );
                if ( pBasic && pBasic->GetModules()->Count() )
                {
                    pBasic->SetErrorHdl( LINK( this, TestWindow, BasicErrorHdl ) );

                    SbModule* pMod = (SbModule*)pBasic->GetModules()->Get( 0 );
                    if ( pMod && pMod->GetMethods()->Count() )
                        pMod->GetMethods()->Get(0)->GetInteger();
                }
            }
        }
        break;

        case TB_LIB0:   nLibX = 0;
        break;
        case TB_LIB1:   nLibX = 1;
        break;
        case TB_LIB2:   nLibX = 2;
        break;
        case TB_LIB3:   nLibX = 3;
        break;
    }

    UpdateToolBox();
    CheckError();
    ShowInfo();
    return 0;
}

void TestWindow::CheckError()
{
     if ( pBasMgr )
     {
        BasicError* pError = pBasMgr->GetFirstError();
        while ( pError )
        {
            String aErrorStr;
            String aReasonStr;
            switch ( pError->GetErrorId() )
            {
                case BASERR_ID_STDLIBOPEN:
                    aErrorStr = "Standard-Lib konnte nicht geoffnet werden.";
                break;
                case BASERR_ID_STDLIBSAVE:
                    aErrorStr = "Standard-Lib konnte nicht gespeichert werden.";
                break;
                case BASERR_ID_LIBLOAD:
                    aErrorStr = "Lib konnte nicht geoffnet werden.";
                break;
                case BASERR_ID_LIBCREATE:
                    aErrorStr = "Lib konnte nicht erzeugt werden.";
                break;
                case BASERR_ID_LIBSAVE:
                    aErrorStr = "Lib konnte nicht gespeichert werden.";
                break;
                case BASERR_ID_MGROPEN:
                    aErrorStr = "Manager konnte nicht geladen werden.";
                break;
                case BASERR_ID_MGRSAVE:
                    aErrorStr = "Manager konnte nicht gespeichert werden.";
                break;
                case BASERR_ID_UNLOADLIB:
                    aErrorStr = "Libary konnte nicht entladen werden.";
                break;
                case BASERR_ID_REMOVELIB:
                    aErrorStr = "Libary konnte nicht entfernt werden.";
                break;
                default:
                    aErrorStr = "Unbekannter Fehler!";
            }

            switch ( pError->GetReason() )
            {
                case BASERR_REASON_OPENSTORAGE:
                    aReasonStr = "Der Storage konnte nicht geoeffnet werden";
                break;
                case BASERR_REASON_OPENLIBSTORAGE:
                    aReasonStr = "Der Lib-Storage konnte nicht geoeffnet werden";
                break;
                case BASERR_REASON_OPENMGRSTREAM:
                    aReasonStr = "Der Manager-Stream konnte nicht geoeffnet werden";
                break;
                case BASERR_REASON_OPENLIBSTREAM:
                    aReasonStr = "Der Basic-Stream konnte nicht geoeffnet werden";
                break;
                case BASERR_REASON_STDLIB:
                    aReasonStr = "STANDARD-Lib";
                break;
                case BASERR_REASON_BASICLOADERROR:
                    aReasonStr = "Fehler beim Laden des Basics";
                default:
                    aReasonStr = " - ";
            }

            String aErr( aErrorStr );
            aErr += "\nGrund: ";
            aErr += aReasonStr;
            InfoBox( 0, aErr ).Execute();

            pError = pBasMgr->GetNextError();
        }
        pBasMgr->ClearErrors();
     }
}

void __EXPORT TestWindow::Paint( const Rectangle& rRec )
{
}

void __EXPORT TestWindow::Resize()
{
    Size aTBSz = aToolBox.CalcWindowSizePixel();
    aToolBox.SetSizePixel( Size( GetOutputSizePixel().Width(), aTBSz.Height()) );
    Invalidate();
    ShowInfo();
}

void __EXPORT TestWindow::KeyInput( const KeyEvent& rKEvt )
{
    char nCharCode = rKEvt.GetCharCode();
    USHORT nCode = rKEvt.GetKeyCode().GetCode();

    // Nur bei Alt-Return
    if ( ( nCode == KEY_RETURN ) && rKEvt.GetKeyCode().IsMod2() )
        ;
    else
        WorkWindow::KeyInput( rKEvt );

    UpdateToolBox();
}

void __EXPORT TestWindow::MouseMove( const MouseEvent& rMEvt )
{
}

void __EXPORT TestWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    ShowInfo();
}

void __EXPORT TestWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    UpdateToolBox();
}

IMPL_LINK( TestWindow, BasicErrorHdl, StarBASIC *, pBasic )
{
    String aErrorText( pBasic->GetErrorText() );

    String aErrorTextPrefix;
    if( pBasic->IsCompilerError() )
    {
        aErrorTextPrefix = "Compilererror: ";
    }
    else
    {
        aErrorTextPrefix = "Runtimeerror: ";
        aErrorTextPrefix += pBasic->GetErrorCode();
        aErrorTextPrefix += " ";
    }

    InfoBox( 0, String( aErrorTextPrefix + aErrorText ) ).Execute();
    return 0;
}

void __EXPORT TestApp::Main( void )
{
    Main( 0, NULL );
}

void __EXPORT TestApp::Main( int, char*[] )
{
    BasicDLL aBasiDLL;
    SvFactory::Init();
    EnableSVLook();
    TestWindow aWindow;
    Execute();
    SvFactory::DeInit();
}


TestApp aTestApp;
