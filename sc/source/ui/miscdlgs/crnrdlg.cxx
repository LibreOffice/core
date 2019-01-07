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

#include <reffact.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <docsh.hxx>
#include <crnrdlg.hxx>
#include <vcl/weld.hxx>
#include <vcl/fixed.hxx>
#include <memory>

namespace
{
    void ERRORBOX(weld::Window* pParent, const OUString& rString)
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  rString));
        xBox->run();
    }

    int QUERYBOX(weld::Window* pParent, const OUString& rString)
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Question, VclButtonsType::YesNo,
                                                  rString));
        xBox->set_default_response(RET_YES);
        return xBox->run();
    }

}

const sal_uLong nEntryDataCol = 0;
const sal_uLong nEntryDataRow = 1;
const sal_uLong nEntryDataDelim = 2;

//  class ScColRowNameRangesDlg

// note: some of the initialisation is done in Init
ScColRowNameRangesDlg::ScColRowNameRangesDlg( SfxBindings* pB,
                                SfxChildWindow* pCW,
                                vcl::Window* pParent,
                                ScViewData* ptrViewData )

    :   ScAnyRefDlg ( pB, pCW, pParent, "NameRangesDialog" , "modules/scalc/ui/namerangesdialog.ui" ),

        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData->GetDocument() ),

        pEdActive       ( nullptr ),
        bDlgLostFocus   ( false )
{
    get(pLbRange,"range");

    get(pEdAssign,"edassign");
    get(pRbAssign,"rbassign");
    pRbAssign->SetReferences(this, pEdAssign);
    pEdAssign->SetReferences(this, get<VclFrame>("rangeframe")->get_label_widget());
    get(pBtnColHead,"colhead");
    get(pBtnRowHead,"rowhead");
    get(pEdAssign2,"edassign2");
    get(pRbAssign2,"rbassign2");
    pRbAssign2->SetReferences(this, pEdAssign2);
    pEdAssign2->SetReferences(this, get<FixedText>("datarange"));

    get(pBtnOk,"ok");
    get(pBtnCancel,"cancel");
    get(pBtnAdd,"add");
    get(pBtnRemove,"delete");

    xColNameRanges = pDoc->GetColNameRanges()->Clone();
    xRowNameRanges = pDoc->GetRowNameRanges()->Clone();
    Init();
}

ScColRowNameRangesDlg::~ScColRowNameRangesDlg()
{
    disposeOnce();
}

void ScColRowNameRangesDlg::dispose()
{
    pLbRange.clear();
    pEdAssign.clear();
    pRbAssign.clear();
    pBtnColHead.clear();
    pBtnRowHead.clear();
    pEdAssign2.clear();
    pRbAssign2.clear();
    pBtnOk.clear();
    pBtnCancel.clear();
    pBtnAdd.clear();
    pBtnRemove.clear();
    pEdActive.clear();
    ScAnyRefDlg::dispose();
}

// initialises event handlers and start parameters in the dialog
void ScColRowNameRangesDlg::Init()
{
    pBtnOk->SetClickHdl      ( LINK( this, ScColRowNameRangesDlg, OkBtnHdl ) );
    pBtnCancel->SetClickHdl  ( LINK( this, ScColRowNameRangesDlg, CancelBtnHdl ) );
    pBtnAdd->SetClickHdl     ( LINK( this, ScColRowNameRangesDlg, AddBtnHdl ) );
    pBtnRemove->SetClickHdl  ( LINK( this, ScColRowNameRangesDlg, RemoveBtnHdl ) );
    pLbRange->SetSelectHdl   ( LINK( this, ScColRowNameRangesDlg, Range1SelectHdl ) );
    pEdAssign->SetModifyHdl  ( LINK( this, ScColRowNameRangesDlg, Range1DataModifyHdl ) );
    pBtnColHead->SetClickHdl ( LINK( this, ScColRowNameRangesDlg, ColClickHdl ) );
    pBtnRowHead->SetClickHdl ( LINK( this, ScColRowNameRangesDlg, RowClickHdl ) );
    pEdAssign2->SetModifyHdl ( LINK( this, ScColRowNameRangesDlg, Range2DataModifyHdl ) );

    Link<Control&,void> aLink = LINK( this, ScColRowNameRangesDlg, GetFocusHdl );
    pEdAssign->SetGetFocusHdl( aLink );
    pRbAssign->SetGetFocusHdl( aLink );
    pEdAssign2->SetGetFocusHdl( aLink );
    pRbAssign2->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScColRowNameRangesDlg, LoseFocusHdl );
    pEdAssign->SetLoseFocusHdl( aLink );
    pRbAssign->SetLoseFocusHdl( aLink );
    pEdAssign2->SetLoseFocusHdl( aLink );
    pRbAssign2->SetLoseFocusHdl( aLink );

    pLbRange->SetDropDownLineCount(10);

    pEdActive = pEdAssign;

    UpdateNames();

    if (pViewData)
    {
        SCCOL nStartCol = 0;
        SCROW nStartRow = 0;
        SCTAB nStartTab = 0;
        SCCOL nEndCol   = 0;
        SCROW nEndRow   = 0;
        SCTAB nEndTab   = 0;
        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,  nEndTab );
        SetColRowData( ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab));
    }
    else
    {
        pBtnColHead->Check();
        pBtnRowHead->Check( false );
        pEdAssign->SetText( EMPTY_OUSTRING );
        pEdAssign2->SetText( EMPTY_OUSTRING );
    }

    pLbRange->SetBorderStyle( WindowBorderStyle::MONO );
    pBtnColHead->Enable();
    pBtnRowHead->Enable();
    pEdAssign->Enable();
    pEdAssign->GrabFocus();
    pRbAssign->Enable();

    Range1SelectHdl( *pLbRange );
}

