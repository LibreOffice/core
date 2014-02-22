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

#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <svx/dataaccessdescriptor.hxx>

#include <com/sun/star/sdb/CommandType.hpp>

#include "dbdocfun.hxx"
#include "sc.hrc"
#include "dbdata.hxx"
#include "undodat.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "globstr.hrc"
#include "globalnames.hxx"
#include "tabvwsh.hxx"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "olinetab.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dociter.hxx"
#include "editable.hxx"
#include "attrib.hxx"
#include "drwlayer.hxx"
#include "dpshttab.hxx"
#include "hints.hxx"
#include "queryentry.hxx"
#include "markdata.hxx"
#include "progress.hxx"

#include <set>
#include <memory>

using namespace ::com::sun::star;



bool ScDBDocFunc::AddDBRange( const OUString& rName, const ScRange& rRange, bool /* bApi */ )
{

    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();
    sal_Bool bUndo (pDoc->IsUndoEnabled());

    ScDBCollection* pUndoColl = NULL;
    if (bUndo)
        pUndoColl = new ScDBCollection( *pDocColl );

    ScDBData* pNew = new ScDBData( rName, rRange.aStart.Tab(),
                                    rRange.aStart.Col(), rRange.aStart.Row(),
                                    rRange.aEnd.Col(), rRange.aEnd.Row() );

    
    
    
    bool bCompile = !pDoc->IsImportingXML();
    bool bOk;
    if ( bCompile )
        pDoc->CompileDBFormula( true );     
    if ( rName == STR_DB_LOCAL_NONAME )
    {
        pDoc->SetAnonymousDBData(rRange.aStart.Tab() , pNew);
        bOk = true;
    }
    else
    {
        bOk = pDocColl->getNamedDBs().insert(pNew);
    }
    if ( bCompile )
        pDoc->CompileDBFormula( false );    

    if (!bOk)
    {
        delete pNew;
        delete pUndoColl;
        return false;
    }

    if (bUndo)
    {
        ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
        rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );
    }

    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
    return true;
}

bool ScDBDocFunc::DeleteDBRange(const OUString& rName)
{
    bool bDone = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();
    bool bUndo = pDoc->IsUndoEnabled();

    ScDBCollection::NamedDBs& rDBs = pDocColl->getNamedDBs();
    const ScDBData* p = rDBs.findByUpperName(ScGlobal::pCharClass->uppercase(rName));
    if (p)
    {
        ScDocShellModificator aModificator( rDocShell );

        ScDBCollection* pUndoColl = NULL;
        if (bUndo)
            pUndoColl = new ScDBCollection( *pDocColl );

        pDoc->CompileDBFormula( true );     
        rDBs.erase(*p);
        pDoc->CompileDBFormula( false );    

        if (bUndo)
        {
            ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
            rDocShell.GetUndoManager()->AddUndoAction(
                            new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );
        }

        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
        bDone = true;
    }

    return bDone;
}

bool ScDBDocFunc::RenameDBRange( const OUString& rOld, const OUString& rNew )
{
    bool bDone = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();
    bool bUndo = pDoc->IsUndoEnabled();
    ScDBCollection::NamedDBs& rDBs = pDocColl->getNamedDBs();
    const ScDBData* pOld = rDBs.findByUpperName(ScGlobal::pCharClass->uppercase(rOld));
    const ScDBData* pNew = rDBs.findByUpperName(ScGlobal::pCharClass->uppercase(rNew));
    if (pOld && !pNew)
    {
        ScDocShellModificator aModificator( rDocShell );

        ScDBData* pNewData = new ScDBData(rNew, *pOld);

        ScDBCollection* pUndoColl = new ScDBCollection( *pDocColl );

        pDoc->CompileDBFormula(true);               
        rDBs.erase(*pOld);
        bool bInserted = rDBs.insert(pNewData);
        if (!bInserted)                             
            pDoc->SetDBCollection(pUndoColl);       
                                                    //
        pDoc->CompileDBFormula( false );            

        if (bInserted)                              
        {
            if (bUndo)
            {
                ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
                rDocShell.GetUndoManager()->AddUndoAction(
                                new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );
            }
            else
                delete pUndoColl;

            aModificator.SetDocumentModified();
            SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
            bDone = true;
        }
    }

    return bDone;
}

bool ScDBDocFunc::ModifyDBData( const ScDBData& rNewData )
{
    bool bDone = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();
    bool bUndo = pDoc->IsUndoEnabled();

    ScDBData* pData = NULL;
    if (rNewData.GetName() == STR_DB_LOCAL_NONAME)
    {
        ScRange aRange;
        rNewData.GetArea(aRange);
        SCTAB nTab = aRange.aStart.Tab();
        pData = pDoc->GetAnonymousDBData(nTab);
    }
    else
        pData = pDocColl->getNamedDBs().findByUpperName(rNewData.GetUpperName());

    if (pData)
    {
        ScDocShellModificator aModificator( rDocShell );
        ScRange aOldRange, aNewRange;
        pData->GetArea(aOldRange);
        rNewData.GetArea(aNewRange);
        bool bAreaChanged = ( aOldRange != aNewRange );     

        ScDBCollection* pUndoColl = NULL;
        if (bUndo)
            pUndoColl = new ScDBCollection( *pDocColl );

        *pData = rNewData;
        if (bAreaChanged)
            pDoc->CompileDBFormula();

        if (bUndo)
        {
            ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
            rDocShell.GetUndoManager()->AddUndoAction(
                            new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );
        }

        aModificator.SetDocumentModified();
        bDone = true;
    }

    return bDone;
}



