/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <rtl/ustring.hxx>
#include <svl/aeitem.hxx>

#include <cstddef>
#include <vector>

struct SfxAllEnumValue_Impl
{
    sal_uInt16 nValue;
    OUString aText;
};

class SfxAllEnumValueArr : public std::vector<SfxAllEnumValue_Impl> {};


SfxAllEnumItem::SfxAllEnumItem(sal_uInt16 which, sal_uInt16 nVal):
    SfxAllEnumItem_Base(which, nVal),
    pValues( nullptr )
{
    InsertValue( nVal );
}

SfxAllEnumItem::SfxAllEnumItem( sal_uInt16 which, SvStream &rStream ):
    SfxAllEnumItem_Base(which, rStream)
{
    InsertValue( GetValue() );
}

SfxAllEnumItem::SfxAllEnumItem(sal_uInt16 which):
    SfxAllEnumItem_Base(which, 0)
{
}

SfxAllEnumItem::SfxAllEnumItem(const SfxAllEnumItem &rCopy):
    SfxAllEnumItem_Base(rCopy)
{
    if ( rCopy.pValues )
        pValues.reset( new SfxAllEnumValueArr(*rCopy.pValues) );
}

SfxAllEnumItem::~SfxAllEnumItem()
{
}

sal_uInt16 SfxAllEnumItem::GetValueCount() const
{
    return pValues ? pValues->size() : 0;
}

OUString SfxAllEnumItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    assert(pValues && nPos < pValues->size());
    return (*pValues)[nPos].aText;
}

sal_uInt16 SfxAllEnumItem::GetValueByPos( sal_uInt16 nPos ) const
{
    assert(pValues && nPos < pValues->size());
    return (*pValues)[nPos].nValue;
}

SfxPoolItem* SfxAllEnumItem::Clone( SfxItemPool * ) const
{
    return new SfxAllEnumItem(*this);
}

SfxPoolItem* SfxAllEnumItem::Create( SvStream & rStream, sal_uInt16 ) const
{
    return new SfxAllEnumItem( Which(), rStream );
}

/**
 * In contrast to @see GetPosByValue(sal_uInt16) const
 * this internal method returns the position the value would be for non-present values.
 */
std::size_t SfxAllEnumItem::GetPosByValue_( sal_uInt16 nVal ) const
{
    if ( !pValues )
        return 0;

    //FIXME: Optimisation: use binary search or SortArray
    std::size_t nPos;
    for ( nPos = 0; nPos < pValues->size(); ++nPos )
        if ( (*pValues)[nPos].nValue >= nVal )
            return nPos;
    return nPos;
}

sal_uInt16 SfxAllEnumItem::GetPosByValue( sal_uInt16 nValue ) const
{
    if ( !pValues || pValues->empty() )
        return nValue;

    sal_uInt16 nCount = GetValueCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
        if (GetValueByPos(i) == nValue)
            return i;
    return USHRT_MAX;
}

void SfxAllEnumItem::InsertValue( sal_uInt16 nValue, const OUString &rValue )
{
    SfxAllEnumValue_Impl aVal;
    aVal.nValue = nValue;
    aVal.aText = rValue;
    if ( !pValues )
        pValues.reset( new SfxAllEnumValueArr );
    else if ( GetPosByValue( nValue ) != USHRT_MAX )
        // remove when exists
        RemoveValue( nValue );
    // then insert
    pValues->insert(pValues->begin() + GetPosByValue_(nValue), aVal); // FIXME: Duplicates?
}

void SfxAllEnumItem::InsertValue( sal_uInt16 nValue )
{
    SfxAllEnumValue_Impl aVal;
    aVal.nValue = nValue;
    aVal.aText = OUString::number(nValue);
    if ( !pValues )
        pValues.reset( new SfxAllEnumValueArr );

    pValues->insert(pValues->begin() + GetPosByValue_(nValue), aVal); // FIXME: Duplicates?
}

void SfxAllEnumItem::RemoveValue( sal_uInt16 nValue )
{
    sal_uInt16 nPos = GetPosByValue(nValue);
    assert(nPos != USHRT_MAX);
    pValues->erase( pValues->begin() + nPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
