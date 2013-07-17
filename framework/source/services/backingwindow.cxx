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
#include <services.h>

#include <sal/macros.h>

#include "vcl/metric.hxx"
#include "vcl/mnemonic.hxx"
#include "vcl/menu.hxx"
#include "vcl/svapp.hxx"
#include "vcl/virdev.hxx"

#include "tools/urlobj.hxx"

#include "unotools/dynamicmenuoptions.hxx"
#include "unotools/historyoptions.hxx"
#include "svtools/imagemgr.hxx"
#include "svtools/langhelp.hxx"
#include "svtools/colorcfg.hxx"

#include "comphelper/processfactory.hxx"
#include "comphelper/sequenceashashmap.hxx"
#include "comphelper/configurationhelper.hxx"

#include <toolkit/awt/vclxmenu.hxx>

#include "cppuhelper/implbase1.hxx"

#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"
#include "osl/file.h"

#include "com/sun/star/frame/Desktop.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/configuration/theDefaultProvider.hpp"
#include "com/sun/star/system/SystemShellExecute.hpp"
#include "com/sun/star/system/SystemShellExecuteFlags.hpp"
#include "com/sun/star/task/XJobExecutor.hpp"
#include "com/sun/star/util/XStringWidth.hpp"
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/PopupMenuControllerFactory.hpp>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace framework;

#define RECENT_FILE_LIST    ".uno:RecentFileList"

#define WRITER_URL      "private:factory/swriter"
#define CALC_URL        "private:factory/scalc"
#define IMPRESS_WIZARD_URL     "private:factory/simpress?slot=6686"
#define DRAW_URL        "private:factory/sdraw"
#define BASE_URL        "private:factory/sdatabase?Interactive"
#define MATH_URL        "private:factory/smath"
#define TEMPLATE_URL    "slot:5500"
#define OPEN_URL        ".uno:Open"

BackingWindow::BackingWindow( Window* i_pParent ) :
    Window( i_pParent ),
    mbInitControls( false ),
    mnHideExternalLinks( 0 ),
    mpAccExec( NULL )
{
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(),
      "modules/StartModule/ui/startcenter.ui",
      "StartCenter" );
    get(mpWriterButton,    "writer");
    get(mpCalcButton,      "calc");
    get(mpImpressButton,   "impress");
    get(mpOpenButton,      "open");
    get(mpDrawButton,      "draw");
    get(mpDBButton,        "database");
    get(mpMathButton,      "math");
    get(mpTemplateButton,  "templates");

    get(mpExtensionsButton, "extension");
    get(mpInfoButton,       "info");
    get(mpTplRepButton,     "add_temp");

    get(mpStartCenterContainer, "sccontainer");

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

        mxPopupMenuFactory.set(
            frame::PopupMenuControllerFactory::create( mxContext ) );
        // TODO If there is no PopupMenuController, the button should be a nomral one not a MenuButton
        if ( mxPopupMenuFactory->hasController(
            OUString( RECENT_FILE_LIST ) , OUString("com.sun.star.frame.StartModule") ) )
        {
            mxPopupMenu.set( mxContext->getServiceManager()->createInstanceWithContext(
                OUString( "com.sun.star.awt.PopupMenu" ), mxContext ), uno::UNO_QUERY_THROW );
        }
    }
    catch (const Exception& e)
    {
        SAL_WARN( "fwk", "BackingWindow - caught an exception! " << e.Message );
    }

    // clean up resource stack
    //FreeResource();

    // fdo#34392: we do the layout dynamically, the layout depends on the font,
    // so we should handle data changed events (font changing) of the last child
    // control, at this point all the controls have updated settings (i.e. font).

    EnableChildTransparentMode();

    SetStyle( GetStyle() | WB_DIALOGCONTROL );

    // get dispatch provider
    Reference<XDesktop2> xDesktop = Desktop::create( comphelper::getProcessComponentContext() );
    mxDesktopDispatchProvider = xDesktop;

    mpWriterButton->SetHelpId( ".HelpId:StartCenter:WriterButton" );
    mpCalcButton->SetHelpId( ".HelpId:StartCenter:CalcButton" );
    mpImpressButton->SetHelpId( ".HelpId:StartCenter:ImpressButton" );
    mpDrawButton->SetHelpId( ".HelpId:StartCenter:DrawButton" );
    mpDBButton->SetHelpId( ".HelpId:StartCenter:DBButton" );
    mpMathButton->SetHelpId( ".HelpId:StartCenter:MathButton" );
    mpTemplateButton->SetHelpId( ".HelpId:StartCenter:TemplateButton" );
    mpOpenButton->SetHelpId( ".HelpId:StartCenter:OpenButton" );
    mpExtensionsButton->SetHelpId( ".HelpId:StartCenter:Extensions" );
    mpInfoButton->SetHelpId( ".HelpId:StartCenter:Info" );
    mpTplRepButton->SetHelpId( ".HelpId:StartCenter:TemplateRepository" );

    // init background
    initBackground();

    // set a slighly larger font than normal labels on the texts
    maTextFont.SetSize( Size( 0, 11 ) );
    maTextFont.SetWeight( WEIGHT_NORMAL );

    Window::Show();
}


