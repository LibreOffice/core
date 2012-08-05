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
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>
#include <rtl/logfile.hxx>
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
#include <editeng/editeng.hxx>
#include <svx/svxdlg.hxx>
#include <sfx2/zoomitem.hxx>
#include <vcl/decoview.hxx>
#include <vcl/menu.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include <fstream>

#include "unomodel.hxx"
#include "view.hxx"
#include "config.hxx"
#include "dialog.hxx"
#include "document.hxx"
#include "starmath.hrc"
#include "toolbox.hxx"
#include "mathmlimport.hxx"
#include "cursor.hxx"
#include "accessibility.hxx"

#define MINZOOM         25
#define MAXZOOM         800

#define SmViewShell
#include "smslots.hxx"

using namespace com::sun::star;
using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;

//////////////////////////////////////////////////////////////////////

SmGraphicWindow::SmGraphicWindow(SmViewShell* pShell):
    ScrollableWindow(&pShell->GetViewFrame()->GetWindow(), 0),
    pAccessible(0),
    pViewShell(pShell),
    nZoom(100)
{
    // docking windows are usually hidden (often already done in the
    // resource) and will be shown by the sfx framework.
    Hide();

    const Fraction aFraction (1,1);
    SetMapMode( MapMode(MAP_100TH_MM, Point(), aFraction, aFraction));

    ApplyColorConfigValues( SM_MOD()->GetColorConfig() );

    SetTotalSize();

    SetHelpId(HID_SMA_WIN_DOCUMENT);
    SetUniqueId(HID_SMA_WIN_DOCUMENT);

    ShowLine(false);
    CaretBlinkInit();
}

SmGraphicWindow::~SmGraphicWindow()
{
    if (pAccessible)
        pAccessible->ClearWin();    // make Accessible defunctional
    // Note: memory for pAccessible will be freed when the reference
    // xAccessible is released.
    CaretBlinkStop();
}

void SmGraphicWindow::StateChanged( StateChangedType eType )
{
    if ( eType == STATE_CHANGE_INITSHOW )
        Show();
    ScrollableWindow::StateChanged( eType );
}


void SmGraphicWindow::ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg )
{
    // Note: SetTextColor not necessary since the nodes that
    // get painted have the color information.
#if OSL_DEBUG_LEVEL > 1
//   ColorData nVal = rColorCfg.GetColorValue(svtools::DOCCOLOR).nColor;
#endif
    SetBackground( Color( (ColorData) rColorCfg.GetColorValue(svtools::DOCCOLOR).nColor ) );
    Invalidate();
}


void SmGraphicWindow::DataChanged( const DataChangedEvent& rEvt )
{
    ApplyColorConfigValues( SM_MOD()->GetColorConfig() );

    ScrollableWindow::DataChanged( rEvt );
}


void SmGraphicWindow::MouseButtonDown(const MouseEvent& rMEvt)
{
    ScrollableWindow::MouseButtonDown(rMEvt);

    GrabFocus();

    //
    // set formula-cursor and selection of edit window according to the
    // position clicked at
    //
    OSL_ENSURE(rMEvt.GetClicks() > 0, "Sm : 0 clicks");
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
        const SmNode *pNode = 0;
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
                aSel.nEndPos = aSel.nEndPos + sal::static_int_cast< sal_uInt16 >(aToken.aText.Len());

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
    SmDocShell &rDoc = *pViewShell->GetDoc();
    rDoc.Repaint();
}

