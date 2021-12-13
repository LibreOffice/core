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

#include <rangelst.hxx>
#include <scitems.hxx>

#include <editeng/editview.hxx>
#include <svx/fmshell.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/cursor.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <IAnyRefDialog.hxx>
#include <tabview.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <gridwin.hxx>
#include <olinewin.hxx>
#include <overlayobject.hxx>
#include <colrowba.hxx>
#include <tabcont.hxx>
#include <scmod.hxx>
#include <sc.hrc>
#include <viewutil.hxx>
#include <editutil.hxx>
#include <inputhdl.hxx>
#include <inputwin.hxx>
#include <validat.hxx>
#include <inputopt.hxx>
#include <rfindlst.hxx>
#include <hiranges.hxx>
#include <viewuno.hxx>
#include <dpobject.hxx>
#include <seltrans.hxx>
#include <fillinfo.hxx>
#include <rangeutl.hxx>
#include <client.hxx>
#include <tabprotection.hxx>
#include <spellcheckcontext.hxx>
#include <markdata.hxx>
#include <formula/FormulaCompiler.hxx>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <output.hxx>

#include <utility>

#include <com/sun/star/chart2/data/HighlightedRange.hpp>

namespace
{

ScRange lcl_getSubRangeByIndex( const ScRange& rRange, sal_Int32 nIndex )
{
    ScAddress aResult( rRange.aStart );

    SCCOL nWidth = rRange.aEnd.Col() - rRange.aStart.Col() + 1;
    SCROW nHeight = rRange.aEnd.Row() - rRange.aStart.Row() + 1;
    SCTAB nDepth = rRange.aEnd.Tab() - rRange.aStart.Tab() + 1;
    if( (nWidth > 0) && (nHeight > 0) && (nDepth > 0) )
    {
        // row by row from first to last sheet
        sal_Int32 nArea = nWidth * nHeight;
        aResult.IncCol( static_cast< SCCOL >( nIndex % nWidth ) );
        aResult.IncRow( static_cast< SCROW >( (nIndex % nArea) / nWidth ) );
        aResult.IncTab( static_cast< SCTAB >( nIndex / nArea ) );
        if( !rRange.Contains( aResult ) )
            aResult = rRange.aStart;
    }

    return ScRange( aResult );
}

} // anonymous namespace

using namespace com::sun::star;

ScExtraEditViewManager::~ScExtraEditViewManager()
{
    DBG_ASSERT(nTotalWindows == 0, "ScExtraEditViewManager dtor: some out window has not yet been removed!");
}

inline void ScExtraEditViewManager::Add(SfxViewShell* pViewShell, ScSplitPos eWhich)
{
    Apply<Adder>(pViewShell, eWhich);
}

inline void ScExtraEditViewManager::Remove(SfxViewShell* pViewShell, ScSplitPos eWhich)
{
    Apply<Remover>(pViewShell, eWhich);
}


template<ScExtraEditViewManager::ModifierTagType ModifierTag>
void ScExtraEditViewManager::Apply(SfxViewShell* pViewShell, ScSplitPos eWhich)
{
    ScTabViewShell* pOtherViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
    if (pOtherViewShell == nullptr || pOtherViewShell == mpThisViewShell)
        return;

    mpOtherEditView = pOtherViewShell->GetViewData().GetEditView(eWhich);
    if (mpOtherEditView != nullptr)
    {
        DBG_ASSERT(mpOtherEditView->GetEditEngine(), "Edit view has no valid engine.");
        for (int i = 0; i < 4; ++i)
        {
            ScGridWindow* pWin = mpGridWin[i].get();
            if (pWin != nullptr)
            {
                Modifier<ModifierTag>(pWin);
            }
        }
    }
}

template<ScExtraEditViewManager::ModifierTagType ModifierTag>
void ScExtraEditViewManager::Modifier(ScGridWindow* /*pWin*/)
{
    (void)this;
    SAL_WARN("sc", "ScExtraEditViewManager::Modifier<ModifierTag>: non-specialized version should not be invoked.");
}

template<>
void ScExtraEditViewManager::Modifier<ScExtraEditViewManager::Adder>(ScGridWindow* pWin)
{
    if (mpOtherEditView->AddOtherViewWindow(pWin))
        ++nTotalWindows;
}

template<>
void ScExtraEditViewManager::Modifier<ScExtraEditViewManager::Remover>(ScGridWindow* pWin)
{
    if (mpOtherEditView->RemoveOtherViewWindow(pWin))
        --nTotalWindows;
}

// ---  public functions

void ScTabView::ClickCursor( SCCOL nPosX, SCROW nPosY, bool bControl )
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();
    rDoc.SkipOverlapped(nPosX, nPosY, nTab);

    bool bRefMode = SC_MOD()->IsFormulaMode();

    if ( bRefMode )
    {
        DoneRefMode();

        if (bControl)
            SC_MOD()->AddRefEntry();

        InitRefMode( nPosX, nPosY, nTab, SC_REFTYPE_REF );
    }
    else
    {
        DoneBlockMode( bControl );
        aViewData.ResetOldCursor();
        SetCursor( nPosX, nPosY );
    }
}

void ScTabView::UpdateAutoFillMark(bool bFromPaste)
{
    // single selection or cursor
    ScRange aMarkRange;
    ScMarkType eMarkType = aViewData.GetSimpleArea(aMarkRange);
    bool bMarked = eMarkType == SC_MARK_SIMPLE || eMarkType == SC_MARK_SIMPLE_FILTERED;

    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if (pGridWin[i] && pGridWin[i]->IsVisible())
            pGridWin[i]->UpdateAutoFillMark( bMarked, aMarkRange );
    }

    for (sal_uInt16 i = 0; i < 2; i++)
    {
        if (pColBar[i] && pColBar[i]->IsVisible())
            pColBar[i]->SetMark( bMarked, aMarkRange.aStart.Col(), aMarkRange.aEnd.Col() );
        if (pRowBar[i] && pRowBar[i]->IsVisible())
            pRowBar[i]->SetMark( bMarked, aMarkRange.aStart.Row(), aMarkRange.aEnd.Row() );
    }

    //  selection transfer object is checked together with AutoFill marks,
    //  because it has the same requirement of a single continuous block.
    if (!bFromPaste)
        CheckSelectionTransfer();   // update selection transfer object
}

void ScTabView::FakeButtonUp( ScSplitPos eWhich )
{
    if (pGridWin[eWhich])
        pGridWin[eWhich]->FakeButtonUp();
}

void ScTabView::HideAllCursors()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
    {
        if (pWin && pWin->IsVisible())
        {
            vcl::Cursor* pCur = pWin->GetCursor();
            if (pCur && pCur->IsVisible())
                pCur->Hide();
            pWin->HideCursor();
        }
    }
}

void ScTabView::ShowAllCursors()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
    {
        if (pWin && pWin->IsVisible())
        {
            pWin->ShowCursor();
            pWin->CursorChanged();
        }
    }
}

void ScTabView::ShowCursor()
{
    pGridWin[aViewData.GetActivePart()]->ShowCursor();
    pGridWin[aViewData.GetActivePart()]->CursorChanged();
}

void ScTabView::InvalidateAttribs()
{
    SfxBindings& rBindings = aViewData.GetBindings();

    rBindings.Invalidate( SID_STYLE_APPLY );
    rBindings.Invalidate( SID_STYLE_FAMILY2 );
    // StarCalc knows only paragraph- or cell format templates

    rBindings.Invalidate( SID_ATTR_CHAR_FONT );
    rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_COLOR );

    rBindings.Invalidate( SID_ATTR_CHAR_WEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_POSTURE );
    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
    rBindings.Invalidate( SID_ULINE_VAL_NONE );
    rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOTTED );

    rBindings.Invalidate( SID_ATTR_CHAR_OVERLINE );

    rBindings.Invalidate( SID_ATTR_CHAR_KERNING );
    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
    rBindings.Invalidate( SID_ATTR_CHAR_STRIKEOUT );
    rBindings.Invalidate( SID_ATTR_CHAR_SHADOWED );

    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_LEFT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_BLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_CENTER);
    rBindings.Invalidate( SID_NUMBER_TYPE_FORMAT);

    rBindings.Invalidate( SID_ALIGNLEFT );
    rBindings.Invalidate( SID_ALIGNRIGHT );
    rBindings.Invalidate( SID_ALIGNBLOCK );
    rBindings.Invalidate( SID_ALIGNCENTERHOR );

    rBindings.Invalidate( SID_ALIGNTOP );
    rBindings.Invalidate( SID_ALIGNBOTTOM );
    rBindings.Invalidate( SID_ALIGNCENTERVER );

    rBindings.Invalidate( SID_SCATTR_CELLPROTECTION );

    // stuff for sidebar panels
    {
        rBindings.Invalidate( SID_H_ALIGNCELL );
        rBindings.Invalidate( SID_V_ALIGNCELL );
        rBindings.Invalidate( SID_ATTR_ALIGN_INDENT );
        rBindings.Invalidate( SID_FRAME_LINECOLOR );
        rBindings.Invalidate( SID_FRAME_LINESTYLE );
        rBindings.Invalidate( SID_ATTR_BORDER_OUTER );
        rBindings.Invalidate( SID_ATTR_BORDER_INNER );
        rBindings.Invalidate( SID_ATTR_BORDER_DIAG_TLBR );
        rBindings.Invalidate( SID_ATTR_BORDER_DIAG_BLTR );
        rBindings.Invalidate( SID_NUMBER_TYPE_FORMAT );
    }

    rBindings.Invalidate( SID_BACKGROUND_COLOR );

    rBindings.Invalidate( SID_ATTR_ALIGN_LINEBREAK );
    rBindings.Invalidate( SID_NUMBER_FORMAT );

    rBindings.Invalidate( SID_TEXTDIRECTION_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_TEXTDIRECTION_TOP_TO_BOTTOM );
    rBindings.Invalidate( SID_ATTR_PARA_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_RIGHT_TO_LEFT );

    // pseudo slots for Format menu
    rBindings.Invalidate( SID_ALIGN_ANY_HDEFAULT );
    rBindings.Invalidate( SID_ALIGN_ANY_LEFT );
    rBindings.Invalidate( SID_ALIGN_ANY_HCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_RIGHT );
    rBindings.Invalidate( SID_ALIGN_ANY_JUSTIFIED );
    rBindings.Invalidate( SID_ALIGN_ANY_VDEFAULT );
    rBindings.Invalidate( SID_ALIGN_ANY_TOP );
    rBindings.Invalidate( SID_ALIGN_ANY_VCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_BOTTOM );

    rBindings.Invalidate( SID_NUMBER_CURRENCY );
    rBindings.Invalidate( SID_NUMBER_SCIENTIFIC );
    rBindings.Invalidate( SID_NUMBER_DATE );
    rBindings.Invalidate( SID_NUMBER_CURRENCY );
    rBindings.Invalidate( SID_NUMBER_PERCENT );
    rBindings.Invalidate( SID_NUMBER_TWODEC );
    rBindings.Invalidate( SID_NUMBER_TIME );
    rBindings.Invalidate( SID_NUMBER_STANDARD );
    rBindings.Invalidate( SID_NUMBER_THOUSANDS );
}

namespace {

void collectUIInformation(std::map<OUString, OUString>&& aParameters)
{
    EventDescription aDescription;
    aDescription.aID = "grid_window";
    aDescription.aAction = "SELECT";
    aDescription.aParameters = std::move(aParameters);
    aDescription.aParent = "MainWindow";
    aDescription.aKeyWord = "ScGridWinUIObject";

    UITestLogger::getInstance().logEvent(aDescription);
}

}

// SetCursor - Cursor, set, draw, update InputWin
// or send reference
// Optimising breaks the functionality

void ScTabView::SetCursor( SCCOL nPosX, SCROW nPosY, bool bNew )
{
    SCCOL nOldX = aViewData.GetCurX();
    SCROW nOldY = aViewData.GetCurY();

    //  DeactivateIP only for MarkListHasChanged

    // FIXME: this is to limit the number of rows handled in the Online
    // to 1000; this will be removed again when the performance
    // bottlenecks are sorted out
    if (comphelper::LibreOfficeKit::isActive())
        nPosY = std::min(nPosY, MAXTILEDROW);

    if ( !(nPosX != nOldX || nPosY != nOldY || bNew) )
        return;

    ScTabViewShell* pViewShell = aViewData.GetViewShell();
    bool bRefMode = pViewShell && pViewShell->IsRefInputMode();
    if ( aViewData.HasEditView( aViewData.GetActivePart() ) && !bRefMode ) // 23259 or so
    {
        UpdateInputLine();
    }

    HideAllCursors();

    aViewData.SetCurX( nPosX );
    aViewData.SetCurY( nPosY );

    ShowAllCursors();

    CursorPosChanged();

    OUString aCurrAddress = ScAddress(nPosX,nPosY,0).GetColRowString();
    collectUIInformation({{"CELL", aCurrAddress}});

    if (!comphelper::LibreOfficeKit::isActive())
        return;

    if (nPosX <= aViewData.GetMaxTiledCol() - 10 && nPosY <= aViewData.GetMaxTiledRow() - 25)
        return;

    ScDocument& rDoc = aViewData.GetDocument();
    ScDocShell* pDocSh = aViewData.GetDocShell();
    ScModelObj* pModelObj = pDocSh ? comphelper::getFromUnoTunnel<ScModelObj>( pDocSh->GetModel() ) : nullptr;
    Size aOldSize(0, 0);
    if (pModelObj)
        aOldSize = pModelObj->getDocumentSize();

    if (nPosX > aViewData.GetMaxTiledCol() - 10)
        aViewData.SetMaxTiledCol(std::min<SCCOL>(std::max(nPosX, aViewData.GetMaxTiledCol()) + 10, rDoc.MaxCol()));

    if (nPosY > aViewData.GetMaxTiledRow() - 25)
        aViewData.SetMaxTiledRow(std::min<SCROW>(std::max(nPosY, aViewData.GetMaxTiledRow()) + 25,  MAXTILEDROW));

    Size aNewSize(0, 0);
    if (pModelObj)
        aNewSize = pModelObj->getDocumentSize();

    if (!pDocSh)
        return;

    // New area extended to the right of the sheet after last column
    // including overlapping area with aNewRowArea
    tools::Rectangle aNewColArea(aOldSize.getWidth(), 0, aNewSize.getWidth(), aNewSize.getHeight());
    // New area extended to the bottom of the sheet after last row
    // excluding overlapping area with aNewColArea
    tools::Rectangle aNewRowArea(0, aOldSize.getHeight(), aOldSize.getWidth(), aNewSize.getHeight());

    // Only invalidate if spreadsheet extended to the right
    if (aNewColArea.getWidth())
    {
        SfxLokHelper::notifyInvalidation(aViewData.GetViewShell(), &aNewColArea);
    }

    // Only invalidate if spreadsheet extended to the bottom
    if (aNewRowArea.getHeight())
    {
        SfxLokHelper::notifyInvalidation(aViewData.GetViewShell(), &aNewRowArea);
    }

    // Provide size in the payload, so clients don't have to
    // call lok::Document::getDocumentSize().
    std::stringstream ss;
    ss << aNewSize.Width() << ", " << aNewSize.Height();
    OString sSize = ss.str().c_str();
    ScModelObj* pModel = comphelper::getFromUnoTunnel<ScModelObj>(aViewData.GetViewShell()->GetCurrentDocument());
    SfxLokHelper::notifyDocumentSizeChanged(aViewData.GetViewShell(), sSize, pModel, false);
}

