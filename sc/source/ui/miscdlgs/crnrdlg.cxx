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
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
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


// note: some of the initialisation is done in Init
ScColRowNameRangesDlg::ScColRowNameRangesDlg( SfxBindings* pB,
                                SfxChildWindow* pCW,
                                weld::Window* pParent,
                                ScViewData* ptrViewData )

    : ScAnyRefDlgController(pB, pCW, pParent, "modules/scalc/ui/namerangesdialog.ui", "NameRangesDialog")
    , pViewData(ptrViewData)
    , rDoc(ptrViewData->GetDocument())
    , bDlgLostFocus(false)
    , m_pEdActive(nullptr)
    , m_xLbRange(m_xBuilder->weld_tree_view("range"))
    , m_xEdAssign(new formula::RefEdit(m_xBuilder->weld_entry("edassign")))
    , m_xRbAssign(new formula::RefButton(m_xBuilder->weld_button("rbassign")))
    , m_xBtnColHead(m_xBuilder->weld_radio_button("colhead"))
    , m_xBtnRowHead(m_xBuilder->weld_radio_button("rowhead"))
    , m_xEdAssign2(new formula::RefEdit(m_xBuilder->weld_entry("edassign2")))
    , m_xRbAssign2(new formula::RefButton(m_xBuilder->weld_button("rbassign2")))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
    , m_xBtnAdd(m_xBuilder->weld_button("add"))
    , m_xBtnRemove(m_xBuilder->weld_button("delete"))
    , m_xRangeFrame(m_xBuilder->weld_frame("rangeframe"))
    , m_xRangeFT(m_xRangeFrame->weld_label_widget())
    , m_xDataFT(m_xBuilder->weld_label("datarange"))
{
    m_xRbAssign->SetReferences(this, m_xEdAssign.get());
    m_xEdAssign->SetReferences(this, m_xRangeFT.get());
    m_xRbAssign2->SetReferences(this, m_xEdAssign2.get());
    m_xEdAssign2->SetReferences(this, m_xDataFT.get());

    xColNameRanges = rDoc.GetColNameRanges()->Clone();
    xRowNameRanges = rDoc.GetRowNameRanges()->Clone();
    Init();
}

ScColRowNameRangesDlg::~ScColRowNameRangesDlg()
{
}

