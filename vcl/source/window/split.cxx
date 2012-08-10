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


#include <tools/rc.h>
#include <tools/poly.hxx>

#include <vcl/event.hxx>
#include <vcl/split.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>

#include <rtl/instance.hxx>

#include <window.h>

namespace
{
    struct ImplBlackWall
        : public rtl::StaticWithInit<Wallpaper, ImplBlackWall> {
        Wallpaper operator () () {
            return Wallpaper(COL_BLACK);
        }
    };
    struct ImplWhiteWall
        : public rtl::StaticWithInit<Wallpaper, ImplWhiteWall> {
        Wallpaper operator () () {
            return Wallpaper(COL_LIGHTGRAY);
        }
    };
}

// =======================================================================

void Splitter::ImplInitSplitterData()
{
    ImplGetWindowImpl()->mbSplitter        = sal_True;
    mpRefWin          = NULL;
    mnSplitPos        = 0;
    mnLastSplitPos    = 0;
    mnStartSplitPos   = 0;
    mbDragFull        = sal_False;
    mbKbdSplitting    = sal_False;
    mbInKeyEvent      = 0;
    mnKeyboardStepSize = SPLITTER_DEFAULTSTEPSIZE;
}

// -----------------------------------------------------------------------

void Splitter::ImplInit( Window* pParent, WinBits nWinStyle )
{
    Window::ImplInit( pParent, nWinStyle, NULL );

    mpRefWin = pParent;

    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    long nA = rSettings.GetScrollBarSize();
    long nB = rSettings.GetSplitSize();

    PointerStyle ePointerStyle;

    if ( nWinStyle & WB_HSCROLL )
    {
        ePointerStyle = POINTER_HSPLIT;
        mbHorzSplit = sal_True;
        SetSizePixel( Size( nB, nA ) );
    }
    else
    {
        ePointerStyle = POINTER_VSPLIT;
        mbHorzSplit = sal_False;
        SetSizePixel( Size( nA, nB ) );
    }

    SetPointer( Pointer( ePointerStyle ) );

    if( GetSettings().GetStyleSettings().GetFaceColor().IsDark() )
        SetBackground( ImplWhiteWall::get() );
    else
        SetBackground( ImplBlackWall::get() );

    TaskPaneList *pTList = GetSystemWindow()->GetTaskPaneList();
    pTList->AddWindow( this );
}

// -----------------------------------------------------------------------

void Splitter::ImplSplitMousePos( Point& rPos )
{
    if ( mbHorzSplit )
    {
        if ( rPos.X() > maDragRect.Right()-1 )
            rPos.X() = maDragRect.Right()-1;
        if ( rPos.X() < maDragRect.Left()+1 )
            rPos.X() = maDragRect.Left()+1;
    }
    else
    {
        if ( rPos.Y() > maDragRect.Bottom()-1 )
            rPos.Y() = maDragRect.Bottom()-1;
        if ( rPos.Y() < maDragRect.Top()+1 )
            rPos.Y() = maDragRect.Top()+1;
    }
}

// -----------------------------------------------------------------------

void Splitter::ImplDrawSplitter()
{
    Rectangle aInvRect( maDragRect );

    if ( mbHorzSplit )
    {
        aInvRect.Left()     = maDragPos.X() - 1;
        aInvRect.Right()    = maDragPos.X() + 1;
    }
    else
    {
        aInvRect.Top()      = maDragPos.Y() - 1;
        aInvRect.Bottom()   = maDragPos.Y() + 1;
    }

    mpRefWin->InvertTracking( mpRefWin->PixelToLogic(aInvRect), SHOWTRACK_SPLIT );
}

// -----------------------------------------------------------------------