BackingWindow::~BackingWindow()
{
    delete mpAccExec;

    if( mxPopupMenuController.is() )
    {
        Reference< lang::XComponent > xComponent( mxPopupMenuController, UNO_QUERY );
        if( xComponent.is() )
        {
            try
            {
                xComponent->dispose();
            }
            catch (...)
            {}
        }
        mxPopupMenuController.clear();
    }
    mxPopupMenuFactory.clear();
    mxPopupMenu.clear();
}

void BackingWindow::GetFocus()
{
    // CRASH
    //if( IsVisible() )
    //    mpWriterButton->GrabFocus();
    Window::GetFocus();
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
            initBackground();
            Invalidate();
            // fdo#34392: Resize buttons to match the new text size.
            Resize();
        }
    }
    return 0;
}

/*
void BackingWindow::prepareRecentFileMenu()
{
    if( ! mxPopupMenu.is() )
        return;

    if ( !mxPopupMenuController.is() )
    {
        uno::Sequence< uno::Any > aArgs( 2 );
        beans::PropertyValue aProp;

        aProp.Name = OUString( "Frame" );
        aProp.Value <<= mxFrame;
        aArgs[0] <<= aProp;

        aProp.Name = OUString( "ModuleIdentifier" );
        aProp.Value <<= OUString("com.sun.star.frame.StartModule");
        aArgs[1] <<= aProp;
        try
        {
            mxPopupMenuController.set(
                mxPopupMenuFactory->createInstanceWithArgumentsAndContext(
                    OUString( RECENT_FILE_LIST ), aArgs, mxContext),
                        uno::UNO_QUERY_THROW );
            mxPopupMenuController->setPopupMenu( mxPopupMenu );
        }
        catch ( const Exception &e )
        {
            SAL_WARN( "fwk", "BackingWindow - caught an exception! " << e.Message );
        }

        PopupMenu *pRecentMenu = NULL;
        VCLXMenu* pTKMenu = VCLXMenu::GetImplementation( mxPopupMenu );
        if ( pTKMenu )
            pRecentMenu = dynamic_cast< PopupMenu * >( pTKMenu->GetMenu() );
        mpOpenButton->SetPopupMenu( pRecentMenu );
    }
}
*/

