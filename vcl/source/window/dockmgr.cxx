/*************************************************************************
 *
 *  $RCSfile: dockmgr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 16:52:45 $
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

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif
#ifndef _SV_FLOATWIN_HXX
#include <floatwin.hxx>
#endif
#ifndef _SV_DOCKWIN_HXX
#include <dockwin.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <toolbox.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#ifndef _SV_LINEINFO_HXX
#include <lineinfo.hxx>
#endif
#include <unowrap.hxx>
#include <salframe.hxx>


// =======================================================================

#define DOCKWIN_FLOATSTYLES         (WB_SIZEABLE | WB_MOVEABLE | WB_CLOSEABLE | WB_STANDALONE | WB_PINABLE | WB_ROLLABLE )

// =======================================================================


// =======================================================================

class ImplDockFloatWin2 : public FloatingWindow
{
private:
    ImplDockingWindowWrapper*  mpDockWin;
    ULONG           mnLastTicks;
    Timer           maDockTimer;
    Timer           maEndDockTimer;
    Point           maDockPos;
    Rectangle       maDockRect;
    BOOL            mbInMove;
    ULONG           mnLastUserEvent;

    DECL_LINK( DockingHdl, ImplDockFloatWin2* );
    DECL_LINK( DockTimerHdl, ImplDockFloatWin2* );
    DECL_LINK( EndDockTimerHdl, ImplDockFloatWin2* );
public:
    ImplDockFloatWin2( Window* pParent, WinBits nWinBits,
                      ImplDockingWindowWrapper* pDockingWin );
    ~ImplDockFloatWin2();

    virtual void    Move();
    virtual void    Resize();
    virtual void    TitleButtonClick( USHORT nButton );
    virtual void    Pin();
    virtual void    Roll();
    virtual void    PopupModeEnd();
    virtual void    Resizing( Size& rSize );
    virtual BOOL    Close();

    ULONG GetLastTicks() const { return mnLastTicks; }
};

// =======================================================================

ImplDockFloatWin2::ImplDockFloatWin2( Window* pParent, WinBits nWinBits,
                                    ImplDockingWindowWrapper* pDockingWin ) :
        FloatingWindow( pParent, nWinBits ),
        mpDockWin( pDockingWin ),
        mnLastTicks( Time::GetSystemTicks() ),
        mbInMove( FALSE ),
        mnLastUserEvent( 0 )
{
    // Daten vom DockingWindow uebernehmen
    if ( pDockingWin )
    {
        SetSettings( pDockingWin->GetWindow()->GetSettings() );
        Enable( pDockingWin->GetWindow()->IsEnabled(), FALSE );
        EnableInput( pDockingWin->GetWindow()->IsInputEnabled(), FALSE );
        AlwaysEnableInput( pDockingWin->GetWindow()->IsAlwaysEnableInput(), FALSE );
        EnableAlwaysOnTop( pDockingWin->GetWindow()->IsAlwaysOnTopEnabled() );
        SetActivateMode( pDockingWin->GetWindow()->GetActivateMode() );
    }

    SetBackground();

    maDockTimer.SetTimeoutHdl( LINK( this, ImplDockFloatWin2, DockTimerHdl ) );
    maDockTimer.SetTimeout( 50 );
    maEndDockTimer.SetTimeoutHdl( LINK( this, ImplDockFloatWin2, EndDockTimerHdl ) );
    maEndDockTimer.SetTimeout( 50 );
}

// -----------------------------------------------------------------------

ImplDockFloatWin2::~ImplDockFloatWin2()
{
    if( mnLastUserEvent )
        Application::RemoveUserEvent( mnLastUserEvent );
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplDockFloatWin2, DockTimerHdl, ImplDockFloatWin2*, pWin )
{
    DBG_ASSERT( mpDockWin->IsFloatingMode(), "docktimer called but not floating" );

    maDockTimer.Stop();
    PointerState aState = GetPointerState();
    if( ! ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) )
    {
        mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->HideTracking();
        mpDockWin->EndDocking( maDockRect, FALSE );
    }
    else
    {
        maDockTimer.Start();
    }

    return 0;
}

IMPL_LINK( ImplDockFloatWin2, EndDockTimerHdl, ImplDockFloatWin2*, pWin )
{
    DBG_ASSERT( mpDockWin->IsFloatingMode(), "enddocktimer called but not floating" );

    maEndDockTimer.Stop();
    PointerState aState = GetPointerState();
    if( ! ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) )
    {
        mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->HideTracking();
        mpDockWin->EndDocking( maDockRect, TRUE );
    }
    else
    {
        maEndDockTimer.Start();
    }

    return 0;
}


IMPL_LINK( ImplDockFloatWin2, DockingHdl, ImplDockFloatWin2*, pWindow )
{
    // called during move of a floating window
    mnLastUserEvent = 0;

    Window *pDockingArea = mpDockWin->GetWindow()->GetParent();
    PointerState aState = pDockingArea->GetPointerState();

    if( mpDockWin->IsDockable() &&
        mpDockWin->GetWindow()->IsVisible() &&
        (Time::GetSystemTicks() - mnLastTicks > 500) &&
        ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) )
    {
        maDockPos = Point( pDockingArea->OutputToScreenPixel( pDockingArea->AbsoluteScreenToOutputPixel( OutputToAbsoluteScreenPixel( Point() ) ) ) );
        maDockRect = Rectangle( maDockPos, mpDockWin->GetSizePixel() );

        // mouse pos in screen pixels
        Point aMousePos = pDockingArea->OutputToScreenPixel( aState.maPos );

        if( ! mpDockWin->IsDocking() )
            mpDockWin->StartDocking( aMousePos, maDockRect );

        BOOL bFloatMode = mpDockWin->Docking( aMousePos, maDockRect );

        if( ! bFloatMode )
        {
            // indicates that the window could be docked at maDockRect
            maDockRect.SetPos( mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->ScreenToOutputPixel(
                 maDockRect.TopLeft() ) );
            mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->ShowTracking( maDockRect, SHOWTRACK_BIG | SHOWTRACK_WINDOW );
            maEndDockTimer.Stop();
            DockTimerHdl( this );
        }
        else
        {
            mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->HideTracking();
            maDockTimer.Stop();
            EndDockTimerHdl( this );
        }
    }
    mbInMove = FALSE;
    return 0;
}
// -----------------------------------------------------------------------

void ImplDockFloatWin2::Move()
{
    if( mbInMove )
        return;

    mbInMove = TRUE;
    FloatingWindow::Move();
    mpDockWin->GetWindow()->Move();

    /*
     *  note: the window should only dock if KEY_MOD1 is pressed
     *  and the user releases all mouse buttons. The real problem here
     *  is that we don't get mouse events (at least not on X)
     *  if the mouse is on the decoration. So we have to start an
     *  awkward timer based process that polls the modifier/buttons
     *  to see whether they are in the right condition shortly after the
     *  last Move message.
     */
    if( ! mnLastUserEvent )
        mnLastUserEvent = Application::PostUserEvent( LINK( this, ImplDockFloatWin2, DockingHdl ) );
}

