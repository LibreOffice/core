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

ScDataFormDlg::ScDataFormDlg(weld::Window* pParent, ScTabViewShell* pTabViewShellOri)
    : GenericDialogController(pParent, "modules/scalc/ui/dataform.ui", "DataFormDialog")
    , pTabViewShell(pTabViewShellOri)
    , aColLength(0)
    , nCurrentRow(0)
    , nStartCol(0)
    , nEndCol(0)
    , nStartRow(0)
    , nEndRow(0)
    , nTab(0)
    , m_xBtnNew(m_xBuilder->weld_button("new"))
    , m_xBtnDelete(m_xBuilder->weld_button("delete"))
    , m_xBtnRestore(m_xBuilder->weld_button("restore"))
    , m_xBtnPrev(m_xBuilder->weld_button("prev"))
    , m_xBtnNext(m_xBuilder->weld_button("next"))
    , m_xBtnClose(m_xBuilder->weld_button("close"))
    , m_xSlider(m_xBuilder->weld_scrolled_window("scrollbar", true))
    , m_xGrid(m_xBuilder->weld_container("grid"))
    , m_xFixedText(m_xBuilder->weld_label("label"))
{
    sNewRecord = m_xFixedText->get_label();

    //read header from current document, and add new controls
    OSL_ENSURE( pTabViewShell, "pTabViewShell is NULL! :-/" );
    ScViewData& rViewData = pTabViewShell->GetViewData();

    pDoc = &rViewData.GetDocument();

    {
        ScRange aRange;
        rViewData.GetSimpleArea( aRange );
        ScAddress aStart = aRange.aStart;
        ScAddress aEnd = aRange.aEnd;

        nStartCol = aStart.Col();
        nEndCol = aEnd.Col();
        nStartRow   = aStart.Row();
        nEndRow = aEnd.Row();

        nTab = rViewData.GetTabNo();
        bool bNoSelection(false);
        //if there is no selection
        if ((nStartCol == nEndCol) && (nStartRow == nEndRow))
            bNoSelection = true;

        if (bNoSelection)
        {
            //find last not blank cell in row
            for (int i=1;i<=MAX_DATAFORM_COLS;i++)
            {
                nEndCol++;
                OUString aColName = pDoc->GetString(nEndCol, nStartRow, nTab);
                int nColWidth = pDoc->GetColWidth( nEndCol, nTab );
                if (aColName.isEmpty() && nColWidth)
                {
                    nEndCol--;
                    break;
                }
            }

            //find first not blank cell in row
            for (int i=1;i<=MAX_DATAFORM_COLS;i++)
            {
                if (nStartCol <= 0)
                    break;
                nStartCol--;

                OUString aColName = pDoc->GetString(nStartCol, nStartRow, nTab);
                int nColWidth = pDoc->GetColWidth( nEndCol, nTab );
                if (aColName.isEmpty() && nColWidth)
                {
                    nStartCol++;
                    break;
                }
            }

            //skip leading hide column
            for (int i=1;i<=MAX_DATAFORM_COLS;i++)
            {
                int nColWidth = pDoc->GetColWidth( nStartCol, nTab );
                if (nColWidth)
                    break;
                nStartCol++;
            }

            if (nEndCol < nStartCol)
                nEndCol = nStartCol;

            //find last not blank cell in row
            for (int i=1;i<=MAX_DATAFORM_ROWS;i++)
            {
                nEndRow++;
                OUString aColName = pDoc->GetString(nStartCol, nEndRow, nTab);
                if (aColName.isEmpty())
                {
                    nEndRow--;
                    break;
                }
            }

            //find first not blank cell in row
            for (int i=1;i<=MAX_DATAFORM_ROWS;i++)
            {
                if (nStartRow <= 0)
                    break;
                nStartRow--;

                OUString aColName = pDoc->GetString(nStartCol, nStartRow, nTab);
                if (aColName.isEmpty())
                {
                    nStartRow++;
                    break;
                }
            }

            if (nEndRow < nStartRow)
                nEndRow = nStartRow;
        }

        nCurrentRow = nStartRow + 1;

        aColLength = nEndCol - nStartCol + 1;

        //new the controls
        m_aEntries.reserve(aColLength);

        sal_Int32 nGridRow = 0;
        for(sal_uInt16 nIndex = 0; nIndex < aColLength; ++nIndex)
        {
            OUString aFieldName = pDoc->GetString(nIndex + nStartCol, nStartRow, nTab);
            int nColWidth = pDoc->GetColWidth( nIndex + nStartCol, nTab );
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

    m_xSlider->vadjustment_configure(0, 0, nEndRow - nStartRow + 1, 1, 10, 1);

    m_xBtnNew->connect_clicked(LINK( this, ScDataFormDlg, Impl_NewHdl));
    m_xBtnPrev->connect_clicked(LINK( this, ScDataFormDlg, Impl_PrevHdl));
    m_xBtnNext->connect_clicked(LINK( this, ScDataFormDlg, Impl_NextHdl));

    m_xBtnRestore->connect_clicked(LINK( this, ScDataFormDlg, Impl_RestoreHdl));
    m_xBtnDelete->connect_clicked(LINK( this, ScDataFormDlg, Impl_DeleteHdl));
    m_xBtnClose->connect_clicked(LINK( this, ScDataFormDlg, Impl_CloseHdl));

    m_xSlider->connect_vadjustment_changed(LINK( this, ScDataFormDlg, Impl_ScrollHdl));

    SetButtonState();
}

ScDataFormDlg::~ScDataFormDlg()
{
}

void ScDataFormDlg::FillCtrls()
{
    for (sal_uInt16 i = 0; i < aColLength; ++i)
    {
        if (m_aEntries[i])
        {
            if (nCurrentRow<=nEndRow && pDoc)
            {
                OUString aFieldName(pDoc->GetString(i + nStartCol, nCurrentRow, nTab));
                m_aEntries[i]->m_xEdit->set_text(aFieldName);
            }
            else
                m_aEntries[i]->m_xEdit->set_text(OUString());
        }
    }

    if (nCurrentRow <= nEndRow)
    {
        OUString sLabel =
            OUString::number(static_cast<sal_Int32>(nCurrentRow - nStartRow)) +
            " / " +
            OUString::number(static_cast<sal_Int32>(nEndRow - nStartRow));
        m_xFixedText->set_label(sLabel);
    }
    else
        m_xFixedText->set_label(sNewRecord);

    m_xSlider->vadjustment_set_value(nCurrentRow-nStartRow-1);
}

IMPL_LINK( ScDataFormDlg, Impl_DataModifyHdl, weld::Entry&, rEdit, void)
{
    if (rEdit.get_value_changed_from_saved())
        m_xBtnRestore->set_sensitive(true);
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_NewHdl, weld::Button&, void)
{
    ScViewData& rViewData = pTabViewShell->GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    if ( !pDoc )
        return;

    bool bHasData = std::any_of(m_aEntries.begin(), m_aEntries.end(),
        [](const std::unique_ptr<ScDataFormFragment>& rElem) { return (rElem != nullptr) && (!rElem->m_xEdit->get_text().isEmpty()); });

    if ( !bHasData )
        return;

    pTabViewShell->DataFormPutData(nCurrentRow, nStartRow, nStartCol, nEndRow, nEndCol, m_aEntries, aColLength);
    nCurrentRow++;
    if (nCurrentRow >= nEndRow + 2)
    {
        nEndRow++;
        m_xSlider->vadjustment_set_upper(nEndRow - nStartRow + 1);
    }
    SetButtonState();
    FillCtrls();
    pDocSh->SetDocumentModified();
    pDocSh->PostPaintGridAll();
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_PrevHdl, weld::Button&, void)
{
    if (pDoc)
    {
        if ( nCurrentRow > nStartRow +1 )
            nCurrentRow--;

        SetButtonState();
        FillCtrls();
    }
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_NextHdl, weld::Button&, void)
{
    if (pDoc)
    {
        if ( nCurrentRow <= nEndRow)
            nCurrentRow++;

        SetButtonState();
        FillCtrls();
    }
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_RestoreHdl, weld::Button&, void)
{
    if (pDoc)
    {
        FillCtrls();
    }
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_DeleteHdl, weld::Button&, void)
{
    ScViewData& rViewData = pTabViewShell->GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    if (!pDoc)
        return;

    ScRange aRange(nStartCol, nCurrentRow, nTab, nEndCol, nCurrentRow, nTab);
    pDoc->DeleteRow(aRange);
    nEndRow--;

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
    nCurrentRow = nStartRow + nOffset + 1;
    SetButtonState();
    FillCtrls();
}

void ScDataFormDlg::SetButtonState()
{
    if (nCurrentRow > nEndRow)
    {
        m_xBtnDelete->set_sensitive( false );
        m_xBtnNext->set_sensitive( false );
    }
    else
    {
        m_xBtnDelete->set_sensitive(true);
        m_xBtnNext->set_sensitive(true);
    }

    if (nCurrentRow == nStartRow + 1)
        m_xBtnPrev->set_sensitive( false );
    else
        m_xBtnPrev->set_sensitive(true);

    m_xBtnRestore->set_sensitive( false );
    if (!m_aEntries.empty() && m_aEntries[0] != nullptr)
        m_aEntries[0]->m_xEdit->grab_focus();
}

ScDataFormFragment::ScDataFormFragment(weld::Container* pGrid, int nLine)
    : m_xBuilder(Application::CreateBuilder(pGrid, "modules/scalc/ui/dataformfragment.ui"))
    , m_xLabel(m_xBuilder->weld_label("label"))
    , m_xEdit(m_xBuilder->weld_entry("entry"))
{
    m_xLabel->set_grid_left_attach(0);
    m_xLabel->set_grid_top_attach(nLine);

    m_xEdit->set_grid_left_attach(1);
    m_xEdit->set_grid_top_attach(nLine);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
