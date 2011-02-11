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
#define _ZFORLIST_DECLARE_TABLE
#include "scitems.hxx"
#include <svl/smplhint.hxx>
#include <svl/zforlist.hxx>
#include <svx/numfmtsh.hxx>
#include <svx/numinf.hxx>
#include <svx/svxids.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>

#include "tabvwsh.hxx"
#include "sc.hrc"
#include "global.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "globstr.hrc"
#include "scmod.hxx"
#include "uiitems.hxx"
#include "editsh.hxx"
#include "hints.hxx"


//==================================================================

void ScTabViewShell::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA(SfxSimpleHint))                       // ohne Parameter
    {
        ULONG nSlot = ((SfxSimpleHint&)rHint).GetId();
        switch ( nSlot )
        {
            case FID_DATACHANGED:
                UpdateFormulas();
                break;

            case FID_REFMODECHANGED:
                {
                    BOOL bRefMode = SC_MOD()->IsFormulaMode();
                    if (!bRefMode)
                        StopRefMode();
                    else
                    {
                        GetSelEngine()->Reset();
                        GetFunctionSet()->SetAnchorFlag(TRUE);
                        //  AnchorFlag, damit gleich mit Control angehaengt werden kann
                    }
                }
                break;

            case FID_KILLEDITVIEW:
            case FID_KILLEDITVIEW_NOPAINT:
                StopEditShell();
                KillEditView( nSlot == FID_KILLEDITVIEW_NOPAINT );
                break;

            case SFX_HINT_DOCCHANGED:
                {
                    ScDocument* pDoc = GetViewData()->GetDocument();
                    if (!pDoc->HasTable( GetViewData()->GetTabNo() ))
                    {
                        SetTabNo(0);
                    }
                }
                break;

            case SC_HINT_DRWLAYER_NEW:
                MakeDrawView();
                break;

            case SC_HINT_DOC_SAVED:
                {
                    //  beim "Save as" kann ein vorher schreibgeschuetztes Dokument
                    //  bearbeitbar werden, deshalb die Layer-Locks neu (#39884#)
                    //  (Invalidate etc. passiert schon vom Sfx her)
                    //  #42091# bei SID_EDITDOC kommt kein SFX_HINT_TITLECHANGED, darum
                    //  der eigene Hint aus DoSaveCompleted
                    //! was ist mit SFX_HINT_SAVECOMPLETED ?

                    UpdateLayerLocks();

                    //  #54891# Design-Modus bei jedem Speichern anzupassen, waere zuviel
                    //  (beim Speichern unter gleichem Namen soll er unveraendert bleiben)
                    //  Darum nur bei SFX_HINT_MODECHANGED (vom ViewFrame)
                }
                break;

            case SFX_HINT_MODECHANGED:
                //  #54891#/#58510# Da man sich nicht mehr darauf verlassen kann, woher
                //  dieser Hint kommt, den Design-Modus immer dann umschalten, wenn der
                //  ReadOnly-Status sich wirklich geaendert hat:

                if ( GetViewData()->GetSfxDocShell()->IsReadOnly() != bReadOnly )
                {
                    bReadOnly = GetViewData()->GetSfxDocShell()->IsReadOnly();

                    SfxBoolItem aItem( SID_FM_DESIGN_MODE, !bReadOnly);
                    GetViewData()->GetDispatcher().Execute( SID_FM_DESIGN_MODE, SFX_CALLMODE_ASYNCHRON,
                                                &aItem, 0L );

                    UpdateInputContext();
                }
                break;

            case SC_HINT_SHOWRANGEFINDER:
                PaintRangeFinder();
                break;

            case SC_HINT_FORCESETTAB:
                SetTabNo( GetViewData()->GetTabNo(), TRUE );
                break;

            default:
                break;
        }
    }
    else if (rHint.ISA(ScPaintHint))                    // neu zeichnen
    {
        ScPaintHint* pHint = (ScPaintHint*) &rHint;
        USHORT nParts = pHint->GetParts();
        SCTAB nTab = GetViewData()->GetTabNo();
        if (pHint->GetStartTab() <= nTab && pHint->GetEndTab() >= nTab)
        {
            if (nParts & PAINT_EXTRAS)          // zuerst, falls Tabelle weg ist !!!
                if (PaintExtras())
                    nParts = PAINT_ALL;

            // if the current sheet has pending row height updates (sheet links refreshed),
            // execute them before invalidating the window
            GetViewData()->GetDocShell()->UpdatePendingRowHeights( GetViewData()->GetTabNo() );

            if (nParts & PAINT_SIZE)
                RepeatResize();                     //! InvalidateBorder ???
            if (nParts & PAINT_GRID)
                PaintArea( pHint->GetStartCol(), pHint->GetStartRow(),
                           pHint->GetEndCol(), pHint->GetEndRow() );
            if (nParts & PAINT_MARKS)
                PaintArea( pHint->GetStartCol(), pHint->GetStartRow(),
                           pHint->GetEndCol(), pHint->GetEndRow(), SC_UPDATE_MARKS );
            if (nParts & PAINT_LEFT)
                PaintLeftArea( pHint->GetStartRow(), pHint->GetEndRow() );
            if (nParts & PAINT_TOP)
                PaintTopArea( pHint->GetStartCol(), pHint->GetEndCol() );

            // #i84689# call UpdateAllOverlays here instead of in ScTabView::PaintArea
            if (nParts & ( PAINT_LEFT | PAINT_TOP ))    // only if widths or heights changed
                UpdateAllOverlays();

            HideNoteMarker();
        }
    }
    else if (rHint.ISA(ScEditViewHint))                 // Edit-View anlegen
    {
        //  ScEditViewHint kommt nur an aktiver View an

        ScEditViewHint* pHint = (ScEditViewHint*) &rHint;
        SCTAB nTab = GetViewData()->GetTabNo();
        if ( pHint->GetTab() == nTab )
        {
            SCCOL nCol = pHint->GetCol();
            SCROW nRow = pHint->GetRow();
            {
                HideNoteMarker();

                MakeEditView( pHint->GetEngine(), nCol, nRow );

                StopEditShell();                    // sollte nicht gesetzt sein

                ScSplitPos eActive = GetViewData()->GetActivePart();
                if ( GetViewData()->HasEditView(eActive) )
                {
                    //  MakeEditView geht schief, wenn der Cursor ausserhalb des
                    //  Bildschirms steht. GetEditView gibt dann eine nicht aktive
                    //  View zurueck, darum die Abfrage HasEditView.

                    EditView* pView = GetViewData()->GetEditView(eActive);  // ist nicht 0

                    SetEditShell(pView ,TRUE);
                }
            }
        }
    }
    else if (rHint.ISA(ScTablesHint))               // Tabelle eingefuegt / geloescht
    {
            //  aktuelle Tabelle zuerst holen (kann bei DeleteTab an ViewData geaendert werden)
        SCTAB nActiveTab = GetViewData()->GetTabNo();

        const ScTablesHint& rTabHint = (const ScTablesHint&)rHint;
        SCTAB nTab1 = rTabHint.GetTab1();
        SCTAB nTab2 = rTabHint.GetTab2();
        USHORT nId  = rTabHint.GetId();
        switch (nId)
        {
            case SC_TAB_INSERTED:
                GetViewData()->InsertTab( nTab1 );
                break;
            case SC_TAB_DELETED:
                GetViewData()->DeleteTab( nTab1 );
                break;
            case SC_TAB_MOVED:
                GetViewData()->MoveTab( nTab1, nTab2 );
                break;
            case SC_TAB_COPIED:
                GetViewData()->CopyTab( nTab1, nTab2 );
                break;
            case SC_TAB_HIDDEN:
                break;
            default:
                DBG_ERROR("unbekannter ScTablesHint");
        }

        //  hier keine Abfrage auf IsActive() mehr, weil die Aktion von Basic ausgehen
        //  kann und dann auch die aktive View umgeschaltet werden muss.

        SCTAB nNewTab = nActiveTab;
        BOOL bForce = FALSE;
        switch (nId)
        {
            case SC_TAB_INSERTED:
                if ( nTab1 <= nNewTab )             // vorher eingefuegt
                    ++nNewTab;
                break;
            case SC_TAB_DELETED:
                if ( nTab1 < nNewTab )              // vorher geloescht
                    --nNewTab;
                else if ( nTab1 == nNewTab )        // aktuelle geloescht
                    bForce = TRUE;
                break;
            case SC_TAB_MOVED:
                if ( nNewTab == nTab1 )             // verschobene Tabelle
                    nNewTab = nTab2;
                else if ( nTab1 < nTab2 )           // nach hinten verschoben
                {
                    if ( nNewTab > nTab1 && nNewTab <= nTab2 )      // nachrueckender Bereich
                        --nNewTab;
                }
                else                                // nach vorne verschoben
                {
                    if ( nNewTab >= nTab2 && nNewTab < nTab1 )      // nachrueckender Bereich
                        ++nNewTab;
                }
                break;
            case SC_TAB_COPIED:
                if ( nNewTab >= nTab2 )             // vorher eingefuegt
                    ++nNewTab;
                break;
            case SC_TAB_HIDDEN:
                if ( nTab1 == nNewTab )             // aktuelle ausgeblendet
                    bForce = TRUE;
                break;
        }

        ScDocument* pDoc = GetViewData()->GetDocument();
        if ( nNewTab >= pDoc->GetTableCount() )
            nNewTab = pDoc->GetTableCount() - 1;

        SetTabNo( nNewTab, bForce );
    }
    else if (rHint.ISA(ScIndexHint))
    {
        const ScIndexHint& rIndexHint = (const ScIndexHint&)rHint;
        USHORT nId = rIndexHint.GetId();
        USHORT nIndex = rIndexHint.GetIndex();
        switch (nId)
        {
            case SC_HINT_SHOWRANGEFINDER:
                PaintRangeFinder( nIndex );
                break;
        }
    }

    SfxViewShell::Notify( rBC, rHint );
}

