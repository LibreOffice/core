/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <tools/time.hxx>
#include <tools/rc.h>

#include <brdwin.hxx>
#include <svdata.hxx>
#include <salframe.hxx>
#include <window.h>

#include <vcl/event.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/settings.hxx>




#define DOCKWIN_FLOATSTYLES         (WB_SIZEABLE | WB_MOVEABLE | WB_CLOSEABLE | WB_STANDALONE | WB_PINABLE | WB_ROLLABLE )






class ImplDockFloatWin2 : public FloatingWindow
{
private:
    ImplDockingWindowWrapper*  mpDockWin;
    sal_uLong           mnLastTicks;
    Timer           maDockTimer;
    Timer           maEndDockTimer;
    Point           maDockPos;
    Rectangle       maDockRect;
    bool            mbInMove;
    sal_uLong           mnLastUserEvent;

    DECL_LINK(DockingHdl, void *);
    DECL_LINK(DockTimerHdl, void *);
    DECL_LINK(EndDockTimerHdl, void *);
public:
    ImplDockFloatWin2( Window* pParent, WinBits nWinBits,
                      ImplDockingWindowWrapper* pDockingWin );
    ~ImplDockFloatWin2();

    virtual void    Move();
    virtual void    Resize();
    virtual void    TitleButtonClick( sal_uInt16 nButton );
    virtual void    Pin();
    virtual void    Roll();
    virtual void    PopupModeEnd();
    virtual void    Resizing( Size& rSize );
    virtual bool    Close();
    virtual void    setPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     sal_uInt16 nFlags = WINDOW_POSSIZE_ALL );

    sal_uLong GetLastTicks() const { return mnLastTicks; }
};



ImplDockFloatWin2::ImplDockFloatWin2( Window* pParent, WinBits nWinBits,
                                    ImplDockingWindowWrapper* pDockingWin ) :
        FloatingWindow( pParent, nWinBits ),
        mpDockWin( pDockingWin ),
        mnLastTicks( Time::GetSystemTicks() ),
        mbInMove( false ),
        mnLastUserEvent( 0 )
{
    
    if ( pDockingWin )
    {
        SetSettings( pDockingWin->GetWindow()->GetSettings() );
        Enable( pDockingWin->GetWindow()->IsEnabled(), false );
        EnableInput( pDockingWin->GetWindow()->IsInputEnabled(), false );
        AlwaysEnableInput( pDockingWin->GetWindow()->IsAlwaysEnableInput(), false );
        EnableAlwaysOnTop( pDockingWin->GetWindow()->IsAlwaysOnTopEnabled() );
        SetActivateMode( pDockingWin->GetWindow()->GetActivateMode() );
    }

    SetBackground( GetSettings().GetStyleSettings().GetFaceColor() );

    maDockTimer.SetTimeoutHdl( LINK( this, ImplDockFloatWin2, DockTimerHdl ) );
    maDockTimer.SetTimeout( 50 );
    maEndDockTimer.SetTimeoutHdl( LINK( this, ImplDockFloatWin2, EndDockTimerHdl ) );
    maEndDockTimer.SetTimeout( 50 );
}



ImplDockFloatWin2::~ImplDockFloatWin2()
{
    if( mnLastUserEvent )
        Application::RemoveUserEvent( mnLastUserEvent );
}



IMPL_LINK_NOARG(ImplDockFloatWin2, DockTimerHdl)
{
    DBG_ASSERT( mpDockWin->IsFloatingMode(), "docktimer called but not floating" );

    maDockTimer.Stop();
    PointerState aState = GetPointerState();

    if( aState.mnState & KEY_MOD1 )
    {
        
        mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->HideTracking();
        if( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) )
            maDockTimer.Start();
    }
    else if( ! ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) )
    {
        mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->HideTracking();
        mpDockWin->EndDocking( maDockRect, false );
    }
    else
    {
        mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->ShowTracking( maDockRect, SHOWTRACK_BIG | SHOWTRACK_WINDOW );
        maDockTimer.Start();
    }

    return 0;
}

IMPL_LINK_NOARG(ImplDockFloatWin2, EndDockTimerHdl)
{
    DBG_ASSERT( mpDockWin->IsFloatingMode(), "enddocktimer called but not floating" );

    maEndDockTimer.Stop();
    PointerState aState = GetPointerState();
    if( ! ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) )
    {
        mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->HideTracking();
        mpDockWin->EndDocking( maDockRect, true );
    }
    else
    {
        maEndDockTimer.Start();
    }

    return 0;
}


