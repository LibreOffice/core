/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_VIEW_FORWARDER_HXX
#include "AccessibleViewForwarder.hxx"
#endif
#include <svx/svdpntv.hxx>
#include <vcl/outdev.hxx>
#include <svx/sdrpaintwindow.hxx>

namespace accessibility {

/** For the time beeing, the implementation of this class will not use the
    member mrDevice.  Instead the device is retrieved from the view
    everytime it is used.  This is necessary because the device has to stay
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




sal_Bool AccessibleViewForwarder::IsValid (void) const
{
    return sal_True;
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




/** Tansform the given point into pixel coordiantes.  After the the pixel
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
        Rectangle aBBox(static_cast<Window&>(rOutDev).GetWindowExtentsRelative(0L));
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
        Rectangle aBBox (static_cast<Window&>(rOutDev).GetWindowExtentsRelative(0L));
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
