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
#include "precompiled_chart2.hxx"
#include "LegendItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "CharacterPropertyItemConverter.hxx"
#include <svx/chrtitem.hxx>
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/LegendExpansion.hpp>

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;

namespace chart
{
namespace wrapper
{

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
                                 C2U( "ReferencePageSize" ) ));
}

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


bool LegendItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_LEGEND_POS:
        {
            chart2::LegendPosition eNewPos = chart2::LegendPosition_LINE_END;
            chart2::LegendPosition eOldPos;
            bool bIsWide = false;
            sal_Bool bShow = sal_True;

            SvxChartLegendPos eItemPos =
                static_cast< const SvxChartLegendPosItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            switch( eItemPos )
            {
                case CHLEGEND_LEFT:
                    eNewPos = chart2::LegendPosition_LINE_START;
                    break;
                case CHLEGEND_RIGHT:
                    eNewPos = chart2::LegendPosition_LINE_END;
                    break;
                case CHLEGEND_TOP:
                    eNewPos = chart2::LegendPosition_PAGE_START;
                    bIsWide = true;
                    break;
                case CHLEGEND_BOTTOM:
                    eNewPos = chart2::LegendPosition_PAGE_END;
                    bIsWide = true;
                    break;

                case CHLEGEND_NONE:
                case CHLEGEND_NONE_LEFT:
                case CHLEGEND_NONE_RIGHT:
                case CHLEGEND_NONE_TOP:
                case CHLEGEND_NONE_BOTTOM:
                    bShow = sal_False;
                    break;
            }

            try
            {
                sal_Bool bWasShown = sal_True;
                if( ! (GetPropertySet()->getPropertyValue( C2U("Show")) >>= bWasShown) ||
                    ( bWasShown != bShow ))
                {
                    GetPropertySet()->setPropertyValue( C2U("Show"), uno::makeAny( bShow ));
                    bChanged = true;
                }

                if( bShow )
                {
                    if( ! ( GetPropertySet()->getPropertyValue( C2U( "AnchorPosition" )) >>= eOldPos ) ||
                        ( eOldPos != eNewPos ))
                    {
                        GetPropertySet()->setPropertyValue( C2U( "AnchorPosition" ), uno::makeAny( eNewPos ));
                        chart2::LegendExpansion eExp = bIsWide
                            ? chart2::LegendExpansion_WIDE
                            : chart2::LegendExpansion_HIGH;
                        GetPropertySet()->setPropertyValue( C2U( "Expansion" ), uno::makeAny( eExp ));
                        GetPropertySet()->setPropertyValue( C2U( "RelativePosition" ), uno::Any());
                        bChanged = true;
                    }
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
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
        case SCHATTR_LEGEND_POS:
        {
            SvxChartLegendPos eItemPos( CHLEGEND_RIGHT );
            chart2::LegendPosition ePos;

            sal_Bool bShow = sal_True;
            GetPropertySet()->getPropertyValue( C2U( "Show" )) >>= bShow;

            if( ! bShow )
            {
                eItemPos = CHLEGEND_NONE;
            }
            else if( GetPropertySet()->getPropertyValue( C2U( "AnchorPosition" )) >>= ePos )
            {
                switch( ePos )
                {
                    case chart2::LegendPosition_LINE_START:
                        eItemPos = CHLEGEND_LEFT;
                        break;
                    case chart2::LegendPosition_LINE_END:
                        eItemPos = CHLEGEND_RIGHT;
                        break;
                    case chart2::LegendPosition_PAGE_START:
                        eItemPos = CHLEGEND_TOP;
                        break;
                    case chart2::LegendPosition_PAGE_END:
                        eItemPos = CHLEGEND_BOTTOM;
                        break;

                    case chart2::LegendPosition_CUSTOM:
                    default:
                        eItemPos = CHLEGEND_RIGHT;
                        break;
                }
            }

            rOutItemSet.Put( SvxChartLegendPosItem( eItemPos, SCHATTR_LEGEND_POS ) );
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart
