/*************************************************************************
 *
 *  $RCSfile: app.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: gh $ $Date: 2001-03-13 15:55:46 $
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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif
#ifndef _SV_FILEDLG_HXX //autogen
#include <svtools/filedlg.hxx>
#endif
#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif

#include <vcl/system.hxx>

#include <vcl/font.hxx>

#ifndef _BASIC_TTRESHLP_HXX
#include "ttstrhlp.hxx"
#endif

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifdef VCL
#include <svtools/filedlg.hxx>
#endif

#include <osl/module.h>

#include "basic.hrc"
#include "app.hxx"
#include "printer.hxx"
#include "status.hxx"
#include "appedit.hxx"
#include "appbased.hxx"
#include "apperror.hxx"
#include "mybasic.hxx"
#include "ttbasic.hxx"
#include "dialogs.hxx"
#include "basrdll.hxx"

#ifndef _RUNTIME_HXX
#include "runtime.hxx"
#endif

#ifndef _SB_INTERN_HXX
#include "sbintern.hxx"
#endif

#ifdef _USE_UNO
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/configurationkeys.hxx>
#ifndef _COMPHELPER_REGPATHHELPER_HXX_
#include <comphelper/regpathhelper.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HDL_
#include <com/sun/star/beans/PropertyValue.hdl>
#endif
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>

#include <ucbhelper/content.hxx>

using namespace comphelper;
using namespace ucb;
using namespace cppu;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;

#endif /* _USE_UNO */


BasicApp aBasicApp;                     // Applikations-Instanz

#ifndef SAL_MODULENAME      // should be defined from src612 on
#define SAL_MODULENAME( hh ) hh
#endif

#ifdef _USE_UNO
Reference< XContentProviderManager > InitializeUCB( void )
{
    Reference< XContentProviderManager > xUcb;
#ifdef DEBUG
    ::rtl::OUString test(getPathToSystemRegistry());
#endif
    try
    {

    //////////////////////////////////////////////////////////////////////
    // Bootstrap readonly service factory
    Reference< XMultiServiceFactory > xSMgr( createRegistryServiceFactory( getPathToSystemRegistry(), sal_True ) );

    //////////////////////////////////////////////////////////////////////
    // Register libraries, check first if already registered
    if( !xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ).is() )
    {
        //////////////////////////////////////////////////////////////////////
        // Bootstrap writable service factory
        xSMgr.clear();
        xSMgr = createRegistryServiceFactory( getPathToSystemRegistry() );

        Reference< XImplementationRegistration >
            xIR( xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.registry.ImplementationRegistration" ) ), UNO_QUERY );

        xIR->registerImplementation( OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                                        OUString::createFromAscii(SAL_MODULENAME( "ucb1" )),
                                        Reference< XSimpleRegistry >() );
        xIR->registerImplementation( OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                                        OUString::createFromAscii(SAL_MODULENAME( "ucpfile1" )),
                                        Reference< XSimpleRegistry >() );
        xIR->registerImplementation( OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                                        OUString::createFromAscii(SAL_MODULENAME( "fileacc" )),
                                        Reference< XSimpleRegistry >() );

/*      // Packages
        xIR->registerImplementation( OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                                        OUString::createFromAscii(SAL_MODULENAME( "ucppkg1" )),
                                        Reference< XSimpleRegistry >() );
        xIR->registerImplementation( OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                                        OUString::createFromAscii(SAL_MODULENAME( "package2" )),
                                        Reference< XSimpleRegistry >() );
        xIR->registerImplementation( OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                                        OUString::createFromAscii(SAL_MODULENAME( "rdbtdp" )),
                                        Reference< XSimpleRegistry >() );
        xIR->registerImplementation( OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                                        OUString::createFromAscii(SAL_MODULENAME( "cfgmgr2" )),
                                        Reference< XSimpleRegistry >() );
        xIR->registerImplementation( OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                                        OUString::createFromAscii(SAL_MODULENAME( "tcv" )),
                                        Reference< XSimpleRegistry >() );
        xIR->registerImplementation( OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                                        OUString::createFromAscii(SAL_MODULENAME( "sax" )),
                                        Reference< XSimpleRegistry >() );
*/

        // i18n
        xIR->registerImplementation( OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                                        OUString::createFromAscii(SVLIBRARY( "int" )),
                                        Reference< XSimpleRegistry >() );

        //////////////////////////////////////////////////////////////////////
        // Bootstrap readonly service factory again
        xSMgr = createRegistryServiceFactory( getPathToSystemRegistry(), sal_True );
    }

    //////////////////////////////////////////////////////////////////////
    // set global factory
    setProcessServiceFactory( xSMgr );

/*  // Create simple ConfigManager
    Sequence< Any > aConfArgs(3);
    aConfArgs[0] <<= PropertyValue( OUString::createFromAscii("servertype"), 0, makeAny( OUString::createFromAscii("local") ), ::com::sun::star::beans::PropertyState_DIRECT_VALUE );
    aConfArgs[1] <<= PropertyValue( OUString::createFromAscii("sourcepath"), 0, makeAny( OUString::createFromAscii("g:\\") ), ::com::sun::star::beans::PropertyState_DIRECT_VALUE );
    aConfArgs[2] <<= PropertyValue( OUString::createFromAscii("updatepath"), 0, makeAny( OUString::createFromAscii("g:\\") ), ::com::sun::star::beans::PropertyState_DIRECT_VALUE );

    Reference< XContentProvider > xConfProvider
        ( xSMgr->createInstanceWithArguments( OUString::createFromAscii( "com.sun.star.configuration.ConfigurationProvider" ), aConfArgs), UNO_QUERY );

*/
    // Create unconfigured Ucb:
/*  Sequence< Any > aArgs(1);
    aArgs[1] = makeAny ( xConfProvider );*/
    Sequence< Any > aArgs;
    ucb::ContentBroker::initialize( xSMgr, aArgs );
    xUcb = ucb::ContentBroker::get()->getContentProviderManagerInterface();

    Reference< XContentProvider > xFileProvider
        ( xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.ucb.FileContentProvider" ) ), UNO_QUERY );
    xUcb->registerContentProvider( xFileProvider, OUString::createFromAscii( "file" ), sal_True );


/*  Reference< XContentProvider > xPackageProvider
        ( xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.ucb.PackageContentProvider" ) ), UNO_QUERY );
    xUcb->registerContentProvider( xPackageProvider, OUString::createFromAscii( "vnd.sun.star.pkg" ), sal_True );
    */

#ifdef DEBUG
    ucb::Content aTester( OUString::createFromAscii("file:///x:/gh"),com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >());
    BOOL bFolder = aTester.isFolder();
#endif


    }
    catch( Exception & rEx)
    {
        DBG_ERROR( ByteString( String( rEx.Message ), RTL_TEXTENCODING_ASCII_US).GetBuffer() )
        throw;
    }
    catch( ... )
    {
        DBG_ERROR( "unknown exception occured" )
        throw;
    }
    return xUcb;
}
#endif

void BasicApp::Main( )
{
    try
    {
#ifdef _USE_UNO
    Reference< XContentProviderManager > xUcb = InitializeUCB();
#endif
    {
        LanguageType aRequestedLanguage;
        Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));

        // 1033 = LANGUAGE_ENGLISH_US
        // 1031 = LANGUAGE_GERMAN
        aConf.SetGroup("Misc");
        ByteString aLang = aConf.ReadKey( "Language", ByteString::CreateFromInt32( LANGUAGE_SYSTEM ) );
        aRequestedLanguage = LanguageType( aLang.ToInt32() );

        AllSettings aSettings = GetSettings();
        International aInternational;
        aInternational = GetSettings().GetInternational();
        aInternational = International( aRequestedLanguage );
        aSettings.SetInternational( aInternational );
        SetSettings( aSettings );
        aInternational = GetSettings().GetInternational();
    }


//  ResMgr::CreateResMgr( CREATEVERSIONRESMGR( stt ),  )
//const char* ResMgr::GetLang( LanguageType& nType, USHORT nPrio )

    Resource::SetResManager( CREATEVERSIONRESMGR( stt ) );
//  ResMgr::CreateResMgr( CREATEVERSIONRESMGR( stt )
//  ResMgr *pRes = new ResMgr( "testtool.res" );
//  Resource::SetResManager( pRes );

    BasicDLL aBasicDLL;
    nWait = 0;

    // Hilfe:
//  pHelp = new Help;
//  SetHelp( pHelp );
//  Help::EnableContextHelp();
//  Help::EnableExtHelp();
//  DeactivateExtHelp();

    // Acceleratoren
    Accelerator aAccel( ResId( MAIN_ACCEL ) );
    InsertAccel( &aAccel );
    pMainAccel = &aAccel;

    // Frame Window:
    pFrame = new BasicFrame;
    aAccel.SetSelectHdl( LINK( pFrame, BasicFrame, Accel ) );

    pFrame->Show();

    SetSystemWindowMode( SYSTEMWINDOW_MODE_DIALOG );

    PostUserEvent( LINK( this, BasicApp, LateInit ) );
    Execute();

    // Loeschen der Members:
//  delete pHelp;
    delete pFrame;

    RemoveAccel( pMainAccel );

    }
    catch( class Exception & rEx)
    {
        DBG_ERROR( ByteString( String( rEx.Message ), RTL_TEXTENCODING_ASCII_US).GetBuffer() )
        throw;
    }
    catch( ... )
    {
        DBG_ERROR( "unknown exception occured" )
        throw;
    }
}

