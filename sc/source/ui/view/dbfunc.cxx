/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/sdbc/XResultSet.hpp>

#include "dbfunc.hxx"
#include "docsh.hxx"
#include "attrib.hxx"
#include "sc.hrc"
#include "undodat.hxx"
#include "dbcolect.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "dbdocfun.hxx"
#include "editable.hxx"

//==================================================================

ScDBFunc::ScDBFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
    ScViewFunc( pParent, rDocSh, pViewShell )
{
}

ScDBFunc::~ScDBFunc()
{
}

//
//      Hilfsfunktionen
//

void ScDBFunc::GotoDBArea( const String& rDBName )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDBCollection* pDBCol = pDoc->GetDBCollection();

    USHORT nFoundAt = 0;
    if ( pDBCol->SearchName( rDBName, nFoundAt ) )
    {
        ScDBData* pData = (*pDBCol)[nFoundAt];
        DBG_ASSERT( pData, "GotoDBArea: Datenbankbereich nicht gefunden!" );

        if ( pData )
        {
            SCTAB nTab = 0;
            SCCOL nStartCol = 0;
            SCROW nStartRow = 0;
            SCCOL nEndCol = 0;
            SCROW nEndRow = 0;

            pData->GetArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
            SetTabNo( nTab );

            MoveCursorAbs( nStartCol, nStartRow, ScFollowMode( SC_FOLLOW_JUMP ),
                               FALSE, FALSE );  // bShift,bControl
            DoneBlockMode();
            InitBlockMode( nStartCol, nStartRow, nTab );
            MarkCursor( nEndCol, nEndRow, nTab );
            SelectionChanged();
        }
    }
}

//  aktuellen Datenbereich fuer Sortieren / Filtern suchen

ScDBData* ScDBFunc::GetDBData( BOOL bMark, ScGetDBMode eMode, ScGetDBSelection eSel, bool /*bShrinkToData*/, bool bExpandRows )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDBData* pData = NULL;
    ScRange aRange;
    ScMarkType eMarkType = GetViewData()->GetSimpleArea(aRange);
    if ( eMarkType == SC_MARK_SIMPLE || eMarkType == SC_MARK_SIMPLE_FILTERED )
    {
        bool bShrinkColumnsOnly = false;
        if (eSel == SC_DBSEL_ROW_DOWN)
        {
            // Don't alter row range, additional rows may have been selected on
            // purpose to append data, or to have a fake header row.
            bShrinkColumnsOnly = true;
            // Select further rows only if only one row or a portion thereof is
            // selected.
            if (aRange.aStart.Row() != aRange.aEnd.Row())
            {
                // If an area is selected shrink that to the actual used
                // columns, don't draw filter buttons for empty columns.
                eSel = SC_DBSEL_SHRINK_TO_USED_DATA;
            }
            else if (aRange.aStart.Col() == aRange.aEnd.Col())
            {
                // One cell only, if it is not marked obtain entire used data
                // area.
                const ScMarkData& rMarkData = GetViewData()->GetMarkData();
                if (!(rMarkData.IsMarked() || rMarkData.IsMultiMarked()))
                    eSel = SC_DBSEL_KEEP;
            }
        }
        switch (eSel)
        {
            case SC_DBSEL_SHRINK_TO_SHEET_DATA:
                {
                    // Shrink the selection to sheet data area.
                    ScDocument* pDoc = pDocSh->GetDocument();
                    SCCOL nCol1 = aRange.aStart.Col(), nCol2 = aRange.aEnd.Col();
                    SCROW nRow1 = aRange.aStart.Row(), nRow2 = aRange.aEnd.Row();
                    if (pDoc->ShrinkToDataArea( aRange.aStart.Tab(), nCol1, nRow1, nCol2, nRow2))
                    {
                        aRange.aStart.SetCol(nCol1);
                        aRange.aEnd.SetCol(nCol2);
                        aRange.aStart.SetRow(nRow1);
                        aRange.aEnd.SetRow(nRow2);
                    }
                }
                break;
            case SC_DBSEL_SHRINK_TO_USED_DATA:
            case SC_DBSEL_ROW_DOWN:
                {
                    // Shrink the selection to actual used area.
                    ScDocument* pDoc = pDocSh->GetDocument();
                    SCCOL nCol1 = aRange.aStart.Col(), nCol2 = aRange.aEnd.Col();
                    SCROW nRow1 = aRange.aStart.Row(), nRow2 = aRange.aEnd.Row();
                    bool bShrunk;
                    pDoc->ShrinkToUsedDataArea( bShrunk, aRange.aStart.Tab(),
                            nCol1, nRow1, nCol2, nRow2, bShrinkColumnsOnly);
                    if (bShrunk)
                    {
                        aRange.aStart.SetCol(nCol1);
                        aRange.aEnd.SetCol(nCol2);
                        aRange.aStart.SetRow(nRow1);
                        aRange.aEnd.SetRow(nRow2);
                    }
                }
                break;
            default:
                ;   // nothing
        }
        pData = pDocSh->GetDBData( aRange, eMode, eSel );
    }
    else if ( eMode != SC_DB_OLD )
        pData = pDocSh->GetDBData(
                    ScRange( GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                             GetViewData()->GetTabNo() ),
                    eMode, SC_DBSEL_KEEP );

    if (!pData)
        return NULL;

    if (bExpandRows)
    {
        // Dynamically expand rows to include any new data rows that are
        // immediately below the original range.
        GetViewData()->GetDocument()->UpdateDynamicEndRow(*pData);
    }
    if (bMark)
    {
        ScRange aFound;
        pData->GetArea(aFound, bExpandRows);
        MarkRange( aFound, FALSE );
    }
    return pData;
}

