/*************************************************************************
 *
 *  $RCSfile: tooltiplbox.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2002-10-02 14:34:44 $
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

