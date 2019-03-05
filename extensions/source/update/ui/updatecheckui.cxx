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


#include <list>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/window.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/menu.hxx>
#include <vcl/outdev.hxx>
#include <vcl/weld.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/button.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/strings.hrc>
#include <rtl/ustrbuf.hxx>

#include <bitmaps.hlst>

#define PROPERTY_TITLE          "BubbleHeading"
#define PROPERTY_TEXT           "BubbleText"
#define PROPERTY_IMAGE          "BubbleImageURL"
#define PROPERTY_SHOW_BUBBLE    "BubbleVisible"
#define PROPERTY_CLICK_HDL      "MenuClickHDL"
#define PROPERTY_SHOW_MENUICON  "MenuIconVisible"

using namespace ::com::sun::star;


static uno::Sequence< OUString > getServiceNames()
{
    uno::Sequence< OUString > aServiceList { "com.sun.star.setup.UpdateCheckUI" };
    return aServiceList;
}


static OUString getImplementationName()
{
    return OUString("vnd.sun.UpdateCheckUI");
}


namespace
{

Image GetMenuBarIcon( MenuBar const * pMBar )
{
    OUString sResID;
    vcl::Window *pMBarWin = pMBar->GetWindow();
    sal_uInt32 nMBarHeight = 20;

    if ( pMBarWin )
        nMBarHeight = pMBarWin->GetOutputSizePixel().getHeight();

    if (nMBarHeight >= 35)
        sResID = RID_UPDATE_AVAILABLE_26;
    else
        sResID = RID_UPDATE_AVAILABLE_16;

    return Image(StockImage::Yes, sResID);
}

class BubbleWindow : public FloatingWindow
{
    Point           maTipPos;
    vcl::Region     maBounds;
    tools::Polygon  maRectPoly;
    tools::Polygon  maTriPoly;
    OUString        maBubbleTitle;
    OUString        maBubbleText;
    Image           maBubbleImage;
    Size            maMaxTextSize;
    tools::Rectangle       maTitleRect;
    tools::Rectangle       maTextRect;
    long            mnTipOffset;

private:
    void            RecalcTextRects();

public:
                    BubbleWindow( vcl::Window* pParent, const OUString& rTitle,
                                  const OUString& rText, const Image& rImage );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    void            Resize() override;
    void            Show( bool bVisible = true, ShowFlags nFlags = ShowFlags::NoActivate );
    void            SetTipPosPixel( const Point& rTipPos ) { maTipPos = rTipPos; }
    void            SetTitleAndText( const OUString& rTitle, const OUString& rText,
                                     const Image& rImage );
};


class UpdateCheckUI : public ::cppu::WeakImplHelper
                        < lang::XServiceInfo, document::XDocumentEventListener, beans::XPropertySet >
{
    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< task::XJob > mrJob;
    OUString       maBubbleTitle;
    OUString       maBubbleText;
    OUString       maBubbleImageURL;
    Image               maBubbleImage;
    VclPtr<BubbleWindow> mpBubbleWin;
    VclPtr<SystemWindow> mpIconSysWin;
    VclPtr<MenuBar>     mpIconMBar;
    std::locale         maSfxLocale;
    Idle                maWaitIdle;
    Timer               maTimeoutTimer;
    Link<VclWindowEvent&,void> maWindowEventHdl;
    Link<VclSimpleEvent&,void> maApplicationEventHdl;
    bool                mbShowBubble;
    bool                mbShowMenuIcon;
    bool                mbBubbleChanged;
    sal_uInt16              mnIconID;

private:
                    DECL_LINK(ClickHdl, MenuBar::MenuBarButtonCallbackArg&, bool);
                    DECL_LINK(HighlightHdl, MenuBar::MenuBarButtonCallbackArg&, bool);
                    DECL_LINK(WaitTimeOutHdl, Timer *, void);
                    DECL_LINK(TimeOutHdl, Timer *, void);
                    DECL_LINK(UserEventHdl, void *, void);
                    DECL_LINK(WindowEventHdl, VclWindowEvent&, void);
                    DECL_LINK(ApplicationEventHdl, VclSimpleEvent&, void);

    VclPtr<BubbleWindow> GetBubbleWindow();
    void            RemoveBubbleWindow( bool bRemoveIcon );
    void            AddMenuBarIcon( SystemWindow* pSysWin, bool bAddEventHdl );
    Image           GetBubbleImage( OUString const &rURL );

public:
    explicit        UpdateCheckUI(const uno::Reference<uno::XComponentContext>&);
    virtual        ~UpdateCheckUI() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(OUString const & serviceName) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XDocumentEventListener
    virtual void SAL_CALL documentEventOccured(const document::DocumentEvent& Event) override;
    virtual void SAL_CALL disposing(const lang::EventObject& Event) override;

    //XPropertySet
    virtual uno::Reference< beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString& PropertyName, const uno::Any& aValue) override;
    virtual uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString& PropertyName,
                                                    const uno::Reference< beans::XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString& PropertyName,
                                                       const uno::Reference< beans::XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName,
                                                    const uno::Reference< beans::XVetoableChangeListener > & aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName,
                                                       const uno::Reference< beans::XVetoableChangeListener > & aListener) override;
};

