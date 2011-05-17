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
#include "precompiled_chart2.hxx"
#include "DataPointItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"

#include "GraphicPropertyItemConverter.hxx"
#include "CharacterPropertyItemConverter.hxx"
#include "StatisticsItemConverter.hxx"
#include "SeriesOptionsItemConverter.hxx"
#include "DataSeriesHelper.hxx"
#include "DiagramHelper.hxx"
#include "ChartModelHelper.hxx"
#include "ChartTypeHelper.hxx"
#include <svx/chrtitem.hxx>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/Symbol.hpp>

// header for class XFillColorItem
#include <svx/xflclit.hxx>
#include <svl/intitem.hxx>
#include <editeng/sizeitem.hxx>
// header for class SfxStringItem
#include <svl/stritem.hxx>
#include <editeng/brshitem.hxx>
//SfxIntegerListItem
#include <svl/ilstitem.hxx>
#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>
#include <vcl/graph.hxx>
#include <com/sun/star/graphic/XGraphic.hpp>

// for SVX_SYMBOLTYPE_...
#include <svx/tabline.hxx>

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

namespace
{
::comphelper::ItemPropertyMapType & lcl_GetDataPointPropertyMap()
{
    static ::comphelper::ItemPropertyMapType aDataPointPropertyMap(
        ::comphelper::MakeItemPropertyMap
        IPM_MAP_ENTRY( SCHATTR_STYLE_SHAPE, "Geometry3D", 0 )
        );

    return aDataPointPropertyMap;
};

sal_Int32 lcl_getSymbolStyleForSymbol( const chart2::Symbol & rSymbol )
{
    sal_Int32 nStyle = SVX_SYMBOLTYPE_UNKNOWN;
    switch( rSymbol.Style )
    {
        case chart2::SymbolStyle_NONE:
            nStyle = SVX_SYMBOLTYPE_NONE;
            break;
        case chart2::SymbolStyle_AUTO:
            nStyle = SVX_SYMBOLTYPE_AUTO;
            break;
        case chart2::SymbolStyle_GRAPHIC:
            nStyle = SVX_SYMBOLTYPE_BRUSHITEM;
            break;
        case chart2::SymbolStyle_STANDARD:
            nStyle = rSymbol.StandardSymbol;
            break;

        case chart2::SymbolStyle_POLYGON:
            // to avoid warning
        case chart2::SymbolStyle_MAKE_FIXED_SIZE:
            // nothing
            break;
    }
    return nStyle;
}

bool lcl_NumberFormatFromItemToPropertySet( sal_uInt16 nWhichId, const SfxItemSet & rItemSet, const uno::Reference< beans::XPropertySet > & xPropertySet, bool bOverwriteAttributedDataPointsAlso  )
{
    bool bChanged = false;
    if( !xPropertySet.is() )
        return bChanged;
    rtl::OUString aPropertyName = (SID_ATTR_NUMBERFORMAT_VALUE==nWhichId) ? C2U( "NumberFormat" ) : C2U( "PercentageNumberFormat" );
    sal_uInt16 nSourceWhich = (SID_ATTR_NUMBERFORMAT_VALUE==nWhichId) ? SID_ATTR_NUMBERFORMAT_SOURCE : SCHATTR_PERCENT_NUMBERFORMAT_SOURCE;

    if( SFX_ITEM_SET != rItemSet.GetItemState( nSourceWhich ) )
        return bChanged;

    uno::Any aValue;
    bool bUseSourceFormat = (static_cast< const SfxBoolItem & >(
                rItemSet.Get( nSourceWhich )).GetValue() );
    if( !bUseSourceFormat )
    {
        SfxItemState aState = rItemSet.GetItemState( nWhichId );
        if( aState == SFX_ITEM_SET )
        {
            sal_Int32 nFmt = static_cast< sal_Int32 >(
                static_cast< const SfxUInt32Item & >(
                    rItemSet.Get( nWhichId )).GetValue());
            aValue = uno::makeAny(nFmt);
        }
        else
            return bChanged;
    }

    uno::Any aOldValue( xPropertySet->getPropertyValue(aPropertyName) );
    if( bOverwriteAttributedDataPointsAlso )
    {
        Reference< chart2::XDataSeries > xSeries( xPropertySet, uno::UNO_QUERY);
        if( aValue != aOldValue ||
            ::chart::DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries, aPropertyName, aOldValue ) )
        {
            ::chart::DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, aPropertyName, aValue );
            bChanged = true;
        }
    }
    else if( aOldValue != aValue )
    {
        xPropertySet->setPropertyValue(aPropertyName, aValue );
        bChanged = true;
    }
    return bChanged;
}

