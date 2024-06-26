/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <condformatmgr.hxx>
#include <condformateasydlg.hxx>
#include <condformathelper.hxx>
#include <condformatdlg.hxx>
#include <document.hxx>
#include <conditio.hxx>
#include <sc.hrc>
#include <o3tl/safeint.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewsh.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <unotools/viewoptions.hxx>
#include <iostream>

ScCondFormatManagerWindow::ScCondFormatManagerWindow(weld::TreeView& rTreeView,
    ScDocument& rDoc, ScConditionalFormatList* pFormatList)
    : mrTreeView(rTreeView)
    , mrDoc(rDoc)
    , mpFormatList(pFormatList)
{
    mrTreeView.set_size_request(mrTreeView.get_approximate_digit_width() * 70,
                                mrTreeView.get_height_rows(20));
    setColSizes();

    Init();
    mrTreeView.set_selection_mode(SelectionMode::Multiple);
    mrTreeView.make_sorted();
}

void ScCondFormatManagerWindow::Init()
{
    mrTreeView.freeze();

    if (mpFormatList)
    {
        int nRow = 0;
        OUString sRangeStr;
        for(const auto& rItem : *mpFormatList)
        {
            const ScRangeList& aRange = rItem->GetRange();
            aRange.Format(sRangeStr, ScRefFlags::VALID, mrDoc, mrDoc.GetAddressConvention());
            mrTreeView.append(OUString::number(rItem->GetKey()), sRangeStr);
            mrTreeView.set_text(nRow, ScCondFormatHelper::GetExpression(*rItem, aRange.GetTopLeftCorner()), 1);
            ++nRow;
        }
    }

    mrTreeView.thaw();

    if (mpFormatList && !mpFormatList->empty())
        mrTreeView.select(0);
}

void ScCondFormatManagerWindow::DeleteSelection()
{
    auto aSelectedRows = mrTreeView.get_selected_rows();
    std::sort(aSelectedRows.begin(), aSelectedRows.end());
    for (auto it = aSelectedRows.rbegin(); it != aSelectedRows.rend(); ++it)
    {
        sal_Int32 nIndex = mrTreeView.get_id(*it).toInt32();
        mpFormatList->erase(nIndex);
        mrTreeView.remove(*it);
    }
}

ScConditionalFormat* ScCondFormatManagerWindow::GetSelection()
{
    int nEntry = mrTreeView.get_selected_index();
    if (nEntry == -1)
        return nullptr;

    sal_Int32 nIndex = mrTreeView.get_id(nEntry).toInt32();
    return mpFormatList->GetFormat(nIndex);
}

void ScCondFormatManagerWindow::setColSizes()
{
    std::vector<int> aWidths
    {
        o3tl::narrowing<int>(mrTreeView.get_size_request().Width() / 2)
    };
    mrTreeView.set_column_fixed_widths(aWidths);
}

ScCondFormatManagerDlg::ScCondFormatManagerDlg(weld::Window* pParent, ScDocument& rDoc, const ScConditionalFormatList* pFormatList)
    : GenericDialogController(pParent, u"modules/scalc/ui/condformatmanager.ui"_ustr, u"CondFormatManager"_ustr)
    , m_bModified(false)
    , m_xFormatList( pFormatList ? new ScConditionalFormatList(*pFormatList) : nullptr)
    , m_xConditionalType(m_xBuilder->weld_combo_box("type"))
    , m_xConditionalCellValue(m_xBuilder->weld_combo_box("typeis"))
    , m_xConditionalFormula(new formula::RefEdit(m_xBuilder->weld_entry("formula")))
    , m_xConditionalDate(m_xBuilder->weld_combo_box("datetype"))
    , m_xBtnAdd(m_xBuilder->weld_button(u"add"_ustr))
    , m_xBtnRemove(m_xBuilder->weld_button(u"remove"_ustr))
    , m_xBtnEdit(m_xBuilder->weld_button(u"edit"_ustr))
    , m_xTreeView(m_xBuilder->weld_tree_view(u"CONTAINER"_ustr))
    , m_xCtrlManager(new ScCondFormatManagerWindow(*m_xTreeView, rDoc, m_xFormatList.get()))
{
    m_xBtnRemove->connect_clicked(LINK(this, ScCondFormatManagerDlg, RemoveBtnHdl));
    m_xBtnEdit->connect_clicked(LINK(this, ScCondFormatManagerDlg, EditBtnClickHdl));
    m_xBtnAdd->connect_clicked(LINK(this, ScCondFormatManagerDlg, AddBtnHdl));
    m_xTreeView->connect_row_activated(LINK(this, ScCondFormatManagerDlg, EditBtnHdl));
    m_xTreeView->connect_changed(LINK(this, ScCondFormatManagerDlg, EntryFocus));
    m_xConditionalType->connect_changed(LINK(this, ScCondFormatManagerDlg, ComboHdl));

    SvtViewOptions aDlgOpt(EViewType::Dialog, u"CondFormatDialog"_ustr);
    if (aDlgOpt.Exists())
        m_xDialog->set_window_state(aDlgOpt.GetWindowState());

    UpdateButtonSensitivity();
}

ScCondFormatManagerDlg::~ScCondFormatManagerDlg()
{
   // tdf#101285 - Remember position of dialog
    SvtViewOptions aDlgOpt(EViewType::Dialog, u"CondFormatDialog"_ustr);
    aDlgOpt.SetWindowState(m_xDialog->get_window_state(vcl::WindowDataMask::Pos));
}

