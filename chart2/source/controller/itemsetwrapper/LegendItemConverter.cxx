/*************************************************************************
 *
 *  $RCSfile: LegendItemConverter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-14 14:45:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XLEGEND_HPP_
#include <drafts/com/sun/star/chart2/XLegend.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_LEGENDPOSITION_HPP_
#include <drafts/com/sun/star/chart2/LegendPosition.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_LEGENDEXPANSION_HPP_
#include <drafts/com/sun/star/chart2/LegendExpansion.hpp>
#endif

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

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
    SdrModel& rDrawModel  ) :
        ItemConverter( rPropertySet, rItemPool )
{
    m_aConverters.push_back( new GraphicPropertyItemConverter(
                                 rPropertySet, rItemPool, rDrawModel,
                                 GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES ));
    m_aConverters.push_back( new CharacterPropertyItemConverter(
                                 rPropertySet, rItemPool ));
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
    USHORT nWhichId, const SfxItemSet & rItemSet ) const
{
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_LEGEND_POS:
        {
            chart2::LegendPosition eNewPos, eOldPos;
            bool bIsWide = false;

            SvxChartLegendPos eItemPos =
                reinterpret_cast< const SvxChartLegendPosItem & >(
                    rItemSet.Get( nWhichId )).GetValue();
            switch( eItemPos )
            {
                case CHLEGEND_LEFT:
                case CHLEGEND_NONE_LEFT:
                    eNewPos = chart2::LegendPosition_LINE_START;
                    break;
                case CHLEGEND_NONE:
                case CHLEGEND_RIGHT:
                case CHLEGEND_NONE_RIGHT:
                    eNewPos = chart2::LegendPosition_LINE_END;
                    break;
                case CHLEGEND_TOP:
                case CHLEGEND_NONE_TOP:
                    eNewPos = chart2::LegendPosition_PAGE_START;
                    bIsWide = true;
                    break;
                case CHLEGEND_BOTTOM:
                case CHLEGEND_NONE_BOTTOM:
                    eNewPos = chart2::LegendPosition_PAGE_END;
                    bIsWide = true;
                    break;
            }

            bool bPropExisted =
                ( GetPropertySet()->getPropertyValue( C2U( "Position" )) >>= eOldPos );

            if( ! bPropExisted ||
                ( bPropExisted && eOldPos != eNewPos ))
            {
                GetPropertySet()->setPropertyValue( C2U( "Position" ), uno::makeAny( eNewPos ));
                chart2::LegendExpansion eExp = bIsWide
                    ? chart2::LegendExpansion_WIDE
                    : chart2::LegendExpansion_HIGH;
                GetPropertySet()->setPropertyValue( C2U( "Expansion" ), uno::makeAny( eExp ));
                bChanged = true;
            }
        }
        break;
    }

    return bChanged;
}

void LegendItemConverter::FillSpecialItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
{
    switch( nWhichId )
    {
        case SCHATTR_LEGEND_POS:
        {
            SvxChartLegendPos eItemPos( CHLEGEND_RIGHT );
            chart2::LegendPosition ePos;

            if( GetPropertySet()->getPropertyValue( C2U( "Position" )) >>= ePos )
            {
                switch( ePos )
                {
                    case chart2::LegendPosition_LINE_START:
                        eItemPos = CHLEGEND_LEFT;     break;
                    case chart2::LegendPosition_LINE_END:
                        eItemPos = CHLEGEND_RIGHT;    break;
                    case chart2::LegendPosition_PAGE_START:
                        eItemPos = CHLEGEND_TOP;      break;
                    case chart2::LegendPosition_PAGE_END:
                        eItemPos = CHLEGEND_BOTTOM;   break;

                    case chart2::LegendPosition_CUSTOM:
                    default:
                        eItemPos = CHLEGEND_NONE;     break;
                }
            }

            rOutItemSet.Put( SvxChartLegendPosItem( eItemPos ) );
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart
