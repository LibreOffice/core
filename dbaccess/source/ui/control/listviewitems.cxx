/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listviewitems.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:01:47 $
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
#include "precompiled_dbaccess.hxx"

#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#include "listviewitems.hxx"
#endif
//........................................................................
namespace dbaui
{
//........................................................................

    //========================================================================
    // class OBoldListboxString
    //========================================================================
    //------------------------------------------------------------------------
    void OBoldListboxString::InitViewData( SvLBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* _pViewData)
    {
        SvLBoxString::InitViewData(pView,pEntry, _pViewData);
        if (!_pViewData)
            _pViewData = pView->GetViewDataItem( pEntry, this );
        pView->Push(PUSH_ALL);
        Font aFont( pView->GetFont());
        aFont.SetWeight(WEIGHT_BOLD);
        pView->SetFont( aFont );
        _pViewData->aSize = Size(pView->GetTextWidth(GetText()), pView->GetTextHeight());
        pView->Pop();
    }

    //------------------------------------------------------------------------
    USHORT OBoldListboxString::IsA()
    {
        return SV_ITEM_ID_BOLDLBSTRING;
    }

    //------------------------------------------------------------------------
    void OBoldListboxString::Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry )
    {
        if (m_bEmphasized)
        {
            rDev.Push(PUSH_ALL);
            Font aFont( rDev.GetFont());
            aFont.SetWeight(WEIGHT_BOLD);
            rDev.SetFont( aFont );
            Point aPos(rPos);
            rDev.DrawText( aPos, GetText() );
            rDev.Pop();
        }
        else
            SvLBoxString::Paint(rPos, rDev, nFlags, pEntry);
    }

//........................................................................
}   // namespace dbaui
//........................................................................