//  Datenbankbereiche aendern (Dialog)

void ScDBFunc::NotifyCloseDbNameDlg( const ScDBCollection& rNewColl, const List& rDelAreaList )
{

    ScDocShell* pDocShell = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocShell );
    ScDocument* pDoc = pDocShell->GetDocument();
    ScDBCollection* pOldColl = pDoc->GetDBCollection();
    ScDBCollection* pUndoColl = NULL;
    ScDBCollection* pRedoColl = NULL;
    const BOOL bRecord (pDoc->IsUndoEnabled());

    long nDelCount = rDelAreaList.Count();
    for (long nDelPos=0; nDelPos<nDelCount; nDelPos++)
    {
        ScRange* pEntry = (ScRange*) rDelAreaList.GetObject(nDelPos);

        if ( pEntry )
        {
            ScAddress& rStart = pEntry->aStart;
            ScAddress& rEnd   = pEntry->aEnd;
            pDocShell->DBAreaDeleted( rStart.Tab(),
                                       rStart.Col(), rStart.Row(),
                                       rEnd.Col(),   rEnd.Row() );

            //  Targets am SBA abmelden nicht mehr noetig
        }
    }

    if (bRecord)
        pUndoColl = new ScDBCollection( *pOldColl );

    //  neue Targets am SBA anmelden nicht mehr noetig

    pDoc->CompileDBFormula( TRUE );     // CreateFormulaString
    pDoc->SetDBCollection( new ScDBCollection( rNewColl ) );
    pDoc->CompileDBFormula( FALSE );    // CompileFormulaString
    pOldColl = NULL;
    pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );

    if (bRecord)
    {
        pRedoColl = new ScDBCollection( rNewColl );
        pDocShell->GetUndoManager()->AddUndoAction(
            new ScUndoDBData( pDocShell, pUndoColl, pRedoColl ) );
    }
}

//
//      wirkliche Funktionen
//

// Sortieren

void ScDBFunc::UISort( const ScSortParam& rSortParam, BOOL bRecord )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rSortParam.nCol1, rSortParam.nRow1,
                                                    rSortParam.nCol2, rSortParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "Sort: keine DBData" );
        return;
    }

    ScSubTotalParam aSubTotalParam;
    pDBData->GetSubTotalParam( aSubTotalParam );
    if (aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly)
    {
        //  Subtotals wiederholen, mit neuer Sortierung

        DoSubTotals( aSubTotalParam, bRecord, &rSortParam );
    }
    else
    {
        Sort( rSortParam, bRecord );        // nur sortieren
    }
}