void BackingWindow::initBackground()
{
    SetBackground();

    // scale middle segment
    Size aMiddleSize;
    if( !! maBackgroundMiddle )
        aMiddleSize = maBackgroundMiddle.GetSizePixel();
    // load middle segment

    Application::LoadBrandBitmap ("shell/backing_space", maBackgroundMiddle);

    // and scale it to previous size
    if( aMiddleSize.Width() && aMiddleSize.Height() )
        maBackgroundMiddle.Scale( aMiddleSize );

    if( GetSettings().GetLayoutRTL() )
    {
        // replace images by RTL versions
        Application::LoadBrandBitmap ("shell/backing_rtl_right", maBackgroundLeft);
        Application::LoadBrandBitmap ("shell/backing_rtl_left", maBackgroundRight);
    }
    else
    {
        Application::LoadBrandBitmap ("shell/backing_left", maBackgroundLeft);
        Application::LoadBrandBitmap ("shell/backing_right", maBackgroundRight);
    }

    // CRASH
    //mpOpenButton->SetMenuMode( MENUBUTTON_MENUMODE_TIMED );
    //mpOpenButton->SetActivateHdl( LINK( this, BackingWindow, ActivateHdl ) );

    // this will be moved to somewhere saner later
    nSCWidth = 780;
    nSCHeight = maBackgroundLeft.GetSizePixel().Height();
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

    // create mnemonics on the fly, preregister the mnemonics of the menu
    MnemonicGenerator aMnemns;

    SystemWindow* pSysWin = GetSystemWindow();
    if( pSysWin )
    {
        MenuBar* pMBar = pSysWin->GetMenuBar();
        if( pMBar )
        {
            for( sal_uInt16 i = 0; i < pMBar->GetItemCount(); i++ )
            {
                sal_uInt16 nItemId = pMBar->GetItemId( i );
                String aItemText( pMBar->GetItemText( nItemId ) );
                if( aItemText.Len() )
                    aMnemns.RegisterMnemonic( aItemText );
            }
        }
    }

    // layout the buttons

    layoutButton( WRITER_URL, aFileNewAppsAvailable,
                  aModuleOptions, SvtModuleOptions::E_SWRITER,
                  *mpWriterButton, aMnemns );
    layoutButton( DRAW_URL, aFileNewAppsAvailable,
                  aModuleOptions, SvtModuleOptions::E_SDRAW,
                  *mpDrawButton, aMnemns );
    layoutButton( CALC_URL, aFileNewAppsAvailable,
                  aModuleOptions, SvtModuleOptions::E_SCALC,
                  *mpCalcButton, aMnemns );
    layoutButton( BASE_URL, aFileNewAppsAvailable,
                  aModuleOptions, SvtModuleOptions::E_SDATABASE,
                  *mpDBButton, aMnemns );
    layoutButton( IMPRESS_WIZARD_URL, aFileNewAppsAvailable,
                  aModuleOptions, SvtModuleOptions::E_SIMPRESS,
                  *mpImpressButton, aMnemns );
    layoutButton( MATH_URL, aFileNewAppsAvailable,
                  aModuleOptions, SvtModuleOptions::E_SMATH,
                  *mpMathButton, aMnemns );

    layoutButton( NULL, aFileNewAppsAvailable,
                  aModuleOptions, SvtModuleOptions::E_SWRITER,
                  *mpOpenButton, aMnemns );
    layoutButton( NULL, aFileNewAppsAvailable,
                  aModuleOptions, SvtModuleOptions::E_SWRITER,
                  *mpTemplateButton, aMnemns );

    layoutExternalLink( *mpExtensionsButton );
    layoutExternalLink( *mpInfoButton );
    layoutExternalLink( *mpTplRepButton );

    //SAL _DEBUG("container size: " << mpStartCenterContainer->GetSizePixel().Width()
    //            << " " << mpStartCenterContainer->GetSizePixel().Height());

    Resize();

    mpWriterButton->GrabFocus();
}


void BackingWindow::layoutButton(
                          const char* i_pURL,
                          const std::set<OUString>& i_rURLS,
                          SvtModuleOptions& i_rOpt, SvtModuleOptions::EModule i_eMod,
                          PushButton& i_rBtn,
                          MnemonicGenerator& i_rMnemns,
                          const String& i_rStr
                          )
{
    OUString aURL( i_pURL ? OUString::createFromAscii( i_pURL ) : OUString() );
    // setup button
    i_rBtn.SetPaintTransparent( sal_False );
    i_rBtn.SetClickHdl( LINK( this, BackingWindow, ClickHdl ) );
    if( i_pURL && (! i_rOpt.IsModuleInstalled( i_eMod ) || i_rURLS.find( aURL ) == i_rURLS.end()) )
    {
        i_rBtn.Enable( sal_False );
    }

    // setup text
    i_rBtn.SetFont( maTextFont );
    i_rBtn.SetControlFont( maTextFont );

    String aText( i_rStr.Len() ? i_rStr : SvFileInformationManager::GetDescription( INetURLObject( aURL ) ) );
    i_rMnemns.CreateMnemonic( aText );
    i_rBtn.SetText( aText );

    //without this line, imagebuttons with text don't work properly
    i_rBtn.SetImageAlign( IMAGEALIGN_LEFT );

    // show the controls
    i_rBtn.Show();
}

