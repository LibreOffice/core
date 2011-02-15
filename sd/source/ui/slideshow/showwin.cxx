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
#include "precompiled_sd.hxx"

#include <com/sun/star/awt/Key.hpp>

#include "showwindow.hxx"

#include <unotools/syslocale.hxx>
#include <sfx2/viewfrm.hxx>


#include "res_bmp.hrc"
#include "slideshow.hxx"
#include "ViewShellBase.hxx"
#include "slideshow.hxx"
#include "sdresid.hxx"
#include "helpids.h"
#include "strings.hrc"
#include <vcl/virdev.hxx>

using namespace ::com::sun::star;

namespace sd {

static const sal_uLong HIDE_MOUSE_TIMEOUT = 10000;
static const sal_uLong SHOW_MOUSE_TIMEOUT = 1000;

// =============================================================================

ShowWindow::ShowWindow( const ::rtl::Reference< SlideshowImpl >& xController, ::Window* pParent )
: ::sd::Window( pParent )
, mnPauseTimeout( SLIDE_NO_TIMEOUT )
, mnRestartPageIndex( PAGE_NO_END )
, meShowWindowMode(SHOWWINDOWMODE_NORMAL)
, mbShowNavigatorAfterSpecialMode( sal_False )
, mbMouseAutoHide(true)
, mbMouseCursorHidden(false)
, mnFirstMouseMove(0)
, mxController( xController )
{
    SetOutDevViewType( OUTDEV_VIEWTYPE_SLIDESHOW );

    // Do never mirror the preview window.  This explicitly includes right
    // to left writing environments.
    EnableRTL (sal_False);

    MapMode aMap(GetMapMode());
    aMap.SetMapUnit(MAP_100TH_MM);
    SetMapMode(aMap);

    // HelpId setzen
    SetHelpId( HID_SD_WIN_PRESENTATION );
    SetUniqueId( HID_SD_WIN_PRESENTATION );

    maPauseTimer.SetTimeoutHdl( LINK( this, ShowWindow, PauseTimeoutHdl ) );
    maPauseTimer.SetTimeout( 1000 );
    maMouseTimer.SetTimeoutHdl( LINK( this, ShowWindow, MouseTimeoutHdl ) );
    maMouseTimer.SetTimeout( HIDE_MOUSE_TIMEOUT );

    maShowBackground = Wallpaper( Color( COL_BLACK ) );
//  SetBackground( Wallpaper( Color( COL_BLACK ) ) );
    SetBackground(); // avoids that VCL paints any background!
    GetParent()->Show();
    AddEventListener( LINK( this, ShowWindow, EventHdl ) );
}

ShowWindow::~ShowWindow(void)
{
    maPauseTimer.Stop();
    maMouseTimer.Stop();
}

/*************************************************************************
|*
|* Keyboard event
|*
\************************************************************************/

void ShowWindow::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;

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

/*************************************************************************
|*
|* MouseButtonDown event
|*
\************************************************************************/

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

/*************************************************************************
|*
|* MouseMove event
|*
\************************************************************************/

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
                sal_uLong nTime = Time::GetSystemTicks();
                if( (nTime - mnFirstMouseMove) >= SHOW_MOUSE_TIMEOUT )
                {
                    ShowPointer( sal_True );
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
                mnFirstMouseMove = Time::GetSystemTicks();
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

/*************************************************************************
|*
|* MouseButtonUp event
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Paint-Event: wenn FuSlideShow noch erreichbar ist, weiterleiten
|*
\************************************************************************/

void ShowWindow::Paint(const Rectangle& rRect)
{
    if( (meShowWindowMode == SHOWWINDOWMODE_NORMAL) || (meShowWindowMode == SHOWWINDOWMODE_PREVIEW) )
    {
/*
        Region aOldClipRegion( GetClipRegion() );

        Region aClipRegion( rRect );
        aClipRegion.Exclude( maPresArea );
        SetClipRegion( aClipRegion );

        DrawWallpaper( rRect, maShowBackground );

        SetClipRegion( aOldClipRegion );
*/
        if( mxController.is() )
        {
            mxController->paint(rRect);
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
            DrawPauseScene( sal_False );
        }
        else if( SHOWWINDOWMODE_BLANK == meShowWindowMode )
        {
            DrawBlankScene();
        }
    }
}

/*************************************************************************
|*
|* Notify
|*
\************************************************************************/

long ShowWindow::Notify(NotifyEvent& rNEvt)
{
    long nOK = sal_False;
/*
    if( mpViewShell && rNEvt.GetType() == EVENT_GETFOCUS )
    {
        NotifyEvent aNEvt(EVENT_GETFOCUS, this);
        nOK = mpViewShell->GetViewFrame()->GetWindow().Notify(aNEvt);
    }
*/
    if (!nOK)
        nOK = Window::Notify(rNEvt);

    return nOK;
}


// -----------------------------------------------------------------------------

void ShowWindow::GetFocus()
{
    // Basisklasse
    Window::GetFocus();
/*
    if( mpViewShell )
    {
        NotifyEvent aNEvt(EVENT_GETFOCUS, this);
        mpViewShell->GetViewFrame()->GetWindow().Notify(aNEvt);
    }
*/
}

// -----------------------------------------------------------------------------

void ShowWindow::LoseFocus()
{
    Window::LoseFocus();

    if( SHOWWINDOWMODE_PREVIEW == meShowWindowMode)
        TerminateShow();
}

// -----------------------------------------------------------------------------

void ShowWindow::Resize()
{
    ::sd::Window::Resize();
}

// -----------------------------------------------------------------------------

void ShowWindow::Move()
{
    ::sd::Window::Move();
}

// -----------------------------------------------------------------------------

sal_Bool ShowWindow::SetEndMode()
{
    if( ( SHOWWINDOWMODE_NORMAL == meShowWindowMode ) && mpViewShell && mpViewShell->GetView() )
    {
        DeleteWindowFromPaintView();
        meShowWindowMode = SHOWWINDOWMODE_END;
//      maShowBackground = GetBackground();
//      SetBackground( Wallpaper( Color( COL_BLACK ) ) );
        maShowBackground = Wallpaper( Color( COL_BLACK ) );

        // hide navigator if it is visible
        if( mpViewShell->GetViewFrame()->GetChildWindow( SID_NAVIGATOR ) )
        {
            mpViewShell->GetViewFrame()->ShowChildWindow( SID_NAVIGATOR, sal_False );
            mbShowNavigatorAfterSpecialMode = sal_True;
        }

        Invalidate();
    }

    return( SHOWWINDOWMODE_END == meShowWindowMode );
}

// -----------------------------------------------------------------------------

sal_Bool ShowWindow::SetPauseMode( sal_Int32 nPageIndexToRestart, sal_Int32 nTimeout, Graphic* pLogo )
{
    rtl::Reference< SlideShow > xSlideShow;

    if( mpViewShell )
        xSlideShow = SlideShow::GetSlideShow( mpViewShell->GetViewShellBase() );

    if( xSlideShow.is() && !nTimeout )
    {
        xSlideShow->jumpToPageIndex( nPageIndexToRestart );
    }
    else if( ( SHOWWINDOWMODE_NORMAL == meShowWindowMode ) && mpViewShell && mpViewShell->GetView() )
    {
        DeleteWindowFromPaintView();
        mnPauseTimeout = nTimeout;
        mnRestartPageIndex = nPageIndexToRestart;
        meShowWindowMode = SHOWWINDOWMODE_PAUSE;
//      maShowBackground = GetBackground();
//      SetBackground( Wallpaper( Color( COL_BLACK ) ) );
        maShowBackground = Wallpaper( Color( COL_BLACK ) );

        // hide navigator if it is visible
        if( mpViewShell->GetViewFrame()->GetChildWindow( SID_NAVIGATOR ) )
        {
            mpViewShell->GetViewFrame()->ShowChildWindow( SID_NAVIGATOR, sal_False );
            mbShowNavigatorAfterSpecialMode = sal_True;
        }

        if( pLogo )
            maLogo = *pLogo;

        Invalidate();

        if( SLIDE_NO_TIMEOUT != mnPauseTimeout )
            maPauseTimer.Start();
    }

    return( SHOWWINDOWMODE_PAUSE == meShowWindowMode );
}

// -----------------------------------------------------------------------------

sal_Bool ShowWindow::SetBlankMode( sal_Int32 nPageIndexToRestart, const Color& rBlankColor )
{
    if( ( SHOWWINDOWMODE_NORMAL == meShowWindowMode ) && mpViewShell && mpViewShell->GetView() )
    {
        DeleteWindowFromPaintView();
        mnRestartPageIndex = nPageIndexToRestart;
        meShowWindowMode = SHOWWINDOWMODE_BLANK;
//      maShowBackground = GetBackground();
//      SetBackground( Wallpaper( rBlankColor ) );
        maShowBackground = Wallpaper( rBlankColor );

        // hide navigator if it is visible
        if( mpViewShell->GetViewFrame()->GetChildWindow( SID_NAVIGATOR ) )
        {
            mpViewShell->GetViewFrame()->ShowChildWindow( SID_NAVIGATOR, sal_False );
            mbShowNavigatorAfterSpecialMode = sal_True;
        }

        Invalidate();
    }

    return( SHOWWINDOWMODE_BLANK == meShowWindowMode );
}

// -----------------------------------------------------------------------------

void ShowWindow::SetPreviewMode()
{
    meShowWindowMode = SHOWWINDOWMODE_PREVIEW;
}

// -----------------------------------------------------------------------------

void ShowWindow::TerminateShow()
{
    maLogo.Clear();
    maPauseTimer.Stop();
    maMouseTimer.Stop();
    Erase();
//  SetBackground( maShowBackground );
    maShowBackground = Wallpaper( Color( COL_BLACK ) );
    meShowWindowMode = SHOWWINDOWMODE_NORMAL;
    mnPauseTimeout = SLIDE_NO_TIMEOUT;

    if( mpViewShell )
    {
        // show navigator?
        if( mbShowNavigatorAfterSpecialMode )
        {
            mpViewShell->GetViewFrame()->ShowChildWindow( SID_NAVIGATOR, sal_True );
            mbShowNavigatorAfterSpecialMode = sal_False;
        }
    }

    if( mxController.is() )
        mxController->endPresentation();

    mnRestartPageIndex = PAGE_NO_END;
}

// -----------------------------------------------------------------------------

void ShowWindow::RestartShow()
{
    RestartShow( mnRestartPageIndex );
}

// -----------------------------------------------------------------------------

void ShowWindow::RestartShow( sal_Int32 nPageIndexToRestart )

{
    ShowWindowMode eOldShowWindowMode = meShowWindowMode;

    maLogo.Clear();
    maPauseTimer.Stop();
    Erase();
//  SetBackground( maShowBackground );
    maShowBackground = Wallpaper( Color( COL_BLACK ) );
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
        mpViewShell->GetViewFrame()->ShowChildWindow( SID_NAVIGATOR, sal_True );
        mbShowNavigatorAfterSpecialMode = sal_False;
    }
}

// -----------------------------------------------------------------------------

void ShowWindow::DrawPauseScene( sal_Bool bTimeoutOnly )
{
    const MapMode&  rMap = GetMapMode();
    const Point     aOutOrg( PixelToLogic( Point() ) );
    const Size      aOutSize( GetOutputSize() );
    const Size      aTextSize( LogicToLogic( Size( 0, 14 ), MAP_POINT, rMap ) );
    const Size      aOffset( LogicToLogic( Size( 1000, 1000 ), MAP_100TH_MM, rMap ) );
    String          aText( SdResId( STR_PRES_PAUSE ) );
    sal_Bool            bDrawn = sal_False;

    Font            aFont( GetSettings().GetStyleSettings().GetMenuFont() );
    const Font      aOldFont( GetFont() );

    aFont.SetSize( aTextSize );
    aFont.SetColor( COL_WHITE );
    aFont.SetCharSet( aOldFont.GetCharSet() );
    aFont.SetLanguage( aOldFont.GetLanguage() );

    if( !bTimeoutOnly && ( maLogo.GetType() != GRAPHIC_NONE ) )
    {
        Size aGrfSize;

        if( maLogo.GetPrefMapMode() == MAP_PIXEL )
            aGrfSize = PixelToLogic( maLogo.GetPrefSize() );
        else
            aGrfSize = LogicToLogic( maLogo.GetPrefSize(), maLogo.GetPrefMapMode(), rMap );

        const Point aGrfPos( Max( aOutOrg.X() + aOutSize.Width() - aGrfSize.Width() - aOffset.Width(), aOutOrg.X() ),
                             Max( aOutOrg.Y() + aOutSize.Height() - aGrfSize.Height() - aOffset.Height(), aOutOrg.Y() ) );

        if( maLogo.IsAnimated() )
            maLogo.StartAnimation( this, aGrfPos, aGrfSize, (long) this );
        else
            maLogo.Draw( this, aGrfPos, aGrfSize );
    }

    if( SLIDE_NO_TIMEOUT != mnPauseTimeout )
    {
        MapMode         aVMap( rMap );
        VirtualDevice   aVDev( *this );

        aVMap.SetOrigin( Point() );
        aVDev.SetMapMode( aVMap );
        aVDev.SetBackground( Wallpaper( Color( COL_BLACK ) ) );

        // set font first, to determine real output height
        aVDev.SetFont( aFont );

        const Size aVDevSize( aOutSize.Width(), aVDev.GetTextHeight() );

        if( aVDev.SetOutputSize( aVDevSize ) )
        {
            // Note: if performance gets an issue here, we can use NumberFormatter directly
            SvtSysLocale                aSysLocale;
            const LocaleDataWrapper&    aLocaleData = aSysLocale.GetLocaleData();

            aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " ( " ));
            aText += aLocaleData.getDuration( Time( 0, 0, mnPauseTimeout ) );
            aText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " )" ));
            aVDev.DrawText( Point( aOffset.Width(), 0 ), aText );
            DrawOutDev( Point( aOutOrg.X(), aOffset.Height() ), aVDevSize, Point(), aVDevSize, aVDev );
            bDrawn = sal_True;
        }
    }

    if( !bDrawn )
    {
        SetFont( aFont );
        DrawText( Point( aOutOrg.X() + aOffset.Width(), aOutOrg.Y() + aOffset.Height() ), aText );
        SetFont( aOldFont );
    }
}