// initialises event handlers and start parameters in the dialog
void ScColRowNameRangesDlg::Init()
{
    m_xBtnOk->connect_clicked      ( LINK( this, ScColRowNameRangesDlg, OkBtnHdl ) );
    m_xBtnCancel->connect_clicked  ( LINK( this, ScColRowNameRangesDlg, CancelBtnHdl ) );
    m_xBtnAdd->connect_clicked     ( LINK( this, ScColRowNameRangesDlg, AddBtnHdl ) );
    m_xBtnRemove->connect_clicked  ( LINK( this, ScColRowNameRangesDlg, RemoveBtnHdl ) );
    m_xLbRange->connect_changed( LINK( this, ScColRowNameRangesDlg, Range1SelectHdl ) );
    m_xEdAssign->SetModifyHdl  ( LINK( this, ScColRowNameRangesDlg, Range1DataModifyHdl ) );
    m_xBtnColHead->connect_clicked ( LINK( this, ScColRowNameRangesDlg, ColClickHdl ) );
    m_xBtnRowHead->connect_clicked ( LINK( this, ScColRowNameRangesDlg, RowClickHdl ) );
    m_xEdAssign2->SetModifyHdl ( LINK( this, ScColRowNameRangesDlg, Range2DataModifyHdl ) );

    Link<formula::RefEdit&,void> aEditLink = LINK( this, ScColRowNameRangesDlg, GetEditFocusHdl );
    m_xEdAssign->SetGetFocusHdl( aEditLink );
    m_xEdAssign2->SetGetFocusHdl( aEditLink );

    Link<formula::RefButton&,void> aButtonLink = LINK( this, ScColRowNameRangesDlg, GetButtonFocusHdl );
    m_xRbAssign->SetGetFocusHdl( aButtonLink );
    m_xRbAssign2->SetGetFocusHdl( aButtonLink );

    aEditLink = LINK( this, ScColRowNameRangesDlg, LoseEditFocusHdl );
    m_xEdAssign->SetLoseFocusHdl( aEditLink );
    m_xEdAssign2->SetLoseFocusHdl( aEditLink );

    aButtonLink = LINK( this, ScColRowNameRangesDlg, LoseButtonFocusHdl );
    m_xRbAssign2->SetLoseFocusHdl( aButtonLink );
    m_xRbAssign->SetLoseFocusHdl( aButtonLink );

    m_pEdActive = m_xEdAssign.get();

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
        m_xBtnColHead->set_active(true);
        m_xBtnRowHead->set_active(false);
        m_xEdAssign->SetText( EMPTY_OUSTRING );
        m_xEdAssign2->SetText( EMPTY_OUSTRING );
    }

    m_xBtnColHead->set_sensitive(true);
    m_xBtnRowHead->set_sensitive(true);
    m_xEdAssign->GetWidget()->set_sensitive(true);
    m_xEdAssign->GrabFocus();
    m_xRbAssign->GetWidget()->set_sensitive(true);

    Range1SelectHdl( *m_xLbRange );
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
    if ( (static_cast<SCCOLROW>(nCol2 - nCol1) >= nRow2 - nRow1) || (nCol1 == 0 && nCol2 == rDoc.MaxCol()) )
    {   // Column headers and the limiting case of the whole sheet
        m_xBtnColHead->set_active(true);
        m_xBtnRowHead->set_active(false);
        if ( nRow2 == rDoc.MaxRow()  )
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
            theCurData.aEnd.SetRow( rDoc.MaxRow() );
        }
    }
    else
    {   // Column headers
        m_xBtnRowHead->set_active(true);
        m_xBtnColHead->set_active(false);
        if ( nCol2 == rDoc.MaxCol() )
        {   // Header at the right, data to the left
            theCurData.aStart.SetCol( 0 );
            theCurData.aEnd.SetCol( nCol2 - 1 );
        }
        else
        {   // Header at the left, data to the right
            theCurData.aStart.SetCol( nCol2 + 1 );
            theCurData.aEnd.SetCol( rDoc.MaxCol() );
        }
    }
    if ( bValid )
    {
        const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();
        OUString aStr(theCurArea.Format(rDoc, ScRefFlags::RANGE_ABS_3D, eConv));

        if(bRef)
            m_xEdAssign->SetRefString( aStr );
        else
            m_xEdAssign->SetText( aStr );

        m_xEdAssign->SetCursorAtLast();
        aStr = theCurData.Format(rDoc, ScRefFlags::RANGE_ABS_3D, eConv);

        if(bRef)
            m_xEdAssign2->SetRefString( aStr );
        else
            m_xEdAssign2->SetText( aStr );
    }
    else
    {
        theCurData = theCurArea = ScRange();

        if(bRef)
        {
            m_xEdAssign->SetRefString( EMPTY_OUSTRING );
            m_xEdAssign2->SetRefString( EMPTY_OUSTRING );
        }
        else
        {
            m_xEdAssign->SetText( EMPTY_OUSTRING );
            m_xEdAssign2->SetText( EMPTY_OUSTRING );
        }

        m_xBtnColHead->set_sensitive(false);
        m_xBtnRowHead->set_sensitive(false);
        m_xEdAssign2->GetWidget()->set_sensitive(false);
        m_xRbAssign2->GetWidget()->set_sensitive(false);
    }
}