// -----------------------------------------------------------------------

void ImplDockFloatWin2::Resize()
{
    FloatingWindow::Resize();
    Size aSize( GetSizePixel() );
    mpDockWin->GetWindow()->ImplPosSizeWindow( 0, 0, aSize.Width(), aSize.Height(), WINDOW_POSSIZE_POSSIZE );
}

// -----------------------------------------------------------------------


void ImplDockFloatWin2::TitleButtonClick( USHORT nButton )
{
    FloatingWindow::TitleButtonClick( nButton );
    mpDockWin->TitleButtonClick( nButton );
}

// -----------------------------------------------------------------------

void ImplDockFloatWin2::Pin()
{
    FloatingWindow::Pin();
    mpDockWin->Pin();
}

// -----------------------------------------------------------------------

void ImplDockFloatWin2::Roll()
{
    FloatingWindow::Roll();
    mpDockWin->Roll();
}

// -----------------------------------------------------------------------

void ImplDockFloatWin2::PopupModeEnd()
{
    FloatingWindow::PopupModeEnd();
    mpDockWin->PopupModeEnd();
}

// -----------------------------------------------------------------------

void ImplDockFloatWin2::Resizing( Size& rSize )
{
    FloatingWindow::Resizing( rSize );
    mpDockWin->Resizing( rSize );
}

// -----------------------------------------------------------------------

BOOL ImplDockFloatWin2::Close()
{
    return mpDockWin->Close();
}

// =======================================================================

DockingManager::DockingManager()
{
}

DockingManager::~DockingManager()
{
    ::std::vector< ImplDockingWindowWrapper* >::iterator p;
    p = mDockingWindows.begin();
    while( p != mDockingWindows.end() )
    {
        delete (*p);
        mDockingWindows.erase( p );
        p++;
    }
}

ImplDockingWindowWrapper* DockingManager::GetDockingWindowWrapper( const Window *pWindow )
{
    ::std::vector< ImplDockingWindowWrapper* >::iterator p;
    p = mDockingWindows.begin();
    while( p != mDockingWindows.end() )
    {
        if( (*p)->mpDockingWindow == pWindow )
            return (*p);
        else
            p++;
    }
    return NULL;
}

BOOL DockingManager::IsDockable( const Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );

    /*
    if( pWindow->HasDockingHandler() )
        return TRUE;
    */
    return (pWrapper != NULL);
}

BOOL DockingManager::IsFloating( const Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        return pWrapper->IsFloatingMode();
    else
        return FALSE;
}

BOOL DockingManager::IsLocked( const Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper && pWrapper->IsLocked() )
        return TRUE;
    else
        return FALSE;
}

void DockingManager::Lock( const Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->Lock();
}

void DockingManager::Unlock( const Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->Unlock();
}

void DockingManager::SetFloatingMode( const Window *pWindow, BOOL bFloating )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->SetFloatingMode( bFloating );
}

void DockingManager::StartPopupMode( const Window *pWindow, const Rectangle& rRect )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->StartPopupMode( rRect );
}

BOOL DockingManager::IsInPopupMode( const Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper && pWrapper->IsInPopupMode() )
        return TRUE;
    else
        return FALSE;
}

void DockingManager::AddWindow( const Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        return;
    else
        pWrapper = new ImplDockingWindowWrapper( pWindow );

    mDockingWindows.push_back( pWrapper );
}

void DockingManager::RemoveWindow( const Window *pWindow )
{
    ::std::vector< ImplDockingWindowWrapper* >::iterator p;
    p = mDockingWindows.begin();
    while( p != mDockingWindows.end() )
    {
        if( (*p)->mpDockingWindow == pWindow )
        {
            delete (*p);
            mDockingWindows.erase( p );
            break;
        }
        else
            p++;
    }
}

void DockingManager::SetPosSizePixel( Window *pWindow, long nX, long nY,
                                    long nWidth, long nHeight,
                                    USHORT nFlags )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

Rectangle DockingManager::GetPosSizePixel( const Window *pWindow )
{
    Rectangle aRect;
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        aRect = Rectangle( pWrapper->GetPosPixel(), pWrapper->GetSizePixel() );

    return aRect;
}

// =======================================================================
// special floating window for popup mode
// main purpose: provides tear-off area for undocking
// =======================================================================

// if TEAROFF_DASHED defined a single dashed line is used
// otherwise multiple smaller lines will be painted
//#define TEAROFF_DASHED

// size of the drag area
#ifdef TEAROFF_DASHED
#define POPUP_DRAGBORDER    2
#define POPUP_DRAGGRIP      5
#else
#define POPUP_DRAGBORDER    3
#define POPUP_DRAGGRIP      5
#endif
#define POPUP_DRAGHEIGHT    (POPUP_DRAGGRIP+POPUP_DRAGBORDER+POPUP_DRAGBORDER)
#define POPUP_DRAGWIDTH     20

