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

#include <starmath.hrc>
#include <helpids.h>

#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/settings.hxx>

#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/stritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <osl/diagnose.h>

#include <edit.hxx>
#include <smmod.hxx>
#include <view.hxx>
#include <document.hxx>
#include <cfgitem.hxx>
#include "accessibility.hxx"

#define SCROLL_LINE         24


using namespace com::sun::star::accessibility;
using namespace com::sun::star;


void SmGetLeftSelectionPart(const ESelection &rSel,
                            sal_Int32 &nPara, sal_uInt16 &nPos)
    // returns paragraph number and position of the selections left part
{
    // compare start and end of selection and use the one that comes first
    if (    rSel.nStartPara <  rSel.nEndPara
        ||  (rSel.nStartPara == rSel.nEndPara  &&  rSel.nStartPos < rSel.nEndPos) )
    {   nPara = rSel.nStartPara;
        nPos  = rSel.nStartPos;
    }
    else
    {   nPara = rSel.nEndPara;
        nPos  = rSel.nEndPos;
    }
}

SmEditTextWindow::SmEditTextWindow(SmEditWindow& rEditWindow)
    : mrEditWindow(rEditWindow)
    , aModifyIdle("SmEditWindow ModifyIdle")
    , aCursorMoveIdle("SmEditWindow CursorMoveIdle")
{
    SetAcceptsTab(true);

    aModifyIdle.SetInvokeHandler(LINK(this, SmEditTextWindow, ModifyTimerHdl));
    aModifyIdle.SetPriority(TaskPriority::LOWEST);

    if (!SmViewShell::IsInlineEditEnabled())
    {
        aCursorMoveIdle.SetInvokeHandler(LINK(this, SmEditTextWindow, CursorMoveTimerHdl));
        aCursorMoveIdle.SetPriority(TaskPriority::LOWEST);
    }
}

SmEditTextWindow::~SmEditTextWindow()
{
    aModifyIdle.Stop();
    StartCursorMove();
}

EditEngine* SmEditTextWindow::GetEditEngine() const
{
    SmDocShell *pDoc = mrEditWindow.GetDoc();
    assert(pDoc);
    return &pDoc->GetEditEngine();
}

void SmEditTextWindow::EditViewScrollStateChange()
{
    mrEditWindow.SetScrollBarRanges();
}

void SmEditTextWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);

    EnableRTL(false);

    EditEngine* pEditEngine = GetEditEngine();

    m_xEditView.reset(new EditView(pEditEngine, nullptr));
    m_xEditView->setEditViewCallbacks(this);

    pEditEngine->InsertView(m_xEditView.get());

    m_xEditView->SetOutputArea(mrEditWindow.AdjustScrollBars());

    pDrawingArea->set_cursor(PointerStyle::Text);

    pEditEngine->SetStatusEventHdl(LINK(this, SmEditTextWindow, EditStatusHdl));

    InitAccessible();
}

SmEditWindow::SmEditWindow(SmCmdBoxWindow &rMyCmdBoxWin)
    : InterimItemWindow(&rMyCmdBoxWin, "modules/smath/ui/editwindow.ui", "EditWindow")
    , DropTargetHelper(this)
    , rCmdBox(rMyCmdBoxWin)
    , mxScrolledWindow(m_xBuilder->weld_scrolled_window("scrolledwindow", true))
{
    set_id("math_edit");
    SetHelpId(HID_SMA_COMMAND_WIN_EDIT);
    SetMapMode(MapMode(MapUnit::MapPixel));

    // Even RTL languages don't use RTL for math
    EnableRTL( false );

    // compare DataChanged
    SetBackground( GetSettings().GetStyleSettings().GetWindowColor() );

    mxScrolledWindow->connect_hadjustment_changed(LINK(this, SmEditWindow, ScrollHdl));
    mxScrolledWindow->connect_vadjustment_changed(LINK(this, SmEditWindow, ScrollHdl));

    CreateEditView();

    // if not called explicitly the this edit window within the
    // command window will just show an empty gray panel.
    Show();
}

SmEditWindow::~SmEditWindow()
{
    disposeOnce();
}