UpdateCheckUI::UpdateCheckUI(const uno::Reference<uno::XComponentContext>& xContext) :
      m_xContext(xContext)
    , mpIconMBar( nullptr )
    , mbShowBubble( false )
    , mbShowMenuIcon( false )
    , mbBubbleChanged( false )
    , mnIconID( 0 )
{
    maSfxLocale = Translate::Create("sfx");

    maBubbleImage = GetBubbleImage( maBubbleImageURL );

    maWaitIdle.SetPriority( TaskPriority::LOWEST );
    maWaitIdle.SetInvokeHandler( LINK( this, UpdateCheckUI, WaitTimeOutHdl ) );

    maTimeoutTimer.SetTimeout( 10000 );
    maTimeoutTimer.SetInvokeHandler( LINK( this, UpdateCheckUI, TimeOutHdl ) );

    uno::Reference< document::XDocumentEventBroadcaster > xBroadcaster( frame::theGlobalEventBroadcaster::get(m_xContext) );
    xBroadcaster->addDocumentEventListener( this );

    maWindowEventHdl = LINK( this, UpdateCheckUI, WindowEventHdl );
    maApplicationEventHdl = LINK( this, UpdateCheckUI, ApplicationEventHdl );
    Application::AddEventListener( maApplicationEventHdl );
}

UpdateCheckUI::~UpdateCheckUI()
{
    Application::RemoveEventListener( maApplicationEventHdl );
    RemoveBubbleWindow( true );
}

OUString SAL_CALL
UpdateCheckUI::getImplementationName()
{
    return ::getImplementationName();
}

uno::Sequence< OUString > SAL_CALL
UpdateCheckUI::getSupportedServiceNames()
{
    return ::getServiceNames();
}