// -----------------------------------------------------------------------------

void ShowWindow::DrawEndScene()
{
    const Font      aOldFont( GetFont() );
    Font            aFont( GetSettings().GetStyleSettings().GetMenuFont() );

    const Point     aOutOrg( PixelToLogic( Point() ) );
    const Size      aTextSize( LogicToLogic( Size( 0, 14 ), MAP_POINT, GetMapMode() ) );
    const String    aText( SdResId( STR_PRES_SOFTEND ) );

    aFont.SetSize( aTextSize );
    aFont.SetColor( COL_WHITE );
    aFont.SetCharSet( aOldFont.GetCharSet() );
    aFont.SetLanguage( aOldFont.GetLanguage() );
    SetFont( aFont );
    DrawText( Point( aOutOrg.X() + aTextSize.Height(), aOutOrg.Y() + aTextSize.Height() ), aText );
    SetFont( aOldFont );
}

// -----------------------------------------------------------------------------

void ShowWindow::DrawBlankScene()
{
    // just blank through background color => nothing to be done here
}

// -----------------------------------------------------------------------------

IMPL_LINK( ShowWindow, PauseTimeoutHdl, Timer*, pTimer )
{
    if( !( --mnPauseTimeout ) )
        RestartShow();
    else
    {
        DrawPauseScene( sal_True );
        pTimer->Start();
    }

    return 0L;
}

