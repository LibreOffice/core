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

#include <vcl/settings.hxx>

#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <svtools/colorcfg.hxx>

#include <edit.hxx>
#include <smmod.hxx>
#include <view.hxx>
#include <document.hxx>
#include "cfgitem.hxx"
#include "accessibility.hxx"
#include <memory>

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

bool SmEditWindow::IsInlineEditEnabled()
{
    SmViewShell *pView = GetView();
    return pView && pView->IsInlineEditEnabled();
}


SmEditWindow::SmEditWindow( SmCmdBoxWindow &rMyCmdBoxWin ) :
    Window              (&rMyCmdBoxWin),
    DropTargetHelper    ( this ),
    rCmdBox             (rMyCmdBoxWin),
    aModifyIdle         ("SmEditWindow ModifyIdle"),
    aCursorMoveIdle     ("SmEditWindow CursorMoveIdle")
{
    set_id("math_edit");
    SetHelpId(HID_SMA_COMMAND_WIN_EDIT);
    SetMapMode(MapMode(MapUnit::MapPixel));

    // Even RTL languages don't use RTL for math
    EnableRTL( false );

    ApplyColorConfigValues( SM_MOD()->GetColorConfig() );

    // compare DataChanged
    SetBackground( GetSettings().GetStyleSettings().GetWindowColor() );

    aModifyIdle.SetInvokeHandler(LINK(this, SmEditWindow, ModifyTimerHdl));
    aModifyIdle.SetPriority(TaskPriority::LOWEST);

    if (!IsInlineEditEnabled())
    {
        aCursorMoveIdle.SetInvokeHandler(LINK(this, SmEditWindow, CursorMoveTimerHdl));
        aCursorMoveIdle.SetPriority(TaskPriority::LOWEST);
    }

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
    aModifyIdle.Stop();

    StartCursorMove();

    // clean up of classes used for accessibility
    // must be done before EditView (and thus EditEngine) is no longer
    // available for those classes.
    if (mxAccessible.is())
    {
        mxAccessible->ClearWin();    // make Accessible nonfunctional
        mxAccessible.clear();
    }

    if (pEditView)
    {
        EditEngine *pEditEngine = pEditView->GetEditEngine();
        if (pEditEngine)
        {
            pEditEngine->SetStatusEventHdl( Link<EditStatus&,void>() );
            pEditEngine->RemoveView( pEditView.get() );
        }
        pEditView.reset();
    }

    pHScrollBar.disposeAndClear();
    pVScrollBar.disposeAndClear();
    pScrollBox.disposeAndClear();

    DropTargetHelper::dispose();
    vcl::Window::dispose();
}

void SmEditWindow::StartCursorMove()
{
    if (!IsInlineEditEnabled())
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

EditView * SmEditWindow::GetEditView()
{
    return pEditView.get();
}

EditEngine * SmEditWindow::GetEditEngine()
{
    EditEngine *pEditEng = nullptr;
    if (pEditView)
        pEditEng = pEditView->GetEditEngine();
    else
    {
        SmDocShell *pDoc = GetDoc();
        if (pDoc)
            pEditEng = &pDoc->GetEditEngine();
    }
    return pEditEng;
}

void SmEditWindow::ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg )
{
    // Note: SetBackground still done in SmEditWindow::DataChanged
    SetTextColor( rColorCfg.GetColorValue(svtools::FONTCOLOR).nColor );
    Invalidate();
}

void SmEditWindow::DataChanged( const DataChangedEvent& )
{
    const StyleSettings aSettings( GetSettings().GetStyleSettings() );

    // FIXME RenderContext

    ApplyColorConfigValues( SM_MOD()->GetColorConfig() );
    SetBackground( aSettings.GetWindowColor() );

    // edit fields in other Applications use this font instead of
    // the application font thus we use this one too
    SetPointFont(*this, aSettings.GetFieldFont() /*aSettings.GetAppFont()*/);

    EditEngine *pEditEngine = GetEditEngine();
    SmDocShell *pDoc = GetDoc();

    if (pEditEngine && pDoc)
    {
        //!
        //! see also SmDocShell::GetEditEngine() !
        //!

        pEditEngine->SetDefTab(sal_uInt16(GetTextWidth("XXXX")));

        SetEditEngineDefaultFonts(pDoc->GetEditEngineItemPool(), pDoc->GetLinguOptions());

        // forces new settings to be used
        // unfortunately this resets the whole edit engine
        // thus we need to save at least the text
        OUString aTxt( pEditEngine->GetText() );
        pEditEngine->Clear();   //incorrect font size
        pEditEngine->SetText( aTxt );
    }

    AdjustScrollBars();
    Resize();
}

IMPL_LINK_NOARG( SmEditWindow, ModifyTimerHdl, Timer *, void )
{
    UpdateStatus(false);
    aModifyIdle.Stop();
}

IMPL_LINK_NOARG(SmEditWindow, CursorMoveTimerHdl, Timer *, void)
    // every once in a while check cursor position (selection) of edit
    // window and if it has changed (try to) set the formula-cursor
    // according to that.
{
    if (IsInlineEditEnabled())
        return;

    ESelection aNewSelection(GetSelection());

    if (aNewSelection != aOldSelection)
    {
        SmViewShell *pView = rCmdBox.GetView();
        if (pView)
        {
            // get row and column to look for
            sal_Int32  nRow;
            sal_uInt16 nCol;
            SmGetLeftSelectionPart(aNewSelection, nRow, nCol);
            nRow++;
            nCol++;
            pView->GetGraphicWindow().SetCursorPos(static_cast<sal_uInt16>(nRow), nCol);
            aOldSelection = aNewSelection;
        }
    }
    aCursorMoveIdle.Stop();
}

void SmEditWindow::Resize()
{
    if (!pEditView)
        CreateEditView();

    if (pEditView)
    {
        pEditView->SetOutputArea(AdjustScrollBars());
        pEditView->ShowCursor();

        OSL_ENSURE( pEditView->GetEditEngine(), "EditEngine missing" );
        const long nMaxVisAreaStart = pEditView->GetEditEngine()->GetTextHeight() -
                                      pEditView->GetOutputArea().GetHeight();
        if (pEditView->GetVisArea().Top() > nMaxVisAreaStart)
        {
            tools::Rectangle aVisArea(pEditView->GetVisArea() );
            aVisArea.SetTop( std::max<long>(nMaxVisAreaStart, 0) );
            aVisArea.SetSize(pEditView->GetOutputArea().GetSize());
            pEditView->SetVisArea(aVisArea);
            pEditView->ShowCursor();
        }
        InitScrollBars();
    }
    Invalidate();
}

void SmEditWindow::MouseButtonUp(const MouseEvent &rEvt)
{
    if (pEditView)
        pEditView->MouseButtonUp(rEvt);
    else
        Window::MouseButtonUp (rEvt);

    if (!IsInlineEditEnabled())
        CursorMoveTimerHdl(&aCursorMoveIdle);
    InvalidateSlots();
}

void SmEditWindow::MouseButtonDown(const MouseEvent &rEvt)
{
    if (pEditView)
        pEditView->MouseButtonDown(rEvt);
    else
        Window::MouseButtonDown (rEvt);

    GrabFocus();
}

void SmEditWindow::Command(const CommandEvent& rCEvt)
{
    //pass alt press/release to parent impl
    if (rCEvt.GetCommand() == CommandEventId::ModKeyChange)
    {
        Window::Command(rCEvt);
        return;
    }

    bool bForwardEvt = true;
    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        GetParent()->ToTop();

        Point aPoint = rCEvt.GetMousePosPixel();
        SmViewShell* pViewSh = rCmdBox.GetView();
        if (pViewSh)
            pViewSh->GetViewFrame()->GetDispatcher()->ExecutePopup("edit", this, &aPoint);
        bForwardEvt = false;
    }
    else if (rCEvt.GetCommand() == CommandEventId::Wheel)
        bForwardEvt = !HandleWheelCommands( rCEvt );

    if (bForwardEvt)
    {
        if (pEditView)
            pEditView->Command( rCEvt );
        else
            Window::Command (rCEvt);
    }
}


bool SmEditWindow::HandleWheelCommands( const CommandEvent &rCEvt )
{
    bool bCommandHandled = false;    // true if the CommandEvent needs not
                                    // to be passed on (because it has fully
                                    // been taken care of).

    const CommandWheelData* pWData = rCEvt.GetWheelData();
    if (pWData)
    {
        if (CommandWheelMode::ZOOM == pWData->GetMode())
            bCommandHandled = true;     // no zooming in Command window
        else
            bCommandHandled = HandleScrollCommand( rCEvt, pHScrollBar.get(), pVScrollBar.get());
    }

    return bCommandHandled;
}

void SmEditWindow::KeyInput(const KeyEvent& rKEvt)
{
    if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
    {
        bool bCallBase = true;
        SfxViewShell* pViewShell = GetView();
        if ( dynamic_cast<const SmViewShell *>(pViewShell) )
        {
            // Terminate possible InPlace mode
            bCallBase = !pViewShell->Escape();
        }
        if ( bCallBase )
            Window::KeyInput( rKEvt );
    }
    else
    {
        StartCursorMove();

        bool autoClose = false;
        if (!pEditView)
            CreateEditView();
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

        if ( !pEditView->PostKeyEvent(rKEvt) )
        {
            SmViewShell *pView = GetView();
            if ( pView && !pView->KeyInput(rKEvt) )
            {
                // F1 (help) leads to the destruction of this
                Flush();
                if ( aModifyIdle.IsActive() )
                    aModifyIdle.Stop();
                Window::KeyInput(rKEvt);
            }
            else
            {
                // SFX has maybe called a slot of the view and thus (because of a hack in SFX)
                // set the focus to the view
                SfxViewShell* pVShell = GetView();
                if ( dynamic_cast<const SmViewShell *>(pVShell) &&
                     static_cast<SmViewShell*>(pVShell)->GetGraphicWindow().HasFocus() )
                {
                    GrabFocus();
                }
            }
        }
        else
        {
            // have doc-shell modified only for formula input/change and not
            // cursor travelling and such things...
            SmDocShell *pDocShell = GetDoc();
            EditEngine *pEditEngine = GetEditEngine();
            if (pDocShell && pEditEngine)
                pDocShell->SetModified(pEditEngine->IsModified());
            aModifyIdle.Start();
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

        InvalidateSlots();
    }
}

void SmEditWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (!pEditView)
        CreateEditView();
    pEditView->Paint(rRect, &rRenderContext);
}

void SmEditWindow::CreateEditView()
{
    EditEngine *pEditEngine = GetEditEngine();

    //! pEditEngine and pEditView may be 0.
    //! For example when the program is used by the document-converter
    if (pEditView || !pEditEngine)
        return;

    pEditView.reset(new EditView(pEditEngine, this));
    pEditEngine->InsertView( pEditView.get() );

    if (!pVScrollBar)
        pVScrollBar = VclPtr<ScrollBar>::Create(this, WinBits(WB_VSCROLL));
    if (!pHScrollBar)
        pHScrollBar = VclPtr<ScrollBar>::Create(this, WinBits(WB_HSCROLL));
    if (!pScrollBox)
        pScrollBox  = VclPtr<ScrollBarBox>::Create(this);
    pVScrollBar->SetScrollHdl(LINK(this, SmEditWindow, ScrollHdl));
    pHScrollBar->SetScrollHdl(LINK(this, SmEditWindow, ScrollHdl));
    pVScrollBar->EnableDrag();
    pHScrollBar->EnableDrag();

    pEditView->SetOutputArea(AdjustScrollBars());

    ESelection eSelection;

    pEditView->SetSelection(eSelection);
    Update();
    pEditView->ShowCursor();

    pEditEngine->SetStatusEventHdl( LINK(this, SmEditWindow, EditStatusHdl) );
    SetPointer(pEditView->GetPointer());

    SetScrollBarRanges();
}


IMPL_LINK_NOARG( SmEditWindow, EditStatusHdl, EditStatus&, void )
{
    if (pEditView)
        Resize();
}

IMPL_LINK( SmEditWindow, ScrollHdl, ScrollBar *, /*pScrollBar*/, void )
{
    OSL_ENSURE(pEditView, "EditView missing");
    if (pEditView)
    {
        pEditView->SetVisArea(tools::Rectangle(Point(pHScrollBar->GetThumbPos(),
                                            pVScrollBar->GetThumbPos()),
                                        pEditView->GetVisArea().GetSize()));
        pEditView->Invalidate();
    }
}

tools::Rectangle SmEditWindow::AdjustScrollBars()
{
    const Size aOut( GetOutputSizePixel() );
    tools::Rectangle aRect( Point(), aOut );

    if (pVScrollBar && pHScrollBar && pScrollBox)
    {
        const long nTmp = GetSettings().GetStyleSettings().GetScrollBarSize();
        Point aPt( aRect.TopRight() ); aPt.AdjustX( -(nTmp -1) );
        pVScrollBar->SetPosSizePixel( aPt, Size(nTmp, aOut.Height() - nTmp));

        aPt = aRect.BottomLeft(); aPt.AdjustY( -(nTmp - 1) );
        pHScrollBar->SetPosSizePixel( aPt, Size(aOut.Width() - nTmp, nTmp));

        aPt.setX( pHScrollBar->GetSizePixel().Width() );
        aPt.setY( pVScrollBar->GetSizePixel().Height() );
        pScrollBox->SetPosSizePixel(aPt, Size(nTmp, nTmp ));

        aRect.SetRight( aPt.X() - 2 );
        aRect.SetBottom( aPt.Y() - 2 );
    }
    return aRect;
}

void SmEditWindow::SetScrollBarRanges()
{
    // Extra method, not InitScrollBars, since it's also being used for EditEngine events
    EditEngine *pEditEngine = GetEditEngine();
    if (pVScrollBar && pHScrollBar && pEditEngine && pEditView)
    {
        long nTmp = pEditEngine->GetTextHeight();
        pVScrollBar->SetRange(Range(0, nTmp));
        pVScrollBar->SetThumbPos(pEditView->GetVisArea().Top());

        nTmp = pEditEngine->GetPaperSize().Width();
        pHScrollBar->SetRange(Range(0,nTmp));
        pHScrollBar->SetThumbPos(pEditView->GetVisArea().Left());
    }
}

void SmEditWindow::InitScrollBars()
{
    if (!(pVScrollBar && pHScrollBar && pScrollBox && pEditView))
        return;

    const Size aOut( pEditView->GetOutputArea().GetSize() );
    pVScrollBar->SetVisibleSize(aOut.Height());
    pVScrollBar->SetPageSize(aOut.Height() * 8 / 10);
    pVScrollBar->SetLineSize(aOut.Height() * 2 / 10);

    pHScrollBar->SetVisibleSize(aOut.Width());
    pHScrollBar->SetPageSize(aOut.Width() * 8 / 10);
    pHScrollBar->SetLineSize(SCROLL_LINE );

    SetScrollBarRanges();

    pVScrollBar->Show();
    pHScrollBar->Show();
    pScrollBox->Show();
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
    EditEngine *pEditEngine = GetEditEngine();
    OSL_ENSURE( pEditEngine, "EditEngine missing" );
    if (!pEditEngine || pEditEngine->IsModified())
        return;

    if (!pEditView)
        CreateEditView();

    ESelection eSelection = pEditView->GetSelection();

    pEditEngine->SetText(rText);
    pEditEngine->ClearModifyFlag();

    // Restarting the timer here, prevents calling the handlers for other (currently inactive)
    // math tasks
    aModifyIdle.Start();

    pEditView->SetSelection(eSelection);
}


void SmEditWindow::GetFocus()
{
    Window::GetFocus();

    if (mxAccessible.is())
    {
        // Note: will implicitly send the AccessibleStateType::FOCUSED event
        ::accessibility::AccessibleTextHelper *pHelper = mxAccessible->GetTextHelper();
        if (pHelper)
            pHelper->SetFocus();
    }

    if (!pEditView)
         CreateEditView();
    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetStatusEventHdl( LINK(this, SmEditWindow, EditStatusHdl) );

    //Let SmViewShell know we got focus
    if(GetView() && IsInlineEditEnabled())
        GetView()->SetInsertIntoEditWindow(true);
}


void SmEditWindow::LoseFocus()
{
    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine)
        pEditEngine->SetStatusEventHdl( Link<EditStatus&,void>() );

    Window::LoseFocus();

    if (mxAccessible.is())
    {
        // Note: will implicitly send the AccessibleStateType::FOCUSED event
        ::accessibility::AccessibleTextHelper *pHelper = mxAccessible->GetTextHelper();
        if (pHelper)
            pHelper->SetFocus(false);
    }
}


bool SmEditWindow::IsAllSelected() const
{
    bool bRes = false;
    EditEngine *pEditEngine = const_cast<SmEditWindow *>(this)->GetEditEngine();
    OSL_ENSURE( pEditView, "NULL pointer" );
    OSL_ENSURE( pEditEngine, "NULL pointer" );
    if (pEditEngine  &&  pEditView)
    {
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
    }
    return bRes;
}

void SmEditWindow::SelectAll()
{
    OSL_ENSURE( pEditView, "NULL pointer" );
    if (pEditView)
    {
        // ALL as last two parameters refers to the end of the text
        pEditView->SetSelection( ESelection( 0, 0, EE_PARA_ALL, EE_TEXTPOS_ALL ) );
    }
}

void SmEditWindow::MarkError(const Point &rPos)
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
    {
        const sal_uInt16        nCol = sal::static_int_cast< sal_uInt16 >(rPos.X());
        const sal_uInt16        nRow = sal::static_int_cast< sal_uInt16 >(rPos.Y() - 1);

        pEditView->SetSelection(ESelection(nRow, nCol - 1, nRow, nCol));
        GrabFocus();
    }
}

// Makes selection to next <?> symbol
void SmEditWindow::SelNextMark()
{
    EditEngine *pEditEngine = GetEditEngine();
    OSL_ENSURE( pEditView, "NULL pointer" );
    OSL_ENSURE( pEditEngine, "NULL pointer" );
    if (!pEditEngine || !pEditView)
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
    OSL_ENSURE( pEditEngine, "NULL pointer" );
    OSL_ENSURE( pEditView, "NULL pointer" );
    if (!(pEditEngine  &&  pEditView))
        return;

    ESelection eSelection = pEditView->GetSelection();
    sal_Int32 nPara = eSelection.nStartPara;
    sal_Int32 nMax = eSelection.nStartPos;
    OUString aText(pEditEngine->GetText(nPara));
    const OUString aMark("<?>");
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

bool SmEditWindow::HasMark(const OUString& rText)
    // returns true iff 'rText' contains a mark
{
    return rText.indexOf("<?>") != -1;
}

void SmEditWindow::MouseMove(const MouseEvent &rEvt)
{
    if (pEditView)
        pEditView->MouseMove(rEvt);
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
    // pointer may be 0 when reloading a document and the old view
    // was already destroyed
    //(OSL_ENSURE( pEditView, "NULL pointer" );
    ESelection eSel;
    if (pEditView)
        eSel = pEditView->GetSelection();
    return eSel;
}

void SmEditWindow::SetSelection(const ESelection &rSel)
{
    OSL_ENSURE( pEditView, "NULL pointer" );
    if (pEditView)
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
    return pEditView && pEditView->HasSelection();
}


void SmEditWindow::UpdateStatus( bool bSetDocModified )
{
    SmModule *pMod = SM_MOD();
    if (pMod && pMod->GetConfig()->IsAutoRedraw())
        Flush();
    if ( bSetDocModified )
        GetDoc()->SetModified();
}

void SmEditWindow::Cut()
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
    {
        pEditView->Cut();
        UpdateStatus(true);
    }
}

void SmEditWindow::Copy()
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
        pEditView->Copy();
}

void SmEditWindow::Paste()
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
    {
        pEditView->Paste();
        UpdateStatus(true);
    }
}

void SmEditWindow::Delete()
{
    OSL_ENSURE( pEditView, "EditView missing" );
    if (pEditView)
    {
        pEditView->DeleteSelected();
        UpdateStatus(true);
    }
}

void SmEditWindow::InsertText(const OUString& rText)
{
    OSL_ENSURE( pEditView, "EditView missing" );
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
    pVScrollBar->Hide();
    pHScrollBar->Hide();
    pEditView->InsertText(string);
    AdjustScrollBars();
    pVScrollBar->Show();
    pHScrollBar->Show();

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

void SmEditWindow::Flush()
{
    EditEngine *pEditEngine = GetEditEngine();
    if (pEditEngine  &&  pEditEngine->IsModified())
    {
        pEditEngine->ClearModifyFlag();
        SmViewShell *pViewSh = rCmdBox.GetView();
        if (pViewSh)
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
    if (pEditView)
    {
        std::unique_ptr<EditEngine> xEditEngine(pEditView->GetEditEngine());
        if (xEditEngine)
        {
            xEditEngine->SetStatusEventHdl( Link<EditStatus&,void>() );
            xEditEngine->RemoveView( pEditView.get() );
        }
        pEditView.reset();
    }
}

uno::Reference< XAccessible > SmEditWindow::CreateAccessible()
{
    if (!mxAccessible.is())
    {
        mxAccessible.set(new SmEditAccessible( this ));
        mxAccessible->Init();
    }
    return uno::Reference< XAccessible >(mxAccessible.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