static bool lcl_IsRefDlgActive(SfxViewFrame* pViewFrm)
{
    ScModule* pScMod = SC_MOD();
    if (!pScMod->IsRefDialogOpen())
       return false;

    auto nDlgId = pScMod->GetCurRefDlgId();
    if (!pViewFrm->HasChildWindow(nDlgId))
        return false;

    SfxChildWindow* pChild = pViewFrm->GetChildWindow(nDlgId);
    if (!pChild)
        return false;

    auto xDlgController = pChild->GetController();
    if (!xDlgController || !xDlgController->getDialog()->get_visible())
        return false;

    IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(xDlgController.get());
    return pRefDlg && pRefDlg->IsRefInputMode();
}

void ScTabView::CheckSelectionTransfer()
{
    if ( !aViewData.IsActive() )     // only for active view
        return;

    ScModule* pScMod = SC_MOD();
    ScSelectionTransferObj* pOld = pScMod->GetSelectionTransfer();
    rtl::Reference<ScSelectionTransferObj> pNew = ScSelectionTransferObj::CreateFromView( this );
    if ( !pNew )
        return;

    //  create new selection

    if (pOld)
        pOld->ForgetView();

    pScMod->SetSelectionTransfer( pNew.get() );

    // tdf#124975/tdf#136242 changing the calc selection can trigger removal of the
    // selection of an open RefDlg dialog, so don't inform the
    // desktop clipboard of the changed selection if that dialog is open
    if (!lcl_IsRefDlgActive(aViewData.GetViewShell()->GetViewFrame()))
        pNew->CopyToPrimarySelection();                    // may delete pOld

    // Log the selection change
    ScMarkData& rMark = aViewData.GetMarkData();
    if (rMark.IsMarked())
    {
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );
        OUString aStartAddress =  aMarkRange.aStart.GetColRowString();
        OUString aEndAddress = aMarkRange.aEnd.GetColRowString();
        collectUIInformation({{"RANGE", aStartAddress + ":" + aEndAddress}});
    }
}

// update input row / menus
// CursorPosChanged calls SelectionChanged
// SelectionChanged calls CellContentChanged

void ScTabView::CellContentChanged()
{
    SfxBindings& rBindings = aViewData.GetBindings();

    rBindings.Invalidate( SID_ATTR_SIZE );      // -> show error message
    rBindings.Invalidate( SID_THESAURUS );
    rBindings.Invalidate( SID_HYPERLINK_GETLINK );
    rBindings.Invalidate( SID_ROWCOL_SELCOUNT );

    InvalidateAttribs();                    // attributes updates

    aViewData.GetViewShell()->UpdateInputHandler();
}

void ScTabView::SetTabProtectionSymbol( SCTAB nTab, const bool bProtect )
{
    pTabControl->SetProtectionSymbol( static_cast<sal_uInt16>(nTab)+1, bProtect);
}

void ScTabView::SelectionChanged(bool bFromPaste)
{
    SfxViewFrame* pViewFrame = aViewData.GetViewShell()->GetViewFrame();
    if (pViewFrame)
    {
        uno::Reference<frame::XController> xController = pViewFrame->GetFrame().GetController();
        if (xController.is())
        {
            ScTabViewObj* pImp = comphelper::getFromUnoTunnel<ScTabViewObj>( xController );
            if (pImp)
                pImp->SelectionChanged();
        }
    }

    UpdateAutoFillMark(bFromPaste);   // also calls CheckSelectionTransfer

    SfxBindings& rBindings = aViewData.GetBindings();

    rBindings.Invalidate( SID_CURRENTCELL );    // -> Navigator
    rBindings.Invalidate( SID_AUTO_FILTER );    // -> Menu
    rBindings.Invalidate( FID_NOTE_VISIBLE );
    rBindings.Invalidate( FID_SHOW_NOTE );
    rBindings.Invalidate( FID_HIDE_NOTE );
    rBindings.Invalidate( FID_SHOW_ALL_NOTES );
    rBindings.Invalidate( FID_HIDE_ALL_NOTES );
    rBindings.Invalidate( SID_TOGGLE_NOTES );
    rBindings.Invalidate( SID_DELETE_NOTE );
    rBindings.Invalidate( SID_ROWCOL_SELCOUNT );

        //  functions than may need to be disabled

    rBindings.Invalidate( FID_INS_ROWBRK );
    rBindings.Invalidate( FID_INS_COLBRK );
    rBindings.Invalidate( FID_DEL_ROWBRK );
    rBindings.Invalidate( FID_DEL_COLBRK );
    rBindings.Invalidate( FID_MERGE_ON );
    rBindings.Invalidate( FID_MERGE_OFF );
    rBindings.Invalidate( FID_MERGE_TOGGLE );
    rBindings.Invalidate( SID_AUTOFILTER_HIDE );
    rBindings.Invalidate( SID_UNFILTER );
    rBindings.Invalidate( SID_REIMPORT_DATA );
    rBindings.Invalidate( SID_REFRESH_DBAREA );
    rBindings.Invalidate( SID_OUTLINE_SHOW );
    rBindings.Invalidate( SID_OUTLINE_HIDE );
    rBindings.Invalidate( SID_OUTLINE_REMOVE );
    rBindings.Invalidate( FID_FILL_TO_BOTTOM );
    rBindings.Invalidate( FID_FILL_TO_RIGHT );
    rBindings.Invalidate( FID_FILL_TO_TOP );
    rBindings.Invalidate( FID_FILL_TO_LEFT );
    rBindings.Invalidate( FID_FILL_SERIES );
    rBindings.Invalidate( SID_SCENARIOS );
    rBindings.Invalidate( SID_AUTOFORMAT );
    rBindings.Invalidate( SID_OPENDLG_TABOP );
    rBindings.Invalidate( SID_DATA_SELECT );

    rBindings.Invalidate( SID_CUT );
    rBindings.Invalidate( SID_COPY );
    rBindings.Invalidate( SID_PASTE );
    rBindings.Invalidate( SID_PASTE_SPECIAL );
    rBindings.Invalidate( SID_PASTE_UNFORMATTED );

    rBindings.Invalidate( FID_INS_ROW );
    rBindings.Invalidate( FID_INS_COLUMN );
    rBindings.Invalidate( FID_INS_ROWS_BEFORE );
    rBindings.Invalidate( FID_INS_COLUMNS_BEFORE );
    rBindings.Invalidate( FID_INS_ROWS_AFTER );
    rBindings.Invalidate( FID_INS_COLUMNS_AFTER );
    rBindings.Invalidate( FID_INS_CELL );
    rBindings.Invalidate( FID_INS_CELLSDOWN );
    rBindings.Invalidate( FID_INS_CELLSRIGHT );

    rBindings.Invalidate( FID_CHG_COMMENT );

        // only due to  protect cell:

    rBindings.Invalidate( SID_CELL_FORMAT_RESET );
    rBindings.Invalidate( SID_DELETE );
    rBindings.Invalidate( SID_DELETE_CONTENTS );
    rBindings.Invalidate( FID_DELETE_CELL );
    rBindings.Invalidate( FID_CELL_FORMAT );
    rBindings.Invalidate( SID_ENABLE_HYPHENATION );
    rBindings.Invalidate( SID_INSERT_POSTIT );
    rBindings.Invalidate( SID_CHARMAP );
    rBindings.Invalidate( SID_OPENDLG_FUNCTION );
    rBindings.Invalidate( FID_VALIDATION );
    rBindings.Invalidate( SID_EXTERNAL_SOURCE );
    rBindings.Invalidate( SID_TEXT_TO_COLUMNS );
    rBindings.Invalidate( SID_SORT_ASCENDING );
    rBindings.Invalidate( SID_SORT_DESCENDING );
    rBindings.Invalidate( SID_SELECT_UNPROTECTED_CELLS );

    if (aViewData.GetViewShell()->HasAccessibilityObjects())
        aViewData.GetViewShell()->BroadcastAccessibility(SfxHint(SfxHintId::ScAccCursorChanged));

    CellContentChanged();
}

void ScTabView::CursorPosChanged()
{
    bool bRefMode = SC_MOD()->IsFormulaMode();
    if ( !bRefMode ) // check that RefMode works when switching sheets
        aViewData.GetDocShell()->Broadcast( SfxHint( SfxHintId::ScKillEditView ) );

    //  Broadcast, so that other Views of the document also switch

    ScDocument& rDoc = aViewData.GetDocument();
    bool bDP = nullptr != rDoc.GetDPAtCursor(
        aViewData.GetCurX(), aViewData.GetCurY(), aViewData.GetTabNo() );
    aViewData.GetViewShell()->SetPivotShell(bDP);

    //  UpdateInputHandler now in CellContentChanged

    SelectionChanged();

    aViewData.SetTabStartCol( SC_TABSTART_NONE );
}

namespace {

Point calcHintWindowPosition(
    const Point& rCellPos, const Size& rCellSize, const Size& rFrameWndSize, const Size& rHintWndSize)
{
    const tools::Long nMargin = 20;

    tools::Long nMLeft = rCellPos.X();
    tools::Long nMRight = rFrameWndSize.Width() - rCellPos.X() - rCellSize.Width();
    tools::Long nMTop = rCellPos.Y();
    tools::Long nMBottom = rFrameWndSize.Height() - rCellPos.Y() - rCellSize.Height();

    // First, see if we can fit the entire hint window in the visible region.

    if (nMRight - nMargin >= rHintWndSize.Width())
    {
        // Right margin is wide enough.
        if (rFrameWndSize.Height() >= rHintWndSize.Height())
        {
            // The frame has enough height.  Take it.
            Point aPos = rCellPos;
            aPos.AdjustX(rCellSize.Width() + nMargin );
            if (aPos.Y() + rHintWndSize.Height() > rFrameWndSize.Height())
            {
                // Push the hint window up a bit to make it fit.
                aPos.setY( rFrameWndSize.Height() - rHintWndSize.Height() );
            }
            return aPos;
        }
    }

    if (nMBottom - nMargin >= rHintWndSize.Height())
    {
        // Bottom margin is high enough.
        if (rFrameWndSize.Width() >= rHintWndSize.Width())
        {
            // The frame has enough width.  Take it.
            Point aPos = rCellPos;
            aPos.AdjustY(rCellSize.Height() + nMargin );
            if (aPos.X() + rHintWndSize.Width() > rFrameWndSize.Width())
            {
                // Move the hint window to the left to make it fit.
                aPos.setX( rFrameWndSize.Width() - rHintWndSize.Width() );
            }
            return aPos;
        }
    }

    if (nMLeft - nMargin >= rHintWndSize.Width())
    {
        // Left margin is wide enough.
        if (rFrameWndSize.Height() >= rHintWndSize.Height())
        {
            // The frame is high enough.  Take it.
            Point aPos = rCellPos;
            aPos.AdjustX( -(rHintWndSize.Width() + nMargin) );
            if (aPos.Y() + rHintWndSize.Height() > rFrameWndSize.Height())
            {
                // Push the hint window up a bit to make it fit.
                aPos.setY( rFrameWndSize.Height() - rHintWndSize.Height() );
            }
            return aPos;
        }
    }

    if (nMTop - nMargin >= rHintWndSize.Height())
    {
        // Top margin is high enough.
        if (rFrameWndSize.Width() >= rHintWndSize.Width())
        {
            // The frame is wide enough.  Take it.
            Point aPos = rCellPos;
            aPos.AdjustY( -(rHintWndSize.Height() + nMargin) );
            if (aPos.X() + rHintWndSize.Width() > rFrameWndSize.Width())
            {
                // Move the hint window to the left to make it fit.
                aPos.setX( rFrameWndSize.Width() - rHintWndSize.Width() );
            }
            return aPos;
        }
    }

    // The popup doesn't fit in any direction in its entirety.  Do our best.

    if (nMRight - nMargin >= rHintWndSize.Width())
    {
        // Right margin is good enough.
        Point aPos = rCellPos;
        aPos.AdjustX(nMargin + rCellSize.Width() );
        aPos.setY( 0 );
        return aPos;
    }

    if (nMBottom - nMargin >= rHintWndSize.Height())
    {
        // Bottom margin is good enough.
        Point aPos = rCellPos;
        aPos.AdjustY(nMargin + rCellSize.Height() );
        aPos.setX( 0 );
        return aPos;
    }

    if (nMLeft - nMargin >= rHintWndSize.Width())
    {
        // Left margin is good enough.
        Point aPos = rCellPos;
        aPos.AdjustX( -(rHintWndSize.Width() + nMargin) );
        aPos.setY( 0 );
        return aPos;
    }

    if (nMTop - nMargin >= rHintWndSize.Height())
    {
        // Top margin is good enough.
        Point aPos = rCellPos;
        aPos.AdjustY( -(rHintWndSize.Height() + nMargin) );
        aPos.setX( 0 );
        return aPos;
    }

    // None of the above.  Hopeless.  At least try not to cover the current
    // cell.
    Point aPos = rCellPos;
    aPos.AdjustX(rCellSize.Width() );
    return aPos;
}

}