// set data range of a labeled range to default values and set the
// form elements for the reference
void ScColRowNameRangesDlg::SetColRowData( const ScRange& rLabelRange, bool bRef)
{
    theCurData = theCurArea = rLabelRange;
    bool bValid = true;
    SCCOL nCol1 = theCurArea.aStart.Col();
    SCCOL nCol2 = theCurArea.aEnd.Col();
    SCROW nRow1 = theCurArea.aStart.Row();
    SCROW nRow2 = theCurArea.aEnd.Row();
    if ( (static_cast<SCCOLROW>(nCol2 - nCol1) >= nRow2 - nRow1) || (nCol1 == 0 && nCol2 == MAXCOL) )
    {   // Column headers and the limiting case of the whole sheet
        pBtnColHead->Check();
        pBtnRowHead->Check( false );
        if ( nRow2 == MAXROW  )
        {
            if ( nRow1 == 0 )
                bValid = false;     // limiting case of the whole sheet
            else
            {   // Header at bottom, data above
                theCurData.aStart.SetRow( 0 );
                theCurData.aEnd.SetRow( nRow1 - 1 );
            }
        }
        else
        {   // Header at top, data below
            theCurData.aStart.SetRow( nRow2 + 1 );
            theCurData.aEnd.SetRow( MAXROW );
        }
    }
    else
    {   // Column headers
        pBtnRowHead->Check();
        pBtnColHead->Check( false );
        if ( nCol2 == MAXCOL )
        {   // Header at the right, data to the left
            theCurData.aStart.SetCol( 0 );
            theCurData.aEnd.SetCol( nCol2 - 1 );
        }
        else
        {   // Header at the left, data to the right
            theCurData.aStart.SetCol( nCol2 + 1 );
            theCurData.aEnd.SetCol( MAXCOL );
        }
    }
    if ( bValid )
    {
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        OUString aStr(theCurArea.Format(ScRefFlags::RANGE_ABS_3D, pDoc, eConv));

        if(bRef)
            pEdAssign->SetRefString( aStr );
        else
            pEdAssign->SetText( aStr );

        pEdAssign->SetSelection( Selection( SELECTION_MAX, SELECTION_MAX ) );
        aStr = theCurData.Format(ScRefFlags::RANGE_ABS_3D, pDoc, eConv);

        if(bRef)
            pEdAssign2->SetRefString( aStr );
        else
            pEdAssign2->SetText( aStr );
    }
    else
    {
        theCurData = theCurArea = ScRange();

        if(bRef)
        {
            pEdAssign->SetRefString( EMPTY_OUSTRING );
            pEdAssign2->SetRefString( EMPTY_OUSTRING );
        }
        else
        {
            pEdAssign->SetText( EMPTY_OUSTRING );
            pEdAssign2->SetText( EMPTY_OUSTRING );
        }

        pBtnColHead->Disable();
        pBtnRowHead->Disable();
        pEdAssign2->Disable();
        pRbAssign2->Disable();
    }
}

