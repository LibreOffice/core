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

#include "PaneDockingWindow.hxx"
#include "Window.hxx"
#include "ViewShellBase.hxx"
#include "framework/FrameworkHelper.hxx"
#include "sdresid.hxx"
#include "res_bmp.hrc"
#include "app.hrc"
#include "strings.hrc"
#include <svx/svxids.hrc>
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/svapp.hxx>
#include <tools/wintypes.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sfx2::TitledDockingWindow;

#define VERT_POS_FIX 3

namespace sd {

PaneDockingWindow::PaneDockingWindow(
        SfxBindings *_pBindings, SfxChildWindow *pChildWindow, vcl::Window* pParent,
        const OUString& rsTitle )
        : TitledDockingWindow(_pBindings, pChildWindow, pParent, WB_MOVEABLE|WB_CLOSEABLE|WB_HIDE|WB_3DLOOK)
{
    SetTitle(rsTitle);
    SetSizePixel(LogicToPixel(Size(80,200), MAP_APPFONT));
}

PaneDockingWindow::~PaneDockingWindow()
{
}

void PaneDockingWindow::StateChanged( StateChangedType nType )
{
    switch (nType)
    {
        case StateChangedType::InitShow:
            Resize();
            GetContentWindow().SetStyle(GetContentWindow().GetStyle() | WB_DIALOGCONTROL);
            break;

        case StateChangedType::Visible:
        {
            // The visibility of the docking window has changed.  Tell the
            // ConfigurationController so that it can activate or deactivate
            // a/the view for the pane.
            // Without this the side panes remain empty after closing an
            // in-place slide show.
            ViewShellBase* pBase = ViewShellBase::GetViewShellBase(
                GetBindings().GetDispatcher()->GetFrame());
            if (pBase != nullptr)
            {
                framework::FrameworkHelper::Instance(*pBase)->UpdateConfiguration();
            }
        }
        break;

        default:;
    }
    SfxDockingWindow::StateChanged (nType);
}

void PaneDockingWindow::MouseButtonDown (const MouseEvent& rEvent)
{
    if (rEvent.GetButtons() == MOUSE_LEFT)
    {
        // For some strange reason we have to set the WB_DIALOGCONTROL at
        // the content window in order to have it pass focus to its content
        // window.  Without setting this flag here that works only on views
        // that have not been taken from the cash and relocated to this pane
        // docking window.
        GetContentWindow().SetStyle(GetContentWindow().GetStyle() | WB_DIALOGCONTROL);
        GetContentWindow().GrabFocus();

        ViewShellBase* pBase = ViewShellBase::GetViewShellBase(
        GetBindings().GetDispatcher()->GetFrame());
        if (pBase != NULL)
        {
            SfxDispatcher* pDispatcher = pBase->GetViewFrame()->GetDispatcher();
            Point aPos(rEvent.GetPosPixel());
            if (aPos.Y() > m_nTitleBarHeight + EXTRA_SLIDEPANE_TOP_HEIGHT)
                pDispatcher->Execute(SID_INSERTPAGE_QUICK,
                                SfxCallMode::SYNCHRON | SfxCallMode::RECORD);
            else if (aPos.Y() > m_nTitleBarHeight - VERT_POS_FIX - 10)
                pDispatcher->Execute(SID_PRESENTATION,
                                SfxCallMode::ASYNCHRON);
        }
    }

    SfxDockingWindow::MouseButtonDown(rEvent);
}

void PaneDockingWindow::SetValidSizeRange (const Range& rValidSizeRange)
{
    SplitWindow* pSplitWindow = dynamic_cast<SplitWindow*>(GetParent());
    if (pSplitWindow != nullptr)
    {
        const sal_uInt16 nId (pSplitWindow->GetItemId(static_cast< vcl::Window*>(this)));
        const sal_uInt16 nSetId (pSplitWindow->GetSet(nId));
        // Because the PaneDockingWindow paints its own decoration, we have
        // to compensate the valid size range for that.
        const SvBorder aBorder (GetDecorationBorder());
        sal_Int32 nCompensation (pSplitWindow->IsHorizontal()
            ? aBorder.Top() + aBorder.Bottom()
            : aBorder.Left() + aBorder.Right());
        pSplitWindow->SetItemSizeRange(
            nSetId,
            Range(
                rValidSizeRange.Min() + nCompensation,
                rValidSizeRange.Max() + nCompensation));
    }
}

PaneDockingWindow::Orientation PaneDockingWindow::GetOrientation() const
{
    SplitWindow* pSplitWindow = dynamic_cast<SplitWindow*>(GetParent());
    if (pSplitWindow == nullptr)
        return UnknownOrientation;
    else if (pSplitWindow->IsHorizontal())
        return HorizontalOrientation;
    else
        return VerticalOrientation;
}

void PaneDockingWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& i_rArea)
{
    TitledDockingWindow::Paint(rRenderContext, i_rArea);
    rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
     const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    // play icon and text
    int nWidth = rRenderContext.GetTextWidth(SdResId(STR_DRAW_COMMONTASK_TOOLBOX));
    int nPos = i_rArea.Right()/2 - (29 + 7 + nWidth)/2;
    if (nPos < 20)
        nPos = 20;
    rRenderContext.SetLineColor(rStyleSettings.GetHighlightColor());
    rRenderContext.SetFillColor();
    int nHeight = m_nTitleBarHeight + EXTRA_SLIDEPANE_TOP_HEIGHT/2 - VERT_POS_FIX;
    Rectangle aPlayBox(Rectangle(nPos, nHeight - 14, nPos + 29, nHeight + 14));
    rRenderContext.DrawRect(aPlayBox, 3, 3);
    aPlayBox.Left()++;
    aPlayBox.Right()--;
    aPlayBox.Top()++;
    aPlayBox.Bottom()--;
    rRenderContext.DrawRect(aPlayBox, 2, 2);
    rRenderContext.DrawText(Rectangle(nPos + 29 + 7, nHeight - 14, i_rArea.Right() - 27 , nHeight + 14), SdResId(STR_DRAW_COMMONTASK_TOOLBOX),
            DrawTextFlags::Left | DrawTextFlags::VCenter | DrawTextFlags::EndEllipsis | DrawTextFlags::Clip);

    for (int i = 0; i <= 5; i++)
    {
        rRenderContext.DrawRect(Rectangle(aPlayBox.Left()+10+i*2, aPlayBox.Top()+9+i, aPlayBox.Left()+10+i*2, aPlayBox.Bottom()-7-i));
        rRenderContext.DrawRect(Rectangle(aPlayBox.Left()+10+i*2+1, aPlayBox.Top()+9+i, aPlayBox.Left()+10+i*2+1, aPlayBox.Bottom()-7-i));
    }

    // don't draw new slide button without place for it
    if (i_rArea.Bottom() < m_nTitleBarHeight + EXTRA_SLIDEPANE_TOP_HEIGHT + EXTRA_SLIDEPANE_BOTTOM_HEIGHT)
        return;

    nWidth = rRenderContext.GetTextWidth(SdResId(STR_INSERTPAGE));
    nPos = i_rArea.Right()/2 - (14 + 7 + nWidth + 10)/2;
    if (nPos < i_rArea.Right() * 0.1 + 7)
        nPos = i_rArea.Right() * 0.1 + 7;

    Rectangle aNewBox(i_rArea.Right() * 0.1, i_rArea.Bottom() - EXTRA_SLIDEPANE_BOTTOM_HEIGHT * 5/6, i_rArea.Right() * 0.9, i_rArea.Bottom() - EXTRA_SLIDEPANE_BOTTOM_HEIGHT / 6);
    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
    rRenderContext.SetFillColor();
    rRenderContext.DrawRect(aNewBox);
    aNewBox.Left()++;
    aNewBox.Right()--;
    aNewBox.Top()++;
    aNewBox.Bottom()--;
    rRenderContext.DrawRect(aNewBox);

    rRenderContext.DrawRect(Rectangle(nPos + 7, i_rArea.Bottom() - EXTRA_SLIDEPANE_BOTTOM_HEIGHT/2-1, nPos + 7 + 13, i_rArea.Bottom() - EXTRA_SLIDEPANE_BOTTOM_HEIGHT/2));
    rRenderContext.DrawRect(Rectangle(nPos + 7 + 6, i_rArea.Bottom() - EXTRA_SLIDEPANE_BOTTOM_HEIGHT/2-1 - 6, nPos + 7 + 7, i_rArea.Bottom() - EXTRA_SLIDEPANE_BOTTOM_HEIGHT/2 + 6));

    aNewBox.Left() = nPos + 7 + 14 + 7;
    aNewBox.Right() -= 10;

    rRenderContext.DrawText(aNewBox, SdResId(STR_INSERTPAGE),
            DrawTextFlags::Left | DrawTextFlags::VCenter | DrawTextFlags::EndEllipsis | DrawTextFlags::Clip);

    rRenderContext.Pop();

}

} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
