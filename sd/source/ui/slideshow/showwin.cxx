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

#include <com/sun/star/awt/Key.hpp>

#include "showwindow.hxx"
#include "slideshowimpl.hxx"

#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <sfx2/viewfrm.hxx>


#include <slideshow.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <sdresid.hxx>
#include <helpids.h>
#include <strings.hrc>

#include <sal/log.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>

using namespace ::com::sun::star;

namespace sd {

static const sal_uInt64 HIDE_MOUSE_TIMEOUT = 10000;
static const sal_uInt64 SHOW_MOUSE_TIMEOUT = 1000;

ShowWindow::ShowWindow( const ::rtl::Reference< SlideshowImpl >& xController, vcl::Window* pParent )
: ::sd::Window( pParent )
, mnPauseTimeout( SLIDE_NO_TIMEOUT )
, mnRestartPageIndex( PAGE_NO_END )
, meShowWindowMode(SHOWWINDOWMODE_NORMAL)
, mbShowNavigatorAfterSpecialMode( false )
, mbMouseAutoHide(true)
, mbMouseCursorHidden(false)
, mnFirstMouseMove(0)
, mxController( xController )
{
    SetOutDevViewType( OutDevViewType::SlideShow );

    // Do never mirror the preview window.  This explicitly includes right
    // to left writing environments.
    EnableRTL (false);

    MapMode aMap(GetMapMode());
    aMap.SetMapUnit(MapUnit::Map100thMM);
    SetMapMode(aMap);

    // set HelpId
    SetHelpId( HID_SD_WIN_PRESENTATION );

    maPauseTimer.SetInvokeHandler( LINK( this, ShowWindow, PauseTimeoutHdl ) );
    maPauseTimer.SetTimeout( 1000 );
    maMouseTimer.SetInvokeHandler( LINK( this, ShowWindow, MouseTimeoutHdl ) );
    maMouseTimer.SetTimeout( HIDE_MOUSE_TIMEOUT );

    maShowBackground = Wallpaper( COL_BLACK );
    SetBackground(); // avoids that VCL paints any background!
    GetParent()->Show();
    AddEventListener( LINK( this, ShowWindow, EventHdl ) );
}

ShowWindow::~ShowWindow()
{
    disposeOnce();
}

void ShowWindow::dispose()
{
    maPauseTimer.Stop();
    maMouseTimer.Stop();
    ::sd::Window::dispose();
}

void ShowWindow::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

    if( SHOWWINDOWMODE_PREVIEW == meShowWindowMode )
    {
        TerminateShow();
        bReturn = true;
    }
    else if( SHOWWINDOWMODE_END == meShowWindowMode )
    {
        const int nKeyCode = rKEvt.GetKeyCode().GetCode();
        switch( nKeyCode )
        {
        case KEY_PAGEUP:
        case KEY_LEFT:
        case KEY_UP:
        case KEY_P:
        case KEY_HOME:
        case KEY_END:
        case awt::Key::CONTEXTMENU:
            // these keys will be handled by the slide show even
            // while in end mode
            break;
        default:
            TerminateShow();
            bReturn = true;
        }
    }
    else if( SHOWWINDOWMODE_BLANK == meShowWindowMode )
    {
        RestartShow();
        bReturn = true;
    }
    else if( SHOWWINDOWMODE_PAUSE == meShowWindowMode )
    {
        const int nKeyCode = rKEvt.GetKeyCode().GetCode();
        switch( nKeyCode )
        {
        case KEY_ESCAPE:
            TerminateShow();
            bReturn = true;
            break;
        case KEY_PAGEUP:
        case KEY_RIGHT:
        case KEY_UP:
        case KEY_P:
        case KEY_HOME:
        case KEY_END:
        case awt::Key::CONTEXTMENU:
            // these keys will be handled by the slide show even
            // while in end mode
            break;
        default:
            RestartShow();
            bReturn = true;
            break;
        }
    }

