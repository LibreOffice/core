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
#include <ChartModelHelper.hxx>
#include <ChartTypeHelper.hxx>
#include <DataSeriesHelper.hxx>
#include <DiagramHelper.hxx>
#include <ItemPropertyMap.hxx>
#include <SchWhichPairs.hxx>
#include <macros.hxx>
#include <unonames.hxx>

#include <editeng/brushitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svl/ilstitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svx/tabline.hxx>

#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <memory>

using namespace com::sun::star;
using namespace com::sun::star::chart2;
using com::sun::star::uno::Reference;

namespace chart { namespace wrapper {

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

    OUString aPropertyName = (SID_ATTR_NUMBERFORMAT_VALUE == nWhichId) ? OUString(CHART_UNONAME_NUMFMT) : OUString("PercentageNumberFormat");
    sal_uInt16 nSourceWhich = (SID_ATTR_NUMBERFORMAT_VALUE == nWhichId) ? SID_ATTR_NUMBERFORMAT_SOURCE : SCHATTR_PERCENT_NUMBERFORMAT_SOURCE;

    if (SfxItemState::SET != rItemSet.GetItemState(nSourceWhich))
        return bChanged;

    uno::Any aValue;
    bool bUseSourceFormat = (static_cast<const SfxBoolItem&>(rItemSet.Get(nSourceWhich)).GetValue());
    if (!bUseSourceFormat)
    {
        SfxItemState aState = rItemSet.GetItemState(nWhichId);
        if (aState == SfxItemState::SET)
        {
            sal_Int32 nFmt = static_cast<sal_Int32>(
                static_cast<const SfxUInt32Item&>(
                    rItemSet.Get(nWhichId)).GetValue());
            aValue = uno::Any(nFmt);
        }
        else
            return bChanged;
    }

