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

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>
#include <i18nutil/unicode.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <svl/poolitem.hxx>
#include <svl/ptitem.hxx>
#include <svl/stritem.hxx>
#include <svtools/transfer.hxx>
#include <svtools/miscopt.hxx>
#include <svl/undo.hxx>
#include <svl/whiter.hxx>
#include <svx/dialogs.hrc>
#include <svx/zoomslideritem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <svx/svxdlg.hxx>
#include <sfx2/zoomitem.hxx>
#include <vcl/decoview.hxx>
#include <vcl/menu.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/settings.hxx>

#include <fstream>
#include <unotools/streamwrap.hxx>

#include "unomodel.hxx"
#include "view.hxx"
#include "cfgitem.hxx"
#include "dialog.hxx"
#include "document.hxx"
#include "starmath.hrc"
#include "mathmlimport.hxx"
#include "cursor.hxx"
#include "accessibility.hxx"
#include "ElementsDockingWindow.hxx"
#include <memory>

#define MINZOOM         25
#define MAXZOOM         800

// space around the edit window, in pixels
// fdo#69111: Increased border on the top so that the window is
// easier to tear off.
#define CMD_BOX_PADDING 4
#define CMD_BOX_PADDING_TOP 10

#define SmViewShell
#include "smslots.hxx"

using namespace css;
using namespace css::accessibility;
using namespace css::uno;

SmGraphicWindow::SmGraphicWindow(SmViewShell* pShell)
    : ScrollableWindow(&pShell->GetViewFrame()->GetWindow(), 0)
    , pAccessible(nullptr)
    , pViewShell(pShell)
    , nZoom(100)
{
    // docking windows are usually hidden (often already done in the
    // resource) and will be shown by the sfx framework.
    Hide();

    const Fraction aFraction(1, 1);
    SetMapMode(MapMode(MAP_100TH_MM, Point(), aFraction, aFraction));

    SetTotalSize();

    SetHelpId(HID_SMA_WIN_DOCUMENT);
    SetUniqueId(HID_SMA_WIN_DOCUMENT);

    ShowLine(false);
    CaretBlinkInit();
}

SmGraphicWindow::~SmGraphicWindow()
{
    disposeOnce();
}

void SmGraphicWindow::dispose()
{
    if (pAccessible)
        pAccessible->ClearWin();    // make Accessible defunctional
    // Note: memory for pAccessible will be freed when the reference
    // xAccessible is released.
    CaretBlinkStop();
    ScrollableWindow::dispose();
}

void SmGraphicWindow::StateChanged(StateChangedType eType)
{
    if (eType == StateChangedType::InitShow)
        Show();
    ScrollableWindow::StateChanged(eType);
}


void SmGraphicWindow::ApplyColorConfigValues(const svtools::ColorConfig &rColorCfg)
{
    // Note: SetTextColor not necessary since the nodes that
    // get painted have the color information.
#if OSL_DEBUG_LEVEL > 1
//   ColorData nVal = rColorCfg.GetColorValue(svtools::DOCCOLOR).nColor;
#endif
    SetBackground(Color( (ColorData) rColorCfg.GetColorValue(svtools::DOCCOLOR).nColor));
}


void SmGraphicWindow::DataChanged( const DataChangedEvent& rEvt )
{
    ScrollableWindow::DataChanged( rEvt );
}


void SmGraphicWindow::MouseButtonDown(const MouseEvent& rMEvt)
{
    ScrollableWindow::MouseButtonDown(rMEvt);

    GrabFocus();

    // set formula-cursor and selection of edit window according to the
    // position clicked at

    SAL_WARN_IF( rMEvt.GetClicks() == 0, "starmath", "0 clicks" );
    if ( rMEvt.IsLeft() )
    {
        // get click position relativ to formula
        Point  aPos (PixelToLogic(rMEvt.GetPosPixel())
                     - GetFormulaDrawPos());

        const SmNode *pTree = pViewShell->GetDoc()->GetFormulaTree();
        if (!pTree)
            return;

        if (IsInlineEditEnabled()) {
            pViewShell->GetDoc()->GetCursor().MoveTo(this, aPos, !rMEvt.IsShift());
            return;
        }
        const SmNode *pNode = nullptr;
        // if it was clicked inside the formula then get the appropriate node
        if (pTree->OrientedDist(aPos) <= 0)
            pNode = pTree->FindRectClosestTo(aPos);

        if (pNode)
        {   SmEditWindow  *pEdit = pViewShell->GetEditWindow();
            if (!pEdit)
                return;
            const SmToken  aToken (pNode->GetToken());

            // set selection to the beginning of the token
            ESelection  aSel (aToken.nRow - 1, aToken.nCol - 1);

            if (rMEvt.GetClicks() != 1 || aToken.eType == TPLACE)
                aSel.nEndPos = aSel.nEndPos + sal::static_int_cast< sal_uInt16 >(aToken.aText.getLength());

            pEdit->SetSelection(aSel);
            SetCursor(pNode);

            // allow for immediate editing and
            //! implicitly synchronize the cursor position mark in this window
            pEdit->GrabFocus();
        }
    }
}

void SmGraphicWindow::MouseMove(const MouseEvent &rMEvt)
{
    ScrollableWindow::MouseMove(rMEvt);

    if (rMEvt.IsLeft() && IsInlineEditEnabled())
    {
        Point aPos(PixelToLogic(rMEvt.GetPosPixel()) - GetFormulaDrawPos());
        pViewShell->GetDoc()->GetCursor().MoveTo(this, aPos, false);

        CaretBlinkStop();
        SetIsCursorVisible(true);
        CaretBlinkStart();
        RepaintViewShellDoc();
    }
}

bool SmGraphicWindow::IsInlineEditEnabled() const
{
    return pViewShell->IsInlineEditEnabled();
}

void SmGraphicWindow::GetFocus()
{
    if (!IsInlineEditEnabled())
        return;
    if (pViewShell->GetEditWindow())
        pViewShell->GetEditWindow()->Flush();
    //Let view shell know what insertions should be done in visual editor
    pViewShell->SetInsertIntoEditWindow(false);
    SetIsCursorVisible(true);
    ShowLine(true);
    CaretBlinkStart();
    RepaintViewShellDoc();
}

void SmGraphicWindow::LoseFocus()
{
    ScrollableWindow::LoseFocus();
    if (xAccessible.is())
    {
        uno::Any aOldValue, aNewValue;
        aOldValue <<= AccessibleStateType::FOCUSED;
        // aNewValue remains empty
        pAccessible->LaunchEvent( AccessibleEventId::STATE_CHANGED,
                aOldValue, aNewValue );
    }
    if (!IsInlineEditEnabled())
        return;
    SetIsCursorVisible(false);
    ShowLine(false);
    CaretBlinkStop();
    RepaintViewShellDoc();
}

void SmGraphicWindow::RepaintViewShellDoc()
{
    SmDocShell* pDoc = pViewShell->GetDoc();
    if (pDoc)
        pDoc->Repaint();
}

IMPL_LINK_NOARG_TYPED(SmGraphicWindow, CaretBlinkTimerHdl, Timer *, void)
{
    if (IsCursorVisible())
        SetIsCursorVisible(false);
    else
        SetIsCursorVisible(true);

    RepaintViewShellDoc();
}

void SmGraphicWindow::CaretBlinkInit()
{
    aCaretBlinkTimer.SetTimeoutHdl(LINK(this, SmGraphicWindow, CaretBlinkTimerHdl));
    aCaretBlinkTimer.SetTimeout( ScrollableWindow::GetSettings().GetStyleSettings().GetCursorBlinkTime() );
}

void SmGraphicWindow::CaretBlinkStart()
{
    if (!IsInlineEditEnabled())
        return;
    if (aCaretBlinkTimer.GetTimeout() != STYLE_CURSOR_NOBLINKTIME)
        aCaretBlinkTimer.Start();
}

void SmGraphicWindow::CaretBlinkStop()
{
    if (!IsInlineEditEnabled())
        return;
    aCaretBlinkTimer.Stop();
}

void SmGraphicWindow::ShowCursor(bool bShow)
    // shows or hides the formula-cursor depending on 'bShow' is true or not
{
    if (IsInlineEditEnabled())
        return;

    bool  bInvert = bShow != IsCursorVisible();

    if (bInvert)
        InvertTracking(aCursorRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW);

    SetIsCursorVisible(bShow);
}

void SmGraphicWindow::ShowLine(bool bShow)
{
    if (!IsInlineEditEnabled())
        return;

    bIsLineVisible = bShow;
}

void SmGraphicWindow::SetCursor(const SmNode *pNode)
{
    if (IsInlineEditEnabled())
        return;

    const SmNode *pTree = pViewShell->GetDoc()->GetFormulaTree();

    // get appropriate rectangle
    Point aOffset (pNode->GetTopLeft() - pTree->GetTopLeft()),
          aTLPos  (GetFormulaDrawPos() + aOffset);
    aTLPos.X() -= pNode->GetItalicLeftSpace();
    Size  aSize   (pNode->GetItalicSize());

    SetCursor(Rectangle(aTLPos, aSize));
}

void SmGraphicWindow::SetCursor(const Rectangle &rRect)
    // sets cursor to new position (rectangle) 'rRect'.
    // The old cursor will be removed, and the new one will be shown if
    // that is activated in the ConfigItem
{
    if (IsInlineEditEnabled())
        return;

    SmModule *pp = SM_MOD();

    if (IsCursorVisible())
        ShowCursor(false);      // clean up remainings of old cursor
    aCursorRect = rRect;
    if (pp->GetConfig()->IsShowFormulaCursor())
        ShowCursor(true);       // draw new cursor
}

const SmNode * SmGraphicWindow::SetCursorPos(sal_uInt16 nRow, sal_uInt16 nCol)
    // looks for a VISIBLE node in the formula tree with its token at
    // (or around) the position 'nRow', 'nCol' in the edit window
    // (row and column numbering starts with 1 there!).
    // If there is such a node the formula-cursor is set to cover that nodes
    // rectangle. If not the formula-cursor will be hidden.
    // In any case the search result is being returned.
{
    if (IsInlineEditEnabled())
        return nullptr;

    // find visible node with token at nRow, nCol
    const SmNode *pTree = pViewShell->GetDoc()->GetFormulaTree(),
                 *pNode = nullptr;
    if (pTree)
        pNode = pTree->FindTokenAt(nRow, nCol);

    if (pNode)
        SetCursor(pNode);
    else
        ShowCursor(false);

    return pNode;
}

void SmGraphicWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    SAL_WARN_IF(!pViewShell, "starmath", "view shell missing");

    ApplyColorConfigValues(SM_MOD()->GetColorConfig());

    SmDocShell& rDoc = *pViewShell->GetDoc();
    Point aPoint;

    rDoc.DrawFormula(rRenderContext, aPoint, true);  //! modifies aPoint to be the topleft
                                //! corner of the formula
    SetFormulaDrawPos(aPoint);
    if (IsInlineEditEnabled())
    {
        //Draw cursor if any...
        if (pViewShell->GetDoc()->HasCursor() && IsLineVisible())
            pViewShell->GetDoc()->GetCursor().Draw(rRenderContext, aPoint, IsCursorVisible());
    }
    else
    {
        SetIsCursorVisible(false);  // (old) cursor must be drawn again

        const SmEditWindow* pEdit = pViewShell->GetEditWindow();
        if (pEdit)
        {   // get new position for formula-cursor (for possible altered formula)
            sal_Int32  nRow;
            sal_uInt16 nCol;
            SmGetLeftSelectionPart(pEdit->GetSelection(), nRow, nCol);
            nRow++;
            nCol++;
            const SmNode *pFound = SetCursorPos(static_cast<sal_uInt16>(nRow), nCol);

            SmModule *pp = SM_MOD();
            if (pFound && pp->GetConfig()->IsShowFormulaCursor())
                ShowCursor(true);
        }
    }
}


void SmGraphicWindow::SetTotalSize ()
{
    SmDocShell &rDoc = *pViewShell->GetDoc();
    const Size aTmp( PixelToLogic( LogicToPixel( rDoc.GetSize() )));
    if ( aTmp != ScrollableWindow::GetTotalSize() )
        ScrollableWindow::SetTotalSize( aTmp );
}

void SmGraphicWindow::KeyInput(const KeyEvent& rKEvt)
{
    if (!IsInlineEditEnabled()) {
        if (! (GetView() && GetView()->KeyInput(rKEvt)) )
            ScrollableWindow::KeyInput(rKEvt);
        return;
    }

    SmCursor& rCursor = pViewShell->GetDoc()->GetCursor();
    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();
    if (eFunc == KeyFuncType::COPY)
        rCursor.Copy();
    else if (eFunc == KeyFuncType::CUT)
        rCursor.Cut();
    else if (eFunc == KeyFuncType::PASTE)
        rCursor.Paste();
    else {
    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    switch(nCode)
    {
        case KEY_LEFT:
        {
            rCursor.Move(this, MoveLeft, !rKEvt.GetKeyCode().IsShift());
        }break;
        case KEY_RIGHT:
        {
            rCursor.Move(this, MoveRight, !rKEvt.GetKeyCode().IsShift());
        }break;
        case KEY_UP:
        {
            rCursor.Move(this, MoveUp, !rKEvt.GetKeyCode().IsShift());
        }break;
        case KEY_DOWN:
        {
            rCursor.Move(this, MoveDown, !rKEvt.GetKeyCode().IsShift());
        }break;
        case KEY_RETURN:
        {
            if(!rKEvt.GetKeyCode().IsShift())
                rCursor.InsertRow();
#ifdef DEBUG_ENABLE_DUMPASDOT
            else {
                SmNode *pTree = (SmNode*)pViewShell->GetDoc()->GetFormulaTree();
                std::fstream file("/tmp/smath-dump.gv", std::fstream::out);
                OUString label(pViewShell->GetDoc()->GetText());
                pTree->DumpAsDot(file, &label);
                file.close();
            }
#endif /* DEBUG_ENABLE_DUMPASDOT */
        }break;
        case KEY_DELETE:
        {
            if(!rCursor.HasSelection()){
                rCursor.Move(this, MoveRight, false);
                if(rCursor.HasComplexSelection()) break;
            }
            rCursor.Delete();
        }break;
        case KEY_BACKSPACE:
        {
            rCursor.DeletePrev(this);
        }break;
        case KEY_ADD:
            rCursor.InsertElement(PlusElement);
            break;
        case KEY_SUBTRACT:
            if(rKEvt.GetKeyCode().IsShift())
                rCursor.InsertSubSup(RSUB);
            else
                rCursor.InsertElement(MinusElement);
            break;
        case KEY_MULTIPLY:
            rCursor.InsertElement(CDotElement);
            break;
        case KEY_DIVIDE:
            rCursor.InsertFraction();
            break;
        case KEY_LESS:
            rCursor.InsertElement(LessThanElement);
            break;
        case KEY_GREATER:
            rCursor.InsertElement(GreaterThanElement);
            break;
        case KEY_EQUAL:
            rCursor.InsertElement(EqualElement);
            break;
        default:
        {
            sal_Unicode code = rKEvt.GetCharCode();
            SmBraceNode* pBraceNode = nullptr;

            if(code == ' ') {
                rCursor.InsertElement(BlankElement);
            }else if(code == '^') {
                rCursor.InsertSubSup(RSUP);
            }else if(code == '(') {
                rCursor.InsertBrackets(RoundBrackets);
            }else if(code == '[') {
                rCursor.InsertBrackets(SquareBrackets);
            }else if(code == '{') {
                rCursor.InsertBrackets(CurlyBrackets);
            }else if(code == '!') {
                rCursor.InsertElement(FactorialElement);
            }else if(code == '%') {
                rCursor.InsertElement(PercentElement);
            }else if(code == ')' && rCursor.IsAtTailOfBracket(RoundBrackets, &pBraceNode)) {
                rCursor.MoveAfterBracket(pBraceNode);
            }else if(code == ']' && rCursor.IsAtTailOfBracket(SquareBrackets, &pBraceNode)) {
                rCursor.MoveAfterBracket(pBraceNode);
            }else if(code == '}' && rCursor.IsAtTailOfBracket(CurlyBrackets, &pBraceNode)) {
                rCursor.MoveAfterBracket(pBraceNode);
            }else{
                if(code != 0){
                    rCursor.InsertText(OUString(code));
                }else if (! (GetView() && GetView()->KeyInput(rKEvt)) )
                    ScrollableWindow::KeyInput(rKEvt);
            }
        }
    }
    }
    CaretBlinkStop();
    CaretBlinkStart();
    SetIsCursorVisible(true);
    RepaintViewShellDoc();
}


void SmGraphicWindow::Command(const CommandEvent& rCEvt)
{
    bool bCallBase = true;
    if ( !pViewShell->GetViewFrame()->GetFrame().IsInPlace() )
    {
        switch ( rCEvt.GetCommand() )
        {
            case CommandEventId::ContextMenu:
            {
                GetParent()->ToTop();
                SmResId aResId( RID_VIEWMENU );
                std::unique_ptr<PopupMenu> xPopupMenu(new PopupMenu(aResId));
                xPopupMenu->SetSelectHdl(LINK(this, SmGraphicWindow, MenuSelectHdl));
                Point aPos(5, 5);
                if (rCEvt.IsMouseEvent())
                    aPos = rCEvt.GetMousePosPixel();
                SAL_WARN_IF( !pViewShell, "starmath", "view shell missing" );

                // added for replaceability of context menus
                pViewShell->GetViewFrame()->GetBindings().GetDispatcher()
                        ->ExecutePopup( aResId, this, &aPos );

                bCallBase = false;
            }
            break;

            case CommandEventId::Wheel:
            {
                const CommandWheelData* pWData = rCEvt.GetWheelData();
                if  ( pWData && CommandWheelMode::ZOOM == pWData->GetMode() )
                {
                    sal_uInt16 nTmpZoom = GetZoom();
                    if( 0L > pWData->GetDelta() )
                        nTmpZoom -= 10;
                    else
                        nTmpZoom += 10;
                    SetZoom( nTmpZoom );
                    bCallBase = false;
                }
            }
            break;

            default: break;
        }
    }
    if ( bCallBase )
        ScrollableWindow::Command (rCEvt);
}


IMPL_LINK_TYPED( SmGraphicWindow, MenuSelectHdl, Menu *, pMenu, bool )
{
    SmViewShell *pViewSh = GetView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetDispatcher()->Execute( pMenu->GetCurItemId() );
    return false;
}

void SmGraphicWindow::SetZoom(sal_uInt16 Factor)
{
    nZoom = std::min(std::max((sal_uInt16) Factor, (sal_uInt16) MINZOOM), (sal_uInt16) MAXZOOM);
    Fraction   aFraction (nZoom, 100);
    SetMapMode( MapMode(MAP_100TH_MM, Point(), aFraction, aFraction) );
    SetTotalSize();
    SmViewShell *pViewSh = GetView();
    if (pViewSh)
    {
        pViewSh->GetViewFrame()->GetBindings().Invalidate(SID_ATTR_ZOOM);
        pViewSh->GetViewFrame()->GetBindings().Invalidate(SID_ATTR_ZOOMSLIDER);
    }
    Invalidate();
}


void SmGraphicWindow::ZoomToFitInWindow()
{
    SmDocShell &rDoc = *pViewShell->GetDoc();

    // set defined mapmode before calling 'LogicToPixel' below
    SetMapMode(MapMode(MAP_100TH_MM));

    Size       aSize (LogicToPixel(rDoc.GetSize()));
    Size       aWindowSize (GetSizePixel());

    if (aSize.Width() > 0  &&  aSize.Height() > 0)
    {
        long nVal = std::min ((85 * aWindowSize.Width())  / aSize.Width(),
                      (85 * aWindowSize.Height()) / aSize.Height());
        SetZoom ( sal::static_int_cast< sal_uInt16 >(nVal) );
    }
}

uno::Reference< XAccessible > SmGraphicWindow::CreateAccessible()
{
    if (!pAccessible)
    {
        pAccessible = new SmGraphicAccessible( this );
        xAccessible = pAccessible;
    }
    return xAccessible;
}

/**************************************************************************/


SmGraphicController::SmGraphicController(SmGraphicWindow &rSmGraphic,
                        sal_uInt16          nId_,
                        SfxBindings     &rBindings) :
    SfxControllerItem(nId_, rBindings),
    rGraphic(rSmGraphic)
{
}


void SmGraphicController::StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    rGraphic.SetTotalSize();
    rGraphic.Invalidate();
    SfxControllerItem::StateChanged (nSID, eState, pState);
}


/**************************************************************************/


SmEditController::SmEditController(SmEditWindow &rSmEdit,
                     sal_uInt16       nId_,
                     SfxBindings  &rBindings) :
    SfxControllerItem(nId_, rBindings),
    rEdit(rSmEdit)
{
}


#if OSL_DEBUG_LEVEL > 1
SmEditController::~SmEditController()
{
}
#endif


void SmEditController::StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    const SfxStringItem *pItem =  dynamic_cast<const SfxStringItem*>( pState);

    if ((pItem != nullptr) && (rEdit.GetText() != OUString(pItem->GetValue())))
        rEdit.SetText(pItem->GetValue());
    SfxControllerItem::StateChanged (nSID, eState, pState);
}

/**************************************************************************/
SmCmdBoxWindow::SmCmdBoxWindow(SfxBindings *pBindings_, SfxChildWindow *pChildWindow,
                               vcl::Window *pParent) :
    SfxDockingWindow(pBindings_, pChildWindow, pParent, WB_MOVEABLE|WB_CLOSEABLE|WB_SIZEABLE|WB_DOCKABLE),
    aEdit       (VclPtr<SmEditWindow>::Create(*this)),
    aController (*(aEdit.get()), SID_TEXT, *pBindings_),
    bExiting    (false)
{
    SetHelpId( HID_SMA_COMMAND_WIN );
    SetSizePixel(LogicToPixel(Size(292 , 94), MapMode(MAP_APPFONT)));
    SetText(SM_RESSTR(STR_CMDBOXWINDOW));

    Hide();

    aInitialFocusTimer.SetTimeoutHdl(LINK(this, SmCmdBoxWindow, InitialFocusTimerHdl));
    aInitialFocusTimer.SetTimeout(100);
}

SmCmdBoxWindow::~SmCmdBoxWindow ()
{
    disposeOnce();
}

void SmCmdBoxWindow::dispose()
{
    aInitialFocusTimer.Stop();
    bExiting = true;
    aController.dispose();
    aEdit.disposeAndClear();
    SfxDockingWindow::dispose();
}

SmViewShell * SmCmdBoxWindow::GetView()
{
    SfxDispatcher *pDispatcher = GetBindings().GetDispatcher();
    SfxViewShell *pView = pDispatcher ? pDispatcher->GetFrame()->GetViewShell() : nullptr;
    return  dynamic_cast<SmViewShell*>( pView);
}

void SmCmdBoxWindow::Resize()
{
     Rectangle aRect(Point(0, 0), GetOutputSizePixel());
     aRect.Left()   += CMD_BOX_PADDING;
     aRect.Top()    += CMD_BOX_PADDING_TOP;
     aRect.Right()  -= CMD_BOX_PADDING;
     aRect.Bottom() -= CMD_BOX_PADDING;

    DecorationView aView(this);
    aRect = aView.DrawFrame(aRect, DrawFrameStyle::In, DrawFrameFlags::NoDraw);

    aEdit->SetPosSizePixel(aRect.TopLeft(), aRect.GetSize());
    SfxDockingWindow::Resize();
    Invalidate();
}

void SmCmdBoxWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
{
    Rectangle aRect(Point(0, 0), GetOutputSizePixel());
    aRect.Left()   += CMD_BOX_PADDING;
    aRect.Top()    += CMD_BOX_PADDING_TOP;
    aRect.Right()  -= CMD_BOX_PADDING;
    aRect.Bottom() -= CMD_BOX_PADDING;

    aEdit->SetPosSizePixel(aRect.TopLeft(), aRect.GetSize());

    DecorationView aView(&rRenderContext);
    aView.DrawFrame( aRect, DrawFrameStyle::In );
}

Size SmCmdBoxWindow::CalcDockingSize(SfxChildAlignment eAlign)
{
    switch (eAlign)
    {
        case SfxChildAlignment::LEFT:
        case SfxChildAlignment::RIGHT:
            return Size();
        default:
            break;
    }
    return SfxDockingWindow::CalcDockingSize(eAlign);
}

SfxChildAlignment SmCmdBoxWindow::CheckAlignment(SfxChildAlignment eActual,
                                             SfxChildAlignment eWish)
{
    switch (eWish)
    {
        case SfxChildAlignment::TOP:
        case SfxChildAlignment::BOTTOM:
        case SfxChildAlignment::NOALIGNMENT:
            return eWish;
        default:
            break;
    }

    return eActual;
}

void SmCmdBoxWindow::StateChanged( StateChangedType nStateChange )
{
    if (StateChangedType::InitShow == nStateChange)
    {
        Resize();   // avoid SmEditWindow not being painted correctly

        // set initial position of window in floating mode
        if (IsFloatingMode())
            AdjustPosition();   //! don't change pos in docking-mode !

        aInitialFocusTimer.Start();
    }

    SfxDockingWindow::StateChanged( nStateChange );
}

IMPL_LINK_NOARG_TYPED( SmCmdBoxWindow, InitialFocusTimerHdl, Timer *, void )
{
    // We want to have the focus in the edit window once Math has been opened
    // to allow for immediate typing.
    // Problem: There is no proper way to do this
    // Thus: this timer based solution has been implemented (see GrabFocus below)

    // Follow-up problem (#i114910): grabbing the focus may bust the help system since
    // it relies on getting the current frame which conflicts with grabbing the focus.
    // Thus aside from the 'GrabFocus' call everything else is to get the
    // help reliably working despite using 'GrabFocus'.

    try
    {
        uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create( comphelper::getProcessComponentContext() );

        aEdit->GrabFocus();

        bool bInPlace = GetView()->GetViewFrame()->GetFrame().IsInPlace();
        uno::Reference< frame::XFrame > xFrame( GetBindings().GetDispatcher()->GetFrame()->GetFrame().GetFrameInterface());
        if ( bInPlace )
        {
            uno::Reference< container::XChild > xModel( GetView()->GetDoc()->GetModel(), uno::UNO_QUERY_THROW );
            uno::Reference< frame::XModel > xParent( xModel->getParent(), uno::UNO_QUERY_THROW );
            uno::Reference< frame::XController > xParentCtrler( xParent->getCurrentController() );
            uno::Reference< frame::XFramesSupplier > xParentFrame( xParentCtrler->getFrame(), uno::UNO_QUERY_THROW );
            xParentFrame->setActiveFrame( xFrame );
        }
        else
        {
            xDesktop->setActiveFrame( xFrame );
        }
    }
    catch (uno::Exception &)
    {
        SAL_WARN( "starmath", "failed to properly set initial focus to edit window" );
    }
}

void SmCmdBoxWindow::AdjustPosition()
{
    Point aPt;
    const Rectangle aRect( aPt, GetParent()->GetOutputSizePixel() );
    Point aTopLeft( Point( aRect.Left(),
                           aRect.Bottom() - GetSizePixel().Height() ) );
    Point aPos( GetParent()->OutputToScreenPixel( aTopLeft ) );
    if (aPos.X() < 0)
        aPos.X() = 0;
    if (aPos.Y() < 0)
        aPos.Y() = 0;
    SetPosPixel( aPos );
}

void SmCmdBoxWindow::ToggleFloatingMode()
{
    SfxDockingWindow::ToggleFloatingMode();

    if (GetFloatingWindow())
        GetFloatingWindow()->SetMinOutputSizePixel(Size (200, 50));
}

void SmCmdBoxWindow::GetFocus()
{
    if (!bExiting)
        aEdit->GrabFocus();
}

SFX_IMPL_DOCKINGWINDOW_WITHID(SmCmdBoxWrapper, SID_CMDBOXWINDOW);

SmCmdBoxWrapper::SmCmdBoxWrapper(vcl::Window *pParentWindow, sal_uInt16 nId,
                                 SfxBindings *pBindings,
                                 SfxChildWinInfo *pInfo) :
    SfxChildWindow(pParentWindow, nId)
{
    SetWindow(VclPtr<SmCmdBoxWindow>::Create(pBindings, this, pParentWindow));

    // make window docked to the bottom initially (after first start)
    SetAlignment(SfxChildAlignment::BOTTOM);
    static_cast<SfxDockingWindow *>(GetWindow())->Initialize(pInfo);
}

#if OSL_DEBUG_LEVEL > 1
SmCmdBoxWrapper::~SmCmdBoxWrapper()
{
}
#endif

struct SmViewShell_Impl
{
    std::unique_ptr<sfx2::DocumentInserter> pDocInserter;
    std::unique_ptr<SfxRequest> pRequest;
    SvtMiscOptions          aOpts;
};


SFX_IMPL_SUPERCLASS_INTERFACE(SmViewShell, SfxViewShell)

void SmViewShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_TOOLS | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                            RID_MATH_TOOLBOX);
    //Dummy-Objectbar, to avoid quiver while activating

    GetStaticInterface()->RegisterChildWindow(SID_TASKPANE);
    GetStaticInterface()->RegisterChildWindow(SmCmdBoxWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SmElementsDockingWindowWrapper::GetChildWindowId());
}

SFX_IMPL_NAMED_VIEWFACTORY(SmViewShell, "Default")
{
    SFX_VIEW_REGISTRATION(SmDocShell);
}

void SmViewShell::AdjustPosSizePixel(const Point &rPos, const Size &rSize)
{
    aGraphic->SetPosSizePixel(rPos, rSize);
}

void SmViewShell::InnerResizePixel(const Point &rOfs, const Size &rSize)
{
    Size aObjSize = GetObjectShell()->GetVisArea().GetSize();
    if ( aObjSize.Width() > 0 && aObjSize.Height() > 0 )
    {
        Size aProvidedSize = GetWindow()->PixelToLogic( rSize, MAP_100TH_MM );
        SfxViewShell::SetZoomFactor( Fraction( aProvidedSize.Width(), aObjSize.Width() ),
                        Fraction( aProvidedSize.Height(), aObjSize.Height() ) );
    }

    SetBorderPixel( SvBorder() );
    GetGraphicWindow().SetPosSizePixel(rOfs, rSize);
    GetGraphicWindow().SetTotalSize();
}

void SmViewShell::OuterResizePixel(const Point &rOfs, const Size &rSize)
{
    SmGraphicWindow &rWin = GetGraphicWindow();
    rWin.SetPosSizePixel(rOfs, rSize);
    if (GetDoc()->IsPreview())
        rWin.ZoomToFitInWindow();
    rWin.Update();
}

void SmViewShell::QueryObjAreaPixel( Rectangle& rRect ) const
{
    rRect.SetSize( GetGraphicWindow().GetSizePixel() );
}

void SmViewShell::SetZoomFactor( const Fraction &rX, const Fraction &rY )
{
    const Fraction &rFrac = rX < rY ? rX : rY;
    GetGraphicWindow().SetZoom( (sal_uInt16) long(rFrac * Fraction( 100, 1 )) );

    //To avoid rounding errors base class regulates crooked values too
    //if necessary
    SfxViewShell::SetZoomFactor( rX, rY );
}

Size SmViewShell::GetTextLineSize(OutputDevice& rDevice, const OUString& rLine)
{
    Size   aSize(rDevice.GetTextWidth(rLine), rDevice.GetTextHeight());
    const long nTabPos = rLine.isEmpty() ? 0 : rDevice.approximate_char_width() * 8;

    if (nTabPos)
    {
        aSize.Width() = 0;
        sal_Int32 nPos = 0;
        do
        {
            if (nPos > 0)
                aSize.Width() = ((aSize.Width() / nTabPos) + 1) * nTabPos;

            const OUString aText = rLine.getToken(0, '\t', nPos);
            aSize.Width() += rDevice.GetTextWidth(aText);
        }
        while (nPos >= 0);
    }

    return aSize;
}

Size SmViewShell::GetTextSize(OutputDevice& rDevice, const OUString& rText, long MaxWidth)
{
    Size aSize;
    Size aTextSize;
    if (rText.isEmpty())
        return aTextSize;

    sal_Int32 nPos = 0;
    do
    {
        OUString aLine = rText.getToken(0, '\n', nPos);
        aLine = comphelper::string::remove(aLine, '\r');

        aSize = GetTextLineSize(rDevice, aLine);

        if (aSize.Width() > MaxWidth)
        {
            do
            {
                OUString aText;
                sal_Int32 m = aLine.getLength();
                sal_Int32 nLen = m;

                for (sal_Int32 n = 0; n < nLen; n++)
                {
                    sal_Unicode cLineChar = aLine[n];
                    if ((cLineChar == ' ') || (cLineChar == '\t'))
                    {
                        aText = aLine.copy(0, n);
                        if (GetTextLineSize(rDevice, aText).Width() < MaxWidth)
                            m = n;
                        else
                            break;
                    }
                }

                aText = aLine.copy(0, m);
                aLine = aLine.replaceAt(0, m, "");
                aSize = GetTextLineSize(rDevice, aText);
                aTextSize.Height() += aSize.Height();
                aTextSize.Width() = std::max(aTextSize.Width(), std::min(aSize.Width(), MaxWidth));

                aLine = comphelper::string::stripStart(aLine, ' ');
                aLine = comphelper::string::stripStart(aLine, '\t');
                aLine = comphelper::string::stripStart(aLine, ' ');
            }
            while (!aLine.isEmpty());
        }
        else
        {
            aTextSize.Height() += aSize.Height();
            aTextSize.Width() = std::max(aTextSize.Width(), aSize.Width());
        }
    }
    while (nPos >= 0);

    return aTextSize;
}

void SmViewShell::DrawTextLine(OutputDevice& rDevice, const Point& rPosition, const OUString& rLine)
{
    Point aPoint(rPosition);
    const long nTabPos = rLine.isEmpty() ? 0 : rDevice.approximate_char_width() * 8;

    if (nTabPos)
    {
        sal_Int32 nPos = 0;
        do
        {
            if (nPos > 0)
                aPoint.X() = ((aPoint.X() / nTabPos) + 1) * nTabPos;

            OUString aText = rLine.getToken(0, '\t', nPos);
            rDevice.DrawText(aPoint, aText);
            aPoint.X() += rDevice.GetTextWidth(aText);
        }
        while ( nPos >= 0 );
    }
    else
        rDevice.DrawText(aPoint, rLine);
}


void SmViewShell::DrawText(OutputDevice& rDevice, const Point& rPosition, const OUString& rText, sal_uInt16 MaxWidth)
{
    if (rText.isEmpty())
        return;

    Point aPoint(rPosition);
    Size aSize;

    sal_Int32 nPos = 0;
    do
    {
        OUString aLine = rText.getToken(0, '\n', nPos);
        aLine = comphelper::string::remove(aLine, '\r');
        aSize = GetTextLineSize(rDevice, aLine);
        if (aSize.Width() > MaxWidth)
        {
            do
            {
                OUString aText;
                sal_Int32 m = aLine.getLength();
                sal_Int32 nLen = m;

                for (sal_Int32 n = 0; n < nLen; n++)
                {
                    sal_Unicode cLineChar = aLine[n];
                    if ((cLineChar == ' ') || (cLineChar == '\t'))
                    {
                        aText = aLine.copy(0, n);
                        if (GetTextLineSize(rDevice, aText).Width() < MaxWidth)
                            m = n;
                        else
                            break;
                    }
                }
                aText = aLine.copy(0, m);
                aLine = aLine.replaceAt(0, m, "");

                DrawTextLine(rDevice, aPoint, aText);
                aPoint.Y() += aSize.Height();

                aLine = comphelper::string::stripStart(aLine, ' ');
                aLine = comphelper::string::stripStart(aLine, '\t');
                aLine = comphelper::string::stripStart(aLine, ' ');
            }
            while (GetTextLineSize(rDevice, aLine).Width() > MaxWidth);

            // print the remaining text
            if (!aLine.isEmpty())
            {
                DrawTextLine(rDevice, aPoint, aLine);
                aPoint.Y() += aSize.Height();
            }
        }
        else
        {
            DrawTextLine(rDevice, aPoint, aLine);
            aPoint.Y() += aSize.Height();
        }
    }
    while ( nPos >= 0 );
}

void SmViewShell::Impl_Print(OutputDevice &rOutDev, const SmPrintUIOptions &rPrintUIOptions, Rectangle aOutRect, Point aZeroPoint )
{
    const bool bIsPrintTitle = rPrintUIOptions.getBoolValue( PRTUIOPT_TITLE_ROW, true );
    const bool bIsPrintFrame = rPrintUIOptions.getBoolValue( PRTUIOPT_BORDER, true );
    const bool bIsPrintFormulaText = rPrintUIOptions.getBoolValue( PRTUIOPT_FORMULA_TEXT, true );
    SmPrintSize ePrintSize( static_cast< SmPrintSize >( rPrintUIOptions.getIntValue( PRTUIOPT_PRINT_FORMAT, PRINT_SIZE_NORMAL ) ));
    const sal_uInt16 nZoomFactor = static_cast< sal_uInt16 >(rPrintUIOptions.getIntValue( PRTUIOPT_PRINT_SCALE, 100 ));

    rOutDev.Push();
    rOutDev.SetLineColor( Color(COL_BLACK) );

    // output text on top
    if (bIsPrintTitle)
    {
        Size aSize600 (0, 600);
        Size aSize650 (0, 650);
        vcl::Font aFont(FAMILY_DONTKNOW, aSize600);

        aFont.SetAlign(ALIGN_TOP);
        aFont.SetWeight(WEIGHT_BOLD);
        aFont.SetSize(aSize650);
        aFont.SetColor( Color(COL_BLACK) );
        rOutDev.SetFont(aFont);

        Size aTitleSize (GetTextSize(rOutDev, GetDoc()->GetTitle(), aOutRect.GetWidth() - 200));

        aFont.SetWeight(WEIGHT_NORMAL);
        aFont.SetSize(aSize600);
        rOutDev.SetFont(aFont);

        Size aDescSize (GetTextSize(rOutDev, GetDoc()->GetComment(), aOutRect.GetWidth() - 200));

        if (bIsPrintFrame)
            rOutDev.DrawRect(Rectangle(aOutRect.TopLeft(),
                               Size(aOutRect.GetWidth(), 100 + aTitleSize.Height() + 200 + aDescSize.Height() + 100)));
        aOutRect.Top() += 200;

        // output title
        aFont.SetWeight(WEIGHT_BOLD);
        aFont.SetSize(aSize650);
        rOutDev.SetFont(aFont);
        Point aPoint(aOutRect.Left() + (aOutRect.GetWidth() - aTitleSize.Width())  / 2,
                     aOutRect.Top());
        DrawText(rOutDev, aPoint, GetDoc()->GetTitle(),
                 sal::static_int_cast< sal_uInt16 >(aOutRect.GetWidth() - 200));
        aOutRect.Top() += aTitleSize.Height() + 200;

        // output description
        aFont.SetWeight(WEIGHT_NORMAL);
        aFont.SetSize(aSize600);
        rOutDev.SetFont(aFont);
        aPoint.X() = aOutRect.Left() + (aOutRect.GetWidth()  - aDescSize.Width())  / 2;
        aPoint.Y() = aOutRect.Top();
        DrawText(rOutDev, aPoint, GetDoc()->GetComment(),
                 sal::static_int_cast< sal_uInt16 >(aOutRect.GetWidth() - 200));
        aOutRect.Top() += aDescSize.Height() + 300;
    }

    // output text on bottom
    if (bIsPrintFormulaText)
    {
        vcl::Font aFont(FAMILY_DONTKNOW, Size(0, 600));
        aFont.SetAlign(ALIGN_TOP);
        aFont.SetColor( Color(COL_BLACK) );

        // get size
        rOutDev.SetFont(aFont);

        Size aSize (GetTextSize(rOutDev, GetDoc()->GetText(), aOutRect.GetWidth() - 200));

        aOutRect.Bottom() -= aSize.Height() + 600;

        if (bIsPrintFrame)
            rOutDev.DrawRect(Rectangle(aOutRect.BottomLeft(),
                               Size(aOutRect.GetWidth(), 200 + aSize.Height() + 200)));

        Point aPoint (aOutRect.Left() + (aOutRect.GetWidth()  - aSize.Width())  / 2,
                      aOutRect.Bottom() + 300);
        DrawText(rOutDev, aPoint, GetDoc()->GetText(),
                 sal::static_int_cast< sal_uInt16 >(aOutRect.GetWidth() - 200));
        aOutRect.Bottom() -= 200;
    }

    if (bIsPrintFrame)
        rOutDev.DrawRect(aOutRect);

    aOutRect.Top()    += 100;
    aOutRect.Left()   += 100;
    aOutRect.Bottom() -= 100;
    aOutRect.Right()  -= 100;

    Size aSize (GetDoc()->GetSize());

    MapMode    OutputMapMode;
    // PDF export should always use PRINT_SIZE_NORMAL ...
    if (!rPrintUIOptions.getBoolValue( "IsPrinter" ) )
        ePrintSize = PRINT_SIZE_NORMAL;
    switch (ePrintSize)
    {
        case PRINT_SIZE_NORMAL:
            OutputMapMode = MapMode(MAP_100TH_MM);
            break;

        case PRINT_SIZE_SCALED:
            if ((aSize.Width() > 0) && (aSize.Height() > 0))
            {
                Size     OutputSize (rOutDev.LogicToPixel(Size(aOutRect.GetWidth(),
                                                            aOutRect.GetHeight()), MapMode(MAP_100TH_MM)));
                Size     GraphicSize (rOutDev.LogicToPixel(aSize, MapMode(MAP_100TH_MM)));
                sal_uInt16   nZ = (sal_uInt16) std::min((long)Fraction(OutputSize.Width()  * 100L, GraphicSize.Width()),
                                              (long)Fraction(OutputSize.Height() * 100L, GraphicSize.Height()));
                Fraction aFraction ((sal_uInt16) std::max ((sal_uInt16) MINZOOM, std::min((sal_uInt16) MAXZOOM, (sal_uInt16) (nZ - 10))), (sal_uInt16) 100);

                OutputMapMode = MapMode(MAP_100TH_MM, aZeroPoint, aFraction, aFraction);
            }
            else
                OutputMapMode = MapMode(MAP_100TH_MM);
            break;

        case PRINT_SIZE_ZOOMED:
        {
            Fraction aFraction( nZoomFactor, 100 );

            OutputMapMode = MapMode(MAP_100TH_MM, aZeroPoint, aFraction, aFraction);
            break;
        }
    }

    aSize = rOutDev.PixelToLogic(rOutDev.LogicToPixel(aSize, OutputMapMode),
                                   MapMode(MAP_100TH_MM));

    Point aPos (aOutRect.Left() + (aOutRect.GetWidth()  - aSize.Width())  / 2,
                aOutRect.Top()  + (aOutRect.GetHeight() - aSize.Height()) / 2);

    aPos     = rOutDev.PixelToLogic(rOutDev.LogicToPixel(aPos, MapMode(MAP_100TH_MM)),
                                          OutputMapMode);
    aOutRect   = rOutDev.PixelToLogic(rOutDev.LogicToPixel(aOutRect, MapMode(MAP_100TH_MM)),
                                          OutputMapMode);

    rOutDev.SetMapMode(OutputMapMode);
    rOutDev.SetClipRegion(vcl::Region(aOutRect));
    GetDoc()->DrawFormula(rOutDev, aPos);
    rOutDev.SetClipRegion();

    rOutDev.Pop();
}

SfxPrinter* SmViewShell::GetPrinter(bool bCreate)
{
    SmDocShell* pDoc = GetDoc();
    if (pDoc->HasPrinter() || bCreate)
        return pDoc->GetPrinter();
    return nullptr;
}

sal_uInt16 SmViewShell::SetPrinter(SfxPrinter *pNewPrinter, SfxPrinterChangeFlags nDiffFlags, bool )
{
    SfxPrinter *pOld = GetDoc()->GetPrinter();
    if ( pOld && pOld->IsPrinting() )
        return SFX_PRINTERROR_BUSY;

    if ((nDiffFlags & SfxPrinterChangeFlags::PRINTER) == SfxPrinterChangeFlags::PRINTER)
        GetDoc()->SetPrinter( pNewPrinter );

    if ((nDiffFlags & SfxPrinterChangeFlags::OPTIONS) == SfxPrinterChangeFlags::OPTIONS)
    {
        SmModule *pp = SM_MOD();
        pp->GetConfig()->ItemSetToConfig(pNewPrinter->GetOptions());
    }
    return 0;
}

bool SmViewShell::HasPrintOptionsPage() const
{
    return true;
}

VclPtr<SfxTabPage> SmViewShell::CreatePrintOptionsPage(vcl::Window *pParent,
                                                       const SfxItemSet &rOptions)
{
    return SmPrintOptionsTabPage::Create(pParent, rOptions);
}

SmEditWindow *SmViewShell::GetEditWindow()
{
    SmCmdBoxWrapper* pWrapper = static_cast<SmCmdBoxWrapper*>(
                                    GetViewFrame()->GetChildWindow(SmCmdBoxWrapper::GetChildWindowId()));

    if (pWrapper != nullptr)
    {
        SmEditWindow& rEditWin = pWrapper->GetEditWindow();
        return &rEditWin;
    }

    return nullptr;
}

void SmViewShell::SetStatusText(const OUString& rText)
{
    aStatusText = rText;
    GetViewFrame()->GetBindings().Invalidate(SID_TEXTSTATUS);
}

void SmViewShell::ShowError(const SmErrorDesc* pErrorDesc)
{
    SAL_WARN_IF( !GetDoc(), "starmath", "Document missing" );
    if (pErrorDesc || nullptr != (pErrorDesc = GetDoc()->GetParser().GetError(0)) )
    {
        SetStatusText( pErrorDesc->m_aText );
        GetEditWindow()->MarkError( Point( pErrorDesc->m_pNode->GetColumn(),
                                           pErrorDesc->m_pNode->GetRow()));
    }
}

void SmViewShell::NextError()
{
    SAL_WARN_IF( !GetDoc(), "starmath", "Document missing" );
    const SmErrorDesc   *pErrorDesc = GetDoc()->GetParser().NextError();

    if (pErrorDesc)
        ShowError( pErrorDesc );
}

void SmViewShell::PrevError()
{
    SAL_WARN_IF( !GetDoc(), "starmath", "Document missing" );
    const SmErrorDesc   *pErrorDesc = GetDoc()->GetParser().PrevError();

    if (pErrorDesc)
        ShowError( pErrorDesc );
}

void SmViewShell::Insert( SfxMedium& rMedium )
{
    SmDocShell *pDoc = GetDoc();
    bool bRet = false;

    uno::Reference <embed::XStorage> xStorage = rMedium.GetStorage();
    uno::Reference <container::XNameAccess> xNameAccess(xStorage, uno::UNO_QUERY);
    if (xNameAccess.is() && xNameAccess->getElementNames().getLength())
    {
        if (xNameAccess->hasByName("content.xml") || xNameAccess->hasByName("Content.xml"))
        {
            // is this a fabulous math package ?
            Reference<css::frame::XModel> xModel(pDoc->GetModel());
            SmXMLImportWrapper aEquation(xModel);    //!! modifies the result of pDoc->GetText() !!
            bRet = 0 == aEquation.Import(rMedium);
        }
    }

    if (bRet)
    {
        OUString aText = pDoc->GetText();
        SmEditWindow *pEditWin = GetEditWindow();
        if (pEditWin)
            pEditWin->InsertText( aText );
        else
        {
            SAL_WARN( "starmath", "EditWindow missing" );
        }

        pDoc->Parse();
        pDoc->SetModified(true);

        SfxBindings &rBnd = GetViewFrame()->GetBindings();
        rBnd.Invalidate(SID_GAPHIC_SM);
        rBnd.Invalidate(SID_TEXT);
    }
}

void SmViewShell::InsertFrom(SfxMedium &rMedium)
{
    bool bSuccess = false;
    SmDocShell* pDoc = GetDoc();
    SvStream* pStream = rMedium.GetInStream();

    if (pStream)
    {
        const OUString& rFltName = rMedium.GetFilter()->GetFilterName();
        if ( rFltName == MATHML_XML )
        {
            Reference<css::frame::XModel> xModel(pDoc->GetModel());
            SmXMLImportWrapper aEquation(xModel);    //!! modifies the result of pDoc->GetText() !!
            bSuccess = 0 == aEquation.Import(rMedium);
        }
    }

    if (bSuccess)
    {
        OUString aText = pDoc->GetText();
        SmEditWindow *pEditWin = GetEditWindow();
        if (pEditWin)
            pEditWin->InsertText(aText);
        else
            SAL_WARN( "starmath", "EditWindow missing" );

        pDoc->Parse();
        pDoc->SetModified(true);

        SfxBindings& rBnd = GetViewFrame()->GetBindings();
        rBnd.Invalidate(SID_GAPHIC_SM);
        rBnd.Invalidate(SID_TEXT);
    }
}

void SmViewShell::Execute(SfxRequest& rReq)
{
    SmEditWindow *pWin = GetEditWindow();

    switch (rReq.GetSlot())
    {
        case SID_FORMULACURSOR:
        {
            SmModule *pp = SM_MOD();

            const SfxItemSet  *pArgs = rReq.GetArgs();
            const SfxPoolItem *pItem;

            bool  bVal;
            if ( pArgs &&
                 SfxItemState::SET == pArgs->GetItemState( SID_FORMULACURSOR, false, &pItem))
                bVal = static_cast<const SfxBoolItem *>(pItem)->GetValue();
            else
                bVal = !pp->GetConfig()->IsShowFormulaCursor();

            pp->GetConfig()->SetShowFormulaCursor(bVal);
            if (!IsInlineEditEnabled())
                GetGraphicWindow().ShowCursor(bVal);
            break;
        }
        case SID_DRAW:
            if (pWin)
            {
                GetDoc()->SetText( pWin->GetText() );
                SetStatusText(OUString());
                ShowError( nullptr );
                GetDoc()->Repaint();
            }
            break;

        case SID_ZOOM_OPTIMAL:
            aGraphic->ZoomToFitInWindow();
            break;

        case SID_ZOOMIN:
            aGraphic->SetZoom(aGraphic->GetZoom() + 25);
            break;

        case SID_ZOOMOUT:
            SAL_WARN_IF( aGraphic->GetZoom() < 25, "starmath", "incorrect sal_uInt16 argument" );
            aGraphic->SetZoom(aGraphic->GetZoom() - 25);
            break;

        case SID_COPYOBJECT:
        {
            //TODO/LATER: does not work because of UNO Tunneling - will be fixed later
            Reference< datatransfer::XTransferable > xTrans( GetDoc()->GetModel(), uno::UNO_QUERY );
            if( xTrans.is() )
            {
                Reference< lang::XUnoTunnel> xTnnl( xTrans, uno::UNO_QUERY);
                if( xTnnl.is() )
                {
                    TransferableHelper* pTrans = reinterpret_cast< TransferableHelper * >(
                            sal::static_int_cast< sal_uIntPtr >(
                            xTnnl->getSomething( TransferableHelper::getUnoTunnelId() )));
                    if( pTrans )
                        pTrans->CopyToClipboard(GetEditWindow());
                }
            }
        }
        break;

        case SID_PASTEOBJECT:
        {
            TransferableDataHelper aData( TransferableDataHelper::CreateFromSystemClipboard(GetEditWindow()) );
            uno::Reference < io::XInputStream > xStrm;
            SotClipboardFormatId nId;
            if( aData.GetTransferable().is() &&
                ( aData.HasFormat( nId = SotClipboardFormatId::EMBEDDED_OBJ ) ||
                  (aData.HasFormat( SotClipboardFormatId::OBJECTDESCRIPTOR ) &&
                   aData.HasFormat( nId = SotClipboardFormatId::EMBED_SOURCE ))))
                xStrm = aData.GetInputStream(nId, OUString());

            if (xStrm.is())
            {
                try
                {
                    uno::Reference < embed::XStorage > xStorage =
                            ::comphelper::OStorageHelper::GetStorageFromInputStream( xStrm, ::comphelper::getProcessComponentContext() );
                    uno::Reference < beans::XPropertySet > xProps( xStorage, uno::UNO_QUERY );
                    SfxMedium aMedium( xStorage, OUString() );
                    Insert( aMedium );
                    GetDoc()->UpdateText();
                }
                catch (uno::Exception &)
                {
                    SAL_WARN( "starmath", "SmViewShell::Execute (SID_PASTEOBJECT): failed to get storage from input stream" );
                }
            }
        }
        break;


        case SID_CUT:
            if (pWin)
                pWin->Cut();
            break;

        case SID_COPY:
            if (pWin)
            {
                if (pWin->IsAllSelected())
                {
                    GetViewFrame()->GetDispatcher()->Execute(
                                SID_COPYOBJECT, SfxCallMode::RECORD,
                                new SfxVoidItem(SID_COPYOBJECT), 0L);
                }
                else
                    pWin->Copy();
            }
            break;

        case SID_PASTE:
            {
                bool bCallExec = nullptr == pWin;
                if( !bCallExec )
                {
                    TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                    GetEditWindow()) );

                    if( aDataHelper.GetTransferable().is() &&
                        aDataHelper.HasFormat( SotClipboardFormatId::STRING ))
                        pWin->Paste();
                    else
                        bCallExec = true;
                }
                if( bCallExec )
                {
                    GetViewFrame()->GetDispatcher()->Execute(
                            SID_PASTEOBJECT, SfxCallMode::RECORD,
                            new SfxVoidItem(SID_PASTEOBJECT), 0L);
                }
            }
            break;

        case SID_DELETE:
            if (pWin)
                pWin->Delete();
            break;

        case SID_SELECT:
            if (pWin)
                pWin->SelectAll();
            break;

        case SID_INSERTCOMMAND:
        {
            const SfxInt16Item& rItem =
                static_cast<const SfxInt16Item&>(rReq.GetArgs()->Get(SID_INSERTCOMMAND));

            if (pWin && (bInsertIntoEditWindow || !IsInlineEditEnabled()))
                pWin->InsertCommand(rItem.GetValue());
            if (IsInlineEditEnabled() && (GetDoc() && !bInsertIntoEditWindow)) {
                GetDoc()->GetCursor().InsertCommand(rItem.GetValue());
                GetGraphicWindow().GrabFocus();
            }
            break;
        }

        case SID_INSERTCOMMANDTEXT:
        {
            const SfxStringItem& rItem = static_cast<const SfxStringItem&>(rReq.GetArgs()->Get(SID_INSERTCOMMANDTEXT));

            if (pWin && (bInsertIntoEditWindow || !IsInlineEditEnabled()))
            {
                pWin->InsertText(rItem.GetValue());
            }
            if (IsInlineEditEnabled() && (GetDoc() && !bInsertIntoEditWindow))
            {
                GetDoc()->GetCursor().InsertCommandText(rItem.GetValue());
                GetGraphicWindow().GrabFocus();
            }
            break;

        }

        case SID_INSERTSYMBOL:
        {
            const SfxStringItem& rItem =
                static_cast<const SfxStringItem&>(rReq.GetArgs()->Get(SID_INSERTSYMBOL));

            if (pWin && (bInsertIntoEditWindow || !IsInlineEditEnabled()))
                pWin->InsertText(rItem.GetValue());
            if (IsInlineEditEnabled() && (GetDoc() && !bInsertIntoEditWindow))
                GetDoc()->GetCursor().InsertSpecial(rItem.GetValue());
            break;
        }

        case SID_IMPORT_FORMULA:
        {
            pImpl->pRequest.reset(new SfxRequest( rReq ));
            pImpl->pDocInserter.reset(new ::sfx2::DocumentInserter(
                              GetDoc()->GetFactory().GetFactoryName(), false ));
            pImpl->pDocInserter->StartExecuteModal( LINK( this, SmViewShell, DialogClosedHdl ) );
            break;
        }

        case SID_IMPORT_MATHML_CLIPBOARD:
        {
            TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard(GetEditWindow()) );
            uno::Reference < io::XInputStream > xStrm;
            SotClipboardFormatId nId = SOT_FORMAT_SYSTEM_START; //dummy initialize to avoid warning
            if  ( aDataHelper.GetTransferable().is() )
            {
                if (aDataHelper.HasFormat(nId = SotClipboardFormatId::MATHML))
                {
                    xStrm = aDataHelper.GetInputStream(nId, "");
                    if (xStrm.is())
                    {
                        std::unique_ptr<SfxMedium> pClipboardMedium(new SfxMedium());
                        pClipboardMedium->GetItemSet(); //generate initial itemset, not sure if necessary
                        const SfxFilter* pMathFilter =
                            SfxFilter::GetFilterByName(MATHML_XML);
                        pClipboardMedium->SetFilter(pMathFilter);
                        pClipboardMedium->setStreamToLoadFrom(xStrm, true /*bIsReadOnly*/);
                        InsertFrom(*pClipboardMedium);
                        GetDoc()->UpdateText();
                    }
                }
                else
                {
                    if (aDataHelper.HasFormat(nId = SotClipboardFormatId::STRING))
                    {
                        // In case of FORMAT_STRING no stream exists, need to generate one
                        ::rtl::OUString aString;
                        if (aDataHelper.GetString( nId, aString))
                        {
                            std::unique_ptr<SfxMedium> pClipboardMedium(new SfxMedium());
                            pClipboardMedium->GetItemSet(); //generates initial itemset, not sure if necessary
                            const SfxFilter* pMathFilter =
                                SfxFilter::GetFilterByName(MATHML_XML);
                            pClipboardMedium->SetFilter(pMathFilter);

                            std::unique_ptr<SvMemoryStream> pStrm;
                            // The text to be imported might asserts encoding like 'encoding="utf-8"' but FORMAT_STRING is UTF-16.
                            // Force encoding to UTF-16, if encoding exists.
                            bool bForceUTF16 = false;
                            sal_Int32 nPosL = aString.indexOf("encoding=\"");
                            sal_Int32 nPosU = -1;
                            if ( nPosL >= 0 && nPosL +10 < aString.getLength() )
                            {
                                nPosL += 10;
                                nPosU = aString.indexOf( '"',nPosL);
                                if (nPosU > nPosL)
                                {
                                    bForceUTF16 = true;
                                }
                            }
                            if ( bForceUTF16 )
                            {
                                OUString aNewString = aString.replaceAt( nPosL,nPosU-nPosL,"UTF-16");
                                pStrm.reset(new SvMemoryStream( const_cast<sal_Unicode *>(aNewString.getStr()), aNewString.getLength() * sizeof(sal_Unicode), StreamMode::READ));
                            }
                            else
                            {
                                pStrm.reset(new SvMemoryStream( const_cast<sal_Unicode *>(aString.getStr()), aString.getLength() * sizeof(sal_Unicode), StreamMode::READ));
                            }
                            uno::Reference<io::XInputStream> xStrm2( new ::utl::OInputStreamWrapper(*pStrm) );
                            pClipboardMedium->setStreamToLoadFrom(xStrm2, true /*bIsReadOnly*/);
                            InsertFrom(*pClipboardMedium);
                            GetDoc()->UpdateText();
                        }
                    }
                }
            }
            break;
        }

        case SID_NEXTERR:
            NextError();
            if (pWin)
                pWin->GrabFocus();
            break;

        case SID_PREVERR:
            PrevError();
            if (pWin)
                pWin->GrabFocus();
            break;

        case SID_NEXTMARK:
            if (pWin)
            {
                pWin->SelNextMark();
                pWin->GrabFocus();
            }
            break;

        case SID_PREVMARK:
            if (pWin)
            {
                pWin->SelPrevMark();
                pWin->GrabFocus();
            }
            break;

        case SID_TEXTSTATUS:
        {
            if (rReq.GetArgs() != nullptr)
            {
                const SfxStringItem& rItem =
                    static_cast<const SfxStringItem&>(rReq.GetArgs()->Get(SID_TEXTSTATUS));

                SetStatusText(rItem.GetValue());
            }

            break;
        }

        case SID_GETEDITTEXT:
            if (pWin)
                if (!pWin->GetText().isEmpty()) GetDoc()->SetText( pWin->GetText() );
            break;

        case SID_ATTR_ZOOM:
        {
            if ( !GetViewFrame()->GetFrame().IsInPlace() )
            {
                std::unique_ptr<AbstractSvxZoomDialog> xDlg;
                const SfxItemSet *pSet = rReq.GetArgs();
                if ( !pSet )
                {
                    SfxItemSet aSet( SmDocShell::GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
                    aSet.Put( SvxZoomItem( SvxZoomType::PERCENT, aGraphic->GetZoom()));
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    if(pFact)
                    {
                        xDlg.reset(pFact->CreateSvxZoomDialog(&GetViewFrame()->GetWindow(), aSet));
                        SAL_WARN_IF( !xDlg, "starmath", "Dialog creation failed!" );
                        xDlg->SetLimits( MINZOOM, MAXZOOM );
                        if (xDlg->Execute() != RET_CANCEL)
                            pSet = xDlg->GetOutputItemSet();
                    }
                }
                if ( pSet )
                {
                    const SvxZoomItem &rZoom = static_cast<const SvxZoomItem &>(pSet->Get(SID_ATTR_ZOOM));
                    switch( rZoom.GetType() )
                    {
                        case SvxZoomType::PERCENT:
                            aGraphic->SetZoom((sal_uInt16)rZoom.GetValue ());
                            break;

                        case SvxZoomType::OPTIMAL:
                            aGraphic->ZoomToFitInWindow();
                            break;

                        case SvxZoomType::PAGEWIDTH:
                        case SvxZoomType::WHOLEPAGE:
                        {
                            const MapMode aMap( MAP_100TH_MM );
                            SfxPrinter *pPrinter = GetPrinter( true );
                            Point aPoint;
                            Rectangle  OutputRect(aPoint, pPrinter->GetOutputSize());
                            Size       OutputSize(pPrinter->LogicToPixel(Size(OutputRect.GetWidth(),
                                                                              OutputRect.GetHeight()), aMap));
                            Size       GraphicSize(pPrinter->LogicToPixel(GetDoc()->GetSize(), aMap));
                            sal_uInt16     nZ = (sal_uInt16) std::min((long)Fraction(OutputSize.Width()  * 100L, GraphicSize.Width()),
                                                         (long)Fraction(OutputSize.Height() * 100L, GraphicSize.Height()));
                            aGraphic->SetZoom (nZ);
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
        }
        break;

        case SID_ATTR_ZOOMSLIDER:
        {
            const SfxItemSet *pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;

            if ( pArgs && SfxItemState::SET == pArgs->GetItemState(SID_ATTR_ZOOMSLIDER, true, &pItem ) )
            {
                const sal_uInt16 nCurrentZoom = static_cast<const SvxZoomSliderItem *>(pItem)->GetValue();
                aGraphic->SetZoom( nCurrentZoom );
            }
        }
        break;

        case SID_ELEMENTSDOCKINGWINDOW:
        {
            GetViewFrame()->ToggleChildWindow( SmElementsDockingWindowWrapper::GetChildWindowId() );
            GetViewFrame()->GetBindings().Invalidate( SID_ELEMENTSDOCKINGWINDOW );

            rReq.Ignore ();
        }
        break;

        case SID_UNICODE_NOTATION_TOGGLE:
        {
            EditEngine* pEditEngine = nullptr;
            if( pWin )
                pEditEngine = pWin->GetEditEngine();

            EditView* pEditView = nullptr;
            if( pEditEngine )
                pEditView = pEditEngine->GetView();

            if( pEditView )
            {
                const OUString sInput = pEditView->GetSurroundingText();
                ESelection aSel(  pWin->GetSelection() );

                if ( aSel.nStartPos > aSel.nEndPos )
                    aSel.nEndPos = aSel.nStartPos;

                //calculate a valid end-position by reading logical characters
                sal_Int32 nUtf16Pos=0;
                while( (nUtf16Pos < sInput.getLength()) && (nUtf16Pos < aSel.nEndPos) )
                {
                    sInput.iterateCodePoints(&nUtf16Pos);
                    if( nUtf16Pos > aSel.nEndPos )
                        aSel.nEndPos = nUtf16Pos;
                }

                ToggleUnicodeCodepoint aToggle;
                while( nUtf16Pos && aToggle.AllowMoreInput( sInput[nUtf16Pos-1]) )
                    --nUtf16Pos;
                const OUString sReplacement = aToggle.ReplacementString();
                if( !sReplacement.isEmpty() )
                {
                    pEditView->SetSelection( aSel );
                    pEditEngine->UndoActionStart(EDITUNDO_REPLACEALL);
                    aSel.nStartPos = aSel.nEndPos - aToggle.StringToReplace().getLength();
                    pWin->SetSelection( aSel );
                    pEditView->InsertText( sReplacement, true );
                    pEditEngine->UndoActionEnd(EDITUNDO_REPLACEALL);
                    pWin->Flush();
                }
            }
        }
        break;

        case SID_SYMBOLS_CATALOGUE:
        {

            // get device used to retrieve the FontList
            SmDocShell *pDoc = GetDoc();
            OutputDevice *pDev = pDoc->GetPrinter();
            if (!pDev || pDev->GetDevFontCount() == 0)
                pDev = &SM_MOD()->GetDefaultVirtualDev();
            SAL_WARN_IF( !pDev, "starmath", "device for font list missing" );

            SmModule *pp = SM_MOD();
            ScopedVclPtrInstance<SmSymbolDialog>::Create( nullptr, pDev, pp->GetSymbolManager(), *this )->Execute();
        }
        break;
    }
    rReq.Done();
}


void SmViewShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);

    SmEditWindow *pEditWin = GetEditWindow();
    for (sal_uInt16 nWh = aIter.FirstWhich(); nWh != 0; nWh = aIter.NextWhich())
    {
        switch (nWh)
        {
        case SID_CUT:
        case SID_COPY:
        case SID_DELETE:
            if (! pEditWin || ! pEditWin->IsSelected())
                rSet.DisableItem(nWh);
            break;

        case SID_PASTE:
            if (pEditWin)
            {
                TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                        pEditWin) );

                bPasteState = aDataHelper.GetTransferable().is() &&
                 ( aDataHelper.HasFormat( SotClipboardFormatId::STRING ) ||
                   aDataHelper.HasFormat( SotClipboardFormatId::EMBEDDED_OBJ ) ||
                   (aDataHelper.HasFormat( SotClipboardFormatId::OBJECTDESCRIPTOR )
                      && aDataHelper.HasFormat( SotClipboardFormatId::EMBED_SOURCE )));
            }
            if( !bPasteState )
                rSet.DisableItem( nWh );
            break;

        case SID_ATTR_ZOOM:
            rSet.Put(SvxZoomItem( SvxZoomType::PERCENT, aGraphic->GetZoom()));
            SAL_FALLTHROUGH;
        case SID_ZOOMIN:
        case SID_ZOOMOUT:
        case SID_ZOOM_OPTIMAL:
            if ( GetViewFrame()->GetFrame().IsInPlace() )
                rSet.DisableItem( nWh );
            break;

        case SID_ATTR_ZOOMSLIDER :
            {
                const sal_uInt16 nCurrentZoom = aGraphic->GetZoom();
                SvxZoomSliderItem aZoomSliderItem( nCurrentZoom, MINZOOM, MAXZOOM );
                aZoomSliderItem.AddSnappingPoint( 100 );
                rSet.Put( aZoomSliderItem );
            }
        break;

        case SID_NEXTERR:
        case SID_PREVERR:
        case SID_NEXTMARK:
        case SID_PREVMARK:
        case SID_DRAW:
        case SID_SELECT:
            if (! pEditWin || pEditWin->IsEmpty())
                rSet.DisableItem(nWh);
            break;

        case SID_TEXTSTATUS:
            {
                rSet.Put(SfxStringItem(nWh, aStatusText));
            }
            break;

        case SID_FORMULACURSOR:
            {
                SmModule *pp = SM_MOD();
                rSet.Put(SfxBoolItem(nWh, pp->GetConfig()->IsShowFormulaCursor()));
            }
            break;
        case SID_ELEMENTSDOCKINGWINDOW:
            {
                bool bState = false;
                SfxChildWindow *pChildWnd = GetViewFrame()->
                        GetChildWindow( SmElementsDockingWindowWrapper::GetChildWindowId() );
                if (pChildWnd  &&  pChildWnd->GetWindow()->IsVisible())
                    bState = true;
                rSet.Put(SfxBoolItem(SID_ELEMENTSDOCKINGWINDOW, bState));
            }
            break;
        }
    }
}