void ScTabView::TestHintWindow()
{
    //  show input help window and list drop-down button for validity

    mxInputHintOO.reset();

    bool bListValButton = false;
    ScAddress aListValPos;

    ScDocument& rDoc = aViewData.GetDocument();
    const SfxUInt32Item* pItem = rDoc.GetAttr( aViewData.GetCurX(),
                                               aViewData.GetCurY(),
                                               aViewData.GetTabNo(),
                                               ATTR_VALIDDATA );
    if ( pItem->GetValue() )
    {
        const ScValidationData* pData = rDoc.GetValidationEntry( pItem->GetValue() );
        OSL_ENSURE(pData,"ValidationData not found");
        OUString aTitle, aMessage;

        if ( pData && pData->GetInput( aTitle, aMessage ) && !aMessage.isEmpty() )
        {
            ScSplitPos eWhich = aViewData.GetActivePart();
            ScGridWindow* pWin = pGridWin[eWhich].get();
            SCCOL nCol = aViewData.GetCurX();
            SCROW nRow = aViewData.GetCurY();
            Point aPos = aViewData.GetScrPos( nCol, nRow, eWhich );
            Size aWinSize = pWin->GetOutputSizePixel();
            // cursor visible?
            if ( nCol >= aViewData.GetPosX(WhichH(eWhich)) &&
                 nRow >= aViewData.GetPosY(WhichV(eWhich)) &&
                 aPos.X() < aWinSize.Width() && aPos.Y() < aWinSize.Height() )
            {
                const svtools::ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
                Color aCommentColor = rColorCfg.GetColorValue(svtools::CALCNOTESBACKGROUND).nColor;
                // create HintWindow, determines its size by itself
                ScOverlayHint* pOverlay = new ScOverlayHint(aTitle, aMessage, aCommentColor, pFrameWin->GetFont());

                mxInputHintOO.reset(new sdr::overlay::OverlayObjectList);
                mxInputHintOO->append(std::unique_ptr<sdr::overlay::OverlayObject>(pOverlay));

                Size aHintWndSize = pOverlay->GetSizePixel();
                tools::Long nCellSizeX = 0;
                tools::Long nCellSizeY = 0;
                aViewData.GetMergeSizePixel(nCol, nRow, nCellSizeX, nCellSizeY);

                Point aHintPos = calcHintWindowPosition(
                    aPos, Size(nCellSizeX,nCellSizeY), aWinSize, aHintWndSize);

                pOverlay->SetPos(pWin->PixelToLogic(aHintPos, pWin->GetDrawMapMode()), pWin->GetDrawMapMode());
                for (VclPtr<ScGridWindow> & pWindow : pGridWin)
                {
                    if (!pWindow)
                        continue;
                    if (!pWindow->IsVisible())
                        continue;
                    rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = pWindow->getOverlayManager();
                    if (!xOverlayManager.is())
                        continue;
                    if (pWindow == pWin)
                    {
                        xOverlayManager->add(*pOverlay);
                        pWindow->updateLOKInputHelp(aTitle, aMessage);
                    }
                    else
                    {
                        //tdf#92530 if the help tip doesn't fit into its allocated area in a split window
                        //scenario, then because here we place it into the other split windows as well the
                        //missing portions will be displayed in the other split windows to form an apparent
                        //single tip, albeit "under" the split lines
                        Point aOtherPos(pWindow->ScreenToOutputPixel(pWin->OutputToScreenPixel(aHintPos)));
                        std::unique_ptr<ScOverlayHint> pOtherOverlay(new ScOverlayHint(aTitle, aMessage, aCommentColor, pFrameWin->GetFont()));
                        Point aFooPos(pWindow->PixelToLogic(aOtherPos, pWindow->GetDrawMapMode()));
                        pOtherOverlay->SetPos(aFooPos, pWindow->GetDrawMapMode());
                        xOverlayManager->add(*pOtherOverlay);
                        mxInputHintOO->append(std::move(pOtherOverlay));
                    }
                }
            }
        }

        // list drop-down button
        if ( pData && pData->HasSelectionList() )
        {
            aListValPos.Set( aViewData.GetCurX(), aViewData.GetCurY(), aViewData.GetTabNo() );
            bListValButton = true;
        }
    }

    for (VclPtr<ScGridWindow> const & pWin : pGridWin)
    {
        if (pWin && pWin->IsVisible())
            pWin->UpdateListValPos(bListValButton, aListValPos);
    }
}

bool ScTabView::HasHintWindow() const { return mxInputHintOO != nullptr; }

void ScTabView::RemoveHintWindow()
{
    mxInputHintOO.reset();
}

// find window that should not be over the cursor
static weld::Window* lcl_GetCareWin(SfxViewFrame* pViewFrm)
{
    //! also spelling ??? (then set the member variables when calling)

    // search & replace
    if (pViewFrm->HasChildWindow(SID_SEARCH_DLG))
    {
        SfxChildWindow* pChild = pViewFrm->GetChildWindow(SID_SEARCH_DLG);
        if (pChild)
        {
            auto xDlgController = pChild->GetController();
            if (xDlgController && xDlgController->getDialog()->get_visible())
                return xDlgController->getDialog();
        }
    }

    // apply changes
    if ( pViewFrm->HasChildWindow(FID_CHG_ACCEPT) )
    {
        SfxChildWindow* pChild = pViewFrm->GetChildWindow(FID_CHG_ACCEPT);
        if (pChild)
        {
            auto xDlgController = pChild->GetController();
            if (xDlgController && xDlgController->getDialog()->get_visible())
                return xDlgController->getDialog();
        }
    }

    return nullptr;
}

    // adjust screen with respect to cursor position

void ScTabView::AlignToCursor( SCCOL nCurX, SCROW nCurY, ScFollowMode eMode,
                                const ScSplitPos* pWhich )
{
    // now switch active part here

    ScSplitPos eActive = aViewData.GetActivePart();
    ScHSplitPos eActiveX = WhichH(eActive);
    ScVSplitPos eActiveY = WhichV(eActive);
    bool bHFix = (aViewData.GetHSplitMode() == SC_SPLIT_FIX);
    bool bVFix = (aViewData.GetVSplitMode() == SC_SPLIT_FIX);
    if (bHFix && eActiveX == SC_SPLIT_LEFT && nCurX >= aViewData.GetFixPosX())
    {
        ActivatePart( (eActiveY==SC_SPLIT_TOP) ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT );
        eActiveX = SC_SPLIT_RIGHT;
    }
    if (bVFix && eActiveY == SC_SPLIT_TOP && nCurY >= aViewData.GetFixPosY())
    {
        ActivatePart( (eActiveX==SC_SPLIT_LEFT) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT );
        eActiveY = SC_SPLIT_BOTTOM;
    }

    // actual align

    if ( eMode != SC_FOLLOW_NONE )
    {
        ScSplitPos eAlign;
        if (pWhich)
            eAlign = *pWhich;
        else
            eAlign = aViewData.GetActivePart();
        ScHSplitPos eAlignX = WhichH(eAlign);
        ScVSplitPos eAlignY = WhichV(eAlign);

        SCCOL nDeltaX = aViewData.GetPosX(eAlignX);
        SCROW nDeltaY = aViewData.GetPosY(eAlignY);
        SCCOL nSizeX = aViewData.VisibleCellsX(eAlignX);
        SCROW nSizeY = aViewData.VisibleCellsY(eAlignY);

        tools::Long nCellSizeX;
        tools::Long nCellSizeY;
        if ( nCurX >= 0 && nCurY >= 0 )
            aViewData.GetMergeSizePixel( nCurX, nCurY, nCellSizeX, nCellSizeY );
        else
            nCellSizeX = nCellSizeY = 0;
        Size aScrSize = aViewData.GetScrSize();
        tools::Long nSpaceX = ( aScrSize.Width()  - nCellSizeX ) / 2;
        tools::Long nSpaceY = ( aScrSize.Height() - nCellSizeY ) / 2;
        //  nSpaceY: desired start position of cell for FOLLOW_JUMP, modified if dialog interferes

        bool bForceNew = false;     // force new calculation of JUMP position (vertical only)

        // VisibleCellsY == CellsAtY( GetPosY( eWhichY ), 1, eWhichY )

        // when for instance a search dialog is open, don't put the cursor behind the dialog
        // if possible, put the row with the cursor above or below the dialog
        //! not if already completely visible

        if ( eMode == SC_FOLLOW_JUMP )
        {
            weld::Window* pCare = lcl_GetCareWin( aViewData.GetViewShell()->GetViewFrame() );
            if (pCare)
            {
                bool bLimit = false;
                tools::Rectangle aDlgPixel;
                Size aWinSize;
                vcl::Window* pWin = GetActiveWin();
                weld::Window* pFrame = pWin ? pWin->GetFrameWeld() : nullptr;
                int x, y, width, height;
                if (pFrame && pCare->get_extents_relative_to(*pFrame, x, y, width, height))
                {
                    aDlgPixel = tools::Rectangle(Point(x, y), Size(width, height));
                    aWinSize = pWin->GetOutputSizePixel();
                    // dos the dialog cover the GridWin?
                    if ( aDlgPixel.Right() >= 0 && aDlgPixel.Left() < aWinSize.Width() )
                    {
                        if ( nCurX < nDeltaX || nCurX >= nDeltaX+nSizeX ||
                             nCurY < nDeltaY || nCurY >= nDeltaY+nSizeY )
                            bLimit = true;          // scroll anyway
                        else
                        {
                            // cursor is on the screen
                            Point aStart = aViewData.GetScrPos( nCurX, nCurY, eAlign );
                            tools::Long nCSX, nCSY;
                            aViewData.GetMergeSizePixel( nCurX, nCurY, nCSX, nCSY );
                            tools::Rectangle aCursor( aStart, Size( nCSX, nCSY ) );
                            if ( aCursor.Overlaps( aDlgPixel ) )
                                bLimit = true;      // cell is covered by the dialog
                        }
                    }
                }

                if (bLimit)
                {
                    bool bBottom = false;
                    tools::Long nTopSpace = aDlgPixel.Top();
                    tools::Long nBotSpace = aWinSize.Height() - aDlgPixel.Bottom();
                    if ( nBotSpace > 0 && nBotSpace > nTopSpace )
                    {
                        tools::Long nDlgBot = aDlgPixel.Bottom();
                        SCCOL nWPosX;
                        SCROW nWPosY;
                        aViewData.GetPosFromPixel( 0,nDlgBot, eAlign, nWPosX, nWPosY );
                        ++nWPosY;   // below the last affected cell

                        SCROW nDiff = nWPosY - nDeltaY;
                        if ( nCurY >= nDiff )           // position can not be negative
                        {
                            nSpaceY = nDlgBot + ( nBotSpace - nCellSizeY ) / 2;
                            bBottom = true;
                            bForceNew = true;
                        }
                    }
                    if ( !bBottom && nTopSpace > 0 )
                    {
                        nSpaceY = ( nTopSpace - nCellSizeY ) / 2;
                        bForceNew = true;
                    }
                }
            }
        }

        SCCOL nNewDeltaX = nDeltaX;
        SCROW nNewDeltaY = nDeltaY;
        bool bDoLine = false;

        switch (eMode)
        {
            case SC_FOLLOW_JUMP:
                if ( nCurX < nDeltaX || nCurX >= nDeltaX+nSizeX )
                {
                    nNewDeltaX = nCurX - aViewData.CellsAtX( nCurX, -1, eAlignX, static_cast<sal_uInt16>(nSpaceX) );
                    if (nNewDeltaX < 0)
                        nNewDeltaX = 0;
                    nSizeX = aViewData.CellsAtX( nNewDeltaX, 1, eAlignX );
                }
                if ( nCurY < nDeltaY || nCurY >= nDeltaY+nSizeY || bForceNew )
                {
                    nNewDeltaY = nCurY - aViewData.CellsAtY( nCurY, -1, eAlignY, static_cast<sal_uInt16>(nSpaceY) );
                    if (nNewDeltaY < 0)
                        nNewDeltaY = 0;
                    nSizeY = aViewData.CellsAtY( nNewDeltaY, 1, eAlignY );
                }
                bDoLine = true;
                break;

            case SC_FOLLOW_LINE:
                bDoLine = true;
                break;

            case SC_FOLLOW_FIX:
                if ( nCurX < nDeltaX || nCurX >= nDeltaX+nSizeX )
                {
                    nNewDeltaX = nDeltaX + nCurX - aViewData.GetCurX();
                    if (nNewDeltaX < 0)
                        nNewDeltaX = 0;
                    nSizeX = aViewData.CellsAtX( nNewDeltaX, 1, eAlignX );
                }
                if ( nCurY < nDeltaY || nCurY >= nDeltaY+nSizeY )
                {
                    nNewDeltaY = nDeltaY + nCurY - aViewData.GetCurY();
                    if (nNewDeltaY < 0)
                        nNewDeltaY = 0;
                    nSizeY = aViewData.CellsAtY( nNewDeltaY, 1, eAlignY );
                }

                //  like old version of SC_FOLLOW_JUMP:

                if ( nCurX < nNewDeltaX || nCurX >= nNewDeltaX+nSizeX )
                {
                    nNewDeltaX = nCurX - (nSizeX / 2);
                    if (nNewDeltaX < 0)
                        nNewDeltaX = 0;
                    nSizeX = aViewData.CellsAtX( nNewDeltaX, 1, eAlignX );
                }
                if ( nCurY < nNewDeltaY || nCurY >= nNewDeltaY+nSizeY )
                {
                    nNewDeltaY = nCurY - (nSizeY / 2);
                    if (nNewDeltaY < 0)
                        nNewDeltaY = 0;
                    nSizeY = aViewData.CellsAtY( nNewDeltaY, 1, eAlignY );
                }

                bDoLine = true;
                break;

            case SC_FOLLOW_NONE:
                break;
            default:
                OSL_FAIL("Wrong cursor mode");
                break;
        }

        ScDocument& rDoc = aViewData.GetDocument();
        if (bDoLine)
        {
            while ( nCurX >= nNewDeltaX+nSizeX )
            {
                nNewDeltaX = nCurX-nSizeX+1;
                SCTAB nTab = aViewData.GetTabNo();
                while ( nNewDeltaX < rDoc.MaxCol() && !rDoc.GetColWidth( nNewDeltaX, nTab ) )
                    ++nNewDeltaX;
                nSizeX = aViewData.CellsAtX( nNewDeltaX, 1, eAlignX );
            }
            while ( nCurY >= nNewDeltaY+nSizeY )
            {
                nNewDeltaY = nCurY-nSizeY+1;
                SCTAB nTab = aViewData.GetTabNo();
                while ( nNewDeltaY < rDoc.MaxRow() && !rDoc.GetRowHeight( nNewDeltaY, nTab ) )
                    ++nNewDeltaY;
                nSizeY = aViewData.CellsAtY( nNewDeltaY, 1, eAlignY );
            }
            if ( nCurX < nNewDeltaX )
                nNewDeltaX = nCurX;
            if ( nCurY < nNewDeltaY )
                nNewDeltaY = nCurY;
        }

        if ( nNewDeltaX != nDeltaX )
            nSizeX = aViewData.CellsAtX( nNewDeltaX, 1, eAlignX );
        if (nNewDeltaX+nSizeX-1 > rDoc.MaxCol())
            nNewDeltaX = rDoc.MaxCol()-nSizeX+1;
        if (nNewDeltaX < 0)
            nNewDeltaX = 0;

        if ( nNewDeltaY != nDeltaY )
            nSizeY = aViewData.CellsAtY( nNewDeltaY, 1, eAlignY );
        if (nNewDeltaY+nSizeY-1 > rDoc.MaxRow())
            nNewDeltaY = rDoc.MaxRow()-nSizeY+1;
        if (nNewDeltaY < 0)
            nNewDeltaY = 0;

        if ( nNewDeltaX != nDeltaX )
            ScrollX( nNewDeltaX - nDeltaX, eAlignX );
        if ( nNewDeltaY != nDeltaY )
            ScrollY( nNewDeltaY - nDeltaY, eAlignY );
    }

    // switch active part again

    if (bHFix)
        if (eActiveX == SC_SPLIT_RIGHT && nCurX < aViewData.GetFixPosX())
        {
            ActivatePart( (eActiveY==SC_SPLIT_TOP) ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT );
            eActiveX = SC_SPLIT_LEFT;
        }
    if (bVFix)
        if (eActiveY == SC_SPLIT_BOTTOM && nCurY < aViewData.GetFixPosY())
        {
            ActivatePart( (eActiveX==SC_SPLIT_LEFT) ? SC_SPLIT_TOPLEFT : SC_SPLIT_TOPRIGHT );
        }
}

bool ScTabView::SelMouseButtonDown( const MouseEvent& rMEvt )
{
    bool bRet = false;

    // #i3875# *Hack*
    bool bMod1Locked = (aViewData.GetViewShell()->GetLockedModifiers() & KEY_MOD1) != 0;
    aViewData.SetSelCtrlMouseClick( rMEvt.IsMod1() || bMod1Locked );

    if ( pSelEngine )
    {
        bMoveIsShift = rMEvt.IsShift();
        bRet = pSelEngine->SelMouseButtonDown( rMEvt );
        bMoveIsShift = false;
    }

    aViewData.SetSelCtrlMouseClick( false ); // #i3875# *Hack*

    return bRet;
}

    //  MoveCursor - with adjustment of the view section

void ScTabView::MoveCursorAbs( SCCOL nCurX, SCROW nCurY, ScFollowMode eMode,
                               bool bShift, bool bControl, bool bKeepOld, bool bKeepSel )
{
    if (!bKeepOld)
        aViewData.ResetOldCursor();

    ScDocument& rDoc = aViewData.GetDocument();
    // #i123629#
    if( aViewData.GetViewShell()->GetForceFocusOnCurCell() )
        aViewData.GetViewShell()->SetForceFocusOnCurCell( !rDoc.ValidColRow(nCurX, nCurY) );

    if (nCurX < 0) nCurX = 0;
    if (nCurY < 0) nCurY = 0;
    if (nCurX > rDoc.MaxCol()) nCurX = rDoc.MaxCol();
    if (nCurY > rDoc.MaxRow()) nCurY = rDoc.MaxRow();

    // FIXME: this is to limit the number of rows handled in the Online
    // to 1000; this will be removed again when the performance
    // bottlenecks are sorted out
    if (comphelper::LibreOfficeKit::isActive())
        nCurY = std::min(nCurY, MAXTILEDROW);

    HideAllCursors();

    // switch of active now in AlignToCursor

    AlignToCursor( nCurX, nCurY, eMode );

    if (bKeepSel)
    {
        SetCursor( nCurX, nCurY );      // keep selection

        // If the cursor is in existing selection, it's a cursor movement by
        // ENTER or TAB.  If not, then it's a new selection during ADD
        // selection mode.

        const ScMarkData& rMark = aViewData.GetMarkData();
        ScRangeList aSelList;
        rMark.FillRangeListWithMarks(&aSelList, false);
        if (!aSelList.Contains(ScRange(nCurX, nCurY, aViewData.GetTabNo())))
            // Cursor not in existing selection.  Start a new selection.
            DoneBlockMode(true);
    }
    else
    {
        if (!bShift)
        {
            // Remove all marked data on cursor movement unless the Shift is locked.
            ScMarkData& rMark = aViewData.GetMarkData();
            bool bMarked = rMark.IsMarked() || rMark.IsMultiMarked();
            if (bMarked)
            {
                rMark.ResetMark();
                DoneBlockMode();
                InitOwnBlockMode();
                MarkDataChanged();
            }
        }

        bool bSame = ( nCurX == aViewData.GetCurX() && nCurY == aViewData.GetCurY() );
        bMoveIsShift = bShift;
        pSelEngine->CursorPosChanging( bShift, bControl );
        bMoveIsShift = false;
        aFunctionSet.SetCursorAtCell( nCurX, nCurY, false );

        // If the cursor has not been moved, the SelectionChanged for canceling the
        // selection has to happen here individually:
        if (bSame)
            SelectionChanged();
    }

    ShowAllCursors();
    TestHintWindow();
}

void ScTabView::MoveCursorRel( SCCOL nMovX, SCROW nMovY, ScFollowMode eMode,
                               bool bShift, bool bKeepSel )
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    bool bSkipProtected = false, bSkipUnprotected = false;
    const ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
    if ( pProtect && pProtect->isProtected() )
    {
        bSkipProtected   = !pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bSkipUnprotected = !pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }

    if ( bSkipProtected && bSkipUnprotected )
        return;

    SCCOL nOldX;
    SCROW nOldY;
    SCCOL nCurX;
    SCROW nCurY;
    if ( aViewData.IsRefMode() )
    {
        nOldX = aViewData.GetRefEndX();
        nOldY = aViewData.GetRefEndY();
        nCurX = nOldX + nMovX;
        nCurY = nOldY + nMovY;
    }
    else
    {
        nOldX = aViewData.GetCurX();
        nOldY = aViewData.GetCurY();
        nCurX = (nMovX != 0) ? nOldX+nMovX : aViewData.GetOldCurX();
        nCurY = (nMovY != 0) ? nOldY+nMovY : aViewData.GetOldCurY();
    }

    if (nMovX < 0 && nOldX == 0)
    { // trying to go left from 1st column
        if (nMovY == 0) // done, because no vertical move is requested
            return;
    }
    if (nMovY < 0 && nOldY == 0)
    { // trying to go up from 1st row
        if (nMovX == 0) // done, because no horizontal move is requested
            return;
    }

    aViewData.ResetOldCursor();

    if (nMovX != 0 && rDoc.ValidColRow(nCurX,nCurY))
        SkipCursorHorizontal(nCurX, nCurY, nOldX, nMovX);

    if (nMovY != 0 && rDoc.ValidColRow(nCurX,nCurY))
        SkipCursorVertical(nCurX, nCurY, nOldY, nMovY);

    MoveCursorAbs( nCurX, nCurY, eMode, bShift, false, true, bKeepSel );
}

void ScTabView::MoveCursorPage( SCCOL nMovX, SCROW nMovY, ScFollowMode eMode, bool bShift, bool bKeepSel )
{
    SCCOL nPageX;
    SCROW nPageY;
    GetPageMoveEndPosition(nMovX, nMovY, nPageX, nPageY);
    MoveCursorRel( nPageX, nPageY, eMode, bShift, bKeepSel );
}

void ScTabView::MoveCursorArea( SCCOL nMovX, SCROW nMovY, ScFollowMode eMode, bool bShift, bool bKeepSel )
{
    SCCOL nNewX;
    SCROW nNewY;
    GetAreaMoveEndPosition(nMovX, nMovY, eMode, nNewX, nNewY, eMode);
    MoveCursorRel(nNewX, nNewY, eMode, bShift, bKeepSel);
}

void ScTabView::MoveCursorEnd( SCCOL nMovX, SCROW nMovY, ScFollowMode eMode, bool bShift, bool bKeepSel )
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    SCCOL nCurX;
    SCROW nCurY;
    aViewData.GetMoveCursor( nCurX,nCurY );
    SCCOL nNewX = nCurX;
    SCROW nNewY = nCurY;

    SCCOL nUsedX = 0;
    SCROW nUsedY = 0;
    if ( nMovX > 0 || nMovY > 0 )
        rDoc.GetPrintArea( nTab, nUsedX, nUsedY );     // get end

    if (nMovX<0)
        nNewX=0;
    else if (nMovX>0)
        nNewX=nUsedX;                                   // last used range

    if (nMovY<0)
        nNewY=0;
    else if (nMovY>0)
        nNewY=nUsedY;

    aViewData.ResetOldCursor();
    MoveCursorRel( nNewX-nCurX, nNewY-nCurY, eMode, bShift, bKeepSel );
}

void ScTabView::MoveCursorScreen( SCCOL nMovX, SCROW nMovY, ScFollowMode eMode, bool bShift )
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    SCCOL nCurX;
    SCROW nCurY;
    aViewData.GetMoveCursor( nCurX,nCurY );
    SCCOL nNewX = nCurX;
    SCROW nNewY = nCurY;

    ScSplitPos eWhich = aViewData.GetActivePart();
    SCCOL nPosX = aViewData.GetPosX( WhichH(eWhich) );
    SCROW nPosY = aViewData.GetPosY( WhichV(eWhich) );

    SCCOL nAddX = aViewData.VisibleCellsX( WhichH(eWhich) );
    if (nAddX != 0)
        --nAddX;
    SCROW nAddY = aViewData.VisibleCellsY( WhichV(eWhich) );
    if (nAddY != 0)
        --nAddY;

    if (nMovX<0)
        nNewX=nPosX;
    else if (nMovX>0)
        nNewX=nPosX+nAddX;

    if (nMovY<0)
        nNewY=nPosY;
    else if (nMovY>0)
        nNewY=nPosY+nAddY;

    aViewData.SetOldCursor( nNewX,nNewY );
    rDoc.SkipOverlapped(nNewX, nNewY, nTab);
    MoveCursorAbs( nNewX, nNewY, eMode, bShift, false, true );
}

void ScTabView::MoveCursorEnter( bool bShift )          // bShift -> up/down
{
    const ScInputOptions& rOpt = SC_MOD()->GetInputOptions();
    if (!rOpt.GetMoveSelection())
    {
        aViewData.UpdateInputHandler(true);
        return;
    }

    SCCOL nMoveX = 0;
    SCROW nMoveY = 0;
    switch (static_cast<ScDirection>(rOpt.GetMoveDir()))
    {
        case DIR_BOTTOM:
            nMoveY = bShift ? -1 : 1;
            break;
        case DIR_RIGHT:
            nMoveX = bShift ? -1 : 1;
            break;
        case DIR_TOP:
            nMoveY = bShift ? 1 : -1;
            break;
        case DIR_LEFT:
            nMoveX = bShift ? 1 : -1;
            break;
    }

    SCCOL nCurX;
    SCROW nCurY;
    aViewData.GetMoveCursor( nCurX,nCurY );
    SCCOL nNewX = nCurX;
    SCROW nNewY = nCurY;
    SCTAB nTab  = aViewData.GetTabNo();

    ScMarkData& rMark = aViewData.GetMarkData();
    ScDocument& rDoc  = aViewData.GetDocument();

    if (rMark.IsMarked() || rMark.IsMultiMarked())
    {
        rDoc.GetNextPos( nNewX, nNewY, nTab, nMoveX, nMoveY, true, false, rMark );

        MoveCursorRel( nNewX - nCurX, nNewY - nCurY, SC_FOLLOW_LINE, false, true );

        //  update input line even if cursor was not moved
        if ( nNewX == nCurX && nNewY == nCurY )
            aViewData.UpdateInputHandler(true);
    }
    else
    {
        // After Tab and Enter back to the starting column again.
        const SCCOL nTabStartCol = ((nMoveY != 0 && !nMoveX) ? aViewData.GetTabStartCol() : SC_TABSTART_NONE);
        rDoc.GetNextPos( nNewX, nNewY, nTab, nMoveX, nMoveY, false, true, rMark, nTabStartCol );

        MoveCursorRel( nNewX - nCurX, nNewY - nCurY, SC_FOLLOW_LINE, false);
    }
}

bool ScTabView::MoveCursorKeyInput( const KeyEvent& rKeyEvent )
{
    const vcl::KeyCode& rKCode = rKeyEvent.GetKeyCode();

    enum { MOD_NONE, MOD_CTRL, MOD_ALT, MOD_BOTH } eModifier =
        rKCode.IsMod1() ?
            (rKCode.IsMod2() ? MOD_BOTH : MOD_CTRL) :
            (rKCode.IsMod2() ? MOD_ALT : MOD_NONE);

    bool bSel = rKCode.IsShift();
    sal_uInt16 nCode = rKCode.GetCode();

    // CURSOR keys
    SCCOL nDX = 0;
    SCROW nDY = 0;
    switch( nCode )
    {
        case KEY_LEFT:  nDX = -1;   break;
        case KEY_RIGHT: nDX = 1;    break;
        case KEY_UP:    nDY = -1;   break;
        case KEY_DOWN:  nDY = 1;    break;
    }
    if( nDX != 0 || nDY != 0 )
    {
        switch( eModifier )
        {
            case MOD_NONE:  MoveCursorRel( nDX, nDY, SC_FOLLOW_LINE, bSel );    break;
            case MOD_CTRL:  MoveCursorArea( nDX, nDY, SC_FOLLOW_JUMP, bSel );   break;
            default:
            {
                // added to avoid warnings
            }
        }
        // always true to suppress changes of col/row size (ALT+CURSOR)
        return true;
    }

    // PAGEUP/PAGEDOWN
    if( (nCode == KEY_PAGEUP) || (nCode == KEY_PAGEDOWN) )
    {
        nDX = (nCode == KEY_PAGEUP) ? -1 : 1;
        switch( eModifier )
        {
            case MOD_NONE:  MoveCursorPage( 0, static_cast<SCCOLROW>(nDX), SC_FOLLOW_FIX, bSel );  break;
            case MOD_ALT:   MoveCursorPage( nDX, 0, SC_FOLLOW_FIX, bSel );  break;
            case MOD_CTRL:  SelectNextTab( nDX, false );                    break;
            default:
            {
                // added to avoid warnings
            }
        }
        return true;
    }

    // HOME/END
    if( (nCode == KEY_HOME) || (nCode == KEY_END) )
    {
        nDX = (nCode == KEY_HOME) ? -1 : 1;
        ScFollowMode eMode = (nCode == KEY_HOME) ? SC_FOLLOW_LINE : SC_FOLLOW_JUMP;
        switch( eModifier )
        {
            case MOD_NONE:  MoveCursorEnd( nDX, 0, eMode, bSel );   break;
            case MOD_CTRL:  MoveCursorEnd( nDX, static_cast<SCCOLROW>(nDX), eMode, bSel ); break;
            default:
            {
                // added to avoid warnings
            }
        }
        return true;
    }

    return false;
}

        // next/previous unprotected cell
void ScTabView::FindNextUnprot( bool bShift, bool bInSelection )
{
    short nMove = bShift ? -1 : 1;

    ScMarkData& rMark = aViewData.GetMarkData();
    bool bMarked = bInSelection && (rMark.IsMarked() || rMark.IsMultiMarked());

    SCCOL nCurX;
    SCROW nCurY;
    aViewData.GetMoveCursor( nCurX,nCurY );
    SCCOL nNewX = nCurX;
    SCROW nNewY = nCurY;
    SCTAB nTab = aViewData.GetTabNo();

    ScDocument& rDoc = aViewData.GetDocument();
    rDoc.GetNextPos( nNewX,nNewY, nTab, nMove,0, bMarked, true, rMark );

    SCCOL nTabCol = aViewData.GetTabStartCol();
    if ( nTabCol == SC_TABSTART_NONE )
        nTabCol = nCurX;                    // back to this column after Enter

    MoveCursorRel( nNewX-nCurX, nNewY-nCurY, SC_FOLLOW_LINE, false, true );

    // TabCol is reset in MoveCursorRel...
    aViewData.SetTabStartCol( nTabCol );
}

void ScTabView::MarkColumns()
{
    SCCOL nStartCol;
    SCCOL nEndCol;

    ScMarkData& rMark = aViewData.GetMarkData();
    if (rMark.IsMarked())
    {
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );
        nStartCol = aMarkRange.aStart.Col();
        nEndCol = aMarkRange.aEnd.Col();
    }
    else
    {
        SCROW nDummy;
        aViewData.GetMoveCursor( nStartCol, nDummy );
        nEndCol=nStartCol;
    }

    SCTAB nTab = aViewData.GetTabNo();
    ScDocument& rDoc = aViewData.GetDocument();
    DoneBlockMode();
    InitBlockMode( nStartCol,0, nTab );
    MarkCursor( nEndCol, rDoc.MaxRow(), nTab );
    SelectionChanged();
}

void ScTabView::MarkRows()
{
    SCROW nStartRow;
    SCROW nEndRow;

    ScMarkData& rMark = aViewData.GetMarkData();
    if (rMark.IsMarked())
    {
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );
        nStartRow = aMarkRange.aStart.Row();
        nEndRow = aMarkRange.aEnd.Row();
    }
    else
    {
        SCCOL nDummy;
        aViewData.GetMoveCursor( nDummy, nStartRow );
        nEndRow=nStartRow;
    }

    SCTAB nTab = aViewData.GetTabNo();
    ScDocument& rDoc = aViewData.GetDocument();
    DoneBlockMode();
    InitBlockMode( 0,nStartRow, nTab );
    MarkCursor( rDoc.MaxCol(), nEndRow, nTab );
    SelectionChanged();
}


void ScTabView::MarkColumns(SCCOL nCol, sal_Int16 nModifier)
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCCOL nStartCol = nCol;
    SCTAB nTab = aViewData.GetTabNo();

    if ((nModifier & KEY_SHIFT) == KEY_SHIFT)
        bMoveIsShift = true;

    DoneBlockMode( nModifier != 0 );
    InitBlockMode( nStartCol, 0, nTab, true, true);
    MarkCursor( nCol, rDoc.MaxRow(), nTab );
    bMoveIsShift = false;
    SetCursor( nCol, 0 );
    SelectionChanged();
}

void ScTabView::MarkRows(SCROW nRow, sal_Int16 nModifier)
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCROW nStartRow = nRow;
    SCTAB nTab = aViewData.GetTabNo();

    if ((nModifier & KEY_SHIFT) == KEY_SHIFT)
        bMoveIsShift = true;

    DoneBlockMode( nModifier != 0 );
    InitBlockMode( 0, nStartRow, nTab, true, false, true );
    MarkCursor( rDoc.MaxCol(), nRow, nTab );
    bMoveIsShift = false;
    SetCursor( 0, nRow );
    SelectionChanged();
}

void ScTabView::MarkDataArea( bool bIncludeCursor )
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();
    SCCOL nStartCol = aViewData.GetCurX();
    SCROW nStartRow = aViewData.GetCurY();
    SCCOL nEndCol = nStartCol;
    SCROW nEndRow = nStartRow;

    rDoc.GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow, bIncludeCursor, false );

    HideAllCursors();
    DoneBlockMode();
    InitBlockMode( nStartCol, nStartRow, nTab );
    MarkCursor( nEndCol, nEndRow, nTab );
    ShowAllCursors();

    SelectionChanged();
}

void ScTabView::MarkMatrixFormula()
{
    ScDocument& rDoc = aViewData.GetDocument();
    ScAddress aCursor( aViewData.GetCurX(), aViewData.GetCurY(), aViewData.GetTabNo() );
    ScRange aMatrix;
    if ( rDoc.GetMatrixFormulaRange( aCursor, aMatrix ) )
    {
        MarkRange( aMatrix, false );        // cursor is already within the range
    }
}

void ScTabView::MarkRange( const ScRange& rRange, bool bSetCursor, bool bContinue )
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCTAB nTab = rRange.aStart.Tab();
    SetTabNo( nTab );

    HideAllCursors();
    DoneBlockMode( bContinue ); // bContinue==true -> clear old mark
    if (bSetCursor)             // if Cursor is set, also always align
    {
        SCCOL nAlignX = rRange.aStart.Col();
        SCROW nAlignY = rRange.aStart.Row();
        bool bCol = ( rRange.aStart.Col() == 0 && rRange.aEnd.Col() == rDoc.MaxCol() ) && !aViewData.GetDocument().IsInVBAMode();
        bool bRow = ( rRange.aStart.Row() == 0 && rRange.aEnd.Row() == rDoc.MaxRow() );
        if ( bCol )
            nAlignX = aViewData.GetPosX(WhichH(aViewData.GetActivePart()));
        if ( bRow )
            nAlignY = aViewData.GetPosY(WhichV(aViewData.GetActivePart()));
        AlignToCursor( nAlignX, nAlignY, SC_FOLLOW_JUMP );
    }
    InitBlockMode( rRange.aStart.Col(), rRange.aStart.Row(), nTab );
    MarkCursor( rRange.aEnd.Col(), rRange.aEnd.Row(), nTab );
    if (bSetCursor)
    {
        SCCOL nPosX = rRange.aStart.Col();
        SCROW nPosY = rRange.aStart.Row();
        rDoc.SkipOverlapped(nPosX, nPosY, nTab);

        aViewData.ResetOldCursor();
        SetCursor( nPosX, nPosY );
    }
    ShowAllCursors();

    SelectionChanged();
}

void ScTabView::Unmark()
{
    ScMarkData& rMark = aViewData.GetMarkData();
    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        SCCOL nCurX;
        SCROW nCurY;
        aViewData.GetMoveCursor( nCurX,nCurY );
        MoveCursorAbs( nCurX, nCurY, SC_FOLLOW_NONE, false, false );

        SelectionChanged();
    }
}

void ScTabView::SetMarkData( const ScMarkData& rNew )
{
    DoneBlockMode();
    InitOwnBlockMode();
    aViewData.GetMarkData() = rNew;

    MarkDataChanged();
}

void ScTabView::MarkDataChanged()
{
    // has to be called after making direct changes to mark data (not via MarkCursor etc)

    UpdateSelectionOverlay();
}

void ScTabView::SelectNextTab( short nDir, bool bExtendSelection )
{
    if (!nDir)
        return;
    OSL_ENSURE( nDir==-1 || nDir==1, "SelectNextTab: invalid value");

    ScDocument& rDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();
    if (nDir<0)
    {
        if (!nTab)
            return;
        --nTab;
        while (!rDoc.IsVisible(nTab))
        {
            if (!nTab)
                return;
            --nTab;
        }
    }
    else
    {
        SCTAB nCount = rDoc.GetTableCount();
        ++nTab;
        if (nTab >= nCount)
            return;
        while (!rDoc.IsVisible(nTab))
        {
            ++nTab;
            if (nTab >= nCount)
                return;
        }
    }

    SetTabNo( nTab, false, bExtendSelection );
    PaintExtras();
}

void ScTabView::SelectTabPage( const sal_uInt16 nTab )
{
    pTabControl->SwitchToPageId( nTab );
}

//  SetTabNo - set the displayed sheet

