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
#include <vector>

TYPEINIT1_AUTOFACTORY(SfxAllEnumItem, SfxEnumItem)



struct SfxAllEnumValue_Impl
{
    sal_uInt16 nValue;
    OUString aText;
};

class SfxAllEnumValueArr : public std::vector<SfxAllEnumValue_Impl*>
{
public:
    ~SfxAllEnumValueArr()
    {
        for( const_iterator it = begin(); it != end(); ++it )
            delete *it;
    }
};



SfxAllEnumItem::SfxAllEnumItem() :
    SfxEnumItem(),
    pValues( 0 ),
    pDisabledValues( 0 )
{
}



SfxAllEnumItem::SfxAllEnumItem(sal_uInt16 which, sal_uInt16 nVal):
    SfxEnumItem(which, nVal),
    pValues( 0 ),
    pDisabledValues( 0 )
{
    InsertValue( nVal );
}



SfxAllEnumItem::SfxAllEnumItem( sal_uInt16 which, SvStream &rStream ):
    SfxEnumItem(which, rStream),
    pValues( 0 ),
    pDisabledValues( 0 )
{
    InsertValue( GetValue() );
}




SfxAllEnumItem::SfxAllEnumItem(sal_uInt16 which):
    SfxEnumItem(which, 0),
    pValues( 0 ),
    pDisabledValues( 0 )
{
}




SfxAllEnumItem::SfxAllEnumItem(const SfxAllEnumItem &rCopy):
    SfxEnumItem(rCopy),
    pValues(0),
    pDisabledValues( 0 )
{
    if ( !rCopy.pValues )
        return;

    pValues = new SfxAllEnumValueArr;

    for ( sal_uInt16 nPos = 0; nPos < rCopy.pValues->size(); ++nPos )
    {
        SfxAllEnumValue_Impl *pVal = new SfxAllEnumValue_Impl;
        pVal->nValue = (*rCopy.pValues)[nPos]->nValue;
        pVal->aText = (*rCopy.pValues)[nPos]->aText;
        pValues->insert( pValues->begin() + nPos, pVal );
    }

    if( rCopy.pDisabledValues )
    {
        pDisabledValues = new std::vector<sal_uInt16>( *(rCopy.pDisabledValues) );
    }
}



SfxAllEnumItem::~SfxAllEnumItem()
{
    delete pValues;
    delete pDisabledValues;
}



sal_uInt16 SfxAllEnumItem::GetValueCount() const
{
    return pValues ? pValues->size() : 0;
}



OUString SfxAllEnumItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( pValues && nPos < pValues->size(), "enum overflow" );
    return (*pValues)[nPos]->aText;
}



sal_uInt16 SfxAllEnumItem::GetValueByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( pValues && nPos < pValues->size(), "enum overflow" );
    return (*pValues)[nPos]->nValue;
}



SfxPoolItem* SfxAllEnumItem::Clone( SfxItemPool * ) const
{
    return new SfxAllEnumItem(*this);
}



SfxPoolItem* SfxAllEnumItem::Create( SvStream & rStream, sal_uInt16 ) const
{
    return new SfxAllEnumItem( Which(), rStream );
}




sal_uInt16 SfxAllEnumItem::_GetPosByValue( sal_uInt16 nVal ) const

/*  [Description]

    In constrast to <SfxEnumItemInterface::GetPosByValue(sal_uInt16)const>,
    for missing values this internal method returns the position
    where the value would have been.
*/

{
    if ( !pValues )
        return 0;

    //! linear search, binary search of SortAarray would be better
    sal_uInt16 nPos;
    for ( nPos = 0; nPos < pValues->size(); ++nPos )
        if ( (*pValues)[nPos]->nValue >= nVal )
            return nPos;
    return nPos;
}



sal_uInt16 SfxAllEnumItem::GetPosByValue( sal_uInt16 nValue ) const

/*  [Description]

    In constrast to <SfxEnumItemInterface::GetPosByValue(sal_uInt16)const>,
    until at least one value has been inserted with the
    method <SfxAllEnumItem::InsertValue()> this will return nValue.
*/

{
    if ( !pValues || pValues->empty() )
        return nValue;

    return SfxEnumItem::GetPosByValue( nValue );
}



void SfxAllEnumItem::InsertValue( sal_uInt16 nValue, const OUString &rValue )
{
    SfxAllEnumValue_Impl *pVal = new SfxAllEnumValue_Impl;
    pVal->nValue = nValue;
    pVal->aText = rValue;
    if ( !pValues )
        pValues = new SfxAllEnumValueArr;
    else if ( GetPosByValue( nValue ) != USHRT_MAX )
        // remove when exists
        RemoveValue( nValue );
    // then insert
    pValues->insert( pValues->begin() + _GetPosByValue(nValue), pVal ); //! doppelte?!
}



void SfxAllEnumItem::InsertValue( sal_uInt16 nValue )
{
    SfxAllEnumValue_Impl *pVal = new SfxAllEnumValue_Impl;
    pVal->nValue = nValue;
    pVal->aText = OUString::number(nValue);
    if ( !pValues )
        pValues = new SfxAllEnumValueArr;

    pValues->insert( pValues->begin() + _GetPosByValue(nValue), pVal ); //! doppelte?!
}

void SfxAllEnumItem::DisableValue( sal_uInt16 nValue )
{
    if ( !pDisabledValues )
        pDisabledValues = new std::vector<sal_uInt16>;

    pDisabledValues->push_back( nValue );
}

bool SfxAllEnumItem::IsEnabled( sal_uInt16 nValue ) const
{
    if ( pDisabledValues )
    {
        for ( size_t i=0; i<pDisabledValues->size(); i++ )
            if ( (*pDisabledValues)[i] == nValue )
                return false;
    }

    return true;
}



void SfxAllEnumItem::RemoveValue( sal_uInt16 nValue )
{
    sal_uInt16 nPos = GetPosByValue(nValue);
    DBG_ASSERT( nPos != USHRT_MAX, "removing value not in enum" );
    pValues->erase( pValues->begin() + nPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
