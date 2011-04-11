/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Novell Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Amelia Wang <amwang@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION

//------------------------------------------------------------------

#include "datafdlg.hxx"
#include "scresid.hxx"
#include "datafdlg.hrc"
#include "viewdata.hxx"
#include "docsh.hxx"
#include "refundo.hxx"
#include "undodat.hxx"

#include "rtl/ustrbuf.hxx"

#define HDL(hdl)            LINK( this, ScDataFormDlg, hdl )

using ::rtl::OUStringBuffer;

//zhangyun
ScDataFormDlg::ScDataFormDlg( Window* pParent, ScTabViewShell*  pTabViewShellOri) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DATAFORM ) ),
    aBtnNew          ( this, ScResId( BTN_DATAFORM_NEW ) ),
    aBtnDelete          ( this, ScResId( BTN_DATAFORM_DELETE ) ),
    aBtnRestore          ( this, ScResId( BTN_DATAFORM_RESTORE ) ),
    aBtnLast          ( this, ScResId( BTN_DATAFORM_LAST ) ),
    aBtnNext          ( this, ScResId( BTN_DATAFORM_NEXT ) ),
    aBtnClose          ( this, ScResId( BTN_DATAFORM_CLOSE ) ),
    aSlider         ( this, ScResId( WND_DATAFORM_SCROLLBAR ) ),
    aFixedText          ( this, ScResId( LAB_DATAFORM_RECORDNO ) )
{
    pTabViewShell = pTabViewShellOri;
    FreeResource();
    //read header form current document, and add new controls
    DBG_ASSERT( pTabViewShell, "pTabViewShell is NULL! :-/" );
    ScViewData* pViewData = pTabViewShell->GetViewData();

    pDoc = pViewData->GetDocument();
    if (pDoc)
    {
        ScRange aRange;
        pViewData->GetSimpleArea( aRange );
        ScAddress aStart = aRange.aStart;
        ScAddress aEnd = aRange.aEnd;

        nStartCol = aStart.Col();
        nEndCol = aEnd.Col();
        nStartRow   = aStart.Row();
        nEndRow = aEnd.Row();

        nTab = pViewData->GetTabNo();
        //if there is no selection
        if ((nStartCol == nEndCol) && (nStartRow == nEndRow))
            bNoSelection = true;

        if (bNoSelection)
        {
            //find last not blank cell in row
            for (int i=1;i<=MAX_DATAFORM_COLS;i++)
            {
                String  aColName;
                nEndCol++;
                pDoc->GetString( nEndCol, nStartRow, nTab, aColName );
                int nColWidth = pDoc->GetColWidth( nEndCol, nTab );
                if ( aColName.Len() == 0 && nColWidth)
                {
                    nEndCol--;
                    break;
                }
            }

            //find first not blank cell in row
            for (int i=1;i<=MAX_DATAFORM_COLS;i++)
            {
                String  aColName;
                if (nStartCol <= 0)
                    break;
                nStartCol--;

                pDoc->GetString( nStartCol, nStartRow, nTab, aColName );
                int nColWidth = pDoc->GetColWidth( nEndCol, nTab );
                if ( aColName.Len() == 0 && nColWidth)
                {
                    nStartCol++;
                    break;
                }
            }

            //skip leading hide column
            for (int i=1;i<=MAX_DATAFORM_COLS;i++)
            {
                String  aColName;
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
                String  aColName;
                nEndRow++;
                pDoc->GetString( nStartCol, nEndRow, nTab, aColName );
                if ( aColName.Len() == 0 )
                {
                    nEndRow--;
                    break;
                }
            }

            //find first not blank cell in row
            for (int i=1;i<=MAX_DATAFORM_ROWS;i++)
            {
                String  aColName;
                if (nStartRow <= 0)
                    break;
                nStartRow--;

                pDoc->GetString( nStartCol, nStartRow, nTab, aColName );
                if ( aColName.Len() == 0 )
                {
                    nStartRow++;
                    break;
                }
            }

            if (nEndRow < nStartRow)
                nEndRow = nStartRow;
        }

        aCurrentRow = nStartRow + 1;

        String  aFieldName;

        int nTop = 12;

        Size    nFixedSize(FIXED_WIDTH, CTRL_HEIGHT );
        Size    nEditSize(EDIT_WIDTH, CTRL_HEIGHT );

        aColLength = nEndCol - nStartCol + 1;

        //new the controls
        pFixedTexts = new FixedText*[aColLength];
        pEdits = new Edit*[aColLength];

        for(sal_uInt16 nIndex = 0; nIndex < aColLength; nIndex++)
        {
            pDoc->GetString( nIndex + nStartCol, nStartRow, nTab, aFieldName );
            int nColWidth = pDoc->GetColWidth( nIndex + nStartCol, nTab );
            if (nColWidth)
            {
                pFixedTexts[nIndex] = new FixedText(this);
                pEdits[nIndex] = new Edit(this, WB_BORDER);

                pFixedTexts[nIndex]->SetSizePixel(nFixedSize);
                pEdits[nIndex]->SetSizePixel(nEditSize);
                pFixedTexts[nIndex]->SetPosPixel(Point(FIXED_LEFT, nTop));
                pEdits[nIndex]->SetPosPixel(Point(EDIT_LEFT, nTop));
                pFixedTexts[nIndex]->SetText(aFieldName);
                pFixedTexts[nIndex]->Show();
                pEdits[nIndex]->Show();

                nTop += LINE_HEIGHT;
            }
            else
            {
                pFixedTexts[nIndex] = NULL;
                pEdits[nIndex] = NULL;
            }
        pEdits[nIndex]->SetModifyHdl( HDL(Impl_DataModifyHdl) );
        }

        Size nDialogSize = this->GetSizePixel();
        if (nTop > nDialogSize.Height())
        {
            nDialogSize.setHeight(nTop);
            this->SetSizePixel(nDialogSize);
        }
        Size nScrollSize = aSlider.GetSizePixel();
        nScrollSize.setHeight(nDialogSize.Height()-20);
        aSlider.SetSizePixel(nScrollSize);
    }

    FillCtrls(aCurrentRow);

    aSlider.SetPageSize( 10 );
    aSlider.SetVisibleSize( 1 );
    aSlider.SetLineSize( 1 );
    aSlider.SetRange( Range( 0, nEndRow - nStartRow + 1) );
    aSlider.Show();

    aBtnNew.SetClickHdl     ( HDL(Impl_NewHdl)    );
    aBtnLast.SetClickHdl    ( HDL(Impl_LastHdl)    );
    aBtnNext.SetClickHdl    ( HDL(Impl_NextHdl)    );

    aBtnRestore.SetClickHdl     ( HDL(Impl_RestoreHdl)    );
    aBtnDelete.SetClickHdl  ( HDL(Impl_DeleteHdl)    );
    aBtnClose.SetClickHdl   ( HDL(Impl_CloseHdl)    );

    aSlider.SetEndScrollHdl( HDL( Impl_ScrollHdl ) );

    SetButtonState();
}