bool ScDBDocFunc::RepeatDB( const OUString& rDBName, bool bRecord, bool bApi, bool bIsUnnamed, SCTAB aTab )
{
    

    bool bDone = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    ScDBData* pDBData = NULL;
    if (bIsUnnamed)
    {
        pDBData = pDoc->GetAnonymousDBData( aTab );
    }
    else
    {
        ScDBCollection* pColl = pDoc->GetDBCollection();
        if (pColl)
            pDBData = pColl->getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(rDBName));
    }

    if ( pDBData )
    {
        ScQueryParam aQueryParam;
        pDBData->GetQueryParam( aQueryParam );
        sal_Bool bQuery = aQueryParam.GetEntry(0).bDoQuery;

        ScSortParam aSortParam;
        pDBData->GetSortParam( aSortParam );
        sal_Bool bSort = aSortParam.maKeyState[0].bDoSort;

        ScSubTotalParam aSubTotalParam;
        pDBData->GetSubTotalParam( aSubTotalParam );
        sal_Bool bSubTotal = aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly;

        if ( bQuery || bSort || bSubTotal )
        {
            sal_Bool bQuerySize = false;
            ScRange aOldQuery;
            ScRange aNewQuery;
            if (bQuery && !aQueryParam.bInplace)
            {
                ScDBData* pDest = pDoc->GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                        aQueryParam.nDestTab, true );
                if (pDest && pDest->IsDoSize())
                {
                    pDest->GetArea( aOldQuery );
                    bQuerySize = sal_True;
                }
            }

            SCTAB nTab;
            SCCOL nStartCol;
            SCROW nStartRow;
            SCCOL nEndCol;
            SCROW nEndRow;
            pDBData->GetArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );

            

            ScDocument* pUndoDoc = NULL;
            ScOutlineTable* pUndoTab = NULL;
            ScRangeName* pUndoRange = NULL;
            ScDBCollection* pUndoDB = NULL;

            if (bRecord)
            {
                SCTAB nTabCount = pDoc->GetTableCount();
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
                if (pTable)
                {
                    pUndoTab = new ScOutlineTable( *pTable );

                    
                    SCCOLROW nOutStartCol, nOutEndCol;
                    SCCOLROW nOutStartRow, nOutEndRow;
                    pTable->GetColArray()->GetRange( nOutStartCol, nOutEndCol );
                    pTable->GetRowArray()->GetRange( nOutStartRow, nOutEndRow );

                    pUndoDoc->InitUndo( pDoc, nTab, nTab, true, true );
                    pDoc->CopyToDocument( static_cast<SCCOL>(nOutStartCol), 0,
                            nTab, static_cast<SCCOL>(nOutEndCol), MAXROW, nTab,
                            IDF_NONE, false, pUndoDoc );
                    pDoc->CopyToDocument( 0, static_cast<SCROW>(nOutStartRow),
                            nTab, MAXCOL, static_cast<SCROW>(nOutEndRow), nTab,
                            IDF_NONE, false, pUndoDoc );
                }
                else
                    pUndoDoc->InitUndo( pDoc, nTab, nTab, false, true );

                
                pDoc->CopyToDocument( 0,nStartRow,nTab, MAXCOL,nEndRow,nTab, IDF_ALL, false, pUndoDoc );

                
                pDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1, IDF_FORMULA, false, pUndoDoc );

                
                ScRangeName* pDocRange = pDoc->GetRangeName();
                if (!pDocRange->empty())
                    pUndoRange = new ScRangeName( *pDocRange );
                ScDBCollection* pDocDB = pDoc->GetDBCollection();
                if (!pDocDB->empty())
                    pUndoDB = new ScDBCollection( *pDocDB );
            }

            if (bSort && bSubTotal)
            {
                

                aSubTotalParam.bRemoveOnly = true;      
                DoSubTotals( nTab, aSubTotalParam, NULL, false, bApi );
            }

            if (bSort)
            {
                pDBData->GetSortParam( aSortParam );            
                Sort( nTab, aSortParam, false, false, bApi );
            }
            if (bQuery)
            {
                pDBData->GetQueryParam( aQueryParam );          
                ScRange aAdvSource;
                if (pDBData->GetAdvancedQuerySource(aAdvSource))
                    Query( nTab, aQueryParam, &aAdvSource, false, bApi );
                else
                    Query( nTab, aQueryParam, NULL, false, bApi );

                


            }
            if (bSubTotal)
            {
                pDBData->GetSubTotalParam( aSubTotalParam );    
                aSubTotalParam.bRemoveOnly = false;
                DoSubTotals( nTab, aSubTotalParam, NULL, false, bApi );
            }

            if (bRecord)
            {
                SCTAB nDummyTab;
                SCCOL nDummyCol;
                SCROW nDummyRow;
                SCROW nNewEndRow;
                pDBData->GetArea( nDummyTab, nDummyCol,nDummyRow, nDummyCol,nNewEndRow );

                const ScRange* pOld = NULL;
                const ScRange* pNew = NULL;
                if (bQuerySize)
                {
                    ScDBData* pDest = pDoc->GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                            aQueryParam.nDestTab, true );
                    if (pDest)
                    {
                        pDest->GetArea( aNewQuery );
                        pOld = &aOldQuery;
                        pNew = &aNewQuery;
                    }
                }

                rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoRepeatDB( &rDocShell, nTab,
                                            nStartCol, nStartRow, nEndCol, nEndRow,
                                            nNewEndRow,
                                            
                                            nStartCol, nStartRow,
                                            pUndoDoc, pUndoTab,
                                            pUndoRange, pUndoDB,
                                            pOld, pNew ) );
            }

            rDocShell.PostPaint(ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab),
                                PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE);
            bDone = true;
        }
        else if (!bApi)     
            rDocShell.ErrorMessage(STR_MSSG_REPEATDB_0);
    }

    return bDone;
}



bool ScDBDocFunc::Sort( SCTAB nTab, const ScSortParam& rSortParam,
                            bool bRecord, bool bPaint, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    SCTAB nSrcTab = nTab;
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();

    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rSortParam.nCol1, rSortParam.nRow1,
                                                    rSortParam.nCol2, rSortParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "Sort: keine DBData" );
        return false;
    }

    ScDBData* pDestData = NULL;
    ScRange aOldDest;
    sal_Bool bCopy = !rSortParam.bInplace;
    if ( bCopy && rSortParam.nDestCol == rSortParam.nCol1 &&
                  rSortParam.nDestRow == rSortParam.nRow1 && rSortParam.nDestTab == nTab )
        bCopy = false;
    ScSortParam aLocalParam( rSortParam );
    if ( bCopy )
    {
        aLocalParam.MoveToDest();
        if ( !ValidColRow( aLocalParam.nCol2, aLocalParam.nRow2 ) )
        {
            if (!bApi)
                rDocShell.ErrorMessage(STR_PASTE_FULL);
            return false;
        }

        nTab = rSortParam.nDestTab;
        pDestData = pDoc->GetDBAtCursor( rSortParam.nDestCol, rSortParam.nDestRow,
                                            rSortParam.nDestTab, true );
        if (pDestData)
            pDestData->GetArea(aOldDest);
    }

    ScEditableTester aTester( pDoc, nTab, aLocalParam.nCol1,aLocalParam.nRow1,
                                        aLocalParam.nCol2,aLocalParam.nRow2 );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if ( aLocalParam.bIncludePattern && pDoc->HasAttrib(
                                        aLocalParam.nCol1, aLocalParam.nRow1, nTab,
                                        aLocalParam.nCol2, aLocalParam.nRow2, nTab,
                                        HASATTR_MERGED | HASATTR_OVERLAPPED ) )
    {
        
        if (!bApi)
            rDocShell.ErrorMessage(STR_SORT_ERR_MERGED);
        return false;
    }


    

    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    sal_Bool bRepeatQuery = false;                          
    ScQueryParam aQueryParam;
    pDBData->GetQueryParam( aQueryParam );
    if ( aQueryParam.GetEntry(0).bDoQuery )
        bRepeatQuery = sal_True;

    if (bRepeatQuery && bCopy)
    {
        if ( aQueryParam.bInplace ||
                aQueryParam.nDestCol != rSortParam.nDestCol ||
                aQueryParam.nDestRow != rSortParam.nDestRow ||
                aQueryParam.nDestTab != rSortParam.nDestTab )       
            bRepeatQuery = false;
    }

    ScUndoSort* pUndoAction = 0;
    if ( bRecord )
    {
        

        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        
        
        pUndoDoc->InitUndo( pDoc, nTab, nTab, false, true );

        /*  #i59745# Do not copy note captions to undo document. All existing
            caption objects will be repositioned while sorting which is tracked
            in drawing undo. When undo is executed, the old positions will be
            restored, and the cells with the old notes (which still refer to the
            existing captions) will be copied back into the source document. */
        pDoc->CopyToDocument( aLocalParam.nCol1, aLocalParam.nRow1, nTab,
                                aLocalParam.nCol2, aLocalParam.nRow2, nTab,
                                IDF_ALL|IDF_NOCAPTIONS, false, pUndoDoc );

        const ScRange* pR = 0;
        if (pDestData)
        {
            /*  #i59745# Do not copy note captions from destination range to
                undo document. All existing caption objects will be removed
                which is tracked in drawing undo. When undo is executed, the
                caption objects are reinserted with drawing undo, and the cells
                with the old notes (which still refer to the existing captions)
                will be copied back into the source document. */
            pDoc->CopyToDocument( aOldDest, IDF_ALL|IDF_NOCAPTIONS, false, pUndoDoc );
            pR = &aOldDest;
        }

        
        

            pDoc->CopyToDocument( 0, aLocalParam.nRow1, nTab, MAXCOL, aLocalParam.nRow2, nTab,
                                    IDF_NONE, false, pUndoDoc );

        ScDBCollection* pUndoDB = NULL;
        ScDBCollection* pDocDB = pDoc->GetDBCollection();
        if (!pDocDB->empty())
            pUndoDB = new ScDBCollection( *pDocDB );

        pUndoAction = new ScUndoSort( &rDocShell, nTab, rSortParam, pUndoDoc, pUndoDB, pR );
        rDocShell.GetUndoManager()->AddUndoAction( pUndoAction );

        
        if( pDrawLayer )
            pDrawLayer->BeginCalcUndo(false);
    }

    if ( bCopy )
    {
        if (pDestData)
            pDoc->DeleteAreaTab(aOldDest, IDF_CONTENTS);            

        ScRange aSource( rSortParam.nCol1,rSortParam.nRow1,nSrcTab,
                            rSortParam.nCol2,rSortParam.nRow2,nSrcTab );
        ScAddress aDest( rSortParam.nDestCol, rSortParam.nDestRow, rSortParam.nDestTab );

        rDocShell.GetDocFunc().MoveBlock( aSource, aDest, false, false, false, true );
    }

    
    if (aLocalParam.GetSortKeyCount() && aLocalParam.maKeyState[0].bDoSort)
    {
        ScProgress aProgress(&rDocShell, ScGlobal::GetRscString(STR_PROGRESS_SORTING), 0);
        pDoc->Sort( nTab, aLocalParam, bRepeatQuery, &aProgress );
    }

    bool bSave = true;
    if (bCopy)
    {
        ScSortParam aOldSortParam;
        pDBData->GetSortParam( aOldSortParam );
        if (aOldSortParam.GetSortKeyCount() &&
            aOldSortParam.maKeyState[0].bDoSort && aOldSortParam.bInplace)
        {
            bSave = false;
            aOldSortParam.nDestCol = rSortParam.nDestCol;
            aOldSortParam.nDestRow = rSortParam.nDestRow;
            aOldSortParam.nDestTab = rSortParam.nDestTab;
            pDBData->SetSortParam( aOldSortParam );                 
        }
    }
    if (bSave)                                              
    {
        pDBData->SetSortParam( rSortParam );
        pDBData->SetHeader( rSortParam.bHasHeader );        
        pDBData->SetByRow( rSortParam.bByRow );             
    }

    if (bCopy)                                          
    {
        
        

        ScRange aDestPos( aLocalParam.nCol1, aLocalParam.nRow1, nTab,
                            aLocalParam.nCol2, aLocalParam.nRow2, nTab );
        ScDBData* pNewData;
        if (pDestData)
            pNewData = pDestData;               
        else                                    
            pNewData = rDocShell.GetDBData(aDestPos, SC_DB_MAKE, SC_DBSEL_FORCE_MARK );
        if (pNewData)
        {
            pNewData->SetArea( nTab,
                                aLocalParam.nCol1,aLocalParam.nRow1,
                                aLocalParam.nCol2,aLocalParam.nRow2 );
            pNewData->SetSortParam( aLocalParam );
            pNewData->SetHeader( aLocalParam.bHasHeader );      
            pNewData->SetByRow( aLocalParam.bByRow );
        }
        else
        {
            OSL_FAIL("Zielbereich nicht da");
        }
    }

    ScRange aDirtyRange( aLocalParam.nCol1, aLocalParam.nRow1, nTab,
        aLocalParam.nCol2, aLocalParam.nRow2, nTab );
    pDoc->SetDirty( aDirtyRange );

    if (bPaint)
    {
        sal_uInt16 nPaint = PAINT_GRID;
        SCCOL nStartX = aLocalParam.nCol1;
        SCROW nStartY = aLocalParam.nRow1;
        SCCOL nEndX = aLocalParam.nCol2;
        SCROW nEndY = aLocalParam.nRow2;
        if ( bRepeatQuery )
        {
            nPaint |= PAINT_LEFT;
            nStartX = 0;
            nEndX = MAXCOL;
        }
        if (pDestData)
        {
            if ( nEndX < aOldDest.aEnd.Col() )
                nEndX = aOldDest.aEnd.Col();
            if ( nEndY < aOldDest.aEnd.Row() )
                nEndY = aOldDest.aEnd.Row();
        }
        rDocShell.PostPaint(ScRange(nStartX, nStartY, nTab, nEndX, nEndY, nTab), nPaint);
    }

    
    rDocShell.AdjustRowHeight( aLocalParam.nRow1, aLocalParam.nRow2, nTab );

    
    if( pUndoAction && pDrawLayer )
        pUndoAction->SetDrawUndoAction( pDrawLayer->GetCalcUndo() );

    aModificator.SetDocumentModified();

    return true;
}



bool ScDBDocFunc::Query( SCTAB nTab, const ScQueryParam& rQueryParam,
                        const ScRange* pAdvSource, bool bRecord, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rQueryParam.nCol1, rQueryParam.nRow1,
                                                    rQueryParam.nCol2, rQueryParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "Query: keine DBData" );
        return false;
    }

    
    

    if ( !rQueryParam.bInplace && pDBData->HasQueryParam() && rQueryParam.bDestPers )
    {
        ScQueryParam aOldQuery;
        pDBData->GetQueryParam(aOldQuery);
        if (aOldQuery.bInplace)
        {
            

            SCSIZE nEC = aOldQuery.GetEntryCount();
            for (SCSIZE i=0; i<nEC; i++)
                aOldQuery.GetEntry(i).bDoQuery = false;
            aOldQuery.bDuplicate = true;
            Query( nTab, aOldQuery, NULL, bRecord, bApi );
        }
    }

    ScQueryParam aLocalParam( rQueryParam );        
    sal_Bool bCopy = !rQueryParam.bInplace;             
    ScDBData* pDestData = NULL;                     
    sal_Bool bDoSize = false;                           
    SCCOL nFormulaCols = 0;                     
    sal_Bool bKeepFmt = false;
    ScRange aOldDest;
    ScRange aDestTotal;
    if ( bCopy && rQueryParam.nDestCol == rQueryParam.nCol1 &&
                  rQueryParam.nDestRow == rQueryParam.nRow1 && rQueryParam.nDestTab == nTab )
        bCopy = false;
    SCTAB nDestTab = nTab;
    if ( bCopy )
    {
        aLocalParam.MoveToDest();
        nDestTab = rQueryParam.nDestTab;
        if ( !ValidColRow( aLocalParam.nCol2, aLocalParam.nRow2 ) )
        {
            if (!bApi)
                rDocShell.ErrorMessage(STR_PASTE_FULL);
            return false;
        }

        ScEditableTester aTester( pDoc, nDestTab, aLocalParam.nCol1,aLocalParam.nRow1,
                                                aLocalParam.nCol2,aLocalParam.nRow2);
        if (!aTester.IsEditable())
        {
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());
            return false;
        }

        pDestData = pDoc->GetDBAtCursor( rQueryParam.nDestCol, rQueryParam.nDestRow,
                                            rQueryParam.nDestTab, true );
        if (pDestData)
        {
            pDestData->GetArea( aOldDest );
            aDestTotal=ScRange( rQueryParam.nDestCol,
                                rQueryParam.nDestRow,
                                nDestTab,
                                rQueryParam.nDestCol + rQueryParam.nCol2 - rQueryParam.nCol1,
                                rQueryParam.nDestRow + rQueryParam.nRow2 - rQueryParam.nRow1,
                                nDestTab );

            bDoSize = pDestData->IsDoSize();
            
            if ( bDoSize && aOldDest.aEnd.Col() == aDestTotal.aEnd.Col() )
            {
                SCCOL nTestCol = aOldDest.aEnd.Col() + 1;       
                SCROW nTestRow = rQueryParam.nDestRow +
                                    ( aLocalParam.bHasHeader ? 1 : 0 );
                while ( nTestCol <= MAXCOL &&
                        pDoc->GetCellType(ScAddress( nTestCol, nTestRow, nTab )) == CELLTYPE_FORMULA )
                    ++nTestCol, ++nFormulaCols;
            }

            bKeepFmt = pDestData->IsKeepFmt();
            if ( bDoSize && !pDoc->CanFitBlock( aOldDest, aDestTotal ) )
            {
                if (!bApi)
                    rDocShell.ErrorMessage(STR_MSSG_DOSUBTOTALS_2);     
                return false;
            }
        }
    }

    

    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    sal_Bool bKeepSub = false;                          
    ScSubTotalParam aSubTotalParam;
    if (rQueryParam.GetEntry(0).bDoQuery)           
    {
        pDBData->GetSubTotalParam( aSubTotalParam );    

        if ( aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly )
            bKeepSub = sal_True;
    }

    ScDocument* pUndoDoc = NULL;
    ScDBCollection* pUndoDB = NULL;
    const ScRange* pOld = NULL;

    if ( bRecord )
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bCopy)
        {
            pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab, false, true );
            pDoc->CopyToDocument( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                    aLocalParam.nCol2, aLocalParam.nRow2, nDestTab,
                                    IDF_ALL, false, pUndoDoc );
            

            if (pDestData)
            {
                pDoc->CopyToDocument( aOldDest, IDF_ALL, false, pUndoDoc );
                pOld = &aOldDest;
            }
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, false, true );
            pDoc->CopyToDocument( 0, rQueryParam.nRow1, nTab, MAXCOL, rQueryParam.nRow2, nTab,
                                        IDF_NONE, false, pUndoDoc );
        }

        ScDBCollection* pDocDB = pDoc->GetDBCollection();
        if (!pDocDB->empty())
            pUndoDB = new ScDBCollection( *pDocDB );

        pDoc->BeginDrawUndo();
    }

    ScDocument* pAttribDoc = NULL;
    ScRange aAttribRange;
    if (pDestData)                                      
    {
        if ( bKeepFmt )
        {
            
            aAttribRange = aOldDest;
            if ( aAttribRange.aEnd.Col() > aDestTotal.aEnd.Col() )
                aAttribRange.aEnd.SetCol( aDestTotal.aEnd.Col() );
            aAttribRange.aEnd.SetRow( aAttribRange.aStart.Row() +
                                        ( aLocalParam.bHasHeader ? 1 : 0 ) );

            
            aAttribRange.aEnd.SetCol( aAttribRange.aEnd.Col() + nFormulaCols );

            pAttribDoc = new ScDocument( SCDOCMODE_UNDO );
            pAttribDoc->InitUndo( pDoc, nDestTab, nDestTab, false, true );
            pDoc->CopyToDocument( aAttribRange, IDF_ATTRIB, false, pAttribDoc );
        }

        if ( bDoSize )
            pDoc->FitBlock( aOldDest, aDestTotal );
        else
            pDoc->DeleteAreaTab(aOldDest, IDF_ALL);         
    }

    
    SCSIZE nCount = pDoc->Query( nTab, rQueryParam, bKeepSub );
    if (bCopy)
    {
        aLocalParam.nRow2 = aLocalParam.nRow1 + nCount;
        if (!aLocalParam.bHasHeader && nCount > 0)
            --aLocalParam.nRow2;

        if ( bDoSize )
        {
            
            

            ScRange aNewDest( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                aLocalParam.nCol2, aLocalParam.nRow2, nDestTab );
            pDoc->FitBlock( aDestTotal, aNewDest, false );      

            if ( nFormulaCols > 0 )
            {
                
                

                ScRange aNewForm( aLocalParam.nCol2+1, aLocalParam.nRow1, nDestTab,
                                  aLocalParam.nCol2+nFormulaCols, aLocalParam.nRow2, nDestTab );
                ScRange aOldForm = aNewForm;
                aOldForm.aEnd.SetRow( aOldDest.aEnd.Row() );
                pDoc->FitBlock( aOldForm, aNewForm, false );

                ScMarkData aMark;
                aMark.SelectOneTable(nDestTab);
                SCROW nFStartY = aLocalParam.nRow1 + ( aLocalParam.bHasHeader ? 1 : 0 );

                sal_uLong nProgCount = nFormulaCols;
                nProgCount *= aLocalParam.nRow2 - nFStartY;
                ScProgress aProgress( pDoc->GetDocumentShell(),
                        ScGlobal::GetRscString(STR_FILL_SERIES_PROGRESS), nProgCount );

                pDoc->Fill( aLocalParam.nCol2+1, nFStartY,
                            aLocalParam.nCol2+nFormulaCols, nFStartY, &aProgress, aMark,
                            aLocalParam.nRow2 - nFStartY,
                            FILL_TO_BOTTOM, FILL_SIMPLE );
            }
        }

        if ( pAttribDoc )       
        {
            
            if (aLocalParam.bHasHeader)
            {
                ScRange aHdrRange = aAttribRange;
                aHdrRange.aEnd.SetRow( aHdrRange.aStart.Row() );
                pAttribDoc->CopyToDocument( aHdrRange, IDF_ATTRIB, false, pDoc );
            }

            
            SCCOL nAttrEndCol = aAttribRange.aEnd.Col();
            SCROW nAttrRow = aAttribRange.aStart.Row() + ( aLocalParam.bHasHeader ? 1 : 0 );
            for (SCCOL nCol = aAttribRange.aStart.Col(); nCol<=nAttrEndCol; nCol++)
            {
                const ScPatternAttr* pSrcPattern = pAttribDoc->GetPattern(
                                                    nCol, nAttrRow, nDestTab );
                OSL_ENSURE(pSrcPattern,"Pattern ist 0");
                if (pSrcPattern)
                {
                    pDoc->ApplyPatternAreaTab( nCol, nAttrRow, nCol, aLocalParam.nRow2,
                                                    nDestTab, *pSrcPattern );
                    const ScStyleSheet* pStyle = pSrcPattern->GetStyleSheet();
                    if (pStyle)
                        pDoc->ApplyStyleAreaTab( nCol, nAttrRow, nCol, aLocalParam.nRow2,
                                                    nDestTab, *pStyle );
                }
            }

            delete pAttribDoc;
        }
    }

    
    

    bool bSave = rQueryParam.bInplace || rQueryParam.bDestPers;
    if (bSave)                                                  
    {
        pDBData->SetQueryParam( rQueryParam );
        pDBData->SetHeader( rQueryParam.bHasHeader );       
        pDBData->SetAdvancedQuerySource( pAdvSource );      
    }

    if (bCopy)                                              
    {
        
        
        

        ScDBData* pNewData;
        if (pDestData)
            pNewData = pDestData;               
        else                                    
            pNewData = rDocShell.GetDBData(
                            ScRange( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                     aLocalParam.nCol2, aLocalParam.nRow2, nDestTab ),
                            SC_DB_MAKE, SC_DBSEL_FORCE_MARK );

        if (pNewData)
        {
            pNewData->SetArea( nDestTab, aLocalParam.nCol1, aLocalParam.nRow1,
                                            aLocalParam.nCol2, aLocalParam.nRow2 );

            
            
        }
        else
        {
            OSL_FAIL("Zielbereich nicht da");
        }
    }

    if (!bCopy)
    {
        pDoc->InvalidatePageBreaks(nTab);
        pDoc->UpdatePageBreaks( nTab );
    }

    
    ScRange aDirtyRange(0 , aLocalParam.nRow1, nDestTab, MAXCOL, aLocalParam.nRow2, nDestTab);
    pDoc->SetSubTotalCellsDirty(aDirtyRange);

    if ( bRecord )
    {
        
        rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoQuery( &rDocShell, nTab, rQueryParam, pUndoDoc, pUndoDB,
                                        pOld, bDoSize, pAdvSource ) );
    }


    if (bCopy)
    {
        SCCOL nEndX = aLocalParam.nCol2;
        SCROW nEndY = aLocalParam.nRow2;
        if (pDestData)
        {
            if ( aOldDest.aEnd.Col() > nEndX )
                nEndX = aOldDest.aEnd.Col();
            if ( aOldDest.aEnd.Row() > nEndY )
                nEndY = aOldDest.aEnd.Row();
        }
        if (bDoSize)
            nEndY = MAXROW;
        rDocShell.PostPaint(
            ScRange(aLocalParam.nCol1, aLocalParam.nRow1, nDestTab, nEndX, nEndY, nDestTab),
            PAINT_GRID);
    }
    else
        rDocShell.PostPaint(
            ScRange(0, rQueryParam.nRow1, nTab, MAXCOL, MAXROW, nTab),
            PAINT_GRID | PAINT_LEFT);
    aModificator.SetDocumentModified();

    return true;
}



bool ScDBDocFunc::DoSubTotals( SCTAB nTab, const ScSubTotalParam& rParam,
                                const ScSortParam* pForceNewSort, bool bRecord, bool bApi )
{
    
    
    
    

    sal_Bool bDo = !rParam.bRemoveOnly;                         
    sal_Bool bRet = false;

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rParam.nCol1, rParam.nRow1,
                                                rParam.nCol2, rParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "SubTotals: keine DBData" );
        return false;
    }

    ScEditableTester aTester( pDoc, nTab, 0,rParam.nRow1+1, MAXCOL,MAXROW );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if (pDoc->HasAttrib( rParam.nCol1, rParam.nRow1+1, nTab,
                         rParam.nCol2, rParam.nRow2, nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0); 
        return false;
    }

    sal_Bool bOk = true;
    if (rParam.bReplace)
        if (pDoc->TestRemoveSubTotals( nTab, rParam ))
        {
            bOk = ( MessBox( rDocShell.GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_1 ) ).Execute()
                == RET_YES );
        }

    if (bOk)
    {
        WaitObject aWait( rDocShell.GetActiveDialogParent() );
        ScDocShellModificator aModificator( rDocShell );

        ScSubTotalParam aNewParam( rParam );        
        ScDocument*     pUndoDoc = NULL;
        ScOutlineTable* pUndoTab = NULL;
        ScRangeName*    pUndoRange = NULL;
        ScDBCollection* pUndoDB = NULL;

        if (bRecord)                                        
        {
            sal_Bool bOldFilter = bDo && rParam.bDoSort;

            SCTAB nTabCount = pDoc->GetTableCount();
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
            if (pTable)
            {
                pUndoTab = new ScOutlineTable( *pTable );

                
                SCCOLROW nOutStartCol, nOutEndCol;
                SCCOLROW nOutStartRow, nOutEndRow;
                pTable->GetColArray()->GetRange( nOutStartCol, nOutEndCol );
                pTable->GetRowArray()->GetRange( nOutStartRow, nOutEndRow );

                pUndoDoc->InitUndo( pDoc, nTab, nTab, true, true );
                pDoc->CopyToDocument( static_cast<SCCOL>(nOutStartCol), 0, nTab, static_cast<SCCOL>(nOutEndCol), MAXROW, nTab, IDF_NONE, false, pUndoDoc );
                pDoc->CopyToDocument( 0, nOutStartRow, nTab, MAXCOL, nOutEndRow, nTab, IDF_NONE, false, pUndoDoc );
            }
            else
                pUndoDoc->InitUndo( pDoc, nTab, nTab, false, bOldFilter );

            
            pDoc->CopyToDocument( 0,rParam.nRow1+1,nTab, MAXCOL,rParam.nRow2,nTab,
                                    IDF_ALL, false, pUndoDoc );

            
            pDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1,
                                        IDF_FORMULA, false, pUndoDoc );

            
            ScRangeName* pDocRange = pDoc->GetRangeName();
            if (!pDocRange->empty())
                pUndoRange = new ScRangeName( *pDocRange );
            ScDBCollection* pDocDB = pDoc->GetDBCollection();
            if (!pDocDB->empty())
                pUndoDB = new ScDBCollection( *pDocDB );
        }


        ScOutlineTable* pOut = pDoc->GetOutlineTable( nTab );
        if (pOut)
            pOut->GetRowArray()->RemoveAll();       

        if (rParam.bReplace)
            pDoc->RemoveSubTotals( nTab, aNewParam );
        sal_Bool bSuccess = sal_True;
        if (bDo)
        {
            
            if ( rParam.bDoSort || pForceNewSort )
            {
                pDBData->SetArea( nTab, aNewParam.nCol1,aNewParam.nRow1, aNewParam.nCol2,aNewParam.nRow2 );

                
                

                ScSortParam aOldSort;
                pDBData->GetSortParam( aOldSort );
                ScSortParam aSortParam( aNewParam, pForceNewSort ? *pForceNewSort : aOldSort );
                Sort( nTab, aSortParam, false, false, bApi );
            }

            bSuccess = pDoc->DoSubTotals( nTab, aNewParam );
            pDoc->SetDrawPageSize(nTab);
        }
        ScRange aDirtyRange( aNewParam.nCol1, aNewParam.nRow1, nTab,
            aNewParam.nCol2, aNewParam.nRow2, nTab );
        pDoc->SetDirty( aDirtyRange );

        if (bRecord)
        {

            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoSubTotals( &rDocShell, nTab,
                                        rParam, aNewParam.nRow2,
                                        pUndoDoc, pUndoTab, 
                                        pUndoRange, pUndoDB ) );
        }

        if (!bSuccess)
        {
            
            if (!bApi)
                rDocShell.ErrorMessage(STR_MSSG_DOSUBTOTALS_2);
        }

                                                    
        pDBData->SetSubTotalParam( aNewParam );
        pDBData->SetArea( nTab, aNewParam.nCol1,aNewParam.nRow1, aNewParam.nCol2,aNewParam.nRow2 );
        pDoc->CompileDBFormula();

        rDocShell.PostPaint(ScRange(0, 0, nTab, MAXCOL,MAXROW,nTab),
                            PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE);
        aModificator.SetDocumentModified();

        bRet = bSuccess;
    }
    return bRet;
}

