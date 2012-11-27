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

#include "DialogListBox.hxx"

namespace sd
{

DialogListBox::DialogListBox( Window* pParent, WinBits nWinStyle ) :
    Control( pParent, nWinStyle ),
    mpChild( 0 )
{
    mpVScrollBar    = new ScrollBar( this, WB_VSCROLL | WB_DRAG );
    mpHScrollBar    = new ScrollBar( this, WB_HSCROLL | WB_DRAG );
    mpScrollBarBox  = new ScrollBarBox( this );

    Link aLink( LINK( this, DialogListBox, ScrollBarHdl ) );
    mpVScrollBar->SetScrollHdl( aLink );
    mpHScrollBar->SetScrollHdl( aLink );

    mbVScroll       = false;
    mbHScroll       = false;
    mbAutoHScroll   = ( nWinStyle & WB_AUTOHSCROLL ) ? true : false;
}

// -----------------------------------------------------------------------

DialogListBox::~DialogListBox()
{
    delete mpHScrollBar;
    delete mpVScrollBar;
    delete mpScrollBarBox;
    delete mpChild;
}

// -----------------------------------------------------------------------

void DialogListBox::SetChildWindow( Window* pChild, const Size& rMinSize )
{
    if( mpChild )
        delete mpChild;

    mpChild = pChild;
    maMinSize = rMinSize;
    ImplResizeControls();
    ImplCheckScrollBars();
}

// -----------------------------------------------------------------------

void DialogListBox::GetFocus()
{
    if( mpChild )
        mpChild->GrabFocus();
}

// -----------------------------------------------------------------------

::Window* DialogListBox::GetPreferredKeyInputWindow()
{
    if( mpChild )
        return mpChild;
    else
        return this;
}

// -----------------------------------------------------------------------

void DialogListBox::Resize()
{
    Control::Resize();
    ImplResizeControls();
    ImplCheckScrollBars();
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(DialogListBox, ScrollBarHdl)
{
    ImplResizeChild();
    return 1;
}

// -----------------------------------------------------------------------

void DialogListBox::ImplCheckScrollBars()
{
    bool bArrange = false;

    Size aOutSz = GetOutputSizePixel();

    // vert. ScrollBar
    if( aOutSz.Height() < maMinSize.Height() )
    {
        if( !mbVScroll )
            bArrange = true;
        mbVScroll = true;
    }
    else
    {
        if( mbVScroll )
            bArrange = true;
        mbVScroll = false;
    }

    // horz. ScrollBar
    if( mbAutoHScroll )
    {
        long nWidth = aOutSz.Width();
        if ( mbVScroll )
            nWidth -= mpVScrollBar->GetSizePixel().Width();
        if( nWidth < maMinSize.Width() )
        {
            if( !mbHScroll )
                bArrange = true;
            mbHScroll = true;

            if ( !mbVScroll )
            {
                int nHeight = aOutSz.Height() - mpHScrollBar->GetSizePixel().Height();
                if( nHeight < maMinSize.Height() )
                {
                    if( !mbVScroll )
                        bArrange = true;
                    mbVScroll = true;
                }
            }
        }
        else
        {
            if( mbHScroll )
                bArrange = true;
            mbHScroll = false;
        }
    }

    if( bArrange )
        ImplResizeControls();

    ImplInitScrollBars();
}

// -----------------------------------------------------------------------

void DialogListBox::ImplInitScrollBars()
{
    if( mpChild )
    {
        Size aOutSize( GetOutputSizePixel() );
        if( mbHScroll ) aOutSize.Height() -= mpHScrollBar->GetSizePixel().Height();
        if( mbVScroll ) aOutSize.Width() -= mpVScrollBar->GetSizePixel().Width();

        if ( mbVScroll )
        {
            mpVScrollBar->SetRangeMax( maMinSize.Height() );
            mpVScrollBar->SetVisibleSize( aOutSize.Height() );
            mpVScrollBar->SetPageSize( 16 );
        }

        if ( mbHScroll )
        {
            mpHScrollBar->SetRangeMax( maMinSize.Width() );
            mpHScrollBar->SetVisibleSize( aOutSize.Width() );
            mpHScrollBar->SetPageSize( 16 );
        }
    }
}

// -----------------------------------------------------------------------

void DialogListBox::ImplResizeControls()
{
    Size aOutSz( GetOutputSizePixel() );
    long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
    nSBWidth = CalcZoom( nSBWidth );

    maInnerSize = aOutSz;
    if ( mbVScroll )
        maInnerSize.Width() -= nSBWidth;
    if ( mbHScroll )
        maInnerSize.Height() -= nSBWidth;

    // ScrollBarBox
    if( mbVScroll && mbHScroll )
    {
        Point aBoxPos( maInnerSize.Width(), maInnerSize.Height() );
        mpScrollBarBox->SetPosSizePixel( aBoxPos, Size( nSBWidth, nSBWidth ) );
        mpScrollBarBox->Show();
    }
    else
    {
        mpScrollBarBox->Hide();
    }

    // vert. ScrollBar
    if( mbVScroll )
    {
        // Scrollbar on left or right side?
        Point aVPos( aOutSz.Width() - nSBWidth, 0 );
        mpVScrollBar->SetPosSizePixel( aVPos, Size( nSBWidth, maInnerSize.Height() ) );
        mpVScrollBar->Show();
    }
    else
    {
        mpVScrollBar->Hide();
    }

    // horz. ScrollBar
    if( mbHScroll )
    {
        Point aHPos( 0, aOutSz.Height() - nSBWidth );
        mpHScrollBar->SetPosSizePixel( aHPos, Size( maInnerSize.Width(), nSBWidth ) );
        mpHScrollBar->Show();
    }
    else
    {
        mpHScrollBar->Hide();
    }

    ImplResizeChild();
}

void DialogListBox::ImplResizeChild()
{
    Point aWinPos;
    Size aSize( maInnerSize );

    int nOffset;
    if( mbHScroll )
    {
        nOffset = mpHScrollBar->GetThumbPos();
        aWinPos.X() = -nOffset;
        aSize.Width() += nOffset;
    }

    if( mbVScroll )
    {
        nOffset = mpVScrollBar->GetThumbPos();
        aWinPos.Y() = -nOffset;
        aSize.Height() += nOffset;
    }

    mpChild->SetPosSizePixel( aWinPos, aSize );
}

// -----------------------------------------------------------------------

void DialogListBox::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
    {
        ImplCheckScrollBars();
    }
    else if ( ( nType == STATE_CHANGE_UPDATEMODE ) || ( nType == STATE_CHANGE_DATA ) )
    {
        sal_Bool bUpdate = IsUpdateMode();
        mpChild->SetUpdateMode( bUpdate );
        if ( bUpdate && IsReallyVisible() )
            ImplCheckScrollBars();
    }
    else if( nType == STATE_CHANGE_ENABLE )
    {
        mpHScrollBar->Enable( IsEnabled() );
        mpVScrollBar->Enable( IsEnabled() );
        mpScrollBarBox->Enable( IsEnabled() );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        mpChild->SetZoom( GetZoom() );
        Resize();
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        mpChild->SetControlFont( GetControlFont() );
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        mpChild->SetControlForeground( GetControlForeground() );
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        mpChild->SetControlBackground( GetControlBackground() );
    }
    else if( nType == STATE_CHANGE_VISIBLE )
    {
        mpChild->Show( IsVisible() );
    }

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

long DialogListBox::Notify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if ( rNEvt.GetType() == EVENT_COMMAND )
    {
        const CommandEvent& rCEvt = *rNEvt.GetCommandEvent();
        if ( rCEvt.GetCommand() == COMMAND_WHEEL )
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();
            if( !pData->GetModifier() && ( pData->GetMode() == COMMAND_WHEEL_SCROLL ) )
            {
                nDone = HandleScrollCommand( rCEvt, mpHScrollBar, mpVScrollBar );
            }
        }
    }

    return nDone ? nDone : Window::Notify( rNEvt );
}

} //  namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
