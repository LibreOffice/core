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
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <editeng/editobj.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/printer.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>
#include <svl/zforlist.hxx>
#include <svl/PasswordHelper.hxx>

#include <basic/sbstar.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>

#include <list>

#include "docfunc.hxx"

#include "sc.hrc"

#include "arealink.hxx"
#include "attrib.hxx"
#include "dociter.hxx"
#include "autoform.hxx"
#include "cell.hxx"
#include "detdata.hxx"
#include "detfunc.hxx"
#include "docpool.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "editutil.hxx"
#include "globstr.hrc"
//CHINA001 #include "namecrea.hxx"      // NAME_TOP etc.
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
#include "scui_def.hxx" //CHINA001
#include "tabprotection.hxx"
#include "clipparam.hxx"
#include "externalrefmgr.hxx"

#include <memory>
#include <basic/basmgr.hxx>
#include <boost/scoped_ptr.hpp>

using namespace com::sun::star;
using ::com::sun::star::uno::Sequence;

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
            pDoc->SetStreamValid(nTab, sal_False);

    return 0;
}

//------------------------------------------------------------------------

//  Zeile ueber dem Range painten (fuer Linien nach AdjustRowHeight)

void lcl_PaintAbove( ScDocShell& rDocShell, const ScRange& rRange )
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
        return sal_False;
    }
    if ( !pDoc->IsAdjustHeightEnabled() )
    {
        return sal_False;
    }

    SCTAB nTab      = rRange.aStart.Tab();
    SCROW nStartRow = rRange.aStart.Row();
    SCROW nEndRow   = rRange.aEnd.Row();

    ScSizeDeviceProvider aProv( &rDocShell );
    Fraction aOne(1,1);

    sal_Bool bChanged = pDoc->SetOptimalHeight( nStartRow, nEndRow, nTab, 0, aProv.GetDevice(),
                                            aProv.GetPPTX(), aProv.GetPPTY(), aOne, aOne, sal_False );

    if ( bPaint && bChanged )
        rDocShell.PostPaint( 0, nStartRow, nTab, MAXCOL, MAXROW, nTab,
                                            PAINT_GRID | PAINT_LEFT );

    return bChanged;
}


//------------------------------------------------------------------------

sal_Bool ScDocFunc::DetectiveAddPred(const ScAddress& rPos)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument* pDoc = rDocShell.GetDocument();
    sal_Bool bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo();
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

    sal_Bool bUndo(pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return sal_False;

    ScDocShellModificator aModificator( rDocShell );

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo();
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

    sal_Bool bUndo(pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo();
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

    sal_Bool bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return sal_False;

    ScDocShellModificator aModificator( rDocShell );

    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo();
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

    sal_Bool bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo();
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

    sal_Bool bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();

    Window* pWaitWin = rDocShell.GetActiveDialogParent();
    if (pWaitWin)
        pWaitWin->EnterWait();
    if (bUndo)
        pModel->BeginCalcUndo();
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

    sal_Bool bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return sal_False;

    ScDocShellModificator aModificator( rDocShell );

    if (bUndo)
        pModel->BeginCalcUndo();
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
    sal_Bool bDone = sal_False;
    ScDocument* pDoc = rDocShell.GetDocument();

    sal_Bool bUndo (pDoc->IsUndoEnabled());
    ScDetOpList* pList = pDoc->GetDetOpList();
    if ( pList && pList->Count() )
    {
        rDocShell.MakeDrawLayer();
        ScDrawLayer* pModel = pDoc->GetDrawLayer();
        if (bUndo)
            pModel->BeginCalcUndo();

        //  Loeschen auf allen Tabellen

        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
            ScDetectiveFunc( pDoc,nTab ).DeleteAll( SC_DET_ARROWS );    // don't remove circles

        //  Wiederholen

        sal_uInt16 nCount = pList->Count();
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            ScDetOpData* pData = (*pList)[i];
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
                        DBG_ERROR("falsche Op bei DetectiveRefresh");
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

//------------------------------------------------------------------------

sal_Bool ScDocFunc::DeleteContents( const ScMarkData& rMark, sal_uInt16 nFlags,
                                    sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        DBG_ERROR("ScDocFunc::DeleteContents ohne Markierung");
        return sal_False;
    }

    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;

    ScEditableTester aTester( pDoc, rMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return sal_False;
    }

    ScRange aMarkRange;
    sal_Bool bSimple = sal_False;

    ScMarkData aMultiMark = rMark;
    aMultiMark.SetMarking(sal_False);       // fuer MarkToMulti

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
            bMulti = sal_False;
    }

    // keine Objekte auf geschuetzten Tabellen
    sal_Bool bObjects = sal_False;
    if ( nFlags & IDF_OBJECTS )
    {
        bObjects = sal_True;
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
            if (aMultiMark.GetTableSelect(nTab) && pDoc->IsTabProtected(nTab))
                bObjects = sal_False;
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
            nUndoDocFlags |= IDF_CONTENTS;  // #68795# copy all cells with their notes
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
//       aMultiMark.MarkToSimple();
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

//  rDocShell.UpdateOle(GetViewData());     //! an der View?
    aModificator.SetDocumentModified();
//! CellContentChanged();
//! ShowAllCursors();

#if 0
    //! muss an der View bleiben !!!!
    if ( nFlags & IDF_ATTRIB )
    {
        if ( nFlags & IDF_CONTENTS )
            ForgetFormatArea();
        else
            StartFormatArea();              // Attribute loeschen ist auch Attributierung
    }
#endif

    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::TransliterateText( const ScMarkData& rMark, sal_Int32 nType,
                                    sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;

    ScEditableTester aTester( pDoc, rMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return sal_False;
    }

    ScRange aMarkRange;
    ScMarkData aMultiMark = rMark;
    aMultiMark.SetMarking(sal_False);       // for MarkToMulti
    aMultiMark.MarkToMulti();
    aMultiMark.GetMultiMarkArea( aMarkRange );

    if (bRecord)
    {
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCTAB nTabCount = pDoc->GetTableCount();

        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
        for (SCTAB i=0; i<nTabCount; i++)
            if (i != nStartTab && rMark.GetTableSelect(i))
                pUndoDoc->AddUndoTab( i, i );

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

sal_Bool ScDocFunc::SetNormalString( const ScAddress& rPos, const String& rText, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument* pDoc = rDocShell.GetDocument();

    sal_Bool bUndo(pDoc->IsUndoEnabled());
    ScEditableTester aTester( pDoc, rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return sal_False;
    }

    SCTAB* pTabs = NULL;
    ScBaseCell** ppOldCells = NULL;
    sal_Bool* pHasFormat = NULL;
    sal_uLong* pOldFormats = NULL;
    ScBaseCell* pDocCell = pDoc->GetCell( rPos );
    sal_Bool bEditDeleted = (pDocCell && pDocCell->GetCellType() == CELLTYPE_EDIT);
    if (bUndo)
    {
        pTabs = new SCTAB[1];
        pTabs[0] = rPos.Tab();
        ppOldCells  = new ScBaseCell*[1];
        ppOldCells[0] = pDocCell ? pDocCell->CloneWithoutNote( *pDoc ) : 0;

        pHasFormat = new sal_Bool[1];
        pOldFormats = new sal_uLong[1];
        const SfxPoolItem* pItem;
        const ScPatternAttr* pPattern = pDoc->GetPattern( rPos.Col(),rPos.Row(),rPos.Tab() );
        if ( SFX_ITEM_SET == pPattern->GetItemSet().GetItemState(
                                ATTR_VALUE_FORMAT,sal_False,&pItem) )
        {
            pHasFormat[0] = sal_True;
            pOldFormats[0] = ((const SfxUInt32Item*)pItem)->GetValue();
        }
        else
            pHasFormat[0] = sal_False;
    }

    pDoc->SetString( rPos.Col(), rPos.Row(), rPos.Tab(), rText );

    if (bUndo)
    {
        //  wegen ChangeTracking darf UndoAction erst nach SetString angelegt werden
        rDocShell.GetUndoManager()->AddUndoAction(new ScUndoEnterData( &rDocShell, rPos.Col(),rPos.Row(),rPos.Tab(), 1,pTabs,
                                     ppOldCells, pHasFormat, pOldFormats, rText, NULL ) );
    }

    if ( bEditDeleted || pDoc->HasAttrib( ScRange(rPos), HASATTR_NEEDHEIGHT ) )
        AdjustRowHeight( ScRange(rPos) );

    rDocShell.PostPaintCell( rPos );
    aModificator.SetDocumentModified();

    // #107160# notify input handler here the same way as in PutCell
    if (bApi)
        NotifyInputHandler( rPos );

    return sal_True;
}

sal_Bool ScDocFunc::PutCell( const ScAddress& rPos, ScBaseCell* pNewCell, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument* pDoc = rDocShell.GetDocument();
    sal_Bool bUndo (pDoc->IsUndoEnabled());
    sal_Bool bXMLLoading(pDoc->IsImportingXML());

    // #i925#; it is not neccessary to test whether the cell is editable on loading a XML document
    if (!bXMLLoading)
    {
        ScEditableTester aTester( pDoc, rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() );
        if (!aTester.IsEditable())
        {
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());
            pNewCell->Delete();
            return sal_False;
        }
    }

    sal_Bool bEditCell = ( pNewCell->GetCellType() == CELLTYPE_EDIT );
    ScBaseCell* pDocCell = pDoc->GetCell( rPos );
    sal_Bool bEditDeleted = (pDocCell && pDocCell->GetCellType() == CELLTYPE_EDIT);
    sal_Bool bHeight = ( bEditDeleted || bEditCell ||
                    pDoc->HasAttrib( ScRange(rPos), HASATTR_NEEDHEIGHT ) );

    ScBaseCell* pUndoCell = (bUndo && pDocCell) ? pDocCell->CloneWithoutNote( *pDoc, rPos ) : 0;
    ScBaseCell* pRedoCell = (bUndo && pNewCell) ? pNewCell->CloneWithoutNote( *pDoc, rPos ) : 0;

    pDoc->PutCell( rPos, pNewCell );

    //  wegen ChangeTracking darf UndoAction erst nach PutCell angelegt werden
    if (bUndo)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoPutCell( &rDocShell, rPos, pUndoCell, pRedoCell, bHeight ) );
    }

    if (bHeight)
        AdjustRowHeight( ScRange(rPos) );

    if (!bXMLLoading)
        rDocShell.PostPaintCell( rPos );

    aModificator.SetDocumentModified();

    // #i925#; it is not neccessary to notify on loading a XML document
    // #103934#; notify editline and cell in edit mode
    if (bApi && !bXMLLoading)
        NotifyInputHandler( rPos );

    return sal_True;
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
            pViewSh->UpdateInputHandler(sal_False, !bIsEditMode);
        }
    }
}

        struct ScMyRememberItem
        {
            sal_uInt16      nIndex;
            SfxItemSet  aItemSet;

            ScMyRememberItem(const SfxItemSet& rItemSet, sal_uInt16 nTempIndex) :
                nIndex(nTempIndex), aItemSet(rItemSet) {}
        };

        typedef ::std::list<ScMyRememberItem*> ScMyRememberItemList;