namespace {

bool lcl_EmptyExcept( ScDocument* pDoc, const ScRange& rRange, const ScRange& rExcept )
{
    ScCellIterator aIter( pDoc, rRange );
    for (bool bHasCell = aIter.first(); bHasCell; bHasCell = aIter.next())
    {
        if (!aIter.isEmpty())      
        {
            if (!rExcept.In(aIter.GetPos()))
                return false;       
        }
    }

    return true;        
}

bool isEditable(ScDocShell& rDocShell, const ScRangeList& rRanges, bool bApi)
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if (!rDocShell.IsEditable() || pDoc->GetChangeTrack())
    {
        
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);

        return false;
    }

    for (size_t i = 0, n = rRanges.size(); i < n; ++i)
    {
        const ScRange* p = rRanges[i];
        ScEditableTester aTester(pDoc, *p);
        if (!aTester.IsEditable())
        {
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());

            return false;
        }
    }

    return true;
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
void createUndoDoc(std::auto_ptr<ScDocument>& pUndoDoc, ScDocument* pDoc, const ScRange& rRange)
{
    SCTAB nTab = rRange.aStart.Tab();
    pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
    pUndoDoc->InitUndo(pDoc, nTab, nTab);
    pDoc->CopyToDocument(rRange, IDF_ALL, false, pUndoDoc.get());
}
SAL_WNODEPRECATED_DECLARATIONS_POP

bool checkNewOutputRange(ScDPObject& rDPObj, ScDocShell& rDocShell, ScRange& rNewOut, bool bApi)
{
    ScDocument* pDoc = rDocShell.GetDocument();

    bool bOverflow = false;
    rNewOut = rDPObj.GetNewOutputRange(bOverflow);

    
    
    const ScSheetSourceDesc* pSheetDesc = rDPObj.GetSheetDesc();
    if (pSheetDesc && pSheetDesc->GetSourceRange().Intersects(rNewOut))
    {
        
        
        ScRange aOldRange = rDPObj.GetOutRange();
        SCsROW nDiff = aOldRange.aStart.Row() - rNewOut.aStart.Row();
        rNewOut.aStart.SetRow(aOldRange.aStart.Row());
        rNewOut.aEnd.IncRow(nDiff);
        if (!ValidRow(rNewOut.aStart.Row()) || !ValidRow(rNewOut.aEnd.Row()))
            bOverflow = true;
    }

    if (bOverflow)
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PIVOT_ERROR);

        return false;
    }

    ScEditableTester aTester(pDoc, rNewOut);
    if (!aTester.IsEditable())
    {
        
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());

        return false;
    }

    return true;
}

}

bool ScDBDocFunc::DataPilotUpdate( ScDPObject* pOldObj, const ScDPObject* pNewObj,
                                   bool bRecord, bool bApi, bool bAllowMove )
{
    if (!pOldObj)
    {
        if (!pNewObj)
            return false;

        return CreatePivotTable(*pNewObj, bRecord, bApi);
    }

    if (pOldObj)
    {
        if (!pNewObj)
            return RemovePivotTable(*pOldObj, bRecord, bApi);

        if (pOldObj == pNewObj)
            return UpdatePivotTable(*pOldObj, bRecord, bApi);
    }

    OSL_ASSERT(pOldObj && pNewObj && pOldObj != pNewObj);

    ScDocShellModificator aModificator( rDocShell );
    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    ScRangeList aRanges;
    aRanges.Append(pOldObj->GetOutRange());
    aRanges.Append(pNewObj->GetOutRange().aStart); 
    if (!isEditable(rDocShell, aRanges, bApi))
        return false;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<ScDocument> pOldUndoDoc;
    std::auto_ptr<ScDocument> pNewUndoDoc;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    ScDPObject aUndoDPObj(*pOldObj); 

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    if (bRecord)
        createUndoDoc(pOldUndoDoc, pDoc, pOldObj->GetOutRange());

    pNewObj->WriteSourceDataTo(*pOldObj);     

    ScDPSaveData* pData = pNewObj->GetSaveData();
    OSL_ENSURE( pData, "no SaveData from living DPObject" );
    if (pData)
        pOldObj->SetSaveData(*pData);     

    pOldObj->SetAllowMove(bAllowMove);
    pOldObj->ReloadGroupTableData();
    pOldObj->SyncAllDimensionMembers();
    pOldObj->InvalidateData();             

    
    if (pOldObj->GetName().isEmpty())
        pOldObj->SetName( pDoc->GetDPCollection()->CreateNewName() );

    ScRange aNewOut;
    if (!checkNewOutputRange(*pOldObj, rDocShell, aNewOut, bApi))
    {
        *pOldObj = aUndoDPObj;
        return false;
    }

    
    if (!bApi)
    {
        
        if (!lcl_EmptyExcept(pDoc, aNewOut, pOldObj->GetOutRange()))
        {
            QueryBox aBox( rDocShell.GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                             ScGlobal::GetRscString(STR_PIVOT_NOTEMPTY) );
            if (aBox.Execute() == RET_NO)
            {
                
                *pOldObj = aUndoDPObj;
                return false;
            }
        }
    }

    if (bRecord)
        createUndoDoc(pNewUndoDoc, pDoc, aNewOut);

    pOldObj->Output(aNewOut.aStart);
    rDocShell.PostPaintGridAll();           

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDataPilot(
                &rDocShell, pOldUndoDoc.release(), pNewUndoDoc.release(), &aUndoDPObj, pOldObj, bAllowMove));
    }

    
    pDoc->BroadcastUno( ScDataPilotModifiedHint(pOldObj->GetName()) );
    aModificator.SetDocumentModified();

    return true;
}

bool ScDBDocFunc::RemovePivotTable(ScDPObject& rDPObj, bool bRecord, bool bApi)
{
    ScDocShellModificator aModificator(rDocShell);
    WaitObject aWait(rDocShell.GetActiveDialogParent());

    if (!isEditable(rDocShell, rDPObj.GetOutRange(), bApi))
        return false;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<ScDocument> pOldUndoDoc;
    std::auto_ptr<ScDPObject> pUndoDPObj;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    if (bRecord)
        pUndoDPObj.reset(new ScDPObject(rDPObj));    

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    

    ScRange aRange = rDPObj.GetOutRange();
    SCTAB nTab = aRange.aStart.Tab();

    if (bRecord)
        createUndoDoc(pOldUndoDoc, pDoc, aRange);

    pDoc->DeleteAreaTab( aRange.aStart.Col(), aRange.aStart.Row(),
                         aRange.aEnd.Col(),   aRange.aEnd.Row(),
                         nTab, IDF_ALL );
    pDoc->RemoveFlagsTab( aRange.aStart.Col(), aRange.aStart.Row(),
                          aRange.aEnd.Col(),   aRange.aEnd.Row(),
                          nTab, SC_MF_AUTO );

    pDoc->GetDPCollection()->FreeTable(&rDPObj);  

    rDocShell.PostPaintGridAll();   
    rDocShell.PostPaint(aRange, PAINT_GRID);

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDataPilot(
                &rDocShell, pOldUndoDoc.release(), NULL, pUndoDPObj.get(), NULL, false));

        
    }

    aModificator.SetDocumentModified();
    return true;
}

bool ScDBDocFunc::CreatePivotTable(const ScDPObject& rDPObj, bool bRecord, bool bApi)
{
    ScDocShellModificator aModificator(rDocShell);
    WaitObject aWait(rDocShell.GetActiveDialogParent());

    
    if (!isEditable(rDocShell, ScRange(rDPObj.GetOutRange().aStart), bApi))
        return false;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<ScDocument> pNewUndoDoc;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<ScDPObject> pDestObj(new ScDPObject(rDPObj));
    SAL_WNODEPRECATED_DECLARATIONS_POP

    ScDPObject& rDestObj = *pDestObj;

    
    
    
    if (pDoc->GetDPCollection()->GetByName(rDestObj.GetName()))
        rDestObj.SetName(OUString());      

    if (!pDoc->GetDPCollection()->InsertNewTable(pDestObj.release()))
        
        return false;

    rDestObj.ReloadGroupTableData();
    rDestObj.SyncAllDimensionMembers();
    rDestObj.InvalidateData();             

    
    if (rDestObj.GetName().isEmpty())
        rDestObj.SetName(pDoc->GetDPCollection()->CreateNewName());

    bool bOverflow = false;
    ScRange aNewOut = rDestObj.GetNewOutputRange(bOverflow);

    if (bOverflow)
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PIVOT_ERROR);

        return false;
    }

    {
        ScEditableTester aTester(pDoc, aNewOut);
        if (!aTester.IsEditable())
        {
            
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());

            return false;
        }
    }

    
    if (!bApi)
    {
        bool bEmpty = pDoc->IsBlockEmpty(
            aNewOut.aStart.Tab(), aNewOut.aStart.Col(), aNewOut.aStart.Row(),
            aNewOut.aEnd.Col(), aNewOut.aEnd.Row());

        if (!bEmpty)
        {
            QueryBox aBox(
                rDocShell.GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                ScGlobal::GetRscString(STR_PIVOT_NOTEMPTY));

            if (aBox.Execute() == RET_NO)
            {
                
                return false;
            }
        }
    }

    if (bRecord)
        createUndoDoc(pNewUndoDoc, pDoc, aNewOut);

    rDestObj.Output(aNewOut.aStart);
    rDocShell.PostPaintGridAll();           

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDataPilot(&rDocShell, NULL, pNewUndoDoc.release(), NULL, &rDestObj, false));
    }

    
    pDoc->BroadcastUno(ScDataPilotModifiedHint(rDestObj.GetName()));
    aModificator.SetDocumentModified();

    return true;
}

