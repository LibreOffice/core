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

#include <TextLabelItemConverter.hxx>
#include <CharacterPropertyItemConverter.hxx>
#include <ChartModel.hxx>
#include <ChartType.hxx>
#include <ChartTypeHelper.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <DataSeriesProperties.hxx>
#include <Diagram.hxx>
#include <ItemPropertyMap.hxx>
#include "SchWhichPairs.hxx"
#include <unonames.hxx>

#include <editeng/brushitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svl/eitem.hxx>
#include <svl/ilstitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svx/tabline.hxx>
#include <svx/sdangitm.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/graph.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <memory>

using namespace com::sun::star;
using namespace com::sun::star::chart2;
using com::sun::star::uno::Reference;
using namespace ::chart::DataSeriesProperties;

namespace chart::wrapper {

namespace {

const ItemPropertyMapType& getTextLabelPropertyMap()
{
    static ItemPropertyMapType aMap{
        {XATTR_LINESTYLE, {CHART_UNONAME_LABEL_BORDER_STYLE, 0}},
        {XATTR_LINEWIDTH, {CHART_UNONAME_LABEL_BORDER_WIDTH, 0}},
        {XATTR_LINEDASH,  {CHART_UNONAME_LABEL_BORDER_DASH, 0}},
        {XATTR_LINECOLOR, {CHART_UNONAME_LABEL_BORDER_COLOR, 0}},
        {XATTR_LINETRANSPARENCE, {CHART_UNONAME_LABEL_BORDER_TRANS, 0}}};
    return aMap;
};

sal_Int32 getSymbolStyleForSymbol( const chart2::Symbol& rSymbol )
{
    sal_Int32 nStyle = SVX_SYMBOLTYPE_UNKNOWN;
    switch (rSymbol.Style)
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
        default:
            ;
    }
    return nStyle;
}

bool numberFormatFromItemToPropertySet(
    sal_uInt16 nWhichId, const SfxItemSet& rItemSet, const uno::Reference<beans::XPropertySet>& xPropertySet,
    bool bOverwriteDataPoints )
{
    bool bChanged = false;
    if (!xPropertySet.is())
        return bChanged;

    OUString aPropertyName = (nWhichId == SID_ATTR_NUMBERFORMAT_VALUE) ? CHART_UNONAME_NUMFMT : u"PercentageNumberFormat"_ustr;
    sal_uInt16 nSourceWhich = (nWhichId == SID_ATTR_NUMBERFORMAT_VALUE) ? SID_ATTR_NUMBERFORMAT_SOURCE : SCHATTR_PERCENT_NUMBERFORMAT_SOURCE;

    if (rItemSet.GetItemState(nSourceWhich) != SfxItemState::SET)
        return bChanged;

    uno::Any aValue;
    bool bUseSourceFormat = static_cast<const SfxBoolItem&>(rItemSet.Get(nSourceWhich)).GetValue();
    if (!bUseSourceFormat)
    {
        SfxItemState aState = rItemSet.GetItemState(nWhichId);
        if (aState == SfxItemState::SET)
        {
            sal_Int32 nFmt = static_cast<sal_Int32>(
                static_cast<const SfxUInt32Item&>(
                    rItemSet.Get(nWhichId)).GetValue());
            aValue <<= nFmt;
        }
        else
            return bChanged;
    }

    uno::Any aOldValue = xPropertySet->getPropertyValue(aPropertyName);
    if (bOverwriteDataPoints)
    {
        rtl::Reference<DataSeries> xSeries( dynamic_cast<DataSeries*>(xPropertySet.get()) );
        if (aValue != aOldValue ||
            ::chart::DataSeriesHelper::hasAttributedDataPointDifferentValue(xSeries, aPropertyName, aOldValue))
        {
            ::chart::DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(xSeries, aPropertyName, aValue);
            bChanged = true;
        }
    }
    else if (aOldValue != aValue)
    {
        xPropertySet->setPropertyValue(aPropertyName, aValue);
        bChanged = true;
    }
    return bChanged;
}

bool useSourceFormatFromItemToPropertySet(
    sal_uInt16 nWhichId, const SfxItemSet& rItemSet, const uno::Reference<beans::XPropertySet>& xPropertySet,
    bool bOverwriteDataPoints )
{
    bool bChanged = false;
    if (!xPropertySet.is())
        return bChanged;
    OUString aPropertyName = (nWhichId == SID_ATTR_NUMBERFORMAT_SOURCE) ? CHART_UNONAME_NUMFMT : u"PercentageNumberFormat"_ustr;
    sal_uInt16 nFormatWhich = (nWhichId == SID_ATTR_NUMBERFORMAT_SOURCE) ? SID_ATTR_NUMBERFORMAT_VALUE : SCHATTR_PERCENT_NUMBERFORMAT_VALUE;

    if (rItemSet.GetItemState(nWhichId) != SfxItemState::SET)
        return bChanged;

    uno::Any aNewValue;
    bool bUseSourceFormat = static_cast<const SfxBoolItem&>(
            rItemSet.Get(nWhichId)).GetValue();
    xPropertySet->setPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT, uno::Any(bUseSourceFormat));
    if (!bUseSourceFormat)
    {
        SfxItemState aState = rItemSet.GetItemState(nFormatWhich);
        if (aState == SfxItemState::SET)
        {
            sal_Int32 nFormatKey = static_cast<sal_Int32>(
                static_cast<const SfxUInt32Item&>(
                    rItemSet.Get(nFormatWhich)).GetValue());
            aNewValue <<= nFormatKey;
        }
        else
            return bChanged;
    }

