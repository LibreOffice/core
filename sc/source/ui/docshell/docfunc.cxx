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

#include <scitems.hxx>

#include <comphelper/lok.hxx>
#include <sfx2/app.hxx>
#include <editeng/editobj.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/weld.hxx>
#include <vcl/waitobj.hxx>
#include <svx/svdocapt.hxx>
#include <sal/log.hxx>
#include <unotools/charclass.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>

#include <docfunc.hxx>

#include <sc.hrc>
#include <strings.hrc>

#include <arealink.hxx>
#include <attrib.hxx>
#include <dociter.hxx>
#include <autoform.hxx>
#include <formulacell.hxx>
#include <cellmergeoption.hxx>
#include <detdata.hxx>
#include <detfunc.hxx>
#include <docpool.hxx>
#include <docsh.hxx>
#include <drwlayer.hxx>
#include <editutil.hxx>
#include <globstr.hrc>
#include <olinetab.hxx>
#include <patattr.hxx>
#include <rangenam.hxx>
#include <refundo.hxx>
#include <scresid.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <tablink.hxx>
#include <tabvwsh.hxx>
#include <uiitems.hxx>
#include <undoblk.hxx>
#include <undocell.hxx>
#include <undodraw.hxx>
#include <undotab.hxx>
#include <sizedev.hxx>
#include <scmod.hxx>
#include <inputhdl.hxx>
#include <editable.hxx>
#include <compiler.hxx>
#include <scui_def.hxx>
#include <tabprotection.hxx>
#include <clipparam.hxx>
#include <externalrefmgr.hxx>
#include <undorangename.hxx>
#include <progress.hxx>
#include <dpobject.hxx>
#include <stringutil.hxx>
#include <cellvalue.hxx>
#include <tokenarray.hxx>
#include <rowheightcontext.hxx>
#include <cellvalues.hxx>
#include <undoconvert.hxx>
#include <docfuncutil.hxx>
#include <sheetevents.hxx>
#include <conditio.hxx>
#include <columnspanset.hxx>

#include <memory>
#include <utility>
#include <basic/basmgr.hxx>
#include <set>
#include <vector>

using namespace com::sun::star;
using ::std::vector;

void ScDocFunc::NotifyDrawUndo( std::unique_ptr<SdrUndoAction> pUndoAction)
{
    // #i101118# if drawing layer collects the undo actions, add it there
    ScDrawLayer* pDrawLayer = rDocShell.GetDocument().GetDrawLayer();
    if( pDrawLayer && pDrawLayer->IsRecording() )
        pDrawLayer->AddCalcUndo( std::move(pUndoAction) );
    else
        rDocShell.GetUndoManager()->AddUndoAction( std::make_unique<ScUndoDraw>( std::move(pUndoAction), &rDocShell ) );
    rDocShell.SetDrawModified();

    // the affected sheet isn't known, so all stream positions are invalidated
    ScDocument& rDoc = rDocShell.GetDocument();
    SCTAB nTabCount = rDoc.GetTableCount();
    for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        rDoc.SetStreamValid(nTab, false);
}

//  paint row above the range (because of lines after AdjustRowHeight)

static void lcl_PaintAbove( ScDocShell& rDocShell, const ScRange& rRange )
{
    SCROW nRow = rRange.aStart.Row();
    if ( nRow > 0 )
    {
        SCTAB nTab = rRange.aStart.Tab();   //! all of them?
        --nRow;
        rDocShell.PostPaint( ScRange(0,nRow,nTab, MAXCOL,nRow,nTab), PaintPartFlags::Grid );
    }
}

bool ScDocFunc::AdjustRowHeight( const ScRange& rRange, bool bPaint )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    if ( rDoc.IsImportingXML() )
    {
        //  for XML import, all row heights are updated together after importing
        return false;
    }
    if ( rDoc.IsAdjustHeightLocked() )
    {
        return false;
    }

    SCTAB nTab      = rRange.aStart.Tab();
    SCROW nStartRow = rRange.aStart.Row();
    SCROW nEndRow   = rRange.aEnd.Row();

    ScSizeDeviceProvider aProv( &rDocShell );
    Fraction aOne(1,1);

    sc::RowHeightContext aCxt(aProv.GetPPTX(), aProv.GetPPTY(), aOne, aOne, aProv.GetDevice());
    bool bChanged = rDoc.SetOptimalHeight(aCxt, nStartRow, nEndRow, nTab);
    // tdf#76183: recalculate objects' positions
    if (bChanged)
        rDoc.SetDrawPageSize(nTab);

    if ( bPaint && bChanged )
        rDocShell.PostPaint(ScRange(0, nStartRow, nTab, MAXCOL, MAXROW, nTab),
                            PaintPartFlags::Grid | PaintPartFlags::Left);

    return bChanged;
}

bool ScDocFunc::DetectiveAddPred(const ScAddress& rPos)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bUndo (rDoc.IsUndoEnabled());
    ScDrawLayer* pModel = rDoc.GetDrawLayer();
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo(false);
    bool bDone = ScDetectiveFunc( &rDoc,nTab ).ShowPred( nCol, nRow );
    std::unique_ptr<SdrUndoGroup> pUndo;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_ADDPRED );
        rDoc.AddDetectiveOperation( aOperation );
        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoDetective>( &rDocShell, std::move(pUndo), &aOperation ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }

    return bDone;
}

bool ScDocFunc::DetectiveDelPred(const ScAddress& rPos)
{
    ScDocument& rDoc = rDocShell.GetDocument();

    bool bUndo(rDoc.IsUndoEnabled());
    ScDrawLayer* pModel = rDoc.GetDrawLayer();
    if (!pModel)
        return false;

    ScDocShellModificator aModificator( rDocShell );

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo(false);
    bool bDone = ScDetectiveFunc( &rDoc,nTab ).DeletePred( nCol, nRow );
    std::unique_ptr<SdrUndoGroup> pUndo;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_DELPRED );
        rDoc.AddDetectiveOperation( aOperation );
        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoDetective>( &rDocShell, std::move(pUndo), &aOperation ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }

    return bDone;
}

bool ScDocFunc::DetectiveAddSucc(const ScAddress& rPos)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument& rDoc = rDocShell.GetDocument();

    bool bUndo(rDoc.IsUndoEnabled());
    ScDrawLayer* pModel = rDoc.GetDrawLayer();
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo(false);
    bool bDone = ScDetectiveFunc( &rDoc,nTab ).ShowSucc( nCol, nRow );
    std::unique_ptr<SdrUndoGroup> pUndo;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_ADDSUCC );
        rDoc.AddDetectiveOperation( aOperation );
        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoDetective>( &rDocShell, std::move(pUndo), &aOperation ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }

    return bDone;
}

bool ScDocFunc::DetectiveDelSucc(const ScAddress& rPos)
{
    ScDocument& rDoc = rDocShell.GetDocument();

    bool bUndo (rDoc.IsUndoEnabled());
    ScDrawLayer* pModel = rDoc.GetDrawLayer();
    if (!pModel)
        return false;

    ScDocShellModificator aModificator( rDocShell );

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo(false);
    bool bDone = ScDetectiveFunc( &rDoc,nTab ).DeleteSucc( nCol, nRow );
    std::unique_ptr<SdrUndoGroup> pUndo;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_DELSUCC );
        rDoc.AddDetectiveOperation( aOperation );
        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoDetective>( &rDocShell, std::move(pUndo), &aOperation ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }

    return bDone;
}

bool ScDocFunc::DetectiveAddError(const ScAddress& rPos)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument& rDoc = rDocShell.GetDocument();

    bool bUndo (rDoc.IsUndoEnabled());
    ScDrawLayer* pModel = rDoc.GetDrawLayer();
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo(false);
    bool bDone = ScDetectiveFunc( &rDoc,nTab ).ShowError( nCol, nRow );
    std::unique_ptr<SdrUndoGroup> pUndo;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_ADDERROR );
        rDoc.AddDetectiveOperation( aOperation );
        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoDetective>( &rDocShell, std::move(pUndo), &aOperation ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }

    return bDone;
}

bool ScDocFunc::DetectiveMarkInvalid(SCTAB nTab)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument& rDoc = rDocShell.GetDocument();

    bool bUndo (rDoc.IsUndoEnabled());
    ScDrawLayer* pModel = rDoc.GetDrawLayer();

    vcl::Window* pWaitWin = ScDocShell::GetActiveDialogParent();
    if (pWaitWin)
        pWaitWin->EnterWait();
    if (bUndo)
        pModel->BeginCalcUndo(false);
    bool bOverflow;
    bool bDone = ScDetectiveFunc( &rDoc,nTab ).MarkInvalid( bOverflow );
    std::unique_ptr<SdrUndoGroup> pUndo;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (pWaitWin)
        pWaitWin->LeaveWait();
    if (bDone)
    {
        if (pUndo && bUndo)
        {
            pUndo->SetComment( ScResId( STR_UNDO_DETINVALID ) );
            rDocShell.GetUndoManager()->AddUndoAction( std::move(pUndo) );
        }
        aModificator.SetDocumentModified();
        if ( bOverflow )
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(nullptr,
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          ScResId(STR_DETINVALID_OVERFLOW)));
            xInfoBox->run();
        }
    }

    return bDone;
}

bool ScDocFunc::DetectiveDelAll(SCTAB nTab)
{
    ScDocument& rDoc = rDocShell.GetDocument();

    bool bUndo (rDoc.IsUndoEnabled());
    ScDrawLayer* pModel = rDoc.GetDrawLayer();
    if (!pModel)
        return false;

    ScDocShellModificator aModificator( rDocShell );

    if (bUndo)
        pModel->BeginCalcUndo(false);
    bool bDone = ScDetectiveFunc( &rDoc,nTab ).DeleteAll( ScDetectiveDelete::Detective );
    std::unique_ptr<SdrUndoGroup> pUndo;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpList* pOldList = rDoc.GetDetOpList();
        std::unique_ptr<ScDetOpList> pUndoList;
        if (bUndo && pOldList)
            pUndoList.reset(new ScDetOpList(*pOldList));

        rDoc.ClearDetectiveOperations();

        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoDetective>( &rDocShell, std::move(pUndo), nullptr, std::move(pUndoList) ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }

    return bDone;
}

bool ScDocFunc::DetectiveRefresh( bool bAutomatic )
{
    bool bDone = false;
    ScDocument& rDoc = rDocShell.GetDocument();

    ScDetOpList* pList = rDoc.GetDetOpList();
    if ( pList && pList->Count() )
    {
        rDocShell.MakeDrawLayer();
        ScDrawLayer* pModel = rDoc.GetDrawLayer();
        const bool bUndo (rDoc.IsUndoEnabled());
        if (bUndo)
            pModel->BeginCalcUndo(false);

        //  Delete in all sheets

        SCTAB nTabCount = rDoc.GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
            ScDetectiveFunc( &rDoc,nTab ).DeleteAll( ScDetectiveDelete::Arrows );    // don't remove circles

        //  repeat

        size_t nCount = pList->Count();
        for (size_t i=0; i < nCount; ++i)
        {
            const ScDetOpData& rData = pList->GetObject(i);
            const ScAddress& aPos = rData.GetPos();
            ScDetectiveFunc aFunc( &rDoc, aPos.Tab() );
            SCCOL nCol = aPos.Col();
            SCROW nRow = aPos.Row();
            switch (rData.GetOperation())
            {
                case SCDETOP_ADDSUCC:
                    aFunc.ShowSucc( nCol, nRow );
                    break;
                case SCDETOP_DELSUCC:
                    aFunc.DeleteSucc( nCol, nRow );
                    break;
                case SCDETOP_ADDPRED:
                     aFunc.ShowPred( nCol, nRow );
                     break;
                case SCDETOP_DELPRED:
                    aFunc.DeletePred( nCol, nRow );
                    break;
                case SCDETOP_ADDERROR:
                    aFunc.ShowError( nCol, nRow );
                    break;
                default:
                    OSL_FAIL("wrong operation in DetectiveRefresh");
            }
        }

        if (bUndo)
        {
            std::unique_ptr<SdrUndoGroup> pUndo = pModel->GetCalcUndo();
            if (pUndo)
            {
                pUndo->SetComment( ScResId( STR_UNDO_DETREFRESH ) );
                // associate with the last action
                rDocShell.GetUndoManager()->AddUndoAction(
                                                std::make_unique<ScUndoDraw>( std::move(pUndo), &rDocShell ),
                                                    bAutomatic );
            }
        }
        rDocShell.SetDrawModified();
        bDone = true;
    }
    return bDone;
}

static void lcl_collectAllPredOrSuccRanges(
    const ScRangeList& rSrcRanges, vector<ScTokenRef>& rRefTokens, ScDocShell& rDocShell,
    bool bPred)
{
    ScDocument& rDoc = rDocShell.GetDocument();
    vector<ScTokenRef> aRefTokens;
    if (rSrcRanges.empty())
        return;
    ScRange const & rFrontRange = rSrcRanges.front();
    ScDetectiveFunc aDetFunc(&rDoc, rFrontRange.aStart.Tab());
    for (size_t i = 0, n = rSrcRanges.size(); i < n; ++i)
    {
        ScRange const & r = rSrcRanges[i];
        if (bPred)
        {
            aDetFunc.GetAllPreds(
                r.aStart.Col(), r.aStart.Row(), r.aEnd.Col(), r.aEnd.Row(), aRefTokens);
        }
        else
        {
            aDetFunc.GetAllSuccs(
                r.aStart.Col(), r.aStart.Row(), r.aEnd.Col(), r.aEnd.Row(), aRefTokens);
        }
    }
    rRefTokens.swap(aRefTokens);
}

void ScDocFunc::DetectiveCollectAllPreds(const ScRangeList& rSrcRanges, vector<ScTokenRef>& rRefTokens)
{
    lcl_collectAllPredOrSuccRanges(rSrcRanges, rRefTokens, rDocShell, true);
}

void ScDocFunc::DetectiveCollectAllSuccs(const ScRangeList& rSrcRanges, vector<ScTokenRef>& rRefTokens)
{
    lcl_collectAllPredOrSuccRanges(rSrcRanges, rRefTokens, rDocShell, false);
}

bool ScDocFunc::DeleteContents(
    const ScMarkData& rMark, InsertDeleteFlags nFlags, bool bRecord, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        OSL_FAIL("ScDocFunc::DeleteContents without markings");
        return false;
    }

    ScDocument& rDoc = rDocShell.GetDocument();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScEditableTester aTester( &rDoc, rMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    ScRange aMarkRange;

    ScMarkData aMultiMark = rMark;
    aMultiMark.SetMarking(false);       // for MarkToMulti

    ScDocumentUniquePtr pUndoDoc;
    bool bMulti = aMultiMark.IsMultiMarked();
    aMultiMark.MarkToMulti();
    aMultiMark.GetMultiMarkArea( aMarkRange );
    ScRange aExtendedRange(aMarkRange);
    if ( rDoc.ExtendMerge( aExtendedRange, true ) )
        bMulti = false;

    // no objects on protected tabs
    bool bObjects = (nFlags & InsertDeleteFlags::OBJECTS) && !sc::DocFuncUtil::hasProtectedTab(rDoc, rMark);

    sal_uInt16 nExtFlags = 0;       // extra flags are needed only if attributes are deleted
    if ( nFlags & InsertDeleteFlags::ATTRIB )
        rDocShell.UpdatePaintExt( nExtFlags, aMarkRange );

    //  order of operations:
    //  1) BeginDrawUndo
    //  2) Delete objects (DrawUndo will be filled)
    //  3) Copy content for undo and set up undo actions
    //  4) Delete content

    bool bDrawUndo = bObjects || (nFlags & InsertDeleteFlags::NOTE);
    if (bRecord && bDrawUndo)
        rDoc.BeginDrawUndo();

    if (bObjects)
    {
        if (bMulti)
            rDoc.DeleteObjectsInSelection( aMultiMark );
        else
            rDoc.DeleteObjectsInArea( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                                       aMarkRange.aEnd.Col(),   aMarkRange.aEnd.Row(),
                                       aMultiMark );
    }

    // To keep track of all non-empty cells within the deleted area.
    std::shared_ptr<ScSimpleUndo::DataSpansType> pDataSpans;

    if ( bRecord )
    {
        pUndoDoc = sc::DocFuncUtil::createDeleteContentsUndoDoc(rDoc, aMultiMark, aMarkRange, nFlags, bMulti);
        pDataSpans = sc::DocFuncUtil::getNonEmptyCellSpans(rDoc, aMultiMark, aMarkRange);
    }

    rDoc.DeleteSelection( nFlags, aMultiMark );

    // add undo action after drawing undo is complete (objects and note captions)
    if( bRecord )
    {
        sc::DocFuncUtil::addDeleteContentsUndo(
            rDocShell.GetUndoManager(), &rDocShell, aMultiMark, aExtendedRange,
            std::move(pUndoDoc), nFlags, pDataSpans, bMulti, bDrawUndo);
    }

    if (!AdjustRowHeight( aExtendedRange ))
        rDocShell.PostPaint( aExtendedRange, PaintPartFlags::Grid, nExtFlags );
    else if (nExtFlags & SC_PF_LINES)
        lcl_PaintAbove( rDocShell, aExtendedRange );    // for lines above the range

    aModificator.SetDocumentModified();

    return true;
}

bool ScDocFunc::DeleteCell(
    const ScAddress& rPos, const ScMarkData& rMark, InsertDeleteFlags nFlags, bool bRecord )
{
    ScDocShellModificator aModificator(rDocShell);

    ScDocument& rDoc = rDocShell.GetDocument();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScEditableTester aTester(&rDoc, rPos.Col(), rPos.Row(), rPos.Col(), rPos.Row(), rMark);
    if (!aTester.IsEditable())
    {
        rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    // no objects on protected tabs
    bool bObjects = (nFlags & InsertDeleteFlags::OBJECTS) && !sc::DocFuncUtil::hasProtectedTab(rDoc, rMark);

    sal_uInt16 nExtFlags = 0;       // extra flags are needed only if attributes are deleted
    if (nFlags & InsertDeleteFlags::ATTRIB)
        rDocShell.UpdatePaintExt(nExtFlags, rPos);

    //  order of operations:
    //  1) BeginDrawUndo
    //  2) delete objects (DrawUndo is filled)
    //  3) copy contents for undo
    //  4) delete contents
    //  5) add undo-action

    bool bDrawUndo = bObjects || (nFlags & InsertDeleteFlags::NOTE);     // needed for shown notes
    if (bDrawUndo && bRecord)
        rDoc.BeginDrawUndo();

    if (bObjects)
        rDoc.DeleteObjectsInArea(rPos.Col(), rPos.Row(), rPos.Col(), rPos.Row(), rMark);

    // To keep track of all non-empty cells within the deleted area.
    std::shared_ptr<ScSimpleUndo::DataSpansType> pDataSpans;

    ScDocumentUniquePtr pUndoDoc;
    if (bRecord)
    {
        pUndoDoc = sc::DocFuncUtil::createDeleteContentsUndoDoc(rDoc, rMark, rPos, nFlags, false);
        pDataSpans = sc::DocFuncUtil::getNonEmptyCellSpans(rDoc, rMark, rPos);
    }

    rDoc.DeleteArea(rPos.Col(), rPos.Row(), rPos.Col(), rPos.Row(), rMark, nFlags);

    if (bRecord)
    {
        sc::DocFuncUtil::addDeleteContentsUndo(
            rDocShell.GetUndoManager(), &rDocShell, rMark, rPos, std::move(pUndoDoc),
            nFlags, pDataSpans, false, bDrawUndo);
    }

    if (!AdjustRowHeight(rPos))
        rDocShell.PostPaint(
            rPos.Col(), rPos.Row(), rPos.Tab(), rPos.Col(), rPos.Row(), rPos.Tab(),
            PaintPartFlags::Grid, nExtFlags);

    aModificator.SetDocumentModified();

    return true;
}

bool ScDocFunc::TransliterateText( const ScMarkData& rMark, TransliterationFlags nType,
                                    bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    ScEditableTester aTester( &rDoc, rMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    ScRange aMarkRange;
    ScMarkData aMultiMark = rMark;
    aMultiMark.SetMarking(false);       // for MarkToMulti
    aMultiMark.MarkToMulti();
    aMultiMark.GetMultiMarkArea( aMarkRange );

    if (bRecord)
    {
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCTAB nTabCount = rDoc.GetTableCount();

        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( &rDoc, nStartTab, nStartTab );
        for (const auto& rTab : rMark)
        {
            if (rTab >= nTabCount)
                break;

            if (rTab != nStartTab)
                pUndoDoc->AddUndoTab( rTab, rTab );
        }

        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        rDoc.CopyToDocument(aCopyRange, InsertDeleteFlags::CONTENTS, true, *pUndoDoc, &aMultiMark);

        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoTransliterate>( &rDocShell, aMultiMark, std::move(pUndoDoc), nType ) );
    }

    rDoc.TransliterateText( aMultiMark, nType );

    if (!AdjustRowHeight( aMarkRange ))
        rDocShell.PostPaint( aMarkRange, PaintPartFlags::Grid );

    aModificator.SetDocumentModified();

    return true;
}

bool ScDocFunc::SetNormalString( bool& o_rbNumFmtSet, const ScAddress& rPos, const OUString& rText, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument& rDoc = rDocShell.GetDocument();

    bool bUndo(rDoc.IsUndoEnabled());
    ScEditableTester aTester( &rDoc, rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    bool bEditDeleted = (rDoc.GetCellType(rPos) == CELLTYPE_EDIT);
    ScUndoEnterData::ValuesType aOldValues;

    if (bUndo)
    {
        ScUndoEnterData::Value aOldValue;

        aOldValue.mnTab = rPos.Tab();
        aOldValue.maCell.assign(rDoc, rPos);

        const SfxPoolItem* pItem;
        const ScPatternAttr* pPattern = rDoc.GetPattern( rPos.Col(),rPos.Row(),rPos.Tab() );
        if ( SfxItemState::SET == pPattern->GetItemSet().GetItemState(
                                ATTR_VALUE_FORMAT,false,&pItem) )
        {
            aOldValue.mbHasFormat = true;
            aOldValue.mnFormat = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
        }
        else
            aOldValue.mbHasFormat = false;

        aOldValues.push_back(aOldValue);
    }

    o_rbNumFmtSet = rDoc.SetString( rPos.Col(), rPos.Row(), rPos.Tab(), rText );

    if (bUndo)
    {
        //  because of ChangeTracking, UndoAction can be created only after SetString was called
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoEnterData>(&rDocShell, rPos, aOldValues, rText, nullptr));
    }

    if ( bEditDeleted || rDoc.HasAttrib( ScRange(rPos), HasAttrFlags::NeedHeight ) )
        AdjustRowHeight( ScRange(rPos) );

    rDocShell.PostPaintCell( rPos );
    aModificator.SetDocumentModified();

    // notify input handler here the same way as in PutCell
    if (bApi)
        NotifyInputHandler( rPos );

    return true;
}

bool ScDocFunc::SetValueCell( const ScAddress& rPos, double fVal, bool bInteraction )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bUndo = rDoc.IsUndoEnabled();

    bool bHeight = rDoc.HasAttrib(rPos, HasAttrFlags::NeedHeight);

    ScCellValue aOldVal;
    if (bUndo)
        aOldVal.assign(rDoc, rPos);

    rDoc.SetValue(rPos, fVal);

    if (bUndo)
    {
        SfxUndoManager* pUndoMgr = rDocShell.GetUndoManager();
        ScCellValue aNewVal;
        aNewVal.assign(rDoc, rPos);
        pUndoMgr->AddUndoAction(std::make_unique<ScUndoSetCell>(&rDocShell, rPos, aOldVal, aNewVal));
    }

    if (bHeight)
        AdjustRowHeight(rPos);

    rDocShell.PostPaintCell( rPos );
    aModificator.SetDocumentModified();

    // #103934#; notify editline and cell in edit mode
    if (!bInteraction)
        NotifyInputHandler( rPos );

    return true;
}

