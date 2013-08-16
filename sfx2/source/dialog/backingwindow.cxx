/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "backingwindow.hxx"

#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

#include <unotools/dynamicmenuoptions.hxx>
#include <svtools/langhelp.hxx>
#include <svtools/colorcfg.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <toolkit/awt/vclxmenu.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

const char RECENT_FILE_LIST[] =   ".uno:RecentFileList";

const char WRITER_URL[] =         "private:factory/swriter";
const char CALC_URL[] =           "private:factory/scalc";
const char IMPRESS_WIZARD_URL[] = "private:factory/simpress?slot=6686";
const char DRAW_URL[] =           "private:factory/sdraw";
const char BASE_URL[] =           "private:factory/sdatabase?Interactive";
const char MATH_URL[] =           "private:factory/smath";
const char TEMPLATE_URL[] =       "slot:5500";
const char OPEN_URL[] =           ".uno:Open";
const char SERVICENAME_CFGREADACCESS[] = "com.sun.star.configuration.ConfigurationAccess";

const int nItemId_Extensions = 1;
const int nItemId_Info = 3;
const int nItemId_TplRep = 4;

BackingWindow::BackingWindow( Window* i_pParent ) :
    Window( i_pParent ),
    mbInitControls( false ),
    mnHideExternalLinks( 0 ),
    mpAccExec( NULL )
{
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(),
      "sfx/ui/startcenter.ui",
      "StartCenter" );

    get(mpOpenButton,       "open");
    get(mpTemplateButton,   "templates");

    get(mpModuleNotebook,   "modules_notebook");

    get(mpWriterButton,     "writer");
    get(mpCalcButton,       "calc");
    get(mpImpressButton,    "impress");
    get(mpDrawButton,       "draw");
    get(mpDBButton,         "database");
    get(mpMathButton,       "math");

    get(mpWriterAllButton,  "writer_all");
    get(mpCalcAllButton,    "calc_all");
    get(mpImpressAllButton, "impress_all");
    get(mpDrawAllButton,    "draw_all");
    get(mpDBAllButton,      "database_all");
    get(mpMathAllButton,    "math_all");

    get(mpExtensionsButton, "extension");
    get(mpInfoButton,       "info");
    get(mpTplRepButton,     "add_temp");

    get( mpAllRecentThumbnails,         "all_recent");
    get( mpWriterRecentThumbnails,      "writer_recent");
    get( mpCalcRecentThumbnails,        "calc_recent");
    get( mpImpressRecentThumbnails,     "impress_recent");
    get( mpDrawRecentThumbnails,        "draw_recent");
    get( mpDatabaseRecentThumbnails,    "database_recent");
    get( mpMathRecentThumbnails,        "math_recent");

    try
    {
        mxContext.set( ::comphelper::getProcessComponentContext(), uno::UNO_SET_THROW );
        Reference<lang::XMultiServiceFactory> xConfig = configuration::theDefaultProvider::get( mxContext );
        Sequence<Any> args(1);
        PropertyValue val(
            "nodepath",
            0,
            Any(OUString("/org.openoffice.Office.Common/Help/StartCenter")),
            PropertyState_DIRECT_VALUE);
        args.getArray()[0] <<= val;
        Reference<container::XNameAccess> xNameAccess(xConfig->createInstanceWithArguments(SERVICENAME_CFGREADACCESS,args), UNO_QUERY);
        if( xNameAccess.is() )
        {
            //throws css::container::NoSuchElementException, css::lang::WrappedTargetException
            Any value( xNameAccess->getByName("StartCenterHideExternalLinks") );
            mnHideExternalLinks = value.get<sal_Int32>();
        }
    }
    catch (const Exception& e)
    {
        SAL_WARN( "fwk", "BackingWindow - caught an exception! " << e.Message );
    }

    // fdo#34392: we do the layout dynamically, the layout depends on the font,
    // so we should handle data changed events (font changing) of the last child
    // control, at this point all the controls have updated settings (i.e. font).

    EnableChildTransparentMode();

    SetStyle( GetStyle() | WB_DIALOGCONTROL );

    // get dispatch provider
    Reference<XDesktop2> xDesktop = Desktop::create( comphelper::getProcessComponentContext() );
    mxDesktopDispatchProvider = xDesktop;

    mpTemplateButton->SetHelpId( ".HelpId:StartCenter:TemplateButton" );
    mpOpenButton->SetHelpId( ".HelpId:StartCenter:OpenButton" );

    mpWriterButton->SetHelpId( ".HelpId:StartCenter:WriterButton" );
    mpCalcButton->SetHelpId( ".HelpId:StartCenter:CalcButton" );
    mpImpressButton->SetHelpId( ".HelpId:StartCenter:ImpressButton" );
    mpDrawButton->SetHelpId( ".HelpId:StartCenter:DrawButton" );
    mpDBButton->SetHelpId( ".HelpId:StartCenter:DBButton" );
    mpMathButton->SetHelpId( ".HelpId:StartCenter:MathButton" );

    mpExtensionsButton->SetHelpId( ".HelpId:StartCenter:Extensions" );
    mpInfoButton->SetHelpId( ".HelpId:StartCenter:Info" );
    mpTplRepButton->SetHelpId( ".HelpId:StartCenter:TemplateRepository" );

    // init background
    SetBackground();
}