// adjust label range and set the data reference form element
void ScColRowNameRangesDlg::AdjustColRowData( const ScRange& rDataRange, bool bRef)
{
    theCurData = rDataRange;
    if ( pBtnColHead->IsChecked() )
    {   // Data range is the same columns as the header
        theCurData.aStart.SetCol( theCurArea.aStart.Col() );
        theCurData.aEnd.SetCol( theCurArea.aEnd.Col() );
        if ( theCurData.Intersects( theCurArea ) )
        {
            SCROW nRow1 = theCurArea.aStart.Row();
            SCROW nRow2 = theCurArea.aEnd.Row();
            if ( nRow1 > 0
              && (theCurData.aEnd.Row() < nRow2 || nRow2 == MAXROW) )
            {   // Data above header
                theCurData.aEnd.SetRow( nRow1 - 1 );
                if ( theCurData.aStart.Row() > theCurData.aEnd.Row() )
                    theCurData.aStart.SetRow( theCurData.aEnd.Row() );
            }
            else
            {   // Data below header
                theCurData.aStart.SetRow( nRow2 + 1 );
                if ( theCurData.aStart.Row() > theCurData.aEnd.Row() )
                    theCurData.aEnd.SetRow( theCurData.aStart.Row() );
            }
        }
    }
    else
    {   // Data range in the same rows as header
        theCurData.aStart.SetRow( theCurArea.aStart.Row() );
        theCurData.aEnd.SetRow( theCurArea.aEnd.Row() );
        if ( theCurData.Intersects( theCurArea ) )
        {
            SCCOL nCol1 = theCurArea.aStart.Col();
            SCCOL nCol2 = theCurArea.aEnd.Col();
            if ( nCol1 > 0
              && (theCurData.aEnd.Col() < nCol2 || nCol2 == MAXCOL) )
            {   // Data left of header
                theCurData.aEnd.SetCol( nCol1 - 1 );
                if ( theCurData.aStart.Col() > theCurData.aEnd.Col() )
                    theCurData.aStart.SetCol( theCurData.aEnd.Col() );
            }
            else
            {   // Data right of header
                theCurData.aStart.SetCol( nCol2 + 1 );
                if ( theCurData.aStart.Col() > theCurData.aEnd.Col() )
                    theCurData.aEnd.SetCol( theCurData.aStart.Col() );
            }
        }
    }
    OUString aStr(theCurData.Format(ScRefFlags::RANGE_ABS_3D, pDoc, pDoc->GetAddressConvention()));

    if(bRef)
        pEdAssign2->SetRefString( aStr );
    else
        pEdAssign2->SetText( aStr );

    pEdAssign2->SetSelection( Selection( SELECTION_MAX, SELECTION_MAX ) );
}

// Set the reference to a cell range selected with the mouse and update
// the selection form element
void ScColRowNameRangesDlg::SetReference( const ScRange& rRef, ScDocument* /* pDoc */ )
{
    if ( pEdActive )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pEdActive );

        if ( pEdActive == pEdAssign )
            SetColRowData( rRef, true );
        else
            AdjustColRowData( rRef, true );
        pBtnColHead->Enable();
        pBtnRowHead->Enable();
        pBtnAdd->Enable();
        pBtnRemove->Disable();
    }
}

bool ScColRowNameRangesDlg::Close()
{
    return DoClose( ScColRowNameRangesDlgWrapper::GetChildWindowId() );
}

void ScColRowNameRangesDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;
        if( pEdActive )
            pEdActive->GrabFocus();
    }
    else
        GrabFocus();

    if( pEdActive == pEdAssign )
        Range1DataModifyHdl( *pEdAssign );
    else if( pEdActive == pEdAssign2 )
        Range2DataModifyHdl( *pEdAssign2 );

    RefInputDone();
}