void SmEditWindow::dispose()
{
    DeleteEditView();

    mxScrolledWindow.reset();

    DropTargetHelper::dispose();
    InterimItemWindow::dispose();
}

void SmEditTextWindow::StartCursorMove()
{
    if (!SmViewShell::IsInlineEditEnabled())
        aCursorMoveIdle.Stop();
}

void SmEditWindow::InvalidateSlots()
{
    SfxBindings& rBind = GetView()->GetViewFrame()->GetBindings();
    rBind.Invalidate(SID_COPY);
    rBind.Invalidate(SID_CUT);
    rBind.Invalidate(SID_DELETE);
}

SmViewShell * SmEditWindow::GetView()
{
    return rCmdBox.GetView();
}

SmDocShell * SmEditWindow::GetDoc()
{
    SmViewShell *pView = rCmdBox.GetView();
    return pView ? pView->GetDoc() : nullptr;
}

EditView * SmEditWindow::GetEditView() const
{
    return mxTextControl ? mxTextControl->GetEditView() : nullptr;
}

EditEngine * SmEditWindow::GetEditEngine()
{
    if (SmDocShell *pDoc = GetDoc())
        return &pDoc->GetEditEngine();
    return nullptr;
}

void SmEditWindow::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    rRenderContext.SetBackground(rStyleSettings.GetWindowColor());
}

void SmEditWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    InterimItemWindow::DataChanged( rDCEvt );

    if (!((rDCEvt.GetType() == DataChangedEventType::FONTS) ||
          (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
          ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
           (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))))
        return;

    EditEngine *pEditEngine = GetEditEngine();
    SmDocShell *pDoc = GetDoc();

    if (pEditEngine && pDoc)
    {
        //!
        //! see also SmDocShell::GetEditEngine() !
        //!
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        pDoc->UpdateEditEngineDefaultFonts(rStyleSettings.GetFieldTextColor());
        pEditEngine->SetBackgroundColor(rStyleSettings.GetFieldColor());
        pEditEngine->SetDefTab(sal_uInt16(GetTextWidth("XXXX")));

        // forces new settings to be used
        // unfortunately this resets the whole edit engine
        // thus we need to save at least the text
        OUString aTxt( pEditEngine->GetText() );
        pEditEngine->Clear();   //incorrect font size
        pEditEngine->SetText( aTxt );

        AdjustScrollBars();
        Resize();
    }
}

IMPL_LINK_NOARG(SmEditTextWindow, ModifyTimerHdl, Timer *, void)
{
    UpdateStatus(false);
    aModifyIdle.Stop();
}

IMPL_LINK_NOARG(SmEditTextWindow, CursorMoveTimerHdl, Timer *, void)
    // every once in a while check cursor position (selection) of edit
    // window and if it has changed (try to) set the formula-cursor
    // according to that.
{
    if (SmViewShell::IsInlineEditEnabled())
        return;

    ESelection aNewSelection(GetSelection());

    if (aNewSelection != aOldSelection)
    {
        if (SmViewShell *pViewSh = mrEditWindow.GetView())
        {
            // get row and column to look for
            sal_Int32  nRow;
            sal_uInt16 nCol;
            SmGetLeftSelectionPart(aNewSelection, nRow, nCol);
            pViewSh->GetGraphicWindow().SetCursorPos(static_cast<sal_uInt16>(nRow), nCol);
            aOldSelection = aNewSelection;
        }
    }
    aCursorMoveIdle.Stop();
}

void SmEditWindow::Resize()
{
    InterimItemWindow::Resize();

    if (EditView* pEditView = GetEditView())
    {
        // Resizes the edit engine to adjust to the size of the output area
        const Size aSize( pEditView->GetOutputArea().GetSize() );
        pEditView->GetEditEngine()->SetPaperSize(aSize);

        pEditView->SetOutputArea(AdjustScrollBars());
        pEditView->ShowCursor();

        OSL_ENSURE( pEditView->GetEditEngine(), "EditEngine missing" );
        const tools::Long nMaxVisAreaStart = pEditView->GetEditEngine()->GetTextHeight() -
                                      pEditView->GetOutputArea().GetHeight();
        if (pEditView->GetVisArea().Top() > nMaxVisAreaStart)
        {
            tools::Rectangle aVisArea(pEditView->GetVisArea() );
            aVisArea.SetTop( std::max<tools::Long>(nMaxVisAreaStart, 0) );
            aVisArea.SetSize(pEditView->GetOutputArea().GetSize());
            pEditView->SetVisArea(aVisArea);
            pEditView->ShowCursor();
        }
        SetScrollBarRanges();
    }
}

