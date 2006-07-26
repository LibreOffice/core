/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tooltiplbox.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 08:26:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SVTOOLS_TOOLTIPLBOX_HXX
#include "tooltiplbox.hxx"
#endif

#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif

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