    if( !bReturn )
    {
        if( mxController.is() )
            bReturn = mxController->keyInput(rKEvt);

        if( !bReturn )
        {
            if( mpViewShell )
            {
                mpViewShell->KeyInput(rKEvt,this);
            }
            else
            {
                Window::KeyInput(rKEvt);
            }
        }
    }

    if( mpViewShell )
        mpViewShell->SetActiveWindow( this );
}

void ShowWindow::MouseButtonDown(const MouseEvent& /*rMEvt*/)
{
    if( SHOWWINDOWMODE_PREVIEW == meShowWindowMode )
    {
        TerminateShow();
    }
    else if( mpViewShell )
    {
        mpViewShell->SetActiveWindow( this );
    }
}

void ShowWindow::MouseMove(const MouseEvent& /*rMEvt*/)
{
    if( mbMouseAutoHide )
    {
        if( mbMouseCursorHidden )
        {
            if( mnFirstMouseMove )
            {
                // if this is not the first mouse move while hidden, see if
                // enough time has pasted to show mouse pointer again
                sal_uInt64 nTime = ::tools::Time::GetSystemTicks();
                if( (nTime - mnFirstMouseMove) >= SHOW_MOUSE_TIMEOUT )
                {
                    ShowPointer( true );
                    mnFirstMouseMove = 0;
                    mbMouseCursorHidden = false;
                    maMouseTimer.SetTimeout( HIDE_MOUSE_TIMEOUT );
                    maMouseTimer.Start();
                }
            }
            else
            {
                // if this is the first mouse move, note current
                // time and start idle timer to cancel show mouse pointer
                // again if not enough mouse movement is measured
                mnFirstMouseMove = ::tools::Time::GetSystemTicks();
                maMouseTimer.SetTimeout( 2*SHOW_MOUSE_TIMEOUT );
                maMouseTimer.Start();
            }
        }
        else
        {
            // current mousemove restarts the idle timer to hide the mouse
            maMouseTimer.Start();
        }
    }

    if( mpViewShell )
        mpViewShell->SetActiveWindow( this );
}

void ShowWindow::MouseButtonUp(const MouseEvent& rMEvt)
{
    if( SHOWWINDOWMODE_PREVIEW == meShowWindowMode )
    {
        TerminateShow();
    }
    else if( (SHOWWINDOWMODE_END == meShowWindowMode) && !rMEvt.IsRight() )
    {
        TerminateShow();
    }
    else if( (( SHOWWINDOWMODE_BLANK == meShowWindowMode ) || ( SHOWWINDOWMODE_PAUSE == meShowWindowMode ))
             && !rMEvt.IsRight() )
    {
        RestartShow();
    }
    else
    {
        if( mxController.is() )
            mxController->mouseButtonUp( rMEvt );
    }
}

/**
 * if FuSlideShow is still available, forward it
 */
void ShowWindow::Paint(vcl::RenderContext& /*rRenderContext*/, const ::tools::Rectangle& rRect)
{
    if( (meShowWindowMode == SHOWWINDOWMODE_NORMAL) || (meShowWindowMode == SHOWWINDOWMODE_PREVIEW) )
    {
        if( mxController.is() )
        {
            mxController->paint();
        }
        else if(mpViewShell )
        {
            mpViewShell->Paint(rRect, this);
        }
    }
    else
    {
        DrawWallpaper( rRect, maShowBackground );

        if( SHOWWINDOWMODE_END == meShowWindowMode )
        {
            DrawEndScene();
        }
        else if( SHOWWINDOWMODE_PAUSE == meShowWindowMode )
        {
            DrawPauseScene( false );
        }
        else if( SHOWWINDOWMODE_BLANK == meShowWindowMode )
        {
            // just blank through background color => nothing to be done here
        }
    }
}

void ShowWindow::LoseFocus()
{
    Window::LoseFocus();

    if( SHOWWINDOWMODE_PREVIEW == meShowWindowMode)
        TerminateShow();
}

