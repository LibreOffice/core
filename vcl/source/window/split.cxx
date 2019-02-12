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

#include <tools/poly.hxx>

#include <vcl/event.hxx>
#include <vcl/split.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>

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

// Should only be called from a ImplInit method for initialization or
// after checking bNew is different from the current mbHorzSplit value.
// The public method that does that check is Splitter::SetHorizontal().
void Splitter::ImplInitHorVer(bool bNew)
{
    mbHorzSplit = bNew;

    PointerStyle ePointerStyle;
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();

    if ( mbHorzSplit )
    {
        ePointerStyle = PointerStyle::HSplit;
        SetSizePixel( Size( StyleSettings::GetSplitSize(), rSettings.GetScrollBarSize() ) );
    }
    else
    {
        ePointerStyle = PointerStyle::VSplit;
        SetSizePixel( Size( rSettings.GetScrollBarSize(), StyleSettings::GetSplitSize() ) );
    }

    SetPointer( ePointerStyle );
}

void Splitter::ImplInit( vcl::Window* pParent, WinBits nWinStyle )
{
    Window::ImplInit( pParent, nWinStyle, nullptr );

    mpRefWin = pParent;

    ImplInitHorVer(nWinStyle & WB_HSCROLL);

    if( GetSettings().GetStyleSettings().GetFaceColor().IsDark() )
        SetBackground( ImplWhiteWall::get() );
    else
        SetBackground( ImplBlackWall::get() );

    TaskPaneList *pTList = GetSystemWindow()->GetTaskPaneList();
    pTList->AddWindow( this );
}

void Splitter::ImplSplitMousePos( Point& rPos )
{
    if ( mbHorzSplit )
    {
        if ( rPos.X() > maDragRect.Right()-1 )
            rPos.setX( maDragRect.Right()-1 );
        if ( rPos.X() < maDragRect.Left()+1 )
            rPos.setX( maDragRect.Left()+1 );
    }
    else
    {
        if ( rPos.Y() > maDragRect.Bottom()-1 )
            rPos.setY( maDragRect.Bottom()-1 );
        if ( rPos.Y() < maDragRect.Top()+1 )
            rPos.setY( maDragRect.Top()+1 );
    }
}

void Splitter::ImplDrawSplitter()
{
    tools::Rectangle aInvRect( maDragRect );

    if ( mbHorzSplit )
    {
        aInvRect.SetLeft( maDragPos.X() - 1 );
        aInvRect.SetRight( maDragPos.X() + 1 );
    }
    else
    {
        aInvRect.SetTop( maDragPos.Y() - 1 );
        aInvRect.SetBottom( maDragPos.Y() + 1 );
    }

    mpRefWin->InvertTracking( mpRefWin->PixelToLogic(aInvRect), ShowTrackFlags::Split );
}

Splitter::Splitter( vcl::Window* pParent, WinBits nStyle ) :
    Window( WindowType::SPLITTER ),
    mpRefWin( nullptr ),
    mnSplitPos( 0 ),
    mnLastSplitPos( 0 ),
    mnStartSplitPos( 0 ),
    mbDragFull( false ),
    mbKbdSplitting( false ),
    mbInKeyEvent( false ),
    mnKeyboardStepSize( SPLITTER_DEFAULTSTEPSIZE )
{
    ImplGetWindowImpl()->mbSplitter        = true;

    ImplInit( pParent, nStyle );

    SetLineColor();
    SetFillColor();
}

Splitter::~Splitter()
{
    disposeOnce();
}

void Splitter::dispose()
{
    SystemWindow *pSysWin = GetSystemWindow();
    if(pSysWin)
    {
        TaskPaneList *pTList = pSysWin->GetTaskPaneList();
        pTList->RemoveWindow(this);
    }
    mpRefWin.clear();
    Window::dispose();
}

void Splitter::SetHorizontal(bool bNew)
{
    if(bNew != mbHorzSplit)
    {
        ImplInitHorVer(bNew);
    }
}

void Splitter::SetKeyboardStepSize( long nStepSize )
{
    mnKeyboardStepSize = nStepSize;
}

Splitter* Splitter::ImplFindSibling()
{
    // look for another splitter with the same parent but different orientation
    vcl::Window *pWin = GetParent()->GetWindow( GetWindowType::FirstChild );
    Splitter *pSplitter = nullptr;
    while( pWin )
    {
        if( pWin->ImplIsSplitter() )
        {
            pSplitter = static_cast<Splitter*>(pWin);
            if( pSplitter != this && IsHorizontal() != pSplitter->IsHorizontal() )
                return pSplitter;
        }
        pWin = pWin->GetWindow( GetWindowType::Next );
    }
    return nullptr;
}

bool Splitter::ImplSplitterActive()
{
    // is splitter in document or at scrollbar handle ?

    bool bActive = true;
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    long nA = rSettings.GetScrollBarSize();
    long nB = StyleSettings::GetSplitSize();

    Size aSize = GetOutputSize();
    if ( mbHorzSplit )
    {
        if( aSize.Width() == nB && aSize.Height() == nA )
            bActive = false;
    }
    else
    {
        if( aSize.Width() == nA && aSize.Height() == nB )
            bActive = false;
    }
    return bActive;
}

