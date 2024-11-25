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
#include <unotools/viewoptions.hxx>

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

ScCondFormatManagerDlg::ScCondFormatManagerDlg(weld::Window* pParent, ScDocument& rDoc,
                                               const ScConditionalFormatList* pFormatList)
    : GenericDialogController(pParent, "modules/scalc/ui/condformatmanager.ui", "CondFormatManager")
    , m_bModified(false)
    , m_xFormatList(pFormatList ? new ScConditionalFormatList(*pFormatList) : nullptr)
    , m_xConditionalType(m_xBuilder->weld_combo_box("type"))
    , m_xConditionalCellValue(m_xBuilder->weld_combo_box("typeis"))
    , m_xBtnAdd(m_xBuilder->weld_button("add"))
    , m_xBtnRemove(m_xBuilder->weld_button("remove"))
    , m_xBtnEdit(m_xBuilder->weld_button("edit"))
    , m_xTreeView(m_xBuilder->weld_tree_view("CONTAINER"))
    , m_xCtrlManager(new ScCondFormatManagerWindow(*m_xTreeView, rDoc, m_xFormatList.get()))
{
    m_xBtnRemove->connect_clicked(LINK(this, ScCondFormatManagerDlg, RemoveBtnHdl));
    m_xBtnEdit->connect_clicked(LINK(this, ScCondFormatManagerDlg, EditBtnClickHdl));
    m_xBtnAdd->connect_clicked(LINK(this, ScCondFormatManagerDlg, AddBtnHdl));
    m_xTreeView->connect_row_activated(LINK(this, ScCondFormatManagerDlg, EditBtnHdl));

    SvtViewOptions aDlgOpt(EViewType::Dialog, "CondFormatDialog");
    if (aDlgOpt.Exists())
        m_xDialog->set_window_state(aDlgOpt.GetWindowState());

    UpdateButtonSensitivity();
}

ScCondFormatManagerDlg::~ScCondFormatManagerDlg()
{
   // tdf#101285 - Remember position of dialog
    SvtViewOptions aDlgOpt(EViewType::Dialog, "CondFormatDialog");
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
    auto id = m_xConditionalType->get_active();
    switch (id)
    {
        case 0: // Cell value
        {
            SfxInt16Item FormatRule(FN_PARAM_1,
                                    m_xConditionalCellValue->get_active_id().toUInt32());
            SfxBoolItem IsManaged(FN_PARAM_2, true);
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
