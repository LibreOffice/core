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

#include <tabsplit.hxx>
#include <viewdata.hxx>

#include <vcl/settings.hxx>

ScTabSplitter::ScTabSplitter( vcl::Window* pParent, WinBits nWinStyle, const ScViewData* pData ) :
    Splitter(pParent, nWinStyle),
    pViewData(pData)
{
    SetFixed(false);
    EnableRTL(false);
}

ScTabSplitter::~ScTabSplitter()
{
}

void ScTabSplitter::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (bFixed)
        Window::MouseButtonDown( rMEvt );
    else
        Splitter::MouseButtonDown( rMEvt );
}

void ScTabSplitter::SetFixed(bool bSet)
{
    bFixed = bSet;
    if (bSet)
        SetPointer(PointerStyle::Arrow);
    else if (IsHorizontal())
        SetPointer(PointerStyle::HSplit);
    else
        SetPointer(PointerStyle::VSplit);
}

void ScTabSplitter::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect )
{
    rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    if (IsHorizontal())
    {
        switch (pViewData->GetHSplitMode())
        {
            case SC_SPLIT_NONE:
            {
                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());
                rRenderContext.DrawRect(tools::Rectangle(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom()));

                // Draw handle
                rRenderContext.SetLineColor(COL_BLACK);
                rRenderContext.SetFillColor(COL_BLACK);
                const tools::Long xc = rRect.Right() + rRect.Left();
                const tools::Long h4 = rRect.GetHeight() / 4;
                // First xc fraction is truncated, second one is rounded. This will draw a centered line
                // in handlers with odd width and a centered rectangle in those with even width.
                rRenderContext.DrawRect(tools::Rectangle(Point(xc / 2, rRect.Top() + h4),
                                                  Point((xc + 1) / 2, rRect.Bottom() - h4)));
                break;
            }
            case SC_SPLIT_NORMAL:
                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());
                rRenderContext.DrawRect(tools::Rectangle(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom()));
                break;
            case SC_SPLIT_FIX:
                // Nothing to draw
                break;
        }
    }
    else
    {
        switch (pViewData->GetVSplitMode())
        {
            case SC_SPLIT_NONE:
            {
                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());
                rRenderContext.DrawRect(tools::Rectangle(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom()));

                 // Draw handle
                rRenderContext.SetLineColor(COL_BLACK);
                rRenderContext.SetFillColor(COL_BLACK);
                const tools::Long yc = rRect.Top() + rRect.Bottom();
                const tools::Long w4 = rRect.GetWidth() / 4;
                // First yc fraction is truncated, second one is rounded. This will draw a centered line
                // in handlers with odd height and a centered rectangle in those with even height.
                GetOutDev()->DrawRect(tools::Rectangle(Point(rRect.Left() + w4, yc / 2),
                                   Point(rRect.Right() - w4, (yc + 1) / 2)));
                break;
            }
            case SC_SPLIT_NORMAL:
                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());
                rRenderContext.DrawRect(tools::Rectangle(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom()));
                break;
            case SC_SPLIT_FIX:
                // Nothing to draw
                break;
        }
    }

    rRenderContext.Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