void ScColRowNameRangesDlg::UpdateNames()
{
    pLbRange->SetUpdateMode( false );

    pLbRange->Clear();
    aRangeMap.clear();
    pEdAssign->SetText( EMPTY_OUSTRING );

    size_t nCount, j;

    SCCOL nCol1;
    SCROW nRow1;    //Extension for range names
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    OUString rString;
    const ScAddress::Details aDetails(pDoc->GetAddressConvention());

    OUString aString;
    OUString strDelim(" --- ");
    aString = strDelim;
    aString += ScResId( STR_COLUMN );
    aString += strDelim;
    sal_Int32 nPos = pLbRange->InsertEntry( aString );
    pLbRange->SetEntryData( nPos, reinterpret_cast<void*>(nEntryDataDelim) );
    if ( (nCount = xColNameRanges->size()) > 0 )
    {
        std::vector<const ScRangePair*> aSortArray(xColNameRanges->CreateNameSortedArray(
               pDoc ));
        nCount = aSortArray.size();
        for ( j=0; j < nCount; j++ )
        {
            const ScRange aRange(aSortArray[j]->GetRange(0));
            aString = aRange.Format(ScRefFlags::RANGE_ABS_3D, pDoc, aDetails);

            //@008 get range parameters from document
            aSortArray[j]->GetRange(0).GetVars( nCol1, nRow1, nTab1,
                                            nCol2, nRow2, nTab2 );
            SCCOL q=nCol1+3;
            if(q>nCol2) q=nCol2;
            //@008 construct string
            OUStringBuffer strShow = " [";
            rString = pDoc->GetString(nCol1, nRow1, nTab1);
            strShow.append(rString);
            for(SCCOL i=nCol1+1;i<=q;i++)
            {
                strShow.append(", ");
                rString = pDoc->GetString(i, nRow1, nTab1);
                strShow.append(rString);
            }
            if(q<nCol2) // Too long? Add ",..."
            {
                strShow.append(", ...");
            }
            strShow.append("]");

            //@008 Add string to listbox
            OUString aInsStr = aString + strShow.makeStringAndClear();
            nPos = pLbRange->InsertEntry( aInsStr );
            aRangeMap.emplace( aInsStr, aRange );
            pLbRange->SetEntryData( nPos, reinterpret_cast<void*>(nEntryDataCol) );
        }
    }
    aString = strDelim;
    aString += ScResId( STR_ROW );
    aString += strDelim;
    nPos = pLbRange->InsertEntry( aString );
    pLbRange->SetEntryData( nPos, reinterpret_cast<void*>(nEntryDataDelim) );
    if ( (nCount = xRowNameRanges->size()) > 0 )
    {
        std::vector<const ScRangePair*> aSortArray(xRowNameRanges->CreateNameSortedArray(
               pDoc ));
        nCount = aSortArray.size();
        for ( j=0; j < nCount; j++ )
        {
            const ScRange aRange(aSortArray[j]->GetRange(0));
            aString = aRange.Format(ScRefFlags::RANGE_ABS_3D, pDoc, aDetails);

            //@008 Build string for rows below
            aSortArray[j]->GetRange(0).GetVars( nCol1, nRow1, nTab1,
                                            nCol2, nRow2, nTab2 );
            SCROW q=nRow1+3;
            if(q>nRow2) q=nRow2;
            OUStringBuffer strShow = " [";
            rString = pDoc->GetString(nCol1, nRow1, nTab1);
            strShow.append(rString);
            for(SCROW i=nRow1+1;i<=q;i++)
            {
                strShow.append(", ");
                rString = pDoc->GetString(nCol1, i, nTab1);
                strShow.append(rString);
            }
            if(q<nRow2)
            {
                strShow.append(", ...");
            }
            strShow.append("]");

            OUString aInsStr = aString + strShow.makeStringAndClear();
            nPos = pLbRange->InsertEntry( aInsStr );
            aRangeMap.emplace( aInsStr, aRange );
            pLbRange->SetEntryData( nPos, reinterpret_cast<void*>(nEntryDataRow) );
        }
    }

    pLbRange->SetUpdateMode( true );
    pLbRange->Invalidate();
}

void ScColRowNameRangesDlg::UpdateRangeData( const ScRange& rRange, bool bColName )
{
    ScRangePair* pPair = nullptr;
    bool bFound = false;
    if ( bColName && (pPair = xColNameRanges->Find( rRange )) != nullptr )
        bFound = true;
    else if ( !bColName && (pPair = xRowNameRanges->Find( rRange )) != nullptr )
        bFound = true;

    if ( bFound )
    {
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        theCurArea = rRange;
        OUString aStr(theCurArea.Format(ScRefFlags::RANGE_ABS_3D, pDoc, eConv));
        pEdAssign->SetText( aStr );
        pBtnAdd->Disable();
        pBtnRemove->Enable();
        pBtnColHead->Check( bColName );
        pBtnRowHead->Check( !bColName );
        theCurData = pPair->GetRange(1);
        aStr = theCurData.Format(ScRefFlags::RANGE_ABS_3D, pDoc, eConv);
        pEdAssign2->SetText( aStr );
    }
    else
    {
        pBtnAdd->Enable();
        pBtnRemove->Disable();
    }
    pBtnColHead->Enable();
    pBtnRowHead->Enable();
    pEdAssign2->Enable();
    pRbAssign2->Enable();
}

