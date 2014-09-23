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

#include "AccessibleViewForwarder.hxx"
#include <svx/svdpntv.hxx>
#include <vcl/outdev.hxx>
#include <svx/sdrpaintwindow.hxx>

namespace accessibility {

/** For the time being, the implementation of this class will not use the
    member mrDevice.  Instead the device is retrieved from the view
    every time it is used.  This is necessary because the device has to stay
    up-to-date with the current view and the class has to stay compatible.
    May change in the future.
*/

AccessibleViewForwarder::AccessibleViewForwarder (SdrPaintView* pView, OutputDevice& rDevice)
    : mpView (pView),
      mnWindowId (0),
      mrDevice (rDevice)
{
    // Search the output device to determine its id.
    for(sal_uInt32 a(0L); a < mpView->PaintWindowCount(); a++)
    {
        SdrPaintWindow* pPaintWindow = mpView->GetPaintWindow(a);
        OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();

        if(&rOutDev == &rDevice)
        {
            mnWindowId = (sal_uInt16)a;
            break;
        }
    }
}

AccessibleViewForwarder::~AccessibleViewForwarder (void)
{
    // empty
}

bool AccessibleViewForwarder::IsValid (void) const
{
    return true;
}

Rectangle AccessibleViewForwarder::GetVisibleArea (void) const
{
    Rectangle aVisibleArea;

    if((sal_uInt32)mnWindowId < mpView->PaintWindowCount())
    {
        SdrPaintWindow* pPaintWindow = mpView->GetPaintWindow((sal_uInt32)mnWindowId);
        aVisibleArea = pPaintWindow->GetVisibleArea();
    }

    return aVisibleArea;
}

/** Tansform the given point into pixel coordiantes.  After the pixel
    coordiantes of the window origin are added to make the point coordinates
    absolute.
*/
Point AccessibleViewForwarder::LogicToPixel (const Point& rPoint) const
{
    OSL_ASSERT (mpView != NULL);
    if((sal_uInt32)mnWindowId < mpView->PaintWindowCount())
    {
        SdrPaintWindow* pPaintWindow = mpView->GetPaintWindow((sal_uInt32)mnWindowId);
        OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
        Rectangle aBBox(static_cast<::vcl::Window&>(rOutDev).GetWindowExtentsRelative(0L));
        return rOutDev.LogicToPixel (rPoint) + aBBox.TopLeft();
    }
    else
        return Point();
}

Size AccessibleViewForwarder::LogicToPixel (const Size& rSize) const
{
    OSL_ASSERT (mpView != NULL);
    if((sal_uInt32)mnWindowId < mpView->PaintWindowCount())
    {
        SdrPaintWindow* pPaintWindow = mpView->GetPaintWindow((sal_uInt32)mnWindowId);
        OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
        return rOutDev.LogicToPixel (rSize);
    }
    else
        return Size();
}

/** First subtract the window origin to make the point coordinates relative
    to the window and then transform them into internal coordinates.
*/
Point AccessibleViewForwarder::PixelToLogic (const Point& rPoint) const
{
    OSL_ASSERT (mpView != NULL);
    if((sal_uInt32)mnWindowId < mpView->PaintWindowCount())
    {
        SdrPaintWindow* pPaintWindow = mpView->GetPaintWindow((sal_uInt32)mnWindowId);
        OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
        Rectangle aBBox (static_cast<::vcl::Window&>(rOutDev).GetWindowExtentsRelative(0L));
        return rOutDev.PixelToLogic (rPoint - aBBox.TopLeft());
    }
    else
        return Point();
}

Size AccessibleViewForwarder::PixelToLogic (const Size& rSize) const
{
    OSL_ASSERT (mpView != NULL);
    if((sal_uInt32)mnWindowId < mpView->PaintWindowCount())
    {
        SdrPaintWindow* pPaintWindow = mpView->GetPaintWindow((sal_uInt32)mnWindowId);
        OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
        return rOutDev.PixelToLogic (rSize);
    }
    else
        return Size();
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
