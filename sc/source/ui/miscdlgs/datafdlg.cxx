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


//zhangyun
ScDataFormDlg::ScDataFormDlg( Window* pParent, ScTabViewShell*  pTabViewShellOri) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DATAFORM ) ),
    aBtnNew          ( this, ScResId( BTN_DATAFORM_NEW ) ),
    aBtnDelete          ( this, ScResId( BTN_DATAFORM_DELETE ) ),
    aBtnRestore          ( this, ScResId( BTN_DATAFORM_RESTORE ) ),
    aBtnPrev          ( this, ScResId( BTN_DATAFORM_PREV ) ),
    aBtnNext          ( this, ScResId( BTN_DATAFORM_NEXT ) ),
    aBtnClose          ( this, ScResId( BTN_DATAFORM_CLOSE ) ),
    aSlider         ( this, ScResId( WND_DATAFORM_SCROLLBAR ) ),
    aFixedText          ( this, ScResId( LAB_DATAFORM_RECORDNO ) ),
    sNewRecord(SC_RESSTR(STR_NEW_RECORD))
{
    pTabViewShell = pTabViewShellOri;
    FreeResource();
    //read header form current document, and add new controls
    OSL_ENSURE( pTabViewShell, "pTabViewShell is NULL! :-/" );
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

        String  aFieldName;

        //align with LAB_DATAFORM_RECORDNO
        int nTop = LogicToPixel( Size(1,6), MapMode(MAP_APPFONT) ).getHeight();
        const int nOne = LogicToPixel( Size(1,1), MapMode(MAP_APPFONT) ).getHeight();
        const int nLineHeight = LogicToPixel( Size(1, LINE_HEIGHT), MapMode(MAP_APPFONT) ).getHeight();
        const int nFixedLeft = LogicToPixel( Size(FIXED_LEFT, 1), MapMode(MAP_APPFONT) ).getWidth();
        const int nEditLeft = LogicToPixel( Size(EDIT_LEFT, 1), MapMode(MAP_APPFONT) ).getWidth();

        Size    nFixedSize(LogicToPixel( Size(FIXED_WIDTH, FIXED_HEIGHT), MapMode(MAP_APPFONT) ));
        Size    nEditSize(LogicToPixel( Size(EDIT_WIDTH, EDIT_HEIGHT), MapMode(MAP_APPFONT) ));

        aColLength = nEndCol - nStartCol + 1;

        //new the controls
        maFixedTexts.reserve(aColLength);
        maEdits.reserve(aColLength);

        for(sal_uInt16 nIndex = 0; nIndex < aColLength; nIndex++)
        {
            aFieldName = pDoc->GetString(nIndex + nStartCol, nStartRow, nTab);
            int nColWidth = pDoc->GetColWidth( nIndex + nStartCol, nTab );
            if (nColWidth)
            {
                maFixedTexts.push_back( new FixedText(this) );
                maEdits.push_back( new Edit(this, WB_BORDER) );

                maFixedTexts[nIndex].SetSizePixel(nFixedSize);
                maEdits[nIndex].SetSizePixel(nEditSize);
                maFixedTexts[nIndex].SetPosPixel(Point(nFixedLeft, nTop + nOne));
                maEdits[nIndex].SetPosPixel(Point(nEditLeft, nTop));
                maFixedTexts[nIndex].SetText(aFieldName);
                maFixedTexts[nIndex].Show();
                maEdits[nIndex].Show();

                nTop += nLineHeight;
            }
            else
            {
                maFixedTexts.push_back( NULL );
                maEdits.push_back( NULL );
            }
            if (!maEdits.is_null(nIndex))
                maEdits[nIndex].SetModifyHdl( HDL(Impl_DataModifyHdl) );
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

    FillCtrls(nCurrentRow);

    aSlider.SetPageSize( 10 );
    aSlider.SetVisibleSize( 1 );
    aSlider.SetLineSize( 1 );
    aSlider.SetRange( Range( 0, nEndRow - nStartRow + 1) );
    aSlider.Show();

    aBtnNew.SetClickHdl     ( HDL(Impl_NewHdl)    );
    aBtnPrev.SetClickHdl    ( HDL(Impl_PrevHdl)    );
    aBtnNext.SetClickHdl    ( HDL(Impl_NextHdl)    );

    aBtnRestore.SetClickHdl     ( HDL(Impl_RestoreHdl)    );
    aBtnDelete.SetClickHdl  ( HDL(Impl_DeleteHdl)    );
    aBtnClose.SetClickHdl   ( HDL(Impl_CloseHdl)    );

    aSlider.SetEndScrollHdl( HDL( Impl_ScrollHdl ) );

    SetButtonState();
}

ScDataFormDlg::~ScDataFormDlg()
{

}

void ScDataFormDlg::FillCtrls(SCROW /*nCurrentRow*/)
{
    String  aFieldName;
    for (sal_uInt16 i = 0; i < aColLength; ++i)
    {
        if (!maEdits.is_null(i))
        {
            if (nCurrentRow<=nEndRow)
            {
                aFieldName = pDoc->GetString(i + nStartCol, nCurrentRow, nTab);
                maEdits[i].SetText(aFieldName);
            }
            else
                maEdits[i].SetText(String());
        }
    }

    if (nCurrentRow <= nEndRow)
    {
        OUStringBuffer aBuf;
        aBuf.append(static_cast<sal_Int32>(nCurrentRow - nStartRow));
        aBuf.appendAscii(" / ");
        aBuf.append(static_cast<sal_Int32>(nEndRow - nStartRow));
        aFixedText.SetText(aBuf.makeStringAndClear());
    }
    else
        aFixedText.SetText(sNewRecord);

    aSlider.SetThumbPos(nCurrentRow-nStartRow-1);
}

IMPL_LINK( ScDataFormDlg, Impl_DataModifyHdl, Edit*, pEdit)
{
    if ( pEdit->IsModified() )
        aBtnRestore.Enable( true );
    return 0;
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_NewHdl)
{
    ScViewData* pViewData = pTabViewShell->GetViewData();
    ScDocShell* pDocSh = pViewData->GetDocShell();
    if ( pDoc )
    {
        bool bHasData = false;
        boost::ptr_vector<Edit>::iterator itr = maEdits.begin(), itrEnd = maEdits.end();
        for(; itr != itrEnd; ++itr)
            if (!boost::is_null(itr))
                if ( !(*itr).GetText().isEmpty() )
                {
                    bHasData = true;
                    break;
                }

        if ( bHasData )
        {
            pTabViewShell->DataFormPutData( nCurrentRow , nStartRow , nStartCol , nEndRow , nEndCol , maEdits , aColLength );
            nCurrentRow++;
            if (nCurrentRow >= nEndRow + 2)
            {
                    nEndRow ++ ;
                    aSlider.SetRange( Range( 0, nEndRow - nStartRow + 1) );
            }
            SetButtonState();
            FillCtrls(nCurrentRow);
            pDocSh->SetDocumentModified();
            pDocSh->PostPaintGridAll();
            }
    }
    return 0;
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_PrevHdl)
{
    if (pDoc)
    {
        if ( nCurrentRow > nStartRow +1 )
            nCurrentRow--;

        SetButtonState();
        FillCtrls(nCurrentRow);
    }
    return 0;
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_NextHdl)
{
    if (pDoc)
    {
        if ( nCurrentRow <= nEndRow)
            nCurrentRow++;

        SetButtonState();
        FillCtrls(nCurrentRow);
    }
    return 0;
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_RestoreHdl)
{
    if (pDoc)
    {
        FillCtrls(nCurrentRow);
    }
    return 0;
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_DeleteHdl)
{
    ScViewData* pViewData = pTabViewShell->GetViewData();
    ScDocShell* pDocSh = pViewData->GetDocShell();
    if (pDoc)
    {
        ScRange aRange(nStartCol, nCurrentRow, nTab, nEndCol, nCurrentRow, nTab);
        pDoc->DeleteRow(aRange);
        nEndRow--;

        SetButtonState();
        pDocSh->GetUndoManager()->Clear();

        FillCtrls(nCurrentRow);
        pDocSh->SetDocumentModified();
        pDocSh->PostPaintGridAll();
    }
    return 0;
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_CloseHdl)
{
    EndDialog( );
    return 0;
}

IMPL_LINK_NOARG(ScDataFormDlg, Impl_ScrollHdl)
{
    long nOffset = aSlider.GetThumbPos();
    nCurrentRow = nStartRow + nOffset + 1;
    SetButtonState();
    FillCtrls(nCurrentRow);
    return 0;
}

void ScDataFormDlg::SetButtonState()
{
    if (nCurrentRow > nEndRow)
    {
        aBtnDelete.Enable( false );
        aBtnNext.Enable( false );
    }
    else
    {
        aBtnDelete.Enable( true );
        aBtnNext.Enable( true );
    }

    if (nCurrentRow == nStartRow + 1)
        aBtnPrev.Enable( false );
    else
        aBtnPrev.Enable( true );

    aBtnRestore.Enable( false );
    if ( maEdits.size()>=1 && !maEdits.is_null(0) )
        maEdits[0].GrabFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
