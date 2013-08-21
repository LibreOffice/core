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

#include "scitems.hxx"

#include <sfx2/app.hxx>
#include <editeng/editobj.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>
#include <svl/PasswordHelper.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>

#include "docfunc.hxx"

#include "sc.hrc"

#include "arealink.hxx"
#include "attrib.hxx"
#include "dociter.hxx"
#include "autoform.hxx"
#include "formulacell.hxx"
#include "cellmergeoption.hxx"
#include "detdata.hxx"
#include "detfunc.hxx"
#include "docpool.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "editutil.hxx"
#include "globstr.hrc"
#include "globalnames.hxx"
#include "olinetab.hxx"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "refundo.hxx"
#include "scresid.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "tablink.hxx"
#include "tabvwsh.hxx"
#include "uiitems.hxx"
#include "undoblk.hxx"
#include "undocell.hxx"
#include "undodraw.hxx"
#include "undotab.hxx"
#include "waitoff.hxx"
#include "sizedev.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "inputwin.hxx"
#include "editable.hxx"
#include "compiler.hxx"
#include "scui_def.hxx"
#include "tabprotection.hxx"
#include "clipparam.hxx"
#include "externalrefmgr.hxx"
#include "undorangename.hxx"
#include "progress.hxx"
#include "dpobject.hxx"
#include "stringutil.hxx"
#include "cellvalue.hxx"
#include "tokenarray.hxx"

#include <memory>
#include <basic/basmgr.hxx>
#include <boost/scoped_ptr.hpp>
#include <set>
#include <vector>

using namespace com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::std::vector;


// STATIC DATA -----------------------------------------------------------

//========================================================================

IMPL_LINK( ScDocFunc, NotifyDrawUndo, SdrUndoAction*, pUndoAction )
{
    // #i101118# if drawing layer collects the undo actions, add it there
    ScDrawLayer* pDrawLayer = rDocShell.GetDocument()->GetDrawLayer();
    if( pDrawLayer && pDrawLayer->IsRecording() )
        pDrawLayer->AddCalcUndo( pUndoAction );
    else
        rDocShell.GetUndoManager()->AddUndoAction( new ScUndoDraw( pUndoAction, &rDocShell ) );
    rDocShell.SetDrawModified();

    // the affected sheet isn't known, so all stream positions are invalidated
    ScDocument* pDoc = rDocShell.GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        if (pDoc->IsStreamValid(nTab))
            pDoc->SetStreamValid(nTab, false);

    return 0;
}

//------------------------------------------------------------------------

//  Zeile ueber dem Range painten (fuer Linien nach AdjustRowHeight)

static void lcl_PaintAbove( ScDocShell& rDocShell, const ScRange& rRange )
{
    SCROW nRow = rRange.aStart.Row();
    if ( nRow > 0 )
    {
        SCTAB nTab = rRange.aStart.Tab();   //! alle?
        --nRow;
        rDocShell.PostPaint( ScRange(0,nRow,nTab, MAXCOL,nRow,nTab), PAINT_GRID );
    }
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::AdjustRowHeight( const ScRange& rRange, sal_Bool bPaint )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if ( pDoc->IsImportingXML() )
    {
        //  for XML import, all row heights are updated together after importing
        return false;
    }
    if ( !pDoc->IsAdjustHeightEnabled() )
    {
        return false;
    }

    SCTAB nTab      = rRange.aStart.Tab();
    SCROW nStartRow = rRange.aStart.Row();
    SCROW nEndRow   = rRange.aEnd.Row();

    ScSizeDeviceProvider aProv( &rDocShell );
    Fraction aOne(1,1);

    sal_Bool bChanged = pDoc->SetOptimalHeight( nStartRow, nEndRow, nTab, 0, aProv.GetDevice(),
                                            aProv.GetPPTX(), aProv.GetPPTY(), aOne, aOne, false );

    if ( bPaint && bChanged )
        rDocShell.PostPaint(ScRange(0, nStartRow, nTab, MAXCOL, MAXROW, nTab),
                            PAINT_GRID | PAINT_LEFT);

    return bChanged;
}


//------------------------------------------------------------------------

sal_Bool ScDocFunc::DetectiveAddPred(const ScAddress& rPos)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument* pDoc = rDocShell.GetDocument();
    bool bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo(false);
    sal_Bool bDone = ScDetectiveFunc( pDoc,nTab ).ShowPred( nCol, nRow );
    SdrUndoGroup* pUndo = NULL;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_ADDPRED );
        pDoc->AddDetectiveOperation( aOperation );
        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDetective( &rDocShell, pUndo, &aOperation ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }
    else
        delete pUndo;

    return bDone;
}

sal_Bool ScDocFunc::DetectiveDelPred(const ScAddress& rPos)
{
    ScDocument* pDoc = rDocShell.GetDocument();

    bool bUndo(pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    ScDocShellModificator aModificator( rDocShell );

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo(false);
    sal_Bool bDone = ScDetectiveFunc( pDoc,nTab ).DeletePred( nCol, nRow );
    SdrUndoGroup* pUndo = NULL;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_DELPRED );
        pDoc->AddDetectiveOperation( aOperation );
        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDetective( &rDocShell, pUndo, &aOperation ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }
    else
        delete pUndo;

    return bDone;
}

sal_Bool ScDocFunc::DetectiveAddSucc(const ScAddress& rPos)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument* pDoc = rDocShell.GetDocument();

    bool bUndo(pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo(false);
    sal_Bool bDone = ScDetectiveFunc( pDoc,nTab ).ShowSucc( nCol, nRow );
    SdrUndoGroup* pUndo = NULL;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_ADDSUCC );
        pDoc->AddDetectiveOperation( aOperation );
        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDetective( &rDocShell, pUndo, &aOperation ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }
    else
        delete pUndo;

    return bDone;
}

sal_Bool ScDocFunc::DetectiveDelSucc(const ScAddress& rPos)
{
    ScDocument* pDoc = rDocShell.GetDocument();

    bool bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    ScDocShellModificator aModificator( rDocShell );

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo(false);
    sal_Bool bDone = ScDetectiveFunc( pDoc,nTab ).DeleteSucc( nCol, nRow );
    SdrUndoGroup* pUndo = NULL;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_DELSUCC );
        pDoc->AddDetectiveOperation( aOperation );
        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDetective( &rDocShell, pUndo, &aOperation ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }
    else
        delete pUndo;

    return bDone;
}

sal_Bool ScDocFunc::DetectiveAddError(const ScAddress& rPos)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument* pDoc = rDocShell.GetDocument();

    bool bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo(false);
    sal_Bool bDone = ScDetectiveFunc( pDoc,nTab ).ShowError( nCol, nRow );
    SdrUndoGroup* pUndo = NULL;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpData aOperation( ScAddress(nCol,nRow,nTab), SCDETOP_ADDERROR );
        pDoc->AddDetectiveOperation( aOperation );
        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDetective( &rDocShell, pUndo, &aOperation ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }
    else
        delete pUndo;

    return bDone;
}

sal_Bool ScDocFunc::DetectiveMarkInvalid(SCTAB nTab)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument* pDoc = rDocShell.GetDocument();

    bool bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();

    Window* pWaitWin = rDocShell.GetActiveDialogParent();
    if (pWaitWin)
        pWaitWin->EnterWait();
    if (bUndo)
        pModel->BeginCalcUndo(false);
    sal_Bool bOverflow;
    sal_Bool bDone = ScDetectiveFunc( pDoc,nTab ).MarkInvalid( bOverflow );
    SdrUndoGroup* pUndo = NULL;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (pWaitWin)
        pWaitWin->LeaveWait();
    if (bDone)
    {
        if (pUndo && bUndo)
        {
            pUndo->SetComment( ScGlobal::GetRscString( STR_UNDO_DETINVALID ) );
            rDocShell.GetUndoManager()->AddUndoAction( pUndo );
        }
        aModificator.SetDocumentModified();
        if ( bOverflow )
        {
            InfoBox( NULL,
                    ScGlobal::GetRscString( STR_DETINVALID_OVERFLOW ) ).Execute();
        }
    }
    else
        delete pUndo;

    return bDone;
}

sal_Bool ScDocFunc::DetectiveDelAll(SCTAB nTab)
{
    ScDocument* pDoc = rDocShell.GetDocument();

    bool bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    ScDocShellModificator aModificator( rDocShell );

    if (bUndo)
        pModel->BeginCalcUndo(false);
    sal_Bool bDone = ScDetectiveFunc( pDoc,nTab ).DeleteAll( SC_DET_DETECTIVE );
    SdrUndoGroup* pUndo = NULL;
    if (bUndo)
        pUndo = pModel->GetCalcUndo();
    if (bDone)
    {
        ScDetOpList* pOldList = pDoc->GetDetOpList();
        ScDetOpList* pUndoList = NULL;
        if (bUndo)
            pUndoList = pOldList ? new ScDetOpList(*pOldList) : NULL;

        pDoc->ClearDetectiveOperations();

        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDetective( &rDocShell, pUndo, NULL, pUndoList ) );
        }
        aModificator.SetDocumentModified();
        SfxBindings* pBindings = rDocShell.GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_DETECTIVE_REFRESH );
    }
    else
        delete pUndo;

    return bDone;
}

sal_Bool ScDocFunc::DetectiveRefresh( sal_Bool bAutomatic )
{
    sal_Bool bDone = false;
    ScDocument* pDoc = rDocShell.GetDocument();

    bool bUndo (pDoc->IsUndoEnabled());
    ScDetOpList* pList = pDoc->GetDetOpList();
    if ( pList && pList->Count() )
    {
        rDocShell.MakeDrawLayer();
        ScDrawLayer* pModel = pDoc->GetDrawLayer();
        if (bUndo)
            pModel->BeginCalcUndo(false);

        //  Loeschen auf allen Tabellen

        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
            ScDetectiveFunc( pDoc,nTab ).DeleteAll( SC_DET_ARROWS );    // don't remove circles

        //  Wiederholen

        size_t nCount = pList->Count();
        for (size_t i=0; i < nCount; ++i)
        {
            const ScDetOpData* pData = pList->GetObject(i);
            if (pData)
            {
                ScAddress aPos = pData->GetPos();
                ScDetectiveFunc aFunc( pDoc, aPos.Tab() );
                SCCOL nCol = aPos.Col();
                SCROW nRow = aPos.Row();
                switch (pData->GetOperation())
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
                        OSL_FAIL("falsche Op bei DetectiveRefresh");
                }
            }
        }

        if (bUndo)
        {
            SdrUndoGroup* pUndo = pModel->GetCalcUndo();
            if (pUndo)
            {
                pUndo->SetComment( ScGlobal::GetRscString( STR_UNDO_DETREFRESH ) );
                //  wenn automatisch, an letzte Aktion anhaengen
                rDocShell.GetUndoManager()->AddUndoAction(
                                                new ScUndoDraw( pUndo, &rDocShell ),
                                                bAutomatic );
            }
        }
        rDocShell.SetDrawModified();
        bDone = sal_True;
    }
    return bDone;
}

static void lcl_collectAllPredOrSuccRanges(
    const ScRangeList& rSrcRanges, vector<ScTokenRef>& rRefTokens, ScDocShell& rDocShell,
    bool bPred)
{
    ScDocument* pDoc = rDocShell.GetDocument();
    vector<ScTokenRef> aRefTokens;
    ScRangeList aSrcRanges(rSrcRanges);
    if (aSrcRanges.empty())
        return;
    ScRange* p = aSrcRanges.front();
    ScDetectiveFunc aDetFunc(pDoc, p->aStart.Tab());
    ScRangeList aDestRanges;
    for (size_t i = 0, n = aSrcRanges.size(); i < n; ++i)
    {
        p = aSrcRanges[i];
        if (bPred)
        {
            aDetFunc.GetAllPreds(
                p->aStart.Col(), p->aStart.Row(), p->aEnd.Col(), p->aEnd.Row(), aRefTokens);
        }
        else
        {
            aDetFunc.GetAllSuccs(
                p->aStart.Col(), p->aStart.Row(), p->aEnd.Col(), p->aEnd.Row(), aRefTokens);
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

//------------------------------------------------------------------------

sal_Bool ScDocFunc::DeleteContents( const ScMarkData& rMark, sal_uInt16 nFlags,
                                    sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        OSL_FAIL("ScDocFunc::DeleteContents ohne Markierung");
        return false;
    }

    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScEditableTester aTester( pDoc, rMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    ScRange aMarkRange;
    sal_Bool bSimple = false;

    ScMarkData aMultiMark = rMark;
    aMultiMark.SetMarking(false);       // fuer MarkToMulti

    ScDocument* pUndoDoc = NULL;
    sal_Bool bMulti = !bSimple && aMultiMark.IsMultiMarked();
    if (!bSimple)
    {
        aMultiMark.MarkToMulti();
        aMultiMark.GetMultiMarkArea( aMarkRange );
    }
    ScRange aExtendedRange(aMarkRange);
    if (!bSimple)
    {
        if ( pDoc->ExtendMerge( aExtendedRange, sal_True ) )
            bMulti = false;
    }

    // keine Objekte auf geschuetzten Tabellen
    sal_Bool bObjects = false;
    if ( nFlags & IDF_OBJECTS )
    {
        bObjects = sal_True;
        SCTAB nTabCount = pDoc->GetTableCount();
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
            if (pDoc->IsTabProtected(*itr))
                bObjects = false;
    }

    sal_uInt16 nExtFlags = 0;       // extra flags are needed only if attributes are deleted
    if ( nFlags & IDF_ATTRIB )
        rDocShell.UpdatePaintExt( nExtFlags, aMarkRange );

    //  Reihenfolge:
    //  1) BeginDrawUndo
    //  2) Objekte loeschen (DrawUndo wird gefuellt)
    //  3) Inhalte fuer Undo kopieren und Undo-Aktion anlegen
    //  4) Inhalte loeschen

    bool bDrawUndo = bObjects || (nFlags & IDF_NOTE);
    if (bRecord && bDrawUndo)
        pDoc->BeginDrawUndo();

    if (bObjects)
    {
        if (bMulti)
            pDoc->DeleteObjectsInSelection( aMultiMark );
        else
            pDoc->DeleteObjectsInArea( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                                       aMarkRange.aEnd.Col(),   aMarkRange.aEnd.Row(),
                                       aMultiMark );
    }

    if ( bRecord )
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, aMarkRange.aStart.Tab(), aMarkRange.aEnd.Tab() );

        //  bei "Format/Standard" alle Attribute kopieren, weil CopyToDocument
        //  nur mit IDF_HARDATTR zu langsam ist:
        sal_uInt16 nUndoDocFlags = nFlags;
        if (nFlags & IDF_ATTRIB)
            nUndoDocFlags |= IDF_ATTRIB;
        if (nFlags & IDF_EDITATTR)          // Edit-Engine-Attribute
            nUndoDocFlags |= IDF_STRING;    // -> Zellen werden geaendert
        if (nFlags & IDF_NOTE)
            nUndoDocFlags |= IDF_CONTENTS;  // copy all cells with their notes
        // note captions are handled in drawing undo
        nUndoDocFlags |= IDF_NOCAPTIONS;
        pDoc->CopyToDocument( aExtendedRange, nUndoDocFlags, bMulti, pUndoDoc, &aMultiMark );
    }

//! HideAllCursors();   // falls Zusammenfassung aufgehoben wird
    if (bSimple)
        pDoc->DeleteArea( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                          aMarkRange.aEnd.Col(),   aMarkRange.aEnd.Row(),
                          aMultiMark, nFlags );
    else
    {
        pDoc->DeleteSelection( nFlags, aMultiMark );
    }

    // add undo action after drawing undo is complete (objects and note captions)
    if( bRecord )
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDeleteContents( &rDocShell, aMultiMark, aExtendedRange,
                                      pUndoDoc, bMulti, nFlags, bDrawUndo ) );

    if (!AdjustRowHeight( aExtendedRange ))
        rDocShell.PostPaint( aExtendedRange, PAINT_GRID, nExtFlags );
    else if (nExtFlags & SC_PF_LINES)
        lcl_PaintAbove( rDocShell, aExtendedRange );    // fuer Linien ueber dem Bereich

    aModificator.SetDocumentModified();

    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::TransliterateText( const ScMarkData& rMark, sal_Int32 nType,
                                    sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScEditableTester aTester( pDoc, rMark );
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
        SCTAB nTabCount = pDoc->GetTableCount();

        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
            if (*itr != nStartTab)
                pUndoDoc->AddUndoTab( *itr, *itr );

        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pDoc->CopyToDocument( aCopyRange, IDF_CONTENTS, sal_True, pUndoDoc, &aMultiMark );

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoTransliterate( &rDocShell, aMultiMark, pUndoDoc, nType ) );
    }

    pDoc->TransliterateText( aMultiMark, nType );

    if (!AdjustRowHeight( aMarkRange ))
        rDocShell.PostPaint( aMarkRange, PAINT_GRID );

    aModificator.SetDocumentModified();

    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::SetNormalString( bool& o_rbNumFmtSet, const ScAddress& rPos, const String& rText, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument* pDoc = rDocShell.GetDocument();

    bool bUndo(pDoc->IsUndoEnabled());
    ScEditableTester aTester( pDoc, rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    bool bEditDeleted = (pDoc->GetCellType(rPos) == CELLTYPE_EDIT);
    ScUndoEnterData::ValuesType aOldValues;

    if (bUndo)
    {
        ScUndoEnterData::Value aOldValue;

        aOldValue.mnTab = rPos.Tab();
        aOldValue.maCell.assign(*pDoc, rPos);

        const SfxPoolItem* pItem;
        const ScPatternAttr* pPattern = pDoc->GetPattern( rPos.Col(),rPos.Row(),rPos.Tab() );
        if ( SFX_ITEM_SET == pPattern->GetItemSet().GetItemState(
                                ATTR_VALUE_FORMAT,false,&pItem) )
        {
            aOldValue.mbHasFormat = true;
            aOldValue.mnFormat = ((const SfxUInt32Item*)pItem)->GetValue();
        }
        else
            aOldValue.mbHasFormat = false;

        aOldValues.push_back(aOldValue);
    }

    o_rbNumFmtSet = pDoc->SetString( rPos.Col(), rPos.Row(), rPos.Tab(), rText );

    if (bUndo)
    {
        //  wegen ChangeTracking darf UndoAction erst nach SetString angelegt werden
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoEnterData(&rDocShell, rPos, aOldValues, rText, NULL));
    }

    if ( bEditDeleted || pDoc->HasAttrib( ScRange(rPos), HASATTR_NEEDHEIGHT ) )
        AdjustRowHeight( ScRange(rPos) );

    rDocShell.PostPaintCell( rPos );
    aModificator.SetDocumentModified();

    // notify input handler here the same way as in PutCell
    if (bApi)
        NotifyInputHandler( rPos );

    return sal_True;
}

bool ScDocFunc::SetValueCell( const ScAddress& rPos, double fVal, bool bInteraction )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument* pDoc = rDocShell.GetDocument();
    bool bUndo = pDoc->IsUndoEnabled();

    bool bHeight = pDoc->HasAttrib(rPos, HASATTR_NEEDHEIGHT);

    ScCellValue aOldVal;
    if (bUndo)
        aOldVal.assign(*pDoc, rPos);

    pDoc->SetValue(rPos, fVal);

    if (bUndo)
    {
        svl::IUndoManager* pUndoMgr = rDocShell.GetUndoManager();
        ScCellValue aNewVal;
        aNewVal.assign(*pDoc, rPos);
        pUndoMgr->AddUndoAction(new ScUndoSetCell(&rDocShell, rPos, aOldVal, aNewVal));
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

bool ScDocFunc::SetStringCell( const ScAddress& rPos, const OUString& rStr, bool bInteraction )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument* pDoc = rDocShell.GetDocument();
    bool bUndo = pDoc->IsUndoEnabled();

    bool bHeight = pDoc->HasAttrib(rPos, HASATTR_NEEDHEIGHT);

    ScCellValue aOldVal;
    if (bUndo)
        aOldVal.assign(*pDoc, rPos);

    ScSetStringParam aParam;
    aParam.setTextInput();
    pDoc->SetString(rPos, rStr, &aParam);

    if (bUndo)
    {
        svl::IUndoManager* pUndoMgr = rDocShell.GetUndoManager();
        ScCellValue aNewVal;
        aNewVal.assign(*pDoc, rPos);
        pUndoMgr->AddUndoAction(new ScUndoSetCell(&rDocShell, rPos, aOldVal, aNewVal));
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
    ScDocument* pDoc = rDocShell.GetDocument();
    bool bUndo = pDoc->IsUndoEnabled();

    bool bHeight = pDoc->HasAttrib(rPos, HASATTR_NEEDHEIGHT);

    ScCellValue aOldVal;
    if (bUndo)
        aOldVal.assign(*pDoc, rPos);

    pDoc->SetEditText(rPos, rStr.Clone());

    if (bUndo)
    {
        svl::IUndoManager* pUndoMgr = rDocShell.GetUndoManager();
        ScCellValue aNewVal;
        aNewVal.assign(*pDoc, rPos);
        pUndoMgr->AddUndoAction(new ScUndoSetCell(&rDocShell, rPos, aOldVal, aNewVal));
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
    ScDocument* pDoc = rDocShell.GetDocument();

    if (ScStringUtil::isMultiline(rStr))
    {
        ScFieldEditEngine& rEngine = pDoc->GetEditEngine();
        rEngine.SetText(rStr);
        boost::scoped_ptr<EditTextObject> pEditText(rEngine.CreateTextObject());
        return SetEditCell(rPos, *pEditText, bInteraction);
    }
    else
        return SetStringCell(rPos, rStr, bInteraction);
}

bool ScDocFunc::SetFormulaCell( const ScAddress& rPos, ScFormulaCell* pCell, bool bInteraction )
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<ScFormulaCell> xCell(pCell);
    SAL_WNODEPRECATED_DECLARATIONS_POP

    ScDocShellModificator aModificator( rDocShell );
    ScDocument* pDoc = rDocShell.GetDocument();
    bool bUndo = pDoc->IsUndoEnabled();

    bool bHeight = pDoc->HasAttrib(rPos, HASATTR_NEEDHEIGHT);

    ScCellValue aOldVal;
    if (bUndo)
        aOldVal.assign(*pDoc, rPos);

    pDoc->SetFormulaCell(rPos, xCell.release());

    if (bUndo)
    {
        svl::IUndoManager* pUndoMgr = rDocShell.GetUndoManager();
        ScCellValue aNewVal;
        aNewVal.assign(*pDoc, rPos);
        pUndoMgr->AddUndoAction(new ScUndoSetCell(&rDocShell, rPos, aOldVal, aNewVal));
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
    if ( pViewSh && pViewSh->GetViewData()->GetDocShell() == &rDocShell )
    {
        ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
        if ( pInputHdl && pInputHdl->GetCursorPos() == rPos )
        {
            sal_Bool bIsEditMode(pInputHdl->IsEditMode());

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
            sal_Int32   nIndex;
            SfxItemSet  aItemSet;

            ScMyRememberItem(const SfxItemSet& rItemSet, sal_Int32 nTempIndex) :
                nIndex(nTempIndex), aItemSet(rItemSet) {}
        };

        typedef ::std::list<ScMyRememberItem*> ScMyRememberItemList;

bool ScDocFunc::PutData( const ScAddress& rPos, ScEditEngineDefaulter& rEngine, bool bApi )
{
    //  PutData ruft PutCell oder SetNormalString

    sal_Bool bRet = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScEditAttrTester aTester( &rEngine );
    sal_Bool bEditCell = aTester.NeedsObject();
    if ( bEditCell )
    {
        // #i61702# With bLoseContent set, the content of rEngine isn't restored
        // (used in loading XML, where after the removeActionLock call the API obejct's
        // EditEngine isn't accessed again.
        sal_Bool bLoseContent = pDoc->IsImportingXML();

        sal_Bool bUpdateMode(rEngine.GetUpdateMode());
        if (bUpdateMode)
            rEngine.SetUpdateMode(false);

        ScMyRememberItemList aRememberItems;
        ScMyRememberItem* pRememberItem = NULL;

        //  All paragraph attributes must be removed before calling CreateTextObject,
        //  not only alignment, so the object doesn't contain the cell attributes as
        //  paragraph attributes. Before remove the attributes store they in a list to
        //  set they back to the EditEngine.
        sal_Int32 nCount = rEngine.GetParagraphCount();
        for (sal_Int32 i=0; i<nCount; i++)
        {
            const SfxItemSet& rOld = rEngine.GetParaAttribs( i );
            if ( rOld.Count() )
            {
                if ( !bLoseContent )
                {
                    pRememberItem = new ScMyRememberItem(rEngine.GetParaAttribs(i), i);
                    aRememberItems.push_back(pRememberItem);
                }
                rEngine.SetParaAttribs( i, SfxItemSet( *rOld.GetPool(), rOld.GetRanges() ) );
            }
        }

        // A copy of pNewData will be stored in the cell.
        boost::scoped_ptr<EditTextObject> pNewData(rEngine.CreateTextObject());
        bRet = SetEditCell(rPos, *pNewData, !bApi);

        // Set the paragraph attributes back to the EditEngine.
        if (!aRememberItems.empty())
        {
            ScMyRememberItemList::iterator aItr = aRememberItems.begin();
            while (aItr != aRememberItems.end())
            {
                pRememberItem = *aItr;
                rEngine.SetParaAttribs(pRememberItem->nIndex, pRememberItem->aItemSet);
                delete pRememberItem;
                aItr = aRememberItems.erase(aItr);
            }
        }

        // #i61702# if the content isn't accessed, there's no need to set the UpdateMode again
        if ( bUpdateMode && !bLoseContent )
            rEngine.SetUpdateMode(sal_True);
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
        ScPatternAttr aPattern( pDoc->GetPool() );
        aPattern.GetFromEditItemSet( &rEditAttr );
        aPattern.DeleteUnchanged( pDoc->GetPattern( rPos.Col(), rPos.Row(), rPos.Tab() ) );
        aPattern.GetItemSet().ClearItem( ATTR_HOR_JUSTIFY );    // wasn't removed above if no edit object
        if ( aPattern.GetItemSet().Count() > 0 )
        {
            ScMarkData aMark;
            aMark.SelectTable( rPos.Tab(), sal_True );
            aMark.SetMarkArea( ScRange( rPos ) );
            ApplyAttributes( aMark, aPattern, sal_True, bApi );
        }
    }

    return bRet;
}


static ScTokenArray* lcl_ScDocFunc_CreateTokenArrayXML( const String& rText, const String& rFormulaNmsp, const formula::FormulaGrammar::Grammar eGrammar )
{
    ScTokenArray* pCode = new ScTokenArray;
    pCode->AddStringXML( rText );
    if( (eGrammar == formula::FormulaGrammar::GRAM_EXTERNAL) && (rFormulaNmsp.Len() > 0) )
        pCode->AddStringXML( rFormulaNmsp );
    return pCode;
}

bool ScDocFunc::SetCellText(
    const ScAddress& rPos, const OUString& rText, bool bInterpret, bool bEnglish, bool bApi,
    const formula::FormulaGrammar::Grammar eGrammar )
{
    if ( bInterpret )
    {
        if ( bEnglish )
        {
            ScDocument* pDoc = rDocShell.GetDocument();

            ::boost::scoped_ptr<ScExternalRefManager::ApiGuard> pExtRefGuard;
            if (bApi)
                pExtRefGuard.reset(new ScExternalRefManager::ApiGuard(pDoc));

            ScInputStringType aRes =
                ScStringUtil::parseInputString(*pDoc->GetFormatTable(), rText, LANGUAGE_ENGLISH_US);

            switch (aRes.meType)
            {
                case ScInputStringType::Formula:
                    SetFormulaCell(rPos, new ScFormulaCell(pDoc, rPos, aRes.maText, eGrammar), !bApi);
                break;
                case ScInputStringType::Number:
                    SetValueCell(rPos, aRes.mfValue, !bApi);
                break;
                case ScInputStringType::Text:
                    SetStringOrEditCell(rPos, aRes.maText, !bApi);
                break;
                default:
                    ;
            }
        }
        // sonst Null behalten -> SetString mit lokalen Formeln/Zahlformat
    }
    else if (!rText.isEmpty())
    {
        SetStringOrEditCell(rPos, rText, !bApi);
    }

    bool bNumFmtSet = false;
    return SetNormalString( bNumFmtSet, rPos, rText, bApi );
}

//------------------------------------------------------------------------

bool ScDocFunc::ShowNote( const ScAddress& rPos, bool bShow )
{
    ScDocument& rDoc = *rDocShell.GetDocument();
    ScPostIt* pNote = rDoc.GetNotes( rPos.Tab() )->findByAddress( rPos );
    if( !pNote || (bShow == pNote->IsCaptionShown()) ) return false;

    // move the caption to internal or hidden layer and create undo action
    pNote->ShowCaption( rPos, bShow );
    if( rDoc.IsUndoEnabled() )
        rDocShell.GetUndoManager()->AddUndoAction( new ScUndoShowHideNote( rDocShell, rPos, bShow ) );

    if (rDoc.IsStreamValid(rPos.Tab()))
        rDoc.SetStreamValid(rPos.Tab(), false);

    rDocShell.SetDocumentModified();

    return true;
}

//------------------------------------------------------------------------

bool ScDocFunc::SetNoteText( const ScAddress& rPos, const String& rText, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    ScEditableTester aTester( pDoc, rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    String aNewText = convertLineEnd(rText, GetSystemLineEnd()); //! ist das noetig ???

    if( ScPostIt* pNote = (aNewText.Len() > 0) ? pDoc->GetNotes(rPos.Tab())->GetOrCreateNote( rPos ) : pDoc->GetNotes( rPos.Tab() )->findByAddress(rPos) )
        pNote->SetText( rPos, aNewText );

    //! Undo !!!

    if (pDoc->IsStreamValid(rPos.Tab()))
        pDoc->SetStreamValid(rPos.Tab(), false);

    rDocShell.PostPaintCell( rPos );
    aModificator.SetDocumentModified();

    return true;
}

//------------------------------------------------------------------------

bool ScDocFunc::ReplaceNote( const ScAddress& rPos, const OUString& rNoteText, const OUString* pAuthor, const OUString* pDate, sal_Bool bApi )
{
    bool bDone = false;

    ScDocShellModificator aModificator( rDocShell );
    ScDocument& rDoc = *rDocShell.GetDocument();
    ScEditableTester aTester( &rDoc, rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() );
    if (aTester.IsEditable())
    {
        ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
        ::svl::IUndoManager* pUndoMgr = (pDrawLayer && rDoc.IsUndoEnabled()) ? rDocShell.GetUndoManager() : 0;

        ScNoteData aOldData;
        ScPostIt* pOldNote = rDoc.GetNotes(rPos.Tab())->ReleaseNote( rPos );
        if( pOldNote )
        {
            // ensure existing caption object before draw undo tracking starts
            pOldNote->GetOrCreateCaption( rPos );
            // rescue note data for undo
            aOldData = pOldNote->GetNoteData();
        }

        // collect drawing undo actions for deleting/inserting caption obejcts
        if( pUndoMgr )
            pDrawLayer->BeginCalcUndo(false);

        // delete the note (creates drawing undo action for the caption object)
        delete pOldNote;

        // create new note (creates drawing undo action for the new caption object)
        ScNoteData aNewData;
        if( ScPostIt* pNewNote = ScNoteUtil::CreateNoteFromString( rDoc, rPos, rNoteText, false, true ) )
        {
            if( pAuthor ) pNewNote->SetAuthor( *pAuthor );
            if( pDate ) pNewNote->SetDate( *pDate );
            // rescue note data for undo
            aNewData = pNewNote->GetNoteData();
        }

        // create the undo action
        if( pUndoMgr && (aOldData.mpCaption || aNewData.mpCaption) )
            pUndoMgr->AddUndoAction( new ScUndoReplaceNote( rDocShell, rPos, aOldData, aNewData, pDrawLayer->GetCalcUndo() ) );

        // repaint cell (to make note marker visible)
        rDocShell.PostPaintCell( rPos );

        if (rDoc.IsStreamValid(rPos.Tab()))
            rDoc.SetStreamValid(rPos.Tab(), false);

        aModificator.SetDocumentModified();
        bDone = true;
    }
    else if (!bApi)
    {
        rDocShell.ErrorMessage(aTester.GetMessageId());
    }

    return bDone;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
                                    sal_Bool bRecord, sal_Bool bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if ( bRecord && !pDoc->IsUndoEnabled() )
        bRecord = false;

    bool bImportingXML = pDoc->IsImportingXML();
    // Cell formats can still be set if the range isn't editable only because of matrix formulas.
    // #i62483# When loading XML, the check can be skipped altogether.
    bool bOnlyNotBecauseOfMatrix;
    if ( !bImportingXML && !pDoc->IsSelectionEditable( rMark, &bOnlyNotBecauseOfMatrix )
            && !bOnlyNotBecauseOfMatrix )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return false;
    }

    ScDocShellModificator aModificator( rDocShell );

    //! Umrandung

    ScRange aMultiRange;
    sal_Bool bMulti = rMark.IsMultiMarked();
    if ( bMulti )
        rMark.GetMultiMarkArea( aMultiRange );
    else
        rMark.GetMarkArea( aMultiRange );

    if ( bRecord )
    {
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, aMultiRange.aStart.Tab(), aMultiRange.aEnd.Tab() );
        pDoc->CopyToDocument( aMultiRange, IDF_ATTRIB, bMulti, pUndoDoc, &rMark );

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoSelectionAttr(
                    &rDocShell, rMark,
                    aMultiRange.aStart.Col(), aMultiRange.aStart.Row(), aMultiRange.aStart.Tab(),
                    aMultiRange.aEnd.Col(), aMultiRange.aEnd.Row(), aMultiRange.aEnd.Tab(),
                    pUndoDoc, bMulti, &rPattern ) );
    }

    // While loading XML it is not necessary to ask HasAttrib. It needs too much time.
    sal_uInt16 nExtFlags = 0;
    if ( !bImportingXML )
        rDocShell.UpdatePaintExt( nExtFlags, aMultiRange );     // content before the change
    pDoc->ApplySelectionPattern( rPattern, rMark );
    if ( !bImportingXML )
        rDocShell.UpdatePaintExt( nExtFlags, aMultiRange );     // content after the change

    if (!AdjustRowHeight( aMultiRange ))
        rDocShell.PostPaint( aMultiRange, PAINT_GRID, nExtFlags );
    else if (nExtFlags & SC_PF_LINES)
        lcl_PaintAbove( rDocShell, aMultiRange );   // fuer Linien ueber dem Bereich

    aModificator.SetDocumentModified();

    return sal_True;
}


sal_Bool ScDocFunc::ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
                                sal_Bool bRecord, sal_Bool bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if ( bRecord && !pDoc->IsUndoEnabled() )
        bRecord = false;

    bool bImportingXML = pDoc->IsImportingXML();
    // Cell formats can still be set if the range isn't editable only because of matrix formulas.
    // #i62483# When loading XML, the check can be skipped altogether.
    bool bOnlyNotBecauseOfMatrix;
    if ( !bImportingXML && !pDoc->IsSelectionEditable( rMark, &bOnlyNotBecauseOfMatrix )
            && !bOnlyNotBecauseOfMatrix )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return false;
    }

    ScStyleSheet* pStyleSheet = (ScStyleSheet*) pDoc->GetStyleSheetPool()->Find(
                                                rStyleName, SFX_STYLE_FAMILY_PARA );
    if (!pStyleSheet)
        return false;

    ScDocShellModificator aModificator( rDocShell );

    ScRange aMultiRange;
    sal_Bool bMulti = rMark.IsMultiMarked();
    if ( bMulti )
        rMark.GetMultiMarkArea( aMultiRange );
    else
        rMark.GetMarkArea( aMultiRange );

    if ( bRecord )
    {
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        SCTAB nStartTab = aMultiRange.aStart.Tab();
        SCTAB nTabCount = pDoc->GetTableCount();
        pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
            if (*itr != nStartTab)
                pUndoDoc->AddUndoTab( *itr, *itr );

        ScRange aCopyRange = aMultiRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, bMulti, pUndoDoc, &rMark );

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoSelectionStyle(
                    &rDocShell, rMark, aMultiRange, rStyleName, pUndoDoc ) );

    }

    pDoc->ApplySelectionStyle( (ScStyleSheet&)*pStyleSheet, rMark );

    sal_uInt16 nExtFlags = 0;
    if (!AdjustRowHeight( aMultiRange ))
        rDocShell.PostPaint( aMultiRange, PAINT_GRID, nExtFlags );
    else if (nExtFlags & SC_PF_LINES)
        lcl_PaintAbove( rDocShell, aMultiRange );   // fuer Linien ueber dem Bereich

    aModificator.SetDocumentModified();

    return sal_True;
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
    ScMarkData::const_iterator itBeg = rMarkData.begin(), itEnd = rMarkData.end();

    ScRange aRange(rRange); // local copy
    switch (eCmd)
    {
        case INS_INSROWS:
        {
            aRange.aStart.SetCol(0);
            aRange.aEnd.SetCol(MAXCOL);
            // Continue below.
        }
        case INS_CELLSDOWN:
        {
            for (ScMarkData::const_iterator it = itBeg; it != itEnd; ++it)
            {
                if (pDPs->IntersectsTableByColumns(aRange.aStart.Col(), aRange.aEnd.Col(), aRange.aStart.Row(), *it))
                    // This column range cuts through at least one pivot table.  Not good.
                    return false;
            }

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
            for (ScMarkData::const_iterator it = itBeg; it != itEnd; ++it)
            {
                aTest.aStart.SetTab(*it);
                aTest.aEnd.SetTab(*it);
                if (pDPs->HasTable(aTest))
                    return false;
            }
        }
        break;
        case INS_INSCOLS:
        {
            aRange.aStart.SetRow(0);
            aRange.aEnd.SetRow(MAXROW);
            // Continue below.
        }
        case INS_CELLSRIGHT:
        {
            for (ScMarkData::const_iterator it = itBeg; it != itEnd; ++it)
            {
                if (pDPs->IntersectsTableByRows(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Row(), *it))
                    // This column range cuts through at least one pivot table.  Not good.
                    return false;
            }

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
            for (ScMarkData::const_iterator it = itBeg; it != itEnd; ++it)
            {
                aTest.aStart.SetTab(*it);
                aTest.aEnd.SetTab(*it);
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
    ScMarkData::const_iterator itBeg = rMarkData.begin(), itEnd = rMarkData.end();

    ScRange aRange(rRange); // local copy

    switch (eCmd)
    {
        case DEL_DELROWS:
        {
            aRange.aStart.SetCol(0);
            aRange.aEnd.SetCol(MAXCOL);
            // Continue below.
        }
        case DEL_CELLSUP:
        {
            for (ScMarkData::const_iterator it = itBeg; it != itEnd; ++it)
            {
                if (pDPs->IntersectsTableByColumns(aRange.aStart.Col(), aRange.aEnd.Col(), aRange.aStart.Row(), *it))
                    // This column range cuts through at least one pivot table.  Not good.
                    return false;
            }

            ScRange aTest(aRange);
            for (ScMarkData::const_iterator it = itBeg; it != itEnd; ++it)
            {
                aTest.aStart.SetTab(*it);
                aTest.aEnd.SetTab(*it);
                if (pDPs->HasTable(aTest))
                    return false;
            }
        }
        break;
        case DEL_DELCOLS:
        {
            aRange.aStart.SetRow(0);
            aRange.aEnd.SetRow(MAXROW);
            // Continue below.
        }
        case DEL_CELLSLEFT:
        {
            for (ScMarkData::const_iterator it = itBeg; it != itEnd; ++it)
            {
                if (pDPs->IntersectsTableByRows(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Row(), *it))
                    // This column range cuts through at least one pivot table.  Not good.
                    return false;
            }

            ScRange aTest(aRange);
            for (ScMarkData::const_iterator it = itBeg; it != itEnd; ++it)
            {
                aTest.aStart.SetTab(*it);
                aTest.aEnd.SetTab(*it);
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

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if ( !ValidRow(nStartRow) || !ValidRow(nEndRow) )
    {
        OSL_FAIL("invalid row in InsertCells");
        return false;
    }

    ScDocument* pDoc = rDocShell.GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    SCCOL nPaintStartCol = nStartCol;
    SCROW nPaintStartRow = nStartRow;
    SCCOL nPaintEndCol = nEndCol;
    SCROW nPaintEndRow = nEndRow;
    sal_uInt16 nPaintFlags = PAINT_GRID;
    bool bSuccess;
    SCTAB i;

    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();  //preserve current cursor position
    SCCOL nCursorCol = 0;
    SCROW nCursorRow = 0;
    if( pViewSh )
    {
        nCursorCol = pViewSh->GetViewData()->GetCurX();
        nCursorRow = pViewSh->GetViewData()->GetCurY();
    }

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        SCTAB nCount = 0;
        for( i=0; i<nTabCount; i++ )
        {
            if( !pDoc->IsScenario(i) )
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
    ScMarkData::iterator itr = aMark.begin(), itrEnd = aMark.end();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
        for( SCTAB j = *itr+1; j<nTabCount && pDoc->IsScenario(j); j++ )
            aFullMark.SelectTable( j, true );

    SCTAB nSelCount = aMark.GetSelectCount();

    //  zugehoerige Szenarien auch anpassen
    // Test zusammengefasste

    SCCOL nMergeTestStartCol = nStartCol;
    SCROW nMergeTestStartRow = nStartRow;
    SCCOL nMergeTestEndCol = nEndCol;
    SCROW nMergeTestEndRow = nEndRow;

    ScRange aExtendMergeRange( rRange );

    if( rRange.aStart == rRange.aEnd && pDoc->HasAttrib(rRange, HASATTR_MERGED) )
    {
        pDoc->ExtendMerge( aExtendMergeRange );
        pDoc->ExtendOverlapped( aExtendMergeRange );
        nMergeTestEndCol = aExtendMergeRange.aEnd.Col();
        nMergeTestEndRow = aExtendMergeRange.aEnd.Row();
        nPaintEndCol = nMergeTestEndCol;
        nPaintEndRow = nMergeTestEndRow;
    }

    if ( eCmd == INS_INSROWS )
    {
        nMergeTestStartCol = 0;
        nMergeTestEndCol = MAXCOL;
    }
    if ( eCmd == INS_INSCOLS )
    {
        nMergeTestStartRow = 0;
        nMergeTestEndRow = MAXROW;
    }
    if ( eCmd == INS_CELLSDOWN )
        nMergeTestEndRow = MAXROW;
    if ( eCmd == INS_CELLSRIGHT )
        nMergeTestEndCol = MAXCOL;

    bool bNeedRefresh = false;

    SCCOL nEditTestEndCol = (eCmd==INS_INSCOLS) ? MAXCOL : nMergeTestEndCol;
    SCROW nEditTestEndRow = (eCmd==INS_INSROWS) ? MAXROW : nMergeTestEndRow;
    ScEditableTester aTester( pDoc, nMergeTestStartCol, nMergeTestStartRow, nEditTestEndCol, nEditTestEndRow, aMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    // Check if this insertion is allowed with respect to pivot table.
    if (!canInsertCellsByPivot(rRange, aMark, eCmd, pDoc))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_NO_INSERT_DELETE_OVER_PIVOT_TABLE);
        return false;
    }

    WaitObject aWait( rDocShell.GetActiveDialogParent() );      // wichtig wegen TrackFormulas bei UpdateReference

    ScDocument* pRefUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if ( bRecord )
    {
        pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1, false, false );

        // pRefUndoDoc is filled in InsertCol / InsertRow

        pUndoData = new ScRefUndoData( pDoc );

        pDoc->BeginDrawUndo();
    }

    // #i8302 : we unmerge overwhelming ranges, before insertion all the actions are put in the same ListAction
    // the patch comes from mloiseleur and maoyg
    bool bInsertMerge = false;
    std::vector<ScRange> qIncreaseRange;
    String aUndo = ScGlobal::GetRscString( STR_UNDO_INSERTCELLS );
    if (bRecord)
        rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo );

    itr = aMark.begin();
    for (; itr != itrEnd && nTabCount; ++itr)
    {
        pDoc->InitializeNoteCaptions(*itr);

        i = *itr;
        if( pDoc->HasAttrib( nMergeTestStartCol, nMergeTestStartRow, i, nMergeTestEndCol, nMergeTestEndRow, i, HASATTR_MERGED | HASATTR_OVERLAPPED ) )
        {
            if (eCmd==INS_CELLSRIGHT)
                bNeedRefresh = true;

            SCCOL nMergeStartCol = nMergeTestStartCol;
            SCROW nMergeStartRow = nMergeTestStartRow;
            SCCOL nMergeEndCol   = nMergeTestEndCol;
            SCROW nMergeEndRow   = nMergeTestEndRow;

            pDoc->ExtendMerge( nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i );
            pDoc->ExtendOverlapped( nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i );

            if(( eCmd == INS_CELLSDOWN && ( nMergeStartCol != nMergeTestStartCol || nMergeEndCol != nMergeTestEndCol )) ||
                (eCmd == INS_CELLSRIGHT && ( nMergeStartRow != nMergeTestStartRow || nMergeEndRow != nMergeTestEndRow )) )
            {
                if (!bApi)
                    rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0);
                rDocShell.GetUndoManager()->LeaveListAction();
                delete pUndoData;
                return false;
            }

            SCCOL nTestCol = -1;
            SCROW nTestRow1 = -1;
            SCROW nTestRow2 = -1;

            ScDocAttrIterator aTestIter( pDoc, i, nMergeTestStartCol, nMergeTestStartRow, nMergeTestEndCol, nMergeTestEndRow );
            ScRange aExtendRange( nMergeTestStartCol, nMergeTestStartRow, i, nMergeTestEndCol, nMergeTestEndRow, i );
            const ScPatternAttr* pPattern = NULL;
            const ScMergeAttr* pMergeFlag = NULL;
            const ScMergeFlagAttr* pMergeFlagAttr = NULL;
            while ( ( pPattern = aTestIter.GetNext( nTestCol, nTestRow1, nTestRow2 ) ) != NULL )
            {
                pMergeFlag = (const ScMergeAttr*) &pPattern->GetItem(ATTR_MERGE);
                pMergeFlagAttr = (const ScMergeFlagAttr*) &pPattern->GetItem(ATTR_MERGE_FLAG);
                sal_Int16 nNewFlags = pMergeFlagAttr->GetValue() & ( SC_MF_HOR | SC_MF_VER );
                if( ( pMergeFlag && pMergeFlag->IsMerged() ) || nNewFlags == SC_MF_HOR || nNewFlags == SC_MF_VER )
                {
                    ScRange aRange( nTestCol, nTestRow1, i );
                    pDoc->ExtendOverlapped(aRange);
                    pDoc->ExtendMerge(aRange, true);

                    if( nTestRow1 < nTestRow2 && nNewFlags == SC_MF_HOR )
                    {
                        for( SCROW nTestRow = nTestRow1; nTestRow <= nTestRow2; nTestRow++ )
                        {
                            ScRange aTestRange( nTestCol, nTestRow, i );
                            pDoc->ExtendOverlapped( aTestRange );
                            pDoc->ExtendMerge( aTestRange, true);
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
                if( eCmd == INS_INSROWS || eCmd == INS_CELLSDOWN )
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
                else if( eCmd == INS_CELLSRIGHT || eCmd == INS_INSCOLS )
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
                    for( ::std::vector<ScRange>::const_iterator iIter( qIncreaseRange.begin()); iIter != qIncreaseRange.end(); ++iIter )
                    {
                        ScRange aRange( *iIter );
                        if( pDoc->HasAttrib( aRange, HASATTR_OVERLAPPED | HASATTR_MERGED ) )
                        {
                            UnmergeCells( aRange, true );
                        }
                    }
                }
            }
            else
            {
                if (!bApi)
                    rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0);
                rDocShell.GetUndoManager()->LeaveListAction();
                delete pUndoData;
                return false;
            }
        }
    }

    switch (eCmd)
    {
        case INS_CELLSDOWN:
            bSuccess = pDoc->InsertRow( nStartCol, 0, nEndCol, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc, &aFullMark );
            nPaintEndRow = MAXROW;
            break;
        case INS_INSROWS:
            bSuccess = pDoc->InsertRow( 0, 0, MAXCOL, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc, &aFullMark );
            nPaintStartCol = 0;
            nPaintEndCol = MAXCOL;
            nPaintEndRow = MAXROW;
            nPaintFlags |= PAINT_LEFT;
            break;
        case INS_CELLSRIGHT:
            bSuccess = pDoc->InsertCol( nStartRow, 0, nEndRow, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc, &aFullMark );
            nPaintEndCol = MAXCOL;
            break;
        case INS_INSCOLS:
            bSuccess = pDoc->InsertCol( 0, 0, MAXROW, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc, &aFullMark );
            nPaintStartRow = 0;
            nPaintEndRow = MAXROW;
            nPaintEndCol = MAXCOL;
            nPaintFlags |= PAINT_TOP;
            break;
        default:
            OSL_FAIL("Falscher Code beim Einfuegen");
            bSuccess = false;
            break;
    }

    if ( bSuccess )
    {
        SCTAB* pTabs      = NULL;
        SCTAB* pScenarios = NULL;
        SCTAB  nUndoPos  = 0;

        if ( bRecord )
        {
            pTabs       = new SCTAB[nSelCount];
            pScenarios  = new SCTAB[nSelCount];
            nUndoPos    = 0;
            itr = aMark.begin();
            for (; itr != itrEnd && *itr < nTabCount; ++itr)
            {
                SCTAB nCount = 0;
                for( SCTAB j=*itr+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                    nCount ++;

                pScenarios[nUndoPos] = nCount;
                pTabs[nUndoPos] = *itr;
                nUndoPos ++;
            }

            if( !bInsertMerge )
            {
                rDocShell.GetUndoManager()->LeaveListAction();
            }

            rDocShell.GetUndoManager()->AddUndoAction( new ScUndoInsertCells(
                &rDocShell, ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab ),
                nUndoPos, pTabs, pScenarios, eCmd, pRefUndoDoc, pUndoData, bPartOfPaste ) );
        }

        // #i8302 : we remerge growing ranges, with the new part inserted

        while( !qIncreaseRange.empty() )
        {
            ScRange aRange = qIncreaseRange.back();
            if( !pDoc->HasAttrib( aRange, HASATTR_OVERLAPPED | HASATTR_MERGED ) )
            {
                switch (eCmd)
                {
                    case INS_CELLSDOWN:
                    case INS_INSROWS:
                        aRange.aEnd.IncRow(static_cast<SCsCOL>(nEndRow-nStartRow+1));
                        break;
                    case INS_CELLSRIGHT:
                    case INS_INSCOLS:
                        aRange.aEnd.IncCol(static_cast<SCsCOL>(nEndCol-nStartCol+1));
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

        itr = aMark.begin();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
        {
            i = *itr;
            pDoc->SetDrawPageSize(i);

            if (bNeedRefresh)
                pDoc->ExtendMerge( nMergeTestStartCol, nMergeTestStartRow, nMergeTestEndCol, nMergeTestEndRow, i, true );
            else
                pDoc->RefreshAutoFilter( nMergeTestStartCol, nMergeTestStartRow, nMergeTestEndCol, nMergeTestEndRow, i );

            if ( eCmd == INS_INSROWS || eCmd == INS_INSCOLS )
                pDoc->UpdatePageBreaks( i );

            sal_uInt16 nExtFlags = 0;
            rDocShell.UpdatePaintExt( nExtFlags, nPaintStartCol, nPaintStartRow, i, nPaintEndCol, nPaintEndRow, i );

            SCTAB nScenarioCount = 0;

            for( SCTAB j = i+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                nScenarioCount ++;

            bool bAdjusted = ( eCmd == INS_INSROWS ) ? AdjustRowHeight(ScRange(0, nStartRow, i, MAXCOL, nEndRow, i+nScenarioCount )) :
                                                       AdjustRowHeight(ScRange(0, nPaintStartRow, i, MAXCOL, nPaintEndRow, i+nScenarioCount ));
            if (bAdjusted)
            {
                //  paint only what is not done by AdjustRowHeight
                if (nPaintFlags & PAINT_TOP)
                    rDocShell.PostPaint( nPaintStartCol, nPaintStartRow, i, nPaintEndCol, nPaintEndRow, i+nScenarioCount, PAINT_TOP );
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
                pViewSh->MarkRange( rRange, false );
                pViewSh->SetCursor( nCursorCol, nCursorRow );
            }
        }

        rDocShell.GetUndoManager()->LeaveListAction();
        rDocShell.GetUndoManager()->RemoveLastUndoAction();

        delete pRefUndoDoc;
        delete pUndoData;
        if (!bApi)
            rDocShell.ErrorMessage(STR_INSERT_FULL);        // Spalte/Zeile voll
    }

    aModificator.SetDocumentModified();

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
    return bSuccess;
}

bool ScDocFunc::DeleteCells( const ScRange& rRange, const ScMarkData* pTabMark, DelCellCmd eCmd,
                             bool bRecord, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

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

    ScDocument* pDoc = rDocShell.GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    SCCOL nPaintStartCol = nStartCol;
    SCROW nPaintStartRow = nStartRow;
    SCCOL nPaintEndCol = nEndCol;
    SCROW nPaintEndRow = nEndRow;
    sal_uInt16 nPaintFlags = PAINT_GRID;

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        SCTAB nCount = 0;
        for(SCTAB i=0; i<nTabCount; i++ )
        {
            if( !pDoc->IsScenario(i) )
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
    ScMarkData::iterator itr = aMark.begin(), itrEnd = aMark.end();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
        for( SCTAB j = *itr+1; j<nTabCount && pDoc->IsScenario(j); j++ )
            aFullMark.SelectTable( j, true );

    SCTAB nSelCount = aMark.GetSelectCount();

    SCCOL nUndoStartCol = nStartCol;
    SCROW nUndoStartRow = nStartRow;
    SCCOL nUndoEndCol = nEndCol;
    SCROW nUndoEndRow = nEndRow;

    ScRange aExtendMergeRange( rRange );

    if( rRange.aStart == rRange.aEnd && pDoc->HasAttrib(rRange, HASATTR_MERGED) )
    {
        pDoc->ExtendMerge( aExtendMergeRange );
        pDoc->ExtendOverlapped( aExtendMergeRange );
        nUndoEndCol = aExtendMergeRange.aEnd.Col();
        nUndoEndRow = aExtendMergeRange.aEnd.Row();
        nPaintEndCol = nUndoEndCol;
        nPaintEndRow = nUndoEndRow;
    }

    if (eCmd==DEL_DELROWS)
    {
        nUndoStartCol = 0;
        nUndoEndCol = MAXCOL;
    }
    if (eCmd==DEL_DELCOLS)
    {
        nUndoStartRow = 0;
        nUndoEndRow = MAXROW;
    }
                    // Test Zellschutz

    SCCOL nEditTestEndX = nUndoEndCol;
    if ( eCmd==DEL_DELCOLS || eCmd==DEL_CELLSLEFT )
        nEditTestEndX = MAXCOL;
    SCROW nEditTestEndY = nUndoEndRow;
    if ( eCmd==DEL_DELROWS || eCmd==DEL_CELLSUP )
        nEditTestEndY = MAXROW;
    ScEditableTester aTester( pDoc, nUndoStartCol, nUndoStartRow, nEditTestEndX, nEditTestEndY, aMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if (!canDeleteCellsByPivot(rRange, aMark, eCmd, pDoc))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_NO_INSERT_DELETE_OVER_PIVOT_TABLE);
        return false;
    }
                    // Test zusammengefasste

    SCCOL nMergeTestEndCol = (eCmd==DEL_CELLSLEFT) ? MAXCOL : nUndoEndCol;
    SCROW nMergeTestEndRow = (eCmd==DEL_CELLSUP)   ? MAXROW : nUndoEndRow;
    SCCOL nExtendStartCol = nUndoStartCol;
    SCROW nExtendStartRow = nUndoStartRow;
    bool bNeedRefresh = false;

    //Issue 8302 want to be able to insert into the middle of merged cells
    //the patch comes from maoyg
    ::std::vector<ScRange> qDecreaseRange;
    bool bDeletingMerge = false;
    String aUndo = ScGlobal::GetRscString( STR_UNDO_DELETECELLS );
    if (bRecord)
        rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo );

    itr = aMark.begin();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
    {
        pDoc->InitializeNoteCaptions(*itr);

        SCTAB i = *itr;
        if ( pDoc->HasAttrib( nUndoStartCol, nUndoStartRow, i, nMergeTestEndCol, nMergeTestEndRow, i, HASATTR_MERGED | HASATTR_OVERLAPPED ))
        {
            SCCOL nMergeStartCol = nUndoStartCol;
            SCROW nMergeStartRow = nUndoStartRow;
            SCCOL nMergeEndCol   = nMergeTestEndCol;
            SCROW nMergeEndRow   = nMergeTestEndRow;

            pDoc->ExtendMerge( nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i );
            pDoc->ExtendOverlapped( nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i );
            if( ( eCmd == DEL_CELLSUP && ( nMergeStartCol != nUndoStartCol || nMergeEndCol != nMergeTestEndCol))||
                ( eCmd == DEL_CELLSLEFT && ( nMergeStartRow != nUndoStartRow || nMergeEndRow != nMergeTestEndRow)))
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

            ScDocAttrIterator aTestIter( pDoc, i, nUndoStartCol, nUndoStartRow, nMergeTestEndCol, nMergeTestEndRow );
            ScRange aExtendRange( nUndoStartCol, nUndoStartRow, i, nMergeTestEndCol, nMergeTestEndRow, i );
            const ScPatternAttr* pPattern = NULL;
            const ScMergeAttr* pMergeFlag = NULL;
            const ScMergeFlagAttr* pMergeFlagAttr = NULL;
            while ( ( pPattern = aTestIter.GetNext( nTestCol, nTestRow1, nTestRow2 ) ) != NULL )
            {
                pMergeFlag = (const ScMergeAttr*) &pPattern->GetItem( ATTR_MERGE );
                pMergeFlagAttr = (const ScMergeFlagAttr*) &pPattern->GetItem( ATTR_MERGE_FLAG );
                sal_Int16 nNewFlags = pMergeFlagAttr->GetValue() & ( SC_MF_HOR | SC_MF_VER );
                if( ( pMergeFlag && pMergeFlag->IsMerged() ) || nNewFlags == SC_MF_HOR || nNewFlags == SC_MF_VER )
                {
                    ScRange aRange( nTestCol, nTestRow1, i );
                    pDoc->ExtendOverlapped( aRange );
                    pDoc->ExtendMerge( aRange, true );

                    if( nTestRow1 < nTestRow2 && nNewFlags == SC_MF_HOR )
                    {
                        for( SCROW nTestRow = nTestRow1; nTestRow <= nTestRow2; nTestRow++ )
                        {
                            ScRange aTestRange( nTestCol, nTestRow, i );
                            pDoc->ExtendOverlapped( aTestRange );
                            pDoc->ExtendMerge( aTestRange, true );
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

                if( eCmd == DEL_DELROWS || eCmd == DEL_CELLSUP )
                {
                    nStartRow = aExtendMergeRange.aStart.Row();
                    nEndRow = aExtendMergeRange.aEnd.Row();
                    bNeedRefresh = true;

                    if( eCmd == DEL_CELLSUP )
                    {
                        nEndCol = aExtendMergeRange.aEnd.Col();
                    }
                    else
                    {
                        nStartCol = 0;
                        nEndCol = MAXCOL;
                    }
                }
                else if( eCmd == DEL_CELLSLEFT || eCmd == DEL_DELCOLS )
                {

                    nStartCol = aExtendMergeRange.aStart.Col();
                    nEndCol = aExtendMergeRange.aEnd.Col();
                    if( eCmd == DEL_CELLSLEFT )
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
                    for( ::std::vector<ScRange>::const_iterator iIter( qDecreaseRange.begin()); iIter != qDecreaseRange.end(); ++iIter )
                    {
                        ScRange aRange( *iIter );
                        if( pDoc->HasAttrib( aRange, HASATTR_OVERLAPPED | HASATTR_MERGED ) )
                        {
                            UnmergeCells( aRange, true );
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

    //
    //      ausfuehren
    //

    WaitObject aWait( rDocShell.GetActiveDialogParent() );      // wichtig wegen TrackFormulas bei UpdateReference

    ScDocument* pUndoDoc = NULL;
    ScDocument* pRefUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if ( bRecord )
    {
        // With the fix for #101329#, UpdateRef always puts cells into pRefUndoDoc at their old position,
        // so it's no longer necessary to copy more than the deleted range into pUndoDoc.

        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, 0, nTabCount-1, (eCmd==DEL_DELCOLS), (eCmd==DEL_DELROWS) );
        itr = aMark.begin();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
        {
            SCTAB nScenarioCount = 0;

            for( SCTAB j = *itr+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                nScenarioCount ++;

            pDoc->CopyToDocument( nUndoStartCol, nUndoStartRow, *itr, nUndoEndCol, nUndoEndRow, *itr+nScenarioCount,
                IDF_ALL | IDF_NOCAPTIONS, false, pUndoDoc );
        }

        pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1, false, false );

        pUndoData = new ScRefUndoData( pDoc );

        pDoc->BeginDrawUndo();
    }

    sal_uInt16 nExtFlags = 0;
    itr = aMark.begin();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
    {
        rDocShell.UpdatePaintExt( nExtFlags, nStartCol, nStartRow, *itr, nEndCol, nEndRow, *itr );
    }

    bool bUndoOutline = false;
    switch (eCmd)
    {
        case DEL_CELLSUP:
            pDoc->DeleteRow( nStartCol, 0, nEndCol, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc, NULL, &aFullMark );
            nPaintEndRow = MAXROW;
            break;
        case DEL_DELROWS:
            pDoc->DeleteRow( 0, 0, MAXCOL, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc, &bUndoOutline, &aFullMark );
            nPaintStartCol = 0;
            nPaintEndCol = MAXCOL;
            nPaintEndRow = MAXROW;
            nPaintFlags |= PAINT_LEFT;
            break;
        case DEL_CELLSLEFT:
            pDoc->DeleteCol( nStartRow, 0, nEndRow, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc, NULL, &aFullMark );
            nPaintEndCol = MAXCOL;
            break;
        case DEL_DELCOLS:
            pDoc->DeleteCol( 0, 0, MAXROW, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc, &bUndoOutline, &aFullMark );
            nPaintStartRow = 0;
            nPaintEndRow = MAXROW;
            nPaintEndCol = MAXCOL;
            nPaintFlags |= PAINT_TOP;
            break;
        default:
            OSL_FAIL("Falscher Code beim Loeschen");
            break;
    }

    //! Test, ob Outline in Groesse geaendert

    if ( bRecord )
    {
        itr = aFullMark.begin(), itrEnd = aFullMark.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
            pRefUndoDoc->DeleteAreaTab(nUndoStartCol,nUndoStartRow,nUndoEndCol,nUndoEndRow, *itr, IDF_ALL);

            //  alle Tabellen anlegen, damit Formeln kopiert werden koennen:
        pUndoDoc->AddUndoTab( 0, nTabCount-1, false, false );

            //  kopieren mit bColRowFlags=false (#54194#)
        pRefUndoDoc->CopyToDocument(0,0,0,MAXCOL,MAXROW,MAXTAB,IDF_FORMULA,false,pUndoDoc,NULL,false);
        delete pRefUndoDoc;

        SCTAB* pTabs      = new SCTAB[nSelCount];
        SCTAB* pScenarios = new SCTAB[nSelCount];
        SCTAB   nUndoPos  = 0;

        itr = aMark.begin(), itrEnd = aMark.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
        {
            SCTAB nCount = 0;
            for( SCTAB j=*itr+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                nCount ++;

            pScenarios[nUndoPos] = nCount;
            pTabs[nUndoPos] = *itr;
            nUndoPos ++;
        }

        if( !bDeletingMerge )
        {
            rDocShell.GetUndoManager()->LeaveListAction();
        }

        rDocShell.GetUndoManager()->AddUndoAction( new ScUndoDeleteCells(
            &rDocShell, ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab ),nUndoPos, pTabs, pScenarios,
            eCmd, pUndoDoc, pUndoData ) );
    }

    // #i8302 want to be able to insert into the middle of merged cells
    // the patch comes from maoyg

    while( !qDecreaseRange.empty() )
    {
        ScRange aRange = qDecreaseRange.back();

        long nDecreaseRowCount = 0;
        long nDecreaseColCount = 0;
        if( eCmd == DEL_CELLSUP || eCmd == DEL_DELROWS )
        {
            if( nStartRow >= aRange.aStart.Row() && nStartRow <= aRange.aEnd.Row() && nEndRow>= aRange.aStart.Row() && nEndRow <= aRange.aEnd.Row() )
                nDecreaseRowCount = nEndRow-nStartRow+1;
            else if( nStartRow >= aRange.aStart.Row() && nStartRow <= aRange.aEnd.Row() && nEndRow >= aRange.aStart.Row() && nEndRow >= aRange.aEnd.Row() )
                nDecreaseRowCount = aRange.aEnd.Row()-nStartRow+1;
            else if( nStartRow >= aRange.aStart.Row() && nStartRow >= aRange.aEnd.Row() && nEndRow>= aRange.aStart.Row() && nEndRow <= aRange.aEnd.Row() )
                nDecreaseRowCount = aRange.aEnd.Row()-nEndRow+1;
        }
        else if( eCmd == DEL_CELLSLEFT || eCmd == DEL_DELCOLS )
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
            case DEL_CELLSUP:
            case DEL_DELROWS:
                aRange.aEnd.SetRow(static_cast<SCsCOL>( aRange.aEnd.Row()-nDecreaseRowCount));
                break;
            case DEL_CELLSLEFT:
            case DEL_DELCOLS:
                aRange.aEnd.SetCol(static_cast<SCsCOL>( aRange.aEnd.Col()-nDecreaseColCount));
                break;
            default:
                break;
        }

        if( !pDoc->HasAttrib( aRange, HASATTR_OVERLAPPED | HASATTR_MERGED ) )
        {
            ScCellMergeOption aMergeOption(aRange);
            MergeCells( aMergeOption, false, true, true );
        }
        qDecreaseRange.pop_back();
    }

    if( bDeletingMerge )
        rDocShell.GetUndoManager()->LeaveListAction();

    if ( bNeedRefresh )
    {
        // #i51445# old merge flag attributes must be deleted also for single cells,
        // not only for whole columns/rows

        if ( eCmd==DEL_DELCOLS || eCmd==DEL_CELLSLEFT )
            nMergeTestEndCol = MAXCOL;
        if ( eCmd==DEL_DELROWS || eCmd==DEL_CELLSUP )
            nMergeTestEndRow = MAXROW;
        ScPatternAttr aPattern( pDoc->GetPool() );
        aPattern.GetItemSet().Put( ScMergeFlagAttr() );

        pDoc->ApplyPatternArea( nExtendStartCol, nExtendStartRow, nMergeTestEndCol, nMergeTestEndRow, aMark, aPattern );

        itr = aMark.begin(), itrEnd = aMark.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
        {
            SCTAB nScenarioCount = 0;

            for( SCTAB j = *itr+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                nScenarioCount ++;

            ScRange aMergedRange( nExtendStartCol, nExtendStartRow, *itr, nMergeTestEndCol, nMergeTestEndRow, *itr+nScenarioCount );
            pDoc->ExtendMerge( aMergedRange, true );
        }
    }

    itr = aMark.begin(), itrEnd = aMark.end();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
    {
        pDoc->SetDrawPageSize(*itr);

        if ( eCmd == DEL_DELCOLS || eCmd == DEL_DELROWS )
            pDoc->UpdatePageBreaks( *itr );

        rDocShell.UpdatePaintExt( nExtFlags, nPaintStartCol, nPaintStartRow, *itr, nPaintEndCol, nPaintEndRow, *itr );

        SCTAB nScenarioCount = 0;

        for( SCTAB j = *itr+1; j<nTabCount && pDoc->IsScenario(j); j++ )
            nScenarioCount ++;

        //  ganze Zeilen loeschen: nichts anpassen
        if ( eCmd == DEL_DELROWS || !AdjustRowHeight(ScRange( 0, nPaintStartRow, *itr, MAXCOL, nPaintEndRow, *itr+nScenarioCount )) )
            rDocShell.PostPaint( nPaintStartCol, nPaintStartRow, *itr, nPaintEndCol, nPaintEndRow, *itr+nScenarioCount, nPaintFlags,  nExtFlags );
        else
        {
            //  paint only what is not done by AdjustRowHeight
            if (nExtFlags & SC_PF_LINES)
                lcl_PaintAbove( rDocShell, ScRange( nPaintStartCol, nPaintStartRow, *itr, nPaintEndCol, nPaintEndRow, *itr+nScenarioCount) );
            if (nPaintFlags & PAINT_TOP)
                rDocShell.PostPaint( nPaintStartCol, nPaintStartRow, *itr, nPaintEndCol, nPaintEndRow, *itr+nScenarioCount, PAINT_TOP );
        }
    }
    aModificator.SetDocumentModified();

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );

    return true;
}

sal_Bool ScDocFunc::MoveBlock( const ScRange& rSource, const ScAddress& rDestPos,
                                sal_Bool bCut, sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi )
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

    //  zugehoerige Szenarien auch anpassen - nur wenn innerhalb einer Tabelle verschoben wird!
    sal_Bool bScenariosAdded = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    SCTAB nTabCount = pDoc->GetTableCount();
    if ( nDestTab == nStartTab && !pDoc->IsScenario(nEndTab) )
        while ( nEndTab+1 < nTabCount && pDoc->IsScenario(nEndTab+1) )
        {
            ++nEndTab;
            bScenariosAdded = sal_True;
        }

    SCTAB nSrcTabCount = nEndTab-nStartTab+1;
    SCTAB nDestEndTab = nDestTab+nSrcTabCount-1;
    SCTAB nTab;

    ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );

    ScMarkData aSourceMark;
    for (nTab=nStartTab; nTab<=nEndTab; nTab++)
        aSourceMark.SelectTable( nTab, sal_True );      // Source selektieren
    aSourceMark.SetMarkArea( rSource );

    ScDocShellRef aDragShellRef;
    if ( pDoc->HasOLEObjectsInArea( rSource ) )
    {
        aDragShellRef = new ScDocShell;     // DocShell needs a Ref immediately
        aDragShellRef->DoInitNew(NULL);
    }
    ScDrawLayer::SetGlobalDrawPersist(aDragShellRef);

    ScClipParam aClipParam(ScRange(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nStartTab), bCut);
    pDoc->CopyToClip(aClipParam, pClipDoc, &aSourceMark, false, bScenariosAdded, true);

    ScDrawLayer::SetGlobalDrawPersist(NULL);

    SCCOL nOldEndCol = nEndCol;
    SCROW nOldEndRow = nEndRow;
    sal_Bool bClipOver = false;
    for (nTab=nStartTab; nTab<=nEndTab; nTab++)
    {
        SCCOL nTmpEndCol = nOldEndCol;
        SCROW nTmpEndRow = nOldEndRow;
        if (pDoc->ExtendMerge( nStartCol, nStartRow, nTmpEndCol, nTmpEndRow, nTab ))
            bClipOver = sal_True;
        if ( nTmpEndCol > nEndCol ) nEndCol = nTmpEndCol;
        if ( nTmpEndRow > nEndRow ) nEndRow = nTmpEndRow;
    }

    SCCOL nDestEndCol = nDestCol + ( nOldEndCol-nStartCol );
    SCROW nDestEndRow = nDestRow + ( nOldEndRow-nStartRow );

    SCCOL nUndoEndCol = nDestCol + ( nEndCol-nStartCol );       // erweitert im Zielblock
    SCROW nUndoEndRow = nDestRow + ( nEndRow-nStartRow );

    sal_Bool bIncludeFiltered = bCut;
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
        delete pClipDoc;
        return false;
    }

    //  Test auf Zellschutz

    ScEditableTester aTester;
    for (nTab=nDestTab; nTab<=nDestEndTab; nTab++)
        aTester.TestBlock( pDoc, nTab, nDestCol,nDestRow, nUndoEndCol,nUndoEndRow );
    if (bCut)
        for (nTab=nStartTab; nTab<=nEndTab; nTab++)
            aTester.TestBlock( pDoc, nTab, nStartCol,nStartRow, nEndCol,nEndRow );

    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        delete pClipDoc;
        return false;
    }

    //  Test auf zusammengefasste - beim Verschieben erst nach dem Loeschen

    if (bClipOver && !bCut)
        if (pDoc->HasAttrib( nDestCol,nDestRow,nDestTab, nUndoEndCol,nUndoEndRow,nDestEndTab,
                                HASATTR_MERGED | HASATTR_OVERLAPPED ))
        {       // "Zusammenfassen nicht verschachteln !"
            if (!bApi)
                rDocShell.ErrorMessage(STR_MSSG_MOVEBLOCKTO_0);
            delete pClipDoc;
            return false;
        }

    //  Are there borders in the cells? (for painting)

    sal_uInt16 nSourceExt = 0;
    rDocShell.UpdatePaintExt( nSourceExt, nStartCol,nStartRow,nStartTab, nEndCol,nEndRow,nEndTab );
    sal_uInt16 nDestExt = 0;
    rDocShell.UpdatePaintExt( nDestExt, nDestCol,nDestRow,nDestTab, nDestEndCol,nDestEndRow,nDestEndTab );

    //
    //  ausfuehren
    //

    ScDocument* pUndoDoc = NULL;
    ScDocument* pRefUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if (bRecord)
    {
        sal_Bool bWholeCols = ( nStartRow == 0 && nEndRow == MAXROW );
        sal_Bool bWholeRows = ( nStartCol == 0 && nEndCol == MAXCOL );
        sal_uInt16 nUndoFlags = (IDF_ALL & ~IDF_OBJECTS) | IDF_NOCAPTIONS;

        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab, bWholeCols, bWholeRows );

        if (bCut)
        {
            pDoc->CopyToDocument( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                                    nUndoFlags, false, pUndoDoc );
            pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1, false, false );
        }

        if ( nDestTab != nStartTab )
            pUndoDoc->AddUndoTab( nDestTab, nDestEndTab, bWholeCols, bWholeRows );
        pDoc->CopyToDocument( nDestCol, nDestRow, nDestTab,
                                    nDestEndCol, nDestEndRow, nDestEndTab,
                                    nUndoFlags, false, pUndoDoc );

        pUndoData = new ScRefUndoData( pDoc );

        pDoc->BeginDrawUndo();
    }

    sal_Bool bSourceHeight = false;     // Hoehen angepasst?
    if (bCut)
    {
        ScMarkData aDelMark;    // only for tables
        for (nTab=nStartTab; nTab<=nEndTab; nTab++)
        {
            pDoc->DeleteAreaTab( nStartCol,nStartRow, nOldEndCol,nOldEndRow, nTab, IDF_ALL );
            aDelMark.SelectTable( nTab, sal_True );
        }
        pDoc->DeleteObjectsInArea( nStartCol,nStartRow, nOldEndCol,nOldEndRow, aDelMark );

        //  Test auf zusammengefasste

        if (bClipOver)
            if (pDoc->HasAttrib( nDestCol,nDestRow,nDestTab,
                                    nUndoEndCol,nUndoEndRow,nDestEndTab,
                                    HASATTR_MERGED | HASATTR_OVERLAPPED ))
            {
                pDoc->CopyFromClip( rSource, aSourceMark, IDF_ALL, pRefUndoDoc, pClipDoc );
                for (nTab=nStartTab; nTab<=nEndTab; nTab++)
                {
                    SCCOL nTmpEndCol = nEndCol;
                    SCROW nTmpEndRow = nEndRow;
                    pDoc->ExtendMerge( nStartCol, nStartRow, nTmpEndCol, nTmpEndRow, nTab, sal_True );
                }

                //  Fehlermeldung erst nach dem Wiederherstellen des Inhalts
                if (!bApi)      // "Zusammenfassen nicht verschachteln !"
                    rDocShell.ErrorMessage(STR_MSSG_MOVEBLOCKTO_0);

                delete pUndoDoc;
                delete pRefUndoDoc;
                delete pUndoData;
                delete pClipDoc;
                return false;
            }

        bSourceHeight = AdjustRowHeight( rSource, false );
    }

    ScRange aPasteDest( nDestCol, nDestRow, nDestTab, nDestEndCol, nDestEndRow, nDestEndTab );

    ScMarkData aDestMark;
    for (nTab=nDestTab; nTab<=nDestEndTab; nTab++)
        aDestMark.SelectTable( nTab, sal_True );        // Destination selektieren
    aDestMark.SetMarkArea( aPasteDest );

    /*  Do not copy cell notes and drawing objects here. While pasting, the
        function ScDocument::UpdateReference() is called which calls
        ScDrawLayer::MoveCells() which may move away inserted objects to wrong
        positions (e.g. if source and destination range overlaps). Cell notes
        and drawing objects are pasted below after doing all adjusting. */
    pDoc->CopyFromClip( aPasteDest, aDestMark, IDF_ALL & ~(IDF_NOTE | IDF_OBJECTS),
                        pRefUndoDoc, pClipDoc, sal_True, false, bIncludeFiltered );

    // skipped rows and merged cells don't mix
    if ( !bIncludeFiltered && pClipDoc->HasClipFilteredRows() )
        UnmergeCells( aPasteDest, false );

    VirtualDevice aVirtDev;
    sal_Bool bDestHeight = AdjustRowHeight(
                            ScRange( 0,nDestRow,nDestTab, MAXCOL,nDestEndRow,nDestEndTab ),
                            false );

    /*  Paste cell notes and drawing objects after adjusting formula references
        and row heights. There are no cell notes or drawing objects, if the
        clipdoc does not contain a drawing layer.
        #i102056# Passing IDF_NOTE only would overwrite cell contents with
        empty note cells, therefore the special modifier IDF_ADDNOTES is passed
        here too which changes the behaviour of ScColumn::CopyFromClip() to not
        touch existing cells. */
    if ( pClipDoc->GetDrawLayer() )
        pDoc->CopyFromClip( aPasteDest, aDestMark, IDF_NOTE | IDF_ADDNOTES | IDF_OBJECTS,
                            pRefUndoDoc, pClipDoc, sal_True, false, bIncludeFiltered );

    if (bRecord)
    {
        if (pRefUndoDoc)
        {
                //  alle Tabellen anlegen, damit Formeln kopiert werden koennen:
            pUndoDoc->AddUndoTab( 0, nTabCount-1, false, false );

            pRefUndoDoc->DeleteArea( nDestCol, nDestRow, nDestEndCol, nDestEndRow, aSourceMark, IDF_ALL );
            //  kopieren mit bColRowFlags=sal_False (#54194#)
            pRefUndoDoc->CopyToDocument( 0, 0, 0, MAXCOL, MAXROW, MAXTAB,
                                            IDF_FORMULA, false, pUndoDoc, NULL, false );
            delete pRefUndoDoc;
        }

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDragDrop( &rDocShell, ScRange(
                                    nStartCol, nStartRow, nStartTab,
                                    nOldEndCol, nOldEndRow, nEndTab ),
                                ScAddress( nDestCol, nDestRow, nDestTab ),
                                bCut, pUndoDoc, pUndoData, bScenariosAdded ) );
    }

    SCCOL nDestPaintEndCol = nDestEndCol;
    SCROW nDestPaintEndRow = nDestEndRow;
    for (nTab=nDestTab; nTab<=nDestEndTab; nTab++)
    {
        SCCOL nTmpEndCol = nDestEndCol;
        SCROW nTmpEndRow = nDestEndRow;
        pDoc->ExtendMerge( nDestCol, nDestRow, nTmpEndCol, nTmpEndRow, nTab, sal_True );
        if (nTmpEndCol > nDestPaintEndCol) nDestPaintEndCol = nTmpEndCol;
        if (nTmpEndRow > nDestPaintEndRow) nDestPaintEndRow = nTmpEndRow;
    }

    if (bCut)
        for (nTab=nStartTab; nTab<=nEndTab; nTab++)
            pDoc->RefreshAutoFilter( nStartCol, nStartRow, nEndCol, nEndRow, nTab );

    if (bPaint)
    {
            //  Zielbereich:

        SCCOL nPaintStartX = nDestCol;
        SCROW nPaintStartY = nDestRow;
        SCCOL nPaintEndX = nDestPaintEndCol;
        SCROW nPaintEndY = nDestPaintEndRow;
        sal_uInt16 nFlags = PAINT_GRID;

        if ( nStartRow==0 && nEndRow==MAXROW )      // Breiten mitkopiert?
        {
            nPaintEndX = MAXCOL;
            nPaintStartY = 0;
            nPaintEndY = MAXROW;
            nFlags |= PAINT_TOP;
        }
        if ( bDestHeight || ( nStartCol == 0 && nEndCol == MAXCOL ) )
        {
            nPaintEndY = MAXROW;
            nPaintStartX = 0;
            nPaintEndX = MAXCOL;
            nFlags |= PAINT_LEFT;
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
                //  Quellbereich:

            nPaintStartX = nStartCol;
            nPaintStartY = nStartRow;
            nPaintEndX = nEndCol;
            nPaintEndY = nEndRow;
            nFlags = PAINT_GRID;

            if ( bSourceHeight )
            {
                nPaintEndY = MAXROW;
                nPaintStartX = 0;
                nPaintEndX = MAXCOL;
                nFlags |= PAINT_LEFT;
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

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );

    delete pClipDoc;
    return sal_True;
}

//------------------------------------------------------------------------
uno::Reference< uno::XInterface > GetDocModuleObject( SfxObjectShell& rDocSh, String& sCodeName )
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

static script::ModuleInfo lcl_InitModuleInfo( SfxObjectShell& rDocSh, String& sModule )
{
    script::ModuleInfo sModuleInfo;
    sModuleInfo.ModuleType = script::ModuleType::DOCUMENT;
    sModuleInfo.ModuleObject = GetDocModuleObject( rDocSh, sModule );
    return sModuleInfo;
}

void VBA_InsertModule( ScDocument& rDoc, SCTAB nTab, const OUString& sModuleName, const OUString& sSource )
{
    SfxObjectShell& rDocSh = *rDoc.GetDocumentShell();
    uno::Reference< script::XLibraryContainer > xLibContainer = rDocSh.GetBasicContainer();
    OSL_ENSURE( xLibContainer.is(), "No BasicContainer!" );

    uno::Reference< container::XNameContainer > xLib;
    if( xLibContainer.is() )
    {
        String aLibName( "Standard" );
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
        sal_Int32 nNum = 0;
        String genModuleName;
        if ( !sModuleName.isEmpty() )
            genModuleName = sModuleName;
        else
        {
             genModuleName = OUString( "Sheet1" );
             nNum = 1;
        }
        while( xLib->hasByName( genModuleName ) )
            genModuleName = OUString( "Sheet") + OUString::number( ++nNum );

        uno::Any aSourceAny;
        OUString sTmpSource = sSource;
        if ( sTmpSource.isEmpty() )
            sTmpSource = OUString( "Rem Attribute VBA_ModuleType=VBADocumentModule\nOption VBASupport 1\n" );
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
        String aLibName( "Standard" );
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


sal_Bool ScDocFunc::InsertTable( SCTAB nTab, const String& rName, sal_Bool bRecord, sal_Bool bApi )
{
    sal_Bool bSuccess = false;
    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();


    // Strange loop, also basic is loaded too early ( InsertTable )
    // is called via the xml import for sheets in described in ODF
    sal_Bool bInsertDocModule = false;

    if(  !rDocShell.GetDocument()->IsImportingXML() )
    {
        bInsertDocModule = pDoc ? pDoc->IsInVBAMode() : false;
    }
    if ( bInsertDocModule || ( bRecord && !pDoc->IsUndoEnabled() ) )
        bRecord = false;

    if (bRecord)
        pDoc->BeginDrawUndo();                          //  InsertTab erzeugt ein SdrUndoNewPage

    SCTAB nTabCount = pDoc->GetTableCount();
    sal_Bool bAppend = ( nTab >= nTabCount );
    if ( bAppend )
        nTab = nTabCount;       // wichtig fuer Undo

    if (pDoc->InsertTab( nTab, rName ))
    {
        if (bRecord)
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoInsertTab( &rDocShell, nTab, bAppend, rName));
        //  Views updaten:
        // Only insert vba modules if vba mode ( and not currently importing XML )
        if( bInsertDocModule )
        {
            OUString sSource, sCodeName;
            VBA_InsertModule( *pDoc, nTab, sCodeName, sSource );
        }
        rDocShell.Broadcast( ScTablesHint( SC_TAB_INSERTED, nTab ) );

        rDocShell.PostPaintExtras();
        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        bSuccess = sal_True;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(STR_TABINSERT_ERROR);

    return bSuccess;
}

sal_Bool ScDocFunc::DeleteTable( SCTAB nTab, sal_Bool bRecord, sal_Bool /* bApi */ )
{
    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bSuccess = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    sal_Bool bVbaEnabled = pDoc ? pDoc->IsInVBAMode() : false;
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    if ( bVbaEnabled )
        bRecord = false;
    sal_Bool bWasLinked = pDoc->IsLinked(nTab);
    ScDocument* pUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        SCTAB nCount = pDoc->GetTableCount();

        pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, sal_True );     // nur nTab mit Flags
        pUndoDoc->AddUndoTab( 0, nCount-1 );                    // alle Tabs fuer Referenzen

        pDoc->CopyToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, IDF_ALL,false, pUndoDoc );
        OUString aOldName;
        pDoc->GetName( nTab, aOldName );
        pUndoDoc->RenameTab( nTab, aOldName, false );
        if (bWasLinked)
            pUndoDoc->SetLink( nTab, pDoc->GetLinkMode(nTab), pDoc->GetLinkDoc(nTab),
                                pDoc->GetLinkFlt(nTab), pDoc->GetLinkOpt(nTab),
                                pDoc->GetLinkTab(nTab),
                                pDoc->GetLinkRefreshDelay(nTab) );

        if ( pDoc->IsScenario(nTab) )
        {
            pUndoDoc->SetScenario( nTab, sal_True );
            OUString aComment;
            Color  aColor;
            sal_uInt16 nScenFlags;
            pDoc->GetScenarioData( nTab, aComment, aColor, nScenFlags );
            pUndoDoc->SetScenarioData( nTab, aComment, aColor, nScenFlags );
            sal_Bool bActive = pDoc->IsActiveScenario( nTab );
            pUndoDoc->SetActiveScenario( nTab, bActive );
        }
        pUndoDoc->SetVisible( nTab, pDoc->IsVisible( nTab ) );
        pUndoDoc->SetTabBgColor( nTab, pDoc->GetTabBgColor(nTab) );
        pUndoDoc->SetSheetEvents( nTab, pDoc->GetSheetEvents( nTab ) );

        //  Drawing-Layer muss sein Undo selbst in der Hand behalten !!!
        pDoc->BeginDrawUndo();                          //  DeleteTab erzeugt ein SdrUndoDelPage

        pUndoData = new ScRefUndoData( pDoc );
    }

    if (pDoc->DeleteTab(nTab))
    {
        if (bRecord)
        {
            vector<SCTAB> theTabs;
            theTabs.push_back(nTab);
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDeleteTab( &rDocShell, theTabs, pUndoDoc, pUndoData ));
        }
        //  Views updaten:
        if( bVbaEnabled )
        {
            OUString sCodeName;
            if( pDoc->GetCodeName( nTab, sCodeName ) )
            {
                VBA_DeleteModule( rDocShell, sCodeName );
            }
        }
        rDocShell.Broadcast( ScTablesHint( SC_TAB_DELETED, nTab ) );

        if (bWasLinked)
        {
            rDocShell.UpdateLinks();                // Link-Manager updaten
            SfxBindings* pBindings = rDocShell.GetViewBindings();
            if (pBindings)
                pBindings->Invalidate(SID_LINKS);
        }

        rDocShell.PostPaintExtras();
        aModificator.SetDocumentModified();

        SfxApplication* pSfxApp = SFX_APP();                                // Navigator
        pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
        pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );

        bSuccess = sal_True;
    }
    else
    {
        delete pUndoDoc;
        delete pUndoData;
    }
    return bSuccess;
}

sal_Bool ScDocFunc::SetTableVisible( SCTAB nTab, bool bVisible, sal_Bool bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    bool bUndo(pDoc->IsUndoEnabled());
    if ( pDoc->IsVisible( nTab ) == bVisible )
        return sal_True;                                // nichts zu tun - ok

    if ( !pDoc->IsDocEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return false;
    }

    ScDocShellModificator aModificator( rDocShell );

    if ( !bVisible && !pDoc->IsImportingXML() )     // #i57869# allow hiding in any order for loading
    {
        //  nicht alle Tabellen ausblenden

        sal_uInt16 nVisCount = 0;
        SCTAB nCount = pDoc->GetTableCount();
        for (SCTAB i=0; i<nCount && nVisCount<2; i++)
            if (pDoc->IsVisible(i))
                ++nVisCount;

        if (nVisCount <= 1)
        {
            if (!bApi)
                rDocShell.ErrorMessage(STR_PROTECTIONERR);  //! eigene Meldung?
            return false;
        }
    }

    pDoc->SetVisible( nTab, bVisible );
    if (bUndo)
    {
        std::vector<SCTAB> undoTabs;
        undoTabs.push_back(nTab);
        rDocShell.GetUndoManager()->AddUndoAction( new ScUndoShowHideTab( &rDocShell, undoTabs, bVisible ) );
    }

    //  Views updaten:
    if (!bVisible)
        rDocShell.Broadcast( ScTablesHint( SC_TAB_HIDDEN, nTab ) );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    rDocShell.PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_EXTRAS);
    aModificator.SetDocumentModified();

    return sal_True;
}

sal_Bool ScDocFunc::SetLayoutRTL( SCTAB nTab, bool bRTL, sal_Bool /* bApi */ )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    bool bUndo(pDoc->IsUndoEnabled());
    if ( pDoc->IsLayoutRTL( nTab ) == bRTL )
        return sal_True;                                // nothing to do - ok

    //! protection (sheet or document?)

    ScDocShellModificator aModificator( rDocShell );

    pDoc->SetLayoutRTL( nTab, bRTL );

    if (bUndo)
    {
        rDocShell.GetUndoManager()->AddUndoAction( new ScUndoLayoutRTL( &rDocShell, nTab, bRTL ) );
    }

    rDocShell.PostPaint( 0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_ALL );
    aModificator.SetDocumentModified();

    SfxBindings* pBindings = rDocShell.GetViewBindings();
    if (pBindings)
    {
        pBindings->Invalidate( FID_TAB_RTL );
        pBindings->Invalidate( SID_ATTR_SIZE );
    }

    return sal_True;
}

sal_Bool ScDocFunc::RenameTable( SCTAB nTab, const String& rName, sal_Bool bRecord, sal_Bool bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    if ( !pDoc->IsDocEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return false;
    }

    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bSuccess = false;
    OUString sOldName;
    pDoc->GetName(nTab, sOldName);
    if (pDoc->RenameTab( nTab, rName ))
    {
        if (bRecord)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                            new ScUndoRenameTab( &rDocShell, nTab, sOldName, rName));
        }
        rDocShell.PostPaintExtras();
        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

        bSuccess = sal_True;
    }
    return bSuccess;
}

bool ScDocFunc::SetTabBgColor( SCTAB nTab, const Color& rColor, bool bRecord, bool bApi )
{

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    if ( !pDoc->IsDocEditable() || pDoc->IsTabProtected(nTab) )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR); //TODO Check to see what this string is...
        return false;
    }

    Color aOldTabBgColor;
    aOldTabBgColor = pDoc->GetTabBgColor(nTab);

    bool bSuccess = false;
    pDoc->SetTabBgColor(nTab, rColor);
    if ( pDoc->GetTabBgColor(nTab) == rColor)
        bSuccess = true;
    if (bSuccess)
    {
        if (bRecord)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoTabColor( &rDocShell, nTab, aOldTabBgColor, rColor));
        }
        rDocShell.PostPaintExtras();
        ScDocShellModificator aModificator( rDocShell );
        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

        bSuccess = true;
    }
    return bSuccess;
}

bool ScDocFunc::SetTabBgColor(
    ScUndoTabColorInfo::List& rUndoTabColorList, bool bRecord, bool bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    if ( !pDoc->IsDocEditable() )
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
        if ( !pDoc->IsTabProtected(nTab) )
        {
            aNewTabBgColor = rInfo.maNewTabBgColor;
            rInfo.maOldTabBgColor = pDoc->GetTabBgColor(nTab);
            pDoc->SetTabBgColor(nTab, aNewTabBgColor);
            if ( pDoc->GetTabBgColor(nTab) != aNewTabBgColor)
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
                new ScUndoTabColor( &rDocShell, rUndoTabColorList));
        }
        rDocShell.PostPaintExtras();
        ScDocShellModificator aModificator( rDocShell );
        aModificator.SetDocumentModified();
    }
    return bSuccess;
}

//------------------------------------------------------------------------

//! SetWidthOrHeight - noch doppelt zu ViewFunc !!!!!!
//! Probleme:
//! - Optimale Hoehe fuer Edit-Zellen ist unterschiedlich zwischen Drucker und Bildschirm
//! - Optimale Breite braucht Selektion, um evtl. nur selektierte Zellen zu beruecksichtigen

static sal_uInt16 lcl_GetOptimalColWidth( ScDocShell& rDocShell, SCCOL nCol, SCTAB nTab, sal_Bool bFormula )
{
    ScSizeDeviceProvider aProv(&rDocShell);
    OutputDevice* pDev = aProv.GetDevice();         // has pixel MapMode
    double nPPTX = aProv.GetPPTX();
    double nPPTY = aProv.GetPPTY();

    ScDocument* pDoc = rDocShell.GetDocument();
    Fraction aOne(1,1);
    sal_uInt16 nTwips = pDoc->GetOptimalColWidth( nCol, nTab, pDev, nPPTX, nPPTY, aOne, aOne,
                                                    bFormula, NULL );

    return nTwips;
}

sal_Bool ScDocFunc::SetWidthOrHeight( sal_Bool bWidth, SCCOLROW nRangeCnt, SCCOLROW* pRanges, SCTAB nTab,
                                        ScSizeMode eMode, sal_uInt16 nSizeTwips,
                                        sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    if (!nRangeCnt)
        return sal_True;

    ScDocument* pDoc = rDocShell.GetDocument();
    if ( bRecord && !pDoc->IsUndoEnabled() )
        bRecord = false;

    // import into read-only document is possible
    if ( !pDoc->IsChangeReadOnlyEnabled() && !rDocShell.IsEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);      //! eigene Meldung?
        return false;
    }

    sal_Bool bSuccess = false;
    SCCOLROW nStart = pRanges[0];
    SCCOLROW nEnd = pRanges[2*nRangeCnt-1];

    sal_Bool bFormula = false;
    if ( eMode == SC_SIZE_OPTIMAL )
    {
        //! Option "Formeln anzeigen" - woher nehmen?
    }

    ScDocument*     pUndoDoc = NULL;
    ScOutlineTable* pUndoTab = NULL;
    SCCOLROW*       pUndoRanges = NULL;

    if ( bRecord )
    {
        pDoc->BeginDrawUndo();                          // Drawing Updates

        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bWidth)
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, false );
            pDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab, static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, false, pUndoDoc );
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, false, sal_True );
            pDoc->CopyToDocument( 0, static_cast<SCROW>(nStart), nTab, MAXCOL, static_cast<SCROW>(nEnd), nTab, IDF_NONE, false, pUndoDoc );
        }

        pUndoRanges = new SCCOLROW[ 2*nRangeCnt ];
        memcpy( pUndoRanges, pRanges, 2*nRangeCnt*sizeof(SCCOLROW) );

        ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
        if (pTable)
            pUndoTab = new ScOutlineTable( *pTable );
    }

    sal_Bool bShow = nSizeTwips > 0 || eMode != SC_SIZE_DIRECT;
    sal_Bool bOutline = false;

    pDoc->InitializeNoteCaptions(nTab);
    for (SCCOLROW nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
    {
        SCCOLROW nStartNo = *(pRanges++);
        SCCOLROW nEndNo = *(pRanges++);

        if ( !bWidth )                      // Hoehen immer blockweise
        {
            if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
            {
                sal_Bool bAll = ( eMode==SC_SIZE_OPTIMAL );
                if (!bAll)
                {
                    //  fuer alle eingeblendeten CR_MANUALSIZE loeschen,
                    //  dann SetOptimalHeight mit bShrink = FALSE
                    for (SCROW nRow=nStartNo; nRow<=nEndNo; nRow++)
                    {
                        sal_uInt8 nOld = pDoc->GetRowFlags(nRow,nTab);
                        SCROW nLastRow = -1;
                        bool bHidden = pDoc->RowHidden(nRow, nTab, NULL, &nLastRow);
                        if ( !bHidden && ( nOld & CR_MANUALSIZE ) )
                            pDoc->SetRowFlags( nRow, nTab, nOld & ~CR_MANUALSIZE );
                    }
                }

                ScSizeDeviceProvider aProv( &rDocShell );
                Fraction aOne(1,1);
                pDoc->SetOptimalHeight( nStartNo, nEndNo, nTab, 0, aProv.GetDevice(),
                                        aProv.GetPPTX(), aProv.GetPPTY(), aOne, aOne, bAll );

                if (bAll)
                    pDoc->ShowRows( nStartNo, nEndNo, nTab, sal_True );

                //  Manual-Flag wird bei bAll=sal_True schon in SetOptimalHeight gesetzt
                //  (an bei Extra-Height, sonst aus).
            }
            else if ( eMode==SC_SIZE_DIRECT || eMode==SC_SIZE_ORIGINAL )
            {
                if (nSizeTwips)
                {
                    pDoc->SetRowHeightRange( nStartNo, nEndNo, nTab, nSizeTwips );
                    pDoc->SetManualHeight( nStartNo, nEndNo, nTab, sal_True );          // height was set manually
                }
                if ( eMode != SC_SIZE_ORIGINAL )
                    pDoc->ShowRows( nStartNo, nEndNo, nTab, nSizeTwips != 0 );
            }
            else if ( eMode==SC_SIZE_SHOW )
            {
                pDoc->ShowRows( nStartNo, nEndNo, nTab, sal_True );
            }
        }
        else                                // Spaltenbreiten
        {
            for (SCCOL nCol=static_cast<SCCOL>(nStartNo); nCol<=static_cast<SCCOL>(nEndNo); nCol++)
            {
                if ( eMode != SC_SIZE_VISOPT || !pDoc->ColHidden(nCol, nTab) )
                {
                    sal_uInt16 nThisSize = nSizeTwips;

                    if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
                        nThisSize = nSizeTwips +
                                    lcl_GetOptimalColWidth( rDocShell, nCol, nTab, bFormula );
                    if ( nThisSize )
                        pDoc->SetColWidth( nCol, nTab, nThisSize );

                    if ( eMode != SC_SIZE_ORIGINAL )
                        pDoc->ShowCol( nCol, nTab, bShow );
                }
            }
        }

                            //  adjust outlines

        if ( eMode != SC_SIZE_ORIGINAL )
        {
            if (bWidth)
                bOutline = bOutline || pDoc->UpdateOutlineCol(
                        static_cast<SCCOL>(nStartNo),
                        static_cast<SCCOL>(nEndNo), nTab, bShow );
            else
                bOutline = bOutline || pDoc->UpdateOutlineRow(
                        static_cast<SCROW>(nStartNo),
                        static_cast<SCROW>(nEndNo), nTab, bShow );
        }
    }
    pDoc->SetDrawPageSize(nTab);

    if (!bOutline)
        DELETEZ(pUndoTab);

    if (bRecord)
    {
        ScMarkData aMark;
        aMark.SelectOneTable( nTab );
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoWidthOrHeight( &rDocShell, aMark,
                                     nStart, nTab, nEnd, nTab,
                                     pUndoDoc, nRangeCnt, pUndoRanges,
                                     pUndoTab, eMode, nSizeTwips, bWidth ) );
    }

    pDoc->UpdatePageBreaks( nTab );

    rDocShell.PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_ALL);
    aModificator.SetDocumentModified();

    return bSuccess;
}


sal_Bool ScDocFunc::InsertPageBreak( sal_Bool bColumn, const ScAddress& rPos,
                                sal_Bool bRecord, sal_Bool bSetModified, sal_Bool /* bApi */ )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    SCTAB nTab = rPos.Tab();
    SfxBindings* pBindings = rDocShell.GetViewBindings();

    SCCOLROW nPos = bColumn ? static_cast<SCCOLROW>(rPos.Col()) :
        static_cast<SCCOLROW>(rPos.Row());
    if (nPos == 0)
        return false;                   // erste Spalte / Zeile

    ScBreakType nBreak = bColumn ?
        pDoc->HasColBreak(static_cast<SCCOL>(nPos), nTab) :
        pDoc->HasRowBreak(static_cast<SCROW>(nPos), nTab);
    if (nBreak & BREAK_MANUAL)
        return true;

    if (bRecord)
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoPageBreak( &rDocShell, rPos.Col(), rPos.Row(), nTab, bColumn, sal_True ) );

    if (bColumn)
        pDoc->SetColBreak(static_cast<SCCOL>(nPos), nTab, false, true);
    else
        pDoc->SetRowBreak(static_cast<SCROW>(nPos), nTab, false, true);

    pDoc->InvalidatePageBreaks(nTab);
    pDoc->UpdatePageBreaks( nTab );

    if (pDoc->IsStreamValid(nTab))
        pDoc->SetStreamValid(nTab, false);

    if (bColumn)
    {
        rDocShell.PostPaint( static_cast<SCCOL>(nPos)-1, 0, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID );
        if (pBindings)
        {
            pBindings->Invalidate( FID_INS_COLBRK );
            pBindings->Invalidate( FID_DEL_COLBRK );
        }
    }
    else
    {
        rDocShell.PostPaint( 0, static_cast<SCROW>(nPos)-1, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID );
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

    return sal_True;
}

sal_Bool ScDocFunc::RemovePageBreak( sal_Bool bColumn, const ScAddress& rPos,
                                sal_Bool bRecord, sal_Bool bSetModified, sal_Bool /* bApi */ )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    SCTAB nTab = rPos.Tab();
    SfxBindings* pBindings = rDocShell.GetViewBindings();

    SCCOLROW nPos = bColumn ? static_cast<SCCOLROW>(rPos.Col()) :
        static_cast<SCCOLROW>(rPos.Row());

    ScBreakType nBreak;
    if (bColumn)
        nBreak = pDoc->HasColBreak(static_cast<SCCOL>(nPos), nTab);
    else
        nBreak = pDoc->HasRowBreak(static_cast<SCROW>(nPos), nTab);
    if ((nBreak & BREAK_MANUAL) == 0)
        // There is no manual break.
        return false;

    if (bRecord)
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoPageBreak( &rDocShell, rPos.Col(), rPos.Row(), nTab, bColumn, false ) );

    if (bColumn)
        pDoc->RemoveColBreak(static_cast<SCCOL>(nPos), nTab, false, true);
    else
        pDoc->RemoveRowBreak(static_cast<SCROW>(nPos), nTab, false, true);

    pDoc->UpdatePageBreaks( nTab );

    if (pDoc->IsStreamValid(nTab))
        pDoc->SetStreamValid(nTab, false);

    if (bColumn)
    {
        rDocShell.PostPaint( static_cast<SCCOL>(nPos)-1, 0, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID );
        if (pBindings)
        {
            pBindings->Invalidate( FID_INS_COLBRK );
            pBindings->Invalidate( FID_DEL_COLBRK );
        }
    }
    else
    {
        rDocShell.PostPaint( 0, nPos-1, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID );
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

    return sal_True;
}

//------------------------------------------------------------------------

void ScDocFunc::ProtectSheet( SCTAB nTab, const ScTableProtection& rProtect )
{
    ScDocument* pDoc = rDocShell.GetDocument();

    pDoc->SetTabProtection(nTab, &rProtect);
    if (pDoc->IsUndoEnabled())
    {
        ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
        OSL_ENSURE(pProtect, "ScDocFunc::Unprotect: ScTableProtection pointer is NULL!");
        if (pProtect)
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            ::std::auto_ptr<ScTableProtection> p(new ScTableProtection(*pProtect));
            SAL_WNODEPRECATED_DECLARATIONS_POP
            p->setProtected(true); // just in case ...
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoTabProtect(&rDocShell, nTab, p) );

            // ownership of auto_ptr now transferred to ScUndoTabProtect.
        }
    }

    rDocShell.PostPaintGridAll();
    ScDocShellModificator aModificator(rDocShell);
    aModificator.SetDocumentModified();
}

sal_Bool ScDocFunc::Protect( SCTAB nTab, const String& rPassword, sal_Bool /*bApi*/ )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if (nTab == TABLEID_DOC)
    {
        // document protection
        ScDocProtection aProtection;
        aProtection.setProtected(true);
        aProtection.setPassword(rPassword);
        pDoc->SetDocProtection(&aProtection);
        if (pDoc->IsUndoEnabled())
        {
            ScDocProtection* pProtect = pDoc->GetDocProtection();
            OSL_ENSURE(pProtect, "ScDocFunc::Unprotect: ScDocProtection pointer is NULL!");
            if (pProtect)
            {
                SAL_WNODEPRECATED_DECLARATIONS_PUSH
                ::std::auto_ptr<ScDocProtection> p(new ScDocProtection(*pProtect));
                SAL_WNODEPRECATED_DECLARATIONS_POP
                p->setProtected(true); // just in case ...
                rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoDocProtect(&rDocShell, p) );
                // ownership of auto_ptr is transferred to ScUndoDocProtect.
            }
        }
    }
    else
    {
        // sheet protection

        ScTableProtection aProtection;
        aProtection.setProtected(true);
        aProtection.setPassword(rPassword);
        pDoc->SetTabProtection(nTab, &aProtection);
        if (pDoc->IsUndoEnabled())
        {
            ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
            OSL_ENSURE(pProtect, "ScDocFunc::Unprotect: ScTableProtection pointer is NULL!");
            if (pProtect)
            {
                SAL_WNODEPRECATED_DECLARATIONS_PUSH
                ::std::auto_ptr<ScTableProtection> p(new ScTableProtection(*pProtect));
                SAL_WNODEPRECATED_DECLARATIONS_POP
                p->setProtected(true); // just in case ...
                rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoTabProtect(&rDocShell, nTab, p) );
                // ownership of auto_ptr now transferred to ScUndoTabProtect.
            }
        }
    }

    rDocShell.PostPaintGridAll();
    ScDocShellModificator aModificator( rDocShell );
    aModificator.SetDocumentModified();

    return true;
}

sal_Bool ScDocFunc::Unprotect( SCTAB nTab, const String& rPassword, sal_Bool bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();

    if (nTab == TABLEID_DOC)
    {
        // document protection

        ScDocProtection* pDocProtect = pDoc->GetDocProtection();
        if (!pDocProtect || !pDocProtect->isProtected())
            // already unprotected (should not happen)!
            return true;

        // save the protection state before unprotect (for undo).
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<ScDocProtection> pProtectCopy(new ScDocProtection(*pDocProtect));
        SAL_WNODEPRECATED_DECLARATIONS_POP

        if (!pDocProtect->verifyPassword(rPassword))
        {
            if (!bApi)
            {
                InfoBox aBox( rDocShell.GetActiveDialogParent(), String( ScResId( SCSTR_WRONGPASSWORD ) ) );
                aBox.Execute();
            }
            return false;
        }

        pDoc->SetDocProtection(NULL);
        if (pDoc->IsUndoEnabled())
        {
            pProtectCopy->setProtected(false);
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoDocProtect(&rDocShell, pProtectCopy) );
            // ownership of auto_ptr now transferred to ScUndoDocProtect.
        }
    }
    else
    {
        // sheet protection

        ScTableProtection* pTabProtect = pDoc->GetTabProtection(nTab);
        if (!pTabProtect || !pTabProtect->isProtected())
            // already unprotected (should not happen)!
            return true;

        // save the protection state before unprotect (for undo).
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<ScTableProtection> pProtectCopy(new ScTableProtection(*pTabProtect));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if (!pTabProtect->verifyPassword(rPassword))
        {
            if (!bApi)
            {
                InfoBox aBox( rDocShell.GetActiveDialogParent(), String( ScResId( SCSTR_WRONGPASSWORD ) ) );
                aBox.Execute();
            }
            return false;
        }

        pDoc->SetTabProtection(nTab, NULL);
        if (pDoc->IsUndoEnabled())
        {
            pProtectCopy->setProtected(false);
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoTabProtect(&rDocShell, nTab, pProtectCopy) );
            // ownership of auto_ptr now transferred to ScUndoTabProtect.
        }
    }

    rDocShell.PostPaintGridAll();
    ScDocShellModificator aModificator( rDocShell );
    aModificator.SetDocumentModified();

    return true;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::ClearItems( const ScMarkData& rMark, const sal_uInt16* pWhich, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    bool bUndo (pDoc->IsUndoEnabled());
    ScEditableTester aTester( pDoc, rMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
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

        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab );
        pDoc->CopyToDocument( aMarkRange, IDF_ATTRIB, sal_True, pUndoDoc, (ScMarkData*)&aMultiMark );

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoClearItems( &rDocShell, aMultiMark, pUndoDoc, pWhich ) );
    }

    pDoc->ClearSelectionItems( pWhich, aMultiMark );

    rDocShell.PostPaint( aMarkRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );
    aModificator.SetDocumentModified();

    //! Bindings-Invalidate etc.?

    return sal_True;
}

sal_Bool ScDocFunc::ChangeIndent( const ScMarkData& rMark, sal_Bool bIncrement, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    bool bUndo(pDoc->IsUndoEnabled());
    ScEditableTester aTester( pDoc, rMark );
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
        SCTAB nTabCount = pDoc->GetTableCount();

        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
            if (*itr != nStartTab)
                pUndoDoc->AddUndoTab( *itr, *itr );

        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, sal_True, pUndoDoc, (ScMarkData*)&rMark );

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoIndent( &rDocShell, rMark, pUndoDoc, bIncrement ) );
    }

    pDoc->ChangeSelectionIndent( bIncrement, rMark );

    rDocShell.PostPaint( aMarkRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );
    aModificator.SetDocumentModified();

    SfxBindings* pBindings = rDocShell.GetViewBindings();
    if (pBindings)
    {
        pBindings->Invalidate( SID_ALIGNLEFT );         // ChangeIndent setzt auf links
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

    return sal_True;
}

bool ScDocFunc::AutoFormat( const ScRange& rRange, const ScMarkData* pTabMark,
                            sal_uInt16 nFormatNo, bool bRecord, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bSuccess = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, sal_True );
    }

    ScAutoFormat* pAutoFormat = ScGlobal::GetOrCreateAutoFormat();
    ScEditableTester aTester( pDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
    if ( nFormatNo < pAutoFormat->size() && aTester.IsEditable() )
    {
        WaitObject aWait( rDocShell.GetActiveDialogParent() );

        bool bSize = pAutoFormat->findByIndex(nFormatNo)->GetIncludeWidthHeight();

        SCTAB nTabCount = pDoc->GetTableCount();
        ScDocument* pUndoDoc = NULL;
        if ( bRecord )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab, bSize, bSize );
            ScMarkData::iterator itr = aMark.begin(), itrEnd = aMark.end();
            for (; itr != itrEnd && *itr < nTabCount; ++itr)
                if (*itr != nStartTab)
                    pUndoDoc->AddUndoTab( *itr, *itr, bSize, bSize );

            ScRange aCopyRange = rRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aStart.SetTab(nTabCount-1);
            pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, false, pUndoDoc, &aMark );
            if (bSize)
            {
                pDoc->CopyToDocument( nStartCol,0,0, nEndCol,MAXROW,nTabCount-1,
                                                            IDF_NONE, false, pUndoDoc, &aMark );
                pDoc->CopyToDocument( 0,nStartRow,0, MAXCOL,nEndRow,nTabCount-1,
                                                            IDF_NONE, false, pUndoDoc, &aMark );
            }
            pDoc->BeginDrawUndo();
        }

        pDoc->AutoFormat( nStartCol, nStartRow, nEndCol, nEndRow, nFormatNo, aMark );

        if (bSize)
        {
            SCCOLROW nCols[2] = { nStartCol, nEndCol };
            SCCOLROW nRows[2] = { nStartRow, nEndRow };

            ScMarkData::iterator itr = aMark.begin(), itrEnd = aMark.end();
            for (; itr != itrEnd && *itr < nTabCount; ++itr)
            {
                SetWidthOrHeight( sal_True, 1,nCols, *itr, SC_SIZE_VISOPT, STD_EXTRA_WIDTH, false, sal_True);
                SetWidthOrHeight( false,1,nRows, *itr, SC_SIZE_VISOPT, 0, false, false);
                rDocShell.PostPaint( 0,0,*itr, MAXCOL,MAXROW,*itr,
                                PAINT_GRID | PAINT_LEFT | PAINT_TOP );
            }
        }
        else
        {
            ScMarkData::iterator itr = aMark.begin(), itrEnd = aMark.end();
            for (; itr != itrEnd && *itr < nTabCount; ++itr)
            {
                sal_Bool bAdj = AdjustRowHeight( ScRange(nStartCol, nStartRow, *itr,
                                                    nEndCol, nEndRow, *itr), false );
                if (bAdj)
                    rDocShell.PostPaint( 0,nStartRow,*itr, MAXCOL,MAXROW,*itr,
                                        PAINT_GRID | PAINT_LEFT );
                else
                    rDocShell.PostPaint( nStartCol, nStartRow, *itr,
                                        nEndCol, nEndRow, *itr, PAINT_GRID );
            }
        }

        if ( bRecord )      // Draw-Undo erst jetzt verfuegbar
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoAutoFormat( &rDocShell, rRange, pUndoDoc, aMark, bSize, nFormatNo ) );
        }

        aModificator.SetDocumentModified();
    }
    else if (!bApi)
        rDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::EnterMatrix( const ScRange& rRange, const ScMarkData* pTabMark,
        const ScTokenArray* pTokenArray, const String& rString, sal_Bool bApi, sal_Bool bEnglish,
        const String& rFormulaNmsp, const formula::FormulaGrammar::Grammar eGrammar )
{
    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bSuccess = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    bool bUndo(pDoc->IsUndoEnabled());

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, sal_True );
    }

    ScEditableTester aTester( pDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
    if ( aTester.IsEditable() )
    {
        WaitObject aWait( rDocShell.GetActiveDialogParent() );

        ScDocument* pUndoDoc = NULL;

        if (bUndo)
        {
            //! auch bei Undo selektierte Tabellen beruecksichtigen
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab );
            pDoc->CopyToDocument( rRange, IDF_ALL & ~IDF_NOTE, false, pUndoDoc );
        }

        // use TokenArray if given, string (and flags) otherwise
        if ( pTokenArray )
        {
            pDoc->InsertMatrixFormula( nStartCol, nStartRow, nEndCol, nEndRow,
                    aMark, EMPTY_STRING, pTokenArray, eGrammar);
        }
        else if ( pDoc->IsImportingXML() )
        {
            ScTokenArray* pCode = lcl_ScDocFunc_CreateTokenArrayXML( rString, rFormulaNmsp, eGrammar );
            pDoc->InsertMatrixFormula( nStartCol, nStartRow, nEndCol, nEndRow,
                    aMark, EMPTY_STRING, pCode, eGrammar);
            delete pCode;
            pDoc->IncXMLImportedFormulaCount( rString.Len() );
        }
        else if (bEnglish)
        {
            ScCompiler aComp( pDoc, rRange.aStart);
            aComp.SetGrammar(eGrammar);
            ScTokenArray* pCode = aComp.CompileString( rString );
            pDoc->InsertMatrixFormula( nStartCol, nStartRow, nEndCol, nEndRow,
                    aMark, EMPTY_STRING, pCode, eGrammar);
            delete pCode;
        }
        else
            pDoc->InsertMatrixFormula( nStartCol, nStartRow, nEndCol, nEndRow,
                    aMark, rString, NULL, eGrammar);

        if (bUndo)
        {
            //! auch bei Undo selektierte Tabellen beruecksichtigen
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoEnterMatrix( &rDocShell, rRange, pUndoDoc, rString ) );
        }

        //  Err522 beim Paint von DDE-Formeln werden jetzt beim Interpretieren abgefangen
        rDocShell.PostPaint( nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab, PAINT_GRID );
        aModificator.SetDocumentModified();

        bSuccess = sal_True;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::TabOp( const ScRange& rRange, const ScMarkData* pTabMark,
                            const ScTabOpParam& rParam, sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bSuccess = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, sal_True );
    }

    ScEditableTester aTester( pDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
    if ( aTester.IsEditable() )
    {
        WaitObject aWait( rDocShell.GetActiveDialogParent() );
        pDoc->SetDirty( rRange );
        if ( bRecord )
        {
            //! auch bei Undo selektierte Tabellen beruecksichtigen
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab );
            pDoc->CopyToDocument( rRange, IDF_ALL & ~IDF_NOTE, false, pUndoDoc );

            rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoTabOp( &rDocShell,
                                     nStartCol, nStartRow, nStartTab,
                                     nEndCol, nEndRow, nEndTab, pUndoDoc,
                                     rParam.aRefFormulaCell,
                                     rParam.aRefFormulaEnd,
                                     rParam.aRefRowCell,
                                     rParam.aRefColCell,
                                     rParam.nMode) );
        }
        pDoc->InsertTableOp(rParam, nStartCol, nStartRow, nEndCol, nEndRow, aMark);
        rDocShell.PostPaintGridAll();
        aModificator.SetDocumentModified();
        bSuccess = sal_True;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}

//------------------------------------------------------------------------

inline ScDirection DirFromFillDir( FillDir eDir )
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
                            FillDir eDir, bool bRecord, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument* pDoc = rDocShell.GetDocument();

    bool bSuccess = false;
    ScRange aRange = rRange;
    adjustFillRangeForAdjacentCopy(aRange, eDir);

    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCTAB nStartTab = aRange.aStart.Tab();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    SCTAB nEndTab = aRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, sal_True );
    }

    ScEditableTester aTester( pDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
    if ( aTester.IsEditable() )
    {
        WaitObject aWait( rDocShell.GetActiveDialogParent() );

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

        ScDocument* pUndoDoc = NULL;
        if ( bRecord )
        {
            SCTAB nTabCount = pDoc->GetTableCount();
            SCTAB nDestStartTab = aDestArea.aStart.Tab();

            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nDestStartTab, nDestStartTab );
            ScMarkData::iterator itr = aMark.begin(), itrEnd = aMark.end();
            for (; itr != itrEnd && *itr < nTabCount; ++itr)
                if (*itr != nDestStartTab)
                    pUndoDoc->AddUndoTab( *itr, *itr );

            ScRange aCopyRange = aDestArea;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);
            pDoc->CopyToDocument( aCopyRange, IDF_AUTOFILL, false, pUndoDoc, &aMark );
        }

        sal_uLong nProgCount;
        if (eDir == FILL_TO_BOTTOM || eDir == FILL_TO_TOP)
            nProgCount = aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1;
        else
            nProgCount = aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1;
        nProgCount *= nCount;
        ScProgress aProgress( pDoc->GetDocumentShell(),
                ScGlobal::GetRscString(STR_FILL_SERIES_PROGRESS), nProgCount );

        pDoc->Fill( aSourceArea.aStart.Col(), aSourceArea.aStart.Row(),
                aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), &aProgress,
                aMark, nCount, eDir, FILL_SIMPLE );
        AdjustRowHeight(aRange);

        if ( bRecord )      // Draw-Undo erst jetzt verfuegbar
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoAutoFill( &rDocShell, aDestArea, aSourceArea, pUndoDoc, aMark,
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

sal_Bool ScDocFunc::FillSeries( const ScRange& rRange, const ScMarkData* pTabMark,
                            FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                            double fStart, double fStep, double fMax,
                            sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bSuccess = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, sal_True );
    }

    ScEditableTester aTester( pDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
    if ( aTester.IsEditable() )
    {
        WaitObject aWait( rDocShell.GetActiveDialogParent() );

        ScRange aSourceArea = rRange;
        ScRange aDestArea   = rRange;

        SCSIZE nCount = pDoc->GetEmptyLinesInBlock(
                aSourceArea.aStart.Col(), aSourceArea.aStart.Row(), aSourceArea.aStart.Tab(),
                aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), aSourceArea.aEnd.Tab(),
                DirFromFillDir(eDir) );

        //  mindestens eine Zeile/Spalte als Quellbereich behalten:
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

        ScDocument* pUndoDoc = NULL;
        if ( bRecord )
        {
            SCTAB nTabCount = pDoc->GetTableCount();
            SCTAB nDestStartTab = aDestArea.aStart.Tab();

            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nDestStartTab, nDestStartTab );
            ScMarkData::iterator itr = aMark.begin(), itrEnd = aMark.end();
            for (; itr != itrEnd && *itr < nTabCount; ++itr)
                if (*itr != nDestStartTab)
                    pUndoDoc->AddUndoTab( *itr, *itr );

            pDoc->CopyToDocument(
                aDestArea.aStart.Col(), aDestArea.aStart.Row(), 0,
                aDestArea.aEnd.Col(), aDestArea.aEnd.Row(), nTabCount-1,
                IDF_AUTOFILL, false, pUndoDoc, &aMark );
        }

        if (aDestArea.aStart.Col() <= aDestArea.aEnd.Col() &&
            aDestArea.aStart.Row() <= aDestArea.aEnd.Row())
        {
            if ( fStart != MAXDOUBLE )
            {
                SCCOL nValX = (eDir == FILL_TO_LEFT) ? aDestArea.aEnd.Col() : aDestArea.aStart.Col();
                SCROW nValY = (eDir == FILL_TO_TOP ) ? aDestArea.aEnd.Row() : aDestArea.aStart.Row();
                SCTAB nTab = aDestArea.aStart.Tab();
                pDoc->SetValue( nValX, nValY, nTab, fStart );
            }

            sal_uLong nProgCount;
            if (eDir == FILL_TO_BOTTOM || eDir == FILL_TO_TOP)
                nProgCount = aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1;
            else
                nProgCount = aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1;
            nProgCount *= nCount;
            ScProgress aProgress( pDoc->GetDocumentShell(),
                    ScGlobal::GetRscString(STR_FILL_SERIES_PROGRESS), nProgCount );

            pDoc->Fill( aSourceArea.aStart.Col(), aSourceArea.aStart.Row(),
                        aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), &aProgress,
                        aMark, nCount, eDir, eCmd, eDateCmd, fStep, fMax );
            AdjustRowHeight(rRange);

            rDocShell.PostPaintGridAll();
            aModificator.SetDocumentModified();
        }

        if ( bRecord )      // Draw-Undo erst jetzt verfuegbar
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoAutoFill( &rDocShell, aDestArea, aSourceArea, pUndoDoc, aMark,
                                    eDir, eCmd, eDateCmd, fStart, fStep, fMax) );
        }

        bSuccess = sal_True;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}

sal_Bool ScDocFunc::FillAuto( ScRange& rRange, const ScMarkData* pTabMark,
                            FillDir eDir, sal_uLong nCount, sal_Bool bRecord, sal_Bool bApi )
{
    double      fStep = 1.0;
    double      fMax = MAXDOUBLE;
    return FillAuto( rRange, pTabMark, eDir, FILL_AUTO, FILL_DAY, nCount, fStep, fMax, bRecord, bApi );
}

sal_Bool ScDocFunc::FillAuto( ScRange& rRange, const ScMarkData* pTabMark, FillDir eDir, FillCmd eCmd, FillDateCmd  eDateCmd, sal_uLong nCount, double fStep, double fMax,  sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, sal_True );
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
            OSL_FAIL("Falsche Richtung bei FillAuto");
            break;
    }

    //      Zellschutz testen
    //!     Quellbereich darf geschuetzt sein !!!
    //!     aber kein Matrixfragment enthalten !!!

    ScEditableTester aTester( pDoc, aDestArea );
    if ( !aTester.IsEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if ( pDoc->HasSelectedBlockMatrixFragment( nStartCol, nStartRow,
            nEndCol, nEndRow, aMark ) )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_MATRIXFRAGMENTERR);
        return false;
    }

    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    ScDocument* pUndoDoc = NULL;
    if ( bRecord )
    {
        SCTAB nTabCount = pDoc->GetTableCount();
        SCTAB nDestStartTab = aDestArea.aStart.Tab();

        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nDestStartTab, nDestStartTab );
        ScMarkData::iterator itr = aMark.begin(), itrEnd = aMark.end();
        for (; itr != itrEnd && nTabCount; ++itr)
            if (*itr != nDestStartTab)
                pUndoDoc->AddUndoTab( *itr, *itr );

        // do not clone note captions in undo document
        pDoc->CopyToDocument(
            aDestArea.aStart.Col(), aDestArea.aStart.Row(), 0,
            aDestArea.aEnd.Col(), aDestArea.aEnd.Row(), nTabCount-1,
            IDF_AUTOFILL, false, pUndoDoc, &aMark );
    }

    sal_uLong nProgCount;
    if (eDir == FILL_TO_BOTTOM || eDir == FILL_TO_TOP)
        nProgCount = aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1;
    else
        nProgCount = aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1;
    nProgCount *= nCount;
    ScProgress aProgress( pDoc->GetDocumentShell(),
            ScGlobal::GetRscString(STR_FILL_SERIES_PROGRESS), nProgCount );

    pDoc->Fill( aSourceArea.aStart.Col(), aSourceArea.aStart.Row(),
            aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), &aProgress,
            aMark, nCount, eDir, eCmd, eDateCmd, fStep, fMax );

    AdjustRowHeight(aDestArea);

    if ( bRecord )      // Draw-Undo erst jetzt verfuegbar
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoAutoFill( &rDocShell, aDestArea, aSourceArea, pUndoDoc, aMark,
                                eDir, eCmd, eDateCmd, MAXDOUBLE, fStep, fMax) );
    }

    rDocShell.PostPaintGridAll();
    aModificator.SetDocumentModified();

    rRange = aDestArea;         // Zielbereich zurueckgeben (zum Markieren)
    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::MergeCells( const ScCellMergeOption& rOption, sal_Bool bContents, sal_Bool bRecord, sal_Bool bApi )
{
    using ::std::set;

    ScDocShellModificator aModificator( rDocShell );

    SCCOL nStartCol = rOption.mnStartCol;
    SCROW nStartRow = rOption.mnStartRow;
    SCCOL nEndCol = rOption.mnEndCol;
    SCROW nEndRow = rOption.mnEndRow;
    if ((nStartCol == nEndCol && nStartRow == nEndRow) || rOption.maTabs.empty())
    {
        // Nothing to do.  Bail out quick.
        return true;
    }

    ScDocument* pDoc = rDocShell.GetDocument();
    set<SCTAB>::const_iterator itrBeg = rOption.maTabs.begin(), itrEnd = rOption.maTabs.end();
    SCTAB nTab1 = *itrBeg, nTab2 = *rOption.maTabs.rbegin();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    for (set<SCTAB>::const_iterator itr = itrBeg; itr != itrEnd; ++itr)
    {
        ScEditableTester aTester( pDoc, *itr, nStartCol, nStartRow, nEndCol, nEndRow );
        if (!aTester.IsEditable())
        {
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());
            return false;
        }

        if ( pDoc->HasAttrib( nStartCol, nStartRow, *itr, nEndCol, nEndRow, *itr,
                                HASATTR_MERGED | HASATTR_OVERLAPPED ) )
        {
            // "Zusammenfassen nicht verschachteln !"
            if (!bApi)
                rDocShell.ErrorMessage(STR_MSSG_MERGECELLS_0);
            return false;
        }
    }

    ScDocument* pUndoDoc = NULL;
    bool bNeedContentsUndo = false;
    for (set<SCTAB>::const_iterator itr = itrBeg; itr != itrEnd; ++itr)
    {
        SCTAB nTab = *itr;
        bool bNeedContents = bContents &&
                ( !pDoc->IsBlockEmpty( nTab, nStartCol,nStartRow+1, nStartCol,nEndRow, true ) ||
                  !pDoc->IsBlockEmpty( nTab, nStartCol+1,nStartRow, nEndCol,nEndRow, true ) );

        if (bRecord)
        {
            // test if the range contains other notes which also implies that we need an undo document
            bool bHasNotes = false;
            for( ScAddress aPos( nStartCol, nStartRow, nTab ); !bHasNotes && (aPos.Col() <= nEndCol); aPos.IncCol() )
                for( aPos.SetRow( nStartRow ); !bHasNotes && (aPos.Row() <= nEndRow); aPos.IncRow() )
                    bHasNotes = ((aPos.Col() != nStartCol) || (aPos.Row() != nStartRow)) && (pDoc->GetNotes( aPos.Tab() )->findByAddress(aPos) != 0);

            if (!pUndoDoc)
            {
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pUndoDoc->InitUndo(pDoc, nTab1, nTab2);
            }
            // note captions are collected by drawing undo
            pDoc->CopyToDocument( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                  IDF_ALL|IDF_NOCAPTIONS, false, pUndoDoc );
            if( bHasNotes )
                pDoc->BeginDrawUndo();
        }

        if (bNeedContents)
            pDoc->DoMergeContents( nTab, nStartCol,nStartRow, nEndCol,nEndRow );
        pDoc->DoMerge( nTab, nStartCol,nStartRow, nEndCol,nEndRow );

        if (rOption.mbCenter)
        {
            pDoc->ApplyAttr( nStartCol, nStartRow, nTab, SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY ) );
            pDoc->ApplyAttr( nStartCol, nStartRow, nTab, SvxVerJustifyItem( SVX_VER_JUSTIFY_CENTER, ATTR_VER_JUSTIFY ) );
        }

        if ( !AdjustRowHeight( ScRange( 0,nStartRow,nTab, MAXCOL,nEndRow,nTab ) ) )
            rDocShell.PostPaint( nStartCol, nStartRow, nTab,
                                 nEndCol, nEndRow, nTab, PAINT_GRID );
        if (bNeedContents || rOption.mbCenter)
        {
            ScRange aRange(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab);
            pDoc->SetDirty(aRange);
        }

        bNeedContentsUndo |= bNeedContents;
    }

    if (pUndoDoc)
    {
        SdrUndoGroup* pDrawUndo = pDoc->GetDrawLayer() ? pDoc->GetDrawLayer()->GetCalcUndo() : NULL;
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoMerge(&rDocShell, rOption, bNeedContentsUndo, pUndoDoc, pDrawUndo) );
    }

    aModificator.SetDocumentModified();

    SfxBindings* pBindings = rDocShell.GetViewBindings();
    if (pBindings)
    {
        pBindings->Invalidate( FID_MERGE_ON );
        pBindings->Invalidate( FID_MERGE_OFF );
        pBindings->Invalidate( FID_MERGE_TOGGLE );
    }

    return sal_True;
}

sal_Bool ScDocFunc::UnmergeCells( const ScRange& rRange, sal_Bool bRecord )
{
    ScCellMergeOption aOption(rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row());
    SCTAB nTab1 = rRange.aStart.Tab(), nTab2 = rRange.aEnd.Tab();
    for (SCTAB i = nTab1; i <= nTab2; ++i)
        aOption.maTabs.insert(i);

    return UnmergeCells(aOption, bRecord);
}

bool ScDocFunc::UnmergeCells( const ScCellMergeOption& rOption, sal_Bool bRecord )
{
    using ::std::set;

    if (rOption.maTabs.empty())
        // Nothing to unmerge.
        return true;

    ScDocShellModificator aModificator( rDocShell );
    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScDocument* pUndoDoc = NULL;
    for (set<SCTAB>::const_iterator itr = rOption.maTabs.begin(), itrEnd = rOption.maTabs.end();
          itr != itrEnd; ++itr)
    {
        SCTAB nTab = *itr;
        ScRange aRange = rOption.getSingleRange(nTab);
        if ( !pDoc->HasAttrib(aRange, HASATTR_MERGED) )
            continue;

        ScRange aExtended = aRange;
        pDoc->ExtendMerge(aExtended);
        ScRange aRefresh = aExtended;
        pDoc->ExtendOverlapped(aRefresh);

        if (bRecord)
        {
            if (!pUndoDoc)
            {
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pUndoDoc->InitUndo(pDoc, *rOption.maTabs.begin(), *rOption.maTabs.rbegin());
            }
            pDoc->CopyToDocument(aExtended, IDF_ATTRIB, false, pUndoDoc);
        }

        const SfxPoolItem& rDefAttr = pDoc->GetPool()->GetDefaultItem( ATTR_MERGE );
        ScPatternAttr aPattern( pDoc->GetPool() );
        aPattern.GetItemSet().Put( rDefAttr );
        pDoc->ApplyPatternAreaTab( aRange.aStart.Col(), aRange.aStart.Row(),
                                   aRange.aEnd.Col(), aRange.aEnd.Row(), nTab,
                                   aPattern );

        pDoc->RemoveFlagsTab( aExtended.aStart.Col(), aExtended.aStart.Row(),
                              aExtended.aEnd.Col(), aExtended.aEnd.Row(), nTab,
                              SC_MF_HOR | SC_MF_VER );

        pDoc->ExtendMerge( aRefresh, sal_True );

        if ( !AdjustRowHeight( aExtended ) )
            rDocShell.PostPaint( aExtended, PAINT_GRID );
    }

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoRemoveMerge( &rDocShell, rOption, pUndoDoc ) );
    }
    aModificator.SetDocumentModified();

    return sal_True;
}

//------------------------------------------------------------------------

bool ScDocFunc::ModifyRangeNames( const ScRangeName& rNewRanges, SCTAB nTab )
{
    return SetNewRangeNames( new ScRangeName(rNewRanges), true, nTab );
}

bool ScDocFunc::SetNewRangeNames( ScRangeName* pNewRanges, bool bModifyDoc, SCTAB nTab )     // takes ownership of pNewRanges
{
    ScDocShellModificator aModificator( rDocShell );

    OSL_ENSURE( pNewRanges, "pNewRanges is 0" );
    ScDocument* pDoc = rDocShell.GetDocument();
    bool bUndo(pDoc->IsUndoEnabled());

    if (bUndo)
    {
        ScRangeName* pOld;
        if (nTab >=0)
        {
            pOld = pDoc->GetRangeName(nTab);
        }
        else
        {
            pOld = pDoc->GetRangeName();
        }
        ScRangeName* pUndoRanges = new ScRangeName(*pOld);
        ScRangeName* pRedoRanges = new ScRangeName(*pNewRanges);
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoRangeNames( &rDocShell, pUndoRanges, pRedoRanges, nTab ) );
    }

    // #i55926# While loading XML, formula cells only have a single string token,
    // so CompileNameFormula would never find any name (index) tokens, and would
    // unnecessarily loop through all cells.
    sal_Bool bCompile = ( !pDoc->IsImportingXML() && pDoc->GetNamedRangesLockCount() == 0 );

    if ( bCompile )
        pDoc->CompileNameFormula( sal_True );   // CreateFormulaString
    if (nTab >= 0)
        pDoc->SetRangeName( nTab, pNewRanges ); // takes ownership
    else
        pDoc->SetRangeName( pNewRanges );       // takes ownership
    if ( bCompile )
        pDoc->CompileNameFormula( false );  // CompileFormulaString

    if (bModifyDoc)
    {
        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint(SC_HINT_AREAS_CHANGED) );
    }

    return true;
}

void ScDocFunc::ModifyAllRangeNames( const boost::ptr_map<OUString, ScRangeName>& rRangeMap )
{
    ScDocShellModificator aModificator(rDocShell);
    ScDocument* pDoc = rDocShell.GetDocument();

    if (pDoc->IsUndoEnabled())
    {
        std::map<OUString, ScRangeName*> aOldRangeMap;
        pDoc->GetRangeNameMap(aOldRangeMap);
        rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoAllRangeNames(&rDocShell, aOldRangeMap, rRangeMap));
    }

    pDoc->CompileNameFormula(true);

    // set all range names
    pDoc->SetAllRangeNames(rRangeMap);

    pDoc->CompileNameFormula(false);

    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast(SfxSimpleHint(SC_HINT_AREAS_CHANGED));
}

//------------------------------------------------------------------------

void ScDocFunc::CreateOneName( ScRangeName& rList,
                                SCCOL nPosX, SCROW nPosY, SCTAB nTab,
                                SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                sal_Bool& rCancel, sal_Bool bApi )
{
    if (rCancel)
        return;

    ScDocument* pDoc = rDocShell.GetDocument();
    if (!pDoc->HasValueData( nPosX, nPosY, nTab ))
    {
        String aName = pDoc->GetString(nPosX, nPosY, nTab);
        ScRangeData::MakeValidName(aName);
        if (aName.Len())
        {
            String aContent;
            ScRange( nX1, nY1, nTab, nX2, nY2, nTab ).Format( aContent, SCR_ABS_3D, pDoc );

            bool bInsert = false;
            ScRangeData* pOld = rList.findByUpperName(ScGlobal::pCharClass->uppercase(aName));
            if (pOld)
            {
                String aOldStr;
                pOld->GetSymbol( aOldStr );
                if (aOldStr != aContent)
                {
                    if (bApi)
                        bInsert = sal_True;     // per API nicht nachfragen
                    else
                    {
                        String aTemplate = ScGlobal::GetRscString( STR_CREATENAME_REPLACE );

                        String aMessage = aTemplate.GetToken( 0, '#' );
                        aMessage += aName;
                        aMessage += aTemplate.GetToken( 1, '#' );

                        short nResult = QueryBox( rDocShell.GetActiveDialogParent(),
                                                    WinBits(WB_YES_NO_CANCEL | WB_DEF_YES),
                                                    aMessage ).Execute();
                        if ( nResult == RET_YES )
                        {
                            rList.erase(*pOld);
                            bInsert = true;
                        }
                        else if ( nResult == RET_CANCEL )
                            rCancel = sal_True;
                    }
                }
            }
            else
                bInsert = true;

            if (bInsert)
            {
                ScRangeData* pData = new ScRangeData( pDoc, aName, aContent,
                        ScAddress( nPosX, nPosY, nTab));
                if (!rList.insert(pData))
                {
                    OSL_FAIL("nanu?");
                }
            }
        }
    }
}

sal_Bool ScDocFunc::CreateNames( const ScRange& rRange, sal_uInt16 nFlags, sal_Bool bApi, SCTAB aTab )
{
    if (!nFlags)
        return false;       // war nix

    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bDone = false;
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();
    OSL_ENSURE(rRange.aEnd.Tab() == nTab, "CreateNames: mehrere Tabellen geht nicht");

    sal_Bool bValid = sal_True;
    if ( nFlags & ( NAME_TOP | NAME_BOTTOM ) )
        if ( nStartRow == nEndRow )
            bValid = false;
    if ( nFlags & ( NAME_LEFT | NAME_RIGHT ) )
        if ( nStartCol == nEndCol )
            bValid = false;

    if (bValid)
    {
        ScDocument* pDoc = rDocShell.GetDocument();
        ScRangeName* pNames;
        if (aTab >=0)
            pNames = pDoc->GetRangeName(nTab);
        else
            pNames = pDoc->GetRangeName();

        if (!pNames)
            return false;   // soll nicht sein
        ScRangeName aNewRanges( *pNames );

        sal_Bool bTop    = ( ( nFlags & NAME_TOP ) != 0 );
        sal_Bool bLeft   = ( ( nFlags & NAME_LEFT ) != 0 );
        sal_Bool bBottom = ( ( nFlags & NAME_BOTTOM ) != 0 );
        sal_Bool bRight  = ( ( nFlags & NAME_RIGHT ) != 0 );

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

        sal_Bool bCancel = false;
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

        bDone = ModifyRangeNames( aNewRanges, aTab );

        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
    }

    return bDone;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::InsertNameList( const ScAddress& rStartPos, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bDone = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    const bool bRecord = pDoc->IsUndoEnabled();
    SCTAB nTab = rStartPos.Tab();
    ScDocument* pUndoDoc = NULL;

    //local names have higher priority than global names
    ScRangeName* pLocalList = pDoc->GetRangeName(nTab);
    sal_uInt16 nValidCount = 0;
    ScRangeName::iterator itrLocalBeg = pLocalList->begin(), itrLocalEnd = pLocalList->end();
    for (ScRangeName::iterator itr = itrLocalBeg; itr != itrLocalEnd; ++itr)
    {
        const ScRangeData& r = *itr->second;
        if (!r.HasType(RT_DATABASE))
            ++nValidCount;
    }
    ScRangeName* pList = pDoc->GetRangeName();
    ScRangeName::iterator itrBeg = pList->begin(), itrEnd = pList->end();
    for (ScRangeName::iterator itr = itrBeg; itr != itrEnd; ++itr)
    {
        const ScRangeData& r = *itr->second;
        if (!r.HasType(RT_DATABASE) && !pLocalList->findByUpperName(r.GetUpperName()))
            ++nValidCount;
    }

    if (nValidCount)
    {
        SCCOL nStartCol = rStartPos.Col();
        SCROW nStartRow = rStartPos.Row();
        SCCOL nEndCol = nStartCol + 1;
        SCROW nEndRow = nStartRow + static_cast<SCROW>(nValidCount) - 1;

        ScEditableTester aTester( pDoc, nTab, nStartCol,nStartRow, nEndCol,nEndRow );
        if (aTester.IsEditable())
        {
            if (bRecord)
            {
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pUndoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab,
                                        IDF_ALL, false, pUndoDoc );

                pDoc->BeginDrawUndo();      // wegen Hoehenanpassung
            }

            ScRangeData** ppSortArray = new ScRangeData* [ nValidCount ];
            sal_uInt16 j = 0;
            for (ScRangeName::iterator itr = itrLocalBeg; itr != itrLocalEnd; ++itr)
            {
                ScRangeData& r = *itr->second;
                if (!r.HasType(RT_DATABASE))
                    ppSortArray[j++] = &r;
            }
            for (ScRangeName::iterator itr = itrBeg; itr != itrEnd; ++itr)
            {
                ScRangeData& r = *itr->second;
                if (!r.HasType(RT_DATABASE) && !pLocalList->findByUpperName(itr->first))
                    ppSortArray[j++] = &r;
            }
#ifndef ICC
            qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                &ScRangeData_QsortNameCompare );
#else
            qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                ICCQsortNameCompare );
#endif
            OUString aName;
            OUStringBuffer aContent;
            String aFormula;
            SCROW nOutRow = nStartRow;
            for (j=0; j<nValidCount; j++)
            {
                ScRangeData* pData = ppSortArray[j];
                pData->GetName(aName);
                // relative Referenzen Excel-konform auf die linke Spalte anpassen:
                pData->UpdateSymbol(aContent, ScAddress( nStartCol, nOutRow, nTab ));
                aFormula = '=';
                aFormula += aContent.toString();
                ScSetStringParam aParam;
                aParam.setTextInput();
                pDoc->SetString(ScAddress(nStartCol,nOutRow,nTab), aName, &aParam);
                pDoc->SetString(ScAddress(nEndCol,nOutRow,nTab), aFormula, &aParam);
                ++nOutRow;
            }

            delete [] ppSortArray;

            if (bRecord)
            {
                ScDocument* pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
                pRedoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab,
                                        IDF_ALL, false, pRedoDoc );

                rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoListNames( &rDocShell,
                                ScRange( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab ),
                                pUndoDoc, pRedoDoc ) );
            }

            if (!AdjustRowHeight(ScRange(0,nStartRow,nTab,MAXCOL,nEndRow,nTab)))
                rDocShell.PostPaint( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab, PAINT_GRID );

            aModificator.SetDocumentModified();
            bDone = sal_True;
        }
        else if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
    }
    return bDone;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::ResizeMatrix( const ScRange& rOldRange, const ScAddress& rNewEnd, sal_Bool bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rOldRange.aStart.Col();
    SCROW nStartRow = rOldRange.aStart.Row();
    SCTAB nTab = rOldRange.aStart.Tab();

    bool bUndo(pDoc->IsUndoEnabled());

    sal_Bool bRet = false;

    String aFormula;
    pDoc->GetFormula( nStartCol, nStartRow, nTab, aFormula );
    if ( aFormula.GetChar(0) == '{' && aFormula.GetChar(aFormula.Len()-1) == '}' )
    {
        String aUndo = ScGlobal::GetRscString( STR_UNDO_RESIZEMATRIX );
        if (bUndo)
            rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo );

        aFormula.Erase(0,1);
        aFormula.Erase(aFormula.Len()-1,1);

        ScMarkData aMark;
        aMark.SetMarkArea( rOldRange );
        aMark.SelectTable( nTab, sal_True );
        ScRange aNewRange( rOldRange.aStart, rNewEnd );

        if ( DeleteContents( aMark, IDF_CONTENTS, sal_True, bApi ) )
        {
            // GRAM_PODF_A1 for API compatibility.
            bRet = EnterMatrix( aNewRange, &aMark, NULL, aFormula, bApi, false, EMPTY_STRING, formula::FormulaGrammar::GRAM_PODF_A1 );
            if (!bRet)
            {
                //  versuchen, alten Zustand wiederherzustellen
                EnterMatrix( rOldRange, &aMark, NULL, aFormula, bApi, false, EMPTY_STRING, formula::FormulaGrammar::GRAM_PODF_A1 );
            }
        }

        if (bUndo)
            rDocShell.GetUndoManager()->LeaveListAction();
    }

    return bRet;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::InsertAreaLink( const String& rFile, const String& rFilter,
                                const String& rOptions, const String& rSource,
                                const ScRange& rDestRange, sal_uLong nRefresh,
                                sal_Bool bFitBlock, sal_Bool bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    sal_Bool bUndo (pDoc->IsUndoEnabled());

    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();

    //  #i52120# if other area links exist at the same start position,
    //  remove them first (file format specifies only one link definition
    //  for a cell)

    sal_uInt16 nLinkCount = pLinkManager->GetLinks().size();
    sal_uInt16 nRemoved = 0;
    sal_uInt16 nLinkPos = 0;
    while (nLinkPos<nLinkCount)
    {
        ::sfx2::SvBaseLink* pBase = *pLinkManager->GetLinks()[nLinkPos];
        if ( pBase->ISA(ScAreaLink) &&
             static_cast<ScAreaLink*>(pBase)->GetDestArea().aStart == rDestRange.aStart )
        {
            if ( bUndo )
            {
                if ( !nRemoved )
                {
                    // group all remove and the insert action
                    String aUndo = ScGlobal::GetRscString( STR_UNDO_INSERTAREALINK );
                    rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo );
                }

                ScAreaLink* pOldArea = static_cast<ScAreaLink*>(pBase);
                rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoRemoveAreaLink( &rDocShell,
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
    String aTmp = aFilterName;
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, rFile, &aTmp, &rSource );

    //  Undo fuer den leeren Link

    if (bUndo)
    {
        rDocShell.GetUndoManager()->AddUndoAction( new ScUndoInsertAreaLink( &rDocShell,
                                                    rFile, aFilterName, aNewOptions,
                                                    rSource, rDestRange, nRefresh ) );
        if ( nRemoved )
            rDocShell.GetUndoManager()->LeaveListAction();  // undo for link update is still separate
    }

    //  Update hat sein eigenes Undo
    if (pDoc->IsExecuteLinkEnabled())
    {
        pLink->SetDoInsert(bFitBlock);  // beim ersten Update ggf. nichts einfuegen
        pLink->Update();                // kein SetInCreate -> Update ausfuehren
    }
    pLink->SetDoInsert(sal_True);       // Default = sal_True

    SfxBindings* pBindings = rDocShell.GetViewBindings();
    if (pBindings)
        pBindings->Invalidate( SID_LINKS );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator

    return sal_True;
}

namespace {

void RemoveCondFormatAttributes(ScDocument* pDoc, const ScConditionalFormat* pFormat, SCTAB nTab)
{
    const ScRangeList& rRangeList = pFormat->GetRange();
    pDoc->RemoveCondFormatData( rRangeList, nTab, pFormat->GetKey() );
}

void SetConditionalFormatAttributes(ScDocument* pDoc, const ScRangeList& rRanges, sal_uLong nIndex, SCTAB nTab)
{
    pDoc->AddCondFormatData( rRanges, nTab, nIndex );
}

}

void ScDocFunc::ReplaceConditionalFormat( sal_uLong nOldFormat, ScConditionalFormat* pFormat, SCTAB nTab, const ScRangeList& rRanges )
{
    ScDocShellModificator aModificator(rDocShell);
    ScDocument* pDoc = rDocShell.GetDocument();
    if(pDoc->IsTabProtected(nTab))
        return;

    boost::scoped_ptr<ScRange> pRepaintRange;
    if(nOldFormat)
    {
        ScConditionalFormat* pOldFormat = pDoc->GetCondFormList(nTab)->GetFormat(nOldFormat);
        if(pOldFormat)
        {
            pRepaintRange.reset(new ScRange( pOldFormat->GetRange().Combine() ));
            RemoveCondFormatAttributes(pDoc, pOldFormat, nTab);
        }

        pDoc->DeleteConditionalFormat(nOldFormat, nTab);
        pDoc->SetStreamValid(nTab, false);
    }
    if(pFormat)
    {
        if(pRepaintRange)
            pRepaintRange->ExtendTo(rRanges.Combine());
        else
            pRepaintRange.reset(new ScRange(rRanges.Combine()));

	sal_uLong nIndex = pDoc->AddCondFormat(pFormat, nTab);

        SetConditionalFormatAttributes(pDoc, rRanges, nIndex, nTab);
        pDoc->SetStreamValid(nTab, false);
    }

    if(pRepaintRange)
        rDocShell.PostPaint(*pRepaintRange, PAINT_GRID);

    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast(SfxSimpleHint(SC_HINT_AREAS_CHANGED));
}

void ScDocFunc::SetConditionalFormatList( ScConditionalFormatList* pList, SCTAB nTab )
{
    ScDocShellModificator aModificator(rDocShell);
    ScDocument* pDoc = rDocShell.GetDocument();
    if(pDoc->IsTabProtected(nTab))
        return;

    // first remove all old entries
    ScConditionalFormatList* pOldList = pDoc->GetCondFormList(nTab);
    for(ScConditionalFormatList::const_iterator itr = pOldList->begin(), itrEnd = pOldList->end(); itr != itrEnd; ++itr)
    {
        RemoveCondFormatAttributes(pDoc, &(*itr), nTab);
    }

    // then set new entries
    for(ScConditionalFormatList::iterator itr = pList->begin(); itr != pList->end(); ++itr)
    {
        sal_uLong nIndex = itr->GetKey();
        const ScRangeList& rRange = itr->GetRange();
        SetConditionalFormatAttributes(pDoc, rRange, nIndex, nTab);
    }

    pDoc->SetCondFormList(pList, nTab);
    rDocShell.PostPaintGridAll();

    pDoc->SetStreamValid(nTab, false);
    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast(SfxSimpleHint(SC_HINT_AREAS_CHANGED));
}

void ScDocFunc::EnterListAction( sal_uInt16 nNameResId )
{
    String aUndo( ScGlobal::GetRscString( nNameResId ) );
    rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo );
}

void ScDocFunc::EndListAction()
{
    rDocShell.GetUndoManager()->LeaveListAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