IMPL_LINK_NOARG(SmGraphicWindow, CaretBlinkTimerHdl)
{
    if (IsCursorVisible())
        SetIsCursorVisible(false);
    else
        SetIsCursorVisible(true);

    RepaintViewShellDoc();

    return 0;
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
    if ( aCaretBlinkTimer.GetTimeout() != STYLE_CURSOR_NOBLINKTIME )
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
    Point aBRPos  (aTLPos.X() + aSize.Width(), aTLPos.Y() + aSize.Height());

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
    // looks for a VISIBLE node in the formula tree with it's token at
    // (or around) the position 'nRow', 'nCol' in the edit window
    // (row and column numbering starts with 1 there!).
    // If there is such a node the formula-cursor is set to cover that nodes
    // rectangle. If not the formula-cursor will be hidden.
    // In any case the search result is being returned.
{
    if (IsInlineEditEnabled())
        return NULL;

    // find visible node with token at nRow, nCol
    const SmNode *pTree = pViewShell->GetDoc()->GetFormulaTree(),
                 *pNode = 0;
    if (pTree)
        pNode = pTree->FindTokenAt(nRow, nCol);

    if (pNode)
        SetCursor(pNode);
    else
        ShowCursor(false);

    return pNode;
}

void SmGraphicWindow::Paint(const Rectangle&)
{
    OSL_ENSURE(pViewShell, "Sm : NULL pointer");

    SmDocShell &rDoc = *pViewShell->GetDoc();
    Point aPoint;

    rDoc.DrawFormula(*this, aPoint, true);  //! modifies aPoint to be the topleft
                                //! corner of the formula
    SetFormulaDrawPos(aPoint);
    if(IsInlineEditEnabled()) {
        //Draw cursor if any...
        if(pViewShell->GetDoc()->HasCursor() && IsLineVisible())
            pViewShell->GetDoc()->GetCursor().Draw(*this, aPoint, IsCursorVisible());
    } else {
    SetIsCursorVisible(false);  // (old) cursor must be drawn again

    const SmEditWindow *pEdit = pViewShell->GetEditWindow();
    if (pEdit)
    {   // get new position for formula-cursor (for possible altered formula)
        sal_uInt16  nRow, nCol;
        SmGetLeftSelectionPart(pEdit->GetSelection(), nRow, nCol);
        nRow++;
        nCol++;
        const SmNode *pFound = SetCursorPos(nRow, nCol);

        SmModule  *pp = SM_MOD();
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
    if (eFunc == KEYFUNC_COPY)
        rCursor.Copy();
    else if (eFunc == KEYFUNC_CUT)
        rCursor.Cut();
    else if (eFunc == KEYFUNC_PASTE)
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
                String label(pViewShell->GetDoc()->GetText());
                pTree->DumpAsDot(file, &label);
                file.close();
            }
#endif /* DEBUG_ENABLE_DUMPASDOT */
        }break;
        case KEY_DELETE:
        {
            if(!rCursor.HasSelection()){
                rCursor.Move(this, nCode == KEY_DELETE ? MoveRight : MoveLeft, false);
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
            SmBraceNode* pBraceNode = NULL;

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
                    rCursor.InsertText(rtl::OUString(code));
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
            case COMMAND_CONTEXTMENU:
            {
                GetParent()->ToTop();
                SmResId aResId( RID_VIEWMENU );
                PopupMenu* pPopupMenu = new PopupMenu(aResId);
                pPopupMenu->SetSelectHdl(LINK(this, SmGraphicWindow, MenuSelectHdl));
                Point aPos(5, 5);
                if (rCEvt.IsMouseEvent())
                    aPos = rCEvt.GetMousePosPixel();
                OSL_ENSURE( pViewShell, "view shell missing" );

                // added for replaceability of context menus
                pViewShell->GetViewFrame()->GetBindings().GetDispatcher()
                        ->ExecutePopup( aResId, this, &aPos );

                delete pPopupMenu;
                bCallBase = false;
            }
            break;

            case COMMAND_WHEEL:
            {
                const CommandWheelData* pWData = rCEvt.GetWheelData();
                if  ( pWData && COMMAND_WHEEL_ZOOM == pWData->GetMode() )
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
        }
    }
    if ( bCallBase )
        ScrollableWindow::Command (rCEvt);
}


IMPL_LINK_INLINE_START( SmGraphicWindow, MenuSelectHdl, Menu *, pMenu )
{
    SmViewShell *pViewSh = GetView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetDispatcher()->Execute( pMenu->GetCurItemId() );
    return 0;
}
IMPL_LINK_INLINE_END( SmGraphicWindow, MenuSelectHdl, Menu *, pMenu )


void SmGraphicWindow::SetZoom(sal_uInt16 Factor)
{
    nZoom = Min(Max((sal_uInt16) Factor, (sal_uInt16) MINZOOM), (sal_uInt16) MAXZOOM);
    Fraction   aFraction (nZoom, 100);
    SetMapMode( MapMode(MAP_100TH_MM, Point(), aFraction, aFraction) );
    SetTotalSize();
    SmViewShell *pViewSh = GetView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetBindings().Invalidate(SID_ATTR_ZOOM);
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
        long nVal = Min ((85 * aWindowSize.Width())  / aSize.Width(),
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
    const SfxStringItem *pItem = PTR_CAST(SfxStringItem, pState);

    if ((pItem != NULL) && (rEdit.GetText() != pItem->GetValue()))
        rEdit.SetText(pItem->GetValue());
    SfxControllerItem::StateChanged (nSID, eState, pState);
}


/**************************************************************************/

SmCmdBoxWindow::SmCmdBoxWindow(SfxBindings *pBindings_, SfxChildWindow *pChildWindow,
                               Window *pParent) :
    SfxDockingWindow(pBindings_, pChildWindow, pParent, SmResId(RID_CMDBOXWINDOW)),
    aEdit       (*this),
    aController (aEdit, SID_TEXT, *pBindings_),
    bExiting    (false)
{
    Hide ();

    aInitialFocusTimer.SetTimeoutHdl(LINK(this, SmCmdBoxWindow, InitialFocusTimerHdl));
    aInitialFocusTimer.SetTimeout(100);
}


SmCmdBoxWindow::~SmCmdBoxWindow ()
{
    aInitialFocusTimer.Stop();
    bExiting = true;
}


SmViewShell * SmCmdBoxWindow::GetView()
{
    SfxDispatcher *pDispatcher = GetBindings().GetDispatcher();
    SfxViewShell *pView = pDispatcher ? pDispatcher->GetFrame()->GetViewShell() : NULL;
    return PTR_CAST(SmViewShell, pView);
}

void SmCmdBoxWindow::Resize()
{
    Rectangle aRect = Rectangle(Point(0, 0), GetOutputSizePixel());
    DecorationView aView(this);
    aRect.Left() += 8; aRect.Top()   += 8;
    aRect.Right()-= 8; aRect.Bottom()-= 8;
    aRect = aView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN | FRAME_DRAW_NODRAW );

    aEdit.SetPosSizePixel(aRect.TopLeft(), aRect.GetSize());
    SfxDockingWindow::Resize();
    Invalidate();
}


void SmCmdBoxWindow::Paint(const Rectangle& /*rRect*/)
{
    Rectangle aRect = Rectangle(Point(0, 0), GetOutputSizePixel());
    DecorationView aView(this);
    aRect.Left() += 8; aRect.Top()   += 8;
    aRect.Right()-= 8; aRect.Bottom()-= 8;
    aRect = aView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );
}


Size SmCmdBoxWindow::CalcDockingSize(SfxChildAlignment eAlign)
{
    switch (eAlign)
    {
        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
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
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_BOTTOM:
        case SFX_ALIGN_NOALIGNMENT:
            return eWish;
        default:
            break;
    }

    return eActual;
}


void SmCmdBoxWindow::StateChanged( StateChangedType nStateChange )
{
    if (STATE_CHANGE_INITSHOW == nStateChange)
    {
        Resize();   // avoid SmEditWindow not being painted correctly

        // set initial position of window in floating mode
        if (IsFloatingMode())
            AdjustPosition();   //! don't change pos in docking-mode !

        aInitialFocusTimer.Start();
    }

    SfxDockingWindow::StateChanged( nStateChange );
}


IMPL_LINK( SmCmdBoxWindow, InitialFocusTimerHdl, Timer *, EMPTYARG /*pTimer*/ )
{
    // We want to have the focus in the edit window once Math has been opened
    // to allow for immediate typing.
    // Problem: There is no proper way to do this
    // Thus: this timer based soultion has been implemented (see GrabFocus below)
    //
    // Follow-up problem (#i114910): grabing the focus may bust the help system since
    // it relies on getting the current frame which conflicts with grabbing the focus.
    // Thus aside from the 'GrabFocus' call everything else is to get the
    // help reliably working despite using 'GrabFocus'.

    try
    {
        uno::Reference< frame::XDesktop > xDesktop;
        uno::Reference< lang::XMultiServiceFactory > xSMGR( comphelper::getProcessServiceFactory() );
        if (xSMGR.is())
        {
            xDesktop = uno::Reference< frame::XDesktop >(
                xSMGR->createInstance( "com.sun.star.frame.Desktop"), uno::UNO_QUERY_THROW );
        }

        aEdit.GrabFocus();

        if (xDesktop.is())
        {
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
                uno::Reference< frame::XFramesSupplier > xFramesSupplier( xDesktop, uno::UNO_QUERY );
                xFramesSupplier->setActiveFrame( xFrame );
            }
        }
    }
    catch (uno::Exception &)
    {
        OSL_FAIL( "failed to properly set initial focus to edit window" );
    }
    return 0;
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
        aEdit.GrabFocus();
}