bool ScColRowNameRangesDlg::IsRefInputMode() const
{
    return (pEdActive != nullptr);
}

// Handler:

// handler called when OK is clicked, calls the add button handler before
// passing the range lists to the document
IMPL_LINK_NOARG(ScColRowNameRangesDlg, OkBtnHdl, Button*, void)
{
    AddBtnHdl( nullptr );

    // assign RangeLists to the references in the document
    pDoc->GetColNameRangesRef() = xColNameRanges;
    pDoc->GetRowNameRangesRef() = xRowNameRanges;
    // changed ranges need to take effect
    pDoc->CompileColRowNameFormula();
    ScDocShell* pDocShell = pViewData->GetDocShell();
    pDocShell->PostPaint(ScRange(0, 0, 0, MAXCOL, MAXROW, MAXTAB), PaintPartFlags::Grid);
    pDocShell->SetDocumentModified();

    Close();
}

IMPL_LINK_NOARG(ScColRowNameRangesDlg, CancelBtnHdl, Button*, void)
{
    Close();
}

// handler called when add button clicked: set ranges and add to listbox
IMPL_LINK_NOARG(ScColRowNameRangesDlg, AddBtnHdl, Button*, void)
{
    OUString aNewArea( pEdAssign->GetText() );
    OUString aNewData( pEdAssign2->GetText() );

    if ( !aNewArea.isEmpty() && !aNewData.isEmpty() )
    {
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        ScRange aRange1, aRange2;
        bool bOk1 = (aRange1.ParseAny( aNewArea, pDoc, eConv ) & ScRefFlags::VALID) == ScRefFlags::VALID;
        if ( bOk1 && (aRange2.ParseAny( aNewData, pDoc, eConv ) & ScRefFlags::VALID) == ScRefFlags::VALID)
        {
            theCurArea = aRange1;
            AdjustColRowData( aRange2 );
            ScRangePair* pPair;
            if ( ( pPair = xColNameRanges->Find( theCurArea ) ) != nullptr )
            {
                xColNameRanges->Remove( *pPair );
            }
            if ( ( pPair = xRowNameRanges->Find( theCurArea ) ) != nullptr )
            {
                xRowNameRanges->Remove( *pPair );
            }
            if ( pBtnColHead->IsChecked() )
                xColNameRanges->Join( ScRangePair( theCurArea, theCurData ) );
            else
                xRowNameRanges->Join( ScRangePair( theCurArea, theCurData ) );

            UpdateNames();

            pEdAssign->GrabFocus();
            pBtnAdd->Disable();
            pBtnRemove->Disable();
            pEdAssign->SetText( EMPTY_OUSTRING );
            pBtnColHead->Check();
            pBtnRowHead->Check( false );
            pEdAssign2->SetText( EMPTY_OUSTRING );
            theCurArea = ScRange();
            theCurData = theCurArea;
            Range1SelectHdl( *pLbRange );
        }
        else
        {
            ERRORBOX(GetFrameWeld(), ScResId(STR_INVALIDTABNAME));
            if ( !bOk1 )
                pEdAssign->GrabFocus();
            else
                pEdAssign2->GrabFocus();
        }
    }
}

