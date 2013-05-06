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
#include "SidebarToolBox.hxx"
#include "ToolBoxBackground.hxx"
#include "sfx2/sidebar/Theme.hxx"
#include "sfx2/sidebar/Tools.hxx"

#include <vcl/gradient.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace sfx2 { namespace sidebar {


SidebarToolBox::SidebarToolBox (
    Window* pParentWindow,
    const ResId& rResId)
    : ToolBox(pParentWindow, rResId),
      mbParentIsBorder(false),
      maItemSeparator(Theme::GetImage(Theme::Image_ToolBoxItemSeparator))
{
    SetBackground(Wallpaper());
    SetPaintTransparent(true);
#ifdef DEBUG
    SetText(A2S("SidebarToolBox"));
#endif
}




SidebarToolBox::~SidebarToolBox (void)
{
}




void SidebarToolBox::SetBorderWindow (const Window* pBorderWindow)
{
    if (pBorderWindow != GetParent())
    {
        OSL_ASSERT("SetBorderWindow can only handle parent as border window");
        return;
    }

    if ( ! mbParentIsBorder)
    {
        mbParentIsBorder = true;

        setPosSizePixel (
            GetPosPixel().X(),
            GetPosPixel().Y(),
            GetSizePixel().Width(),
            GetSizePixel().Height(),
            WINDOW_POSSIZE_ALL);
    }
}




void SidebarToolBox::Paint (const Rectangle& rRect)
{
    ToolBox::Paint(rRect);

    if (Theme::GetBoolean(Theme::Bool_UseToolBoxItemSeparator))
    {
        const sal_Int32 nSeparatorY ((GetSizePixel().Height() - maItemSeparator.GetSizePixel().Height())/2);
        const sal_uInt16 nItemCount (GetItemCount());
        int nLastRight (-1);
        for (sal_uInt16 nIndex=0; nIndex<nItemCount; ++nIndex)
        {
            const Rectangle aItemBoundingBox (GetItemPosRect(nIndex));
            if (nLastRight >= 0)
            {
                const int nSeparatorX ((nLastRight + aItemBoundingBox.Left() - 1) / 2);
                DrawImage(Point(nSeparatorX,nSeparatorY), maItemSeparator);
            }

            nLastRight = aItemBoundingBox.Right();
        }
    }
}




Point SidebarToolBox::GetPosPixel (void) const
{
    if (mbParentIsBorder)
    {
        const Point aParentPoint (GetParent()->GetPosPixel());
        const Point aChildPoint (ToolBox::GetPosPixel());
        return Point(
            aParentPoint.X() + aChildPoint.X(),
            aParentPoint.Y() + aChildPoint.Y());
    }
    else
        return ToolBox::GetPosPixel();
}




void SidebarToolBox::setPosSizePixel (
    long nX,
    long nY,
    long nWidth,
    long nHeight,
    sal_uInt16 nFlags)
{
    if (mbParentIsBorder)
    {
        const Point aRelativePosition (static_cast<ToolBoxBackground*>(GetParent())->SetToolBoxChild(
                this,
                nX,
                nY,
                nWidth,
                nHeight,
                nFlags));
        ToolBox::setPosSizePixel(
            aRelativePosition.X(),
            aRelativePosition.Y(),
            nWidth,
            nHeight,
            nFlags);
    }
    else
        ToolBox::setPosSizePixel(nX, nY, nWidth, nHeight, nFlags);
}




long SidebarToolBox::Notify (NotifyEvent& rEvent)
{
    if (rEvent.GetType() == EVENT_KEYINPUT)
    {
        if (rEvent.GetKeyEvent()->GetKeyCode().GetCode() == KEY_TAB)
        {
            // Special handling for transferring handling of KEY_TAB
            // that becomes necessary because of our parent that is
            // not the dialog but a background control.
            return DockingWindow::Notify(rEvent);
        }
    }
    return ToolBox::Notify(rEvent);
}


} } // end of namespace sfx2::sidebar
