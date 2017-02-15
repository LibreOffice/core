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
#include <svl/hint.hxx>
#include <comphelper/lok.hxx>
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
    if (const ScPaintHint* pPaintHint = dynamic_cast<const ScPaintHint*>(&rHint))                    // neu zeichnen
    {
        PaintPartFlags nParts = pPaintHint->GetParts();
        SCTAB nTab = GetViewData().GetTabNo();
        if (pPaintHint->GetStartTab() <= nTab && pPaintHint->GetEndTab() >= nTab)
        {
            if (nParts & PaintPartFlags::Extras)          // zuerst, falls Tabelle weg ist !!!
                if (PaintExtras())
                    nParts = PaintPartFlags::All;

            // if the current sheet has pending row height updates (sheet links refreshed),
            // execute them before invalidating the window
            GetViewData().GetDocShell()->UpdatePendingRowHeights( GetViewData().GetTabNo() );

            if (nParts & PaintPartFlags::Size)
                RepeatResize();                     //! InvalidateBorder ???
            if (nParts & PaintPartFlags::Grid)
                PaintArea( pPaintHint->GetStartCol(), pPaintHint->GetStartRow(),
                           pPaintHint->GetEndCol(), pPaintHint->GetEndRow() );
            if (nParts & PaintPartFlags::Marks)
                PaintArea( pPaintHint->GetStartCol(), pPaintHint->GetStartRow(),
                           pPaintHint->GetEndCol(), pPaintHint->GetEndRow(), ScUpdateMode::Marks );
            if (nParts & PaintPartFlags::Left)
                PaintLeftArea( pPaintHint->GetStartRow(), pPaintHint->GetEndRow() );
            if (nParts & PaintPartFlags::Top)
                PaintTopArea( pPaintHint->GetStartCol(), pPaintHint->GetEndCol() );

            // #i84689# call UpdateAllOverlays here instead of in ScTabView::PaintArea
            if (nParts & ( PaintPartFlags::Left | PaintPartFlags::Top ))    // only if widths or heights changed
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
        sal_uInt16 nId  = rTabHint.GetTablesHintId();
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
                OSL_FAIL("unknown ScTablesHint");
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
    else if (const ScIndexHint* pIndexHint = dynamic_cast<const ScIndexHint*>(&rHint))
    {
        SfxHintId nId = pIndexHint->GetId();
        sal_uInt16 nIndex = pIndexHint->GetIndex();
        switch (nId)
        {
            case SfxHintId::ScShowRangeFinder:
                PaintRangeFinder( nIndex );
                break;
            default: break;
        }
    }
    else                       // ohne Parameter
    {
        const SfxHintId nSlot = rHint.GetId();
        switch ( nSlot )
        {
            case SfxHintId::ScDataChanged:
                UpdateFormulas();
                break;

            case SfxHintId::ScRefModeChanged:
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

            case SfxHintId::ScKillEditView:
            case SfxHintId::ScKillEditViewNoPaint:
                if (!comphelper::LibreOfficeKit::isActive()
                    || this == SfxViewShell::Current()
                    || bInPrepareClose
                    || bInDispose)
                {
                    StopEditShell();
                    KillEditView( nSlot == SfxHintId::ScKillEditViewNoPaint );
                }
                break;

            case SfxHintId::DocChanged:
                {
                    ScDocument* pDoc = GetViewData().GetDocument();
                    if (!pDoc->HasTable( GetViewData().GetTabNo() ))
                    {
                        SetTabNo(0);
                    }
                }
                break;

            case SfxHintId::ScDrawLayerNew:
                MakeDrawView(TRISTATE_INDET);
                break;

            case SfxHintId::ScDocSaved:
                {
                    //  beim "Save as" kann ein vorher schreibgeschuetztes Dokument
                    //  bearbeitbar werden, deshalb die Layer-Locks neu (#39884#)
                    //  (Invalidate etc. passiert schon vom Sfx her)
                    //  bei SID_EDITDOC kommt kein SfxHintId::TitleChanged, darum
                    //  der eigene Hint aus DoSaveCompleted
                    //! was ist mit SfxHintId::SAVECOMPLETED ?

                    UpdateLayerLocks();

                    //  Design-Modus bei jedem Speichern anzupassen, waere zuviel
                    //  (beim Speichern unter gleichem Namen soll er unveraendert bleiben)
                    //  Darum nur bei SfxHintId::ModeChanged (vom ViewFrame)
                }
                break;

            case SfxHintId::ModeChanged:
                //  Da man sich nicht mehr darauf verlassen kann, woher
                //  dieser Hint kommt, den Design-Modus immer dann umschalten, wenn der
                //  ReadOnly-Status sich wirklich geaendert hat:

                if ( GetViewData().GetSfxDocShell()->IsReadOnly() != bReadOnly )
                {
                    bReadOnly = GetViewData().GetSfxDocShell()->IsReadOnly();

                    SfxBoolItem aItem( SID_FM_DESIGN_MODE, !bReadOnly);
                    GetViewData().GetDispatcher().ExecuteList(SID_FM_DESIGN_MODE,
                            SfxCallMode::ASYNCHRON, { &aItem });

                    UpdateInputContext();
                }
                break;

            case SfxHintId::ScShowRangeFinder:
                PaintRangeFinder(-1);
                break;

            case SfxHintId::ScForceSetTab:
                SetTabNo( GetViewData().GetTabNo(), true );
                break;

            default:
                break;
        }
    }

    SfxViewShell::Notify( rBC, rHint );
}

SvxNumberInfoItem* ScTabViewShell::MakeNumberInfoItem( ScDocument* pDoc, ScViewData* pViewData )
{

    // NumberInfo-Item konstruieren:

    SvxNumberValueType  eValType        = SvxNumberValueType::Undefined;
    double              nCellValue      = 0;
    OUString aCellString;

    ScRefCellValue aCell(*pDoc, pViewData->GetCurPos());

    switch (aCell.meType)
    {
        case CELLTYPE_VALUE:
        {
            nCellValue = aCell.mfValue;
            eValType = SvxNumberValueType::Number;
        }
        break;

        case CELLTYPE_STRING:
        {
            aCellString = aCell.mpString->getString();
            eValType = SvxNumberValueType::String;
        }
        break;

        case CELLTYPE_FORMULA:
        {
            if (aCell.mpFormula->IsValue())
            {
                nCellValue = aCell.mpFormula->GetValue();
                eValType = SvxNumberValueType::Number;
            }
            else
            {
                nCellValue = 0;
                eValType   = SvxNumberValueType::Undefined;
            }
        }
        break;

        default:
            nCellValue = 0;
            eValType   = SvxNumberValueType::Undefined;
    }

    switch ( eValType )
    {
        case SvxNumberValueType::String:
            return new SvxNumberInfoItem(
                                pDoc->GetFormatTable(),
                                aCellString,
                                SID_ATTR_NUMBERFORMAT_INFO );

        case SvxNumberValueType::Number:
            return new SvxNumberInfoItem(
                                pDoc->GetFormatTable(),
                                nCellValue,
                                SID_ATTR_NUMBERFORMAT_INFO );

        case SvxNumberValueType::Undefined:
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