void ScTabView::SetTabNo( SCTAB nTab, bool bNew, bool bExtendSelection, bool bSameTabButMoved )
{
    if ( !ValidTab(nTab) )
    {
        OSL_FAIL("SetTabNo: invalid sheet");
        return;
    }

    if (!bNew && nTab == aViewData.GetTabNo())
        return;

    // FormShell would like to be informed before the switch
    FmFormShell* pFormSh = aViewData.GetViewShell()->GetFormShell();
    if (pFormSh)
    {
        bool bAllowed = pFormSh->PrepareClose();
        if (!bAllowed)
        {
            //! error message? or does FormShell do it?
            //! return error flag and cancel actions

            return;     // FormShell says that it can not be switched
        }
    }

                                    // not InputEnterHandler due to reference input

    ScDocument& rDoc = aViewData.GetDocument();

    rDoc.MakeTable( nTab );

    // Update pending row heights before switching the sheet, so Reschedule from the progress bar
    // doesn't paint the new sheet with old heights
    aViewData.GetDocShell()->UpdatePendingRowHeights( nTab );

    SCTAB nTabCount = rDoc.GetTableCount();
    SCTAB nOldPos = nTab;
    while (!rDoc.IsVisible(nTab))              // search for next visible
    {
        bool bUp = (nTab>=nOldPos);
        if (bUp)
        {
            ++nTab;
            if (nTab>=nTabCount)
            {
                nTab = nOldPos;
                bUp = false;
            }
        }

        if (!bUp)
        {
            if (nTab != 0)
                --nTab;
            else
            {
                OSL_FAIL("no visible sheets");
                rDoc.SetVisible( 0, true );
            }
        }
    }

    // #i71490# Deselect drawing objects before changing the sheet number in view data,
    // so the handling of notes still has the sheet selected on which the notes are.
    DrawDeselectAll();

    ScModule* pScMod = SC_MOD();
    bool bRefMode = pScMod->IsFormulaMode();
    if ( !bRefMode ) // query, so that RefMode works when switching sheet
    {
        DoneBlockMode();
        pSelEngine->Reset();                // reset all flags, including locked modifiers
        aViewData.SetRefTabNo( nTab );
    }

    ScSplitPos eOldActive = aViewData.GetActivePart();      // before switching
    bool bFocus = pGridWin[eOldActive] && pGridWin[eOldActive]->HasFocus();

    aViewData.SetTabNo( nTab );
    if (mpSpellCheckCxt)
        mpSpellCheckCxt->setTabNo( nTab );
    // UpdateShow before SetCursor, so that UpdateAutoFillMark finds the correct
    // window  (is called from SetCursor)
    UpdateShow();

    SfxBindings& rBindings = aViewData.GetBindings();
    ScMarkData& rMark = aViewData.GetMarkData();

    bool bAllSelected = true;
    for (SCTAB nSelTab = 0; nSelTab < nTabCount; ++nSelTab)
    {
        if (!rDoc.IsVisible(nSelTab) || rMark.GetTableSelect(nSelTab))
        {
            if (nTab == nSelTab)
                // This tab is already in selection.  Keep the current
                // selection.
                bExtendSelection = true;
        }
        else
        {
            bAllSelected = false;
            if (bExtendSelection)
                // We got what we need.  No need to stay in the loop.
                break;
        }
    }
    if (bAllSelected && !bNew)
        // #i6327# if all tables are selected, a selection event (#i6330#) will deselect all
        // (not if called with bNew to update settings)
        bExtendSelection = false;

    if (bExtendSelection)
        rMark.SelectTable( nTab, true );
    else
    {
        rMark.SelectOneTable( nTab );
        rBindings.Invalidate( FID_FILL_TAB );
        rBindings.Invalidate( FID_TAB_DESELECTALL );
    }

    bool bUnoRefDialog = pScMod->IsRefDialogOpen() && pScMod->GetCurRefDlgId() == WID_SIMPLE_REF;

    // recalc zoom-dependent values (before TabChanged, before UpdateEditViewPos)
    RefreshZoom();
    UpdateVarZoom();

    if ( bRefMode )     // hide EditView if necessary (after aViewData.SetTabNo !)
    {
        for (VclPtr<ScGridWindow> & pWin : pGridWin)
        {
            if (pWin && pWin->IsVisible())
                pWin->UpdateEditViewPos();
        }
    }

    TabChanged(bSameTabButMoved);                                       // DrawView
    collectUIInformation({{"TABLE", OUString::number(nTab)}});
    UpdateVisibleRange();

    aViewData.GetViewShell()->WindowChanged();          // if the active window has changed
    aViewData.ResetOldCursor();
    SetCursor( aViewData.GetCurX(), aViewData.GetCurY(), true );

    if ( !bUnoRefDialog )
        aViewData.GetViewShell()->DisconnectAllClients();   // important for floating frames
    else
    {
        // hide / show inplace client
        ScClient* pClient = static_cast<ScClient*>(aViewData.GetViewShell()->GetIPClient());
        if ( pClient && pClient->IsObjectInPlaceActive() )
        {
            tools::Rectangle aObjArea = pClient->GetObjArea();
            if ( nTab == aViewData.GetRefTabNo() )
            {
                // move to its original position

                SdrOle2Obj* pDrawObj = pClient->GetDrawObj();
                if ( pDrawObj )
                {
                    tools::Rectangle aRect = pDrawObj->GetLogicRect();
                    MapMode aMapMode( MapUnit::Map100thMM );
                    Size aOleSize = pDrawObj->GetOrigObjSize( &aMapMode );
                    aRect.SetSize( aOleSize );
                    aObjArea = aRect;
                }
            }
            else
            {
                // move to an invisible position

                aObjArea.SetPos( Point( 0, -2*aObjArea.GetHeight() ) );
            }
            pClient->SetObjArea( aObjArea );
        }
    }

    if ( bFocus && aViewData.GetActivePart() != eOldActive && !bRefMode )
        ActiveGrabFocus();      // grab focus to the pane that's active now

        // freeze

    bool bResize = false;
    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        if (aViewData.UpdateFixX())
            bResize = true;
    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        if (aViewData.UpdateFixY())
            bResize = true;
    if (bResize)
        RepeatResize();
    InvalidateSplit();

    if ( aViewData.IsPagebreakMode() )
        UpdatePageBreakData();              //! asynchronously ??

    // Form Layer must know the visible area of the new sheet
    // that is why MapMode must already be correct here
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
    {
        if (pWin)
            pWin->SetMapMode(pWin->GetDrawMapMode());
    }
    SetNewVisArea();

    PaintGrid();
    PaintTop();
    PaintLeft();
    PaintExtras();

    DoResize( aBorderPos, aFrameSize );
    rBindings.Invalidate( SID_DELETE_PRINTAREA );   // Menu
    rBindings.Invalidate( FID_DEL_MANUALBREAKS );
    rBindings.Invalidate( FID_RESET_PRINTZOOM );
    rBindings.Invalidate( SID_STATUS_DOCPOS );      // Status bar
    rBindings.Invalidate( SID_ROWCOL_SELCOUNT );    // Status bar
    rBindings.Invalidate( SID_STATUS_PAGESTYLE );   // Status bar
    rBindings.Invalidate( SID_CURRENTTAB );         // Navigator
    rBindings.Invalidate( SID_STYLE_FAMILY2 );      // Designer
    rBindings.Invalidate( SID_STYLE_FAMILY4 );      // Designer
    rBindings.Invalidate( SID_TABLES_COUNT );

    if (pScMod->IsRefDialogOpen())
    {
        sal_uInt16 nCurRefDlgId=pScMod->GetCurRefDlgId();
        SfxViewFrame* pViewFrm = aViewData.GetViewShell()->GetViewFrame();
        SfxChildWindow* pChildWnd = pViewFrm->GetChildWindow( nCurRefDlgId );
        if (pChildWnd)
        {
            if (pChildWnd->GetController())
            {
                IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetController().get());
                if (pRefDlg)
                    pRefDlg->ViewShellChanged();
            }
        }
    }

    OnLibreOfficeKitTabChanged();
}

void ScTabView::AddWindowToForeignEditView(SfxViewShell* pViewShell, ScSplitPos eWhich)
{
    aExtraEditViewManager.Add(pViewShell, eWhich);
}

void ScTabView::RemoveWindowFromForeignEditView(SfxViewShell* pViewShell, ScSplitPos eWhich)
{
    aExtraEditViewManager.Remove(pViewShell, eWhich);
}

void ScTabView::OnLibreOfficeKitTabChanged()
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    ScTabViewShell* pThisViewShell = aViewData.GetViewShell();
    SCTAB nThisTabNo = pThisViewShell->GetViewData().GetTabNo();
    auto lTabSwitch = [pThisViewShell, nThisTabNo] (ScTabViewShell* pOtherViewShell)
    {
        ScViewData& rOtherViewData = pOtherViewShell->GetViewData();
        SCTAB nOtherTabNo = rOtherViewData.GetTabNo();
        if (nThisTabNo == nOtherTabNo)
        {
            for (int i = 0; i < 4; ++i)
            {
                if (rOtherViewData.HasEditView(ScSplitPos(i)))
                {
                    pThisViewShell->AddWindowToForeignEditView(pOtherViewShell, ScSplitPos(i));
                }
            }
        }
        else
        {
            for (int i = 0; i < 4; ++i)
            {
                if (rOtherViewData.HasEditView(ScSplitPos(i)))
                {
                    pThisViewShell->RemoveWindowFromForeignEditView(pOtherViewShell, ScSplitPos(i));
                }
            }
        }
    };

    SfxLokHelper::forEachOtherView(pThisViewShell, lTabSwitch);

    pThisViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_HEADER, "all");

    if (pThisViewShell->GetInputHandler())
        pThisViewShell->GetInputHandler()->UpdateLokReferenceMarks();
}

// paint functions - only for this View

void ScTabView::MakeEditView( ScEditEngineDefaulter* pEngine, SCCOL nCol, SCROW nRow )
{
    DrawDeselectAll();

    if (pDrawView)
        DrawEnableAnim( false );

    EditView* pSpellingView = aViewData.GetSpellingView();

    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if (pGridWin[i] && pGridWin[i]->IsVisible() && !aViewData.HasEditView(ScSplitPos(i)))
        {
            ScHSplitPos eHWhich = WhichH( static_cast<ScSplitPos>(i) );
            ScVSplitPos eVWhich = WhichV( static_cast<ScSplitPos>(i) );
            SCCOL nScrX = aViewData.GetPosX( eHWhich );
            SCROW nScrY = aViewData.GetPosY( eVWhich );

            bool bPosVisible =
                 ( nCol >= nScrX && nCol <= nScrX + aViewData.VisibleCellsX(eHWhich) - 1 &&
                   nRow >= nScrY && nRow <= nScrY + aViewData.VisibleCellsY(eVWhich) - 1 );

            //  for the active part, create edit view even if outside the visible area,
            //  so input isn't lost (and the edit view may be scrolled into the visible area)

            //  #i26433# during spelling, the spelling view must be active
            if ( bPosVisible || aViewData.GetActivePart() == static_cast<ScSplitPos>(i) ||
                 ( pSpellingView && aViewData.GetEditView(static_cast<ScSplitPos>(i)) == pSpellingView ) )
            {
                pGridWin[i]->HideCursor();

                pGridWin[i]->DeleteCursorOverlay();
                pGridWin[i]->DeleteAutoFillOverlay();
                pGridWin[i]->DeleteCopySourceOverlay();

                // flush OverlayManager before changing MapMode to text edit
                pGridWin[i]->flushOverlayManager();

                // MapMode must be set after HideCursor
                pGridWin[i]->SetMapMode(aViewData.GetLogicMode());

                aViewData.SetEditEngine( static_cast<ScSplitPos>(i), pEngine, pGridWin[i], nCol, nRow );

                if ( !bPosVisible )
                {
                    //  move the edit view area to the real (possibly negative) position,
                    //  or hide if completely above or left of the window
                    pGridWin[i]->UpdateEditViewPos();
                }
            }
        }
    }

    if (aViewData.GetViewShell()->HasAccessibilityObjects())
        aViewData.GetViewShell()->BroadcastAccessibility(SfxHint(SfxHintId::ScAccEnterEditMode));
}

void ScTabView::UpdateEditView()
{
    ScSplitPos eActive = aViewData.GetActivePart();
    for (sal_uInt16 i = 0; i < 4; i++)
    {
        ScSplitPos eCurrent = ScSplitPos(i);
        if (aViewData.HasEditView(eCurrent))
        {
            EditView* pEditView = aViewData.GetEditView(eCurrent);

            tools::Long nRefTabNo = GetViewData().GetRefTabNo();
            tools::Long nX = GetViewData().GetCurXForTab(nRefTabNo);
            tools::Long nY = GetViewData().GetCurYForTab(nRefTabNo);

            aViewData.SetEditEngine(eCurrent,
                static_cast<ScEditEngineDefaulter*>(pEditView->GetEditEngine()),
                pGridWin[i], nX, nY );
            if (eCurrent == eActive)
                pEditView->ShowCursor( false );
        }
    }
}

void ScTabView::KillEditView( bool bNoPaint )
{
    SCCOL nCol1 = aViewData.GetEditStartCol();
    SCROW nRow1 = aViewData.GetEditStartRow();
    SCCOL nCol2 = aViewData.GetEditEndCol();
    SCROW nRow2 = aViewData.GetEditEndRow();
    bool bPaint[4];
    bool bNotifyAcc = false;
    tools::Rectangle aRectangle[4];

    bool bExtended = nRow1 != nRow2;                    // column is painted to the end anyway

    bool bAtCursor = nCol1 <= aViewData.GetCurX() &&
                     nCol2 >= aViewData.GetCurX() &&
                     nRow1 == aViewData.GetCurY();
    for (sal_uInt16 i = 0; i < 4; i++)
    {
        bPaint[i] = aViewData.HasEditView( static_cast<ScSplitPos>(i) );
        if (bPaint[i])
        {
            bNotifyAcc = true;

            EditView* pView = aViewData.GetEditView( static_cast<ScSplitPos>(i) );
            aRectangle[i] = pView->GetInvalidateRect();
        }
    }

    // notify accessibility before all things happen
    if (bNotifyAcc && aViewData.GetViewShell()->HasAccessibilityObjects())
        aViewData.GetViewShell()->BroadcastAccessibility(SfxHint(SfxHintId::ScAccLeaveEditMode));

    aViewData.ResetEditView();
    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if (pGridWin[i] && bPaint[i] && pGridWin[i]->IsVisible())
        {
            pGridWin[i]->ShowCursor();

            pGridWin[i]->SetMapMode(pGridWin[i]->GetDrawMapMode());

            if (comphelper::LibreOfficeKit::isActive())
            {
                const tools::Rectangle& rInvRect = aRectangle[i];
                pGridWin[i]->Invalidate(rInvRect);

                // invalidate other views
                auto lInvalidateWindows =
                        [&rInvRect] (ScTabView* pTabView)
                        {
                            for (VclPtr<ScGridWindow> const & pWin: pTabView->pGridWin)
                            {
                                if (pWin)
                                    pWin->Invalidate(rInvRect);
                            }
                        };

                SfxLokHelper::forEachOtherView(GetViewData().GetViewShell(), lInvalidateWindows);
            }
            // #i73567# the cell still has to be repainted
            else if (bExtended || ( bAtCursor && !bNoPaint ))
            {
                pGridWin[i]->Draw( nCol1, nRow1, nCol2, nRow2, ScUpdateMode::All );
                pGridWin[i]->UpdateSelectionOverlay();
            }
        }
    }

    if (pDrawView)
        DrawEnableAnim( true );

        // GrabFocus always when this View is active and
        // when the input row has the focus

    bool bGrabFocus = false;
    if (aViewData.IsActive())
    {
        ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
        if ( pInputHdl )
        {
            ScInputWindow* pInputWin = pInputHdl->GetInputWindow();
            if (pInputWin && pInputWin->IsInputActive())
                bGrabFocus = true;
        }
    }

    if (bGrabFocus)
    {
//      should be done like this, so that Sfx notice it, but it does not work:
//!     aViewData.GetViewShell()->GetViewFrame()->GetWindow().GrabFocus();
//      therefore first like this:
        GetActiveWin()->GrabFocus();
    }

    // cursor query only after GrabFocus

    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if (pGridWin[i] && pGridWin[i]->IsVisible())
        {
            vcl::Cursor* pCur = pGridWin[i]->GetCursor();
            if (pCur && pCur->IsVisible())
                pCur->Hide();

            if (bPaint[i])
            {
                pGridWin[i]->UpdateCursorOverlay();
                pGridWin[i]->UpdateAutoFillOverlay();
            }
        }
    }
}

void ScTabView::UpdateFormulas(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow)
{
    if ( aViewData.GetDocument().IsAutoCalcShellDisabled() )
        return;

    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if (pGridWin[i] && pGridWin[i]->IsVisible())
            pGridWin[i]->UpdateFormulas(nStartCol, nStartRow, nEndCol, nEndRow);
    }

    if ( aViewData.IsPagebreakMode() )
        UpdatePageBreakData();              //! asynchronous

    UpdateHeaderWidth();

    //  if in edit mode, adjust edit view area because widths/heights may have changed
    if ( aViewData.HasEditView( aViewData.GetActivePart() ) )
        UpdateEditView();
}

//  PaintArea - repaint block

