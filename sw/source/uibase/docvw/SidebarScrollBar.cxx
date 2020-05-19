/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SidebarScrollBar.hxx"

#include <sfx2/lokhelper.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include <AnnotationWin.hxx>

namespace sw::sidebarwindows
{
SidebarScrollBar::SidebarScrollBar(sw::annotation::SwAnnotationWin& rSidebarWin, WinBits nStyle,
                                   SwView& rView)
    : ScrollBar(&rSidebarWin, nStyle)
    , m_rSidebarWin(rSidebarWin)
    , m_rView(rView)
{
}

void SidebarScrollBar::LogicInvalidate(const tools::Rectangle* pRectangle)
{
    tools::Rectangle aRectangle;

    if (!pRectangle)
    {
        Push(PushFlags::MAPMODE);
        EnableMapMode();
        MapMode aMapMode = GetMapMode();
        aMapMode.SetMapUnit(MapUnit::MapTwip);
        SetMapMode(aMapMode);
        aRectangle = tools::Rectangle(Point(0, 0), PixelToLogic(GetSizePixel()));
        Pop();
    }
    else
        aRectangle = *pRectangle;

    // Convert from relative twips to absolute ones.
    vcl::Window& rParent = m_rSidebarWin.EditWin();
    Point aOffset(GetOutOffXPixel() - rParent.GetOutOffXPixel(),
                  GetOutOffYPixel() - rParent.GetOutOffYPixel());
    rParent.Push(PushFlags::MAPMODE);
    rParent.EnableMapMode();
    aOffset = rParent.PixelToLogic(aOffset);
    rParent.Pop();
    aRectangle.Move(aOffset.getX(), aOffset.getY());

    OString sRectangle = aRectangle.toString();
    SwWrtShell& rWrtShell = m_rView.GetWrtShell();
    SfxLokHelper::notifyInvalidation(rWrtShell.GetSfxViewShell(), sRectangle);
}

void SidebarScrollBar::MouseButtonUp(const MouseEvent& /*rMouseEvent*/) { EndTracking(); }

void SidebarScrollBar::MouseMove(const MouseEvent& rMouseEvent)
{
    TrackingEvent aEvent(rMouseEvent);
    Tracking(aEvent);
}

SidebarScrollBar::~SidebarScrollBar() { disposeOnce(); }

} // end of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
