/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stringlistitem.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:18:52 $
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

#ifndef _DBAUI_STRINGLISTITEM_HXX_
#include "stringlistitem.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

using namespace ::com::sun::star::uno;

//=========================================================================
//= OStringListItem
//=========================================================================
TYPEINIT1(OStringListItem, SfxPoolItem);
//-------------------------------------------------------------------------
OStringListItem::OStringListItem(sal_Int16 _nWhich, const Sequence< ::rtl::OUString >& _rList)
    :SfxPoolItem(_nWhich)
    ,m_aList(_rList)
{
}

//-------------------------------------------------------------------------
OStringListItem::OStringListItem(const OStringListItem& _rSource)
    :SfxPoolItem(_rSource)
    ,m_aList(_rSource.m_aList)
{
}

//-------------------------------------------------------------------------
int OStringListItem::operator==(const SfxPoolItem& _rItem) const
{
    const OStringListItem* pCompare = PTR_CAST(OStringListItem, &_rItem);
    if ((!pCompare) || (pCompare->m_aList.getLength() != m_aList.getLength()))
        return 0;

    // compare all strings individually
    const ::rtl::OUString* pMyStrings = m_aList.getConstArray();
    const ::rtl::OUString* pCompareStrings = pCompare->m_aList.getConstArray();

    for (sal_Int32 i=0; i<m_aList.getLength(); ++i, ++pMyStrings, ++pCompareStrings)
        if (!pMyStrings->equals(*pCompareStrings))
            return 0;

    return 1;
}

//-------------------------------------------------------------------------
SfxPoolItem* OStringListItem::Clone(SfxItemPool* /* _pPool */) const
{
    return new OStringListItem(*this);
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