ScDataFormDlg::~ScDataFormDlg()
{
    for(sal_uInt16 i = 0; i < aColLength; i++)
    {
        if (pEdits[i])
            delete pEdits[i];
        if (pFixedTexts[i])
            delete pFixedTexts[i];
    }
    if (pEdits)
        delete pEdits;
    if (pFixedTexts)
        delete pFixedTexts;
}

void ScDataFormDlg::FillCtrls(SCROW /*nCurrentRow*/)
{
    String  aFieldName;
    for (sal_uInt16 i = 0; i < aColLength; ++i)
    {
        if (pEdits[i])
        {
            if (aCurrentRow<=nEndRow)
            {
                pDoc->GetString( i + nStartCol, aCurrentRow, nTab, aFieldName );
                pEdits[i]->SetText(aFieldName);
            }
            else
                pEdits[i]->SetText(String());
        }
    }

    if (aCurrentRow <= nEndRow)
    {
        OUStringBuffer aBuf;
        aBuf.append(static_cast<sal_Int32>(aCurrentRow - nStartRow));
        aBuf.appendAscii(" / ");
        aBuf.append(static_cast<sal_Int32>(nEndRow - nStartRow));
        aFixedText.SetText(aBuf.makeStringAndClear());
    }
    else
        aFixedText.SetText(String(ScResId(STR_NEW_RECORD)));

    aSlider.SetThumbPos(aCurrentRow-nStartRow-1);
}