IMPL_LINK_NOARG(ImplDockFloatWin2, DockingHdl)
{
    
    mnLastUserEvent = 0;

    Window *pDockingArea = mpDockWin->GetWindow()->GetParent();
    PointerState aState = pDockingArea->GetPointerState();

    bool bRealMove = true;
    if( GetStyle() & WB_OWNERDRAWDECORATION )
    {
        
        
        
        
        Window *pBorder = GetWindow( WINDOW_BORDER );
        if( pBorder != this )
        {
            Point aPt;
            Rectangle aBorderRect( aPt, pBorder->GetSizePixel() );
            sal_Int32 nLeft, nTop, nRight, nBottom;
            GetBorder( nLeft, nTop, nRight, nBottom );
            
            aBorderRect.Bottom() = aBorderRect.Top() + nTop;
            aBorderRect.Left() += nLeft;
            aBorderRect.Right() -= nRight;

            PointerState aBorderState = pBorder->GetPointerState();
            if( aBorderRect.IsInside( aBorderState.maPos ) )
                bRealMove = true;
            else
                bRealMove = false;
        }
    }

    if( mpDockWin->IsDockable() &&
        mpDockWin->GetWindow()->IsVisible() &&
        (Time::GetSystemTicks() - mnLastTicks > 500) &&
        ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) &&
        !(aState.mnState & KEY_MOD1) && 
        bRealMove )
    {
        maDockPos = Point( pDockingArea->OutputToScreenPixel( pDockingArea->AbsoluteScreenToOutputPixel( OutputToAbsoluteScreenPixel( Point() ) ) ) );
        maDockRect = Rectangle( maDockPos, mpDockWin->GetSizePixel() );

        
        Point aMousePos = pDockingArea->OutputToScreenPixel( aState.maPos );

        if( ! mpDockWin->IsDocking() )
            mpDockWin->StartDocking( aMousePos, maDockRect );

        bool bFloatMode = mpDockWin->Docking( aMousePos, maDockRect );

        if( ! bFloatMode )
        {
            
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
    mbInMove = false;
    return 0;
}


void ImplDockFloatWin2::Move()
{
    if( mbInMove )
        return;

    mbInMove = true;
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



void ImplDockFloatWin2::Resize()
{
    
    if( GetWindow( WINDOW_BORDER ) == this )
    {
        FloatingWindow::Resize();
        Size aSize( GetSizePixel() );
        mpDockWin->GetWindow()->ImplPosSizeWindow( 0, 0, aSize.Width(), aSize.Height(), WINDOW_POSSIZE_POSSIZE ); 
    }
}

void ImplDockFloatWin2::setPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     sal_uInt16 nFlags )
{
    FloatingWindow::setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}




void ImplDockFloatWin2::TitleButtonClick( sal_uInt16 nButton )
{
    FloatingWindow::TitleButtonClick( nButton );
    mpDockWin->TitleButtonClick( nButton );
}



void ImplDockFloatWin2::Pin()
{
    FloatingWindow::Pin();
    mpDockWin->Pin();
}



void ImplDockFloatWin2::Roll()
{
    FloatingWindow::Roll();
    mpDockWin->Roll();
}



void ImplDockFloatWin2::PopupModeEnd()
{
    FloatingWindow::PopupModeEnd();
    mpDockWin->PopupModeEnd();
}



void ImplDockFloatWin2::Resizing( Size& rSize )
{
    FloatingWindow::Resizing( rSize );
    mpDockWin->Resizing( rSize );
}



bool ImplDockFloatWin2::Close()
{
    return mpDockWin->Close();
}



DockingManager::DockingManager()
{
}

DockingManager::~DockingManager()
{
    ::std::vector< ImplDockingWindowWrapper* >::iterator p;
    p = mDockingWindows.begin();
    for(; p != mDockingWindows.end(); ++p )
    {
        delete (*p);
    }
    mDockingWindows.clear();
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
            ++p;
    }
    return NULL;
}

bool DockingManager::IsDockable( const Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );

    /*
    if( pWindow->HasDockingHandler() )
        return sal_True;
    */
    return (pWrapper != NULL);
}

bool DockingManager::IsFloating( const Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        return pWrapper->IsFloatingMode();
    else
        return false;
}

bool DockingManager::IsLocked( const Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper && pWrapper->IsLocked() )
        return true;
    else
        return false;
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

void DockingManager::SetFloatingMode( const Window *pWindow, bool bFloating )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->SetFloatingMode( bFloating );
}

void DockingManager::StartPopupMode( ToolBox *pParentToolBox, const Window *pWindow, sal_uLong nFlags )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->StartPopupMode( pParentToolBox, nFlags );
}

void DockingManager::StartPopupMode( ToolBox *pParentToolBox, const Window *pWindow )
{
    StartPopupMode( pParentToolBox, pWindow, FLOATWIN_POPUPMODE_ALLOWTEAROFF         |
                    FLOATWIN_POPUPMODE_NOFOCUSCLOSE         |
                    FLOATWIN_POPUPMODE_ALLMOUSEBUTTONCLOSE  |
                    FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE );
}