/**************************************************************************/


SFX_IMPL_DOCKINGWINDOW_WITHID(SmCmdBoxWrapper, SID_CMDBOXWINDOW);

SmCmdBoxWrapper::SmCmdBoxWrapper(Window *pParentWindow, sal_uInt16 nId,
                                 SfxBindings *pBindings,
                                 SfxChildWinInfo *pInfo) :
    SfxChildWindow(pParentWindow, nId)
{
    pWindow = new SmCmdBoxWindow(pBindings, this, pParentWindow);

    // make window docked to the bottom initially (after first start)
    eChildAlignment = SFX_ALIGN_BOTTOM;
    ((SfxDockingWindow *)pWindow)->Initialize(pInfo);
}


#if OSL_DEBUG_LEVEL > 1
SmCmdBoxWrapper::~SmCmdBoxWrapper()
{
}
#endif


/**************************************************************************/

struct SmViewShell_Impl
{
    sfx2::DocumentInserter* pDocInserter;
    SfxRequest*             pRequest;
    SvtMiscOptions          aOpts;

    SmViewShell_Impl() :
          pDocInserter( NULL )
        , pRequest( NULL )
    {}

    ~SmViewShell_Impl()
    {
        delete pDocInserter;
        delete pRequest;
    }
};

TYPEINIT1( SmViewShell, SfxViewShell );

SFX_IMPL_INTERFACE(SmViewShell, SfxViewShell, SmResId(0))
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                SmResId(RID_MATH_TOOLBOX ));
    //Dummy-Objectbar, to avoid quiver while activating

    SFX_CHILDWINDOW_REGISTRATION(SID_TASKPANE);
    SFX_CHILDWINDOW_REGISTRATION(SmToolBoxWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(SmCmdBoxWrapper::GetChildWindowId());
}


SFX_IMPL_NAMED_VIEWFACTORY(SmViewShell, "Default")
{
    SFX_VIEW_REGISTRATION(SmDocShell);
}


Size SmViewShell::GetOptimalSizePixel() const
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::GetOptimalSizePixel" );

    return aGraphic.LogicToPixel( ((SmViewShell*)this)->GetDoc()->GetSize() );
}


void SmViewShell::AdjustPosSizePixel(const Point &rPos, const Size &rSize)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::AdjustPosSizePixel" );

    aGraphic.SetPosSizePixel(rPos, rSize);
}


void SmViewShell::InnerResizePixel(const Point &rOfs, const Size &rSize)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::InnerResizePixel" );

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
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::OuterResizePixel" );

    SmGraphicWindow &rWin = GetGraphicWindow();
    rWin.SetPosSizePixel(rOfs, rSize);
    if (GetDoc()->IsPreview())
        rWin.ZoomToFitInWindow();
    rWin.Update();
}


void SmViewShell::QueryObjAreaPixel( Rectangle& rRect ) const
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::QueryObjAreaPixel" );

    rRect.SetSize( GetGraphicWindow().GetSizePixel() );
}


