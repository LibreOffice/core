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

#include "RegressionEquationItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "CharacterPropertyItemConverter.hxx"
#include "MultipleItemConverter.hxx"
#include <unonames.hxx>

#include <svl/intitem.hxx>
#include <rtl/math.hxx>

#include <functional>
#include <algorithm>
#include <memory>

using namespace ::com::sun::star;

namespace chart { namespace wrapper {

namespace {

ItemPropertyMapType & lcl_GetEquationPropertyMap()
{
    static ItemPropertyMapType aEquationPropertyMap;

    return aEquationPropertyMap;
};

} // anonymous namespace

RegressionEquationItemConverter::RegressionEquationItemConverter(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
    const awt::Size* pRefSize ) :
        ItemConverter( rPropertySet, rItemPool )
{
    m_aConverters.push_back( new GraphicPropertyItemConverter(
                                 rPropertySet, rItemPool, rDrawModel,
                                 xNamedPropertyContainerFactory,
                                 GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES ));

    m_aConverters.push_back(
        new CharacterPropertyItemConverter(rPropertySet, rItemPool, pRefSize, "ReferencePageSize"));
}

RegressionEquationItemConverter::~RegressionEquationItemConverter()
{
    ::std::for_each(m_aConverters.begin(), m_aConverters.end(), std::default_delete<ItemConverter>());
}

void RegressionEquationItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    for( const auto& pConv : m_aConverters )
        pConv->FillItemSet( rOutItemSet );

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool RegressionEquationItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    for( const auto& pConv : m_aConverters )
        bResult = pConv->ApplyItemSet( rItemSet );

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const sal_uInt16 * RegressionEquationItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nRegEquationWhichPairs;
}

bool RegressionEquationItemConverter::GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const
{
    ItemPropertyMapType & rMap( lcl_GetEquationPropertyMap());
    ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

    if( aIt == rMap.end())
        return false;

    rOutProperty =(*aIt).second;
    return true;
}

bool RegressionEquationItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    bool bChanged = false;

    switch( nWhichId )
    {
        case SID_ATTR_NUMBERFORMAT_VALUE:
        {
            uno::Any aValue( static_cast< sal_Int32 >(
                static_cast< const SfxUInt32Item & >(
                    rItemSet.Get( nWhichId )).GetValue()));
            if (GetPropertySet()->getPropertyValue(CHART_UNONAME_NUMFMT) != aValue)
            {
                GetPropertySet()->setPropertyValue(CHART_UNONAME_NUMFMT, aValue);
                bChanged = true;
            }
        }
        break;
    }

    return bChanged;
}

void RegressionEquationItemConverter::FillSpecialItem(
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    switch( nWhichId )
    {
        case SID_ATTR_NUMBERFORMAT_VALUE:
        {
            sal_Int32 nFormatKey = 0;
            if (GetPropertySet()->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nFormatKey)
            {
                rOutItemSet.Put( SfxUInt32Item( nWhichId, nFormatKey ));
            }
        }
        break;
   }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