// adjust label range and set the data reference form element
void ScColRowNameRangesDlg::AdjustColRowData( const ScRange& rDataRange, bool bRef)
{
    theCurData = rDataRange;
    if ( m_xBtnColHead->get_active() )
    {   // Data range is the same columns as the header
        theCurData.aStart.SetCol( theCurArea.aStart.Col() );
        theCurData.aEnd.SetCol( theCurArea.aEnd.Col() );
        if ( theCurData.Intersects( theCurArea ) )
        {
            SCROW nRow1 = theCurArea.aStart.Row();
            SCROW nRow2 = theCurArea.aEnd.Row();
            if ( nRow1 > 0
              && (theCurData.aEnd.Row() < nRow2 || nRow2 == rDoc.MaxRow()) )
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
              && (theCurData.aEnd.Col() < nCol2 || nCol2 == rDoc.MaxCol()) )
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
    OUString aStr(theCurData.Format(rDoc, ScRefFlags::RANGE_ABS_3D, rDoc.GetAddressConvention()));

    if(bRef)
        m_xEdAssign2->SetRefString( aStr );
    else
        m_xEdAssign2->SetText( aStr );

    m_xEdAssign2->SetCursorAtLast();
}

// Set the reference to a cell range selected with the mouse and update
// the selection form element
void ScColRowNameRangesDlg::SetReference( const ScRange& rRef, ScDocument& /* rDoc */ )
{
    if ( !m_pEdActive )
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart( m_pEdActive );

    if (m_pEdActive == m_xEdAssign.get())
        SetColRowData( rRef, true );
    else
        AdjustColRowData( rRef, true );
    m_xBtnColHead->set_sensitive(true);
    m_xBtnRowHead->set_sensitive(true);
    m_xBtnAdd->set_sensitive(true);
    m_xBtnRemove->set_sensitive(false);
}

void ScColRowNameRangesDlg::Close()
{
    DoClose( ScColRowNameRangesDlgWrapper::GetChildWindowId() );
}

void ScColRowNameRangesDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;
        if( m_pEdActive )
            m_pEdActive->GrabFocus();
    }
    else
        m_xDialog->grab_focus();

    if( m_pEdActive == m_xEdAssign.get() )
        Range1DataModifyHdl( *m_xEdAssign );
    else if( m_pEdActive == m_xEdAssign2.get() )
        Range2DataModifyHdl( *m_xEdAssign2 );

    RefInputDone();
}

