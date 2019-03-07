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

#include <rtl/ustrbuf.hxx>

#define HDL(hdl)            LINK( this, ScDataFormDlg, hdl )

ScDataFormDlg::ScDataFormDlg(vcl::Window* pParent, ScTabViewShell* pTabViewShellOri)
    : ModalDialog(pParent, "DataFormDialog", "modules/scalc/ui/dataform.ui")
    , pTabViewShell(pTabViewShellOri)
    , aColLength(0)
    , nCurrentRow(0)
    , nStartCol(0)
    , nEndCol(0)
    , nStartRow(0)
    , nEndRow(0)
    , nTab(0)
{
    get(m_pBtnNew, "new");
    get(m_pBtnDelete, "delete");
    get(m_pBtnRestore, "restore");
    get(m_pBtnPrev, "prev");
    get(m_pBtnNext, "next");
    get(m_pBtnClose, "close");
    get(m_pFixedText, "label");
    sNewRecord = m_pFixedText->GetText();
    get(m_pSlider, "scrollbar");
    get(m_pGrid, "grid");

    //read header form current document, and add new controls
    OSL_ENSURE( pTabViewShell, "pTabViewShell is NULL! :-/" );
    ScViewData& rViewData = pTabViewShell->GetViewData();

    pDoc = rViewData.GetDocument();
    if (pDoc)
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
        maFixedTexts.reserve(aColLength);
        maEdits.reserve(aColLength);

        sal_Int32 nGridRow = 0;
        for(sal_uInt16 nIndex = 0; nIndex < aColLength; ++nIndex)
        {
            OUString aFieldName = pDoc->GetString(nIndex + nStartCol, nStartRow, nTab);
            int nColWidth = pDoc->GetColWidth( nIndex + nStartCol, nTab );
            if (nColWidth)
            {
                maFixedTexts.push_back( VclPtr<FixedText>::Create(m_pGrid) );
                maEdits.push_back( VclPtr<Edit>::Create(m_pGrid, WB_BORDER) );

                maFixedTexts[nIndex]->set_grid_left_attach(0);
                maEdits[nIndex]->set_grid_left_attach(1);
                maFixedTexts[nIndex]->set_grid_top_attach(nGridRow);
                maEdits[nIndex]->set_grid_top_attach(nGridRow);

                maEdits[nIndex]->SetWidthInChars(32);
                maEdits[nIndex]->set_hexpand(true);

                ++nGridRow;

                maFixedTexts[nIndex]->SetText(aFieldName);
                maFixedTexts[nIndex]->Show();
                maEdits[nIndex]->Show();
            }
            else
            {
                maFixedTexts.emplace_back(nullptr );
                maEdits.emplace_back(nullptr );
            }
            if (maEdits[nIndex] != nullptr)
                maEdits[nIndex]->SetModifyHdl( HDL(Impl_DataModifyHdl) );
        }
    }

    FillCtrls();

    m_pSlider->SetPageSize( 10 );
    m_pSlider->SetVisibleSize( 1 );
    m_pSlider->SetLineSize( 1 );
    m_pSlider->SetRange( Range( 0, nEndRow - nStartRow + 1) );
    m_pSlider->Show();

    m_pBtnNew->SetClickHdl(HDL(Impl_NewHdl));
    m_pBtnPrev->SetClickHdl(HDL(Impl_PrevHdl));
    m_pBtnNext->SetClickHdl(HDL(Impl_NextHdl));

    m_pBtnRestore->SetClickHdl(HDL(Impl_RestoreHdl));
    m_pBtnDelete->SetClickHdl(HDL(Impl_DeleteHdl));
    m_pBtnClose->SetClickHdl(HDL(Impl_CloseHdl));

    m_pSlider->SetEndScrollHdl(HDL(Impl_ScrollHdl));

    SetButtonState();
}

ScDataFormDlg::~ScDataFormDlg()
{
    disposeOnce();
}

void ScDataFormDlg::dispose()
{
    m_pBtnNew.clear();
    m_pBtnDelete.clear();
    m_pBtnRestore.clear();
    m_pBtnPrev.clear();
    m_pBtnNext.clear();
    m_pBtnClose.clear();
    m_pSlider.clear();
    m_pFixedText.clear();
    for ( auto& rxFTIter : maFixedTexts )
        rxFTIter.disposeAndClear();
    for ( auto& rxEdit : maEdits )
        rxEdit.disposeAndClear();
    maFixedTexts.clear();
    maEdits.clear();
    m_pGrid.clear();
    ModalDialog::dispose();
}