class ImplPopupFloatWin : public FloatingWindow
{
private:
    ImplDockingWindowWrapper*   mpDockingWin;
    BOOL                        mbHighlight;
    BOOL                        mbMoving;
    Point                       maDelta;
    Point                       maTearOffPosition;

public:
    ImplPopupFloatWin( Window* pParent, ImplDockingWindowWrapper* pDockingWin );
    ~ImplPopupFloatWin();

    virtual void        Paint( const Rectangle& rRect );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void        Tracking( const TrackingEvent& rTEvt );

    void                SetPopupSizePixel( const Size& rSize );
    Rectangle           GetDragRect() const;
    Point               GetToolboxPosition() const;
    Point               GetTearOffPosition() const;
    void                DrawGrip();
    void                DrawBorder();
};

ImplPopupFloatWin::ImplPopupFloatWin( Window* pParent, ImplDockingWindowWrapper* pDockingWin ) :
    FloatingWindow( pParent, WB_NOBORDER | WB_SYSTEMWINDOW )
{
    mpDockingWin = pDockingWin;
    mbHighlight = FALSE;
    mbMoving = FALSE;
}

ImplPopupFloatWin::~ImplPopupFloatWin()
{
    mpDockingWin = NULL;
}
Rectangle ImplPopupFloatWin::GetDragRect() const
{
    return Rectangle( 1, 1, GetOutputSizePixel().Width()-1, 2+POPUP_DRAGHEIGHT );
}

void ImplPopupFloatWin::SetPopupSizePixel( const Size& rSize )
{
    Size aFloatSize( rSize );

    // add in grip
    aFloatSize.Height() += GetDragRect().getHeight();
    // add in border
    aFloatSize.Height() += 2;
    aFloatSize.Width() += 2;

    SetOutputSizePixel( aFloatSize );
}

Point ImplPopupFloatWin::GetToolboxPosition() const
{
    // return inner position where a toolbox could be placed
    Point aPt( 1, 1+GetDragRect().getHeight() );    // grip + border
    return aPt;
}

Point ImplPopupFloatWin::GetTearOffPosition() const
{
    Point aPt( maTearOffPosition );
    aPt += GetToolboxPosition();    // remove 'decoration'
    return aPt;
}

void ImplPopupFloatWin::DrawBorder()
{
    SetFillColor();
    SetLineColor( GetSettings().GetStyleSettings().GetShadowColor() );
    Point aPt;
    Rectangle aRect( aPt, GetOutputSizePixel() );
    DrawRect( aRect );
}

void ImplPopupFloatWin::DrawGrip()
{
    BOOL bLinecolor     = IsLineColor();
    Color aLinecolor    = GetLineColor();
    BOOL bFillcolor     = IsFillColor();
    Color aFillcolor    = GetFillColor();

    // draw background
    Rectangle aRect( GetDragRect() );
    aRect.nTop      += POPUP_DRAGBORDER;
    aRect.nBottom   -= POPUP_DRAGBORDER;
    aRect.nLeft+=3;
    aRect.nRight-=3;

    if( mbHighlight )
    {
        Erase( aRect );
        DrawSelectionBackground( aRect, 2, FALSE, TRUE, FALSE );
    }
    else
    {
        SetFillColor( GetSettings().GetStyleSettings().GetFaceColor() );
        SetLineColor();
        DrawRect( aRect );
    }

#ifdef TEAROFF_DASHED
    // draw single dashed line
    LineInfo aLineInfo( LINE_DASH );
    aLineInfo.SetDistance( 4 );
    aLineInfo.SetDashLen( 12 );
    aLineInfo.SetDashCount( 1 );

    aRect.nLeft+=2; aRect.nRight-=2;

    aRect.nTop+=2;
    aRect.nBottom = aRect.nTop;
    SetLineColor( GetSettings().GetStyleSettings().GetDarkShadowColor() );
    DrawLine( aRect.TopLeft(), aRect.TopRight(), aLineInfo );

    if( !mbHighlight )
    {
        aRect.nTop++; aRect.nBottom++;
        SetLineColor( GetSettings().GetStyleSettings().GetLightColor() );
        DrawLine( aRect.TopLeft(), aRect.TopRight(), aLineInfo );
    }

#else
    // draw several grip lines
    SetFillColor( GetSettings().GetStyleSettings().GetShadowColor() );
    aRect.nTop++;
    aRect.nBottom = aRect.nTop;

    int width = POPUP_DRAGWIDTH;
    while( width >= aRect.getWidth() )
        width -= 4;
    if( width <= 0 )
        width = aRect.getWidth();
    //aRect.nLeft = aRect.nLeft + (aRect.getWidth() - width) / 2;
    aRect.nLeft = (aRect.nLeft + aRect.nRight - width) / 2;
    aRect.nRight = aRect.nLeft + width;

    int i=0;
    while( i< POPUP_DRAGGRIP )
    {
        DrawRect( aRect );
        aRect.nTop+=2;
        aRect.nBottom+=2;
        i+=2;
    }
#endif

    if( bLinecolor )
        SetLineColor( aLinecolor );
    else
        SetLineColor();
    if( bFillcolor )
        SetFillColor( aFillcolor );
    else
        SetFillColor();
}

void ImplPopupFloatWin::Paint( const Rectangle& rRect )
{
    DrawBorder();
    DrawGrip();
    FloatingWindow::Paint( rRect );
}

void ImplPopupFloatWin::MouseMove( const MouseEvent& rMEvt )
{
    Point aMousePos = rMEvt.GetPosPixel();

    if( rMEvt.IsLeft() && GetDragRect().IsInside( aMousePos ) )
    {
        // start window move
        mbMoving = TRUE;
        StartTracking( STARTTRACK_NOKEYCANCEL );
        return;
    }
    if( !mbHighlight && GetDragRect().IsInside( aMousePos ) )
    {
        mbHighlight = TRUE;
        DrawGrip();
    }
    if( mbHighlight && ( rMEvt.IsLeaveWindow() || !GetDragRect().IsInside( aMousePos ) ) )
    {
        mbHighlight = FALSE;
        DrawGrip();
    }
}

void ImplPopupFloatWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    FloatingWindow::MouseButtonUp( rMEvt );
}

void ImplPopupFloatWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aMousePos = rMEvt.GetPosPixel();
    if( GetDragRect().IsInside( aMousePos ) )
    {
        // get mouse pos at a static window to have a fixed reference point
        PointerState aState = GetParent()->GetPointerState();
        maTearOffPosition = GetWindow( WINDOW_BORDER )->GetPosPixel();
        maDelta = aState.maPos - maTearOffPosition;
    }
}

void ImplPopupFloatWin::Tracking( const TrackingEvent& rTEvt )
{
    if( mbMoving )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbMoving = FALSE;
            EndPopupMode( FLOATWIN_POPUPMODEEND_TEAROFF );
        }
        else if ( !rTEvt.GetMouseEvent().IsSynthetic() )
        {
            // move the window according to mouse pos
            PointerState aState = GetParent()->GetPointerState();
            maTearOffPosition = aState.maPos - maDelta;
            GetWindow( WINDOW_BORDER )->SetPosPixel( maTearOffPosition );
        }
    }
}


// =======================================================================

ImplDockingWindowWrapper::ImplDockingWindowWrapper( const Window *pWindow )
{
    ImplInitData();

    mpDockingWindow = (Window*) pWindow;
    mpParent        = pWindow->GetParent();
    mbDockable      = TRUE;
    mbLocked        = FALSE;
    mnFloatBits     = WB_BORDER | WB_CLOSEABLE | WB_SIZEABLE | (pWindow->GetStyle() & DOCKWIN_FLOATSTYLES);

    // must be enabled in Window::Notify to prevent permanent docking during mouse move
    mbStartDockingEnabled = FALSE;
}

ImplDockingWindowWrapper::~ImplDockingWindowWrapper()
{
    if ( IsFloatingMode() )
    {
        GetWindow()->Show( FALSE, SHOW_NOFOCUSCHANGE );
        SetFloatingMode( FALSE );
    }
}

// -----------------------------------------------------------------------

BOOL ImplDockingWindowWrapper::ImplStartDocking( const Point& rPos )
{
    if ( !mbDockable )
        return FALSE;

    if( !mbStartDockingEnabled )
        return FALSE;

    maMouseOff      = rPos;
    maMouseStart    = maMouseOff;
    mbDocking       = TRUE;
    mbLastFloatMode = IsFloatingMode();
    mbStartFloat    = mbLastFloatMode;

    // FloatingBorder berechnen
    FloatingWindow* pWin;
    if ( mpFloatWin )
        pWin = mpFloatWin;
    else
        pWin = new ImplDockFloatWin2( mpParent, mnFloatBits, NULL );
    pWin->GetBorder( mnDockLeft, mnDockTop, mnDockRight, mnDockBottom );
    if ( !mpFloatWin )
        delete pWin;

    Point   aPos    = GetWindow()->ImplOutputToFrame( Point() );
    Size    aSize   = GetWindow()->GetOutputSizePixel();
    mnTrackX        = aPos.X();
    mnTrackY        = aPos.Y();
    mnTrackWidth    = aSize.Width();
    mnTrackHeight   = aSize.Height();

    if ( mbLastFloatMode )
    {
        maMouseOff.X()  += mnDockLeft;
        maMouseOff.Y()  += mnDockTop;
        mnTrackX        -= mnDockLeft;
        mnTrackY        -= mnDockTop;
        mnTrackWidth    += mnDockLeft+mnDockRight;
        mnTrackHeight   += mnDockTop+mnDockBottom;
    }

    Window *pDockingArea = GetWindow()->GetParent();
    Window::PointerState aState = pDockingArea->GetPointerState();

    // mouse pos in screen pixels
    Point aMousePos = pDockingArea->OutputToScreenPixel( aState.maPos );
    Point aDockPos = Point( pDockingArea->AbsoluteScreenToOutputPixel( GetWindow()->OutputToAbsoluteScreenPixel( GetWindow()->GetPosPixel() ) ) );
    Rectangle aDockRect( aDockPos, GetWindow()->GetSizePixel() );
    StartDocking( aMousePos, aDockRect );

    GetWindow()->ImplUpdateAll();
    GetWindow()->ImplGetFrameWindow()->ImplUpdateAll();

    GetWindow()->StartTracking( STARTTRACK_KEYMOD );
    return TRUE;
}

// =======================================================================

void ImplDockingWindowWrapper::ImplInitData()
{
    mpDockingWindow     = NULL;

    //GetWindow()->mbDockWin  = TRUE;     // TODO: must be eliminated
    mpFloatWin          = NULL;
    mbDockCanceled      = FALSE;
    mbFloatPrevented    = FALSE;
    mbDocking           = FALSE;
    mbPined             = FALSE;
    mbRollUp            = FALSE;
    mbDockBtn           = FALSE;
    mbHideBtn           = FALSE;
}