void ScColRowNameRangesDlg::UpdateNames()
{
    m_xLbRange->freeze();

    m_xLbRange->clear();
    aRangeMap.clear();
    m_xEdAssign->SetText( EMPTY_OUSTRING );

    size_t nCount, j;

    SCCOL nCol1;
    SCROW nRow1;    //Extension for range names
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    OUString rString;
    const ScAddress::Details aDetails(rDoc.GetAddressConvention());

    OUString strDelim(" --- ");
    OUString aString = strDelim + ScResId( STR_COLUMN ) + strDelim;
    m_xLbRange->append(OUString::number(nEntryDataDelim), aString);
    if ( xColNameRanges->size() > 0 )
    {
        std::vector<const ScRangePair*> aSortArray(xColNameRanges->CreateNameSortedArray(
               rDoc ));
        nCount = aSortArray.size();
        for ( j=0; j < nCount; j++ )
        {
            const ScRange aRange(aSortArray[j]->GetRange(0));
            aString = aRange.Format(rDoc, ScRefFlags::RANGE_ABS_3D, aDetails);

            //@008 get range parameters from document
            aSortArray[j]->GetRange(0).GetVars( nCol1, nRow1, nTab1,
                                            nCol2, nRow2, nTab2 );
            SCCOL q=nCol1+3;
            if(q>nCol2) q=nCol2;
            //@008 construct string
            OUStringBuffer strShow = " [";
            rString = rDoc.GetString(nCol1, nRow1, nTab1);
            strShow.append(rString);
            for(SCCOL i=nCol1+1;i<=q;i++)
            {
                strShow.append(", ");
                rString = rDoc.GetString(i, nRow1, nTab1);
                strShow.append(rString);
            }
            if(q<nCol2) // Too long? Add ",..."
            {
                strShow.append(", ...");
            }
            strShow.append("]");

            //@008 Add string to listbox
            OUString aInsStr = aString + strShow.makeStringAndClear();
            aRangeMap.emplace( aInsStr, aRange );
            m_xLbRange->append(OUString::number(nEntryDataCol), aInsStr);
        }
    }
    aString = strDelim + ScResId( STR_ROW ) + strDelim;
    m_xLbRange->append(OUString::number(nEntryDataDelim), aString);
    if ( xRowNameRanges->size() > 0 )
    {
        std::vector<const ScRangePair*> aSortArray(xRowNameRanges->CreateNameSortedArray(
               rDoc ));
        nCount = aSortArray.size();
        for ( j=0; j < nCount; j++ )
        {
            const ScRange aRange(aSortArray[j]->GetRange(0));
            aString = aRange.Format(rDoc, ScRefFlags::RANGE_ABS_3D, aDetails);

            //@008 Build string for rows below
            aSortArray[j]->GetRange(0).GetVars( nCol1, nRow1, nTab1,
                                            nCol2, nRow2, nTab2 );
            SCROW q=nRow1+3;
            if(q>nRow2) q=nRow2;
            OUStringBuffer strShow = " [";
            rString = rDoc.GetString(nCol1, nRow1, nTab1);
            strShow.append(rString);
            for(SCROW i=nRow1+1;i<=q;i++)
            {
                strShow.append(", ");
                rString = rDoc.GetString(nCol1, i, nTab1);
                strShow.append(rString);
            }
            if(q<nRow2)
            {
                strShow.append(", ...");
            }
            strShow.append("]");

            OUString aInsStr = aString + strShow.makeStringAndClear();
            aRangeMap.emplace( aInsStr, aRange );
            m_xLbRange->append(OUString::number(nEntryDataRow), aInsStr);
        }
    }

    m_xLbRange->thaw();
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
        const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();
        theCurArea = rRange;
        OUString aStr(theCurArea.Format(rDoc, ScRefFlags::RANGE_ABS_3D, eConv));
        m_xEdAssign->SetText( aStr );
        m_xBtnAdd->set_sensitive(false);
        m_xBtnRemove->set_sensitive(true);
        m_xBtnColHead->set_active(bColName);
        m_xBtnRowHead->set_active(!bColName);
        theCurData = pPair->GetRange(1);
        aStr = theCurData.Format(rDoc, ScRefFlags::RANGE_ABS_3D, eConv);
        m_xEdAssign2->SetText( aStr );
    }
    else
    {
        m_xBtnAdd->set_sensitive(true);
        m_xBtnRemove->set_sensitive(false);
    }
    m_xBtnColHead->set_sensitive(true);
    m_xBtnRowHead->set_sensitive(true);
    m_xEdAssign2->GetWidget()->set_sensitive(true);
    m_xRbAssign2->GetWidget()->set_sensitive(true);
}

bool ScColRowNameRangesDlg::IsRefInputMode() const
{
    return (m_pEdActive != nullptr);
}

// Handler:

// handler called when OK is clicked, calls the add button handler before
// passing the range lists to the document
IMPL_LINK_NOARG(ScColRowNameRangesDlg, OkBtnHdl, weld::Button&, void)
{
    AddBtnHdl(*m_xBtnAdd);

    // assign RangeLists to the references in the document
    rDoc.GetColNameRangesRef() = xColNameRanges;
    rDoc.GetRowNameRangesRef() = xRowNameRanges;
    // changed ranges need to take effect
    rDoc.CompileColRowNameFormula();
    ScDocShell* pDocShell = pViewData->GetDocShell();
    pDocShell->PostPaint(ScRange(0, 0, 0, rDoc.MaxCol(), rDoc.MaxRow(), MAXTAB), PaintPartFlags::Grid);
    pDocShell->SetDocumentModified();

    response(RET_OK);
}

IMPL_LINK_NOARG(ScColRowNameRangesDlg, CancelBtnHdl, weld::Button&, void)
{
    response(RET_CANCEL);
}

