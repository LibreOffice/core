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

#include <sal/config.h>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>
#include <i18nutil/unicode.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <officecfg/Office/Common.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/infobar.hxx>
#include <sfx2/lokcomponenthelpers.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/viewfac.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <svl/poolitem.hxx>
#include <svl/stritem.hxx>
#include <svl/voiditem.hxx>
#include <vcl/transfer.hxx>
#include <svtools/colorcfg.hxx>
#include <svl/whiter.hxx>
#include <svx/sidebar/SelectionChangeHandler.hxx>
#include <svx/zoomslideritem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editund2.hxx>
#include <svx/svxdlg.hxx>
#include <sfx2/zoomitem.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <sal/log.hxx>
#include <tools/svborder.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/temporary.hxx>

#include <unotools/streamwrap.hxx>

#include <unomodel.hxx>
#include <view.hxx>
#include <cfgitem.hxx>
#include <dialog.hxx>
#include <document.hxx>
#include <starmath.hrc>
#include <strings.hrc>
#include <smmod.hxx>
#include <mathmlimport.hxx>
#include <cursor.hxx>
#include "accessibility.hxx"
#include <ElementsDockingWindow.hxx>
#include <helpids.h>

// space around the edit window, in pixels
// fdo#69111: Increased border on the top so that the window is
// easier to tear off.
#define CMD_BOX_PADDING 3
#define CMD_BOX_PADDING_TOP 11

#define ShellClass_SmViewShell
#include <smslots.hxx>

using namespace css;
using namespace css::accessibility;
using namespace css::uno;

SmGraphicWindow::SmGraphicWindow(SmViewShell& rShell)
    : InterimItemWindow(&rShell.GetViewFrame().GetWindow(), u"modules/smath/ui/mathwindow.ui"_ustr, u"MathWindow"_ustr)
    , nLinePixH(GetSettings().GetStyleSettings().GetScrollBarSize())
    , nColumnPixW(nLinePixH)
    , nZoom(100)
    // continue to use user-scrolling to make this work equivalent to how it 'always' worked
    , mxScrolledWindow(m_xBuilder->weld_scrolled_window(u"scrolledwindow"_ustr, true))
    , mxGraphic(new SmGraphicWidget(rShell, *this))
    , mxGraphicWin(new weld::CustomWeld(*m_xBuilder, u"mathview"_ustr, *mxGraphic))
{
    InitControlBase(mxGraphic->GetDrawingArea());

    mxScrolledWindow->connect_hadjustment_changed(LINK(this, SmGraphicWindow, ScrollHdl));
    mxScrolledWindow->connect_vadjustment_changed(LINK(this, SmGraphicWindow, ScrollHdl));

    // docking windows are usually hidden (often already done in the
    // resource) and will be shown by the sfx framework.
    Hide();
}

void SmGraphicWindow::dispose()
{
    InitControlBase(nullptr);
    mxGraphicWin.reset();
    mxGraphic.reset();
    mxScrolledWindow.reset();
    InterimItemWindow::dispose();
}

SmGraphicWindow::~SmGraphicWindow()
{
    disposeOnce();
}

void SmGraphicWindow::Resize()
{
    InterimItemWindow::Resize();

    // get the new output-size in pixel
    Size aOutPixSz = GetOutputSizePixel();

    // determine the size of the output-area and if we need scrollbars
    const auto nScrSize = mxScrolledWindow->get_scroll_thickness();
    bool bVVisible = false; // by default no vertical-ScrollBar
    bool bHVisible = false; // by default no horizontal-ScrollBar
    bool bChanged;          // determines if a visibility was changed
    do
    {
        bChanged = false;

        // does we need a vertical ScrollBar
        if ( aOutPixSz.Width() < aTotPixSz.Width() && !bHVisible )
        {
            bHVisible = true;
            aOutPixSz.AdjustHeight( -nScrSize );
            bChanged = true;
        }

        // does we need a horizontal ScrollBar
        if ( aOutPixSz.Height() < aTotPixSz.Height() && !bVVisible )
        {
            bVVisible = true;
            aOutPixSz.AdjustWidth( -nScrSize );
            bChanged = true;
        }

    }
    while ( bChanged );   // until no visibility has changed

    // store the old offset and map-mode
    MapMode aMap(GetGraphicMapMode());
    Point aOldPixOffset(aPixOffset);

    // justify (right/bottom borders should never exceed the virtual window)
    Size aPixDelta;
    if ( aPixOffset.X() < 0 &&
         aPixOffset.X() + aTotPixSz.Width() < aOutPixSz.Width() )
        aPixDelta.setWidth(
            aOutPixSz.Width() - ( aPixOffset.X() + aTotPixSz.Width() ) );
    if ( aPixOffset.Y() < 0 &&
         aPixOffset.Y() + aTotPixSz.Height() < aOutPixSz.Height() )
        aPixDelta.setHeight(
            aOutPixSz.Height() - ( aPixOffset.Y() + aTotPixSz.Height() ) );
    if ( aPixDelta.Width() || aPixDelta.Height() )
    {
        aPixOffset.AdjustX(aPixDelta.Width() );
        aPixOffset.AdjustY(aPixDelta.Height() );
    }

    // for axis without scrollbar restore the origin
    if ( !bVVisible || !bHVisible )
    {
        aPixOffset = Point(
                     bHVisible
                     ? aPixOffset.X()
                     : (aOutPixSz.Width()-aTotPixSz.Width()) / 2,
                     bVVisible
                     ? aPixOffset.Y()
                     : (aOutPixSz.Height()-aTotPixSz.Height()) / 2 );
    }
    if (bHVisible && mxScrolledWindow->get_hpolicy() == VclPolicyType::NEVER)
        aPixOffset.setX( 0 );
    if (bVVisible && mxScrolledWindow->get_vpolicy() == VclPolicyType::NEVER)
        aPixOffset.setY( 0 );

    // select the shifted map-mode
    if (aPixOffset != aOldPixOffset)
        SetGraphicMapMode(aMap);

    // show or hide scrollbars
    mxScrolledWindow->set_vpolicy(bVVisible ? VclPolicyType::ALWAYS : VclPolicyType::NEVER);
    mxScrolledWindow->set_hpolicy(bHVisible ? VclPolicyType::ALWAYS : VclPolicyType::NEVER);

    // resize scrollbars and set their ranges
    if ( bHVisible )
    {
        mxScrolledWindow->hadjustment_configure(-aPixOffset.X(), 0, aTotPixSz.Width(), nColumnPixW,
                                                aOutPixSz.Width(), aOutPixSz.Width());
    }
    if ( bVVisible )
    {
        mxScrolledWindow->vadjustment_configure(-aPixOffset.Y(), 0, aTotPixSz.Height(), nLinePixH,
                                                aOutPixSz.Height(), aOutPixSz.Height());
    }
}

IMPL_LINK_NOARG(SmGraphicWindow, ScrollHdl, weld::ScrolledWindow&, void)
{
    MapMode aMap(GetGraphicMapMode());
    Point aNewPixOffset(aPixOffset);

    // scrolling horizontally?
    if (mxScrolledWindow->get_hpolicy() == VclPolicyType::ALWAYS)
        aNewPixOffset.setX(-mxScrolledWindow->hadjustment_get_value());

    // scrolling vertically?
    if (mxScrolledWindow->get_vpolicy() == VclPolicyType::ALWAYS)
        aNewPixOffset.setY(-mxScrolledWindow->vadjustment_get_value());

    // scrolling?
    if (aPixOffset == aNewPixOffset)
        return;

    // recompute the logical scroll units
    aPixOffset = aNewPixOffset;

    SetGraphicMapMode(aMap);
}

void SmGraphicWindow::SetGraphicMapMode(const MapMode& rNewMapMode)
{
    OutputDevice& rDevice = mxGraphic->GetOutputDevice();
    MapMode aMap( rNewMapMode );
    aMap.SetOrigin( aMap.GetOrigin() + rDevice.PixelToLogic( aPixOffset, aMap ) );
    rDevice.SetMapMode( aMap );
    mxGraphic->Invalidate();
}

MapMode SmGraphicWindow::GetGraphicMapMode() const
{
    OutputDevice& rDevice = mxGraphic->GetOutputDevice();
    MapMode aMap(rDevice.GetMapMode());
    aMap.SetOrigin( aMap.GetOrigin() - rDevice.PixelToLogic( aPixOffset ) );
    return aMap;
}