sal_Bool SAL_CALL
UpdateCheckUI::supportsService( OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

Image UpdateCheckUI::GetBubbleImage( OUString const &rURL )
{
    Image aImage;

    if ( !maBubbleImageURL.isEmpty() )
    {
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

        if( !xContext.is() )
            throw uno::RuntimeException(
                "UpdateCheckUI: unable to obtain service manager from component context" );

        try
        {
            uno::Reference< graphic::XGraphicProvider > xGraphProvider(graphic::GraphicProvider::create(xContext));
            uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
            aMediaProps[0].Name = "URL";
            aMediaProps[0].Value <<= rURL;

            uno::Reference< graphic::XGraphic > xGraphic = xGraphProvider->queryGraphic( aMediaProps );
            if ( xGraphic.is() )
            {
                aImage = Image( xGraphic );
            }
        }
        catch( const uno::Exception& )
        {
        }
    }

    if ( aImage.GetSizePixel().Width() == 0 )
        aImage = Image(StockImage::Yes, SV_RESID_BITMAP_INFOBOX);

    return aImage;
}


void UpdateCheckUI::AddMenuBarIcon( SystemWindow *pSysWin, bool bAddEventHdl )
{
    if ( ! mbShowMenuIcon )
        return;

    SolarMutexGuard aGuard;

    MenuBar *pActiveMBar = pSysWin->GetMenuBar();
    if ( ( pSysWin != mpIconSysWin ) || ( pActiveMBar != mpIconMBar ) )
    {
        if ( bAddEventHdl && mpIconSysWin )
            mpIconSysWin->RemoveEventListener( maWindowEventHdl );

        RemoveBubbleWindow( true );

        if ( pActiveMBar )
        {
            OUStringBuffer aBuf;
            if( !maBubbleTitle.isEmpty() )
                aBuf.append( maBubbleTitle );
            if( !maBubbleText.isEmpty() )
            {
                if( !maBubbleTitle.isEmpty() )
                    aBuf.append( "\n\n" );
                aBuf.append( maBubbleText );
            }

            Image aImage = GetMenuBarIcon( pActiveMBar );
            mnIconID = pActiveMBar->AddMenuBarButton( aImage,
                                    LINK( this, UpdateCheckUI, ClickHdl ),
                                    aBuf.makeStringAndClear()
                                    );
            pActiveMBar->SetMenuBarButtonHighlightHdl( mnIconID,
                                    LINK( this, UpdateCheckUI, HighlightHdl ) );
        }
        mpIconMBar = pActiveMBar;
        mpIconSysWin = pSysWin;
        if ( bAddEventHdl && mpIconSysWin )
            mpIconSysWin->AddEventListener( maWindowEventHdl );
    }

    if ( mbShowBubble && pActiveMBar )
    {
        mpBubbleWin = GetBubbleWindow();
        if ( mpBubbleWin )
        {
            mpBubbleWin->Show();
            maTimeoutTimer.Start();
        }
        mbShowBubble = false;
    }
}


void SAL_CALL UpdateCheckUI::documentEventOccured(const document::DocumentEvent& rEvent)
{
    SolarMutexGuard aGuard;

    if( rEvent.EventName == "OnPrepareViewClosing" )
    {
        RemoveBubbleWindow( true );
    }
}


void SAL_CALL UpdateCheckUI::disposing(const lang::EventObject&)
{
}


uno::Reference< beans::XPropertySetInfo > UpdateCheckUI::getPropertySetInfo()
{
    return nullptr;
}


void UpdateCheckUI::setPropertyValue(const OUString& rPropertyName,
                                     const uno::Any& rValue)
{
    SolarMutexGuard aGuard;

    OUString aString;

    if( rPropertyName == PROPERTY_TITLE ) {
        rValue >>= aString;
        if ( aString != maBubbleTitle ) {
            maBubbleTitle = aString;
            mbBubbleChanged = true;
        }
    }
    else if( rPropertyName == PROPERTY_TEXT ) {
        rValue >>= aString;
        if ( aString != maBubbleText ) {
            maBubbleText = aString;
            mbBubbleChanged = true;
        }
    }
    else if( rPropertyName == PROPERTY_IMAGE ) {
        rValue >>= aString;
        if ( aString != maBubbleImageURL ) {
            maBubbleImageURL = aString;
            maBubbleImage = GetBubbleImage( maBubbleImageURL );
            mbBubbleChanged = true;
        }
    }
    else if( rPropertyName == PROPERTY_SHOW_BUBBLE ) {
        rValue >>= mbShowBubble;
        if ( mbShowBubble )
            Application::PostUserEvent( LINK( this, UpdateCheckUI, UserEventHdl ) );
        else if ( mpBubbleWin )
            mpBubbleWin->Show( false );
    }
    else if( rPropertyName == PROPERTY_CLICK_HDL ) {
        uno::Reference< task::XJob > aJob;
        rValue >>= aJob;
        if ( !aJob.is() )
            throw lang::IllegalArgumentException();
        mrJob = aJob;
    }
    else if (rPropertyName == PROPERTY_SHOW_MENUICON ) {
        bool bShowMenuIcon = false;
        rValue >>= bShowMenuIcon;
        if ( bShowMenuIcon != mbShowMenuIcon )
        {
            mbShowMenuIcon = bShowMenuIcon;
            if ( bShowMenuIcon )
                Application::PostUserEvent( LINK( this, UpdateCheckUI, UserEventHdl ) );
            else
                RemoveBubbleWindow( true );
        }
    }
    else
        throw beans::UnknownPropertyException();

    if ( mbBubbleChanged && mpBubbleWin )
        mpBubbleWin->Show( false );
}


uno::Any UpdateCheckUI::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;

    if( rPropertyName == PROPERTY_TITLE )
        aRet <<= maBubbleTitle;
    else if( rPropertyName == PROPERTY_TEXT )
        aRet <<= maBubbleText;
    else if( rPropertyName == PROPERTY_SHOW_BUBBLE )
        aRet <<= mbShowBubble;
    else if( rPropertyName == PROPERTY_IMAGE )
        aRet <<= maBubbleImageURL;
    else if( rPropertyName == PROPERTY_CLICK_HDL )
        aRet <<= mrJob;
    else if( rPropertyName == PROPERTY_SHOW_MENUICON )
        aRet <<= mbShowMenuIcon;
    else
        throw beans::UnknownPropertyException();

    return aRet;
}


