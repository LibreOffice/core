/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
#include "dbdata.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "dbdocfun.hxx"
#include "editable.hxx"
#include "queryentry.hxx"
#include "markdata.hxx"



ScDBFunc::ScDBFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
    ScViewFunc( pParent, rDocSh, pViewShell )
{
}

ScDBFunc::~ScDBFunc()
{
}

//

//

void ScDBFunc::GotoDBArea( const OUString& rDBName )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDBCollection* pDBCol = pDoc->GetDBCollection();
    ScDBData* pData = pDBCol->getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(rDBName));
    if (pData)
    {
        SCTAB nTab = 0;
        SCCOL nStartCol = 0;
        SCROW nStartRow = 0;
        SCCOL nEndCol = 0;
        SCROW nEndRow = 0;

        pData->GetArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
        SetTabNo( nTab );

        MoveCursorAbs( nStartCol, nStartRow, ScFollowMode( SC_FOLLOW_JUMP ),
                       false, false );  
        DoneBlockMode();
        InitBlockMode( nStartCol, nStartRow, nTab );
        MarkCursor( nEndCol, nEndRow, nTab );
        SelectionChanged();
    }
}



ScDBData* ScDBFunc::GetDBData( bool bMark, ScGetDBMode eMode, ScGetDBSelection eSel )
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
            
            
            bShrinkColumnsOnly = true;
            
            
            if (aRange.aStart.Row() != aRange.aEnd.Row())
            {
                
                
                eSel = SC_DBSEL_SHRINK_TO_USED_DATA;
            }
            else if (aRange.aStart.Col() == aRange.aEnd.Col())
            {
                
                
                const ScMarkData& rMarkData = GetViewData()->GetMarkData();
                if (!(rMarkData.IsMarked() || rMarkData.IsMultiMarked()))
                    eSel = SC_DBSEL_KEEP;
            }
        }
        switch (eSel)
        {
            case SC_DBSEL_SHRINK_TO_SHEET_DATA:
                {
                    
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
                ;   
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

    if (bMark)
    {
        ScRange aFound;
        pData->GetArea(aFound);
        MarkRange( aFound, false );
    }
    return pData;
}

ScDBData* ScDBFunc::GetAnonymousDBData()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScRange aRange;
    ScMarkType eMarkType = GetViewData()->GetSimpleArea(aRange);
    if (eMarkType != SC_MARK_SIMPLE && eMarkType != SC_MARK_SIMPLE_FILTERED)
        return NULL;

    
    const ScMarkData& rMarkData = GetViewData()->GetMarkData();
    if (!rMarkData.IsMarked() && !rMarkData.IsMultiMarked())
    {
        SCCOL nCol1 = aRange.aStart.Col();
        SCCOL nCol2 = aRange.aEnd.Col();
        SCROW nRow1 = aRange.aStart.Row();
        SCROW nRow2 = aRange.aEnd.Row();
        pDocSh->GetDocument()->GetDataArea(aRange.aStart.Tab(), nCol1, nRow1, nCol2, nRow2, false, false);
        aRange.aStart.SetCol(nCol1);
        aRange.aStart.SetRow(nRow1);
        aRange.aEnd.SetCol(nCol2);
        aRange.aEnd.SetRow(nRow2);
    }

    return pDocSh->GetAnonymousDBData(aRange);
}