bool lcl_UseSourceFormatFromItemToPropertySet( sal_uInt16 nWhichId, const SfxItemSet & rItemSet, const uno::Reference< beans::XPropertySet > & xPropertySet, bool bOverwriteAttributedDataPointsAlso  )
{
    bool bChanged = false;
    if( !xPropertySet.is() )
        return bChanged;
    rtl::OUString aPropertyName = (SID_ATTR_NUMBERFORMAT_SOURCE==nWhichId) ? C2U( "NumberFormat" ) : C2U( "PercentageNumberFormat" );
    sal_uInt16 nFormatWhich = (SID_ATTR_NUMBERFORMAT_SOURCE==nWhichId) ? SID_ATTR_NUMBERFORMAT_VALUE : SCHATTR_PERCENT_NUMBERFORMAT_VALUE;

    if( SFX_ITEM_SET != rItemSet.GetItemState( nWhichId ) )
        return bChanged;

    uno::Any aNewValue;
    bool bUseSourceFormat = (static_cast< const SfxBoolItem & >(
                rItemSet.Get( nWhichId )).GetValue() );
    if( !bUseSourceFormat )
    {
        SfxItemState aState = rItemSet.GetItemState( nFormatWhich );
        if( aState == SFX_ITEM_SET )
        {
            sal_Int32 nFormatKey = static_cast< sal_Int32 >(
            static_cast< const SfxUInt32Item & >(
                rItemSet.Get( nFormatWhich )).GetValue());
            aNewValue <<= nFormatKey;
        }
        else
            return bChanged;
    }

    uno::Any aOldValue( xPropertySet->getPropertyValue(aPropertyName) );
    if( bOverwriteAttributedDataPointsAlso )
    {
        Reference< chart2::XDataSeries > xSeries( xPropertySet, uno::UNO_QUERY);
        if( aNewValue != aOldValue ||
            ::chart::DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries, aPropertyName, aOldValue ) )
        {
            ::chart::DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, aPropertyName, aNewValue );
            bChanged = true;
        }
    }
    else if( aOldValue != aNewValue )
    {
        xPropertySet->setPropertyValue( aPropertyName, aNewValue );
        bChanged = true;
    }

    return bChanged;
}

} // anonymous namespace

