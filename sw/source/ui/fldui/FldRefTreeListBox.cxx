/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FldRefTreeListBox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:46:27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <FldRefTreeListBox.hxx>

#include <helpid.h>
#include <vcl/help.hxx>

SwFldRefTreeListBox::SwFldRefTreeListBox( Window* pParent,
                                          const ResId& rResId)
    : SvTreeListBox( pParent, rResId )
{
    SetHelpId( HID_REFSELECTION_TOOLTIP );
}

SwFldRefTreeListBox::~SwFldRefTreeListBox()
{
}

void SwFldRefTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    bool bCallBase( true );
    if ( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvLBoxEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            String sEntry( GetEntryText( pEntry ) );
            SvLBoxTab* pTab;
            SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
            if( pItem )
            {
                aPos = GetEntryPosition( pEntry );

                aPos.X() = GetTabPos( pEntry, pTab );
                Size aSize( pItem->GetSize( this, pEntry ) );

                if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                    aSize.Width() = GetSizePixel().Width() - aPos.X();

                aPos = OutputToScreenPixel(aPos);
                Rectangle aItemRect( aPos, aSize );
                Help::ShowQuickHelp( this, aItemRect, sEntry,
                    QUICKHELP_LEFT|QUICKHELP_VCENTER );
                bCallBase = false;
            }
        }
    }
    if ( bCallBase )
    {
        Window::RequestHelp( rHEvt );
    }
}
