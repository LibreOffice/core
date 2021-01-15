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

#include <DataPointItemConverter.hxx>
#include "SchWhichPairs.hxx"
#include <ItemPropertyMap.hxx>

#include <GraphicPropertyItemConverter.hxx>
#include <CharacterPropertyItemConverter.hxx>
#include <StatisticsItemConverter.hxx>
#include <SeriesOptionsItemConverter.hxx>
#include <DataSeriesHelper.hxx>
#include <DiagramHelper.hxx>
#include <ChartModelHelper.hxx>
#include <ChartTypeHelper.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <svx/xflclit.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/brushitem.hxx>
#include <svl/ilstitem.hxx>
#include <svx/sdangitm.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/graph.hxx>
#include <oox/helper/containerhelper.hxx>
#include <rtl/math.hxx>

#include <svx/tabline.hxx>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

namespace chart::wrapper {

namespace {

ItemPropertyMapType & lcl_GetDataPointPropertyMap()
{
    static ItemPropertyMapType aDataPointPropertyMap{
        {SCHATTR_STYLE_SHAPE, {"Geometry3D", 0}}};
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
        case chart2::SymbolStyle::SymbolStyle_MAKE_FIXED_SIZE:
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
    OUString aPropertyName = (nWhichId==SID_ATTR_NUMBERFORMAT_VALUE) ? OUString(CHART_UNONAME_NUMFMT) : OUString( "PercentageNumberFormat" );
    sal_uInt16 nSourceWhich = (nWhichId==SID_ATTR_NUMBERFORMAT_VALUE) ? SID_ATTR_NUMBERFORMAT_SOURCE : SCHATTR_PERCENT_NUMBERFORMAT_SOURCE;

    if( rItemSet.GetItemState( nSourceWhich ) != SfxItemState::SET )
        return bChanged;

    uno::Any aValue;
    bool bUseSourceFormat = static_cast< const SfxBoolItem & >(
                rItemSet.Get( nSourceWhich )).GetValue();
    if( !bUseSourceFormat )
    {
        SfxItemState aState = rItemSet.GetItemState( nWhichId );
        if( aState == SfxItemState::SET )
        {
            sal_Int32 nFmt = static_cast< sal_Int32 >(
                static_cast< const SfxUInt32Item & >(
                    rItemSet.Get( nWhichId )).GetValue());
            aValue <<= nFmt;
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
    OUString aPropertyName = (nWhichId==SID_ATTR_NUMBERFORMAT_SOURCE) ? OUString(CHART_UNONAME_NUMFMT) : OUString( "PercentageNumberFormat" );
    sal_uInt16 nFormatWhich = (nWhichId==SID_ATTR_NUMBERFORMAT_SOURCE) ? SID_ATTR_NUMBERFORMAT_VALUE : SCHATTR_PERCENT_NUMBERFORMAT_VALUE;

    if( rItemSet.GetItemState( nWhichId ) != SfxItemState::SET )
        return bChanged;

    uno::Any aNewValue;
    bool bUseSourceFormat = static_cast< const SfxBoolItem & >(
                rItemSet.Get( nWhichId )).GetValue();
    xPropertySet->setPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT, uno::Any(bUseSourceFormat));
    if( !bUseSourceFormat )
    {
        SfxItemState aState = rItemSet.GetItemState( nFormatWhich );
        if( aState == SfxItemState::SET )
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

DataPointItemConverter::DataPointItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    const uno::Reference< uno::XComponentContext > & xContext,
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    const uno::Reference< XDataSeries > & xSeries,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference<lang::XMultiServiceFactory>& xNamedPropertyContainerFactory,
    GraphicObjectType eMapTo,
    const awt::Size* pRefSize,
    bool bDataSeries,
    bool bUseSpecialFillColor,
    sal_Int32 nSpecialFillColor,
    bool bOverwriteLabelsForAttributedDataPointsAlso,
    sal_Int32 nNumberFormat,
    sal_Int32 nPercentNumberFormat,
    sal_Int32 nPointIndex ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_bDataSeries( bDataSeries ),
        m_bOverwriteLabelsForAttributedDataPointsAlso(m_bDataSeries && bOverwriteLabelsForAttributedDataPointsAlso),
        m_bUseSpecialFillColor(bUseSpecialFillColor),
        m_nSpecialFillColor(ColorTransparency, nSpecialFillColor),
        m_nNumberFormat(nNumberFormat),
        m_nPercentNumberFormat(nPercentNumberFormat),
        m_aAvailableLabelPlacements(),
        m_bForbidPercentValue(true),
        m_bHideLegendEntry(false),
        m_nPointIndex(nPointIndex),
        m_xSeries(xSeries)
{
    m_aConverters.emplace_back( new GraphicPropertyItemConverter(
                                 rPropertySet, rItemPool, rDrawModel, xNamedPropertyContainerFactory, eMapTo ));
    m_aConverters.emplace_back( new CharacterPropertyItemConverter(rPropertySet, rItemPool, pRefSize, "ReferencePageSize"));
    if( bDataSeries )
    {
        m_aConverters.emplace_back( new StatisticsItemConverter( xChartModel, rPropertySet, rItemPool ));
        m_aConverters.emplace_back( new SeriesOptionsItemConverter( xChartModel, xContext, rPropertySet, rItemPool ));
    }

    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram(xChartModel) );
    uno::Reference< XChartType > xChartType( DiagramHelper::getChartTypeOfSeries( xDiagram , xSeries ) );
    bool bFound = false;
    bool bAmbiguous = false;
    bool bSwapXAndY = DiagramHelper::getVertical( xDiagram, bFound, bAmbiguous );
    m_aAvailableLabelPlacements = ChartTypeHelper::getSupportedLabelPlacements( xChartType, bSwapXAndY, xSeries );

    m_bForbidPercentValue = ChartTypeHelper::getAxisType( xChartType, 0 ) != AxisType::CATEGORY;

    if (bDataSeries)
        return;

    uno::Reference<beans::XPropertySet> xSeriesProp(xSeries, uno::UNO_QUERY);
    uno::Sequence<sal_Int32> deletedLegendEntriesSeq;
    xSeriesProp->getPropertyValue("DeletedLegendEntries") >>= deletedLegendEntriesSeq;
    for (auto& deletedLegendEntry : deletedLegendEntriesSeq)
    {
        if (nPointIndex == deletedLegendEntry)
        {
            m_bHideLegendEntry = true;
            break;
        }
    }
}

DataPointItemConverter::~DataPointItemConverter()
{
}

void DataPointItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    for( const auto& pConv : m_aConverters )
        pConv->FillItemSet( rOutItemSet );

    // own items
    ItemConverter::FillItemSet( rOutItemSet );

    if( m_bUseSpecialFillColor )
    {
        Color aColor(m_nSpecialFillColor);
        rOutItemSet.Put( XFillColorItem( OUString(), aColor ) );
    }
}

bool DataPointItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    for( const auto& pConv : m_aConverters )
        bResult = pConv->ApplyItemSet( rItemSet ) || bResult;

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
    ItemPropertyMapType & rMap( lcl_GetDataPointPropertyMap());
    ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

    if( aIt == rMap.end())
        return false;

    rOutProperty =(*aIt).second;
    return true;
}

bool DataPointItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
{
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_DATADESCR_SHOW_NUMBER:
        case SCHATTR_DATADESCR_SHOW_PERCENTAGE:
        case SCHATTR_DATADESCR_SHOW_CATEGORY:
        case SCHATTR_DATADESCR_SHOW_SYMBOL:
        {
            const SfxBoolItem & rItem = static_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId ));