BackingWindow::~BackingWindow()
{
}

IMPL_LINK( BackingWindow, WindowEventListener, VclSimpleEvent*, pEvent )
{
    VclWindowEvent* pWinEvent = dynamic_cast<VclWindowEvent*>( pEvent );
    if ( pWinEvent && pWinEvent->GetId() == VCLEVENT_WINDOW_DATACHANGED )
    {
        DataChangedEvent* pDCEvt =
            static_cast<DataChangedEvent*>( pWinEvent->GetData() );
        if ( pDCEvt->GetFlags() & SETTINGS_STYLE )
        {
            SetBackground();
            Invalidate();
            // fdo#34392: Resize buttons to match the new text size.
            Resize();
        }
    }
    return 0;
}

void BackingWindow::initControls()
{
    if( mbInitControls )
        return;

    mbInitControls = true;

    // collect the URLs of the entries in the File/New menu
    SvtModuleOptions    aModuleOptions;
    std::set< OUString > aFileNewAppsAvailable;
    SvtDynamicMenuOptions aOpt;
    Sequence < Sequence < PropertyValue > > aNewMenu = aOpt.GetMenu( E_NEWMENU );
    const OUString sURLKey( "URL"  );

    const Sequence< PropertyValue >* pNewMenu = aNewMenu.getConstArray();
    const Sequence< PropertyValue >* pNewMenuEnd = aNewMenu.getConstArray() + aNewMenu.getLength();
    for ( ; pNewMenu != pNewMenuEnd; ++pNewMenu )
    {
        comphelper::SequenceAsHashMap aEntryItems( *pNewMenu );
        OUString sURL( aEntryItems.getUnpackedValueOrDefault( sURLKey, OUString() ) );
        if ( !sURL.isEmpty() )
            aFileNewAppsAvailable.insert( sURL );
    }

    setupModuleTab( "tab_writer", mpWriterRecentThumbnails, TYPE_WRITER,
        WRITER_URL, aFileNewAppsAvailable, aModuleOptions,
        SvtModuleOptions::E_SWRITER );
    setupModuleTab( "tab_calc", mpCalcRecentThumbnails, TYPE_CALC,
        DRAW_URL, aFileNewAppsAvailable, aModuleOptions,
        SvtModuleOptions::E_SDRAW );
    setupModuleTab( "tab_impress", mpImpressRecentThumbnails, TYPE_IMPRESS,
        CALC_URL, aFileNewAppsAvailable, aModuleOptions,
        SvtModuleOptions::E_SCALC );
    setupModuleTab( "tab_draw", mpDrawRecentThumbnails, TYPE_DRAW,
        BASE_URL, aFileNewAppsAvailable, aModuleOptions,
        SvtModuleOptions::E_SDATABASE );
    setupModuleTab( "tab_database", mpDatabaseRecentThumbnails, TYPE_DATABASE,
        IMPRESS_WIZARD_URL, aFileNewAppsAvailable, aModuleOptions,
        SvtModuleOptions::E_SIMPRESS );
    setupModuleTab( "tab_math", mpMathRecentThumbnails, TYPE_MATH,
        MATH_URL, aFileNewAppsAvailable, aModuleOptions,
        SvtModuleOptions::E_SMATH );

    // File types for mpAllRecentThumbnails are added in the above calls
    // of setupModuleTab. TYPE_OTHER is always added.
    mpAllRecentThumbnails->mnFileTypes |= TYPE_OTHER;
    mpAllRecentThumbnails->loadRecentDocs();

    setupButton( mpWriterButton );
    setupButton( mpDrawButton );
    setupButton( mpCalcButton );
    setupButton( mpDBButton );
    setupButton( mpImpressButton );
    setupButton( mpMathButton );

    setupButton( mpWriterAllButton );
    setupButton( mpDrawAllButton );
    setupButton( mpCalcAllButton );
    setupButton( mpDBAllButton );
    setupButton( mpImpressAllButton );
    setupButton( mpMathAllButton );

    setupButton( mpOpenButton );
    setupButton( mpTemplateButton );

    setupExternalLink( mpExtensionsButton );
    setupExternalLink( mpInfoButton );
    setupExternalLink( mpTplRepButton );

    Resize();
}