sal_Bool ScDocFunc::PutData( const ScAddress& rPos, ScEditEngineDefaulter& rEngine, sal_Bool bInterpret, sal_Bool bApi )
{
    //  PutData ruft PutCell oder SetNormalString

    sal_Bool bRet = sal_False;
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
            rEngine.SetUpdateMode(sal_False);

        ScMyRememberItemList aRememberItems;
        ScMyRememberItem* pRememberItem = NULL;

        //  All paragraph attributes must be removed before calling CreateTextObject,
        //  not only alignment, so the object doesn't contain the cell attributes as
        //  paragraph attributes. Before remove the attributes store they in a list to
        //  set they back to the EditEngine.
        sal_uInt16 nCount = rEngine.GetParagraphCount();
        for (sal_uInt16 i=0; i<nCount; i++)
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

        EditTextObject* pNewData = rEngine.CreateTextObject();
        bRet = PutCell( rPos,
                        new ScEditCell( pNewData, pDoc, rEngine.GetEditTextObjectPool() ),
                        bApi );
        delete pNewData;

        // Set the paragraph attributes back to the EditEngine.
        if (!aRememberItems.empty())
        {
//            ScMyRememberItem* pRememberItem = NULL;
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
        String aText = rEngine.GetText();
        if ( bInterpret || !aText.Len() )
            bRet = SetNormalString( rPos, aText, bApi );
        else
            bRet = PutCell( rPos, new ScStringCell( aText ), bApi );
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


ScTokenArray* lcl_ScDocFunc_CreateTokenArrayXML( const String& rText, const String& rFormulaNmsp, const formula::FormulaGrammar::Grammar eGrammar )
{
    ScTokenArray* pCode = new ScTokenArray;
    pCode->AddString( rText );
    if( (eGrammar == formula::FormulaGrammar::GRAM_EXTERNAL) && (rFormulaNmsp.Len() > 0) )
        pCode->AddString( rFormulaNmsp );
    return pCode;
}


ScBaseCell* ScDocFunc::InterpretEnglishString( const ScAddress& rPos,
        const String& rText, const String& rFormulaNmsp, const formula::FormulaGrammar::Grammar eGrammar, short* pRetFormatType )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    ScBaseCell* pNewCell = NULL;

    if ( rText.Len() > 1 && rText.GetChar(0) == '=' )
    {
        ScTokenArray* pCode;
        if ( pDoc->IsImportingXML() )
        {   // temporary formula string as string tokens
            pCode = lcl_ScDocFunc_CreateTokenArrayXML( rText, rFormulaNmsp, eGrammar );
            pDoc->IncXMLImportedFormulaCount( rText.Len() );
        }
        else
        {
            ScCompiler aComp( pDoc, rPos );
            aComp.SetGrammar(eGrammar);
            pCode = aComp.CompileString( rText );
        }
        pNewCell = new ScFormulaCell( pDoc, rPos, pCode, eGrammar, MM_NONE );
        delete pCode;   // Zell-ctor hat das TokenArray kopiert
    }
    else if ( rText.Len() > 1 && rText.GetChar(0) == '\'' )
    {
        //  for bEnglish, "'" at the beginning is always interpreted as text
        //  marker and stripped
        pNewCell = ScBaseCell::CreateTextCell( rText.Copy( 1 ), pDoc );
    }
    else        // (nur) auf englisches Zahlformat testen
    {
        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        sal_uInt32 nEnglish = pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);
        double fVal;
        if ( pFormatter->IsNumberFormat( rText, nEnglish, fVal ) )
        {
            pNewCell = new ScValueCell( fVal );
            // return the format type from the English format, so a localized format can be created
            if ( pRetFormatType )
                *pRetFormatType = pFormatter->GetType( nEnglish );
        }
        else if ( rText.Len() )
            pNewCell = ScBaseCell::CreateTextCell( rText, pDoc );

        //  das (englische) Zahlformat wird nicht gesetzt
        //! passendes lokales Format suchen und setzen???
    }

    return pNewCell;
}


sal_Bool ScDocFunc::SetCellText( const ScAddress& rPos, const String& rText,
        sal_Bool bInterpret, sal_Bool bEnglish, sal_Bool bApi,
        const String& rFormulaNmsp, const formula::FormulaGrammar::Grammar eGrammar )
{
    //  SetCellText ruft PutCell oder SetNormalString

    ScDocument* pDoc = rDocShell.GetDocument();
    ScBaseCell* pNewCell = NULL;
    if ( bInterpret )
    {
        if ( bEnglish )
        {
            ::boost::scoped_ptr<ScExternalRefManager::ApiGuard> pExtRefGuard;
            if (bApi)
                pExtRefGuard.reset(new ScExternalRefManager::ApiGuard(pDoc));

            //  code moved to own method InterpretEnglishString because it is also used in
            //  ScCellRangeObj::setFormulaArray

            pNewCell = InterpretEnglishString( rPos, rText, rFormulaNmsp, eGrammar );
        }
        // sonst Null behalten -> SetString mit lokalen Formeln/Zahlformat
    }
    else if ( rText.Len() )
    {
        OSL_ENSURE( rFormulaNmsp.Len() == 0, "ScDocFunc::SetCellText - formula namespace, but do not interpret?" );
        pNewCell = ScBaseCell::CreateTextCell( rText, pDoc );   // immer Text
    }

    if (pNewCell)
        return PutCell( rPos, pNewCell, bApi );
    else
        return SetNormalString( rPos, rText, bApi );
}

//------------------------------------------------------------------------

bool ScDocFunc::ShowNote( const ScAddress& rPos, bool bShow )
{
    ScDocument& rDoc = *rDocShell.GetDocument();
    ScPostIt* pNote = rDoc.GetNote( rPos );
    if( !pNote || (bShow == pNote->IsCaptionShown()) ) return false;

    // move the caption to internal or hidden layer and create undo action
    pNote->ShowCaption( rPos, bShow );
    if( rDoc.IsUndoEnabled() )
        rDocShell.GetUndoManager()->AddUndoAction( new ScUndoShowHideNote( rDocShell, rPos, bShow ) );

    if (rDoc.IsStreamValid(rPos.Tab()))
        rDoc.SetStreamValid(rPos.Tab(), sal_False);

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

    String aNewText = rText;
    aNewText.ConvertLineEnd();      //! ist das noetig ???

    if( ScPostIt* pNote = (aNewText.Len() > 0) ? pDoc->GetOrCreateNote( rPos ) : pDoc->GetNote( rPos ) )
        pNote->SetText( rPos, aNewText );

    //! Undo !!!

    if (pDoc->IsStreamValid(rPos.Tab()))
        pDoc->SetStreamValid(rPos.Tab(), sal_False);

    rDocShell.PostPaintCell( rPos );
    aModificator.SetDocumentModified();

    return true;
}

//------------------------------------------------------------------------

bool ScDocFunc::ReplaceNote( const ScAddress& rPos, const String& rNoteText, const String* pAuthor, const String* pDate, sal_Bool bApi )
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
        ScPostIt* pOldNote = rDoc.ReleaseNote( rPos );
        if( pOldNote )
        {
            // ensure existing caption object before draw undo tracking starts
            pOldNote->GetOrCreateCaption( rPos );
            // rescue note data for undo
            aOldData = pOldNote->GetNoteData();
        }

        // collect drawing undo actions for deleting/inserting caption obejcts
        if( pUndoMgr )
            pDrawLayer->BeginCalcUndo();

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
            rDoc.SetStreamValid(rPos.Tab(), sal_False);

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
        bRecord = sal_False;

    sal_Bool bImportingXML = pDoc->IsImportingXML();
    // Cell formats can still be set if the range isn't editable only because of matrix formulas.
    // #i62483# When loading XML, the check can be skipped altogether.
    sal_Bool bOnlyNotBecauseOfMatrix;
    if ( !bImportingXML && !pDoc->IsSelectionEditable( rMark, &bOnlyNotBecauseOfMatrix )
            && !bOnlyNotBecauseOfMatrix )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return sal_False;
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

    // While loading XML it is not neccessary to ask HasAttrib. It needs too much time.
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
        bRecord = sal_False;

    sal_Bool bImportingXML = pDoc->IsImportingXML();
    // Cell formats can still be set if the range isn't editable only because of matrix formulas.
    // #i62483# When loading XML, the check can be skipped altogether.
    sal_Bool bOnlyNotBecauseOfMatrix;
    if ( !bImportingXML && !pDoc->IsSelectionEditable( rMark, &bOnlyNotBecauseOfMatrix )
            && !bOnlyNotBecauseOfMatrix )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return sal_False;
    }

    ScStyleSheet* pStyleSheet = (ScStyleSheet*) pDoc->GetStyleSheetPool()->Find(
                                                rStyleName, SFX_STYLE_FAMILY_PARA );
    if (!pStyleSheet)
        return sal_False;

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
        for (SCTAB i=0; i<nTabCount; i++)
            if (i != nStartTab && rMark.GetTableSelect(i))
                pUndoDoc->AddUndoTab( i, i );

        ScRange aCopyRange = aMultiRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, bMulti, pUndoDoc, &rMark );

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoSelectionStyle(
                    &rDocShell, rMark, aMultiRange, rStyleName, pUndoDoc ) );

    }

//  sal_Bool bPaintExt = pDoc->HasAttrib( aMultiRange, HASATTR_PAINTEXT );
//  pDoc->ApplySelectionPattern( rPattern, rMark );

    pDoc->ApplySelectionStyle( (ScStyleSheet&)*pStyleSheet, rMark );

//  if (!bPaintExt)
//      bPaintExt = pDoc->HasAttrib( aMultiRange, HASATTR_PAINTEXT );
//  sal_uInt16 nExtFlags = bPaintExt ? SC_PF_LINES : 0;
    sal_uInt16 nExtFlags = 0;
    if (!AdjustRowHeight( aMultiRange ))
        rDocShell.PostPaint( aMultiRange, PAINT_GRID, nExtFlags );
    else if (nExtFlags & SC_PF_LINES)
        lcl_PaintAbove( rDocShell, aMultiRange );   // fuer Linien ueber dem Bereich

    aModificator.SetDocumentModified();

    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::InsertCells( const ScRange& rRange, const ScMarkData* pTabMark, InsCellCmd eCmd,
                                sal_Bool bRecord, sal_Bool bApi, sal_Bool bPartOfPaste )
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
        DBG_ERROR("invalid row in InsertCells");
        return sal_False;
    }

    ScDocument* pDoc = rDocShell.GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    SCCOL nPaintStartX = nStartCol;
    SCROW nPaintStartY = nStartRow;
    SCCOL nPaintEndX = nEndCol;
    SCROW nPaintEndY = nEndRow;
    sal_uInt16 nPaintFlags = PAINT_GRID;
    sal_Bool bSuccess;
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
        bRecord = sal_False;

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
                    aMark.SelectTable( i, sal_True );
                    break;
                }
            }
        }
    }

    ScMarkData aFullMark( aMark );          // including scenario sheets
    for( i=0; i<nTabCount; i++ )
        if( aMark.GetTableSelect( i ) )
        {
            for( SCTAB j = i+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                aFullMark.SelectTable( j, sal_True );
        }

    SCTAB nSelCount = aMark.GetSelectCount();

    //  zugehoerige Szenarien auch anpassen
    // Test zusammengefasste

    SCCOL nMergeTestStartX = nStartCol;
    SCROW nMergeTestStartY = nStartRow;
    SCCOL nMergeTestEndX = nEndCol;
    SCROW nMergeTestEndY = nEndRow;

    ScRange aExtendMergeRange( rRange );

    if( rRange.aStart == rRange.aEnd && pDoc->HasAttrib(rRange, HASATTR_MERGED) )
    {
        pDoc->ExtendMerge( aExtendMergeRange );
        pDoc->ExtendOverlapped( aExtendMergeRange );
        nMergeTestEndX = aExtendMergeRange.aEnd.Col();
        nMergeTestEndY = aExtendMergeRange.aEnd.Row();
        nPaintEndX = nMergeTestEndX;
        nPaintEndY = nMergeTestEndY;
    }

    if ( eCmd == INS_INSROWS )
    {
        nMergeTestStartX = 0;
        nMergeTestEndX = MAXCOL;
    }
    if ( eCmd == INS_INSCOLS )
    {
        nMergeTestStartY = 0;
        nMergeTestEndY = MAXROW;
    }
    if ( eCmd == INS_CELLSDOWN )
        nMergeTestEndY = MAXROW;
    if ( eCmd == INS_CELLSRIGHT )
        nMergeTestEndX = MAXCOL;

    sal_Bool bNeedRefresh = sal_False;

    SCCOL nEditTestEndX = (eCmd==INS_INSCOLS) ? MAXCOL : nMergeTestEndX;
    SCROW nEditTestEndY = (eCmd==INS_INSROWS) ? MAXROW : nMergeTestEndY;
    ScEditableTester aTester( pDoc, nMergeTestStartX, nMergeTestStartY, nEditTestEndX, nEditTestEndY, aMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return sal_False;
    }

    WaitObject aWait( rDocShell.GetActiveDialogParent() );      // wichtig wegen TrackFormulas bei UpdateReference

    ScDocument* pRefUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if ( bRecord )
    {
        pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1, sal_False, sal_False );

        // pRefUndoDoc is filled in InsertCol / InsertRow

        pUndoData = new ScRefUndoData( pDoc );

        pDoc->BeginDrawUndo();
    }

    // #i8302 : we unmerge overwhelming ranges, before insertion all the actions are put in the same ListAction
    // the patch comes from mloiseleur and maoyg
    sal_Bool bInsertMerge = sal_False;
    std::vector<ScRange> qIncreaseRange;
    String aUndo = ScGlobal::GetRscString( STR_UNDO_INSERTCELLS );
    if (bRecord)
        rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo );

    for( i=0; i<nTabCount; i++ )
    {
        if( aMark.GetTableSelect(i) )
        {
            if( pDoc->HasAttrib( nMergeTestStartX, nMergeTestStartY, i, nMergeTestEndX, nMergeTestEndY, i, HASATTR_MERGED | HASATTR_OVERLAPPED ) )
            {
                if (eCmd==INS_CELLSRIGHT)
                    bNeedRefresh = sal_True;

                SCCOL nMergeStartX = nMergeTestStartX;
                SCROW nMergeStartY = nMergeTestStartY;
                SCCOL nMergeEndX   = nMergeTestEndX;
                SCROW nMergeEndY   = nMergeTestEndY;

                pDoc->ExtendMerge( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, i );
                pDoc->ExtendOverlapped( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, i );

                if(( eCmd == INS_CELLSDOWN && ( nMergeStartX != nMergeTestStartX || nMergeEndX != nMergeTestEndX )) ||
                    (eCmd == INS_CELLSRIGHT && ( nMergeStartY != nMergeTestStartY || nMergeEndY != nMergeTestEndY )) )
                {
                    if (!bApi)
                        rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0);
                    rDocShell.GetUndoManager()->LeaveListAction();
                    return sal_False;
                }

                SCCOL nTestCol = -1;
                SCROW nTestRow1 = -1;
                SCROW nTestRow2 = -1;

                ScDocAttrIterator aTestIter( pDoc, i, nMergeTestStartX, nMergeTestStartY, nMergeTestEndX, nMergeTestEndY );
                ScRange aExtendRange( nMergeTestStartX, nMergeTestStartY, i, nMergeTestEndX, nMergeTestEndY, i );
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
                        pDoc->ExtendMerge(aRange, sal_True, sal_True);

                        if( nTestRow1 < nTestRow2 && nNewFlags == SC_MF_HOR )
                        {
                            for( SCROW nTestRow = nTestRow1; nTestRow <= nTestRow2; nTestRow++ )
                            {
                                ScRange aTestRange( nTestCol, nTestRow, i );
                                pDoc->ExtendOverlapped( aTestRange );
                                pDoc->ExtendMerge( aTestRange, sal_True, sal_True);
                                ScRange aMergeRange( aTestRange.aStart.Col(),aTestRange.aStart.Row(), i );
                                if( !aExtendRange.In( aMergeRange ) )
                                {
                                    qIncreaseRange.push_back( aTestRange );
                                    bInsertMerge = sal_True;
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
                            bInsertMerge = sal_True;
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
                            nEndCol = nMergeTestEndX;
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
                            nEndRow = nMergeTestEndY;
                        }
                        else
                        {
                            nStartRow = 0;
                            nEndRow = MAXROW;
                        }
                    }

                    if( !qIncreaseRange.empty() )
                    {
                        for( ::std::vector<ScRange>::const_iterator iIter( qIncreaseRange.begin()); iIter != qIncreaseRange.end(); iIter++ )
                        {
                            ScRange aRange( *iIter );
                            if( pDoc->HasAttrib( aRange, HASATTR_OVERLAPPED | HASATTR_MERGED ) )
                            {
                                UnmergeCells( aRange, sal_True, sal_True );
                            }
                        }
                    }
                }
                else
                {
                    if (!bApi)
                        rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0);
                    rDocShell.GetUndoManager()->LeaveListAction();
                    return sal_False;
                }
            }
        }
    }

    switch (eCmd)
    {
        case INS_CELLSDOWN:
            bSuccess = pDoc->InsertRow( nStartCol, 0, nEndCol, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc, &aFullMark );
            nPaintEndY = MAXROW;
            break;
        case INS_INSROWS:
            bSuccess = pDoc->InsertRow( 0, 0, MAXCOL, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc, &aFullMark );
            nPaintStartX = 0;
            nPaintEndX = MAXCOL;
            nPaintEndY = MAXROW;
            nPaintFlags |= PAINT_LEFT;
            break;
        case INS_CELLSRIGHT:
            bSuccess = pDoc->InsertCol( nStartRow, 0, nEndRow, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc, &aFullMark );
            nPaintEndX = MAXCOL;
            break;
        case INS_INSCOLS:
            bSuccess = pDoc->InsertCol( 0, 0, MAXROW, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc, &aFullMark );
            nPaintStartY = 0;
            nPaintEndY = MAXROW;
            nPaintEndX = MAXCOL;
            nPaintFlags |= PAINT_TOP;
            break;
        default:
            DBG_ERROR("Falscher Code beim Einfuegen");
            bSuccess = sal_False;
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
            for( i=0; i<nTabCount; i++ )
            {
                if( aMark.GetTableSelect( i ) )
                {
                    SCTAB nCount = 0;
                    for( SCTAB j=i+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                        nCount ++;

                    pScenarios[nUndoPos] = nCount;
                    pTabs[nUndoPos] = i;
                    nUndoPos ++;
                }
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
                MergeCells(aRange, sal_False, sal_True, sal_True);
            }
            qIncreaseRange.pop_back();
        }

        if( bInsertMerge )
            rDocShell.GetUndoManager()->LeaveListAction();

        for( i=0; i<nTabCount; i++ )
        {
            if( aMark.GetTableSelect( i ) )
            {
                if (bNeedRefresh)
                    pDoc->ExtendMerge( nMergeTestStartX, nMergeTestStartY, nMergeTestEndX, nMergeTestEndY, i, sal_True );
                else
                    pDoc->RefreshAutoFilter( nMergeTestStartX, nMergeTestStartY, nMergeTestEndX, nMergeTestEndY, i );

                if ( eCmd == INS_INSROWS || eCmd == INS_INSCOLS )
                    pDoc->UpdatePageBreaks( i );

                sal_uInt16 nExtFlags = 0;
                rDocShell.UpdatePaintExt( nExtFlags, nPaintStartX, nPaintStartY, i, nPaintEndX, nPaintEndY, i );

                SCTAB nScenarioCount = 0;

                for( SCTAB j = i+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                    nScenarioCount ++;

                sal_Bool bAdjusted = ( eCmd == INS_INSROWS ) ? AdjustRowHeight(ScRange(0, nStartRow, i, MAXCOL, nEndRow, i+nScenarioCount )) :
                                                           AdjustRowHeight(ScRange(0, nPaintStartY, i, MAXCOL, nPaintEndY, i+nScenarioCount ));
                if (bAdjusted)
                {
                    //  paint only what is not done by AdjustRowHeight
                    if (nPaintFlags & PAINT_TOP)
                        rDocShell.PostPaint( nPaintStartX, nPaintStartY, i, nPaintEndX, nPaintEndY, i+nScenarioCount, PAINT_TOP );
                }
                else
                    rDocShell.PostPaint( nPaintStartX, nPaintStartY, i, nPaintEndX, nPaintEndY, i+nScenarioCount, nPaintFlags, nExtFlags );
            }
        }
        //aModificator.SetDocumentModified();
    }
    else
    {
        if( bInsertMerge )
        {
            while( !qIncreaseRange.empty() )
            {
                ScRange aRange = qIncreaseRange.back();
                MergeCells(aRange, sal_False, sal_True, sal_True);
                qIncreaseRange.pop_back();
            }

            if( pViewSh )
            {
                pViewSh->MarkRange( rRange, sal_False );
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

sal_Bool ScDocFunc::DeleteCells( const ScRange& rRange, const ScMarkData* pTabMark, DelCellCmd eCmd,
                             sal_Bool bRecord, sal_Bool bApi )
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
        DBG_ERROR("invalid row in DeleteCells");
        return sal_False;
    }

    ScDocument* pDoc = rDocShell.GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    SCCOL nPaintStartX = nStartCol;
    SCROW nPaintStartY = nStartRow;
    SCCOL nPaintEndX = nEndCol;
    SCROW nPaintEndY = nEndRow;
    sal_uInt16 nPaintFlags = PAINT_GRID;
    SCTAB i;

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;

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
                    aMark.SelectTable( i, sal_True );
                    break;
                }
            }
        }
    }

    ScMarkData aFullMark( aMark );          // including scenario sheets
    for( i=0; i<nTabCount; i++ )
        if( aMark.GetTableSelect( i ) )
        {
            for( SCTAB j = i+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                aFullMark.SelectTable( j, sal_True );
        }

    SCTAB nSelCount = aMark.GetSelectCount();

    SCCOL nUndoStartX = nStartCol;
    SCROW nUndoStartY = nStartRow;
    SCCOL nUndoEndX = nEndCol;
    SCROW nUndoEndY = nEndRow;

    ScRange aExtendMergeRange( rRange );

    if( rRange.aStart == rRange.aEnd && pDoc->HasAttrib(rRange, HASATTR_MERGED) )
    {
        pDoc->ExtendMerge( aExtendMergeRange );
        pDoc->ExtendOverlapped( aExtendMergeRange );
        nUndoEndX = aExtendMergeRange.aEnd.Col();
        nUndoEndY = aExtendMergeRange.aEnd.Row();
        nPaintEndX = nUndoEndX;
        nPaintEndY = nUndoEndY;
    }

    if (eCmd==DEL_DELROWS)
    {
        nUndoStartX = 0;
        nUndoEndX = MAXCOL;
    }
    if (eCmd==DEL_DELCOLS)
    {
        nUndoStartY = 0;
        nUndoEndY = MAXROW;
    }
                    // Test Zellschutz

    SCCOL nEditTestEndX = nUndoEndX;
    if ( eCmd==DEL_DELCOLS || eCmd==DEL_CELLSLEFT )
        nEditTestEndX = MAXCOL;
    SCROW nEditTestEndY = nUndoEndY;
    if ( eCmd==DEL_DELROWS || eCmd==DEL_CELLSUP )
        nEditTestEndY = MAXROW;
    ScEditableTester aTester( pDoc, nUndoStartX, nUndoStartY, nEditTestEndX, nEditTestEndY, aMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return sal_False;
    }

                    // Test zusammengefasste

    SCCOL nMergeTestEndX = (eCmd==DEL_CELLSLEFT) ? MAXCOL : nUndoEndX;
    SCROW nMergeTestEndY = (eCmd==DEL_CELLSUP)   ? MAXROW : nUndoEndY;
    SCCOL nExtendStartCol = nUndoStartX;
    SCROW nExtendStartRow = nUndoStartY;
    sal_Bool bNeedRefresh = sal_False;

    //Issue 8302 want to be able to insert into the middle of merged cells
    //the patch comes from maoyg
    ::std::vector<ScRange> qDecreaseRange;
    sal_Bool bDeletingMerge = sal_False;
    String aUndo = ScGlobal::GetRscString( STR_UNDO_DELETECELLS );
    if (bRecord)
        rDocShell.GetUndoManager()->EnterListAction( aUndo, aUndo );

    for( i=0; i<nTabCount; i++ )
    {
        if( aMark.GetTableSelect(i) )
        {
            if ( pDoc->HasAttrib( nUndoStartX, nUndoStartY, i, nMergeTestEndX, nMergeTestEndY, i, HASATTR_MERGED | HASATTR_OVERLAPPED ))
            {
                SCCOL nMergeStartX = nUndoStartX;
                SCROW nMergeStartY = nUndoStartY;
                SCCOL nMergeEndX   = nMergeTestEndX;
                SCROW nMergeEndY   = nMergeTestEndY;

                pDoc->ExtendMerge( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, i );
                pDoc->ExtendOverlapped( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, i );
                if( ( eCmd == DEL_CELLSUP && ( nMergeStartX != nUndoStartX || nMergeEndX != nMergeTestEndX))||
                    ( eCmd == DEL_CELLSLEFT && ( nMergeStartY != nUndoStartY || nMergeEndY != nMergeTestEndY)))
                {
                    if (!bApi)
                        rDocShell.ErrorMessage(STR_MSSG_DELETECELLS_0);
                    rDocShell.GetUndoManager()->LeaveListAction();
                    return sal_False;
                }

                nExtendStartCol = nMergeStartX;
                nExtendStartRow = nMergeStartY;
                SCCOL nTestCol = -1;
                SCROW nTestRow1 = -1;
                SCROW nTestRow2 = -1;

                ScDocAttrIterator aTestIter( pDoc, i, nUndoStartX, nUndoStartY, nMergeTestEndX, nMergeTestEndY );
                ScRange aExtendRange( nUndoStartX, nUndoStartY, i, nMergeTestEndX, nMergeTestEndY, i );
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
                        pDoc->ExtendMerge( aRange, sal_True, sal_True );

                        if( nTestRow1 < nTestRow2 && nNewFlags == SC_MF_HOR )
                        {
                            for( SCROW nTestRow = nTestRow1; nTestRow <= nTestRow2; nTestRow++ )
                            {
                                ScRange aTestRange( nTestCol, nTestRow, i );
                                pDoc->ExtendOverlapped( aTestRange );
                                pDoc->ExtendMerge( aTestRange, sal_True, sal_True);
                                ScRange aMergeRange( aTestRange.aStart.Col(),aTestRange.aStart.Row(), i );
                                if( !aExtendRange.In( aMergeRange ) )
                                {
                                    qDecreaseRange.push_back( aTestRange );
                                    bDeletingMerge = sal_True;
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
                            bDeletingMerge = sal_True;
                        }
                    }
                }

                if( bDeletingMerge )
                {

                    if( eCmd == DEL_DELROWS || eCmd == DEL_CELLSUP )
                    {
                        nStartRow = aExtendMergeRange.aStart.Row();
                        nEndRow = aExtendMergeRange.aEnd.Row();
                        bNeedRefresh = sal_True;

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
                            bNeedRefresh = sal_True;
                        }
                        else
                        {
                            nStartRow = 0;
                            nEndRow = MAXROW;
                        }
                    }

                    if( !qDecreaseRange.empty() )
                    {
                        for( ::std::vector<ScRange>::const_iterator iIter( qDecreaseRange.begin()); iIter != qDecreaseRange.end(); iIter++ )
                        {
                            ScRange aRange( *iIter );
                            if( pDoc->HasAttrib( aRange, HASATTR_OVERLAPPED | HASATTR_MERGED ) )
                            {
                                UnmergeCells( aRange, sal_True, sal_True );
                            }
                        }
                    }
                }
                else
                {
                    if (!bApi)
                        rDocShell.ErrorMessage(STR_MSSG_DELETECELLS_0);
                    rDocShell.GetUndoManager()->LeaveListAction();
                    return sal_False;
                }
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
        for( i=0; i<nTabCount; i++ )
        {
            if( aMark.GetTableSelect( i ) )
            {
                SCTAB nScenarioCount = 0;

                for( SCTAB j = i+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                    nScenarioCount ++;

                pDoc->CopyToDocument( nUndoStartX, nUndoStartY, i, nUndoEndX, nUndoEndY, i+nScenarioCount,
                    IDF_ALL | IDF_NOCAPTIONS, sal_False, pUndoDoc );
            }
        }

        pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1, sal_False, sal_False );

        pUndoData = new ScRefUndoData( pDoc );

        pDoc->BeginDrawUndo();
    }

    sal_uInt16 nExtFlags = 0;
    for( i=0; i<nTabCount; i++ )
    {
        if( aMark.GetTableSelect( i ) )
            rDocShell.UpdatePaintExt( nExtFlags, nStartCol, nStartRow, i, nEndCol, nEndRow, i );
    }

    sal_Bool bUndoOutline = sal_False;
    switch (eCmd)
    {
        case DEL_CELLSUP:
            pDoc->DeleteRow( nStartCol, 0, nEndCol, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc, NULL, &aFullMark );
            nPaintEndY = MAXROW;
            break;
        case DEL_DELROWS:
            pDoc->DeleteRow( 0, 0, MAXCOL, MAXTAB, nStartRow, static_cast<SCSIZE>(nEndRow-nStartRow+1), pRefUndoDoc, &bUndoOutline, &aFullMark );
            nPaintStartX = 0;
            nPaintEndX = MAXCOL;
            nPaintEndY = MAXROW;
            nPaintFlags |= PAINT_LEFT;
            break;
        case DEL_CELLSLEFT:
            pDoc->DeleteCol( nStartRow, 0, nEndRow, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc, NULL, &aFullMark );
            nPaintEndX = MAXCOL;
            break;
        case DEL_DELCOLS:
            pDoc->DeleteCol( 0, 0, MAXROW, MAXTAB, nStartCol, static_cast<SCSIZE>(nEndCol-nStartCol+1), pRefUndoDoc, &bUndoOutline, &aFullMark );
            nPaintStartY = 0;
            nPaintEndY = MAXROW;
            nPaintEndX = MAXCOL;
            nPaintFlags |= PAINT_TOP;
            break;
        default:
            DBG_ERROR("Falscher Code beim Loeschen");
            break;
    }

    //! Test, ob Outline in Groesse geaendert

    if ( bRecord )
    {
        for( i=0; i<nTabCount; i++ )
            if( aFullMark.GetTableSelect( i ) )
                pRefUndoDoc->DeleteAreaTab(nUndoStartX,nUndoStartY,nUndoEndX,nUndoEndY, i, IDF_ALL);

            //  alle Tabellen anlegen, damit Formeln kopiert werden koennen:
        pUndoDoc->AddUndoTab( 0, nTabCount-1, sal_False, sal_False );

            //  kopieren mit bColRowFlags=sal_False (#54194#)
        pRefUndoDoc->CopyToDocument(0,0,0,MAXCOL,MAXROW,MAXTAB,IDF_FORMULA,sal_False,pUndoDoc,NULL,sal_False);
        delete pRefUndoDoc;

        SCTAB* pTabs      = new SCTAB[nSelCount];
        SCTAB* pScenarios = new SCTAB[nSelCount];
        SCTAB   nUndoPos  = 0;

        for( i=0; i<nTabCount; i++ )
        {
            if( aMark.GetTableSelect( i ) )
            {
                SCTAB nCount = 0;
                for( SCTAB j=i+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                    nCount ++;

                pScenarios[nUndoPos] = nCount;
                pTabs[nUndoPos] = i;
                nUndoPos ++;
            }
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
            MergeCells( aRange, sal_False, sal_True, sal_True );
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
            nMergeTestEndX = MAXCOL;
        if ( eCmd==DEL_DELROWS || eCmd==DEL_CELLSUP )
            nMergeTestEndY = MAXROW;
        ScPatternAttr aPattern( pDoc->GetPool() );
        aPattern.GetItemSet().Put( ScMergeFlagAttr() );

        pDoc->ApplyPatternArea( nExtendStartCol, nExtendStartRow, nMergeTestEndX, nMergeTestEndY, aMark, aPattern );

        for( i=0; i<nTabCount; i++ )
        {
            if( aMark.GetTableSelect( i ) )
            {
                SCTAB nScenarioCount = 0;

                for( SCTAB j = i+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                    nScenarioCount ++;

                ScRange aMergedRange( nExtendStartCol, nExtendStartRow, i, nMergeTestEndX, nMergeTestEndY, i+nScenarioCount );
                pDoc->ExtendMerge( aMergedRange, sal_True );
            }
        }
    }

    for( i=0; i<nTabCount; i++ )
    {
        if( aMark.GetTableSelect( i ) )
        {
            if ( eCmd == DEL_DELCOLS || eCmd == DEL_DELROWS )
                pDoc->UpdatePageBreaks( i );

            rDocShell.UpdatePaintExt( nExtFlags, nPaintStartX, nPaintStartY, i, nPaintEndX, nPaintEndY, i );

            SCTAB nScenarioCount = 0;

            for( SCTAB j = i+1; j<nTabCount && pDoc->IsScenario(j); j++ )
                nScenarioCount ++;

            //  ganze Zeilen loeschen: nichts anpassen
            if ( eCmd == DEL_DELROWS || !AdjustRowHeight(ScRange( 0, nPaintStartY, i, MAXCOL, nPaintEndY, i+nScenarioCount )) )
                rDocShell.PostPaint( nPaintStartX, nPaintStartY, i, nPaintEndX, nPaintEndY, i+nScenarioCount, nPaintFlags,  nExtFlags );
            else
            {
                //  paint only what is not done by AdjustRowHeight
                if (nExtFlags & SC_PF_LINES)
                    lcl_PaintAbove( rDocShell, ScRange( nPaintStartX, nPaintStartY, i, nPaintEndX, nPaintEndY, i+nScenarioCount) );
                if (nPaintFlags & PAINT_TOP)
                    rDocShell.PostPaint( nPaintStartX, nPaintStartY, i, nPaintEndX, nPaintEndY, i+nScenarioCount, PAINT_TOP );
            }
        }
    }
    aModificator.SetDocumentModified();

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );

    return sal_True;
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
        DBG_ERROR("invalid row in MoveBlock");
        return sal_False;
    }

    //  zugehoerige Szenarien auch anpassen - nur wenn innerhalb einer Tabelle verschoben wird!
    sal_Bool bScenariosAdded = sal_False;
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;

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

    ScClipParam aClipParam(ScRange(nStartCol, nStartRow, 0, nEndCol, nEndRow, 0), bCut);
    pDoc->CopyToClip(aClipParam, pClipDoc, &aSourceMark, false, bScenariosAdded, true);

    ScDrawLayer::SetGlobalDrawPersist(NULL);

    SCCOL nOldEndCol = nEndCol;
    SCROW nOldEndRow = nEndRow;
    sal_Bool bClipOver = sal_False;
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
        pClipDoc->GetClipArea( nClipX, nClipY, sal_False );
        SCROW nUndoAdd = nUndoEndRow - nDestEndRow;
        nDestEndRow = nDestRow + nClipY;
        nUndoEndRow = nDestEndRow + nUndoAdd;
    }

    if (!ValidCol(nUndoEndCol) || !ValidRow(nUndoEndRow))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PASTE_FULL);
        delete pClipDoc;
        return sal_False;
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
        return sal_False;
    }

    //  Test auf zusammengefasste - beim Verschieben erst nach dem Loeschen

    if (bClipOver && !bCut)
        if (pDoc->HasAttrib( nDestCol,nDestRow,nDestTab, nUndoEndCol,nUndoEndRow,nDestEndTab,
                                HASATTR_MERGED | HASATTR_OVERLAPPED ))
        {       // "Zusammenfassen nicht verschachteln !"
            if (!bApi)
                rDocShell.ErrorMessage(STR_MSSG_MOVEBLOCKTO_0);
            delete pClipDoc;
            return sal_False;
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
                                    nUndoFlags, sal_False, pUndoDoc );
            pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1, sal_False, sal_False );
        }

        if ( nDestTab != nStartTab )
            pUndoDoc->AddUndoTab( nDestTab, nDestEndTab, bWholeCols, bWholeRows );
        pDoc->CopyToDocument( nDestCol, nDestRow, nDestTab,
                                    nDestEndCol, nDestEndRow, nDestEndTab,
                                    nUndoFlags, sal_False, pUndoDoc );

        pUndoData = new ScRefUndoData( pDoc );

        pDoc->BeginDrawUndo();
    }

    sal_Bool bSourceHeight = sal_False;     // Hoehen angepasst?
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
                return sal_False;
            }

        bSourceHeight = AdjustRowHeight( rSource, sal_False );
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
                        pRefUndoDoc, pClipDoc, sal_True, sal_False, bIncludeFiltered );

    // skipped rows and merged cells don't mix
    if ( !bIncludeFiltered && pClipDoc->HasClipFilteredRows() )
        UnmergeCells( aPasteDest, sal_False, sal_True );

    VirtualDevice aVirtDev;
    sal_Bool bDestHeight = AdjustRowHeight(
                            ScRange( 0,nDestRow,nDestTab, MAXCOL,nDestEndRow,nDestEndTab ),
                            sal_False );

    /*  Paste cell notes and drawing objects after adjusting formula references
        and row heights. There are no cell notes or drawing objects, if the
        clipdoc does not contain a drawing layer.
        #i102056# Passing IDF_NOTE only would overwrite cell contents with
        empty note cells, therefore the special modifier IDF_ADDNOTES is passed
        here too which changes the behaviour of ScColumn::CopyFromClip() to not
        touch existing cells. */
    if ( pClipDoc->GetDrawLayer() )
        pDoc->CopyFromClip( aPasteDest, aDestMark, IDF_NOTE | IDF_ADDNOTES | IDF_OBJECTS,
                            pRefUndoDoc, pClipDoc, sal_True, sal_False, bIncludeFiltered );

    if (bRecord)
    {
        if (pRefUndoDoc)
        {
                //  alle Tabellen anlegen, damit Formeln kopiert werden koennen:
            pUndoDoc->AddUndoTab( 0, nTabCount-1, sal_False, sal_False );

            pRefUndoDoc->DeleteArea( nDestCol, nDestRow, nDestEndCol, nDestEndRow, aSourceMark, IDF_ALL );
            //  kopieren mit bColRowFlags=sal_False (#54194#)
            pRefUndoDoc->CopyToDocument( 0, 0, 0, MAXCOL, MAXROW, MAXTAB,
                                            IDF_FORMULA, sal_False, pUndoDoc, NULL, sal_False );
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
        xVBACodeNamedObjectAccess.set( xSF->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBAObjectModuleObjectProvider"))), uno::UNO_QUERY );
        xDocModuleApiObject.set( xVBACodeNamedObjectAccess->getByName( sCodeName ), uno::UNO_QUERY );
    }
    return xDocModuleApiObject;

}

script::ModuleInfo lcl_InitModuleInfo( SfxObjectShell& rDocSh, String& sModule )
{
    script::ModuleInfo sModuleInfo;
    sModuleInfo.ModuleType = script::ModuleType::DOCUMENT;
    sModuleInfo.ModuleObject = GetDocModuleObject( rDocSh, sModule );
    return sModuleInfo;
}

void VBA_InsertModule( ScDocument& rDoc, SCTAB nTab, String& sModuleName, String& sSource )
{
    SfxObjectShell& rDocSh = *rDoc.GetDocumentShell();
    uno::Reference< script::XLibraryContainer > xLibContainer = rDocSh.GetBasicContainer();
    DBG_ASSERT( xLibContainer.is(), "No BasicContainer!" );

    uno::Reference< container::XNameContainer > xLib;
    if( xLibContainer.is() )
    {
        String aLibName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
        if ( rDocSh.GetBasicManager() && rDocSh.GetBasicManager()->GetName().Len() )
            aLibName = rDocSh.GetBasicManager()->GetName();
        uno::Any aLibAny = xLibContainer->getByName( aLibName );
        aLibAny >>= xLib;
    }
    if( xLib.is() )
    {
        // if the Module with codename exists then find a new name
        sal_Int32 nNum = 0;
        String genModuleName;
        if ( sModuleName.Len() )
            sModuleName = sModuleName;
        else
        {
             genModuleName = String::CreateFromAscii( "Sheet1" );
             nNum = 1;
        }
        while( xLib->hasByName( genModuleName  ) )
            genModuleName = rtl::OUString::createFromAscii( "Sheet" ) + rtl::OUString::valueOf( ++nNum );

        uno::Any aSourceAny;
        rtl::OUString sTmpSource = sSource;
        if ( sTmpSource.getLength() == 0 )
            sTmpSource = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Rem Attribute VBA_ModuleType=VBADocumentModule\nOption VBASupport 1\n" ));
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

void VBA_DeleteModule( ScDocShell& rDocSh, String& sModuleName )
{
    uno::Reference< script::XLibraryContainer > xLibContainer = rDocSh.GetBasicContainer();
    DBG_ASSERT( xLibContainer.is(), "No BasicContainer!" );

    uno::Reference< container::XNameContainer > xLib;
    if( xLibContainer.is() )
    {
        String aLibName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
        if ( rDocSh.GetBasicManager() && rDocSh.GetBasicManager()->GetName().Len() )
            aLibName = rDocSh.GetBasicManager()->GetName();
        uno::Any aLibAny = xLibContainer->getByName( aLibName );
        aLibAny >>= xLib;
    }
    if( xLib.is() )
    {
        uno::Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( xLib, uno::UNO_QUERY );
        if( xLib->hasByName( sModuleName ) )
            xLib->removeByName( sModuleName );
        if ( xVBAModuleInfo.is() )
            xVBAModuleInfo->removeModuleInfo( sModuleName );

    }
}


sal_Bool ScDocFunc::InsertTable( SCTAB nTab, const String& rName, sal_Bool bRecord, sal_Bool bApi )
{
    sal_Bool bSuccess = sal_False;
    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();


    // Strange loop, also basic is loaded too early ( InsertTable )
    // is called via the xml import for sheets in described in odf
    sal_Bool bInsertDocModule = false;

    if(  !rDocShell.GetDocument()->IsImportingXML() )
    {
        bInsertDocModule = pDoc ? pDoc->IsInVBAMode() : false;
    }
    if ( bInsertDocModule || ( bRecord && !pDoc->IsUndoEnabled() ) )
        bRecord = sal_False;

    if (bRecord)
        pDoc->BeginDrawUndo();                          //  InsertTab erzeugt ein SdrUndoNewPage

    SCTAB nTabCount = pDoc->GetTableCount();
    sal_Bool bAppend = ( nTab >= nTabCount );
    if ( bAppend )
        nTab = nTabCount;       // wichtig fuer Undo

    if (pDoc->InsertTab( nTab, rName ))
    {
        String sCodeName;
        if (bRecord)
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoInsertTab( &rDocShell, nTab, bAppend, rName));
        //  Views updaten:
        // Only insert vba modules if vba mode ( and not currently importing XML )
        if( bInsertDocModule )
        {
            String sSource;
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

    sal_Bool bSuccess = sal_False;
    ScDocument* pDoc = rDocShell.GetDocument();
    sal_Bool bVbaEnabled = pDoc ? pDoc->IsInVBAMode() : false;
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;
    if ( bVbaEnabled )
        bRecord = sal_False;
    sal_Bool bWasLinked = pDoc->IsLinked(nTab);
    ScDocument* pUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        SCTAB nCount = pDoc->GetTableCount();

        pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, sal_True );     // nur nTab mit Flags
        pUndoDoc->AddUndoTab( 0, nCount-1 );                    // alle Tabs fuer Referenzen

        pDoc->CopyToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, IDF_ALL,sal_False, pUndoDoc );
        String aOldName;
        pDoc->GetName( nTab, aOldName );
        pUndoDoc->RenameTab( nTab, aOldName, sal_False );
        if (bWasLinked)
            pUndoDoc->SetLink( nTab, pDoc->GetLinkMode(nTab), pDoc->GetLinkDoc(nTab),
                                pDoc->GetLinkFlt(nTab), pDoc->GetLinkOpt(nTab),
                                pDoc->GetLinkTab(nTab),
                                pDoc->GetLinkRefreshDelay(nTab) );

        if ( pDoc->IsScenario(nTab) )
        {
            pUndoDoc->SetScenario( nTab, sal_True );
            String aComment;
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

    String sCodeName;
    sal_Bool bHasCodeName = pDoc->GetCodeName( nTab, sCodeName );
    if (pDoc->DeleteTab( nTab, pUndoDoc ))
    {
        if (bRecord)
        {
            SvShorts theTabs;
            theTabs.push_back(nTab);
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDeleteTab( &rDocShell, theTabs, pUndoDoc, pUndoData ));
        }
        //  Views updaten:
        if( bVbaEnabled )
        {
            if( bHasCodeName )
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

sal_Bool ScDocFunc::SetTableVisible( SCTAB nTab, sal_Bool bVisible, sal_Bool bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    sal_Bool bUndo(pDoc->IsUndoEnabled());
    if ( pDoc->IsVisible( nTab ) == bVisible )
        return sal_True;                                // nichts zu tun - ok

    if ( !pDoc->IsDocEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return sal_False;
    }

    ScDocShellModificator aModificator( rDocShell );

    if ( !bVisible && !pDoc->IsImportingXML() )     // #i57869# allow hiding in any order for loading
    {
        //  nicht alle Tabellen ausblenden

        sal_uInt16 nVisCount = 0;
        SCTAB nCount = pDoc->GetTableCount();
        for (SCTAB i=0; i<nCount; i++)
            if (pDoc->IsVisible(i))
                ++nVisCount;

        if (nVisCount <= 1)
        {
            if (!bApi)
                rDocShell.ErrorMessage(STR_PROTECTIONERR);  //! eigene Meldung?
            return sal_False;
        }
    }

    pDoc->SetVisible( nTab, bVisible );
    if (bUndo)
        rDocShell.GetUndoManager()->AddUndoAction( new ScUndoShowHideTab( &rDocShell, nTab, bVisible ) );

    //  Views updaten:
    if (!bVisible)
        rDocShell.Broadcast( ScTablesHint( SC_TAB_HIDDEN, nTab ) );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    rDocShell.PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_EXTRAS);
    aModificator.SetDocumentModified();

    return sal_True;
}

sal_Bool ScDocFunc::SetLayoutRTL( SCTAB nTab, sal_Bool bRTL, sal_Bool /* bApi */ )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    sal_Bool bUndo(pDoc->IsUndoEnabled());
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

//UNUSED2009-05 sal_Bool ScDocFunc::SetGrammar( formula::FormulaGrammar::Grammar eGrammar )
//UNUSED2009-05 {
//UNUSED2009-05     ScDocument* pDoc = rDocShell.GetDocument();
//UNUSED2009-05
//UNUSED2009-05     if ( pDoc->GetGrammar() == eGrammar )
//UNUSED2009-05         return sal_True;
//UNUSED2009-05
//UNUSED2009-05     sal_Bool bUndo(pDoc->IsUndoEnabled());
//UNUSED2009-05     ScDocShellModificator aModificator( rDocShell );
//UNUSED2009-05
//UNUSED2009-05     pDoc->SetGrammar( eGrammar );
//UNUSED2009-05
//UNUSED2009-05     if (bUndo)
//UNUSED2009-05     {
//UNUSED2009-05         rDocShell.GetUndoManager()->AddUndoAction( new ScUndoSetGrammar( &rDocShell, eGrammar ) );
//UNUSED2009-05     }
//UNUSED2009-05
//UNUSED2009-05     rDocShell.PostPaint( 0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_ALL );
//UNUSED2009-05
//UNUSED2009-05     ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
//UNUSED2009-05     if (NULL != pViewSh)
//UNUSED2009-05     {
//UNUSED2009-05         pViewSh->UpdateInputHandler( sal_False, sal_False );
//UNUSED2009-05     }
//UNUSED2009-05
//UNUSED2009-05     aModificator.SetDocumentModified();
//UNUSED2009-05
//UNUSED2009-05     SfxBindings* pBindings = rDocShell.GetViewBindings();
//UNUSED2009-05     if (pBindings)
//UNUSED2009-05     {
//UNUSED2009-05         // erAck: 2006-09-07T22:19+0200  commented out in CWS scr1c1
//UNUSED2009-05         //pBindings->Invalidate( FID_TAB_USE_R1C1 );
//UNUSED2009-05     }
//UNUSED2009-05
//UNUSED2009-05     return sal_True;
//UNUSED2009-05 }

sal_Bool ScDocFunc::RenameTable( SCTAB nTab, const String& rName, sal_Bool bRecord, sal_Bool bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;
    if ( !pDoc->IsDocEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return sal_False;
    }

    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bSuccess = sal_False;
    String sOldName;
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

sal_uInt16 lcl_GetOptimalColWidth( ScDocShell& rDocShell, SCCOL nCol, SCTAB nTab, sal_Bool bFormula )
{
    sal_uInt16 nTwips = 0;

    ScSizeDeviceProvider aProv(&rDocShell);
    OutputDevice* pDev = aProv.GetDevice();         // has pixel MapMode
    double nPPTX = aProv.GetPPTX();
    double nPPTY = aProv.GetPPTY();

    ScDocument* pDoc = rDocShell.GetDocument();
    Fraction aOne(1,1);
    nTwips = pDoc->GetOptimalColWidth( nCol, nTab, pDev, nPPTX, nPPTY, aOne, aOne,
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
        bRecord = sal_False;

    // import into read-only document is possible
    if ( !pDoc->IsChangeReadOnlyEnabled() && !rDocShell.IsEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);      //! eigene Meldung?
        return sal_False;
    }

    sal_Bool bSuccess = sal_False;
    SCCOLROW nStart = pRanges[0];
    SCCOLROW nEnd = pRanges[2*nRangeCnt-1];

    sal_Bool bFormula = sal_False;
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
            pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, sal_False );
            pDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab, static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, sal_False, pUndoDoc );
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_False, sal_True );
            pDoc->CopyToDocument( 0, static_cast<SCROW>(nStart), nTab, MAXCOL, static_cast<SCROW>(nEnd), nTab, IDF_NONE, sal_False, pUndoDoc );
        }

        pUndoRanges = new SCCOLROW[ 2*nRangeCnt ];
        memmove( pUndoRanges, pRanges, 2*nRangeCnt*sizeof(SCCOLROW) );

        ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
        if (pTable)
            pUndoTab = new ScOutlineTable( *pTable );
    }

    sal_Bool bShow = nSizeTwips > 0 || eMode != SC_SIZE_DIRECT;
    sal_Bool bOutline = sal_False;

    pDoc->IncSizeRecalcLevel( nTab );       // nicht fuer jede Spalte einzeln
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
                        bool bHidden = pDoc->RowHidden(nRow, nTab, nLastRow);
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
                SCCOL nLastCol = -1;
                if ( eMode != SC_SIZE_VISOPT || !pDoc->ColHidden(nCol, nTab, nLastCol) )
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
    pDoc->DecSizeRecalcLevel( nTab );       // nicht fuer jede Spalte einzeln

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
        bRecord = sal_False;
    SCTAB nTab = rPos.Tab();
    SfxBindings* pBindings = rDocShell.GetViewBindings();

    SCCOLROW nPos = bColumn ? static_cast<SCCOLROW>(rPos.Col()) :
        static_cast<SCCOLROW>(rPos.Row());
    if (nPos == 0)
        return sal_False;                   // erste Spalte / Zeile

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
        pDoc->SetStreamValid(nTab, sal_False);

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
        bRecord = sal_False;
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
            new ScUndoPageBreak( &rDocShell, rPos.Col(), rPos.Row(), nTab, bColumn, sal_False ) );

    if (bColumn)
        pDoc->RemoveColBreak(static_cast<SCCOL>(nPos), nTab, false, true);
    else
        pDoc->RemoveRowBreak(static_cast<SCROW>(nPos), nTab, false, true);

    pDoc->UpdatePageBreaks( nTab );

    if (pDoc->IsStreamValid(nTab))
        pDoc->SetStreamValid(nTab, sal_False);

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
        DBG_ASSERT(pProtect, "ScDocFunc::Unprotect: ScTableProtection pointer is NULL!");
        if (pProtect)
        {
            ::std::auto_ptr<ScTableProtection> p(new ScTableProtection(*pProtect));
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
            DBG_ASSERT(pProtect, "ScDocFunc::Unprotect: ScDocProtection pointer is NULL!");
            if (pProtect)
            {
                ::std::auto_ptr<ScDocProtection> p(new ScDocProtection(*pProtect));
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
            DBG_ASSERT(pProtect, "ScDocFunc::Unprotect: ScTableProtection pointer is NULL!");
            if (pProtect)
            {
                ::std::auto_ptr<ScTableProtection> p(new ScTableProtection(*pProtect));
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
        ::std::auto_ptr<ScDocProtection> pProtectCopy(new ScDocProtection(*pDocProtect));

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
        ::std::auto_ptr<ScTableProtection> pProtectCopy(new ScTableProtection(*pTabProtect));
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
    sal_Bool bUndo (pDoc->IsUndoEnabled());
    ScEditableTester aTester( pDoc, rMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return sal_False;
    }

    //  #i12940# ClearItems is called (from setPropertyToDefault) directly with uno object's cached
    //  MarkData (GetMarkData), so rMark must be changed to multi selection for ClearSelectionItems
    //  here.

    ScRange aMarkRange;
    ScMarkData aMultiMark = rMark;
    aMultiMark.SetMarking(sal_False);       // for MarkToMulti
    aMultiMark.MarkToMulti();
    aMultiMark.GetMultiMarkArea( aMarkRange );

//  if (bRecord)
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
    sal_Bool bUndo(pDoc->IsUndoEnabled());
    ScEditableTester aTester( pDoc, rMark );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return sal_False;
    }

    ScRange aMarkRange;
    rMark.GetMultiMarkArea( aMarkRange );

//  if (bRecord)
    if (bUndo)
    {
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCTAB nTabCount = pDoc->GetTableCount();

        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
        for (SCTAB i=0; i<nTabCount; i++)
            if (i != nStartTab && rMark.GetTableSelect(i))
                pUndoDoc->AddUndoTab( i, i );

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
        // pseudo slots for Format menu
        pBindings->Invalidate( SID_ALIGN_ANY_HDEFAULT );
        pBindings->Invalidate( SID_ALIGN_ANY_LEFT );
        pBindings->Invalidate( SID_ALIGN_ANY_HCENTER );
        pBindings->Invalidate( SID_ALIGN_ANY_RIGHT );
        pBindings->Invalidate( SID_ALIGN_ANY_JUSTIFIED );
    }

    return sal_True;
}

sal_Bool ScDocFunc::AutoFormat( const ScRange& rRange, const ScMarkData* pTabMark,
                            sal_uInt16 nFormatNo, sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bSuccess = sal_False;
    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;
    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, sal_True );
    }

    ScAutoFormat* pAutoFormat = ScGlobal::GetAutoFormat();
    ScEditableTester aTester( pDoc, nStartCol,nStartRow, nEndCol,nEndRow, aMark );
    if ( pAutoFormat && nFormatNo < pAutoFormat->GetCount() && aTester.IsEditable() )
    {
        WaitObject aWait( rDocShell.GetActiveDialogParent() );

        sal_Bool bSize = (*pAutoFormat)[nFormatNo]->GetIncludeWidthHeight();

        SCTAB nTabCount = pDoc->GetTableCount();
        ScDocument* pUndoDoc = NULL;
        if ( bRecord )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab, bSize, bSize );
            for (SCTAB i=0; i<nTabCount; i++)
                if (i != nStartTab && aMark.GetTableSelect(i))
                    pUndoDoc->AddUndoTab( i, i, bSize, bSize );

            ScRange aCopyRange = rRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aStart.SetTab(nTabCount-1);
            pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, sal_False, pUndoDoc, &aMark );
            if (bSize)
            {
                pDoc->CopyToDocument( nStartCol,0,0, nEndCol,MAXROW,nTabCount-1,
                                                            IDF_NONE, sal_False, pUndoDoc, &aMark );
                pDoc->CopyToDocument( 0,nStartRow,0, MAXCOL,nEndRow,nTabCount-1,
                                                            IDF_NONE, sal_False, pUndoDoc, &aMark );
            }
            pDoc->BeginDrawUndo();
        }

        pDoc->AutoFormat( nStartCol, nStartRow, nEndCol, nEndRow, nFormatNo, aMark );

        if (bSize)
        {
/*          SCCOL nCols[2];
            nCols[0] = nStartCol;
            nCols[1] = nEndCol;
            SCROW nRows[2];
            nRows[0] = nStartRow;
            nRows[1] = nEndRow;
*/
            SCCOLROW nCols[2] = { nStartCol, nEndCol };
            SCCOLROW nRows[2] = { nStartRow, nEndRow };

            for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                if (aMark.GetTableSelect(nTab))
                {
                    SetWidthOrHeight( sal_True, 1,nCols, nTab, SC_SIZE_VISOPT, STD_EXTRA_WIDTH, sal_False, sal_True);
                    SetWidthOrHeight( sal_False,1,nRows, nTab, SC_SIZE_VISOPT, 0, sal_False, sal_False);
                    rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab,
                                    PAINT_GRID | PAINT_LEFT | PAINT_TOP );
                }
        }
        else
        {
            for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                if (aMark.GetTableSelect(nTab))
                {
                    sal_Bool bAdj = AdjustRowHeight( ScRange(nStartCol, nStartRow, nTab,
                                                        nEndCol, nEndRow, nTab), sal_False );
                    if (bAdj)
                        rDocShell.PostPaint( 0,nStartRow,nTab, MAXCOL,MAXROW,nTab,
                                            PAINT_GRID | PAINT_LEFT );
                    else
                        rDocShell.PostPaint( nStartCol, nStartRow, nTab,
                                            nEndCol, nEndRow, nTab, PAINT_GRID );
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

    sal_Bool bSuccess = sal_False;
    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    sal_Bool bUndo(pDoc->IsUndoEnabled());

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
//      if (bRecord)    // immer
        if (bUndo)
        {
            //! auch bei Undo selektierte Tabellen beruecksichtigen
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab );
            pDoc->CopyToDocument( rRange, IDF_ALL & ~IDF_NOTE, sal_False, pUndoDoc );
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

//      if (bRecord)    // immer
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

    sal_Bool bSuccess = sal_False;
    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;

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
            pDoc->CopyToDocument( rRange, IDF_ALL & ~IDF_NOTE, sal_False, pUndoDoc );

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

sal_Bool ScDocFunc::FillSimple( const ScRange& rRange, const ScMarkData* pTabMark,
                            FillDir eDir, sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bSuccess = sal_False;
    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;

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
            for (SCTAB i=0; i<nTabCount; i++)
                if (i != nDestStartTab && aMark.GetTableSelect(i))
                    pUndoDoc->AddUndoTab( i, i );

            ScRange aCopyRange = aDestArea;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);
            pDoc->CopyToDocument( aCopyRange, IDF_AUTOFILL, sal_False, pUndoDoc, &aMark );
        }

        pDoc->Fill( aSourceArea.aStart.Col(), aSourceArea.aStart.Row(),
                    aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), aMark,
                    nCount, eDir, FILL_SIMPLE );
        AdjustRowHeight(rRange);

        if ( bRecord )      // Draw-Undo erst jetzt verfuegbar
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoAutoFill( &rDocShell, aDestArea, aSourceArea, pUndoDoc, aMark,
                                    eDir, FILL_SIMPLE, FILL_DAY, MAXDOUBLE, 1.0, 1e307,
                                    pDoc->GetRangeName()->GetSharedMaxIndex()+1 ) );
        }

        rDocShell.PostPaintGridAll();
//      rDocShell.PostPaintDataChanged();
        aModificator.SetDocumentModified();

        bSuccess = sal_True;
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

    sal_Bool bSuccess = sal_False;
    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;

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

        //  #27665# mindestens eine Zeile/Spalte als Quellbereich behalten:
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
            for (SCTAB i=0; i<nTabCount; i++)
                if (i != nDestStartTab && aMark.GetTableSelect(i))
                    pUndoDoc->AddUndoTab( i, i );

            pDoc->CopyToDocument(
                aDestArea.aStart.Col(), aDestArea.aStart.Row(), 0,
                aDestArea.aEnd.Col(), aDestArea.aEnd.Row(), nTabCount-1,
                IDF_AUTOFILL, sal_False, pUndoDoc, &aMark );
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
            pDoc->Fill( aSourceArea.aStart.Col(), aSourceArea.aStart.Row(),
                        aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), aMark,
                        nCount, eDir, eCmd, eDateCmd, fStep, fMax );
            AdjustRowHeight(rRange);

            rDocShell.PostPaintGridAll();
//          rDocShell.PostPaintDataChanged();
            aModificator.SetDocumentModified();
        }

        if ( bRecord )      // Draw-Undo erst jetzt verfuegbar
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoAutoFill( &rDocShell, aDestArea, aSourceArea, pUndoDoc, aMark,
                                    eDir, eCmd, eDateCmd, fStart, fStep, fMax,
                                    pDoc->GetRangeName()->GetSharedMaxIndex()+1 ) );
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
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;

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

    FillCmd     eCmd = FILL_AUTO;
    FillDateCmd eDateCmd = FILL_DAY;
    double      fStep = 1.0;
    double      fMax = MAXDOUBLE;

    switch (eDir)
    {
        case FILL_TO_BOTTOM:
            aDestArea.aEnd.SetRow( sal::static_int_cast<SCROW>( aSourceArea.aEnd.Row() + nCount ) );
            break;
        case FILL_TO_TOP:
            if (nCount > sal::static_int_cast<sal_uLong>( aSourceArea.aStart.Row() ))
            {
                DBG_ERROR("FillAuto: Row < 0");
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
                DBG_ERROR("FillAuto: Col < 0");
                nCount = aSourceArea.aStart.Col();
            }
            aDestArea.aStart.SetCol( sal::static_int_cast<SCCOL>( aSourceArea.aStart.Col() - nCount ) );
            break;
        default:
            DBG_ERROR("Falsche Richtung bei FillAuto");
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
        return sal_False;
    }

    if ( pDoc->HasSelectedBlockMatrixFragment( nStartCol, nStartRow,
            nEndCol, nEndRow, aMark ) )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_MATRIXFRAGMENTERR);
        return sal_False;
    }

    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    ScDocument* pUndoDoc = NULL;
    if ( bRecord )
    {
        SCTAB nTabCount = pDoc->GetTableCount();
        SCTAB nDestStartTab = aDestArea.aStart.Tab();

        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nDestStartTab, nDestStartTab );
        for (SCTAB i=0; i<nTabCount; i++)
            if (i != nDestStartTab && aMark.GetTableSelect(i))
                pUndoDoc->AddUndoTab( i, i );

        // do not clone note captions in undo document
        pDoc->CopyToDocument(
            aDestArea.aStart.Col(), aDestArea.aStart.Row(), 0,
            aDestArea.aEnd.Col(), aDestArea.aEnd.Row(), nTabCount-1,
            IDF_AUTOFILL, sal_False, pUndoDoc, &aMark );
    }

    pDoc->Fill( aSourceArea.aStart.Col(), aSourceArea.aStart.Row(),
                aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), aMark,
                nCount, eDir, eCmd, eDateCmd, fStep, fMax );

    AdjustRowHeight(aDestArea);

    if ( bRecord )      // Draw-Undo erst jetzt verfuegbar
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoAutoFill( &rDocShell, aDestArea, aSourceArea, pUndoDoc, aMark,
                                eDir, eCmd, eDateCmd, MAXDOUBLE, fStep, fMax,
                                pDoc->GetRangeName()->GetSharedMaxIndex()+1 ) );
    }

    rDocShell.PostPaintGridAll();
//  rDocShell.PostPaintDataChanged();
    aModificator.SetDocumentModified();

    rRange = aDestArea;         // Zielbereich zurueckgeben (zum Markieren)
    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::MergeCells( const ScRange& rRange, sal_Bool bContents, sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;

    ScEditableTester aTester( pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return sal_False;
    }

    if ( nStartCol == nEndCol && nStartRow == nEndRow )
    {
        // nichts zu tun
        return sal_True;
    }

    if ( pDoc->HasAttrib( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                            HASATTR_MERGED | HASATTR_OVERLAPPED ) )
    {
        // "Zusammenfassen nicht verschachteln !"
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_MERGECELLS_0);
        return sal_False;
    }

    sal_Bool bNeedContents = bContents &&
            ( !pDoc->IsBlockEmpty( nTab, nStartCol,nStartRow+1, nStartCol,nEndRow, true ) ||
              !pDoc->IsBlockEmpty( nTab, nStartCol+1,nStartRow, nEndCol,nEndRow, true ) );

    ScDocument* pUndoDoc = 0;
    if (bRecord)
    {
        // test if the range contains other notes which also implies that we need an undo document
        bool bHasNotes = false;
        for( ScAddress aPos( nStartCol, nStartRow, nTab ); !bHasNotes && (aPos.Col() <= nEndCol); aPos.IncCol() )
            for( aPos.SetRow( nStartRow ); !bHasNotes && (aPos.Row() <= nEndRow); aPos.IncRow() )
                bHasNotes = ((aPos.Col() != nStartCol) || (aPos.Row() != nStartRow)) && (pDoc->GetNote( aPos ) != 0);

        if (bNeedContents || bHasNotes)
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab );
            // note captions are collected by drawing undo
            pDoc->CopyToDocument( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                    IDF_ALL|IDF_NOCAPTIONS, sal_False, pUndoDoc );
        }
        if( bHasNotes )
            pDoc->BeginDrawUndo();
    }

    if (bNeedContents)
        pDoc->DoMergeContents( nTab, nStartCol,nStartRow, nEndCol,nEndRow );
    pDoc->DoMerge( nTab, nStartCol,nStartRow, nEndCol,nEndRow );

    if( bRecord )
    {
        SdrUndoGroup* pDrawUndo = pDoc->GetDrawLayer() ? pDoc->GetDrawLayer()->GetCalcUndo() : 0;
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoMerge( &rDocShell,
                            nStartCol, nStartRow, nTab,
                            nEndCol, nEndRow, nTab, bNeedContents, pUndoDoc, pDrawUndo ) );
    }

    if ( !AdjustRowHeight( ScRange( 0,nStartRow,nTab, MAXCOL,nEndRow,nTab ) ) )
        rDocShell.PostPaint( nStartCol, nStartRow, nTab,
                                            nEndCol, nEndRow, nTab, PAINT_GRID );
    if (bNeedContents)
        pDoc->SetDirty( rRange );
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

sal_Bool ScDocFunc::UnmergeCells( const ScRange& rRange, sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    SCTAB nTab = rRange.aStart.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = sal_False;

    if ( pDoc->HasAttrib( rRange, HASATTR_MERGED ) )
    {
        ScRange aExtended = rRange;
        pDoc->ExtendMerge( aExtended );
        ScRange aRefresh = aExtended;
        pDoc->ExtendOverlapped( aRefresh );

        if (bRecord)
        {
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab );
            pDoc->CopyToDocument( aExtended, IDF_ATTRIB, sal_False, pUndoDoc );
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoRemoveMerge( &rDocShell, rRange, pUndoDoc ) );
        }

        const SfxPoolItem& rDefAttr = pDoc->GetPool()->GetDefaultItem( ATTR_MERGE );
        ScPatternAttr aPattern( pDoc->GetPool() );
        aPattern.GetItemSet().Put( rDefAttr );
        pDoc->ApplyPatternAreaTab( rRange.aStart.Col(), rRange.aStart.Row(),
                                    rRange.aEnd.Col(), rRange.aEnd.Row(), nTab,
                                    aPattern );

        pDoc->RemoveFlagsTab( aExtended.aStart.Col(), aExtended.aStart.Row(),
                                aExtended.aEnd.Col(), aExtended.aEnd.Row(), nTab,
                                SC_MF_HOR | SC_MF_VER );

        pDoc->ExtendMerge( aRefresh, sal_True, sal_False );

        if ( !AdjustRowHeight( aExtended ) )
            rDocShell.PostPaint( aExtended, PAINT_GRID );
        aModificator.SetDocumentModified();
    }
    else if (!bApi)
        Sound::Beep();      //! sal_False zurueck???

    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::ModifyRangeNames( const ScRangeName& rNewRanges, sal_Bool bApi )
{
    return SetNewRangeNames( new ScRangeName( rNewRanges ), bApi );
}

sal_Bool ScDocFunc::SetNewRangeNames( ScRangeName* pNewRanges, sal_Bool /* bApi */ )     // takes ownership of pNewRanges
{
    ScDocShellModificator aModificator( rDocShell );

    DBG_ASSERT( pNewRanges, "pNewRanges is 0" );
    ScDocument* pDoc = rDocShell.GetDocument();
    sal_Bool bUndo(pDoc->IsUndoEnabled());

    if (bUndo)
    {
        ScRangeName* pOld = pDoc->GetRangeName();
        ScRangeName* pUndoRanges = new ScRangeName(*pOld);
        ScRangeName* pRedoRanges = new ScRangeName(*pNewRanges);
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoRangeNames( &rDocShell, pUndoRanges, pRedoRanges ) );
    }

    // #i55926# While loading XML, formula cells only have a single string token,
    // so CompileNameFormula would never find any name (index) tokens, and would
    // unnecessarily loop through all cells.
    sal_Bool bCompile = ( !pDoc->IsImportingXML() && pDoc->GetNamedRangesLockCount() == 0 );

    if ( bCompile )
        pDoc->CompileNameFormula( sal_True );   // CreateFormulaString
    pDoc->SetRangeName( pNewRanges );       // takes ownership
    if ( bCompile )
        pDoc->CompileNameFormula( sal_False );  // CompileFormulaString

    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );

    return sal_True;
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
        String aName;
        pDoc->GetString( nPosX, nPosY, nTab, aName );
        ScRangeData::MakeValidName(aName);
        if (aName.Len())
        {
            String aContent;
            ScRange( nX1, nY1, nTab, nX2, nY2, nTab ).Format( aContent, SCR_ABS_3D, pDoc );

            sal_Bool bInsert = sal_False;
            sal_uInt16 nOldPos;
            if (rList.SearchName( aName, nOldPos ))         // vorhanden ?
            {
                ScRangeData* pOld = rList[nOldPos];
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
                            rList.AtFree(nOldPos);
                            bInsert = sal_True;
                        }
                        else if ( nResult == RET_CANCEL )
                            rCancel = sal_True;
                    }
                }
            }
            else
                bInsert = sal_True;

            if (bInsert)
            {
                ScRangeData* pData = new ScRangeData( pDoc, aName, aContent,
                        ScAddress( nPosX, nPosY, nTab));
                if (!rList.Insert(pData))
                {
                    DBG_ERROR("nanu?");
                    delete pData;
                }
            }
        }
    }
}

sal_Bool ScDocFunc::CreateNames( const ScRange& rRange, sal_uInt16 nFlags, sal_Bool bApi )
{
    if (!nFlags)
        return sal_False;       // war nix

    ScDocShellModificator aModificator( rDocShell );

    sal_Bool bDone = sal_False;
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();
    DBG_ASSERT(rRange.aEnd.Tab() == nTab, "CreateNames: mehrere Tabellen geht nicht");

    sal_Bool bValid = sal_True;
    if ( nFlags & ( NAME_TOP | NAME_BOTTOM ) )
        if ( nStartRow == nEndRow )
            bValid = sal_False;
    if ( nFlags & ( NAME_LEFT | NAME_RIGHT ) )
        if ( nStartCol == nEndCol )
            bValid = sal_False;

    if (bValid)
    {
        ScDocument* pDoc = rDocShell.GetDocument();
        ScRangeName* pNames = pDoc->GetRangeName();
        if (!pNames)
            return sal_False;   // soll nicht sein
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

        sal_Bool bCancel = sal_False;
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

        bDone = ModifyRangeNames( aNewRanges, bApi );

        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
    }

    return bDone;
}

//------------------------------------------------------------------------

sal_Bool ScDocFunc::InsertNameList( const ScAddress& rStartPos, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );


    sal_Bool bDone = sal_False;
    ScDocument* pDoc = rDocShell.GetDocument();
    const sal_Bool bRecord = pDoc->IsUndoEnabled();
    SCTAB nTab = rStartPos.Tab();
    ScDocument* pUndoDoc = NULL;

    ScRangeName* pList = pDoc->GetRangeName();
    sal_uInt16 nCount = pList->GetCount();
    sal_uInt16 nValidCount = 0;
    sal_uInt16 i;
    for (i=0; i<nCount; i++)
    {
        ScRangeData* pData = (*pList)[i];
        if ( !pData->HasType( RT_DATABASE ) && !pData->HasType( RT_SHARED ) )
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
                                        IDF_ALL, sal_False, pUndoDoc );

                pDoc->BeginDrawUndo();      // wegen Hoehenanpassung
            }

            ScRangeData** ppSortArray = new ScRangeData* [ nValidCount ];
            sal_uInt16 j = 0;
            for (i=0; i<nCount; i++)
            {
                ScRangeData* pData = (*pList)[i];
                if ( !pData->HasType( RT_DATABASE ) && !pData->HasType( RT_SHARED ) )
                    ppSortArray[j++] = pData;
            }
#ifndef ICC
            qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                &ScRangeData_QsortNameCompare );
#else
            qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                ICCQsortNameCompare );
#endif
            String aName;
            rtl::OUStringBuffer aContent;
            String aFormula;
            SCROW nOutRow = nStartRow;
            for (j=0; j<nValidCount; j++)
            {
                ScRangeData* pData = ppSortArray[j];
                pData->GetName(aName);
                // relative Referenzen Excel-konform auf die linke Spalte anpassen:
                pData->UpdateSymbol(aContent, ScAddress( nStartCol, nOutRow, nTab ));
                aFormula = '=';
                aFormula += aContent;
                pDoc->PutCell( nStartCol,nOutRow,nTab, new ScStringCell( aName ) );
                pDoc->PutCell( nEndCol  ,nOutRow,nTab, new ScStringCell( aFormula ) );
                ++nOutRow;
            }

            delete [] ppSortArray;

            if (bRecord)
            {
                ScDocument* pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
                pRedoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab,
                                        IDF_ALL, sal_False, pRedoDoc );

                rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoListNames( &rDocShell,
                                ScRange( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab ),
                                pUndoDoc, pRedoDoc ) );
            }

            if (!AdjustRowHeight(ScRange(0,nStartRow,nTab,MAXCOL,nEndRow,nTab)))
                rDocShell.PostPaint( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab, PAINT_GRID );
//!         rDocShell.UpdateOle(GetViewData());
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

    sal_Bool bUndo(pDoc->IsUndoEnabled());

    sal_Bool bRet = sal_False;

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
            bRet = EnterMatrix( aNewRange, &aMark, NULL, aFormula, bApi, sal_False, EMPTY_STRING, formula::FormulaGrammar::GRAM_PODF_A1 );
            if (!bRet)
            {
                //  versuchen, alten Zustand wiederherzustellen
                EnterMatrix( rOldRange, &aMark, NULL, aFormula, bApi, sal_False, EMPTY_STRING, formula::FormulaGrammar::GRAM_PODF_A1 );
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
    //! auch fuer ScViewFunc::InsertAreaLink benutzen!

    ScDocument* pDoc = rDocShell.GetDocument();
    sal_Bool bUndo (pDoc->IsUndoEnabled());

    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();

    //  #i52120# if other area links exist at the same start position,
    //  remove them first (file format specifies only one link definition
    //  for a cell)

    sal_uInt16 nLinkCount = pLinkManager->GetLinks().Count();
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
            nLinkCount = pLinkManager->GetLinks().Count();
            ++nRemoved;
        }
        else
            ++nLinkPos;
    }

    String aFilterName = rFilter;
    String aNewOptions = rOptions;
    if (!aFilterName.Len())
        ScDocumentLoader::GetFilterName( rFile, aFilterName, aNewOptions, sal_True, !bApi );

    //  remove application prefix from filter name here, so the filter options
    //  aren't reset when the filter name is changed in ScAreaLink::DataChanged
    ScDocumentLoader::RemoveAppPrefix( aFilterName );

    ScAreaLink* pLink = new ScAreaLink( &rDocShell, rFile, aFilterName,
                                        aNewOptions, rSource, rDestRange, nRefresh );
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, rFile, &aFilterName, &rSource );

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




