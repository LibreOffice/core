/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LegendItemConverter.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:29:43 $
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
#include "LegendItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "CharacterPropertyItemConverter.hxx"

#ifndef _SVX_CHRTITEM_HXX
#define ITEMID_CHARTLEGENDPOS SCHATTR_LEGEND_POS
#include <svx/chrtitem.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XLEGEND_HPP_
#include <com/sun/star/chart2/XLegend.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_LEGENDPOSITION_HPP_
#include <com/sun/star/chart2/LegendPosition.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_LEGENDEXPANSION_HPP_
#include <com/sun/star/chart2/LegendExpansion.hpp>
#endif

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;

namespace
{
// ::comphelper::ItemPropertyMapType & lcl_GetLegendPropertyMap()
// {
//     static ::comphelper::ItemPropertyMapType aLegendPropertyMap(
//         ::comphelper::MakeItemPropertyMap
//         ( SCHATTR_TEXT_STACKED,   C2U( "StackCharacters" ))
//         );

//     return aLegendPropertyMap;
// };
} // anonymous namespace

namespace chart
{
namespace wrapper
{

LegendItemConverter::LegendItemConverter(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize ) :
        ItemConverter( rPropertySet, rItemPool )
{
    m_aConverters.push_back( new GraphicPropertyItemConverter(
                                 rPropertySet, rItemPool, rDrawModel,
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

const USHORT * LegendItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nLegendWhichPairs;
}

bool LegendItemConverter::GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const
{
    // No own (non-special) properties
    return false;
//     ::comphelper::ItemPropertyMapType & rMap( lcl_GetLegendPropertyMap());
//     ::comphelper::ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

//     if( aIt == rMap.end())
//         return false;

//     rOutName =(*aIt).second;
//     return true;
}


bool LegendItemConverter::ApplySpecialItem(
    USHORT nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_LEGEND_POS:
        {
            chart2::LegendPosition eNewPos, eOldPos;
            bool bIsWide = false;
            bool bShow = true;

            SvxChartLegendPos eItemPos =
                reinterpret_cast< const SvxChartLegendPosItem & >(
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
                    bShow = false;
                    break;
            }

            try
            {
                sal_Bool bWasShown = sal_True;
                if( ! (GetPropertySet()->getPropertyValue( C2U("Show")) >>= bWasShown) ||
                    ( bWasShown != bShow ))
                {
                    GetPropertySet()->setPropertyValue( C2U("Show"), uno::Any( &bShow, ::getBooleanCppuType() ));
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
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
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

            rOutItemSet.Put( SvxChartLegendPosItem( eItemPos ) );
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart
