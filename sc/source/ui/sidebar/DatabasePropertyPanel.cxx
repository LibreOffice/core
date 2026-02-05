/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "DatabasePropertyPanel.hxx"
#include <editeng/justifyitem.hxx>
#include <sc.hrc>
#include <attrib.hxx>
#include <scitems.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <svx/rotmodit.hxx>
#include <svtools/unitconv.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <dbdata.hxx>

using namespace css;
using namespace css::uno;

// namespace open

namespace sc::sidebar
{
ScDatabasePropertyPanel::ScDatabasePropertyPanel(weld::Widget* pParent,
                                                 const css::uno::Reference<css::frame::XFrame>&,
                                                 SfxBindings* pBindings)
    : PanelLayout(pParent, u"DatabasePropertyPanel"_ustr,
                  u"modules/scalc/ui/sidebardatabase.ui"_ustr)
    , m_xChkHeaderRow(m_xBuilder->weld_check_button(u"chk_header_row"_ustr))
    , m_xChkTotalRow(m_xBuilder->weld_check_button(u"chk_total_row"_ustr))
    , m_xChkFilterButtons(m_xBuilder->weld_check_button(u"chk_filter_buttons"_ustr))
    , m_xChkBandedRows(m_xBuilder->weld_check_button(u"chk_banded_rows"_ustr))
    , m_xChkBandedColumns(m_xBuilder->weld_check_button(u"chk_banded_cols"_ustr))
    , m_xChkFirstColumn(m_xBuilder->weld_check_button(u"chk_first_column"_ustr))
    , m_xChkLastColumn(m_xBuilder->weld_check_button(u"chk_last_column"_ustr))
    , m_xCmbStyle(m_xBuilder->weld_combo_box(u"cb_styles"_ustr))
    , maHeaderRowCtrl(SID_DATABASE_SETTINGS, *pBindings, *this)
    , mpBindings(pBindings)
{
    Initialize();
}

ScDatabasePropertyPanel::~ScDatabasePropertyPanel()
{
    m_xChkHeaderRow.reset();
    m_xChkTotalRow.reset();
    m_xChkFilterButtons.reset();
    m_xChkBandedRows.reset();
    m_xChkBandedColumns.reset();
    m_xChkFirstColumn.reset();
    m_xChkLastColumn.reset();
    m_xCmbStyle.reset();

    maHeaderRowCtrl.dispose();
}

void ScDatabasePropertyPanel::Initialize()
{
    m_xChkHeaderRow->connect_toggled(LINK(this, ScDatabasePropertyPanel, EntryChanged));
    m_xChkTotalRow->connect_toggled(LINK(this, ScDatabasePropertyPanel, EntryChanged));
    m_xChkFilterButtons->connect_toggled(LINK(this, ScDatabasePropertyPanel, EntryChanged));
    m_xChkBandedRows->connect_toggled(LINK(this, ScDatabasePropertyPanel, EntryChanged));
    m_xChkBandedColumns->connect_toggled(LINK(this, ScDatabasePropertyPanel, EntryChanged));
    m_xChkFirstColumn->connect_toggled(LINK(this, ScDatabasePropertyPanel, EntryChanged));
    m_xChkLastColumn->connect_toggled(LINK(this, ScDatabasePropertyPanel, EntryChanged));
    m_xCmbStyle->connect_changed(LINK(this, ScDatabasePropertyPanel, StyleChanged));
}

std::unique_ptr<PanelLayout>
ScDatabasePropertyPanel::Create(weld::Widget* pParent,
                                const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            u"no parent Window given to ScDatabasePropertyPanel::Create"_ustr, nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException(
            u"no XFrame given to ScDatabasePropertyPanel::Create"_ustr, nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException(
            u"no SfxBindings given to ScDatabasePropertyPanel::Create"_ustr, nullptr, 2);

    return std::make_unique<ScDatabasePropertyPanel>(pParent, rxFrame, pBindings);
}

void ScDatabasePropertyPanel::HandleContextChange(const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
}

void ScDatabasePropertyPanel::NotifyItemUpdate(sal_uInt16 nSID, SfxItemState eState,
                                               const SfxPoolItem* pState)
{
    switch (nSID)
    {
        case SID_DATABASE_SETTINGS:
            if (eState >= SfxItemState::DEFAULT
                && dynamic_cast<const ScDatabaseSettingItem*>(pState))
            {
                const ScDatabaseSettingItem* pItem
                    = static_cast<const ScDatabaseSettingItem*>(pState);
                m_xChkHeaderRow->set_active(pItem->HasHeaderRow());
                m_xChkTotalRow->set_active(pItem->HasTotalRow());
                m_xChkFirstColumn->set_active(pItem->HasFirstCol());
                m_xChkLastColumn->set_active(pItem->HasLastCol());
                m_xChkBandedRows->set_active(pItem->HasStripedRows());
                m_xChkBandedColumns->set_active(pItem->HasStripedCols());
                m_xChkFilterButtons->set_active(pItem->HasShowFilters());

                const OUString& rName = pItem->GetStyleID();
                if (!rName.isEmpty())
                {
                    int nPos = m_xCmbStyle->find_id(rName);
                    if (nPos == -1)
                    {
                        m_xCmbStyle->insert(0, rName);
                        m_xCmbStyle->set_id(0, rName);
                        nPos = 0;
                    }

                    m_xCmbStyle->set_active(nPos);
                }
            }
            break;
    }
}

IMPL_LINK_NOARG(ScDatabasePropertyPanel, EntryChanged, weld::Toggleable&, void)
{
    ScDatabaseSettingItem aItem(m_xChkHeaderRow->get_active(), m_xChkTotalRow->get_active(),
                                m_xChkFirstColumn->get_active(), m_xChkLastColumn->get_active(),
                                m_xChkBandedRows->get_active(), m_xChkBandedColumns->get_active(),
                                m_xChkFilterButtons->get_active(), m_xCmbStyle->get_active_id());
    GetBindings()->GetDispatcher()->ExecuteList(SID_DATABASE_SETTINGS, SfxCallMode::RECORD,
                                                { &aItem });
}

IMPL_LINK_NOARG(ScDatabasePropertyPanel, StyleChanged, weld::ComboBox&, void)
{
    ScDatabaseSettingItem aItem(m_xChkHeaderRow->get_active(), m_xChkTotalRow->get_active(),
                                m_xChkFirstColumn->get_active(), m_xChkLastColumn->get_active(),
                                m_xChkBandedRows->get_active(), m_xChkBandedColumns->get_active(),
                                m_xChkFilterButtons->get_active(), m_xCmbStyle->get_active_id());
    GetBindings()->GetDispatcher()->ExecuteList(SID_DATABASE_SETTINGS, SfxCallMode::RECORD,
                                                { &aItem });
}

// namespace close

} // end of namespace ::sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
