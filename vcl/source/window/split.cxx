
/*************************************************************************
 *
 *  $RCSfile: split.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 17:09:38 $
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

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#define private public
#ifndef _SV_SPLIT_HXX
#include <split.hxx>
#endif
#undef private
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_SYSWIN_HXX
#include <syswin.hxx>
#endif
#ifndef _SV_TASKPANELIST_HXX
#include <taskpanelist.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <gradient.hxx>
#endif
#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _SV_LINEINFO_HXX
#include <lineinfo.hxx>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

namespace
{
    struct BlackInstance
    {
        Wallpaper * operator ()()
        {
            static Wallpaper instance(COL_BLACK);
            return &instance;
        }
    };

    struct WhiteInstance
    {
        Wallpaper * operator ()()
        {
            static Wallpaper instance(COL_LIGHTGRAY);
            return &instance;
        }
    };

    struct ImplBlackWall : public rtl::Static< Wallpaper, ImplBlackWall, BlackInstance > {};
    struct ImplWhiteWall : public rtl::Static< Wallpaper, ImplWhiteWall, WhiteInstance > {};
}

// =======================================================================

void Splitter::ImplInitData()
{
    mbSplitter        = TRUE;
    mpRefWin          = NULL;
    mnSplitPos        = 0;
    mnLastSplitPos    = 0;
    mnStartSplitPos   = 0;
    mbDragFull        = FALSE;
    mbKbdSplitting    = FALSE;
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
        mbHorzSplit = TRUE;
        SetSizePixel( Size( nB, nA ) );
    }
    else
    {
        ePointerStyle = POINTER_VSPLIT;
        mbHorzSplit = FALSE;
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

    mpRefWin->InvertTracking( aInvRect, SHOWTRACK_SPLIT );
}

// -----------------------------------------------------------------------

Splitter::Splitter( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_SPLITTER )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

Splitter::Splitter( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_SPLITTER )
{
    ImplInitData();
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

long Splitter::GetKeyboardStepSize() const
{
    return mnKeyboardStepSize;
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

BOOL Splitter::ImplSplitterActive()
{
    // is splitter in document or at scrollbar handle ?

    BOOL bActive = TRUE;
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    long nA = rSettings.GetScrollBarSize();
    long nB = rSettings.GetSplitSize();

    Size aSize = GetOutputSize();
    if ( mbHorzSplit )
    {
        if( aSize.Width() == nB && aSize.Height() == nA )
            bActive = FALSE;
    }
    else
    {
        if( aSize.Width() == nA && aSize.Height() == nB )
            bActive = FALSE;
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
    USHORT nCode = aKeyCode.GetCode();
    if ( nCode == KEY_ESCAPE || nCode == KEY_RETURN )
    {
        if( !mbKbdSplitting )
            return;
        else
            mbKbdSplitting = FALSE;

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

void Splitter::SetLastSplitPosPixel( long nNewPos )
{
    mnLastSplitPos = nNewPos;
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

    mbKbdSplitting = TRUE;

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
        mbKbdSplitting = FALSE;
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
    USHORT nCode = aKeyCode.GetCode();
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
