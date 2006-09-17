/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertysetitem.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:18:11 $
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

#ifndef _DBAUI_PROPERTYSETITEM_HXX_
#include "propertysetitem.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    //=========================================================================
    //= OPropertySetItem
    //=========================================================================
    TYPEINIT1(OPropertySetItem, SfxPoolItem);
    //-------------------------------------------------------------------------
    OPropertySetItem::OPropertySetItem(sal_Int16 _nWhich)
        :SfxPoolItem(_nWhich)
    {
    }

    //-------------------------------------------------------------------------
    OPropertySetItem::OPropertySetItem(sal_Int16 _nWhich, const Reference< XPropertySet >& _rxSet)
        :SfxPoolItem(_nWhich)
        ,m_xSet(_rxSet)
    {
    }

    //-------------------------------------------------------------------------
    OPropertySetItem::OPropertySetItem(const OPropertySetItem& _rSource)
        :SfxPoolItem(_rSource)
        ,m_xSet(_rSource.m_xSet)
    {
    }

    //-------------------------------------------------------------------------
    int OPropertySetItem::operator==(const SfxPoolItem& _rItem) const
    {
        const OPropertySetItem* pCompare = PTR_CAST(OPropertySetItem, &_rItem);
        if ((!pCompare) || (pCompare->m_xSet.get() != m_xSet.get()))
            return 0;

        return 1;
    }

    //-------------------------------------------------------------------------
    SfxPoolItem* OPropertySetItem::Clone(SfxItemPool* /* _pPool */) const
    {
        return new OPropertySetItem(*this);
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

