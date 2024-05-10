/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "TextColumnsPropertyPanel.hxx"

#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svl/itempool.hxx>
#include <svtools/unitconv.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svl/itemset.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace
{
MapUnit GetUnit(const SfxBindings* pBindings, sal_uInt16 nWhich)
{
    assert(pBindings);

    SfxObjectShell* pSh = nullptr;
    if (auto pShell = pBindings->GetDispatcher()->GetShell(0))
        pSh = pShell->GetObjectShell();
    if (!pSh)
        pSh = SfxObjectShell::Current();
    SfxItemPool& rPool = pSh ? pSh->GetPool() : SfxGetpApp()->GetPool();
    return rPool.GetMetric(nWhich);
}
}

namespace svx::sidebar
{
TextColumnsPropertyPanel::TextColumnsPropertyPanel(weld::Widget* pParent, SfxBindings* pBindings)
    : PanelLayout(pParent, u"TextColumnsPropertyPanel"_ustr,
                  u"svx/ui/sidebartextcolumnspanel.ui"_ustr)
    , mpBindings(pBindings)
    , m_xColumnsNumber(m_xBuilder->weld_spin_button(u"FLD_COL_NUMBER"_ustr))
    , m_xColumnsSpacing(
          m_xBuilder->weld_metric_spin_button(u"MTR_FLD_COL_SPACING"_ustr, FieldUnit::CM))
    , maColumnsNumberController(SID_ATTR_TEXTCOLUMNS_NUMBER, *pBindings, *this)
    , maColumnsSpacingController(SID_ATTR_TEXTCOLUMNS_SPACING, *pBindings, *this)
{
    m_xColumnsNumber->connect_value_changed(
        LINK(this, TextColumnsPropertyPanel, ModifyColumnsNumberHdl));
    m_xColumnsSpacing->connect_value_changed(
        LINK(this, TextColumnsPropertyPanel, ModifyColumnsSpacingHdl));
}

TextColumnsPropertyPanel::~TextColumnsPropertyPanel()
{
    maColumnsSpacingController.dispose();
    maColumnsNumberController.dispose();

    m_xColumnsSpacing.reset();
    m_xColumnsNumber.reset();
}

IMPL_LINK_NOARG(TextColumnsPropertyPanel, ModifyColumnsNumberHdl, weld::SpinButton&, void)
{
    SfxInt16Item aItem(SDRATTR_TEXTCOLUMNS_NUMBER, m_xColumnsNumber->get_value());
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_TEXTCOLUMNS_NUMBER, SfxCallMode::RECORD,
                                             { &aItem });
}

IMPL_LINK_NOARG(TextColumnsPropertyPanel, ModifyColumnsSpacingHdl, weld::MetricSpinButton&, void)
{
    const MapUnit aUnit = GetUnit(mpBindings, SDRATTR_TEXTCOLUMNS_SPACING);
    SdrMetricItem aItem(SDRATTR_TEXTCOLUMNS_SPACING, GetCoreValue(*m_xColumnsSpacing, aUnit));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_TEXTCOLUMNS_SPACING, SfxCallMode::RECORD,
                                             { &aItem });
}

void TextColumnsPropertyPanel::NotifyItemUpdate(sal_uInt16 nSID, SfxItemState eState,
                                                const SfxPoolItem* pState)
{
    switch (nSID)
    {
        case SID_ATTR_TEXTCOLUMNS_NUMBER:
            if (eState >= SfxItemState::DEFAULT)
            {
                if (const auto pItem = dynamic_cast<const SfxInt16Item*>(pState))
                    m_xColumnsNumber->set_value(pItem->GetValue());
            }
            break;
        case SID_ATTR_TEXTCOLUMNS_SPACING:
            if (eState >= SfxItemState::DEFAULT)
            {
                const MapUnit aUnit = GetUnit(mpBindings, SDRATTR_TEXTCOLUMNS_SPACING);
                if (const auto pItem = dynamic_cast<const SdrMetricItem*>(pState))
                    SetMetricValue(*m_xColumnsSpacing, pItem->GetValue(), aUnit);
            }
            break;
    }
}

std::unique_ptr<PanelLayout> TextColumnsPropertyPanel::Create(weld::Widget* pParent,
                                                              SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no parent Window given to TextColumnsPropertyPanel::Create"_ustr, nullptr, 0);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no SfxBindings given to TextColumnsPropertyPanel::Create"_ustr, nullptr, 2);

    return std::make_unique<TextColumnsPropertyPanel>(pParent, pBindings);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