// -----------------------------------------------------------------------
/*
void DockingWindow::ImplInit( Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;

    mpParent                = pParent;
    mbDockable              = (nStyle & WB_DOCKABLE) != 0;
    mnFloatBits             = WB_BORDER | (nStyle & DOCKWIN_FLOATSTYLES);
    nStyle                 &= ~(DOCKWIN_FLOATSTYLES | WB_BORDER);
    if ( nStyle & WB_DOCKBORDER )
        nStyle |= WB_BORDER;

    Window::ImplInit( pParent, nStyle, NULL );

    ImplInitSettings();
}

// -----------------------------------------------------------------------

void DockingWindow::ImplInitSettings()
{
    // Hack, damit man auch DockingWindows ohne Hintergrund bauen kann
    // und noch nicht alles umgestellt ist
    if ( IsBackground() )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else if ( Window::GetStyle() & WB_3DLOOK )
            aColor = rStyleSettings.GetFaceColor();
        else
            aColor = rStyleSettings.GetWindowColor();
        SetBackground( aColor );
    }
}

// -----------------------------------------------------------------------

void DockingWindow::ImplLoadRes( const ResId& rResId )
{
    Window::ImplLoadRes( rResId );

    USHORT  nMask = ReadShortRes();

    if ( (RSC_DOCKINGWINDOW_XYMAPMODE | RSC_DOCKINGWINDOW_X |
          RSC_DOCKINGWINDOW_Y) & nMask )
    {
        // Groessenangabe aus der Resource verwenden
        Point   aPos;
        MapUnit ePosMap = MAP_PIXEL;

        if ( RSC_DOCKINGWINDOW_XYMAPMODE & nMask )
            ePosMap = (MapUnit)ReadShortRes();

        if ( RSC_DOCKINGWINDOW_X & nMask )
        {
            aPos.X() = ReadShortRes();
            aPos.X() = ImplLogicUnitToPixelX( aPos.X(), ePosMap );
        }

        if ( RSC_DOCKINGWINDOW_Y & nMask )
        {
            aPos.Y() = ReadShortRes();
            aPos.Y() = ImplLogicUnitToPixelY( aPos.Y(), ePosMap );
        }

        SetFloatingPos( aPos );
    }

    if ( nMask & RSC_DOCKINGWINDOW_FLOATING )
    {
        if ( (BOOL)ReadShortRes() )
            SetFloatingMode( TRUE );
    }
}

// -----------------------------------------------------------------------

DockingWindow::DockingWindow( WindowType nType ) :
    Window( nType )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

DockingWindow::DockingWindow( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_DOCKINGWINDOW )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

DockingWindow::DockingWindow( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_DOCKINGWINDOW )
{
    ImplInitData();
    rResId.SetRT( RSC_DOCKINGWINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

DockingWindow::~DockingWindow()
{
    if ( IsFloatingMode() )
    {
        Show( FALSE, SHOW_NOFOCUSCHANGE );
        SetFloatingMode( FALSE );
    }
}
*/
// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::Tracking( const TrackingEvent& rTEvt )
{
    // used during docking of a currently docked window
    if ( mbDocking )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbDocking = FALSE;
            GetWindow()->HideTracking();
            if ( rTEvt.IsTrackingCanceled() )
            {
                mbDockCanceled = TRUE;
                EndDocking( Rectangle( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) ), mbLastFloatMode );
                mbDockCanceled = FALSE;
            }
            else
                EndDocking( Rectangle( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) ), mbLastFloatMode );
        }
        // Docking only upon non-synthetic MouseEvents
        else if ( !rTEvt.GetMouseEvent().IsSynthetic() || rTEvt.GetMouseEvent().IsModifierChanged() )
        {
            Point   aMousePos = rTEvt.GetMouseEvent().GetPosPixel();
            Point   aFrameMousePos = GetWindow()->ImplOutputToFrame( aMousePos );
            Size    aFrameSize = GetWindow()->ImplGetFrameWindow()->GetOutputSizePixel();
            if ( aFrameMousePos.X() < 0 )
                aFrameMousePos.X() = 0;
            if ( aFrameMousePos.Y() < 0 )
                aFrameMousePos.Y() = 0;
            if ( aFrameMousePos.X() > aFrameSize.Width()-1 )
                aFrameMousePos.X() = aFrameSize.Width()-1;
            if ( aFrameMousePos.Y() > aFrameSize.Height()-1 )
                aFrameMousePos.Y() = aFrameSize.Height()-1;
            aMousePos = GetWindow()->ImplFrameToOutput( aFrameMousePos );
            aMousePos.X() -= maMouseOff.X();
            aMousePos.Y() -= maMouseOff.Y();
            Point aPos = GetWindow()->ImplOutputToFrame( aMousePos );
            Rectangle aTrackRect( aPos, Size( mnTrackWidth, mnTrackHeight ) );
            Rectangle aCompRect = aTrackRect;
            aPos.X()    += maMouseOff.X();
            aPos.Y()    += maMouseOff.Y();

            BOOL bFloatMode = Docking( aPos, aTrackRect );

            mbFloatPrevented = FALSE;
            if ( mbLastFloatMode != bFloatMode )
            {
                if ( bFloatMode )
                {
                    aTrackRect.Left()   -= mnDockLeft;
                    aTrackRect.Top()    -= mnDockTop;
                    aTrackRect.Right()  += mnDockRight;
                    aTrackRect.Bottom() += mnDockBottom;
                }
                else
                {
                    if ( aCompRect == aTrackRect )
                    {
                        aTrackRect.Left()   += mnDockLeft;
                        aTrackRect.Top()    += mnDockTop;
                        aTrackRect.Right()  -= mnDockRight;
                        aTrackRect.Bottom() -= mnDockBottom;
                    }
                }
                mbLastFloatMode = bFloatMode;
            }

            USHORT nTrackStyle;
            if ( bFloatMode )
                nTrackStyle = SHOWTRACK_OBJECT;
            else
                nTrackStyle = SHOWTRACK_BIG;
            Rectangle aShowTrackRect = aTrackRect;
            aShowTrackRect.SetPos( GetWindow()->ImplFrameToOutput( aShowTrackRect.TopLeft() ) );
            //if( bFloatMode )
                GetWindow()->ShowTracking( aShowTrackRect, nTrackStyle );
            /*else
            {
                GetWindow()->HideTracking();
                Point aPt( GetWindow()->GetParent()->ScreenToOutputPixel( aTrackRect.TopLeft() ) );
                GetWindow()->SetPosPixel( aPt );
            }*/

            // Maus-Offset neu berechnen, da Rechteck veraendert werden
            // konnte
            maMouseOff.X()  = aPos.X() - aTrackRect.Left();
            maMouseOff.Y()  = aPos.Y() - aTrackRect.Top();

            mnTrackX        = aTrackRect.Left();
            mnTrackY        = aTrackRect.Top();
            mnTrackWidth    = aTrackRect.GetWidth();
            mnTrackHeight   = aTrackRect.GetHeight();
        }
    }
}


// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::StartDocking( const Point& rPoint, Rectangle& rRect )
{
    DockingData data( rPoint, rRect, IsFloatingMode() );

    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_STARTDOCKING, &data );
    mbDocking = TRUE;
}

// -----------------------------------------------------------------------

BOOL ImplDockingWindowWrapper::Docking( const Point& rPoint, Rectangle& rRect )
{
    DockingData data( rPoint, rRect, IsFloatingMode() );

    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_DOCKING, &data );
    rRect = data.maTrackRect;
    return data.mbFloating;
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::EndDocking( const Rectangle& rRect, BOOL bFloatMode )
{
    Rectangle aRect( rRect );

    if ( !IsDockingCanceled() )
    {
        BOOL bShow = FALSE;
        if ( bFloatMode != IsFloatingMode() )
        {
            GetWindow()->Show( FALSE, SHOW_NOFOCUSCHANGE );
            SetFloatingMode( bFloatMode );
            bShow = TRUE;
            if ( bFloatMode )
                mpFloatWin->SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );
        }
        if ( !bFloatMode )
        {
            Point aPos = aRect.TopLeft();
            aPos = GetWindow()->GetParent()->ScreenToOutputPixel( aPos );
            GetWindow()->SetPosSizePixel( aPos, aRect.GetSize() );
        }

        if ( bShow )
            GetWindow()->Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
    }

    EndDockingData data( aRect, IsFloatingMode(), IsDockingCanceled() );
    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_ENDDOCKING, &data );

    mbDocking = FALSE;

    // must be enabled in Window::Notify to prevent permanent docking during mouse move
    mbStartDockingEnabled = FALSE;
}

// -----------------------------------------------------------------------

BOOL ImplDockingWindowWrapper::PrepareToggleFloatingMode()
{
    BOOL bFloating = TRUE;
    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_PREPARETOGGLEFLOATING, &bFloating );
    return bFloating;
}

// -----------------------------------------------------------------------