// handler called when add button clicked: set ranges and add to listbox
IMPL_LINK_NOARG(ScColRowNameRangesDlg, AddBtnHdl, weld::Button&, void)
{
    OUString aNewArea( m_xEdAssign->GetText() );
    OUString aNewData( m_xEdAssign2->GetText() );

    if (aNewArea.isEmpty() || aNewData.isEmpty())
        return;

    const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();
    ScRange aRange1, aRange2;
    bool bOk1 = (aRange1.ParseAny( aNewArea, rDoc, eConv ) & ScRefFlags::VALID) == ScRefFlags::VALID;
    if ( bOk1 && (aRange2.ParseAny( aNewData, rDoc, eConv ) & ScRefFlags::VALID) == ScRefFlags::VALID)
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
        if ( m_xBtnColHead->get_active() )
            xColNameRanges->Join( ScRangePair( theCurArea, theCurData ) );
        else
            xRowNameRanges->Join( ScRangePair( theCurArea, theCurData ) );

        UpdateNames();

        m_xEdAssign->GrabFocus();
        m_xBtnAdd->set_sensitive(false);
        m_xBtnRemove->set_sensitive(false);
        m_xEdAssign->SetText( EMPTY_OUSTRING );
        m_xBtnColHead->set_active(true);
        m_xBtnRowHead->set_active(false);
        m_xEdAssign2->SetText( EMPTY_OUSTRING );
        theCurArea = ScRange();
        theCurData = theCurArea;
        Range1SelectHdl( *m_xLbRange );
    }
    else
    {
        ERRORBOX(m_xDialog.get(), ScResId(STR_INVALIDTABNAME));
        if ( !bOk1 )
            m_xEdAssign->GrabFocus();
        else
            m_xEdAssign2->GrabFocus();
    }
}

IMPL_LINK_NOARG(ScColRowNameRangesDlg, RemoveBtnHdl, weld::Button&, void)
{
    OUString aRangeStr = m_xLbRange->get_selected_text();
    sal_Int32 nSelectPos = m_xLbRange->get_selected_index();
    bool bColName = nSelectPos != -1 && m_xLbRange->get_id(nSelectPos).toInt32() == nEntryDataCol;
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
    if ( !bFound )
        return;

    OUString aStrDelMsg = ScResId( STR_QUERY_DELENTRY );
    OUString aMsg       = aStrDelMsg.getToken( 0, '#' )
                        + aRangeStr
                        + aStrDelMsg.getToken( 1, '#' );

    if (RET_YES != QUERYBOX(m_xDialog.get(), aMsg))
        return;

    if ( bColName )
        xColNameRanges->Remove( *pPair );
    else
        xRowNameRanges->Remove( *pPair );

    UpdateNames();
    const sal_Int32 nCnt = m_xLbRange->n_children();
    if ( nSelectPos >= nCnt )
    {
        if ( nCnt )
            nSelectPos = nCnt - 1;
        else
            nSelectPos = 0;
    }
    m_xLbRange->select(nSelectPos);
    if (nSelectPos && m_xLbRange->get_id(nSelectPos).toInt32() == nEntryDataDelim)
        m_xLbRange->select( --nSelectPos );    // ---Row---

    m_xLbRange->grab_focus();
    m_xBtnAdd->set_sensitive(false);
    m_xBtnRemove->set_sensitive(false);
    m_xEdAssign->SetText( EMPTY_OUSTRING );
    theCurArea = theCurData = ScRange();
    m_xBtnColHead->set_active(true);
    m_xBtnRowHead->set_active(false);
    m_xEdAssign2->SetText( EMPTY_OUSTRING );
    Range1SelectHdl( *m_xLbRange );
}

