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
#include "precompiled_dbaccess.hxx"

#include "stringlistitem.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