void BackingWindow::setupModuleTab(const OString& rTabName, RecentDocsView* pRecView, int nFileTypes,
    const OUString &rURL, const std::set<OUString>& rURLS, SvtModuleOptions& rOpt,
    SvtModuleOptions::EModule eMod)
{
    if( !rURL.isEmpty() && (!rOpt.IsModuleInstalled( eMod ) || rURLS.find( rURL ) == rURLS.end()) )
    {
        // disable the parts that are not installed
        mpModuleNotebook->RemovePage( mpModuleNotebook->GetPageId(rTabName) );
    }
    else
    {
        // if a module is installed, add that filetype to the "All" page
        mpAllRecentThumbnails-> mnFileTypes |= nFileTypes;
        pRecView->mnFileTypes |= nFileTypes;
        pRecView->loadRecentDocs();
    }
}

void BackingWindow::setupButton( PushButton* pButton )
{
    pButton->SetClickHdl( LINK( this, BackingWindow, ClickHdl ) );

    // setup text - slighly larger font than normal labels on the texts
    Font aFont;
    aFont.SetSize( Size( 0, 11 ) );
    aFont.SetWeight( WEIGHT_NORMAL );

    pButton->SetFont( aFont );
    pButton->SetControlFont( aFont );
}

void BackingWindow::setupExternalLink( PushButton* pButton )
{
    if( mnHideExternalLinks == 0 )
        pButton->Show();
    else
        pButton->Hide();

    pButton->SetClickHdl( LINK( this, BackingWindow, ExtLinkClickHdl ) );
}

void BackingWindow::Paint( const Rectangle& )
{
    Resize();

    Wallpaper aBack( svtools::ColorConfig().GetColorValue(::svtools::APPBACKGROUND).nColor );
    Region aClip( Rectangle( Point( 0, 0 ), GetOutputSizePixel() ) );

    aClip.Exclude( maStartCentButtons );

    Push( PUSH_CLIPREGION );
    IntersectClipRegion( aClip );
    DrawWallpaper( Rectangle( Point( 0, 0 ), GetOutputSizePixel() ), aBack );
    Pop();

    VirtualDevice aDev( *this );
    aDev.EnableRTL( IsRTLEnabled() );
    aDev.SetOutputSizePixel( maStartCentButtons.GetSize() );
    Point aOffset( Point( 0, 0 ) - maStartCentButtons.TopLeft());
    aDev.DrawWallpaper( Rectangle( aOffset, GetOutputSizePixel() ), aBack );

    DrawOutDev( maStartCentButtons.TopLeft(), maStartCentButtons.GetSize(),
                Point( 0, 0 ), maStartCentButtons.GetSize(),
                aDev );
}

