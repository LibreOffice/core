/*************************************************************************
 *
 *  $RCSfile: docfunc.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-26 19:25:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/editobj.hxx>
#include <svx/eeitem.hxx>
#include <svx/linkmgr.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/printer.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>
#include <svtools/zforlist.hxx>
#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#include <svtools/PasswordHelper.hxx>
#endif

#include "docfunc.hxx"

#include "sc.hrc"

#include "arealink.hxx"
#include "attrib.hxx"
#include "autoform.hxx"
#include "cell.hxx"
#include "detdata.hxx"
#include "detfunc.hxx"
#include "docpool.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "editutil.hxx"
#include "globstr.hrc"
#include "namecrea.hxx"     // NAME_TOP etc.
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

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

//========================================================================

IMPL_LINK( ScDocFunc, NotifyDrawUndo, SfxUndoAction*, pUndoAction )
{
    rDocShell.GetUndoManager()->AddUndoAction( new ScUndoDraw( pUndoAction, &rDocShell ) );
    rDocShell.SetDrawModified();
    return 0;
}

//------------------------------------------------------------------------

//  Zeile ueber dem Range painten (fuer Linien nach AdjustRowHeight)

void lcl_PaintAbove( ScDocShell& rDocShell, const ScRange& rRange )
{
    USHORT nRow = rRange.aStart.Row();
    if ( nRow > 0 )
    {
        USHORT nTab = rRange.aStart.Tab();  //! alle?
        --nRow;
        rDocShell.PostPaint( ScRange(0,nRow,nTab, MAXCOL,nRow,nTab), PAINT_GRID );
    }
}

//------------------------------------------------------------------------

BOOL ScDocFunc::AdjustRowHeight( const ScRange& rRange, BOOL bPaint )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if ( pDoc->IsImportingXML() )
    {
        //  for XML import, all row heights are updated together after importing
        return FALSE;
    }

//! OutputDevice* pDev = rDocShell.GetPrinter();
    VirtualDevice aVDev;
    OutputDevice* pDev = &aVDev;        // aus Kompatibilitaetsgruenden...

    if (!pDev)
    {
        DBG_ERROR("kein Printer !??!?");
        return FALSE;
    }

    USHORT nTab      = rRange.aStart.Tab();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndRow   = rRange.aEnd.Row();

    Fraction aOne(1,1);

    MapMode aOldMap = pDev->GetMapMode();
    pDev->SetMapMode( MAP_PIXEL );              // wichtig fuer GetNeededSize
    Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = aPix1000.X() / 1000.0;
    double nPPTY = aPix1000.Y() / 1000.0;

    nPPTX /= rDocShell.GetOutputFactor();       // noetig fuer Bildschirm/VDev

    BOOL bChanged = pDoc->SetOptimalHeight( nStartRow, nEndRow, nTab, 0, pDev,
                                            nPPTX, nPPTY, aOne, aOne, FALSE );

    pDev->SetMapMode( aOldMap );        //! noetig ???

    if ( bPaint && bChanged )
        rDocShell.PostPaint( 0, nStartRow, nTab, MAXCOL, MAXROW, nTab,
                                            PAINT_GRID | PAINT_LEFT );

    return bChanged;
}


//------------------------------------------------------------------------

BOOL ScDocFunc::DetectiveAddPred(const ScAddress& rPos)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument* pDoc = rDocShell.GetDocument();
    BOOL bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    USHORT nCol = rPos.Col();
    USHORT nRow = rPos.Row();
    USHORT nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo();
    BOOL bDone = ScDetectiveFunc( pDoc,nTab ).ShowPred( nCol, nRow );
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

BOOL ScDocFunc::DetectiveDelPred(const ScAddress& rPos)
{
    ScDocument* pDoc = rDocShell.GetDocument();

    BOOL bUndo(pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    ScDocShellModificator aModificator( rDocShell );

    USHORT nCol = rPos.Col();
    USHORT nRow = rPos.Row();
    USHORT nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo();
    BOOL bDone = ScDetectiveFunc( pDoc,nTab ).DeletePred( nCol, nRow );
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

BOOL ScDocFunc::DetectiveAddSucc(const ScAddress& rPos)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument* pDoc = rDocShell.GetDocument();

    BOOL bUndo(pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    USHORT nCol = rPos.Col();
    USHORT nRow = rPos.Row();
    USHORT nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo();
    BOOL bDone = ScDetectiveFunc( pDoc,nTab ).ShowSucc( nCol, nRow );
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

BOOL ScDocFunc::DetectiveDelSucc(const ScAddress& rPos)
{
    ScDocument* pDoc = rDocShell.GetDocument();

    BOOL bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    ScDocShellModificator aModificator( rDocShell );

    USHORT nCol = rPos.Col();
    USHORT nRow = rPos.Row();
    USHORT nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo();
    BOOL bDone = ScDetectiveFunc( pDoc,nTab ).DeleteSucc( nCol, nRow );
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

BOOL ScDocFunc::DetectiveAddError(const ScAddress& rPos)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument* pDoc = rDocShell.GetDocument();

    BOOL bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    USHORT nCol = rPos.Col();
    USHORT nRow = rPos.Row();
    USHORT nTab = rPos.Tab();

    if (bUndo)
        pModel->BeginCalcUndo();
    BOOL bDone = ScDetectiveFunc( pDoc,nTab ).ShowError( nCol, nRow );
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

BOOL ScDocFunc::DetectiveMarkInvalid(USHORT nTab)
{
    ScDocShellModificator aModificator( rDocShell );

    rDocShell.MakeDrawLayer();
    ScDocument* pDoc = rDocShell.GetDocument();

    BOOL bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();

    Window* pWaitWin = rDocShell.GetDialogParent();
    if (pWaitWin)
        pWaitWin->EnterWait();
    if (bUndo)
        pModel->BeginCalcUndo();
    BOOL bOverflow;
    BOOL bDone = ScDetectiveFunc( pDoc,nTab ).MarkInvalid( bOverflow );
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

BOOL ScDocFunc::DetectiveDelAll(USHORT nTab)
{
    ScDocument* pDoc = rDocShell.GetDocument();

    BOOL bUndo (pDoc->IsUndoEnabled());
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    ScDocShellModificator aModificator( rDocShell );

    if (bUndo)
        pModel->BeginCalcUndo();
    BOOL bDone = ScDetectiveFunc( pDoc,nTab ).DeleteAll( SC_DET_DETECTIVE );
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

BOOL ScDocFunc::DetectiveRefresh( BOOL bAutomatic )
{
    BOOL bDone = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();

    BOOL bUndo (pDoc->IsUndoEnabled());
    ScDetOpList* pList = pDoc->GetDetOpList();
    if ( pList && pList->Count() )
    {
        rDocShell.MakeDrawLayer();
        ScDrawLayer* pModel = pDoc->GetDrawLayer();
        if (bUndo)
            pModel->BeginCalcUndo();

        //  Loeschen auf allen Tabellen

        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT nTab=0; nTab<nTabCount; nTab++)
            ScDetectiveFunc( pDoc,nTab ).DeleteAll( SC_DET_DETECTIVE );

        //  Wiederholen

        USHORT nCount = pList->Count();
        for (USHORT i=0; i<nCount; i++)
        {
            ScDetOpData* pData = (*pList)[i];
            if (pData)
            {
                ScAddress aPos = pData->GetPos();
                ScDetectiveFunc aFunc( pDoc, aPos.Tab() );
                USHORT nCol = aPos.Col();
                USHORT nRow = aPos.Row();
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
        bDone = TRUE;
    }
    return bDone;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::DeleteContents( const ScMarkData& rMark, USHORT nFlags,
                                    BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        DBG_ERROR("ScDocFunc::DeleteContents ohne Markierung");
        return FALSE;
    }

    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    if (!pDoc->IsSelectionEditable(rMark))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    ScRange aMarkRange;
    BOOL bSimple = FALSE;

    ScMarkData aMultiMark = rMark;
    aMultiMark.SetMarking(FALSE);       // fuer MarkToMulti

    ScDocument* pUndoDoc = NULL;
    BOOL bMulti = !bSimple && aMultiMark.IsMultiMarked();
    if (!bSimple)
    {
        aMultiMark.MarkToMulti();
        aMultiMark.GetMultiMarkArea( aMarkRange );
    }
    ScRange aExtendedRange(aMarkRange);
    if (!bSimple)
    {
        if ( pDoc->ExtendMerge( aExtendedRange, TRUE ) )
            bMulti = FALSE;
    }

    // keine Objekte auf geschuetzten Tabellen
    BOOL bObjects = FALSE;
    if ( nFlags & IDF_OBJECTS )
    {
        bObjects = TRUE;
        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT nTab=0; nTab<nTabCount; nTab++)
            if (aMultiMark.GetTableSelect(nTab) && pDoc->IsTabProtected(nTab))
                bObjects = FALSE;
    }

    USHORT nExtFlags = 0;                       // Linien interessieren nur, wenn Attribute
    if ( nFlags & IDF_ATTRIB )                  // geloescht werden
        if (pDoc->HasAttrib( aMarkRange, HASATTR_PAINTEXT ))
            nExtFlags |= SC_PF_LINES;

    //  Reihenfolge:
    //  1) BeginDrawUndo
    //  2) Objekte loeschen (DrawUndo wird gefuellt)
    //  3) Inhalte fuer Undo kopieren und Undo-Aktion anlegen
    //  4) Inhalte loeschen

    if (bObjects)
    {
        if (bRecord)
            pDoc->BeginDrawUndo();

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
        USHORT nUndoDocFlags = nFlags;
        if (nFlags & IDF_ATTRIB)
            nUndoDocFlags |= IDF_ATTRIB;
        if (nFlags & IDF_EDITATTR)          // Edit-Engine-Attribute
            nUndoDocFlags |= IDF_STRING;    // -> Zellen werden geaendert
        if (nFlags & IDF_NOTE)
            nUndoDocFlags |= IDF_CONTENTS;  // #68795# copy all cells with their notes
        pDoc->CopyToDocument( aExtendedRange, nUndoDocFlags, bMulti, pUndoDoc, &aMultiMark );
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDeleteContents( &rDocShell, aMultiMark, aExtendedRange,
                                      pUndoDoc, bMulti, nFlags, bObjects ) );
    }

//! HideAllCursors();   // falls Zusammenfassung aufgehoben wird
    if (bSimple)
        pDoc->DeleteArea( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                          aMarkRange.aEnd.Col(),   aMarkRange.aEnd.Row(),
                          aMultiMark, nFlags );
    else
    {
        pDoc->DeleteSelection( nFlags, aMultiMark );
        aMultiMark.MarkToSimple();
    }

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

    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::TransliterateText( const ScMarkData& rMark, sal_Int32 nType,
                                    BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    if (!pDoc->IsSelectionEditable(rMark))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    ScRange aMarkRange;
    ScMarkData aMultiMark = rMark;
    aMultiMark.SetMarking(FALSE);       // for MarkToMulti
    aMultiMark.MarkToMulti();
    aMultiMark.GetMultiMarkArea( aMarkRange );

    //! undo

    pDoc->TransliterateText( aMultiMark, nType );

    if (!AdjustRowHeight( aMarkRange ))
        rDocShell.PostPaint( aMarkRange, PAINT_GRID );

    aModificator.SetDocumentModified();

    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::SetNormalString( const ScAddress& rPos, const String& rText, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument* pDoc = rDocShell.GetDocument();

    BOOL bUndo(pDoc->IsUndoEnabled());
    if (!pDoc->IsBlockEditable( rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    USHORT* pTabs = NULL;
    ScBaseCell** ppOldCells = NULL;
    BOOL* pHasFormat = NULL;
    ULONG* pOldFormats = NULL;
    ScBaseCell* pDocCell = pDoc->GetCell( rPos );
    BOOL bEditDeleted = (pDocCell && pDocCell->GetCellType() == CELLTYPE_EDIT);
    if (bUndo)
    {
        pTabs = new USHORT[1];
        pTabs[0] = rPos.Tab();
        ppOldCells  = new ScBaseCell*[1];
        ppOldCells[0] = pDocCell ? pDocCell->Clone(pDoc) : NULL;

        pHasFormat = new BOOL[1];
        pOldFormats = new ULONG[1];
        const SfxPoolItem* pItem;
        const ScPatternAttr* pPattern = pDoc->GetPattern( rPos.Col(),rPos.Row(),rPos.Tab() );
        if ( SFX_ITEM_SET == pPattern->GetItemSet().GetItemState(
                                ATTR_VALUE_FORMAT,FALSE,&pItem) )
        {
            pHasFormat[0] = TRUE;
            pOldFormats[0] = ((const SfxUInt32Item*)pItem)->GetValue();
        }
        else
            pHasFormat[0] = FALSE;
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

    rDocShell.PostPaintCell( rPos.Col(), rPos.Row(), rPos.Tab() );
    aModificator.SetDocumentModified();

    return TRUE;
}

BOOL ScDocFunc::PutCell( const ScAddress& rPos, ScBaseCell* pNewCell, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );
    ScDocument* pDoc = rDocShell.GetDocument();
    BOOL bUndo (pDoc->IsUndoEnabled());
    if (!pDoc->IsBlockEditable( rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        pNewCell->Delete();
        return FALSE;
    }

    BOOL bEditCell(FALSE);
    BOOL bEditDeleted(FALSE);
    BOOL bHeight;
    ScBaseCell* pUndoCell = NULL;
    ScBaseCell* pRedoCell = NULL;
    if (bUndo)
    {
        bEditCell = ( pNewCell->GetCellType() == CELLTYPE_EDIT );
        ScBaseCell* pDocCell = pDoc->GetCell( rPos );
        bEditDeleted = (pDocCell && pDocCell->GetCellType() == CELLTYPE_EDIT);
        bHeight = ( bEditDeleted || bEditCell ||
                    pDoc->HasAttrib( ScRange(rPos), HASATTR_NEEDHEIGHT ) );
        pUndoCell = pDocCell ? pDocCell->Clone(pDoc) : NULL;
        pRedoCell = pNewCell ? pNewCell->Clone(pDoc) : NULL;
    }

    pDoc->PutCell( rPos, pNewCell );

    //  wegen ChangeTracking darf UndoAction erst nach PutCell angelegt werden
    if (bUndo)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoPutCell( &rDocShell, rPos, pUndoCell, pRedoCell, bHeight ) );
    }

    if (bHeight)
        AdjustRowHeight( ScRange(rPos) );

    rDocShell.PostPaintCell( rPos.Col(), rPos.Row(), rPos.Tab() );
    aModificator.SetDocumentModified();

    return TRUE;
}

BOOL ScDocFunc::PutData( const ScAddress& rPos, EditEngine& rEngine, BOOL bInterpret, BOOL bApi )
{
    //  PutData ruft PutCell oder SetNormalString

    //  harte Ausrichtungs-Attribute aus der EditEngine loeschen
    USHORT nCount = rEngine.GetParagraphCount();
    for (USHORT i=0; i<nCount; i++)
    {
        const SfxItemSet& rOld = rEngine.GetParaAttribs( i );
        if ( rOld.GetItemState( EE_PARA_JUST ) == SFX_ITEM_SET )
        {
            SfxItemSet aNew( rOld );
            aNew.ClearItem( EE_PARA_JUST );
            rEngine.SetParaAttribs( i, aNew );
        }
    }

    BOOL bRet = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScEditAttrTester aTester( &rEngine );
    BOOL bEditCell = aTester.NeedsObject();
    if ( bEditCell )
    {
        EditTextObject* pData = rEngine.CreateTextObject();
        bRet = PutCell( rPos,
                        new ScEditCell( pData, pDoc, rEngine.GetEditTextObjectPool() ),
                        bApi );
        delete pData;
    }
    else
    {
        String aText = rEngine.GetText();
        if (bInterpret)
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
        if ( aPattern.GetItemSet().Count() > 0 )
        {
            ScMarkData aMark;
            aMark.SelectTable( rPos.Tab(), TRUE );
            aMark.SetMarkArea( ScRange( rPos ) );
            ApplyAttributes( aMark, aPattern, TRUE, bApi );
        }
    }

    return bRet;
}


ScTokenArray* lcl_ScDocFunc_CreateTokenArrayXML( const String& rText )
{
    ScTokenArray* pCode = new ScTokenArray;
    xub_StrLen nPos = 0;
    while ( nPos < rText.Len() )
    {
        xub_StrLen nChars = Min( rText.Len() - nPos, MAXSTRLEN-1 );
        String aTmpStr( rText.Copy( nPos, nChars ) );
        pCode->AddString( aTmpStr.GetBuffer() );
        nPos += nChars;
    }
    return pCode;
}


BOOL ScDocFunc::SetCellText( const ScAddress& rPos, const String& rText,
                                BOOL bInterpret, BOOL bEnglish, BOOL bApi )
{
    //  SetCellText ruft PutCell oder SetNormalString

    ScDocument* pDoc = rDocShell.GetDocument();
    ScBaseCell* pNewCell = NULL;
    if ( bInterpret )
    {
        if ( bEnglish )
        {
            if ( rText.Len() > 1 && rText.GetChar(0) == '=' )
            {
                ScTokenArray* pCode;
                if ( pDoc->IsImportingXML() )
                {   // temporary formula string as string tokens
                    pCode = lcl_ScDocFunc_CreateTokenArrayXML( rText );
                }
                else
                {
                    ScCompiler aComp( pDoc, rPos );
                    aComp.SetCompileEnglish( TRUE );
                    pCode = aComp.CompileString( rText );
                }
                pNewCell = new ScFormulaCell( pDoc, rPos, pCode, 0 );
                delete pCode;   // Zell-ctor hat das TokenArray kopiert
            }
            else        // (nur) auf englisches Zahlformat testen
            {
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                ULONG nEnglish = pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);
                double fVal;
                if ( pFormatter->IsNumberFormat( rText, nEnglish, fVal ) )
                    pNewCell = new ScValueCell( fVal );
                else
                    pNewCell = ScBaseCell::CreateTextCell( rText, pDoc );

                //  das (englische) Zahlformat wird nicht gesetzt
                //! passendes lokales Format suchen und setzen???
            }
        }
        // sonst Null behalten -> SetString mit lokalen Formeln/Zahlformat
    }
    else if ( rText.Len() )
        pNewCell = ScBaseCell::CreateTextCell( rText, pDoc );   // immer Text

    if (pNewCell)
        return PutCell( rPos, pNewCell, bApi );
    else
        return SetNormalString( rPos, rText, bApi );
}

//------------------------------------------------------------------------

BOOL ScDocFunc::SetNoteText( const ScAddress& rPos, const String& rText, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (!pDoc->IsBlockEditable( rPos.Tab(), rPos.Col(),rPos.Row(), rPos.Col(),rPos.Row() ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    String aNewText = rText;
    aNewText.ConvertLineEnd();      //! ist das noetig ???

    ScPostIt aNote;
    pDoc->GetNote( rPos.Col(), rPos.Row(), rPos.Tab(), aNote );
    aNote.AutoSetText( aNewText );      // setzt auch Author und Date
    pDoc->SetNote( rPos.Col(), rPos.Row(), rPos.Tab(), aNote );

    if ( aNote.IsShown() )
    {
        //  Zeichenobjekt updaten
        //! bei gelocktem Paint auch erst spaeter !!!

        ScDetectiveFunc aDetFunc( pDoc, rPos.Tab() );
        aDetFunc.HideComment( rPos.Col(), rPos.Row() );
        aDetFunc.ShowComment( rPos.Col(), rPos.Row(), FALSE );  // FALSE: nur wenn gefunden
    }

    //! Undo !!!

    rDocShell.PostPaintCell( rPos.Col(), rPos.Row(), rPos.Tab() );
    aModificator.SetDocumentModified();

    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
                                    BOOL bRecord, BOOL bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if ( bRecord && !pDoc->IsUndoEnabled() )
        bRecord = FALSE;

    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    if ( !pDoc->IsSelectionEditable( rMark, &bOnlyNotBecauseOfMatrix )
            && !bOnlyNotBecauseOfMatrix )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    ScDocShellModificator aModificator( rDocShell );

    //! Umrandung

    ScRange aMultiRange;
    BOOL bMulti = rMark.IsMultiMarked();
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

    BOOL bPaintExt = pDoc->HasAttrib( aMultiRange, HASATTR_PAINTEXT );
    pDoc->ApplySelectionPattern( rPattern, rMark );

    if (!bPaintExt)
        bPaintExt = pDoc->HasAttrib( aMultiRange, HASATTR_PAINTEXT );
    USHORT nExtFlags = bPaintExt ? SC_PF_LINES : 0;
    if (!AdjustRowHeight( aMultiRange ))
        rDocShell.PostPaint( aMultiRange, PAINT_GRID, nExtFlags );
    else if (nExtFlags & SC_PF_LINES)
        lcl_PaintAbove( rDocShell, aMultiRange );   // fuer Linien ueber dem Bereich

    aModificator.SetDocumentModified();

    return TRUE;
}


BOOL ScDocFunc::ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
                                    BOOL bRecord, BOOL bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if ( bRecord && !pDoc->IsUndoEnabled() )
        bRecord = FALSE;

    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    if ( !pDoc->IsSelectionEditable( rMark, &bOnlyNotBecauseOfMatrix )
            && !bOnlyNotBecauseOfMatrix )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    ScStyleSheet* pStyleSheet = (ScStyleSheet*) pDoc->GetStyleSheetPool()->Find(
                                                rStyleName, SFX_STYLE_FAMILY_PARA );
    if (!pStyleSheet)
        return FALSE;

    ScDocShellModificator aModificator( rDocShell );

    ScRange aMultiRange;
    BOOL bMulti = rMark.IsMultiMarked();
    if ( bMulti )
        rMark.GetMultiMarkArea( aMultiRange );
    else
        rMark.GetMarkArea( aMultiRange );

    if ( bRecord )
    {
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        USHORT nStartTab = aMultiRange.aStart.Tab();
        USHORT nTabCount = pDoc->GetTableCount();
        pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
        for (USHORT i=0; i<nTabCount; i++)
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

//  BOOL bPaintExt = pDoc->HasAttrib( aMultiRange, HASATTR_PAINTEXT );
//  pDoc->ApplySelectionPattern( rPattern, rMark );

    pDoc->ApplySelectionStyle( (ScStyleSheet&)*pStyleSheet, rMark );

//  if (!bPaintExt)
//      bPaintExt = pDoc->HasAttrib( aMultiRange, HASATTR_PAINTEXT );
//  USHORT nExtFlags = bPaintExt ? SC_PF_LINES : 0;
    USHORT nExtFlags = 0;
    if (!AdjustRowHeight( aMultiRange ))
        rDocShell.PostPaint( aMultiRange, PAINT_GRID, nExtFlags );
    else if (nExtFlags & SC_PF_LINES)
        lcl_PaintAbove( rDocShell, aMultiRange );   // fuer Linien ueber dem Bereich

    aModificator.SetDocumentModified();

    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::InsertCells( const ScRange& rRange, InsCellCmd eCmd,
                                BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();

    if ( nStartRow > MAXROW || nEndRow > MAXROW )
    {
        DBG_ERROR("invalid row in InsertCells");
        return FALSE;
    }

    USHORT nTab = nStartTab;
    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nTabCount = pDoc->GetTableCount();
    USHORT nPaintStartX = nStartCol;
    USHORT nPaintStartY = nStartRow;
    USHORT nPaintEndX = nEndCol;
    USHORT nPaintEndY = nEndRow;
    USHORT nPaintFlags = PAINT_GRID;
    BOOL bSuccess;

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    //  zugehoerige Szenarien auch anpassen
    if ( !pDoc->IsScenario(nEndTab) )
        while ( nEndTab+1 < nTabCount && pDoc->IsScenario(nEndTab+1) )
            ++nEndTab;

                    // Test zusammengefasste

    USHORT nMergeTestStartX = nStartCol;
    USHORT nMergeTestStartY = nStartRow;
    USHORT nMergeTestEndX = nEndCol;
    USHORT nMergeTestEndY = nEndRow;

    if (eCmd==INS_INSROWS)
    {
        nMergeTestStartX = 0;
        nMergeTestEndX = MAXCOL;
    }
    if (eCmd==INS_INSCOLS)
    {
        nMergeTestStartY = 0;
        nMergeTestEndY = MAXROW;
    }
    if (eCmd==INS_CELLSDOWN)
        nMergeTestEndY = MAXROW;
    if (eCmd==INS_CELLSRIGHT)
        nMergeTestEndX = MAXCOL;

    BOOL bCanDo = TRUE;
    BOOL bNeedRefresh = FALSE;

    USHORT nEditTestEndX = (eCmd==INS_INSCOLS) ? MAXCOL : nMergeTestEndX;
    USHORT nEditTestEndY = (eCmd==INS_INSROWS) ? MAXROW : nMergeTestEndY;
    if (!pDoc->IsBlockEditable( nTab, nMergeTestStartX,nMergeTestStartY,
                                        nEditTestEndX,nEditTestEndY ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    if (pDoc->HasAttrib( nMergeTestStartX,nMergeTestStartY,nTab,
                            nMergeTestEndX,nMergeTestEndY,nTab,
                            HASATTR_MERGED | HASATTR_OVERLAPPED ))
    {
        if (eCmd==INS_CELLSRIGHT)
            bNeedRefresh = TRUE;

        USHORT nMergeStartX = nMergeTestStartX;
        USHORT nMergeStartY = nMergeTestStartY;
        USHORT nMergeEndX   = nMergeTestEndX;
        USHORT nMergeEndY   = nMergeTestEndY;

        pDoc->ExtendMerge( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, nTab );
        pDoc->ExtendOverlapped( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, nTab );
        if ( nMergeStartX != nMergeTestStartX || nMergeStartY != nMergeTestStartY ||
             nMergeEndX   != nMergeTestEndX   || nMergeEndY   != nMergeTestEndY )
            bCanDo = FALSE;

        //!     ? nur Start testen ?

        if (!bCanDo)
            if ( eCmd==INS_INSCOLS || eCmd==INS_INSROWS )
                if ( nMergeStartX == nMergeTestStartX && nMergeStartY == nMergeTestStartY )
                {
                    bCanDo = TRUE;
//                  bNeedRefresh = TRUE;
                }
    }

    if (!bCanDo)
    {
        //!         auf Verschieben (Drag&Drop) zurueckfuehren !!!
        //  "Kann nicht in zusammengefasste Bereiche einfuegen"
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0);
        return FALSE;
    }

    //
    //      ausfuehren
    //

    WaitObject aWait( rDocShell.GetDialogParent() );        // wichtig wegen TrackFormulas bei UpdateReference

    ScDocument* pRefUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if ( bRecord )
    {
        pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1, FALSE, FALSE );

                //  alle Formeln wegen Referenzen
                //! stattdessen bei InsertRow uebergeben
        pDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,MAXTAB, IDF_FORMULA,FALSE,pRefUndoDoc );

        pUndoData = new ScRefUndoData( pDoc );

        pDoc->BeginDrawUndo();
    }

    switch (eCmd)
    {
        case INS_CELLSDOWN:
            bSuccess = pDoc->InsertRow( nStartCol,nStartTab, nEndCol,nEndTab,
                                        nStartRow, nEndRow-nStartRow+1 );
            nPaintEndY = MAXROW;
            break;
        case INS_INSROWS:
            bSuccess = pDoc->InsertRow( 0,nStartTab, MAXCOL,nEndTab,
                                        nStartRow, nEndRow-nStartRow+1 );
            nPaintStartX = 0;
            nPaintEndX = MAXCOL;
            nPaintEndY = MAXROW;
            nPaintFlags |= PAINT_LEFT;
            break;
        case INS_CELLSRIGHT:
            bSuccess = pDoc->InsertCol( nStartRow,nStartTab, nEndRow,nEndTab,
                                        nStartCol, nEndCol-nStartCol+1 );
            nPaintEndX = MAXCOL;
            break;
        case INS_INSCOLS:
            bSuccess = pDoc->InsertCol( 0,nStartTab, MAXROW,nEndTab,
                                        nStartCol, nEndCol-nStartCol+1 );
            nPaintStartY = 0;
            nPaintEndY = MAXROW;
            nPaintEndX = MAXCOL;
            nPaintFlags |= PAINT_TOP;
            break;
        default:
            DBG_ERROR("Falscher Code beim Einfuegen");
            bSuccess = FALSE;
            break;
    }

    if ( bSuccess )
    {
        if ( bRecord )
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoInsertCells( &rDocShell, ScRange(
                                                nStartCol,nStartRow,nStartTab,
                                                nEndCol,nEndRow,nEndTab ),
                                        eCmd, pRefUndoDoc, pUndoData ) );
        }

        if (bNeedRefresh)
            pDoc->ExtendMerge( nMergeTestStartX,nMergeTestStartY,
                                nMergeTestEndX,nMergeTestEndY, nTab, TRUE );
        else
            pDoc->RefreshAutoFilter( nMergeTestStartX,nMergeTestStartY,
                                        nMergeTestEndX,nMergeTestEndY, nTab);

        if ( eCmd == INS_INSROWS || eCmd == INS_INSCOLS )
            pDoc->UpdatePageBreaks( nTab );

        USHORT nExtFlags = 0;
        if (pDoc->HasAttrib( nPaintStartX,nPaintStartY,nTab,
                                nPaintEndX,nPaintEndY,nTab, HASATTR_PAINTEXT ))
            nExtFlags |= SC_PF_LINES;

        //  ganze Zeilen einfuegen: nur neue Zeilen anpassen
        BOOL bAdjusted = ( eCmd == INS_INSROWS ) ?
                AdjustRowHeight(ScRange(0,nStartRow,nStartTab, MAXCOL,nEndRow,nEndTab)) :
                AdjustRowHeight(ScRange(0,nPaintStartY,nStartTab, MAXCOL,nPaintEndY,nEndTab));
        if (bAdjusted)
        {
            //  paint only what is not done by AdjustRowHeight
            if (nPaintFlags & PAINT_TOP)
                rDocShell.PostPaint( nPaintStartX, nPaintStartY, nStartTab,
                                     nPaintEndX,   nPaintEndY,   nEndTab,   PAINT_TOP );
        }
        else
            rDocShell.PostPaint( nPaintStartX, nPaintStartY, nStartTab,
                                 nPaintEndX,   nPaintEndY,   nEndTab,
                                 nPaintFlags,  nExtFlags);
        aModificator.SetDocumentModified();

//!     pDocSh->UpdateOle(GetViewData());   // muss an der View bleiben
//!     CellContentChanged();               // muss an der View bleiben
    }
    else
    {
        delete pRefUndoDoc;
        delete pUndoData;
        if (!bApi)
            rDocShell.ErrorMessage(STR_INSERT_FULL);        // Spalte/Zeile voll
    }
    return bSuccess;
}

BOOL ScDocFunc::DeleteCells( const ScRange& rRange, DelCellCmd eCmd, BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();

    if ( nStartRow > MAXROW || nEndRow > MAXROW )
    {
        DBG_ERROR("invalid row in DeleteCells");
        return FALSE;
    }

    USHORT nTab = nStartTab;
    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nTabCount = pDoc->GetTableCount();
    USHORT nPaintStartX = nStartCol;
    USHORT nPaintStartY = nStartRow;
    USHORT nPaintEndX = nEndCol;
    USHORT nPaintEndY = nEndRow;
    USHORT nPaintFlags = PAINT_GRID;

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    //  zugehoerige Szenarien auch anpassen
    if ( !pDoc->IsScenario(nEndTab) )
        while ( nEndTab+1 < nTabCount && pDoc->IsScenario(nEndTab+1) )
            ++nEndTab;

    USHORT nUndoStartX = nStartCol;
    USHORT nUndoStartY = nStartRow;
    USHORT nUndoEndX = nEndCol;
    USHORT nUndoEndY = nEndRow;

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

    USHORT nDelEndX = nUndoEndX;
    if (eCmd==DEL_CELLSLEFT||eCmd==DEL_DELCOLS) nDelEndX = MAXCOL;
    USHORT nDelEndY = nUndoEndY;
    if (eCmd==DEL_CELLSUP||eCmd==DEL_DELROWS) nDelEndY = MAXROW;

                    // Test Zellschutz

    USHORT nEditTestEndX = nUndoEndX;
    if ( eCmd==DEL_DELCOLS || eCmd==DEL_CELLSLEFT ) nEditTestEndX = MAXCOL;
    USHORT nEditTestEndY = nUndoEndY;
    if ( eCmd==DEL_DELROWS || eCmd==DEL_CELLSUP ) nEditTestEndY = MAXROW;
    if (!pDoc->IsBlockEditable( nTab, nUndoStartX,nUndoStartY,nEditTestEndX,nEditTestEndY ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

                    // Test zusammengefasste

    USHORT nMergeTestEndX = (eCmd==DEL_CELLSLEFT) ? MAXCOL : nUndoEndX;
    USHORT nMergeTestEndY = (eCmd==DEL_CELLSUP)   ? MAXROW : nUndoEndY;
    BOOL bCanDo = TRUE;
    BOOL bNeedRefresh = FALSE;

    if (pDoc->HasAttrib( nUndoStartX,nUndoStartY,nTab, nMergeTestEndX,nMergeTestEndY,nTab,
                            HASATTR_MERGED | HASATTR_OVERLAPPED ))
    {
        if (eCmd==DEL_CELLSLEFT)
            bNeedRefresh = TRUE;

        USHORT nMergeStartX = nUndoStartX;
        USHORT nMergeStartY = nUndoStartY;
        USHORT nMergeEndX   = nMergeTestEndX;
        USHORT nMergeEndY   = nMergeTestEndY;

        pDoc->ExtendMerge( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, nTab );
        pDoc->ExtendOverlapped( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, nTab );
        if ( nMergeStartX != nUndoStartX  || nMergeStartY != nUndoStartY ||
             nMergeEndX != nMergeTestEndX || nMergeEndY != nMergeTestEndY )
            bCanDo = FALSE;

        //      ganze Zeilen/Spalten: Testen, ob Merge komplett geloescht werden kann

        if (!bCanDo)
            if ( eCmd==DEL_DELCOLS || eCmd==DEL_DELROWS )
                if ( nMergeStartX == nUndoStartX && nMergeStartY == nUndoStartY )
                {
                    bCanDo = TRUE;
                    bNeedRefresh = TRUE;
                }
    }

    if (!bCanDo)
    {
        //!         auf Verschieben (Drag&Drop) zurueckfuehren !!!
        //  "Kann nicht aus zusammengefassten Bereichen loeschen"
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_DELETECELLS_0);
        return FALSE;
    }

    //
    //      ausfuehren
    //

    WaitObject aWait( rDocShell.GetDialogParent() );        // wichtig wegen TrackFormulas bei UpdateReference

    ScDocument* pUndoDoc = NULL;
    ScDocument* pRefUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if ( bRecord )
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab,
                                (eCmd==DEL_DELCOLS), (eCmd==DEL_DELROWS) );
        pDoc->CopyToDocument( nUndoStartX, nUndoStartY, nStartTab, nDelEndX, nDelEndY, nEndTab,
                                IDF_ALL, FALSE, pUndoDoc );
        pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1, FALSE, FALSE );

        pUndoData = new ScRefUndoData( pDoc );

        pDoc->BeginDrawUndo();
    }

    USHORT nExtFlags = 0;
    if (pDoc->HasAttrib( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab, HASATTR_PAINTEXT ))
        nExtFlags |= SC_PF_LINES;

    BOOL bUndoOutline = FALSE;
    switch (eCmd)
    {
        case DEL_CELLSUP:
            pDoc->DeleteRow( nStartCol, nStartTab, nEndCol, nEndTab,
                                nStartRow, nEndRow-nStartRow+1, pRefUndoDoc );
            nPaintEndY = MAXROW;
            break;
        case DEL_DELROWS:
            pDoc->DeleteRow( 0, nStartTab, MAXCOL, nEndTab,
                                nStartRow, nEndRow-nStartRow+1, pRefUndoDoc, &bUndoOutline );
            nPaintStartX = 0;
            nPaintEndX = MAXCOL;
            nPaintEndY = MAXROW;
            nPaintFlags |= PAINT_LEFT;
            break;
        case DEL_CELLSLEFT:
            pDoc->DeleteCol( nStartRow, nStartTab, nEndRow, nEndTab,
                                nStartCol, nEndCol-nStartCol+1, pRefUndoDoc );
            nPaintEndX = MAXCOL;
            break;
        case DEL_DELCOLS:
            pDoc->DeleteCol( 0, nStartTab, MAXROW, nEndTab,
                                nStartCol, nEndCol-nStartCol+1, pRefUndoDoc, &bUndoOutline );
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
        for (USHORT i=nStartTab; i<=nEndTab; i++)
            pRefUndoDoc->DeleteAreaTab(nUndoStartX,nUndoStartY,nDelEndX,nDelEndY,i,
                                        IDF_ALL);

            //  alle Tabellen anlegen, damit Formeln kopiert werden koennen:
        pUndoDoc->AddUndoTab( 0, nTabCount-1, FALSE, FALSE );

            //  kopieren mit bColRowFlags=FALSE (#54194#)
        pRefUndoDoc->CopyToDocument(0,0,0,MAXCOL,MAXROW,MAXTAB,IDF_FORMULA,FALSE,pUndoDoc,NULL,FALSE);
        delete pRefUndoDoc;
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDeleteCells( &rDocShell, ScRange(
                                            nStartCol,nStartRow,nStartTab,
                                            nEndCol,nEndRow,nEndTab ),
                                    eCmd, pUndoDoc, pUndoData ) );
    }

    if (bNeedRefresh)
    {
        if ( eCmd==DEL_DELCOLS || eCmd==DEL_DELROWS )
        {
            if (eCmd==DEL_DELCOLS) nMergeTestEndX = MAXCOL;
            if (eCmd==DEL_DELROWS) nMergeTestEndY = MAXROW;
            ScPatternAttr aPattern( pDoc->GetPool() );
            aPattern.GetItemSet().Put( ScMergeFlagAttr() );

            ScMarkData aMark;                           // nur fuer Tabellen
            for (USHORT i=nStartTab; i<=nEndTab; i++)
                aMark.SelectTable( i, TRUE );
            pDoc->ApplyPatternArea( nUndoStartX, nUndoStartY, nMergeTestEndX, nMergeTestEndY,
                                        aMark, aPattern );
        }
        pDoc->ExtendMerge( nUndoStartX, nUndoStartY, nMergeTestEndX, nMergeTestEndY, nTab, TRUE );
    }

    if ( eCmd == DEL_DELCOLS || eCmd == DEL_DELROWS )
        pDoc->UpdatePageBreaks( nTab );

    if (pDoc->HasAttrib( nPaintStartX,nPaintStartY,nTab,
                            nPaintEndX,nPaintEndY,nTab, HASATTR_PAINTEXT ))
        nExtFlags |= SC_PF_LINES;

    //  ganze Zeilen loeschen: nichts anpassen
    if ( eCmd == DEL_DELROWS ||
            !AdjustRowHeight(ScRange(0,nPaintStartY,nStartTab, MAXCOL,nPaintEndY,nEndTab)) )
        rDocShell.PostPaint( nPaintStartX, nPaintStartY, nStartTab,
                             nPaintEndX,   nPaintEndY,   nEndTab,
                             nPaintFlags,  nExtFlags );
    else
    {
        //  paint only what is not done by AdjustRowHeight
        if (nExtFlags & SC_PF_LINES)
            lcl_PaintAbove( rDocShell, ScRange( nPaintStartX, nPaintStartY, nStartTab,
                                                nPaintEndX,   nPaintEndY,   nEndTab) );
        if (nPaintFlags & PAINT_TOP)
            rDocShell.PostPaint( nPaintStartX, nPaintStartY, nStartTab,
                                 nPaintEndX,   nPaintEndY,   nEndTab,   PAINT_TOP );
    }
    aModificator.SetDocumentModified();

//! pDocSh->UpdateOle(GetViewData());   // muss an der View bleiben
//! CellContentChanged();               // muss an der View bleiben

    return TRUE;
}

BOOL ScDocFunc::MoveBlock( const ScRange& rSource, const ScAddress& rDestPos,
                                BOOL bCut, BOOL bRecord, BOOL bPaint, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    USHORT nStartCol = rSource.aStart.Col();
    USHORT nStartRow = rSource.aStart.Row();
    USHORT nStartTab = rSource.aStart.Tab();
    USHORT nEndCol = rSource.aEnd.Col();
    USHORT nEndRow = rSource.aEnd.Row();
    USHORT nEndTab = rSource.aEnd.Tab();
    USHORT nDestCol = rDestPos.Col();
    USHORT nDestRow = rDestPos.Row();
    USHORT nDestTab = rDestPos.Tab();

    if ( nStartRow > MAXROW || nEndRow > MAXROW || nDestRow > MAXROW )
    {
        DBG_ERROR("invalid row in MoveBlock");
        return FALSE;
    }

    //  zugehoerige Szenarien auch anpassen - nur wenn innerhalb einer Tabelle verschoben wird!
    BOOL bScenariosAdded = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    USHORT nTabCount = pDoc->GetTableCount();
    if ( nDestTab == nStartTab && !pDoc->IsScenario(nEndTab) )
        while ( nEndTab+1 < nTabCount && pDoc->IsScenario(nEndTab+1) )
        {
            ++nEndTab;
            bScenariosAdded = TRUE;
        }

    USHORT nSrcTabCount = nEndTab-nStartTab+1;
    USHORT nDestEndTab = nDestTab+nSrcTabCount-1;
    USHORT nTab;

    ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );

    ScMarkData aSourceMark;
    for (nTab=nStartTab; nTab<=nEndTab; nTab++)
        aSourceMark.SelectTable( nTab, TRUE );      // Source selektieren
    aSourceMark.SetMarkArea( rSource );

    pDoc->CopyToClip( nStartCol, nStartRow, nEndCol, nEndRow, bCut, pClipDoc,
                        FALSE, &aSourceMark, bScenariosAdded );

    USHORT nOldEndCol = nEndCol;
    USHORT nOldEndRow = nEndRow;
    BOOL bClipOver = FALSE;
    for (nTab=nStartTab; nTab<=nEndTab; nTab++)
    {
        USHORT nTmpEndCol = nOldEndCol;
        USHORT nTmpEndRow = nOldEndRow;
        if (pDoc->ExtendMerge( nStartCol, nStartRow, nTmpEndCol, nTmpEndRow, nTab ))
            bClipOver = TRUE;
        if ( nTmpEndCol > nEndCol ) nEndCol = nTmpEndCol;
        if ( nTmpEndRow > nEndRow ) nEndRow = nTmpEndRow;
    }

    USHORT nDestEndCol = nDestCol + ( nOldEndCol-nStartCol );
    USHORT nDestEndRow = nDestRow + ( nOldEndRow-nStartRow );

    USHORT nUndoEndCol = nDestCol + ( nEndCol-nStartCol );      // erweitert im Zielblock
    USHORT nUndoEndRow = nDestRow + ( nEndRow-nStartRow );

    if (nUndoEndCol>MAXCOL || nUndoEndRow>MAXROW)
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PASTE_FULL);
        delete pClipDoc;
        return FALSE;
    }

    //  Test auf Zellschutz

    BOOL bOk = TRUE;
    for (nTab=nDestTab; nTab<=nDestEndTab && bOk; nTab++)
        bOk = pDoc->IsBlockEditable( nTab, nDestCol,nDestRow, nUndoEndCol,nUndoEndRow );
    if (bCut)
        for (nTab=nStartTab; nTab<=nEndTab && bOk; nTab++)
            bOk = pDoc->IsBlockEditable( nTab, nStartCol,nStartRow, nEndCol,nEndRow );

    if (!bOk)
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        delete pClipDoc;
        return FALSE;
    }

    //  Test auf zusammengefasste - beim Verschieben erst nach dem Loeschen

    if (bClipOver && !bCut)
        if (pDoc->HasAttrib( nDestCol,nDestRow,nDestTab, nUndoEndCol,nUndoEndRow,nDestEndTab,
                                HASATTR_MERGED | HASATTR_OVERLAPPED ))
        {       // "Zusammenfassen nicht verschachteln !"
            if (!bApi)
                rDocShell.ErrorMessage(STR_MSSG_MOVEBLOCKTO_0);
            delete pClipDoc;
            return FALSE;
        }

    //  Linien drin? (fuer Paint)

    BOOL bSourceLines = pDoc->HasAttrib( nStartCol,nStartRow,nStartTab,
                                nEndCol,nEndRow,nEndTab, HASATTR_PAINTEXT );
    BOOL bDestLines = pDoc->HasAttrib( nDestCol,nDestRow,nDestTab,
                                nDestEndCol,nDestEndRow,nDestEndTab, HASATTR_PAINTEXT );

    //
    //  ausfuehren
    //

    ScDocument* pUndoDoc = NULL;
    ScDocument* pRefUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab, FALSE, FALSE );

        if (bCut)
        {
            pDoc->CopyToDocument( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                                    IDF_ALL, FALSE, pUndoDoc );
            pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1, FALSE, FALSE );
        }

        if ( nDestTab != nStartTab )
            pUndoDoc->AddUndoTab( nDestTab, nDestEndTab, FALSE, FALSE );
        pDoc->CopyToDocument( nDestCol, nDestRow, nDestTab,
                                    nDestEndCol, nDestEndRow, nDestEndTab,
                                    IDF_ALL, FALSE, pUndoDoc );

        pUndoData = new ScRefUndoData( pDoc );

        pDoc->BeginDrawUndo();
    }

    BOOL bSourceHeight = FALSE;     // Hoehen angepasst?
    if (bCut)
    {
        for (nTab=nStartTab; nTab<=nEndTab; nTab++)
            pDoc->DeleteAreaTab( nStartCol,nStartRow, nOldEndCol,nOldEndRow, nTab, IDF_ALL );
//!!!   pDoc->DeleteAreaTab( nStartCol,nStartRow, nEndCol,nEndRow, nStartTab, IDF_ALL );

        //  Test auf zusammengefasste

        if (bClipOver)
            if (pDoc->HasAttrib( nDestCol,nDestRow,nDestTab,
                                    nUndoEndCol,nUndoEndRow,nDestEndTab,
                                    HASATTR_MERGED | HASATTR_OVERLAPPED ))
            {
                pDoc->CopyFromClip( rSource, aSourceMark, IDF_ALL, pRefUndoDoc, pClipDoc );
                for (nTab=nStartTab; nTab<=nEndTab; nTab++)
                {
                    USHORT nTmpEndCol = nEndCol;
                    USHORT nTmpEndRow = nEndRow;
                    pDoc->ExtendMerge( nStartCol, nStartRow, nTmpEndCol, nTmpEndRow, nTab, TRUE );
                }

                //  Fehlermeldung erst nach dem Wiederherstellen des Inhalts
                if (!bApi)      // "Zusammenfassen nicht verschachteln !"
                    rDocShell.ErrorMessage(STR_MSSG_MOVEBLOCKTO_0);

                delete pUndoDoc;
                delete pRefUndoDoc;
                delete pUndoData;
                delete pClipDoc;
                return FALSE;
            }

        bSourceHeight = AdjustRowHeight( rSource, FALSE );
    }

    ScRange aPasteDest( nDestCol, nDestRow, nDestTab, nDestEndCol, nDestEndRow, nDestEndTab );

    ScMarkData aDestMark;
    for (nTab=nDestTab; nTab<=nDestEndTab; nTab++)
        aDestMark.SelectTable( nTab, TRUE );        // Destination selektieren
    aDestMark.SetMarkArea( aPasteDest );

        //! markierte Tabellen bei CopyFromClip uebergeben !!!!!
    pDoc->CopyFromClip( aPasteDest, aDestMark, IDF_ALL, pRefUndoDoc, pClipDoc );

    VirtualDevice aVirtDev;
    BOOL bDestHeight = AdjustRowHeight(
                            ScRange( 0,nDestRow,nDestTab, MAXCOL,nDestEndRow,nDestEndTab ),
                            FALSE );

    if (bRecord)
    {
        if (pRefUndoDoc)
        {
                //  alle Tabellen anlegen, damit Formeln kopiert werden koennen:
            pUndoDoc->AddUndoTab( 0, nTabCount-1, FALSE, FALSE );

            pRefUndoDoc->DeleteArea( nDestCol, nDestRow, nDestEndCol, nDestEndRow, aSourceMark, IDF_ALL );
            //  kopieren mit bColRowFlags=FALSE (#54194#)
            pRefUndoDoc->CopyToDocument( 0, 0, 0, MAXCOL, MAXROW, MAXTAB,
                                            IDF_FORMULA, FALSE, pUndoDoc, NULL, FALSE );
            delete pRefUndoDoc;
        }

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDragDrop( &rDocShell, ScRange(
                                    nStartCol, nStartRow, nStartTab,
                                    nOldEndCol, nOldEndRow, nEndTab ),
                                ScAddress( nDestCol, nDestRow, nDestTab ),
                                bCut, pUndoDoc, pUndoData, bScenariosAdded ) );
    }

    USHORT nDestPaintEndCol = nDestEndCol;
    USHORT nDestPaintEndRow = nDestEndRow;
    for (nTab=nDestTab; nTab<=nDestEndTab; nTab++)
    {
        USHORT nTmpEndCol = nDestEndCol;
        USHORT nTmpEndRow = nDestEndRow;
        pDoc->ExtendMerge( nDestCol, nDestRow, nTmpEndCol, nTmpEndRow, nTab, TRUE );
        if (nTmpEndCol > nDestPaintEndCol) nDestPaintEndCol = nTmpEndCol;
        if (nTmpEndRow > nDestPaintEndRow) nDestPaintEndRow = nTmpEndRow;
    }

    if (bCut)
        for (nTab=nStartTab; nTab<=nEndTab; nTab++)
            pDoc->RefreshAutoFilter( nStartCol, nStartRow, nEndCol, nEndRow, nTab );

    if (bPaint)
    {
            //  Zielbereich:

        USHORT nPaintStartX = nDestCol;
        USHORT nPaintStartY = nDestRow;
        USHORT nPaintEndX = nDestPaintEndCol;
        USHORT nPaintEndY = nDestPaintEndRow;
        USHORT nFlags = PAINT_GRID;
        USHORT nExt = 0;
        if ( bSourceLines || bDestLines )
            nExt |= SC_PF_LINES;

        if ( nStartRow==0 && nEndRow==MAXROW )      // Breiten mitkopiert?
        {
            nPaintEndX = MAXCOL;
            nPaintStartY = 0;
            nPaintEndY = MAXROW;
            nFlags |= PAINT_TOP;
        }
        if ( bDestHeight )
        {
            nPaintEndY = MAXROW;
            nPaintStartX = 0;
            nPaintEndX = MAXCOL;
            nFlags |= PAINT_LEFT;
        }
        if ( bScenariosAdded )
        {
            nPaintStartX = nPaintStartY = 0;
            nPaintEndX = MAXCOL;
            nPaintEndY = MAXROW;
        }

        rDocShell.PostPaint( nPaintStartX,nPaintStartY,nDestTab,
                            nPaintEndX,nPaintEndY,nDestEndTab, nFlags, nExt );

        if ( bCut )
        {
                //  Quellbereich:

            nPaintStartX = nStartCol;
            nPaintStartY = nStartRow;
            nPaintEndX = nEndCol;
            nPaintEndY = nEndRow;
            nFlags = PAINT_GRID;
            nExt = 0;
            if ( bSourceLines )
                nExt |= SC_PF_LINES;

            if ( bSourceHeight )
            {
                nPaintEndY = MAXROW;
                nPaintStartX = 0;
                nPaintEndX = MAXCOL;
                nFlags |= PAINT_LEFT;
            }
            if ( bScenariosAdded )
            {
                nPaintStartX = nPaintStartY = 0;
                nPaintEndX = MAXCOL;
                nPaintEndY = MAXROW;
            }

            rDocShell.PostPaint( nPaintStartX,nPaintStartY,nStartTab,
                                nPaintEndX,nPaintEndY,nEndTab, nFlags, nExt );
        }
    }

    aModificator.SetDocumentModified();

    delete pClipDoc;
    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::InsertTable( USHORT nTab, const String& rName, BOOL bRecord, BOOL bApi )
{
    BOOL bSuccess = FALSE;
    WaitObject aWait( rDocShell.GetDialogParent() );

    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    if (bRecord)
        pDoc->BeginDrawUndo();                          //  InsertTab erzeugt ein SdrUndoNewPage

    USHORT nTabCount = pDoc->GetTableCount();
    BOOL bAppend = ( nTab >= nTabCount );
    if ( bAppend )
        nTab = nTabCount;       // wichtig fuer Undo

    if (pDoc->InsertTab( nTab, rName ))
    {
        if (bRecord)
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoInsertTab( &rDocShell, nTab, bAppend, rName));
        //  Views updaten:
        rDocShell.Broadcast( ScTablesHint( SC_TAB_INSERTED, nTab ) );

        rDocShell.PostPaintExtras();
        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        bSuccess = TRUE;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(STR_TABINSERT_ERROR);

    return bSuccess;
}

BOOL ScDocFunc::DeleteTable( USHORT nTab, BOOL bRecord, BOOL bApi )
{
    WaitObject aWait( rDocShell.GetDialogParent() );

    ScDocShellModificator aModificator( rDocShell );

    BOOL bSuccess = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    BOOL bWasLinked = pDoc->IsLinked(nTab);
    ScDocument* pUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        USHORT nCount = pDoc->GetTableCount();

        pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );     // nur nTab mit Flags
        pUndoDoc->AddUndoTab( 0, nCount-1 );                    // alle Tabs fuer Referenzen

        pDoc->CopyToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, IDF_ALL,FALSE, pUndoDoc );
        String aOldName;
        pDoc->GetName( nTab, aOldName );
        pUndoDoc->RenameTab( nTab, aOldName, FALSE );
        if (bWasLinked)
            pUndoDoc->SetLink( nTab, pDoc->GetLinkMode(nTab), pDoc->GetLinkDoc(nTab),
                                pDoc->GetLinkFlt(nTab), pDoc->GetLinkOpt(nTab),
                                pDoc->GetLinkTab(nTab) );

        if ( pDoc->IsScenario(nTab) )
        {
            pUndoDoc->SetScenario( nTab, TRUE );
            String aComment;
            Color  aColor;
            USHORT nScenFlags;
            pDoc->GetScenarioData( nTab, aComment, aColor, nScenFlags );
            pUndoDoc->SetScenarioData( nTab, aComment, aColor, nScenFlags );
            BOOL bActive = pDoc->IsActiveScenario( nTab );
            pUndoDoc->SetActiveScenario( nTab, bActive );
        }
        pUndoDoc->SetVisible( nTab, pDoc->IsVisible( nTab ) );

        //  Drawing-Layer muss sein Undo selbst in der Hand behalten !!!
        pDoc->BeginDrawUndo();                          //  DeleteTab erzeugt ein SdrUndoDelPage

        pUndoData = new ScRefUndoData( pDoc );
    }

    if (pDoc->DeleteTab( nTab, pUndoDoc ))
    {
        if (bRecord)
        {
            SvUShorts theTabs;
            theTabs.Insert(nTab,theTabs.Count());
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDeleteTab( &rDocShell, theTabs, pUndoDoc, pUndoData ));
        }
        //  Views updaten:
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
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        bSuccess = TRUE;
    }
    else
    {
        delete pUndoDoc;
        delete pUndoData;
    }
    return bSuccess;
}

BOOL ScDocFunc::SetTableVisible( USHORT nTab, BOOL bVisible, BOOL bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    BOOL bUndo(pDoc->IsUndoEnabled());
    if ( pDoc->IsVisible( nTab ) == bVisible )
        return TRUE;                                // nichts zu tun - ok

    if ( !pDoc->IsDocEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    ScDocShellModificator aModificator( rDocShell );

    if ( !bVisible )
    {
        //  nicht alle Tabellen ausblenden

        USHORT nVisCount = 0;
        USHORT nCount = pDoc->GetTableCount();
        for (USHORT i=0; i<nCount; i++)
            if (pDoc->IsVisible(i))
                ++nVisCount;

        if (nVisCount <= 1)
        {
            if (!bApi)
                rDocShell.ErrorMessage(STR_PROTECTIONERR);  //! eigene Meldung?
            return FALSE;
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

    return TRUE;
}

BOOL ScDocFunc::RenameTable( USHORT nTab, const String& rName, BOOL bRecord, BOOL bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    if ( !pDoc->IsDocEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    ScDocShellModificator aModificator( rDocShell );

    BOOL bSuccess = FALSE;
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

        bSuccess = TRUE;
    }
    return bSuccess;
}

//------------------------------------------------------------------------

//! SetWidthOrHeight - noch doppelt zu ViewFunc !!!!!!
//! Probleme:
//! - Optimale Hoehe fuer Edit-Zellen ist unterschiedlich zwischen Drucker und Bildschirm
//! - Optimale Breite braucht Selektion, um evtl. nur selektierte Zellen zu beruecksichtigen

USHORT lcl_GetOptimalColWidth( ScDocShell& rDocShell, USHORT nCol, USHORT nTab, BOOL bFormula )
{
    USHORT nTwips = 0;

//! OutputDevice* pDev = rDocShell.GetPrinter();
    VirtualDevice aVDev;
    OutputDevice* pDev = &aVDev;        // aus Kompatibilitaetsgruenden...

    if (pDev)
    {
        ScDocument* pDoc = rDocShell.GetDocument();
        Fraction aOne(1,1);
        MapMode aOldMap = pDev->GetMapMode();
        pDev->SetMapMode( MAP_PIXEL );              // wichtig fuer GetNeededSize
        Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
        double nPPTX = aPix1000.X() / 1000.0;
        double nPPTY = aPix1000.Y() / 1000.0;
        nTwips = pDoc->GetOptimalColWidth( nCol, nTab, pDev, nPPTX, nPPTY, aOne, aOne,
                                            bFormula, NULL );
        pDev->SetMapMode( aOldMap );        //! noetig ???
    }
    return nTwips;
}

BOOL ScDocFunc::SetWidthOrHeight( BOOL bWidth, USHORT nRangeCnt, USHORT* pRanges, USHORT nTab,
                                        ScSizeMode eMode, USHORT nSizeTwips,
                                        BOOL bRecord, BOOL bApi )
{
    if (!nRangeCnt)
        return TRUE;

    ScDocument* pDoc = rDocShell.GetDocument();
    if ( bRecord && !pDoc->IsUndoEnabled() )
        bRecord = FALSE;

    if ( !rDocShell.IsEditable() )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);      //! eigene Meldung?
        return FALSE;
    }

    BOOL bSuccess = FALSE;
    USHORT nStart = pRanges[0];
    USHORT nEnd = pRanges[2*nRangeCnt-1];

    BOOL bFormula = FALSE;
    if ( eMode == SC_SIZE_OPTIMAL )
    {
        //! Option "Formeln anzeigen" - woher nehmen?
    }

    ScDocument*     pUndoDoc = NULL;
    ScOutlineTable* pUndoTab = NULL;
    USHORT*         pUndoRanges = NULL;

    if ( bRecord )
    {
        pDoc->BeginDrawUndo();                          // Drawing Updates

        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bWidth)
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, FALSE );
            pDoc->CopyToDocument( nStart, 0, nTab, nEnd, MAXROW, nTab, IDF_NONE, FALSE, pUndoDoc );
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, FALSE, TRUE );
            pDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, FALSE, pUndoDoc );
        }

        pUndoRanges = new USHORT[ 2*nRangeCnt ];
        memmove( pUndoRanges, pRanges, 2*nRangeCnt*sizeof(USHORT) );

        ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
        if (pTable)
            pUndoTab = new ScOutlineTable( *pTable );
    }

    BOOL bShow = nSizeTwips > 0 || eMode != SC_SIZE_DIRECT;
    BOOL bOutline = FALSE;

    pDoc->IncSizeRecalcLevel( nTab );       // nicht fuer jede Spalte einzeln
    for (USHORT nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
    {
        USHORT nStartNo = *(pRanges++);
        USHORT nEndNo = *(pRanges++);

        if ( !bWidth )                      // Hoehen immer blockweise
        {
            if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
            {
                BOOL bAll = ( eMode==SC_SIZE_OPTIMAL );
                if (!bAll)
                {
                    //  fuer alle eingeblendeten CR_MANUALSIZE loeschen,
                    //  dann SetOptimalHeight mit bShrink = FALSE
                    for (USHORT nRow=nStartNo; nRow<=nEndNo; nRow++)
                    {
                        BYTE nOld = pDoc->GetRowFlags(nRow,nTab);
                        if ( (nOld & CR_HIDDEN) == 0 && ( nOld & CR_MANUALSIZE ) )
                            pDoc->SetRowFlags( nRow, nTab, nOld & ~CR_MANUALSIZE );
                    }
                }

//!             OutputDevice* pDev = rDocShell.GetPrinter();
                VirtualDevice aVDev;
                OutputDevice* pDev = &aVDev;        // aus Kompatibilitaetsgruenden...

                if (pDev)
                {
                    Fraction aOne(1,1);
                    MapMode aOldMap = pDev->GetMapMode();
                    pDev->SetMapMode( MAP_PIXEL );              // wichtig fuer GetNeededSize
                    Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
                    double nPPTX = aPix1000.X() / 1000.0;
                    double nPPTY = aPix1000.Y() / 1000.0;
                    pDoc->SetOptimalHeight( nStartNo, nEndNo, nTab, 0, pDev,
                                            nPPTX, nPPTY, aOne, aOne, bAll );
                    pDev->SetMapMode( aOldMap );        //! noetig ???
                }

                if (bAll)
                    pDoc->ShowRows( nStartNo, nEndNo, nTab, TRUE );

                //  Manual-Flag wird bei bAll=TRUE schon in SetOptimalHeight gesetzt
                //  (an bei Extra-Height, sonst aus).
            }
            else if ( eMode==SC_SIZE_DIRECT || eMode==SC_SIZE_ORIGINAL )
            {
                if (nSizeTwips)
                    pDoc->SetRowHeightRange( nStartNo, nEndNo, nTab, nSizeTwips );
                if ( eMode != SC_SIZE_ORIGINAL )
                    pDoc->ShowRows( nStartNo, nEndNo, nTab, nSizeTwips != 0 );
                pDoc->SetManualHeight( nStartNo, nEndNo, nTab, TRUE );      // Manual-Flag
            }
            else if ( eMode==SC_SIZE_SHOW )
            {
                pDoc->ShowRows( nStartNo, nEndNo, nTab, TRUE );
            }
        }
        else                                // Spaltenbreiten
        {
            for (USHORT nCol=nStartNo; nCol<=nEndNo; nCol++)
            {
                if ( eMode != SC_SIZE_VISOPT ||
                     (pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN) == 0 )
                {
                    USHORT nThisSize = nSizeTwips;

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
                bOutline = bOutline || pDoc->UpdateOutlineCol( nStartNo, nEndNo, nTab, bShow );
            else
                bOutline = bOutline || pDoc->UpdateOutlineRow( nStartNo, nEndNo, nTab, bShow );
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

    return bSuccess;
}


BOOL ScDocFunc::InsertPageBreak( BOOL bColumn, const ScAddress& rPos,
                                BOOL bRecord, BOOL bSetModified, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    USHORT nTab = rPos.Tab();
    SfxBindings* pBindings = rDocShell.GetViewBindings();

    USHORT nPos = bColumn ? rPos.Col() : rPos.Row();
    if (nPos == 0)
        return FALSE;                   // erste Spalte / Zeile

    BYTE nFlags = bColumn ? pDoc->GetColFlags( nPos, nTab ) : pDoc->GetRowFlags( nPos, nTab );
    if (nFlags & CR_MANUALBREAK)
        return TRUE;                    // Umbruch schon gesetzt

    if (bRecord)
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoPageBreak( &rDocShell, rPos.Col(), rPos.Row(), nTab, bColumn, TRUE ) );

    nFlags |= CR_MANUALBREAK;
    if (bColumn)
        pDoc->SetColFlags( nPos, nTab, nFlags );
    else
        pDoc->SetRowFlags( nPos, nTab, nFlags );
    pDoc->UpdatePageBreaks( nTab );

    if (bColumn)
    {
        rDocShell.PostPaint( nPos-1, 0, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID );
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

    return TRUE;
}

BOOL ScDocFunc::RemovePageBreak( BOOL bColumn, const ScAddress& rPos,
                                BOOL bRecord, BOOL bSetModified, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    USHORT nTab = rPos.Tab();
    SfxBindings* pBindings = rDocShell.GetViewBindings();

    USHORT nPos = bColumn ? rPos.Col() : rPos.Row();
    BYTE nFlags = bColumn ? pDoc->GetColFlags( nPos, nTab ) : pDoc->GetRowFlags( nPos, nTab );
    if ((nFlags & CR_MANUALBREAK)==0)
        return FALSE;                           // kein Umbruch gesetzt

    if (bRecord)
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoPageBreak( &rDocShell, rPos.Col(), rPos.Row(), nTab, bColumn, FALSE ) );

    nFlags &= ~CR_MANUALBREAK;
    if (bColumn)
        pDoc->SetColFlags( nPos, nTab, nFlags );
    else
        pDoc->SetRowFlags( nPos, nTab, nFlags );
    pDoc->UpdatePageBreaks( nTab );

    if (bColumn)
    {
        rDocShell.PostPaint( nPos-1, 0, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID );
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

    return TRUE;
}

//------------------------------------------------------------------------

BOOL lcl_ValidPassword( ScDocument* pDoc, USHORT nTab,
                        const uno::Sequence<sal_Int8>& rPassword, uno::Sequence<sal_Int8>* pReturnOld = NULL )
{
    uno::Sequence<sal_Int8> aOldPassword;
    if ( nTab == TABLEID_DOC )
    {
        if (pDoc->IsDocProtected())
            aOldPassword = pDoc->GetDocPassword();
    }
    else
    {
        if (pDoc->IsTabProtected(nTab))
            aOldPassword = pDoc->GetTabPassword(nTab);
    }

    if (pReturnOld)
        *pReturnOld = aOldPassword;

    return ((aOldPassword.getLength() == 0) || (rPassword == aOldPassword));
}

BOOL ScDocFunc::Protect( USHORT nTab, const String& rPassword, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    BOOL bUndo(pDoc->IsUndoEnabled());
    uno::Sequence<sal_Int8> aPass;
    if (rPassword.Len())
        SvPasswordHelper::GetHashPassword(aPass, rPassword);
    BOOL bOk = lcl_ValidPassword( pDoc, nTab, aPass );

    if ( bOk )
    {
        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoProtect( &rDocShell, nTab, TRUE, aPass ) );
        }

        if ( nTab == TABLEID_DOC )
            pDoc->SetDocProtection( TRUE, aPass );
        else
            pDoc->SetTabProtection( nTab, TRUE, aPass );

        rDocShell.PostPaintGridAll();
        aModificator.SetDocumentModified();
    }
    else if (!bApi)
    {
        //  different password was set before

//!     rDocShell.ErrorMessage(...);

        InfoBox aBox( rDocShell.GetDialogParent(), String( ScResId( SCSTR_WRONGPASSWORD ) ) );
        aBox.Execute();
    }

    return bOk;
}

BOOL ScDocFunc::Unprotect( USHORT nTab, const String& rPassword, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    BOOL bUndo(pDoc->IsUndoEnabled());
    uno::Sequence<sal_Int8> aOldPassword;
    uno::Sequence<sal_Int8> aPass;
    if (rPassword.Len())
        SvPasswordHelper::GetHashPassword(aPass, rPassword);
    BOOL bOk = lcl_ValidPassword( pDoc, nTab, aPass, &aOldPassword );

    if ( bOk )
    {
        uno::Sequence<sal_Int8> aEmptyPass;
        if ( nTab == TABLEID_DOC )
            pDoc->SetDocProtection( FALSE, aEmptyPass );
        else
            pDoc->SetTabProtection( nTab, FALSE, aEmptyPass );

        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoProtect( &rDocShell, nTab, FALSE, aOldPassword ) );
        }

        rDocShell.PostPaintGridAll();
        aModificator.SetDocumentModified();
    }
    else if (!bApi)
    {
//!     rDocShell.ErrorMessage(...);

        InfoBox aBox( rDocShell.GetDialogParent(), String( ScResId( SCSTR_WRONGPASSWORD ) ) );
        aBox.Execute();
    }

    return bOk;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::ClearItems( const ScMarkData& rMark, const USHORT* pWhich, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    BOOL bUndo (pDoc->IsUndoEnabled());
    if (!pDoc->IsSelectionEditable(rMark))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    ScRange aMarkRange;
    rMark.GetMultiMarkArea( aMarkRange );

//  if (bRecord)
    if (bUndo)
    {
        USHORT nStartTab = aMarkRange.aStart.Tab();
        USHORT nEndTab = aMarkRange.aEnd.Tab();

        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab );
        pDoc->CopyToDocument( aMarkRange, IDF_ATTRIB, TRUE, pUndoDoc, (ScMarkData*)&rMark );

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoClearItems( &rDocShell, rMark, pUndoDoc, pWhich ) );
    }

    pDoc->ClearSelectionItems( pWhich, rMark );

    rDocShell.PostPaint( aMarkRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );
    aModificator.SetDocumentModified();

    //! Bindings-Invalidate etc.?

    return TRUE;
}

BOOL ScDocFunc::ChangeIndent( const ScMarkData& rMark, BOOL bIncrement, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    BOOL bUndo(pDoc->IsUndoEnabled());
    if (!pDoc->IsSelectionEditable(rMark))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    ScRange aMarkRange;
    rMark.GetMultiMarkArea( aMarkRange );

//  if (bRecord)
    if (bUndo)
    {
        USHORT nStartTab = aMarkRange.aStart.Tab();
        USHORT nTabCount = pDoc->GetTableCount();

        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
        for (USHORT i=0; i<nTabCount; i++)
            if (i != nStartTab && rMark.GetTableSelect(i))
                pUndoDoc->AddUndoTab( i, i );

        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, TRUE, pUndoDoc, (ScMarkData*)&rMark );

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
    }

    return TRUE;
}

BOOL ScDocFunc::AutoFormat( const ScRange& rRange, const ScMarkData* pTabMark,
                            USHORT nFormatNo, BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    BOOL bSuccess = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (USHORT nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, TRUE );
    }

    ScAutoFormat* pAutoFormat = ScGlobal::GetAutoFormat();
    if ( pAutoFormat && nFormatNo < pAutoFormat->GetCount() &&
            pDoc->IsSelectedBlockEditable( nStartCol,nStartRow, nEndCol,nEndRow, aMark ) )
    {
        WaitObject aWait( rDocShell.GetDialogParent() );

        BOOL bSize = (*pAutoFormat)[nFormatNo]->GetIncludeWidthHeight();

        USHORT nTabCount = pDoc->GetTableCount();
        ScDocument* pUndoDoc = NULL;
        if ( bRecord )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab, bSize, bSize );
            for (USHORT i=0; i<nTabCount; i++)
                if (i != nStartTab && aMark.GetTableSelect(i))
                    pUndoDoc->AddUndoTab( i, i, bSize, bSize );

            ScRange aCopyRange = rRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aStart.SetTab(nTabCount-1);
            pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, FALSE, pUndoDoc, &aMark );
            if (bSize)
            {
                pDoc->CopyToDocument( nStartCol,0,0, nEndCol,MAXROW,nTabCount-1,
                                                            IDF_NONE, FALSE, pUndoDoc, &aMark );
                pDoc->CopyToDocument( 0,nStartRow,0, MAXCOL,nEndRow,nTabCount-1,
                                                            IDF_NONE, FALSE, pUndoDoc, &aMark );
            }
            pDoc->BeginDrawUndo();
        }

        pDoc->AutoFormat( nStartCol, nStartRow, nEndCol, nEndRow, nFormatNo, aMark );

        if (bSize)
        {
/*          USHORT nCols[2];
            nCols[0] = nStartCol;
            nCols[1] = nEndCol;
            USHORT nRows[2];
            nRows[0] = nStartRow;
            nRows[1] = nEndRow;
*/
            USHORT nCols[2] = { nStartCol, nEndCol };
            USHORT nRows[2] = { nStartRow, nEndRow };

            for (USHORT nTab=0; nTab<nTabCount; nTab++)
                if (aMark.GetTableSelect(nTab))
                {
                    SetWidthOrHeight( TRUE, 1,nCols, nTab, SC_SIZE_VISOPT, STD_EXTRA_WIDTH, FALSE, TRUE );
                    SetWidthOrHeight( FALSE,1,nRows, nTab, SC_SIZE_VISOPT, 0, FALSE, FALSE );
                    rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab,
                                    PAINT_GRID | PAINT_LEFT | PAINT_TOP );
                }
        }
        else
        {
            for (USHORT nTab=0; nTab<nTabCount; nTab++)
                if (aMark.GetTableSelect(nTab))
                {
                    BOOL bAdj = AdjustRowHeight( ScRange(nStartCol, nStartRow, nTab,
                                                        nEndCol, nEndRow, nTab), FALSE );
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
        rDocShell.ErrorMessage(STR_PROTECTIONERR);

    return bSuccess;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::EnterMatrix( const ScRange& rRange, const ScMarkData* pTabMark,
                                const String& rString, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    BOOL bSuccess = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();

    BOOL bUndo(pDoc->IsUndoEnabled());

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (USHORT nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, TRUE );
    }

    if ( pDoc->IsSelectedBlockEditable( nStartCol,nStartRow, nEndCol,nEndRow, aMark ) )
    {
        WaitObject aWait( rDocShell.GetDialogParent() );

        ScDocument* pUndoDoc;
//      if (bRecord)    // immer
        if (bUndo)
        {
            //! auch bei Undo selektierte Tabellen beruecksichtigen
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab );
            pDoc->CopyToDocument( rRange, IDF_ALL, FALSE, pUndoDoc );
        }

        if ( pDoc->IsImportingXML() )
        {
            ScTokenArray* pCode = lcl_ScDocFunc_CreateTokenArrayXML( rString );
            pDoc->InsertMatrixFormula(nStartCol,nStartRow,nEndCol,nEndRow,aMark,EMPTY_STRING,pCode);
            delete pCode;
        }
        else
            pDoc->InsertMatrixFormula(nStartCol,nStartRow,nEndCol,nEndRow,aMark,rString);

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

        bSuccess = TRUE;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(STR_PROTECTIONERR);

    return bSuccess;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::TabOp( const ScRange& rRange, const ScMarkData* pTabMark,
                            const ScTabOpParam& rParam, BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    BOOL bSuccess = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (USHORT nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, TRUE );
    }

    if ( pDoc->IsSelectedBlockEditable( nStartCol,nStartRow, nEndCol,nEndRow, aMark ) )
    {
        WaitObject aWait( rDocShell.GetDialogParent() );
        pDoc->SetDirty( rRange );
        if ( bRecord )
        {
            //! auch bei Undo selektierte Tabellen beruecksichtigen
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab );
            pDoc->CopyToDocument( rRange, IDF_ALL, FALSE, pUndoDoc );

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
        bSuccess = TRUE;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(STR_PROTECTIONERR);

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

BOOL ScDocFunc::FillSimple( const ScRange& rRange, const ScMarkData* pTabMark,
                            FillDir eDir, BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    BOOL bSuccess = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (USHORT nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, TRUE );
    }

    if ( pDoc->IsSelectedBlockEditable( nStartCol,nStartRow, nEndCol,nEndRow, aMark ) )
    {
        WaitObject aWait( rDocShell.GetDialogParent() );

        ScRange aSourceArea = rRange;
        ScRange aDestArea   = rRange;

        USHORT nCount;
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
            USHORT nTabCount = pDoc->GetTableCount();
            USHORT nStartTab = aDestArea.aStart.Tab();

            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
            for (USHORT i=0; i<nTabCount; i++)
                if (i != nStartTab && aMark.GetTableSelect(i))
                    pUndoDoc->AddUndoTab( i, i );

            ScRange aCopyRange = aDestArea;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);
            pDoc->CopyToDocument( aCopyRange, IDF_ALL, FALSE, pUndoDoc, &aMark );
            pDoc->BeginDrawUndo();
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

        bSuccess = TRUE;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(STR_PROTECTIONERR);

    return bSuccess;
}

BOOL ScDocFunc::FillSeries( const ScRange& rRange, const ScMarkData* pTabMark,
                            FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                            double fStart, double fStep, double fMax,
                            BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    BOOL bSuccess = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (USHORT nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, TRUE );
    }

    if ( pDoc->IsSelectedBlockEditable( nStartCol,nStartRow, nEndCol,nEndRow, aMark ) )
    {
        WaitObject aWait( rDocShell.GetDialogParent() );

        ScRange aSourceArea = rRange;
        ScRange aDestArea   = rRange;

        USHORT nCount = pDoc->GetEmptyLinesInBlock(
                aSourceArea.aStart.Col(), aSourceArea.aStart.Row(), aSourceArea.aStart.Tab(),
                aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), aSourceArea.aEnd.Tab(),
                DirFromFillDir(eDir) );

        //  #27665# mindestens eine Zeile/Spalte als Quellbereich behalten:
        USHORT nTotLines = ( eDir == FILL_TO_BOTTOM || eDir == FILL_TO_TOP ) ?
                            ( aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1 ) :
                            ( aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1 );
        if ( nCount >= nTotLines )
            nCount = nTotLines - 1;

        switch (eDir)
        {
            case FILL_TO_BOTTOM:
                aSourceArea.aEnd.SetRow( aSourceArea.aEnd.Row() - nCount );
                break;
            case FILL_TO_RIGHT:
                aSourceArea.aEnd.SetCol( aSourceArea.aEnd.Col() - nCount );
                break;
            case FILL_TO_TOP:
                aSourceArea.aStart.SetRow( aSourceArea.aStart.Row() + nCount );
                break;
            case FILL_TO_LEFT:
                aSourceArea.aStart.SetCol( aSourceArea.aStart.Col() + nCount );
                break;
        }

        ScDocument* pUndoDoc = NULL;
        if ( bRecord )
        {
            USHORT nTabCount = pDoc->GetTableCount();
            USHORT nStartTab = aDestArea.aStart.Tab();

            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
            for (USHORT i=0; i<nTabCount; i++)
                if (i != nStartTab && aMark.GetTableSelect(i))
                    pUndoDoc->AddUndoTab( i, i );

            pDoc->CopyToDocument(
                aDestArea.aStart.Col(), aDestArea.aStart.Row(), 0,
                aDestArea.aEnd.Col(), aDestArea.aEnd.Row(), nTabCount-1,
                IDF_ALL, FALSE, pUndoDoc, &aMark );
            pDoc->BeginDrawUndo();
        }

        if (aDestArea.aStart.Col() <= aDestArea.aEnd.Col() &&
            aDestArea.aStart.Row() <= aDestArea.aEnd.Row())
        {
            if ( fStart != MAXDOUBLE )
            {
                USHORT nValX = (eDir == FILL_TO_LEFT) ? aDestArea.aEnd.Col() : aDestArea.aStart.Col();
                USHORT nValY = (eDir == FILL_TO_TOP ) ? aDestArea.aEnd.Row() : aDestArea.aStart.Row();
                USHORT nTab = aDestArea.aStart.Tab();
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

        bSuccess = TRUE;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(STR_PROTECTIONERR);

    return bSuccess;
}

BOOL ScDocFunc::FillAuto( ScRange& rRange, const ScMarkData* pTabMark,
                            FillDir eDir, USHORT nCount, BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    ScMarkData aMark;
    if (pTabMark)
        aMark = *pTabMark;
    else
    {
        for (USHORT nTab=nStartTab; nTab<=nEndTab; nTab++)
            aMark.SelectTable( nTab, TRUE );
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
            aDestArea.aEnd.SetRow( aSourceArea.aEnd.Row() + nCount );
            break;
        case FILL_TO_TOP:
            if (nCount > aSourceArea.aStart.Row())
            {
                DBG_ERROR("FillAuto: Row < 0");
                nCount = aSourceArea.aStart.Row();
            }
            aDestArea.aStart.SetRow( aSourceArea.aStart.Row() - nCount );
            break;
        case FILL_TO_RIGHT:
            aDestArea.aEnd.SetCol( aSourceArea.aEnd.Col() + nCount );
            break;
        case FILL_TO_LEFT:
            if (nCount > aSourceArea.aStart.Col())
            {
                DBG_ERROR("FillAuto: Col < 0");
                nCount = aSourceArea.aStart.Col();
            }
            aDestArea.aStart.SetCol( aSourceArea.aStart.Col() - nCount );
            break;
        default:
            DBG_ERROR("Falsche Richtung bei FillAuto");
            break;
    }

    //      Zellschutz testen
    //!     Quellbereich darf geschuetzt sein !!!
    //!     aber kein Matrixfragment enthalten !!!

    if ( !pDoc->IsBlockEditable(aDestArea.aStart.Tab(),
                        aDestArea.aStart.Col(), aDestArea.aStart.Row(),
                        aDestArea.aEnd.Col(), aDestArea.aEnd.Row())
      || pDoc->HasSelectedBlockMatrixFragment( nStartCol, nStartRow,
            nEndCol, nEndRow, aMark ) )
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    WaitObject aWait( rDocShell.GetDialogParent() );

    ScDocument* pUndoDoc = NULL;
    if ( bRecord )
    {
        USHORT nTabCount = pDoc->GetTableCount();
        USHORT nStartTab = aDestArea.aStart.Tab();

        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
        for (USHORT i=0; i<nTabCount; i++)
            if (i != nStartTab && aMark.GetTableSelect(i))
                pUndoDoc->AddUndoTab( i, i );

        pDoc->CopyToDocument(
            aDestArea.aStart.Col(), aDestArea.aStart.Row(), 0,
            aDestArea.aEnd.Col(), aDestArea.aEnd.Row(), nTabCount-1,
            IDF_ALL, FALSE, pUndoDoc, &aMark );
        pDoc->BeginDrawUndo();
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
    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::MergeCells( const ScRange& rRange, BOOL bContents, BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nTab = rRange.aStart.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    if (!pDoc->IsBlockEditable( nTab, nStartCol, nStartRow, nEndCol, nEndRow ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    if ( nStartCol == nEndCol && nStartRow == nEndRow )
    {
        // nichts zu tun
        return TRUE;
    }

    if ( pDoc->HasAttrib( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                            HASATTR_MERGED | HASATTR_OVERLAPPED ) )
    {
        // "Zusammenfassen nicht verschachteln !"
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_MERGECELLS_0);
        return FALSE;
    }

    BOOL bNeedContents = bContents &&
            ( !pDoc->IsBlockEmpty( nTab, nStartCol,nStartRow+1, nStartCol,nEndRow ) ||
              !pDoc->IsBlockEmpty( nTab, nStartCol+1,nStartRow, nEndCol,nEndRow ) );

    if (bRecord)
    {
        ScDocument* pUndoDoc = NULL;
        if (bNeedContents && bContents)
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab );
            pDoc->CopyToDocument( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                    IDF_ALL, FALSE, pUndoDoc );
        }
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoMerge( &rDocShell,
                            nStartCol, nStartRow, nTab,
                            nEndCol, nEndRow, nTab, TRUE, pUndoDoc ) );
    }

    if (bNeedContents && bContents)
        pDoc->DoMergeContents( nTab, nStartCol,nStartRow, nEndCol,nEndRow );
    pDoc->DoMerge( nTab, nStartCol,nStartRow, nEndCol,nEndRow );

    if ( !AdjustRowHeight( ScRange( 0,nStartRow,nTab, MAXCOL,nEndRow,nTab ) ) )
        rDocShell.PostPaint( nStartCol, nStartRow, nTab,
                                            nEndCol, nEndRow, nTab, PAINT_GRID );
    if (bNeedContents && bContents)
        pDoc->SetDirty( rRange );
    aModificator.SetDocumentModified();

    SfxBindings* pBindings = rDocShell.GetViewBindings();
    if (pBindings)
    {
        pBindings->Invalidate( FID_MERGE_ON );
        pBindings->Invalidate( FID_MERGE_OFF );
    }

    return TRUE;
}

BOOL ScDocFunc::UnmergeCells( const ScRange& rRange, BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nTab = rRange.aStart.Tab();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

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
            pDoc->CopyToDocument( aExtended, IDF_ATTRIB, FALSE, pUndoDoc );
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

        pDoc->ExtendMerge( aRefresh, TRUE, FALSE );

        if ( !AdjustRowHeight( aExtended ) )
            rDocShell.PostPaint( aExtended, PAINT_GRID );
        aModificator.SetDocumentModified();
    }
    else if (!bApi)
        Sound::Beep();      //! FALSE zurueck???

    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::SetNote( const ScAddress& rPos, const ScPostIt& rNote, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    BOOL bDone = FALSE;
    USHORT nCol = rPos.Col();
    USHORT nRow = rPos.Row();
    USHORT nTab = rPos.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();
    BOOL bUndo (pDoc->IsUndoEnabled());
    if (pDoc->IsBlockEditable( nTab, nCol,nRow, nCol,nRow ))
    {
        pDoc->SetNote( nCol, nRow, nTab, rNote );

        if (bUndo)
        {
            ScPostIt aOld;
            pDoc->GetNote( nCol, nRow, nTab, aOld );
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoEditNote( &rDocShell, rPos, aOld, rNote ) );
        }

        rDocShell.PostPaintCell( nCol, nRow, nTab );
        aModificator.SetDocumentModified();
        bDone = TRUE;
    }
    else if (!bApi)
        rDocShell.ErrorMessage(STR_PROTECTIONERR);

    return bDone;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::ModifyRangeNames( const ScRangeName& rNewRanges, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    BOOL bUndo(pDoc->IsUndoEnabled());

    if (bUndo)
    {
        ScRangeName* pOld = pDoc->GetRangeName();
        ScRangeName* pUndoRanges = new ScRangeName(*pOld);
        ScRangeName* pRedoRanges = new ScRangeName(rNewRanges);
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoRangeNames( &rDocShell, pUndoRanges, pRedoRanges ) );
    }

    pDoc->CompileNameFormula( TRUE );   // CreateFormulaString
    pDoc->SetRangeName( new ScRangeName( rNewRanges ) );
    pDoc->CompileNameFormula( FALSE );  // CompileFormulaString

    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );

    return TRUE;
}

//------------------------------------------------------------------------

void ScDocFunc::CreateOneName( ScRangeName& rList,
                                USHORT nPosX, USHORT nPosY, USHORT nTab,
                                USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                BOOL& rCancel, BOOL bApi )
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
            ScRangeUtil().MakeAreaString( ScArea( nTab,nX1,nY1,nX2,nY2 ),
                                            aContent, pDoc, FALSE );

            ScRangeName* pList = pDoc->GetRangeName();

            BOOL bInsert = FALSE;
            USHORT nOldPos;
            if (rList.SearchName( aName, nOldPos ))         // vorhanden ?
            {
                ScRangeData* pOld = rList[nOldPos];
                String aOldStr;
                pOld->GetSymbol( aOldStr );
                if (aOldStr != aContent)
                {
                    if (bApi)
                        bInsert = TRUE;     // per API nicht nachfragen
                    else
                    {
                        String aTemplate = ScGlobal::GetRscString( STR_CREATENAME_REPLACE );

                        String aMessage = aTemplate.GetToken( 0, '#' );
                        aMessage += aName;
                        aMessage += aTemplate.GetToken( 1, '#' );

                        short nResult = QueryBox( rDocShell.GetDialogParent(),
                                                    WinBits(WB_YES_NO_CANCEL | WB_DEF_YES),
                                                    aMessage ).Execute();
                        if ( nResult == RET_YES )
                        {
                            rList.AtFree(nOldPos);
                            bInsert = TRUE;
                        }
                        else if ( nResult == RET_CANCEL )
                            rCancel = TRUE;
                    }
                }
            }
            else
                bInsert = TRUE;

            if (bInsert)
            {
                ScRangeData* pData = new ScRangeData( pDoc, aName, aContent, nPosX, nPosY, nTab );
                if (!rList.Insert(pData))
                {
                    DBG_ERROR("nanu?");
                    delete pData;
                }
            }
        }
    }
}