IMPL_LINK( ScDataFormDlg, Impl_DataModifyHdl, Edit*, pEdit)
{
    if ( pEdit->IsModified() )
        aBtnRestore.Enable( true );
    return 0;
}

IMPL_LINK( ScDataFormDlg, Impl_NewHdl, PushButton*, EMPTYARG )
{
    ScViewData* pViewData = pTabViewShell->GetViewData();
    ScDocShell* pDocSh = pViewData->GetDocShell();
    if ( pDoc )
    {
        sal_Bool bHasData = false;
        for(sal_uInt16 i = 0; i < aColLength; i++)
            if (pEdits[i])
                if ( pEdits[i]->GetText().Len() != 0 )
                {
                    bHasData = sal_True;
                    break;
                }

        if ( bHasData )
        {
            pTabViewShell->DataFormPutData( aCurrentRow , nStartRow , nStartCol , nEndRow , nEndCol , pEdits , aColLength );
            aCurrentRow++;
            if (aCurrentRow >= nEndRow + 2)
            {
                    nEndRow ++ ;
                    aSlider.SetRange( Range( 0, nEndRow - nStartRow + 1) );
            }
            SetButtonState();
            FillCtrls(aCurrentRow);
            pDocSh->SetDocumentModified();
            pDocSh->PostPaintGridAll();
            }
    }
    return 0;
}

IMPL_LINK( ScDataFormDlg, Impl_LastHdl, PushButton*, EMPTYARG )
{
    if (pDoc)
    {
        if ( aCurrentRow > nStartRow +1 )
            aCurrentRow--;

        SetButtonState();
        FillCtrls(aCurrentRow);
    }
    return 0;
}

IMPL_LINK( ScDataFormDlg, Impl_NextHdl, PushButton*, EMPTYARG )
{
    if (pDoc)
    {
        if ( aCurrentRow <= nEndRow)
            aCurrentRow++;

        SetButtonState();
        FillCtrls(aCurrentRow);
    }
    return 0;
}

IMPL_LINK( ScDataFormDlg, Impl_RestoreHdl, PushButton*, EMPTYARG )
{
    if (pDoc)
    {
        FillCtrls(aCurrentRow);
    }
    return 0;
}

IMPL_LINK( ScDataFormDlg, Impl_DeleteHdl, PushButton*, EMPTYARG )
{
    ScViewData* pViewData = pTabViewShell->GetViewData();
    ScDocShell* pDocSh = pViewData->GetDocShell();
    if (pDoc)
    {
        ScRange aRange(nStartCol, aCurrentRow, nTab, nEndCol, aCurrentRow, nTab);
        pDoc->DeleteRow(aRange);
        nEndRow--;

        SetButtonState();
        pDocSh->GetUndoManager()->Clear();

        FillCtrls(aCurrentRow);
        pDocSh->SetDocumentModified();
        pDocSh->PostPaintGridAll();
    }
    return 0;
}

IMPL_LINK( ScDataFormDlg, Impl_CloseHdl, PushButton*, EMPTYARG )
{
    EndDialog( );
    return 0;
}

IMPL_LINK( ScDataFormDlg, Impl_ScrollHdl, ScrollBar*, EMPTYARG )
{
    long nOffset = aSlider.GetThumbPos();
    aCurrentRow = nStartRow + nOffset + 1;
    SetButtonState();
    FillCtrls(aCurrentRow);
    return 0;
}

void ScDataFormDlg::SetButtonState()
{
    if ( aCurrentRow > nEndRow )
    {
        aBtnDelete.Enable( false );
        aBtnLast.Enable( true );
        aBtnNext.Enable( false );
    }
    else
    {
        aBtnDelete.Enable( true );
        aBtnNext.Enable( true );
    }
    if ( 1 == aCurrentRow )
        aBtnLast.Enable( false );

    aBtnRestore.Enable( false );
    if ( pEdits )
        pEdits[0]->GrabFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