void ShowWindow::SetEndMode()
{
    if( !(( SHOWWINDOWMODE_NORMAL == meShowWindowMode ) && mpViewShell && mpViewShell->GetView()) )
        return;

    DeleteWindowFromPaintView();
    meShowWindowMode = SHOWWINDOWMODE_END;
    maShowBackground = Wallpaper( COL_BLACK );

    // hide navigator if it is visible
    if( mpViewShell->GetViewFrame()->GetChildWindow( SID_NAVIGATOR ) )
    {
        mpViewShell->GetViewFrame()->ShowChildWindow( SID_NAVIGATOR, false );
        mbShowNavigatorAfterSpecialMode = true;
    }

    Invalidate();
}

bool ShowWindow::SetPauseMode( sal_Int32 nTimeout, Graphic const * pLogo )
{
    rtl::Reference< SlideShow > xSlideShow;

    if( mpViewShell )
        xSlideShow = SlideShow::GetSlideShow( mpViewShell->GetViewShellBase() );

    if( xSlideShow.is() && !nTimeout )
    {
        xSlideShow->jumpToPageIndex( 0 );
    }
    else if( ( SHOWWINDOWMODE_NORMAL == meShowWindowMode ) && mpViewShell && mpViewShell->GetView() )
    {
        DeleteWindowFromPaintView();
        mnPauseTimeout = nTimeout;
        mnRestartPageIndex = 0;
        meShowWindowMode = SHOWWINDOWMODE_PAUSE;
        maShowBackground = Wallpaper( COL_BLACK );

        // hide navigator if it is visible
        if( mpViewShell->GetViewFrame()->GetChildWindow( SID_NAVIGATOR ) )
        {
            mpViewShell->GetViewFrame()->ShowChildWindow( SID_NAVIGATOR, false );
            mbShowNavigatorAfterSpecialMode = true;
        }

        if( pLogo )
            maLogo = *pLogo;

        Invalidate();

        if( SLIDE_NO_TIMEOUT != mnPauseTimeout )
            maPauseTimer.Start();
    }

    return( SHOWWINDOWMODE_PAUSE == meShowWindowMode );
}

bool ShowWindow::SetBlankMode( sal_Int32 nPageIndexToRestart, const Color& rBlankColor )
{
    if( ( SHOWWINDOWMODE_NORMAL == meShowWindowMode ) && mpViewShell && mpViewShell->GetView() )
    {
        DeleteWindowFromPaintView();
        mnRestartPageIndex = nPageIndexToRestart;
        meShowWindowMode = SHOWWINDOWMODE_BLANK;
        maShowBackground = Wallpaper( rBlankColor );

        // hide navigator if it is visible
        if( mpViewShell->GetViewFrame()->GetChildWindow( SID_NAVIGATOR ) )
        {
            mpViewShell->GetViewFrame()->ShowChildWindow( SID_NAVIGATOR, false );
            mbShowNavigatorAfterSpecialMode = true;
        }

        Invalidate();
    }

    return( SHOWWINDOWMODE_BLANK == meShowWindowMode );
}

void ShowWindow::SetPreviewMode()
{
    meShowWindowMode = SHOWWINDOWMODE_PREVIEW;
}

void ShowWindow::TerminateShow()
{
    maLogo.Clear();
    maPauseTimer.Stop();
    maMouseTimer.Stop();
    Erase();
    maShowBackground = Wallpaper( COL_BLACK );
    meShowWindowMode = SHOWWINDOWMODE_NORMAL;
    mnPauseTimeout = SLIDE_NO_TIMEOUT;

    if( mpViewShell )
    {
        // show navigator?
        if( mbShowNavigatorAfterSpecialMode )
        {
            mpViewShell->GetViewFrame()->ShowChildWindow( SID_NAVIGATOR );
            mbShowNavigatorAfterSpecialMode = false;
        }
    }

    if( mxController.is() )
        mxController->endPresentation();

    mnRestartPageIndex = PAGE_NO_END;
}

void ShowWindow::RestartShow()
{
    RestartShow( mnRestartPageIndex );
}