SmViewShell::SmViewShell(SfxViewFrame *pFrame_, SfxViewShell *)
    : SfxViewShell(pFrame_, SfxViewShellFlags::HAS_PRINTOPTIONS | SfxViewShellFlags::CAN_PRINT)
    , pImpl(new SmViewShell_Impl)
    , aGraphic(VclPtr<SmGraphicWindow>::Create(this))
    , aGraphicController(*aGraphic.get(), SID_GAPHIC_SM, pFrame_->GetBindings())
    , bPasteState(false)
    , bInsertIntoEditWindow(false)
{
    SetStatusText(OUString());
    SetWindow(aGraphic.get());
    SfxShell::SetName("SmView");
    SfxShell::SetUndoManager( &GetDoc()->GetEditEngine().GetUndoManager() );
    SetHelpId( HID_SMA_VIEWSHELL_DOCUMENT );
}


SmViewShell::~SmViewShell()
{
    //!! this view shell is not active anymore !!
    // Thus 'SmGetActiveView' will give a 0 pointer.
    // Thus we need to supply this view as argument
    SmEditWindow *pEditWin = GetEditWindow();
    if (pEditWin)
        pEditWin->DeleteEditView( *this );
    aGraphic.disposeAndClear();
}

void SmViewShell::Deactivate( bool bIsMDIActivate )
{
    SmEditWindow *pEdit = GetEditWindow();
    if ( pEdit )
        pEdit->Flush();

    SfxViewShell::Deactivate( bIsMDIActivate );
}

void SmViewShell::Activate( bool bIsMDIActivate )
{
    SfxViewShell::Activate( bIsMDIActivate );

    SmEditWindow *pEdit = GetEditWindow();
    if ( pEdit )
    {
        //! Since there is no way to be informed if a "drag and drop"
        //! event has taken place, we call SetText here in order to
        //! synchronize the GraphicWindow display with the text in the
        //! EditEngine.
        SmDocShell *pDoc = GetDoc();
        pDoc->SetText( pDoc->GetEditEngine().GetText() );

        if ( bIsMDIActivate )
            pEdit->GrabFocus();
    }
}

IMPL_LINK_TYPED( SmViewShell, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    assert(_pFileDlg && "SmViewShell::DialogClosedHdl(): no file dialog");
    assert(pImpl->pDocInserter && "ScDocShell::DialogClosedHdl(): no document inserter");

    if ( ERRCODE_NONE == _pFileDlg->GetError() )
    {
        SfxMedium* pMedium = pImpl->pDocInserter->CreateMedium();

        if ( pMedium != nullptr )
        {
            if ( pMedium->IsStorage() )
                Insert( *pMedium );
            else
                InsertFrom( *pMedium );
            delete pMedium;

            SmDocShell* pDoc = GetDoc();
            pDoc->UpdateText();
            pDoc->ArrangeFormula();
            pDoc->Repaint();
            // adjust window, repaint, increment ModifyCount,...
            GetViewFrame()->GetBindings().Invalidate(SID_GAPHIC_SM);
        }
    }

    pImpl->pRequest->SetReturnValue( SfxBoolItem( pImpl->pRequest->GetSlot(), true ) );
    pImpl->pRequest->Done();
}

void SmViewShell::Notify( SfxBroadcaster& , const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint )
    {
        switch( pSimpleHint->GetId() )
        {
            case SFX_HINT_MODECHANGED:
            case SFX_HINT_DOCCHANGED:
                GetViewFrame()->GetBindings().InvalidateAll(false);
                break;
            default:
                break;
        }
    }
}

bool SmViewShell::IsInlineEditEnabled() const
{
    return pImpl->aOpts.IsExperimentalMode();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