namespace chart
{
namespace wrapper
{

DataPointItemConverter::DataPointItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    const uno::Reference< uno::XComponentContext > & xContext,
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    const uno::Reference< XDataSeries > & xSeries,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    NumberFormatterWrapper * pNumFormatter,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
    GraphicPropertyItemConverter::eGraphicObjectType eMapTo /* = FILL_PROPERTIES */,
    ::std::auto_ptr< awt::Size > pRefSize /* = NULL */,
    bool bDataSeries /* = false */,
    bool bUseSpecialFillColor /* = false */,
    sal_Int32 nSpecialFillColor /* =0 */,
    bool bOverwriteLabelsForAttributedDataPointsAlso /*false*/,
    sal_Int32 nNumberFormat,
    sal_Int32 nPercentNumberFormat
    ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_pNumberFormatterWrapper( pNumFormatter ),
        m_bDataSeries( bDataSeries ),
        m_bOverwriteLabelsForAttributedDataPointsAlso(m_bDataSeries && bOverwriteLabelsForAttributedDataPointsAlso),
        m_bUseSpecialFillColor(bUseSpecialFillColor),
        m_nSpecialFillColor(nSpecialFillColor),
        m_nNumberFormat(nNumberFormat),
        m_nPercentNumberFormat(nPercentNumberFormat),
        m_aAvailableLabelPlacements(),
        m_bForbidPercentValue(true)
{
    m_aConverters.push_back( new GraphicPropertyItemConverter(
                                 rPropertySet, rItemPool, rDrawModel, xNamedPropertyContainerFactory, eMapTo ));
    m_aConverters.push_back( new CharacterPropertyItemConverter( rPropertySet, rItemPool, pRefSize,
                                                                 C2U( "ReferencePageSize" )));
    if( bDataSeries )
    {
        m_aConverters.push_back( new StatisticsItemConverter( xChartModel, rPropertySet, rItemPool ));
        m_aConverters.push_back( new SeriesOptionsItemConverter( xChartModel, xContext, rPropertySet, rItemPool ));
    }

    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram(xChartModel) );
    uno::Reference< XChartType > xChartType( DiagramHelper::getChartTypeOfSeries( xDiagram , xSeries ) );
    bool bFound = false;
    bool bAmbiguous = false;
    sal_Bool bSwapXAndY = DiagramHelper::getVertical( xDiagram, bFound, bAmbiguous );
    m_aAvailableLabelPlacements = ChartTypeHelper::getSupportedLabelPlacements( xChartType, DiagramHelper::getDimension( xDiagram ), bSwapXAndY, xSeries );

    m_bForbidPercentValue = AxisType::CATEGORY != ChartTypeHelper::getAxisType( xChartType, 0 );
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

    if( m_bUseSpecialFillColor )
    {
        Color aColor(m_nSpecialFillColor);
        rOutItemSet.Put( XFillColorItem( String(), aColor ) );
    }
}

bool DataPointItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::ApplyItemSetFunc( rItemSet, bResult ));

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const sal_uInt16 * DataPointItemConverter::GetWhichPairs() const
{
    // must span all used items!
    if( m_bDataSeries )
        return nRowWhichPairs;
    return nDataPointWhichPairs;
}

bool DataPointItemConverter::GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const
{
    ::comphelper::ItemPropertyMapType & rMap( lcl_GetDataPointPropertyMap());
    ::comphelper::ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

    if( aIt == rMap.end())
        return false;

    rOutProperty =(*aIt).second;
    return true;
}


