/*************************************************************************
 *
 *  $RCSfile: AccessibleViewForwarder.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2002-04-11 13:30:12 $
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


AccessibleViewForwarder::AccessibleViewForwarder (SdrPaintView* pView, USHORT nWindowId)
    : mpView (pView),
      mnWindowId (nWindowId),
      mrDevice (*pView->GetWin(nWindowId))
{
    // empty
}




AccessibleViewForwarder::AccessibleViewForwarder (SdrPaintView* pView, OutputDevice& rDevice)
    : mpView (pView),
      mnWindowId (-1),
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
}




sal_Bool AccessibleViewForwarder::IsValid (void) const
{
    return sal_True;
}




Rectangle AccessibleViewForwarder::GetVisibleArea (void) const
{
    return mpView->GetVisibleArea (mnWindowId);
}




Point AccessibleViewForwarder::LogicToPixel (const Point& rPoint) const
{
    Rectangle aBBox (static_cast<Window*>(&mrDevice)->GetWindowExtentsRelative(NULL));
    return mrDevice.LogicToPixel (rPoint)
        + aBBox.TopLeft();
}




Size AccessibleViewForwarder::LogicToPixel (const Size& rSize) const
{
    return mrDevice.LogicToPixel (rSize);
}




Point AccessibleViewForwarder::PixelToLogic (const Point& rPoint) const
{
    return mrDevice.PixelToLogic (rPoint);
}




Size AccessibleViewForwarder::PixelToLogic (const Size& rSize) const
{
    return mrDevice.PixelToLogic (rSize);
}


} // end of namespace accessibility