void SmViewShell::SetZoomFactor( const Fraction &rX, const Fraction &rY )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::SetZoomFactor" );

    const Fraction &rFrac = rX < rY ? rX : rY;
    GetGraphicWindow().SetZoom( (sal_uInt16) long(rFrac * Fraction( 100, 1 )) );

    //To avoid rounding errors base class regulates crooked values too
    //if necessary
    SfxViewShell::SetZoomFactor( rX, rY );
}


Size SmViewShell::GetTextLineSize(OutputDevice& rDevice, const String& rLine)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::GetTextLineSize" );

    String aText;
    Size   aSize(rDevice.GetTextWidth(rLine), rDevice.GetTextHeight());
    sal_uInt16 nTabs = comphelper::string::getTokenCount(rLine, '\t');

    if (nTabs > 0)
    {
        long TabPos = rDevice.GetTextWidth(rtl::OUString('n')) * 8;

        aSize.Width() = 0;

        for (sal_uInt16 i = 0; i < nTabs; i++)
        {
            if (i > 0)
                aSize.Width() = ((aSize.Width() / TabPos) + 1) * TabPos;

            aText = rLine.GetToken(i, '\t');
            aText = comphelper::string::stripStart(aText, '\t');
            aText = comphelper::string::stripEnd(aText, '\t');
            aSize.Width() += rDevice.GetTextWidth(aText);
        }
    }

    return aSize;
}


Size SmViewShell::GetTextSize(OutputDevice& rDevice, const String& rText, long MaxWidth)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::GetTextSize" );

    Size    aSize;
    String  aLine;
    Size    TextSize;
    String  aText;
    sal_uInt16  nLines = comphelper::string::getTokenCount(rText, '\n');

    for (sal_uInt16 i = 0; i < nLines; i++)
    {
        aLine = rText.GetToken(i, '\n');
        aLine = comphelper::string::remove(aLine, '\r');
        aLine = comphelper::string::stripStart(aLine, '\n');
        aLine = comphelper::string::stripEnd(aLine, '\n');

        aSize = GetTextLineSize(rDevice, aLine);

        if (aSize.Width() > MaxWidth)
        {
            do
            {
                xub_StrLen m    = aLine.Len();
                xub_StrLen nLen = m;

                for (xub_StrLen n = 0; n < nLen; n++)
                {
                    sal_Unicode cLineChar = aLine.GetChar(n);
                    if ((cLineChar == ' ') || (cLineChar == '\t'))
                    {
                        aText = aLine.Copy(0, n);
                        if (GetTextLineSize(rDevice, aText).Width() < MaxWidth)
                            m = n;
                        else
                            break;
                    }
                }

                aText = aLine.Copy(0, m);
                aLine.Erase(0, m);
                aSize = GetTextLineSize(rDevice, aText);
                TextSize.Height() += aSize.Height();
                TextSize.Width() = Max(TextSize.Width(), Min(aSize.Width(), MaxWidth));

                aLine = comphelper::string::stripStart(aLine, ' ');
                aLine = comphelper::string::stripStart(aLine, '\t');
                aLine = comphelper::string::stripStart(aLine, ' ');
            }
            while (aLine.Len() > 0);
        }
        else
        {
            TextSize.Height() += aSize.Height();
            TextSize.Width() = Max(TextSize.Width(), aSize.Width());
        }
    }

    return TextSize;
}


void SmViewShell::DrawTextLine(OutputDevice& rDevice, const Point& rPosition, const String& rLine)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::DrawTextLine" );

    String  aText;
    Point   aPoint (rPosition);
    sal_uInt16 nTabs = comphelper::string::getTokenCount(rLine, '\t');

    if (nTabs > 0)
    {
        long TabPos = rDevice.GetTextWidth(rtl::OUString('n')) * 8;

        for (sal_uInt16 i = 0; i < nTabs; i++)
        {
            if (i > 0)
                aPoint.X() = ((aPoint.X() / TabPos) + 1) * TabPos;

            aText = rLine.GetToken(i, '\t');
            aText = comphelper::string::stripStart(aText, '\t');
            aText = comphelper::string::stripEnd(aText, '\t');
            rDevice.DrawText(aPoint, aText);
            aPoint.X() += rDevice.GetTextWidth(aText);
        }
    }
    else
        rDevice.DrawText(aPoint, rLine);
}


void SmViewShell::DrawText(OutputDevice& rDevice, const Point& rPosition, const String& rText, sal_uInt16 MaxWidth)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::DrawText" );

    sal_uInt16 nLines = comphelper::string::getTokenCount(rText, '\n');
    Point   aPoint (rPosition);
    Size    aSize;
    String  aLine;
    String  aText;

    for (sal_uInt16 i = 0; i < nLines; i++)
    {
        aLine = rText.GetToken(i, '\n');
        aLine = comphelper::string::remove(aLine, '\r');
        aLine = comphelper::string::stripEnd(aLine, '\n');
        aLine = comphelper::string::stripEnd(aLine, '\n');
        aSize = GetTextLineSize(rDevice, aLine);
        if (aSize.Width() > MaxWidth)
        {
            do
            {
                xub_StrLen m    = aLine.Len();
                xub_StrLen nLen = m;

                for (xub_StrLen n = 0; n < nLen; n++)
                {
                    sal_Unicode cLineChar = aLine.GetChar(n);
                    if ((cLineChar == ' ') || (cLineChar == '\t'))
                    {
                        aText = aLine.Copy(0, n);
                        if (GetTextLineSize(rDevice, aText).Width() < MaxWidth)
                            m = n;
                        else
                            break;
                    }
                }
                aText = aLine.Copy(0, m);
                aLine.Erase(0, m);

                DrawTextLine(rDevice, aPoint, aText);
                aPoint.Y() += aSize.Height();

                aLine = comphelper::string::stripStart(aLine, ' ');
                aLine = comphelper::string::stripStart(aLine, '\t');
                aLine = comphelper::string::stripStart(aLine, ' ');
            }
            while (GetTextLineSize(rDevice, aLine).Width() > MaxWidth);

            // print the remaining text
            if (aLine.Len() > 0)
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
}

void SmViewShell::Impl_Print(
        OutputDevice &rOutDev,
        const SmPrintUIOptions &rPrintUIOptions,
        Rectangle aOutRect, Point aZeroPoint )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::Impl_Print" );

    const bool bIsPrintTitle = rPrintUIOptions.getBoolValue( PRTUIOPT_TITLE_ROW, sal_True );
    const bool bIsPrintFrame = rPrintUIOptions.getBoolValue( PRTUIOPT_BORDER, sal_True );
    const bool bIsPrintFormulaText = rPrintUIOptions.getBoolValue( PRTUIOPT_FORMULA_TEXT, sal_True );
    SmPrintSize ePrintSize( static_cast< SmPrintSize >( rPrintUIOptions.getIntValue( PRTUIOPT_PRINT_FORMAT, PRINT_SIZE_NORMAL ) ));
    const sal_uInt16 nZoomFactor = static_cast< sal_uInt16 >(rPrintUIOptions.getIntValue( PRTUIOPT_PRINT_SCALE, 100 ));

    rOutDev.Push();
    rOutDev.SetLineColor( Color(COL_BLACK) );

    // output text on top
    if (bIsPrintTitle)
    {
        Size aSize600 (0, 600);
        Size aSize650 (0, 650);
        Font aFont(FAMILY_DONTKNOW, aSize600);

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
        Font aFont(FAMILY_DONTKNOW, Size(0, 600));
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
    if (!rPrintUIOptions.getBoolValue( "IsPrinter", sal_False ) )
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
                sal_uInt16   nZ = (sal_uInt16) Min((long)Fraction(OutputSize.Width()  * 100L, GraphicSize.Width()),
                                              (long)Fraction(OutputSize.Height() * 100L, GraphicSize.Height()));
                Fraction aFraction ((sal_uInt16) Max ((sal_uInt16) MINZOOM, Min((sal_uInt16) MAXZOOM, (sal_uInt16) (nZ - 10))), (sal_uInt16) 100);

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
    rOutDev.SetClipRegion(Region(aOutRect));
    GetDoc()->DrawFormula(rOutDev, aPos, false);
    rOutDev.SetClipRegion();

    rOutDev.Pop();
}

sal_uInt16 SmViewShell::Print(SfxProgress & /*rProgress*/, sal_Bool /*bIsAPI*/)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::Print" );
    OSL_FAIL( "SmViewShell::Print: no longer used with new UI print dialog. Should be removed!!" );
    return 0;
}


SfxPrinter* SmViewShell::GetPrinter(sal_Bool bCreate)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::GetPrinter" );

    SmDocShell *pDoc = GetDoc();
    if ( pDoc->HasPrinter() || bCreate )
        return pDoc->GetPrinter();
    return 0;
}


sal_uInt16 SmViewShell::SetPrinter(SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags, bool )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::SetPrinter" );
    SfxPrinter *pOld = GetDoc()->GetPrinter();
    if ( pOld && pOld->IsPrinting() )
        return SFX_PRINTERROR_BUSY;

    if ((nDiffFlags & SFX_PRINTER_PRINTER) == SFX_PRINTER_PRINTER)
        GetDoc()->SetPrinter( pNewPrinter );

    if ((nDiffFlags & SFX_PRINTER_OPTIONS) == SFX_PRINTER_OPTIONS)
    {
        SmModule *pp = SM_MOD();
        pp->GetConfig()->ItemSetToConfig(pNewPrinter->GetOptions());
    }
    return 0;
}


SfxTabPage* SmViewShell::CreatePrintOptionsPage(Window *pParent,
                                                const SfxItemSet &rOptions)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::CreatePrintOptionsPage" );

    return SmPrintOptionsTabPage::Create(pParent, rOptions);
}


SmEditWindow *SmViewShell::GetEditWindow()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::GetEditWindow" );

    SmCmdBoxWrapper *pWrapper = (SmCmdBoxWrapper *) GetViewFrame()->
            GetChildWindow( SmCmdBoxWrapper::GetChildWindowId() );

    if (pWrapper != NULL)
    {
        SmEditWindow *pEditWin  = pWrapper->GetEditWindow();
        OSL_ENSURE( pEditWin, "SmEditWindow missing" );
        return pEditWin;
    }

    return NULL;
}


void SmViewShell::SetStatusText(const String& Text)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::SetStatusText" );

    StatusText = Text;
    GetViewFrame()->GetBindings().Invalidate(SID_TEXTSTATUS);
}


void SmViewShell::ShowError( const SmErrorDesc *pErrorDesc )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::ShowError" );

    OSL_ENSURE(GetDoc(), "Sm : Document missing");
    if (pErrorDesc || 0 != (pErrorDesc = GetDoc()->GetParser().GetError(0)) )
    {
        SetStatusText( pErrorDesc->Text );
        GetEditWindow()->MarkError( Point( pErrorDesc->pNode->GetColumn(),
                                           pErrorDesc->pNode->GetRow()));
    }
}


