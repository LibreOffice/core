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

#include "LegendItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "CharacterPropertyItemConverter.hxx"
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>

#include <svl/intitem.hxx>
#include <svl/eitem.hxx>

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;

namespace chart
{
namespace wrapper
{

SAL_WNODEPRECATED_DECLARATIONS_PUSH
LegendItemConverter::LegendItemConverter(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
    ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize ) :
        ItemConverter( rPropertySet, rItemPool )
{
    m_aConverters.push_back( new GraphicPropertyItemConverter(
                                 rPropertySet, rItemPool, rDrawModel, xNamedPropertyContainerFactory,
                                 GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES ));
    m_aConverters.push_back( new CharacterPropertyItemConverter(
                                 rPropertySet, rItemPool, pRefSize,
                                 "ReferencePageSize" ));
}
SAL_WNODEPRECATED_DECLARATIONS_POP

LegendItemConverter::~LegendItemConverter()
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::DeleteItemConverterPtr() );
}

void LegendItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::FillItemSetFunc( rOutItemSet ));

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool LegendItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::ApplyItemSetFunc( rItemSet, bResult ));

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const sal_uInt16 * LegendItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nLegendWhichPairs;
}

bool LegendItemConverter::GetItemProperty( tWhichIdType /*nWhichId*/, tPropertyNameWithMemberId & /*rOutProperty*/ ) const
{
    // No own (non-special) properties
    return false;
}

bool LegendItemConverter::ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet& rInItemSet )
    throw( uno::Exception )
{
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_LEGEND_SHOW:
        {
            const SfxPoolItem* pPoolItem = NULL;
            if( rInItemSet.GetItemState( SCHATTR_LEGEND_SHOW, sal_True, &pPoolItem ) == SFX_ITEM_SET )
            {
                sal_Bool bShow = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
                sal_Bool bWasShown = sal_True;
                if( ! (GetPropertySet()->getPropertyValue( "Show" ) >>= bWasShown) ||
                    ( bWasShown != bShow ))
                {
                    GetPropertySet()->setPropertyValue( "Show" , uno::makeAny( bShow ));
                    bChanged = true;
                }
            }

        }
        break;
        case SCHATTR_LEGEND_POS:
        {
            const SfxPoolItem* pPoolItem = NULL;
            if( rInItemSet.GetItemState( SCHATTR_LEGEND_POS, sal_True, &pPoolItem ) == SFX_ITEM_SET )
            {
                chart2::LegendPosition eNewPos = static_cast<chart2::LegendPosition>(((const SfxInt32Item*)pPoolItem)->GetValue());

                ::com::sun::star::chart::ChartLegendExpansion eExpansion = ::com::sun::star::chart::ChartLegendExpansion_HIGH;
                switch( eNewPos )
                {
                    case chart2::LegendPosition_LINE_START:
                    case chart2::LegendPosition_LINE_END:
                        eExpansion = ::com::sun::star::chart::ChartLegendExpansion_HIGH;
                        break;
                    case chart2::LegendPosition_PAGE_START:
                    case chart2::LegendPosition_PAGE_END:
                        eExpansion = ::com::sun::star::chart::ChartLegendExpansion_WIDE;
                        break;
                    default:
                        break;
                }

                try
                {
                    chart2::LegendPosition eOldPos;
                    if( ! ( GetPropertySet()->getPropertyValue( "AnchorPosition" ) >>= eOldPos ) ||
                        ( eOldPos != eNewPos ))
                    {
                        GetPropertySet()->setPropertyValue( "AnchorPosition" , uno::makeAny( eNewPos ));
                        GetPropertySet()->setPropertyValue( "Expansion" , uno::makeAny( eExpansion ));
                        GetPropertySet()->setPropertyValue( "RelativePosition" , uno::Any());
                        bChanged = true;
                    }
                }
                catch( const uno::Exception & ex )
                {
                    ASSERT_EXCEPTION( ex );
                }
            }
        }
        break;
    }

    return bChanged;
}

void LegendItemConverter::FillSpecialItem(
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    switch( nWhichId )
    {
        case SCHATTR_LEGEND_SHOW:
        {
            sal_Bool bShow = sal_True;
            GetPropertySet()->getPropertyValue( "Show" ) >>= bShow;
            rOutItemSet.Put( SfxBoolItem(SCHATTR_LEGEND_SHOW, bShow) );
        }
        break;
        case SCHATTR_LEGEND_POS:
        {
            chart2::LegendPosition eLegendPos( chart2::LegendPosition_LINE_END );
            GetPropertySet()->getPropertyValue( "AnchorPosition" ) >>= eLegendPos;
            rOutItemSet.Put( SfxInt32Item(SCHATTR_LEGEND_POS, eLegendPos ) );
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
