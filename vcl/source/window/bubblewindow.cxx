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

#include <rtl/ustrbuf.hxx>
#include <vcl/bubblewindow.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <unotools/resmgr.hxx>
#include <bitmaps.hlst>

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
    FloatingWindow::Resize();

    Size aSize = GetSizePixel();

    if ( ( aSize.Height() < 20 ) || ( aSize.Width() < 60 ) )
        return;

    tools::Rectangle aRect( 0, TIP_HEIGHT, aSize.Width(), aSize.Height() - TIP_HEIGHT );
    maRectPoly = tools::Polygon( aRect, 6, 6 );
    vcl::Region aRegion( maRectPoly );
    tools::Long nTipOffset = aSize.Width() - TIP_RIGHT_OFFSET + mnTipOffset;

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
    LineInfo aThickLine( LineStyle::Solid, 2 );

    DrawPolyLine( maRectPoly, aThickLine );
    DrawPolyLine( maTriPoly );

    Color aOldLine = GetLineColor();
    Size aSize = GetSizePixel();
    tools::Long nTipOffset = aSize.Width() - TIP_RIGHT_OFFSET + mnTipOffset;

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

void BubbleWindow::Show( bool bVisible )
{
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

    FloatingWindow::Show( bVisible, ShowFlags::NoActivate );
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

MenuBarUpdateIconManager::MenuBarUpdateIconManager()
    : mnIconID (0)
    , mbShowMenuIcon(false)
    , mbShowBubble(false)
    , mbBubbleChanged( false )
{
    maTimeoutTimer.SetTimeout( 10000 );
    maTimeoutTimer.SetInvokeHandler(LINK(this, MenuBarUpdateIconManager, TimeOutHdl));

    maWaitIdle.SetPriority( TaskPriority::LOWEST );
    maWaitIdle.SetInvokeHandler(LINK(this, MenuBarUpdateIconManager, WaitTimeOutHdl));

    maApplicationEventHdl = LINK(this, MenuBarUpdateIconManager, ApplicationEventHdl);
    Application::AddEventListener( maApplicationEventHdl );

    maWindowEventHdl = LINK(this, MenuBarUpdateIconManager, WindowEventHdl);
}

VclPtr<BubbleWindow> MenuBarUpdateIconManager::GetBubbleWindow()
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

IMPL_LINK_NOARG(MenuBarUpdateIconManager, TimeOutHdl, Timer *, void)
{
    RemoveBubbleWindow( false );
}

IMPL_LINK(MenuBarUpdateIconManager, WindowEventHdl, VclWindowEvent&, rEvent, void)
{
    VclEventId nEventID = rEvent.GetId();

    if ( VclEventId::ObjectDying == nEventID )
    {
        if ( mpIconSysWin == rEvent.GetWindow() )
        {
            mpIconSysWin->RemoveEventListener( maWindowEventHdl );
            RemoveBubbleWindow( true );
        }
    }
    else if ( VclEventId::WindowMenubarAdded == nEventID )
    {
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
        MenuBar *pMBar = static_cast<MenuBar*>(rEvent.GetData());
        if ( pMBar && ( pMBar == mpIconMBar ) )
            RemoveBubbleWindow( true );
    }
    else if ( ( nEventID == VclEventId::WindowMove ) ||
              ( nEventID == VclEventId::WindowResize ) )
    {
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

IMPL_LINK(MenuBarUpdateIconManager, ApplicationEventHdl, VclSimpleEvent&, rEvent, void)
{
    switch (rEvent.GetId())
    {
        case VclEventId::WindowShow:
        case VclEventId::WindowActivate:
        case VclEventId::WindowGetFocus: {

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

IMPL_LINK_NOARG(MenuBarUpdateIconManager, UserEventHdl, void*, void)
{
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

IMPL_LINK_NOARG(MenuBarUpdateIconManager, ClickHdl, MenuBar::MenuBarButtonCallbackArg&, bool)
{
    maWaitIdle.Stop();
    if ( mpBubbleWin )
        mpBubbleWin->Show( false );

    maClickHdl.Call(nullptr);

    return false;
}

IMPL_LINK(MenuBarUpdateIconManager, HighlightHdl, MenuBar::MenuBarButtonCallbackArg&, rData, bool)
{
    if ( rData.bHighlight )
        maWaitIdle.Start();
    else
        RemoveBubbleWindow(false);

    return false;
}

IMPL_LINK_NOARG(MenuBarUpdateIconManager, WaitTimeOutHdl, Timer *, void)
{
    mpBubbleWin = GetBubbleWindow();

    if ( mpBubbleWin )
    {
        mpBubbleWin->Show();
    }
}

MenuBarUpdateIconManager::~MenuBarUpdateIconManager()
{
    Application::RemoveEventListener( maApplicationEventHdl );

    RemoveBubbleWindow(true);
}

void MenuBarUpdateIconManager::SetShowMenuIcon(bool bShowMenuIcon)
{
    if ( bShowMenuIcon != mbShowMenuIcon )
    {
        mbShowMenuIcon = bShowMenuIcon;
        if ( bShowMenuIcon )
            Application::PostUserEvent(LINK(this, MenuBarUpdateIconManager, UserEventHdl));
        else
            RemoveBubbleWindow( true );
    }
}

void MenuBarUpdateIconManager::SetShowBubble(bool bShowBubble)
{
    mbShowBubble = bShowBubble;
    if ( mbShowBubble )
        Application::PostUserEvent(LINK(this, MenuBarUpdateIconManager, UserEventHdl));
    else if ( mpBubbleWin )
        mpBubbleWin->Show( false );
}

void MenuBarUpdateIconManager::SetBubbleChanged()
{
    mbBubbleChanged = true;
    if (mbBubbleChanged && mpBubbleWin)
        mpBubbleWin->Show( false );
}

void MenuBarUpdateIconManager::SetBubbleImage(const Image& rImage)
{
    maBubbleImage = rImage;
    SetBubbleChanged();
}

void MenuBarUpdateIconManager::SetBubbleTitle(const OUString& rTitle)
{
    if (rTitle != maBubbleTitle)
    {
        maBubbleTitle = rTitle;
        SetBubbleChanged();
    }
}

void MenuBarUpdateIconManager::SetBubbleText(const OUString& rText)
{
    if (rText != maBubbleText)
    {
        maBubbleText = rText;
        SetBubbleChanged();
    }
}

namespace {
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
}

void MenuBarUpdateIconManager::AddMenuBarIcon(SystemWindow *pSysWin, bool bAddEventHdl)
{
    if ( ! mbShowMenuIcon )
        return;

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
                                    LINK( this, MenuBarUpdateIconManager, ClickHdl ),
                                    aBuf.makeStringAndClear()
                                    );
            pActiveMBar->SetMenuBarButtonHighlightHdl( mnIconID,
                                    LINK( this, MenuBarUpdateIconManager, HighlightHdl ) );
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

void MenuBarUpdateIconManager::RemoveBubbleWindow( bool bRemoveIcon )
{
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