bool DockingManager::IsInPopupMode( const Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper && pWrapper->IsInPopupMode() )
        return true;
    else
        return false;
}



void DockingManager::EndPopupMode( const Window *pWin )
{
    ImplDockingWindowWrapper *pWrapper = GetDockingWindowWrapper( pWin );
    if( pWrapper && pWrapper->GetFloatingWindow() && pWrapper->GetFloatingWindow()->IsInPopupMode() )
        pWrapper->GetFloatingWindow()->EndPopupMode();
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
            ++p;
    }
}

void DockingManager::SetPosSizePixel( Window *pWindow, long nX, long nY,
                                    long nWidth, long nHeight,
                                    sal_uInt16 nFlags )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

Rectangle DockingManager::GetPosSizePixel( const Window *pWindow )
{
    Rectangle aRect;
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        aRect = Rectangle( pWrapper->GetPosPixel(), pWrapper->GetSizePixel() );

    return aRect;
}











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
    bool                        mbHighlight;
    bool                        mbMoving;
    bool                        mbTrackingEnabled;
    Point                       maDelta;
    Point                       maTearOffPosition;
    bool                        mbGripAtBottom;
    bool                        mbHasGrip;
    void                        ImplSetBorder();

public:
    ImplPopupFloatWin( Window* pParent, ImplDockingWindowWrapper* pDockingWin, bool bHasGrip );
    ~ImplPopupFloatWin();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
    virtual void        Paint( const Rectangle& rRect );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void        Tracking( const TrackingEvent& rTEvt );
    virtual void        Resize();
    virtual Window*     GetPreferredKeyInputWindow();

    Rectangle           GetDragRect() const;
    Point               GetToolboxPosition() const;
    Point               GetTearOffPosition() const;
    void                DrawGrip();
    void                DrawBorder();

    bool                hasGrip() const { return mbHasGrip; }
};

ImplPopupFloatWin::ImplPopupFloatWin( Window* pParent, ImplDockingWindowWrapper* pDockingWin, bool bHasGrip ) :
    FloatingWindow( pParent, WB_NOBORDER | WB_SYSTEMWINDOW | WB_NOSHADOW)
{
    mpWindowImpl->mbToolbarFloatingWindow = true;   
                                                    
    mpDockingWin = pDockingWin;
    mbHighlight = false;
    mbMoving = false;
    mbTrackingEnabled = false;
    mbGripAtBottom = true;
    mbHasGrip = bHasGrip;

    ImplSetBorder();
}