void UpdateCheckUI::addPropertyChangeListener( const OUString& /*aPropertyName*/,
                                               const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
    //no bound properties
}


void UpdateCheckUI::removePropertyChangeListener( const OUString& /*aPropertyName*/,
                                                  const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
    //no bound properties
}


void UpdateCheckUI::addVetoableChangeListener( const OUString& /*aPropertyName*/,
                                               const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    //no vetoable properties
}


void UpdateCheckUI::removeVetoableChangeListener( const OUString& /*aPropertyName*/,
                                                  const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    //no vetoable properties
}


VclPtr<BubbleWindow> UpdateCheckUI::GetBubbleWindow()
{
    if ( !mpIconSysWin )
        return nullptr;

    tools::Rectangle aIconRect = mpIconMBar->GetMenuBarButtonRectPixel( mnIconID );
    if( aIconRect.IsEmpty() )
        return nullptr;

    auto pBubbleWin = mpBubbleWin;

    if ( !pBubbleWin ) {
        pBubbleWin = VclPtr<BubbleWindow>::Create( mpIconSysWin, maBubbleTitle,
                                       maBubbleText, maBubbleImage );
        mbBubbleChanged = false;
    }
    else if ( mbBubbleChanged ) {
        pBubbleWin->SetTitleAndText( maBubbleTitle, maBubbleText,
                                     maBubbleImage );
        mbBubbleChanged = false;
    }

    Point aWinPos = aIconRect.BottomCenter();

    pBubbleWin->SetTipPosPixel( aWinPos );

    return pBubbleWin;
}


void UpdateCheckUI::RemoveBubbleWindow( bool bRemoveIcon )
{
    SolarMutexGuard aGuard;

    maWaitIdle.Stop();
    maTimeoutTimer.Stop();

    if ( mpBubbleWin )
    {
        mpBubbleWin.disposeAndClear();
    }

    if ( bRemoveIcon )
    {
        try {
            if ( mpIconMBar && ( mnIconID != 0 ) )
            {
                mpIconMBar->RemoveMenuBarButton( mnIconID );
                mpIconMBar = nullptr;
                mnIconID = 0;
            }
        }
        catch ( ... ) {
            mpIconMBar = nullptr;
            mnIconID = 0;
        }

        mpIconSysWin = nullptr;
    }
}


IMPL_LINK_NOARG(UpdateCheckUI, ClickHdl, MenuBar::MenuBarButtonCallbackArg&, bool)
{
    SolarMutexGuard aGuard;

    maWaitIdle.Stop();
    if ( mpBubbleWin )
        mpBubbleWin->Show( false );

    if ( mrJob.is() )
    {
        try {
            uno::Sequence<beans::NamedValue> aEmpty;
            mrJob->execute( aEmpty );
        }
        catch(const uno::Exception&) {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(nullptr,
                                                           VclMessageType::Warning, VclButtonsType::Ok,
                                                           Translate::get(STR_NO_WEBBROWSER_FOUND, maSfxLocale)));
            xErrorBox->run();
        }
    }

    return false;
}


IMPL_LINK( UpdateCheckUI, HighlightHdl, MenuBar::MenuBarButtonCallbackArg&, rData, bool )
{
    if ( rData.bHighlight )
        maWaitIdle.Start();
    else
        RemoveBubbleWindow( false );

    return false;
}


IMPL_LINK_NOARG(UpdateCheckUI, WaitTimeOutHdl, Timer *, void)
{
    SolarMutexGuard aGuard;

    mpBubbleWin = GetBubbleWindow();

    if ( mpBubbleWin )
    {
        mpBubbleWin->Show();
    }
}


IMPL_LINK_NOARG(UpdateCheckUI, TimeOutHdl, Timer *, void)
{
    RemoveBubbleWindow( false );
}


IMPL_LINK_NOARG(UpdateCheckUI, UserEventHdl, void*, void)
{
    SolarMutexGuard aGuard;

    vcl::Window *pTopWin = Application::GetFirstTopLevelWindow();
    vcl::Window *pActiveWin = Application::GetActiveTopWindow();
    SystemWindow *pActiveSysWin = nullptr;

    vcl::Window *pBubbleWin = nullptr;
    if ( mpBubbleWin )
        pBubbleWin = mpBubbleWin;

    if ( pActiveWin && ( pActiveWin != pBubbleWin ) && pActiveWin->IsTopWindow() )
        pActiveSysWin = pActiveWin->GetSystemWindow();

    if ( pActiveWin == pBubbleWin )
        pActiveSysWin = nullptr;

    while ( !pActiveSysWin && pTopWin )
    {
        if ( ( pTopWin != pBubbleWin ) && pTopWin->IsTopWindow() )
            pActiveSysWin = pTopWin->GetSystemWindow();
        if ( !pActiveSysWin )
            pTopWin = Application::GetNextTopLevelWindow( pTopWin );
    }

    if ( pActiveSysWin )
        AddMenuBarIcon( pActiveSysWin, true );
}


IMPL_LINK( UpdateCheckUI, WindowEventHdl, VclWindowEvent&, rEvent, void )
{
    VclEventId nEventID = rEvent.GetId();

    if ( VclEventId::ObjectDying == nEventID )
    {
        SolarMutexGuard aGuard;
        if ( mpIconSysWin == rEvent.GetWindow() )
        {
            mpIconSysWin->RemoveEventListener( maWindowEventHdl );
            RemoveBubbleWindow( true );
        }
    }
    else if ( VclEventId::WindowMenubarAdded == nEventID )
    {
        SolarMutexGuard aGuard;
        vcl::Window *pWindow = rEvent.GetWindow();
        if ( pWindow )
        {
            SystemWindow *pSysWin = pWindow->GetSystemWindow();
            if ( pSysWin )
            {
                AddMenuBarIcon( pSysWin, false );
            }
        }
    }
    else if ( VclEventId::WindowMenubarRemoved == nEventID )
    {
        SolarMutexGuard aGuard;
        MenuBar *pMBar = static_cast<MenuBar*>(rEvent.GetData());
        if ( pMBar && ( pMBar == mpIconMBar ) )
            RemoveBubbleWindow( true );
    }
    else if ( ( nEventID == VclEventId::WindowMove ) ||
              ( nEventID == VclEventId::WindowResize ) )
    {
        SolarMutexGuard aGuard;
        if ( ( mpIconSysWin == rEvent.GetWindow() ) &&
             mpBubbleWin && ( mpIconMBar != nullptr ) )
        {
            tools::Rectangle aIconRect = mpIconMBar->GetMenuBarButtonRectPixel( mnIconID );
            Point aWinPos = aIconRect.BottomCenter();
            mpBubbleWin->SetTipPosPixel( aWinPos );
            if ( mpBubbleWin->IsVisible() )
                mpBubbleWin->Show();    // This will recalc the screen position of the bubble
        }
    }
}


IMPL_LINK( UpdateCheckUI, ApplicationEventHdl, VclSimpleEvent&, rEvent, void)
{
    switch (rEvent.GetId())
    {
        case VclEventId::WindowShow:
        case VclEventId::WindowActivate:
        case VclEventId::WindowGetFocus: {
            SolarMutexGuard aGuard;

            vcl::Window *pWindow = static_cast< VclWindowEvent * >(&rEvent)->GetWindow();
            if ( pWindow && pWindow->IsTopWindow() )
            {
                SystemWindow *pSysWin = pWindow->GetSystemWindow();
                MenuBar *pMBar = pSysWin ? pSysWin->GetMenuBar() : nullptr;
                if (pMBar)
                {
                    AddMenuBarIcon( pSysWin, true );
                }
            }
            break;
        }
        default: break;
    }
}