bool DataPointItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    bool bChanged = false;
    uno::Any aValue;

    switch( nWhichId )
    {
        case SCHATTR_DATADESCR_SHOW_NUMBER:
        case SCHATTR_DATADESCR_SHOW_PERCENTAGE:
        case SCHATTR_DATADESCR_SHOW_CATEGORY:
        case SCHATTR_DATADESCR_SHOW_SYMBOL:
        {
            const SfxBoolItem & rItem = static_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId ));

            uno::Any aOldValue( GetPropertySet()->getPropertyValue( C2U( "Label" ) ));
            chart2::DataPointLabel aLabel;
            if( aOldValue >>= aLabel )
            {
                sal_Bool& rValue = (SCHATTR_DATADESCR_SHOW_NUMBER==nWhichId) ? aLabel.ShowNumber : (
                    (SCHATTR_DATADESCR_SHOW_PERCENTAGE==nWhichId) ? aLabel.ShowNumberInPercent : (
                    (SCHATTR_DATADESCR_SHOW_CATEGORY==nWhichId) ? aLabel.ShowCategoryName : aLabel.ShowLegendSymbol ));
                sal_Bool bOldValue = rValue;
                rValue = static_cast< sal_Bool >( rItem.GetValue() );
                if( m_bOverwriteLabelsForAttributedDataPointsAlso )
                {
                    Reference< chart2::XDataSeries > xSeries( GetPropertySet(), uno::UNO_QUERY);
                    if( bOldValue != rValue ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries, C2U( "Label" ), aOldValue ) )
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, C2U( "Label" ), uno::makeAny( aLabel ) );
                        bChanged = true;
                    }
                }
                else if( bOldValue != rValue )
                {
                    GetPropertySet()->setPropertyValue( C2U( "Label" ), uno::makeAny( aLabel ));
                    bChanged = true;
                }
            }
        }
        break;

        case SID_ATTR_NUMBERFORMAT_VALUE:
        case SCHATTR_PERCENT_NUMBERFORMAT_VALUE:  //fall through intended
        {
            bChanged = lcl_NumberFormatFromItemToPropertySet( nWhichId, rItemSet, GetPropertySet(), m_bOverwriteLabelsForAttributedDataPointsAlso );
        }
        break;

        case SID_ATTR_NUMBERFORMAT_SOURCE:
        case SCHATTR_PERCENT_NUMBERFORMAT_SOURCE: //fall through intended
        {
            bChanged = lcl_UseSourceFormatFromItemToPropertySet( nWhichId, rItemSet, GetPropertySet(), m_bOverwriteLabelsForAttributedDataPointsAlso );
        }
        break;

        case SCHATTR_DATADESCR_SEPARATOR:
        {
            rtl::OUString aNewValue = static_cast< const SfxStringItem & >( rItemSet.Get( nWhichId )).GetValue();
            rtl::OUString aOldValue;
            try
            {
                GetPropertySet()->getPropertyValue( C2U( "LabelSeparator" ) ) >>= aOldValue;
                if( m_bOverwriteLabelsForAttributedDataPointsAlso )
                {
                    Reference< chart2::XDataSeries > xSeries( GetPropertySet(), uno::UNO_QUERY);
                    if( !aOldValue.equals(aNewValue) ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries, C2U( "LabelSeparator" ), uno::makeAny( aOldValue ) ) )
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, C2U( "LabelSeparator" ), uno::makeAny( aNewValue ) );
                        bChanged = true;
                    }
                }
                else if( !aOldValue.equals(aNewValue) )
                {
                    GetPropertySet()->setPropertyValue( C2U( "LabelSeparator" ), uno::makeAny( aNewValue ));
                    bChanged = true;
                }
            }
            catch( uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
        break;

        case SCHATTR_DATADESCR_PLACEMENT:
        {

            try
            {
                sal_Int32 nNew = static_cast< const SfxInt32Item & >( rItemSet.Get( nWhichId )).GetValue();
                sal_Int32 nOld =0;
                if( !(GetPropertySet()->getPropertyValue( C2U( "LabelPlacement" ) ) >>= nOld) )
                {
                    if( m_aAvailableLabelPlacements.getLength() )
                        nOld = m_aAvailableLabelPlacements[0];
                }
                if( m_bOverwriteLabelsForAttributedDataPointsAlso )
                {
                    Reference< chart2::XDataSeries > xSeries( GetPropertySet(), uno::UNO_QUERY);
                    if( nOld!=nNew ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries, C2U( "LabelPlacement" ), uno::makeAny( nOld ) ) )
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, C2U( "LabelPlacement" ), uno::makeAny( nNew ) );
                        bChanged = true;
                    }
                }
                else if( nOld!=nNew )
                {
                    GetPropertySet()->setPropertyValue( C2U( "LabelPlacement" ), uno::makeAny( nNew ));
                    bChanged = true;
                }
            }
            catch( uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
        break;

        case SCHATTR_STYLE_SYMBOL:
        {
            sal_Int32 nStyle =
                static_cast< const SfxInt32Item & >(
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
                        aSymbol.Style = chart2::SymbolStyle_NONE;
                        break;
                    case SVX_SYMBOLTYPE_AUTO:
                        aSymbol.Style = chart2::SymbolStyle_AUTO;
                        break;
                    case SVX_SYMBOLTYPE_BRUSHITEM:
                        aSymbol.Style = chart2::SymbolStyle_GRAPHIC;
                        break;
                    case SVX_SYMBOLTYPE_UNKNOWN:
                        bDeleteSymbol = true;
                        break;

                    default:
                        aSymbol.Style = chart2::SymbolStyle_STANDARD;
                        aSymbol.StandardSymbol = nStyle;
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
            Size aSize = static_cast< const SvxSizeItem & >(
                rItemSet.Get( nWhichId )).GetSize();
            chart2::Symbol aSymbol;

            GetPropertySet()->getPropertyValue( C2U( "Symbol" )) >>= aSymbol;
            if( aSize.getWidth() != aSymbol.Size.Width ||
                aSize.getHeight() != aSymbol.Size.Height )
            {
                aSymbol.Size.Width = aSize.getWidth();
                aSymbol.Size.Height = aSize.getHeight();

                GetPropertySet()->setPropertyValue( C2U( "Symbol" ), uno::makeAny( aSymbol ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_SYMBOL_BRUSH:
        {
            const SvxBrushItem & rBrshItem( static_cast< const SvxBrushItem & >(
                                                rItemSet.Get( nWhichId )));
            uno::Any aXGraphicAny;
            const Graphic *pGraphic( rBrshItem.GetGraphic());
            if( pGraphic )
            {
                uno::Reference< graphic::XGraphic > xGraphic( pGraphic->GetXGraphic());
                if( xGraphic.is())
                {
                    aXGraphicAny <<= xGraphic;
                    chart2::Symbol aSymbol;
                    GetPropertySet()->getPropertyValue( C2U( "Symbol" )) >>= aSymbol;
                    if( aSymbol.Graphic != xGraphic )
                    {
                        aSymbol.Graphic = xGraphic;
                        GetPropertySet()->setPropertyValue( C2U( "Symbol" ), uno::makeAny( aSymbol ));
                        bChanged = true;
                    }
                }
            }
        }
        break;

        case SCHATTR_TEXT_DEGREES:
        {
            double fValue = static_cast< double >(
                static_cast< const SfxInt32Item & >(
                    rItemSet.Get( nWhichId )).GetValue()) / 100.0;
            double fOldValue = 0.0;
            bool bPropExisted =
                ( GetPropertySet()->getPropertyValue( C2U( "TextRotation" )) >>= fOldValue );

            if( ! bPropExisted ||
                ( bPropExisted && fOldValue != fValue ))
            {
                GetPropertySet()->setPropertyValue( C2U( "TextRotation" ), uno::makeAny( fValue ));
                bChanged = true;
            }
        }
        break;
    }

    return bChanged;
}

void DataPointItemConverter::FillSpecialItem(
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    switch( nWhichId )
    {
        case SCHATTR_DATADESCR_SHOW_NUMBER:
        case SCHATTR_DATADESCR_SHOW_PERCENTAGE:
        case SCHATTR_DATADESCR_SHOW_CATEGORY:
        case SCHATTR_DATADESCR_SHOW_SYMBOL:
        {
            chart2::DataPointLabel aLabel;
            if( GetPropertySet()->getPropertyValue( C2U( "Label" )) >>= aLabel )
            {
                sal_Bool bValue = (SCHATTR_DATADESCR_SHOW_NUMBER==nWhichId) ? aLabel.ShowNumber : (
                    (SCHATTR_DATADESCR_SHOW_PERCENTAGE==nWhichId) ? aLabel.ShowNumberInPercent : (
                    (SCHATTR_DATADESCR_SHOW_CATEGORY==nWhichId) ? aLabel.ShowCategoryName : aLabel.ShowLegendSymbol ));

                rOutItemSet.Put( SfxBoolItem( nWhichId, bValue ));

                if( m_bOverwriteLabelsForAttributedDataPointsAlso )
                {
                    if( DataSeriesHelper::hasAttributedDataPointDifferentValue(
                        Reference< chart2::XDataSeries >( GetPropertySet(), uno::UNO_QUERY), C2U( "Label" ), uno::makeAny(aLabel) ) )
                    {
                        rOutItemSet.InvalidateItem(nWhichId);
                    }
                }
            }
        }
        break;

        case SID_ATTR_NUMBERFORMAT_VALUE:
        {
            sal_Int32 nKey = 0;
            if( !(GetPropertySet()->getPropertyValue( C2U( "NumberFormat" )) >>= nKey) )
                nKey = m_nNumberFormat;
            rOutItemSet.Put( SfxUInt32Item( nWhichId, nKey ));
        }
        break;

        case SCHATTR_PERCENT_NUMBERFORMAT_VALUE:
        {
            sal_Int32 nKey = 0;
            if( !(GetPropertySet()->getPropertyValue( C2U( "PercentageNumberFormat" )) >>= nKey) )
                nKey = m_nPercentNumberFormat;
            rOutItemSet.Put( SfxUInt32Item( nWhichId, nKey ));
        }
        break;

        case SID_ATTR_NUMBERFORMAT_SOURCE:
        {
            bool bNumberFormatIsSet = ( GetPropertySet()->getPropertyValue( C2U( "NumberFormat" )).hasValue());
            rOutItemSet.Put( SfxBoolItem( nWhichId, ! bNumberFormatIsSet ));
        }
        break;
        case SCHATTR_PERCENT_NUMBERFORMAT_SOURCE:
        {
            bool bNumberFormatIsSet = ( GetPropertySet()->getPropertyValue( C2U( "PercentageNumberFormat" )).hasValue());
            rOutItemSet.Put( SfxBoolItem( nWhichId, ! bNumberFormatIsSet ));
        }
        break;

        case SCHATTR_DATADESCR_SEPARATOR:
        {
            rtl::OUString aValue;
            try
            {
                GetPropertySet()->getPropertyValue( C2U( "LabelSeparator" ) ) >>= aValue;
                rOutItemSet.Put( SfxStringItem( nWhichId, aValue ));
            }
            catch( uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
        break;

        case SCHATTR_DATADESCR_PLACEMENT:
        {
            try
            {
                sal_Int32 nPlacement=0;
                if( GetPropertySet()->getPropertyValue( C2U( "LabelPlacement" ) ) >>= nPlacement )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, nPlacement ));
                else if( m_aAvailableLabelPlacements.getLength() )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, m_aAvailableLabelPlacements[0] ));
            }
            catch( uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
        break;

        case SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS:
        {
            SvULongs aList;
            for ( sal_Int32 nN=0; nN<m_aAvailableLabelPlacements.getLength(); nN++ )
                aList.Insert( m_aAvailableLabelPlacements[nN], sal::static_int_cast< sal_uInt16 >(nN) );
            rOutItemSet.Put( SfxIntegerListItem( nWhichId, aList ) );
        }
        break;

        case SCHATTR_DATADESCR_NO_PERCENTVALUE:
        {
            rOutItemSet.Put( SfxBoolItem( nWhichId, m_bForbidPercentValue ));
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
                    SvxSizeItem( nWhichId, Size( aSymbol.Size.Width, aSymbol.Size.Height ) ));
        }
        break;

        case SCHATTR_SYMBOL_BRUSH:
        {
            chart2::Symbol aSymbol;
            if(( GetPropertySet()->getPropertyValue( C2U( "Symbol" )) >>= aSymbol )
               && aSymbol.Graphic.is() )
            {
                rOutItemSet.Put( SvxBrushItem( Graphic( aSymbol.Graphic ), GPOS_MM, SCHATTR_SYMBOL_BRUSH ));
            }
        }
        break;

        case SCHATTR_TEXT_DEGREES:
        {
            double fValue = 0;

            if( GetPropertySet()->getPropertyValue( C2U( "TextRotation" )) >>= fValue )
            {
                rOutItemSet.Put( SfxInt32Item( nWhichId, static_cast< sal_Int32 >(
                                                   ::rtl::math::round( fValue * 100.0 ) ) ));
            }
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