bool SmEditTextWindow::MouseButtonUp(const MouseEvent &rEvt)
{
    bool bRet = WeldEditView::MouseButtonUp(rEvt);
    if (!SmViewShell::IsInlineEditEnabled())
        CursorMoveTimerHdl(&aCursorMoveIdle);
    mrEditWindow.InvalidateSlots();
    return bRet;
}

void SmEditWindow::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        GetParent()->ToTop();
        Point aPoint = rCEvt.GetMousePosPixel();
        SmViewShell *pViewSh = GetView();
        if (pViewSh)
            pViewSh->GetViewFrame()->GetDispatcher()->ExecutePopup("edit", this, &aPoint);
        return;
    }

    InterimItemWindow::Command(rCEvt);
}

bool SmEditTextWindow::Command(const CommandEvent& rCEvt)
{
    // no zooming in Command window
    const CommandWheelData* pWData = rCEvt.GetWheelData();
    if (pWData && CommandWheelMode::ZOOM == pWData->GetMode())
        return true;

    //pass alt press/release to parent impl
    if (rCEvt.GetCommand() == CommandEventId::ModKeyChange)
        return false;

    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        // purely for "ExecutePopup" taking a vcl::Window and
        // we assume SmEditTextWindow 0,0 is at SmEditWindow 0,0
        mrEditWindow.Command(rCEvt);
        return false;
    }

    bool bConsumed = WeldEditView::Command(rCEvt);
    if (bConsumed)
        UserPossiblyChangedText();
    return bConsumed;
}

bool SmEditTextWindow::KeyInput(const KeyEvent& rKEvt)
{
    if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
    {
        bool bCallBase = true;
        SfxViewShell* pViewShell = mrEditWindow.GetView();
        if ( dynamic_cast<const SmViewShell *>(pViewShell) )
        {
            // Terminate possible InPlace mode
            bCallBase = !pViewShell->Escape();
        }
        return !bCallBase;
    }

    StartCursorMove();

    bool autoClose = false;
    EditView* pEditView = GetEditView();
    ESelection aSelection = pEditView->GetSelection();
    // as we don't support RTL in Math, we need to swap values from selection when they were done
    // in RTL form
    aSelection.Adjust();
    OUString selected = pEditView->GetEditEngine()->GetText(aSelection);

    // Check is auto close brackets/braces is disabled
    SmModule *pMod = SM_MOD();
    if (pMod && !pMod->GetConfig()->IsAutoCloseBrackets())
        autoClose = false;
    else if (selected.trim() == "<?>")
        autoClose = true;
    else if (selected.isEmpty() && !aSelection.HasRange())
    {
        selected = pEditView->GetEditEngine()->GetText(aSelection.nEndPara);
        if (!selected.isEmpty())
        {
            sal_Int32 index = selected.indexOf("\n", aSelection.nEndPos);
            if (index != -1)
            {
                selected = selected.copy(index, sal_Int32(aSelection.nEndPos-index));
                if (selected.trim().isEmpty())
                    autoClose = true;
            }
            else
            {
                sal_Int32 length = selected.getLength();
                if (aSelection.nEndPos == length)
                    autoClose = true;
                else
                {
                    selected = selected.copy(aSelection.nEndPos);
                    if (selected.trim().isEmpty())
                        autoClose = true;
                }
            }
        }
        else
            autoClose = true;
    }

    bool bConsumed = WeldEditView::KeyInput(rKEvt);
    if (!bConsumed)
    {
        SmViewShell *pView = mrEditWindow.GetView();
        if ( pView && !pView->KeyInput(rKEvt) )
        {
            // F1 (help) leads to the destruction of this
            Flush();
            if ( aModifyIdle.IsActive() )
                aModifyIdle.Stop();
        }
        else
        {
            // SFX has maybe called a slot of the view and thus (because of a hack in SFX)
            // set the focus to the view
            SmViewShell* pVShell = mrEditWindow.GetView();
            if ( pVShell && pVShell->GetGraphicWindow().HasFocus() )
            {
                GrabFocus();
            }
        }
    }
    else
    {
        UserPossiblyChangedText();
    }

    // get the current char of the key event
    sal_Unicode cCharCode = rKEvt.GetCharCode();
    OUString sClose;

    if (cCharCode == '{')
        sClose = "  }";
    else if (cCharCode == '[')
        sClose = "  ]";
    else if (cCharCode == '(')
        sClose = "  )";

    // auto close the current character only when needed
    if (!sClose.isEmpty() && autoClose)
    {
        pEditView->InsertText(sClose);
        // position it at center of brackets
        aSelection.nStartPos += 2;
        aSelection.nEndPos = aSelection.nStartPos;
        pEditView->SetSelection(aSelection);
    }

    mrEditWindow.InvalidateSlots();
    return bConsumed;
}

