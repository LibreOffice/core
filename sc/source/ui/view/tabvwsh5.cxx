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
#include "formulacell.hxx"
#include "globstr.hrc"
#include "scmod.hxx"
#include "uiitems.hxx"
#include "editsh.hxx"
#include "hints.hxx"
#include "cellvalue.hxx"
#include <svl/sharedstring.hxx>

void ScTabViewShell::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (dynamic_cast<const SfxSimpleHint*>(&rHint))                       // ohne Parameter
    {
        sal_uLong nSlot = static_cast<const SfxSimpleHint&>(rHint).GetId();
        switch ( nSlot )
        {
            case FID_DATACHANGED:
                UpdateFormulas();
                break;

            case FID_REFMODECHANGED:
                {
                    bool bRefMode = SC_MOD()->IsFormulaMode();
                    if (!bRefMode)
                        StopRefMode();
                    else
                    {
                        GetSelEngine()->Reset();
                        GetFunctionSet().SetAnchorFlag(true);
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
                    ScDocument* pDoc = GetViewData().GetDocument();
                    if (!pDoc->HasTable( GetViewData().GetTabNo() ))
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
                    //  bei SID_EDITDOC kommt kein SFX_HINT_TITLECHANGED, darum
                    //  der eigene Hint aus DoSaveCompleted
                    //! was ist mit SFX_HINT_SAVECOMPLETED ?

                    UpdateLayerLocks();

                    //  Design-Modus bei jedem Speichern anzupassen, waere zuviel
                    //  (beim Speichern unter gleichem Namen soll er unveraendert bleiben)
                    //  Darum nur bei SFX_HINT_MODECHANGED (vom ViewFrame)
                }
                break;

            case SFX_HINT_MODECHANGED:
                //  Da man sich nicht mehr darauf verlassen kann, woher
                //  dieser Hint kommt, den Design-Modus immer dann umschalten, wenn der
                //  ReadOnly-Status sich wirklich geaendert hat:

                if ( GetViewData().GetSfxDocShell()->IsReadOnly() != bReadOnly )
                {
                    bReadOnly = GetViewData().GetSfxDocShell()->IsReadOnly();

                    SfxBoolItem aItem( SID_FM_DESIGN_MODE, !bReadOnly);
                    GetViewData().GetDispatcher().Execute( SID_FM_DESIGN_MODE, SfxCallMode::ASYNCHRON,
                                                &aItem, 0L );

                    UpdateInputContext();
                }
                break;

            case SC_HINT_SHOWRANGEFINDER:
                PaintRangeFinder();
                break;

            case SC_HINT_FORCESETTAB:
                SetTabNo( GetViewData().GetTabNo(), true );
                break;

            default:
                break;
        }
    }
    else if (dynamic_cast<const ScPaintHint*>(&rHint))                    // neu zeichnen
    {
        const ScPaintHint* pHint = static_cast<const ScPaintHint*>(&rHint);
        sal_uInt16 nParts = pHint->GetParts();
        SCTAB nTab = GetViewData().GetTabNo();
        if (pHint->GetStartTab() <= nTab && pHint->GetEndTab() >= nTab)
        {
            if (nParts & PAINT_EXTRAS)          // zuerst, falls Tabelle weg ist !!!
                if (PaintExtras())
                    nParts = PAINT_ALL;

            // if the current sheet has pending row height updates (sheet links refreshed),
            // execute them before invalidating the window
            GetViewData().GetDocShell()->UpdatePendingRowHeights( GetViewData().GetTabNo() );

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
    else if (dynamic_cast<const ScEditViewHint*>(&rHint))                 // Edit-View anlegen
    {
        //  ScEditViewHint kommt nur an aktiver View an

        const ScEditViewHint* pHint = static_cast<const ScEditViewHint*>(&rHint);
        SCTAB nTab = GetViewData().GetTabNo();
        if ( pHint->GetTab() == nTab )
        {
            SCCOL nCol = pHint->GetCol();
            SCROW nRow = pHint->GetRow();
            {
                HideNoteMarker();

                MakeEditView( pHint->GetEngine(), nCol, nRow );

                StopEditShell();                    // sollte nicht gesetzt sein

                ScSplitPos eActive = GetViewData().GetActivePart();
                if ( GetViewData().HasEditView(eActive) )
                {
                    //  MakeEditView geht schief, wenn der Cursor ausserhalb des
                    //  Bildschirms steht. GetEditView gibt dann eine nicht aktive
                    //  View zurueck, darum die Abfrage HasEditView.

                    EditView* pView = GetViewData().GetEditView(eActive);  // ist nicht 0

                    SetEditShell(pView, true);
                }
            }
        }
    }
    else if (dynamic_cast<const ScTablesHint*>(&rHint))               // Tabelle eingefuegt / geloescht
    {
            //  aktuelle Tabelle zuerst holen (kann bei DeleteTab an ViewData geaendert werden)
        SCTAB nActiveTab = GetViewData().GetTabNo();

        const ScTablesHint& rTabHint = static_cast<const ScTablesHint&>(rHint);
        SCTAB nTab1 = rTabHint.GetTab1();
        SCTAB nTab2 = rTabHint.GetTab2();
        sal_uInt16 nId  = rTabHint.GetId();
        switch (nId)
        {
            case SC_TAB_INSERTED:
                GetViewData().InsertTab( nTab1 );
                break;
            case SC_TAB_DELETED:
                GetViewData().DeleteTab( nTab1 );
                break;
            case SC_TAB_MOVED:
                GetViewData().MoveTab( nTab1, nTab2 );
                break;
            case SC_TAB_COPIED:
                GetViewData().CopyTab( nTab1, nTab2 );
                break;
            case SC_TAB_HIDDEN:
                break;
            case SC_TABS_INSERTED:
                GetViewData().InsertTabs( nTab1, nTab2 );
                break;
            case SC_TABS_DELETED:
                GetViewData().DeleteTabs( nTab1, nTab2 );
                break;
            default:
                OSL_FAIL("unbekannter ScTablesHint");
        }

        //  hier keine Abfrage auf IsActive() mehr, weil die Aktion von Basic ausgehen
        //  kann und dann auch die aktive View umgeschaltet werden muss.

        SCTAB nNewTab = nActiveTab;
        bool bStayOnActiveTab = true;
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
                    bStayOnActiveTab = false;
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
                    bStayOnActiveTab = false;
                break;
            case SC_TABS_INSERTED:
                if ( nTab1 <= nNewTab )
                    nNewTab += nTab2;
                break;
            case SC_TABS_DELETED:
                if ( nTab1 < nNewTab )
                    nNewTab -= nTab2;
                break;
        }

        ScDocument* pDoc = GetViewData().GetDocument();
        if ( nNewTab >= pDoc->GetTableCount() )
            nNewTab = pDoc->GetTableCount() - 1;

        bool bForce = !bStayOnActiveTab;
        SetTabNo( nNewTab, bForce, false, bStayOnActiveTab );
    }
    else if (dynamic_cast<const ScIndexHint*>(&rHint))
    {
        const ScIndexHint& rIndexHint = static_cast<const ScIndexHint&>(rHint);
        sal_uInt16 nId = rIndexHint.GetId();
        sal_uInt16 nIndex = rIndexHint.GetIndex();
        switch (nId)
        {
            case SC_HINT_SHOWRANGEFINDER:
                PaintRangeFinder( nIndex );
                break;
        }
    }

    SfxViewShell::Notify( rBC, rHint );
}

SvxNumberInfoItem* ScTabViewShell::MakeNumberInfoItem( ScDocument* pDoc, ScViewData* pViewData )
{

    // NumberInfo-Item konstruieren:

    SvxNumberValueType  eValType        = SVX_VALUE_TYPE_UNDEFINED;
    double              nCellValue      = 0;
    OUString aCellString;

    ScRefCellValue aCell(*pDoc, pViewData->GetCurPos());

    switch (aCell.meType)
    {
        case CELLTYPE_VALUE:
        {
            nCellValue = aCell.mfValue;
            eValType = SVX_VALUE_TYPE_NUMBER;
        }
        break;

        case CELLTYPE_STRING:
        {
            aCellString = aCell.mpString->getString();
            eValType = SVX_VALUE_TYPE_STRING;
        }
        break;

        case CELLTYPE_FORMULA:
        {
            if (aCell.mpFormula->IsValue())
            {
                nCellValue = aCell.mpFormula->GetValue();
                eValType = SVX_VALUE_TYPE_NUMBER;
            }
            else
            {
                nCellValue = 0;
                eValType   = SVX_VALUE_TYPE_UNDEFINED;
            }
        }
        break;

        default:
            nCellValue = 0;
            eValType   = SVX_VALUE_TYPE_UNDEFINED;
    }

    switch ( eValType )
    {
        case SVX_VALUE_TYPE_STRING:
            return new SvxNumberInfoItem(
                                pDoc->GetFormatTable(),
                                aCellString,
                                SID_ATTR_NUMBERFORMAT_INFO );

        case SVX_VALUE_TYPE_NUMBER:
            return new SvxNumberInfoItem(
                                pDoc->GetFormatTable(),
                                nCellValue,
                                SID_ATTR_NUMBERFORMAT_INFO );

        case SVX_VALUE_TYPE_UNDEFINED:
        default:
            ;
    }

    return new SvxNumberInfoItem(
        pDoc->GetFormatTable(), static_cast<const sal_uInt16>(SID_ATTR_NUMBERFORMAT_INFO));
}

void ScTabViewShell::UpdateNumberFormatter(
                        const SvxNumberInfoItem& rInfoItem )
{
    const sal_uInt32 nDelCount = rInfoItem.GetDelCount();

    if ( nDelCount > 0 )
    {
        const sal_uInt32* pDelArr = rInfoItem.GetDelArray();

        for ( sal_uInt32 i=0; i<nDelCount; i++ )
            rInfoItem.GetNumberFormatter()->DeleteEntry( pDelArr[i] );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