bool ScDBDocFunc::UpdatePivotTable(ScDPObject& rDPObj, bool bRecord, bool bApi)
{
    ScDocShellModificator aModificator( rDocShell );
    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    if (!isEditable(rDocShell, rDPObj.GetOutRange(), bApi))
        return false;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<ScDocument> pOldUndoDoc;
    std::auto_ptr<ScDocument> pNewUndoDoc;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    ScDPObject aUndoDPObj(rDPObj); 

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    if (bRecord)
        createUndoDoc(pOldUndoDoc, pDoc, rDPObj.GetOutRange());

    rDPObj.SetAllowMove(false);
    rDPObj.ReloadGroupTableData();
    if (!rDPObj.SyncAllDimensionMembers())
        return false;

    rDPObj.InvalidateData();             

    
    if (rDPObj.GetName().isEmpty())
        rDPObj.SetName( pDoc->GetDPCollection()->CreateNewName() );

    ScRange aNewOut;
    if (!checkNewOutputRange(rDPObj, rDocShell, aNewOut, bApi))
    {
        rDPObj = aUndoDPObj;
        return false;
    }

    
    if (!bApi)
    {
        if (!lcl_EmptyExcept(pDoc, aNewOut, rDPObj.GetOutRange()))
        {
            QueryBox aBox( rDocShell.GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                             ScGlobal::GetRscString(STR_PIVOT_NOTEMPTY) );
            if (aBox.Execute() == RET_NO)
            {
                rDPObj = aUndoDPObj;
                return false;
            }
        }
    }

    if (bRecord)
        createUndoDoc(pNewUndoDoc, pDoc, aNewOut);

    rDPObj.Output(aNewOut.aStart);
    rDocShell.PostPaintGridAll();           

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDataPilot(
                &rDocShell, pOldUndoDoc.release(), pNewUndoDoc.release(), &aUndoDPObj, &rDPObj, false));
    }

    
    pDoc->BroadcastUno( ScDataPilotModifiedHint(rDPObj.GetName()) );
    aModificator.SetDocumentModified();
    return true;
}

sal_uLong ScDBDocFunc::RefreshPivotTables(ScDPObject* pDPObj, bool bApi)
{
    ScDPCollection* pDPs = rDocShell.GetDocument()->GetDPCollection();
    if (!pDPs)
        return 0;

    std::set<ScDPObject*> aRefs;
    sal_uLong nErrId = pDPs->ReloadCache(pDPObj, aRefs);
    if (nErrId)
        return nErrId;

    std::set<ScDPObject*>::iterator it = aRefs.begin(), itEnd = aRefs.end();
    for (; it != itEnd; ++it)
    {
        ScDPObject* pObj = *it;

        
        UpdatePivotTable(*pObj, false, bApi);
    }

    return 0;
}

void ScDBDocFunc::RefreshPivotTableGroups(ScDPObject* pDPObj)
{
    if (!pDPObj)
        return;

    ScDPCollection* pDPs = rDocShell.GetDocument()->GetDPCollection();
    if (!pDPs)
        return;

    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    if (!pSaveData)
        return;

    std::set<ScDPObject*> aRefs;
    if (!pDPs->ReloadGroupsInCache(pDPObj, aRefs))
        return;

    
    const ScDPDimensionSaveData* pDimData = pSaveData->GetExistingDimensionData();
    std::set<ScDPObject*>::iterator it = aRefs.begin(), itEnd = aRefs.end();
    for (; it != itEnd; ++it)
    {
        ScDPObject* pObj = *it;
        if (pObj != pDPObj)
        {
            pSaveData = pObj->GetSaveData();
            if (pSaveData)
                pSaveData->SetDimensionData(pDimData);
        }

        
        UpdatePivotTable(*pObj, false, false);
    }
}


//


void ScDBDocFunc::UpdateImport( const OUString& rTarget, const svx::ODataAccessDescriptor& rDescriptor )
{
    

    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection& rDBColl = *pDoc->GetDBCollection();
    const ScDBData* pData = rDBColl.getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(rTarget));
    if (!pData)
    {
        InfoBox aInfoBox(rDocShell.GetActiveDialogParent(),
                    ScGlobal::GetRscString( STR_TARGETNOTFOUND ) );
        aInfoBox.Execute();
        return;
    }

    SCTAB nTab;
    SCCOL nDummyCol;
    SCROW nDummyRow;
    pData->GetArea( nTab, nDummyCol,nDummyRow,nDummyCol,nDummyRow );

    ScImportParam aImportParam;
    pData->GetImportParam( aImportParam );

    OUString sDBName;
    OUString sDBTable;
    sal_Int32 nCommandType = 0;
    sDBName = rDescriptor.getDataSource();
    rDescriptor[svx::daCommand]     >>= sDBTable;
    rDescriptor[svx::daCommandType] >>= nCommandType;

    aImportParam.aDBName    = sDBName;
    aImportParam.bSql       = ( nCommandType == sdb::CommandType::COMMAND );
    aImportParam.aStatement = sDBTable;
    aImportParam.bNative    = false;
    aImportParam.nType      = static_cast<sal_uInt8>( ( nCommandType == sdb::CommandType::QUERY ) ? ScDbQuery : ScDbTable );
    aImportParam.bImport    = true;

    bool bContinue = DoImport( nTab, aImportParam, &rDescriptor, true );

    

    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();
    if (pViewSh)
    {
        ScRange aRange;
        pData->GetArea(aRange);
        pViewSh->MarkRange(aRange);         

        if ( bContinue )        
        {
            

            if ( pData->HasQueryParam() || pData->HasSortParam() || pData->HasSubTotalParam() )
                pViewSh->RepeatDB();

            

            rDocShell.RefreshPivotTables(aRange);
        }
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