    uno::Any aOldValue = xPropertySet->getPropertyValue(aPropertyName);
    if (bOverwriteDataPoints)
    {
        Reference<chart2::XDataSeries> xSeries(xPropertySet, uno::UNO_QUERY);
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
    OUString aPropertyName = (SID_ATTR_NUMBERFORMAT_SOURCE == nWhichId) ? OUString(CHART_UNONAME_NUMFMT) : OUString("PercentageNumberFormat");
    sal_uInt16 nFormatWhich = (SID_ATTR_NUMBERFORMAT_SOURCE == nWhichId) ? SID_ATTR_NUMBERFORMAT_VALUE : SCHATTR_PERCENT_NUMBERFORMAT_VALUE;

    if (SfxItemState::SET != rItemSet.GetItemState(nWhichId))
        return bChanged;

    uno::Any aNewValue;
    bool bUseSourceFormat = (static_cast<const SfxBoolItem&>(
            rItemSet.Get(nWhichId)).GetValue());
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
        Reference<chart2::XDataSeries> xSeries(xPropertySet, uno::UNO_QUERY);
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
    const uno::Reference<frame::XModel>& xChartModel,
    const uno::Reference<beans::XPropertySet>& rPropertySet,
    const uno::Reference<XDataSeries>& xSeries,
    SfxItemPool& rItemPool, const awt::Size* pRefSize,
    bool bDataSeries, sal_Int32 nNumberFormat, sal_Int32 nPercentNumberFormat ) :
    ItemConverter(rPropertySet, rItemPool),
    mnNumberFormat(nNumberFormat),
    mnPercentNumberFormat(nPercentNumberFormat),
    mbDataSeries(bDataSeries),
    mbForbidPercentValue(true)
{
    maConverters.push_back(new CharacterPropertyItemConverter(rPropertySet, rItemPool, pRefSize, "ReferencePageSize"));

    uno::Reference<XDiagram> xDiagram(ChartModelHelper::findDiagram(xChartModel));
    uno::Reference<XChartType> xChartType(DiagramHelper::getChartTypeOfSeries(xDiagram, xSeries));
    bool bFound = false;
    bool bAmbiguous = false;
    bool bSwapXAndY = DiagramHelper::getVertical(xDiagram, bFound, bAmbiguous);
    maAvailableLabelPlacements = ChartTypeHelper::getSupportedLabelPlacements(xChartType, DiagramHelper::getDimension(xDiagram), bSwapXAndY, xSeries);

    mbForbidPercentValue = AxisType::CATEGORY != ChartTypeHelper::getAxisType(xChartType, 0);
}

TextLabelItemConverter::~TextLabelItemConverter()
{
    std::for_each(maConverters.begin(), maConverters.end(), std::default_delete<ItemConverter>());
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

const sal_uInt16* TextLabelItemConverter::GetWhichPairs() const
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
        case SCHATTR_DATADESCR_SHOW_SYMBOL:
        {
            const SfxBoolItem& rItem = static_cast<const SfxBoolItem&>(rItemSet.Get(nWhichId));

            uno::Any aOldValue = GetPropertySet()->getPropertyValue(CHART_UNONAME_LABEL);
            chart2::DataPointLabel aLabel;
            if (aOldValue >>= aLabel)
            {
                sal_Bool& rValue = (SCHATTR_DATADESCR_SHOW_NUMBER == nWhichId) ? aLabel.ShowNumber : (
                    (SCHATTR_DATADESCR_SHOW_PERCENTAGE == nWhichId) ? aLabel.ShowNumberInPercent : (
                        (SCHATTR_DATADESCR_SHOW_CATEGORY == nWhichId) ? aLabel.ShowCategoryName : aLabel.ShowLegendSymbol));
                bool bOldValue = rValue;
                rValue = rItem.GetValue();
                if (mbDataSeries)
                {
                    Reference<chart2::XDataSeries> xSeries(GetPropertySet(), uno::UNO_QUERY);
                    if (bOldValue != bool(rValue) ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue(xSeries, CHART_UNONAME_LABEL, aOldValue))
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(xSeries, CHART_UNONAME_LABEL, uno::Any(aLabel));
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
            OUString aOldValue;
            try
            {
                GetPropertySet()->getPropertyValue("LabelSeparator") >>= aOldValue;
                if (mbDataSeries)
                {
                    Reference<chart2::XDataSeries> xSeries(GetPropertySet(), uno::UNO_QUERY);
                    if (!aOldValue.equals(aNewValue) ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue(xSeries, "LabelSeparator", uno::Any(aOldValue)))
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(xSeries, "LabelSeparator", uno::Any(aNewValue));
                        bChanged = true;
                    }
                }
                else if (!aOldValue.equals(aNewValue))
                {
                    GetPropertySet()->setPropertyValue("LabelSeparator", uno::Any(aNewValue));
                    bChanged = true;
                }
            }
            catch (const uno::Exception& e)
            {
                ASSERT_EXCEPTION(e);
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
                if( mbDataSeries )
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
            catch( const uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
        break;
        case SCHATTR_DATADESCR_PLACEMENT:
        {
            try
            {
                sal_Int32 nNew = static_cast<const SfxInt32Item&>(rItemSet.Get(nWhichId)).GetValue();
                sal_Int32 nOld = 0;
                if (!(GetPropertySet()->getPropertyValue("LabelPlacement") >>= nOld))
                {
                    if (maAvailableLabelPlacements.getLength())
                        nOld = maAvailableLabelPlacements[0];
                }
                if (mbDataSeries)
                {
                    Reference<chart2::XDataSeries> xSeries(GetPropertySet(), uno::UNO_QUERY);
                    if (nOld != nNew ||
                        DataSeriesHelper::hasAttributedDataPointDifferentValue(xSeries, "LabelPlacement", uno::Any(nOld)))
                    {
                        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(xSeries, "LabelPlacement", uno::Any(nNew));
                        bChanged = true;
                    }
                }
                else if (nOld != nNew)
                {
                    GetPropertySet()->setPropertyValue("LabelPlacement", uno::Any(nNew));
                    bChanged = true;
                }
            }
            catch (const uno::Exception& e)
            {
                ASSERT_EXCEPTION(e);
            }
        }
        break;
        case SCHATTR_STYLE_SYMBOL:
        {
            sal_Int32 nStyle =
                static_cast<const SfxInt32Item&>(
                rItemSet.Get(nWhichId)).GetValue();
            chart2::Symbol aSymbol;

            GetPropertySet()->getPropertyValue("Symbol") >>= aSymbol;
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
                    GetPropertySet()->setPropertyValue("Symbol", uno::Any());
                else
                    GetPropertySet()->setPropertyValue("Symbol", uno::Any(aSymbol));
                bChanged = true;
            }
        }
        break;
        case SCHATTR_SYMBOL_SIZE:
        {
            Size aSize = static_cast<const SvxSizeItem&>(
                rItemSet.Get(nWhichId)).GetSize();
            chart2::Symbol aSymbol;

            GetPropertySet()->getPropertyValue("Symbol") >>= aSymbol;
            if (aSize.getWidth() != aSymbol.Size.Width ||
                aSize.getHeight() != aSymbol.Size.Height)
            {
                aSymbol.Size.Width = aSize.getWidth();
                aSymbol.Size.Height = aSize.getHeight();

                GetPropertySet()->setPropertyValue("Symbol", uno::Any(aSymbol));
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
                    GetPropertySet()->getPropertyValue("Symbol") >>= aSymbol;
                    if (aSymbol.Graphic != xGraphic)
                    {
                        aSymbol.Graphic = xGraphic;
                        GetPropertySet()->setPropertyValue("Symbol", uno::Any(aSymbol));
                        bChanged = true;
                    }
                }
            }
        }
        break;
        case SCHATTR_TEXT_DEGREES:
        {
            double fValue = static_cast<double>(
                static_cast<const SfxInt32Item&>(
                    rItemSet.Get(nWhichId)).GetValue()) / 100.0;
            double fOldValue = 0.0;
            bool bPropExisted =
                (GetPropertySet()->getPropertyValue("TextRotation") >>= fOldValue);

            if (!bPropExisted || fOldValue != fValue)
            {
                GetPropertySet()->setPropertyValue("TextRotation", uno::Any(fValue));
                bChanged = true;
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
        case SCHATTR_DATADESCR_SHOW_SYMBOL:
        {
            chart2::DataPointLabel aLabel;
            if (GetPropertySet()->getPropertyValue(CHART_UNONAME_LABEL) >>= aLabel)
            {
                bool bValue = (SCHATTR_DATADESCR_SHOW_NUMBER == nWhichId) ? aLabel.ShowNumber : (
                    (SCHATTR_DATADESCR_SHOW_PERCENTAGE == nWhichId) ? aLabel.ShowNumberInPercent : (
                        (SCHATTR_DATADESCR_SHOW_CATEGORY == nWhichId) ? aLabel.ShowCategoryName : aLabel.ShowLegendSymbol));

                rOutItemSet.Put(SfxBoolItem(nWhichId, bValue));

                if (mbDataSeries)
                {
                    if (DataSeriesHelper::hasAttributedDataPointDifferentValue(
                            Reference<chart2::XDataSeries>(GetPropertySet(), uno::UNO_QUERY), CHART_UNONAME_LABEL, uno::Any(aLabel)))
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
            if (!(GetPropertySet()->getPropertyValue("PercentageNumberFormat") >>= nKey))
                nKey = mnPercentNumberFormat;
            rOutItemSet.Put(SfxUInt32Item(nWhichId, nKey));
        }
        break;
        case SID_ATTR_NUMBERFORMAT_SOURCE:
        {
            bool bNumberFormatIsSet = GetPropertySet()->getPropertyValue(CHART_UNONAME_NUMFMT).hasValue();
            rOutItemSet.Put(SfxBoolItem(nWhichId, !bNumberFormatIsSet));
        }
        break;
        case SCHATTR_PERCENT_NUMBERFORMAT_SOURCE:
        {
            bool bNumberFormatIsSet = (GetPropertySet()->getPropertyValue("PercentageNumberFormat").hasValue());
            rOutItemSet.Put(SfxBoolItem(nWhichId, !bNumberFormatIsSet));
        }
        break;
        case SCHATTR_DATADESCR_SEPARATOR:
        {
            OUString aValue;
            try
            {
                GetPropertySet()->getPropertyValue("LabelSeparator") >>= aValue;
                rOutItemSet.Put(SfxStringItem(nWhichId, aValue));
            }
            catch (const uno::Exception& e)
            {
                ASSERT_EXCEPTION(e);
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
            catch( const uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
        break;
        case SCHATTR_DATADESCR_PLACEMENT:
        {
            try
            {
                sal_Int32 nPlacement = 0;
                if (GetPropertySet()->getPropertyValue("LabelPlacement") >>= nPlacement)
                    rOutItemSet.Put(SfxInt32Item(nWhichId, nPlacement));
                else if (maAvailableLabelPlacements.getLength())
                    rOutItemSet.Put(SfxInt32Item(nWhichId, maAvailableLabelPlacements[0]));
            }
            catch (const uno::Exception& e)
            {
                ASSERT_EXCEPTION(e);
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
        case SCHATTR_STYLE_SYMBOL:
        {
            chart2::Symbol aSymbol;
            if (GetPropertySet()->getPropertyValue("Symbol") >>= aSymbol)
                rOutItemSet.Put(SfxInt32Item(nWhichId, getSymbolStyleForSymbol(aSymbol)));
        }
        break;
        case SCHATTR_SYMBOL_SIZE:
        {
            chart2::Symbol aSymbol;
            if (GetPropertySet()->getPropertyValue("Symbol") >>= aSymbol)
                rOutItemSet.Put(
                    SvxSizeItem(nWhichId, Size(aSymbol.Size.Width, aSymbol.Size.Height)));
        }
        break;
        case SCHATTR_SYMBOL_BRUSH:
        {
            chart2::Symbol aSymbol;
            if ((GetPropertySet()->getPropertyValue("Symbol") >>= aSymbol)
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

            if (GetPropertySet()->getPropertyValue("TextRotation") >>= fValue)
            {
                rOutItemSet.Put(
                    SfxInt32Item(nWhichId, static_cast<sal_Int32>(rtl::math::round(fValue * 100.0))));
            }
        }
        break;
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