void ScTabView::PaintArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            ScUpdateMode eMode )
{
    SCCOL nCol1;
    SCROW nRow1;
    SCCOL nCol2;
    SCROW nRow2;
    bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();
    ScDocument& rDoc = aViewData.GetDocument();

    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartRow, nEndRow );

    for (size_t i = 0; i < 4; ++i)
    {
        if (!pGridWin[i] || !pGridWin[i]->IsVisible())
            continue;

        ScHSplitPos eHWhich = WhichH( static_cast<ScSplitPos>(i) );
        ScVSplitPos eVWhich = WhichV( static_cast<ScSplitPos>(i) );
        bool bOut = false;

        nCol1 = nStartCol;
        nRow1 = nStartRow;
        nCol2 = nEndCol;
        nRow2 = nEndRow;

        SCCOL nLastX = 0;
        SCROW nLastY = 0;

        if (bIsTiledRendering)
        {
            nLastX = aViewData.GetMaxTiledCol();
            nLastY = aViewData.GetMaxTiledRow();
        }
        else
        {

            SCCOL nScrX = aViewData.GetPosX( eHWhich );
            SCROW nScrY = aViewData.GetPosY( eVWhich );

            if (nCol1 < nScrX)
                nCol1 = nScrX;
            if (nCol2 < nScrX)
            {
                if ( eMode == ScUpdateMode::All )   // for UPDATE_ALL, paint anyway
                    nCol2 = nScrX;              // (because of extending strings to the right)
                else
                    bOut = true;                // completely outside the window
            }
            if (nRow1 < nScrY)
                nRow1 = nScrY;
            if (nRow2 < nScrY)
                bOut = true;

            nLastX = nScrX + aViewData.VisibleCellsX( eHWhich ) + 1;
            nLastY = nScrY + aViewData.VisibleCellsY( eVWhich ) + 1;
        }

        if (nCol1 > nLastX)
            bOut = true;
        if (nCol2 > nLastX)
            nCol2 = nLastX;
        if (nRow1 > nLastY)
            bOut = true;
        if (nRow2 > nLastY)
            nRow2 = nLastY;

        if (bOut)
            continue;

        bool bLayoutRTL = aViewData.GetDocument().IsLayoutRTL( aViewData.GetTabNo() );
        tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

        Point aStart = aViewData.GetScrPos( nCol1, nRow1, static_cast<ScSplitPos>(i) );
        Point aEnd   = aViewData.GetScrPos( nCol2+1, nRow2+1, static_cast<ScSplitPos>(i) );
        if ( eMode == ScUpdateMode::All )
        {
            if (bIsTiledRendering)
            {
                // When a cell content is deleted we have no clue about
                // the width of the embedded text.
                // Anyway, clients will ask only for tiles that overlaps
                // the visible area.
                // Remember that wsd expects int and that aEnd.X() is
                // in pixels and will be converted in twips, before performing
                // the lok callback, so we need to avoid that an overflow occurs.
                aEnd.setX( bLayoutRTL ? 0 : std::numeric_limits<int>::max() / 1000 );
            }
            else
            {
                aEnd.setX( bLayoutRTL ? 0 : pGridWin[i]->GetOutputSizePixel().Width() );
            }
        }
        aEnd.AdjustX( -nLayoutSign );
        aEnd.AdjustY( -1 );

        // #i85232# include area below cells (could be done in GetScrPos?)
        if ( eMode == ScUpdateMode::All && nRow2 >= rDoc.MaxRow() && !bIsTiledRendering )
            aEnd.setY( pGridWin[i]->GetOutputSizePixel().Height() );

        aStart.AdjustX( -nLayoutSign );      // include change marks
        aStart.AdjustY( -1 );

        bool bMarkClipped = aViewData.GetOptions().GetOption( VOPT_CLIPMARKS );
        if (bMarkClipped)
        {
            // ScColumn::IsEmptyBlock has to be optimized for this
            //  (switch to Search() )
            //!if ( nCol1 > 0 && !aViewData.GetDocument()->IsBlockEmpty(
            //!                     aViewData.GetTabNo(),
            //!                     0, nRow1, nCol1-1, nRow2 ) )
            tools::Long nMarkPixel = static_cast<tools::Long>( SC_CLIPMARK_SIZE * aViewData.GetPPTX() );
            aStart.AdjustX( -(nMarkPixel * nLayoutSign) );
        }

        pGridWin[i]->Invalidate( pGridWin[i]->PixelToLogic( tools::Rectangle( aStart,aEnd ) ) );
    }

    // #i79909# Calling UpdateAllOverlays here isn't necessary and would lead to overlay calls from a timer,
    // with a wrong MapMode if editing in a cell (reference input).
    // #i80499# Overlays need updates in a lot of cases, e.g. changing row/column size,
    // or showing/hiding outlines. TODO: selections in inactive windows are vanishing.
    // #i84689# With relative conditional formats, PaintArea may be called often (for each changed cell),
    // so UpdateAllOverlays was moved to ScTabViewShell::Notify and is called only if PaintPartFlags::Left/PaintPartFlags::Top
    // is set (width or height changed).
}

void ScTabView::PaintRangeFinderEntry (const ScRangeFindData* pData, const SCTAB nTab)
{
    ScRange aRef = pData->aRef;
    aRef.PutInOrder();                 // PutInOrder for the queries below

    if ( aRef.aStart == aRef.aEnd )     //! ignore sheet?
        aViewData.GetDocument().ExtendMerge(aRef);

    if (aRef.aStart.Tab() < nTab || aRef.aEnd.Tab() > nTab)
        return;

    SCCOL nCol1 = aRef.aStart.Col();
    SCROW nRow1 = aRef.aStart.Row();
    SCCOL nCol2 = aRef.aEnd.Col();
    SCROW nRow2 = aRef.aEnd.Row();

    //  remove -> repaint
    //  ScUpdateMode::Marks: Invalidate, nothing until end of row

    bool bHiddenEdge = false;
    SCROW nTmp;
    ScDocument& rDoc = aViewData.GetDocument();
    while ( nCol1 > 0 && rDoc.ColHidden(nCol1, nTab) )
    {
        --nCol1;
        bHiddenEdge = true;
    }
    while ( nCol2 < rDoc.MaxCol() && rDoc.ColHidden(nCol2, nTab) )
    {
        ++nCol2;
        bHiddenEdge = true;
    }
    nTmp = rDoc.LastVisibleRow(0, nRow1, nTab);
    if (!rDoc.ValidRow(nTmp))
        nTmp = 0;
    if (nTmp < nRow1)
    {
        nRow1 = nTmp;
        bHiddenEdge = true;
    }
    nTmp = rDoc.FirstVisibleRow(nRow2, rDoc.MaxRow(), nTab);
    if (!rDoc.ValidRow(nTmp))
        nTmp = rDoc.MaxRow();
    if (nTmp > nRow2)
    {
        nRow2 = nTmp;
        bHiddenEdge = true;
    }

    if ( nCol2 - nCol1 > 1 && nRow2 - nRow1 > 1 && !bHiddenEdge )
    {
        // only along the edges
        PaintArea( nCol1, nRow1, nCol2, nRow1, ScUpdateMode::Marks );
        PaintArea( nCol1, nRow1+1, nCol1, nRow2-1, ScUpdateMode::Marks );
        PaintArea( nCol2, nRow1+1, nCol2, nRow2-1, ScUpdateMode::Marks );
        PaintArea( nCol1, nRow2, nCol2, nRow2, ScUpdateMode::Marks );
    }
    else    // all in one
        PaintArea( nCol1, nRow1, nCol2, nRow2, ScUpdateMode::Marks );
}

void ScTabView::PaintRangeFinder( tools::Long nNumber )
{
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( aViewData.GetViewShell() );
    if (!pHdl)
        return;

    ScRangeFindList* pRangeFinder = pHdl->GetRangeFindList();
    if ( !(pRangeFinder && pRangeFinder->GetDocName() == aViewData.GetDocShell()->GetTitle()) )
        return;

    SCTAB nTab = aViewData.GetTabNo();
    sal_uInt16 nCount = static_cast<sal_uInt16>(pRangeFinder->Count());

    if (nNumber < 0)
    {
        for (sal_uInt16 i=0; i<nCount; i++)
            PaintRangeFinderEntry(&pRangeFinder->GetObject(i),nTab);
    }
    else
    {
        sal_uInt16 idx = nNumber;
        if (idx < nCount)
            PaintRangeFinderEntry(&pRangeFinder->GetObject(idx),nTab);
    }
}

// for chart data selection

void ScTabView::AddHighlightRange( const ScRange& rRange, const Color& rColor )
{
    maHighlightRanges.emplace_back( rRange, rColor );

    SCTAB nTab = aViewData.GetTabNo();
    if ( nTab >= rRange.aStart.Tab() && nTab <= rRange.aEnd.Tab() )
        PaintArea( rRange.aStart.Col(), rRange.aStart.Row(),
                    rRange.aEnd.Col(), rRange.aEnd.Row(), ScUpdateMode::Marks );
}

void ScTabView::ClearHighlightRanges()
{
    SCTAB nTab = aViewData.GetTabNo();
    for (ScHighlightEntry const & rEntry : maHighlightRanges)
    {
        ScRange aRange = rEntry.aRef;
        if ( nTab >= aRange.aStart.Tab() && nTab <= aRange.aEnd.Tab() )
            PaintArea( aRange.aStart.Col(), aRange.aStart.Row(),
                       aRange.aEnd.Col(), aRange.aEnd.Row(), ScUpdateMode::Marks );
    }

    maHighlightRanges.clear();
}

void ScTabView::DoChartSelection(
    const uno::Sequence< chart2::data::HighlightedRange > & rHilightRanges )
{
    ClearHighlightRanges();
    const sal_Unicode sep = ::formula::FormulaCompiler::GetNativeSymbolChar(ocSep);
    size_t nSize = 0;
    size_t nIndex = 0;
    std::vector<ReferenceMark> aReferenceMarks( nSize );

    for (chart2::data::HighlightedRange const & rHighlightedRange : rHilightRanges)
    {
        Color aSelColor(ColorTransparency, rHighlightedRange.PreferredColor);
        ScRangeList aRangeList;
        ScDocument& rDoc = aViewData.GetDocShell()->GetDocument();
        if( ScRangeStringConverter::GetRangeListFromString(
                aRangeList, rHighlightedRange.RangeRepresentation, rDoc, rDoc.GetAddressConvention(), sep ))
        {
            size_t nListSize = aRangeList.size();
            nSize += nListSize;
            aReferenceMarks.resize(nSize);

            for ( size_t j = 0; j < nListSize; ++j )
            {
                ScRange& p = aRangeList[j];
                ScRange aTargetRange;
                if( rHighlightedRange.Index == - 1 )
                {
                    aTargetRange = p;
                    AddHighlightRange( aTargetRange, aSelColor );
                }
                else
                {
                    aTargetRange = lcl_getSubRangeByIndex( p, rHighlightedRange.Index );
                    AddHighlightRange( aTargetRange, aSelColor );
                }

                if ( comphelper::LibreOfficeKit::isActive() && aViewData.GetViewShell() )
                {
                    aTargetRange.PutInOrder();

                    tools::Long nX1 = aTargetRange.aStart.Col();
                    tools::Long nX2 = aTargetRange.aEnd.Col();
                    tools::Long nY1 = aTargetRange.aStart.Row();
                    tools::Long nY2 = aTargetRange.aEnd.Row();
                    tools::Long nTab = aTargetRange.aStart.Tab();

                    aReferenceMarks[nIndex++] = ScInputHandler::GetReferenceMark( aViewData, aViewData.GetDocShell(),
                                                                            nX1, nX2, nY1, nY2,
                                                                            nTab, aSelColor );
                }
            }
        }
    }

    if ( comphelper::LibreOfficeKit::isActive() && aViewData.GetViewShell() )
        ScInputHandler::SendReferenceMarks( aViewData.GetViewShell(), aReferenceMarks );
}

void ScTabView::DoDPFieldPopup(std::u16string_view rPivotTableName, sal_Int32 nDimensionIndex, Point aPoint, Size aSize)
{
    ScDocument& rDocument = aViewData.GetDocShell()->GetDocument();
    ScGridWindow* pWin = pGridWin[aViewData.GetActivePart()].get();

    if (!pWin)
        return;

    ScDPCollection* pDPCollection = rDocument.GetDPCollection();
    ScDPObject* pDPObject = pDPCollection->GetByName(rPivotTableName);
    if (!pDPObject)
        return;

    pDPObject->BuildAllDimensionMembers();

    Point aScreenPoint = pWin->OutputToScreenPixel(pWin->LogicToPixel(aPoint));
    Size aScreenSize = pWin->LogicToPixel(aSize);

    pWin->DPLaunchFieldPopupMenu(aScreenPoint, aScreenSize, nDimensionIndex, pDPObject);
}

//  PaintGrid - repaint data range

void ScTabView::PaintGrid()
{
    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if (pGridWin[i] && pGridWin[i]->IsVisible())
            pGridWin[i]->Invalidate();
    }
}

//  PaintTop - repaint top control elements

void ScTabView::PaintTop()
{
    for (sal_uInt16 i = 0; i < 2; i++)
    {
        if (pColBar[i])
            pColBar[i]->Invalidate();
        if (pColOutline[i])
            pColOutline[i]->Invalidate();
    }
}

void ScTabView::CreateAnchorHandles(SdrHdlList& rHdl, const ScAddress& rAddress)
{
    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if(pGridWin[i] && pGridWin[i]->IsVisible())
            pGridWin[i]->CreateAnchorHandle(rHdl, rAddress);
    }
}

void ScTabView::PaintTopArea( SCCOL nStartCol, SCCOL nEndCol )
{
        // pixel position of the left edge

    if ( nStartCol < aViewData.GetPosX(SC_SPLIT_LEFT) ||
         nStartCol < aViewData.GetPosX(SC_SPLIT_RIGHT) )
        aViewData.RecalcPixPos();

        // adjust freeze (UpdateFixX resets HSplitPos)

    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX && nStartCol < aViewData.GetFixPosX() )
        if (aViewData.UpdateFixX())
            RepeatResize();

        // paint

    if (nStartCol>0)
        --nStartCol;                //! general ?

    ScDocument& rDoc = aViewData.GetDocument();
    bool bLayoutRTL = rDoc.IsLayoutRTL( aViewData.GetTabNo() );
    tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

    for (sal_uInt16 i = 0; i < 2; i++)
    {
        ScHSplitPos eWhich = ScHSplitPos(i);
        if (pColBar[eWhich])
        {
            Size aWinSize = pColBar[eWhich]->GetSizePixel();
            tools::Long nStartX = aViewData.GetScrPos( nStartCol, 0, eWhich ).X();
            tools::Long nEndX;
            if (nEndCol >= rDoc.MaxCol())
                nEndX = bLayoutRTL ? 0 : ( aWinSize.Width()-1 );
            else
                nEndX = aViewData.GetScrPos( nEndCol+1, 0, eWhich ).X() - nLayoutSign;
            if (nStartX > nEndX)
                std::swap(nStartX, nEndX);
            pColBar[eWhich]->Invalidate(
                    tools::Rectangle( nStartX, 0, nEndX, aWinSize.Height()-1 ) );
        }
        if (pColOutline[eWhich])
            pColOutline[eWhich]->Invalidate();
    }
}

