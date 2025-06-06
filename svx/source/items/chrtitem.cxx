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

#include <rtl/math.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <o3tl/hash_combine.hxx>

#include <svx/ChartColorPaletteType.hxx>
#include <svx/chrtitem.hxx>
#include <svx/unomid.hxx>

using namespace ::com::sun::star;


SfxPoolItem* SvxDoubleItem::CreateDefault() { return new  SvxDoubleItem(0.0, TypedWhichId<SvxDoubleItem>(0));}

SvxChartTextOrderItem::SvxChartTextOrderItem(SvxChartTextOrder eOrder,
                                             TypedWhichId<SvxChartTextOrderItem> nId) :
    SfxEnumItem(nId, eOrder)
{
}

SvxChartTextOrderItem* SvxChartTextOrderItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartTextOrderItem(*this);
}

bool SvxChartTextOrderItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    // the order of the two enums is not equal, so a mapping is required
    css::chart::ChartAxisArrangeOrderType eAO;
    SvxChartTextOrder eOrder( GetValue());

    switch( eOrder )
    {
        case SvxChartTextOrder::SideBySide:
            eAO = css::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE; break;
        case SvxChartTextOrder::UpDown:
            eAO = css::chart::ChartAxisArrangeOrderType_STAGGER_ODD; break;
        case SvxChartTextOrder::DownUp:
            eAO = css::chart::ChartAxisArrangeOrderType_STAGGER_EVEN; break;
        case SvxChartTextOrder::Auto:
            eAO = css::chart::ChartAxisArrangeOrderType_AUTO; break;
    }

    rVal <<= eAO;

    return true;
}


bool SvxChartTextOrderItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // the order of the two enums is not equal, so a mapping is required
    css::chart::ChartAxisArrangeOrderType eAO;
    SvxChartTextOrder eOrder;

    if(!(rVal >>= eAO))
    {
        // also try an int (for Basic)
        sal_Int32 nAO = 0;
        if(!(rVal >>= nAO))
            return false;
        eAO = static_cast< css::chart::ChartAxisArrangeOrderType >( nAO );
    }

    switch( eAO )
    {
        case css::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE:
            eOrder = SvxChartTextOrder::SideBySide; break;
        case css::chart::ChartAxisArrangeOrderType_STAGGER_ODD:
            eOrder = SvxChartTextOrder::UpDown; break;
        case css::chart::ChartAxisArrangeOrderType_STAGGER_EVEN:
            eOrder = SvxChartTextOrder::DownUp; break;
        case css::chart::ChartAxisArrangeOrderType_AUTO:
            eOrder = SvxChartTextOrder::Auto; break;
        default:
            return false;
    }

    SetValue( eOrder );

    return true;
}

SvxDoubleItem::SvxDoubleItem(double fValue, TypedWhichId<SvxDoubleItem> nId) :
    SfxPoolItem(nId),
    m_fVal(fValue)
{
}

SvxDoubleItem::SvxDoubleItem(const SvxDoubleItem& rItem) :
    SfxPoolItem(rItem),
    m_fVal(rItem.m_fVal)
{
}

bool SvxDoubleItem::GetPresentation
            ( SfxItemPresentation /*ePresentation*/, MapUnit /*eCoreMetric*/,
              MapUnit /*ePresentationMetric*/, OUString& rText,
              const IntlWrapper& rIntlWrapper) const
{
    rText = ::rtl::math::doubleToUString( m_fVal, rtl_math_StringFormat_E, 4,
        rIntlWrapper.getLocaleData()->getNumDecimalSep()[0], true );
    return true;
}

bool SvxDoubleItem::operator == (const SfxPoolItem& rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    return static_cast<const SvxDoubleItem&>(rItem).m_fVal == m_fVal;
}

SvxDoubleItem* SvxDoubleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxDoubleItem(*this);
}

bool SvxDoubleItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= m_fVal;
    return true;
}

bool SvxDoubleItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    return rVal >>= m_fVal;
}

SvxChartKindErrorItem::SvxChartKindErrorItem(SvxChartKindError eOrient,
                                               TypedWhichId<SvxChartKindErrorItem> nId) :
    SfxEnumItem(nId, eOrient)
{
}

SvxChartKindErrorItem* SvxChartKindErrorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartKindErrorItem(*this);
}

SvxChartIndicateItem::SvxChartIndicateItem(SvxChartIndicate eOrient,
                                               TypedWhichId<SvxChartIndicateItem> nId) :
    SfxEnumItem(nId, eOrient)
{
}

SvxChartIndicateItem* SvxChartIndicateItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartIndicateItem(*this);
}

SvxChartRegressItem::SvxChartRegressItem(SvxChartRegress eOrient,
                                               TypedWhichId<SvxChartRegressItem> nId) :
    SfxEnumItem(nId, eOrient)
{
}

SvxChartRegressItem* SvxChartRegressItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartRegressItem(*this);
}

// SvxChartColorPaletteItem implementation

SvxChartColorPaletteItem::SvxChartColorPaletteItem(const ChartColorPaletteType eType,
                                                   const sal_uInt32 nIndex,
                                                   const TypedWhichId<SvxChartColorPaletteItem> nId)
    : SfxPoolItem(nId)
    , meType(eType)
    , mnIndex(nIndex)
{
}

SvxChartColorPaletteItem::SvxChartColorPaletteItem(const SvxChartColorPaletteItem& rItem)
    : SfxPoolItem(rItem)
    , meType(rItem.meType)
    , mnIndex(rItem.mnIndex)
{
}

bool SvxChartColorPaletteItem::QueryValue(uno::Any& rVal, const sal_uInt8 nMemberId) const
{
    if (nMemberId == MID_CHART_COLOR_PALETTE_TYPE)
    {
        rVal <<= static_cast<sal_Int32>(meType);
        return true;
    }
    if (nMemberId == MID_CHART_COLOR_PALETTE_INDEX)
    {
        rVal <<= mnIndex;
        return true;
    }
    return false;
}

bool SvxChartColorPaletteItem::PutValue(const uno::Any& rVal, const sal_uInt8 nMemberId)
{
    if (nMemberId == MID_CHART_COLOR_PALETTE_TYPE)
    {
        sal_Int32 nType = -1;
        rVal >>= nType;
        meType = static_cast<ChartColorPaletteType>(nType);
        return true;
    }
    if (nMemberId == MID_CHART_COLOR_PALETTE_INDEX)
    {
        rVal >>= mnIndex;
        return true;
    }
    return false;
}

bool SvxChartColorPaletteItem::GetPresentation(SfxItemPresentation /*ePres*/,
                                               MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
                                               OUString& rText, const IntlWrapper&) const
{
    if (meType == ChartColorPaletteType::Colorful)
        rText = u"Colorful"_ustr;
    else if (meType == ChartColorPaletteType::Monochromatic)
        rText = u"Monochromatic"_ustr;
    else
        rText = u"Unknown"_ustr;

    rText += u" "_ustr;
    rText += OUString::number(mnIndex);

    return true;
}

bool SvxChartColorPaletteItem::operator==(const SfxPoolItem& rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    const auto& rOther = static_cast<const SvxChartColorPaletteItem&>(rItem);
    return (meType == rOther.meType && mnIndex == rOther.mnIndex);
}

SvxChartColorPaletteItem* SvxChartColorPaletteItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartColorPaletteItem(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