void Splitter::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.GetClicks() == 2 )
    {
        if ( mnLastSplitPos != mnSplitPos )
        {
            StartSplit();
            Point aPos = rMEvt.GetPosPixel();
            if ( mbHorzSplit )
                aPos.setX( mnLastSplitPos );
            else
                aPos.setY( mnLastSplitPos );
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

void Splitter::ImplKbdTracking( vcl::KeyCode aKeyCode )
{
    sal_uInt16 nCode = aKeyCode.GetCode();
    if ( nCode == KEY_ESCAPE || nCode == KEY_RETURN )
    {
        if( !mbKbdSplitting )
            return;
        else
            mbKbdSplitting = false;

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
        // depending on the position calc allows continuous moves or snaps to row/columns
        // continuous mode is active when position is at the origin or end of the splitter
        // otherwise snap mode is active
        // default here is snap, holding shift sets continuous mode
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
                aNewPos.AdjustX( -delta );
                break;
            case KEY_RIGHT:
                aNewPos.AdjustX(delta );
                break;
            case KEY_UP:
                aNewPos.AdjustY( -delta );
                break;
            case KEY_DOWN:
                aNewPos.AdjustY(delta );
                break;
            default:
                maxiter = 0;    // leave loop
                break;
            }
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

void Splitter::StartSplit()
{
    maStartSplitHdl.Call( this );
}

void Splitter::Split()
{
    maSplitHdl.Call( this );
}

void Splitter::EndSplit()
{
    maEndSplitHdl.Call( this );
}

void Splitter::SetDragRectPixel( const tools::Rectangle& rDragRect, vcl::Window* _pRefWin )
{
    maDragRect = rDragRect;
    if ( !_pRefWin )
        mpRefWin = GetParent();
    else
        mpRefWin = _pRefWin;
}

void Splitter::SetSplitPosPixel( long nNewPos )
{
    mnSplitPos = nNewPos;
}

void Splitter::StartDrag()
{
    if ( IsTracking() )
        return;

    StartSplit();

    // Tracking starten
    StartTracking();

    // Start-Position ermitteln
    maDragPos = mpRefWin->GetPointerPosPixel();
    ImplSplitMousePos( maDragPos );
    if ( mbHorzSplit )
        mnStartSplitPos = maDragPos.X();
    else
        mnStartSplitPos = maDragPos.Y();

    mbDragFull = bool(Application::GetSettings().GetStyleSettings().GetDragFullOptions() & DragFullOptions::Split);
    if ( !mbDragFull )
        ImplDrawSplitter();
}

void Splitter::ImplStartKbdSplitting()
{
    if( mbKbdSplitting )
        return;

    mbKbdSplitting = true;

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
    if ( mbHorzSplit )
        mnStartSplitPos = maDragPos.X();
    else
        mnStartSplitPos = maDragPos.Y();
}

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
            aPos.setX( mnLastSplitPos );
        else
            aPos.setY( mnLastSplitPos );
    }

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

void Splitter::GetFocus()
{
    if( !ImplSplitterActive() )
        ImplRestoreSplitter();

    Invalidate();
}

void Splitter::LoseFocus()
{
    if( mbKbdSplitting )
    {
        vcl::KeyCode aReturnKey( KEY_RETURN );
        ImplKbdTracking( aReturnKey );
        mbKbdSplitting = false;
    }
    Invalidate();
}

void Splitter::KeyInput( const KeyEvent& rKEvt )
{
    if( mbInKeyEvent )
        return;

    mbInKeyEvent = true;

    Splitter *pSibling = ImplFindSibling();
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
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
                    vcl::KeyCode aKey( KEY_ESCAPE );
                    ImplKbdTracking( aKey );
                }

                StartSplit();
                Point aPos;
                if ( mbHorzSplit )
                    aPos.setX( 0 );
                else
                    aPos.setY( 0 );
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
    mbInKeyEvent = false;
}

void Splitter::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    if( rDCEvt.GetType() == DataChangedEventType::SETTINGS )
    {
        const AllSettings* pOldSettings = rDCEvt.GetOldSettings();
        if(!pOldSettings)
            return;

        Color oldFaceColor = pOldSettings->GetStyleSettings().GetFaceColor();
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

void Splitter::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rPaintRect)
{
    rRenderContext.DrawRect(rPaintRect);

    tools::Polygon aPoly(rPaintRect);
    tools::PolyPolygon aPolyPoly(aPoly);
    rRenderContext.DrawTransparent(aPolyPoly, 85);

    if (mbKbdSplitting)
    {
        LineInfo aInfo( LineStyle::Dash );
        //aInfo.SetDashLen( 2 );
        //aInfo.SetDashCount( 1 );
        aInfo.SetDistance( 1 );
        aInfo.SetDotLen( 2 );
        aInfo.SetDotCount( 3 );

        rRenderContext.DrawPolyLine( aPoly, aInfo );
    }
    else
    {
        rRenderContext.DrawRect(rPaintRect);
    }
}

Size Splitter::GetOptimalSize() const
{
    return LogicToPixel(Size(3, 3), MapMode(MapUnit::MapAppFont));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