long BackingWindow::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        // try the 'normal' accelerators (so that eg. Ctrl+Q works)
        if( !mpAccExec )
        {
            mpAccExec = svt::AcceleratorExecute::createAcceleratorHelper();
            mpAccExec->init( comphelper::getProcessComponentContext(), mxFrame);
        }

        const KeyEvent* pEvt = rNEvt.GetKeyEvent();
        const KeyCode& rKeyCode(pEvt->GetKeyCode());
        if( pEvt && mpAccExec->execute(rKeyCode) )
            return 1;
    }

    return Window::Notify( rNEvt );
}

void BackingWindow::setOwningFrame( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& xFrame )
{
    mxFrame = xFrame;
    if( ! mbInitControls )
        initControls();
}

void BackingWindow::Resize()
{
    maStartCentButtons = Rectangle( Point(0, 0), GetOutputSizePixel() );

    if (isLayoutEnabled(this))
        VclContainer::setLayoutAllocation(*GetWindow(WINDOW_FIRSTCHILD),
            maStartCentButtons.TopLeft(), maStartCentButtons.GetSize());

    if( !IsInPaint())
        Invalidate();
}

IMPL_LINK( BackingWindow, ExtLinkClickHdl, Button*, pButton )
{
    const char* pNode = NULL;

    if( pButton == mpExtensionsButton )
        pNode = "AddFeatureURL";
    else if( pButton == mpInfoButton )
        pNode = "InfoURL";
    else if( pButton == mpTplRepButton )
        pNode = "TemplateRepositoryURL";

    if( pNode )
    {
        const char* pNodePath = "/org.openoffice.Office.Common/Help/StartCenter";
        try
        {
            Reference<lang::XMultiServiceFactory> xConfig = configuration::theDefaultProvider::get( comphelper::getProcessComponentContext() );
            Sequence<Any> args(1);
            PropertyValue val(
                "nodepath",
                0,
                Any(OUString::createFromAscii(pNodePath)),
                PropertyState_DIRECT_VALUE);
            args.getArray()[0] <<= val;
            Reference<container::XNameAccess> xNameAccess(xConfig->createInstanceWithArguments(SERVICENAME_CFGREADACCESS,args), UNO_QUERY);
            if( xNameAccess.is() )
            {
                OUString sURL;
                //throws css::container::NoSuchElementException, css::lang::WrappedTargetException
                Any value( xNameAccess->getByName(OUString::createFromAscii(pNode)) );
                sURL = value.get<OUString> ();
                localizeWebserviceURI(sURL);

                Reference< com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
                    com::sun::star::system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
                //throws css::lang::IllegalArgumentException, css::system::SystemShellExecuteException
                xSystemShellExecute->execute( sURL, OUString(), com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY);
            }
        }
        catch (const Exception&)
        {
        }
    }
    return 0;
}