            uno::Any aOldValue = GetPropertySet()->getPropertyValue(CHART_UNONAME_LABEL);
            chart2::DataPointLabel aLabel;
            if( aOldValue >>= aLabel )
            {
                sal_Bool& rValue = (nWhichId==SCHATTR_DATADESCR_SHOW_NUMBER) ? aLabel.ShowNumber : (
                    (nWhichId==SCHATTR_DATADESCR_SHOW_PERCENTAGE) ? aLabel.ShowNumberInPercent : (
                    (nWhichId==SCHATTR_DATADESCR_SHOW_CATEGORY) ? aLabel.ShowCategoryName : aLabel.ShowLegendSymbol ));
                bool bOldValue = rValue;
                rValue = rItem.GetValue();
                if( m_bOverwriteLabelsForAttributedDataPointsAlso )
                {
                    Reference< chart2::XDataSeries > xSeries( GetPropertySet(), uno::UNO_QUERY);
                    if( bOldValue != bool(rValue) ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries, CHART_UNONAME_LABEL , aOldValue ) )
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, CHART_UNONAME_LABEL , uno::Any( aLabel ) );
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, CHART_UNONAME_CUSTOM_LABEL_FIELDS, uno::Any() );
                        bChanged = true;
                    }
                }
                else if( bOldValue != bool(rValue) )
                {
                    GetPropertySet()->setPropertyValue(CHART_UNONAME_LABEL , uno::Any(aLabel));
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
            OUString aNewValue = static_cast< const SfxStringItem & >( rItemSet.Get( nWhichId )).GetValue();
            try
            {
                OUString aOldValue;
                GetPropertySet()->getPropertyValue( "LabelSeparator" ) >>= aOldValue;
                if( m_bOverwriteLabelsForAttributedDataPointsAlso )
                {
                    Reference< chart2::XDataSeries > xSeries( GetPropertySet(), uno::UNO_QUERY);
                    if( aOldValue != aNewValue ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries, "LabelSeparator" , uno::Any( aOldValue ) ) )
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "LabelSeparator" , uno::Any( aNewValue ) );
                        bChanged = true;
                    }
                }
                else if( aOldValue != aNewValue )
                {
                    GetPropertySet()->setPropertyValue( "LabelSeparator" , uno::Any( aNewValue ));
                    bChanged = true;
                }
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
        break;

        case SCHATTR_DATADESCR_WRAP_TEXT:
        {

            try
            {
                bool bNew = static_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue();
                bool bOld = false;
                GetPropertySet()->getPropertyValue( "TextWordWrap" ) >>= bOld;
                if( m_bOverwriteLabelsForAttributedDataPointsAlso )
                {
                    Reference< chart2::XDataSeries > xSeries( GetPropertySet(), uno::UNO_QUERY);
                    if( bOld!=bNew ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries, "TextWordWrap", uno::Any( bOld ) ) )
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "TextWordWrap", uno::Any( bNew ) );
                        bChanged = true;
                    }
                }
                else if( bOld!=bNew )
                {
                    GetPropertySet()->setPropertyValue( "TextWordWrap", uno::Any( bNew ));
                    bChanged = true;
                }
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
        break;

        case SCHATTR_DATADESCR_PLACEMENT:
        {

            try
            {
                sal_Int32 nNew = static_cast< const SfxInt32Item & >( rItemSet.Get( nWhichId )).GetValue();
                sal_Int32 nOld = -1;
                RelativePosition aCustomLabelPosition;
                GetPropertySet()->getPropertyValue("LabelPlacement") >>= nOld;
                if( m_bOverwriteLabelsForAttributedDataPointsAlso )
                {
                    Reference< chart2::XDataSeries > xSeries( GetPropertySet(), uno::UNO_QUERY);
                    if( nOld!=nNew ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries, "LabelPlacement" , uno::Any( nOld ) ) )
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "LabelPlacement" , uno::Any( nNew ) );
                        bChanged = true;
                    }
                }
                else if( nOld!=nNew || (GetPropertySet()->getPropertyValue("CustomLabelPosition") >>= aCustomLabelPosition) )
                {
                    GetPropertySet()->setPropertyValue("LabelPlacement", uno::Any(nNew));
                    GetPropertySet()->setPropertyValue("CustomLabelPosition", uno::Any());
                    bChanged = true;
                }
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
        break;

        case SCHATTR_STYLE_SYMBOL:
        {
            sal_Int32 nStyle =
                static_cast< const SfxInt32Item & >(
                    rItemSet.Get( nWhichId )).GetValue();
            chart2::Symbol aSymbol;

            GetPropertySet()->getPropertyValue( "Symbol" ) >>= aSymbol;
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
                    GetPropertySet()->setPropertyValue( "Symbol" , uno::Any());
                else
                    GetPropertySet()->setPropertyValue( "Symbol" , uno::Any( aSymbol ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_SYMBOL_SIZE:
        {
            Size aSize = static_cast< const SvxSizeItem & >(
                rItemSet.Get( nWhichId )).GetSize();
            chart2::Symbol aSymbol;

            GetPropertySet()->getPropertyValue( "Symbol" ) >>= aSymbol;
            if( aSize.getWidth() != aSymbol.Size.Width ||
                aSize.getHeight() != aSymbol.Size.Height )
            {
                aSymbol.Size.Width = aSize.getWidth();
                aSymbol.Size.Height = aSize.getHeight();

                GetPropertySet()->setPropertyValue( "Symbol" , uno::Any( aSymbol ));
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
                    GetPropertySet()->getPropertyValue( "Symbol" ) >>= aSymbol;
                    if( aSymbol.Graphic != xGraphic )
                    {
                        aSymbol.Graphic = xGraphic;
                        GetPropertySet()->setPropertyValue( "Symbol" , uno::Any( aSymbol ));
                        bChanged = true;
                    }
                }
            }
        }
        break;

        case SCHATTR_TEXT_DEGREES:
        {
            double fValue = static_cast< double >(
                static_cast< const SdrAngleItem & >(
                    rItemSet.Get( nWhichId )).GetValue().get()) / 100.0;
            double fOldValue = 0.0;
            bool bPropExisted =
                ( GetPropertySet()->getPropertyValue( "TextRotation" ) >>= fOldValue );

            if( ! bPropExisted || fOldValue != fValue )
            {
                GetPropertySet()->setPropertyValue( "TextRotation" , uno::Any( fValue ));
                bChanged = true;
            }
        }
        break;

        case SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY:
        {
            bool bHideLegendEntry = static_cast<const SfxBoolItem &>(rItemSet.Get(nWhichId)).GetValue();
            if (bHideLegendEntry != m_bHideLegendEntry)
            {
                uno::Sequence<sal_Int32> deletedLegendEntriesSeq;
                Reference<beans::XPropertySet> xSeriesProp(m_xSeries, uno::UNO_QUERY);
                xSeriesProp->getPropertyValue("DeletedLegendEntries") >>= deletedLegendEntriesSeq;
                std::vector<sal_Int32> deletedLegendEntries;
                for (auto& deletedLegendEntry : deletedLegendEntriesSeq)
                {
                    if (bHideLegendEntry || m_nPointIndex != deletedLegendEntry)
                        deletedLegendEntries.push_back(deletedLegendEntry);
                }
                if (bHideLegendEntry)
                    deletedLegendEntries.push_back(m_nPointIndex);
                xSeriesProp->setPropertyValue("DeletedLegendEntries", uno::makeAny(oox::ContainerHelper::vectorToSequence(deletedLegendEntries)));
            }
        }
        break;

        case SCHATTR_DATADESCR_CUSTOM_LEADER_LINES:
        {
            try
            {
                bool bNew = static_cast<const SfxBoolItem&>(rItemSet.Get(nWhichId)).GetValue();
                bool bOld = true;
                Reference<beans::XPropertySet> xSeriesProp(m_xSeries, uno::UNO_QUERY);
                if( (xSeriesProp->getPropertyValue("ShowCustomLeaderLines") >>= bOld) && bOld != bNew )
                {
                    xSeriesProp->setPropertyValue("ShowCustomLeaderLines", uno::Any(bNew));
                    bChanged = true;
                }
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("chart2", "");
            }
        }
        break;
    }

    return bChanged;
}