void SmEditTextWindow::UserPossiblyChangedText()
{
    // have doc-shell modified only for formula input/change and not
    // cursor travelling and such things...
    SmDocShell *pDocShell = mrEditWindow.GetDoc();
    EditEngine *pEditEngine = GetEditEngine();
    if (pDocShell && pEditEngine)
        pDocShell->SetModified(pEditEngine->IsModified());
    aModifyIdle.Start();
}

void SmEditWindow::CreateEditView()
{
    assert(!mxTextControl);

    EditEngine *pEditEngine = GetEditEngine();
    //! pEditEngine may be 0.
    //! For example when the program is used by the document-converter
    if (!pEditEngine)
        return;

    mxTextControl.reset(new SmEditTextWindow(*this));
    mxTextControlWin.reset(new weld::CustomWeld(*m_xBuilder, "editview", *mxTextControl));
    InitControlBase(mxTextControl->GetDrawingArea());

    SetScrollBarRanges();
}

IMPL_LINK_NOARG(SmEditTextWindow, EditStatusHdl, EditStatus&, void)
{
    if (GetEditView())
        mrEditWindow.Resize();
}

IMPL_LINK(SmEditWindow, ScrollHdl, weld::ScrolledWindow&, rScrolledWindow, void)
{
    if (EditView* pEditView = GetEditView())
    {
        pEditView->SetVisArea(tools::Rectangle(
                    Point(rScrolledWindow.hadjustment_get_value(),
                          rScrolledWindow.vadjustment_get_value()),
                    pEditView->GetVisArea().GetSize()));
        pEditView->Invalidate();
    }
}

tools::Rectangle SmEditWindow::AdjustScrollBars()
{
    tools::Rectangle aRect(Point(), GetOutputSizePixel());

    if (mxScrolledWindow)
    {
        const auto nScrollSize = mxScrolledWindow->get_scroll_thickness();
        const auto nMargin = nScrollSize + 2;
        aRect.AdjustRight(-nMargin);
        aRect.AdjustBottom(-nMargin);
    }

    return aRect;
}