void BasicApp::LoadIniFile()
{
    pFrame->LoadIniFile();
}

void BasicApp::SetFocus()
{
    if( pFrame->pWork && pFrame->pWork->ISA(AppEdit) )
      ((AppEdit*)pFrame->pWork)->pDataEdit->GrabFocus();
}

IMPL_LINK( BasicApp, LateInit, void *, pDummy )
{
    BOOL bFileLoaded = FALSE;
    for ( int i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
    {
        if ( Application::GetCommandLineParam( i ).Copy(0,1).CompareToAscii("-") != COMPARE_EQUAL )
        {
            pFrame->LoadFile( Application::GetCommandLineParam( i ) );
            bFileLoaded = TRUE;
        }
        else
        {
            if ( Application::GetCommandLineParam( i ).Copy(0,4).CompareIgnoreCaseToAscii("-run") == COMPARE_EQUAL )
                pFrame->SetAutoRun( TRUE );
        }
    }

    if ( !bFileLoaded )
    {
        AppWin *pWin = new AppBasEd( pFrame, NULL );
        pWin->Show();
    }

    pFrame->pStatus->SetStatusSize( pFrame->pStatus->GetStatusSize()+1 );
    pFrame->pStatus->SetStatusSize( pFrame->pStatus->GetStatusSize()-1 );

    if ( pFrame->IsAutoRun() )
    {
        pFrame->Command( RID_RUNSTART );
    }

    if ( pFrame->IsAutoRun() )
        pFrame->Command( RID_QUIT );

    return 0;
}

//////////////////////////////////////////////////////////////////////////

class FloatingExecutionStatus : public FloatingWindow
{
public:
    FloatingExecutionStatus( Window * pParent );
    void SetStatus( String aW );
    void SetAdditionalInfo( String aF );

private:
    Timer aAusblend;
    DECL_LINK(HideNow, FloatingExecutionStatus* );
    FixedText aStatus;
    FixedText aAdditionalInfo;
};


FloatingExecutionStatus::FloatingExecutionStatus( Window * pParent )
    : FloatingWindow( pParent, ResId(LOAD_CONF) ),
    aStatus( this, ResId( WORK ) ),
    aAdditionalInfo( this, ResId( FILENAME ) )
{
    FreeResource();
    aAusblend.SetTimeoutHdl( LINK(this, FloatingExecutionStatus, HideNow ) );
    aAusblend.SetTimeout(5000);             // in ms
    aAusblend.Start();
}

void FloatingExecutionStatus::SetStatus( String aW )
{
    Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
    ToTop( TOTOP_NOGRABFOCUS );
    aAusblend.Start();
    aStatus.SetText( aW );
}

void FloatingExecutionStatus::SetAdditionalInfo( String aF )
{
    Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
    ToTop( TOTOP_NOGRABFOCUS );
    aAusblend.Start();
    aAdditionalInfo.SetText( aF );
}

IMPL_LINK(FloatingExecutionStatus, HideNow, FloatingExecutionStatus*, pFLC )
{
    Hide();
    return 0;
}

//////////////////////////////////////////////////////////////////////////

TYPEINIT1(TTExecutionStatusHint, SfxSimpleHint);

BasicFrame::BasicFrame() : WorkWindow( NULL,
    WinBits( WB_APP | WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) )
, bIsAutoRun ( FALSE )
, pDisplayHidDlg( NULL )
, pBasic( NULL )
, pWork( NULL )
, pExecutionStatus( NULL )
{

    Application::SetDefDialogParent( this );
    pBasic  = TTBasic::CreateMyBasic();     // depending on what was linked to the executable
    bInBreak = FALSE;
    bDisas = FALSE;
    nFlags  = 0;
//  Icon aAppIcon;

    if ( pBasic->pTestObject )  // also sid wir testtool
    {
//      aAppIcon = Icon( ResId( RID_APPICON2 ) );
        aAppName = String( ResId( IDS_APPNAME2 ) );
    }
    else
    {
//      aAppIcon = Icon( ResId( RID_APPICON ) );
        aAppName = String( ResId( IDS_APPNAME ) );
    }

    // Menu:
    MenuBar *pBar = new MenuBar( ResId( RID_APPMENUBAR ) );
    SetMenuBar( pBar );

    pBar->SetHighlightHdl( LINK( this, BasicFrame, HighlightMenu ) );


    // Menu Handler:
    PopupMenu* pFileMenu = pBar->GetPopupMenu( RID_APPFILE );
    pFileMenu->SetSelectHdl( LINK( this, BasicFrame, MenuCommand ) );
    pFileMenu->SetHighlightHdl( LINK( this, BasicFrame, HighlightMenu ) );
    pFileMenu->SetActivateHdl( LINK( this, BasicFrame, InitMenu ) );
    pFileMenu->SetDeactivateHdl( LINK( this, BasicFrame, DeInitMenu ) );
    if (Basic().pTestObject )       // Wir sind also TestTool
    {
        pFileMenu->RemoveItem( pFileMenu->GetItemPos( RID_FILELOADLIB ) -1 );   // Der Trenner davor
        pFileMenu->RemoveItem( pFileMenu->GetItemPos( RID_FILELOADLIB ) );
        pFileMenu->RemoveItem( pFileMenu->GetItemPos( RID_FILESAVELIB ) );
    }

    PopupMenu* pEditMenu = pBar->GetPopupMenu( RID_APPEDIT );
    pEditMenu->SetSelectHdl( LINK( this, BasicFrame, MenuCommand ) );
    pEditMenu->SetHighlightHdl( LINK( this, BasicFrame, HighlightMenu ) );
    pEditMenu->SetActivateHdl( LINK( this, BasicFrame, InitMenu ) );
    pEditMenu->SetDeactivateHdl( LINK( this, BasicFrame, DeInitMenu ) );
    PopupMenu* pRunMenu = pBar->GetPopupMenu( RID_APPRUN );
    pRunMenu->SetSelectHdl( LINK( this, BasicFrame, MenuCommand ) );
    pRunMenu->SetHighlightHdl( LINK( this, BasicFrame, HighlightMenu ) );
    pRunMenu->SetActivateHdl( LINK( this, BasicFrame, InitMenu ) );
    pRunMenu->SetDeactivateHdl( LINK( this, BasicFrame, DeInitMenu ) );
    if (Basic().pTestObject )       // Wir sind also TestTool
    {
        pRunMenu->RemoveItem( pRunMenu->GetItemPos( RID_RUNDISAS ) );
    }

    PopupMenu *pExtras;
    if (Basic().pTestObject )       // Wir sind also TestTool
    {
        pExtras = new PopupMenu( ResId( RID_TT_EXTRAS ) );
        pBar->InsertItem( RID_TT_EXTRAS, String( ResId( RID_TT_EXTRAS_NAME ) ), 0, pBar->GetItemPos( RID_APPWINDOW ) );
        pBar->SetPopupMenu( RID_TT_EXTRAS, pExtras );

        pExtras->SetSelectHdl( LINK( this, BasicFrame, MenuCommand ) );
        pExtras->SetHighlightHdl( LINK( this, BasicFrame, HighlightMenu ) );
        pExtras->SetDeactivateHdl( LINK( this, BasicFrame, DeInitMenu ) );
    }

    PopupMenu* pWinMenu = pBar->GetPopupMenu( RID_APPWINDOW );
    pWinMenu->SetSelectHdl( LINK( this, BasicFrame, MenuCommand ) );
    pWinMenu->SetHighlightHdl( LINK( this, BasicFrame, HighlightMenu ) );
    pWinMenu->SetDeactivateHdl( LINK( this, BasicFrame, DeInitMenu ) );
    PopupMenu* pHelpMenu = pBar->GetPopupMenu( RID_APPHELP );
    pHelpMenu->SetSelectHdl( LINK( this, BasicFrame, MenuCommand ) );
    pHelpMenu->SetHighlightHdl( LINK( this, BasicFrame, HighlightMenu ) );
    pHelpMenu->SetActivateHdl( LINK( this, BasicFrame, InitMenu ) );
    pHelpMenu->SetDeactivateHdl( LINK( this, BasicFrame, DeInitMenu ) );

    LoadLRU();

    LoadIniFile();

#ifndef UNX
    pPrn    = new BasicPrinter;
#else
    pPrn    = NULL;
#endif
    pList   = new EditList;
    pStatus = new StatusLine( this );

    UpdateTitle();
//  SetIcon( aAppIcon );

    // Groesse: halbe Breite, dreiviertel Hoehe minus 2 * IconSize
    {
        Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
        aConf.SetGroup("WinGeom");
        SetWindowState( aConf.ReadKey("WinParams", "") );
    }

//  pWork = new AppEdit( this, NULL );
//  pWork->Show();
//  pWork->Close();

    aLineNum.SetTimeoutHdl( LINK( this, BasicFrame, ShowLineNr ) );
    aLineNum.SetTimeout(200);
    aLineNum.Start();


    aCheckFiles.SetTimeout( 10000 );
    aCheckFiles.SetTimeoutHdl( LINK( this, BasicFrame, CheckAllFiles ) );
    aCheckFiles.Start();

    GetMenuBar()->SetCloserHdl( LINK( this, BasicFrame, CloseButtonClick ) );
    GetMenuBar()->SetFloatButtonClickHdl( LINK( this, BasicFrame, FloatButtonClick ) );
    GetMenuBar()->SetHideButtonClickHdl( LINK( this, BasicFrame, HideButtonClick ) );
}

void BasicFrame::LoadIniFile()
{
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("Misc");

    ByteString aTemp = aConf.ReadKey( "AutoReload", "0" );
    bAutoReload = ( aTemp.CompareTo("1") == COMPARE_EQUAL );

    if ( pBasic )
        pBasic->LoadIniFile();
}

BasicFrame::~BasicFrame()
{
    AppWin* p = pList->First();
    DBG_ASSERT( !p, "Still open FileWindows");
    if( p )
        while( (p = pList->Remove() ) != NULL )
            delete p;

    MenuBar *pBar = GetMenuBar();
    SetMenuBar( NULL );
    delete pBar;

    delete pStatus;
    delete pPrn;
    delete pList;
//  delete pExecutionStatus;
//  delete pBasic;
    pBasic.Clear();     // Da jetzt REF
}

void BasicFrame::UpdateTitle()
{
    String aTitle;
    aTitle += aAppName;
    if ( aAppMode.Len() )
    {
        aTitle.AppendAscii(" [");
        aTitle += aAppMode;
        aTitle.AppendAscii("]");
    }
    aTitle.AppendAscii(" - ");
    aTitle += aAppFile;
    SetText( aTitle );
}

IMPL_LINK( BasicFrame, CheckAllFiles, Timer*, pTimer )
{
    if ( pWork )
    {
        AppWin* pStartWin = pWork;
        Window* pFocusWin = Application::GetFocusWindow();
        for ( int i = pList->Count()-1 ; i >= 0 ; i-- )
            pList->GetObject( i )->CheckReload();

        if ( pWork != pStartWin )
        {
            pWork = pStartWin;
            pWork->ToTop();
        }
        if ( pFocusWin )
            pFocusWin->GrabFocus();
    }
    pTimer->Start();
    return 0;
}

BOOL BasicFrame::IsAutoRun()
{
    return bIsAutoRun;
}

void BasicFrame::SetAutoRun( BOOL bAuto )
{
    bIsAutoRun = bAuto;
}

void BasicFrame::SFX_NOTIFY( SfxBroadcaster&, const TypeId&,
                            const SfxHint& rHint, const TypeId& )
{
    if ( rHint.ISA( TTExecutionStatusHint ) )
    {
        TTExecutionStatusHint *pStatusHint = ( TTExecutionStatusHint* )&rHint;
        switch ( pStatusHint->GetType() )
        {
            case TT_EXECUTION_ENTERWAIT:
                {
                    EnterWait();
                }
                break;
            case TT_EXECUTION_LEAVEWAIT:
                {
                    LeaveWait();
                }
                break;
            case TT_EXECUTION_SHOW_ACTION:
                {
                    if ( !pExecutionStatus )
                        pExecutionStatus = new FloatingExecutionStatus( this );
                    pExecutionStatus->SetStatus( pStatusHint->GetExecutionStatus() );
                    pExecutionStatus->SetAdditionalInfo( pStatusHint->GetAdditionalExecutionStatus() );
                }
                break;
        }
    }


    Broadcast( rHint );
}

void BasicFrame::Resize()
{
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("WinGeom");
    aConf.WriteKey("WinParams",GetWindowState());

    // Statusbar
    Size aOutSize = GetOutputSizePixel();
    Size aStatusSize = pStatus->GetSizePixel();
    Point aStatusPos( 0, aOutSize.Height() - aStatusSize.Height() );
    aStatusSize.Width() = aOutSize.Width();

    pStatus->SetPosPixel( aStatusPos );
    pStatus->SetSizePixel( aStatusSize );


    // Eventuell Maximized window resizen
    ULONG i;
    for( i = pList->Count(); i > 0 ; i-- )
    {
        if ( pList->GetObject( i-1 )->GetWinState() == TT_WIN_STATE_MAX )
            pList->GetObject( i-1 )->Maximize();    // resized auch
    }
}

void BasicFrame::Move()
{
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("WinGeom");
    aConf.WriteKey("WinParams",GetWindowState());
}

IMPL_LINK( BasicFrame, CloseButtonClick, void*, EMPTYARG )
{
    AppWin* p;
    for ( p = pList->Last() ; p && p->GetWinState() != TT_WIN_STATE_MAX ; p = pList->Prev() )
    {};
    if ( p )
        p->GrabFocus();
    return Command( RID_FILECLOSE, FALSE );
}

IMPL_LINK( BasicFrame, FloatButtonClick, void*, EMPTYARG )
{
    AppWin* p;
    for ( p = pList->Last() ; p && p->GetWinState() != TT_WIN_STATE_MAX ; p = pList->Prev() )
    {};
    if ( p )
        p->TitleButtonClick( TITLE_BUTTON_DOCKING );
    return 1;
}

IMPL_LINK( BasicFrame, HideButtonClick, void*, EMPTYARG )
{
    AppWin* p;
    for ( p = pList->Last() ; p && p->GetWinState() != TT_WIN_STATE_MAX ; p = pList->Prev() )
    {};
    if ( p )
        p->TitleButtonClick( TITLE_BUTTON_HIDE );
    return 1;
}

void BasicFrame::WinShow_Hide()
{
    if ( !pList->Count() )
        return;

    AppWin* p;
    BOOL bWasFullscreen = FALSE;
    for ( p = pList->Last() ; p ; p = pList->Prev() )
    {
        if ( p->pDataEdit )
        {
            if ( p->GetWinState() & TT_WIN_STATE_HIDE   // Versteckt
                ||  ( bWasFullscreen
                        && ( !p->IsPined() || p->GetWinState() & TT_WIN_STATE_MAX )
                    )
               )
                p->Hide( SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
            else
                p->Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
        }
        bWasFullscreen |= p->GetWinState() == TT_WIN_STATE_MAX;
    }
}

void BasicFrame::WinMax_Restore()
{
    // Die ApplicationButtons
    AppWin* p;
    BOOL bHasFullscreenWin = FALSE;
    for( p = pList->First(); p && !bHasFullscreenWin ; p = pList->Next() )
        bHasFullscreenWin |= ( p->GetWinState() == TT_WIN_STATE_MAX );
    GetMenuBar()->ShowButtons( bHasFullscreenWin, bHasFullscreenWin, bHasFullscreenWin );
    WinShow_Hide();
}

void BasicFrame::RemoveWindow( AppWin *pWin )
{
//  delete pIcon;
    pList->Remove( pWin );
    pWork = pList->Last();

    WinShow_Hide();

    if ( pWork )
        pWork->ToTop();

    WinMax_Restore();

    Menu* pMenu = GetMenuBar();
    if( pList->Count() == 0 ) {
        pMenu->EnableItem( RID_APPEDIT,   FALSE );
        pMenu->EnableItem( RID_APPRUN,    FALSE );
        pMenu->EnableItem( RID_APPWINDOW, FALSE );
    }

    PopupMenu* pWinMenu = pMenu->GetPopupMenu( RID_APPWINDOW );

    pWinMenu->RemoveItem( pWinMenu->GetItemPos( pWin->GetWinId() ) );

    // Trenner entfernen
    if ( pWinMenu->GetItemType( pWinMenu->GetItemCount() - 1 ) == MENUITEM_SEPARATOR )
        pWinMenu->RemoveItem( pWinMenu->GetItemCount() - 1 );

    pStatus->LoadTaskToolBox();
}

void BasicFrame::AddWindow( AppWin *pWin )
{
    // Eintragen:
    pList->Insert( pWin, LIST_APPEND );
    pWork = pWin;

    WinMax_Restore();

    // Hauptmenue aktivieren:
    MenuBar* pMenu = GetMenuBar();
    if( pList->Count() > 0 ) {
        pMenu->EnableItem( RID_APPEDIT,   TRUE );
        pMenu->EnableItem( RID_APPRUN,    TRUE );
        pMenu->EnableItem( RID_APPWINDOW, TRUE );
    }

    PopupMenu* pWinMenu = pMenu->GetPopupMenu( RID_APPWINDOW );
    USHORT nLastID = pWinMenu->GetItemId( pWinMenu->GetItemCount() - 1 );

    // Trenner erforderlich
    if ( nLastID < RID_WIN_FILE1 && pWinMenu->GetItemType( pWinMenu->GetItemCount() - 1 ) != MENUITEM_SEPARATOR )
        pWinMenu->InsertSeparator();

    // Freie ID finden
    USHORT nFreeID = RID_WIN_FILE1;
    while ( pWinMenu->GetItemPos( nFreeID ) != MENU_ITEM_NOTFOUND && nFreeID < RID_WIN_FILEn )
        nFreeID++;

    pWin->SetWinId( nFreeID );
    pWinMenu->InsertItem( nFreeID, pWin->GetText() );
}

void BasicFrame::WindowRenamed( AppWin *pWin )
{
    MenuBar* pMenu = GetMenuBar();
    PopupMenu* pWinMenu = pMenu->GetPopupMenu( RID_APPWINDOW );

    pWinMenu->SetItemText( pWin->GetWinId(), pWin->GetText() );

    pStatus->LoadTaskToolBox();

    aAppFile = pWin->GetText();
    UpdateTitle();
}

void BasicFrame::FocusWindow( AppWin *pWin )
{
    pWork = pWin;
    pList->Remove( pWin );
    pList->Insert( pWin, LIST_APPEND );
    pWin->Minimize( FALSE );

    aAppFile = pWin->GetText();
    UpdateTitle();

    WinShow_Hide();
    pStatus->LoadTaskToolBox();
}

BOOL BasicFrame::Close()
{
    if( bInBreak || Basic().IsRunning() )
        if( RET_NO == QueryBox( this, ResId( IDS_RUNNING ) ).Execute() )
            return FALSE;

    StarBASIC::Stop();
    bInBreak = FALSE;
    if( CloseAll() )
    {
        aLineNum.Stop();

        // Alle übrigen Dialoge schliessen um assertions zu vermeiden!!
        while ( GetWindow( WINDOW_OVERLAP )->GetWindow( WINDOW_FIRSTOVERLAP ) )
        {
            delete GetWindow( WINDOW_OVERLAP )->GetWindow( WINDOW_FIRSTOVERLAP )->GetWindow( WINDOW_CLIENT );
        }

        Application::SetDefDialogParent( NULL );
        WorkWindow::Close();

        return TRUE;
    } else return FALSE;
}

BOOL BasicFrame::CloseAll()
{
    while ( pList->Count() )
        if ( !pList->Last()->Close() )
            return FALSE;
    return TRUE;
}

BOOL BasicFrame::CompileAll()
{
    AppWin* p;
    for( p = pList->First(); p; p = pList->Next() )
      if( p->ISA(AppBasEd) && !((AppBasEd*)p)->Compile() ) return FALSE;
    return TRUE;
}

// Menu aufsetzen

#define MENU2FILENAME Copy(3)
#define FILENAME2MENU( Nr, Name ) CUniString("~").Append( UniString::CreateFromInt32(i) ).AppendAscii(" ").Append( Name )
#define LRUNr( nNr ) CByteString("LRU").Append( ByteString::CreateFromInt32(nNr) )
void BasicFrame::AddToLRU(String const& aFile)
{
    Config aConfig(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    PopupMenu *pPopup  = GetMenuBar()->GetPopupMenu(RID_APPFILE);

    aConfig.SetGroup("LRU");
    DirEntry aFileEntry( aFile );
    USHORT i,nLastMove = 4;

    for ( i = 1 ; i<4 && nLastMove == 4 ; i++ )
    {
        if ( DirEntry( UniString( aConfig.ReadKey(LRUNr(i),""), RTL_TEXTENCODING_UTF8 ) ) == aFileEntry )
            nLastMove = i;
    }

    for ( i = nLastMove ; i>1 ; i-- )
    {
        aConfig.WriteKey(LRUNr(i), aConfig.ReadKey(LRUNr(i-1),""));
        pPopup->SetItemText(IDM_FILE_LRU1 + i-1,FILENAME2MENU( i, pPopup->GetItemText(IDM_FILE_LRU1 + i-1-1).MENU2FILENAME ));
    }
    aConfig.WriteKey(LRUNr(1), ByteString( aFile, RTL_TEXTENCODING_UTF8 ) );
    pPopup->SetItemText(IDM_FILE_LRU1,FILENAME2MENU( 1, aFile) );
}

void BasicFrame::LoadLRU()
{
    Config     aConfig(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    PopupMenu *pPopup  = GetMenuBar()->GetPopupMenu(RID_APPFILE);
    BOOL       bAddSep = TRUE;

    aConfig.SetGroup("LRU");

    for (int i=1; i<=4 && pPopup!=NULL; i++)
    {
        String aFile = UniString( aConfig.ReadKey(LRUNr(i)), RTL_TEXTENCODING_UTF8 );

        if (aFile.Len() != 0)
        {
            if (bAddSep)
            {
                pPopup->InsertSeparator();
                bAddSep = FALSE;
            }

            pPopup->InsertItem(IDM_FILE_LRU1 + i-1, FILENAME2MENU( i, aFile ));
        }
    }
}

IMPL_LINK( BasicFrame, InitMenu, Menu *, pMenu )
{
    BOOL bNormal = BOOL( !bInBreak );
    pMenu->EnableItem( RID_RUNCOMPILE, bNormal );

    BOOL bHasEdit = BOOL( /*bNormal &&*/ pWork != NULL );

//  pMenu->EnableItem( RID_FILENEW,     bNormal );  // immer möglich
//  pMenu->EnableItem( RID_FILEOPEN,    bNormal );
    pMenu->EnableItem( RID_FILECLOSE,   bHasEdit );
    pMenu->EnableItem( RID_FILESAVE,    bHasEdit );
    pMenu->EnableItem( RID_FILESAVEAS,  bHasEdit );
    pMenu->EnableItem( RID_FILEPRINT,   bHasEdit );
    pMenu->EnableItem( RID_FILESETUP,   bHasEdit );
    pMenu->EnableItem( RID_FILELOADLIB, bNormal );
    pMenu->EnableItem( RID_FILESAVELIB, bHasEdit );

    BOOL bHasErr = BOOL( bNormal && pBasic->GetErrors() != 0 );
    BOOL bNext   = bHasErr & bNormal;
    BOOL bPrev   = bHasErr & bNormal;
    if( bHasErr ) {
        short n = (short) pBasic->aErrors.GetCurPos();
        if( n == 0 ) bPrev = FALSE;
        if( n ==( pBasic->GetErrors() - 1 ) ) bNext = FALSE;
    }
    pMenu->EnableItem( RID_RUNNEXTERR, bNext );
    pMenu->EnableItem( RID_RUNPREVERR, bPrev );
    pMenu->CheckItem( RID_RUNDISAS, bDisas );
    if( pWork ) pWork->InitMenu( pMenu );

    return TRUE;
}

IMPL_LINK_INLINE_START( BasicFrame, DeInitMenu, Menu *, pMenu )
{
/*  pMenu->EnableItem( RID_RUNCOMPILE );

    pMenu->EnableItem( RID_FILECLOSE );
    pMenu->EnableItem( RID_FILESAVE );
    pMenu->EnableItem( RID_FILESAVEAS );
    pMenu->EnableItem( RID_FILEPRINT );
    pMenu->EnableItem( RID_FILESETUP );
    pMenu->EnableItem( RID_FILELOADLIB );
    pMenu->EnableItem( RID_FILESAVELIB );

    pMenu->EnableItem( RID_RUNNEXTERR );
    pMenu->EnableItem( RID_RUNPREVERR );
    if( pWork ) pWork->DeInitMenu( pMenu );
*/

    SetAutoRun( FALSE );
    String aString;
    pStatus->Message( aString );
    return 0L;
}
IMPL_LINK_INLINE_END( BasicFrame, DeInitMenu, Menu *, pMenu )

IMPL_LINK_INLINE_START( BasicFrame, HighlightMenu, Menu *, pMenu )
{
    String s = pMenu->GetHelpText( pMenu->GetCurItemId() );
    pStatus->Message( s );
    return 0L;
}
IMPL_LINK_INLINE_END( BasicFrame, HighlightMenu, Menu *, pMenu )

IMPL_LINK_INLINE_START( BasicFrame, MenuCommand, Menu *, pMenu )
{
    USHORT nId = pMenu->GetCurItemId();
    BOOL bChecked = pMenu->IsItemChecked( nId );
    return Command( nId, bChecked );
}
IMPL_LINK_INLINE_END( BasicFrame, MenuCommand, Menu *, pMenu )

IMPL_LINK_INLINE_START( BasicFrame, Accel, Accelerator*, pAcc )
{
    SetAutoRun( FALSE );
    return Command( pAcc->GetCurItemId() );
}
IMPL_LINK_INLINE_END( BasicFrame, Accel, Accelerator*, pAcc )

IMPL_LINK_INLINE_START( BasicFrame, ShowLineNr, AutoTimer *, pTimer )
{
    String aPos;
    if ( pWork )
    {
        aPos = String::CreateFromInt32(pWork->GetLineNr());
    }
    pStatus->Pos( aPos );
    return 0L;
}
IMPL_LINK_INLINE_END( BasicFrame, ShowLineNr, AutoTimer *, pTimer )


MsgEdit* BasicFrame::GetMsgTree( String aLogFileName )
{
    if ( FindErrorWin( aLogFileName ) )
    {
        return FindErrorWin( aLogFileName )->GetMsgTree();
    }
    else
    {   // create new Window on the fly
        AppError *pNewWindow = new AppError( this, aLogFileName );
        pNewWindow->Show();
        pNewWindow->GrabFocus();
        return pNewWindow->GetMsgTree();
    }
}

IMPL_LINK( BasicFrame, Log, TTLogMsg *, pLogMsg )
{
    GetMsgTree( pLogMsg->aLogFileName )->AddAnyMsg( pLogMsg );
    return 0L;
}

IMPL_LINK( BasicFrame, WinInfo, WinInfoRec*, pWinInfo )
{
    if ( !pDisplayHidDlg )
        pDisplayHidDlg = new DisplayHidDlg( this );
    if ( pDisplayHidDlg )
    {
        pDisplayHidDlg->AddData( pWinInfo );
        pDisplayHidDlg->Show();
    }
    return 0;
}

BOOL BasicFrame::LoadFile( String aFilename )
{
    BOOL bIsResult = DirEntry( aFilename ).GetExtension().CompareIgnoreCaseToAscii("RES") == COMPARE_EQUAL;
    BOOL bIsBasic = DirEntry( aFilename ).GetExtension().CompareIgnoreCaseToAscii("BAS") == COMPARE_EQUAL;
    bIsBasic |= DirEntry( aFilename ).GetExtension().CompareIgnoreCaseToAscii("INC") == COMPARE_EQUAL;

    AppWin* p;
    if ( bIsResult )
    {
        p = new AppError( this, aFilename );
    }
    else if ( bIsBasic )
    {
        p = new AppBasEd( this, NULL );
        p->Load( aFilename );
    }
    else
    {
        p = new AppEdit( this );
        p->Load( aFilename );
    }
    p->Show();
    p->GrabFocus();
    return TRUE;
}

// Kommando ausfuehren

long BasicFrame::Command( short nID, BOOL bChecked )
{
    BasicError* pErr;

    switch( nID ) {
        case RID_FILENEW: {
            AppBasEd* p = new AppBasEd( this, NULL );
            p->Show();
            p->GrabFocus();
    //      InitMenu(GetMenuBar()->GetPopupMenu( RID_APPRUN ));
            } break;
        case RID_FILEOPEN:
            {
                String s;
                if( QueryFileName( s, FT_BASIC_SOURCE | FT_RESULT_FILE, FALSE ) ) {
                    AddToLRU( s );
                    LoadFile( s );
//                  InitMenu(GetMenuBar()->GetPopupMenu( RID_APPRUN ));
                }
            } break;
        case RID_FILELOADLIB:
            LoadLibrary();
            break;
        case RID_FILESAVELIB:
            SaveLibrary();
            break;
        case RID_FILECLOSE:
            if( pWork && pWork->Close() ){};
//          InitMenu(GetMenuBar()->GetPopupMenu( RID_APPRUN ));
            break;
        case RID_FILEPRINT:
#ifndef UNX
            if( pWork )
                pPrn->Print( pWork->GetText(), pWork->pDataEdit->GetText(), this );
#else
            InfoBox( this, ResId( IDS_NOPRINTERERROR ) ).Execute();
#endif
            break;
        case RID_FILESETUP:
#ifndef UNX
            pPrn->Setup();
#else
            InfoBox( this, ResId( IDS_NOPRINTERERROR ) ).Execute();
#endif
            break;
        case RID_QUIT:
            if( Close() ) aBasicApp.Quit();
            break;
        case IDM_FILE_LRU1:
        case IDM_FILE_LRU2:
        case IDM_FILE_LRU3:
        case IDM_FILE_LRU4:
            {
                String s = GetMenuBar()->GetPopupMenu(RID_APPFILE)->GetItemText(nID).MENU2FILENAME;

                AddToLRU( s );
                LoadFile( s );
//              InitMenu(GetMenuBar()->GetPopupMenu( RID_APPRUN ));
            }
            break;


        case RID_RUNSTART:
            nFlags = SbDEBUG_BREAK;
            goto start;
        case RID_RUNSTEPOVER:
            nFlags = SbDEBUG_STEPINTO | SbDEBUG_STEPOVER;
            goto start;
        case RID_RUNSTEPINTO:
            nFlags = SbDEBUG_STEPINTO;
            goto start;
        case RID_RUNTOCURSOR:
            if ( pWork && pWork->ISA(AppBasEd) && ((AppBasEd*)pWork)->GetModule()->SetBP(pWork->GetLineNr()) )
            {
                SbModule *pModule = ((AppBasEd*)pWork)->GetModule();
#ifdef DEBUG
                USHORT x;
                x = pWork->GetLineNr();
                x = ((AppBasEd*)pWork)->GetModule()->GetBPCount();
                if ( !x )
                    x = pModule->SetBP(pWork->GetLineNr());
                x = pModule->GetBPCount();
#endif

                for ( USHORT nMethod = 0; nMethod < pModule->GetMethods()->Count(); nMethod++ )
                {
                    SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Get( nMethod );
                    DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
                    pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
                }
            }
            nFlags = SbDEBUG_BREAK;
            goto start;
        start: {
//          InitMenu(GetMenuBar()->GetPopupMenu( RID_APPRUN ));
            if ( !Basic().IsRunning() || bInBreak )
            {
                AppBasEd* p = NULL;
                if( pWork && pWork->ISA(AppBasEd) )
                {
                    p = ((AppBasEd*)pWork);
                    p->ToTop();
                }
                else
                    SetAutoRun( FALSE );    // Wenn kein Programm geladen wurde, dann auch nicht beenden

                if( bInBreak )
                    // Nur das Flag zuruecksetzen
                    bInBreak = FALSE;
                else
                {
                    if( !SaveAll() ) break;
                    if( !CompileAll() ) break;
                    String aString;
                    pStatus->Message( aString );
                    if( p )
                    {
                        BasicDLL::SetDebugMode( TRUE );
                        Basic().ClearGlobalVars();
                        p->Run();
                        BasicDLL::SetDebugMode( FALSE );
                        // Falls waehrend Interactive=FALSE abgebrochen
//                      BasicDLL::EnableBreak( TRUE );
                    }
                }}
            }
//          InitMenu(GetMenuBar()->GetPopupMenu( RID_APPRUN )); // nach run
            break;
        case RID_RUNCOMPILE:
            if( pWork && pWork->ISA(AppBasEd) && SaveAll() )
            {
                ((AppBasEd*)pWork)->Compile();
                pWork->ToTop();
                pWork->GrabFocus();
            }
            break;
        case RID_RUNDISAS:
            bDisas = BOOL( !bChecked );
            break;
        case RID_RUNBREAK:
            if ( Basic().IsRunning() && !bInBreak )
            {
//              pINST->CalcBreakCallLevel(SbDEBUG_STEPINTO);
                pINST->nBreakCallLvl = pINST->nCallLvl;
            }
            break;
        case RID_RUNSTOP:
            Basic().Stop();
            bInBreak = FALSE;
            break;
        case RID_RUNNEXTERR:
            pErr = pBasic->aErrors.Next();
            if( pErr ) pErr->Show();
            break;
        case RID_RUNPREVERR:
            pErr = pBasic->aErrors.Prev();
            if( pErr ) pErr->Show();
            break;

        case RID_OPTIONS:
            {
                new OptionsDialog( this, ResId(IDD_OPTIONS_DLG) );
            }
            break;
        case RID_DECLARE_HELPER:
            InfoBox( this, ResId( IDS_NOT_YET_IMPLEMENTED ) ).Execute();
            break;

        case RID_WINTILE:
            {
                WindowArrange aArange;
                for ( ULONG i = 0 ; i < pList->Count() ; i++ )
                {
                    aArange.AddWindow( pList->GetObject( i ) );
                    pList->GetObject( i )->Restore();
                }


                long nTitleHeight;
                {
                    long nDummy1, nDummy2, nDummy3;
                    GetBorder( nDummy1, nTitleHeight, nDummy2, nDummy3 );
                }

                Size aSize = GetOutputSizePixel();
                aSize.Height() -= nTitleHeight;
                Rectangle aRect( Point( 0, nTitleHeight ), aSize );

                aArange.Arrange( WINDOWARRANGE_TILE, aRect );

            }
            break;
        case RID_WINTILEHORZ:
            {
                WindowArrange aArange;
                for ( ULONG i = 0 ; i < pList->Count() ; i++ )
                {
                    aArange.AddWindow( pList->GetObject( i ) );
                    pList->GetObject( i )->Restore();
                }


                long nTitleHeight;
                {
                    long nDummy1, nDummy2, nDummy3;
                    GetBorder( nDummy1, nTitleHeight, nDummy2, nDummy3 );
                }

                Size aSize = GetOutputSizePixel();
                aSize.Height() -= nTitleHeight;
                Rectangle aRect( Point( 0, nTitleHeight ), aSize );

                aArange.Arrange( WINDOWARRANGE_HORZ, aRect );

            }
            break;
        case RID_WINTILEVERT:
//#define WINDOWARRANGE_TILE        1
//#define WINDOWARRANGE_HORZ        2
//#define WINDOWARRANGE_VERT        3
//#define WINDOWARRANGE_CASCADE 4
            {
                WindowArrange aArange;
                for ( ULONG i = 0 ; i < pList->Count() ; i++ )
                {
                    aArange.AddWindow( pList->GetObject( i ) );
                    pList->GetObject( i )->Restore();
                }


                long nTitleHeight;
                {
                    long nDummy1, nDummy2, nDummy3;
                    GetBorder( nDummy1, nTitleHeight, nDummy2, nDummy3 );
                }

                Size aSize = GetOutputSizePixel();
                aSize.Height() -= nTitleHeight;
                Rectangle aRect( Point( 0, nTitleHeight ), aSize );

                aArange.Arrange( WINDOWARRANGE_VERT, aRect );

            }
            break;
        case RID_WINCASCADE:
            {
                for ( ULONG i = 0 ; i < pList->Count() ; i++ )
                {
                    pList->GetObject( i )->Cascade( i );
                }
            }
            break;

/*      case RID_HELPTOPIC:
            if( pWork ) pWork->Help();
            break;
        case RID_HELPKEYS:
            aBasicApp.pHelp->Start( CUniString( "Keyboard" ) );
            break;
        case RID_HELPINDEX:
            aBasicApp.pHelp->Start( HELP_INDEX );
            break;
        case RID_HELPINTRO:
            aBasicApp.pHelp->Start( HELP_HELPONHELP );
            break;
*/      case RID_HELPABOUT:
            {
                ResId aResId( IDD_ABOUT_DIALOG );
                if ( Basic().pTestObject )      // Wir sind also TestTool
                    aResId = ResId( IDD_TT_ABOUT_DIALOG );
                else
                    aResId = ResId( IDD_ABOUT_DIALOG );
                AboutDialog( this, aResId ).Execute();
            }
            break;
        case RID_POPUPEDITVAR:
            {
                new VarEditDialog( this, pEditVar );
            }
            break;
        default:
            if ( nID >= RID_WIN_FILE1 && nID  <= RID_WIN_FILEn )
            {
                MenuBar* pMenu = GetMenuBar();
                PopupMenu* pWinMenu = pMenu->GetPopupMenu( RID_APPWINDOW );
                String aName = pWinMenu->GetItemText( nID );
                AppWin* pWin = FindWin( aName );
                if ( pWin )
                    pWin->ToTop();
            }
            else
            {
//              InitMenu(GetMenuBar()->GetPopupMenu( RID_APPEDIT ));    // So daß Delete richtig ist
                if( pWork )
                    pWork->Command( CommandEvent( Point(), nID ) );
//              InitMenu(GetMenuBar()->GetPopupMenu( RID_APPEDIT ));    // So daß Delete richtig ist
            }
    }
    return TRUE;
}

BOOL BasicFrame::SaveAll()
{
    AppWin* p, *q = pWork;
    for( p = pList->First(); p; p = pList->Next() )
    {
        USHORT nRes = p->QuerySave( QUERY_DISK_CHANGED );
        if( (( nRes == SAVE_RES_ERROR ) && QueryBox(this,ResId(IDS_ASKSAVEERROR)).Execute() == RET_NO )
            || ( nRes == SAVE_RES_CANCEL ) )
            return FALSE;
    }
    if ( q )
        q->ToTop();
    return TRUE;
}

IMPL_LINK( BasicFrame, ModuleWinExists, String*, pFilename )
{
    return FindModuleWin( *pFilename ) != NULL;
}

AppBasEd* BasicFrame::FindModuleWin( const String& rName )
{
    AppWin* p;
    for( p = pList->First(); p; p = pList->Next() )
    {
        if( p->ISA(AppBasEd) && ((AppBasEd*)p)->GetModName() == rName )
            return ((AppBasEd*)p);
    }
    return NULL;
}

AppError* BasicFrame::FindErrorWin( const String& rName )
{
    AppWin* p;
    for( p = pList->First(); p; p = pList->Next() )
    {
        if( p->ISA(AppError) && ((AppError*)p)->GetText() == rName )
            return ((AppError*)p);
    }
    return NULL;
}

AppWin* BasicFrame::FindWin( const String& rName )
{
    AppWin* p;
    for( p = pList->First(); p; p = pList->Next() )
    {
        if( p->GetText() == rName )
            return p;
    }
    return NULL;
}

AppWin* BasicFrame::FindWin( USHORT nWinId )
{
    AppWin* p;
    for( p = pList->First(); p; p = pList->Next() )
    {
        if( p->GetWinId() == nWinId )
            return p;
    }
    return NULL;
}

class NewFileDialog : public FileDialog
{
private:
    String aLastPath;
public:
    ByteString aPathName;
    NewFileDialog( Window* pParent, WinBits nWinStyle ):FileDialog( pParent, nWinStyle ){};
    virtual short   Execute();
    virtual void    FilterSelect();
};

void NewFileDialog::FilterSelect()
{
    String aTemp = GetPath();
    if ( aLastPath.Len() == 0 )
        aLastPath = DirEntry( GetPath() ).GetPath().GetFull();
    if ( aLastPath.CompareIgnoreCaseToAscii( DirEntry( GetPath() ).GetPath().GetFull() ) != COMPARE_EQUAL )
        return;     // Der Benutzer entscheidet sich nachdem er den Pfad geändert hat.

    String aCurFilter = GetCurFilter();
    USHORT nFilterNr = 0;
    while ( nFilterNr < GetFilterCount() && aCurFilter != GetFilterName( nFilterNr ) )
    {
        nFilterNr++;
    }
    aPathName = ByteString( GetFilterType( nFilterNr ), RTL_TEXTENCODING_UTF8 );

    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup( "Path" );
    aLastPath = UniString( aConf.ReadKey( aPathName, aConf.ReadKey( "Basisverzeichnis" ) ), RTL_TEXTENCODING_UTF8 );
    SetPath( aLastPath );
//  if ( IsInExecute() )
//      SetPath( "" );
}

short NewFileDialog::Execute()
{
    BOOL bRet = FileDialog::Execute();
    if ( bRet )
    {
        Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
        aConf.SetGroup( "Path" );
        aConf.WriteKey( aPathName, ByteString( DirEntry( GetPath() ).GetPath().GetFull(), RTL_TEXTENCODING_UTF8 ) );
    }
    return bRet;
}

BOOL BasicFrame::QueryFileName
                (String& rName, FileType nFileType, BOOL bSave )
{
    NewFileDialog aDlg( this, bSave ? WinBits( WB_SAVEAS ) :
                                WinBits( WB_OPEN ) );
    aDlg.SetText( String( ResId( bSave ? IDS_SAVEDLG : IDS_LOADDLG ) ) );

    if ( nFileType & FT_RESULT_FILE )
    {
        aDlg.SetDefaultExt( String( ResId( IDS_RESFILE ) ) );
        aDlg.AddFilter( String( ResId( IDS_RESFILTER ) ), String( ResId( IDS_RESFILE ) ) );
        aDlg.AddFilter( String( ResId( IDS_TXTFILTER ) ), String( ResId( IDS_TXTFILE ) ) );
        aDlg.SetCurFilter( ResId( IDS_RESFILTER ) );
    }

    if ( nFileType & FT_BASIC_SOURCE )
    {
        aDlg.SetDefaultExt( String( ResId( IDS_NONAMEFILE ) ) );
        aDlg.AddFilter( String( ResId( IDS_BASFILTER ) ), String( ResId( IDS_NONAMEFILE ) ) );
        aDlg.AddFilter( String( ResId( IDS_INCFILTER ) ), String( ResId( IDS_INCFILE ) ) );
        aDlg.SetCurFilter( ResId( IDS_BASFILTER ) );
    }

    if ( nFileType & FT_BASIC_LIBRARY )
    {
        aDlg.SetDefaultExt( String( ResId( IDS_LIBFILE ) ) );
        aDlg.AddFilter( String( ResId( IDS_LIBFILTER ) ), String( ResId( IDS_LIBFILE ) ) );
        aDlg.SetCurFilter( ResId( IDS_LIBFILTER ) );
    }


    aDlg.FilterSelect();    // Setzt den Pfad vom letzten mal.
//  if ( bSave )
    if ( rName.Len() > 0 )
        aDlg.SetPath( rName );

    if( aDlg.Execute() )
    {
        rName = aDlg.GetPath();
/*      rExtension = aDlg.GetCurrentFilter();
        var i:integer;
        for ( i = 0 ; i < aDlg.GetFilterCount() ; i++ )
            if ( rExtension == aDlg.GetFilterName( i ) )
                rExtension = aDlg.GetFilterType( i );
*/
        return TRUE;
    } else return FALSE;
}

USHORT BasicFrame::BreakHandler()
{
    bInBreak = TRUE;
//  InitMenu(GetMenuBar()->GetPopupMenu( RID_APPRUN ));
//  MenuBar aBar( ResId( RID_APPMENUBAR ) );
//  aBar.EnableItem( RID_APPEDIT, FALSE );
    SetAppMode( String( ResId ( IDS_APPMODE_BREAK ) ) );
    while( bInBreak )
#if SUPD >= 357
        GetpApp()->Yield();
#else
        GetpApp()->Reschedule();
#endif
    SetAppMode( String( ResId ( IDS_APPMODE_RUN ) ) );
//  aBar.EnableItem( RID_APPEDIT, TRUE );
//  InitMenu(GetMenuBar()->GetPopupMenu( RID_APPRUN ));
    return nFlags;
}

void BasicFrame::LoadLibrary()
{
    String s;
    if( QueryFileName( s, FT_BASIC_LIBRARY, FALSE ) )
    {
        CloseAll();
        SvFileStream aStrm( s, STREAM_STD_READ );
        MyBasic* pNew = (MyBasic*) SbxBase::Load( aStrm );
        if( pNew && pNew->ISA( MyBasic ) )
        {
            pBasic = pNew;
            // Alle Inhalte - sofern vorhanden - anzeigen
            SbxArray* pMods = pBasic->GetModules();
            for( USHORT i = 0; i < pMods->Count(); i++ )
            {
                SbModule* pMod = (SbModule*) pMods->Get( i );
                AppWin* p = new AppBasEd( this, pMod );
                p->Show();
            }
        }
        else
        {
            delete pNew;
            ErrorBox( this, ResId( IDS_READERROR ) ).Execute();
        }
    }
}

void BasicFrame::SaveLibrary()
{
    String s;
    if( QueryFileName( s, FT_BASIC_LIBRARY, TRUE ) )
    {
        SvFileStream aStrm( s, STREAM_STD_WRITE );
        if( !Basic().Store( aStrm ) )
            ErrorBox( this, ResId( IDS_WRITEERROR ) ).Execute();
    }
}

String BasicFrame::GenRealString( const String &aResString )
{
    xub_StrLen nStart,nGleich,nEnd,nStartPos = 0;
    String aType,aValue,aResult(aResString);
    String aString;
    xub_StrLen nInsertPos;
    BOOL bFound;

    while ( (nStart = aResult.Search(StartKenn,nStartPos)) != STRING_NOTFOUND &&
            (nGleich = aResult.SearchAscii("=",nStart+StartKenn.Len())) != STRING_NOTFOUND &&
            (nEnd = aResult.Search(EndKenn,nGleich+1)) != STRING_NOTFOUND)
    {
        aType = aResult.Copy(nStart,nGleich-nStart);
        aValue = aResult.Copy(nGleich+1,nEnd-nGleich-1);
        bFound = FALSE;
        if ( aType.CompareTo(ResKenn) == COMPARE_EQUAL )
        {
//          if ( Resource::GetResManager()->IsAvailable( ResId( aValue ) ) )
                aString = String( ResId( aValue.ToInt32() ) );
//          else
            {
//              DBG_ERROR( "Ressource konnte nicht geladen werden" );
//              return aResString;
            }
            nInsertPos = nStart;
            nStartPos = nStart;
            aResult.Erase( nStart, nEnd-nStart+1 );
        }
        else if ( aType.Search(BaseArgKenn) == 0 )      // Fängt mit BaseArgKenn an
        {
            USHORT nArgNr = aType.Copy( BaseArgKenn.Len() ).ToInt32();
            DBG_ASSERT( aString.Search( CUniString("($Arg").Append( String::CreateFromInt32(nArgNr) ).AppendAscii(")") ) != STRING_NOTFOUND, "Extra Argument given in String");
            aString.SearchAndReplace( CUniString("($Arg").Append( String::CreateFromInt32(nArgNr) ).AppendAscii(")"), aValue );
            nStartPos = nStart;
            aResult.Erase( nStart, nEnd-nStart+1 );
        }
        else
        {
            DBG_ERROR( CByteString("Unknown replacement in String: ").Append( ByteString( aResult.Copy(nStart,nEnd-nStart), RTL_TEXTENCODING_UTF8 ) ).GetBuffer() );
            nStartPos += StartKenn.Len();
        }
    }
    DBG_ASSERT( aString.SearchAscii( "($Arg" ) == STRING_NOTFOUND, "Argument missing in String");
    aResult.Insert( aString, nInsertPos );
    return aResult;
}