Splitter::Splitter( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_SPLITTER )
{
    ImplInitSplitterData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

Splitter::Splitter( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_SPLITTER )
{
    ImplInitSplitterData();
    rResId.SetRT( RSC_SPLITTER );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

Splitter::~Splitter()
{
    TaskPaneList *pTList = GetSystemWindow()->GetTaskPaneList();
    pTList->RemoveWindow( this );
}

// -----------------------------------------------------------------------

void Splitter::SetKeyboardStepSize( long nStepSize )
{
    mnKeyboardStepSize = nStepSize;
}

// -----------------------------------------------------------------------

Splitter* Splitter::ImplFindSibling()
{
    // look for another splitter with the same parent but different orientation
    Window *pWin = GetParent()->GetWindow( WINDOW_FIRSTCHILD );
    Splitter *pSplitter = NULL;
    while( pWin )
    {
        if( pWin->ImplIsSplitter() )
        {
            pSplitter = (Splitter*) pWin;
            if( pSplitter != this && IsHorizontal() != pSplitter->IsHorizontal() )
                return pSplitter;
        }
        pWin = pWin->GetWindow( WINDOW_NEXT );
    }
    return NULL;
}

// -----------------------------------------------------------------------

sal_Bool Splitter::ImplSplitterActive()
{
    // is splitter in document or at scrollbar handle ?

    sal_Bool bActive = sal_True;
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    long nA = rSettings.GetScrollBarSize();
    long nB = rSettings.GetSplitSize();

    Size aSize = GetOutputSize();
    if ( mbHorzSplit )
    {
        if( aSize.Width() == nB && aSize.Height() == nA )
            bActive = sal_False;
    }
    else
    {
        if( aSize.Width() == nA && aSize.Height() == nB )
            bActive = sal_False;
    }
    return bActive;
}

// -----------------------------------------------------------------------

void Splitter::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.GetClicks() == 2 )
    {
        if ( mnLastSplitPos != mnSplitPos )
        {
            StartSplit();
            Point aPos = rMEvt.GetPosPixel();
            if ( mbHorzSplit )
                aPos.X() = mnLastSplitPos;
            else
                aPos.Y() = mnLastSplitPos;
            ImplSplitMousePos( aPos );
            Splitting( aPos );
            ImplSplitMousePos( aPos );
            long nTemp = mnSplitPos;
            if ( mbHorzSplit )
                SetSplitPosPixel( aPos.X() );
            else
                SetSplitPosPixel( aPos.Y() );
            mnLastSplitPos = nTemp;
            Split();
            EndSplit();
        }
    }
    else
        StartDrag();
}

// -----------------------------------------------------------------------

void Splitter::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( !mbDragFull )
            ImplDrawSplitter();

        if ( !rTEvt.IsTrackingCanceled() )
        {
            long nNewPos;
            if ( mbHorzSplit )
                nNewPos = maDragPos.X();
            else
                nNewPos = maDragPos.Y();
            if ( nNewPos != mnStartSplitPos )
            {
                SetSplitPosPixel( nNewPos );
                mnLastSplitPos = 0;
                Split();
            }
            EndSplit();
        }
        else if ( mbDragFull )
        {
            SetSplitPosPixel( mnStartSplitPos );
            Split();
        }
        mnStartSplitPos = 0;
    }
    else
    {
        //Point aNewPos = mpRefWin->ScreenToOutputPixel( OutputToScreenPixel( rTEvt.GetMouseEvent().GetPosPixel() ) );
        Point aNewPos = mpRefWin->NormalizedScreenToOutputPixel( OutputToNormalizedScreenPixel( rTEvt.GetMouseEvent().GetPosPixel() ) );
        ImplSplitMousePos( aNewPos );
        Splitting( aNewPos );
        ImplSplitMousePos( aNewPos );

        if ( mbHorzSplit )
        {
            if ( aNewPos.X() == maDragPos.X() )
                return;
        }
        else
        {
            if ( aNewPos.Y() == maDragPos.Y() )
                return;
        }

        if ( mbDragFull )
        {
            maDragPos = aNewPos;
            long nNewPos;
            if ( mbHorzSplit )
                nNewPos = maDragPos.X();
            else
                nNewPos = maDragPos.Y();
            if ( nNewPos != mnSplitPos )
            {
                SetSplitPosPixel( nNewPos );
                mnLastSplitPos = 0;
                Split();
            }

            GetParent()->Update();
        }
        else
        {
            ImplDrawSplitter();
            maDragPos = aNewPos;
            ImplDrawSplitter();
        }
    }
}

