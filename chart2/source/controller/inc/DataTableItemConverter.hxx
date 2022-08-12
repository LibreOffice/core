/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ItemConverter.hxx"
#include <rtl/ref.hxx>
#include <vector>

namespace com::sun::star::awt
{
struct Size;
}
namespace com::sun::star::beans
{
class XPropertySet;
}
namespace chart
{
class ChartModel;
}

class SdrModel;

namespace chart::wrapper
{
class DataTableItemConverter final : public ItemConverter
{
public:
    DataTableItemConverter(const css::uno::Reference<css::beans::XPropertySet>& rPropertySet,
                           SfxItemPool& rItemPool, SdrModel& rDrawModel,
                           const rtl::Reference<::chart::ChartModel>& xChartDoc);

    virtual ~DataTableItemConverter() override;

    virtual void FillItemSet(SfxItemSet& rOutItemSet) const override;
    virtual bool ApplyItemSet(const SfxItemSet& rItemSet) override;

protected:
    virtual const WhichRangesContainer& GetWhichPairs() const override;
    virtual bool GetItemProperty(tWhichIdType nWhichId,
                                 tPropertyNameWithMemberId& rOutProperty) const override;

private:
    std::vector<std::unique_ptr<ItemConverter>> m_aConverters;
    rtl::Reference<::chart::ChartModel> m_xChartDoc;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