void ScDBFunc::NotifyCloseDbNameDlg( const ScDBCollection& rNewColl, const std::vector<ScRange> &rDelAreaList )
{

    ScDocShell* pDocShell = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocShell );
    ScDocument* pDoc = pDocShell->GetDocument();
    ScDBCollection* pOldColl = pDoc->GetDBCollection();
    ScDBCollection* pUndoColl = NULL;
    const sal_Bool bRecord (pDoc->IsUndoEnabled());

    std::vector<ScRange>::const_iterator iter;
    for (iter = rDelAreaList.begin(); iter != rDelAreaList.end(); ++iter)
    {
        
        const ScAddress& rStart = iter->aStart;
        const ScAddress& rEnd   = iter->aEnd;
        pDocShell->DBAreaDeleted( rStart.Tab(),
                                  rStart.Col(), rStart.Row(),
                                  rEnd.Col(),   rEnd.Row() );

    }

    if (bRecord)
        pUndoColl = new ScDBCollection( *pOldColl );

    

    pDoc->CompileDBFormula( true );     
    pDoc->SetDBCollection( new ScDBCollection( rNewColl ) );
    pDoc->CompileDBFormula( false );    
    pOldColl = NULL;
    pDocShell->PostPaint(ScRange(0, 0, 0, MAXCOL, MAXROW, MAXTAB), PAINT_GRID);
    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );

    if (bRecord)
    {
        ScDBCollection* pRedoColl = new ScDBCollection( rNewColl );
        pDocShell->GetUndoManager()->AddUndoAction(
            new ScUndoDBData( pDocShell, pUndoColl, pRedoColl ) );
    }
}

//

//



void ScDBFunc::UISort( const ScSortParam& rSortParam, bool bRecord )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rSortParam.nCol1, rSortParam.nRow1,
                                                    rSortParam.nCol2, rSortParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "Sort: no DBData" );
        return;
    }

    ScSubTotalParam aSubTotalParam;
    pDBData->GetSubTotalParam( aSubTotalParam );
    if (aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly)
    {
        

        DoSubTotals( aSubTotalParam, bRecord, &rSortParam );
    }
    else
    {
        Sort( rSortParam, bRecord );        
    }
}

void ScDBFunc::Sort( const ScSortParam& rSortParam, bool bRecord, bool bPaint )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScDBDocFunc aDBDocFunc( *pDocSh );
    sal_Bool bSuccess = aDBDocFunc.Sort( nTab, rSortParam, bRecord, bPaint, false );
    if ( bSuccess && !rSortParam.bInplace )
    {
        
        ScRange aDestRange( rSortParam.nDestCol, rSortParam.nDestRow, rSortParam.nDestTab,
                            rSortParam.nDestCol + rSortParam.nCol2 - rSortParam.nCol1,
                            rSortParam.nDestRow + rSortParam.nRow2 - rSortParam.nRow1,
                            rSortParam.nDestTab );
        MarkRange( aDestRange );
    }

    ResetAutoSpell();
}



void ScDBFunc::Query( const ScQueryParam& rQueryParam, const ScRange* pAdvSource, bool bRecord )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScDBDocFunc aDBDocFunc( *pDocSh );
    sal_Bool bSuccess = aDBDocFunc.Query( nTab, rQueryParam, pAdvSource, bRecord, false );

    if (bSuccess)
    {
        sal_Bool bCopy = !rQueryParam.bInplace;
        if (bCopy)
        {
            
            ScDocument* pDoc = pDocSh->GetDocument();
            ScDBData* pDestData = pDoc->GetDBAtCursor(
                                            rQueryParam.nDestCol, rQueryParam.nDestRow,
                                            rQueryParam.nDestTab, true );
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
            SelectionChanged();     
        }

        GetViewData()->GetBindings().Invalidate( SID_UNFILTER );
    }
}



