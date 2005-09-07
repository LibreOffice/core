/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataPointItemConverter.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:29:00 $
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
#include "DataPointItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"

#include "GraphicPropertyItemConverter.hxx"
#include "CharacterPropertyItemConverter.hxx"
#include "StatisticsItemConverter.hxx"

#ifndef _SVX_CHRTITEM_HXX
#define ITEMID_CHARTDATADESCR SCHATTR_DATADESCR_DESCR
#include <svx/chrtitem.hxx>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATAPOINTLABEL_HPP_
#include <com/sun/star/chart2/DataPointLabel.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_SYMBOL_HPP_
#include <com/sun/star/chart2/Symbol.hpp>
#endif

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX
#define ITEMID_SIZE SCHATTR_SYMBOL_SIZE
#include <svx/sizeitem.hxx>
#endif

// for SVX_SYMBOLTYPE_...
#ifndef _SVX_TAB_LINE_HXX
#include <svx/tabline.hxx>
#endif

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;

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

sal_Int32 lcl_getSymbolStyleForSymbol( const chart2::Symbol & rSymbol )
{
    sal_Int32 nStyle = SVX_SYMBOLTYPE_UNKNOWN;
    switch( rSymbol.aStyle )
    {
        case chart2::SymbolStyle_NONE:
            nStyle = SVX_SYMBOLTYPE_NONE;
            break;
        case chart2::SymbolStyle_AUTO:
            nStyle = SVX_SYMBOLTYPE_AUTO;
            break;
        case chart2::SymbolStyle_BITMAP:
            nStyle = SVX_SYMBOLTYPE_BRUSHITEM;
            break;
        case chart2::SymbolStyle_STANDARD:
            nStyle = rSymbol.nStandardSymbol;
            break;

        case chart2::SymbolStyle_POLYGON:
            // to avoid warning
        case chart2::SymbolStyle_MAKE_FIXED_SIZE:
            // nothing
            break;
    }
    return nStyle;
}
} // anonymous namespace

namespace chart
{
namespace wrapper
{

DataPointItemConverter::DataPointItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    NumberFormatterWrapper * pNumFormatter,
    GraphicPropertyItemConverter::eGraphicObjectType eMapTo /* = FILL_PROPERTIES */,
    ::std::auto_ptr< awt::Size > pRefSize /* = NULL */,
    bool bIncludeStatistics /* = false */ ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_pNumberFormatterWrapper( pNumFormatter ),
        m_bIncludeStatistics( bIncludeStatistics )
{
    m_aConverters.push_back( new GraphicPropertyItemConverter( rPropertySet, rItemPool, rDrawModel, eMapTo ));
    m_aConverters.push_back( new CharacterPropertyItemConverter( rPropertySet, rItemPool, pRefSize,
                                                                 C2U( "ReferenceDiagramSize" )));
    if( m_bIncludeStatistics )
        m_aConverters.push_back( new StatisticsItemConverter( xChartModel, rPropertySet, rItemPool ));
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
    if( m_bIncludeStatistics )
        return nRowWhichPairs;
    return nDataPointWhichPairs;
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
    USHORT nWhichId, const SfxItemSet & rItemSet )
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

            chart2::DataPointLabel aLabel;
            if( GetPropertySet()->getPropertyValue( C2U( "Label" )) >>= aLabel )
            {
                switch( rItem.GetValue())
                {
                    case CHDESCR_NONE:
                        aLabel.ShowNumber = sal_False;
                        aLabel.ShowCategoryName = sal_False;
                        break;
                    case CHDESCR_VALUE:
                        aLabel.ShowNumber = sal_True;
                        aLabel.ShowNumberInPercent = sal_False;
                        aLabel.ShowCategoryName = sal_False;
                        break;
                    case CHDESCR_PERCENT:
                        aLabel.ShowNumber = sal_True;
                        aLabel.ShowNumberInPercent = sal_True;
                        aLabel.ShowCategoryName = sal_False;
                        break;
                    case CHDESCR_TEXT:
                        aLabel.ShowNumber = sal_False;
                        aLabel.ShowCategoryName = sal_True;
                        break;
                    case CHDESCR_TEXTANDPERCENT:
                        aLabel.ShowNumber = sal_True;
                        aLabel.ShowNumberInPercent = sal_True;
                        aLabel.ShowCategoryName = sal_True;
                        break;
                    case CHDESCR_TEXTANDVALUE:
                        aLabel.ShowNumber = sal_True;
                        aLabel.ShowNumberInPercent = sal_False;
                        aLabel.ShowCategoryName = sal_True;
                        break;
                    default:
                        break;
                }

                aValue <<= aLabel;
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "Label" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "Label" ), aValue );
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