ImplPopupFloatWin::~ImplPopupFloatWin()
{
    mpDockingWin = NULL;
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > ImplPopupFloatWin::CreateAccessible()
{
    

    
    
    
    
    return ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >();
}

Window* ImplPopupFloatWin::GetPreferredKeyInputWindow()
{
    if( mpWindowImpl->mpClientWindow )
        return mpWindowImpl->mpClientWindow;
    else
        return FloatingWindow::GetPreferredKeyInputWindow();
}


void ImplPopupFloatWin::ImplSetBorder()
{
    
    
    
    
    mpWindowImpl->mnTopBorder     = 1;
    if( hasGrip() )
        mpWindowImpl->mnTopBorder += POPUP_DRAGHEIGHT+2;
    mpWindowImpl->mnBottomBorder  = 1;
    mpWindowImpl->mnLeftBorder    = 1;
    mpWindowImpl->mnRightBorder   = 1;
}

void ImplPopupFloatWin::Resize()
{
    
    ImplSetBorder();
}

Rectangle ImplPopupFloatWin::GetDragRect() const
{
    Rectangle aRect;
    if( hasGrip() )
    {
        aRect = Rectangle( 1,1, GetOutputSizePixel().Width()-1, 2+POPUP_DRAGHEIGHT );
        if( mbGripAtBottom )
        {
            int height = GetOutputSizePixel().Height();
            aRect.Top() = height - 3 - POPUP_DRAGHEIGHT;
            aRect.Bottom() = aRect.Top() + 1 + POPUP_DRAGHEIGHT;
        }
    }
    return aRect;
}

Point ImplPopupFloatWin::GetToolboxPosition() const
{
    
    Point aPt( 1, 1 + ((mbGripAtBottom || !hasGrip()) ? 0 : GetDragRect().getHeight()) );    

    return aPt;
}

Point ImplPopupFloatWin::GetTearOffPosition() const
{
    Point aPt( maTearOffPosition );
    
    return aPt;
}

void ImplPopupFloatWin::DrawBorder()
{
    SetFillColor();
    Point aPt;
    Rectangle aRect( aPt, GetOutputSizePixel() );

    Region oldClipRgn( GetClipRegion( ) );
    Region aClipRgn( aRect );
    Rectangle aItemClipRect( ImplGetItemEdgeClipRect() );
    if( !aItemClipRect.IsEmpty() )
    {
        aItemClipRect.SetPos( AbsoluteScreenToOutputPixel( aItemClipRect.TopLeft() ) );

        
        SetClipRegion( Region( aItemClipRect ) );
        SetLineColor( GetSettings().GetStyleSettings().GetFaceColor() );
        DrawRect( aRect );

        aClipRgn.Exclude( aItemClipRect );
        SetClipRegion( aClipRgn );
    }
    SetLineColor( GetSettings().GetStyleSettings().GetShadowColor() );
    DrawRect( aRect );
    SetClipRegion( oldClipRgn );
}

void ImplPopupFloatWin::DrawGrip()
{
    bool bLinecolor     = IsLineColor();
    Color aLinecolor    = GetLineColor();
    bool bFillcolor     = IsFillColor();
    Color aFillcolor    = GetFillColor();

    
    Rectangle aRect( GetDragRect() );
    aRect.Top()      += POPUP_DRAGBORDER;
    aRect.Bottom()   -= POPUP_DRAGBORDER;
    aRect.Left()+=3;
    aRect.Right()-=3;

    if( mbHighlight )
    {
        Erase( aRect );
        DrawSelectionBackground( aRect, 2, false, true, false );
    }
    else
    {
        SetFillColor( GetSettings().GetStyleSettings().GetFaceColor() );
        SetLineColor();
        DrawRect( aRect );
    }

    if( !ToolBox::AlwaysLocked() )  
    {
#ifdef TEAROFF_DASHED
        
        LineInfo aLineInfo( LINE_DASH );
        aLineInfo.SetDistance( 4 );
        aLineInfo.SetDashLen( 12 );
        aLineInfo.SetDashCount( 1 );

        aRect.Left()+=2;
        aRect.Right()-=2;

        aRect.Top()+=2;
        aRect.Bottom() = aRect.Top();
        SetLineColor( GetSettings().GetStyleSettings().GetDarkShadowColor() );
        DrawLine( aRect.TopLeft(), aRect.TopRight(), aLineInfo );

        if( !mbHighlight )
        {
            ++aRect.Top();
            ++aRect.Bottom();
            SetLineColor( GetSettings().GetStyleSettings().GetLightColor() );
            DrawLine( aRect.TopLeft(), aRect.TopRight(), aLineInfo );
        }

#else
        
        SetFillColor( GetSettings().GetStyleSettings().GetShadowColor() );
        aRect.Top()++;
        aRect.Bottom() = aRect.Top();

        int width = POPUP_DRAGWIDTH;
        while( width >= aRect.getWidth() )
            width -= 4;
        if( width <= 0 )
            width = aRect.getWidth();
        
        aRect.Left() = (aRect.Left() + aRect.Right() - width) / 2;
        aRect.Right() = aRect.Left() + width;

        int i=0;
        while( i< POPUP_DRAGGRIP )
        {
            DrawRect( aRect );
            aRect.Top()+=2;
            aRect.Bottom()+=2;
            i+=2;
        }
#endif
    }

    if( bLinecolor )
        SetLineColor( aLinecolor );
    else
        SetLineColor();
    if( bFillcolor )
        SetFillColor( aFillcolor );
    else
        SetFillColor();
}

void ImplPopupFloatWin::Paint( const Rectangle& )
{
    Point aPt;
    Rectangle aRect( aPt, GetOutputSizePixel() );
    DrawWallpaper( aRect, Wallpaper( GetSettings().GetStyleSettings().GetFaceGradientColor() ) );
    DrawBorder();
    if( hasGrip() )
        DrawGrip();
}

void ImplPopupFloatWin::MouseMove( const MouseEvent& rMEvt )
{
    Point aMousePos = rMEvt.GetPosPixel();

    if( !ToolBox::AlwaysLocked() )  
    {
        if( mbTrackingEnabled && rMEvt.IsLeft() && GetDragRect().IsInside( aMousePos ) )
        {
            
            mbMoving = true;
            StartTracking( STARTTRACK_NOKEYCANCEL );
            return;
        }
        if( !mbHighlight && GetDragRect().IsInside( aMousePos ) )
        {
            mbHighlight = true;
            DrawGrip();
        }
        if( mbHighlight && ( rMEvt.IsLeaveWindow() || !GetDragRect().IsInside( aMousePos ) ) )
        {
            mbHighlight = false;
            DrawGrip();
        }
    }
}

void ImplPopupFloatWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    mbTrackingEnabled = false;
    FloatingWindow::MouseButtonUp( rMEvt );
}

void ImplPopupFloatWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aMousePos = rMEvt.GetPosPixel();
    if( GetDragRect().IsInside( aMousePos ) )
    {
        
        PointerState aState = GetParent()->GetPointerState();
        if (HasMirroredGraphics() && IsRTLEnabled())
            ImplMirrorFramePos(aState.maPos);
        maTearOffPosition = GetWindow( WINDOW_BORDER )->GetPosPixel();
        maDelta = aState.maPos - maTearOffPosition;
        mbTrackingEnabled = true;
    }
    else
    {
        mbTrackingEnabled = false;
    }
}

void ImplPopupFloatWin::Tracking( const TrackingEvent& rTEvt )
{
    if( mbMoving )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbMoving = false;
            EndPopupMode( FLOATWIN_POPUPMODEEND_TEAROFF );
        }
        else if ( !rTEvt.GetMouseEvent().IsSynthetic() )
        {
            
            PointerState aState = GetParent()->GetPointerState();
            const OutputDevice *pOutDev = GetOutDev();
            if (pOutDev->HasMirroredGraphics() && IsRTLEnabled())
                ImplMirrorFramePos(aState.maPos);
            maTearOffPosition = aState.maPos - maDelta;
            GetWindow( WINDOW_BORDER )->SetPosPixel( maTearOffPosition );
        }
    }
}




ImplDockingWindowWrapper::ImplDockingWindowWrapper( const Window *pWindow )
{
    ImplInitData();

    mpDockingWindow = (Window*) pWindow;
    mpParent        = pWindow->GetParent();
    mbDockable      = true;
    mbLocked        = false;
    mnFloatBits     = WB_BORDER | WB_CLOSEABLE | WB_SIZEABLE | (pWindow->GetStyle() & DOCKWIN_FLOATSTYLES);
    DockingWindow *pDockWin = dynamic_cast< DockingWindow* > ( mpDockingWindow );
    if( pDockWin )
        mnFloatBits = pDockWin->GetFloatStyle();

    
    mbStartDockingEnabled = false;
}

ImplDockingWindowWrapper::~ImplDockingWindowWrapper()
{
    if ( IsFloatingMode() )
    {
        GetWindow()->Show( false, SHOW_NOFOCUSCHANGE );
        SetFloatingMode( false );
    }
}



bool ImplDockingWindowWrapper::ImplStartDocking( const Point& rPos )
{
    if ( !mbDockable )
        return false;

    if( !mbStartDockingEnabled )
        return false;

    maMouseOff      = rPos;
    maMouseStart    = maMouseOff;
    mbDocking       = true;
    mbLastFloatMode = IsFloatingMode();
    mbStartFloat    = mbLastFloatMode;

    
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

    
    Point aMousePos = pDockingArea->OutputToScreenPixel( aState.maPos );
    Point aDockPos = Point( pDockingArea->AbsoluteScreenToOutputPixel( GetWindow()->OutputToAbsoluteScreenPixel( GetWindow()->GetPosPixel() ) ) );
    Rectangle aDockRect( aDockPos, GetWindow()->GetSizePixel() );
    StartDocking( aMousePos, aDockRect );

    GetWindow()->ImplUpdateAll();
    GetWindow()->ImplGetFrameWindow()->ImplUpdateAll();

    GetWindow()->StartTracking( STARTTRACK_KEYMOD );
    return true;
}



void ImplDockingWindowWrapper::ImplInitData()
{
    mpDockingWindow     = NULL;

    
    mpFloatWin          = NULL;
    mbDockCanceled      = false;
    mbFloatPrevented    = false;
    mbDocking           = false;
    mbPined             = false;
    mbRollUp            = false;
    mbDockBtn           = false;
    mbHideBtn           = false;
    maMaxOutSize        = Size( SHRT_MAX, SHRT_MAX );
}



void ImplDockingWindowWrapper::Tracking( const TrackingEvent& rTEvt )
{
    
    if ( mbDocking )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbDocking = false;
            GetWindow()->HideTracking();
            if ( rTEvt.IsTrackingCanceled() )
            {
                mbDockCanceled = true;
                EndDocking( Rectangle( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) ), mbLastFloatMode );
                mbDockCanceled = false;
            }
            else
                EndDocking( Rectangle( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) ), mbLastFloatMode );
        }
        
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

            bool bFloatMode = Docking( aPos, aTrackRect );

            mbFloatPrevented = false;
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

            sal_uInt16 nTrackStyle;
            if ( bFloatMode )
                nTrackStyle = SHOWTRACK_OBJECT;
            else
                nTrackStyle = SHOWTRACK_BIG;
            Rectangle aShowTrackRect = aTrackRect;
            aShowTrackRect.SetPos( GetWindow()->ImplFrameToOutput( aShowTrackRect.TopLeft() ) );

            GetWindow()->ShowTracking( aShowTrackRect, nTrackStyle );

            
            
            maMouseOff.X()  = aPos.X() - aTrackRect.Left();
            maMouseOff.Y()  = aPos.Y() - aTrackRect.Top();

            mnTrackX        = aTrackRect.Left();
            mnTrackY        = aTrackRect.Top();
            mnTrackWidth    = aTrackRect.GetWidth();
            mnTrackHeight   = aTrackRect.GetHeight();
        }
    }
}




void ImplDockingWindowWrapper::StartDocking( const Point& rPoint, Rectangle& rRect )
{
    DockingData data( rPoint, rRect, IsFloatingMode() );

    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_STARTDOCKING, &data );
    mbDocking = true;
}



bool ImplDockingWindowWrapper::Docking( const Point& rPoint, Rectangle& rRect )
{
    DockingData data( rPoint, rRect, IsFloatingMode() );

    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_DOCKING, &data );
    rRect = data.maTrackRect;
    return data.mbFloating;
}



void ImplDockingWindowWrapper::EndDocking( const Rectangle& rRect, bool bFloatMode )
{
    Rectangle aRect( rRect );

    if ( !IsDockingCanceled() )
    {
        bool bShow = false;
        if ( bFloatMode != IsFloatingMode() )
        {
            GetWindow()->Show( false, SHOW_NOFOCUSCHANGE );
            SetFloatingMode( bFloatMode );
            bShow = true;
            if ( bFloatMode )
            {
                
                mpFloatWin->SetOutputSizePixel( aRect.GetSize() );
                mpFloatWin->SetPosPixel( aRect.TopLeft() );
            }
        }
        if ( !bFloatMode )
        {
            Point aPos = aRect.TopLeft();
            aPos = GetWindow()->GetParent()->ScreenToOutputPixel( aPos );
            GetWindow()->SetPosSizePixel( aPos, aRect.GetSize() );
        }

        if ( bShow )
            GetWindow()->Show( true, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
    }

    EndDockingData data( aRect, IsFloatingMode(), IsDockingCanceled() );
    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_ENDDOCKING, &data );

    mbDocking = false;

    
    mbStartDockingEnabled = false;
}



bool ImplDockingWindowWrapper::PrepareToggleFloatingMode()
{
    sal_Bool bFloating = sal_True;
    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_PREPARETOGGLEFLOATING, &bFloating );
    return bFloating;
}



bool ImplDockingWindowWrapper::Close()
{
    
    return true;
}



void ImplDockingWindowWrapper::ToggleFloatingMode()
{
    
    
    
    if( GetWindow()->ImplIsDockingWindow() )
        ((DockingWindow*) GetWindow())->ToggleFloatingMode();

    
    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_TOGGLEFLOATING );

    
    mbStartDockingEnabled = false;
}



void ImplDockingWindowWrapper::TitleButtonClick( sal_uInt16 nType )
{
    if( nType == TITLE_BUTTON_MENU )
    {
        ToolBox *pToolBox = dynamic_cast< ToolBox* >( GetWindow() );
        if( pToolBox )
        {
            pToolBox->ExecuteCustomMenu();
        }
    }
    if( nType == TITLE_BUTTON_DOCKING )
    {
        SetFloatingMode( !IsFloatingMode() );
    }
}



void ImplDockingWindowWrapper::Pin()
{
    
}



void ImplDockingWindowWrapper::Roll()
{
    
}



void ImplDockingWindowWrapper::PopupModeEnd()
{
    
}



void ImplDockingWindowWrapper::Resizing( Size& rSize )
{
    
    DockingWindow *pDockingWindow = dynamic_cast< DockingWindow* >( GetWindow() );
    if( pDockingWindow )
        pDockingWindow->Resizing( rSize );
}



void ImplDockingWindowWrapper::ShowTitleButton( sal_uInt16 nButton, bool bVisible )
{
    if ( mpFloatWin )
        mpFloatWin->ShowTitleButton( nButton, bVisible );
    else
    {
        if ( nButton == TITLE_BUTTON_DOCKING )
            mbDockBtn = bVisible;
        else 
            mbHideBtn = bVisible;
    }
}



void ImplDockingWindowWrapper::StartPopupMode( ToolBox *pParentToolBox, sal_uLong nFlags )
{
    
    if( IsFloatingMode() )
        return;

    GetWindow()->Show( false, SHOW_NOFOCUSCHANGE );

    
    Window* pRealParent = GetWindow()->GetWindow( WINDOW_PARENT );
    mpOldBorderWin = GetWindow()->GetWindow( WINDOW_BORDER );
    if( mpOldBorderWin == GetWindow() )
        mpOldBorderWin = NULL;  

    
    ImplPopupFloatWin* pWin = new ImplPopupFloatWin( mpParent, this, (nFlags & FLOATWIN_POPUPMODE_ALLOWTEAROFF) != 0 );

    pWin->SetPopupModeEndHdl( LINK( this, ImplDockingWindowWrapper, PopupModeEnd ) );
    pWin->SetText( GetWindow()->GetText() );

    pWin->SetOutputSizePixel( GetWindow()->GetSizePixel() );

    GetWindow()->mpWindowImpl->mpBorderWindow  = NULL;
    GetWindow()->mpWindowImpl->mnLeftBorder    = 0;
    GetWindow()->mpWindowImpl->mnTopBorder     = 0;
    GetWindow()->mpWindowImpl->mnRightBorder   = 0;
    GetWindow()->mpWindowImpl->mnBottomBorder  = 0;

    
    GetWindow()->SetPosPixel( pWin->GetToolboxPosition() );

    
    if ( mpOldBorderWin )
        mpOldBorderWin->SetParent( pWin );
    GetWindow()->SetParent( pWin );

    
    GetWindow()->mpWindowImpl->mpBorderWindow = pWin;
    pWin->mpWindowImpl->mpClientWindow = GetWindow();
    GetWindow()->mpWindowImpl->mpRealParent = pRealParent;

    
    
    mpFloatWin = pWin;

    
    
    if( pParentToolBox->IsKeyEvent() )
        nFlags |= FLOATWIN_POPUPMODE_GRABFOCUS;

    mpFloatWin->StartPopupMode( pParentToolBox, nFlags );
    GetWindow()->Show();

    if( pParentToolBox->IsKeyEvent() )
    {
        
        KeyEvent aEvent( 0, KeyCode( KEY_HOME ) );
        mpFloatWin->GetPreferredKeyInputWindow()->KeyInput( aEvent );
    }
}

IMPL_LINK_NOARG(ImplDockingWindowWrapper, PopupModeEnd)
{
    GetWindow()->Show( false, SHOW_NOFOCUSCHANGE );

    
    ImplPopupFloatWin *pPopupFloatWin = (ImplPopupFloatWin*) mpFloatWin;
    EndPopupModeData aData( pPopupFloatWin->GetTearOffPosition(), mpFloatWin->IsPopupModeTearOff() );

    
    Window* pRealParent = GetWindow()->GetWindow( WINDOW_PARENT );
    GetWindow()->mpWindowImpl->mpBorderWindow = NULL;
    if ( mpOldBorderWin )
    {
        GetWindow()->SetParent( mpOldBorderWin );
        ((ImplBorderWindow*)mpOldBorderWin)->GetBorder(
            GetWindow()->mpWindowImpl->mnLeftBorder, GetWindow()->mpWindowImpl->mnTopBorder,
            GetWindow()->mpWindowImpl->mnRightBorder, GetWindow()->mpWindowImpl->mnBottomBorder );
        mpOldBorderWin->Resize();
    }
    GetWindow()->mpWindowImpl->mpBorderWindow = mpOldBorderWin;
    GetWindow()->SetParent( pRealParent );
    GetWindow()->mpWindowImpl->mpRealParent = pRealParent;

    delete mpFloatWin;
    mpFloatWin = NULL;

    
    GetWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_ENDPOPUPMODE, &aData );

    return 0;
}


bool ImplDockingWindowWrapper::IsInPopupMode() const
{
    if( GetFloatingWindow() )
        return GetFloatingWindow()->IsInPopupMode();
    else
        return false;
}



void ImplDockingWindowWrapper::SetFloatingMode( bool bFloatMode )
{
    
    if( !IsFloatingMode() && IsLocked() )
        return;

    if ( IsFloatingMode() != bFloatMode )
    {
        if ( PrepareToggleFloatingMode() )
        {
            bool bVisible = GetWindow()->IsVisible();

            if ( bFloatMode )
            {
                GetWindow()->Show( false, SHOW_NOFOCUSCHANGE );

                maDockPos = GetWindow()->GetPosPixel();

                Window* pRealParent = GetWindow()->GetWindow( WINDOW_PARENT );
                mpOldBorderWin = GetWindow()->GetWindow( WINDOW_BORDER );
                if( mpOldBorderWin == mpDockingWindow )
                    mpOldBorderWin = NULL;  

                ImplDockFloatWin2* pWin =
                    new ImplDockFloatWin2(
                                         mpParent,
                                         mnFloatBits & ( WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) ?
                                          mnFloatBits | WB_SYSTEMWINDOW
                                          | WB_OWNERDRAWDECORATION
                                          : mnFloatBits,
                                         this );

                GetWindow()->mpWindowImpl->mpBorderWindow  = NULL;
                GetWindow()->mpWindowImpl->mnLeftBorder    = 0;
                GetWindow()->mpWindowImpl->mnTopBorder     = 0;
                GetWindow()->mpWindowImpl->mnRightBorder   = 0;
                GetWindow()->mpWindowImpl->mnBottomBorder  = 0;

                
                
                if ( mpOldBorderWin )
                    mpOldBorderWin->SetParent( pWin );
                GetWindow()->SetParent( pWin );
                pWin->SetPosPixel( Point() );

                GetWindow()->mpWindowImpl->mpBorderWindow = pWin;
                pWin->mpWindowImpl->mpClientWindow = mpDockingWindow;
                GetWindow()->mpWindowImpl->mpRealParent = pRealParent;

                pWin->SetText( GetWindow()->GetText() );
                pWin->SetOutputSizePixel( GetWindow()->GetSizePixel() );
                pWin->SetPosPixel( maFloatPos );
                
                pWin->ShowTitleButton( TITLE_BUTTON_DOCKING, mbDockBtn );
                pWin->ShowTitleButton( TITLE_BUTTON_HIDE, mbHideBtn );
                pWin->SetPin( mbPined );
                if ( mbRollUp )
                    pWin->RollUp();
                else
                    pWin->RollDown();
                pWin->SetRollUpOutputSizePixel( maRollUpOutSize );
                pWin->SetMinOutputSizePixel( maMinOutSize );
                pWin->SetMaxOutputSizePixel( maMaxOutSize );

                mpFloatWin      = pWin;

                if ( bVisible )
                    GetWindow()->Show( true, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );

                ToggleFloatingMode();
            }
            else
            {
                GetWindow()->Show( false, SHOW_NOFOCUSCHANGE );

                
                maFloatPos      = mpFloatWin->GetPosPixel();
                mbDockBtn       = mpFloatWin->IsTitleButtonVisible( TITLE_BUTTON_DOCKING );
                mbHideBtn       = mpFloatWin->IsTitleButtonVisible( TITLE_BUTTON_HIDE );
                mbPined         = mpFloatWin->IsPined();
                mbRollUp        = mpFloatWin->IsRollUp();
                maRollUpOutSize = mpFloatWin->GetRollUpOutputSizePixel();
                maMinOutSize    = mpFloatWin->GetMinOutputSizePixel();
                maMaxOutSize    = mpFloatWin->GetMaxOutputSizePixel();

                Window* pRealParent = GetWindow()->GetWindow( WINDOW_PARENT ); 
                GetWindow()->mpWindowImpl->mpBorderWindow = NULL;
                if ( mpOldBorderWin )
                {
                    GetWindow()->SetParent( mpOldBorderWin );
                    ((ImplBorderWindow*)mpOldBorderWin)->GetBorder(
                        GetWindow()->mpWindowImpl->mnLeftBorder, GetWindow()->mpWindowImpl->mnTopBorder,
                        GetWindow()->mpWindowImpl->mnRightBorder, GetWindow()->mpWindowImpl->mnBottomBorder );
                    mpOldBorderWin->Resize();
                }
                GetWindow()->mpWindowImpl->mpBorderWindow = mpOldBorderWin;
                GetWindow()->SetParent( pRealParent );
                GetWindow()->mpWindowImpl->mpRealParent = pRealParent;

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



void ImplDockingWindowWrapper::SetFloatStyle( WinBits nStyle )
{
    mnFloatBits = nStyle;
}



WinBits ImplDockingWindowWrapper::GetFloatStyle() const
{
    return mnFloatBits;
}



void ImplDockingWindowWrapper::setPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     sal_uInt16 nFlags )
{
    if ( mpFloatWin )
        mpFloatWin->setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
    else
        GetWindow()->setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}



Point ImplDockingWindowWrapper::GetPosPixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetPosPixel();
    else
        return mpDockingWindow->GetPosPixel();
}



Size ImplDockingWindowWrapper::GetSizePixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetSizePixel();
    else
        return mpDockingWindow->GetSizePixel();
}





void ImplDockingWindowWrapper::SetMinOutputSizePixel( const Size& rSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetMinOutputSizePixel( rSize );
    maMinOutSize = rSize;
}

void ImplDockingWindowWrapper::SetMaxOutputSizePixel( const Size& rSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetMaxOutputSizePixel( rSize );
    maMaxOutSize = rSize;
}

bool ImplDockingWindowWrapper::IsFloatingMode() const
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
    mbLocked = true;
    
    ToolBox *pToolBox = dynamic_cast< ToolBox * >( GetWindow() );
    if( pToolBox )
        pToolBox->Lock( mbLocked );
}

void ImplDockingWindowWrapper::Unlock()
{
    mbLocked = false;
    
    ToolBox *pToolBox = dynamic_cast< ToolBox * >( GetWindow() );
    if( pToolBox )
        pToolBox->Lock( mbLocked );
}

bool ImplDockingWindowWrapper::IsLocked() const
{
    return mbLocked;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
