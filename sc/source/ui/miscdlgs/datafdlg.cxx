/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#undef SC_DLLIMPLEMENTATION

#include <datafdlg.hxx>
#include <viewdata.hxx>
#include <docsh.hxx>
#include <tabvwsh.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld/Dialog.hxx>
#include <osl/diagnose.h>

ScDataFormDlg::ScDataFormDlg(weld::Window* pParent, ScTabViewShell& rTabViewShellOri)
    : GenericDialogController(pParent, u"modules/scalc/ui/dataform.ui"_ustr, u"DataFormDialog"_ustr)
    , m_rTabViewShell(rTabViewShellOri)
    , m_rDoc(m_rTabViewShell.GetViewData().GetDocument())
    , m_aColLength(0)
    , m_nCurrentRow(0)
    , m_nStartCol(0)
    , m_nEndCol(0)
    , m_nStartRow(0)
    , m_nEndRow(0)
    , m_nTab(0)
    , m_xBtnNew(m_xBuilder->weld_button(u"new"_ustr))
    , m_xBtnDelete(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xBtnRestore(m_xBuilder->weld_button(u"restore"_ustr))
    , m_xBtnPrev(m_xBuilder->weld_button(u"prev"_ustr))
    , m_xBtnNext(m_xBuilder->weld_button(u"next"_ustr))
    , m_xBtnClose(m_xBuilder->weld_button(u"close"_ustr))
    , m_xSlider(m_xBuilder->weld_scrolled_window(u"scrollbar"_ustr, true))
    , m_xGrid(m_xBuilder->weld_grid(u"grid"_ustr))
    , m_xFixedText(m_xBuilder->weld_label(u"label"_ustr))
{
    m_sNewRecord = m_xFixedText->get_label();

    //read header from current document, and add new controls
    ScViewData& rViewData = m_rTabViewShell.GetViewData();

    {
        ScRange aRange;
        rViewData.GetSimpleArea( aRange );
        ScAddress aStart = aRange.aStart;
        ScAddress aEnd = aRange.aEnd;

        m_nStartCol = aStart.Col();
        m_nEndCol = aEnd.Col();
        m_nStartRow = aStart.Row();
        m_nEndRow = aEnd.Row();

        m_nTab = rViewData.CurrentTabForData();
        bool bNoSelection(false);
        //if there is no selection
        if ((m_nStartCol == m_nEndCol) && (m_nStartRow == m_nEndRow))
            bNoSelection = true;

        if (bNoSelection)
        {
            //find last not blank cell in row
            for (int i=1;i<=MAX_DATAFORM_COLS;i++)
            {
                m_nEndCol++;
                OUString aColName = m_rDoc.GetString(m_nEndCol, m_nStartRow, m_nTab);
                int nColWidth = m_rDoc.GetColWidth(m_nEndCol, m_nTab);
                if (aColName.isEmpty() && nColWidth)
                {
                    m_nEndCol--;
                    break;
                }
            }

            //find first not blank cell in row
            for (int i=1;i<=MAX_DATAFORM_COLS;i++)
            {
                if (m_nStartCol <= 0)
                    break;
                m_nStartCol--;

                OUString aColName = m_rDoc.GetString(m_nStartCol, m_nStartRow, m_nTab);
                int nColWidth = m_rDoc.GetColWidth(m_nEndCol, m_nTab);
                if (aColName.isEmpty() && nColWidth)
                {
                    m_nStartCol++;
                    break;
                }
            }

            //skip leading hide column
            for (int i=1;i<=MAX_DATAFORM_COLS;i++)
            {
                int nColWidth = m_rDoc.GetColWidth(m_nStartCol, m_nTab);
                if (nColWidth)
                    break;
                m_nStartCol++;
            }

            if (m_nEndCol < m_nStartCol)
                m_nEndCol = m_nStartCol;

            //find last not blank cell in row
            for (int i=1;i<=MAX_DATAFORM_ROWS;i++)
            {
                m_nEndRow++;
                OUString aColName = m_rDoc.GetString(m_nStartCol, m_nEndRow, m_nTab);
                if (aColName.isEmpty())
                {
                    m_nEndRow--;
                    break;
                }
            }

            //find first not blank cell in row
            for (int i=1;i<=MAX_DATAFORM_ROWS;i++)
            {
                if (m_nStartRow <= 0)
                    break;
                m_nStartRow--;

                OUString aColName = m_rDoc.GetString(m_nStartCol, m_nStartRow, m_nTab);
                if (aColName.isEmpty())
                {
                    m_nStartRow++;
                    break;
                }
            }

            if (m_nEndRow < m_nStartRow)
                m_nEndRow = m_nStartRow;
        }

        m_nCurrentRow = m_nStartRow + 1;

        m_aColLength = m_nEndCol - m_nStartCol + 1;

        //new the controls
        m_aEntries.reserve(m_aColLength);

        sal_Int32 nGridRow = 0;
        for (sal_uInt16 nIndex = 0; nIndex < m_aColLength; ++nIndex)
        {
            OUString aFieldName = m_rDoc.GetString(nIndex + m_nStartCol, m_nStartRow, m_nTab);
            int nColWidth = m_rDoc.GetColWidth(nIndex + m_nStartCol, m_nTab);
            if (nColWidth)
            {
                m_aEntries.emplace_back(new ScDataFormFragment(m_xGrid.get(), nGridRow));

                ++nGridRow;

                m_aEntries[nIndex]->m_xLabel->set_label(aFieldName);
                m_aEntries[nIndex]->m_xLabel->show();
                m_aEntries[nIndex]->m_xEdit->show();
            }
            else
            {
                m_aEntries.emplace_back(nullptr );
            }
            if (m_aEntries[nIndex] != nullptr)
            {
                m_aEntries[nIndex]->m_xEdit->connect_changed(LINK( this, ScDataFormDlg, Impl_DataModifyHdl));
                m_aEntries[nIndex]->m_xEdit->save_value();
            }
        }
    }

    FillCtrls();

    m_xSlider->vadjustment_configure(0, m_nEndRow - m_nStartRow + 1, 1, 10, 1);

    m_xBtnNew->connect_clicked(LINK( this, ScDataFormDlg, Impl_NewHdl));
    m_xBtnPrev->connect_clicked(LINK( this, ScDataFormDlg, Impl_PrevHdl));
    m_xBtnNext->connect_clicked(LINK( this, ScDataFormDlg, Impl_NextHdl));

    m_xBtnRestore->connect_clicked(LINK( this, ScDataFormDlg, Impl_RestoreHdl));
    m_xBtnDelete->connect_clicked(LINK( this, ScDataFormDlg, Impl_DeleteHdl));
    m_xBtnClose->connect_clicked(LINK( this, ScDataFormDlg, Impl_CloseHdl));

    m_xSlider->connect_vadjustment_value_changed(LINK( this, ScDataFormDlg, Impl_ScrollHdl));

    SetButtonState();
}

ScDataFormDlg::~ScDataFormDlg()
{
}

void ScDataFormDlg::FillCtrls()
{
    for (sal_uInt16 i = 0; i < m_aColLength; ++i)
    {
        if (m_aEntries[i])
        {
            if (m_nCurrentRow <= m_nEndRow)
            {
                OUString aFieldName(m_rDoc.GetString(i + m_nStartCol, m_nCurrentRow, m_nTab));
                m_aEntries[i]->m_xEdit->set_text(aFieldName);
            }
            else
                m_aEntries[i]->m_xEdit->set_text(OUString());
        }
    }

    if (m_nCurrentRow <= m_nEndRow)
    {
        OUString sLabel = OUString::number(static_cast<sal_Int32>(m_nCurrentRow - m_nStartRow))
                          + " / "
                          + OUString::number(static_cast<sal_Int32>(m_nEndRow - m_nStartRow));
        m_xFixedText->set_label(sLabel);
    }
    else
        m_xFixedText->set_label(m_sNewRecord);

    m_xSlider->vadjustment_set_value(m_nCurrentRow - m_nStartRow - 1);
}

IMPL_LINK( ScDataFormDlg, Impl_DataModifyHdl, weld::Entry&, rEdit, void)
{
    if (rEdit.get_value_changed_from_saved())
        m_xBtnRestore->set_sensitive(true);
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_NewHdl, weld::Button&, void)
{
    ScViewData& rViewData = m_rTabViewShell.GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();

    bool bHasData = std::any_of(m_aEntries.begin(), m_aEntries.end(),
        [](const std::unique_ptr<ScDataFormFragment>& rElem) { return (rElem != nullptr) && (!rElem->m_xEdit->get_text().isEmpty()); });

    if ( !bHasData )
        return;

    m_rTabViewShell.DataFormPutData(m_nCurrentRow, m_nStartRow, m_nStartCol, m_nEndRow, m_nEndCol,
                                    m_aEntries, m_aColLength);
    m_nCurrentRow++;
    if (m_nCurrentRow >= m_nEndRow + 2)
    {
        m_nEndRow++;
        m_xSlider->vadjustment_set_upper(m_nEndRow - m_nStartRow + 1);
    }
    SetButtonState();
    FillCtrls();
    pDocSh->SetDocumentModified();
    pDocSh->PostPaintGridAll();
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_PrevHdl, weld::Button&, void)
{
    if (m_nCurrentRow > m_nStartRow + 1)
        m_nCurrentRow--;

    SetButtonState();
    FillCtrls();
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_NextHdl, weld::Button&, void)
{
    if (m_nCurrentRow <= m_nEndRow)
        m_nCurrentRow++;

    SetButtonState();
    FillCtrls();
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_RestoreHdl, weld::Button&, void)
{
    FillCtrls();
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_DeleteHdl, weld::Button&, void)
{
    ScViewData& rViewData = m_rTabViewShell.GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();

    ScRange aRange(m_nStartCol, m_nCurrentRow, m_nTab, m_nEndCol, m_nCurrentRow, m_nTab);
    m_rDoc.DeleteRow(aRange);
    m_nEndRow--;

    SetButtonState();
    pDocSh->GetUndoManager()->Clear();

    FillCtrls();
    pDocSh->SetDocumentModified();
    pDocSh->PostPaintGridAll();
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_CloseHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_ScrollHdl, weld::ScrolledWindow&, void)
{
    auto nOffset = m_xSlider->vadjustment_get_value();
    m_nCurrentRow = m_nStartRow + nOffset + 1;
    SetButtonState();
    FillCtrls();
}

void ScDataFormDlg::SetButtonState()
{
    if (m_nCurrentRow > m_nEndRow)
    {
        m_xBtnDelete->set_sensitive( false );
        m_xBtnNext->set_sensitive( false );
    }
    else
    {
        m_xBtnDelete->set_sensitive(true);
        m_xBtnNext->set_sensitive(true);
    }

    if (m_nCurrentRow == m_nStartRow + 1)
        m_xBtnPrev->set_sensitive( false );
    else
        m_xBtnPrev->set_sensitive(true);

    m_xBtnRestore->set_sensitive( false );
    if (!m_aEntries.empty() && m_aEntries[0] != nullptr)
        m_aEntries[0]->m_xEdit->grab_focus();
}

ScDataFormFragment::ScDataFormFragment(weld::Grid* pGrid, int nLine)
    : m_xBuilder(Application::CreateBuilder(pGrid, u"modules/scalc/ui/dataformfragment.ui"_ustr))
    , m_xLabel(m_xBuilder->weld_label(u"label"_ustr))
    , m_xEdit(m_xBuilder->weld_entry(u"entry"_ustr))
{
    pGrid->set_child_left_attach(*m_xLabel, 0);
    pGrid->set_child_top_attach(*m_xLabel, nLine);

    pGrid->set_child_left_attach(*m_xEdit, 1);
    pGrid->set_child_top_attach(*m_xEdit, nLine);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