    uno::Any aOldValue(xPropertySet->getPropertyValue(aPropertyName));
    if (bOverwriteDataPoints)
    {
        rtl::Reference<DataSeries> xSeries(dynamic_cast<DataSeries*>(xPropertySet.get()));
        if (aNewValue != aOldValue ||
            ::chart::DataSeriesHelper::hasAttributedDataPointDifferentValue(xSeries, aPropertyName, aOldValue))
        {
            ::chart::DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(xSeries, aPropertyName, aNewValue);
            bChanged = true;
        }
    }
    else if (aOldValue != aNewValue)
    {
        xPropertySet->setPropertyValue(aPropertyName, aNewValue);
        bChanged = true;
    }

    return bChanged;
}

} // anonymous namespace

TextLabelItemConverter::TextLabelItemConverter(
    const rtl::Reference<::chart::ChartModel>& xChartModel,
    const uno::Reference<beans::XPropertySet>& rPropertySet,
    const rtl::Reference<DataSeries>& xSeries,
    SfxItemPool& rItemPool, const std::optional<awt::Size>& pRefSize,
    bool bDataSeries, sal_Int32 nNumberFormat, sal_Int32 nPercentNumberFormat ) :
    ItemConverter(rPropertySet, rItemPool),
    mnNumberFormat(nNumberFormat),
    mnPercentNumberFormat(nPercentNumberFormat),
    mbDataSeries(bDataSeries),
    mbForbidPercentValue(true),
    m_xSeries(xSeries)
{
    maConverters.emplace_back(new CharacterPropertyItemConverter(rPropertySet, rItemPool, pRefSize, u"ReferencePageSize"_ustr));

    rtl::Reference< Diagram > xDiagram(xChartModel->getFirstChartDiagram());
    rtl::Reference< ChartType > xChartType(xDiagram->getChartTypeOfSeries(xSeries));
    bool bFound = false;
    bool bAmbiguous = false;
    bool bSwapXAndY = xDiagram->getVertical(bFound, bAmbiguous);
    maAvailableLabelPlacements = ChartTypeHelper::getSupportedLabelPlacements(xChartType, bSwapXAndY, xSeries);

    mbForbidPercentValue = ChartTypeHelper::getAxisType(xChartType, 0) != AxisType::CATEGORY;
}

TextLabelItemConverter::~TextLabelItemConverter()
{
}

void TextLabelItemConverter::FillItemSet( SfxItemSet& rOutItemSet ) const
{
    for( const auto& pConv : maConverters )
        pConv->FillItemSet( rOutItemSet );

    // own items
    ItemConverter::FillItemSet(rOutItemSet);
}