BOOL ScDocFunc::CreateNames( const ScRange& rRange, USHORT nFlags, BOOL bApi )
{
    if (!nFlags)
        return FALSE;       // war nix

    ScDocShellModificator aModificator( rDocShell );

    BOOL bDone = FALSE;
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nTab = rRange.aStart.Tab();
    DBG_ASSERT(rRange.aEnd.Tab() == nTab, "CreateNames: mehrere Tabellen geht nicht");

    BOOL bValid = TRUE;
    if ( nFlags & ( NAME_TOP | NAME_BOTTOM ) )
        if ( nStartRow == nEndRow )
            bValid = FALSE;
    if ( nFlags & ( NAME_LEFT | NAME_RIGHT ) )
        if ( nStartCol == nEndCol )
            bValid = FALSE;

    if (bValid)
    {
        ScDocument* pDoc = rDocShell.GetDocument();
        ScRangeName* pNames = pDoc->GetRangeName();
        if (!pNames)
            return FALSE;   // soll nicht sein
        ScRangeName aNewRanges( *pNames );

        BOOL bTop    = ( ( nFlags & NAME_TOP ) != 0 );
        BOOL bLeft   = ( ( nFlags & NAME_LEFT ) != 0 );
        BOOL bBottom = ( ( nFlags & NAME_BOTTOM ) != 0 );
        BOOL bRight  = ( ( nFlags & NAME_RIGHT ) != 0 );

        USHORT nContX1 = nStartCol;
        USHORT nContY1 = nStartRow;
        USHORT nContX2 = nEndCol;
        USHORT nContY2 = nEndRow;

        if ( bTop )
            ++nContY1;
        if ( bLeft )
            ++nContX1;
        if ( bBottom )
            --nContY2;
        if ( bRight )
            --nContX2;

        BOOL bCancel = FALSE;
        USHORT i;

        if ( bTop )
            for (i=nContX1; i<=nContX2; i++)
                CreateOneName( aNewRanges, i,nStartRow,nTab, i,nContY1,i,nContY2, bCancel, bApi );
        if ( bLeft )
            for (i=nContY1; i<=nContY2; i++)
                CreateOneName( aNewRanges, nStartCol,i,nTab, nContX1,i,nContX2,i, bCancel, bApi );
        if ( bBottom )
            for (i=nContX1; i<=nContX2; i++)
                CreateOneName( aNewRanges, i,nEndRow,nTab, i,nContY1,i,nContY2, bCancel, bApi );
        if ( bRight )
            for (i=nContY1; i<=nContY2; i++)
                CreateOneName( aNewRanges, nEndCol,i,nTab, nContX1,i,nContX2,i, bCancel, bApi );

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

BOOL ScDocFunc::InsertNameList( const ScAddress& rStartPos, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );


    BOOL bDone = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    const BOOL bRecord = pDoc->IsUndoEnabled();
    USHORT nTab = rStartPos.Tab();
    ScDocument* pUndoDoc = NULL;

    ScRangeName* pList = pDoc->GetRangeName();
    USHORT nCount = pList->GetCount();
    USHORT nValidCount = 0;
    USHORT i;
    for (i=0; i<nCount; i++)
    {
        ScRangeData* pData = (*pList)[i];
        if ( !pData->HasType( RT_DATABASE ) && !pData->HasType( RT_SHARED ) )
            ++nValidCount;
    }

    if (nValidCount)
    {
        USHORT nStartCol = rStartPos.Col();
        USHORT nStartRow = rStartPos.Row();
        USHORT nEndCol = nStartCol + 1;
        USHORT nEndRow = nStartRow + nValidCount - 1;

        if (pDoc->IsBlockEditable( nTab, nStartCol,nStartRow, nEndCol,nEndRow ))
        {
            if (bRecord)
            {
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pUndoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab,
                                        IDF_ALL, FALSE, pUndoDoc );

                pDoc->BeginDrawUndo();      // wegen Hoehenanpassung
            }

            ScRangeData** ppSortArray = new ScRangeData* [ nValidCount ];
            USHORT j = 0;
            for (i=0; i<nCount; i++)
            {
                ScRangeData* pData = (*pList)[i];
                if ( !pData->HasType( RT_DATABASE ) && !pData->HasType( RT_SHARED ) )
                    ppSortArray[j++] = pData;
            }
#ifndef ICC
            qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                &ScRangeData::QsortNameCompare );
#else
            qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                ICCQsortNameCompare );
#endif
            String aName;
            String aContent;
            String aFormula;
            USHORT nOutRow = nStartRow;
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
                                        IDF_ALL, FALSE, pRedoDoc );

                rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoListNames( &rDocShell,
                                ScRange( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab ),
                                pUndoDoc, pRedoDoc ) );
            }

            if (!AdjustRowHeight(ScRange(0,nStartRow,nTab,MAXCOL,nEndRow,nTab)))
                rDocShell.PostPaint( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab, PAINT_GRID );
//!         rDocShell.UpdateOle(GetViewData());
            aModificator.SetDocumentModified();
            bDone = TRUE;
        }
        else if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
    }
    return bDone;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::ResizeMatrix( const ScRange& rOldRange, const ScAddress& rNewEnd, BOOL bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nStartCol = rOldRange.aStart.Col();
    USHORT nStartRow = rOldRange.aStart.Row();
    USHORT nNewEndCol = rNewEnd.Col();
    USHORT nNewEndRow = rNewEnd.Row();
    USHORT nTab = rOldRange.aStart.Tab();

    BOOL bUndo(pDoc->IsUndoEnabled());

    BOOL bRet = FALSE;

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
        aMark.SelectTable( nTab, TRUE );
        ScRange aNewRange( rOldRange.aStart, rNewEnd );

        if ( DeleteContents( aMark, IDF_CONTENTS, TRUE, bApi ) )
        {
            bRet = EnterMatrix( aNewRange, &aMark, aFormula, bApi );
            if (!bRet)
            {
                //  versuchen, alten Zustand wiederherzustellen
                EnterMatrix( rOldRange, &aMark, aFormula, bApi );
            }
        }

        if (bUndo)
            rDocShell.GetUndoManager()->LeaveListAction();
    }

    return bRet;
}

