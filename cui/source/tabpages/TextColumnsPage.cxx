/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <svtools/unitconv.hxx>
#include <svx/dlgutil.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svddef.hxx>

#include <TextColumnsPage.hxx>

const WhichRangesContainer
    SvxTextColumnsPage::pRanges(svl::Items<SDRATTR_TEXTCOLUMNS_FIRST, SDRATTR_TEXTCOLUMNS_LAST>);

SvxTextColumnsPage::SvxTextColumnsPage(weld::Container* pPage, weld::DialogController* pController,
                                       const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"cui/ui/textcolumnstabpage.ui"_ustr, u"TextColumnsPage"_ustr,
                 &rInAttrs)
    , m_xColumnsNumber(m_xBuilder->weld_spin_button(u"FLD_COL_NUMBER"_ustr))
    , m_xColumnsSpacing(m_xBuilder->weld_metric_spin_button(u"MTR_FLD_COL_SPACING"_ustr,
                                                            GetModuleFieldUnit(rInAttrs)))
{
}

SvxTextColumnsPage::~SvxTextColumnsPage() = default;

// read the passed item set
void SvxTextColumnsPage::Reset(const SfxItemSet* rAttrs)
{
    SfxItemPool* pPool = rAttrs->GetPool();
    assert(pPool);

    {
        auto pItem = GetItem(*rAttrs, SDRATTR_TEXTCOLUMNS_NUMBER);
        if (!pItem)
            pItem = &pPool->GetUserOrPoolDefaultItem(SDRATTR_TEXTCOLUMNS_NUMBER);
        m_xColumnsNumber->set_value(pItem->GetValue());
        m_xColumnsNumber->save_value();
    }

    {
        MapUnit eUnit = pPool->GetMetric(SDRATTR_TEXTCOLUMNS_SPACING);
        auto pItem = GetItem(*rAttrs, SDRATTR_TEXTCOLUMNS_SPACING);
        if (!pItem)
            pItem = &pPool->GetUserOrPoolDefaultItem(SDRATTR_TEXTCOLUMNS_SPACING);
        SetMetricValue(*m_xColumnsSpacing, pItem->GetValue(), eUnit);
        m_xColumnsSpacing->save_value();
    }
}

// fill the passed item set with dialog box attributes
bool SvxTextColumnsPage::FillItemSet(SfxItemSet* rAttrs)
{
    if (m_xColumnsNumber->get_value_changed_from_saved())
        rAttrs->Put(SfxInt16Item(SDRATTR_TEXTCOLUMNS_NUMBER, m_xColumnsNumber->get_value()));

    if (m_xColumnsSpacing->get_value_changed_from_saved())
    {
        SfxItemPool* pPool = rAttrs->GetPool();
        assert(pPool);
        MapUnit eUnit = pPool->GetMetric(SDRATTR_TEXTCOLUMNS_SPACING);
        sal_Int32 nValue = GetCoreValue(*m_xColumnsSpacing, eUnit);
        rAttrs->Put(SdrMetricItem(SDRATTR_TEXTCOLUMNS_SPACING, nValue));
    }

    return true;
}

std::unique_ptr<SfxTabPage> SvxTextColumnsPage::Create(weld::Container* pPage,
                                                       weld::DialogController* pController,
                                                       const SfxItemSet* rAttrs)
{
    return std::make_unique<SvxTextColumnsPage>(pPage, pController, *rAttrs);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
