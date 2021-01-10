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

#include <svl/hint.hxx>
#include <comphelper/lok.hxx>
#include <svl/zforlist.hxx>
#include <svx/numfmtsh.hxx>
#include <svx/numinf.hxx>
#include <svx/svxids.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>

#include <tabvwsh.hxx>
#include <global.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <scmod.hxx>
#include <uiitems.hxx>
#include <hints.hxx>
#include <cellvalue.hxx>
#include <svl/sharedstring.hxx>

void ScTabViewShell::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (const ScPaintHint* pPaintHint = dynamic_cast<const ScPaintHint*>(&rHint))                    // draw new
    {
        PaintPartFlags nParts = pPaintHint->GetParts();
        SCTAB nTab = GetViewData().GetTabNo();
        if (pPaintHint->GetStartTab() <= nTab && pPaintHint->GetEndTab() >= nTab)
        {
            if (nParts & PaintPartFlags::Extras)          // first if table vanished !!!
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
    else if (auto pEditViewHint = dynamic_cast<const ScEditViewHint*>(&rHint))                 // create Edit-View
    {
        //  ScEditViewHint is only received at active view

        SCTAB nTab = GetViewData().GetTabNo();
        if ( pEditViewHint->GetTab() == nTab )
        {
            SCCOL nCol = pEditViewHint->GetCol();
            SCROW nRow = pEditViewHint->GetRow();
            {
                HideNoteMarker();

                MakeEditView( pEditViewHint->GetEngine(), nCol, nRow );

                StopEditShell();                    // shouldn't be set

                ScSplitPos eActive = GetViewData().GetActivePart();
                if ( GetViewData().HasEditView(eActive) )
                {
                    //  MakeEditView will fail, if the cursor is outside the screen.
                    //  Then GetEditView will return a none-active view, therefore
                    //  calling HasEditView.

                    EditView* pView = GetViewData().GetEditView(eActive);  // isn't zero

                    SetEditShell(pView, true);
                }
            }
        }
    }
    else if (dynamic_cast<const ScTablesHint*>(&rHint))               // table insert / deleted
    {
        // first fetch current table (can be changed during DeleteTab on ViewData)
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

        //  No calling of IsActive() here, because the actions can be coming from Basic
        //  and then also the active view has to be switched.

        SCTAB nNewTab = nActiveTab;
        bool bStayOnActiveTab = true;
        switch (nId)
        {
            case SC_TAB_INSERTED:
                if ( nTab1 <= nNewTab )             // insert before
                    ++nNewTab;
                break;
            case SC_TAB_DELETED:
                if ( nTab1 < nNewTab )              // deleted before
                    --nNewTab;
                else if ( nTab1 == nNewTab )        // deleted current
                    bStayOnActiveTab = false;
                break;
            case SC_TAB_MOVED:
                if ( nNewTab == nTab1 )             // moved table
                    nNewTab = nTab2;
                else if ( nTab1 < nTab2 )           // moved back
                {
                    if ( nNewTab > nTab1 && nNewTab <= nTab2 )      // succeeding area
                        --nNewTab;
                }
                else                                // move in front
                {
                    if ( nNewTab >= nTab2 && nNewTab < nTab1 )      // succeeding area
                        ++nNewTab;
                }
                break;
            case SC_TAB_COPIED:
                if ( nNewTab >= nTab2 )             // insert before
                    ++nNewTab;
                break;
            case SC_TAB_HIDDEN:
                if ( nTab1 == nNewTab )             // current is hidden
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

        ScDocument& rDoc = GetViewData().GetDocument();
        if ( nNewTab >= rDoc.GetTableCount() )
            nNewTab = rDoc.GetTableCount() - 1;

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
    else                       // without parameter
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
                        GetSelEngine()->Reset();
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
                    ScDocument& rDoc = GetViewData().GetDocument();
                    if (!rDoc.HasTable( GetViewData().GetTabNo() ))
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
                    //  "Save as" can make a write-protected document writable,
                    //  therefore the Layer-Locks anew (#39884#)
                    //  (Invalidate etc. is happening already from Sfx)
                    //  by SID_EDITDOC no SfxHintId::TitleChanged will occur, that
                    //  is why the own hint from DoSaveCompleted
                    //! what is with SfxHintId::SAVECOMPLETED ?

                    UpdateLayerLocks();

                    //  Would be too much to change Design-Mode with every save
                    //  (when saving under the name, it should remain unchanged)
                    //  Therefore only by SfxHintId::ModeChanged (from ViewFrame)
                }
                break;

            case SfxHintId::ModeChanged:
                //  Since you can no longer rely on it where this hint was coming
                //  from, always switch the design mode when the ReadOnly state
                //  really was changed:

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

            case SfxHintId::LanguageChanged:
            {
                GetViewFrame()->GetBindings().Invalidate(SID_LANGUAGE_STATUS);
                if ( ScGridWindow* pWin = GetViewData().GetActiveWin() )
                    pWin->ResetAutoSpell();
            }
                break;

            default:
                break;
        }
    }

    SfxViewShell::Notify( rBC, rHint );
}

std::unique_ptr<SvxNumberInfoItem> ScTabViewShell::MakeNumberInfoItem( ScDocument& rDoc, const ScViewData& rViewData )
{

    // construct NumberInfo item

    SvxNumberValueType  eValType        = SvxNumberValueType::Undefined;
    double              nCellValue      = 0;
    OUString aCellString;

    ScRefCellValue aCell(rDoc, rViewData.GetCurPos());

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
            return std::make_unique<SvxNumberInfoItem>(
                                rDoc.GetFormatTable(),
                                aCellString,
                                SID_ATTR_NUMBERFORMAT_INFO );

        case SvxNumberValueType::Number:
            return std::make_unique<SvxNumberInfoItem>(
                                rDoc.GetFormatTable(),
                                nCellValue,
                                SID_ATTR_NUMBERFORMAT_INFO );

        case SvxNumberValueType::Undefined:
        default:
            ;
    }

    return std::make_unique<SvxNumberInfoItem>(
        rDoc.GetFormatTable(), static_cast<sal_uInt16>(SID_ATTR_NUMBERFORMAT_INFO));
}

void ScTabViewShell::UpdateNumberFormatter(
                        const SvxNumberInfoItem& rInfoItem )
{
    for ( sal_uInt32 key : rInfoItem.GetDelFormats() )
        rInfoItem.GetNumberFormatter()->DeleteEntry( key );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