// -----------------------------------------------------------------------

void Splitter::ImplKbdTracking( KeyCode aKeyCode )
{
    sal_uInt16 nCode = aKeyCode.GetCode();
    if ( nCode == KEY_ESCAPE || nCode == KEY_RETURN )
    {
        if( !mbKbdSplitting )
            return;
        else
            mbKbdSplitting = sal_False;

        if ( nCode != KEY_ESCAPE )
        {
            long nNewPos;
            if ( mbHorzSplit )
                nNewPos = maDragPos.X();
            else
                nNewPos = maDragPos.Y();
            if ( nNewPos != mnStartSplitPos )
            {
                SetSplitPosPixel( nNewPos );
                mnLastSplitPos = 0;
                Split();
            }
        }
        else
        {
            SetSplitPosPixel( mnStartSplitPos );
            Split();
            EndSplit();
        }
        mnStartSplitPos = 0;
    }
    else
    {
        Point aNewPos;
        Size aSize = mpRefWin->GetOutputSize();
        Point aPos = GetPosPixel();
        // depending on the position calc allows continous moves or snaps to row/columns
        // continous mode is active when position is at the origin or end of the splitter
        // otherwise snap mode is active
        // default here is snap, holding shift sets continous mode
        if( mbHorzSplit )
            aNewPos = Point( ImplSplitterActive() ? aPos.X() : mnSplitPos, aKeyCode.IsShift() ? 0 : aSize.Height()/2);
        else
            aNewPos = Point( aKeyCode.IsShift() ? 0 : aSize.Width()/2, ImplSplitterActive() ? aPos.Y() : mnSplitPos );

        Point aOldWindowPos = GetPosPixel();

        int maxiter = 500;  // avoid endless loop
        int delta=0;
        int delta_step = mbHorzSplit  ? aSize.Width()/10 : aSize.Height()/10;

        // use the specified step size if it was set
        if( mnKeyboardStepSize != SPLITTER_DEFAULTSTEPSIZE )
            delta_step = mnKeyboardStepSize;

        while( maxiter-- && aOldWindowPos == GetPosPixel() )
        {
            // inc/dec position until application performs changes
            // thus a single key press really moves the splitter
            if( aKeyCode.IsShift() )
                delta++;
            else
                delta += delta_step;

            switch( nCode )
            {
            case KEY_LEFT:
                aNewPos.X()-=delta;
                break;
            case KEY_RIGHT:
                aNewPos.X()+=delta;
                break;
            case KEY_UP:
                aNewPos.Y()-=delta;
                break;
            case KEY_DOWN:
                aNewPos.Y()+=delta;
                break;
            default:
                maxiter = 0;    // leave loop
                break;
            }
            ImplSplitMousePos( aNewPos );
            Splitting( aNewPos );
            ImplSplitMousePos( aNewPos );

            if ( mbHorzSplit )
            {
                if ( aNewPos.X() == maDragPos.X() )
                    continue;
            }
            else
            {
                if ( aNewPos.Y() == maDragPos.Y() )
                    continue;
            }

            maDragPos = aNewPos;
            long nNewPos;
            if ( mbHorzSplit )
                nNewPos = maDragPos.X();
            else
                nNewPos = maDragPos.Y();
            if ( nNewPos != mnSplitPos )
            {
                SetSplitPosPixel( nNewPos );
                mnLastSplitPos = 0;
                Split();
            }
            GetParent()->Update();
        }
    }
}

// -----------------------------------------------------------------------

void Splitter::StartSplit()
{
    maStartSplitHdl.Call( this );
}

// -----------------------------------------------------------------------

void Splitter::Split()
{
    maSplitHdl.Call( this );
}

// -----------------------------------------------------------------------

void Splitter::EndSplit()
{
    if ( maEndSplitHdl.IsSet() )
        maEndSplitHdl.Call( this );
}

// -----------------------------------------------------------------------

void Splitter::Splitting( Point& /* rSplitPos */ )
{
}

// -----------------------------------------------------------------------

void Splitter::SetDragRectPixel( const Rectangle& rDragRect, Window* _pRefWin )
{
    maDragRect = rDragRect;
    if ( !_pRefWin )
        mpRefWin = GetParent();
    else
        mpRefWin = _pRefWin;
}

// -----------------------------------------------------------------------

void Splitter::SetSplitPosPixel( long nNewPos )
{
    mnSplitPos = nNewPos;
}

// -----------------------------------------------------------------------

void Splitter::StartDrag()
{
    if ( IsTracking() )
        return;

    StartSplit();

    // Tracking starten
    StartTracking();

    // Start-Positon ermitteln
    maDragPos = mpRefWin->GetPointerPosPixel();
    ImplSplitMousePos( maDragPos );
    Splitting( maDragPos );
    ImplSplitMousePos( maDragPos );
    if ( mbHorzSplit )
        mnStartSplitPos = maDragPos.X();
    else
        mnStartSplitPos = maDragPos.Y();

    mbDragFull = (Application::GetSettings().GetStyleSettings().GetDragFullOptions() & DRAGFULL_OPTION_SPLIT) != 0;
    if ( !mbDragFull )
        ImplDrawSplitter();
}


// -----------------------------------------------------------------------

void Splitter::ImplStartKbdSplitting()
{
    if( mbKbdSplitting )
        return;

    mbKbdSplitting = sal_True;

    StartSplit();

    // determine start position
    // because we have no mouse position we take either the position
    // of the splitter window or the last split position
    // the other coordinate is just the center of the reference window
    Size aSize = mpRefWin->GetOutputSize();
    Point aPos = GetPosPixel();
    if( mbHorzSplit )
        maDragPos = Point( ImplSplitterActive() ? aPos.X() : mnSplitPos, aSize.Height()/2 );
    else
        maDragPos = Point( aSize.Width()/2, ImplSplitterActive() ? aPos.Y() : mnSplitPos );
    ImplSplitMousePos( maDragPos );
    Splitting( maDragPos );
    ImplSplitMousePos( maDragPos );
    if ( mbHorzSplit )
        mnStartSplitPos = maDragPos.X();
    else
        mnStartSplitPos = maDragPos.Y();
}

// -----------------------------------------------------------------------

void Splitter::ImplRestoreSplitter()
{
    // set splitter in the center of the ref window
    StartSplit();
    Size aSize = mpRefWin->GetOutputSize();
    Point aPos = Point( aSize.Width()/2 , aSize.Height()/2);
    if ( mnLastSplitPos != mnSplitPos && mnLastSplitPos > 5 )
    {
        // restore last pos if it was a useful position (>5)
        if ( mbHorzSplit )
            aPos.X() = mnLastSplitPos;
        else
            aPos.Y() = mnLastSplitPos;
    }

    ImplSplitMousePos( aPos );
    Splitting( aPos );
    ImplSplitMousePos( aPos );
    long nTemp = mnSplitPos;
    if ( mbHorzSplit )
        SetSplitPosPixel( aPos.X() );
    else
        SetSplitPosPixel( aPos.Y() );
    mnLastSplitPos = nTemp;
    Split();
    EndSplit();
}


// -----------------------------------------------------------------------

void Splitter::GetFocus()
{
    if( !ImplSplitterActive() )
        ImplRestoreSplitter();

    Invalidate();
}

// -----------------------------------------------------------------------

void Splitter::LoseFocus()
{
    if( mbKbdSplitting )
    {
        KeyCode aReturnKey( KEY_RETURN );
        ImplKbdTracking( aReturnKey );
        mbKbdSplitting = sal_False;
    }
    Invalidate();
}

// -----------------------------------------------------------------------