void BackingWindow::layoutExternalLink( PushButton& i_rBtn )
{
    i_rBtn.SetPaintTransparent( sal_False );
    i_rBtn.SetClickHdl( LINK( this, BackingWindow, ExtLinkClickHdl ) );

    if( mnHideExternalLinks == 0 )
        i_rBtn.Show();
}

void BackingWindow::Paint( const Rectangle& )
{
    Resize();

    Wallpaper aBack( svtools::ColorConfig().GetColorValue(::svtools::APPBACKGROUND).nColor );
    Region aClip( Rectangle( Point( 0, 0 ), GetOutputSizePixel() ) );

    Rectangle aBmpRect(Point((GetOutputSizePixel().Width()-nSCWidth)/2,
                             (GetOutputSizePixel().Height()-nSCHeight)/2),
                       Size(nSCWidth,nSCHeight));
    aClip.Exclude( aBmpRect );

    Push( PUSH_CLIPREGION );
    IntersectClipRegion( aClip );
    DrawWallpaper( Rectangle( Point( 0, 0 ), GetOutputSizePixel() ), aBack );
    Pop();

    VirtualDevice aDev( *this );
    aDev.EnableRTL( IsRTLEnabled() );
    aDev.SetOutputSizePixel( aBmpRect.GetSize() );
    Point aOffset( Point( 0, 0 ) - aBmpRect.TopLeft());
    aDev.DrawWallpaper( Rectangle( aOffset, GetOutputSizePixel() ), aBack );

    maBackgroundMiddle.Scale(
        Size(nSCWidth - maBackgroundLeft.GetSizePixel().Width() - maBackgroundRight.GetSizePixel().Width(),
        maBackgroundMiddle.GetSizePixel().Height()),
        BMP_SCALE_FAST);

    // draw bitmap
    Point aTL( 0, 0 );
    aDev.DrawBitmapEx( aTL, maBackgroundLeft );
    aTL.X() += maBackgroundLeft.GetSizePixel().Width();
    if( !!maBackgroundMiddle )
    {
        aDev.DrawBitmapEx( aTL, maBackgroundMiddle );
        aTL.X() += maBackgroundMiddle.GetSizePixel().Width();
    }
    aDev.DrawBitmapEx( aTL, maBackgroundRight );

    DrawOutDev( aBmpRect.TopLeft(), aBmpRect.GetSize(),
                Point( 0, 0 ), aBmpRect.GetSize(),
                aDev );

}

long BackingWindow::Notify( NotifyEvent& rNEvt )
{
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
    maStartCentButtons = Rectangle(
                        Point((GetOutputSizePixel().Width()-nSCWidth)/2 + nShadowTop + nPaddingTop,
                              (GetOutputSizePixel().Height()-nSCHeight)/2 + nShadowLeft + nPaddingLeft + nLogoHeight),
                        Size(nSCWidth - nShadowLeft - nShadowRight - nPaddingLeft - nPaddingRight,
                             nSCHeight - nShadowTop - nShadowBottom - nPaddingTop - nPaddingBottom - nLogoHeight));
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
    if( pButton == mpWriterButton )
        dispatchURL( WRITER_URL );
    else if( pButton == mpCalcButton )
        dispatchURL( CALC_URL );
    else if( pButton == mpImpressButton )
        dispatchURL( IMPRESS_WIZARD_URL );
    else if( pButton == mpDrawButton )
        dispatchURL( DRAW_URL );
    else if( pButton == mpDBButton )
        dispatchURL( BASE_URL );
    else if( pButton == mpMathButton )
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

/*
IMPL_LINK( BackingWindow, ActivateHdl, Button*, pButton )
{
    if( pButton == mpOpenButton )
        prepareRecentFileMenu();
    return 0;
}
*/

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