void ShowWindow::RestartShow( sal_Int32 nPageIndexToRestart )

{
    ShowWindowMode eOldShowWindowMode = meShowWindowMode;

    maLogo.Clear();
    maPauseTimer.Stop();
    Erase();
    maShowBackground = Wallpaper( COL_BLACK );
    meShowWindowMode = SHOWWINDOWMODE_NORMAL;
    mnPauseTimeout = SLIDE_NO_TIMEOUT;

    if( mpViewShell )
    {
        rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( mpViewShell->GetViewShellBase() ) );

        if( xSlideShow.is() )
        {
            AddWindowToPaintView();

            if( SHOWWINDOWMODE_BLANK == eOldShowWindowMode )
            {
                xSlideShow->pause(false);
                Invalidate();
            }
            else
            {
                xSlideShow->jumpToPageIndex( nPageIndexToRestart );
            }
        }
    }

    mnRestartPageIndex = PAGE_NO_END;

    // show navigator?
    if( mbShowNavigatorAfterSpecialMode )
    {
        if (mpViewShell)
            mpViewShell->GetViewFrame()->ShowChildWindow( SID_NAVIGATOR );
        mbShowNavigatorAfterSpecialMode = false;
    }
}

void ShowWindow::DrawPauseScene( bool bTimeoutOnly )
{
    const MapMode&  rMap = GetMapMode();
    const Point     aOutOrg( PixelToLogic( Point() ) );
    const Size      aOutSize( GetOutputSize() );
    const Size      aTextSize(LogicToLogic(Size(0, 14), MapMode(MapUnit::MapPoint), rMap));
    const Size      aOffset(LogicToLogic(Size(1000, 1000), MapMode(MapUnit::Map100thMM), rMap));
    OUString        aText( SdResId( STR_PRES_PAUSE ) );
    bool            bDrawn = false;

    vcl::Font       aFont( GetSettings().GetStyleSettings().GetMenuFont() );
    const vcl::Font aOldFont( GetFont() );

    aFont.SetFontSize( aTextSize );
    aFont.SetColor( COL_WHITE );
    aFont.SetCharSet( aOldFont.GetCharSet() );
    aFont.SetLanguage( aOldFont.GetLanguage() );

    if( !bTimeoutOnly && ( maLogo.GetType() != GraphicType::NONE ) )
    {
        Size aGrfSize;

        if (maLogo.GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel)
            aGrfSize = PixelToLogic( maLogo.GetPrefSize() );
        else
            aGrfSize = LogicToLogic( maLogo.GetPrefSize(), maLogo.GetPrefMapMode(), rMap );

        const Point aGrfPos( std::max( aOutOrg.X() + aOutSize.Width() - aGrfSize.Width() - aOffset.Width(), aOutOrg.X() ),
                             std::max( aOutOrg.Y() + aOutSize.Height() - aGrfSize.Height() - aOffset.Height(), aOutOrg.Y() ) );

        if( maLogo.IsAnimated() )
            maLogo.StartAnimation( this, aGrfPos, aGrfSize, reinterpret_cast<sal_IntPtr>(this) );
        else
            maLogo.Draw( this, aGrfPos, aGrfSize );
    }

    if( SLIDE_NO_TIMEOUT != mnPauseTimeout )
    {
        MapMode         aVMap( rMap );
        ScopedVclPtrInstance< VirtualDevice > pVDev( *this );

        aVMap.SetOrigin( Point() );
        pVDev->SetMapMode( aVMap );
        pVDev->SetBackground( Wallpaper( COL_BLACK ) );

        // set font first, to determine real output height
        pVDev->SetFont( aFont );

        const Size aVDevSize( aOutSize.Width(), pVDev->GetTextHeight() );

        if( pVDev->SetOutputSize( aVDevSize ) )
        {
            // Note: if performance gets an issue here, we can use NumberFormatter directly
            SvtSysLocale                aSysLocale;
            const LocaleDataWrapper&    aLocaleData = aSysLocale.GetLocaleData();

            aText += " ( ";
            aText += aLocaleData.getDuration( ::tools::Time( 0, 0, mnPauseTimeout ) );
            aText += " )";
            pVDev->DrawText( Point( aOffset.Width(), 0 ), aText );
            DrawOutDev( Point( aOutOrg.X(), aOffset.Height() ), aVDevSize, Point(), aVDevSize, *pVDev );
            bDrawn = true;
        }
    }

    if( !bDrawn )
    {
        SetFont( aFont );
        DrawText( Point( aOutOrg.X() + aOffset.Width(), aOutOrg.Y() + aOffset.Height() ), aText );
        SetFont( aOldFont );
    }
}