void DataPointItemConverter::FillSpecialItem(
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
{
    switch( nWhichId )
    {
        case SCHATTR_DATADESCR_SHOW_NUMBER:
        case SCHATTR_DATADESCR_SHOW_PERCENTAGE:
        case SCHATTR_DATADESCR_SHOW_CATEGORY:
        case SCHATTR_DATADESCR_SHOW_SYMBOL:
        {
            chart2::DataPointLabel aLabel;
            if (GetPropertySet()->getPropertyValue(CHART_UNONAME_LABEL) >>= aLabel)
            {
                bool bValue = (nWhichId==SCHATTR_DATADESCR_SHOW_NUMBER) ? aLabel.ShowNumber : (
                    (nWhichId==SCHATTR_DATADESCR_SHOW_PERCENTAGE) ? aLabel.ShowNumberInPercent : (
                    (nWhichId==SCHATTR_DATADESCR_SHOW_CATEGORY) ? aLabel.ShowCategoryName : aLabel.ShowLegendSymbol ));

                rOutItemSet.Put( SfxBoolItem( nWhichId, bValue ));

                if( m_bOverwriteLabelsForAttributedDataPointsAlso )
                {
                    if( DataSeriesHelper::hasAttributedDataPointDifferentValue(
                        Reference< chart2::XDataSeries >( GetPropertySet(), uno::UNO_QUERY), CHART_UNONAME_LABEL , uno::Any(aLabel) ) )
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
            if (!(GetPropertySet()->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nKey))
                nKey = m_nNumberFormat;
            rOutItemSet.Put( SfxUInt32Item( nWhichId, nKey ));
        }
        break;

        case SCHATTR_PERCENT_NUMBERFORMAT_VALUE:
        {
            sal_Int32 nKey = 0;
            if( !(GetPropertySet()->getPropertyValue( "PercentageNumberFormat" ) >>= nKey) )
                nKey = m_nPercentNumberFormat;
            rOutItemSet.Put( SfxUInt32Item( nWhichId, nKey ));
        }
        break;

        case SID_ATTR_NUMBERFORMAT_SOURCE:
        {
            bool bUseSourceFormat = false;
            try
            {
                GetPropertySet()->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bUseSourceFormat;
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("chart2", "");
            }
            bool bNumberFormatIsSet = GetPropertySet()->getPropertyValue(CHART_UNONAME_NUMFMT).hasValue() && !bUseSourceFormat;
            rOutItemSet.Put( SfxBoolItem( nWhichId, ! bNumberFormatIsSet ));
        }
        break;
        case SCHATTR_PERCENT_NUMBERFORMAT_SOURCE:
        {
            bool bUseSourceFormat = false;
            try
            {
                GetPropertySet()->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bUseSourceFormat;
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("chart2", "");
            }
            bool bNumberFormatIsSet = GetPropertySet()->getPropertyValue( "PercentageNumberFormat" ).hasValue() && !bUseSourceFormat;
            rOutItemSet.Put( SfxBoolItem( nWhichId, ! bNumberFormatIsSet ));
        }
        break;

        case SCHATTR_DATADESCR_SEPARATOR:
        {
            try
            {
                OUString aValue;
                GetPropertySet()->getPropertyValue( "LabelSeparator" ) >>= aValue;
                rOutItemSet.Put( SfxStringItem( nWhichId, aValue ));
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
        break;

        case SCHATTR_DATADESCR_WRAP_TEXT:
        {
            try
            {
                bool bValue = false;
                GetPropertySet()->getPropertyValue( "TextWordWrap" ) >>= bValue;
                rOutItemSet.Put( SfxBoolItem( nWhichId, bValue ));
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
        break;

        case SCHATTR_DATADESCR_PLACEMENT:
        {
            try
            {
                sal_Int32 nPlacement=0;
                RelativePosition aCustomLabelPosition;
                if( !m_bOverwriteLabelsForAttributedDataPointsAlso && (GetPropertySet()->getPropertyValue("CustomLabelPosition") >>= aCustomLabelPosition) )
                    rOutItemSet.Put(SfxInt32Item(nWhichId, css::chart::DataLabelPlacement::CUSTOM));
                else if( GetPropertySet()->getPropertyValue( "LabelPlacement" ) >>= nPlacement )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, nPlacement ));
                else if( m_aAvailableLabelPlacements.hasElements() )
                    rOutItemSet.Put( SfxInt32Item( nWhichId, m_aAvailableLabelPlacements[0] ));
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
        break;

        case SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS:
        {
            rOutItemSet.Put( SfxIntegerListItem( nWhichId, m_aAvailableLabelPlacements ) );
        }
        break;

        case SCHATTR_DATADESCR_NO_PERCENTVALUE:
        {
            rOutItemSet.Put( SfxBoolItem( nWhichId, m_bForbidPercentValue ));
        }
        break;

        case SCHATTR_DATADESCR_CUSTOM_LEADER_LINES:
        {
            try
            {
                bool bValue = true;
                Reference<beans::XPropertySet> xSeriesProp(m_xSeries, uno::UNO_QUERY);
                if( xSeriesProp->getPropertyValue( "ShowCustomLeaderLines" ) >>= bValue )
                    rOutItemSet.Put(SfxBoolItem(nWhichId, bValue));
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("chart2", "");
            }
        }
        break;

        case SCHATTR_STYLE_SYMBOL:
        {
            chart2::Symbol aSymbol;
            if( GetPropertySet()->getPropertyValue( "Symbol" ) >>= aSymbol )
                rOutItemSet.Put( SfxInt32Item( nWhichId, lcl_getSymbolStyleForSymbol( aSymbol ) ));
        }
        break;

        case SCHATTR_SYMBOL_SIZE:
        {
            chart2::Symbol aSymbol;
            if( GetPropertySet()->getPropertyValue( "Symbol" ) >>= aSymbol )
                rOutItemSet.Put(
                    SvxSizeItem( nWhichId, Size( aSymbol.Size.Width, aSymbol.Size.Height ) ));
        }
        break;

        case SCHATTR_SYMBOL_BRUSH:
        {
            chart2::Symbol aSymbol;
            if(( GetPropertySet()->getPropertyValue( "Symbol" ) >>= aSymbol )
               && aSymbol.Graphic.is() )
            {
                rOutItemSet.Put( SvxBrushItem( Graphic( aSymbol.Graphic ), GPOS_MM, SCHATTR_SYMBOL_BRUSH ));
            }
        }
        break;

        case SCHATTR_TEXT_DEGREES:
        {
            double fValue = 0;

            if( GetPropertySet()->getPropertyValue( "TextRotation" ) >>= fValue )
            {
                rOutItemSet.Put( SdrAngleItem( nWhichId, Degree100(static_cast< sal_Int32 >(
                                                   ::rtl::math::round( fValue * 100.0 ) ) )));
            }
        }
        break;

        case SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY:
        {
            rOutItemSet.Put(SfxBoolItem(nWhichId, m_bHideLegendEntry));
            break;
        }
        break;
   }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