void SmEditWindow::SetScrollBarRanges()
{
    EditEngine *pEditEngine = GetEditEngine();
    if (!pEditEngine)
        return;
    if (!mxScrolledWindow)
        return;
    EditView* pEditView = GetEditView();
    if (!pEditView)
        return;

    int nVUpper = pEditEngine->GetTextHeight();
    int nHUpper = pEditEngine->GetPaperSize().Width();
    int nVCurrentDocPos = pEditView->GetVisArea().Top();
    int nHCurrentDocPos = pEditView->GetVisArea().Left();
    const Size aOut(pEditView->GetOutputArea().GetSize());
    int nVStepIncrement = aOut.Height() * 2 / 10;
    int nHStepIncrement = SCROLL_LINE;
    int nVPageIncrement = aOut.Height() * 8 / 10;
    int nHPageIncrement = aOut.Width() * 8 / 10;
    int nVPageSize = aOut.Height();
    int nHPageSize = aOut.Width();

    /* limit the page size to below nUpper because gtk's gtk_scrolled_window_start_deceleration has
       effectively...

       lower = gtk_adjustment_get_lower
       upper = gtk_adjustment_get_upper - gtk_adjustment_get_page_size

       and requires that upper > lower or the deceleration animation never ends
    */
    nVPageSize = std::min(nVPageSize, nVUpper);
    nHPageSize = std::min(nHPageSize, nHUpper);

    mxScrolledWindow->vadjustment_configure(nVCurrentDocPos, 0, nVUpper,
                                            nVStepIncrement, nVPageIncrement, nVPageSize);
    mxScrolledWindow->hadjustment_configure(nHCurrentDocPos, 0, nHUpper,
                                            nHStepIncrement, nHPageIncrement, nHPageSize);
}

OUString SmEditWindow::GetText() const
{
    OUString aText;
    EditEngine *pEditEngine = const_cast< SmEditWindow* >(this)->GetEditEngine();
    OSL_ENSURE( pEditEngine, "EditEngine missing" );
    if (pEditEngine)
        aText = pEditEngine->GetText();
    return aText;
}

void SmEditWindow::SetText(const OUString& rText)
{
    if (!mxTextControl)
        return;
    mxTextControl->SetText(rText);
}

void SmEditTextWindow::SetText(const OUString& rText)
{
    EditEngine *pEditEngine = GetEditEngine();
    OSL_ENSURE( pEditEngine, "EditEngine missing" );
    if (!pEditEngine || pEditEngine->IsModified())
        return;

    EditView* pEditView = GetEditView();
    ESelection eSelection = pEditView->GetSelection();

    pEditEngine->SetText(rText);
    pEditEngine->ClearModifyFlag();

    // Restarting the timer here, prevents calling the handlers for other (currently inactive)
    // math tasks
    aModifyIdle.Start();

    pEditView->SetSelection(eSelection);
}

void SmEditTextWindow::GetFocus()
{
    WeldEditView::GetFocus();

    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetStatusEventHdl(LINK(this, SmEditTextWindow, EditStatusHdl));

    //Let SmViewShell know we got focus
    if (mrEditWindow.GetView() && SmViewShell::IsInlineEditEnabled())
        mrEditWindow.GetView()->SetInsertIntoEditWindow(true);
}

void SmEditTextWindow::LoseFocus()
{
    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetStatusEventHdl( Link<EditStatus&,void>() );

    WeldEditView::LoseFocus();
}

bool SmEditWindow::IsAllSelected() const
{
    EditEngine *pEditEngine = const_cast<SmEditWindow *>(this)->GetEditEngine();
    if (!pEditEngine)
        return false;
    EditView* pEditView = GetEditView();
    if (!pEditView)
        return false;
    bool bRes = false;
    ESelection eSelection( pEditView->GetSelection() );
    sal_Int32 nParaCnt = pEditEngine->GetParagraphCount();
    if (!(nParaCnt - 1))
    {
        sal_Int32 nTextLen = pEditEngine->GetText().getLength();
        bRes = !eSelection.nStartPos && (eSelection.nEndPos == nTextLen - 1);
    }
    else
    {
        bRes = !eSelection.nStartPara && (eSelection.nEndPara == nParaCnt - 1);
    }
    return bRes;
}

void SmEditWindow::SelectAll()
{
    if (EditView* pEditView = GetEditView())
    {
        // ALL as last two parameters refers to the end of the text
        pEditView->SetSelection( ESelection( 0, 0, EE_PARA_ALL, EE_TEXTPOS_ALL ) );
    }
}

void SmEditWindow::MarkError(const Point &rPos)
{
    if (EditView* pEditView = GetEditView())
    {
        const sal_uInt16        nCol = sal::static_int_cast< sal_uInt16 >(rPos.X());
        const sal_uInt16        nRow = sal::static_int_cast< sal_uInt16 >(rPos.Y() - 1);

        pEditView->SetSelection(ESelection(nRow, nCol - 1, nRow, nCol));
        GrabFocus();
    }
}

void SmEditWindow::SelNextMark()
{
    if (!mxTextControl)
        return;
    mxTextControl->SelNextMark();
}

// Makes selection to next <?> symbol
void SmEditTextWindow::SelNextMark()
{
    EditEngine *pEditEngine = GetEditEngine();
    if (!pEditEngine)
        return;
    EditView* pEditView = GetEditView();
    if (!pEditView)
        return;

    ESelection eSelection = pEditView->GetSelection();
    sal_Int32 nPos = eSelection.nEndPos;
    sal_Int32 nCounts = pEditEngine->GetParagraphCount();

    while (eSelection.nEndPara < nCounts)
    {
        OUString aText = pEditEngine->GetText(eSelection.nEndPara);
        nPos = aText.indexOf("<?>", nPos);
        if (nPos != -1)
        {
            pEditView->SetSelection(ESelection(
                eSelection.nEndPara, nPos, eSelection.nEndPara, nPos + 3));
            break;
        }

        nPos = 0;
        eSelection.nEndPara++;
    }
}

void SmEditWindow::SelPrevMark()
{
    EditEngine *pEditEngine = GetEditEngine();
    if (!pEditEngine)
        return;
    EditView* pEditView = GetEditView();
    if (!pEditView)
        return;

    ESelection eSelection = pEditView->GetSelection();
    sal_Int32 nPara = eSelection.nStartPara;
    sal_Int32 nMax = eSelection.nStartPos;
    OUString aText(pEditEngine->GetText(nPara));
    static const OUStringLiteral aMark(u"<?>");
    sal_Int32 nPos;

    while ( (nPos = aText.lastIndexOf(aMark, nMax)) < 0 )
    {
        if (--nPara < 0)
            return;
        aText = pEditEngine->GetText(nPara);
        nMax = aText.getLength();
    }
    pEditView->SetSelection(ESelection(nPara, nPos, nPara, nPos + 3));
}

// returns true iff 'rText' contains a mark
static bool HasMark(const OUString& rText)
{
    return rText.indexOf("<?>") != -1;
}

sal_Int8 SmEditWindow::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}

sal_Int8 SmEditWindow::ExecuteDrop( const ExecuteDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}

ESelection SmEditWindow::GetSelection() const
{
    if (mxTextControl)
        return mxTextControl->GetSelection();
    return ESelection();
}

ESelection SmEditTextWindow::GetSelection() const
{
    // pointer may be 0 when reloading a document and the old view
    // was already destroyed
    if (EditView* pEditView = GetEditView())
        return pEditView->GetSelection();
    return ESelection();
}

void SmEditWindow::SetSelection(const ESelection &rSel)
{
    if (EditView* pEditView = GetEditView())
        pEditView->SetSelection(rSel);
    InvalidateSlots();
}

bool SmEditWindow::IsEmpty() const
{
    EditEngine *pEditEngine = const_cast<SmEditWindow *>(this)->GetEditEngine();
    bool bEmpty = ( pEditEngine && pEditEngine->GetTextLen() == 0 );
    return bEmpty;
}

bool SmEditWindow::IsSelected() const
{
    EditView* pEditView = GetEditView();
    return pEditView && pEditView->HasSelection();
}

void SmEditTextWindow::UpdateStatus(bool bSetDocModified)
{
    SmModule *pMod = SM_MOD();
    if (pMod && pMod->GetConfig()->IsAutoRedraw())
        Flush();
    if ( bSetDocModified )
        mrEditWindow.GetDoc()->SetModified();
}

void SmEditWindow::Cut()
{
    if (mxTextControl)
    {
        mxTextControl->Cut();
        mxTextControl->UpdateStatus(true);
    }
}

void SmEditWindow::Copy()
{
    if (mxTextControl)
        mxTextControl->Copy();
}

void SmEditWindow::Paste()
{
    if (mxTextControl)
    {
        mxTextControl->Paste();
        mxTextControl->UpdateStatus(true);
    }
}