//------------------------------------------------------------------------

BOOL ScDocFunc::InsertAreaLink( const String& rFile, const String& rFilter,
                                const String& rOptions, const String& rSource,
                                const ScRange& rDestRange, BOOL bFitBlock, BOOL bApi )
{
    //! auch fuer ScViewFunc::InsertAreaLink benutzen!

    ScDocument* pDoc = rDocShell.GetDocument();
    BOOL bUndo (pDoc->IsUndoEnabled());

    String aFilterName = rFilter;
    String aNewOptions = rOptions;
    if (!aFilterName.Len())
        ScDocumentLoader::GetFilterName( rFile, aFilterName, aNewOptions );

    //  remove application prefix from filter name here, so the filter options
    //  aren't reset when the filter name is changed in ScAreaLink::DataChanged
    ScDocumentLoader::RemoveAppPrefix( aFilterName );

    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();

    ScAreaLink* pLink = new ScAreaLink( &rDocShell, rFile, aFilterName,
                                        aNewOptions, rSource, rDestRange );
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, rFile, &aFilterName, &rSource );

    //  Undo fuer den leeren Link

    if (bUndo)
        rDocShell.GetUndoManager()->AddUndoAction( new ScUndoInsertAreaLink( &rDocShell,
                                                    rFile, aFilterName, aNewOptions,
                                                    rSource, rDestRange ) );

    //  Update hat sein eigenes Undo

    pLink->SetDoInsert(bFitBlock);  // beim ersten Update ggf. nichts einfuegen
    pLink->Update();                // kein SetInCreate -> Update ausfuehren
    pLink->SetDoInsert(TRUE);       // Default = TRUE

    SfxBindings* pBindings = rDocShell.GetViewBindings();
    if (pBindings)
        pBindings->Invalidate( SID_LINKS );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator

    return TRUE;
}