// handler called when a row in the listbox is selected, updates form input fields
IMPL_LINK_NOARG(ScColRowNameRangesDlg, Range1SelectHdl, weld::TreeView&, void)
{
    sal_Int32 nSelectPos = m_xLbRange->get_selected_index();
    const sal_Int32 nCnt = m_xLbRange->n_children();
    sal_uInt16 nMoves = 0;
    while (nSelectPos != -1 && nSelectPos < nCnt && m_xLbRange->get_id(nSelectPos).toInt32() == nEntryDataDelim)
    {   // skip Delimiter
        ++nMoves;
        ++nSelectPos;
    }
    OUString aRangeStr = m_xLbRange->get_selected_text();
    if ( nMoves )
    {
        if ( nSelectPos > 1 && nSelectPos >= nCnt )
        {   // if entries exist before the " --- Row --- " Delimiter then
            // do not stop at the delimiter
            nSelectPos = nCnt - 2;
            m_xLbRange->select(nSelectPos);
            aRangeStr = m_xLbRange->get_selected_text();
        }
        else if ( nSelectPos > 2 && nSelectPos < nCnt && !aRangeStr.isEmpty()
                  && aRangeStr == m_xEdAssign->GetText() )
        {   // move upwards instead of below to the previous position
            nSelectPos -= 2;
            m_xLbRange->select( nSelectPos );
            aRangeStr = m_xLbRange->get_selected_text();
        }
        else
            m_xLbRange->select(nSelectPos);
    }
    NameRangeMap::const_iterator itr = aRangeMap.find(aRangeStr);
    if ( itr != aRangeMap.end() )
    {
        bool bColName = m_xLbRange->get_id(nSelectPos).toInt32() == nEntryDataCol;
        UpdateRangeData( itr->second, bColName );
        m_xBtnAdd->set_sensitive(false);
        m_xBtnRemove->set_sensitive(true);
    }
    else
    {
        if ( !m_xEdAssign->GetText().isEmpty() )
        {
            if ( !m_xEdAssign2->GetText().isEmpty() )
                m_xBtnAdd->set_sensitive(true);
            else
                m_xBtnAdd->set_sensitive(false);
            m_xBtnColHead->set_sensitive(true);
            m_xBtnRowHead->set_sensitive(true);
            m_xEdAssign2->GetWidget()->set_sensitive(true);
            m_xRbAssign2->GetWidget()->set_sensitive(true);
        }
        else
        {
            m_xBtnAdd->set_sensitive(false);
            m_xBtnColHead->set_sensitive(false);
            m_xBtnRowHead->set_sensitive(false);
            m_xEdAssign2->GetWidget()->set_sensitive(false);
            m_xRbAssign2->GetWidget()->set_sensitive(false);
        }
        m_xBtnRemove->set_sensitive(false);
        m_xEdAssign->GrabFocus();
    }

    m_xEdAssign->GetWidget()->set_sensitive(true);
    m_xRbAssign->GetWidget()->set_sensitive(true);
}

// handler called when the label range has changed
IMPL_LINK_NOARG(ScColRowNameRangesDlg, Range1DataModifyHdl, formula::RefEdit&, void)
{
    OUString aNewArea( m_xEdAssign->GetText() );
    bool bValid = false;
    if (!aNewArea.isEmpty())
    {
        ScRange aRange;
        if ( (aRange.ParseAny(aNewArea, rDoc, rDoc.GetAddressConvention() ) & ScRefFlags::VALID) == ScRefFlags::VALID)
        {
            SetColRowData( aRange );
            bValid = true;
        }
    }
    if ( bValid )
    {
        m_xBtnAdd->set_sensitive(true);
        m_xBtnColHead->set_sensitive(true);
        m_xBtnRowHead->set_sensitive(true);
        m_xEdAssign2->GetWidget()->set_sensitive(true);
        m_xRbAssign2->GetWidget()->set_sensitive(true);
    }
    else
    {
        m_xBtnAdd->set_sensitive(false);
        m_xBtnColHead->set_sensitive(false);
        m_xBtnRowHead->set_sensitive(false);
        m_xEdAssign2->GetWidget()->set_sensitive(false);
        m_xRbAssign2->GetWidget()->set_sensitive(false);
    }
    m_xBtnRemove->set_sensitive(false);
}