void SmViewShell::NextError()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::NextError" );

    OSL_ENSURE(GetDoc(), "Sm : Document missing");
    const SmErrorDesc   *pErrorDesc = GetDoc()->GetParser().NextError();

    if (pErrorDesc)
        ShowError( pErrorDesc );
}


void SmViewShell::PrevError()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::PrevError" );

    OSL_ENSURE(GetDoc(), "Sm : Document missing");
    const SmErrorDesc   *pErrorDesc = GetDoc()->GetParser().PrevError();

    if (pErrorDesc)
        ShowError( pErrorDesc );
}


bool SmViewShell::Insert( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::Insert" );

    SmDocShell *pDoc = GetDoc();
    String aText( pDoc->GetText() );
    String aTemp = aText;
    bool bRet = false;

    uno::Reference < embed::XStorage > xStorage = rMedium.GetStorage();
    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY );
    if ( xNameAccess.is() && xNameAccess->getElementNames().getLength() )
    {
        if ( xNameAccess->hasByName( rtl::OUString("content.xml") ) || xNameAccess->hasByName( rtl::OUString("Content.xml") ))
        {
            // is this a fabulous math package ?
            Reference<com::sun::star::frame::XModel> xModel(pDoc->GetModel());
            SmXMLImportWrapper aEquation(xModel);    //!! modifies the result of pDoc->GetText() !!
            bRet = 0 == aEquation.Import(rMedium);
        }
    }

    if( bRet )
    {
        aText = pDoc->GetText();
        SmEditWindow *pEditWin = GetEditWindow();
        if (pEditWin)
            pEditWin->InsertText( aText );
        else
        {
            OSL_FAIL( "EditWindow missing" );
            aTemp += aText;
            aText  = aTemp;
        }

        pDoc->Parse();
        pDoc->SetModified(true);

        SfxBindings &rBnd = GetViewFrame()->GetBindings();
        rBnd.Invalidate(SID_GAPHIC_SM);
        rBnd.Invalidate(SID_TEXT);
    }
    return bRet;
}


bool SmViewShell::InsertFrom(SfxMedium &rMedium)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::InsertFrom" );

    bool        bSuccess = false;
    SmDocShell *pDoc = GetDoc();
    SvStream   *pStream = rMedium.GetInStream();
    String      aText( pDoc->GetText() );
    String      aTemp = aText;

    if (pStream)
    {
        const String& rFltName = rMedium.GetFilter()->GetFilterName();
        if ( rFltName.EqualsAscii(MATHML_XML) )
        {
            Reference<com::sun::star::frame::XModel> xModel( pDoc->GetModel() );
            SmXMLImportWrapper aEquation(xModel);    //!! modifies the result of pDoc->GetText() !!
            bSuccess = 0 == aEquation.Import(rMedium);
        }
    }

    if( bSuccess )
    {
        aText = pDoc->GetText();
        SmEditWindow *pEditWin = GetEditWindow();
        if (pEditWin)
            pEditWin->InsertText( aText );
        else
        {
            OSL_FAIL( "EditWindow missing" );
            aTemp += aText;
            aText  = aTemp;
        }

        pDoc->Parse();
        pDoc->SetModified(true);

        SfxBindings &rBnd = GetViewFrame()->GetBindings();
        rBnd.Invalidate(SID_GAPHIC_SM);
        rBnd.Invalidate(SID_TEXT);
    }

    return bSuccess;
}


void SmViewShell::Execute(SfxRequest& rReq)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::Execute" );

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
                 SFX_ITEM_SET == pArgs->GetItemState( SID_FORMULACURSOR, false, &pItem))
                bVal = ((SfxBoolItem *) pItem)->GetValue();
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
                SetStatusText(String());
                ShowError( 0 );
                GetDoc()->Repaint();
            }
            break;

        case SID_ADJUST:
        case SID_FITINWINDOW:
            aGraphic.ZoomToFitInWindow();
            break;

        case SID_VIEW050:
            aGraphic.SetZoom(50);
            break;

        case SID_VIEW100:
            aGraphic.SetZoom(100);
            break;

        case SID_VIEW200:
            aGraphic.SetZoom(200);
            break;

        case SID_ZOOMIN:
            aGraphic.SetZoom(aGraphic.GetZoom() + 25);
            break;

        case SID_ZOOMOUT:
            OSL_ENSURE(aGraphic.GetZoom() >= 25, "Sm: incorrect sal_uInt16 argument");
            aGraphic.SetZoom(aGraphic.GetZoom() - 25);
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
                        pTrans->CopyToClipboard( this ? GetEditWindow() : 0 );
                }
            }
        }
        break;

        case SID_PASTEOBJECT:
        {
            TransferableDataHelper aData( TransferableDataHelper::CreateFromSystemClipboard(this ? GetEditWindow(): 0) );
            uno::Reference < io::XInputStream > xStrm;
            SotFormatStringId nId;
            if( aData.GetTransferable().is() &&
                ( aData.HasFormat( nId = SOT_FORMATSTR_ID_EMBEDDED_OBJ ) ||
                  (aData.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) &&
                   aData.HasFormat( nId = SOT_FORMATSTR_ID_EMBED_SOURCE ))) &&
                aData.GetInputStream( nId, xStrm ) && xStrm.is() )
            {
                try
                {
                    uno::Reference < embed::XStorage > xStorage =
                            ::comphelper::OStorageHelper::GetStorageFromInputStream( xStrm, ::comphelper::getProcessServiceFactory() );
                    uno::Reference < beans::XPropertySet > xProps( xStorage, uno::UNO_QUERY );
                    SfxMedium aMedium( xStorage, String() );
                    Insert( aMedium );
                    GetDoc()->UpdateText();
                }
                catch (uno::Exception &)
                {
                    OSL_FAIL( "SmViewShell::Execute (SID_PASTEOBJECT): failed to get storage from input stream" );
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
                                SID_COPYOBJECT, SFX_CALLMODE_STANDARD,
                                new SfxVoidItem(SID_COPYOBJECT), 0L);
                }
                else
                    pWin->Copy();
            }
            break;

        case SID_PASTE:
            {
                bool bCallExec = 0 == pWin;
                if( !bCallExec )
                {
                    TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                    GetEditWindow()) );

                    if( aDataHelper.GetTransferable().is() &&
                        aDataHelper.HasFormat( FORMAT_STRING ))
                        pWin->Paste();
                    else
                        bCallExec = true;
                }
                if( bCallExec )
                {
                    GetViewFrame()->GetDispatcher()->Execute(
                            SID_PASTEOBJECT, SFX_CALLMODE_STANDARD,
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
                (const SfxInt16Item&)rReq.GetArgs()->Get(SID_INSERTCOMMAND);

            if (pWin && (bInsertIntoEditWindow || !IsInlineEditEnabled()))
                pWin->InsertCommand(rItem.GetValue());
            if (IsInlineEditEnabled() && (GetDoc() && !bInsertIntoEditWindow)) {
                GetDoc()->GetCursor().InsertCommand(rItem.GetValue());
                GetGraphicWindow().GrabFocus();
            }
            break;
        }

        case SID_INSERTSYMBOL:
        {
            const SfxStringItem& rItem =
                (const SfxStringItem&)rReq.GetArgs()->Get(SID_INSERTSYMBOL);

            if (pWin && (bInsertIntoEditWindow || !IsInlineEditEnabled()))
                pWin->InsertText(rItem.GetValue());
            if (IsInlineEditEnabled() && (GetDoc() && !bInsertIntoEditWindow))
                GetDoc()->GetCursor().InsertSpecial(rItem.GetValue());
            break;
        }

        case SID_IMPORT_FORMULA:
        {
            delete pImpl->pRequest;
            pImpl->pRequest = new SfxRequest( rReq );
            delete pImpl->pDocInserter;
            pImpl->pDocInserter = new ::sfx2::DocumentInserter(
                        GetDoc()->GetFactory().GetFactoryName(), 0 );
            pImpl->pDocInserter->StartExecuteModal( LINK( this, SmViewShell, DialogClosedHdl ) );
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
            if (rReq.GetArgs() != NULL)
            {
                const SfxStringItem& rItem =
                    (const SfxStringItem&)rReq.GetArgs()->Get(SID_TEXTSTATUS);

                SetStatusText(rItem.GetValue());
            }

            break;
        }

        case SID_GETEDITTEXT:
            if (pWin)
                if (pWin->GetText ().Len ()) GetDoc()->SetText( pWin->GetText() );
            break;

        case SID_ATTR_ZOOM:
        {
            if ( !GetViewFrame()->GetFrame().IsInPlace() )
            {
                AbstractSvxZoomDialog *pDlg = 0;
                const SfxItemSet *pSet = rReq.GetArgs();
                if ( !pSet )
                {
                    SfxItemSet aSet( GetDoc()->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
                    aSet.Put( SvxZoomItem( SVX_ZOOM_PERCENT, aGraphic.GetZoom()));
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    if(pFact)
                    {
                        pDlg = pFact->CreateSvxZoomDialog(&GetViewFrame()->GetWindow(), aSet);
                        OSL_ENSURE(pDlg, "Dialogdiet fail!");
                        pDlg->SetLimits( MINZOOM, MAXZOOM );
                        if( pDlg->Execute() != RET_CANCEL )
                            pSet = pDlg->GetOutputItemSet();
                    }
                }
                if ( pSet )
                {
                    const SvxZoomItem &rZoom = (const SvxZoomItem &)pSet->Get(SID_ATTR_ZOOM);
                    switch( rZoom.GetType() )
                    {
                        case SVX_ZOOM_PERCENT:
                            aGraphic.SetZoom((sal_uInt16)rZoom.GetValue ());
                            break;

                        case SVX_ZOOM_OPTIMAL:
                            aGraphic.ZoomToFitInWindow();
                            break;

                        case SVX_ZOOM_PAGEWIDTH:
                        case SVX_ZOOM_WHOLEPAGE:
                        {
                            const MapMode aMap( MAP_100TH_MM );
                            SfxPrinter *pPrinter = GetPrinter( true );
                            Point aPoint;
                            Rectangle  OutputRect(aPoint, pPrinter->GetOutputSize());
                            Size       OutputSize(pPrinter->LogicToPixel(Size(OutputRect.GetWidth(),
                                                                              OutputRect.GetHeight()), aMap));
                            Size       GraphicSize(pPrinter->LogicToPixel(GetDoc()->GetSize(), aMap));
                            sal_uInt16     nZ = (sal_uInt16) Min((long)Fraction(OutputSize.Width()  * 100L, GraphicSize.Width()),
                                                         (long)Fraction(OutputSize.Height() * 100L, GraphicSize.Height()));
                            aGraphic.SetZoom (nZ);
                            break;
                        }
                        default:
                            break;
                    }
                }
                delete pDlg;
            }
        }
        break;

        case SID_TOOLBOX:
        {
            GetViewFrame()->ToggleChildWindow( SmToolBoxWrapper::GetChildWindowId() );
        }
        break;

        case SID_SYMBOLS_CATALOGUE:
        {

            // get device used to retrieve the FontList
            SmDocShell *pDoc = GetDoc();
            OutputDevice *pDev = pDoc->GetPrinter();
            if (!pDev || pDev->GetDevFontCount() == 0)
                pDev = &SM_MOD()->GetDefaultVirtualDev();
            OSL_ENSURE (pDev, "device for font list missing" );

            SmModule *pp = SM_MOD();
            SmSymbolDialog( NULL, pDev, pp->GetSymbolManager(), *this ).Execute();
        }
        break;
    }
    rReq.Done();
}