void ScDBFunc::Sort( const ScSortParam& rSortParam, BOOL bRecord, BOOL bPaint )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScDBDocFunc aDBDocFunc( *pDocSh );
    BOOL bSuccess = aDBDocFunc.Sort( nTab, rSortParam, bRecord, bPaint, FALSE );
    if ( bSuccess && !rSortParam.bInplace )
    {
        //  Ziel markieren
        ScRange aDestRange( rSortParam.nDestCol, rSortParam.nDestRow, rSortParam.nDestTab,
                            rSortParam.nDestCol + rSortParam.nCol2 - rSortParam.nCol1,
                            rSortParam.nDestRow + rSortParam.nRow2 - rSortParam.nRow1,
                            rSortParam.nDestTab );
        MarkRange( aDestRange );
    }
}

//  Filtern

void ScDBFunc::Query( const ScQueryParam& rQueryParam, const ScRange* pAdvSource, BOOL bRecord )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScDBDocFunc aDBDocFunc( *pDocSh );
    BOOL bSuccess = aDBDocFunc.Query( nTab, rQueryParam, pAdvSource, bRecord, FALSE );

    if (bSuccess)
    {
        BOOL bCopy = !rQueryParam.bInplace;
        if (bCopy)
        {
            //  Zielbereich markieren (DB-Bereich wurde ggf. angelegt)
            ScDocument* pDoc = pDocSh->GetDocument();
            ScDBData* pDestData = pDoc->GetDBAtCursor(
                                            rQueryParam.nDestCol, rQueryParam.nDestRow,
                                            rQueryParam.nDestTab, TRUE );
            if (pDestData)
            {
                ScRange aDestRange;
                pDestData->GetArea(aDestRange);
                MarkRange( aDestRange );
            }
        }

        if (!bCopy)
        {
            UpdateScrollBars();
            SelectionChanged();     // for attribute states (filtered rows are ignored)
        }

        GetViewData()->GetBindings().Invalidate( SID_UNFILTER );
    }
}

//  Autofilter-Knoepfe ein-/ausblenden

