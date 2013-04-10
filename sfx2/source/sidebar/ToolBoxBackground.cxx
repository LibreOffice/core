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
#include "ToolBoxBackground.hxx"
#include "Paint.hxx"
#include "DrawHelper.hxx"
#include "Tools.hxx"
#include "sfx2/sidebar/Theme.hxx"

#include <vcl/toolbox.hxx>
#include <vcl/gradient.hxx>
#include <svl/smplhint.hxx>


namespace sfx2 { namespace sidebar {

ToolBoxBackground::ToolBoxBackground (Window* pParentWindow)
    : Window(pParentWindow, WB_DIALOGCONTROL),
      maPadding(Tools::RectangleToSvBorder(Theme::GetRectangle(Theme::Rect_ToolBoxPadding)))
{
    SetBackground(Theme::GetPaint(Theme::Paint_ToolBoxBackground).GetWallpaper());

#ifdef DEBUG
    SetText(A2S("ToolBoxBackground"));
#endif
}




ToolBoxBackground::~ToolBoxBackground (void)
{
    Link aEventListener (LINK(this, ToolBoxBackground, WindowEventHandler));
    if (GetChildCount() > 0)
        GetChild(0)->RemoveEventListener(aEventListener);
}




Point ToolBoxBackground::SetToolBoxChild (
    ToolBox* pChild,
    long nX,
    long nY,
    long nWidth,
    long nHeight,
    sal_uInt16 nFlags)
{
    if (pChild == NULL)
    {
        OSL_ASSERT(pChild!=NULL);
        return Point(nX, nY);
    }

    Link aEventListener (LINK(this, ToolBoxBackground, WindowEventHandler));
    pChild->AddEventListener(aEventListener);

    setPosSizePixel(
        nX - maPadding.Left(),
        nY - maPadding.Top(),
        nWidth + maPadding.Left() + maPadding.Right(),
        nHeight + maPadding.Top() + maPadding.Bottom(),
        nFlags);
    return Point(
        maPadding.Left(),
        maPadding.Top());
}




void ToolBoxBackground::Paint (const Rectangle& rRect)
{
    Window::Paint(rRect);

    Rectangle aBox (Point(0,0), GetSizePixel());

    const sidebar::Paint aTopLeftBorderPaint (Theme::GetPaint(Theme::Paint_ToolBoxBorderTopLeft));
    const sidebar::Paint aCenterBorderPaint (Theme::GetPaint(Theme::Paint_ToolBoxBorderCenterCorners));
    const sidebar::Paint aBottomRightBorderPaint (Theme::GetPaint(Theme::Paint_ToolBoxBorderBottomRight));
    const Rectangle aBorderSize (Theme::GetRectangle(Theme::Rect_ToolBoxBorder));
    DrawHelper::DrawBevelBorder (
        *this,
        aBox,
        Tools::RectangleToSvBorder(aBorderSize),
        aTopLeftBorderPaint,
        aCenterBorderPaint,
        aBottomRightBorderPaint);
}




void ToolBoxBackground::DataChanged (const DataChangedEvent& rEvent)
{
    (void)rEvent;

    SetBackground(Theme::GetPaint(Theme::Paint_ToolBoxBackground).GetWallpaper());
    maPadding = Tools::RectangleToSvBorder(Theme::GetRectangle(Theme::Rect_ToolBoxPadding));
}




IMPL_LINK(ToolBoxBackground, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        switch (pEvent->GetId())
        {
            case VCLEVENT_WINDOW_SHOW:
                if (GetChild(0)->IsVisible())
                    Show();
                break;

            case VCLEVENT_WINDOW_HIDE:
                if ( ! GetChild(0)->IsVisible())
                    Hide();
                break;

            case SFX_HINT_DYING:
                doLazyDelete();
                break;

            default:
                break;
        }
    }

    return sal_True;
}


} } // end of namespace sfx2::sidebar