IMPL_LINK( ShowWindow, MouseTimeoutHdl, Timer*, EMPTYARG )
{
    if( mbMouseCursorHidden )
    {
        // not enough mouse movements since first recording so
        // cancle show mouse pointer for now
        mnFirstMouseMove = 0;
    }
    else
    {
        // mouse has been idle to long, hide pointer
        ShowPointer( sal_False );
        mbMouseCursorHidden = true;
    }
    return 0L;
}

IMPL_LINK( ShowWindow, EventHdl, VclWindowEvent*, pEvent )
{
    if( mbMouseAutoHide )
    {
        if (pEvent->GetId() == VCLEVENT_WINDOW_SHOW)
        {
            maMouseTimer.SetTimeout( HIDE_MOUSE_TIMEOUT );
            maMouseTimer.Start();
        }
    }
    return 0L;
}

void ShowWindow::SetPresentationArea( const Rectangle& rPresArea )
{
    maPresArea = rPresArea;
}

void ShowWindow::DeleteWindowFromPaintView()
{
    if( mpViewShell->GetView() )
        mpViewShell->GetView()->DeleteWindowFromPaintView( this );

    sal_uInt16 nChild = GetChildCount();
    while( nChild-- )
        GetChild( nChild )->Show( sal_False );
}

void ShowWindow::AddWindowToPaintView()
{
    if( mpViewShell->GetView() )
        mpViewShell->GetView()->AddWindowToPaintView( this );

    sal_uInt16 nChild = GetChildCount();
    while( nChild-- )
        GetChild( nChild )->Show( sal_True );
}

} // end of namespace sd