void ScDBFunc::ToggleAutoFilter()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );

    ScQueryParam    aParam;
    ScDocument*     pDoc    = GetViewData()->GetDocument();
    ScDBData*       pDBData = GetDBData(false, SC_DB_MAKE, SC_DBSEL_ROW_DOWN, false, true);

    pDBData->SetByRow( TRUE );              //! Undo, vorher abfragen ??
    pDBData->GetQueryParam( aParam );


    SCCOL  nCol;
    SCROW  nRow = aParam.nRow1;
    SCTAB  nTab = GetViewData()->GetTabNo();
    INT16   nFlag;
    BOOL    bHasAuto = TRUE;
    BOOL    bHeader  = pDBData->HasHeader();
    BOOL    bPaint   = FALSE;

    //!     stattdessen aus DB-Bereich abfragen?

    for (nCol=aParam.nCol1; nCol<=aParam.nCol2 && bHasAuto; nCol++)
    {
        nFlag = ((ScMergeFlagAttr*) pDoc->
                GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();

        if ( (nFlag & SC_MF_AUTO) == 0 )
            bHasAuto = FALSE;
    }

    if (bHasAuto)                               // aufheben
    {
        //  Filterknoepfe ausblenden

        for (nCol=aParam.nCol1; nCol<=aParam.nCol2; nCol++)
        {
            nFlag = ((ScMergeFlagAttr*) pDoc->
                    GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();
            pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr( nFlag & ~SC_MF_AUTO ) );
        }

        // use a list action for the AutoFilter buttons (ScUndoAutoFilter) and the filter operation

        String aUndo = ScGlobal::GetRscString( STR_UNDO_QUERY );
        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );

        ScRange aRange;
        pDBData->GetArea( aRange );
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), FALSE ) );

        pDBData->SetAutoFilter(FALSE);

        //  Filter aufheben (incl. Paint / Undo)

        SCSIZE nEC = aParam.GetEntryCount();
        for (SCSIZE i=0; i<nEC; i++)
            aParam.GetEntry(i).bDoQuery = FALSE;
        aParam.bDuplicate = TRUE;
        Query( aParam, NULL, TRUE );

        pDocSh->GetUndoManager()->LeaveListAction();

        bPaint = TRUE;
    }
    else                                    // Filterknoepfe einblenden
    {
        if ( !pDoc->IsBlockEmpty( nTab,
                                  aParam.nCol1, aParam.nRow1,
                                  aParam.nCol2, aParam.nRow2 ) )
        {
            if (!bHeader)
            {
                if ( MessBox( GetViewData()->GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),       // "StarCalc"
                        ScGlobal::GetRscString( STR_MSSG_MAKEAUTOFILTER_0 )     // Koepfe aus erster Zeile?
                    ).Execute() == RET_YES )
                {
                    pDBData->SetHeader( TRUE );     //! Undo ??
                    bHeader = TRUE;
                }
            }

            ScRange aRange;
            pDBData->GetArea( aRange );
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), TRUE ) );

            pDBData->SetAutoFilter(TRUE);

            for (nCol=aParam.nCol1; nCol<=aParam.nCol2; nCol++)
            {
                nFlag = ((ScMergeFlagAttr*) pDoc->
                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();
                pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr( nFlag | SC_MF_AUTO ) );
            }
            pDocSh->PostPaint( aParam.nCol1, nRow, nTab, aParam.nCol2, nRow, nTab,
                                                     PAINT_GRID );
            bPaint = TRUE;
        }
        else
        {
            ErrorBox aErrorBox( GetViewData()->GetDialogParent(), WinBits( WB_OK | WB_DEF_OK ),
                                ScGlobal::GetRscString( STR_ERR_AUTOFILTER ) );
            aErrorBox.Execute();
        }
    }

    if ( bPaint )
    {
        aModificator.SetDocumentModified();

        SfxBindings& rBindings = GetViewData()->GetBindings();
        rBindings.Invalidate( SID_AUTO_FILTER );
        rBindings.Invalidate( SID_AUTOFILTER_HIDE );
    }
}

//      nur ausblenden, keine Daten veraendern

void ScDBFunc::HideAutoFilter()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );

    ScDocument* pDoc = pDocSh->GetDocument();

    ScQueryParam aParam;
    ScDBData* pDBData = GetDBData( FALSE );

    SCTAB nTab;
    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    pDBData->GetArea(nTab, nCol1, nRow1, nCol2, nRow2);

    for (SCCOL nCol=nCol1; nCol<=nCol2; nCol++)
    {
        INT16 nFlag = ((ScMergeFlagAttr*) pDoc->
                                GetAttr( nCol, nRow1, nTab, ATTR_MERGE_FLAG ))->GetValue();
        pDoc->ApplyAttr( nCol, nRow1, nTab, ScMergeFlagAttr( nFlag & ~SC_MF_AUTO ) );
    }

    ScRange aRange;
    pDBData->GetArea( aRange );
    pDocSh->GetUndoManager()->AddUndoAction(
        new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), FALSE ) );

    pDBData->SetAutoFilter(FALSE);

    pDocSh->PostPaint( nCol1,nRow1,nTab, nCol2,nRow1,nTab, PAINT_GRID );
    aModificator.SetDocumentModified();

    SfxBindings& rBindings = GetViewData()->GetBindings();
    rBindings.Invalidate( SID_AUTO_FILTER );
    rBindings.Invalidate( SID_AUTOFILTER_HIDE );
}

//      Re-Import

BOOL ScDBFunc::ImportData( const ScImportParam& rParam, BOOL bRecord )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScEditableTester aTester( pDoc, GetViewData()->GetTabNo(), rParam.nCol1,rParam.nRow1,
                                                            rParam.nCol2,rParam.nRow2 );
    if ( !aTester.IsEditable() )
    {
        ErrorMessage(aTester.GetMessageId());
        return FALSE;
    }

    ScDBDocFunc aDBDocFunc( *GetViewData()->GetDocShell() );
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > xResultSet;
    return aDBDocFunc.DoImport( GetViewData()->GetTabNo(), rParam, xResultSet, NULL, bRecord );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
