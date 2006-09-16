/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleViewForwarder.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:27:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_VIEW_FORWARDER_HXX
#include "AccessibleViewForwarder.hxx"
#endif

#ifndef _SVDPNTV_HXX
#include <svx/svdpntv.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

namespace accessibility {

/** For the time beeing, the implementation of this class will not use the
    member mrDevice.  Instead the device is retrieved from the view
    everytime it is used.  This is necessary because the device has to stay
    up-to-date with the current view and the class has to stay compatible.
    May change in the future.
*/

AccessibleViewForwarder::AccessibleViewForwarder (SdrPaintView* pView, USHORT nWindowId)
    : mpView (pView),
      mnWindowId (nWindowId),
      mrDevice (*pView->GetWin(nWindowId))
{
    OSL_ASSERT (mpView != NULL);
    // empty
}




AccessibleViewForwarder::AccessibleViewForwarder (SdrPaintView* pView, OutputDevice& rDevice)
    : mpView (pView),
      mnWindowId (0),
      mrDevice (rDevice)
{
    // Search the output device to determine its id.
    for (USHORT i=0; i<mpView->GetWinCount(); i++)
        if (mpView->GetWin(i) == &rDevice)
        {
            mnWindowId = i;
            break;
        }
}




AccessibleViewForwarder::~AccessibleViewForwarder (void)
{
    // empty
}




void AccessibleViewForwarder::SetView (SdrPaintView* pView)
{
    mpView = pView;
    OSL_ASSERT (mpView != NULL);
}




sal_Bool AccessibleViewForwarder::IsValid (void) const
{
    return sal_True;
}




Rectangle AccessibleViewForwarder::GetVisibleArea (void) const
{
    return mpView->GetVisibleArea (mnWindowId);
}




/** Tansform the given point into pixel coordiantes.  After the the pixel
    coordiantes of the window origin are added to make the point coordinates
    absolute.
*/
Point AccessibleViewForwarder::LogicToPixel (const Point& rPoint) const
{
    OSL_ASSERT (mpView != NULL);
    OutputDevice* pDevice = mpView->GetWin(mnWindowId);
    if (pDevice != NULL)
    {
        Rectangle aBBox (static_cast<Window*>(pDevice)->GetWindowExtentsRelative(NULL));
        return pDevice->LogicToPixel (rPoint) + aBBox.TopLeft();
    }
    else
        return Point();
}




Size AccessibleViewForwarder::LogicToPixel (const Size& rSize) const
{
    OSL_ASSERT (mpView != NULL);
    OutputDevice* pDevice = mpView->GetWin(mnWindowId);
    if (pDevice != NULL)
        return pDevice->LogicToPixel (rSize);
    else
        return Size();
}




/** First subtract the window origin to make the point coordinates relative
    to the window and then transform them into internal coordinates.
*/
Point AccessibleViewForwarder::PixelToLogic (const Point& rPoint) const
{
    OSL_ASSERT (mpView != NULL);
    OutputDevice* pDevice = mpView->GetWin(mnWindowId);
    if (pDevice != NULL)
    {
        Rectangle aBBox (static_cast<Window*>(pDevice)->GetWindowExtentsRelative(NULL));
        return pDevice->PixelToLogic (rPoint - aBBox.TopLeft());
    }
    else
        return Point();
}




Size AccessibleViewForwarder::PixelToLogic (const Size& rSize) const
{
    OSL_ASSERT (mpView != NULL);
    OutputDevice* pDevice = mpView->GetWin(mnWindowId);
    if (pDevice != NULL)
        return pDevice->PixelToLogic (rSize);
    else
        return Size();
}


} // end of namespace accessibility