BOOL ImplDockingWindowWrapper::Close()
{
    // TODO: send event
/*
    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_CLOSE );
    if ( aDelData.IsDelete() )
        return FALSE;
    ImplRemoveDel( &aDelData );

    if ( mxWindowPeer.is() && IsCreatedWithToolkit() )
        return FALSE;

    GetWindow()->Show( FALSE, SHOW_NOFOCUSCHANGE );
    */
    return TRUE;
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::ToggleFloatingMode()
{
    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_TOGGLEFLOATING );

    // notify dockingwindow/toolbox
    if( GetWindow()->ImplIsDockingWindow() )
        ((DockingWindow*) GetWindow())->ToggleFloatingMode();

    // must be enabled in Window::Notify to prevent permanent docking during mouse move
    mbStartDockingEnabled = FALSE;
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::TitleButtonClick( USHORT )
{
    // TODO: send event
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::Pin()
{
    // TODO: send event
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::Roll()
{
    // TODO: send event
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::PopupModeEnd()
{
    // TODO: send event
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::Resizing( Size& )
{
    // TODO: send event
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::ShowTitleButton( USHORT nButton, BOOL bVisible )
{
    if ( mpFloatWin )
        mpFloatWin->ShowTitleButton( nButton, bVisible );
    else
    {
        if ( nButton == TITLE_BUTTON_DOCKING )
            mbDockBtn = bVisible;
        else // if ( nButton == TITLE_BUTTON_HIDE )
            mbHideBtn = bVisible;
    }
}

// -----------------------------------------------------------------------

BOOL ImplDockingWindowWrapper::IsTitleButtonVisible( USHORT nButton ) const
{
    if ( mpFloatWin )
        return mpFloatWin->IsTitleButtonVisible( nButton );
    else
    {
        if ( nButton == TITLE_BUTTON_DOCKING )
            return mbDockBtn;
        else // if ( nButton == TITLE_BUTTON_HIDE )
            return mbHideBtn;
    }
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::StartPopupMode( const Rectangle& rRect )
{
    // do nothing if window is floating
    if( IsFloatingMode() )
        return;

    GetWindow()->Show( FALSE, SHOW_NOFOCUSCHANGE );

    // prepare reparenting
    Window* pRealParent = GetWindow()->GetWindow( WINDOW_PARENT );
    mpOldBorderWin = GetWindow()->GetWindow( WINDOW_BORDER );
    if( mpOldBorderWin == GetWindow() )
        mpOldBorderWin = NULL;  // no border window found

    // the new parent for popup mode
    ImplPopupFloatWin* pWin = new ImplPopupFloatWin( mpParent, this );

    pWin->SetPopupModeEndHdl( LINK( this, ImplDockingWindowWrapper, PopupModeEnd ) );
    pWin->SetText( GetWindow()->GetText() );

    pWin->SetPopupSizePixel( GetWindow()->GetSizePixel() );

    GetWindow()->mpBorderWindow  = NULL;
    GetWindow()->mnLeftBorder    = 0;
    GetWindow()->mnTopBorder     = 0;
    GetWindow()->mnRightBorder   = 0;
    GetWindow()->mnBottomBorder  = 0;

    // position toolbox below dragrect
    GetWindow()->SetPosPixel( pWin->GetToolboxPosition() );

    // reparent borderwindow and window
    if ( mpOldBorderWin )
        mpOldBorderWin->SetParent( pWin );
    GetWindow()->SetParent( pWin );

    // correct border window pointers
    GetWindow()->mpBorderWindow = pWin;
    pWin->mpClientWindow = GetWindow();
    GetWindow()->mpRealParent = pRealParent;

    // set mpFloatWin not until all window positioning is done !!!
    // (SetPosPixel etc. check for valid mpFloatWin pointer)
    mpFloatWin = pWin;

    ULONG nFlags =  FLOATWIN_POPUPMODE_ALLOWTEAROFF         |
                    FLOATWIN_POPUPMODE_NOFOCUSCLOSE         |
                    FLOATWIN_POPUPMODE_ALLMOUSEBUTTONCLOSE  |
                    FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE       |
                    FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE      |
                    FLOATWIN_POPUPMODE_DOWN;

    Rectangle aRect( rRect );
    aRect.setHeight( 0 );
    aRect.setWidth( 0 );
    mpFloatWin->StartPopupMode( aRect, nFlags );
    GetWindow()->Show();
}

IMPL_LINK( ImplDockingWindowWrapper, PopupModeEnd, void*, EMPTYARG )
{
    GetWindow()->Show( FALSE, SHOW_NOFOCUSCHANGE );

    // set parameter for handler before destroying floating window
    ImplPopupFloatWin *pPopupFloatWin = (ImplPopupFloatWin*) mpFloatWin;
    EndPopupModeData aData( pPopupFloatWin->GetTearOffPosition(), mpFloatWin->IsPopupModeTearOff() );

    // before deleting change parent back, so we can delete the floating window alone
    Window* pRealParent = GetWindow()->GetWindow( WINDOW_PARENT );
    GetWindow()->mpBorderWindow = NULL;
    if ( mpOldBorderWin )
    {
        GetWindow()->SetParent( mpOldBorderWin );
        ((ImplBorderWindow*)mpOldBorderWin)->GetBorder(
            GetWindow()->mnLeftBorder, GetWindow()->mnTopBorder,
            GetWindow()->mnRightBorder, GetWindow()->mnBottomBorder );
        mpOldBorderWin->Resize();
    }
    GetWindow()->mpBorderWindow = mpOldBorderWin;
    GetWindow()->SetParent( pRealParent );
    GetWindow()->mpRealParent = pRealParent;

    delete mpFloatWin;
    mpFloatWin = NULL;

    // call handler - which will destroy the window and thus the wrapper as well !
    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_ENDPOPUPMODE, &aData );

    return 0;
}


BOOL ImplDockingWindowWrapper::IsInPopupMode() const
{
    if( GetFloatingWindow() )
        return GetFloatingWindow()->IsInPopupMode();
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::SetFloatingMode( BOOL bFloatMode )
{
    // do nothing if window is docked and locked
    if( !IsFloatingMode() && IsLocked() )
        return;

    if ( IsFloatingMode() != bFloatMode )
    {
        if ( PrepareToggleFloatingMode() )
        {
            BOOL bVisible = GetWindow()->IsVisible();

            if ( bFloatMode )
            {
                GetWindow()->Show( FALSE, SHOW_NOFOCUSCHANGE );

                maDockPos = GetWindow()->GetPosPixel();

                Window* pRealParent = GetWindow()->GetWindow( WINDOW_PARENT );
                mpOldBorderWin = GetWindow()->GetWindow( WINDOW_BORDER );
                if( mpOldBorderWin == mpDockingWindow )
                    mpOldBorderWin = NULL;  // no border window found

                ImplDockFloatWin2* pWin =
                    new ImplDockFloatWin2(
                                         mpParent,
                                         mnFloatBits & ( WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) ?  mnFloatBits | WB_SYSTEMWINDOW : mnFloatBits,
                                         this );
                mpFloatWin      = pWin;


                GetWindow()->mpBorderWindow  = NULL;
                GetWindow()->mnLeftBorder    = 0;
                GetWindow()->mnTopBorder     = 0;
                GetWindow()->mnRightBorder   = 0;
                GetWindow()->mnBottomBorder  = 0;

                // Falls Parent zerstoert wird, muessen wir auch vom
                // BorderWindow den Parent umsetzen
                if ( mpOldBorderWin )
                    mpOldBorderWin->SetParent( pWin );
                GetWindow()->SetParent( pWin );
                pWin->SetPosPixel( Point() );

                GetWindow()->mpBorderWindow = pWin;
                pWin->mpClientWindow = mpDockingWindow;
                GetWindow()->mpRealParent = pRealParent;

                pWin->SetText( GetWindow()->GetText() );
                pWin->SetOutputSizePixel( GetWindow()->GetSizePixel() );
                pWin->SetPosPixel( maFloatPos );
                // DockingDaten ans FloatingWindow weiterreichen
                pWin->ShowTitleButton( TITLE_BUTTON_DOCKING, mbDockBtn );
                pWin->ShowTitleButton( TITLE_BUTTON_HIDE, mbHideBtn );
                pWin->SetPin( mbPined );
                if ( mbRollUp )
                    pWin->RollUp();
                else
                    pWin->RollDown();
                pWin->SetRollUpOutputSizePixel( maRollUpOutSize );
                pWin->SetMinOutputSizePixel( maMinOutSize );

                if ( bVisible )
                    GetWindow()->Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );

                ToggleFloatingMode();
            }
            else
            {
                GetWindow()->Show( FALSE, SHOW_NOFOCUSCHANGE );

                // FloatingDaten wird im FloatingWindow speichern
                maFloatPos      = mpFloatWin->GetPosPixel();
                mbDockBtn       = mpFloatWin->IsTitleButtonVisible( TITLE_BUTTON_DOCKING );
                mbHideBtn       = mpFloatWin->IsTitleButtonVisible( TITLE_BUTTON_HIDE );
                mbPined         = mpFloatWin->IsPined();
                mbRollUp        = mpFloatWin->IsRollUp();
                maRollUpOutSize = mpFloatWin->GetRollUpOutputSizePixel();
                maMinOutSize    = mpFloatWin->GetMinOutputSizePixel();

                Window* pRealParent = GetWindow()->GetWindow( WINDOW_PARENT ); //mpRealParent;
                GetWindow()->mpBorderWindow = NULL;
                if ( mpOldBorderWin )
                {
                    GetWindow()->SetParent( mpOldBorderWin );
                    ((ImplBorderWindow*)mpOldBorderWin)->GetBorder(
                        GetWindow()->mnLeftBorder, GetWindow()->mnTopBorder,
                        GetWindow()->mnRightBorder, GetWindow()->mnBottomBorder );
                    mpOldBorderWin->Resize();
                }
                GetWindow()->mpBorderWindow = mpOldBorderWin;
                GetWindow()->SetParent( pRealParent );
                GetWindow()->mpRealParent = pRealParent;

                delete static_cast<ImplDockFloatWin2*>(mpFloatWin);
                mpFloatWin = NULL;
                GetWindow()->SetPosPixel( maDockPos );

                if ( bVisible )
                    GetWindow()->Show();

                ToggleFloatingMode();

            }
        }
    }
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::SetFloatStyle( WinBits nStyle )
{
    mnFloatBits = nStyle;
}

// -----------------------------------------------------------------------

WinBits ImplDockingWindowWrapper::GetFloatStyle() const
{
    return mnFloatBits;
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::SetTabStop()
{
    GetWindow()->SetStyle( GetWindow()->GetStyle() | (WB_GROUP | WB_TABSTOP) );
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::SetPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     USHORT nFlags )
{
    if ( mpFloatWin )
        mpFloatWin->SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
    else
        GetWindow()->SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

// -----------------------------------------------------------------------

Point ImplDockingWindowWrapper::GetPosPixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetPosPixel();
    else
        return mpDockingWindow->GetPosPixel();
}

// -----------------------------------------------------------------------

Size ImplDockingWindowWrapper::GetSizePixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetSizePixel();
    else
        return mpDockingWindow->GetSizePixel();
}

// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::SetOutputSizePixel( const Size& rNewSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetOutputSizePixel( rNewSize );
    else
        GetWindow()->SetOutputSizePixel( rNewSize );
}

// -----------------------------------------------------------------------

Size ImplDockingWindowWrapper::GetOutputSizePixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetOutputSizePixel();
    else
        return mpDockingWindow->GetOutputSizePixel();
}

Point ImplDockingWindowWrapper::GetFloatingPos() const
{
    if ( mpFloatWin )
    {
        //Rectangle aRect = mpFloatWin->GetWindow( WINDOW_CLIENT)->GetWindowExtentsRelative( mpFloatWin->GetParent() );
        WindowStateData aData;
        aData.SetMask( WINDOWSTATE_MASK_POS );
        mpFloatWin->GetWindowStateData( aData );
        Point aPos( aData.GetX(), aData.GetY() );
        aPos = mpFloatWin->GetParent()->ImplGetFrameWindow()->AbsoluteScreenToOutputPixel( aPos );
        return aPos;
    }
    else
        return maFloatPos;
}

// -----------------------------------------------------------------------
// old inlines from DockingWindow
// -----------------------------------------------------------------------

void ImplDockingWindowWrapper::SetPin( BOOL bPin )
{
    if ( mpFloatWin )
        mpFloatWin->SetPin( bPin );
    mbPined = bPin;
}

BOOL ImplDockingWindowWrapper::IsPined() const
{
    if ( mpFloatWin )
        return mpFloatWin->IsPined();
    return mbPined;
}

void ImplDockingWindowWrapper::RollUp()
{
    if ( mpFloatWin )
        mpFloatWin->RollUp();
    mbRollUp = TRUE;
}

void ImplDockingWindowWrapper::RollDown()
{
    if ( mpFloatWin )
        mpFloatWin->RollDown();
    mbRollUp = FALSE;
}

BOOL ImplDockingWindowWrapper::IsRollUp() const
{
    if ( mpFloatWin )
        return mpFloatWin->IsRollUp();
    return mbRollUp;
}

void ImplDockingWindowWrapper::SetRollUpOutputSizePixel( const Size& rSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetRollUpOutputSizePixel( rSize );
    maRollUpOutSize = rSize;
}

Size ImplDockingWindowWrapper::GetRollUpOutputSizePixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetRollUpOutputSizePixel();
    return maRollUpOutSize;
}

void ImplDockingWindowWrapper::SetMinOutputSizePixel( const Size& rSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetMinOutputSizePixel( rSize );
    maMinOutSize = rSize;
}

const Size& ImplDockingWindowWrapper::GetMinOutputSizePixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetMinOutputSizePixel();
    return maMinOutSize;
}

void ImplDockingWindowWrapper::SetFloatingPos( const Point& rNewPos )
{
    if ( mpFloatWin )
        mpFloatWin->SetPosPixel( rNewPos );
    else
        maFloatPos = rNewPos;
}

BOOL ImplDockingWindowWrapper::IsFloatingMode() const
{
    return (mpFloatWin != NULL);
}


void    ImplDockingWindowWrapper::SetDragArea( const Rectangle& rRect )
{
    maDragArea = rRect;
}

Rectangle  ImplDockingWindowWrapper::GetDragArea() const
{
    return maDragArea;
}

void ImplDockingWindowWrapper::Lock()
{
    mbLocked = TRUE;
    // only toolbars support locking
    ToolBox *pToolBox = dynamic_cast< ToolBox * >( GetWindow() );
    if( pToolBox )
        pToolBox->Lock( mbLocked );
}

void ImplDockingWindowWrapper::Unlock()
{
    mbLocked = FALSE;
    // only toolbars support locking
    ToolBox *pToolBox = dynamic_cast< ToolBox * >( GetWindow() );
    if( pToolBox )
        pToolBox->Lock( mbLocked );
}

BOOL ImplDockingWindowWrapper::IsLocked() const
{
    return mbLocked;
}