bool TextLabelItemConverter::ApplyItemSet( const SfxItemSet& rItemSet )
{
    bool bResult = false;

    for( const auto& pConv: maConverters )
        bResult = pConv->ApplyItemSet( rItemSet ) || bResult;

    // own items
    return ItemConverter::ApplyItemSet(rItemSet) || bResult;
}

const WhichRangesContainer& TextLabelItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nTextLabelWhichPairs;
}

bool TextLabelItemConverter::GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId& rOutProperty ) const
{
    const ItemPropertyMapType& rMap = getTextLabelPropertyMap();
    ItemPropertyMapType::const_iterator it = rMap.find(nWhichId);

    if (it == rMap.end())
        return false;

    rOutProperty = it->second;
    return true;
}

bool TextLabelItemConverter::ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet& rItemSet )
{
    bool bChanged = false;

    switch (nWhichId)
    {
        case SCHATTR_DATADESCR_SHOW_NUMBER:
        case SCHATTR_DATADESCR_SHOW_PERCENTAGE:
        case SCHATTR_DATADESCR_SHOW_CATEGORY:
        case SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME:
        case SCHATTR_DATADESCR_SHOW_SYMBOL:
        {
            const SfxBoolItem& rItem = static_cast<const SfxBoolItem&>(rItemSet.Get(nWhichId));

            uno::Any aOldValue = GetPropertySet()->getPropertyValue(CHART_UNONAME_LABEL);
            chart2::DataPointLabel aLabel;
            if (aOldValue >>= aLabel)
            {
                sal_Bool& rValue = (nWhichId == SCHATTR_DATADESCR_SHOW_NUMBER) ? aLabel.ShowNumber : (
                    (nWhichId == SCHATTR_DATADESCR_SHOW_PERCENTAGE) ? aLabel.ShowNumberInPercent : (
                    (nWhichId == SCHATTR_DATADESCR_SHOW_CATEGORY) ? aLabel.ShowCategoryName :
                    (nWhichId == SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME) ? aLabel.ShowSeriesName : aLabel.ShowLegendSymbol));
                bool bOldValue = rValue;
                rValue = rItem.GetValue();
                if (mbDataSeries)
                {
                    rtl::Reference<DataSeries> xSeries(dynamic_cast<DataSeries*>(GetPropertySet().get()));
                    if (bOldValue != bool(rValue) ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue(xSeries, CHART_UNONAME_LABEL, aOldValue))
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(xSeries, CHART_UNONAME_LABEL, uno::Any(aLabel));
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(xSeries, CHART_UNONAME_CUSTOM_LABEL_FIELDS, uno::Any());
                        bChanged = true;
                    }
                }
                else if (bOldValue != bool(rValue))
                {
                    GetPropertySet()->setPropertyValue(CHART_UNONAME_LABEL, uno::Any(aLabel));
                    bChanged = true;
                }
            }
        }
        break;
        case SID_ATTR_NUMBERFORMAT_VALUE:
        case SCHATTR_PERCENT_NUMBERFORMAT_VALUE:  //fall through intended
        {
            bChanged = numberFormatFromItemToPropertySet(nWhichId, rItemSet, GetPropertySet(), mbDataSeries);
        }
        break;
        case SID_ATTR_NUMBERFORMAT_SOURCE:
        case SCHATTR_PERCENT_NUMBERFORMAT_SOURCE: //fall through intended
        {
            bChanged = useSourceFormatFromItemToPropertySet(nWhichId, rItemSet, GetPropertySet(), mbDataSeries);
        }
        break;
        case SCHATTR_DATADESCR_SEPARATOR:
        {
            OUString aNewValue = static_cast<const SfxStringItem&>(rItemSet.Get(nWhichId)).GetValue();
            try
            {
                OUString aOldValue;
                GetPropertySet()->getPropertyValue(u"LabelSeparator"_ustr) >>= aOldValue;
                if (mbDataSeries)
                {
                    rtl::Reference<DataSeries> xSeries(dynamic_cast<DataSeries*>(GetPropertySet().get()));
                    if (aOldValue != aNewValue ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue(xSeries, u"LabelSeparator"_ustr, uno::Any(aOldValue)))
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(xSeries, u"LabelSeparator"_ustr, uno::Any(aNewValue));
                        bChanged = true;
                    }
                }
                else if (aOldValue != aNewValue)
                {
                    GetPropertySet()->setPropertyValue(u"LabelSeparator"_ustr, uno::Any(aNewValue));
                    bChanged = true;
                }
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("chart2", "");
            }
        }
        break;
        case SCHATTR_DATADESCR_WRAP_TEXT:
        {

            try
            {
                bool bNew = static_cast< const SfxBoolItem & >( rItemSet.Get( nWhichId )).GetValue();
                bool bOld = false;
                GetPropertySet()->getPropertyValue( u"TextWordWrap"_ustr ) >>= bOld;
                if( mbDataSeries )
                {
                    rtl::Reference< DataSeries > xSeries( dynamic_cast<DataSeries*>(GetPropertySet().get()) );
                    if( bOld!=bNew ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries, u"TextWordWrap"_ustr, uno::Any( bOld ) ) )
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, u"TextWordWrap"_ustr, uno::Any( bNew ) );
                        bChanged = true;
                    }
                }
                else if( bOld!=bNew )
                {
                    GetPropertySet()->setPropertyValue( u"TextWordWrap"_ustr, uno::Any( bNew ));
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
                sal_Int32 nNew = static_cast<const SfxInt32Item&>(rItemSet.Get(nWhichId)).GetValue();
                sal_Int32 nOld = -1;
                RelativePosition aCustomLabelPosition;
                GetPropertySet()->getPropertyValue(u"LabelPlacement"_ustr) >>= nOld;
                if (mbDataSeries)
                {
                    rtl::Reference<DataSeries> xSeries(dynamic_cast<DataSeries*>(GetPropertySet().get()));
                    if (nOld != nNew ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue(xSeries, u"LabelPlacement"_ustr, uno::Any(nOld)))
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(xSeries, u"LabelPlacement"_ustr, uno::Any(nNew));
                        bChanged = true;
                    }
                }
                else if (nOld != nNew || (GetPropertySet()->getPropertyValue(u"CustomLabelPosition"_ustr) >>= aCustomLabelPosition))
                {
                    GetPropertySet()->setPropertyValue(u"LabelPlacement"_ustr, uno::Any(nNew));
                    GetPropertySet()->setPropertyValue(u"CustomLabelPosition"_ustr, uno::Any());
                    GetPropertySet()->setPropertyValue(u"CustomLabelSize"_ustr, uno::Any());
                    bChanged = true;
                }
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
        break;
        case SCHATTR_STYLE_SYMBOL:
        {
            sal_Int32 nStyle =
                static_cast<const SfxInt32Item&>(
                rItemSet.Get(nWhichId)).GetValue();
            chart2::Symbol aSymbol;

            GetPropertySet()->getPropertyValue(u"Symbol"_ustr) >>= aSymbol;
            sal_Int32 nOldStyle = getSymbolStyleForSymbol(aSymbol);

            if (nStyle != nOldStyle)
            {
                bool bDeleteSymbol = false;
                switch (nStyle)
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

                if (bDeleteSymbol)
                    GetPropertySet()->setPropertyValue(u"Symbol"_ustr, uno::Any());
                else
                    GetPropertySet()->setPropertyValue(u"Symbol"_ustr, uno::Any(aSymbol));
                bChanged = true;
            }
        }
        break;
        case SCHATTR_SYMBOL_SIZE:
        {
            Size aSize = static_cast<const SvxSizeItem&>(
                rItemSet.Get(nWhichId)).GetSize();
            chart2::Symbol aSymbol;

            GetPropertySet()->getPropertyValue(u"Symbol"_ustr) >>= aSymbol;
            if (aSize.getWidth() != aSymbol.Size.Width ||
                aSize.getHeight() != aSymbol.Size.Height)
            {
                aSymbol.Size.Width = aSize.getWidth();
                aSymbol.Size.Height = aSize.getHeight();

                GetPropertySet()->setPropertyValue(u"Symbol"_ustr, uno::Any(aSymbol));
                bChanged = true;
            }
        }
        break;
        case SCHATTR_SYMBOL_BRUSH:
        {
            const SvxBrushItem& rBrshItem(static_cast<const SvxBrushItem&>(
                    rItemSet.Get(nWhichId)));
            uno::Any aXGraphicAny;
            const Graphic* pGraphic(rBrshItem.GetGraphic());
            if (pGraphic)
            {
                uno::Reference<graphic::XGraphic> xGraphic(pGraphic->GetXGraphic());
                if (xGraphic.is())
                {
                    aXGraphicAny <<= xGraphic;
                    chart2::Symbol aSymbol;
                    GetPropertySet()->getPropertyValue(u"Symbol"_ustr) >>= aSymbol;
                    if (aSymbol.Graphic != xGraphic)
                    {
                        aSymbol.Graphic = xGraphic;
                        GetPropertySet()->setPropertyValue(u"Symbol"_ustr, uno::Any(aSymbol));
                        bChanged = true;
                    }
                }
            }
        }
        break;
        case SCHATTR_TEXT_DEGREES:
        {
            double fValue = static_cast<double>(
                static_cast<const SdrAngleItem&>(
                    rItemSet.Get(nWhichId)).GetValue().get()) / 100.0;
            double fOldValue = 0.0;
            bool bPropExisted =
                (GetPropertySet()->getPropertyValue(u"TextRotation"_ustr) >>= fOldValue);

            if (!bPropExisted || fOldValue != fValue)
            {
                GetPropertySet()->setPropertyValue(u"TextRotation"_ustr, uno::Any(fValue));
                bChanged = true;
            }
        }
        break;
        case SCHATTR_DATADESCR_CUSTOM_LEADER_LINES:
        {
            try
            {
                bool bNew = static_cast<const SfxBoolItem&>(rItemSet.Get(nWhichId)).GetValue();
                bool bOld = true;
                if( (m_xSeries->getFastPropertyValue(PROP_DATASERIES_SHOW_CUSTOM_LEADERLINES) >>= bOld) && bOld != bNew )
                {
                    m_xSeries->setFastPropertyValue(PROP_DATASERIES_SHOW_CUSTOM_LEADERLINES, uno::Any(bNew));
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

void TextLabelItemConverter::FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet& rOutItemSet ) const
{
    switch (nWhichId)
    {
        case SCHATTR_DATADESCR_SHOW_NUMBER:
        case SCHATTR_DATADESCR_SHOW_PERCENTAGE:
        case SCHATTR_DATADESCR_SHOW_CATEGORY:
        case SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME:
        case SCHATTR_DATADESCR_SHOW_SYMBOL:
        {
            chart2::DataPointLabel aLabel;
            if (GetPropertySet()->getPropertyValue(CHART_UNONAME_LABEL) >>= aLabel)
            {
                bool bValue = (nWhichId == SCHATTR_DATADESCR_SHOW_NUMBER) ? aLabel.ShowNumber : (
                    (nWhichId == SCHATTR_DATADESCR_SHOW_PERCENTAGE) ? aLabel.ShowNumberInPercent : (
                    (nWhichId == SCHATTR_DATADESCR_SHOW_CATEGORY) ? aLabel.ShowCategoryName : (
                    (nWhichId == SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME) ? aLabel.ShowSeriesName :  aLabel.ShowLegendSymbol)));

                rOutItemSet.Put(SfxBoolItem(nWhichId, bValue));

                if (mbDataSeries)
                {
                    if (DataSeriesHelper::hasAttributedDataPointDifferentValue(
                            dynamic_cast<DataSeries*>(GetPropertySet().get()), CHART_UNONAME_LABEL, uno::Any(aLabel)))
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
                nKey = mnNumberFormat;
            rOutItemSet.Put(SfxUInt32Item(nWhichId, nKey));
        }
        break;
        case SCHATTR_PERCENT_NUMBERFORMAT_VALUE:
        {
            sal_Int32 nKey = 0;
            if (!(GetPropertySet()->getPropertyValue(u"PercentageNumberFormat"_ustr) >>= nKey))
                nKey = mnPercentNumberFormat;
            rOutItemSet.Put(SfxUInt32Item(nWhichId, nKey));
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
            rOutItemSet.Put(SfxBoolItem(nWhichId, !bNumberFormatIsSet));
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
            bool bNumberFormatIsSet = GetPropertySet()->getPropertyValue(u"PercentageNumberFormat"_ustr).hasValue() && !bUseSourceFormat;
            rOutItemSet.Put(SfxBoolItem(nWhichId, !bNumberFormatIsSet));
        }
        break;
        case SCHATTR_DATADESCR_SEPARATOR:
        {
            try
            {
                OUString aValue;
                GetPropertySet()->getPropertyValue(u"LabelSeparator"_ustr) >>= aValue;
                rOutItemSet.Put(SfxStringItem(nWhichId, aValue));
            }
            catch (const uno::Exception&)
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
                GetPropertySet()->getPropertyValue( u"TextWordWrap"_ustr ) >>= bValue;
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
                sal_Int32 nPlacement = 0;
                RelativePosition aCustomLabelPosition;
                if (!mbDataSeries && (GetPropertySet()->getPropertyValue(u"CustomLabelPosition"_ustr) >>= aCustomLabelPosition))
                    rOutItemSet.Put(SfxInt32Item(nWhichId, css::chart::DataLabelPlacement::CUSTOM));
                else if (GetPropertySet()->getPropertyValue(u"LabelPlacement"_ustr) >>= nPlacement)
                    rOutItemSet.Put(SfxInt32Item(nWhichId, nPlacement));
                else if (maAvailableLabelPlacements.hasElements())
                    rOutItemSet.Put(SfxInt32Item(nWhichId, maAvailableLabelPlacements[0]));
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
        break;
        case SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS:
        {
            rOutItemSet.Put(SfxIntegerListItem(nWhichId, maAvailableLabelPlacements));
        }
        break;
        case SCHATTR_DATADESCR_NO_PERCENTVALUE:
        {
            rOutItemSet.Put(SfxBoolItem(nWhichId, mbForbidPercentValue));
        }
        break;
        case SCHATTR_DATADESCR_CUSTOM_LEADER_LINES:
        {
            try
            {
                bool bValue = true;
                if( m_xSeries->getFastPropertyValue( PROP_DATASERIES_SHOW_CUSTOM_LEADERLINES ) >>= bValue )
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
            if (GetPropertySet()->getPropertyValue(u"Symbol"_ustr) >>= aSymbol)
                rOutItemSet.Put(SfxInt32Item(nWhichId, getSymbolStyleForSymbol(aSymbol)));
        }
        break;
        case SCHATTR_SYMBOL_SIZE:
        {
            chart2::Symbol aSymbol;
            if (GetPropertySet()->getPropertyValue(u"Symbol"_ustr) >>= aSymbol)
                rOutItemSet.Put(
                    SvxSizeItem(nWhichId, Size(aSymbol.Size.Width, aSymbol.Size.Height)));
        }
        break;
        case SCHATTR_SYMBOL_BRUSH:
        {
            chart2::Symbol aSymbol;
            if ((GetPropertySet()->getPropertyValue(u"Symbol"_ustr) >>= aSymbol)
                && aSymbol.Graphic.is())
            {
                rOutItemSet.Put(
                    SvxBrushItem(Graphic(aSymbol.Graphic), GPOS_MM, SCHATTR_SYMBOL_BRUSH));
            }
        }
        break;
        case SCHATTR_TEXT_DEGREES:
        {
            double fValue = 0;

            if (GetPropertySet()->getPropertyValue(u"TextRotation"_ustr) >>= fValue)
            {
                rOutItemSet.Put(
                    SdrAngleItem(SCHATTR_TEXT_DEGREES, Degree100(static_cast<sal_Int32>(rtl::math::round(fValue * 100.0)))));
            }
        }
        break;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
