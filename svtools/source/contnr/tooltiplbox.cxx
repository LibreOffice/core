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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include <svtools/tooltiplbox.hxx>
#include <vcl/help.hxx>

// ============================================================================

namespace svtools {

// ----------------------------------------------------------------------------

void lcl_ToolTipLBox_ShowToolTip( ListBox& rListBox, const HelpEvent& rHEvt )
{
    // only show tooltip if helpmode is BALLOON or QUICK
    if ( !( rHEvt.GetMode() & HELPMODE_BALLOON ) && !( rHEvt.GetMode() & HELPMODE_QUICK ) )
    {
        // else call base class method
        rListBox.ListBox::RequestHelp( rHEvt );
        return ;
    }

    // find the list box entry the mouse points to
    Point aMousePos( rListBox.ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

    sal_uInt16 nTop = rListBox.GetTopEntry();
    sal_uInt16 nBottom = nTop + rListBox.GetDisplayLineCount();

    sal_uInt16 nPos;
    for( nPos = nTop; nPos < nBottom; ++nPos )
    {
        Rectangle aItemRect( rListBox.GetBoundingRectangle( nPos ) );
        if( (aItemRect.Top() <= aMousePos.Y()) && (aMousePos.Y() <= aItemRect.Bottom()) )
            break;
    }

    // show text content of the entry, if it does not fit
    if( nPos < nBottom )
    {
        String aHelpText( rListBox.GetEntry( nPos ) );
        if( rListBox.GetTextWidth( aHelpText ) > rListBox.GetOutputSizePixel().Width() )
        {
            Point aLBoxPos( rListBox.OutputToScreenPixel( Point( 0, 0 ) ) );
            Size aLBoxSize( rListBox.GetSizePixel() );
            Rectangle aLBoxRect( aLBoxPos, aLBoxSize );

            if( rHEvt.GetMode() == HELPMODE_BALLOON )
                Help::ShowBalloon( &rListBox, aLBoxRect.Center(), aLBoxRect, aHelpText );
            else
                Help::ShowQuickHelp( &rListBox, aLBoxRect, aHelpText );
        }
    }
}

// ----------------------------------------------------------------------------

ToolTipListBox::ToolTipListBox( Window* pParent, WinBits nStyle ) :
    ListBox( pParent, nStyle )
{
}

ToolTipListBox::ToolTipListBox( Window* pParent, const ResId& rResId ) :
    ListBox( pParent, rResId )
{
}

void ToolTipListBox::RequestHelp( const HelpEvent& rHEvt )
{
    lcl_ToolTipLBox_ShowToolTip( *this, rHEvt );
}

// ----------------------------------------------------------------------------

ToolTipMultiListBox::ToolTipMultiListBox( Window* pParent, WinBits nStyle ) :
    MultiListBox( pParent, nStyle )
{
}

ToolTipMultiListBox::ToolTipMultiListBox( Window* pParent, const ResId& rResId ) :
    MultiListBox( pParent, rResId )
{
}

void ToolTipMultiListBox::RequestHelp( const HelpEvent& rHEvt )
{
    lcl_ToolTipLBox_ShowToolTip( *this, rHEvt );
}

// ----------------------------------------------------------------------------

} // namespace svtools

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
