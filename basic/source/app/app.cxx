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
#include "precompiled_basic.hxx"

#include <vcl/msgbox.hxx>
#include <tools/fsys.hxx>
#include <svtools/filedlg.hxx>
#include <tools/config.hxx>

#include <vcl/font.hxx>

#include <basic/ttstrhlp.hxx>
#include <basic/sbx.hxx>
#include <svtools/filedlg.hxx>

#include <osl/module.h>

#include "basic.hrc"
#include "app.hxx"
#include "printer.hxx"
#include "status.hxx"
#include "appedit.hxx"
#include "appbased.hxx"
#include "apperror.hxx"
#include <basic/mybasic.hxx>
#include "ttbasic.hxx"
#include "dialogs.hxx"
#include <basic/basrdll.hxx>
#include "basrid.hxx"

#include "runtime.hxx"
#include "sbintern.hxx"

#ifdef _USE_UNO
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/configurationkeys.hxx>
#include <comphelper/regpathhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>

#include <ucbhelper/content.hxx>
#include <unotools/syslocale.hxx>

using namespace comphelper;
using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;

using ::rtl::OUString;
using ::rtl::OUStringToOString;

#endif /* _USE_UNO */

IMPL_GEN_RES_STR;

#ifdef DBG_UTIL
// filter Messages generated due to missing configuration  Bug:#83887#
void TestToolDebugMessageFilter( const sal_Char *pString, sal_Bool bIsOsl )
{
    static sal_Bool static_bInsideFilter = sal_False;

    // Ignore messages during filtering to avoid endless recursions
    if ( static_bInsideFilter )
        return;

    static_bInsideFilter = sal_True;

    ByteString aMessage( pString );

    sal_Bool bIgnore = sal_False;

    if ( bIsOsl )
    {
        // OSL
        if ( aMessage.Search( CByteString("Cannot open Configuration: Connector: unknown delegatee com.sun.star.connection.Connector.portal") ) != STRING_NOTFOUND )
            bIgnore = sal_True;
    }
    else
    {
        // DBG
#if ! (OSL_DEBUG_LEVEL > 1)
        if ( aMessage.Search( CByteString("SelectAppIconPixmap") ) != STRING_NOTFOUND )
            bIgnore = sal_True;
#endif
        if ( aMessage.Search( CByteString("PropertySetRegistry::") ) != STRING_NOTFOUND )
            bIgnore = sal_True;
        if ( aMessage.Search( CByteString("property value missing") ) != STRING_NOTFOUND )
            bIgnore = sal_True;
        if ( aMessage.Search( CByteString("getDateFormatsImpl") ) != STRING_NOTFOUND
            && aMessage.Search( CByteString("no date formats") ) != STRING_NOTFOUND )
            bIgnore = sal_True;
        if ( aMessage.Search( CByteString("ucb::configureUcb(): Bad arguments") ) != STRING_NOTFOUND )
            bIgnore = sal_True;
        if ( aMessage.Search( CByteString("CreateInstance with arguments exception") ) != STRING_NOTFOUND )
            bIgnore = sal_True;
        if ( aMessage.Search( CByteString("AcquireTree failed") ) != STRING_NOTFOUND )
            bIgnore = sal_True;
    }


    if ( bIgnore )
    {
        static_bInsideFilter = sal_False;
        return;
    }

    if ( bIsOsl )
    {
        // due to issue #i36895 only print on console
        // unfortunately the osl assertions deadlock by design :-( on recursive calls of assertions
        printf("%s\n", pString );
    }
    else
    {
        try
        {
            aBasicApp.DbgPrintMsgBox( pString );
        }
        catch ( ... )

        {
            printf("DbgPrintMsgBox failed: %s\n", pString );
        }
    }
    static_bInsideFilter = sal_False;
}

void SAL_CALL DBG_TestToolDebugMessageFilter( const sal_Char *pString )
{
        TestToolDebugMessageFilter( pString, sal_False );
}

extern "C" void SAL_CALL osl_TestToolDebugMessageFilter( const sal_Char *pString )
{
    if ( !getenv( "DISABLE_SAL_DBGBOX" ) )
        TestToolDebugMessageFilter( pString, sal_True );
}

#endif

// #94145# Due to a tab in TT_SIGNATURE_FOR_UNICODE_TEXTFILES which is changed to blanks by some editors
// this routine became necessary
sal_Bool IsTTSignatureForUnicodeTextfile( String aLine )
{
    aLine.SearchAndReplace( '\t', ' ' );
    String ThreeBlanks = CUniString("   ");
    String TwoBlanks = CUniString("  ");
    while ( aLine.SearchAndReplace( ThreeBlanks, TwoBlanks ) != STRING_NOTFOUND )
    {}
    return aLine.EqualsAscii( TT_SIGNATURE_FOR_UNICODE_TEXTFILES );
}

BasicApp aBasicApp; // Application instance

