/*************************************************************************
 *
 *  $RCSfile: AccessibleViewForwarder.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: af $ $Date: 2002-06-28 08:44:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    Rectangle aBBox (static_cast<Window*>(pDevice)->GetWindowExtentsRelative(NULL));
    return pDevice->LogicToPixel (rPoint) + aBBox.TopLeft();
}




Size AccessibleViewForwarder::LogicToPixel (const Size& rSize) const
{
    OSL_ASSERT (mpView != NULL);
    OutputDevice* pDevice = mpView->GetWin(mnWindowId);
    return pDevice->LogicToPixel (rSize);
}




/** First subtract the window origin to make the point coordinates relative
    to the window and then transform them into internal coordinates.
*/
Point AccessibleViewForwarder::PixelToLogic (const Point& rPoint) const
{
    OSL_ASSERT (mpView != NULL);
    OutputDevice* pDevice = mpView->GetWin(mnWindowId);
    Rectangle aBBox (static_cast<Window*>(pDevice)->GetWindowExtentsRelative(NULL));
    return pDevice->PixelToLogic (rPoint - aBBox.TopLeft());
}




Size AccessibleViewForwarder::PixelToLogic (const Size& rSize) const
{
    OSL_ASSERT (mpView != NULL);
    OutputDevice* pDevice = mpView->GetWin(mnWindowId);
    return pDevice->PixelToLogic (rSize);
}


} // end of namespace accessibility
