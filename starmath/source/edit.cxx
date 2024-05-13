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
#include <svl/itemset.hxx>
#include <sfx2/viewfrm.hxx>
#include <osl/diagnose.h>
#include <o3tl/string_view.hxx>

#include <edit.hxx>
#include <smmod.hxx>
#include <view.hxx>
#include <document.hxx>
#include <cfgitem.hxx>
#include <smediteng.hxx>

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

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    OutputDevice& rDevice = pDrawingArea->get_ref_device();
    rDevice.SetBackground(aBgColor);

    SetHelpId(HID_SMA_COMMAND_WIN_EDIT);

    EnableRTL(false);

    EditEngine* pEditEngine = GetEditEngine();

    m_xEditView.reset(new EditView(pEditEngine, nullptr));
    m_xEditView->setEditViewCallbacks(this);

    pEditEngine->InsertView(m_xEditView.get());

    m_xEditView->SetOutputArea(mrEditWindow.AdjustScrollBars());

    m_xEditView->SetBackgroundColor(aBgColor);

    pDrawingArea->set_cursor(PointerStyle::Text);

    pEditEngine->SetStatusEventHdl(LINK(this, SmEditTextWindow, EditStatusHdl));

    InitAccessible();

    //Apply zoom to smeditwindow text
    if(GetEditView())
        static_cast<SmEditEngine*>(GetEditEngine())->executeZoom(GetEditView());
}

SmEditWindow::SmEditWindow(SmCmdBoxWindow &rMyCmdBoxWin, weld::Builder& rBuilder)
    : rCmdBox(rMyCmdBoxWin)
    , mxScrolledWindow(rBuilder.weld_scrolled_window(u"scrolledwindow"_ustr, true))
{
    mxScrolledWindow->connect_vadjustment_changed(LINK(this, SmEditWindow, ScrollHdl));

    CreateEditView(rBuilder);
}

SmEditWindow::~SmEditWindow() COVERITY_NOEXCEPT_FALSE
{
    DeleteEditView();
    mxScrolledWindow.reset();
}

weld::Window* SmEditWindow::GetFrameWeld() const
{
    return rCmdBox.GetFrameWeld();
}

void SmEditTextWindow::StartCursorMove()
{
    if (!SmViewShell::IsInlineEditEnabled())
        aCursorMoveIdle.Stop();
}

void SmEditWindow::InvalidateSlots()
{
    GetView()->InvalidateSlots();
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

void SmEditTextWindow::StyleUpdated()
{
    WeldEditView::StyleUpdated();
    EditEngine *pEditEngine = GetEditEngine();
    SmDocShell *pDoc = mrEditWindow.GetDoc();

    if (pEditEngine && pDoc)
    {
        //!
        //! see also SmDocShell::GetEditEngine() !
        //!
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        pDoc->UpdateEditEngineDefaultFonts();
        pEditEngine->SetBackgroundColor(rStyleSettings.GetFieldColor());
        pEditEngine->SetDefTab(sal_uInt16(GetTextWidth(u"XXXX"_ustr)));

        // forces new settings to be used
        // unfortunately this resets the whole edit engine
        // thus we need to save at least the text
        OUString aTxt( pEditEngine->GetText() );
        pEditEngine->Clear();   //incorrect font size
        pEditEngine->SetText( aTxt );

        Resize();
    }

    // Apply zoom to smeditwindow text
    static_cast<SmEditEngine*>(GetEditEngine())->executeZoom(GetEditView());
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
            pViewSh->GetGraphicWidget().SetCursorPos(static_cast<sal_uInt16>(nRow), nCol);
            aOldSelection = aNewSelection;
        }
    }
    aCursorMoveIdle.Stop();
}

bool SmEditTextWindow::MouseButtonUp(const MouseEvent &rEvt)
{
    bool bRet = WeldEditView::MouseButtonUp(rEvt);
    if (!SmViewShell::IsInlineEditEnabled())
        CursorMoveTimerHdl(&aCursorMoveIdle);
    mrEditWindow.InvalidateSlots();
    return bRet;
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
        ReleaseMouse();
        SmCmdBoxWindow& rCmdBox = mrEditWindow.GetCmdBox();
        rCmdBox.ShowContextMenu(rCmdBox.WidgetToWindowPos(*GetDrawingArea(), rCEvt.GetMousePosPixel()));
        GrabFocus();
        return true;
    }

    bool bConsumed = WeldEditView::Command(rCEvt);
    if (bConsumed)
        UserPossiblyChangedText();
    return bConsumed;
}

bool SmEditTextWindow::KeyInput(const KeyEvent& rKEvt)
{
    if (rKEvt.GetKeyCode().GetCode() == KEY_F1)
    {
        mrEditWindow.GetView()->StartMainHelp();
        return true;
    }

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
    OUString selected = pEditView->getEditEngine().GetText(aSelection);

    // Check is auto close brackets/braces is disabled
    SmModule *pMod = SM_MOD();
    if (pMod && !pMod->GetConfig()->IsAutoCloseBrackets())
        autoClose = false;
    else if (o3tl::trim(selected) == u"<?>")
        autoClose = true;
    else if (selected.isEmpty() && !aSelection.HasRange())
    {
        selected = pEditView->getEditEngine().GetText(aSelection.nEndPara);
        if (!selected.isEmpty())
        {
            sal_Int32 index = selected.indexOf("\n", aSelection.nEndPos);
            if (index != -1)
            {
                selected = selected.copy(index, sal_Int32(aSelection.nEndPos-index));
                if (o3tl::trim(selected).empty())
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
                    if (o3tl::trim(selected).empty())
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
        if (pView)
            bConsumed = pView->KeyInput(rKEvt);
        if (pView && !bConsumed)
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
            if ( pVShell && pVShell->GetGraphicWidget().HasFocus() )
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
    if (pDocShell && pEditEngine && pEditEngine->IsModified())
        pDocShell->SetModified(true);
    aModifyIdle.Start();
}

void SmEditWindow::CreateEditView(weld::Builder& rBuilder)
{
    assert(!mxTextControl);

    EditEngine *pEditEngine = GetEditEngine();
    //! pEditEngine may be 0.
    //! For example when the program is used by the document-converter
    if (!pEditEngine)
        return;

    mxTextControl.reset(new SmEditTextWindow(*this));
    mxTextControlWin.reset(new weld::CustomWeld(rBuilder, u"editview"_ustr, *mxTextControl));

    SetScrollBarRanges();
}

IMPL_LINK_NOARG(SmEditTextWindow, EditStatusHdl, EditStatus&, void)
{
    Resize();
}

IMPL_LINK(SmEditWindow, ScrollHdl, weld::ScrolledWindow&, rScrolledWindow, void)
{
    if (EditView* pEditView = GetEditView())
    {
        pEditView->SetVisArea(tools::Rectangle(
                    Point(0,
                          rScrolledWindow.vadjustment_get_value()),
                    pEditView->GetVisArea().GetSize()));
        pEditView->Invalidate();
    }
}

tools::Rectangle SmEditWindow::AdjustScrollBars()
{
    tools::Rectangle aRect(Point(), rCmdBox.GetOutputSizePixel());

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
    int nVCurrentDocPos = pEditView->GetVisArea().Top();
    const Size aOut(pEditView->GetOutputArea().GetSize());
    int nVStepIncrement = aOut.Height() * 2 / 10;
    int nVPageIncrement = aOut.Height() * 8 / 10;
    int nVPageSize = aOut.Height();

    /* limit the page size to below nUpper because gtk's gtk_scrolled_window_start_deceleration has
       effectively...

       lower = gtk_adjustment_get_lower
       upper = gtk_adjustment_get_upper - gtk_adjustment_get_page_size

       and requires that upper > lower or the deceleration animation never ends
    */
    nVPageSize = std::min(nVPageSize, nVUpper);

    mxScrolledWindow->vadjustment_configure(nVCurrentDocPos, 0, nVUpper,
                                            nVStepIncrement, nVPageIncrement, nVPageSize);
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

void SmEditWindow::Flush()
{
    if (!mxTextControl)
        return;
    mxTextControl->Flush();
}

void SmEditWindow::GrabFocus()
{
    if (!mxTextControl)
        return;
    mxTextControl->GrabFocus();
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

    // Apply zoom to smeditwindow text
    static_cast<SmEditEngine&>(pEditView->getEditEngine()).executeZoom(pEditView);
    pEditView->SetSelection(eSelection);
}

void SmEditTextWindow::GetFocus()
{
    WeldEditView::GetFocus();

    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetStatusEventHdl(LINK(this, SmEditTextWindow, EditStatusHdl));
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
    static constexpr OUStringLiteral aMark(u"<?>");
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
static bool HasMark(std::u16string_view rText)
{
    return rText.find(u"<?>") != std::u16string_view::npos;
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

    if (SmDocShell *pModifyDoc = bSetDocModified ? mrEditWindow.GetDoc() : nullptr)
        pModifyDoc->SetModified();

    static_cast<SmEditEngine*>(GetEditEngine())->executeZoom(GetEditView());
}

void SmEditWindow::UpdateStatus()
{
    mxTextControl->UpdateStatus(/*bSetDocModified*/false);
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

void SmEditTextWindow::InsertText(const OUString& rText)
{
    EditView* pEditView = GetEditView();
    if (!pEditView)
        return;

    // Note: Insertion of a space in front of commands is done here and
    // in SmEditWindow::InsertCommand.
    ESelection aSelection = pEditView->GetSelection();
    OUString aCurrentFormula = pEditView->getEditEngine().GetText();
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

    pEditView->InsertText(string);

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
            SfxStringItem aTextToFlush(SID_TEXT, GetText());
            pViewSh->GetViewFrame().GetDispatcher()->ExecuteList(
                    SID_TEXT, SfxCallMode::RECORD,
                    { &aTextToFlush });
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
        EditEngine& rEditEngine = pEditView->getEditEngine();
        rEditEngine.SetStatusEventHdl( Link<EditStatus&,void>() );
        rEditEngine.RemoveView(pEditView);
        mxTextControlWin.reset();
        mxTextControl.reset();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