void ScDBFunc::ToggleAutoFilter()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );

    ScQueryParam    aParam;
    ScDocument*     pDoc    = GetViewData()->GetDocument();
    ScDBData*       pDBData = GetDBData(false, SC_DB_MAKE, SC_DBSEL_ROW_DOWN);

    pDBData->SetByRow( true );              
    pDBData->GetQueryParam( aParam );


    SCCOL  nCol;
    SCROW  nRow = aParam.nRow1;
    SCTAB  nTab = GetViewData()->GetTabNo();
    sal_Int16   nFlag;
    sal_Bool    bHasAuto = sal_True;
    sal_Bool    bHeader  = pDBData->HasHeader();
    sal_Bool    bPaint   = false;

    

    for (nCol=aParam.nCol1; nCol<=aParam.nCol2 && bHasAuto; nCol++)
    {
        nFlag = ((ScMergeFlagAttr*) pDoc->
                GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();

        if ( (nFlag & SC_MF_AUTO) == 0 )
            bHasAuto = false;
    }

    if (bHasAuto)                               
    {
        

        for (nCol=aParam.nCol1; nCol<=aParam.nCol2; nCol++)
        {
            nFlag = ((ScMergeFlagAttr*) pDoc->
                    GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();
            pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr( nFlag & ~SC_MF_AUTO ) );
        }

        

        OUString aUndo = ScGlobal::GetRscString( STR_UNDO_QUERY );
        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );

        ScRange aRange;
        pDBData->GetArea( aRange );
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), false ) );

        pDBData->SetAutoFilter(false);

        

        SCSIZE nEC = aParam.GetEntryCount();
        for (SCSIZE i=0; i<nEC; i++)
            aParam.GetEntry(i).bDoQuery = false;
        aParam.bDuplicate = true;
        Query( aParam, NULL, true );

        pDocSh->GetUndoManager()->LeaveListAction();

        bPaint = sal_True;
    }
    else                                    
    {
        if ( !pDoc->IsBlockEmpty( nTab,
                                  aParam.nCol1, aParam.nRow1,
                                  aParam.nCol2, aParam.nRow2 ) )
        {
            if (!bHeader)
            {
                if ( MessBox( GetViewData()->GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),       
                        ScGlobal::GetRscString( STR_MSSG_MAKEAUTOFILTER_0 )     
                    ).Execute() == RET_YES )
                {
                    pDBData->SetHeader( true );     
                    bHeader = sal_True;
                }
            }

            ScRange aRange;
            pDBData->GetArea( aRange );
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), true ) );

            pDBData->SetAutoFilter(true);

            for (nCol=aParam.nCol1; nCol<=aParam.nCol2; nCol++)
            {
                nFlag = ((ScMergeFlagAttr*) pDoc->
                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();
                pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr( nFlag | SC_MF_AUTO ) );
            }
            pDocSh->PostPaint(ScRange(aParam.nCol1, nRow, nTab, aParam.nCol2, nRow, nTab),
                              PAINT_GRID);
            bPaint = sal_True;
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



void ScDBFunc::HideAutoFilter()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );

    ScDocument* pDoc = pDocSh->GetDocument();

    ScQueryParam aParam;
    ScDBData* pDBData = GetDBData( false );

    SCTAB nTab;
    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    pDBData->GetArea(nTab, nCol1, nRow1, nCol2, nRow2);

    for (SCCOL nCol=nCol1; nCol<=nCol2; nCol++)
    {
        sal_Int16 nFlag = ((ScMergeFlagAttr*) pDoc->
                                GetAttr( nCol, nRow1, nTab, ATTR_MERGE_FLAG ))->GetValue();
        pDoc->ApplyAttr( nCol, nRow1, nTab, ScMergeFlagAttr( nFlag & ~SC_MF_AUTO ) );
    }

    ScRange aRange;
    pDBData->GetArea( aRange );
    pDocSh->GetUndoManager()->AddUndoAction(
        new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), false ) );

    pDBData->SetAutoFilter(false);

    pDocSh->PostPaint(ScRange(nCol1, nRow1, nTab, nCol2, nRow1, nTab), PAINT_GRID );
    aModificator.SetDocumentModified();

    SfxBindings& rBindings = GetViewData()->GetBindings();
    rBindings.Invalidate( SID_AUTO_FILTER );
    rBindings.Invalidate( SID_AUTOFILTER_HIDE );
}



bool ScDBFunc::ImportData( const ScImportParam& rParam, bool bRecord )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScEditableTester aTester( pDoc, GetViewData()->GetTabNo(), rParam.nCol1,rParam.nRow1,
                                                            rParam.nCol2,rParam.nRow2 );
    if ( !aTester.IsEditable() )
    {
        ErrorMessage(aTester.GetMessageId());
        return false;
    }

    ScDBDocFunc aDBDocFunc( *GetViewData()->GetDocShell() );
    return aDBDocFunc.DoImport( GetViewData()->GetTabNo(), rParam, NULL, bRecord );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