//------------------------------------------------------------------

void ScTabViewShell::MakeNumberInfoItem( ScDocument*         pDoc,
                                         ScViewData*         pViewData,
                                         SvxNumberInfoItem** ppItem )
{
    //------------------------------
    // NumberInfo-Item konstruieren:
    //------------------------------
    ScBaseCell*         pCell = NULL;
    SvxNumberValueType  eValType        = SVX_VALUE_TYPE_UNDEFINED;
    double              nCellValue      = 0;
    String              aCellString;

    pDoc->GetCell( pViewData->GetCurX(),
                   pViewData->GetCurY(),
                   pViewData->GetTabNo(),
                   pCell );

    if ( pCell )
    {
        switch ( pCell->GetCellType() )
        {
            case CELLTYPE_VALUE:
                {
                    nCellValue = ((ScValueCell*)pCell)->GetValue();
                    eValType = SVX_VALUE_TYPE_NUMBER;
                    aCellString.Erase();
                }
                break;

            case CELLTYPE_STRING:
                {
                    ((ScStringCell*)pCell)->GetString( aCellString );
                    eValType = SVX_VALUE_TYPE_STRING;
                }
                break;

            case CELLTYPE_FORMULA:
                {
                    if ( ((ScFormulaCell*)pCell)->IsValue() )
                    {
                        nCellValue = ((ScFormulaCell*)pCell)->GetValue();
                        eValType = SVX_VALUE_TYPE_NUMBER;
                    }
                    else
                    {
                        nCellValue = 0;
                        eValType   = SVX_VALUE_TYPE_UNDEFINED;
                    }
                    aCellString.Erase();
                }
                break;

            default:
                nCellValue = 0;
                eValType   = SVX_VALUE_TYPE_UNDEFINED;
                aCellString.Erase();
        }
    }
    else // Zelle noch leer (== nicht erzeugt)
    {
        nCellValue = 0;
        eValType   = SVX_VALUE_TYPE_UNDEFINED;
        aCellString.Erase();
    }

    switch ( eValType )
    {
        case SVX_VALUE_TYPE_STRING:
            *ppItem = new SvxNumberInfoItem(
                                pDoc->GetFormatTable(),
                                aCellString,
                                SID_ATTR_NUMBERFORMAT_INFO );
            break;

        case SVX_VALUE_TYPE_NUMBER:
            *ppItem = new SvxNumberInfoItem(
                                pDoc->GetFormatTable(),
                                nCellValue,
                                SID_ATTR_NUMBERFORMAT_INFO );
            break;

        case SVX_VALUE_TYPE_UNDEFINED:
        default:
            *ppItem = new SvxNumberInfoItem(
                                pDoc->GetFormatTable(),
                                (const USHORT)
                                SID_ATTR_NUMBERFORMAT_INFO );
    }
}

//------------------------------------------------------------------

void ScTabViewShell::UpdateNumberFormatter(
                        ScDocument*              pDoc,
                        const SvxNumberInfoItem& rInfoItem )
{
    const sal_uInt32 nDelCount = rInfoItem.GetDelCount();

    if ( nDelCount > 0 )
    {
        const sal_uInt32* pDelArr = rInfoItem.GetDelArray();

        for ( USHORT i=0; i<nDelCount; i++ )
            rInfoItem.GetNumberFormatter()->DeleteEntry( pDelArr[i] );
    }

    // sollte besser UpdateNumberFormats() heissen ?
    pDoc->DeleteNumberFormat( rInfoItem.GetDelArray(),
                              rInfoItem.GetDelCount() );
}






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