            chart2::DataPointLabel aLabel;
            if( GetPropertySet()->getPropertyValue( C2U( "Label" )) >>= aLabel )
            {
                sal_Bool bOldValue = aLabel.ShowLegendSymbol;
                aLabel.ShowLegendSymbol = static_cast< sal_Bool >( rItem.GetValue() );
                if( bOldValue != aLabel.ShowLegendSymbol )
                {
                    GetPropertySet()->setPropertyValue( C2U( "Label" ), uno::makeAny( aLabel ));
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

        case SCHATTR_STYLE_SYMBOL:
        {
            sal_Int32 nStyle =
                reinterpret_cast< const SfxInt32Item & >(
                    rItemSet.Get( nWhichId )).GetValue();
            chart2::Symbol aSymbol;

            GetPropertySet()->getPropertyValue( C2U( "Symbol" )) >>= aSymbol;
            sal_Int32 nOldStyle = lcl_getSymbolStyleForSymbol( aSymbol );

            if( nStyle != nOldStyle )
            {
                bool bDeleteSymbol = false;
                switch( nStyle )
                {
                    case SVX_SYMBOLTYPE_NONE:
                        aSymbol.aStyle = chart2::SymbolStyle_NONE;
                        break;
                    case SVX_SYMBOLTYPE_AUTO:
                        aSymbol.aStyle = chart2::SymbolStyle_AUTO;
                        break;
                    case SVX_SYMBOLTYPE_BRUSHITEM:
                        aSymbol.aStyle = chart2::SymbolStyle_BITMAP;
                        break;
                    case SVX_SYMBOLTYPE_UNKNOWN:
                        bDeleteSymbol = true;
                        break;

                    default:
                        aSymbol.aStyle = chart2::SymbolStyle_STANDARD;
                        aSymbol.nStandardSymbol = nStyle;
                }

                if( bDeleteSymbol )
                    GetPropertySet()->setPropertyValue( C2U( "Symbol" ), uno::Any());
                else
                    GetPropertySet()->setPropertyValue( C2U( "Symbol" ),
                                                        uno::makeAny( aSymbol ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_SYMBOL_SIZE:
        {
            Size aSize = reinterpret_cast< const SvxSizeItem & >(
                rItemSet.Get( nWhichId )).GetSize();
            chart2::Symbol aSymbol;

            GetPropertySet()->getPropertyValue( C2U( "Symbol" )) >>= aSymbol;
            if( aSize.getWidth() != aSymbol.aSize.Width ||
                aSize.getHeight() != aSymbol.aSize.Height )
            {
                aSymbol.aSize.Width = aSize.getWidth();
                aSymbol.aSize.Height = aSize.getHeight();

                GetPropertySet()->setPropertyValue( C2U( "Symbol" ), uno::makeAny( aSymbol ));
                bChanged = true;
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
            chart2::DataPointLabel aLabel;
            if( GetPropertySet()->getPropertyValue( C2U( "Label" )) >>= aLabel )
            {
                SvxChartDataDescr aDescr;

                if( aLabel.ShowNumber )
                {
                    if( aLabel.ShowNumberInPercent )
                    {
                        if( aLabel.ShowCategoryName )
                            aDescr = CHDESCR_TEXTANDPERCENT;
                        else
                            aDescr = CHDESCR_PERCENT;
                    }
                    else
                    {
                        if( aLabel.ShowCategoryName )
                            aDescr = CHDESCR_TEXTANDVALUE;
                        else
                            aDescr = CHDESCR_VALUE;
                    }
                }
                else
                {
                    if( aLabel.ShowCategoryName )
                        aDescr = CHDESCR_TEXT;
                    else
                        aDescr = CHDESCR_NONE;
                }

                rOutItemSet.Put( SvxChartDataDescrItem( aDescr ));
                rOutItemSet.Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_SYM, aLabel.ShowLegendSymbol ));
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

        case SCHATTR_STYLE_SYMBOL:
        {
            chart2::Symbol aSymbol;
            if( GetPropertySet()->getPropertyValue( C2U( "Symbol" )) >>= aSymbol )
                rOutItemSet.Put( SfxInt32Item( nWhichId, lcl_getSymbolStyleForSymbol( aSymbol ) ));
        }
        break;

        case SCHATTR_SYMBOL_SIZE:
        {
            chart2::Symbol aSymbol;
            if( GetPropertySet()->getPropertyValue( C2U( "Symbol" )) >>= aSymbol )
                rOutItemSet.Put(
                    SvxSizeItem( nWhichId, Size( aSymbol.aSize.Width, aSymbol.aSize.Height ) ));
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart
