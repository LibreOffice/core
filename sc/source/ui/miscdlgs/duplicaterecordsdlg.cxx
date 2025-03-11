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

#include <scresid.hxx>
#include <dbdata.hxx>
#include <cellvalue.hxx>
#include <duplicaterecordsdlg.hxx>
#include <string>
#include <strings.hrc>
#include <tabprotection.hxx>
#include <gridwin.hxx>
#include <vector>
#include <officecfg/Office/Calc.hxx>

ScDuplicateRecordsDlg::ScDuplicateRecordsDlg(weld::Window* pParent,
                                             css::uno::Sequence<uno::Sequence<uno::Any>>& rData,
                                             ScViewData& rViewData, ScRange& rRange)
    : weld::GenericDialogController(pParent, "modules/scalc/ui/duplicaterecordsdlg.ui",
                                    "DuplicateRecordsDialog")
    , m_xIncludesHeaders(m_xBuilder->weld_check_button("includesheaders"))
    , m_xRadioRow(m_xBuilder->weld_radio_button("row"))
    , m_xRadioColumn(m_xBuilder->weld_radio_button("column"))
    , m_xRadioSelect(m_xBuilder->weld_radio_button("select"))
    , m_xRadioRemove(m_xBuilder->weld_radio_button("remove"))
    , m_xCheckList(m_xBuilder->weld_tree_view("checklist"))
    , m_xAllChkBtn(m_xBuilder->weld_check_button("allcheckbtn"))
    , m_xOkBtn(m_xBuilder->weld_button("okbtn"))
    , mrCellData(rData)
    , mrRange(rRange)
    , mrViewData(rViewData)
{
    m_xCheckList->enable_toggle_buttons(weld::ColumnToggleType::Check);
    m_xCheckList->connect_toggled(LINK(this, ScDuplicateRecordsDlg, RecordsChkHdl));
    Init();
}

ScDuplicateRecordsDlg::~ScDuplicateRecordsDlg()
{
    auto pChange(comphelper::ConfigurationChanges::create());
    officecfg::Office::Calc::Misc::HandleDuplicateRecords::RemoveDuplicateRows::set(
        m_xRadioRow->get_active(), pChange);
    pChange->commit();

    officecfg::Office::Calc::Misc::HandleDuplicateRecords::DataIncludesHeaders::set(
        m_xIncludesHeaders->get_active(), pChange);
    pChange->commit();

    officecfg::Office::Calc::Misc::HandleDuplicateRecords::RemoveRecords::set(
        m_xRadioRemove->get_active(), pChange);
    pChange->commit();
}

void ScDuplicateRecordsDlg::SetDialogData(bool bToggle)
{
    m_xCheckList->freeze();
    m_xCheckList->clear();

    if (m_xRadioRow->get_active())
    {
        if (m_xIncludesHeaders->get_active())
        {
            // insert the first row's contents
            ScRefCellValue aCell;
            for (SCCOL i = mrRange.aStart.Col(); i <= mrRange.aEnd.Col(); ++i)
            {
                aCell.assign(mrViewData.GetDocument(),
                             ScAddress{ i, mrRange.aStart.Row(), mrRange.aStart.Tab() });
                InsertEntry(aCell.getString(&mrViewData.GetDocument()), bToggle);
            }
        }
        else
        {
            for (int i = mrRange.aStart.Col(); i <= mrRange.aEnd.Col(); ++i)
            {
                OUString aStr(ScAddress(i, 0, mrViewData.GetTabNo())
                                  .Format(ScRefFlags::COL_VALID, &mrViewData.GetDocument()));
                InsertEntry(aStr, bToggle);
            }
        }
    }
    else
    {
        // insert row names
        if (m_xIncludesHeaders->get_active())
        {
            ScRefCellValue aCell;
            for (SCROW i = mrRange.aStart.Row(); i <= mrRange.aEnd.Row(); ++i)
            {
                aCell.assign(mrViewData.GetDocument(),
                             ScAddress{ mrRange.aStart.Col(), i, mrRange.aStart.Tab() });
                InsertEntry(aCell.getString(&mrViewData.GetDocument()), bToggle);
            }
        }
        else
        {
            for (int i = mrRange.aStart.Row() + 1; i <= mrRange.aEnd.Row() + 1; ++i)
            {
                std::string aStr = std::to_string(i);
                InsertEntry(rtl::OUString::fromUtf8(aStr), bToggle);
            }
        }
    }
    m_xCheckList->thaw();
}

void ScDuplicateRecordsDlg::SetDialogLabels()
{
    if (m_xRadioRow->get_active())
        m_xIncludesHeaders->set_label(ScResId(STR_DUPLICATERECORDS_DATACONATINSCOLUMNHEADERS));
    else
        m_xIncludesHeaders->set_label(ScResId(STR_DUPLICATERECORDS_DATACONATINSROWHEADERS));
}