#define TIP_HEIGHT             15
#define TIP_WIDTH               7
#define TIP_RIGHT_OFFSET       18
#define BUBBLE_BORDER          10
#define TEXT_MAX_WIDTH        300
#define TEXT_MAX_HEIGHT       200


BubbleWindow::BubbleWindow( vcl::Window* pParent, const OUString& rTitle,
                            const OUString& rText, const Image& rImage )
    : FloatingWindow( pParent, WB_SYSTEMWINDOW
                               | WB_OWNERDRAWDECORATION
                               | WB_NOBORDER
                    )
    , maBubbleTitle( rTitle )
    , maBubbleText( rText )
    , maBubbleImage( rImage )
    , maMaxTextSize( TEXT_MAX_WIDTH, TEXT_MAX_HEIGHT )
    , mnTipOffset( 0 )
{
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetHelpColor() ) );
}

void BubbleWindow::Resize()
{
    SolarMutexGuard aGuard;

    FloatingWindow::Resize();

    Size aSize = GetSizePixel();

    if ( ( aSize.Height() < 20 ) || ( aSize.Width() < 60 ) )
        return;

    tools::Rectangle aRect( 0, TIP_HEIGHT, aSize.Width(), aSize.Height() - TIP_HEIGHT );
    maRectPoly = tools::Polygon( aRect, 6, 6 );
    vcl::Region aRegion( maRectPoly );
    long nTipOffset = aSize.Width() - TIP_RIGHT_OFFSET + mnTipOffset;

    Point aPointArr[4];
    aPointArr[0] = Point( nTipOffset, TIP_HEIGHT );
    aPointArr[1] = Point( nTipOffset, 0 );
    aPointArr[2] = Point( nTipOffset + TIP_WIDTH , TIP_HEIGHT );
    aPointArr[3] = Point( nTipOffset, TIP_HEIGHT );
    maTriPoly = tools::Polygon( 4, aPointArr );
    vcl::Region aTriRegion( maTriPoly );

    aRegion.Union( aTriRegion);
    maBounds = aRegion;

    SetWindowRegionPixel( maBounds );
}


void BubbleWindow::SetTitleAndText( const OUString& rTitle,
                                    const OUString& rText,
                                    const Image& rImage )
{
    maBubbleTitle = rTitle;
    maBubbleText = rText;
    maBubbleImage = rImage;

    Resize();
}


void BubbleWindow::Paint(vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& /*rRect*/)
{
    SolarMutexGuard aGuard;

    LineInfo aThickLine( LineStyle::Solid, 2 );

    DrawPolyLine( maRectPoly, aThickLine );
    DrawPolyLine( maTriPoly );

    Color aOldLine = GetLineColor();
    Size aSize = GetSizePixel();
    long nTipOffset = aSize.Width() - TIP_RIGHT_OFFSET + mnTipOffset;

    SetLineColor( GetSettings().GetStyleSettings().GetHelpColor() );
    DrawLine( Point( nTipOffset+2, TIP_HEIGHT ),
              Point( nTipOffset + TIP_WIDTH -1 , TIP_HEIGHT ),
              aThickLine );
    SetLineColor( aOldLine );

    Size aImgSize = maBubbleImage.GetSizePixel();

    DrawImage( Point( BUBBLE_BORDER, BUBBLE_BORDER + TIP_HEIGHT ), maBubbleImage );

    vcl::Font aOldFont = GetFont();
    vcl::Font aBoldFont = aOldFont;
    aBoldFont.SetWeight( WEIGHT_BOLD );

    SetFont( aBoldFont );
    tools::Rectangle aTitleRect = maTitleRect;
    aTitleRect.Move( aImgSize.Width(), 0 );
    DrawText( aTitleRect, maBubbleTitle, DrawTextFlags::MultiLine | DrawTextFlags::WordBreak );

    SetFont( aOldFont );
    tools::Rectangle aTextRect = maTextRect;
    aTextRect.Move( aImgSize.Width(), 0 );
    DrawText( aTextRect, maBubbleText, DrawTextFlags::MultiLine | DrawTextFlags::WordBreak );
}


void BubbleWindow::MouseButtonDown( const MouseEvent& )
{
    Show( false );
}