IMPL_LINK( BackingWindow, ClickHdl, Button*, pButton )
{
    // dispatch the appropriate URL and end the dialog
    if( pButton == mpWriterButton       || pButton == mpWriterAllButton )
        dispatchURL( WRITER_URL );
    else if( pButton == mpCalcButton    || pButton == mpCalcAllButton )
        dispatchURL( CALC_URL );
    else if( pButton == mpImpressButton || pButton == mpImpressAllButton )
        dispatchURL( IMPRESS_WIZARD_URL );
    else if( pButton == mpDrawButton    || pButton == mpDrawAllButton )
        dispatchURL( DRAW_URL );
    else if( pButton == mpDBButton      || pButton == mpDBAllButton )
        dispatchURL( BASE_URL );
    else if( pButton == mpMathButton    || pButton == mpMathAllButton )
        dispatchURL( MATH_URL );
    else if( pButton == mpOpenButton )
    {
        Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

        Sequence< com::sun::star::beans::PropertyValue > aArgs(1);
        PropertyValue* pArg = aArgs.getArray();
        pArg[0].Name = "Referer";
        pArg[0].Value <<= OUString("private:user");

        dispatchURL( OPEN_URL, OUString(), xFrame, aArgs );
    }
    else if( pButton == mpTemplateButton )
    {
        Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

        Sequence< com::sun::star::beans::PropertyValue > aArgs(1);
        PropertyValue* pArg = aArgs.getArray();
        pArg[0].Name = OUString("Referer");
        pArg[0].Value <<= OUString("private:user");

        dispatchURL( TEMPLATE_URL, OUString(), xFrame, aArgs );
    }
    return 0;
}

struct ImplDelayedDispatch
{
    Reference< XDispatch >      xDispatch;
    com::sun::star::util::URL   aDispatchURL;
    Sequence< PropertyValue >   aArgs;

    ImplDelayedDispatch( const Reference< XDispatch >& i_xDispatch,
                         const com::sun::star::util::URL& i_rURL,
                         const Sequence< PropertyValue >& i_rArgs )
    : xDispatch( i_xDispatch ),
      aDispatchURL( i_rURL ),
      aArgs( i_rArgs )
    {
    }
    ~ImplDelayedDispatch() {}
};

static long implDispatchDelayed( void*, void* pArg )
{
    struct ImplDelayedDispatch* pDispatch = reinterpret_cast<ImplDelayedDispatch*>(pArg);
    try
    {
        pDispatch->xDispatch->dispatch( pDispatch->aDispatchURL, pDispatch->aArgs );
    }
    catch (const Exception&)
    {
    }

    // clean up
    delete pDispatch;

    return 0;
}

void BackingWindow::dispatchURL( const OUString& i_rURL,
                                 const OUString& rTarget,
                                 const Reference< XDispatchProvider >& i_xProv,
                                 const Sequence< PropertyValue >& i_rArgs )
{
    // if no special dispatch provider is given, get the desktop
    Reference< XDispatchProvider > xProvider( i_xProv.is() ? i_xProv : mxDesktopDispatchProvider );

    // check for dispatch provider
    if( !xProvider.is())
        return;

    // get an URL transformer to clean up the URL
    com::sun::star::util::URL aDispatchURL;
    aDispatchURL.Complete = i_rURL;

    Reference < com::sun::star::util::XURLTransformer > xURLTransformer(
        com::sun::star::util::URLTransformer::create( comphelper::getProcessComponentContext() ) );
    try
    {
        // clean up the URL
        xURLTransformer->parseStrict( aDispatchURL );
        // get a Dispatch for the URL and target
        Reference< XDispatch > xDispatch(
            xProvider->queryDispatch( aDispatchURL, rTarget, 0 )
            );
        // dispatch the URL
        if ( xDispatch.is() )
        {
            ImplDelayedDispatch* pDisp = new ImplDelayedDispatch( xDispatch, aDispatchURL, i_rArgs );
            sal_uLong nEventId = 0;
            if( ! Application::PostUserEvent( nEventId, Link( NULL, implDispatchDelayed ), pDisp ) )
                delete pDisp; // event could not be posted for unknown reason, at least don't leak
        }
    }
    catch (const com::sun::star::uno::RuntimeException&)
    {
        throw;
    }
    catch (const com::sun::star::uno::Exception&)
    {
    }
}

Size BackingWindow::GetOptimalSize() const
{
    if (isLayoutEnabled(this))
        return VclContainer::getLayoutRequisition(*GetWindow(WINDOW_FIRSTCHILD));

    return Window::GetOptimalSize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab:*/