void SmGraphicWindow::SetTotalSize( const Size& rNewSize )
{
    aTotPixSz = mxGraphic->GetOutputDevice().LogicToPixel(rNewSize);
    Resize();
}

Size SmGraphicWindow::GetTotalSize() const
{
    return mxGraphic->GetOutputDevice().PixelToLogic(aTotPixSz);
}

void SmGraphicWindow::ShowContextMenu(const CommandEvent& rCEvt)
{
    GetParent()->ToTop();
    Point aPos(5, 5);
    if (rCEvt.IsMouseEvent())
        aPos = rCEvt.GetMousePosPixel();

    // added for replaceability of context menus
    SfxDispatcher::ExecutePopup( this, &aPos );
}

SmGraphicWidget::SmGraphicWidget(SmViewShell& rShell, SmGraphicWindow& rGraphicWindow)
    : mrGraphicWindow(rGraphicWindow)
    , bIsCursorVisible(false)
    , bIsLineVisible(false)
    , aCaretBlinkTimer("SmGraphicWidget aCaretBlinkTimer")
    , mrViewShell(rShell)
{
}

void SmGraphicWidget::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);

    OutputDevice& rDevice = GetOutputDevice();

    rDevice.EnableRTL(GetDoc()->GetFormat().IsRightToLeft());
    rDevice.SetBackground(SM_MOD()->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor);

    if (comphelper::LibreOfficeKit::isActive())
    {
        // Disable map mode, so that it's possible to send mouse event coordinates
        // directly in twips.
        rDevice.EnableMapMode(false);
    }
    else
    {
        const Fraction aFraction(1, 1);
        rDevice.SetMapMode(MapMode(SmMapUnit(), Point(), aFraction, aFraction));
    }

    SetTotalSize();

    SetHelpId(HID_SMA_WIN_DOCUMENT);

    ShowLine(false);
    CaretBlinkInit();
}

SmGraphicWidget::~SmGraphicWidget()
{
    if (mxAccessible.is())
        mxAccessible->ClearWin();    // make Accessible nonfunctional
    mxAccessible.clear();
    CaretBlinkStop();
}

SmDocShell* SmGraphicWidget::GetDoc() { return GetView().GetDoc(); }

SmCursor& SmGraphicWidget::GetCursor()
{
    assert(GetDoc());
    return GetDoc()->GetCursor();
}

bool SmGraphicWidget::MouseButtonDown(const MouseEvent& rMEvt)
{
    GrabFocus();

    // set formula-cursor and selection of edit window according to the
    // position clicked at

    SAL_WARN_IF( rMEvt.GetClicks() == 0, "starmath", "0 clicks" );
    if ( !rMEvt.IsLeft() )
        return true;

    OutputDevice& rDevice = GetOutputDevice();
    // get click position relative to formula
    Point aPos(rDevice.PixelToLogic(rMEvt.GetPosPixel()) - GetFormulaDrawPos());

    const SmNode *pTree = GetDoc()->GetFormulaTree();
    if (!pTree)
        return true;

    SmEditWindow* pEdit = GetView().GetEditWindow();

    if (SmViewShell::IsInlineEditEnabled()) {
        GetCursor().MoveTo(&rDevice, aPos, !rMEvt.IsShift());
        GetView().InvalidateSlots();
        // 'on grab' window events are missing in lok, do it explicitly
        if (comphelper::LibreOfficeKit::isActive())
            SetIsCursorVisible(true);
        return true;
    }
    const SmNode *pNode = nullptr;
    // if it was clicked inside the formula then get the appropriate node
    if (pTree->OrientedDist(aPos) <= 0)
        pNode = pTree->FindRectClosestTo(aPos);

    if (!pNode)
        return true;

    if (!pEdit)
        return true;

    // set selection to the beginning of the token
    pEdit->SetSelection(pNode->GetSelection());
    SetCursor(pNode);

    // allow for immediate editing and
    //! implicitly synchronize the cursor position mark in this window
    pEdit->GrabFocus();

    return true;
}

bool SmGraphicWidget::MouseMove(const MouseEvent &rMEvt)
{
    if (rMEvt.IsLeft() && SmViewShell::IsInlineEditEnabled())
    {
        OutputDevice& rDevice = GetOutputDevice();
        Point aPos(rDevice.PixelToLogic(rMEvt.GetPosPixel()) - GetFormulaDrawPos());
        GetCursor().MoveTo(&rDevice, aPos, false);

        CaretBlinkStop();
        SetIsCursorVisible(true);
        CaretBlinkStart();
        RepaintViewShellDoc();
    }
    return true;
}

void SmGraphicWidget::GetFocus()
{
    if (!SmViewShell::IsInlineEditEnabled())
        return;
    if (SmEditWindow* pEdit = GetView().GetEditWindow())
        pEdit->Flush();
    SetIsCursorVisible(true);
    ShowLine(true);
    CaretBlinkStart();
    RepaintViewShellDoc();
}

void SmGraphicWidget::LoseFocus()
{
    if (mxAccessible.is())
    {
        uno::Any aOldValue, aNewValue;
        aOldValue <<= AccessibleStateType::FOCUSED;
        // aNewValue remains empty
        mxAccessible->LaunchEvent( AccessibleEventId::STATE_CHANGED,
                aOldValue, aNewValue );
    }
    if (!SmViewShell::IsInlineEditEnabled())
        return;
    SetIsCursorVisible(false);
    ShowLine(false);
    CaretBlinkStop();
    RepaintViewShellDoc();
}

void SmGraphicWidget::RepaintViewShellDoc()
{
    if (SmDocShell* pDoc = GetDoc())
        pDoc->Repaint();
}

IMPL_LINK_NOARG(SmGraphicWidget, CaretBlinkTimerHdl, Timer *, void)
{
    if (IsCursorVisible())
        SetIsCursorVisible(false);
    else
        SetIsCursorVisible(true);

    RepaintViewShellDoc();
}

void SmGraphicWidget::CaretBlinkInit()
{
    if (comphelper::LibreOfficeKit::isActive())
        return; // No blinking in lok case
    aCaretBlinkTimer.SetInvokeHandler(LINK(this, SmGraphicWidget, CaretBlinkTimerHdl));
    aCaretBlinkTimer.SetTimeout(Application::GetSettings().GetStyleSettings().GetCursorBlinkTime());
}

void SmGraphicWidget::CaretBlinkStart()
{
    if (!SmViewShell::IsInlineEditEnabled() || comphelper::LibreOfficeKit::isActive())
        return;
    if (aCaretBlinkTimer.GetTimeout() != STYLE_CURSOR_NOBLINKTIME)
        aCaretBlinkTimer.Start();
}

void SmGraphicWidget::CaretBlinkStop()
{
    if (!SmViewShell::IsInlineEditEnabled() || comphelper::LibreOfficeKit::isActive())
        return;
    aCaretBlinkTimer.Stop();
}

// shows or hides the formula-cursor depending on 'bShow' is true or not
void SmGraphicWidget::ShowCursor(bool bShow)
{
    if (SmViewShell::IsInlineEditEnabled())
        return;

    bool bInvert = bShow != IsCursorVisible();
    if (bInvert)
        InvertFocusRect(GetOutputDevice(), aCursorRect);

    SetIsCursorVisible(bShow);
}

void SmGraphicWidget::ShowLine(bool bShow)
{
    if (!SmViewShell::IsInlineEditEnabled())
        return;

    bIsLineVisible = bShow;
}

void SmGraphicWidget::SetIsCursorVisible(bool bVis)
{
    bIsCursorVisible = bVis;
    if (comphelper::LibreOfficeKit::isActive())
    {
        mrViewShell.SendCaretToLOK();
        mrViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_CURSOR_VISIBLE,
                                               OString::boolean(bVis));
    }
}

void SmGraphicWidget::SetCursor(const SmNode *pNode)
{
    if (SmViewShell::IsInlineEditEnabled())
        return;

    const SmNode *pTree = GetDoc()->GetFormulaTree();

    // get appropriate rectangle
    Point aOffset (pNode->GetTopLeft() - pTree->GetTopLeft()),
          aTLPos  (GetFormulaDrawPos() + aOffset);
    aTLPos.AdjustX( -(pNode->GetItalicLeftSpace()) );
    Size  aSize   (pNode->GetItalicSize());

    SetCursor(tools::Rectangle(aTLPos, aSize));
}

void SmGraphicWidget::SetCursor(const tools::Rectangle &rRect)
    // sets cursor to new position (rectangle) 'rRect'.
    // The old cursor will be removed, and the new one will be shown if
    // that is activated in the ConfigItem
{
    if (SmViewShell::IsInlineEditEnabled())
        return;

    SmModule *pp = SM_MOD();

    if (IsCursorVisible())
        ShowCursor(false);      // clean up remainings of old cursor
    aCursorRect = rRect;
    if (pp->GetConfig()->IsShowFormulaCursor())
        ShowCursor(true);       // draw new cursor
}

const SmNode * SmGraphicWidget::SetCursorPos(sal_uInt16 nRow, sal_uInt16 nCol)
    // looks for a VISIBLE node in the formula tree with its token at
    // (or around) the position 'nRow', 'nCol' in the edit window
    // (row and column numbering starts with 1 there!).
    // If there is such a node the formula-cursor is set to cover that nodes
    // rectangle. If not the formula-cursor will be hidden.
    // In any case the search result is being returned.
{
    if (SmViewShell::IsInlineEditEnabled())
        return nullptr;

    // find visible node with token at nRow, nCol
    const SmNode *pTree = GetDoc()->GetFormulaTree(),
                 *pNode = nullptr;
    if (pTree)
        pNode = pTree->FindTokenAt(nRow, nCol);

    if (pNode)
        SetCursor(pNode);
    else
        ShowCursor(false);

    return pNode;
}

void SmGraphicWidget::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    assert(GetDoc());
    SmDocShell& rDoc = *GetDoc();
    Point aPoint;

    rDoc.DrawFormula(rRenderContext, aPoint, true);  //! modifies aPoint to be the topleft
                                                     //! corner of the formula
    aFormulaDrawPos = aPoint;
    if (SmViewShell::IsInlineEditEnabled())
    {
        //Draw cursor if any...
        if (rDoc.HasCursor() && IsLineVisible())
            rDoc.GetCursor().Draw(rRenderContext, aPoint, IsCursorVisible());
    }
    else
    {
        SetIsCursorVisible(false);  // (old) cursor must be drawn again

        if (const SmEditWindow* pEdit = GetView().GetEditWindow())
        {   // get new position for formula-cursor (for possible altered formula)
            sal_Int32  nRow;
            sal_uInt16 nCol;
            SmGetLeftSelectionPart(pEdit->GetSelection(), nRow, nCol);
            const SmNode *pFound = SetCursorPos(static_cast<sal_uInt16>(nRow), nCol);

            SmModule *pp = SM_MOD();
            if (pFound && pp->GetConfig()->IsShowFormulaCursor())
                ShowCursor(true);
        }
    }
}

void SmGraphicWidget::SetTotalSize()
{
    assert(GetDoc());
    OutputDevice& rDevice = GetOutputDevice();
    const Size aTmp(rDevice.PixelToLogic(rDevice.LogicToPixel(GetDoc()->GetSize())));
    if (aTmp != mrGraphicWindow.GetTotalSize())
        mrGraphicWindow.SetTotalSize(aTmp);
}

namespace
{
SmBracketType BracketTypeOf(sal_uInt32 c)
{
    switch (c)
    {
        case '(':
        case ')':
            return SmBracketType::Round;
        case '[':
        case ']':
            return SmBracketType::Square;
        case '{':
        case '}':
            return SmBracketType::Curly;
    }
    assert(false); // Unreachable
    return SmBracketType::Round;
}

bool CharInput(sal_uInt32 c, SmCursor& rCursor, OutputDevice& rDevice)
{
    switch (c)
    {
        case 0:
            return false;
        case ' ':
            rCursor.InsertElement(BlankElement);
            break;
        case '!':
            rCursor.InsertElement(FactorialElement);
            break;
        case '%':
            rCursor.InsertElement(PercentElement);
            break;
        case '*':
            rCursor.InsertElement(CDotElement);
            break;
        case '+':
            rCursor.InsertElement(PlusElement);
            break;
        case '-':
            rCursor.InsertElement(MinusElement);
            break;
        case '<':
            rCursor.InsertElement(LessThanElement);
            break;
        case '=':
            rCursor.InsertElement(EqualElement);
            break;
        case '>':
            rCursor.InsertElement(GreaterThanElement);
            break;
        case '^':
            rCursor.InsertSubSup(RSUP);
            break;
        case '_':
            rCursor.InsertSubSup(RSUB);
            break;
        case '/':
            rCursor.InsertFraction();
            break;
        case '(':
        case '[':
        case '{':
            rCursor.InsertBrackets(BracketTypeOf(c));
            break;
        case ')':
        case ']':
        case '}':
            if (rCursor.IsAtTailOfBracket(BracketTypeOf(c)))
            {
                rCursor.Move(&rDevice, MoveRight);
                break;
            }
            [[fallthrough]];
        default:
            rCursor.InsertText(OUString(&c, 1));
            break;
    }
    return true;
}
}

bool SmGraphicWidget::KeyInput(const KeyEvent& rKEvt)
{
    if (rKEvt.GetKeyCode().GetCode() == KEY_F1)
    {
        GetView().StartMainHelp();
        return true;
    }

    if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
    {
        // Terminate possible InPlace mode
        return GetView().Escape();
    }

    if (!SmViewShell::IsInlineEditEnabled())
        return GetView().KeyInput(rKEvt);

    bool bConsumed = true;

    SmCursor& rCursor = GetCursor();
    switch (rKEvt.GetKeyCode().GetFunction())
    {
    case KeyFuncType::COPY:
        rCursor.Copy(&mrGraphicWindow);
        break;
    case KeyFuncType::CUT:
        rCursor.Cut(&mrGraphicWindow);
        break;
    case KeyFuncType::PASTE:
        rCursor.Paste(&mrGraphicWindow);
        break;
    case KeyFuncType::UNDO:
        GetDoc()->Execute(o3tl::temporary(SfxRequest(*GetView().GetFrame(), SID_UNDO)));
        break;
    case KeyFuncType::REDO:
        GetDoc()->Execute(o3tl::temporary(SfxRequest(*GetView().GetFrame(), SID_REDO)));
        break;
    default:
        switch (rKEvt.GetKeyCode().GetCode())
        {
            case KEY_LEFT:
                rCursor.Move(&GetOutputDevice(), MoveLeft, !rKEvt.GetKeyCode().IsShift());
                break;
            case KEY_RIGHT:
                rCursor.Move(&GetOutputDevice(), MoveRight, !rKEvt.GetKeyCode().IsShift());
                break;
            case KEY_UP:
                rCursor.Move(&GetOutputDevice(), MoveUp, !rKEvt.GetKeyCode().IsShift());
                break;
            case KEY_DOWN:
                rCursor.Move(&GetOutputDevice(), MoveDown, !rKEvt.GetKeyCode().IsShift());
                break;
            case KEY_RETURN:
                if (!rKEvt.GetKeyCode().IsShift())
                    rCursor.InsertRow();
                break;
            case KEY_DELETE:
                if (!rCursor.HasSelection())
                {
                    rCursor.Move(&GetOutputDevice(), MoveRight, false);
                    if (rCursor.HasComplexSelection())
                        break;
                }
                rCursor.Delete();
                break;
            case KEY_BACKSPACE:
                rCursor.DeletePrev(&GetOutputDevice());
                break;
            default:
                if (!CharInput(rKEvt.GetCharCode(), rCursor, GetOutputDevice()))
                    bConsumed = GetView().KeyInput(rKEvt);
        }
    }

    GetView().InvalidateSlots();
    CaretBlinkStop();
    CaretBlinkStart();
    SetIsCursorVisible(true);
    RepaintViewShellDoc();

    return bConsumed;
}

bool SmGraphicWidget::Command(const CommandEvent& rCEvt)
{
    bool bCallBase = true;
    if (!GetView().GetViewFrame().GetFrame().IsInPlace())
    {
        switch ( rCEvt.GetCommand() )
        {
            case CommandEventId::ContextMenu:
                // purely for "ExecutePopup" taking a vcl::Window and
                // we assume SmGraphicWindow 0,0 is at SmEditWindow 0,0
                mrGraphicWindow.ShowContextMenu(rCEvt);
                bCallBase = false;
            break;

            case CommandEventId::Wheel:
            {
                const CommandWheelData* pWData = rCEvt.GetWheelData();
                if  ( pWData && CommandWheelMode::ZOOM == pWData->GetMode() )
                {
                    sal_uInt16 nTmpZoom = mrGraphicWindow.GetZoom();
                    if( 0 > pWData->GetDelta() )
                        nTmpZoom -= 10;
                    else
                        nTmpZoom += 10;
                    mrGraphicWindow.SetZoom(nTmpZoom);
                    bCallBase = false;
                }
                break;
            }
            case CommandEventId::GestureZoom:
            {
                const CommandGestureZoomData* pData = rCEvt.GetGestureZoomData();
                if (pData)
                {
                    if (pData->meEventType == GestureEventZoomType::Begin)
                    {
                        mfLastZoomScale = pData->mfScaleDelta;
                    }
                    else if (pData->meEventType == GestureEventZoomType::Update)
                    {
                        double deltaBetweenEvents = (pData->mfScaleDelta - mfLastZoomScale) / mfLastZoomScale;
                        mfLastZoomScale = pData->mfScaleDelta;

                        // Accumulate fractional zoom to avoid small zoom changes from being ignored
                        mfAccumulatedZoom += deltaBetweenEvents;
                        int nZoomChangePercent = mfAccumulatedZoom * 100;
                        mfAccumulatedZoom -= nZoomChangePercent / 100.0;

                        sal_uInt16 nZoom = mrGraphicWindow.GetZoom();
                        nZoom += nZoomChangePercent;
                        mrGraphicWindow.SetZoom(nZoom);
                    }
                    bCallBase = false;
                }
                break;
            }

            default: break;
        }
    }

    switch (rCEvt.GetCommand())
    {
        case CommandEventId::ExtTextInput:
            if (SmViewShell::IsInlineEditEnabled())
            {
                const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();
                assert(pData);
                const OUString& rText = pData->GetText();
                SmCursor& rCursor = GetCursor();
                OutputDevice& rDevice = GetOutputDevice();
                for (sal_Int32 i = 0; i < rText.getLength();)
                    CharInput(rText.iterateCodePoints(&i), rCursor, rDevice);
                bCallBase = false;
            }
            break;
        default:
            break;
    }
    return !bCallBase;
}

void SmGraphicWindow::SetZoom(sal_uInt16 Factor)
{
    if (comphelper::LibreOfficeKit::isActive())
        return;
    nZoom = std::clamp(Factor, MINZOOM, MAXZOOM);
    Fraction aFraction(nZoom, 100);
    SetGraphicMapMode(MapMode(SmMapUnit(), Point(), aFraction, aFraction));
    mxGraphic->SetTotalSize();
    SmViewShell& rViewSh = mxGraphic->GetView();
    rViewSh.GetViewFrame().GetBindings().Invalidate(SID_ATTR_ZOOM);
    rViewSh.GetViewFrame().GetBindings().Invalidate(SID_ATTR_ZOOMSLIDER);
}

void SmGraphicWindow::ZoomToFitInWindow()
{
    // set defined mapmode before calling 'LogicToPixel' below
    SetGraphicMapMode(MapMode(SmMapUnit()));

    assert(mxGraphic->GetDoc());
    Size aSize(mxGraphic->GetOutputDevice().LogicToPixel(mxGraphic->GetDoc()->GetSize()));
    Size aWindowSize(GetSizePixel());

    if (!aSize.IsEmpty())
    {
        tools::Long nVal = std::min ((85 * aWindowSize.Width())  / aSize.Width(),
                      (85 * aWindowSize.Height()) / aSize.Height());
        SetZoom ( sal::static_int_cast< sal_uInt16 >(nVal) );
    }
}

uno::Reference< XAccessible > SmGraphicWidget::CreateAccessible()
{
    if (!mxAccessible.is())
    {
        mxAccessible = new SmGraphicAccessible( this );
    }
    return mxAccessible;
}

/**************************************************************************/
SmGraphicController::SmGraphicController(SmGraphicWidget &rSmGraphic,
                        sal_uInt16          nId_,
                        SfxBindings     &rBindings) :
    SfxControllerItem(nId_, rBindings),
    rGraphic(rSmGraphic)
{
}

void SmGraphicController::StateChangedAtToolBoxControl(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    rGraphic.SetTotalSize();
    rGraphic.Invalidate();
    SfxControllerItem::StateChangedAtToolBoxControl (nSID, eState, pState);
}

/**************************************************************************/
SmEditController::SmEditController(SmEditWindow &rSmEdit,
                     sal_uInt16       nId_,
                     SfxBindings  &rBindings) :
    SfxControllerItem(nId_, rBindings),
    rEdit(rSmEdit)
{
}

void SmEditController::StateChangedAtToolBoxControl(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    const SfxStringItem *pItem =  dynamic_cast<const SfxStringItem*>( pState);

    if ((pItem != nullptr) && (rEdit.GetText() != pItem->GetValue()))
        rEdit.SetText(pItem->GetValue());
    SfxControllerItem::StateChangedAtToolBoxControl (nSID, eState, pState);
}

/**************************************************************************/
SmCmdBoxWindow::SmCmdBoxWindow(SfxBindings *pBindings_, SfxChildWindow *pChildWindow,
                               vcl::Window *pParent)
    : SfxDockingWindow(pBindings_, pChildWindow, pParent, u"EditWindow"_ustr, u"modules/smath/ui/editwindow.ui"_ustr)
    , m_xEdit(new SmEditWindow(*this, *m_xBuilder))
    , aController(*m_xEdit, SID_TEXT, *pBindings_)
    , bExiting(false)
    , aInitialFocusTimer("SmCmdBoxWindow aInitialFocusTimer")
{
    set_id(u"math_edit"_ustr);

    SetHelpId( HID_SMA_COMMAND_WIN );
    SetSizePixel(LogicToPixel(Size(292 , 94), MapMode(MapUnit::MapAppFont)));
    SetText(SmResId(STR_CMDBOXWINDOW));

    Hide();

    // Don't try to grab focus in inline edit mode
    if (!SmViewShell::IsInlineEditEnabled())
    {
        aInitialFocusTimer.SetInvokeHandler(LINK(this, SmCmdBoxWindow, InitialFocusTimerHdl));
        aInitialFocusTimer.SetTimeout(100);
    }
}

Point SmCmdBoxWindow::WidgetToWindowPos(const weld::Widget& rWidget, const Point& rPos)
{
    Point aRet(rPos);
    int x(0), y(0), width(0), height(0);
    rWidget.get_extents_relative_to(*m_xContainer, x, y, width, height);
    aRet.Move(x, y);
    aRet.Move(m_xBox->GetPosPixel().X(), m_xBox->GetPosPixel().Y());
    return aRet;
}

void SmCmdBoxWindow::ShowContextMenu(const Point& rPos)
{
    ToTop();
    SmViewShell *pViewSh = GetView();
    if (pViewSh)
        pViewSh->GetViewFrame().GetDispatcher()->ExecutePopup(u"edit"_ustr, this, &rPos);
}

void SmCmdBoxWindow::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        ShowContextMenu(rCEvt.GetMousePosPixel());
        return;
    }

    SfxDockingWindow::Command(rCEvt);
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
    m_xEdit.reset();
    SfxDockingWindow::dispose();
}

SmViewShell * SmCmdBoxWindow::GetView()
{
    SfxDispatcher *pDispatcher = GetBindings().GetDispatcher();
    SfxViewShell *pView = pDispatcher ? pDispatcher->GetFrame()->GetViewShell() : nullptr;
    return  dynamic_cast<SmViewShell*>( pView);
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

IMPL_LINK_NOARG( SmCmdBoxWindow, InitialFocusTimerHdl, Timer *, void )
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

        m_xEdit->GrabFocus();

        SmViewShell* pView = GetView();
        assert(pView);
        bool bInPlace = pView->GetViewFrame().GetFrame().IsInPlace();
        uno::Reference< frame::XFrame > xFrame( GetBindings().GetDispatcher()->GetFrame()->GetFrame().GetFrameInterface());
        if ( bInPlace )
        {
            uno::Reference<container::XChild> xModel(pView->GetDoc()->GetModel(),
                                                     uno::UNO_QUERY_THROW);
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
    const tools::Rectangle aRect( Point(), GetParent()->GetOutputSizePixel() );
    Point aTopLeft( Point( aRect.Left(),
                           aRect.Bottom() - GetSizePixel().Height() ) );
    Point aPos( GetParent()->OutputToScreenPixel( aTopLeft ) );
    if (aPos.X() < 0)
        aPos.setX( 0 );
    if (aPos.Y() < 0)
        aPos.setY( 0 );
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
        m_xEdit->GrabFocus();
}

SFX_IMPL_DOCKINGWINDOW_WITHID(SmCmdBoxWrapper, SID_CMDBOXWINDOW);

SmCmdBoxWrapper::SmCmdBoxWrapper(vcl::Window *pParentWindow, sal_uInt16 nId,
                                 SfxBindings *pBindings,
                                 SfxChildWinInfo *pInfo) :
    SfxChildWindow(pParentWindow, nId)
{
    VclPtrInstance<SmCmdBoxWindow> pDialog(pBindings, this, pParentWindow);
    SetWindow(pDialog);
    // make window docked to the bottom initially (after first start)
    SetAlignment(SfxChildAlignment::BOTTOM);
    pDialog->setDeferredProperties();
    pDialog->set_border_width(CMD_BOX_PADDING);
    pDialog->set_margin_top(CMD_BOX_PADDING_TOP);
    pDialog->Initialize(pInfo);
}

SFX_IMPL_SUPERCLASS_INTERFACE(SmViewShell, SfxViewShell)

void SmViewShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_TOOLS,
                                            SfxVisibilityFlags::Standard | SfxVisibilityFlags::FullScreen | SfxVisibilityFlags::Server,
                                            ToolbarId::Math_Toolbox);
    //Dummy-Objectbar, to avoid quiver while activating

    GetStaticInterface()->RegisterChildWindow(SmCmdBoxWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SfxInfoBarContainerChild::GetChildWindowId());

    GetStaticInterface()->RegisterChildWindow(::sfx2::sidebar::SidebarChildWindow::GetChildWindowId());
}

