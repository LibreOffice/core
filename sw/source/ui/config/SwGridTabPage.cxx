/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cmdid.h>
#include <SwGridTabPage.hxx>

#include <officecfg/Office/Writer.hxx>
#include <svl/eitem.hxx>

SwGridTabPage::SwGridTabPage(weld::Container* pPage, weld::DialogController* pController,
                             const SfxItemSet& rItemSet)
    : SvxGridTabPage(pPage, pController, rItemSet)
{
    m_xBaselineGrid->show();
}

bool SwGridTabPage::FillItemSet(SfxItemSet* pItemSet)
{
    bool bModified = SvxGridTabPage::FillItemSet(pItemSet);

    if (m_xCbxBaselineGridVisible->get_state_changed_from_saved())
    {
        pItemSet->Put(
            SfxBoolItem(FN_PARAM_BASELINE_GRID_VISIBLE, m_xCbxBaselineGridVisible->get_active()));
        bModified = true;
    }

    return bModified;
}

void SwGridTabPage::Reset(const SfxItemSet* pItemSet)
{
    SvxGridTabPage::Reset(pItemSet);

    if (const SfxBoolItem* pItem = pItemSet->GetItemIfSet(FN_PARAM_BASELINE_GRID_VISIBLE, false))
    {
        m_xCbxBaselineGridVisible->set_active(pItem->GetValue());
    }

    const bool bReadOnly = officecfg::Office::Writer::BaselineGrid::VisibleGrid::isReadOnly();
    m_xCbxBaselineGridVisible->set_sensitive(!bReadOnly);
    m_xCbxBaselineGridVisibleImg->set_visible(bReadOnly);
    m_xCbxBaselineGridVisible->save_state();
}

std::unique_ptr<SfxTabPage> SwGridTabPage::Create(weld::Container* pPage,
                                                  weld::DialogController* pController,
                                                  const SfxItemSet* pItemSet)
{
    return std::make_unique<SwGridTabPage>(pPage, pController, *pItemSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