void SmEditWindow::Delete()
{
    if (mxTextControl)
    {
        mxTextControl->Delete();
        mxTextControl->UpdateStatus(true);
    }
}

void SmEditWindow::InsertText(const OUString& rText)
{
    if (!mxTextControl)
        return;
    mxTextControl->InsertText(rText);
}

void SmEditWindow::HideScrollbars()
{
    mxScrolledWindow->set_vpolicy(VclPolicyType::NEVER);
    mxScrolledWindow->set_hpolicy(VclPolicyType::NEVER);
}

void SmEditWindow::ShowScrollbars()
{
    mxScrolledWindow->set_vpolicy(VclPolicyType::ALWAYS);
    mxScrolledWindow->set_hpolicy(VclPolicyType::ALWAYS);
}

void SmEditTextWindow::InsertText(const OUString& rText)
{
    EditView* pEditView = GetEditView();
    if (!pEditView)
        return;

    // Note: Insertion of a space in front of commands is done here and
    // in SmEditWindow::InsertCommand.
    ESelection aSelection = pEditView->GetSelection();
    OUString aCurrentFormula = pEditView->GetEditEngine()->GetText();
    sal_Int32 nStartIndex = 0;

    // get the start position (when we get a multi line formula)
    for (sal_Int32 nParaPos = 0; nParaPos < aSelection.nStartPara; nParaPos++)
         nStartIndex = aCurrentFormula.indexOf("\n", nStartIndex) + 1;

    nStartIndex += aSelection.nStartPos;

    // TODO: unify this function with the InsertCommand: The do the same thing for different
    // callers
    OUString string(rText);

    OUString selected(pEditView->GetSelected());
    // if we have text selected, use it in the first placeholder
    if (!selected.isEmpty())
        string = string.replaceFirst("<?>", selected);

    // put a space before a new command if not in the beginning of a line
    if (aSelection.nStartPos > 0 && aCurrentFormula[nStartIndex - 1] != ' ')
        string = " " + string;

    /*
      fdo#65588 -  Elements Dock: Scrollbar moves into input window
      This change "solves" the visual problem. But I don't think so
      this is the best solution.
    */
    mrEditWindow.HideScrollbars();
    pEditView->InsertText(string);
    mrEditWindow.AdjustScrollBars();
    mrEditWindow.ShowScrollbars();

    // Remember start of the selection and move the cursor there afterwards.
    aSelection.nEndPara = aSelection.nStartPara;
    if (HasMark(string))
    {
        aSelection.nEndPos = aSelection.nStartPos;
        pEditView->SetSelection(aSelection);
        SelNextMark();
    }
    else
    {   // set selection after inserted text
        aSelection.nEndPos = aSelection.nStartPos + string.getLength();
        aSelection.nStartPos = aSelection.nEndPos;
        pEditView->SetSelection(aSelection);
    }

    aModifyIdle.Start();
    StartCursorMove();

    GrabFocus();
}

void SmEditTextWindow::Flush()
{
    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine  &&  pEditEngine->IsModified())
    {
        pEditEngine->ClearModifyFlag();
        if (SmViewShell *pViewSh = mrEditWindow.GetView())
        {
            std::unique_ptr<SfxStringItem> pTextToFlush = std::make_unique<SfxStringItem>(SID_TEXT, GetText());
            pViewSh->GetViewFrame()->GetDispatcher()->ExecuteList(
                    SID_TEXT, SfxCallMode::RECORD,
                    { pTextToFlush.get() });
        }
    }
    if (aCursorMoveIdle.IsActive())
    {
        aCursorMoveIdle.Stop();
        CursorMoveTimerHdl(&aCursorMoveIdle);
    }
}

void SmEditWindow::DeleteEditView()
{
    if (EditView* pEditView = GetEditView())
    {
        if (EditEngine* pEditEngine = pEditView->GetEditEngine())
        {
            pEditEngine->SetStatusEventHdl( Link<EditStatus&,void>() );
            pEditEngine->RemoveView(pEditView);
        }
        InitControlBase(nullptr);
        mxTextControlWin.reset();
        mxTextControl.reset();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