uno::Reference< XContentProviderManager > InitializeUCB( void )
{
    uno::Reference< XMultiServiceFactory > xSMgr;
    try
    {
        xSMgr = uno::Reference< XMultiServiceFactory >(
            defaultBootstrap_InitialComponentContext()->getServiceManager(),
            UNO_QUERY_THROW);
    }
    catch( com::sun::star::uno::Exception & exc )
    {
        fprintf( stderr, "Couldn't bootstrap uno servicemanager for reason : %s\n" ,
                 OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        InfoBox( NULL, String( exc.Message ) ).Execute();
        throw ;
    }


    // set global factory
    setProcessServiceFactory( xSMgr );

//  Create unconfigured Ucb:
    Sequence< Any > aArgs;
    ::ucbhelper::ContentBroker::initialize( xSMgr, aArgs );
    uno::Reference< XContentProviderManager > xUcb =
        ::ucbhelper::ContentBroker::get()->getContentProviderManagerInterface();

    uno::Reference< XContentProvider > xFileProvider
        ( xSMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.FileContentProvider" )) ), UNO_QUERY );
    xUcb->registerContentProvider( xFileProvider, OUString(RTL_CONSTASCII_USTRINGPARAM( "file" )), sal_True );

    return xUcb;
}

static void ReplaceStringHookProc( UniString& rStr )
{
    static String aTestToolName( RTL_CONSTASCII_USTRINGPARAM( "VCLTestTool" ) ); // HACK, should be read from ressources

    if ( rStr.SearchAscii( "%PRODUCT" ) != STRING_NOTFOUND )
    {
        rStr.SearchAndReplaceAllAscii( "%PRODUCTNAME", aTestToolName );
    }
}

int BasicApp::Main( )
{
#ifdef DBG_UTIL
//  Install filter for OSLAsserts
    DbgPrintMsgBox = DbgGetPrintMsgBox();
    DbgSetPrintTestTool( DBG_TestToolDebugMessageFilter );
    DBG_INSTOUTERROR( DBG_OUT_TESTTOOL );

    if ( osl_setDebugMessageFunc( osl_TestToolDebugMessageFilter ) )
        OSL_FAIL("osl_setDebugMessageFunc returns non NULL pointer");
#endif

    ResMgr::SetReadStringHook( ReplaceStringHookProc );

    try
    {
#ifdef _USE_UNO
    uno::Reference< XContentProviderManager > xUcb = InitializeUCB();
#endif

    {
        DirEntry aIniPath( Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ) );
        if ( !aIniPath.Exists() )
        {   // look for it besides the executable
            DirEntry aAppFileName( GetAppFileName() );
            String aAppDir ( aAppFileName.GetPath().GetFull() );

            // Do not use Config::GetConfigName here because is uses a hidden
            // file for UNIX

            DirEntry aDefIniPath( aAppDir );
            ByteString aFileName;
#ifdef UNX
            aFileName = "testtoolrc";
#else
            aFileName = "testtool.ini";
#endif
            aDefIniPath += DirEntry( aFileName );

            if ( aDefIniPath.Exists() )
            {
                aDefIniPath.CopyTo( aIniPath, FSYS_ACTION_COPYFILE );
                FileStat::SetReadOnlyFlag( aIniPath, sal_False );
            }
        }
    }

    {
        LanguageType aRequestedLanguage;
        Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));

        // 1033 = LANGUAGE_ENGLISH_US
        // 1031 = LANGUAGE_GERMAN
        aConf.SetGroup("Misc");
        ByteString aLang = aConf.ReadKey( "Language", ByteString::CreateFromInt32( LANGUAGE_SYSTEM ) );
        aRequestedLanguage = LanguageType( aLang.ToInt32() );

        AllSettings aSettings = GetSettings();
        aSettings.SetUILanguage( aRequestedLanguage );
        aSettings.SetLanguage( aRequestedLanguage );
        SetSettings( aSettings );
    }

    BasicDLL aBasicDLL;
    nWait = 0;

    // Acceleratoren
    Accelerator aAccel( SttResId( MAIN_ACCEL ) );
    InsertAccel( &aAccel );
    pMainAccel = &aAccel;

    // Frame Window:
    pFrame = new BasicFrame;
    aAccel.SetSelectHdl( LINK( pFrame, BasicFrame, Accel ) );

    pFrame->Show();

    SetSystemWindowMode( SYSTEMWINDOW_MODE_NOAUTOMODE );
    SetSystemWindowMode( SYSTEMWINDOW_MODE_DIALOG );

    // Instantiate a SvtSysLocale to avoid permant instatiation
    // and deletion of SvtSysLocale_Impl in SvtSysLocale Ctor/Dtor
    // because in the testtool szenario Basic is the only instance
    // instatiating SvtSysLocale (#107417).
    SvtSysLocale aSysLocale;

    PostUserEvent( LINK( this, BasicApp, LateInit ) );
    Execute();

    delete pFrame;

    RemoveAccel( pMainAccel );

    }
    catch( class Exception & rEx)
    {
        printf( "Exception not caught: %s\n", ByteString( String(rEx.Message), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
        String aMsg( String::CreateFromAscii( "Exception not caught: " ) );
        aMsg.Append( String( rEx.Message ) );
        InfoBox( NULL, aMsg ).Execute();
        throw;
    }
    catch( ... )
    {
        printf( "unknown Exception not caught\n" );
        InfoBox( NULL, String::CreateFromAscii( "unknown Exception not caught" ) ).Execute();
        throw;
    }
    return EXIT_SUCCESS;
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
    (void) pDummy; /* avoid warning about unused parameter */
    sal_uInt16 i;
    for ( i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
    {
        if ( Application::GetCommandLineParam( i ).Copy(0,4).CompareIgnoreCaseToAscii("-run") == COMPARE_EQUAL
#ifndef UNX
            || Application::GetCommandLineParam( i ).Copy(0,4).CompareIgnoreCaseToAscii("/run") == COMPARE_EQUAL
#endif
            )
            pFrame->SetAutoRun( sal_True );
        else if ( Application::GetCommandLineParam( i ).Copy(0,7).CompareIgnoreCaseToAscii("-result") == COMPARE_EQUAL
#ifndef UNX
            || Application::GetCommandLineParam( i ).Copy(0,7).CompareIgnoreCaseToAscii("/result") == COMPARE_EQUAL
#endif
            )
        {
            if ( (i+1) < Application::GetCommandLineParamCount() )
            {
                if ( ByteString( Application::GetCommandLineParam( i+1 ), osl_getThreadTextEncoding() ).IsNumericAscii() )
                {
                    MsgEdit::SetMaxLogLen( sal::static_int_cast< sal_uInt16 >( Application::GetCommandLineParam( i+1 ).ToInt32() ) );
                }
                i++;
            }
        }
    }

    // now load the files after the switches have been set. Espechially -run is of interest sunce it changes the behavior
    for ( i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
    {
        if ( Application::GetCommandLineParam( i ).Copy(0,1).CompareToAscii("-") != COMPARE_EQUAL
#ifndef UNX
          && Application::GetCommandLineParam( i ).Copy(0,1).CompareToAscii("/") != COMPARE_EQUAL
#endif
          )
        {
            pFrame->LoadFile( Application::GetCommandLineParam( i ) );
        }
        else if ( Application::GetCommandLineParam( i ).Copy(0,7).CompareIgnoreCaseToAscii("-result") == COMPARE_EQUAL
#ifndef UNX
            || Application::GetCommandLineParam( i ).Copy(0,7).CompareIgnoreCaseToAscii("/result") == COMPARE_EQUAL
#endif
            )
        {   // Increment count to skip the parameter. This works even if it is not given
            i++;
        }
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
    : FloatingWindow( pParent, SttResId(LOAD_CONF) ),
    aStatus( this, SttResId( WORK ) ),
    aAdditionalInfo( this, SttResId( FILENAME ) )
{
    FreeResource();
    aAusblend.SetTimeoutHdl( LINK(this, FloatingExecutionStatus, HideNow ) );
    aAusblend.SetTimeout(5000);             // in ms
    aAusblend.Start();
}

void FloatingExecutionStatus::SetStatus( String aW )
{
    Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
    ToTop( TOTOP_NOGRABFOCUS );
    aAusblend.Start();
    aStatus.SetText( aW );
}

void FloatingExecutionStatus::SetAdditionalInfo( String aF )
{
    Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
    ToTop( TOTOP_NOGRABFOCUS );
    aAusblend.Start();
    aAdditionalInfo.SetText( aF );
}

IMPL_LINK(FloatingExecutionStatus, HideNow, FloatingExecutionStatus*, pFLC )
{
    (void) pFLC; /* avoid warning about unused parameter */
    Hide();
    return 0;
}

//////////////////////////////////////////////////////////////////////////

TYPEINIT1(TTExecutionStatusHint, SfxSimpleHint);

BasicFrame::BasicFrame() : WorkWindow( NULL,
    WinBits( WB_APP | WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) )
, bIsAutoRun( sal_False )
, pDisplayHidDlg( NULL )
, pEditVar ( 0 )
, bAutoReload( sal_False )
, bAutoSave( sal_True )
, pBasic( NULL )
, pExecutionStatus( NULL )
, pStatus( NULL )
, pList( NULL )
, pWork( NULL )
, pPrn( NULL )
{

    Application::SetDefDialogParent( this );
    AlwaysEnableInput( sal_True );
    pBasic  = TTBasic::CreateMyBasic();     // depending on what was linked to the executable
    bInBreak = sal_False;
    bDisas = sal_False;
    nFlags  = 0;
//  Icon aAppIcon;

    if ( pBasic->pTestObject )  // Are we the testtool?
    {
        aAppName = String( SttResId( IDS_APPNAME2 ) );
    }
    else
    {
        aAppName = String( SttResId( IDS_APPNAME ) );
    }

    // Menu:
    MenuBar *pBar = new MenuBar( SttResId( RID_APPMENUBAR ) );
    SetMenuBar( pBar );

    pBar->SetHighlightHdl( LINK( this, BasicFrame, HighlightMenu ) );


    // Menu Handler:
    PopupMenu* pFileMenu = pBar->GetPopupMenu( RID_APPFILE );
    pFileMenu->SetSelectHdl( LINK( this, BasicFrame, MenuCommand ) );
    pFileMenu->SetHighlightHdl( LINK( this, BasicFrame, HighlightMenu ) );
    pFileMenu->SetActivateHdl( LINK( this, BasicFrame, InitMenu ) );
    pFileMenu->SetDeactivateHdl( LINK( this, BasicFrame, DeInitMenu ) );
    if (Basic().pTestObject )       // Are we TestTool?
    {
        pFileMenu->RemoveItem( pFileMenu->GetItemPos( RID_FILELOADLIB ) -1 );   // Separator before
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
    if (Basic().pTestObject )       // Are we TestTool?
    {
        pRunMenu->RemoveItem( pRunMenu->GetItemPos( RID_RUNDISAS ) );
        pRunMenu->RemoveItem( pRunMenu->GetItemPos( RID_RUNCOMPILE ) );
    }

    PopupMenu *pExtras;
    if (Basic().pTestObject )       // Are we TestTool?
    {
        pExtras = new PopupMenu( SttResId( RID_TT_EXTRAS ) );
        pBar->InsertItem( RID_TT_EXTRAS, String( SttResId( RID_TT_EXTRAS_NAME ) ), 0, pBar->GetItemPos( RID_APPWINDOW ) );
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

#ifndef UNX
    pPrn    = new BasicPrinter;
#else
    pPrn    = NULL;
#endif
    pList   = new EditList;
    pStatus = new StatusLine( this );

    LoadIniFile();

    UpdateTitle();

    // Size: half width, 0.75 * height - 2 * IconSize
    {
        Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
        aConf.SetGroup("WinGeom");
        SetWindowState( aConf.ReadKey("WinParams", "") );
    }

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

const ByteString ProfilePrefix("_profile_");
const sal_uInt16 ProfilePrefixLen = ProfilePrefix.Len();

void BasicFrame::LoadIniFile()
{
    sal_uInt16 i;
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));

    for ( i = 0 ; i < aConf.GetGroupCount() ; i++ )
    {
        aConf.SetGroup( ByteString( aConf.GetGroupName( i ) ) );
        if ( ( aConf.ReadKey( "Aktuell" ).Len() || aConf.ReadKey( "Alle" ).Len() )
           &&( !aConf.ReadKey( "Current" ).Len() && !aConf.ReadKey( "All" ).Len() ) )
        {
            aConf.WriteKey( "Current", aConf.ReadKey( "Aktuell" ) );
            aConf.WriteKey( "All", aConf.ReadKey( "Alle" ) );
        }
    }

    aConf.SetGroup("Misc");
    ByteString aTemp;
    ByteString aCurrentProfile = aConf.ReadKey( "CurrentProfile", "Misc" );

    pStatus->SetProfileName( String( aCurrentProfile.Copy( ProfilePrefixLen ), RTL_TEXTENCODING_UTF8 ) );

    aConf.SetGroup( aCurrentProfile );
    aTemp = aConf.ReadKey( "AutoReload", "0" );
    bAutoReload = ( aTemp.CompareTo("1") == COMPARE_EQUAL );
    aTemp = aConf.ReadKey( "AutoSave", "0" );
    bAutoSave = ( aTemp.CompareTo("1") == COMPARE_EQUAL );

    LoadLRU();

    if ( pBasic )
        pBasic->LoadIniFile();

    for ( i = 0 ; i < pList->size() ; i++ )
        pList->at( i )->LoadIniFile();
}

BasicFrame::~BasicFrame()
{
    for ( size_t i = 0, n = pList->size(); i < n; ++i )
        delete pList->at( i );
    pList->clear();

    MenuBar *pBar = GetMenuBar();
    SetMenuBar( NULL );
    delete pBar;

    delete pStatus;
    delete pPrn;
    delete pList;
    pBasic.Clear();
}

void BasicFrame::Command( const CommandEvent& rCEvt )
{
    switch( rCEvt.GetCommand() ) {
        case COMMAND_SHOWDIALOG:
            {
                const CommandDialogData* pData = rCEvt.GetDialogData();
                if ( pData)
                {
                    const int nCommand = pData->GetDialogId();

                    switch (nCommand)
                    {
                        case SHOWDIALOG_ID_PREFERENCES :
                                Command( RID_OPTIONS );
                                break;

                        case SHOWDIALOG_ID_ABOUT :
                                Command( RID_HELPABOUT );
                                break;

                        default :
                                ;
                    }
                }
            }
            break;
    }
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
        for ( size_t i = pList->size() ; i > 0 ; )
            pList->at( --i )->CheckReload();

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

sal_Bool BasicFrame::IsAutoRun()
{
    return bIsAutoRun;
}

void BasicFrame::SetAutoRun( sal_Bool bAuto )
{
    bIsAutoRun = bAuto;
}

void BasicFrame::Notify( SfxBroadcaster&, const SfxHint& rHint )
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
                    String aTotalStatus( pStatusHint->GetExecutionStatus() );
                    aTotalStatus.AppendAscii( " " );
                    aTotalStatus.Append( pStatusHint->GetAdditionalExecutionStatus() );
                    pStatus->Message( aTotalStatus );
                }
                break;
            case TT_EXECUTION_HIDE_ACTION:
                {
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


    // Resize possibly maximized window
    for( size_t i = pList->size(); i > 0 ; i-- )
    {
        if ( pList->at( i-1 )->GetWinState() == TT_WIN_STATE_MAX )
            pList->at( i-1 )->Maximize();
    }
}

void BasicFrame::Move()
{
    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("WinGeom");
    aConf.WriteKey("WinParams",GetWindowState());
}

void BasicFrame::GetFocus()
{
    if ( pWork )
        pWork->GrabFocus();
}

IMPL_LINK( BasicFrame, CloseButtonClick, void*, EMPTYARG )
{
    AppWin* p;
    for ( size_t i = pList->size(); i > 0; --i )
    {
        p = pList->at( i - 1 );
        if ( p->GetWinState() == TT_WIN_STATE_MAX )
        {
            p->GrabFocus();
            break;
        }
    }
    return Command( RID_FILECLOSE, sal_False );
}

IMPL_LINK( BasicFrame, FloatButtonClick, void*, EMPTYARG )
{
    AppWin* p;
    for ( size_t i = pList->size(); i > 0; --i )
    {
        p = pList->at( i - 1 );
        if ( p->GetWinState() == TT_WIN_STATE_MAX )
        {
            p->TitleButtonClick( TITLE_BUTTON_DOCKING );
            break;
        }
    }
    return 1;
}

IMPL_LINK( BasicFrame, HideButtonClick, void*, EMPTYARG )
{
    AppWin* p;
    for ( size_t i = pList->size(); i > 0; --i )
    {
        p = pList->at( i - 1 );
        if ( p->GetWinState() == TT_WIN_STATE_MAX )
        {
            p->TitleButtonClick( TITLE_BUTTON_HIDE );
            break;
        }
    }
    return 1;
}

void BasicFrame::WinShow_Hide()
{
    if ( pList->empty() )
        return;

    AppWin* p;
    sal_Bool bWasFullscreen = sal_False;
    for ( size_t i = pList->size(); i > 0; --i )
    {
        p = pList->at( i - 1 );
        if ( p->pDataEdit )
        {
            if ( p->GetWinState() & TT_WIN_STATE_HIDE   // Hidden
                 || ( bWasFullscreen && ( !p->IsPined() || p->GetWinState() & TT_WIN_STATE_MAX ))
               )
                p->Hide( SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
            else
                p->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
        }
        bWasFullscreen |= p->GetWinState() == TT_WIN_STATE_MAX;
    }
}

void BasicFrame::WinMax_Restore()
{
    // The application buttons
    AppWin* p;
    sal_Bool bHasFullscreenWin = sal_False;
    for ( size_t i = 0, n = pList->size(); i < n && !bHasFullscreenWin; ++i )
    {
        p = pList->at( i );
        bHasFullscreenWin = ( p->GetWinState() == TT_WIN_STATE_MAX );
    }
    GetMenuBar()->ShowButtons( bHasFullscreenWin, sal_False, sal_False );
    WinShow_Hide();
}

void BasicFrame::RemoveWindow( AppWin *pWin )
{
    for ( EditList::iterator it = pList->begin(); it < pList->end(); ++it )
    {
        if ( *it == pWin )
        {
            pList->erase( it );
            break;
        }
    }
    pWork = ( pList->empty() ) ? NULL : pList->back();

    WinShow_Hide();

    if ( pWork )
        pWork->ToTop();

    WinMax_Restore();

    Menu* pMenu = GetMenuBar();
    if( pList->empty() )
    {
        pMenu->EnableItem( RID_APPEDIT,   sal_False );
        pMenu->EnableItem( RID_APPRUN,    sal_False );
        pMenu->EnableItem( RID_APPWINDOW, sal_False );
    }

    PopupMenu* pWinMenu = pMenu->GetPopupMenu( RID_APPWINDOW );

    pWinMenu->RemoveItem( pWinMenu->GetItemPos( pWin->GetWinId() ) );

    // Remove separator
    if ( pWinMenu->GetItemType( pWinMenu->GetItemCount() - 1 ) == MENUITEM_SEPARATOR )
        pWinMenu->RemoveItem( pWinMenu->GetItemCount() - 1 );

    pStatus->LoadTaskToolBox();
}

void BasicFrame::AddWindow( AppWin *pWin )
{
    pList->push_back( pWin );
    pWork = pWin;

    WinMax_Restore();

    // Enable main menu
    MenuBar* pMenu = GetMenuBar();
    if( !pList->empty() )
    {
        pMenu->EnableItem( RID_APPEDIT,   sal_True );
        pMenu->EnableItem( RID_APPRUN,    sal_True );
        pMenu->EnableItem( RID_APPWINDOW, sal_True );
    }

    PopupMenu* pWinMenu = pMenu->GetPopupMenu( RID_APPWINDOW );
    sal_uInt16 nLastID = pWinMenu->GetItemId( pWinMenu->GetItemCount() - 1 );

    // Separator necessary
    if ( nLastID < RID_WIN_FILE1 && pWinMenu->GetItemType( pWinMenu->GetItemCount() - 1 ) != MENUITEM_SEPARATOR )
        pWinMenu->InsertSeparator();

    // Find free ID
    sal_uInt16 nFreeID = RID_WIN_FILE1;
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
    for ( EditList::iterator it = pList->begin(); it < pList->end(); ++it )
    {
        if ( *it == pWin )
        {
            pList->erase( it );
            break;
        }
    }
    pList->push_back( pWin );
    pWin->Minimize( sal_False );

    aAppFile = pWin->GetText();
    UpdateTitle();

    WinShow_Hide();
    pStatus->LoadTaskToolBox();
}

sal_Bool BasicFrame::Close()
{
    if( bInBreak || Basic().IsRunning() )
        if( RET_NO == QueryBox( this, SttResId( IDS_RUNNING ) ).Execute() )
            return sal_False;

    StarBASIC::Stop();
    bInBreak = sal_False;
    if( CloseAll() )
    {
        aLineNum.Stop();

        // Close remaining dialogs to avoid assertions
        while ( GetWindow( WINDOW_OVERLAP )->GetWindow( WINDOW_FIRSTOVERLAP ) )
        {
            delete GetWindow( WINDOW_OVERLAP )->GetWindow( WINDOW_FIRSTOVERLAP )->GetWindow( WINDOW_CLIENT );
        }

        Application::SetDefDialogParent( NULL );
        WorkWindow::Close();

        return sal_True;
    } else return sal_False;
}

sal_Bool BasicFrame::CloseAll()
{
    while ( !pList->empty() )
        if ( !pList->back()->Close() )
            return sal_False;
    return sal_True;
}

sal_Bool BasicFrame::CompileAll()
{
    AppWin* p;
    for ( size_t i = 0, n = pList->size(); i < n; ++i )
    {
        p = pList->at( i );
        if ( p->ISA(AppBasEd) && !((AppBasEd*)p)->Compile() ) return sal_False;
    }
    return sal_True;
}

// Setup menu
#define MENU2FILENAME( Name ) Name.Copy( Name.SearchAscii(" ") +1).EraseAllChars( '~' )
#define LRUNr( nNr ) CByteString("LRU").Append( ByteString::CreateFromInt32( nNr ) )
String FILENAME2MENU( sal_uInt16 nNr, String aName )
{
    String aRet;
    if ( nNr <= 9 )
        aRet = CUniString("~").Append( UniString::CreateFromInt32( nNr ) );
    else if ( nNr == 10 )
        aRet = CUniString("1~0");
    else
        aRet = UniString::CreateFromInt32( nNr );

    return aRet.AppendAscii(" ").Append( aName );
}

void BasicFrame::AddToLRU(String const& aFile)
{
    Config aConfig(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    PopupMenu *pPopup  = GetMenuBar()->GetPopupMenu(RID_APPFILE);

    aConfig.SetGroup("LRU");
    sal_uInt16 nMaxLRU = (sal_uInt16)aConfig.ReadKey("MaxLRU","4").ToInt32();
    DirEntry aFileEntry( aFile );
    sal_uInt16 i,nLastMove = nMaxLRU;

    for ( i = 1 ; i<nMaxLRU && nLastMove == nMaxLRU ; i++ )
    {
        if ( DirEntry( UniString( aConfig.ReadKey(LRUNr(i),""), RTL_TEXTENCODING_UTF8 ) ) == aFileEntry )
            nLastMove = i;
    }

    if ( pPopup->GetItemPos( IDM_FILE_LRU1 ) == MENU_ITEM_NOTFOUND )
        pPopup->InsertSeparator();
    for ( i = nLastMove ; i>1 ; i-- )
    {
        if ( aConfig.ReadKey(LRUNr(i-1),"").Len() )
        {
            aConfig.WriteKey(LRUNr(i), aConfig.ReadKey(LRUNr(i-1),""));
            if ( pPopup->GetItemPos( IDM_FILE_LRU1 + i-1 ) == MENU_ITEM_NOTFOUND )
                 pPopup->InsertItem(IDM_FILE_LRU1 + i-1, FILENAME2MENU( i, MENU2FILENAME( pPopup->GetItemText(IDM_FILE_LRU1 + i-1-1) ) ));
            else
                pPopup->SetItemText(IDM_FILE_LRU1 + i-1,FILENAME2MENU( i, MENU2FILENAME( pPopup->GetItemText(IDM_FILE_LRU1 + i-1-1) ) ));
        }
    }
    aConfig.WriteKey(LRUNr(1), ByteString( aFile, RTL_TEXTENCODING_UTF8 ) );
    if ( pPopup->GetItemPos( IDM_FILE_LRU1 ) == MENU_ITEM_NOTFOUND )
         pPopup->InsertItem(IDM_FILE_LRU1,FILENAME2MENU( 1, aFile));
    else
        pPopup->SetItemText(IDM_FILE_LRU1,FILENAME2MENU( 1, aFile));
}

void BasicFrame::LoadLRU()
{
    Config     aConfig(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    PopupMenu *pPopup  = GetMenuBar()->GetPopupMenu(RID_APPFILE);
    sal_Bool       bAddSep = sal_True;

    aConfig.SetGroup("LRU");
    sal_uInt16 nMaxLRU = (sal_uInt16)aConfig.ReadKey("MaxLRU","4").ToInt32();

    if ( pPopup )
        bAddSep = pPopup->GetItemPos( IDM_FILE_LRU1 ) == MENU_ITEM_NOTFOUND;

    sal_uInt16 i;
    for ( i = 1; i <= nMaxLRU && pPopup != NULL; i++)
    {
        String aFile = UniString( aConfig.ReadKey(LRUNr(i)), RTL_TEXTENCODING_UTF8 );

        if (aFile.Len() != 0)
        {
            if (bAddSep)
            {
                pPopup->InsertSeparator();
                bAddSep = sal_False;
            }

            if ( pPopup->GetItemPos( IDM_FILE_LRU1 + i-1 ) == MENU_ITEM_NOTFOUND )
                pPopup->InsertItem(IDM_FILE_LRU1 + i-1, FILENAME2MENU( i, aFile ));
            else
                pPopup->SetItemText(IDM_FILE_LRU1 + i-1, FILENAME2MENU( i, aFile ));
        }
    }
    i = nMaxLRU+1;
    while ( pPopup->GetItemPos( IDM_FILE_LRU1 + i-1 ) != MENU_ITEM_NOTFOUND )
    {
        pPopup->RemoveItem( pPopup->GetItemPos( IDM_FILE_LRU1 + i-1 ) );
        i++;
    }
}

IMPL_LINK( BasicFrame, InitMenu, Menu *, pMenu )
{
    sal_Bool bNormal = sal_Bool( !bInBreak );
    pMenu->EnableItem( RID_RUNCOMPILE, bNormal );

    sal_Bool bHasEdit = sal_Bool( pWork != NULL );

    pMenu->EnableItem( RID_FILECLOSE,   bHasEdit );
    pMenu->EnableItem( RID_FILESAVE,    bHasEdit );
    pMenu->EnableItem( RID_FILESAVEAS,  bHasEdit );
    pMenu->EnableItem( RID_FILEPRINT,   bHasEdit );
    pMenu->EnableItem( RID_FILESETUP,   bHasEdit );
    pMenu->EnableItem( RID_FILELOADLIB, bNormal );
    pMenu->EnableItem( RID_FILESAVELIB, bHasEdit );

    sal_Bool bHasErr = sal_Bool( bNormal && pBasic->GetErrors() != 0 );
    sal_Bool bNext   = bHasErr & bNormal;
    sal_Bool bPrev   = bHasErr & bNormal;
    if( bHasErr )
    {
        size_t n = pBasic->GetCurrentError();
        if( n == 0 )
            bPrev = sal_False;
        if( SbError(n+1) == pBasic->GetErrors() )
            bNext = sal_False;
    }
    pMenu->EnableItem( RID_RUNNEXTERR, bNext );
    pMenu->EnableItem( RID_RUNPREVERR, bPrev );
    pMenu->CheckItem( RID_RUNDISAS, bDisas );
    if( pWork )
        pWork->InitMenu( pMenu );

    return sal_True;
}

IMPL_LINK_INLINE_START( BasicFrame, DeInitMenu, Menu *, pMenu )
{
    (void) pMenu; /* avoid warning about unused parameter */

    SetAutoRun( sal_False );
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
    sal_uInt16 nId = pMenu->GetCurItemId();
    sal_Bool bChecked = pMenu->IsItemChecked( nId );
    return Command( nId, bChecked );
}
IMPL_LINK_INLINE_END( BasicFrame, MenuCommand, Menu *, pMenu )

IMPL_LINK_INLINE_START( BasicFrame, Accel, Accelerator*, pAcc )
{
    SetAutoRun( sal_False );
    return Command( pAcc->GetCurItemId() );
}
IMPL_LINK_INLINE_END( BasicFrame, Accel, Accelerator*, pAcc )

IMPL_LINK_INLINE_START( BasicFrame, ShowLineNr, AutoTimer *, pTimer )
{
    (void) pTimer; /* avoid warning about unused parameter */
    String aPos;
    if ( pWork && pWork->ISA(AppBasEd))
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

AppBasEd* BasicFrame::CreateModuleWin( SbModule* pMod )
{
    String aModName = pMod->GetName();
    if ( aModName.Copy(0,2).CompareToAscii("--") == COMPARE_EQUAL )
        aModName.Erase(0,2);
    pMod->SetName(aModName);
    AppBasEd* p = new AppBasEd( this, pMod );
    p->Show();
    p->GrabFocus();
    p->ToTop();
    return p;
}

sal_Bool BasicFrame::LoadFile( String aFilename )
{
    sal_Bool bIsResult = DirEntry( aFilename ).GetExtension().CompareIgnoreCaseToAscii("RES") == COMPARE_EQUAL;
    sal_Bool bIsBasic = DirEntry( aFilename ).GetExtension().CompareIgnoreCaseToAscii("BAS") == COMPARE_EQUAL;
    bIsBasic |= DirEntry( aFilename ).GetExtension().CompareIgnoreCaseToAscii("INC") == COMPARE_EQUAL;

    AppWin* p;
    sal_Bool bSuccess = sal_True;
    if ( bIsResult )
    {
        p = new AppError( this, aFilename );
    }
    else if ( bIsBasic )
    {
        p = new AppBasEd( this, NULL );
        bSuccess = p->Load( aFilename );
    }
    else
    {
        p = new AppEdit( this );
        bSuccess = p->Load( aFilename );
    }
    if ( bSuccess )
    {
        p->Show();
        p->GrabFocus();
    }
    else
        delete p;

    return bSuccess;
}

// Execute command
long BasicFrame::Command( short nID, sal_Bool bChecked )
{
    BasicError* pErr;

    switch( nID ) {
        case RID_FILENEW: {
            AppBasEd* p = new AppBasEd( this, NULL );
            p->Show();
            p->GrabFocus();
            } break;
        case RID_FILEOPEN:
            {
                String s;
                if( QueryFileName( s, FT_BASIC_SOURCE | FT_RESULT_FILE, sal_False ) ) {
                    AddToLRU( s );
                    LoadFile( s );
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
            break;
        case RID_FILEPRINT:
            if( pWork )
                pPrn->Print( pWork->GetText(), pWork->pDataEdit->GetText(), this );
            break;
        case RID_FILESETUP:
            break;
        case RID_QUIT:
            if( Close() ) aBasicApp.Quit();
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
#if OSL_DEBUG_LEVEL > 1
                sal_uInt16 x;
                x = pWork->GetLineNr();
                x = ((AppBasEd*)pWork)->GetModule()->GetBPCount();
                if ( !x )
                    x = pModule->SetBP(pWork->GetLineNr());
                x = pModule->GetBPCount();
#endif

                for ( sal_uInt16 nMethod = 0; nMethod < pModule->GetMethods()->Count(); nMethod++ )
                {
                    SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Get( nMethod );
                    DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
                    pMethod->SetDebugFlags( pMethod->GetDebugFlags() | SbDEBUG_BREAK );
                }
            }
            nFlags = SbDEBUG_BREAK;
            goto start;
        start: {
            if ( !Basic().IsRunning() || bInBreak )
            {
                AppBasEd* p = NULL;
                if( pWork && pWork->ISA(AppBasEd) )
                {
                    p = ((AppBasEd*)pWork);
                    p->ToTop();
                }
                else
                {
                    AppWin *w = NULL;
                    for ( size_t i = pList->size(); i > 0; --i )
                    {
                        if ( pList->at( i-1 )->ISA( AppBasEd ) )
                        {
                            w = pList->at( i-1 );
                            break;
                        }
                    }
                    if ( w )
                    {
                        p = ((AppBasEd*)w);
                        p->ToTop();
                    }
                    else
                        if ( IsAutoRun() )
                            printf( "No file loaded to run.\n" );
                }

                if( bInBreak )
                    // Reset the flag
                    bInBreak = sal_False;
                else
                {
                    if( IsAutoSave() && !SaveAll() ) break;
                    if( !CompileAll() ) break;
                    String aString;
                    pStatus->Message( aString );
                    if( p )
                    {
                        BasicDLL::SetDebugMode( sal_True );
                        Basic().ClearGlobalVars();
                        p->Run();
                        BasicDLL::SetDebugMode( sal_False );
                        // If cancelled during Interactive=FALSE
                    }
                }}
            }
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
            bDisas = sal_Bool( !bChecked );
            break;
        case RID_RUNBREAK:
            if ( Basic().IsRunning() && !bInBreak )
            {
                pINST->nBreakCallLvl = pINST->nCallLvl;
            }
            break;
        case RID_RUNSTOP:
            Basic().Stop();
            bInBreak = sal_False;
            break;
        case RID_RUNNEXTERR:
            pErr = pBasic->NextError();
            if( pErr ) pErr->Show();
            break;
        case RID_RUNPREVERR:
            pErr = pBasic->PrevError();
            if( pErr ) pErr->Show();
            break;

        case RID_OPTIONS:
            {
                OptionsDialog *pOptions = new OptionsDialog( this, SttResId(IDD_OPTIONS_DLG) );
                pOptions->Show();
            }
            break;
        case RID_DECLARE_HELPER:
            InfoBox( this, SttResId( IDS_NOT_YET_IMPLEMENTED ) ).Execute();
            break;

        case RID_WINTILE:
            {
                WindowArrange aArange;
                for ( size_t i = 0, n = pList->size(); i < n ; i++ )
                {
                    aArange.AddWindow( pList->at( i ) );
                    pList->at( i )->Restore();
                }

                sal_Int32 nTitleHeight;
                {
                    sal_Int32 nDummy1, nDummy2, nDummy3;
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
                for ( size_t i = 0, n = pList->size(); i < n ; i++ )
                {
                    aArange.AddWindow( pList->at( i ) );
                    pList->at( i )->Restore();
                }


                sal_Int32 nTitleHeight;
                {
                    sal_Int32 nDummy1, nDummy2, nDummy3;
                    GetBorder( nDummy1, nTitleHeight, nDummy2, nDummy3 );
                }

                Size aSize = GetOutputSizePixel();
                aSize.Height() -= nTitleHeight;
                Rectangle aRect( Point( 0, nTitleHeight ), aSize );

                aArange.Arrange( WINDOWARRANGE_HORZ, aRect );

            }
            break;
        case RID_WINTILEVERT:
            {
                WindowArrange aArange;
                for ( size_t i = 0, n = pList->size(); i < n ; i++ )
                {
                    aArange.AddWindow( pList->at( i ) );
                    pList->at( i )->Restore();
                }


                sal_Int32 nTitleHeight;
                {
                    sal_Int32 nDummy1, nDummy2, nDummy3;
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
                for ( size_t i = 0, n = pList->size(); i < n ; i++ )
                {
                    pList->at( i )->Cascade( i );
                }
            }
            break;

        case RID_HELPABOUT:
            {
                SttResId aResId( IDD_ABOUT_DIALOG );
                if ( Basic().pTestObject )    // Are we TestTool?
                    aResId = SttResId( IDD_TT_ABOUT_DIALOG );
                else
                    aResId = SttResId( IDD_ABOUT_DIALOG );
                AboutDialog aAbout( this, aResId );
                aAbout.Execute();
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
                aName.EraseAllChars( L'~' );
                AppWin* pWin = FindWin( aName );
                if ( pWin )
                    pWin->ToTop();
            }
            else if ( nID >= IDM_FILE_LRU1 && nID  <= IDM_FILE_LRUn )
            {
                String s = MENU2FILENAME( GetMenuBar()->GetPopupMenu(RID_APPFILE)->GetItemText(nID) );

                AddToLRU( s );
                LoadFile( s );
            }
            else
            {
                if( pWork )
                    pWork->Command( CommandEvent( Point(), nID ) );
            }
    }
    return sal_True;
}

sal_Bool BasicFrame::SaveAll()
{
    AppWin* p, *q = pWork;
    for ( size_t i = 0, n = pList->size(); i < n ; i++ )
    {
        p = pList->at( i );
        sal_uInt16 nRes = p->QuerySave( QUERY_DISK_CHANGED );
        if( (( nRes == SAVE_RES_ERROR ) && QueryBox(this,SttResId(IDS_ASKSAVEERROR)).Execute() == RET_NO )
            || ( nRes == SAVE_RES_CANCEL ) )
            return sal_False;
    }
    if ( q )
        q->ToTop();
    return sal_True;
}

IMPL_LINK( BasicFrame, ModuleWinExists, String*, pFilename )
{
    return FindModuleWin( *pFilename ) != NULL;
}

AppBasEd* BasicFrame::FindModuleWin( const String& rName )
{
    AppWin* p;
    for ( size_t i = 0, n = pList->size(); i < n ; i++ )
    {
        p = pList->at( i );
        if( p->ISA(AppBasEd) && ((AppBasEd*)p)->GetModName() == rName )
            return ((AppBasEd*)p);
    }
    return NULL;
}

AppError* BasicFrame::FindErrorWin( const String& rName )
{
    AppWin* p;
    for ( size_t i = 0, n = pList->size(); i < n ; i++ )
    {
        p = pList->at( i );
        if( p->ISA(AppError) && ((AppError*)p)->GetText() == rName )
            return ((AppError*)p);
    }
    return NULL;
}

AppWin* BasicFrame::FindWin( const String& rName )
{
    AppWin* p;
    for ( size_t i = 0, n = pList->size(); i < n ; i++ )
    {
        p = pList->at( i );
        if( p->GetText() == rName )
            return p;
    }
    return NULL;
}

AppWin* BasicFrame::FindWin( sal_uInt16 nWinId )
{
    AppWin* p;
    for ( size_t i = 0, n = pList->size(); i < n ; i++ )
    {
        p = pList->at( i );
        if( p->GetWinId() == nWinId )
            return p;
    }
    return NULL;
}

AppWin* BasicFrame::IsWinValid( AppWin* pMaybeWin )
{
    AppWin* p;
    for ( size_t i = 0, n = pList->size(); i < n ; i++ )
    {
        p = pList->at( i );
        if( p == pMaybeWin )
            return p;
    }
    return NULL;
}

IMPL_LINK( BasicFrame, WriteString, String*, pString )
{
    if ( !pList->empty() )
    {
        pList->back()->pDataEdit->ReplaceSelected( *pString );
        return sal_True;
    }
    else
        return sal_False;
}

class NewFileDialog : public FileDialog
{
private:
    String aLastPath;
public:
    ByteString aFilterType;
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
        return; // User decides after he has changed the path

    String aCurFilter = GetCurFilter();
    sal_uInt16 nFilterNr = 0;
    while ( nFilterNr < GetFilterCount() && aCurFilter != GetFilterName( nFilterNr ) )
    {
        nFilterNr++;
    }
    aFilterType = ByteString( GetFilterType( nFilterNr ), RTL_TEXTENCODING_UTF8 );

    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup( "Misc" );
    ByteString aCurrentProfile = aConf.ReadKey( "CurrentProfile", "Path" );
    aConf.SetGroup( aCurrentProfile );
    aLastPath = UniString( aConf.ReadKey( aFilterType, aConf.ReadKey( "BaseDir" ) ), RTL_TEXTENCODING_UTF8 );
    SetPath( aLastPath );
}

short NewFileDialog::Execute()
{
    sal_Bool bRet = (sal_Bool)FileDialog::Execute();
    if ( bRet )
    {
        Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
        aConf.SetGroup( "Misc" );
        ByteString aCurrentProfile = aConf.ReadKey( "CurrentProfile", "Path" );
        aConf.SetGroup( aCurrentProfile );
        aConf.WriteKey( aFilterType, ByteString( DirEntry( GetPath() ).GetPath().GetFull(), RTL_TEXTENCODING_UTF8 ) );
        aConf.WriteKey( "LastFilterName", ByteString( GetCurFilter(), RTL_TEXTENCODING_UTF8 ) );
    }
    return bRet;
}

sal_Bool BasicFrame::QueryFileName
                (String& rName, FileType nFileType, sal_Bool bSave )
{
    NewFileDialog aDlg( this, bSave ? WinBits( WB_SAVEAS ) :
                                WinBits( WB_OPEN ) );
    aDlg.SetText( String( SttResId( bSave ? IDS_SAVEDLG : IDS_LOADDLG ) ) );

    if ( nFileType & FT_RESULT_FILE )
    {
        aDlg.SetDefaultExt( String( SttResId( IDS_RESFILE ) ) );
        aDlg.AddFilter( String( SttResId( IDS_RESFILTER ) ), String( SttResId( IDS_RESFILE ) ) );
        aDlg.AddFilter( String( SttResId( IDS_TXTFILTER ) ), String( SttResId( IDS_TXTFILE ) ) );
        aDlg.SetCurFilter( SttResId( IDS_RESFILTER ) );
    }

    if ( nFileType & FT_BASIC_SOURCE )
    {
        aDlg.SetDefaultExt( String( SttResId( IDS_NONAMEFILE ) ) );
        aDlg.AddFilter( String( SttResId( IDS_BASFILTER ) ), String( SttResId( IDS_NONAMEFILE ) ) );
        aDlg.AddFilter( String( SttResId( IDS_INCFILTER ) ), String( SttResId( IDS_INCFILE ) ) );
        aDlg.SetCurFilter( SttResId( IDS_BASFILTER ) );
    }

    if ( nFileType & FT_BASIC_LIBRARY )
    {
        aDlg.SetDefaultExt( String( SttResId( IDS_LIBFILE ) ) );
        aDlg.AddFilter( String( SttResId( IDS_LIBFILTER ) ), String( SttResId( IDS_LIBFILE ) ) );
        aDlg.SetCurFilter( SttResId( IDS_LIBFILTER ) );
    }

    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup( "Misc" );
    ByteString aCurrentProfile = aConf.ReadKey( "CurrentProfile", "Path" );
    aConf.SetGroup( aCurrentProfile );
    ByteString aFilter( aConf.ReadKey( "LastFilterName") );
    if ( aFilter.Len() )
        aDlg.SetCurFilter( String( aFilter, RTL_TEXTENCODING_UTF8 ) );
    else
        aDlg.SetCurFilter( String( SttResId( IDS_BASFILTER ) ) );

    aDlg.FilterSelect(); // Selects the last used path
    if ( rName.Len() > 0 )
        aDlg.SetPath( rName );

    if( aDlg.Execute() )
    {
        rName = aDlg.GetPath();
        return sal_True;
    } else return sal_False;
}

sal_uInt16 BasicFrame::BreakHandler()
{
    bInBreak = sal_True;
    SetAppMode( String( SttResId ( IDS_APPMODE_BREAK ) ) );

    while( bInBreak ) {
        GetpApp()->Yield();
    }

    SetAppMode( String( SttResId ( IDS_APPMODE_RUN ) ) );

    return nFlags;
}

void BasicFrame::LoadLibrary()
{
    String s;
    if( QueryFileName( s, FT_BASIC_LIBRARY, sal_False ) )
    {
        CloseAll();
        SvFileStream aStrm( s, STREAM_STD_READ );
        MyBasic* pNew = (MyBasic*) SbxBase::Load( aStrm );
        if( pNew && pNew->ISA( MyBasic ) )
        {
            pBasic = pNew;
            // Show all contents if existing
            SbxArray* pMods = pBasic->GetModules();
            for( sal_uInt16 i = 0; i < pMods->Count(); i++ )
            {
                SbModule* pMod = (SbModule*) pMods->Get( i );
                AppWin* p = new AppBasEd( this, pMod );
                p->Show();
            }
        }
        else
        {
            delete pNew;
            ErrorBox( this, SttResId( IDS_READERROR ) ).Execute();
        }
    }
}

void BasicFrame::SaveLibrary()
{
    String s;
    if( QueryFileName( s, FT_BASIC_LIBRARY, sal_True ) )
    {
        SvFileStream aStrm( s, STREAM_STD_WRITE );
        if( !Basic().Store( aStrm ) )
            ErrorBox( this, SttResId( IDS_WRITEERROR ) ).Execute();
    }
}

String BasicFrame::GenRealString( const String &aResString )
{
    xub_StrLen nStart,nGleich = 0,nEnd = 0,nStartPos = 0;
    String aType,aValue,aResult(aResString);
    String aString;
    xub_StrLen nInsertPos = 0;
    sal_Bool bFound;
    bFound = sal_False;

    while ( (nStart = aResult.Search(StartKenn,nStartPos)) != STRING_NOTFOUND &&
            (nGleich = aResult.SearchAscii("=",nStart+StartKenn.Len())) != STRING_NOTFOUND &&
            (nEnd = aResult.Search(EndKenn,nGleich+1)) != STRING_NOTFOUND)
    {
        aType = aResult.Copy(nStart,nGleich-nStart);
        aValue = aResult.Copy(nGleich+1,nEnd-nGleich-1);
        if ( aType.CompareTo(ResKenn) == COMPARE_EQUAL )
        {
            if ( bFound )
            {
                // insert results of previous resource
                DBG_ASSERT( aString.SearchAscii( "($Arg" ) == STRING_NOTFOUND, "Argument missing in String");
                aResult.Insert( aString, nInsertPos );
                nStart = nStart + aString.Len();
                nEnd = nEnd + aString.Len();
                aString.Erase();
            }

            aString = String( SttResId( (sal_uInt16)(aValue.ToInt32()) ) );
            nInsertPos = nStart;
            nStartPos = nStart;
            aResult.Erase( nStart, nEnd-nStart+1 );
            bFound = sal_True;
        }
        else if ( aType.Search(BaseArgKenn) == 0 ) // Starts with BaseArgKenn
        {
            // TODO: What the hell is that for??
            sal_uInt16 nArgNr = sal_uInt16( aType.Copy( BaseArgKenn.Len() ).ToInt32() );
            DBG_ASSERT( aString.Search( CUniString("($Arg").Append( String::CreateFromInt32(nArgNr) ).AppendAscii(")") ) != STRING_NOTFOUND, "Extra Argument given in String");
            aString.SearchAndReplace( CUniString("($Arg").Append( String::CreateFromInt32(nArgNr) ).AppendAscii(")"), aValue );
            nStartPos = nStart;
            aResult.Erase( nStart, nEnd-nStart+1 );
        }
        else
        {
            OSL_FAIL( CByteString("Unknown replacement in String: ").Append( ByteString( aResult.Copy(nStart,nEnd-nStart), RTL_TEXTENCODING_UTF8 ) ).GetBuffer() );
            nStartPos = nStartPos + StartKenn.Len();
        }
    }
    if ( bFound )
    {
        DBG_ASSERT( aString.SearchAscii( "($Arg" ) == STRING_NOTFOUND, "Argument missing in String");
        aResult.Insert( aString, nInsertPos );
    }
    return aResult;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