IMPL_LINK_NOARG(ScColRowNameRangesDlg, RemoveBtnHdl, Button*, void)
{
    OUString aRangeStr = pLbRange->GetSelectedEntry();
    sal_Int32 nSelectPos = pLbRange->GetSelectedEntryPos();
    bool bColName =
        (reinterpret_cast<sal_uLong>(pLbRange->GetEntryData( nSelectPos )) == nEntryDataCol);
    NameRangeMap::const_iterator itr = aRangeMap.find(aRangeStr);
    if (itr == aRangeMap.end())
        return;
    const ScRange& rRange = itr->second;

    ScRangePair* pPair = nullptr;
    bool bFound = false;
    if ( bColName && (pPair = xColNameRanges->Find( rRange )) != nullptr )
        bFound = true;
    else if ( !bColName && (pPair = xRowNameRanges->Find( rRange )) != nullptr )
        bFound = true;
    if ( bFound )
    {
        OUString aStrDelMsg = ScResId( STR_QUERY_DELENTRY );
        OUString aMsg       = aStrDelMsg.getToken( 0, '#' )
                            + aRangeStr
                            + aStrDelMsg.getToken( 1, '#' );

        if (RET_YES == QUERYBOX(GetFrameWeld(), aMsg))
        {
            if ( bColName )
                xColNameRanges->Remove( *pPair );
            else
                xRowNameRanges->Remove( *pPair );

            UpdateNames();
            const sal_Int32 nCnt = pLbRange->GetEntryCount();
            if ( nSelectPos >= nCnt )
            {
                if ( nCnt )
                    nSelectPos = nCnt - 1;
                else
                    nSelectPos = 0;
            }
            pLbRange->SelectEntryPos( nSelectPos );
            if ( nSelectPos &&
                    reinterpret_cast<sal_uLong>(pLbRange->GetEntryData( nSelectPos )) == nEntryDataDelim )
                pLbRange->SelectEntryPos( --nSelectPos );    // ---Row---

            pLbRange->GrabFocus();
            pBtnAdd->Disable();
            pBtnRemove->Disable();
            pEdAssign->SetText( EMPTY_OUSTRING );
            theCurArea = theCurData = ScRange();
            pBtnColHead->Check();
            pBtnRowHead->Check( false );
            pEdAssign2->SetText( EMPTY_OUSTRING );
            Range1SelectHdl( *pLbRange );
        }
    }
}

// handler called when a row in the listbox is selected, updates form input fields
IMPL_LINK_NOARG(ScColRowNameRangesDlg, Range1SelectHdl, ListBox&, void)
{
    sal_Int32 nSelectPos = pLbRange->GetSelectedEntryPos();
    const sal_Int32 nCnt = pLbRange->GetEntryCount();
    sal_uInt16 nMoves = 0;
    while ( nSelectPos < nCnt
            && reinterpret_cast<sal_uLong>(pLbRange->GetEntryData( nSelectPos )) == nEntryDataDelim )
    {   // skip Delimiter
        ++nMoves;
        pLbRange->SelectEntryPos( ++nSelectPos );
    }
    OUString aRangeStr = pLbRange->GetSelectedEntry();
    if ( nMoves )
    {
        if ( nSelectPos > 1 && nSelectPos >= nCnt )
        {   // if entries exist before the " --- Row --- " Delimiter then
            // do not stop at the delimiter
            nSelectPos = nCnt - 2;
            pLbRange->SelectEntryPos( nSelectPos );
            aRangeStr = pLbRange->GetSelectedEntry();
        }
        else if ( nSelectPos > 2 && nSelectPos < nCnt && !aRangeStr.isEmpty()
                  && aRangeStr == pEdAssign->GetText() )
        {   // move upwards instead of below to the previous position
            nSelectPos -= 2;
            pLbRange->SelectEntryPos( nSelectPos );
            aRangeStr = pLbRange->GetSelectedEntry();
        }
    }
    NameRangeMap::const_iterator itr = aRangeMap.find(aRangeStr);
    if ( itr != aRangeMap.end() )
    {
        bool bColName =
            (reinterpret_cast<sal_uLong>(pLbRange->GetEntryData( nSelectPos )) == nEntryDataCol);
        UpdateRangeData( itr->second, bColName );
        pBtnAdd->Disable();
        pBtnRemove->Enable();
    }
    else
    {
        if ( !pEdAssign->GetText().isEmpty() )
        {
            if ( !pEdAssign2->GetText().isEmpty() )
                pBtnAdd->Enable();
            else
                pBtnAdd->Disable();
            pBtnColHead->Enable();
            pBtnRowHead->Enable();
            pEdAssign2->Enable();
            pRbAssign2->Enable();
        }
        else
        {
            pBtnAdd->Disable();
            pBtnColHead->Disable();
            pBtnRowHead->Disable();
            pEdAssign2->Disable();
            pRbAssign2->Disable();
        }
        pBtnRemove->Disable();
        pEdAssign->GrabFocus();
    }

    pEdAssign->Enable();
    pRbAssign->Enable();
}