//  PaintLeft - repaint left control elements

void ScTabView::PaintLeft()
{
    for (sal_uInt16 i = 0; i < 2; i++)
    {
        if (pRowBar[i])
            pRowBar[i]->Invalidate();
        if (pRowOutline[i])
            pRowOutline[i]->Invalidate();
    }
}

void ScTabView::PaintLeftArea( SCROW nStartRow, SCROW nEndRow )
{
        // pixel position of the upper edge

    if ( nStartRow < aViewData.GetPosY(SC_SPLIT_TOP) ||
         nStartRow < aViewData.GetPosY(SC_SPLIT_BOTTOM) )
        aViewData.RecalcPixPos();

        // adjust freeze (UpdateFixY reset VSplitPos)

    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX && nStartRow < aViewData.GetFixPosY() )
        if (aViewData.UpdateFixY())
            RepeatResize();

        // paint

    if (nStartRow>0)
        --nStartRow;

    ScDocument& rDoc = aViewData.GetDocument();
    for (sal_uInt16 i = 0; i < 2; i++)
    {
        ScVSplitPos eWhich = ScVSplitPos(i);
        if (pRowBar[eWhich])
        {
            Size aWinSize = pRowBar[eWhich]->GetSizePixel();
            tools::Long nStartY = aViewData.GetScrPos( 0, nStartRow, eWhich ).Y();
            tools::Long nEndY;
            if (nEndRow >= rDoc.MaxRow())
                nEndY = aWinSize.Height() - 1;
            else
                nEndY = aViewData.GetScrPos( 0, nEndRow+1, eWhich ).Y() - 1;
            if (nStartY > nEndY)
                std::swap(nStartY, nEndY);
            pRowBar[eWhich]->Invalidate(
                    tools::Rectangle( 0, nStartY, aWinSize.Width()-1, nEndY ) );
        }
        if (pRowOutline[eWhich])
            pRowOutline[eWhich]->Invalidate();
    }
}

bool ScTabView::PaintExtras()
{
    bool bRet = false;
    ScDocument& rDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();
    if (!rDoc.HasTable(nTab))                  // sheet is deleted?
    {
        SCTAB nCount = rDoc.GetTableCount();
        aViewData.SetTabNo(nCount-1);
        bRet = true;
    }
    pTabControl->UpdateStatus();                        // true = active
    return bRet;
}

void ScTabView::RecalcPPT()
{
    //  called after changes that require the PPT values to be recalculated
    //  (currently from detective operations)

    double nOldX = aViewData.GetPPTX();
    double nOldY = aViewData.GetPPTY();

    aViewData.RefreshZoom();                            // pre-calculate new PPT values

    bool bChangedX = ( aViewData.GetPPTX() != nOldX );
    bool bChangedY = ( aViewData.GetPPTY() != nOldY );
    if ( !(bChangedX || bChangedY) )
        return;

    //  call view SetZoom (including draw scale, split update etc)
    //  and paint only if values changed

    Fraction aZoomX = aViewData.GetZoomX();
    Fraction aZoomY = aViewData.GetZoomY();
    SetZoom( aZoomX, aZoomY, false );

    PaintGrid();
    if (bChangedX)
        PaintTop();
    if (bChangedY)
        PaintLeft();
}

void ScTabView::ActivateView( bool bActivate, bool bFirst )
{
    if ( bActivate == aViewData.IsActive() && !bFirst )
    {
        // no assertion anymore - occurs when previously in Drag&Drop switching over
        // to another document
        return;
    }

    // is only called for MDI-(De)Activate
    // aViewData.Activate behind due to cursor show for KillEditView
    // don't delete selection - if Activate(false) is set in ViewData,
    // then the selection is not displayed

    if (!bActivate)
    {
        ScModule* pScMod = SC_MOD();
        bool bRefMode = pScMod->IsFormulaMode();

            // don't cancel reference input, to allow reference
            // to other document

        if (!bRefMode)
        {
            // pass view to GetInputHdl, this view may not be current anymore
            ScInputHandler* pHdl = SC_MOD()->GetInputHdl(aViewData.GetViewShell());
            if (pHdl)
                pHdl->EnterHandler();
        }
    }

    PaintExtras();

    aViewData.Activate(bActivate);

    PaintBlock(false);                  // repaint, selection after active status

    if (!bActivate)
        HideAllCursors();               // Cursor
    else if (!bFirst)
        ShowAllCursors();

    if (bActivate)
    {
        if ( bFirst )
        {
            ScSplitPos eWin = aViewData.GetActivePart();
            OSL_ENSURE( pGridWin[eWin], "Corrupted document, not all SplitPos in GridWin" );
            if ( !pGridWin[eWin] )
            {
                eWin = SC_SPLIT_BOTTOMLEFT;
                if ( !pGridWin[eWin] )
                {
                    short i;
                    for ( i=0; i<4; i++ )
                    {
                        if ( pGridWin[i] )
                        {
                            eWin = static_cast<ScSplitPos>(i);
                            break;  // for
                        }
                    }
                    OSL_ENSURE( i<4, "and BOOM" );
                }
                aViewData.SetActivePart( eWin );
            }
        }
        // do not call GrabFocus from here!
        // if the document is processed, then Sfx calls GrabFocus in the window of the shell.
        // if it is a mail body for instance, then it can't get the focus
        UpdateInputContext();
    }
    else
        pGridWin[aViewData.GetActivePart()]->ClickExtern();
}

void ScTabView::ActivatePart( ScSplitPos eWhich )
{
    ScSplitPos eOld = aViewData.GetActivePart();
    if ( eOld == eWhich )
        return;

    bInActivatePart = true;

    bool bRefMode = SC_MOD()->IsFormulaMode();

    //  the HasEditView call during SetCursor would fail otherwise
    if ( aViewData.HasEditView(eOld) && !bRefMode )
        UpdateInputLine();

    ScHSplitPos eOldH = WhichH(eOld);
    ScVSplitPos eOldV = WhichV(eOld);
    ScHSplitPos eNewH = WhichH(eWhich);
    ScVSplitPos eNewV = WhichV(eWhich);
    bool bTopCap  = pColBar[eOldH] && pColBar[eOldH]->IsMouseCaptured();
    bool bLeftCap = pRowBar[eOldV] && pRowBar[eOldV]->IsMouseCaptured();

    bool bFocus = pGridWin[eOld]->HasFocus();
    bool bCapture = pGridWin[eOld]->IsMouseCaptured();
    if (bCapture)
        pGridWin[eOld]->ReleaseMouse();
    pGridWin[eOld]->ClickExtern();
    pGridWin[eOld]->HideCursor();
    pGridWin[eWhich]->HideCursor();
    aViewData.SetActivePart( eWhich );

    ScTabViewShell* pShell = aViewData.GetViewShell();
    pShell->WindowChanged();

    pSelEngine->SetWindow(pGridWin[eWhich]);
    pSelEngine->SetWhich(eWhich);
    pSelEngine->SetVisibleArea( tools::Rectangle(Point(), pGridWin[eWhich]->GetOutputSizePixel()) );

    pGridWin[eOld]->MoveMouseStatus(*pGridWin[eWhich]);

    if ( bCapture || pGridWin[eWhich]->IsMouseCaptured() )
    {
        // tracking instead of CaptureMouse, so it can be cancelled cleanly
        // (SelectionEngine calls CaptureMouse for SetWindow)
        //! someday SelectionEngine itself should call StartTracking!?!
        pGridWin[eWhich]->ReleaseMouse();
        pGridWin[eWhich]->StartTracking();
    }

    if ( bTopCap && pColBar[eNewH] )
    {
        pColBar[eOldH]->SetIgnoreMove(true);
        pColBar[eNewH]->SetIgnoreMove(false);
        pHdrSelEng->SetWindow( pColBar[eNewH] );
        tools::Long nWidth = pColBar[eNewH]->GetOutputSizePixel().Width();
        pHdrSelEng->SetVisibleArea( tools::Rectangle( 0, LONG_MIN, nWidth-1, LONG_MAX ) );
        pColBar[eNewH]->CaptureMouse();
    }
    if ( bLeftCap && pRowBar[eNewV] )
    {
        pRowBar[eOldV]->SetIgnoreMove(true);
        pRowBar[eNewV]->SetIgnoreMove(false);
        pHdrSelEng->SetWindow( pRowBar[eNewV] );
        tools::Long nHeight = pRowBar[eNewV]->GetOutputSizePixel().Height();
        pHdrSelEng->SetVisibleArea( tools::Rectangle( LONG_MIN, 0, LONG_MAX, nHeight-1 ) );
        pRowBar[eNewV]->CaptureMouse();
    }
    aHdrFunc.SetWhich(eWhich);

    pGridWin[eOld]->ShowCursor();
    pGridWin[eWhich]->ShowCursor();

    SfxInPlaceClient* pClient = aViewData.GetViewShell()->GetIPClient();
    bool bOleActive = ( pClient && pClient->IsObjectInPlaceActive() );

    // don't switch ViewShell's active window during RefInput, because the focus
    // might change, and subsequent SetReference calls wouldn't find the right EditView
    if ( !bRefMode && !bOleActive )
        aViewData.GetViewShell()->SetWindow( pGridWin[eWhich] );

    if ( bFocus && !aViewData.IsAnyFillMode() && !bRefMode )
    {
        // GrabFocus only if previously the other GridWindow had the focus
        // (for instance due to search and replace)
        pGridWin[eWhich]->GrabFocus();
    }

    bInActivatePart = false;
}

void ScTabView::HideListBox()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
    {
        if (pWin)
            pWin->ClickExtern();
    }
}

void ScTabView::UpdateInputContext()
{
    ScGridWindow* pWin = pGridWin[aViewData.GetActivePart()].get();
    if (pWin)
        pWin->UpdateInputContext();

    if (pTabControl)
        pTabControl->UpdateInputContext();
}

// GetGridWidth - width of an output range (for ViewData)

tools::Long ScTabView::GetGridWidth( ScHSplitPos eWhich )
{
    // at present only the size of the current pane is synchronized with
    // the size of the visible area in Online;
    // as a workaround we use the same width for all panes independently
    // from the eWhich value
    if (comphelper::LibreOfficeKit::isActive())
    {
        ScGridWindow* pGridWindow = aViewData.GetActiveWin();
        if (pGridWindow)
            return pGridWindow->GetSizePixel().Width();
    }

    ScSplitPos eGridWhich = ( eWhich == SC_SPLIT_LEFT ) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;
    if (pGridWin[eGridWhich])
        return pGridWin[eGridWhich]->GetSizePixel().Width();
    else
        return 0;
}

// GetGridHeight - height of an output range (for ViewData)

tools::Long ScTabView::GetGridHeight( ScVSplitPos eWhich )
{
    // at present only the size of the current pane is synchronized with
    // the size of the visible area in Online;
    // as a workaround we use the same height for all panes independently
    // from the eWhich value
    if (comphelper::LibreOfficeKit::isActive())
    {
        ScGridWindow* pGridWindow = aViewData.GetActiveWin();
        if (pGridWindow)
            return pGridWindow->GetSizePixel().Height();
    }

    ScSplitPos eGridWhich = ( eWhich == SC_SPLIT_TOP ) ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT;
    if (pGridWin[eGridWhich])
        return pGridWin[eGridWhich]->GetSizePixel().Height();
    else
        return 0;
}

void ScTabView::UpdateInputLine()
{
    SC_MOD()->InputEnterHandler();
}

void ScTabView::ZoomChanged()
{
    ScInputHandler* pHdl = SC_MOD()->GetInputHdl(aViewData.GetViewShell());
    if (pHdl)
        pHdl->SetRefScale( aViewData.GetZoomX(), aViewData.GetZoomY() );

    UpdateFixPos();

    UpdateScrollBars();

    // VisArea...
    // AW: Discussed with NN if there is a reason that new map mode was only set for one window,
    // but is not. Setting only on one window causes the first repaint to have the old mapMode
    // in three of four views, so the overlay will save the wrong content e.g. when zooming out.
    // Changing to setting map mode at all windows.

    for (sal_uInt32 i = 0; i < 4; i++)
    {
        if (pGridWin[i])
            pGridWin[i]->SetMapMode(pGridWin[i]->GetDrawMapMode());
    }

    SetNewVisArea();

    InterpretVisible();     // have everything calculated before painting

    SfxBindings& rBindings = aViewData.GetBindings();
    rBindings.Invalidate( SID_ATTR_ZOOM );
    rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );
    rBindings.Invalidate(SID_ZOOM_IN);
    rBindings.Invalidate(SID_ZOOM_OUT);

    HideNoteMarker();

    // To not change too much, use pWin here
    ScGridWindow* pWin = pGridWin[aViewData.GetActivePart()].get();

    if ( pWin && aViewData.HasEditView( aViewData.GetActivePart() ) )
    {
        // flush OverlayManager before changing the MapMode
        pWin->flushOverlayManager();

        // make sure the EditView's position and size are updated
        // with the right (logic, not drawing) MapMode
        pWin->SetMapMode( aViewData.GetLogicMode() );
        UpdateEditView();
    }
}

void ScTabView::CheckNeedsRepaint()
{
    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if (pGridWin[i] && pGridWin[i]->IsVisible())
            pGridWin[i]->CheckNeedsRepaint();
    }
}

bool ScTabView::NeedsRepaint()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
    {
        if (pWin && pWin->IsVisible() && pWin->NeedsRepaint())
            return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