std::unique_ptr<ScConditionalFormatList> ScCondFormatManagerDlg::GetConditionalFormatList()
{
    return std::move(m_xFormatList);
}

void ScCondFormatManagerDlg::UpdateButtonSensitivity()
{
    bool bNewSensitivity = !m_xFormatList->empty();
    m_xBtnRemove->set_sensitive(bNewSensitivity);
    m_xBtnEdit->set_sensitive(bNewSensitivity);
}

// Get the current conditional format selected.
//
ScConditionalFormat* ScCondFormatManagerDlg::GetCondFormatSelected()
{
    return m_xCtrlManager->GetSelection();
}

void ScCondFormatManagerDlg::ShowEasyConditionalDialog()
{
    if (!SfxViewShell::Current())
        return;

    auto id = m_xConditionalType->get_active();
    SfxBoolItem IsManaged(FN_PARAM_2, true);
    switch (id)
    {
        case 0: // Cell value
        {
            SfxInt16Item FormatRule(FN_PARAM_1,
                                    m_xConditionalCellValue->get_active_id().toUInt32());
            SfxViewShell::Current()->GetDispatcher()->ExecuteList(
                SID_EASY_CONDITIONAL_FORMAT_DIALOG, SfxCallMode::ASYNCHRON,
                { &FormatRule, &IsManaged });
        }
        break;
        case 1: // Formula
        {
            SfxInt16Item FormatRule(FN_PARAM_1, static_cast<sal_Int16>(ScConditionMode::Formula));
            SfxStringItem Formula(FN_PARAM_3, m_xConditionalFormula->GetText());
            SfxViewShell::Current()->GetDispatcher()->ExecuteList(
                SID_EASY_CONDITIONAL_FORMAT_DIALOG, SfxCallMode::ASYNCHRON,
                { &FormatRule, &IsManaged, &Formula });
        }
        break;
        case 2: // Date
        {
            SfxInt16Item FormatRule(FN_PARAM_1, m_xConditionalDate->get_active_id().toUInt32());
            SfxViewShell::Current()->GetDispatcher()->ExecuteList(
                SID_EASY_CONDITIONAL_FORMAT_DIALOG, SfxCallMode::ASYNCHRON,
                { &FormatRule, &IsManaged });
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, RemoveBtnHdl, weld::Button&, void)
{
    m_xCtrlManager->DeleteSelection();
    m_bModified = true;
    UpdateButtonSensitivity();
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, EditBtnClickHdl, weld::Button&, void)
{
    EditBtnHdl(*m_xTreeView);
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, EditBtnHdl, weld::TreeView&, bool)
{
    ScConditionalFormat* pFormat = m_xCtrlManager->GetSelection();

    if (!pFormat)
        return true;

    m_bModified = true;
    m_xDialog->response( DLG_RET_EDIT );

    return true;
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, AddBtnHdl, weld::Button&, void)
{
    m_bModified = true;
    m_xDialog->response( DLG_RET_ADD );
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, ComboHdl, weld::ComboBox&, void)
{
    auto id = m_xConditionalType->get_active();
    switch (id)
    {
        case 0:
        {
            m_xConditionalCellValue->set_visible(true);
            m_xConditionalFormula->GetWidget()->set_visible(false);
            m_xConditionalDate->set_visible(false);
        }
        break;
        case 1:
        {
            m_xConditionalCellValue->set_visible(false);
            m_xConditionalFormula->GetWidget()->set_visible(true);
            m_xConditionalDate->set_visible(false);
        }
        break;
        case 2:
        {
            m_xConditionalCellValue->set_visible(false);
            m_xConditionalFormula->GetWidget()->set_visible(false);
            m_xConditionalDate->set_visible(true);
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG(ScCondFormatManagerDlg, EntryFocus, weld::TreeView&, void)
{
    ScConditionalFormat* conditionFrmt = m_xCtrlManager->GetSelection();

    if (!conditionFrmt)
        return;

    const ScFormatEntry* entry = conditionFrmt->GetEntry(0);
    if (!entry)
        return;
    auto type = entry->GetType();

    if (type == ScFormatEntry::Type::Condition)
    {
        const ScCondFormatEntry* conditionEntry = dynamic_cast<const ScCondFormatEntry*>(entry);
        auto conditionType = conditionEntry->GetOperation();

        if (conditionType == ScConditionMode::Direct) // Formula conditions
        {
            m_xConditionalType->set_active(1);
            this->ComboHdl(*m_xConditionalType);
            m_xConditionalFormula->SetText(
                conditionEntry->GetExpression(conditionEntry->GetSrcPos(), 0));
        }
        else
        {
            m_xConditionalType->set_active(0);
            this->ComboHdl(*m_xConditionalType);
            m_xConditionalCellValue->set_active(static_cast<int>(conditionType));
        }
    }
    else if (type == ScFormatEntry::Type::Date)
    {
        const ScCondDateFormatEntry* dateEntry = dynamic_cast<const ScCondDateFormatEntry*>(entry);
        auto dateType = dateEntry->GetDateType();
        m_xConditionalType->set_active(2);
        this->ComboHdl(*m_xConditionalType);
        m_xConditionalDate->set_active(dateType);
    }
}

void ScCondFormatManagerDlg::SetModified()
{
    m_bModified = true;
    UpdateButtonSensitivity();
}

bool ScCondFormatManagerDlg::CondFormatsChanged() const
{
    return m_bModified;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
