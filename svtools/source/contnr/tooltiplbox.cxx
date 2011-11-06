/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