// handler called when the data range has changed
IMPL_LINK_NOARG(ScColRowNameRangesDlg, Range2DataModifyHdl, formula::RefEdit&, void)
{
    OUString aNewData( m_xEdAssign2->GetText() );
    if ( !aNewData.isEmpty() )
    {
        ScRange aRange;
        if ( (aRange.ParseAny(aNewData, rDoc, rDoc.GetAddressConvention() ) & ScRefFlags::VALID) == ScRefFlags::VALID)
        {
            AdjustColRowData( aRange );
            m_xBtnAdd->set_sensitive(true);
        }
        else
            m_xBtnAdd->set_sensitive(false);
    }
    else
    {
        m_xBtnAdd->set_sensitive(false);
    }
}

// handler for the radio button for columns, adjust ranges
IMPL_LINK_NOARG(ScColRowNameRangesDlg, ColClickHdl, weld::Button&, void)
{
    if (!m_xBtnColHead->get_active())
        return;

    if ( theCurArea.aStart.Row() == 0 && theCurArea.aEnd.Row() == rDoc.MaxRow() )
    {
        theCurArea.aEnd.SetRow( rDoc.MaxRow() - 1 );
        OUString aStr(theCurArea.Format(rDoc, ScRefFlags::RANGE_ABS_3D, rDoc.GetAddressConvention()));
        m_xEdAssign->SetText( aStr );
    }
    ScRange aRange( theCurData );
    aRange.aStart.SetRow( std::min( static_cast<long>(theCurArea.aEnd.Row() + 1), static_cast<long>(rDoc.MaxRow()) ) );
    aRange.aEnd.SetRow( rDoc.MaxRow() );
    AdjustColRowData( aRange );
}

// handler for the radio button for columns, adjust range
IMPL_LINK_NOARG(ScColRowNameRangesDlg, RowClickHdl, weld::Button&, void)
{
    if (!m_xBtnRowHead->get_active())
        return;

    if ( theCurArea.aStart.Col() == 0 && theCurArea.aEnd.Col() == rDoc.MaxCol() )
    {
        theCurArea.aEnd.SetCol( rDoc.MaxCol() - 1 );
        OUString aStr(theCurArea.Format(rDoc, ScRefFlags::RANGE_ABS_3D, rDoc.GetAddressConvention()));
        m_xEdAssign->SetText( aStr );
    }
    ScRange aRange( theCurData );
    aRange.aStart.SetCol( static_cast<SCCOL>(std::min( static_cast<long>(theCurArea.aEnd.Col() + 1), static_cast<long>(rDoc.MaxCol()) )) );
    aRange.aEnd.SetCol( rDoc.MaxCol() );
    AdjustColRowData( aRange );
}

IMPL_LINK( ScColRowNameRangesDlg, GetEditFocusHdl, formula::RefEdit&, rCtrl, void )
{
    if (&rCtrl == m_xEdAssign.get())
        m_pEdActive = m_xEdAssign.get();
    else if (&rCtrl == m_xEdAssign2.get())
        m_pEdActive = m_xEdAssign2.get();
    else
        m_pEdActive = nullptr;

    if( m_pEdActive )
        m_pEdActive->SelectAll();
}

IMPL_LINK( ScColRowNameRangesDlg, GetButtonFocusHdl, formula::RefButton&, rCtrl, void )
{
    if (&rCtrl == m_xRbAssign.get())
        m_pEdActive = m_xEdAssign.get();
    else if (&rCtrl == m_xRbAssign2.get())
        m_pEdActive = m_xEdAssign2.get();
    else
        m_pEdActive = nullptr;

    if( m_pEdActive )
        m_pEdActive->SelectAll();
}

IMPL_LINK_NOARG(ScColRowNameRangesDlg, LoseEditFocusHdl, formula::RefEdit&, void)
{
    bDlgLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScColRowNameRangesDlg, LoseButtonFocusHdl, formula::RefButton&, void)
{
    bDlgLostFocus = !m_xDialog->has_toplevel_focus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
