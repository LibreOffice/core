/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <TableStylesBox.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <vcl/weld/Builder.hxx>

#include <globstr.hrc>
#include <scresid.hxx>
#include <sc.hrc>
#include <docsh.hxx>
#include <viewdata.hxx>

TableStylesBox::TableStylesBox(vcl::Window* pParent)
    : InterimItemWindow(pParent, u"modules/scalc/ui/tablestylesbox.ui"_ustr, u"TableStylesBox"_ustr,
                        true, reinterpret_cast<sal_uInt64>(SfxViewShell::Current()))
    , m_xChkHeaderRow(m_xBuilder->weld_check_button(u"chk_header_row2"_ustr))
    , m_xChkTotalRow(m_xBuilder->weld_check_button(u"chk_total_row2"_ustr))
    , m_xChkFilterButtons(m_xBuilder->weld_check_button(u"chk_filter_buttons2"_ustr))
    , m_xChkBandedRows(m_xBuilder->weld_check_button(u"chk_banded_rows2"_ustr))
    , m_xChkBandedColumns(m_xBuilder->weld_check_button(u"chk_banded_cols2"_ustr))
    , m_xChkFirstColumn(m_xBuilder->weld_check_button(u"chk_first_column2"_ustr))
    , m_xChkLastColumn(m_xBuilder->weld_check_button(u"chk_last_column2"_ustr))
    , m_xCmbStyle(m_xBuilder->weld_combo_box(u"tablestyles_cb2"_ustr))
{
    m_xChkHeaderRow->connect_toggled(LINK(this, TableStylesBox, ToggleHdl));
    m_xChkTotalRow->connect_toggled(LINK(this, TableStylesBox, ToggleHdl));
    m_xChkFilterButtons->connect_toggled(LINK(this, TableStylesBox, ToggleHdl));
    m_xChkBandedRows->connect_toggled(LINK(this, TableStylesBox, ToggleHdl));
    m_xChkBandedColumns->connect_toggled(LINK(this, TableStylesBox, ToggleHdl));
    m_xChkFirstColumn->connect_toggled(LINK(this, TableStylesBox, ToggleHdl));
    m_xChkLastColumn->connect_toggled(LINK(this, TableStylesBox, ToggleHdl));

    m_xCmbStyle->connect_changed(LINK(this, TableStylesBox, SelectHdl));
    m_xCmbStyle->connect_key_press(LINK(this, TableStylesBox, KeyInputHdl));
    // to have enough space for the entries since we have two row of entries
    SetSizePixel(pParent->get_preferred_size() * 2);
    // TODO: no Table tab page for core
}

void TableStylesBox::dispose()
{
    m_xChkHeaderRow.reset();
    m_xChkTotalRow.reset();
    m_xChkFilterButtons.reset();
    m_xChkBandedRows.reset();
    m_xChkBandedColumns.reset();
    m_xChkFirstColumn.reset();
    m_xChkLastColumn.reset();
    m_xCmbStyle.reset();

    InterimItemWindow::dispose();
}

TableStylesBox::~TableStylesBox() { disposeOnce(); }

void TableStylesBox::Update(const ScDatabaseSettingItem* pItem)
{
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

IMPL_LINK_NOARG(TableStylesBox, ToggleHdl, weld::Toggleable&, void)
{
    auto* pViewFrame = SfxViewFrame::Current();
    if (!pViewFrame)
        return;

    SfxDispatcher* pDispatcher = pViewFrame->GetBindings().GetDispatcher();
    if (!pDispatcher)
        return;

    ScDatabaseSettingItem aItem(m_xChkHeaderRow->get_active(), m_xChkTotalRow->get_active(),
                                m_xChkFirstColumn->get_active(), m_xChkLastColumn->get_active(),
                                m_xChkBandedRows->get_active(), m_xChkBandedColumns->get_active(),
                                m_xChkFilterButtons->get_active(), m_xCmbStyle->get_active_id());
    pDispatcher->ExecuteList(SID_DATABASE_SETTINGS, SfxCallMode::RECORD, { &aItem });
    pViewFrame->GetWindow().GrabFocus();
}

IMPL_LINK_NOARG(TableStylesBox, SelectHdl, weld::ComboBox&, void)
{
    auto* pViewFrame = SfxViewFrame::Current();
    if (!pViewFrame)
        return;

    SfxDispatcher* pDispatcher = pViewFrame->GetBindings().GetDispatcher();
    if (!pDispatcher)
        return;

    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return;

    const ScAddress aAddr = pViewData->GetCurPos();
    const ScDBData* pDBData = pViewData->GetDocument().GetTableDBAtCursor(
        aAddr.Col(), aAddr.Row(), aAddr.Tab(), ScDBDataPortion::AREA);

    bool bChanged = true;
    const OUString sValue = m_xCmbStyle->get_active_id();
    if (pDBData)
    {
        const ScTableStyleParam* pParam = pDBData->GetTableStyleInfo();
        if (pParam->maStyleID == sValue)
            bChanged = false;
    }

    if (bChanged)
    {
        ScDatabaseSettingItem aItem(m_xChkHeaderRow->get_active(), m_xChkTotalRow->get_active(),
                                    m_xChkFirstColumn->get_active(), m_xChkLastColumn->get_active(),
                                    m_xChkBandedRows->get_active(),
                                    m_xChkBandedColumns->get_active(),
                                    m_xChkFilterButtons->get_active(), sValue);
        pDispatcher->ExecuteList(SID_DATABASE_SETTINGS, SfxCallMode::RECORD, { &aItem });
        pViewFrame->GetWindow().GrabFocus();
    }
}

IMPL_LINK(TableStylesBox, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
