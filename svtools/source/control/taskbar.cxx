/*************************************************************************
 *
 *  $RCSfile: taskbar.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
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

#define _TASKBAR_CXX

#ifndef _TOOLS_LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _VCL_FLOATWIN_HXX
#include <vcl/floatwin.hxx>
#endif

#include <taskbar.hxx>

// =======================================================================

class ImplTaskBarFloat : public FloatingWindow
{
public:
    TaskBar*            mpTaskBar;

public:
                        ImplTaskBarFloat( TaskBar* pTaskBar );

    void                Paint();
};

// -----------------------------------------------------------------------

ImplTaskBarFloat::ImplTaskBarFloat( TaskBar* pTaskBar ) :
    FloatingWindow( pTaskBar, 0 )
{
    mpTaskBar   = pTaskBar;
}

// -----------------------------------------------------------------------

void ImplTaskBarFloat::Paint()
{
}

// =======================================================================

#define TASKBAR_BORDER              2
#define TASKBAR_OFFSIZE             3
#define TASKBAR_OFFX                2
#define TASKBAR_OFFY                1
#define TASKBAR_BUTTONOFF           5
#define TASKBAR_AUTOHIDE_HEIGHT     2

// =======================================================================

TaskBar::TaskBar( Window* pParent, WinBits nWinStyle ) :
    Window( pParent, WB_3DLOOK )
{
    mpButtonBar         = NULL;
    mpTaskToolBox       = NULL;
    mpStatusBar         = NULL;
    mnStatusWidth       = 0;
    mnOldStatusWidth    = 0;
    mnLines             = 1;
    mnWinBits           = nWinStyle;
    mbStatusText        = FALSE;
    mbShowItems         = FALSE;
    mbAutoHide          = FALSE;

    ImplInitSettings();
}

// -----------------------------------------------------------------------

TaskBar::~TaskBar()
{
    if ( mpButtonBar )
        delete mpButtonBar;
    if ( mpTaskToolBox )
        delete mpTaskToolBox;
    if ( mpStatusBar )
        delete mpStatusBar;
}

// -----------------------------------------------------------------------

void TaskBar::ImplInitSettings()
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

// -----------------------------------------------------------------------

void TaskBar::ImplNewHeight( long nNewHeight )
{
    long nOldHeight = GetSizePixel().Height();
    if ( nNewHeight != nOldHeight )
    {
        long nY = GetPosPixel().Y()-(nNewHeight-nOldHeight);
        SetPosSizePixel( 0, nY, 0, nNewHeight,
                         WINDOW_POSSIZE_Y | WINDOW_POSSIZE_HEIGHT );
        TaskResize();
    }
}

// -----------------------------------------------------------------------

void TaskBar::TaskResize()
{
    maTaskResizeHdl.Call( this );
}

// -----------------------------------------------------------------------

TaskButtonBar* TaskBar::CreateButtonBar()
{
    return new TaskButtonBar( this );
}

// -----------------------------------------------------------------------

TaskToolBox* TaskBar::CreateTaskToolBox()
{
    return new TaskToolBox( this );
}

// -----------------------------------------------------------------------

TaskStatusBar* TaskBar::CreateTaskStatusBar()
{
    return new TaskStatusBar( this );
}

// -----------------------------------------------------------------------

void TaskBar::MouseMove( const MouseEvent& rMEvt )
{
    if ( mnWinBits & WB_SIZEABLE )
    {
        TaskToolBox*    pTempTaskToolBox = GetTaskToolBox();
        TaskStatusBar*  pTempStatusBar = GetStatusBar();

        if ( pTempTaskToolBox && pTempStatusBar )
        {
            long            nStatusX = pTempStatusBar->GetPosPixel().X()-TASKBAR_OFFSIZE-2;
            long            nMouseX = rMEvt.GetPosPixel().X();
            PointerStyle    ePtrStyle;
            if ( (nMouseX >= nStatusX-1) && (nMouseX <= nStatusX+3) )
                ePtrStyle = POINTER_HSIZEBAR;
            else
                ePtrStyle = POINTER_ARROW;
            Pointer aPtr( ePtrStyle );
            SetPointer( aPtr );
        }
    }
}

// -----------------------------------------------------------------------

void TaskBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && (mnWinBits & WB_SIZEABLE) )
    {
        TaskToolBox*    pTempTaskToolBox = GetTaskToolBox();
        TaskStatusBar*  pTempStatusBar = GetStatusBar();

        if ( pTempTaskToolBox && pTempStatusBar )
        {
            long    nStatusX = pTempStatusBar->GetPosPixel().X()-TASKBAR_OFFSIZE-2;
            long    nMouseX = rMEvt.GetPosPixel().X();
            if ( (nMouseX >= nStatusX-1) && (nMouseX <= nStatusX+3) )
            {
                if ( rMEvt.GetClicks() == 2 )
                {
                    if ( mnStatusWidth )
                    {
                        mnStatusWidth = 0;
                        Resize();
                    }
                }
                else
                {
                    StartTracking();
                    mnOldStatusWidth = mnStatusWidth;
                    mnMouseOff = nMouseX-nStatusX;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

void TaskBar::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( rTEvt.IsTrackingCanceled() )
        {
            mnStatusWidth = mnOldStatusWidth;
            Resize();
            Update();
        }
    }
    else
    {
        Size aSize = GetOutputSizePixel();

#ifdef MAC
        if ( !Application::IsFullScreenMode() )
            aSize.Width() -= 17;
#endif

        long nMouseX = rTEvt.GetMouseEvent().GetPosPixel().X()-mnMouseOff;
        if ( nMouseX < 0 )
            nMouseX = 0;
        long nMaxX = aSize.Width()-TASKBAR_OFFX-TASKBAR_OFFSIZE-1;
        if ( nMouseX > nMaxX )
            nMouseX = nMaxX;
        mnStatusWidth = aSize.Width()-nMouseX-TASKBAR_OFFX-TASKBAR_OFFSIZE;
        Resize();
        Update();
    }
}

// -----------------------------------------------------------------------

void TaskBar::Paint( const Rectangle& rRect )
{
    if ( mnWinBits & (WB_BORDER | WB_SIZEABLE) )
    {
        const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
        Size                    aSize = GetOutputSizePixel();
        long    nY = 0;

        if ( mnWinBits & WB_BORDER )
        {
            SetLineColor( rStyleSettings.GetShadowColor() );
            DrawLine( Point( 0, 0 ), Point( aSize.Width()-1, 0 ) );
            SetLineColor( rStyleSettings.GetLightColor() );
            DrawLine( Point( 0, 1 ), Point( aSize.Width()-1, 1 ) );
            nY += 2;
        }

        if ( (mnWinBits & WB_SIZEABLE) )
        {
            TaskButtonBar*  pTempButtonBar = GetButtonBar();
            TaskToolBox*    pTempTaskToolBox = GetTaskToolBox();
            TaskStatusBar*  pTempStatusBar = GetStatusBar();

            if ( pTempTaskToolBox && pTempStatusBar )
            {
                long nStatusX = pTempStatusBar->GetPosPixel().X()-TASKBAR_OFFSIZE-2;
                if ( nStatusX > 0 )
                {
                    SetLineColor( rStyleSettings.GetShadowColor() );
                    DrawLine( Point( nStatusX, nY ), Point( nStatusX, aSize.Height()-1 ) );
                    nStatusX++;
                    SetLineColor( rStyleSettings.GetLightColor() );
                    DrawLine( Point( nStatusX, nY ), Point( nStatusX, aSize.Height()-1 ) );
                }
            }
        }
    }

    Window::Paint( rRect );
}

// -----------------------------------------------------------------------

void TaskBar::Resize()
{
    if ( !IsReallyShown() )
        return;

    TaskButtonBar*  pTempButtonBar = GetButtonBar();
    TaskToolBox*    pTempTaskToolBox = GetTaskToolBox();
    TaskStatusBar*  pTempStatusBar = GetStatusBar();
    Point           aToolPos( TASKBAR_OFFX, 0 );
    Size            aSize = GetOutputSizePixel();
    Size            aStatusSize;
    Size            aToolSize( aSize.Width()-(TASKBAR_OFFX*2), 0 );
    long            nOldStatusX = -1;
    long            nNewStatusX = -1;
    long            nTaskHeight = aSize.Height() - (TASKBAR_OFFY*2);

#ifdef MAC
    if ( !Application::IsFullScreenMode() )
        aSize.Width() -= 17;
#endif

    if ( mnWinBits & WB_BORDER )
    {
        nTaskHeight -= TASKBAR_BORDER;
        aToolPos.Y() += TASKBAR_BORDER;
    }

    if ( pTempButtonBar )
    {
        USHORT  i = 0;
        BOOL    bVisibleItems = FALSE;
        while ( i < pTempButtonBar->GetItemCount() )
        {
            if ( pTempButtonBar->IsItemVisible( pTempButtonBar->GetItemId( i ) ) )
            {
                bVisibleItems = TRUE;
                break;
            }
            i++;
        }
        if ( mbStatusText || !bVisibleItems )
            pTempButtonBar->Hide();
        else
        {
            Size aButtonBarSize = pTempButtonBar->CalcWindowSizePixel();
            if ( pTempButtonBar->GetItemCount() )
                nTaskHeight = aButtonBarSize.Height();
            else
                aButtonBarSize.Height() = nTaskHeight;
            Point aTempPos = aToolPos;
            aTempPos.Y() += (aSize.Height()-aButtonBarSize.Height()-aTempPos.Y())/2;
            pTempButtonBar->SetPosSizePixel( aTempPos, aButtonBarSize );
            pTempButtonBar->Show();
            aToolPos.X() += aButtonBarSize.Width()+TASKBAR_BUTTONOFF;
        }
    }

    if ( pTempStatusBar )
    {
        aStatusSize = pTempStatusBar->CalcWindowSizePixel();
        if ( mnStatusWidth )
            aStatusSize.Width() = mnStatusWidth;
        if ( !pTempTaskToolBox || mbStatusText )
            aStatusSize.Width() = aSize.Width();
        long nMaxHeight = aSize.Height()-(TASKBAR_OFFY*2);
        if ( mnWinBits & WB_BORDER )
            nMaxHeight -= TASKBAR_BORDER;
        if ( nMaxHeight+2 > aStatusSize.Height() )
            aStatusSize.Height() = nMaxHeight;
        Point aPos( aSize.Width()-aStatusSize.Width(), 0 );
        if ( pTempTaskToolBox && (mnWinBits & WB_SIZEABLE) && !mbStatusText )
        {
            long nMinToolWidth = aToolPos.X()+50;
            if ( aPos.X() < nMinToolWidth )
            {
                aStatusSize.Width() -= nMinToolWidth-aPos.X();
                aPos.X() = nMinToolWidth;
            }
        }
        if ( aPos.X() < 0 )
        {
            aStatusSize.Width() = aSize.Width();
            aPos.X() = 0;
        }
        if ( mnWinBits & WB_BORDER )
            aPos.Y() += TASKBAR_BORDER;
        aPos.Y() += (aSize.Height()-aStatusSize.Height()-aPos.Y())/2;
        if ( mnWinBits & WB_SIZEABLE )
        {
            if ( pTempTaskToolBox )
            {
                nOldStatusX = pTempStatusBar->GetPosPixel().X()-TASKBAR_OFFSIZE-2;
                nNewStatusX = aPos.X()-TASKBAR_OFFSIZE-2;
            }
        }
        pTempStatusBar->SetPosSizePixel( aPos, aStatusSize );
        pTempStatusBar->Show();
        aToolSize.Width() = aPos.X()-aToolPos.X()-TASKBAR_OFFX;
        if ( mnWinBits & WB_SIZEABLE )
            aToolSize.Width() -= (TASKBAR_OFFSIZE*2)-2;
    }

    if ( pTempTaskToolBox )
    {
        if ( aToolSize.Width() <= 24 )
            pTempTaskToolBox->Hide();
        else
        {
            aToolSize.Height() = pTempTaskToolBox->CalcWindowSizePixel().Height();
            if ( pTempTaskToolBox->GetItemCount() )
                nTaskHeight = aToolSize.Height();
            else
                aToolSize.Height() = nTaskHeight;
            aToolPos.Y() += (aSize.Height()-aToolSize.Height()-aToolPos.Y())/2;
            pTempTaskToolBox->SetPosSizePixel( aToolPos, aToolSize );
            pTempTaskToolBox->Show();
        }
    }

    if ( nOldStatusX != nNewStatusX )
    {
        if ( nOldStatusX > 0 )
        {
            Rectangle aRect( nOldStatusX, 0, nOldStatusX+2, aSize.Height()-1 );
            Invalidate( aRect );
        }
        if ( nNewStatusX > 0 )
        {
            Rectangle aRect( nNewStatusX, 0, nNewStatusX+2, aSize.Height()-1 );
            Invalidate( aRect );
        }
    }
}

// -----------------------------------------------------------------------

void TaskBar::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
        Format();
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_FORMAT )
    {
        ImplInitSettings();
        ImplNewHeight( CalcWindowSizePixel().Height() );
        Format();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void TaskBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        // Asyncronen StateChanged ausloesen, damit sich die
        // TaskBar an die neuen Groessen der Child-Fenster
        // orientieren kann
        PostStateChanged( STATE_CHANGE_FORMAT );
    }
}

// -----------------------------------------------------------------------

void TaskBar::Format()
{
    ImplNewHeight( CalcWindowSizePixel().Height() );
    Resize();
}

// -----------------------------------------------------------------------

void TaskBar::SetLines( USHORT nLines )
{
    mnLines = nLines;
}

// -----------------------------------------------------------------------

void TaskBar::EnableAutoHide( BOOL bAutoHide )
{
    mbAutoHide = bAutoHide;

    if ( mbAutoHide )
    {
        ImplNewHeight( TASKBAR_AUTOHIDE_HEIGHT );
    }
    else
    {
        ImplNewHeight( CalcWindowSizePixel().Height() );
    }
}

// -----------------------------------------------------------------------

void TaskBar::ShowStatusText( const String& rText )
{
    if ( mpStatusBar )
    {
        if ( !mbStatusText )
        {
            mbStatusText = TRUE;
            if ( mpStatusBar->AreItemsVisible() )
            {
                mbShowItems = TRUE;
                mpStatusBar->HideItems();
            }
            else
                mbShowItems = TRUE;
            maOldText = mpStatusBar->GetText();
            Resize();
            mpStatusBar->SetText( rText );
            Update();
            mpStatusBar->Update();
        }
        else
            mpStatusBar->SetText( rText );
    }
}

// -----------------------------------------------------------------------

void TaskBar::HideStatusText()
{
    if ( mbStatusText && mpStatusBar )
    {
        mbStatusText = FALSE;
        mpStatusBar->SetText( maOldText );
        Resize();
        if ( mbShowItems )
            mpStatusBar->ShowItems();
    }
}

// -----------------------------------------------------------------------

Size TaskBar::CalcWindowSizePixel() const
{
    TaskButtonBar*  pTempButtonBar = GetButtonBar();
    TaskToolBox*    pTempTaskToolBox = GetTaskToolBox();
    TaskStatusBar*  pTempStatusBar = GetStatusBar();
    Size            aSize;
    long            nTempHeight;

    if ( pTempButtonBar && pTempButtonBar->GetItemCount() )
        aSize.Height() = pTempButtonBar->CalcWindowSizePixel().Height()+(TASKBAR_OFFY*2);
    if ( pTempTaskToolBox && pTempTaskToolBox->GetItemCount() )
    {
        nTempHeight = pTempTaskToolBox->CalcWindowSizePixel().Height()+(TASKBAR_OFFY*2);
        if ( nTempHeight > aSize.Height() )
             aSize.Height() = nTempHeight;
    }
    if ( pTempStatusBar )
    {
        nTempHeight = pTempStatusBar->GetSizePixel().Height();
        if ( nTempHeight > aSize.Height() )
             aSize.Height() = nTempHeight;
    }

    if ( mnWinBits & WB_BORDER )
        aSize.Height() += TASKBAR_BORDER;

    return aSize;
}

// -----------------------------------------------------------------------

TaskButtonBar* TaskBar::GetButtonBar() const
{
    if ( !mpButtonBar )
        ((TaskBar*)this)->mpButtonBar = ((TaskBar*)this)->CreateButtonBar();
    return mpButtonBar;
}

// -----------------------------------------------------------------------

TaskToolBox* TaskBar::GetTaskToolBox() const
{
    if ( !mpTaskToolBox )
        ((TaskBar*)this)->mpTaskToolBox = ((TaskBar*)this)->CreateTaskToolBox();
    return mpTaskToolBox;
}

// -----------------------------------------------------------------------

TaskStatusBar* TaskBar::GetStatusBar() const
{
    if ( !mpStatusBar )
    {
        ((TaskBar*)this)->mpStatusBar = ((TaskBar*)this)->CreateTaskStatusBar();
        if ( mpStatusBar )
            mpStatusBar->mpNotifyTaskBar = (TaskBar*)this;
    }
    return mpStatusBar;
}