void ScDuplicateRecordsDlg::InsertEntry(const OUString& rTxt, bool bToggle)
{
    m_xCheckList->append();
    const int nRow = m_xCheckList->n_children() - 1;
    m_xCheckList->set_toggle(nRow, bToggle ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xCheckList->set_text(nRow, rTxt, 0);
}

void ScDuplicateRecordsDlg::Init()
{
    m_xIncludesHeaders->connect_toggled(LINK(this, ScDuplicateRecordsDlg, HeaderCkbHdl));
    m_xRadioRow->connect_toggled(LINK(this, ScDuplicateRecordsDlg, OrientationHdl));
    // m_xRadioColumn->connect_toggled(LINK(this, ScDuplicateRecordsDlg, OrientationHdl));
    m_xOkBtn->connect_clicked(LINK(this, ScDuplicateRecordsDlg, OkHdl));
    m_xAllChkBtn->connect_toggled(LINK(this, ScDuplicateRecordsDlg, AllCheckBtnHdl));

    const ScDocument& rDoc = mrViewData.GetDocument();
    bool bIncludeHeaders
        = officecfg::Office::Calc::Misc::HandleDuplicateRecords::DataIncludesHeaders::get();

    ScDBCollection* pDBColl = rDoc.GetDBCollection();
    const SCTAB nCurTab = mrViewData.GetTabNo();
    if (pDBColl)
    {
        ScDBData* pDBData
            = pDBColl->GetDBAtArea(nCurTab, mrRange.aStart.Col(), mrRange.aStart.Row(),
                                   mrRange.aEnd.Col(), mrRange.aEnd.Row());
        if (pDBData)
            bIncludeHeaders = pDBData->HasHeader();
    }

    // defaults (find duplicate rows | data doesn't include headers)
    m_xIncludesHeaders->set_active(bIncludeHeaders);
    m_xRadioRow->set_active(
        officecfg::Office::Calc::Misc::HandleDuplicateRecords::RemoveDuplicateRows::get());
    m_xRadioColumn->set_active(
        !officecfg::Office::Calc::Misc::HandleDuplicateRecords::RemoveDuplicateRows::get());
    m_xRadioRemove->set_active(
        officecfg::Office::Calc::Misc::HandleDuplicateRecords::RemoveRecords::get());
    m_xRadioSelect->set_active(
        !officecfg::Office::Calc::Misc::HandleDuplicateRecords::RemoveRecords::get());

    SetDialogLabels();
    m_xAllChkBtn->set_state(TRISTATE_TRUE);
    SetDialogData(true);
}

IMPL_LINK_NOARG(ScDuplicateRecordsDlg, OrientationHdl, weld::Toggleable&, void)
{
    SetDialogLabels();
    SetDialogData(true);
}

IMPL_LINK_NOARG(ScDuplicateRecordsDlg, HeaderCkbHdl, weld::Toggleable&, void)
{
    SetDialogData(true);
}

IMPL_LINK_NOARG(ScDuplicateRecordsDlg, RecordsChkHdl, const weld::TreeView::iter_col&, void)
{
    int nRet = 0;
    int nTotalCount = 0;

    m_xCheckList->all_foreach([this, &nRet, &nTotalCount](weld::TreeIter& rEntry) {
        ++nTotalCount;
        if (m_xCheckList->get_toggle(rEntry) == TRISTATE_TRUE)
            ++nRet;
        return false;
    });

    if (nRet == nTotalCount)
        m_xAllChkBtn->set_state(TRISTATE_TRUE);
    else if (nRet == 0)
        m_xAllChkBtn->set_state(TRISTATE_FALSE);
    else
        m_xAllChkBtn->set_state(TRISTATE_INDET);
}

IMPL_LINK_NOARG(ScDuplicateRecordsDlg, AllCheckBtnHdl, weld::Toggleable&, void)
{
    if (m_xAllChkBtn->get_state() == TRISTATE_TRUE)
        SetDialogData(true);
    else
        SetDialogData(false);
}

IMPL_LINK_NOARG(ScDuplicateRecordsDlg, OkHdl, weld::Button&, void)
{
    maResponse.bRemove = m_xRadioRemove->get_active();
    maResponse.bDuplicateRows = m_xRadioRow->get_active();
    maResponse.bIncludesHeaders = m_xIncludesHeaders->get_active();
    int nCount = (maResponse.bDuplicateRows ? mrCellData[0].size() : mrCellData.size());

    for (int i = 0; i < nCount; ++i)
    {
        if (m_xCheckList->get_toggle(i))
            maResponse.vEntries.push_back(i);
    }

    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