void Splitter::KeyInput( const KeyEvent& rKEvt )
{
    if( mbInKeyEvent )
        return;

    mbInKeyEvent = 1;

    Splitter *pSibling = ImplFindSibling();
    KeyCode aKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aKeyCode.GetCode();
    switch ( nCode )
    {
        case KEY_UP:
        case KEY_DOWN:
            if( !mbHorzSplit )
            {
                ImplStartKbdSplitting();
                ImplKbdTracking( aKeyCode );
            }
            else
            {
                if( pSibling )
                {
                    pSibling->GrabFocus();
                    pSibling->KeyInput( rKEvt );
                }
            }
            break;
        case KEY_RIGHT:
        case KEY_LEFT:
            if( mbHorzSplit )
            {
                ImplStartKbdSplitting();
                ImplKbdTracking( aKeyCode );
            }
            else
            {
                if( pSibling )
                {
                    pSibling->GrabFocus();
                    pSibling->KeyInput( rKEvt );
                }
            }
            break;

        case KEY_DELETE:
            if( ImplSplitterActive() )
            {
                if( mbKbdSplitting )
                {
                    KeyCode aKey( KEY_ESCAPE );
                    ImplKbdTracking( aKey );
                }

                StartSplit();
                Point aPos;
                if ( mbHorzSplit )
                    aPos.X() = 0;
                else
                    aPos.Y() = 0;
                ImplSplitMousePos( aPos );
                Splitting( aPos );
                ImplSplitMousePos( aPos );
                long nTemp = mnSplitPos;
                if ( mbHorzSplit )
                    SetSplitPosPixel( aPos.X() );
                else
                    SetSplitPosPixel( aPos.Y() );
                mnLastSplitPos = nTemp;
                Split();
                EndSplit();

                // Shift-Del deletes both splitters
                if( aKeyCode.IsShift() && pSibling )
                    pSibling->KeyInput( rKEvt );

                GrabFocusToDocument();
            }
            break;

        case KEY_ESCAPE:
            if( mbKbdSplitting )
                ImplKbdTracking( aKeyCode );
            else
                GrabFocusToDocument();
            break;

        case KEY_RETURN:
            ImplKbdTracking( aKeyCode );
            GrabFocusToDocument();
            break;
        default:    // let any key input fix the splitter
            Window::KeyInput( rKEvt );
            GrabFocusToDocument();
            break;
    }
    mbInKeyEvent = 0;
}

// -----------------------------------------------------------------------

long Splitter::Notify( NotifyEvent& rNEvt )
{
    return Window::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void Splitter::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    if( rDCEvt.GetType() == DATACHANGED_SETTINGS )
    {
        Color oldFaceColor = ((AllSettings *) rDCEvt.GetData())->GetStyleSettings().GetFaceColor();
        Color newFaceColor = Application::GetSettings().GetStyleSettings().GetFaceColor();
        if( oldFaceColor.IsDark() != newFaceColor.IsDark() )
        {
            if( newFaceColor.IsDark() )
                SetBackground( ImplWhiteWall::get() );
            else
                SetBackground( ImplBlackWall::get() );
        }
    }
}

// -----------------------------------------------------------------------

void Splitter::Paint( const Rectangle& rPaintRect )
{
    if( HasFocus() || mbKbdSplitting )
    {
        Color oldFillCol = GetFillColor();
        Color oldLineCol = GetLineColor();

        SetLineColor();
        SetFillColor( GetSettings().GetStyleSettings().GetFaceColor() );
        DrawRect( rPaintRect );

        Color aSelectionBorderCol( GetSettings().GetStyleSettings().GetActiveColor() );
        SetFillColor( aSelectionBorderCol );
        SetLineColor();

        Polygon aPoly( rPaintRect );
        PolyPolygon aPolyPoly( aPoly );
        DrawTransparent( aPolyPoly, 85 );

        SetLineColor( aSelectionBorderCol );
        SetFillColor();

        if( mbKbdSplitting )
        {
            LineInfo aInfo( LINE_DASH );
            //aInfo.SetDashLen( 2 );
            //aInfo.SetDashCount( 1 );
            aInfo.SetDistance( 1 );
            aInfo.SetDotLen( 2 );
            aInfo.SetDotCount( 1 );

            DrawPolyLine( aPoly, aInfo );
        }
        else
            DrawRect( rPaintRect );

        SetFillColor( oldFillCol);
        SetLineColor( oldLineCol);
    }
    else
    {
        Window::Paint( rPaintRect );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