// handler called when the label range has changed
IMPL_LINK_NOARG(ScColRowNameRangesDlg, Range1DataModifyHdl, Edit&, void)
{
    OUString aNewArea( pEdAssign->GetText() );
    bool bValid = false;
    if (!aNewArea.isEmpty() && pDoc)
    {
        ScRange aRange;
        if ( (aRange.ParseAny(aNewArea, pDoc, pDoc->GetAddressConvention() ) & ScRefFlags::VALID) == ScRefFlags::VALID)
        {
            SetColRowData( aRange );
            bValid = true;
        }
    }
    if ( bValid )
    {
        pBtnAdd->Enable();
        pBtnColHead->Enable();
        pBtnRowHead->Enable();
        pEdAssign2->Enable();
        pRbAssign2->Enable();
    }
    else
    {
        pBtnAdd->Disable();
        pBtnColHead->Disable();
        pBtnRowHead->Disable();
        pEdAssign2->Disable();
        pRbAssign2->Disable();
    }
    pBtnRemove->Disable();
}

// handler called when the data range has changed
IMPL_LINK_NOARG(ScColRowNameRangesDlg, Range2DataModifyHdl, Edit&, void)
{
    OUString aNewData( pEdAssign2->GetText() );
    if ( !aNewData.isEmpty() )
    {
        ScRange aRange;
        if ( (aRange.ParseAny(aNewData, pDoc, pDoc->GetAddressConvention() ) & ScRefFlags::VALID) == ScRefFlags::VALID)
        {
            AdjustColRowData( aRange );
            pBtnAdd->Enable();
        }
        else
            pBtnAdd->Disable();
    }
    else
    {
        pBtnAdd->Disable();
    }
}

// handler for the radio button for columns, adjust ranges
IMPL_LINK_NOARG(ScColRowNameRangesDlg, ColClickHdl, Button*, void)
{
    if ( !pBtnColHead->GetSavedValue() )
    {
        pBtnColHead->Check();
        pBtnRowHead->Check( false );
        if ( theCurArea.aStart.Row() == 0 && theCurArea.aEnd.Row() == MAXROW )
        {
            theCurArea.aEnd.SetRow( MAXROW - 1 );
            OUString aStr(theCurArea.Format(ScRefFlags::RANGE_ABS_3D, pDoc, pDoc->GetAddressConvention()));
            pEdAssign->SetText( aStr );
        }
        ScRange aRange( theCurData );
        aRange.aStart.SetRow( std::min( static_cast<long>(theCurArea.aEnd.Row() + 1), long(MAXROW) ) );
        aRange.aEnd.SetRow( MAXROW );
        AdjustColRowData( aRange );
    }
}

// handler for the radio button for columns, adjust range
IMPL_LINK_NOARG(ScColRowNameRangesDlg, RowClickHdl, Button*, void)
{
    if ( !pBtnRowHead->GetSavedValue() )
    {
        pBtnRowHead->Check();
        pBtnColHead->Check( false );
        if ( theCurArea.aStart.Col() == 0 && theCurArea.aEnd.Col() == MAXCOL )
        {
            theCurArea.aEnd.SetCol( MAXCOL - 1 );
            OUString aStr(theCurArea.Format(ScRefFlags::RANGE_ABS_3D, pDoc, pDoc->GetAddressConvention()));
            pEdAssign->SetText( aStr );
        }
        ScRange aRange( theCurData );
        aRange.aStart.SetCol( static_cast<SCCOL>(std::min( static_cast<long>(theCurArea.aEnd.Col() + 1), long(MAXCOL) )) );
        aRange.aEnd.SetCol( MAXCOL );
        AdjustColRowData( aRange );
    }
}

IMPL_LINK( ScColRowNameRangesDlg, GetFocusHdl, Control&, rCtrl, void )
{
    if( (&rCtrl == static_cast<Control*>(pEdAssign)) || (&rCtrl == static_cast<Control*>(pRbAssign)) )
        pEdActive = pEdAssign;
    else if( (&rCtrl == static_cast<Control*>(pEdAssign2)) || (&rCtrl == static_cast<Control*>(pRbAssign2)) )
        pEdActive = pEdAssign2;
    else
        pEdActive = nullptr;

    if( pEdActive )
        pEdActive->SetSelection( Selection( 0, SELECTION_MAX ) );
}

IMPL_LINK_NOARG(ScColRowNameRangesDlg, LoseFocusHdl, Control&, void)
{
    bDlgLostFocus = !IsActive();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