void ShowWindow::DrawEndScene()
{
    const vcl::Font aOldFont( GetFont() );
    vcl::Font       aFont( GetSettings().GetStyleSettings().GetMenuFont() );

    const Point     aOutOrg( PixelToLogic( Point() ) );
    const Size      aTextSize(LogicToLogic(Size(0, 14), MapMode(MapUnit::MapPoint), GetMapMode()));
    const OUString  aText( SdResId( STR_PRES_SOFTEND ) );

    aFont.SetFontSize( aTextSize );
    aFont.SetColor( COL_WHITE );
    aFont.SetCharSet( aOldFont.GetCharSet() );
    aFont.SetLanguage( aOldFont.GetLanguage() );
    SetFont( aFont );
    DrawText( Point( aOutOrg.X() + aTextSize.Height(), aOutOrg.Y() + aTextSize.Height() ), aText );
    SetFont( aOldFont );
}

IMPL_LINK( ShowWindow, PauseTimeoutHdl, Timer*, pTimer, void )
{
    if( !( --mnPauseTimeout ) )
        RestartShow();
    else
    {
        DrawPauseScene( true );
        pTimer->Start();
    }
}

IMPL_LINK_NOARG(ShowWindow, MouseTimeoutHdl, Timer *, void)
{
    if( mbMouseCursorHidden )
    {
        // not enough mouse movements since first recording so
        // cancel show mouse pointer for now
        mnFirstMouseMove = 0;
    }
    else
    {
        // mouse has been idle to long, hide pointer
        ShowPointer( false );
        mbMouseCursorHidden = true;
    }
}

IMPL_LINK( ShowWindow, EventHdl, VclWindowEvent&, rEvent, void )
{
    if( mbMouseAutoHide )
    {
        if (rEvent.GetId() == VclEventId::WindowShow)
        {
            maMouseTimer.SetTimeout( HIDE_MOUSE_TIMEOUT );
            maMouseTimer.Start();
        }
    }
}

void ShowWindow::DeleteWindowFromPaintView()
{
    if( mpViewShell->GetView() )
        mpViewShell->GetView()->DeleteWindowFromPaintView( this );

    sal_uInt16 nChild = GetChildCount();
    while( nChild-- )
        GetChild( nChild )->Show( false );
}

void ShowWindow::AddWindowToPaintView()
{
    if( mpViewShell->GetView() )
        mpViewShell->GetView()->AddWindowToPaintView( this, nullptr );

    sal_uInt16 nChild = GetChildCount();
    while( nChild-- )
        GetChild( nChild )->Show();
}

// Override the sd::Window's CreateAccessible to create a different accessible object
css::uno::Reference<css::accessibility::XAccessible>
    ShowWindow::CreateAccessible()
{
    css::uno::Reference< css::accessibility::XAccessible > xAcc = GetAccessible(false);
    if (xAcc.get())
    {
        return xAcc;
    }
    if (mpViewShell != nullptr)
    {
        xAcc = mpViewShell->CreateAccessibleDocumentView (this);
        SetAccessible(xAcc);
        return xAcc;
    }
    else
    {
        SAL_WARN("sd", "::sd::Window::CreateAccessible: no view shell");
        return vcl::Window::CreateAccessible ();
    }
}
} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