SFX_IMPL_NAMED_VIEWFACTORY(SmViewShell, "Default")
{
    SFX_VIEW_REGISTRATION(SmDocShell);
}

void SmViewShell::InnerResizePixel(const Point &rOfs, const Size &rSize, bool)
{
    Size aObjSize = GetObjectShell()->GetVisArea().GetSize();
    if ( !aObjSize.IsEmpty() )
    {
        Size aProvidedSize = GetWindow()->PixelToLogic(rSize, MapMode(SmMapUnit()));
        Fraction aZoomX(aProvidedSize.Width(), aObjSize.Width());
        Fraction aZoomY(aProvidedSize.Height(), aObjSize.Height());
        MapMode aMap(mxGraphicWindow->GetGraphicMapMode());
        aMap.SetScaleX(aZoomX);
        aMap.SetScaleY(aZoomY);
        mxGraphicWindow->SetGraphicMapMode(aMap);
    }

    SetBorderPixel( SvBorder() );
    mxGraphicWindow->SetPosSizePixel(rOfs, rSize);
    GetGraphicWidget().SetTotalSize();
}

void SmViewShell::OuterResizePixel(const Point &rOfs, const Size &rSize)
{
    mxGraphicWindow->SetPosSizePixel(rOfs, rSize);
    if (GetDoc()->IsPreview())
        mxGraphicWindow->ZoomToFitInWindow();
}

void SmViewShell::QueryObjAreaPixel( tools::Rectangle& rRect ) const
{
    rRect.SetSize(mxGraphicWindow->GetSizePixel());
}

void SmViewShell::SetZoomFactor( const Fraction &rX, const Fraction &rY )
{
    const Fraction &rFrac = std::min(rX, rY);
    mxGraphicWindow->SetZoom(sal::static_int_cast<sal_uInt16>(tools::Long(rFrac * Fraction( 100, 1 ))));

    //To avoid rounding errors base class regulates crooked values too
    //if necessary
    SfxViewShell::SetZoomFactor( rX, rY );
}

SfxPrinter* SmViewShell::GetPrinter(bool bCreate)
{
    SmDocShell* pDoc = GetDoc();
    if (pDoc->HasPrinter() || bCreate)
        return pDoc->GetPrinter();
    return nullptr;
}

sal_uInt16 SmViewShell::SetPrinter(SfxPrinter *pNewPrinter, SfxPrinterChangeFlags nDiffFlags )
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

std::unique_ptr<SfxTabPage> SmViewShell::CreatePrintOptionsPage(weld::Container* pPage, weld::DialogController* pController,
                                                       const SfxItemSet &rOptions)
{
    return SmPrintOptionsTabPage::Create(pPage, pController, rOptions);
}

SmEditWindow *SmViewShell::GetEditWindow()
{
    SmCmdBoxWrapper* pWrapper = static_cast<SmCmdBoxWrapper*>(
                                    GetViewFrame().GetChildWindow(SmCmdBoxWrapper::GetChildWindowId()));

    if (pWrapper != nullptr)
    {
        SmEditWindow& rEditWin = pWrapper->GetEditWindow();
        return &rEditWin;
    }

    return nullptr;
}

void SmViewShell::SetStatusText(const OUString& rText)
{
    maStatusText = rText;
    GetViewFrame().GetBindings().Invalidate(SID_TEXTSTATUS);
}

void SmViewShell::ShowError(const SmErrorDesc* pErrorDesc)
{
    assert(GetDoc());
    if (pErrorDesc || nullptr != (pErrorDesc = GetDoc()->GetParser()->GetError()) )
    {
        SetStatusText( pErrorDesc->m_aText );
        if (SmEditWindow* pEdit = GetEditWindow())
            pEdit->MarkError( Point( pErrorDesc->m_pNode->GetColumn(),
                                               pErrorDesc->m_pNode->GetRow()));
    }
}

void SmViewShell::NextError()
{
    assert(GetDoc());
    const SmErrorDesc   *pErrorDesc = GetDoc()->GetParser()->NextError();

    if (pErrorDesc)
        ShowError( pErrorDesc );
}

void SmViewShell::PrevError()
{
    assert(GetDoc());
    const SmErrorDesc   *pErrorDesc = GetDoc()->GetParser()->PrevError();

    if (pErrorDesc)
        ShowError( pErrorDesc );
}