void ScDataFormDlg::FillCtrls()
{
    for (sal_uInt16 i = 0; i < aColLength; ++i)
    {
        if (maEdits[i] != nullptr)
        {
            if (nCurrentRow<=nEndRow && pDoc)
            {
                OUString  aFieldName(pDoc->GetString(i + nStartCol, nCurrentRow, nTab));
                maEdits[i]->SetText(aFieldName);
            }
            else
                maEdits[i]->SetText(OUString());
        }
    }

    if (nCurrentRow <= nEndRow)
    {
        OUStringBuffer aBuf;
        aBuf.append(static_cast<sal_Int32>(nCurrentRow - nStartRow));
        aBuf.append(" / ");
        aBuf.append(static_cast<sal_Int32>(nEndRow - nStartRow));
        m_pFixedText->SetText(aBuf.makeStringAndClear());
    }
    else
        m_pFixedText->SetText(sNewRecord);

    m_pSlider->SetThumbPos(nCurrentRow-nStartRow-1);
}

IMPL_LINK( ScDataFormDlg, Impl_DataModifyHdl, Edit&, rEdit, void)
{
    if ( rEdit.IsModified() )
        m_pBtnRestore->Enable();
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_NewHdl, Button*, void)
{
    ScViewData& rViewData = pTabViewShell->GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    if ( pDoc )
    {
        bool bHasData = std::any_of(maEdits.begin(), maEdits.end(),
            [](const VclPtr<Edit>& rxEdit) { return (rxEdit != nullptr) && (!rxEdit->GetText().isEmpty()); });

        if ( bHasData )
        {
            pTabViewShell->DataFormPutData( nCurrentRow , nStartRow , nStartCol , nEndRow , nEndCol , maEdits , aColLength );
            nCurrentRow++;
            if (nCurrentRow >= nEndRow + 2)
            {
                    nEndRow ++ ;
                    m_pSlider->SetRange( Range( 0, nEndRow - nStartRow + 1) );
            }
            SetButtonState();
            FillCtrls();
            pDocSh->SetDocumentModified();
            pDocSh->PostPaintGridAll();
        }
    }
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_PrevHdl, Button*, void)
{
    if (pDoc)
    {
        if ( nCurrentRow > nStartRow +1 )
            nCurrentRow--;

        SetButtonState();
        FillCtrls();
    }
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_NextHdl, Button*, void)
{
    if (pDoc)
    {
        if ( nCurrentRow <= nEndRow)
            nCurrentRow++;

        SetButtonState();
        FillCtrls();
    }
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_RestoreHdl, Button*, void)
{
    if (pDoc)
    {
        FillCtrls();
    }
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_DeleteHdl, Button*, void)
{
    ScViewData& rViewData = pTabViewShell->GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    if (pDoc)
    {
        ScRange aRange(nStartCol, nCurrentRow, nTab, nEndCol, nCurrentRow, nTab);
        pDoc->DeleteRow(aRange);
        nEndRow--;

        SetButtonState();
        pDocSh->GetUndoManager()->Clear();

        FillCtrls();
        pDocSh->SetDocumentModified();
        pDocSh->PostPaintGridAll();
    }
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_CloseHdl, Button*, void)
{
    EndDialog( );
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_ScrollHdl, ScrollBar*, void)
{
    long nOffset = m_pSlider->GetThumbPos();
    nCurrentRow = nStartRow + nOffset + 1;
    SetButtonState();
    FillCtrls();
}

void ScDataFormDlg::SetButtonState()
{
    if (nCurrentRow > nEndRow)
    {
        m_pBtnDelete->Enable( false );
        m_pBtnNext->Enable( false );
    }
    else
    {
        m_pBtnDelete->Enable();
        m_pBtnNext->Enable();
    }

    if (nCurrentRow == nStartRow + 1)
        m_pBtnPrev->Enable( false );
    else
        m_pBtnPrev->Enable();

    m_pBtnRestore->Enable( false );
    if ( !maEdits.empty() && maEdits[0] != nullptr )
        maEdits[0]->GrabFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
