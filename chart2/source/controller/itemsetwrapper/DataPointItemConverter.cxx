/*************************************************************************
 *
 *  $RCSfile: DataPointItemConverter.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-17 14:30:14 $
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
#include "DataPointItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"

#include "GraphicPropertyItemConverter.hxx"
#include "CharacterPropertyItemConverter.hxx"

#ifndef _SVX_CHRTITEM_HXX
#define ITEMID_CHARTDATADESCR SCHATTR_DATADESCR_DESCR
#include <svx/chrtitem.hxx>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_DATACAPTIONSTYLE_HPP__
#include <drafts/com/sun/star/chart2/DataCaptionStyle.hpp>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace
{
::comphelper::ItemPropertyMapType & lcl_GetDataPointPropertyMap()
{
    static ::comphelper::ItemPropertyMapType aDataPointPropertyMap(
        ::comphelper::MakeItemPropertyMap
        ( CHATTR_PIE_SEGMENT_OFFSET,      C2U( "Offset" ))
        );

    return aDataPointPropertyMap;
};
} // anonymous namespace

namespace chart
{
namespace wrapper
{

DataPointItemConverter::DataPointItemConverter(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    NumberFormatterWrapper * pNumFormatter,
    GraphicPropertyItemConverter::eGraphicObjectType eMapTo /* = FILL_PROPERTIES */
    ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_pNumberFormatterWrapper( pNumFormatter )
{
    m_aConverters.push_back( new GraphicPropertyItemConverter( rPropertySet, rItemPool, rDrawModel, eMapTo ));
    m_aConverters.push_back( new CharacterPropertyItemConverter( rPropertySet, rItemPool ));
}

DataPointItemConverter::~DataPointItemConverter()
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::DeleteItemConverterPtr() );
}

void DataPointItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::FillItemSetFunc( rOutItemSet ));

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool DataPointItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::ApplyItemSetFunc( rItemSet, bResult ));

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const USHORT * DataPointItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nRowWhichPairs;
}

bool DataPointItemConverter::GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const
{
    ::comphelper::ItemPropertyMapType & rMap( lcl_GetDataPointPropertyMap());
    ::comphelper::ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

    if( aIt == rMap.end())
        return false;

    rOutName =(*aIt).second;
    return true;
}


bool DataPointItemConverter::ApplySpecialItem(
    USHORT nWhichId, const SfxItemSet & rItemSet ) const
    throw( uno::Exception )
{
    bool bChanged = false;
    uno::Any aValue;

    switch( nWhichId )
    {
        case SCHATTR_DATADESCR_DESCR:
        {
            const SvxChartDataDescrItem & rItem =
                reinterpret_cast< const SvxChartDataDescrItem & >(
                    rItemSet.Get( nWhichId ));

            chart2::DataCaptionStyle aStyle;
            if( GetPropertySet()->getPropertyValue( C2U( "DataCaption" )) >>= aStyle )
            {
                switch( rItem.GetValue())
                {
                    case CHDESCR_NONE:
                        aStyle.ShowNumber = sal_False;
                        aStyle.ShowCategoryName = sal_False;
                        break;
                    case CHDESCR_VALUE:
                        aStyle.ShowNumber = sal_True;
                        aStyle.ShowNumberInPercent = sal_False;
                        aStyle.ShowCategoryName = sal_False;
                        break;
                    case CHDESCR_PERCENT:
                        aStyle.ShowNumber = sal_True;
                        aStyle.ShowNumberInPercent = sal_True;
                        aStyle.ShowCategoryName = sal_False;
                        break;
                    case CHDESCR_TEXT:
                        aStyle.ShowNumber = sal_False;
                        aStyle.ShowCategoryName = sal_True;
                        break;
                    case CHDESCR_TEXTANDPERCENT:
                        aStyle.ShowNumber = sal_True;
                        aStyle.ShowNumberInPercent = sal_True;
                        aStyle.ShowCategoryName = sal_True;
                        break;
                    case CHDESCR_TEXTANDVALUE:
                        aStyle.ShowNumber = sal_True;
                        aStyle.ShowNumberInPercent = sal_False;
                        aStyle.ShowCategoryName = sal_True;
                        break;
                    default:
                        break;
                }

                aValue <<= aStyle;
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "DataCaption" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "DataCaption" ), aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case SCHATTR_DATADESCR_SHOW_SYM:
        {
            const SvxChartDataDescrItem & rItem =
                reinterpret_cast< const SvxChartDataDescrItem & >(
                    rItemSet.Get( nWhichId ));

            chart2::DataCaptionStyle aStyle;
            if( GetPropertySet()->getPropertyValue( C2U( "DataCaption" )) >>= aStyle )
            {
                sal_Bool bOldValue = aStyle.ShowLegendSymbol;
                aStyle.ShowLegendSymbol = static_cast< sal_Bool >( rItem.GetValue() );
                if( bOldValue != aStyle.ShowLegendSymbol )
                {
                    GetPropertySet()->setPropertyValue( C2U( "DataCaption" ), uno::makeAny( aStyle ));
                    bChanged = true;
                }
            }
        }
        break;

        case SID_ATTR_NUMBERFORMAT_VALUE:
        {
            if( m_pNumberFormatterWrapper )
            {
                sal_Int32 nFmt = static_cast< sal_Int32 >(
                    reinterpret_cast< const SfxUInt32Item & >(
                        rItemSet.Get( nWhichId )).GetValue());

                aValue = uno::makeAny(
                    m_pNumberFormatterWrapper->getNumberFormatForKey( nFmt ));
                if( GetPropertySet()->getPropertyValue( C2U( "NumberFormat" )) != aValue )
                {
                    GetPropertySet()->setPropertyValue( C2U( "NumberFormat" ), aValue );
                    bChanged = true;
                }
            }
            else
            {
                OSL_ENSURE( false, "No NumberFormatterWrapper !" );
            }
        }
        break;
    }

    return bChanged;
}

void DataPointItemConverter::FillSpecialItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    switch( nWhichId )
    {
        case SCHATTR_DATADESCR_DESCR:
        case SCHATTR_DATADESCR_SHOW_SYM:
        {
            chart2::DataCaptionStyle aStyle;
            if( GetPropertySet()->getPropertyValue( C2U( "DataCaption" )) >>= aStyle )
            {
                SvxChartDataDescr aDescr;

                if( aStyle.ShowNumber )
                {
                    if( aStyle.ShowNumberInPercent )
                    {
                        if( aStyle.ShowCategoryName )
                            aDescr = CHDESCR_TEXTANDPERCENT;
                        else
                            aDescr = CHDESCR_PERCENT;
                    }
                    else
                    {
                        if( aStyle.ShowCategoryName )
                            aDescr = CHDESCR_TEXTANDVALUE;
                        else
                            aDescr = CHDESCR_VALUE;
                    }
                }
                else
                {
                    if( aStyle.ShowCategoryName )
                        aDescr = CHDESCR_TEXT;
                    else
                        aDescr = CHDESCR_NONE;
                }

                rOutItemSet.Put( SvxChartDataDescrItem( aDescr ));
                rOutItemSet.Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_SYM, aStyle.ShowLegendSymbol ));
            }
        }
        break;

        case SID_ATTR_NUMBERFORMAT_VALUE:
        {
            if( m_pNumberFormatterWrapper )
            {
                chart2::NumberFormat aNumFmt;
                if( GetPropertySet()->getPropertyValue( C2U( "NumberFormat" )) >>= aNumFmt );
                {
                    sal_Int32 nKey = m_pNumberFormatterWrapper->getKeyForNumberFormat( aNumFmt );
                    rOutItemSet.Put( SfxUInt32Item( nWhichId, nKey ));
                }
            }
            else
            {
                OSL_ENSURE( false, "No NumberFormatterWrapper !" );
            }
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart
