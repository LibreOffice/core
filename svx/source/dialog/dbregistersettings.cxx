/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbregistersettings.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:16:40 $
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
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#include "dbregistersettings.hxx"

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= DatabaseMapItem
    //====================================================================
    TYPEINIT1( DatabaseMapItem, SfxPoolItem )
    //--------------------------------------------------------------------
    DatabaseMapItem::DatabaseMapItem( sal_uInt16 _nId, const TNameLocationMap& _rSettings )
        :SfxPoolItem(_nId)
        ,m_aSettings(_rSettings)
    {
    }

    //--------------------------------------------------------------------
    int DatabaseMapItem::operator==( const SfxPoolItem& _rCompare ) const
    {
        const DatabaseMapItem* pItem = PTR_CAST(DatabaseMapItem, &_rCompare);
        if (!pItem)
            return sal_False;

        if (m_aSettings.size() != pItem->m_aSettings.size())
            return sal_False;

        return m_aSettings != pItem->m_aSettings;
    }

    //--------------------------------------------------------------------
    SfxPoolItem* DatabaseMapItem::Clone( SfxItemPool * ) const
    {
        return new DatabaseMapItem(Which(), m_aSettings);
    }

    //--------------------------------------------------------------------

//........................................................................
}   // namespace svx
//........................................................................


