/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DataTableItemConverter.hxx>
#include <ItemPropertyMap.hxx>
#include <CharacterPropertyItemConverter.hxx>
#include <GraphicPropertyItemConverter.hxx>
#include <chartview/ChartSfxItemIds.hxx>
#include <chartview/ExplicitScaleValues.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include "SchWhichPairs.hxx"
#include <ChartModelHelper.hxx>
#include <ChartModel.hxx>
#include <CommonConverters.hxx>
#include <ChartType.hxx>
#include <ChartTypeHelper.hxx>
#include <Diagram.hxx>
#include <unonames.hxx>
#include <BaseCoordinateSystem.hxx>
#include <memory>

#include <osl/diagnose.h>
#include <o3tl/any.hxx>
#include <svl/eitem.hxx>
#include <svx/chrtitem.hxx>
#include <svx/sdangitm.hxx>
#include <svl/intitem.hxx>
#include <rtl/math.hxx>

using namespace css;

namespace chart::wrapper
{
namespace
{
ItemPropertyMapType& lclDataTablePropertyMap()
{
    static ItemPropertyMapType aPropertyMap{
        { SCHATTR_DATA_TABLE_HORIZONTAL_BORDER, { "HBorder", 0 } },
        { SCHATTR_DATA_TABLE_VERTICAL_BORDER, { "VBorder", 0 } },
        { SCHATTR_DATA_TABLE_OUTLINE, { "Outline", 0 } },
        { SCHATTR_DATA_TABLE_KEYS, { "Keys", 0 } },
    };
    return aPropertyMap;
};
}

DataTableItemConverter::DataTableItemConverter(
    const uno::Reference<beans::XPropertySet>& rPropertySet, SfxItemPool& rItemPool,
    SdrModel& rDrawModel, const rtl::Reference<::chart::ChartModel>& xChartDoc,
    const awt::Size* pRefSize)
    : ItemConverter(rPropertySet, rItemPool)
    , m_xChartDoc(xChartDoc)
{
    m_aConverters.emplace_back(new GraphicPropertyItemConverter(
        rPropertySet, rItemPool, rDrawModel, xChartDoc, GraphicObjectType::LineProperties));
    m_aConverters.emplace_back(new CharacterPropertyItemConverter(rPropertySet, rItemPool));
}

DataTableItemConverter::~DataTableItemConverter() = default;

void DataTableItemConverter::FillItemSet(SfxItemSet& rOutItemSet) const
{
    for (const auto& pConv : m_aConverters)
    {
        pConv->FillItemSet(rOutItemSet);
    }

    // own items
    ItemConverter::FillItemSet(rOutItemSet);
}

bool DataTableItemConverter::ApplyItemSet(const SfxItemSet& rItemSet)
{
    bool bResult = false;

    for (const auto& pConv : m_aConverters)
    {
        bResult = pConv->ApplyItemSet(rItemSet) || bResult;
    }

    // own items
    return ItemConverter::ApplyItemSet(rItemSet) || bResult;
}

const WhichRangesContainer& DataTableItemConverter::GetWhichPairs() const
{
    return nDataTableWhichPairs;
}

bool DataTableItemConverter::GetItemProperty(tWhichIdType nWhichId,
                                             tPropertyNameWithMemberId& rOutProperty) const
{
    ItemPropertyMapType& rMap(lclDataTablePropertyMap());
    auto aIt = rMap.find(nWhichId);
    if (aIt == rMap.cend())
        return false;

    rOutProperty = (*aIt).second;

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