void SmViewShell::GetState(SfxItemSet &rSet)
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::GetState" );

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
            if( !xClipEvtLstnr.is()  &&  pEditWin)
            {
                TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                        pEditWin) );

                bPasteState = aDataHelper.GetTransferable().is() &&
                 ( aDataHelper.HasFormat( FORMAT_STRING ) ||
                   aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBEDDED_OBJ ) ||
                   (aDataHelper.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR )
                      && aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE )));
            }
            if( !bPasteState )
                rSet.DisableItem( nWh );
            break;

        case SID_ATTR_ZOOM:
            rSet.Put(SvxZoomItem( SVX_ZOOM_PERCENT, aGraphic.GetZoom()));
            /* no break here */
        case SID_VIEW050:
        case SID_VIEW100:
        case SID_VIEW200:
        case SID_ADJUST:
        case SID_ZOOMIN:
        case SID_ZOOMOUT:
        case SID_FITINWINDOW:
            if ( GetViewFrame()->GetFrame().IsInPlace() )
                rSet.DisableItem( nWh );
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
                rSet.Put(SfxStringItem(nWh, StatusText));
            }
            break;

        case SID_FORMULACURSOR:
            {
                SmModule *pp = SM_MOD();
                rSet.Put(SfxBoolItem(nWh, pp->GetConfig()->IsShowFormulaCursor()));
            }
            break;

        case SID_TOOLBOX:
            {
                bool bState = false;
                SfxChildWindow *pChildWnd = GetViewFrame()->
                        GetChildWindow( SmToolBoxWrapper::GetChildWindowId() );
                if (pChildWnd  &&  pChildWnd->GetWindow()->IsVisible())
                    bState = true;
                rSet.Put(SfxBoolItem(SID_TOOLBOX, bState));
            }
            break;

        }
    }
}


SmViewShell::SmViewShell(SfxViewFrame *pFrame_, SfxViewShell *):
    SfxViewShell(pFrame_, SFX_VIEW_HAS_PRINTOPTIONS | SFX_VIEW_CAN_PRINT),
    pImpl( new SmViewShell_Impl ),
    aGraphic(this),
    aGraphicController(aGraphic, SID_GAPHIC_SM, pFrame_->GetBindings())
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::SmViewShell" );

    SetStatusText(String());
    SetWindow(&aGraphic);
    SfxShell::SetName(rtl::OUString("SmView"));
    SfxShell::SetUndoManager( &GetDoc()->GetEditEngine().GetUndoManager() );
    SetHelpId( HID_SMA_VIEWSHELL_DOCUMENT );
}


SmViewShell::~SmViewShell()
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::~SmViewShell" );

    //!! this view shell is not active anymore !!
    // Thus 'SmGetActiveView' will give a 0 pointer.
    // Thus we need to supply this view as argument
    SmEditWindow *pEditWin = GetEditWindow();
    if (pEditWin)
        pEditWin->DeleteEditView( *this );
    delete pImpl;
}

void SmViewShell::Deactivate( sal_Bool bIsMDIActivate )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::Deactivate" );

    SmEditWindow *pEdit = GetEditWindow();
    if ( pEdit )
        pEdit->Flush();

    SfxViewShell::Deactivate( bIsMDIActivate );
}


void SmViewShell::Activate( sal_Bool bIsMDIActivate )
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmViewShell::Activate" );

    SfxViewShell::Activate( bIsMDIActivate );

    SmEditWindow *pEdit = GetEditWindow();
    if ( pEdit )
    {
        //! Since there is no way to be informed if a "drag and drop"
        //! event has taken place, we call SetText here in order to
        //! syncronize the GraphicWindow display with the text in the
        //! EditEngine.
        SmDocShell *pDoc = GetDoc();
        pDoc->SetText( pDoc->GetEditEngine().GetText( LINEEND_LF ) );

        if ( bIsMDIActivate )
            pEdit->GrabFocus();
    }
}

IMPL_LINK( SmViewShell, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg )
{
    OSL_ENSURE( _pFileDlg, "SmViewShell::DialogClosedHdl(): no file dialog" );
    OSL_ENSURE( pImpl->pDocInserter, "ScDocShell::DialogClosedHdl(): no document inserter" );

    if ( ERRCODE_NONE == _pFileDlg->GetError() )
    {
        SfxMedium* pMedium = pImpl->pDocInserter->CreateMedium();

        if ( pMedium != NULL )
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
    return 0;
}

void SmViewShell::Notify( SfxBroadcaster& , const SfxHint& rHint )
{
    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        switch( ( (SfxSimpleHint&) rHint ).GetId() )
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