void ScDocFunc::SetValueCells( const ScAddress& rPos, const std::vector<double>& aVals, bool bInteraction )
{
    // Check for invalid range.
    SCROW nLastRow = rPos.Row() + aVals.size() - 1;
    if (nLastRow > MAXROW)
        // out of bound.
        return;

    ScRange aRange(rPos);
    aRange.aEnd.SetRow(nLastRow);

    ScDocShellModificator aModificator(rDocShell);
    ScDocument& rDoc = rDocShell.GetDocument();

    if (rDoc.IsUndoEnabled())
    {
        std::unique_ptr<sc::UndoSetCells> pUndoObj(new sc::UndoSetCells(&rDocShell, rPos));
        rDoc.TransferCellValuesTo(rPos, aVals.size(), pUndoObj->GetOldValues());
        pUndoObj->SetNewValues(aVals);
        SfxUndoManager* pUndoMgr = rDocShell.GetUndoManager();
        pUndoMgr->AddUndoAction(std::move(pUndoObj));
    }

    rDoc.SetValues(rPos, aVals);

    rDocShell.PostPaint(aRange, PaintPartFlags::Grid);
    aModificator.SetDocumentModified();

    // #103934#; notify editline and cell in edit mode
    if (!bInteraction)
        NotifyInputHandler(rPos);
}

bool ScDocFunc::SetStringCell( const ScAddress& rPos, const OUString& rStr, bool bInteraction )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bUndo = rDoc.IsUndoEnabled();

    bool bHeight = rDoc.HasAttrib(rPos, HasAttrFlags::NeedHeight);

    ScCellValue aOldVal;
    if (bUndo)
        aOldVal.assign(rDoc, rPos);

    ScSetStringParam aParam;
    aParam.setTextInput();
    rDoc.SetString(rPos, rStr, &aParam);

    if (bUndo)
    {
        SfxUndoManager* pUndoMgr = rDocShell.GetUndoManager();
        ScCellValue aNewVal;
        aNewVal.assign(rDoc, rPos);
        pUndoMgr->AddUndoAction(std::make_unique<ScUndoSetCell>(&rDocShell, rPos, aOldVal, aNewVal));
    }

    if (bHeight)
        AdjustRowHeight(rPos);

    rDocShell.PostPaintCell( rPos );
    aModificator.SetDocumentModified();

    // #103934#; notify editline and cell in edit mode
    if (!bInteraction)
        NotifyInputHandler( rPos );

    return true;
}

bool ScDocFunc::SetEditCell( const ScAddress& rPos, const EditTextObject& rStr, bool bInteraction )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bUndo = rDoc.IsUndoEnabled();

    bool bHeight = rDoc.HasAttrib(rPos, HasAttrFlags::NeedHeight);

    ScCellValue aOldVal;
    if (bUndo)
        aOldVal.assign(rDoc, rPos);

    rDoc.SetEditText(rPos, rStr.Clone());

    if (bUndo)
    {
        SfxUndoManager* pUndoMgr = rDocShell.GetUndoManager();
        ScCellValue aNewVal;
        aNewVal.assign(rDoc, rPos);
        pUndoMgr->AddUndoAction(std::make_unique<ScUndoSetCell>(&rDocShell, rPos, aOldVal, aNewVal));
    }

    if (bHeight)
        AdjustRowHeight(rPos);

    rDocShell.PostPaintCell( rPos );
    aModificator.SetDocumentModified();

    // #103934#; notify editline and cell in edit mode
    if (!bInteraction)
        NotifyInputHandler( rPos );

    return true;
}

bool ScDocFunc::SetStringOrEditCell( const ScAddress& rPos, const OUString& rStr, bool bInteraction )
{
    ScDocument& rDoc = rDocShell.GetDocument();

    if (ScStringUtil::isMultiline(rStr))
    {
        ScFieldEditEngine& rEngine = rDoc.GetEditEngine();
        rEngine.SetText(rStr);
        std::unique_ptr<EditTextObject> pEditText(rEngine.CreateTextObject());
        return SetEditCell(rPos, *pEditText, bInteraction);
    }
    else
        return SetStringCell(rPos, rStr, bInteraction);
}

bool ScDocFunc::SetFormulaCell( const ScAddress& rPos, ScFormulaCell* pCell, bool bInteraction )
{
    std::unique_ptr<ScFormulaCell> xCell(pCell);

    ScDocShellModificator aModificator( rDocShell );
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bUndo = rDoc.IsUndoEnabled();

    bool bHeight = rDoc.HasAttrib(rPos, HasAttrFlags::NeedHeight);

    ScCellValue aOldVal;
    if (bUndo)
        aOldVal.assign(rDoc, rPos);

    pCell = rDoc.SetFormulaCell(rPos, xCell.release());

    // For performance reasons API calls may disable calculation while
    // operating and recalculate once when done. If through user interaction
    // and AutoCalc is disabled, calculate the formula (without its
    // dependencies) once so the result matches the current document's content.
    if (bInteraction && !rDoc.GetAutoCalc() && pCell)
    {
        // calculate just the cell once and set Dirty again
        pCell->Interpret();
        pCell->SetDirtyVar();
        rDoc.PutInFormulaTree( pCell);
    }

    if (bUndo)
    {
        SfxUndoManager* pUndoMgr = rDocShell.GetUndoManager();
        ScCellValue aNewVal;
        aNewVal.assign(rDoc, rPos);
        pUndoMgr->AddUndoAction(std::make_unique<ScUndoSetCell>(&rDocShell, rPos, aOldVal, aNewVal));
    }

    if (bHeight)
        AdjustRowHeight(rPos);

    rDocShell.PostPaintCell( rPos );
    aModificator.SetDocumentModified();

    // #103934#; notify editline and cell in edit mode
    if (!bInteraction)
        NotifyInputHandler( rPos );

    return true;
}

void ScDocFunc::NotifyInputHandler( const ScAddress& rPos )
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if ( pViewSh && pViewSh->GetViewData().GetDocShell() == &rDocShell )
    {
        ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
        if ( pInputHdl && pInputHdl->GetCursorPos() == rPos )
        {
            bool bIsEditMode(pInputHdl->IsEditMode());

            // set modified if in editmode, because so the string is not set in the InputWindow like in the cell
            // (the cell shows the same like the InputWindow)
            if (bIsEditMode)
                pInputHdl->SetModified();
            pViewSh->UpdateInputHandler(false, !bIsEditMode);
        }
    }
}

        struct ScMyRememberItem
        {
            sal_Int32 const   nIndex;
            SfxItemSet const  aItemSet;

            ScMyRememberItem(const SfxItemSet& rItemSet, sal_Int32 nTempIndex) :
                nIndex(nTempIndex), aItemSet(rItemSet) {}
        };

        typedef ::std::vector<std::unique_ptr<ScMyRememberItem>> ScMyRememberItemVector;

void ScDocFunc::PutData( const ScAddress& rPos, ScEditEngineDefaulter& rEngine, bool bApi )
{
    //  PutData calls PutCell or SetNormalString

    bool bRet = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    ScEditAttrTester aTester( &rEngine );
    bool bEditCell = aTester.NeedsObject();
    if ( bEditCell )
    {
        // #i61702# With bLoseContent set, the content of rEngine isn't restored
        // (used in loading XML, where after the removeActionLock call the API object's
        // EditEngine isn't accessed again.
        bool bLoseContent = rDoc.IsImportingXML();

        bool bUpdateMode(rEngine.GetUpdateMode());
        if (bUpdateMode)
            rEngine.SetUpdateMode(false);

        ScMyRememberItemVector aRememberItems;

        //  All paragraph attributes must be removed before calling CreateTextObject,
        //  not only alignment, so the object doesn't contain the cell attributes as
        //  paragraph attributes. Before removing the attributes store them in a vector to
        //  set them back to the EditEngine.
        sal_Int32 nCount = rEngine.GetParagraphCount();
        for (sal_Int32 i=0; i<nCount; i++)
        {
            const SfxItemSet& rOld = rEngine.GetParaAttribs( i );
            if ( rOld.Count() )
            {
                if ( !bLoseContent )
                {
                    aRememberItems.push_back(std::make_unique<ScMyRememberItem>(rEngine.GetParaAttribs(i), i));
                }
                rEngine.SetParaAttribs( i, SfxItemSet( *rOld.GetPool(), rOld.GetRanges() ) );
            }
        }

        // A copy of pNewData will be stored in the cell.
        std::unique_ptr<EditTextObject> pNewData(rEngine.CreateTextObject());
        bRet = SetEditCell(rPos, *pNewData, !bApi);

        // Set the paragraph attributes back to the EditEngine.
        for (const auto& rxItem : aRememberItems)
        {
            rEngine.SetParaAttribs(rxItem->nIndex, rxItem->aItemSet);
        }

        // #i61702# if the content isn't accessed, there's no need to set the UpdateMode again
        if ( bUpdateMode && !bLoseContent )
            rEngine.SetUpdateMode(true);
    }
    else
    {
        OUString aText = rEngine.GetText();
        if (aText.isEmpty())
        {
            bool bNumFmtSet = false;
            bRet = SetNormalString( bNumFmtSet, rPos, aText, bApi );
        }
        else
            bRet = SetStringCell(rPos, aText, !bApi);
    }

    if ( bRet && aTester.NeedsCellAttr() )
    {
        const SfxItemSet& rEditAttr = aTester.GetAttribs();
        ScPatternAttr aPattern( rDoc.GetPool() );
        aPattern.GetFromEditItemSet( &rEditAttr );
        aPattern.DeleteUnchanged( rDoc.GetPattern( rPos.Col(), rPos.Row(), rPos.Tab() ) );
        aPattern.GetItemSet().ClearItem( ATTR_HOR_JUSTIFY );    // wasn't removed above if no edit object
        if ( aPattern.GetItemSet().Count() > 0 )
        {
            ScMarkData aMark;
            aMark.SelectTable( rPos.Tab(), true );
            aMark.SetMarkArea( ScRange( rPos ) );
            ApplyAttributes( aMark, aPattern, bApi );
        }
    }
}

bool ScDocFunc::SetCellText(
    const ScAddress& rPos, const OUString& rText, bool bInterpret, bool bEnglish, bool bApi,
    const formula::FormulaGrammar::Grammar eGrammar )
{
    bool bSet = false;
    if ( bInterpret )
    {
        if ( bEnglish )
        {
            ScDocument& rDoc = rDocShell.GetDocument();

            ::std::unique_ptr<ScExternalRefManager::ApiGuard> pExtRefGuard;
            if (bApi)
                pExtRefGuard.reset(new ScExternalRefManager::ApiGuard(&rDoc));

            ScInputStringType aRes =
                ScStringUtil::parseInputString(*rDoc.GetFormatTable(), rText, LANGUAGE_ENGLISH_US);

            switch (aRes.meType)
            {
                case ScInputStringType::Formula:
                    bSet = SetFormulaCell(rPos, new ScFormulaCell(&rDoc, rPos, aRes.maText, eGrammar), !bApi);
                break;
                case ScInputStringType::Number:
                    bSet = SetValueCell(rPos, aRes.mfValue, !bApi);
                break;
                case ScInputStringType::Text:
                    bSet = SetStringOrEditCell(rPos, aRes.maText, !bApi);
                break;
                default:
                    ;
            }
        }
        // otherwise keep Null -> SetString with local formulas/number formats
    }
    else if (!rText.isEmpty())
    {
        bSet = SetStringOrEditCell(rPos, rText, !bApi);
    }

    if (!bSet)
    {
        bool bNumFmtSet = false;
        bSet = SetNormalString( bNumFmtSet, rPos, rText, bApi );
    }
    return bSet;
}

bool ScDocFunc::ShowNote( const ScAddress& rPos, bool bShow )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    ScPostIt* pNote = rDoc.GetNote( rPos );
    if( !pNote || (bShow == pNote->IsCaptionShown()) ||
        (comphelper::LibreOfficeKit::isActive() && !comphelper::LibreOfficeKit::isTiledAnnotations()) )
        return false;

    // move the caption to internal or hidden layer and create undo action
    pNote->ShowCaption( rPos, bShow );
    if( rDoc.IsUndoEnabled() )
        rDocShell.GetUndoManager()->AddUndoAction( std::make_unique<ScUndoShowHideNote>( rDocShell, rPos, bShow ) );

    rDoc.SetStreamValid(rPos.Tab(), false);

    ScTabView::OnLOKNoteStateChanged(pNote);

    if (ScViewData* pViewData = ScDocShell::GetViewData())
    {
        if (ScDrawView* pDrawView = pViewData->GetScDrawView())
            pDrawView->SyncForGrid( pNote->GetCaption());
    }

    rDocShell.SetDocumentModified();

    return true;
}

void ScDocFunc::SetNoteText( const ScAddress& rPos, const OUString& rText, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    ScEditableTester aTester( &rDoc, rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return;
    }

    OUString aNewText = convertLineEnd(rText, GetSystemLineEnd()); //! is this necessary ???

    if( ScPostIt* pNote = (!aNewText.isEmpty()) ? rDoc.GetOrCreateNote( rPos ) : rDoc.GetNote(rPos) )
        pNote->SetText( rPos, aNewText );

    //! Undo !!!

    rDoc.SetStreamValid(rPos.Tab(), false);

    rDocShell.PostPaintCell( rPos );
    aModificator.SetDocumentModified();
}

void ScDocFunc::ReplaceNote( const ScAddress& rPos, const OUString& rNoteText, const OUString* pAuthor, const OUString* pDate, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument& rDoc = rDocShell.GetDocument();
    ScEditableTester aTester( &rDoc, rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() );
    if (aTester.IsEditable())
    {
        ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
        SfxUndoManager* pUndoMgr = (pDrawLayer && rDoc.IsUndoEnabled()) ? rDocShell.GetUndoManager() : nullptr;

        ScNoteData aOldData;
        std::unique_ptr<ScPostIt> pOldNote = rDoc.ReleaseNote( rPos );
        sal_uInt32 nNoteId = 0;
        if( pOldNote )
        {
            nNoteId = pOldNote->GetId();
            // ensure existing caption object before draw undo tracking starts
            pOldNote->GetOrCreateCaption( rPos );
            // rescue note data for undo
            aOldData = pOldNote->GetNoteData();
        }

        // collect drawing undo actions for deleting/inserting caption objects
        if( pUndoMgr )
            pDrawLayer->BeginCalcUndo(false);

        // delete the note (creates drawing undo action for the caption object)
        bool hadOldNote(pOldNote);
        pOldNote.reset();

        // create new note (creates drawing undo action for the new caption object)
        ScNoteData aNewData;
        ScPostIt* pNewNote = nullptr;
        if( (pNewNote = ScNoteUtil::CreateNoteFromString( rDoc, rPos, rNoteText, false, true, nNoteId )) )
        {
            if( pAuthor ) pNewNote->SetAuthor( *pAuthor );
            if( pDate ) pNewNote->SetDate( *pDate );

            // rescue note data for undo
            aNewData = pNewNote->GetNoteData();
        }

        // create the undo action
        if( pUndoMgr && (aOldData.mxCaption || aNewData.mxCaption) )
            pUndoMgr->AddUndoAction( std::make_unique<ScUndoReplaceNote>( rDocShell, rPos, aOldData, aNewData, pDrawLayer->GetCalcUndo() ) );

        // repaint cell (to make note marker visible)
        rDocShell.PostPaintCell( rPos );

        rDoc.SetStreamValid(rPos.Tab(), false);

        aModificator.SetDocumentModified();

        // Let our LOK clients know about the new/modified note
        if (pNewNote)
        {
            ScDocShell::LOKCommentNotify(hadOldNote ? LOKCommentNotificationType::Modify : LOKCommentNotificationType::Add,
                                         &rDoc, rPos, pNewNote);
        }
    }
    else if (!bApi)
    {
        rDocShell.ErrorMessage(aTester.GetMessageId());
    }
}

bool ScDocFunc::ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
                                    bool bApi )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bRecord = true;
    if ( !rDoc.IsUndoEnabled() )
        bRecord = false;

    bool bImportingXML = rDoc.IsImportingXML();
    // Cell formats can still be set if the range isn't editable only because of matrix formulas.
    // #i62483# When loading XML, the check can be skipped altogether.
    bool bOnlyNotBecauseOfMatrix;
    if ( !bImportingXML && !rDoc.IsSelectionEditable( rMark, &bOnlyNotBecauseOfMatrix )
            && !bOnlyNotBecauseOfMatrix )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return false;
    }

    ScDocShellModificator aModificator( rDocShell );

    //! Border

    ScRange aMultiRange;
    bool bMulti = rMark.IsMultiMarked();
    if ( bMulti )
        rMark.GetMultiMarkArea( aMultiRange );
    else
        rMark.GetMarkArea( aMultiRange );

    if ( bRecord )
    {
        ScDocumentUniquePtr pUndoDoc( new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( &rDoc, aMultiRange.aStart.Tab(), aMultiRange.aEnd.Tab() );
        rDoc.CopyToDocument(aMultiRange, InsertDeleteFlags::ATTRIB, bMulti, *pUndoDoc, &rMark);

        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoSelectionAttr>(
                    &rDocShell, rMark,
                    aMultiRange.aStart.Col(), aMultiRange.aStart.Row(), aMultiRange.aStart.Tab(),
                    aMultiRange.aEnd.Col(), aMultiRange.aEnd.Row(), aMultiRange.aEnd.Tab(),
                    std::move(pUndoDoc), bMulti, &rPattern ) );
    }

    // While loading XML it is not necessary to ask HasAttrib. It needs too much time.
    sal_uInt16 nExtFlags = 0;
    if ( !bImportingXML )
        rDocShell.UpdatePaintExt( nExtFlags, aMultiRange );     // content before the change

    bool bChanged = false;
    rDoc.ApplySelectionPattern( rPattern, rMark, nullptr, &bChanged );

    if(bChanged)
    {
        if ( !bImportingXML )
            rDocShell.UpdatePaintExt( nExtFlags, aMultiRange );     // content after the change

        if (!AdjustRowHeight( aMultiRange ))
            rDocShell.PostPaint( aMultiRange, PaintPartFlags::Grid, nExtFlags );
        else if (nExtFlags & SC_PF_LINES)
            lcl_PaintAbove( rDocShell, aMultiRange );   // because of lines above the range

        aModificator.SetDocumentModified();
    }

    return true;
}

bool ScDocFunc::ApplyStyle( const ScMarkData& rMark, const OUString& rStyleName,
                                bool bApi )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bRecord = true;
    if ( !rDoc.IsUndoEnabled() )
        bRecord = false;

    bool bImportingXML = rDoc.IsImportingXML();
    // Cell formats can still be set if the range isn't editable only because of matrix formulas.
    // #i62483# When loading XML, the check can be skipped altogether.
    bool bOnlyNotBecauseOfMatrix;
    if ( !bImportingXML && !rDoc.IsSelectionEditable( rMark, &bOnlyNotBecauseOfMatrix )
            && !bOnlyNotBecauseOfMatrix )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return false;
    }

    ScStyleSheet* pStyleSheet = static_cast<ScStyleSheet*>( rDoc.GetStyleSheetPool()->Find(
                                                rStyleName, SfxStyleFamily::Para ));
    if (!pStyleSheet)
        return false;

    ScDocShellModificator aModificator( rDocShell );

    ScRange aMultiRange;
    bool bMulti = rMark.IsMultiMarked();
    if ( bMulti )
        rMark.GetMultiMarkArea( aMultiRange );
    else
        rMark.GetMarkArea( aMultiRange );

    if ( bRecord )
    {
        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
        SCTAB nStartTab = aMultiRange.aStart.Tab();
        SCTAB nTabCount = rDoc.GetTableCount();
        pUndoDoc->InitUndo( &rDoc, nStartTab, nStartTab );
        for (const auto& rTab : rMark)
        {
            if (rTab >= nTabCount)
                break;

            if (rTab != nStartTab)
                pUndoDoc->AddUndoTab( rTab, rTab );
        }

        ScRange aCopyRange = aMultiRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        rDoc.CopyToDocument( aCopyRange, InsertDeleteFlags::ATTRIB, bMulti, *pUndoDoc, &rMark );

        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoSelectionStyle>(
                    &rDocShell, rMark, aMultiRange, rStyleName, std::move(pUndoDoc) ) );

    }

    rDoc.ApplySelectionStyle( *pStyleSheet, rMark );

    if (!AdjustRowHeight( aMultiRange ))
        rDocShell.PostPaint( aMultiRange, PaintPartFlags::Grid );

    aModificator.SetDocumentModified();

    return true;
}

namespace {

/**
 * Check if this insertion attempt would end up cutting one or more pivot
 * tables in half, which is not desirable.
 *
 * @return true if this insertion can be done safely without shearing any
 *         existing pivot tables, false otherwise.
 */
bool canInsertCellsByPivot(const ScRange& rRange, const ScMarkData& rMarkData, InsCellCmd eCmd, const ScDocument* pDoc)
{
    if (!pDoc->HasPivotTable())
        // This document has no pivot tables.
        return true;

    const ScDPCollection* pDPs = pDoc->GetDPCollection();

    ScRange aRange(rRange); // local copy
    switch (eCmd)
    {
        case INS_INSROWS_BEFORE:
        {
            aRange.aStart.SetCol(0);
            aRange.aEnd.SetCol(MAXCOL);
            [[fallthrough]];
        }
        case INS_CELLSDOWN:
        {
            auto bIntersects = std::any_of(rMarkData.begin(), rMarkData.end(), [&pDPs, &aRange](const SCTAB& rTab) {
                return pDPs->IntersectsTableByColumns(aRange.aStart.Col(), aRange.aEnd.Col(), aRange.aStart.Row(), rTab); });
            if (bIntersects)
                // This column range cuts through at least one pivot table.  Not good.
                return false;

            // Start row must be either at the top or above any pivot tables.
            if (aRange.aStart.Row() < 0)
                // I don't know how to handle this case.
                return false;

            if (aRange.aStart.Row() == 0)
                // First row is always allowed.
                return true;

            ScRange aTest(aRange);
            aTest.aStart.IncRow(-1); // Test one row up.
            aTest.aEnd.SetRow(aTest.aStart.Row());
            for (const auto& rTab : rMarkData)
            {
                aTest.aStart.SetTab(rTab);
                aTest.aEnd.SetTab(rTab);
                if (pDPs->HasTable(aTest))
                    return false;
            }
        }
        break;
        case INS_INSCOLS_BEFORE:
        {
            aRange.aStart.SetRow(0);
            aRange.aEnd.SetRow(MAXROW);
            [[fallthrough]];
        }
        case INS_CELLSRIGHT:
        {
            auto bIntersects = std::any_of(rMarkData.begin(), rMarkData.end(), [&pDPs, &aRange](const SCTAB& rTab) {
                return pDPs->IntersectsTableByRows(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Row(), rTab); });
            if (bIntersects)
                // This column range cuts through at least one pivot table.  Not good.
                return false;

            // Start row must be either at the top or above any pivot tables.
            if (aRange.aStart.Col() < 0)
                // I don't know how to handle this case.
                return false;

            if (aRange.aStart.Col() == 0)
                // First row is always allowed.
                return true;

            ScRange aTest(aRange);
            aTest.aStart.IncCol(-1); // Test one column to the left.
            aTest.aEnd.SetCol(aTest.aStart.Col());
            for (const auto& rTab : rMarkData)
            {
                aTest.aStart.SetTab(rTab);
                aTest.aEnd.SetTab(rTab);
                if (pDPs->HasTable(aTest))
                    return false;
            }
        }
        break;
        default:
            ;
    }
    return true;
}

/**
 * Check if this deletion attempt would end up cutting one or more pivot
 * tables in half, which is not desirable.
 *
 * @return true if this deletion can be done safely without shearing any
 *         existing pivot tables, false otherwise.
 */
bool canDeleteCellsByPivot(const ScRange& rRange, const ScMarkData& rMarkData, DelCellCmd eCmd, const ScDocument* pDoc)
{
    if (!pDoc->HasPivotTable())
        // This document has no pivot tables.
        return true;

    const ScDPCollection* pDPs = pDoc->GetDPCollection();

    ScRange aRange(rRange); // local copy

    switch (eCmd)
    {
        case DelCellCmd::Rows:
        {
            aRange.aStart.SetCol(0);
            aRange.aEnd.SetCol(MAXCOL);
            [[fallthrough]];
        }
        case DelCellCmd::CellsUp:
        {
            auto bIntersects = std::any_of(rMarkData.begin(), rMarkData.end(), [&pDPs, &aRange](const SCTAB& rTab) {
                return pDPs->IntersectsTableByColumns(aRange.aStart.Col(), aRange.aEnd.Col(), aRange.aStart.Row(), rTab); });
            if (bIntersects)
                // This column range cuts through at least one pivot table.  Not good.
                return false;

            ScRange aTest(aRange);
            for (const auto& rTab : rMarkData)
            {
                aTest.aStart.SetTab(rTab);
                aTest.aEnd.SetTab(rTab);
                if (pDPs->HasTable(aTest))
                    return false;
            }
        }
        break;
        case DelCellCmd::Cols:
        {
            aRange.aStart.SetRow(0);
            aRange.aEnd.SetRow(MAXROW);
            [[fallthrough]];
        }
        case DelCellCmd::CellsLeft:
        {
            auto bIntersects = std::any_of(rMarkData.begin(), rMarkData.end(), [&pDPs, &aRange](const SCTAB& rTab) {
                return pDPs->IntersectsTableByRows(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Row(), rTab); });
            if (bIntersects)
                // This column range cuts through at least one pivot table.  Not good.
                return false;

            ScRange aTest(aRange);
            for (const auto& rTab : rMarkData)
            {
                aTest.aStart.SetTab(rTab);
                aTest.aEnd.SetTab(rTab);
                if (pDPs->HasTable(aTest))
                    return false;
            }
        }
        break;
        default:
            ;
    }
    return true;
}

}

bool ScDocFunc::InsertCells( const ScRange& rRange, const ScMarkData* pTabMark, InsCellCmd eCmd,
                             bool bRecord, bool bApi, bool bPartOfPaste )
{
    ScDocShellModificator aModificator( rDocShell );

    if (rDocShell.GetDocument().GetChangeTrack() &&
            ((eCmd == INS_CELLSDOWN  && (rRange.aStart.Col() != 0 || rRange.aEnd.Col() != MAXCOL)) ||
             (eCmd == INS_CELLSRIGHT && (rRange.aStart.Row() != 0 || rRange.aEnd.Row() != MAXROW))))
    {
        // We should not reach this via UI disabled slots.
        assert(bApi);
        SAL_WARN("sc.ui","ScDocFunc::InsertCells - no change-tracking of partial cell shift");
        return false;
    }

    ScRange aTargetRange( rRange );

    // If insertion is for full cols/rows and after the current
    // selection, then shift the range accordingly
    if ( eCmd == INS_INSROWS_AFTER )
    {
        ScRange aErrorRange( ScAddress::UNINITIALIZED );
        if (!aTargetRange.Move(0, rRange.aEnd.Row() - rRange.aStart.Row() + 1, 0, aErrorRange))
        {
            assert(!"can't move");
        }
    }
    if ( eCmd == INS_INSCOLS_AFTER )
    {
        ScRange aErrorRange( ScAddress::UNINITIALIZED );
        if (!aTargetRange.Move(rRange.aEnd.Col() - rRange.aStart.Col() + 1, 0, 0, aErrorRange))
        {
            assert(!"can't move");
        }
    }

    SCCOL nStartCol = aTargetRange.aStart.Col();
    SCROW nStartRow = aTargetRange.aStart.Row();
    SCTAB nStartTab = aTargetRange.aStart.Tab();
    SCCOL nEndCol = aTargetRange.aEnd.Col();
    SCROW nEndRow = aTargetRange.aEnd.Row();
    SCTAB nEndTab = aTargetRange.aEnd.Tab();

    if ( !ValidRow(nStartRow) || !ValidRow(nEndRow) )
    {
        OSL_FAIL("invalid row in InsertCells");
        return false;
    }

    ScDocument& rDoc = rDocShell.GetDocument();
    SCTAB nTabCount = rDoc.GetTableCount();
    SCCOL nPaintStartCol = nStartCol;
    SCROW nPaintStartRow = nStartRow;
    SCCOL nPaintEndCol = nEndCol;
    SCROW nPaintEndRow = nEndRow;
    PaintPartFlags nPaintFlags = PaintPartFlags::Grid;
    bool bSuccess;

    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();  //preserve current cursor position
    SCCOL nCursorCol = 0;
    SCROW nCursorRow = 0;
    if( pViewSh )
    {
        nCursorCol = pViewSh->GetViewData().GetCurX();
        nCursorRow = pViewSh->GetViewData().GetCurY();
    }

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        SCTAB nCount = 0;
        for( SCTAB i=0; i<nTabCount; i++ )
        {
            if( !rDoc.IsScenario(i) )
            {
                nCount++;
                if( nCount == nEndTab+1 )
                {
                    aMark.SelectTable( i, true );
                    break;
                }
            }
        }
    }

    ScMarkData aFullMark( aMark );          // including scenario sheets
    for (const auto& rTab : aMark)
    {
        if (rTab >= nTabCount)
            break;

        for( SCTAB j = rTab+1; j<nTabCount && rDoc.IsScenario(j); j++ )
            aFullMark.SelectTable( j, true );
    }

    SCTAB nSelCount = aMark.GetSelectCount();

    // Adjust also related scenarios

    SCCOL nMergeTestStartCol = nStartCol;
    SCROW nMergeTestStartRow = nStartRow;
    SCCOL nMergeTestEndCol = nEndCol;
    SCROW nMergeTestEndRow = nEndRow;

    ScRange aExtendMergeRange( aTargetRange );

    if( aTargetRange.aStart == aTargetRange.aEnd && rDoc.HasAttrib(aTargetRange, HasAttrFlags::Merged) )
    {
        rDoc.ExtendMerge( aExtendMergeRange );
        rDoc.ExtendOverlapped( aExtendMergeRange );
        nMergeTestEndCol = aExtendMergeRange.aEnd.Col();
        nMergeTestEndRow = aExtendMergeRange.aEnd.Row();
        nPaintEndCol = nMergeTestEndCol;
        nPaintEndRow = nMergeTestEndRow;
    }

    if ( eCmd == INS_INSROWS_BEFORE || eCmd == INS_INSROWS_AFTER )
    {
        nMergeTestStartCol = 0;
        nMergeTestEndCol = MAXCOL;
    }
    if ( eCmd == INS_INSCOLS_BEFORE || eCmd == INS_INSCOLS_AFTER )
    {
        nMergeTestStartRow = 0;
        nMergeTestEndRow = MAXROW;
    }
    if ( eCmd == INS_CELLSDOWN )
        nMergeTestEndRow = MAXROW;
    if ( eCmd == INS_CELLSRIGHT )
        nMergeTestEndCol = MAXCOL;

    bool bNeedRefresh = false;

    SCCOL nEditTestEndCol = (eCmd==INS_INSCOLS_BEFORE || eCmd==INS_INSCOLS_AFTER) ? MAXCOL : nMergeTestEndCol;
    SCROW nEditTestEndRow = (eCmd==INS_INSROWS_BEFORE || eCmd==INS_INSROWS_AFTER) ? MAXROW : nMergeTestEndRow;

    ScEditableTester aTester;

    switch (eCmd)
    {
        case INS_INSCOLS_BEFORE:
            aTester = ScEditableTester(
                rDoc, sc::ColRowEditAction::InsertColumnsBefore, nMergeTestStartCol, nMergeTestEndCol, aMark);
            break;
        case INS_INSCOLS_AFTER:
            aTester = ScEditableTester(
                rDoc, sc::ColRowEditAction::InsertColumnsAfter, nMergeTestStartCol, nMergeTestEndCol, aMark);
            break;
        case INS_INSROWS_BEFORE:
            aTester = ScEditableTester(
                rDoc, sc::ColRowEditAction::InsertRowsBefore, nMergeTestStartRow, nMergeTestEndRow, aMark);
            break;
        case INS_INSROWS_AFTER:
            aTester = ScEditableTester(
                rDoc, sc::ColRowEditAction::InsertRowsAfter, nMergeTestStartRow, nMergeTestEndRow, aMark);
            break;
        default:
            aTester = ScEditableTester(
                &rDoc, nMergeTestStartCol, nMergeTestStartRow, nEditTestEndCol, nEditTestEndRow, aMark);
    }

    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    // Check if this insertion is allowed with respect to pivot table.
    if (!canInsertCellsByPivot(aTargetRange, aMark, eCmd, &rDoc))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_NO_INSERT_DELETE_OVER_PIVOT_TABLE);
        return false;
    }

    WaitObject aWait( ScDocShell::GetActiveDialogParent() );      // important due to TrackFormulas at UpdateReference

    ScDocumentUniquePtr pRefUndoDoc;
    std::unique_ptr<ScRefUndoData> pUndoData;
    if ( bRecord )
    {
        pRefUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pRefUndoDoc->InitUndo( &rDoc, 0, nTabCount-1 );

        // pRefUndoDoc is filled in InsertCol / InsertRow

        pUndoData.reset(new ScRefUndoData( &rDoc ));

        rDoc.BeginDrawUndo();
    }

    // #i8302 : we unmerge overwhelming ranges, before insertion all the actions are put in the same ListAction
    // the patch comes from mloiseleur and maoyg
    bool bInsertMerge = false;
    std::vector<ScRange> qIncreaseRange;
    OUString aUndo = ScResId( STR_UNDO_INSERTCELLS );
    if (bRecord)
    {
        ViewShellId nViewShellId(-1);
        if (pViewSh)
            nViewShellId = pViewSh->GetViewShellId();
        rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo, 0, nViewShellId );
    }
    std::unique_ptr<ScUndoRemoveMerge> pUndoRemoveMerge;

    for (const SCTAB i : aMark)
    {
        if (i >= nTabCount)
            break;

        if( rDoc.HasAttrib( nMergeTestStartCol, nMergeTestStartRow, i, nMergeTestEndCol, nMergeTestEndRow, i, HasAttrFlags::Merged | HasAttrFlags::Overlapped ) )
        {
            if (eCmd==INS_CELLSRIGHT)
                bNeedRefresh = true;

            SCCOL nMergeStartCol = nMergeTestStartCol;
            SCROW nMergeStartRow = nMergeTestStartRow;
            SCCOL nMergeEndCol   = nMergeTestEndCol;
            SCROW nMergeEndRow   = nMergeTestEndRow;

            rDoc.ExtendMerge( nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i );
            rDoc.ExtendOverlapped( nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i );

            if(( eCmd == INS_CELLSDOWN && ( nMergeStartCol != nMergeTestStartCol || nMergeEndCol != nMergeTestEndCol )) ||
                (eCmd == INS_CELLSRIGHT && ( nMergeStartRow != nMergeTestStartRow || nMergeEndRow != nMergeTestEndRow )) )
            {
                if (!bApi)
                    rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0);
                rDocShell.GetUndoManager()->LeaveListAction();
                return false;
            }

            SCCOL nTestCol = -1;
            SCROW nTestRow1 = -1;
            SCROW nTestRow2 = -1;

            ScDocAttrIterator aTestIter( &rDoc, i, nMergeTestStartCol, nMergeTestStartRow, nMergeTestEndCol, nMergeTestEndRow );
            ScRange aExtendRange( nMergeTestStartCol, nMergeTestStartRow, i, nMergeTestEndCol, nMergeTestEndRow, i );
            const ScPatternAttr* pPattern = nullptr;
            while ( ( pPattern = aTestIter.GetNext( nTestCol, nTestRow1, nTestRow2 ) ) != nullptr )
            {
                const ScMergeAttr& rMergeFlag = pPattern->GetItem(ATTR_MERGE);
                const ScMergeFlagAttr& rMergeFlagAttr = pPattern->GetItem(ATTR_MERGE_FLAG);
                ScMF nNewFlags = rMergeFlagAttr.GetValue() & (ScMF::Hor | ScMF::Ver);
                if (rMergeFlag.IsMerged() || nNewFlags == ScMF::Hor || nNewFlags == ScMF::Ver)
                {
                    ScRange aRange( nTestCol, nTestRow1, i );
                    rDoc.ExtendOverlapped(aRange);
                    rDoc.ExtendMerge(aRange, true);

                    if( nTestRow1 < nTestRow2 && nNewFlags == ScMF::Hor )
                    {
                        for( SCROW nTestRow = nTestRow1; nTestRow <= nTestRow2; nTestRow++ )
                        {
                            ScRange aTestRange( nTestCol, nTestRow, i );
                            rDoc.ExtendOverlapped( aTestRange );
                            rDoc.ExtendMerge( aTestRange, true);
                            ScRange aMergeRange( aTestRange.aStart.Col(),aTestRange.aStart.Row(), i );
                            if( !aExtendRange.In( aMergeRange ) )
                            {
                                qIncreaseRange.push_back( aTestRange );
                                bInsertMerge = true;
                            }
                        }
                    }
                    else
                    {
                        ScRange aMergeRange( aRange.aStart.Col(),aRange.aStart.Row(), i );
                        if( !aExtendRange.In( aMergeRange ) )
                        {
                            qIncreaseRange.push_back( aRange );
                        }
                        bInsertMerge = true;
                    }
                }
            }

            if( bInsertMerge )
            {
                if( eCmd == INS_INSROWS_BEFORE || eCmd == INS_INSROWS_AFTER || eCmd == INS_CELLSDOWN )
                {
                    nStartRow = aExtendMergeRange.aStart.Row();
                    nEndRow = aExtendMergeRange.aEnd.Row();

                    if( eCmd == INS_CELLSDOWN )
                        nEndCol = nMergeTestEndCol;
                    else
                    {
                        nStartCol = 0;
                        nEndCol = MAXCOL;
                    }
                }
                else if( eCmd == INS_CELLSRIGHT || eCmd == INS_INSCOLS_BEFORE || eCmd == INS_INSCOLS_AFTER )
                {

                    nStartCol = aExtendMergeRange.aStart.Col();
                    nEndCol = aExtendMergeRange.aEnd.Col();
                    if( eCmd == INS_CELLSRIGHT )
                    {
                        nEndRow = nMergeTestEndRow;
                    }
                    else
                    {
                        nStartRow = 0;
                        nEndRow = MAXROW;
                    }
                }

                if( !qIncreaseRange.empty() )
                {
                    if (bRecord && !pUndoRemoveMerge)
                    {
                        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
                        pUndoDoc->InitUndo( &rDoc, *aMark.begin(), *aMark.rbegin());
                        pUndoRemoveMerge.reset( new ScUndoRemoveMerge( &rDocShell, rRange, std::move(pUndoDoc) ));
                    }

                    for( const ScRange& aRange : qIncreaseRange )
                    {
                        if( rDoc.HasAttrib( aRange, HasAttrFlags::Overlapped | HasAttrFlags::Merged ) )
                        {
                            UnmergeCells( aRange, bRecord, pUndoRemoveMerge.get() );
                        }
                    }
                }
            }
            else
            {
                if (!bApi)
                    rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0);
                rDocShell.GetUndoManager()->LeaveListAction();
                return false;
            }
        }
    }

    if (bRecord && pUndoRemoveMerge)
    {
        rDocShell.GetUndoManager()->AddUndoAction( std::move(pUndoRemoveMerge));
    }

    switch (eCmd)
    {
        case INS_CELLSDOWN:
            bSuccess = rDoc.InsertRow( nStartCol, 0, nEndCol, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc.get(), &aFullMark );
            nPaintEndRow = MAXROW;
            break;
        case INS_INSROWS_BEFORE:
        case INS_INSROWS_AFTER:
            bSuccess = rDoc.InsertRow( 0, 0, MAXCOL, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc.get(), &aFullMark );
            nPaintStartCol = 0;
            nPaintEndCol = MAXCOL;
            nPaintEndRow = MAXROW;
            nPaintFlags |= PaintPartFlags::Left;
            break;
        case INS_CELLSRIGHT:
            bSuccess = rDoc.InsertCol( nStartRow, 0, nEndRow, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc.get(), &aFullMark );
            nPaintEndCol = MAXCOL;
            break;
        case INS_INSCOLS_BEFORE:
        case INS_INSCOLS_AFTER:
            bSuccess = rDoc.InsertCol( 0, 0, MAXROW, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc.get(), &aFullMark );
            nPaintStartRow = 0;
            nPaintEndRow = MAXROW;
            nPaintEndCol = MAXCOL;
            nPaintFlags |= PaintPartFlags::Top;
            break;
        default:
            OSL_FAIL("Wrong code at inserting");
            bSuccess = false;
            break;
    }

    if ( bSuccess )
    {
        SCTAB  nUndoPos  = 0;

        if ( bRecord )
        {
            std::unique_ptr<SCTAB[]> pTabs(new SCTAB[nSelCount]);
            std::unique_ptr<SCTAB[]> pScenarios(new SCTAB[nSelCount]);
            nUndoPos    = 0;
            for (const auto& rTab : aMark)
            {
                if (rTab >= nTabCount)
                    break;

                SCTAB nCount = 0;
                for( SCTAB j=rTab+1; j<nTabCount && rDoc.IsScenario(j); j++ )
                    nCount ++;

                pScenarios[nUndoPos] = nCount;
                pTabs[nUndoPos] = rTab;
                nUndoPos ++;
            }

            if( !bInsertMerge )
            {
                rDocShell.GetUndoManager()->LeaveListAction();
            }

            rDocShell.GetUndoManager()->AddUndoAction( std::make_unique<ScUndoInsertCells>(
                &rDocShell, ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab ),
                nUndoPos, std::move(pTabs), std::move(pScenarios), eCmd, std::move(pRefUndoDoc), std::move(pUndoData), bPartOfPaste ) );
        }

        // #i8302 : we remerge growing ranges, with the new part inserted

        while( !qIncreaseRange.empty() )
        {
            ScRange aRange = qIncreaseRange.back();
            if( !rDoc.HasAttrib( aRange, HasAttrFlags::Overlapped | HasAttrFlags::Merged ) )
            {
                switch (eCmd)
                {
                    case INS_CELLSDOWN:
                    case INS_INSROWS_BEFORE:
                    case INS_INSROWS_AFTER:
                        aRange.aEnd.IncRow(static_cast<SCCOL>(nEndRow-nStartRow+1));
                        break;
                    case INS_CELLSRIGHT:
                    case INS_INSCOLS_BEFORE:
                    case INS_INSCOLS_AFTER:
                        aRange.aEnd.IncCol(static_cast<SCCOL>(nEndCol-nStartCol+1));
                        break;
                    default:
                        break;
                }
                ScCellMergeOption aMergeOption(
                    aRange.aStart.Col(), aRange.aStart.Row(),
                    aRange.aEnd.Col(), aRange.aEnd.Row() );
                aMergeOption.maTabs.insert(aRange.aStart.Tab());
                MergeCells(aMergeOption, false, true, true);
            }
            qIncreaseRange.pop_back();
        }

        if( bInsertMerge )
            rDocShell.GetUndoManager()->LeaveListAction();

        for (const SCTAB i : aMark)
        {
            if (i >= nTabCount)
                break;

            rDoc.SetDrawPageSize(i);

            if (bNeedRefresh)
                rDoc.ExtendMerge( nMergeTestStartCol, nMergeTestStartRow, nMergeTestEndCol, nMergeTestEndRow, i, true );
            else
                rDoc.RefreshAutoFilter( nMergeTestStartCol, nMergeTestStartRow, nMergeTestEndCol, nMergeTestEndRow, i );

            if ( eCmd == INS_INSROWS_BEFORE ||eCmd == INS_INSCOLS_BEFORE || eCmd == INS_INSROWS_AFTER ||eCmd == INS_INSCOLS_AFTER )
                rDoc.UpdatePageBreaks( i );

            sal_uInt16 nExtFlags = 0;
            rDocShell.UpdatePaintExt( nExtFlags, nPaintStartCol, nPaintStartRow, i, nPaintEndCol, nPaintEndRow, i );

            SCTAB nScenarioCount = 0;

            for( SCTAB j = i+1; j<nTabCount && rDoc.IsScenario(j); j++ )
                nScenarioCount ++;

            bool bAdjusted = ( eCmd == INS_INSROWS_BEFORE || eCmd == INS_INSROWS_AFTER ) ?
                        AdjustRowHeight(ScRange(0, nStartRow, i, MAXCOL, nEndRow, i+nScenarioCount )) :
                        AdjustRowHeight(ScRange(0, nPaintStartRow, i, MAXCOL, nPaintEndRow, i+nScenarioCount ));
            if (bAdjusted)
            {
                //  paint only what is not done by AdjustRowHeight
                if (nPaintFlags & PaintPartFlags::Top)
                    rDocShell.PostPaint( nPaintStartCol, nPaintStartRow, i, nPaintEndCol, nPaintEndRow, i+nScenarioCount, PaintPartFlags::Top );
            }
            else
                rDocShell.PostPaint( nPaintStartCol, nPaintStartRow, i, nPaintEndCol, nPaintEndRow, i+nScenarioCount, nPaintFlags, nExtFlags );
        }
    }
    else
    {
        if( bInsertMerge )
        {
            while( !qIncreaseRange.empty() )
            {
                ScRange aRange = qIncreaseRange.back();
                ScCellMergeOption aMergeOption(
                    aRange.aStart.Col(), aRange.aStart.Row(),
                    aRange.aEnd.Col(), aRange.aEnd.Row() );
                MergeCells(aMergeOption, false, true, true);
                qIncreaseRange.pop_back();
            }

            if( pViewSh )
            {
                pViewSh->MarkRange( aTargetRange, false );
                pViewSh->SetCursor( nCursorCol, nCursorRow );
            }
        }

        rDocShell.GetUndoManager()->LeaveListAction();
        rDocShell.GetUndoManager()->RemoveLastUndoAction();

        pRefUndoDoc.reset();
        if (!bApi)
            rDocShell.ErrorMessage(STR_INSERT_FULL);        // column/row full
    }

    // The cursor position needs to be modified earlier than updating
    // any enabled edit view which is triggered by SetDocumentModified below.
    if (bSuccess)
    {
        bool bInsertCols = ( eCmd == INS_INSCOLS_BEFORE || eCmd == INS_INSCOLS_AFTER);
        bool bInsertRows = ( eCmd == INS_INSROWS_BEFORE || eCmd == INS_INSROWS_AFTER );

        if (bInsertCols)
        {
            pViewSh->OnLOKInsertDeleteColumn(rRange.aStart.Col(), 1);
        }

        if (bInsertRows)
        {
            pViewSh->OnLOKInsertDeleteRow(rRange.aStart.Row(), 1);
        }
    }

    aModificator.SetDocumentModified();

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );
    return bSuccess;
}

bool ScDocFunc::DeleteCells( const ScRange& rRange, const ScMarkData* pTabMark, DelCellCmd eCmd,
                             bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    if (rDocShell.GetDocument().GetChangeTrack() &&
            ((eCmd == DelCellCmd::CellsUp   && (rRange.aStart.Col() != 0 || rRange.aEnd.Col() != MAXCOL)) ||
             (eCmd == DelCellCmd::CellsLeft && (rRange.aStart.Row() != 0 || rRange.aEnd.Row() != MAXROW))))
    {
        // We should not reach this via UI disabled slots.
        assert(bApi);
        SAL_WARN("sc.ui","ScDocFunc::DeleteCells - no change-tracking of partial cell shift");
        return false;
    }

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if ( !ValidRow(nStartRow) || !ValidRow(nEndRow) )
    {
        OSL_FAIL("invalid row in DeleteCells");
        return false;
    }

    ScDocument& rDoc = rDocShell.GetDocument();
    SCTAB nTabCount = rDoc.GetTableCount();
    SCCOL nPaintStartCol = nStartCol;
    SCROW nPaintStartRow = nStartRow;
    SCCOL nPaintEndCol = nEndCol;
    SCROW nPaintEndRow = nEndRow;
    PaintPartFlags nPaintFlags = PaintPartFlags::Grid;

    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        SCTAB nCount = 0;
        for(SCTAB i=0; i<nTabCount; i++ )
        {
            if( !rDoc.IsScenario(i) )
            {
                nCount++;
                if( nCount == nEndTab+1 )
                {
                    aMark.SelectTable(i, true);
                    break;
                }
            }
        }
    }

    ScMarkData aFullMark( aMark );          // including scenario sheets
    for (const auto& rTab : aMark)
    {
        if (rTab >= nTabCount)
            break;

        for( SCTAB j = rTab+1; j<nTabCount && rDoc.IsScenario(j); j++ )
            aFullMark.SelectTable( j, true );
    }

    SCTAB nSelCount = aMark.GetSelectCount();

    SCCOL nUndoStartCol = nStartCol;
    SCROW nUndoStartRow = nStartRow;
    SCCOL nUndoEndCol = nEndCol;
    SCROW nUndoEndRow = nEndRow;

    ScRange aExtendMergeRange( rRange );

    if( rRange.aStart == rRange.aEnd && rDoc.HasAttrib(rRange, HasAttrFlags::Merged) )
    {
        rDoc.ExtendMerge( aExtendMergeRange );
        rDoc.ExtendOverlapped( aExtendMergeRange );
        nUndoEndCol = aExtendMergeRange.aEnd.Col();
        nUndoEndRow = aExtendMergeRange.aEnd.Row();
        nPaintEndCol = nUndoEndCol;
        nPaintEndRow = nUndoEndRow;
    }

    if (eCmd==DelCellCmd::Rows)
    {
        nUndoStartCol = 0;
        nUndoEndCol = MAXCOL;
    }
    if (eCmd==DelCellCmd::Cols)
    {
        nUndoStartRow = 0;
        nUndoEndRow = MAXROW;
    }
                    // Test for cell protection

    SCCOL nEditTestEndX = nUndoEndCol;
    if ( eCmd==DelCellCmd::Cols || eCmd==DelCellCmd::CellsLeft )
        nEditTestEndX = MAXCOL;
    SCROW nEditTestEndY = nUndoEndRow;
    if ( eCmd==DelCellCmd::Rows || eCmd==DelCellCmd::CellsUp )
        nEditTestEndY = MAXROW;

    ScEditableTester aTester;

    switch (eCmd)
    {
        case DelCellCmd::Cols:
            aTester = ScEditableTester(
                rDoc, sc::ColRowEditAction::DeleteColumns, nUndoStartCol, nUndoEndCol, aMark);
            break;
        case DelCellCmd::Rows:
            aTester = ScEditableTester(
                rDoc, sc::ColRowEditAction::DeleteRows, nUndoStartRow, nUndoEndRow, aMark);
            break;
        default:
            aTester = ScEditableTester(
                &rDoc, nUndoStartCol, nUndoStartRow, nEditTestEndX, nEditTestEndY, aMark);
    }

    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if (!canDeleteCellsByPivot(rRange, aMark, eCmd, &rDoc))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_NO_INSERT_DELETE_OVER_PIVOT_TABLE);
        return false;
    }
                    // Test for merged cells

    SCCOL nMergeTestEndCol = (eCmd==DelCellCmd::CellsLeft) ? MAXCOL : nUndoEndCol;
    SCROW nMergeTestEndRow = (eCmd==DelCellCmd::CellsUp)   ? MAXROW : nUndoEndRow;
    SCCOL nExtendStartCol = nUndoStartCol;
    SCROW nExtendStartRow = nUndoStartRow;
    bool bNeedRefresh = false;

    //Issue 8302 want to be able to insert into the middle of merged cells
    //the patch comes from maoyg
    ::std::vector<ScRange> qDecreaseRange;
    bool bDeletingMerge = false;
    OUString aUndo = ScResId( STR_UNDO_DELETECELLS );
    if (bRecord)
    {
        ViewShellId nViewShellId(-1);
        if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
            nViewShellId = pViewSh->GetViewShellId();
        rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo, 0, nViewShellId );
    }
    std::unique_ptr<ScUndoRemoveMerge> pUndoRemoveMerge;

    for (const SCTAB i : aMark)
    {
        if (i >= nTabCount)
            break;

        if ( rDoc.HasAttrib( nUndoStartCol, nUndoStartRow, i, nMergeTestEndCol, nMergeTestEndRow, i, HasAttrFlags::Merged | HasAttrFlags::Overlapped ))
        {
            SCCOL nMergeStartCol = nUndoStartCol;
            SCROW nMergeStartRow = nUndoStartRow;
            SCCOL nMergeEndCol   = nMergeTestEndCol;
            SCROW nMergeEndRow   = nMergeTestEndRow;

            rDoc.ExtendMerge( nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i );
            rDoc.ExtendOverlapped( nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i );
            if( ( eCmd == DelCellCmd::CellsUp && ( nMergeStartCol != nUndoStartCol || nMergeEndCol != nMergeTestEndCol))||
                ( eCmd == DelCellCmd::CellsLeft && ( nMergeStartRow != nUndoStartRow || nMergeEndRow != nMergeTestEndRow)))
            {
                if (!bApi)
                    rDocShell.ErrorMessage(STR_MSSG_DELETECELLS_0);
                rDocShell.GetUndoManager()->LeaveListAction();
                return false;
            }

            nExtendStartCol = nMergeStartCol;
            nExtendStartRow = nMergeStartRow;
            SCCOL nTestCol = -1;
            SCROW nTestRow1 = -1;
            SCROW nTestRow2 = -1;

            ScDocAttrIterator aTestIter( &rDoc, i, nUndoStartCol, nUndoStartRow, nMergeTestEndCol, nMergeTestEndRow );
            ScRange aExtendRange( nUndoStartCol, nUndoStartRow, i, nMergeTestEndCol, nMergeTestEndRow, i );
            const ScPatternAttr* pPattern = nullptr;
            while ( ( pPattern = aTestIter.GetNext( nTestCol, nTestRow1, nTestRow2 ) ) != nullptr )
            {
                const ScMergeAttr& rMergeFlag = pPattern->GetItem(ATTR_MERGE);
                const ScMergeFlagAttr& rMergeFlagAttr = pPattern->GetItem(ATTR_MERGE_FLAG);
                ScMF nNewFlags = rMergeFlagAttr.GetValue() & (ScMF::Hor | ScMF::Ver);
                if (rMergeFlag.IsMerged() || nNewFlags == ScMF::Hor || nNewFlags == ScMF::Ver)
                {
                    ScRange aRange( nTestCol, nTestRow1, i );
                    rDoc.ExtendOverlapped( aRange );
                    rDoc.ExtendMerge( aRange, true );

                    if( nTestRow1 < nTestRow2 && nNewFlags == ScMF::Hor )
                    {
                        for( SCROW nTestRow = nTestRow1; nTestRow <= nTestRow2; nTestRow++ )
                        {
                            ScRange aTestRange( nTestCol, nTestRow, i );
                            rDoc.ExtendOverlapped( aTestRange );
                            rDoc.ExtendMerge( aTestRange, true );
                            ScRange aMergeRange( aTestRange.aStart.Col(),aTestRange.aStart.Row(), i );
                            if( !aExtendRange.In( aMergeRange ) )
                            {
                                qDecreaseRange.push_back( aTestRange );
                                bDeletingMerge = true;
                            }
                        }
                    }
                    else
                    {
                        ScRange aMergeRange( aRange.aStart.Col(),aRange.aStart.Row(), i );
                        if( !aExtendRange.In( aMergeRange ) )
                        {
                            qDecreaseRange.push_back( aRange );
                        }
                        bDeletingMerge = true;
                    }
                }
            }

            if( bDeletingMerge )
            {

                if( eCmd == DelCellCmd::Rows || eCmd == DelCellCmd::CellsUp )
                {
                    nStartRow = aExtendMergeRange.aStart.Row();
                    nEndRow = aExtendMergeRange.aEnd.Row();
                    bNeedRefresh = true;

                    if( eCmd == DelCellCmd::CellsUp )
                    {
                        nEndCol = aExtendMergeRange.aEnd.Col();
                    }
                    else
                    {
                        nStartCol = 0;
                        nEndCol = MAXCOL;
                    }
                }
                else if( eCmd == DelCellCmd::CellsLeft || eCmd == DelCellCmd::Cols )
                {

                    nStartCol = aExtendMergeRange.aStart.Col();
                    nEndCol = aExtendMergeRange.aEnd.Col();
                    if( eCmd == DelCellCmd::CellsLeft )
                    {
                        nEndRow = aExtendMergeRange.aEnd.Row();
                        bNeedRefresh = true;
                    }
                    else
                    {
                        nStartRow = 0;
                        nEndRow = MAXROW;
                    }
                }

                if( !qDecreaseRange.empty() )
                {
                    if (bRecord && !pUndoRemoveMerge)
                    {
                        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
                        pUndoDoc->InitUndo( &rDoc, *aMark.begin(), *aMark.rbegin());
                        pUndoRemoveMerge.reset( new ScUndoRemoveMerge( &rDocShell, rRange, std::move(pUndoDoc) ));
                    }

                    for( const ScRange& aRange : qDecreaseRange )
                    {
                        if( rDoc.HasAttrib( aRange, HasAttrFlags::Overlapped | HasAttrFlags::Merged ) )
                        {
                            UnmergeCells( aRange, bRecord, pUndoRemoveMerge.get() );
                        }
                    }
                }
            }
            else
            {
                if (!bApi)
                    rDocShell.ErrorMessage(STR_MSSG_DELETECELLS_0);
                rDocShell.GetUndoManager()->LeaveListAction();
                return false;
            }
        }
    }

    if (bRecord && pUndoRemoveMerge)
    {
        rDocShell.GetUndoManager()->AddUndoAction( std::move(pUndoRemoveMerge));
    }

    //     do it

    WaitObject aWait( ScDocShell::GetActiveDialogParent() );      // important because of TrackFormulas in UpdateReference

    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScDocument> pRefUndoDoc;
    std::unique_ptr<ScRefUndoData> pUndoData;
    if ( bRecord )
    {
        // With the fix for #101329#, UpdateRef always puts cells into pRefUndoDoc at their old position,
        // so it's no longer necessary to copy more than the deleted range into pUndoDoc.

        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( &rDoc, 0, nTabCount-1, (eCmd==DelCellCmd::Cols), (eCmd==DelCellCmd::Rows) );
        for (const auto& rTab : aMark)
        {
            if (rTab >= nTabCount)
                break;

            SCTAB nScenarioCount = 0;

            for( SCTAB j = rTab+1; j<nTabCount && rDoc.IsScenario(j); j++ )
                nScenarioCount ++;

            rDoc.CopyToDocument( nUndoStartCol, nUndoStartRow, rTab, nUndoEndCol, nUndoEndRow, rTab+nScenarioCount,
                InsertDeleteFlags::ALL | InsertDeleteFlags::NOCAPTIONS, false, *pUndoDoc );
        }

        pRefUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pRefUndoDoc->InitUndo( &rDoc, 0, nTabCount-1 );

        pUndoData.reset(new ScRefUndoData( &rDoc ));

        rDoc.BeginDrawUndo();
    }

    sal_uInt16 nExtFlags = 0;
    for (const auto& rTab : aMark)
    {
        if (rTab >= nTabCount)
            break;

        rDocShell.UpdatePaintExt( nExtFlags, nStartCol, nStartRow, rTab, nEndCol, nEndRow, rTab );
    }

    bool bUndoOutline = false;
    switch (eCmd)
    {
        case DelCellCmd::CellsUp:
            rDoc.DeleteRow( nStartCol, 0, nEndCol, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc.get(), nullptr, &aFullMark );
            nPaintEndRow = MAXROW;
            break;
        case DelCellCmd::Rows:
            rDoc.DeleteRow( 0, 0, MAXCOL, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc.get(), &bUndoOutline, &aFullMark );
            nPaintStartCol = 0;
            nPaintEndCol = MAXCOL;
            nPaintEndRow = MAXROW;
            nPaintFlags |= PaintPartFlags::Left;
            break;
        case DelCellCmd::CellsLeft:
            rDoc.DeleteCol( nStartRow, 0, nEndRow, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc.get(), nullptr, &aFullMark );
            nPaintEndCol = MAXCOL;
            break;
        case DelCellCmd::Cols:
            rDoc.DeleteCol( 0, 0, MAXROW, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc.get(), &bUndoOutline, &aFullMark );
            nPaintStartRow = 0;
            nPaintEndRow = MAXROW;
            nPaintEndCol = MAXCOL;
            nPaintFlags |= PaintPartFlags::Top;
            break;
        default:
            OSL_FAIL("Wrong code at deleting");
            break;
    }

    //! Test if the size of outline has changed

    if ( bRecord )
    {
        for (const auto& rTab : aFullMark)
        {
            if (rTab >= nTabCount)
                break;

            pRefUndoDoc->DeleteAreaTab(nUndoStartCol,nUndoStartRow,nUndoEndCol,nUndoEndRow, rTab, InsertDeleteFlags::ALL);
        }

            //  for all sheets, so that formulas can be copied
        pUndoDoc->AddUndoTab( 0, nTabCount-1 );

            //  copy with bColRowFlags=false (#54194#)
        pRefUndoDoc->CopyToDocument(0,0,0,MAXCOL,MAXROW,MAXTAB,InsertDeleteFlags::FORMULA,false,*pUndoDoc,nullptr,false);
        pRefUndoDoc.reset();

        std::unique_ptr<SCTAB[]> pTabs(      new SCTAB[nSelCount]);
        std::unique_ptr<SCTAB[]> pScenarios( new SCTAB[nSelCount]);
        SCTAB   nUndoPos  = 0;

        for (const auto& rTab : aMark)
        {
            if (rTab >= nTabCount)
                break;

            SCTAB nCount = 0;
            for( SCTAB j=rTab+1; j<nTabCount && rDoc.IsScenario(j); j++ )
                nCount ++;

            pScenarios[nUndoPos] = nCount;
            pTabs[nUndoPos] = rTab;
            nUndoPos ++;
        }

        if( !bDeletingMerge )
        {
            rDocShell.GetUndoManager()->LeaveListAction();
        }

        rDocShell.GetUndoManager()->AddUndoAction( std::make_unique<ScUndoDeleteCells>(
            &rDocShell, ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab ),
            nUndoPos, std::move(pTabs), std::move(pScenarios),
            eCmd, std::move(pUndoDoc), std::move(pUndoData) ) );
    }

    // #i8302 want to be able to insert into the middle of merged cells
    // the patch comes from maoyg

    while( !qDecreaseRange.empty() )
    {
        ScRange aRange = qDecreaseRange.back();

        long nDecreaseRowCount = 0;
        long nDecreaseColCount = 0;
        if( eCmd == DelCellCmd::CellsUp || eCmd == DelCellCmd::Rows )
        {
            if( nStartRow >= aRange.aStart.Row() && nStartRow <= aRange.aEnd.Row() && nEndRow>= aRange.aStart.Row() && nEndRow <= aRange.aEnd.Row() )
                nDecreaseRowCount = nEndRow-nStartRow+1;
            else if( nStartRow >= aRange.aStart.Row() && nStartRow <= aRange.aEnd.Row() && nEndRow >= aRange.aStart.Row() && nEndRow >= aRange.aEnd.Row() )
                nDecreaseRowCount = aRange.aEnd.Row()-nStartRow+1;
            else if( nStartRow >= aRange.aStart.Row() && nStartRow >= aRange.aEnd.Row() && nEndRow>= aRange.aStart.Row() && nEndRow <= aRange.aEnd.Row() )
                nDecreaseRowCount = aRange.aEnd.Row()-nEndRow+1;
        }
        else if( eCmd == DelCellCmd::CellsLeft || eCmd == DelCellCmd::Cols )
        {
            if( nStartCol >= aRange.aStart.Col() && nStartCol <= aRange.aEnd.Col() && nEndCol>= aRange.aStart.Col() && nEndCol <= aRange.aEnd.Col() )
                nDecreaseColCount = nEndCol-nStartCol+1;
            else if( nStartCol >= aRange.aStart.Col() && nStartCol <= aRange.aEnd.Col() && nEndCol >= aRange.aStart.Col() && nEndCol >= aRange.aEnd.Col() )
                nDecreaseColCount = aRange.aEnd.Col()-nStartCol+1;
            else if( nStartCol >= aRange.aStart.Col() && nStartCol >= aRange.aEnd.Col() && nEndCol>= aRange.aStart.Col() && nEndCol <= aRange.aEnd.Col() )
                nDecreaseColCount = aRange.aEnd.Col()-nEndCol+1;
        }

        switch (eCmd)
        {
            case DelCellCmd::CellsUp:
            case DelCellCmd::Rows:
                aRange.aEnd.SetRow(static_cast<SCCOL>( aRange.aEnd.Row()-nDecreaseRowCount));
                break;
            case DelCellCmd::CellsLeft:
            case DelCellCmd::Cols:
                aRange.aEnd.SetCol(static_cast<SCCOL>( aRange.aEnd.Col()-nDecreaseColCount));
                break;
            default:
                break;
        }

        if( !rDoc.HasAttrib( aRange, HasAttrFlags::Overlapped | HasAttrFlags::Merged ) )
        {
            ScCellMergeOption aMergeOption(aRange);
            MergeCells( aMergeOption, false, true, true );
        }
        qDecreaseRange.pop_back();
    }

    if( bDeletingMerge )
        rDocShell.GetUndoManager()->LeaveListAction();

    if ( eCmd==DelCellCmd::Cols || eCmd==DelCellCmd::CellsLeft )
        nMergeTestEndCol = MAXCOL;
    if ( eCmd==DelCellCmd::Rows || eCmd==DelCellCmd::CellsUp )
        nMergeTestEndRow = MAXROW;
    if ( bNeedRefresh )
    {
        // #i51445# old merge flag attributes must be deleted also for single cells,
        // not only for whole columns/rows

        ScPatternAttr aPattern( rDoc.GetPool() );
        aPattern.GetItemSet().Put( ScMergeFlagAttr() );

        rDoc.ApplyPatternArea( nExtendStartCol, nExtendStartRow, nMergeTestEndCol, nMergeTestEndRow, aMark, aPattern );

        for (const auto& rTab : aMark)
        {
            if (rTab >= nTabCount)
                break;

            SCTAB nScenarioCount = 0;

            for( SCTAB j = rTab+1; j<nTabCount && rDoc.IsScenario(j); j++ )
                nScenarioCount ++;

            ScRange aMergedRange( nExtendStartCol, nExtendStartRow, rTab, nMergeTestEndCol, nMergeTestEndRow, rTab+nScenarioCount );
            rDoc.ExtendMerge( aMergedRange, true );
        }
    }

    for (const auto& rTab : aMark)
    {
        if (rTab >= nTabCount)
            break;

        rDoc.RefreshAutoFilter( nExtendStartCol, nExtendStartRow, nMergeTestEndCol, nMergeTestEndRow, rTab );
    }

    for (const auto& rTab : aMark)
    {
        if (rTab >= nTabCount)
            break;

        rDoc.SetDrawPageSize(rTab);

        if ( eCmd == DelCellCmd::Cols || eCmd == DelCellCmd::Rows )
            rDoc.UpdatePageBreaks( rTab );

        rDocShell.UpdatePaintExt( nExtFlags, nPaintStartCol, nPaintStartRow, rTab, nPaintEndCol, nPaintEndRow, rTab );

        SCTAB nScenarioCount = 0;

        for( SCTAB j = rTab+1; j<nTabCount && rDoc.IsScenario(j); j++ )
            nScenarioCount ++;

        //  delete entire rows: do not adjust
        if ( eCmd == DelCellCmd::Rows || !AdjustRowHeight(ScRange( 0, nPaintStartRow, rTab, MAXCOL, nPaintEndRow, rTab+nScenarioCount )) )
            rDocShell.PostPaint( nPaintStartCol, nPaintStartRow, rTab, nPaintEndCol, nPaintEndRow, rTab+nScenarioCount, nPaintFlags,  nExtFlags );
        else
        {
            //  paint only what is not done by AdjustRowHeight
            if (nExtFlags & SC_PF_LINES)
                lcl_PaintAbove( rDocShell, ScRange( nPaintStartCol, nPaintStartRow, rTab, nPaintEndCol, nPaintEndRow, rTab+nScenarioCount) );
            if (nPaintFlags & PaintPartFlags::Top)
                rDocShell.PostPaint( nPaintStartCol, nPaintStartRow, rTab, nPaintEndCol, nPaintEndRow, rTab+nScenarioCount, PaintPartFlags::Top );
        }
    }

    // The cursor position needs to be modified earlier than updating
    // any enabled edit view which is triggered by SetDocumentModified below.
    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();
    if (pViewSh)
    {
        if (eCmd == DelCellCmd::Cols)
        {
            pViewSh->OnLOKInsertDeleteColumn(rRange.aStart.Col(), -1);
        }
        if (eCmd == DelCellCmd::Rows)
        {
            pViewSh->OnLOKInsertDeleteRow(rRange.aStart.Row(), -1);
        }
    }

    aModificator.SetDocumentModified();

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );

    return true;
}

bool ScDocFunc::MoveBlock( const ScRange& rSource, const ScAddress& rDestPos,
                                bool bCut, bool bRecord, bool bPaint, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    SCCOL nStartCol = rSource.aStart.Col();
    SCROW nStartRow = rSource.aStart.Row();
    SCTAB nStartTab = rSource.aStart.Tab();
    SCCOL nEndCol = rSource.aEnd.Col();
    SCROW nEndRow = rSource.aEnd.Row();
    SCTAB nEndTab = rSource.aEnd.Tab();
    SCCOL nDestCol = rDestPos.Col();
    SCROW nDestRow = rDestPos.Row();
    SCTAB nDestTab = rDestPos.Tab();

    if ( !ValidRow(nStartRow) || !ValidRow(nEndRow) || !ValidRow(nDestRow) )
    {
        OSL_FAIL("invalid row in MoveBlock");
        return false;
    }

    //  adjust related scenarios too - but only when moved within one sheet
    bool bScenariosAdded = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    SCTAB nTabCount = rDoc.GetTableCount();
    if ( nDestTab == nStartTab && !rDoc.IsScenario(nEndTab) )
        while ( nEndTab+1 < nTabCount && rDoc.IsScenario(nEndTab+1) )
        {
            ++nEndTab;
            bScenariosAdded = true;
        }

    SCTAB nSrcTabCount = nEndTab-nStartTab+1;
    SCTAB nDestEndTab = nDestTab+nSrcTabCount-1;
    SCTAB nTab;

    ScDocumentUniquePtr pClipDoc(new ScDocument(SCDOCMODE_CLIP));

    ScMarkData aSourceMark;
    for (nTab=nStartTab; nTab<=nEndTab; nTab++)
        aSourceMark.SelectTable( nTab, true );      // select source
    aSourceMark.SetMarkArea( rSource );

    ScDocShellRef aDragShellRef;
    if ( rDoc.HasOLEObjectsInArea( rSource ) )
    {
        aDragShellRef = new ScDocShell;     // DocShell needs a Ref immediately
        aDragShellRef->DoInitNew();
    }
    ScDrawLayer::SetGlobalDrawPersist( aDragShellRef.get() );

    ScClipParam aClipParam(ScRange(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nStartTab), bCut);
    rDoc.CopyToClip(aClipParam, pClipDoc.get(), &aSourceMark, bScenariosAdded, true);

    ScDrawLayer::SetGlobalDrawPersist(nullptr);

    SCCOL nOldEndCol = nEndCol;
    SCROW nOldEndRow = nEndRow;
    bool bClipOver = false;
    for (nTab=nStartTab; nTab<=nEndTab; nTab++)
    {
        SCCOL nTmpEndCol = nOldEndCol;
        SCROW nTmpEndRow = nOldEndRow;
        if (rDoc.ExtendMerge( nStartCol, nStartRow, nTmpEndCol, nTmpEndRow, nTab ))
            bClipOver = true;
        if ( nTmpEndCol > nEndCol ) nEndCol = nTmpEndCol;
        if ( nTmpEndRow > nEndRow ) nEndRow = nTmpEndRow;
    }

    SCCOL nDestEndCol = nDestCol + ( nOldEndCol-nStartCol );
    SCROW nDestEndRow = nDestRow + ( nOldEndRow-nStartRow );

    SCCOL nUndoEndCol = nDestCol + ( nEndCol-nStartCol );       // extended in destination block
    SCROW nUndoEndRow = nDestRow + ( nEndRow-nStartRow );

    bool bIncludeFiltered = bCut;
    if ( !bIncludeFiltered )
    {
        //  adjust sizes to include only non-filtered rows

        SCCOL nClipX;
        SCROW nClipY;
        pClipDoc->GetClipArea( nClipX, nClipY, false );
        SCROW nUndoAdd = nUndoEndRow - nDestEndRow;
        nDestEndRow = nDestRow + nClipY;
        nUndoEndRow = nDestEndRow + nUndoAdd;
    }

    if (!ValidCol(nUndoEndCol) || !ValidRow(nUndoEndRow))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PASTE_FULL);
        return false;
    }

    //  Test for cell protection

    ScEditableTester aTester;
    for (nTab=nDestTab; nTab<=nDestEndTab; nTab++)
        aTester.TestBlock( &rDoc, nTab, nDestCol,nDestRow, nUndoEndCol,nUndoEndRow );
    if (bCut)
        for (nTab=nStartTab; nTab<=nEndTab; nTab++)
            aTester.TestBlock( &rDoc, nTab, nStartCol,nStartRow, nEndCol,nEndRow );

    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    //  Test for merged cells- when moving after delete

    if (bClipOver && !bCut)
        if (rDoc.HasAttrib( nDestCol,nDestRow,nDestTab, nUndoEndCol,nUndoEndRow,nDestEndTab,
                                HasAttrFlags::Merged | HasAttrFlags::Overlapped ))
        {       // "Merge of already merged cells not possible"
            if (!bApi)
                rDocShell.ErrorMessage(STR_MSSG_MOVEBLOCKTO_0);
            return false;
        }

    //  Are there borders in the cells? (for painting)

    sal_uInt16 nSourceExt = 0;
    rDocShell.UpdatePaintExt( nSourceExt, nStartCol,nStartRow,nStartTab, nEndCol,nEndRow,nEndTab );
    sal_uInt16 nDestExt = 0;
    rDocShell.UpdatePaintExt( nDestExt, nDestCol,nDestRow,nDestTab, nDestEndCol,nDestEndRow,nDestEndTab );

    //  do it

    ScDocumentUniquePtr pUndoDoc;

    if (bRecord)
    {
        bool bWholeCols = ( nStartRow == 0 && nEndRow == MAXROW );
        bool bWholeRows = ( nStartCol == 0 && nEndCol == MAXCOL );
        InsertDeleteFlags nUndoFlags = (InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS) | InsertDeleteFlags::NOCAPTIONS;

        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( &rDoc, nStartTab, nEndTab, bWholeCols, bWholeRows );

        if (bCut)
        {
            rDoc.CopyToDocument( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                                    nUndoFlags, false, *pUndoDoc );
        }

        if ( nDestTab != nStartTab )
            pUndoDoc->AddUndoTab( nDestTab, nDestEndTab, bWholeCols, bWholeRows );
        rDoc.CopyToDocument( nDestCol, nDestRow, nDestTab,
                                    nDestEndCol, nDestEndRow, nDestEndTab,
                                    nUndoFlags, false, *pUndoDoc );
        rDoc.BeginDrawUndo();
    }

    bool bSourceHeight = false;     // adjust heights?
    if (bCut)
    {
        ScMarkData aDelMark;    // only for tables
        for (nTab=nStartTab; nTab<=nEndTab; nTab++)
        {
            rDoc.DeleteAreaTab( nStartCol,nStartRow, nOldEndCol,nOldEndRow, nTab, InsertDeleteFlags::ALL );
            aDelMark.SelectTable( nTab, true );
        }
        rDoc.DeleteObjectsInArea( nStartCol,nStartRow, nOldEndCol,nOldEndRow, aDelMark );

        //  Test for merged cells

        if (bClipOver)
            if (rDoc.HasAttrib( nDestCol,nDestRow,nDestTab,
                                    nUndoEndCol,nUndoEndRow,nDestEndTab,
                                    HasAttrFlags::Merged | HasAttrFlags::Overlapped ))
            {
                rDoc.CopyFromClip( rSource, aSourceMark, InsertDeleteFlags::ALL, nullptr, pClipDoc.get() );
                for (nTab=nStartTab; nTab<=nEndTab; nTab++)
                {
                    SCCOL nTmpEndCol = nEndCol;
                    SCROW nTmpEndRow = nEndRow;
                    rDoc.ExtendMerge( nStartCol, nStartRow, nTmpEndCol, nTmpEndRow, nTab, true );
                }

                // Report error only after restoring content
                if (!bApi)      // "Merge of already merged cells not possible"
                    rDocShell.ErrorMessage(STR_MSSG_MOVEBLOCKTO_0);

                return false;
            }

        bSourceHeight = AdjustRowHeight( rSource, false );
    }

    ScRange aPasteDest( nDestCol, nDestRow, nDestTab, nDestEndCol, nDestEndRow, nDestEndTab );

    ScMarkData aDestMark;
    for (nTab=nDestTab; nTab<=nDestEndTab; nTab++)
        aDestMark.SelectTable( nTab, true );        // select destination
    aDestMark.SetMarkArea( aPasteDest );

    /*  Do not copy drawing objects here. While pasting, the
        function ScDocument::UpdateReference() is called which calls
        ScDrawLayer::MoveCells() which may move away inserted objects to wrong
        positions (e.g. if source and destination range overlaps).*/

    rDoc.CopyFromClip(
        aPasteDest, aDestMark, InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS,
        pUndoDoc.get(), pClipDoc.get(), true, false, bIncludeFiltered);

    // skipped rows and merged cells don't mix
    if ( !bIncludeFiltered && pClipDoc->HasClipFilteredRows() )
        UnmergeCells( aPasteDest, false, nullptr );

    bool bDestHeight = AdjustRowHeight(
                            ScRange( 0,nDestRow,nDestTab, MAXCOL,nDestEndRow,nDestEndTab ),
                            false );

    /*  Paste drawing objects after adjusting formula references
        and row heights. There are no cell notes or drawing objects, if the
        clipdoc does not contain a drawing layer.*/
    if ( pClipDoc->GetDrawLayer() )
        rDoc.CopyFromClip( aPasteDest, aDestMark, InsertDeleteFlags::OBJECTS,
                           nullptr, pClipDoc.get(), true, false, bIncludeFiltered );

    if (bRecord)
    {
        ScRange aUndoRange(nStartCol, nStartRow, nStartTab, nOldEndCol, nOldEndRow, nEndTab);
        ScAddress aDestPos(nDestCol, nDestRow, nDestTab);

        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoDragDrop>(
                &rDocShell, aUndoRange, aDestPos, bCut, std::move(pUndoDoc), bScenariosAdded));
    }

    SCCOL nDestPaintEndCol = nDestEndCol;
    SCROW nDestPaintEndRow = nDestEndRow;
    for (nTab=nDestTab; nTab<=nDestEndTab; nTab++)
    {
        SCCOL nTmpEndCol = nDestEndCol;
        SCROW nTmpEndRow = nDestEndRow;
        rDoc.ExtendMerge( nDestCol, nDestRow, nTmpEndCol, nTmpEndRow, nTab, true );
        if (nTmpEndCol > nDestPaintEndCol) nDestPaintEndCol = nTmpEndCol;
        if (nTmpEndRow > nDestPaintEndRow) nDestPaintEndRow = nTmpEndRow;
    }

    if (bCut)
        for (nTab=nStartTab; nTab<=nEndTab; nTab++)
            rDoc.RefreshAutoFilter( nStartCol, nStartRow, nEndCol, nEndRow, nTab );

    if (bPaint)
    {
            //  destination range:

        SCCOL nPaintStartX = nDestCol;
        SCROW nPaintStartY = nDestRow;
        SCCOL nPaintEndX = nDestPaintEndCol;
        SCROW nPaintEndY = nDestPaintEndRow;
        PaintPartFlags nFlags = PaintPartFlags::Grid;

        if ( nStartRow==0 && nEndRow==MAXROW )      // copy widths too?
        {
            nPaintEndX = MAXCOL;
            nPaintStartY = 0;
            nPaintEndY = MAXROW;
            nFlags |= PaintPartFlags::Top;
        }
        if ( bDestHeight || ( nStartCol == 0 && nEndCol == MAXCOL ) )
        {
            nPaintEndY = MAXROW;
            nPaintStartX = 0;
            nPaintEndX = MAXCOL;
            nFlags |= PaintPartFlags::Left;
        }
        if ( bScenariosAdded )
        {
            nPaintStartX = 0;
            nPaintStartY = 0;
            nPaintEndX = MAXCOL;
            nPaintEndY = MAXROW;
        }

        rDocShell.PostPaint( nPaintStartX,nPaintStartY,nDestTab,
                            nPaintEndX,nPaintEndY,nDestEndTab, nFlags, nSourceExt | nDestExt );

        if ( bCut )
        {
                //  source range:

            nPaintStartX = nStartCol;
            nPaintStartY = nStartRow;
            nPaintEndX = nEndCol;
            nPaintEndY = nEndRow;
            nFlags = PaintPartFlags::Grid;

            if ( bSourceHeight )
            {
                nPaintEndY = MAXROW;
                nPaintStartX = 0;
                nPaintEndX = MAXCOL;
                nFlags |= PaintPartFlags::Left;
            }
            if ( bScenariosAdded )
            {
                nPaintStartX = 0;
                nPaintStartY = 0;
                nPaintEndX = MAXCOL;
                nPaintEndY = MAXROW;
            }

            rDocShell.PostPaint( nPaintStartX,nPaintStartY,nStartTab,
                                nPaintEndX,nPaintEndY,nEndTab, nFlags, nSourceExt );
        }
    }

    aModificator.SetDocumentModified();

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );

    return true;
}

static uno::Reference< uno::XInterface > GetDocModuleObject( const SfxObjectShell& rDocSh, const OUString& sCodeName )
{
    uno::Reference< lang::XMultiServiceFactory> xSF(rDocSh.GetModel(), uno::UNO_QUERY);
    uno::Reference< container::XNameAccess > xVBACodeNamedObjectAccess;
    uno::Reference< uno::XInterface > xDocModuleApiObject;
    if ( xSF.is() )
    {
        xVBACodeNamedObjectAccess.set( xSF->createInstance("ooo.vba.VBAObjectModuleObjectProvider"), uno::UNO_QUERY );
        xDocModuleApiObject.set( xVBACodeNamedObjectAccess->getByName( sCodeName ), uno::UNO_QUERY );
    }
    return xDocModuleApiObject;

}

static script::ModuleInfo lcl_InitModuleInfo( const SfxObjectShell& rDocSh, const OUString& sModule )
{
    script::ModuleInfo sModuleInfo;
    sModuleInfo.ModuleType = script::ModuleType::DOCUMENT;
    sModuleInfo.ModuleObject = GetDocModuleObject( rDocSh, sModule );
    return sModuleInfo;
}

void VBA_InsertModule( ScDocument& rDoc, SCTAB nTab, const OUString& sSource )
{
    SfxObjectShell& rDocSh = *rDoc.GetDocumentShell();
    uno::Reference< script::XLibraryContainer > xLibContainer = rDocSh.GetBasicContainer();
    OSL_ENSURE( xLibContainer.is(), "No BasicContainer!" );

    uno::Reference< container::XNameContainer > xLib;
    if( xLibContainer.is() )
    {
        OUString aLibName( "Standard" );
        if ( rDocSh.GetBasicManager() && !rDocSh.GetBasicManager()->GetName().isEmpty() )
        {
            aLibName = rDocSh.GetBasicManager()->GetName();
        }
        uno::Any aLibAny = xLibContainer->getByName( aLibName );
        aLibAny >>= xLib;
    }
    if( xLib.is() )
    {
        // if the Module with codename exists then find a new name
        sal_Int32 nNum = 1;
        OUString genModuleName = "Sheet1";
        while( xLib->hasByName( genModuleName ) )
            genModuleName = "Sheet" + OUString::number( ++nNum );

        uno::Any aSourceAny;
        OUString sTmpSource = sSource;
        if ( sTmpSource.isEmpty() )
            sTmpSource = "Rem Attribute VBA_ModuleType=VBADocumentModule\nOption VBASupport 1\n";
        aSourceAny <<= sTmpSource;
        uno::Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( xLib, uno::UNO_QUERY );
        if ( xVBAModuleInfo.is() )
        {
            rDoc.SetCodeName( nTab, genModuleName );
            script::ModuleInfo sModuleInfo = lcl_InitModuleInfo(  rDocSh, genModuleName );
            xVBAModuleInfo->insertModuleInfo( genModuleName, sModuleInfo );
            xLib->insertByName( genModuleName, aSourceAny );
        }

    }
}

void VBA_DeleteModule( ScDocShell& rDocSh, const OUString& sModuleName )
{
    uno::Reference< script::XLibraryContainer > xLibContainer = rDocSh.GetBasicContainer();
    OSL_ENSURE( xLibContainer.is(), "No BasicContainer!" );

    uno::Reference< container::XNameContainer > xLib;
    if( xLibContainer.is() )
    {
        OUString aLibName( "Standard" );
        if ( rDocSh.GetBasicManager() && !rDocSh.GetBasicManager()->GetName().isEmpty() )
        {
            aLibName = rDocSh.GetBasicManager()->GetName();
        }
        uno::Any aLibAny = xLibContainer->getByName( aLibName );
        aLibAny >>= xLib;
    }
    if( xLib.is() )
    {
        uno::Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( xLib, uno::UNO_QUERY );
        if( xLib->hasByName( sModuleName ) )
            xLib->removeByName( sModuleName );
        if ( xVBAModuleInfo.is() && xVBAModuleInfo->hasModuleInfo(sModuleName) )
            xVBAModuleInfo->removeModuleInfo( sModuleName );

    }
}

bool ScDocFunc::InsertTable( SCTAB nTab, const OUString& rName, bool bRecord, bool bApi )
{
    bool bSuccess = false;
    WaitObject aWait( ScDocShell::GetActiveDialogParent() );

    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();

    // Strange loop, also basic is loaded too early ( InsertTable )
    // is called via the xml import for sheets in described in ODF
    bool bInsertDocModule = false;

    if(  !rDocShell.GetDocument().IsImportingXML() )
    {
        bInsertDocModule = rDoc.IsInVBAMode();
    }
    if ( bInsertDocModule || ( bRecord && !rDoc.IsUndoEnabled() ) )
        bRecord = false;

    if (bRecord)
        rDoc.BeginDrawUndo();                          //  InsertTab generates SdrUndoNewPage

    SCTAB nTabCount = rDoc.GetTableCount();
    bool bAppend = ( nTab >= nTabCount );
    if ( bAppend )
        nTab = nTabCount;       // important for Undo

    if (rDoc.InsertTab( nTab, rName ))
    {
        if (bRecord)
            rDocShell.GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoInsertTab>( &rDocShell, nTab, bAppend, rName));
        //  Update views:
        // Only insert vba modules if vba mode ( and not currently importing XML )
        if( bInsertDocModule )
        {
            VBA_InsertModule( rDoc, nTab, OUString() );
        }
        rDocShell.Broadcast( ScTablesHint( SC_TAB_INSERTED, nTab ) );

        rDocShell.PostPaintExtras();
        aModificator.SetDocumentModified();
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
        bSuccess = true;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(STR_TABINSERT_ERROR);

    return bSuccess;
}

bool ScDocFunc::DeleteTable( SCTAB nTab, bool bRecord )
{
    WaitObject aWait( ScDocShell::GetActiveDialogParent() );

    ScDocShellModificator aModificator( rDocShell );

    bool bSuccess = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bVbaEnabled = rDoc.IsInVBAMode();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    if ( bVbaEnabled )
        bRecord = false;
    bool bWasLinked = rDoc.IsLinked(nTab);
    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScRefUndoData> pUndoData;
    if (bRecord)
    {
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        SCTAB nCount = rDoc.GetTableCount();

        pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );     // only nTab with Flags
        pUndoDoc->AddUndoTab( 0, nCount-1 );                    // all sheets for references

        rDoc.CopyToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, InsertDeleteFlags::ALL,false, *pUndoDoc );
        OUString aOldName;
        rDoc.GetName( nTab, aOldName );
        pUndoDoc->RenameTab( nTab, aOldName );
        if (bWasLinked)
            pUndoDoc->SetLink( nTab, rDoc.GetLinkMode(nTab), rDoc.GetLinkDoc(nTab),
                                rDoc.GetLinkFlt(nTab), rDoc.GetLinkOpt(nTab),
                                rDoc.GetLinkTab(nTab),
                                rDoc.GetLinkRefreshDelay(nTab) );

        if ( rDoc.IsScenario(nTab) )
        {
            pUndoDoc->SetScenario( nTab, true );
            OUString aComment;
            Color  aColor;
            ScScenarioFlags nScenFlags;
            rDoc.GetScenarioData( nTab, aComment, aColor, nScenFlags );
            pUndoDoc->SetScenarioData( nTab, aComment, aColor, nScenFlags );
            bool bActive = rDoc.IsActiveScenario( nTab );
            pUndoDoc->SetActiveScenario( nTab, bActive );
        }
        pUndoDoc->SetVisible( nTab, rDoc.IsVisible( nTab ) );
        pUndoDoc->SetTabBgColor( nTab, rDoc.GetTabBgColor(nTab) );
        auto pSheetEvents = rDoc.GetSheetEvents( nTab );
        pUndoDoc->SetSheetEvents( nTab, std::unique_ptr<ScSheetEvents>(pSheetEvents ? new ScSheetEvents(*pSheetEvents) : nullptr) );

        //  Drawing-Layer has to take care of its own undo!!!
        rDoc.BeginDrawUndo();                          //  DeleteTab generates SdrUndoDelPage

        pUndoData.reset(new ScRefUndoData( &rDoc ));
    }

    if (rDoc.DeleteTab(nTab))
    {
        if (bRecord)
        {
            vector<SCTAB> theTabs;
            theTabs.push_back(nTab);
            rDocShell.GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoDeleteTab>( &rDocShell, theTabs, std::move(pUndoDoc), std::move(pUndoData) ));
        }
        //  Update views:
        if( bVbaEnabled )
        {
            OUString sCodeName;
            if( rDoc.GetCodeName( nTab, sCodeName ) )
            {
                VBA_DeleteModule( rDocShell, sCodeName );
            }
        }
        rDocShell.Broadcast( ScTablesHint( SC_TAB_DELETED, nTab ) );

        if (bWasLinked)
        {
            rDocShell.UpdateLinks();                // update Link-Manager
            SfxBindings* pBindings = rDocShell.GetViewBindings();
            if (pBindings)
                pBindings->Invalidate(SID_LINKS);
        }

        rDocShell.PostPaintExtras();
        aModificator.SetDocumentModified();

        SfxApplication* pSfxApp = SfxGetpApp();                                // Navigator
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScDbAreasChanged ) );
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );

        bSuccess = true;
    }
    return bSuccess;
}

void ScDocFunc::SetTableVisible( SCTAB nTab, bool bVisible, bool bApi )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bUndo(rDoc.IsUndoEnabled());
    if ( rDoc.IsVisible( nTab ) == bVisible )
        return;                                // nothing to do - ok

    if ( !rDoc.IsDocEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScDocShellModificator aModificator( rDocShell );

    if ( !bVisible && !rDoc.IsImportingXML() )     // #i57869# allow hiding in any order for loading
    {
        // do not disable all sheets

        sal_uInt16 nVisCount = 0;
        SCTAB nCount = rDoc.GetTableCount();
        for (SCTAB i=0; i<nCount && nVisCount<2; i++)
            if (rDoc.IsVisible(i))
                ++nVisCount;

        if (nVisCount <= 1)
        {
            if (!bApi)
                rDocShell.ErrorMessage(STR_PROTECTIONERR);  //! separate error message?
            return;
        }
    }

    rDoc.SetVisible( nTab, bVisible );
    if (bUndo)
    {
        std::vector<SCTAB> undoTabs;
        undoTabs.push_back(nTab);
        rDocShell.GetUndoManager()->AddUndoAction( std::make_unique<ScUndoShowHideTab>( &rDocShell, undoTabs, bVisible ) );
    }

    //  update views
    if (!bVisible)
        rDocShell.Broadcast( ScTablesHint( SC_TAB_HIDDEN, nTab ) );

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
    rDocShell.PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PaintPartFlags::Extras);
    aModificator.SetDocumentModified();
}

bool ScDocFunc::SetLayoutRTL( SCTAB nTab, bool bRTL )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bUndo(rDoc.IsUndoEnabled());
    if ( rDoc.IsLayoutRTL( nTab ) == bRTL )
        return true;                                // nothing to do - ok

    //! protection (sheet or document?)

    ScDocShellModificator aModificator( rDocShell );

    rDoc.SetLayoutRTL( nTab, bRTL );

    if (bUndo)
    {
        rDocShell.GetUndoManager()->AddUndoAction( std::make_unique<ScUndoLayoutRTL>( &rDocShell, nTab, bRTL ) );
    }

    rDocShell.PostPaint( 0,0,0,MAXCOL,MAXROW,MAXTAB, PaintPartFlags::All );
    aModificator.SetDocumentModified();

    SfxBindings* pBindings = rDocShell.GetViewBindings();
    if (pBindings)
    {
        pBindings->Invalidate( FID_TAB_RTL );
        pBindings->Invalidate( SID_ATTR_SIZE );
    }

    return true;
}

bool ScDocFunc::RenameTable( SCTAB nTab, const OUString& rName, bool bRecord, bool bApi )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    if ( !rDoc.IsDocEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return false;
    }

    ScDocShellModificator aModificator( rDocShell );

    bool bSuccess = false;
    OUString sOldName;
    rDoc.GetName(nTab, sOldName);
    if (rDoc.RenameTab( nTab, rName ))
    {
        if (bRecord)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                            std::make_unique<ScUndoRenameTab>( &rDocShell, nTab, sOldName, rName));
        }
        rDocShell.PostPaintExtras();
        aModificator.SetDocumentModified();
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );

        bSuccess = true;
    }
    return bSuccess;
}

bool ScDocFunc::SetTabBgColor( SCTAB nTab, const Color& rColor, bool bRecord, bool bApi )
{

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    if ( !rDoc.IsDocEditable() || rDoc.IsTabProtected(nTab) )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR); //TODO Check to see what this string is...
        return false;
    }

    Color aOldTabBgColor;
    aOldTabBgColor = rDoc.GetTabBgColor(nTab);

    bool bSuccess = false;
    rDoc.SetTabBgColor(nTab, rColor);
    if ( rDoc.GetTabBgColor(nTab) == rColor)
        bSuccess = true;
    if (bSuccess)
    {
        if (bRecord)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoTabColor>( &rDocShell, nTab, aOldTabBgColor, rColor));
        }
        rDocShell.PostPaintExtras();
        ScDocShellModificator aModificator( rDocShell );
        aModificator.SetDocumentModified();
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );

        bSuccess = true;
    }
    return bSuccess;
}

bool ScDocFunc::SetTabBgColor(
    ScUndoTabColorInfo::List& rUndoTabColorList, bool bApi )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    if ( !rDoc.IsDocEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR); //TODO Get a better String Error...
        return false;
    }

    sal_uInt16 nTab;
    Color aNewTabBgColor;
    bool bSuccess = true;
    size_t nTabProtectCount = 0;
    size_t nTabListCount = rUndoTabColorList.size();
    for ( size_t i = 0; i < nTabListCount; ++i )
    {
        ScUndoTabColorInfo& rInfo = rUndoTabColorList[i];
        nTab = rInfo.mnTabId;
        if ( !rDoc.IsTabProtected(nTab) )
        {
            aNewTabBgColor = rInfo.maNewTabBgColor;
            rInfo.maOldTabBgColor = rDoc.GetTabBgColor(nTab);
            rDoc.SetTabBgColor(nTab, aNewTabBgColor);
            if ( rDoc.GetTabBgColor(nTab) != aNewTabBgColor)
            {
                bSuccess = false;
                break;
            }
        }
        else
        {
            nTabProtectCount++;
        }
    }

    if ( nTabProtectCount == nTabListCount )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR); //TODO Get a better String Error...
        return false;
    }

    if (bSuccess)
    {
        if (bRecord)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoTabColor>( &rDocShell, rUndoTabColorList));
        }
        rDocShell.PostPaintExtras();
        ScDocShellModificator aModificator( rDocShell );
        aModificator.SetDocumentModified();
    }
    return bSuccess;
}

//! SetWidthOrHeight - duplicated in ViewFunc !!!!!!
//! Problems:
//! - Optimal height of text cells is different for a printer and a screen
//! - Optimal width needs a selection in order to take only selected cells into account

static sal_uInt16 lcl_GetOptimalColWidth( ScDocShell& rDocShell, SCCOL nCol, SCTAB nTab )
{
    ScSizeDeviceProvider aProv(&rDocShell);
    OutputDevice* pDev = aProv.GetDevice();         // has pixel MapMode
    double nPPTX = aProv.GetPPTX();
    double nPPTY = aProv.GetPPTY();

    ScDocument& rDoc = rDocShell.GetDocument();
    Fraction aOne(1,1);
    sal_uInt16 nTwips = rDoc.GetOptimalColWidth( nCol, nTab, pDev, nPPTX, nPPTY, aOne, aOne,
                                                false/*bFormula*/ );

    return nTwips;
}

bool ScDocFunc::SetWidthOrHeight(
    bool bWidth, const std::vector<sc::ColRowSpan>& rRanges, SCTAB nTab,
    ScSizeMode eMode, sal_uInt16 nSizeTwips, bool bRecord, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    if (rRanges.empty())
        return true;

    ScDocument& rDoc = rDocShell.GetDocument();
    if ( bRecord && !rDoc.IsUndoEnabled() )
        bRecord = false;

    // import into read-only document is possible
    if ( !rDoc.IsChangeReadOnlyEnabled() && !rDocShell.IsEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);      //! separate error message?
        return false;
    }

    SCCOLROW nStart = rRanges[0].mnStart;
    SCCOLROW nEnd = rRanges[0].mnEnd;

    if ( eMode == SC_SIZE_OPTIMAL )
    {
        //! Option "Show formulas" - but where to get them from?
    }

    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScOutlineTable> pUndoTab;
    std::vector<sc::ColRowSpan> aUndoRanges;

    if ( bRecord )
    {
        rDoc.BeginDrawUndo();                          // Drawing Updates

        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        if (bWidth)
        {
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, true );
            rDoc.CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab, static_cast<SCCOL>(nEnd), MAXROW, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc );
        }
        else
        {
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, true );
            rDoc.CopyToDocument( 0, static_cast<SCROW>(nStart), nTab, MAXCOL, static_cast<SCROW>(nEnd), nTab, InsertDeleteFlags::NONE, false, *pUndoDoc );
        }

        aUndoRanges = rRanges;

        ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );
        if (pTable)
            pUndoTab.reset(new ScOutlineTable( *pTable ));
    }

    bool bShow = nSizeTwips > 0 || eMode != SC_SIZE_DIRECT;
    bool bOutline = false;

    for (const sc::ColRowSpan& rRange : rRanges)
    {
        SCCOLROW nStartNo = rRange.mnStart;
        SCCOLROW nEndNo   = rRange.mnEnd;

        if ( !bWidth )                      // deal with heights always in blocks
        {
            if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
            {
                bool bAll = ( eMode==SC_SIZE_OPTIMAL );
                if (!bAll)
                {
                    //  delete for all that have CRFlags::ManualSize enabled
                    //  then SetOptimalHeight with bShrink = FALSE
                    for (SCROW nRow=nStartNo; nRow<=nEndNo; nRow++)
                    {
                        CRFlags nOld = rDoc.GetRowFlags(nRow,nTab);
                        SCROW nLastRow = -1;
                        bool bHidden = rDoc.RowHidden(nRow, nTab, nullptr, &nLastRow);
                        if ( !bHidden && ( nOld & CRFlags::ManualSize ) )
                            rDoc.SetRowFlags( nRow, nTab, nOld & ~CRFlags::ManualSize );
                    }
                }

                ScSizeDeviceProvider aProv( &rDocShell );
                Fraction aOne(1,1);
                sc::RowHeightContext aCxt(aProv.GetPPTX(), aProv.GetPPTY(), aOne, aOne, aProv.GetDevice());
                aCxt.setForceAutoSize(bAll);
                rDoc.SetOptimalHeight(aCxt, nStartNo, nEndNo, nTab);

                if (bAll)
                    rDoc.ShowRows( nStartNo, nEndNo, nTab, true );

                //  Manual flag will be set already in SetOptimalHeight if bAll=true
                //  (it is on when Extra-Height, otherwise off).
            }
            else if ( eMode==SC_SIZE_DIRECT || eMode==SC_SIZE_ORIGINAL )
            {
                if (nSizeTwips)
                {
                    rDoc.SetRowHeightRange( nStartNo, nEndNo, nTab, nSizeTwips );
                    rDoc.SetManualHeight( nStartNo, nEndNo, nTab, true );          // height was set manually
                }
                if ( eMode != SC_SIZE_ORIGINAL )
                    rDoc.ShowRows( nStartNo, nEndNo, nTab, nSizeTwips != 0 );
            }
            else if ( eMode==SC_SIZE_SHOW )
            {
                rDoc.ShowRows( nStartNo, nEndNo, nTab, true );
            }
        }
        else                                // Column widths
        {
            for (SCCOL nCol=static_cast<SCCOL>(nStartNo); nCol<=static_cast<SCCOL>(nEndNo); nCol++)
            {
                if ( eMode != SC_SIZE_VISOPT || !rDoc.ColHidden(nCol, nTab) )
                {
                    sal_uInt16 nThisSize = nSizeTwips;

                    if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
                        nThisSize = nSizeTwips +
                                    lcl_GetOptimalColWidth( rDocShell, nCol, nTab );
                    if ( nThisSize )
                        rDoc.SetColWidth( nCol, nTab, nThisSize );

                    if ( eMode != SC_SIZE_ORIGINAL )
                        rDoc.ShowCol( nCol, nTab, bShow );
                }
            }
        }

                            //  adjust outlines

        if ( eMode != SC_SIZE_ORIGINAL )
        {
            if (bWidth)
                bOutline = bOutline || rDoc.UpdateOutlineCol(
                        static_cast<SCCOL>(nStartNo),
                        static_cast<SCCOL>(nEndNo), nTab, bShow );
            else
                bOutline = bOutline || rDoc.UpdateOutlineRow(
                        static_cast<SCROW>(nStartNo),
                        static_cast<SCROW>(nEndNo), nTab, bShow );
        }
    }
    rDoc.SetDrawPageSize(nTab);

    if (!bOutline)
        pUndoTab.reset();

    if (bRecord)
    {
        ScMarkData aMark;
        aMark.SelectOneTable( nTab );
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoWidthOrHeight>(
                &rDocShell, aMark, nStart, nTab, nEnd, nTab, std::move(pUndoDoc),
                aUndoRanges, std::move(pUndoTab), eMode, nSizeTwips, bWidth));
    }

    rDoc.UpdatePageBreaks( nTab );

    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();
    if (pViewSh)
        pViewSh->OnLOKSetWidthOrHeight(nStart, bWidth);

    rDocShell.PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PaintPartFlags::All);
    aModificator.SetDocumentModified();

    return false;
}

bool ScDocFunc::InsertPageBreak( bool bColumn, const ScAddress& rPos,
                                bool bRecord, bool bSetModified )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    SCTAB nTab = rPos.Tab();
    SfxBindings* pBindings = rDocShell.GetViewBindings();

    SCCOLROW nPos = bColumn ? static_cast<SCCOLROW>(rPos.Col()) :
        static_cast<SCCOLROW>(rPos.Row());
    if (nPos == 0)
        return false;                   // first column / row

    ScBreakType nBreak = bColumn ?
        rDoc.HasColBreak(static_cast<SCCOL>(nPos), nTab) :
        rDoc.HasRowBreak(static_cast<SCROW>(nPos), nTab);
    if (nBreak & ScBreakType::Manual)
        return true;

    if (bRecord)
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoPageBreak>( &rDocShell, rPos.Col(), rPos.Row(), nTab, bColumn, true ) );

    if (bColumn)
        rDoc.SetColBreak(static_cast<SCCOL>(nPos), nTab, false, true);
    else
        rDoc.SetRowBreak(static_cast<SCROW>(nPos), nTab, false, true);

    rDoc.InvalidatePageBreaks(nTab);
    rDoc.UpdatePageBreaks( nTab );

    rDoc.SetStreamValid(nTab, false);

    if (bColumn)
    {
        rDocShell.PostPaint( static_cast<SCCOL>(nPos)-1, 0, nTab, MAXCOL, MAXROW, nTab, PaintPartFlags::Grid );
        if (pBindings)
        {
            pBindings->Invalidate( FID_INS_COLBRK );
            pBindings->Invalidate( FID_DEL_COLBRK );
        }
    }
    else
    {
        rDocShell.PostPaint( 0, static_cast<SCROW>(nPos)-1, nTab, MAXCOL, MAXROW, nTab, PaintPartFlags::Grid );
        if (pBindings)
        {
            pBindings->Invalidate( FID_INS_ROWBRK );
            pBindings->Invalidate( FID_DEL_ROWBRK );
        }
    }
    if (pBindings)
        pBindings->Invalidate( FID_DEL_MANUALBREAKS );

    if (bSetModified)
        aModificator.SetDocumentModified();

    return true;
}

bool ScDocFunc::RemovePageBreak( bool bColumn, const ScAddress& rPos,
                                bool bRecord, bool bSetModified )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    SCTAB nTab = rPos.Tab();
    SfxBindings* pBindings = rDocShell.GetViewBindings();

    SCCOLROW nPos = bColumn ? static_cast<SCCOLROW>(rPos.Col()) :
        static_cast<SCCOLROW>(rPos.Row());

    ScBreakType nBreak;
    if (bColumn)
        nBreak = rDoc.HasColBreak(static_cast<SCCOL>(nPos), nTab);
    else
        nBreak = rDoc.HasRowBreak(static_cast<SCROW>(nPos), nTab);
    if (!(nBreak & ScBreakType::Manual))
        // There is no manual break.
        return false;

    if (bRecord)
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoPageBreak>( &rDocShell, rPos.Col(), rPos.Row(), nTab, bColumn, false ) );

    if (bColumn)
        rDoc.RemoveColBreak(static_cast<SCCOL>(nPos), nTab, false, true);
    else
        rDoc.RemoveRowBreak(static_cast<SCROW>(nPos), nTab, false, true);

    rDoc.UpdatePageBreaks( nTab );

    rDoc.SetStreamValid(nTab, false);

    if (bColumn)
    {
        rDocShell.PostPaint( static_cast<SCCOL>(nPos)-1, 0, nTab, MAXCOL, MAXROW, nTab, PaintPartFlags::Grid );
        if (pBindings)
        {
            pBindings->Invalidate( FID_INS_COLBRK );
            pBindings->Invalidate( FID_DEL_COLBRK );
        }
    }
    else
    {
        rDocShell.PostPaint( 0, nPos-1, nTab, MAXCOL, MAXROW, nTab, PaintPartFlags::Grid );
        if (pBindings)
        {
            pBindings->Invalidate( FID_INS_ROWBRK );
            pBindings->Invalidate( FID_DEL_ROWBRK );
        }
    }
    if (pBindings)
        pBindings->Invalidate( FID_DEL_MANUALBREAKS );

    if (bSetModified)
        aModificator.SetDocumentModified();

    return true;
}

void ScDocFunc::ProtectSheet( SCTAB nTab, const ScTableProtection& rProtect )
{
    ScDocument& rDoc = rDocShell.GetDocument();

    rDoc.SetTabProtection(nTab, &rProtect);
    if (rDoc.IsUndoEnabled())
    {
        ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
        OSL_ENSURE(pProtect, "ScDocFunc::Unprotect: ScTableProtection pointer is NULL!");
        if (pProtect)
        {
            ::std::unique_ptr<ScTableProtection> p(new ScTableProtection(*pProtect));
            p->setProtected(true); // just in case ...
            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoTabProtect>(&rDocShell, nTab, std::move(p)) );

            // ownership of unique_ptr now transferred to ScUndoTabProtect.
        }
    }

    rDocShell.PostPaintGridAll();
    ScDocShellModificator aModificator(rDocShell);
    aModificator.SetDocumentModified();
}

bool ScDocFunc::Protect( SCTAB nTab, const OUString& rPassword )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    if (nTab == TABLEID_DOC)
    {
        // document protection
        ScDocProtection aProtection;
        aProtection.setProtected(true);
        aProtection.setPassword(rPassword);
        rDoc.SetDocProtection(&aProtection);
        if (rDoc.IsUndoEnabled())
        {
            ScDocProtection* pProtect = rDoc.GetDocProtection();
            OSL_ENSURE(pProtect, "ScDocFunc::Unprotect: ScDocProtection pointer is NULL!");
            if (pProtect)
            {
                ::std::unique_ptr<ScDocProtection> p(new ScDocProtection(*pProtect));
                p->setProtected(true); // just in case ...
                rDocShell.GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoDocProtect>(&rDocShell, std::move(p)) );
                // ownership of unique_ptr is transferred to ScUndoDocProtect.
            }
        }
    }
    else
    {
        // sheet protection

        const ScTableProtection* pOldProtection = rDoc.GetTabProtection(nTab);
        ::std::unique_ptr<ScTableProtection> pNewProtection(pOldProtection ? new ScTableProtection(*pOldProtection) : new ScTableProtection());
        pNewProtection->setProtected(true);
        pNewProtection->setPassword(rPassword);
        rDoc.SetTabProtection(nTab, pNewProtection.get());
        if (rDoc.IsUndoEnabled())
        {
            ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
            OSL_ENSURE(pProtect, "ScDocFunc::Unprotect: ScTableProtection pointer is NULL!");
            if (pProtect)
            {
                ::std::unique_ptr<ScTableProtection> p(new ScTableProtection(*pProtect));
                p->setProtected(true); // just in case ...
                rDocShell.GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoTabProtect>(&rDocShell, nTab, std::move(p)) );
                // ownership of unique_ptr now transferred to ScUndoTabProtect.
            }
        }
    }

    rDocShell.PostPaintGridAll();
    ScDocShellModificator aModificator( rDocShell );
    aModificator.SetDocumentModified();

    return true;
}

bool ScDocFunc::Unprotect( SCTAB nTab, const OUString& rPassword, bool bApi )
{
    ScDocument& rDoc = rDocShell.GetDocument();

    if (nTab == TABLEID_DOC)
    {
        // document protection

        ScDocProtection* pDocProtect = rDoc.GetDocProtection();
        if (!pDocProtect || !pDocProtect->isProtected())
            // already unprotected (should not happen)!
            return true;

        // save the protection state before unprotect (for undo).
        ::std::unique_ptr<ScDocProtection> pProtectCopy(new ScDocProtection(*pDocProtect));

        if (!pDocProtect->verifyPassword(rPassword))
        {
            if (!bApi)
            {
                vcl::Window* pWin = ScDocShell::GetActiveDialogParent();
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              ScResId(SCSTR_WRONGPASSWORD)));
                xInfoBox->run();
            }
            return false;
        }

        rDoc.SetDocProtection(nullptr);
        if (rDoc.IsUndoEnabled())
        {
            pProtectCopy->setProtected(false);
            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoDocProtect>(&rDocShell, std::move(pProtectCopy)) );
            // ownership of unique_ptr now transferred to ScUndoDocProtect.
        }
    }
    else
    {
        // sheet protection

        ScTableProtection* pTabProtect = rDoc.GetTabProtection(nTab);
        if (!pTabProtect || !pTabProtect->isProtected())
            // already unprotected (should not happen)!
            return true;

        // save the protection state before unprotect (for undo).
        ::std::unique_ptr<ScTableProtection> pProtectCopy(new ScTableProtection(*pTabProtect));
        if (!pTabProtect->verifyPassword(rPassword))
        {
            if (!bApi)
            {
                vcl::Window* pWin = ScDocShell::GetActiveDialogParent();
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              ScResId(SCSTR_WRONGPASSWORD)));
                xInfoBox->run();
            }
            return false;
        }

        ::std::unique_ptr<ScTableProtection> pNewProtection(new ScTableProtection(*pTabProtect));
        pNewProtection->setProtected(false);
        rDoc.SetTabProtection(nTab, pNewProtection.get());
        if (rDoc.IsUndoEnabled())
        {
            pProtectCopy->setProtected(false);
            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoTabProtect>(&rDocShell, nTab, std::move(pProtectCopy)) );
            // ownership of unique_ptr now transferred to ScUndoTabProtect.
        }
    }

    rDocShell.PostPaintGridAll();
    ScDocShellModificator aModificator( rDocShell );
    aModificator.SetDocumentModified();

    return true;
}

void ScDocFunc::ClearItems( const ScMarkData& rMark, const sal_uInt16* pWhich, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    bool bUndo (rDoc.IsUndoEnabled());
    ScEditableTester aTester( &rDoc, rMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return;
    }

    //  #i12940# ClearItems is called (from setPropertyToDefault) directly with uno object's cached
    //  MarkData (GetMarkData), so rMark must be changed to multi selection for ClearSelectionItems
    //  here.

    ScRange aMarkRange;
    ScMarkData aMultiMark = rMark;
    aMultiMark.SetMarking(false);       // for MarkToMulti
    aMultiMark.MarkToMulti();
    aMultiMark.GetMultiMarkArea( aMarkRange );

    if (bUndo)
    {
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCTAB nEndTab = aMarkRange.aEnd.Tab();

        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( &rDoc, nStartTab, nEndTab );
        rDoc.CopyToDocument( aMarkRange, InsertDeleteFlags::ATTRIB, true, *pUndoDoc, &aMultiMark );

        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoClearItems>( &rDocShell, aMultiMark, std::move(pUndoDoc), pWhich ) );
    }

    rDoc.ClearSelectionItems( pWhich, aMultiMark );

    rDocShell.PostPaint( aMarkRange, PaintPartFlags::Grid, SC_PF_LINES | SC_PF_TESTMERGE );
    aModificator.SetDocumentModified();

    //! Bindings-Invalidate etc.?
}

bool ScDocFunc::ChangeIndent( const ScMarkData& rMark, bool bIncrement, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    bool bUndo(rDoc.IsUndoEnabled());
    ScEditableTester aTester( &rDoc, rMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    ScRange aMarkRange;
    rMark.GetMultiMarkArea( aMarkRange );

    if (bUndo)
    {
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCTAB nTabCount = rDoc.GetTableCount();

        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( &rDoc, nStartTab, nStartTab );
        for (const auto& rTab : rMark)
        {
            if (rTab >= nTabCount)
                break;

            if (rTab != nStartTab)
                pUndoDoc->AddUndoTab( rTab, rTab );
        }

        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        rDoc.CopyToDocument( aCopyRange, InsertDeleteFlags::ATTRIB, true, *pUndoDoc, &rMark );

        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoIndent>( &rDocShell, rMark, std::move(pUndoDoc), bIncrement ) );
    }

    rDoc.ChangeSelectionIndent( bIncrement, rMark );

    rDocShell.PostPaint( aMarkRange, PaintPartFlags::Grid, SC_PF_LINES | SC_PF_TESTMERGE );
    aModificator.SetDocumentModified();

    SfxBindings* pBindings = rDocShell.GetViewBindings();
    if (pBindings)
    {
        pBindings->Invalidate( SID_ALIGNLEFT );         // ChangeIndent aligns left
        pBindings->Invalidate( SID_ALIGNRIGHT );
        pBindings->Invalidate( SID_ALIGNBLOCK );
        pBindings->Invalidate( SID_ALIGNCENTERHOR );
        pBindings->Invalidate( SID_ATTR_LRSPACE );
        pBindings->Invalidate( SID_ATTR_PARA_ADJUST_LEFT );
        pBindings->Invalidate( SID_ATTR_PARA_ADJUST_RIGHT );
        pBindings->Invalidate( SID_ATTR_PARA_ADJUST_BLOCK );
        pBindings->Invalidate( SID_ATTR_PARA_ADJUST_CENTER);
        // pseudo slots for Format menu
        pBindings->Invalidate( SID_ALIGN_ANY_HDEFAULT );
        pBindings->Invalidate( SID_ALIGN_ANY_LEFT );
        pBindings->Invalidate( SID_ALIGN_ANY_HCENTER );
        pBindings->Invalidate( SID_ALIGN_ANY_RIGHT );
        pBindings->Invalidate( SID_ALIGN_ANY_JUSTIFIED );
    }

    return true;
}

bool ScDocFunc::AutoFormat( const ScRange& rRange, const ScMarkData* pTabMark,
                            sal_uInt16 nFormatNo, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;
    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, true );
    }

    ScAutoFormat* pAutoFormat = ScGlobal::GetOrCreateAutoFormat();
    ScEditableTester aTester( &rDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
    if ( nFormatNo < pAutoFormat->size() && aTester.IsEditable() )
    {
        WaitObject aWait( ScDocShell::GetActiveDialogParent() );

        bool bSize = pAutoFormat->findByIndex(nFormatNo)->GetIncludeWidthHeight();

        SCTAB nTabCount = rDoc.GetTableCount();
        ScDocumentUniquePtr pUndoDoc;
        if ( bRecord )
        {
            pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nStartTab, nStartTab, bSize, bSize );
            for (const auto& rTab : aMark)
            {
                if (rTab >= nTabCount)
                    break;

                if (rTab != nStartTab)
                    pUndoDoc->AddUndoTab( rTab, rTab, bSize, bSize );
            }

            ScRange aCopyRange = rRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aStart.SetTab(nTabCount-1);
            rDoc.CopyToDocument( aCopyRange, InsertDeleteFlags::ATTRIB, false, *pUndoDoc, &aMark );
            if (bSize)
            {
                rDoc.CopyToDocument( nStartCol,0,0, nEndCol,MAXROW,nTabCount-1,
                                                            InsertDeleteFlags::NONE, false, *pUndoDoc, &aMark );
                rDoc.CopyToDocument( 0,nStartRow,0, MAXCOL,nEndRow,nTabCount-1,
                                                            InsertDeleteFlags::NONE, false, *pUndoDoc, &aMark );
            }
            rDoc.BeginDrawUndo();
        }

        rDoc.AutoFormat( nStartCol, nStartRow, nEndCol, nEndRow, nFormatNo, aMark );

        if (bSize)
        {
            std::vector<sc::ColRowSpan> aCols(1, sc::ColRowSpan(nStartCol,nEndCol));
            std::vector<sc::ColRowSpan> aRows(1, sc::ColRowSpan(nStartRow,nEndRow));

            for (const auto& rTab : aMark)
            {
                if (rTab >= nTabCount)
                    break;

                SetWidthOrHeight(true, aCols, rTab, SC_SIZE_VISOPT, STD_EXTRA_WIDTH, false, true);
                SetWidthOrHeight(false, aRows, rTab, SC_SIZE_VISOPT, 0, false, false);
                rDocShell.PostPaint( 0,0,rTab, MAXCOL,MAXROW,rTab,
                                PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top );
            }
        }
        else
        {
            for (const auto& rTab : aMark)
            {
                if (rTab >= nTabCount)
                    break;

                bool bAdj = AdjustRowHeight( ScRange(nStartCol, nStartRow, rTab,
                                                    nEndCol, nEndRow, rTab), false );
                if (bAdj)
                    rDocShell.PostPaint( 0,nStartRow,rTab, MAXCOL,MAXROW,rTab,
                                        PaintPartFlags::Grid | PaintPartFlags::Left );
                else
                    rDocShell.PostPaint( nStartCol, nStartRow, rTab,
                                        nEndCol, nEndRow, rTab, PaintPartFlags::Grid );
            }
        }

        if ( bRecord )      // only now is Draw-Undo available
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoAutoFormat>( &rDocShell, rRange, std::move(pUndoDoc), aMark, bSize, nFormatNo ) );
        }

        aModificator.SetDocumentModified();
    }
    else if (!bApi)
        rDocShell.ErrorMessage(aTester.GetMessageId());

    return false;
}

bool ScDocFunc::EnterMatrix( const ScRange& rRange, const ScMarkData* pTabMark,
        const ScTokenArray* pTokenArray, const OUString& rString, bool bApi, bool bEnglish,
        const OUString& rFormulaNmsp, const formula::FormulaGrammar::Grammar eGrammar )
{
    if (ScViewData::SelectionFillDOOM( rRange ))
        return false;

    ScDocShellModificator aModificator( rDocShell );

    bool bSuccess = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, true );
    }

    ScEditableTester aTester( &rDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
    if ( aTester.IsEditable() )
    {
        WaitObject aWait( ScDocShell::GetActiveDialogParent() );

        ScDocumentUniquePtr pUndoDoc;

        const bool bUndo(rDoc.IsUndoEnabled());
        if (bUndo)
        {
            //! take selected sheets into account also when undoing
            pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nStartTab, nEndTab );
            rDoc.CopyToDocument( rRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, *pUndoDoc );
        }

        // use TokenArray if given, string (and flags) otherwise
        if ( pTokenArray )
        {
            rDoc.InsertMatrixFormula( nStartCol, nStartRow, nEndCol, nEndRow,
                    aMark, EMPTY_OUSTRING, pTokenArray, eGrammar);
        }
        else if ( rDoc.IsImportingXML() )
        {
            ScTokenArray aCode;
            aCode.AssignXMLString( rString,
                    ((eGrammar == formula::FormulaGrammar::GRAM_EXTERNAL) ? rFormulaNmsp : OUString()));
            rDoc.InsertMatrixFormula( nStartCol, nStartRow, nEndCol, nEndRow,
                    aMark, EMPTY_OUSTRING, &aCode, eGrammar);
            rDoc.IncXMLImportedFormulaCount( rString.getLength() );
        }
        else if (bEnglish)
        {
            ScCompiler aComp( &rDoc, rRange.aStart, eGrammar);
            std::unique_ptr<ScTokenArray> pCode = aComp.CompileString( rString );
            rDoc.InsertMatrixFormula( nStartCol, nStartRow, nEndCol, nEndRow,
                    aMark, EMPTY_OUSTRING, pCode.get(), eGrammar);
        }
        else
            rDoc.InsertMatrixFormula( nStartCol, nStartRow, nEndCol, nEndRow,
                    aMark, rString, nullptr, eGrammar);

        if (bUndo)
        {
            //! take selected sheets into account also when undoing
            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoEnterMatrix>( &rDocShell, rRange, std::move(pUndoDoc), rString ) );
        }

        //  Err522 painting of DDE-Formulas will be intercepted during interpreting
        rDocShell.PostPaint( nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab, PaintPartFlags::Grid );
        aModificator.SetDocumentModified();

        bSuccess = true;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}

bool ScDocFunc::TabOp( const ScRange& rRange, const ScMarkData* pTabMark,
                            const ScTabOpParam& rParam, bool bRecord, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    bool bSuccess = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, true );
    }

    ScEditableTester aTester( &rDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
    if ( aTester.IsEditable() )
    {
        WaitObject aWait( ScDocShell::GetActiveDialogParent() );
        rDoc.SetDirty( rRange, false );
        if ( bRecord )
        {
            //! take selected sheets into account also when undoing
            ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nStartTab, nEndTab );
            rDoc.CopyToDocument( rRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, *pUndoDoc );

            rDocShell.GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoTabOp>( &rDocShell,
                                     nStartCol, nStartRow, nStartTab,
                                     nEndCol, nEndRow, nEndTab, std::move(pUndoDoc),
                                     rParam.aRefFormulaCell,
                                     rParam.aRefFormulaEnd,
                                     rParam.aRefRowCell,
                                     rParam.aRefColCell,
                                     rParam.meMode) );
        }
        rDoc.InsertTableOp(rParam, nStartCol, nStartRow, nEndCol, nEndRow, aMark);
        rDocShell.PostPaintGridAll();
        aModificator.SetDocumentModified();
        bSuccess = true;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}

static ScDirection DirFromFillDir( FillDir eDir )
{
    if (eDir==FILL_TO_BOTTOM)
        return DIR_BOTTOM;
    else if (eDir==FILL_TO_RIGHT)
        return DIR_RIGHT;
    else if (eDir==FILL_TO_TOP)
        return DIR_TOP;
    else // if (eDir==FILL_TO_LEFT)
        return DIR_LEFT;
}

namespace {

/**
 * Expand the fill range as necessary, to allow copying of adjacent cell(s)
 * even when those cells are not in the original range.
 */
void adjustFillRangeForAdjacentCopy(ScRange& rRange, FillDir eDir)
{
    switch (eDir)
    {
        case FILL_TO_BOTTOM:
        {
            if (rRange.aStart.Row() == 0)
                return;

            if (rRange.aStart.Row() != rRange.aEnd.Row())
                return;

            // Include the above row.
            ScAddress& s = rRange.aStart;
            s.SetRow(s.Row()-1);
        }
        break;
        case FILL_TO_TOP:
        {
            if (rRange.aStart.Row() == MAXROW)
                return;

            if (rRange.aStart.Row() != rRange.aEnd.Row())
                return;

            // Include the row below.
            ScAddress& e = rRange.aEnd;
            e.SetRow(e.Row()+1);
        }
        break;
        case FILL_TO_LEFT:
        {
            if (rRange.aStart.Col() == MAXCOL)
                return;

            if (rRange.aStart.Col() != rRange.aEnd.Col())
                return;

            // Include the column to the right.
            ScAddress& e = rRange.aEnd;
            e.SetCol(e.Col()+1);
        }
        break;
        case FILL_TO_RIGHT:
        {
            if (rRange.aStart.Col() == 0)
                return;

            if (rRange.aStart.Col() != rRange.aEnd.Col())
                return;

            // Include the column to the left.
            ScAddress& s = rRange.aStart;
            s.SetCol(s.Col()-1);
        }
        break;
        default:
            ;
    }
}

}

bool ScDocFunc::FillSimple( const ScRange& rRange, const ScMarkData* pTabMark,
                            FillDir eDir, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument& rDoc = rDocShell.GetDocument();

    bool bSuccess = false;
    ScRange aRange = rRange;
    adjustFillRangeForAdjacentCopy(aRange, eDir);

    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCTAB nStartTab = aRange.aStart.Tab();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    SCTAB nEndTab = aRange.aEnd.Tab();

    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, true );
    }

    ScEditableTester aTester( &rDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
    if ( aTester.IsEditable() )
    {
        WaitObject aWait( ScDocShell::GetActiveDialogParent() );

        ScRange aSourceArea = aRange;
        ScRange aDestArea   = aRange;

        SCCOLROW nCount = 0;
        switch (eDir)
        {
            case FILL_TO_BOTTOM:
                nCount = aSourceArea.aEnd.Row()-aSourceArea.aStart.Row();
                aSourceArea.aEnd.SetRow( aSourceArea.aStart.Row() );
                break;
            case FILL_TO_RIGHT:
                nCount = aSourceArea.aEnd.Col()-aSourceArea.aStart.Col();
                aSourceArea.aEnd.SetCol( aSourceArea.aStart.Col() );
                break;
            case FILL_TO_TOP:
                nCount = aSourceArea.aEnd.Row()-aSourceArea.aStart.Row();
                aSourceArea.aStart.SetRow( aSourceArea.aEnd.Row() );
                break;
            case FILL_TO_LEFT:
                nCount = aSourceArea.aEnd.Col()-aSourceArea.aStart.Col();
                aSourceArea.aStart.SetCol( aSourceArea.aEnd.Col() );
                break;
        }

        ScDocumentUniquePtr pUndoDoc;
        if ( bRecord )
        {
            SCTAB nTabCount = rDoc.GetTableCount();
            SCTAB nDestStartTab = aDestArea.aStart.Tab();

            pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nDestStartTab, nDestStartTab );
            for (const auto& rTab : aMark)
            {
                if (rTab >= nTabCount)
                    break;

                if (rTab != nDestStartTab)
                    pUndoDoc->AddUndoTab( rTab, rTab );
            }

            ScRange aCopyRange = aDestArea;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);
            rDoc.CopyToDocument( aCopyRange, InsertDeleteFlags::AUTOFILL, false, *pUndoDoc, &aMark );
        }

        sal_uLong nProgCount;
        if (eDir == FILL_TO_BOTTOM || eDir == FILL_TO_TOP)
            nProgCount = aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1;
        else
            nProgCount = aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1;
        nProgCount *= nCount;
        ScProgress aProgress( rDoc.GetDocumentShell(),
                ScResId(STR_FILL_SERIES_PROGRESS), nProgCount, true );

        rDoc.Fill( aSourceArea.aStart.Col(), aSourceArea.aStart.Row(),
                aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), &aProgress,
                aMark, nCount, eDir, FILL_SIMPLE );
        AdjustRowHeight(aRange);

        if ( bRecord )      // only now is Draw-Undo available
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoAutoFill>( &rDocShell, aDestArea, aSourceArea, std::move(pUndoDoc), aMark,
                                    eDir, FILL_SIMPLE, FILL_DAY, MAXDOUBLE, 1.0, 1e307) );
        }

        rDocShell.PostPaintGridAll();
        aModificator.SetDocumentModified();

        bSuccess = true;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}

bool ScDocFunc::FillSeries( const ScRange& rRange, const ScMarkData* pTabMark,
                            FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                            double fStart, double fStep, double fMax,
                            bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    bool bSuccess = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, true );
    }

    ScEditableTester aTester( &rDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
    if ( aTester.IsEditable() )
    {
        WaitObject aWait( ScDocShell::GetActiveDialogParent() );

        ScRange aSourceArea = rRange;
        ScRange aDestArea   = rRange;

        SCSIZE nCount = rDoc.GetEmptyLinesInBlock(
                aSourceArea.aStart.Col(), aSourceArea.aStart.Row(), aSourceArea.aStart.Tab(),
                aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), aSourceArea.aEnd.Tab(),
                DirFromFillDir(eDir) );

        //  keep at least one row/column as source range
        SCSIZE nTotLines = ( eDir == FILL_TO_BOTTOM || eDir == FILL_TO_TOP ) ?
            static_cast<SCSIZE>( aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1 ) :
            static_cast<SCSIZE>( aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1 );
        if ( nCount >= nTotLines )
            nCount = nTotLines - 1;

        switch (eDir)
        {
            case FILL_TO_BOTTOM:
                aSourceArea.aEnd.SetRow( sal::static_int_cast<SCROW>( aSourceArea.aEnd.Row() - nCount ) );
                break;
            case FILL_TO_RIGHT:
                aSourceArea.aEnd.SetCol( sal::static_int_cast<SCCOL>( aSourceArea.aEnd.Col() - nCount ) );
                break;
            case FILL_TO_TOP:
                aSourceArea.aStart.SetRow( sal::static_int_cast<SCROW>( aSourceArea.aStart.Row() + nCount ) );
                break;
            case FILL_TO_LEFT:
                aSourceArea.aStart.SetCol( sal::static_int_cast<SCCOL>( aSourceArea.aStart.Col() + nCount ) );
                break;
        }

        ScDocumentUniquePtr pUndoDoc;
        if ( bRecord )
        {
            SCTAB nTabCount = rDoc.GetTableCount();
            SCTAB nDestStartTab = aDestArea.aStart.Tab();

            pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nDestStartTab, nDestStartTab );
            for (const auto& rTab : aMark)
            {
                if (rTab >= nTabCount)
                    break;

                if (rTab != nDestStartTab)
                    pUndoDoc->AddUndoTab( rTab, rTab );
            }

            rDoc.CopyToDocument(
                aDestArea.aStart.Col(), aDestArea.aStart.Row(), 0,
                aDestArea.aEnd.Col(), aDestArea.aEnd.Row(), nTabCount-1,
                InsertDeleteFlags::AUTOFILL, false, *pUndoDoc, &aMark );
        }

        if (aDestArea.aStart.Col() <= aDestArea.aEnd.Col() &&
            aDestArea.aStart.Row() <= aDestArea.aEnd.Row())
        {
            if ( fStart != MAXDOUBLE )
            {
                SCCOL nValX = (eDir == FILL_TO_LEFT) ? aDestArea.aEnd.Col() : aDestArea.aStart.Col();
                SCROW nValY = (eDir == FILL_TO_TOP ) ? aDestArea.aEnd.Row() : aDestArea.aStart.Row();
                SCTAB nTab = aDestArea.aStart.Tab();
                rDoc.SetValue( nValX, nValY, nTab, fStart );
            }

            sal_uLong nProgCount;
            if (eDir == FILL_TO_BOTTOM || eDir == FILL_TO_TOP)
                nProgCount = aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1;
            else
                nProgCount = aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1;
            nProgCount *= nCount;
            ScProgress aProgress( rDoc.GetDocumentShell(),
                    ScResId(STR_FILL_SERIES_PROGRESS), nProgCount, true );

            rDoc.Fill( aSourceArea.aStart.Col(), aSourceArea.aStart.Row(),
                        aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), &aProgress,
                        aMark, nCount, eDir, eCmd, eDateCmd, fStep, fMax );
            AdjustRowHeight(rRange);

            rDocShell.PostPaintGridAll();
            aModificator.SetDocumentModified();
        }

        if ( bRecord )      // only now is Draw-Undo available
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoAutoFill>( &rDocShell, aDestArea, aSourceArea, std::move(pUndoDoc), aMark,
                                    eDir, eCmd, eDateCmd, fStart, fStep, fMax) );
        }

        bSuccess = true;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}

bool ScDocFunc::FillAuto( ScRange& rRange, const ScMarkData* pTabMark,
                            FillDir eDir, sal_uLong nCount, bool bApi )
{
    return FillAuto( rRange, pTabMark, eDir, FILL_AUTO, FILL_DAY, nCount, 1.0/*fStep*/, MAXDOUBLE/*fMax*/, true/*bRecord*/, bApi );
}

bool ScDocFunc::FillAuto( ScRange& rRange, const ScMarkData* pTabMark, FillDir eDir, FillCmd eCmd, FillDateCmd  eDateCmd, sal_uLong nCount, double fStep, double fMax,  bool bRecord, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, true );
    }

    ScRange aSourceArea = rRange;
    ScRange aDestArea   = rRange;

    switch (eDir)
    {
        case FILL_TO_BOTTOM:
            aDestArea.aEnd.SetRow( sal::static_int_cast<SCROW>( aSourceArea.aEnd.Row() + nCount ) );
            break;
        case FILL_TO_TOP:
            if (nCount > sal::static_int_cast<sal_uLong>( aSourceArea.aStart.Row() ))
            {
                OSL_FAIL("FillAuto: Row < 0");
                nCount = aSourceArea.aStart.Row();
            }
            aDestArea.aStart.SetRow( sal::static_int_cast<SCROW>( aSourceArea.aStart.Row() - nCount ) );
            break;
        case FILL_TO_RIGHT:
            aDestArea.aEnd.SetCol( sal::static_int_cast<SCCOL>( aSourceArea.aEnd.Col() + nCount ) );
            break;
        case FILL_TO_LEFT:
            if (nCount > sal::static_int_cast<sal_uLong>( aSourceArea.aStart.Col() ))
            {
                OSL_FAIL("FillAuto: Col < 0");
                nCount = aSourceArea.aStart.Col();
            }
            aDestArea.aStart.SetCol( sal::static_int_cast<SCCOL>( aSourceArea.aStart.Col() - nCount ) );
            break;
        default:
            OSL_FAIL("Wrong direction with FillAuto");
            break;
    }

    //      Test for cell protection
    //!     Source range can be protected !!!
    //!     but can't contain matrix fragments !!!

    ScEditableTester aTester( &rDoc, aDestArea );
    if ( !aTester.IsEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if ( rDoc.HasSelectedBlockMatrixFragment( nStartCol, nStartRow,
            nEndCol, nEndRow, aMark ) )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_MATRIXFRAGMENTERR);
        return false;
    }

    // FID_FILL_... slots should already had been disabled, check here for API
    // calls, no message.
    if (ScViewData::SelectionFillDOOM( aDestArea))
        return false;

    WaitObject aWait( ScDocShell::GetActiveDialogParent() );

    ScDocumentUniquePtr pUndoDoc;
    if ( bRecord )
    {
        SCTAB nTabCount = rDoc.GetTableCount();
        SCTAB nDestStartTab = aDestArea.aStart.Tab();

        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( &rDoc, nDestStartTab, nDestStartTab );
        for (const auto& rTab : aMark)
        {
            if (rTab >= nTabCount)
                break;

            if (rTab != nDestStartTab)
                pUndoDoc->AddUndoTab( rTab, rTab );
        }

        // do not clone note captions in undo document
        rDoc.CopyToDocument(
            aDestArea.aStart.Col(), aDestArea.aStart.Row(), 0,
            aDestArea.aEnd.Col(), aDestArea.aEnd.Row(), nTabCount-1,
            InsertDeleteFlags::AUTOFILL, false, *pUndoDoc, &aMark );
    }

    sal_uLong nProgCount;
    if (eDir == FILL_TO_BOTTOM || eDir == FILL_TO_TOP)
        nProgCount = aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1;
    else
        nProgCount = aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1;
    nProgCount *= nCount;
    ScProgress aProgress( rDoc.GetDocumentShell(),
            ScResId(STR_FILL_SERIES_PROGRESS), nProgCount, true );

    rDoc.Fill( aSourceArea.aStart.Col(), aSourceArea.aStart.Row(),
            aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), &aProgress,
            aMark, nCount, eDir, eCmd, eDateCmd, fStep, fMax );

    AdjustRowHeight(aDestArea);

    if ( bRecord )      // only now is Draw-Undo available
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoAutoFill>( &rDocShell, aDestArea, aSourceArea, std::move(pUndoDoc), aMark,
                                eDir, eCmd, eDateCmd, MAXDOUBLE, fStep, fMax) );
    }

    rDocShell.PostPaintGridAll();
    aModificator.SetDocumentModified();

    rRange = aDestArea;         // return destination range (for marking)
    return true;
}

bool ScDocFunc::MergeCells( const ScCellMergeOption& rOption, bool bContents, bool bRecord, bool bApi, bool bEmptyMergedCells /*=false*/ )
{
    using ::std::set;

    ScDocShellModificator aModificator( rDocShell );

    SCCOL nStartCol = rOption.mnStartCol;
    SCROW nStartRow = rOption.mnStartRow;
    SCCOL nEndCol = rOption.mnEndCol;
    SCROW nEndRow = rOption.mnEndRow;
    if ((nStartCol == nEndCol && nStartRow == nEndRow) || rOption.maTabs.empty())
    {
        // Nothing to do.  Bail out quickly
        return true;
    }

    ScDocument& rDoc = rDocShell.GetDocument();
    SCTAB nTab1 = *rOption.maTabs.begin(), nTab2 = *rOption.maTabs.rbegin();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    for (const auto& rTab : rOption.maTabs)
    {
        ScEditableTester aTester( &rDoc, rTab, nStartCol, nStartRow, nEndCol, nEndRow );
        if (!aTester.IsEditable())
        {
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());
            return false;
        }

        if ( rDoc.HasAttrib( nStartCol, nStartRow, rTab, nEndCol, nEndRow, rTab,
                                HasAttrFlags::Merged | HasAttrFlags::Overlapped ) )
        {
            // "Merge of already merged cells not possible"
            if (!bApi)
                rDocShell.ErrorMessage(STR_MSSG_MERGECELLS_0);
            return false;
        }
    }

    ScDocumentUniquePtr pUndoDoc;
    bool bNeedContentsUndo = false;
    for (const SCTAB nTab : rOption.maTabs)
    {
        bool bIsBlockEmpty = ( nStartRow == nEndRow )
                             ? rDoc.IsBlockEmpty( nTab, nStartCol+1,nStartRow, nEndCol,nEndRow, true )
                             : rDoc.IsBlockEmpty( nTab, nStartCol,nStartRow+1, nStartCol,nEndRow, true ) &&
                               rDoc.IsBlockEmpty( nTab, nStartCol+1,nStartRow, nEndCol,nEndRow, true );
        bool bNeedContents = bContents && !bIsBlockEmpty;
        bool bNeedEmpty = bEmptyMergedCells && !bIsBlockEmpty && !bNeedContents; // if DoMergeContents then cells are emptyed

        if (bRecord)
        {
            // test if the range contains other notes which also implies that we need an undo document
            bool bHasNotes = false;
            for( ScAddress aPos( nStartCol, nStartRow, nTab ); !bHasNotes && (aPos.Col() <= nEndCol); aPos.IncCol() )
                for( aPos.SetRow( nStartRow ); !bHasNotes && (aPos.Row() <= nEndRow); aPos.IncRow() )
                    bHasNotes = ((aPos.Col() != nStartCol) || (aPos.Row() != nStartRow)) && (rDoc.HasNote(aPos));

            if (!pUndoDoc)
            {
                pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
                pUndoDoc->InitUndo(&rDoc, nTab1, nTab2);
            }
            // note captions are collected by drawing undo
            rDoc.CopyToDocument( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                  InsertDeleteFlags::ALL|InsertDeleteFlags::NOCAPTIONS, false, *pUndoDoc );
            if( bHasNotes )
                rDoc.BeginDrawUndo();
        }

        if (bNeedContents)
            rDoc.DoMergeContents( nTab, nStartCol,nStartRow, nEndCol,nEndRow );
        else if ( bNeedEmpty )
            rDoc.DoEmptyBlock( nTab, nStartCol,nStartRow, nEndCol,nEndRow );
        rDoc.DoMerge( nTab, nStartCol,nStartRow, nEndCol,nEndRow );

        if (rOption.mbCenter)
        {
            rDoc.ApplyAttr( nStartCol, nStartRow, nTab, SvxHorJustifyItem( SvxCellHorJustify::Center, ATTR_HOR_JUSTIFY ) );
            rDoc.ApplyAttr( nStartCol, nStartRow, nTab, SvxVerJustifyItem( SvxCellVerJustify::Center, ATTR_VER_JUSTIFY ) );
        }

        if ( !AdjustRowHeight( ScRange( 0,nStartRow,nTab, MAXCOL,nEndRow,nTab ) ) )
            rDocShell.PostPaint( nStartCol, nStartRow, nTab,
                                 nEndCol, nEndRow, nTab, PaintPartFlags::Grid );
        if (bNeedContents || rOption.mbCenter)
        {
            ScRange aRange(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab);
            rDoc.SetDirty(aRange, true);
        }

        bNeedContentsUndo |= bNeedContents;
    }

    if (pUndoDoc)
    {
        std::unique_ptr<SdrUndoGroup> pDrawUndo = rDoc.GetDrawLayer() ? rDoc.GetDrawLayer()->GetCalcUndo() : nullptr;
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoMerge>(&rDocShell, rOption, bNeedContentsUndo, std::move(pUndoDoc), std::move(pDrawUndo)) );
    }

    aModificator.SetDocumentModified();

    SfxBindings* pBindings = rDocShell.GetViewBindings();
    if (pBindings)
    {
        pBindings->Invalidate( FID_MERGE_ON );
        pBindings->Invalidate( FID_MERGE_OFF );
        pBindings->Invalidate( FID_MERGE_TOGGLE );
    }

    return true;
}

bool ScDocFunc::UnmergeCells( const ScRange& rRange, bool bRecord, ScUndoRemoveMerge* pUndoRemoveMerge )
{
    ScCellMergeOption aOption(rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row());
    SCTAB nTab1 = rRange.aStart.Tab(), nTab2 = rRange.aEnd.Tab();
    for (SCTAB i = nTab1; i <= nTab2; ++i)
        aOption.maTabs.insert(i);

    return UnmergeCells(aOption, bRecord, pUndoRemoveMerge);
}

bool ScDocFunc::UnmergeCells( const ScCellMergeOption& rOption, bool bRecord, ScUndoRemoveMerge* pUndoRemoveMerge )
{
    using ::std::set;

    if (rOption.maTabs.empty())
        // Nothing to unmerge.
        return true;

    ScDocShellModificator aModificator( rDocShell );
    ScDocument& rDoc = rDocShell.GetDocument();

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScDocument* pUndoDoc = (pUndoRemoveMerge ? pUndoRemoveMerge->GetUndoDoc() : nullptr);
    assert( pUndoDoc || !pUndoRemoveMerge );
    for (const SCTAB nTab : rOption.maTabs)
    {
        ScRange aRange = rOption.getSingleRange(nTab);
        if ( !rDoc.HasAttrib(aRange, HasAttrFlags::Merged) )
            continue;

        ScRange aExtended = aRange;
        rDoc.ExtendMerge(aExtended);
        ScRange aRefresh = aExtended;
        rDoc.ExtendOverlapped(aRefresh);

        if (bRecord)
        {
            if (!pUndoDoc)
            {
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pUndoDoc->InitUndo(&rDoc, *rOption.maTabs.begin(), *rOption.maTabs.rbegin());
            }
            rDoc.CopyToDocument(aExtended, InsertDeleteFlags::ATTRIB, false, *pUndoDoc);
        }

        const SfxPoolItem& rDefAttr = rDoc.GetPool()->GetDefaultItem( ATTR_MERGE );
        ScPatternAttr aPattern( rDoc.GetPool() );
        aPattern.GetItemSet().Put( rDefAttr );
        rDoc.ApplyPatternAreaTab( aRange.aStart.Col(), aRange.aStart.Row(),
                                   aRange.aEnd.Col(), aRange.aEnd.Row(), nTab,
                                   aPattern );

        rDoc.RemoveFlagsTab( aExtended.aStart.Col(), aExtended.aStart.Row(),
                              aExtended.aEnd.Col(), aExtended.aEnd.Row(), nTab,
                              ScMF::Hor | ScMF::Ver );

        rDoc.ExtendMerge( aRefresh, true );

        if ( !AdjustRowHeight( aExtended ) )
            rDocShell.PostPaint( aExtended, PaintPartFlags::Grid );
    }

    if (bRecord)
    {
        if (pUndoRemoveMerge)
        {
            // If pUndoRemoveMerge was passed, the caller is responsible for
            // adding it to Undo. Just add the current option.
            pUndoRemoveMerge->AddCellMergeOption( rOption);
        }
        else
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoRemoveMerge>( &rDocShell, rOption, ScDocumentUniquePtr(pUndoDoc) ) );
        }
    }
    aModificator.SetDocumentModified();

    return true;
}

void ScDocFunc::ModifyRangeNames( const ScRangeName& rNewRanges, SCTAB nTab )
{
    SetNewRangeNames( std::unique_ptr<ScRangeName>(new ScRangeName(rNewRanges)), true, nTab );
}

void ScDocFunc::SetNewRangeNames( std::unique_ptr<ScRangeName> pNewRanges, bool bModifyDoc, SCTAB nTab )     // takes ownership of pNewRanges
{
    ScDocShellModificator aModificator( rDocShell );

    OSL_ENSURE( pNewRanges, "pNewRanges is 0" );
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bUndo(rDoc.IsUndoEnabled());

    if (bUndo)
    {
        ScRangeName* pOld;
        if (nTab >=0)
        {
            pOld = rDoc.GetRangeName(nTab);
        }
        else
        {
            pOld = rDoc.GetRangeName();
        }
        std::unique_ptr<ScRangeName> pUndoRanges(new ScRangeName(*pOld));
        std::unique_ptr<ScRangeName> pRedoRanges(new ScRangeName(*pNewRanges));
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoRangeNames>( &rDocShell, std::move(pUndoRanges), std::move(pRedoRanges), nTab ) );
    }

    // #i55926# While loading XML, formula cells only have a single string token,
    // so CompileNameFormula would never find any name (index) tokens, and would
    // unnecessarily loop through all cells.
    bool bCompile = ( !rDoc.IsImportingXML() && rDoc.GetNamedRangesLockCount() == 0 );

    if ( bCompile )
        rDoc.PreprocessRangeNameUpdate();
    if (nTab >= 0)
        rDoc.SetRangeName( nTab, std::move(pNewRanges) ); // takes ownership
    else
        rDoc.SetRangeName( std::move(pNewRanges) );       // takes ownership
    if ( bCompile )
        rDoc.CompileHybridFormula();

    if (bModifyDoc)
    {
        aModificator.SetDocumentModified();
        SfxGetpApp()->Broadcast( SfxHint(SfxHintId::ScAreasChanged) );
    }
}

void ScDocFunc::ModifyAllRangeNames(const std::map<OUString, std::unique_ptr<ScRangeName>>& rRangeMap)
{
    ScDocShellModificator aModificator(rDocShell);
    ScDocument& rDoc = rDocShell.GetDocument();

    if (rDoc.IsUndoEnabled())
    {
        std::map<OUString, ScRangeName*> aOldRangeMap;
        rDoc.GetRangeNameMap(aOldRangeMap);
        rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoAllRangeNames>(&rDocShell, aOldRangeMap, rRangeMap));
    }

    rDoc.PreprocessAllRangeNamesUpdate(rRangeMap);
    rDoc.SetAllRangeNames(rRangeMap);
    rDoc.CompileHybridFormula();

    aModificator.SetDocumentModified();
    SfxGetpApp()->Broadcast(SfxHint(SfxHintId::ScAreasChanged));
}

void ScDocFunc::CreateOneName( ScRangeName& rList,
                                SCCOL nPosX, SCROW nPosY, SCTAB nTab,
                                SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                bool& rCancel, bool bApi )
{
    if (rCancel)
        return;

    ScDocument& rDoc = rDocShell.GetDocument();
    if (!rDoc.HasValueData( nPosX, nPosY, nTab ))
    {
        OUString aName = rDoc.GetString(nPosX, nPosY, nTab);
        ScRangeData::MakeValidName(aName);
        if (!aName.isEmpty())
        {
            OUString aContent(ScRange( nX1, nY1, nTab, nX2, nY2, nTab ).Format(ScRefFlags::RANGE_ABS_3D, &rDoc));

            bool bInsert = false;
            ScRangeData* pOld = rList.findByUpperName(ScGlobal::pCharClass->uppercase(aName));
            if (pOld)
            {
                OUString aOldStr;
                pOld->GetSymbol( aOldStr );
                if (aOldStr != aContent)
                {
                    if (bApi)
                        bInsert = true;     // don't check via API
                    else
                    {
                        OUString aTemplate = ScResId( STR_CREATENAME_REPLACE );
                        OUString aMessage = aTemplate.getToken( 0, '#' ) + aName + aTemplate.getToken( 1, '#' );

                        vcl::Window* pWin = ScDocShell::GetActiveDialogParent();
                        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                                       aMessage));
                        xQueryBox->add_button(Button::GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
                        xQueryBox->set_default_response(RET_YES);

                        short nResult = xQueryBox->run();
                        if ( nResult == RET_YES )
                        {
                            rList.erase(*pOld);
                            bInsert = true;
                        }
                        else if ( nResult == RET_CANCEL )
                            rCancel = true;
                    }
                }
            }
            else
                bInsert = true;

            if (bInsert)
            {
                ScRangeData* pData = new ScRangeData( &rDoc, aName, aContent,
                        ScAddress( nPosX, nPosY, nTab));
                if (!rList.insert(pData))
                {
                    OSL_FAIL("nanu?");
                }
            }
        }
    }
}

bool ScDocFunc::CreateNames( const ScRange& rRange, CreateNameFlags nFlags, bool bApi, SCTAB aTab )
{
    if (nFlags == CreateNameFlags::NONE)
        return false;       // was nothing

    ScDocShellModificator aModificator( rDocShell );

    bool bDone = false;
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();
    OSL_ENSURE(rRange.aEnd.Tab() == nTab, "CreateNames: multiple tables not possible");

    bool bValid = true;
    if ( nFlags & ( CreateNameFlags::Top | CreateNameFlags::Bottom ) )
        if ( nStartRow == nEndRow )
            bValid = false;
    if ( nFlags & ( CreateNameFlags::Left | CreateNameFlags::Right ) )
        if ( nStartCol == nEndCol )
            bValid = false;

    if (bValid)
    {
        ScDocument& rDoc = rDocShell.GetDocument();
        ScRangeName* pNames;
        if (aTab >=0)
            pNames = rDoc.GetRangeName(nTab);
        else
            pNames = rDoc.GetRangeName();

        if (!pNames)
            return false;   // shouldn't happen
        ScRangeName aNewRanges( *pNames );

        bool bTop   ( nFlags & CreateNameFlags::Top );
        bool bLeft  ( nFlags & CreateNameFlags::Left );
        bool bBottom( nFlags & CreateNameFlags::Bottom );
        bool bRight ( nFlags & CreateNameFlags::Right );

        SCCOL nContX1 = nStartCol;
        SCROW nContY1 = nStartRow;
        SCCOL nContX2 = nEndCol;
        SCROW nContY2 = nEndRow;

        if ( bTop )
            ++nContY1;
        if ( bLeft )
            ++nContX1;
        if ( bBottom )
            --nContY2;
        if ( bRight )
            --nContX2;

        bool bCancel = false;
        SCCOL i;
        SCROW j;

        if ( bTop )
            for (i=nContX1; i<=nContX2; i++)
                CreateOneName( aNewRanges, i,nStartRow,nTab, i,nContY1,i,nContY2, bCancel, bApi );
        if ( bLeft )
            for (j=nContY1; j<=nContY2; j++)
                CreateOneName( aNewRanges, nStartCol,j,nTab, nContX1,j,nContX2,j, bCancel, bApi );
        if ( bBottom )
            for (i=nContX1; i<=nContX2; i++)
                CreateOneName( aNewRanges, i,nEndRow,nTab, i,nContY1,i,nContY2, bCancel, bApi );
        if ( bRight )
            for (j=nContY1; j<=nContY2; j++)
                CreateOneName( aNewRanges, nEndCol,j,nTab, nContX1,j,nContX2,j, bCancel, bApi );

        if ( bTop && bLeft )
            CreateOneName( aNewRanges, nStartCol,nStartRow,nTab, nContX1,nContY1,nContX2,nContY2, bCancel, bApi );
        if ( bTop && bRight )
            CreateOneName( aNewRanges, nEndCol,nStartRow,nTab, nContX1,nContY1,nContX2,nContY2, bCancel, bApi );
        if ( bBottom && bLeft )
            CreateOneName( aNewRanges, nStartCol,nEndRow,nTab, nContX1,nContY1,nContX2,nContY2, bCancel, bApi );
        if ( bBottom && bRight )
            CreateOneName( aNewRanges, nEndCol,nEndRow,nTab, nContX1,nContY1,nContX2,nContY2, bCancel, bApi );

        ModifyRangeNames( aNewRanges, aTab );
        bDone = true;

    }

    return bDone;
}

bool ScDocFunc::InsertNameList( const ScAddress& rStartPos, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    bool bDone = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    const bool bRecord = rDoc.IsUndoEnabled();
    SCTAB nTab = rStartPos.Tab();

    //local names have higher priority than global names
    ScRangeName* pLocalList = rDoc.GetRangeName(nTab);
    sal_uInt16 nValidCount = 0;
    for (const auto& rEntry : *pLocalList)
    {
        const ScRangeData& r = *rEntry.second;
        if (!r.HasType(ScRangeData::Type::Database))
            ++nValidCount;
    }
    ScRangeName* pList = rDoc.GetRangeName();
    for (const auto& rEntry : *pList)
    {
        const ScRangeData& r = *rEntry.second;
        if (!r.HasType(ScRangeData::Type::Database) && !pLocalList->findByUpperName(r.GetUpperName()))
            ++nValidCount;
    }

    if (nValidCount)
    {
        SCCOL nStartCol = rStartPos.Col();
        SCROW nStartRow = rStartPos.Row();
        SCCOL nEndCol = nStartCol + 1;
        SCROW nEndRow = nStartRow + static_cast<SCROW>(nValidCount) - 1;

        ScEditableTester aTester( &rDoc, nTab, nStartCol,nStartRow, nEndCol,nEndRow );
        if (aTester.IsEditable())
        {
            ScDocumentUniquePtr pUndoDoc;

            if (bRecord)
            {
                pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
                pUndoDoc->InitUndo( &rDoc, nTab, nTab );
                rDoc.CopyToDocument(nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab,
                                    InsertDeleteFlags::ALL, false, *pUndoDoc);

                rDoc.BeginDrawUndo();      // because of adjusting heights
            }

            std::unique_ptr<ScRangeData*[]> ppSortArray(new ScRangeData* [ nValidCount ]);
            sal_uInt16 j = 0;
            for (const auto& rEntry : *pLocalList)
            {
                ScRangeData& r = *rEntry.second;
                if (!r.HasType(ScRangeData::Type::Database))
                    ppSortArray[j++] = &r;
            }
            for (const auto& [rName, rxData] : *pList)
            {
                ScRangeData& r = *rxData;
                if (!r.HasType(ScRangeData::Type::Database) && !pLocalList->findByUpperName(rName))
                    ppSortArray[j++] = &r;
            }
            qsort( static_cast<void*>(ppSortArray.get()), nValidCount, sizeof(ScRangeData*),
                &ScRangeData_QsortNameCompare );
            OUString aName;
            OUStringBuffer aContent;
            OUString aFormula;
            SCROW nOutRow = nStartRow;
            for (j=0; j<nValidCount; j++)
            {
                ScRangeData* pData = ppSortArray[j];
                pData->GetName(aName);
                // adjust relative references to the left column in Excel-compliant way:
                pData->UpdateSymbol(aContent, ScAddress( nStartCol, nOutRow, nTab ));
                aFormula = "=" + aContent.toString();
                ScSetStringParam aParam;
                aParam.setTextInput();
                rDoc.SetString(ScAddress(nStartCol,nOutRow,nTab), aName, &aParam);
                rDoc.SetString(ScAddress(nEndCol,nOutRow,nTab), aFormula, &aParam);
                ++nOutRow;
            }

            ppSortArray.reset();

            if (bRecord)
            {
                ScDocumentUniquePtr pRedoDoc(new ScDocument( SCDOCMODE_UNDO ));
                pRedoDoc->InitUndo( &rDoc, nTab, nTab );
                rDoc.CopyToDocument(nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab,
                                    InsertDeleteFlags::ALL, false, *pRedoDoc);

                rDocShell.GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoListNames>( &rDocShell,
                                ScRange( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab ),
                                std::move(pUndoDoc), std::move(pRedoDoc) ) );
            }

            if (!AdjustRowHeight(ScRange(0,nStartRow,nTab,MAXCOL,nEndRow,nTab)))
                rDocShell.PostPaint( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab, PaintPartFlags::Grid );

            aModificator.SetDocumentModified();
            bDone = true;
        }
        else if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
    }
    return bDone;
}

void ScDocFunc::ResizeMatrix( const ScRange& rOldRange, const ScAddress& rNewEnd )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rOldRange.aStart.Col();
    SCROW nStartRow = rOldRange.aStart.Row();
    SCTAB nTab = rOldRange.aStart.Tab();

    OUString aFormula;
    rDoc.GetFormula( nStartCol, nStartRow, nTab, aFormula );
    if ( aFormula.startsWith("{") && aFormula.endsWith("}") )
    {
        OUString aUndo = ScResId( STR_UNDO_RESIZEMATRIX );
        bool bUndo(rDoc.IsUndoEnabled());
        if (bUndo)
        {
            ViewShellId nViewShellId(1);
            if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
                nViewShellId = pViewSh->GetViewShellId();
            rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo, 0, nViewShellId );
        }

        aFormula = aFormula.copy(1, aFormula.getLength()-2);

        ScMarkData aMark;
        aMark.SetMarkArea( rOldRange );
        aMark.SelectTable( nTab, true );
        ScRange aNewRange( rOldRange.aStart, rNewEnd );

        if ( DeleteContents( aMark, InsertDeleteFlags::CONTENTS, true, false/*bApi*/ ) )
        {
            // GRAM_API for API compatibility.
            if (!EnterMatrix( aNewRange, &aMark, nullptr, aFormula, false/*bApi*/, false, EMPTY_OUSTRING, formula::FormulaGrammar::GRAM_API ))
            {
                // try to restore the previous state
                EnterMatrix( rOldRange, &aMark, nullptr, aFormula, false/*bApi*/, false, EMPTY_OUSTRING, formula::FormulaGrammar::GRAM_API );
            }
        }

        if (bUndo)
            rDocShell.GetUndoManager()->LeaveListAction();
    }
}

void ScDocFunc::InsertAreaLink( const OUString& rFile, const OUString& rFilter,
                                const OUString& rOptions, const OUString& rSource,
                                const ScRange& rDestRange, sal_uLong nRefresh,
                                bool bFitBlock, bool bApi )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bUndo (rDoc.IsUndoEnabled());

    sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();

    //  #i52120# if other area links exist at the same start position,
    //  remove them first (file format specifies only one link definition
    //  for a cell)

    sal_uInt16 nLinkCount = pLinkManager->GetLinks().size();
    sal_uInt16 nRemoved = 0;
    sal_uInt16 nLinkPos = 0;
    while (nLinkPos<nLinkCount)
    {
        ::sfx2::SvBaseLink* pBase = pLinkManager->GetLinks()[nLinkPos].get();
        ScAreaLink* pLink = dynamic_cast<ScAreaLink*>(pBase);
        if (pLink && pLink->GetDestArea().aStart == rDestRange.aStart)
        {
            if ( bUndo )
            {
                if ( !nRemoved )
                {
                    // group all remove and the insert action
                    OUString aUndo = ScResId( STR_UNDO_INSERTAREALINK );
                    ViewShellId nViewShellId(-1);
                    if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
                        nViewShellId = pViewSh->GetViewShellId();
                    rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo, 0, nViewShellId );
                }

                ScAreaLink* pOldArea = static_cast<ScAreaLink*>(pBase);
                rDocShell.GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoRemoveAreaLink>( &rDocShell,
                        pOldArea->GetFile(), pOldArea->GetFilter(), pOldArea->GetOptions(),
                        pOldArea->GetSource(), pOldArea->GetDestArea(), pOldArea->GetRefreshDelay() ) );
            }
            pLinkManager->Remove( pBase );
            nLinkCount = pLinkManager->GetLinks().size();
            ++nRemoved;
        }
        else
            ++nLinkPos;
    }

    OUString aFilterName = rFilter;
    OUString aNewOptions = rOptions;
    if (aFilterName.isEmpty())
        ScDocumentLoader::GetFilterName( rFile, aFilterName, aNewOptions, true, !bApi );

    //  remove application prefix from filter name here, so the filter options
    //  aren't reset when the filter name is changed in ScAreaLink::DataChanged
    ScDocumentLoader::RemoveAppPrefix( aFilterName );

    ScAreaLink* pLink = new ScAreaLink( &rDocShell, rFile, aFilterName,
                                        aNewOptions, rSource, rDestRange, nRefresh );
    OUString aTmp = aFilterName;
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, rFile, &aTmp, &rSource );

    //  Undo for an empty link

    if (bUndo)
    {
        rDocShell.GetUndoManager()->AddUndoAction( std::make_unique<ScUndoInsertAreaLink>( &rDocShell,
                                                    rFile, aFilterName, aNewOptions,
                                                    rSource, rDestRange, nRefresh ) );
        if ( nRemoved )
            rDocShell.GetUndoManager()->LeaveListAction();  // undo for link update is still separate
    }

    //  Update has its own undo
    if (rDoc.IsExecuteLinkEnabled())
    {
        pLink->SetDoInsert(bFitBlock);  // if applicable, don't insert anything on first update
        pLink->Update();                // no SetInCreate -> carry out update
    }
    pLink->SetDoInsert(true);       // Default = true

    SfxBindings* pBindings = rDocShell.GetViewBindings();
    if (pBindings)
        pBindings->Invalidate( SID_LINKS );

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );     // Navigator
}

void ScDocFunc::ReplaceConditionalFormat( sal_uLong nOldFormat, std::unique_ptr<ScConditionalFormat> pFormat, SCTAB nTab, const ScRangeList& rRanges )
{
    ScDocShellModificator aModificator(rDocShell);
    ScDocument& rDoc = rDocShell.GetDocument();
    if(rDoc.IsTabProtected(nTab))
        return;

    bool bUndo = rDoc.IsUndoEnabled();
    ScDocumentUniquePtr pUndoDoc;
    ScRange aCombinedRange = rRanges.Combine();
    ScRange aCompleteRange;
    if(bUndo)
    {
        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo( &rDoc, nTab, nTab );

        if(pFormat)
        {
            aCompleteRange = aCombinedRange;
        }
        if(nOldFormat)
        {
            ScConditionalFormat* pOldFormat = rDoc.GetCondFormList(nTab)->GetFormat(nOldFormat);
            if(pOldFormat)
                aCompleteRange.ExtendTo(pOldFormat->GetRange().Combine());
        }

        rDoc.CopyToDocument(aCompleteRange.aStart.Col(),aCompleteRange.aStart.Row(),nTab,
                            aCompleteRange.aEnd.Col(),aCompleteRange.aEnd.Row(),nTab,
                            InsertDeleteFlags::ALL, false, *pUndoDoc);
    }

    std::unique_ptr<ScRange> pRepaintRange;
    if(nOldFormat)
    {
        ScConditionalFormat* pOldFormat = rDoc.GetCondFormList(nTab)->GetFormat(nOldFormat);
        if(pOldFormat)
        {
            pRepaintRange.reset(new ScRange( pOldFormat->GetRange().Combine() ));
            rDoc.RemoveCondFormatData(pOldFormat->GetRange(), nTab, pOldFormat->GetKey());
        }

        rDoc.DeleteConditionalFormat(nOldFormat, nTab);
        rDoc.SetStreamValid(nTab, false);
    }
    if(pFormat)
    {
        if(pRepaintRange)
            pRepaintRange->ExtendTo(aCombinedRange);
        else
            pRepaintRange.reset(new ScRange(aCombinedRange));

        sal_uLong nIndex = rDoc.AddCondFormat(std::move(pFormat), nTab);

        rDoc.AddCondFormatData(rRanges, nTab, nIndex);
        rDoc.SetStreamValid(nTab, false);
    }

    if(bUndo)
    {
        ScDocumentUniquePtr pRedoDoc(new ScDocument(SCDOCMODE_UNDO));
        pRedoDoc->InitUndo( &rDoc, nTab, nTab );
        rDoc.CopyToDocument(aCompleteRange.aStart.Col(),aCompleteRange.aStart.Row(),nTab,
                            aCompleteRange.aEnd.Col(),aCompleteRange.aEnd.Row(),nTab,
                            InsertDeleteFlags::ALL, false, *pRedoDoc);
        rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoConditionalFormat>(&rDocShell, std::move(pUndoDoc), std::move(pRedoDoc), aCompleteRange));
    }

    if(pRepaintRange)
        rDocShell.PostPaint(*pRepaintRange, PaintPartFlags::Grid);

    aModificator.SetDocumentModified();
    SfxGetpApp()->Broadcast(SfxHint(SfxHintId::ScAreasChanged));
}

void ScDocFunc::SetConditionalFormatList( ScConditionalFormatList* pList, SCTAB nTab )
{
    ScDocShellModificator aModificator(rDocShell);
    ScDocument& rDoc = rDocShell.GetDocument();
    if(rDoc.IsTabProtected(nTab))
        return;

    bool bUndo = rDoc.IsUndoEnabled();
    ScDocumentUniquePtr pUndoDoc;
    if (bUndo)
    {
        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo( &rDoc, nTab, nTab );

        ScConditionalFormatList* pOld = rDoc.GetCondFormList(nTab);

        if (pOld)
            pUndoDoc->SetCondFormList(new ScConditionalFormatList(pUndoDoc.get(), *pOld), nTab);
        else
            pUndoDoc->SetCondFormList(nullptr, nTab);

    }

    // first remove all old entries
    ScConditionalFormatList* pOldList = rDoc.GetCondFormList(nTab);
    pOldList->RemoveFromDocument(&rDoc);

    // then set new entries
    pList->AddToDocument(&rDoc);

    rDoc.SetCondFormList(pList, nTab);
    rDocShell.PostPaintGridAll();

    if(bUndo)
    {
        ScDocumentUniquePtr pRedoDoc(new ScDocument(SCDOCMODE_UNDO));
        pRedoDoc->InitUndo( &rDoc, nTab, nTab );
        pRedoDoc->SetCondFormList(new ScConditionalFormatList(pRedoDoc.get(), *pList), nTab);

        rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoConditionalFormatList>(&rDocShell, std::move(pUndoDoc), std::move(pRedoDoc), nTab));
    }

    rDoc.SetStreamValid(nTab, false);
    aModificator.SetDocumentModified();
    SfxGetpApp()->Broadcast(SfxHint(SfxHintId::ScAreasChanged));
}

void ScDocFunc::ConvertFormulaToValue( const ScRange& rRange, bool bInteraction )
{
    ScDocShellModificator aModificator(rDocShell);
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    ScEditableTester aTester(&rDoc, rRange);
    if (!aTester.IsEditable())
    {
        if (bInteraction)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return;
    }

    sc::TableValues aUndoVals(rRange);
    sc::TableValues* pUndoVals = bRecord ? &aUndoVals : nullptr;

    rDoc.ConvertFormulaToValue(rRange, pUndoVals);

    if (bRecord && pUndoVals)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<sc::UndoFormulaToValue>(&rDocShell, *pUndoVals));
    }

    rDocShell.PostPaint(rRange, PaintPartFlags::Grid);
    rDocShell.PostDataChanged();
    rDoc.BroadcastCells(rRange, SfxHintId::ScDataChanged);
    aModificator.SetDocumentModified();
}

void ScDocFunc::EnterListAction(const char* pNameResId)
{
    OUString aUndo(ScResId(pNameResId));
    ViewShellId nViewShellId(-1);
    if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
        nViewShellId = pViewSh->GetViewShellId();
    rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo, 0, nViewShellId );
}

void ScDocFunc::EndListAction()
{
    rDocShell.GetUndoManager()->LeaveListAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