void BubbleWindow::Show( bool bVisible, ShowFlags nFlags )
{
    SolarMutexGuard aGuard;

    if ( !bVisible )
    {
        FloatingWindow::Show( bVisible );
        return;
    }

    // don't show bubbles without a text
    if ( ( maBubbleTitle.isEmpty() ) && ( maBubbleText.isEmpty() ) )
        return;

    Size aWindowSize = GetSizePixel();

    Size aImgSize = maBubbleImage.GetSizePixel();

    RecalcTextRects();

    aWindowSize.setHeight( maTitleRect.GetHeight() * 7 / 4+ maTextRect.GetHeight() +
                           3 * BUBBLE_BORDER + TIP_HEIGHT );

    if ( maTitleRect.GetWidth() > maTextRect.GetWidth() )
        aWindowSize.setWidth( maTitleRect.GetWidth() );
    else
        aWindowSize.setWidth( maTextRect.GetWidth() );

    aWindowSize.setWidth( aWindowSize.Width() + 3 * BUBBLE_BORDER + aImgSize.Width() );

    if ( aWindowSize.Height() < aImgSize.Height() + TIP_HEIGHT + 2 * BUBBLE_BORDER )
        aWindowSize.setHeight( aImgSize.Height() + TIP_HEIGHT + 2 * BUBBLE_BORDER );

    Point aPos;
    aPos.setX( maTipPos.X() - aWindowSize.Width() + TIP_RIGHT_OFFSET );
    aPos.setY( maTipPos.Y() );
    Point aScreenPos = GetParent()->OutputToAbsoluteScreenPixel( aPos );
    if ( aScreenPos.X() < 0 )
    {
        mnTipOffset = aScreenPos.X();
        aPos.AdjustX( -mnTipOffset );
    }
    SetPosSizePixel( aPos, aWindowSize );

    FloatingWindow::Show( bVisible, nFlags );
}


void BubbleWindow::RecalcTextRects()
{
    Size aTotalSize;
    bool bFinished = false;
    vcl::Font aOldFont = GetFont();
    vcl::Font aBoldFont = aOldFont;

    aBoldFont.SetWeight( WEIGHT_BOLD );

    while ( !bFinished )
    {
        SetFont( aBoldFont );

        maTitleRect = GetTextRect( tools::Rectangle( Point( 0, 0 ), maMaxTextSize ),
                                   maBubbleTitle,
                                   DrawTextFlags::MultiLine | DrawTextFlags::WordBreak );

        SetFont( aOldFont );
        maTextRect = GetTextRect( tools::Rectangle( Point( 0, 0 ), maMaxTextSize ),
                                  maBubbleText,
                                  DrawTextFlags::MultiLine | DrawTextFlags::WordBreak );

        if ( maTextRect.GetHeight() < 10 )
            maTextRect.setHeight( 10 );

        aTotalSize.setHeight( maTitleRect.GetHeight() +
                              aBoldFont.GetFontHeight() * 3 / 4 +
                              maTextRect.GetHeight() +
                              3 * BUBBLE_BORDER + TIP_HEIGHT );
        if ( aTotalSize.Height() > maMaxTextSize.Height() )
        {
            maMaxTextSize.setWidth( maMaxTextSize.Width() * 3 / 2 );
            maMaxTextSize.setHeight( maMaxTextSize.Height() * 3 / 2 );
        }
        else
            bFinished = true;
    }
    maTitleRect.Move( 2*BUBBLE_BORDER, BUBBLE_BORDER + TIP_HEIGHT );
    maTextRect.Move( 2*BUBBLE_BORDER, BUBBLE_BORDER + TIP_HEIGHT + maTitleRect.GetHeight() + aBoldFont.GetFontHeight() * 3 / 4 );
}


} // anonymous namespace


static uno::Reference<uno::XInterface>
createInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    SolarMutexGuard aGuard;
    return  *new UpdateCheckUI(xContext);
}


static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createInstance,
        getImplementationName,
        getServiceNames,
        cppu::createSingleComponentFactory,
        nullptr,
        0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
} ;


extern "C" SAL_DLLPUBLIC_EXPORT void * updchkui_component_getFactory(const sal_Char *pszImplementationName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pszImplementationName,
        pServiceManager,
        pRegistryKey,
        kImplementations_entries) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