void SmViewShell::Insert( SfxMedium& rMedium )
{
    SmDocShell *pDoc = GetDoc();
    bool bRet = false;

    uno::Reference <embed::XStorage> xStorage = rMedium.GetStorage();
    if (xStorage.is() && xStorage->getElementNames().hasElements())
    {
        if (xStorage->hasByName(u"content.xml"_ustr))
        {
            // is this a fabulous math package ?
            rtl::Reference<SmModel> xModel(dynamic_cast<SmModel*>(pDoc->GetModel().get()));
            SmXMLImportWrapper aEquation(xModel);    //!! modifies the result of pDoc->GetText() !!
            bRet = ERRCODE_NONE == aEquation.Import(rMedium);
        }
    }

    if (!bRet)
        return;

    OUString aText = pDoc->GetText();
    if (SmEditWindow *pEditWin = GetEditWindow())
        pEditWin->InsertText( aText );
    else
    {
        SAL_WARN( "starmath", "EditWindow missing" );
    }

    pDoc->Parse();
    pDoc->SetModified();

    SfxBindings &rBnd = GetViewFrame().GetBindings();
    rBnd.Invalidate(SID_GRAPHIC_SM);
    rBnd.Invalidate(SID_TEXT);
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
            rtl::Reference<SmModel> xModel(dynamic_cast<SmModel*>(pDoc->GetModel().get()));
            SmXMLImportWrapper aEquation(xModel);    //!! modifies the result of pDoc->GetText() !!
            bSuccess = ERRCODE_NONE == aEquation.Import(rMedium);
        }
    }

    if (!bSuccess)
        return;

    OUString aText = pDoc->GetText();
    if (SmEditWindow *pEditWin = GetEditWindow())
        pEditWin->InsertText(aText);
    else
        SAL_WARN( "starmath", "EditWindow missing" );

    pDoc->Parse();
    pDoc->SetModified();

    SfxBindings& rBnd = GetViewFrame().GetBindings();
    rBnd.Invalidate(SID_GRAPHIC_SM);
    rBnd.Invalidate(SID_TEXT);
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
                GetGraphicWidget().ShowCursor(bVal);
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
            mxGraphicWindow->ZoomToFitInWindow();
            break;

        case SID_ZOOMIN:
            mxGraphicWindow->SetZoom(mxGraphicWindow->GetZoom() + 25);
            break;

        case SID_ZOOMOUT:
            SAL_WARN_IF( mxGraphicWindow->GetZoom() < 25, "starmath", "incorrect sal_uInt16 argument" );
            mxGraphicWindow->SetZoom(mxGraphicWindow->GetZoom() - 25);
            break;

        case SID_COPYOBJECT:
        {
            //TODO/LATER: does not work because of UNO Tunneling - will be fixed later
            Reference< datatransfer::XTransferable > xTrans( GetDoc()->GetModel(), uno::UNO_QUERY );
            if( xTrans.is() )
            {
                auto pTrans = dynamic_cast<TransferableHelper*>(xTrans.get());
                if (pTrans)
                {
                    if (pWin)
                        pTrans->CopyToClipboard(pWin->GetClipboard());
                }
            }
        }
        break;

        case SID_PASTEOBJECT:
        {
            uno::Reference < io::XInputStream > xStrm;
            if (pWin)
            {
                TransferableDataHelper aData(TransferableDataHelper::CreateFromClipboard(pWin->GetClipboard()));
                SotClipboardFormatId nId;
                if( aData.GetTransferable().is() &&
                    ( aData.HasFormat( nId = SotClipboardFormatId::EMBEDDED_OBJ ) ||
                      (aData.HasFormat( SotClipboardFormatId::OBJECTDESCRIPTOR ) &&
                       aData.HasFormat( nId = SotClipboardFormatId::EMBED_SOURCE ))))
                    xStrm = aData.GetInputStream(nId, OUString());
            }

            if (xStrm.is())
            {
                try
                {
                    uno::Reference < embed::XStorage > xStorage =
                            ::comphelper::OStorageHelper::GetStorageFromInputStream( xStrm, ::comphelper::getProcessComponentContext() );
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
            if (IsInlineEditEnabled())
            {
                GetDoc()->GetCursor().Cut(&GetGraphicWindow());
                GetGraphicWidget().GrabFocus();
            }
            else if (pWin)
                pWin->Cut();
            break;

        case SID_COPY:
            if (IsInlineEditEnabled())
            {
                GetDoc()->GetCursor().Copy(&GetGraphicWindow());
                GetGraphicWidget().GrabFocus();
            }
            else if (pWin)
            {
                if (pWin->IsAllSelected())
                {
                    GetViewFrame().GetDispatcher()->ExecuteList(
                                SID_COPYOBJECT, SfxCallMode::RECORD,
                                { new SfxVoidItem(SID_COPYOBJECT) });
                }
                else
                    pWin->Copy();
            }
            break;

        case SID_PASTE:
            {
                if (IsInlineEditEnabled())
                {
                    GetDoc()->GetCursor().Paste(&GetGraphicWindow());
                    GetGraphicWidget().GrabFocus();
                    break;
                }

                bool bCallExec = nullptr == pWin;
                if( !bCallExec )
                {
                    if (pWin)
                    {
                        TransferableDataHelper aDataHelper(
                            TransferableDataHelper::CreateFromClipboard(
                                                        pWin->GetClipboard()));

                        if( aDataHelper.GetTransferable().is() &&
                            aDataHelper.HasFormat( SotClipboardFormatId::STRING ))
                            pWin->Paste();
                        else
                            bCallExec = true;
                    }
                }
                if( bCallExec )
                {
                    GetViewFrame().GetDispatcher()->ExecuteList(
                            SID_PASTEOBJECT, SfxCallMode::RECORD,
                            { new SfxVoidItem(SID_PASTEOBJECT) });
                }
            }
            break;

        case SID_DELETE:
            if (IsInlineEditEnabled())
            {
                if (!GetDoc()->GetCursor().HasSelection())
                {
                    GetDoc()->GetCursor().Move(&GetGraphicWindow().GetGraphicWidget().GetOutputDevice(), MoveRight, false);
                    if (!GetDoc()->GetCursor().HasComplexSelection())
                        GetDoc()->GetCursor().Delete();
                }
                else
                    GetDoc()->GetCursor().Delete();
                GetGraphicWidget().GrabFocus();
            }
            else if (pWin)
                pWin->Delete();
            break;

        case SID_SELECT:
            if (pWin)
                pWin->SelectAll();
            break;

        case SID_INSERTCOMMANDTEXT:
        {
            const SfxStringItem& rItem = rReq.GetArgs()->Get(SID_INSERTCOMMANDTEXT);

            if (IsInlineEditEnabled())
            {
                GetDoc()->GetCursor().InsertCommandText(rItem.GetValue());
                GetGraphicWidget().GrabFocus();
            }
            else if (pWin)
            {
                pWin->InsertText(rItem.GetValue());
            }
            break;

        }

        case SID_INSERTSPECIAL:
        {
            const SfxStringItem& rItem = rReq.GetArgs()->Get(SID_INSERTSPECIAL);

            if (IsInlineEditEnabled())
                GetDoc()->GetCursor().InsertSpecial(rItem.GetValue());
            else if (pWin)
                pWin->InsertText(rItem.GetValue());
            break;
        }

        case SID_IMPORT_FORMULA:
        {
            mpRequest.reset(new SfxRequest( rReq ));
            mpDocInserter.reset(new ::sfx2::DocumentInserter(pWin ? pWin->GetFrameWeld() : nullptr,
                              GetDoc()->GetFactory().GetFactoryName()));
            mpDocInserter->StartExecuteModal( LINK( this, SmViewShell, DialogClosedHdl ) );
            break;
        }

        case SID_IMPORT_MATHML_CLIPBOARD:
        {
            if (pWin)
            {
                TransferableDataHelper aDataHelper(TransferableDataHelper::CreateFromClipboard(pWin->GetClipboard()));
                uno::Reference < io::XInputStream > xStrm;
                if  ( aDataHelper.GetTransferable().is() )
                {
                    SotClipboardFormatId nId = SotClipboardFormatId::MATHML;
                    if (aDataHelper.HasFormat(nId))
                    {
                        xStrm = aDataHelper.GetInputStream(nId, u""_ustr);
                        if (xStrm.is())
                        {
                            SfxMedium aClipboardMedium;
                            aClipboardMedium.GetItemSet(); //generate initial itemset, not sure if necessary
                            std::shared_ptr<const SfxFilter> pMathFilter =
                                SfxFilter::GetFilterByName(MATHML_XML);
                            aClipboardMedium.SetFilter(pMathFilter);
                            aClipboardMedium.setStreamToLoadFrom(xStrm, true /*bIsReadOnly*/);
                            InsertFrom(aClipboardMedium);
                            GetDoc()->UpdateText();
                        }
                    }
                    else
                    {
                        nId = SotClipboardFormatId::STRING;
                        if (aDataHelper.HasFormat(nId))
                        {
                            // In case of FORMAT_STRING no stream exists, need to generate one
                            OUString aString;
                            if (aDataHelper.GetString( nId, aString))
                            {
                                // tdf#117091 force xml declaration to exist
                                if (!aString.startsWith("<?xml"))
                                    aString = "<?xml version=\"1.0\"?>\n" + aString;

                                SfxMedium aClipboardMedium;
                                aClipboardMedium.GetItemSet(); //generates initial itemset, not sure if necessary
                                std::shared_ptr<const SfxFilter> pMathFilter =
                                    SfxFilter::GetFilterByName(MATHML_XML);
                                aClipboardMedium.SetFilter(pMathFilter);

                                SvMemoryStream aStrm( const_cast<sal_Unicode *>(aString.getStr()), aString.getLength() * sizeof(sal_Unicode), StreamMode::READ);
                                uno::Reference<io::XInputStream> xStrm2( new ::utl::OInputStreamWrapper(aStrm) );
                                aClipboardMedium.setStreamToLoadFrom(xStrm2, true /*bIsReadOnly*/);
                                InsertFrom(aClipboardMedium);
                                GetDoc()->UpdateText();
                            }
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
                const SfxStringItem& rItem = rReq.GetArgs()->Get(SID_TEXTSTATUS);

                SetStatusText(rItem.GetValue());
            }

            break;
        }

        case SID_GETEDITTEXT:
            if (pWin && !pWin->GetText().isEmpty())
                GetDoc()->SetText( pWin->GetText() );
            break;

        case SID_ATTR_ZOOM:
        {
            if ( !GetViewFrame().GetFrame().IsInPlace() )
            {
                const SfxItemSet *pSet = rReq.GetArgs();
                if ( pSet )
                {
                    ZoomByItemSet(pSet);
                }
                else
                {
                    SfxItemSetFixed<SID_ATTR_ZOOM, SID_ATTR_ZOOM> aSet( SmDocShell::GetPool() );
                    aSet.Put( SvxZoomItem( SvxZoomType::PERCENT, mxGraphicWindow->GetZoom()));
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    ScopedVclPtr<AbstractSvxZoomDialog> xDlg(pFact->CreateSvxZoomDialog(GetViewFrame().GetWindow().GetFrameWeld(), aSet));
                    xDlg->SetLimits( MINZOOM, MAXZOOM );
                    if (xDlg->Execute() != RET_CANCEL)
                        ZoomByItemSet(xDlg->GetOutputItemSet());
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
                mxGraphicWindow->SetZoom(nCurrentZoom);
            }
        }
        break;

        case SID_ELEMENTSDOCKINGWINDOW:
        {
            // First make sure that the sidebar is visible
            GetViewFrame().ShowChildWindow(SID_SIDEBAR);

            sfx2::sidebar::Sidebar::TogglePanel(u"MathElementsPanel",
                                                GetViewFrame().GetFrame().GetFrameInterface());
            GetViewFrame().GetBindings().Invalidate( SID_ELEMENTSDOCKINGWINDOW );

            rReq.Ignore ();
        }
        break;

        case SID_CMDBOXWINDOW:
        {
            GetViewFrame().ToggleChildWindow(SID_CMDBOXWINDOW);
            GetViewFrame().GetBindings().Invalidate(SID_CMDBOXWINDOW);
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
                    pEditEngine->UndoActionEnd();
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
            if (!pDev || pDev->GetFontFaceCollectionCount() == 0)
                pDev = &SM_MOD()->GetDefaultVirtualDev();
            SAL_WARN_IF( !pDev, "starmath", "device for font list missing" );

            SmModule *pp = SM_MOD();
            SmSymbolDialog aDialog(pWin ? pWin->GetFrameWeld() : nullptr, pDev, pp->GetSymbolManager(), *this);
            aDialog.run();
        }
        break;

        case SID_CHARMAP:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxStringItem* pItem = nullptr;
            if (pArgs && SfxItemState::SET == pArgs->GetItemState(SID_CHARMAP, true, &pItem))
            {
                if (IsInlineEditEnabled())
                    GetDoc()->GetCursor().InsertText(pItem->GetValue());
                else if (pWin)
                    pWin->InsertText(pItem->GetValue());
                break;
            }

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            SfxAllItemSet aSet(GetViewFrame().GetObjectShell()->GetPool());
            aSet.Put(SfxBoolItem(FN_PARAM_1, false));
            aSet.Put(SfxStringItem(SID_FONT_NAME,
                                   GetDoc()->GetFormat().GetFont(FNT_VARIABLE).GetFamilyName()));
            VclPtr<SfxAbstractDialog> pDialog(
                pFact->CreateCharMapDialog(pWin ? pWin->GetFrameWeld() : nullptr, aSet,
                                           GetViewFrame().GetFrame().GetFrameInterface()));
            pDialog->StartExecuteAsync(
                [pDialog] (sal_Int32 /*nResult*/)->void
                {
                    pDialog->disposeOnce();
                }
            );
        }
        break;

        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
            bool bRTL = rReq.GetSlot() == SID_ATTR_PARA_RIGHT_TO_LEFT;
            GetDoc()->SetRightToLeft(bRTL);
            GetGraphicWindow().GetGraphicWidget().GetOutputDevice().EnableRTL(bRTL);
            GetViewFrame().GetBindings().Invalidate(bRTL ? SID_ATTR_PARA_LEFT_TO_RIGHT : SID_ATTR_PARA_RIGHT_TO_LEFT);
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
            if (IsInlineEditEnabled())
            {
                if (!GetDoc()->GetCursor().HasSelection())
                    rSet.DisableItem(nWh);
            }
            else if (! pEditWin || ! pEditWin->IsSelected())
                rSet.DisableItem(nWh);
            break;

        case SID_PASTE:
            if (pEditWin)
            {
                TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromClipboard(
                                                        pEditWin->GetClipboard()) );

                mbPasteState = aDataHelper.GetTransferable().is() &&
                 ( aDataHelper.HasFormat( SotClipboardFormatId::STRING ) ||
                   aDataHelper.HasFormat( SotClipboardFormatId::EMBEDDED_OBJ ) ||
                   (aDataHelper.HasFormat( SotClipboardFormatId::OBJECTDESCRIPTOR )
                      && aDataHelper.HasFormat( SotClipboardFormatId::EMBED_SOURCE )));
            }
            if( !mbPasteState )
                rSet.DisableItem( nWh );
            break;

        case SID_ATTR_ZOOM:
            rSet.Put(SvxZoomItem( SvxZoomType::PERCENT, mxGraphicWindow->GetZoom()));
            [[fallthrough]];
        case SID_ZOOMIN:
        case SID_ZOOMOUT:
        case SID_ZOOM_OPTIMAL:
            if ( GetViewFrame().GetFrame().IsInPlace() )
                rSet.DisableItem( nWh );
            break;

        case SID_ATTR_ZOOMSLIDER :
            {
                const sal_uInt16 nCurrentZoom = mxGraphicWindow->GetZoom();
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
                rSet.Put(SfxStringItem(nWh, maStatusText));
            }
            break;

        case SID_FORMULACURSOR:
            {
                if (IsInlineEditEnabled())
                    rSet.DisableItem(nWh);
                else
                    rSet.Put(SfxBoolItem(nWh, SM_MOD()->GetConfig()->IsShowFormulaCursor()));
            }
            break;
        case SID_ELEMENTSDOCKINGWINDOW:
            {
                const bool bState = sfx2::sidebar::Sidebar::IsPanelVisible(
                    u"MathElementsPanel", GetViewFrame().GetFrame().GetFrameInterface());
                rSet.Put(SfxBoolItem(SID_ELEMENTSDOCKINGWINDOW, bState));
            }
            break;
        case SID_CMDBOXWINDOW:
            {
                bool bState = false;
                auto pCmdWin = GetViewFrame().GetChildWindow(SID_CMDBOXWINDOW);
                if (pCmdWin)
                    bState = pCmdWin->IsVisible();
                rSet.Put(SfxBoolItem(SID_CMDBOXWINDOW, bState));
            }
            break;
        case SID_ATTR_PARA_LEFT_TO_RIGHT:
            rSet.Put(SfxBoolItem(nWh, !GetDoc()->GetFormat().IsRightToLeft()));
            break;

        case SID_ATTR_PARA_RIGHT_TO_LEFT:
            rSet.Put(SfxBoolItem(nWh, GetDoc()->GetFormat().IsRightToLeft()));
            break;
        }
    }
}

namespace
{
css::uno::Reference<css::ui::XSidebar>
getSidebarFromModel(const css::uno::Reference<css::frame::XModel>& xModel)
{
    css::uno::Reference<css::container::XChild> xChild(xModel, css::uno::UNO_QUERY);
    if (!xChild.is())
        return nullptr;
    css::uno::Reference<css::frame::XModel> xParent(xChild->getParent(), css::uno::UNO_QUERY);
    if (!xParent.is())
        return nullptr;
    css::uno::Reference<css::frame::XController2> xController(xParent->getCurrentController(),
                                                              css::uno::UNO_QUERY);
    if (!xController.is())
        return nullptr;
    css::uno::Reference<css::ui::XSidebarProvider> xSidebarProvider = xController->getSidebar();
    if (!xSidebarProvider.is())
        return nullptr;
    return xSidebarProvider->getSidebar();
}
class SmController : public SfxBaseController
{
public:
    SmController(SfxViewShell& rViewShell)
        : SfxBaseController(&rViewShell)
        , mpSelectionChangeHandler(new svx::sidebar::SelectionChangeHandler(
              GetContextName, this, vcl::EnumContext::Context::Math))
    {
        rViewShell.SetContextName(GetContextName());
    }
    // No need to call mpSelectionChangeHandler->Disconnect() unless SmController implements XSelectionSupplier
    // ~SmController() { mpSelectionChangeHandler->Disconnect(); }

    // css::frame::XController
    void SAL_CALL attachFrame(const css::uno::Reference<css::frame::XFrame>& xFrame) override
    {
        SfxBaseController::attachFrame(xFrame);

        if (comphelper::LibreOfficeKit::isActive())
        {
            CopyLokViewCallbackFromFrameCreator();
            // In lok mode, DocumentHolder::ShowUI is not called on OLE in-place activation,
            // because respective code is skipped in OCommonEmbeddedObject::SwitchStateTo_Impl,
            // so sidebar controller does not get registered properly; do it here
            if (auto xSidebar = getSidebarFromModel(getModel()))
            {
                auto pSidebar = dynamic_cast<sfx2::sidebar::SidebarController*>(xSidebar.get());
                assert(pSidebar);
                pSidebar->registerSidebarForFrame(this);
                pSidebar->updateModel(getModel());
            }
        }

        // No need to call mpSelectionChangeHandler->Connect() unless SmController implements XSelectionSupplier
        mpSelectionChangeHandler->selectionChanged({}); // Installs the correct context
    }

    virtual void SAL_CALL dispose() override
    {
        if (comphelper::LibreOfficeKit::isActive())
            if (auto pViewShell = GetViewShell_Impl())
                pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CURSOR_VISIBLE,
                                                       OString::boolean(false));

        SfxBaseController::dispose();
    }

private:
    static OUString GetContextName() { return u"Math"_ustr; } // Static constant for now

    rtl::Reference<svx::sidebar::SelectionChangeHandler> mpSelectionChangeHandler;
};
}

SmViewShell::SmViewShell(SfxViewFrame& rFrame_, SfxViewShell *)
    : SfxViewShell(rFrame_, SfxViewShellFlags::HAS_PRINTOPTIONS)
    , mxGraphicWindow(VclPtr<SmGraphicWindow>::Create(*this))
    , maGraphicController(mxGraphicWindow->GetGraphicWidget(), SID_GRAPHIC_SM, rFrame_.GetBindings())
    , mbPasteState(false)
{
    SetStatusText(OUString());
    SetWindow(mxGraphicWindow.get());
    SfxShell::SetName(u"SmView"_ustr);
    SfxShell::SetUndoManager( &GetDoc()->GetEditEngine().GetUndoManager() );
    SetController(new SmController(*this));
}

SmViewShell::~SmViewShell()
{
    //!! this view shell is not active anymore !!
    // Thus 'SmGetActiveView' will give a 0 pointer.
    // Thus we need to supply this view as argument
    if (SmEditWindow *pEditWin = GetEditWindow())
        pEditWin->DeleteEditView();
    mxGraphicWindow.disposeAndClear();
}

void SmViewShell::Deactivate( bool bIsMDIActivate )
{
    if (SmEditWindow *pEdit = GetEditWindow())
        pEdit->Flush();

    SfxViewShell::Deactivate( bIsMDIActivate );
}

void SmViewShell::Activate( bool bIsMDIActivate )
{
    SfxViewShell::Activate( bIsMDIActivate );

    if (IsInlineEditEnabled())
    {
        // In LOK, activate in-place editing
        GetGraphicWidget().GrabFocus();
    }
    else if (SmEditWindow *pEdit = GetEditWindow())
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

IMPL_LINK( SmViewShell, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    assert(_pFileDlg && "SmViewShell::DialogClosedHdl(): no file dialog");
    assert(mpDocInserter && "ScDocShell::DialogClosedHdl(): no document inserter");

    if ( ERRCODE_NONE == _pFileDlg->GetError() )
    {
        std::unique_ptr<SfxMedium> pMedium = mpDocInserter->CreateMedium();

        if ( pMedium )
        {
            if ( pMedium->IsStorage() )
                Insert( *pMedium );
            else
                InsertFrom( *pMedium );
            pMedium.reset();

            SmDocShell* pDoc = GetDoc();
            pDoc->UpdateText();
            pDoc->ArrangeFormula();
            pDoc->Repaint();
            // adjust window, repaint, increment ModifyCount,...
            GetViewFrame().GetBindings().Invalidate(SID_GRAPHIC_SM);
        }
    }

    mpRequest->SetReturnValue( SfxBoolItem( mpRequest->GetSlot(), true ) );
    mpRequest->Done();
}

void SmViewShell::Notify( SfxBroadcaster& , const SfxHint& rHint )
{
    switch( rHint.GetId() )
    {
        case SfxHintId::ModeChanged:
        case SfxHintId::DocChanged:
            GetViewFrame().GetBindings().InvalidateAll(false);
        break;
        default:
        break;
    }
}

bool SmViewShell::IsInlineEditEnabled()
{
    return comphelper::LibreOfficeKit::isActive()
           || SM_MOD()->GetConfig()->IsInlineEditEnable();
}

void SmViewShell::StartMainHelp()
{
    Help* pHelp = Application::GetHelp();
    if (pHelp)
        pHelp->Start(HID_SMA_MAIN_HELP, GetViewFrame().GetFrameWeld());
}

void SmViewShell::ZoomByItemSet(const SfxItemSet *pSet)
{
    assert(pSet);
    const SvxZoomItem &rZoom = pSet->Get(SID_ATTR_ZOOM);
    switch( rZoom.GetType() )
    {
        case SvxZoomType::PERCENT:
            mxGraphicWindow->SetZoom(sal::static_int_cast<sal_uInt16>(rZoom.GetValue ()));
            break;

        case SvxZoomType::OPTIMAL:
            mxGraphicWindow->ZoomToFitInWindow();
            break;

        case SvxZoomType::PAGEWIDTH:
        case SvxZoomType::WHOLEPAGE:
        {
            const MapMode aMap( SmMapUnit() );
            SfxPrinter *pPrinter = GetPrinter( true );
            tools::Rectangle  OutputRect(Point(), pPrinter->GetOutputSize());
            Size       OutputSize(pPrinter->LogicToPixel(Size(OutputRect.GetWidth(),
                                                              OutputRect.GetHeight()), aMap));
            Size       GraphicSize(pPrinter->LogicToPixel(GetDoc()->GetSize(), aMap));
            if (GraphicSize.Width() <= 0 || GraphicSize.Height() <= 0)
                break;
            sal_uInt16 nZ = std::min(o3tl::convert(OutputSize.Width(), 100, GraphicSize.Width()),
                                     o3tl::convert(OutputSize.Height(), 100, GraphicSize.Height()));
            mxGraphicWindow->SetZoom(nZ);
            break;
        }
        default:
            break;
    }
}

std::optional<OString> SmViewShell::getLOKPayload(int nType, int nViewId) const
{
    switch (nType)
    {
        case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        {
            OString sRectangle;
            if (const SmGraphicWidget& widget = GetGraphicWidget(); widget.IsCursorVisible())
            {
                SmCursor& rCursor = GetDoc()->GetCursor();
                OutputDevice& rOutDev = const_cast<SmGraphicWidget&>(widget).GetOutputDevice();
                tools::Rectangle aCaret = rCursor.GetCaretRectangle(rOutDev);
                Point aFormulaDrawPos = widget.GetFormulaDrawPos();
                aCaret.Move(aFormulaDrawPos.X(), aFormulaDrawPos.Y());
                LokStarMathHelper helper(SfxViewShell::Current());
                tools::Rectangle aBounds = helper.GetBoundingBox();
                aCaret.Move(aBounds.Left(), aBounds.Top());
                sRectangle = aCaret.toString();
            }
            return SfxLokHelper::makeVisCursorInvalidation(nViewId, sRectangle, false, {});
        }
        case LOK_CALLBACK_TEXT_SELECTION:
        {
            OString sRectangle;
            if (const SmGraphicWidget& widget = GetGraphicWidget(); widget.IsCursorVisible())
            {
                SmCursor& rCursor = GetDoc()->GetCursor();
                OutputDevice& rOutDev = const_cast<SmGraphicWidget&>(widget).GetOutputDevice();
                tools::Rectangle aSelection = rCursor.GetSelectionRectangle(rOutDev);
                if (!aSelection.IsEmpty())
                {
                    Point aFormulaDrawPos = widget.GetFormulaDrawPos();
                    aSelection.Move(aFormulaDrawPos.X(), aFormulaDrawPos.Y());
                    LokStarMathHelper helper(SfxViewShell::Current());
                    tools::Rectangle aBounds = helper.GetBoundingBox();

                    aSelection.Move(aBounds.Left(), aBounds.Top());
                    sRectangle = aSelection.toString();
                }
            }
            return sRectangle;
        }
        case LOK_CALLBACK_TEXT_SELECTION_START:
        case LOK_CALLBACK_TEXT_SELECTION_END:
        case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
            return {};
    }
    return SfxViewShell::getLOKPayload(nType, nViewId); // aborts
}

void SmViewShell::SendCaretToLOK() const
{
    const int nViewId = sal_Int32(GetViewShellId());
    if (const auto& payload = getLOKPayload(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, nViewId))
    {
        libreOfficeKitViewCallbackWithViewId(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR,
                                             *payload, nViewId);
    }
    if (const auto& payload = getLOKPayload(LOK_CALLBACK_TEXT_SELECTION, nViewId))
    {
        libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, *payload);
    }
}

void SmViewShell::InvalidateSlots()
{
    auto& rBind = GetViewFrame().GetBindings();
    rBind.Invalidate(SID_COPY);
    rBind.Invalidate(SID_CUT);
    rBind.Invalidate(SID_DELETE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
