/*************************************************************************
 *
 *  $RCSfile: AccessibleShapeTreeInfo.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2002-03-18 10:21:36 $
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

#include "AccessibleShapeTreeInfo.hxx"

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

namespace accessibility {

AccessibleShapeTreeInfo::AccessibleShapeTreeInfo (
    const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessibleComponent>& rxDocumentWindow,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>& rxControllerProperties,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XEventBroadcaster>& rxControllerBroadcaster)
    : mxDocumentWindow (rxDocumentWindow),
      mxControllerProperties (rxControllerProperties),
      mxControllerBroadcaster (rxControllerBroadcaster)
{
    CalcCSChangeTransformation ();
}




AccessibleShapeTreeInfo::AccessibleShapeTreeInfo (void)
{
    //empty
}




AccessibleShapeTreeInfo::~AccessibleShapeTreeInfo (void)
{
    //empty
}




void AccessibleShapeTreeInfo::SetDocumentWindow (const ::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::accessibility::XAccessibleComponent>& rxDocumentWindow)
{
    if (mxDocumentWindow != rxDocumentWindow)
    {
        mxDocumentWindow = rxDocumentWindow;
        CalcCSChangeTransformation ();
    }
}




uno::Reference<XAccessibleComponent>
    AccessibleShapeTreeInfo::GetDocumentWindow (void) const
{
    return mxDocumentWindow;
}




void AccessibleShapeTreeInfo::SetControllerProperties (
    const uno::Reference<beans::XPropertySet>& rxControllerProperties)
{
    if (mxControllerProperties != rxControllerProperties)
    {
        mxControllerProperties = rxControllerProperties;
        CalcCSChangeTransformation ();
    }
}




uno::Reference<beans::XPropertySet>
    AccessibleShapeTreeInfo::GetControllerProperties (void) const
{
    return mxControllerProperties;
}




void AccessibleShapeTreeInfo::SetControllerBroadcaster (
    const uno::Reference<document::XEventBroadcaster>& rxControllerBroadcaster)
{
    mxControllerBroadcaster = rxControllerBroadcaster;
}




uno::Reference<document::XEventBroadcaster>
    AccessibleShapeTreeInfo::GetControllerBroadcaster (void) const
{
    return mxControllerBroadcaster;
}




awt::Point AccessibleShapeTreeInfo::PointToPixel (const awt::Point& aInternalPoint) const
{
    return awt::Point (
        aInternalPoint.X * mnXScale + mnXOffset,
        aInternalPoint.Y * mnYScale + mnYOffset);
}




awt::Size AccessibleShapeTreeInfo::SizeToPixel (const awt::Size& aInternalSize) const
{
    return awt::Size (
        aInternalSize.Width * mnXScale,
        aInternalSize.Height * mnYScale);
}




void AccessibleShapeTreeInfo::CalcCSChangeTransformation (void)
{
    try
    {
        // Get pixel bbox of document view window.
        awt::Rectangle aPixelBBox = mxDocumentWindow->getBounds();

        // Get bbox in internal coordinates.
        uno::Any aVisibleArea = mxControllerProperties->getPropertyValue (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("VisibleArea")));
        awt::Rectangle aInternalBBox;
        aVisibleArea >>= aInternalBBox;

        // Transform internal coordinates into pixel.
        mnXScale = aPixelBBox.Width * 1.0 / aInternalBBox.Width;
        mnXOffset = aPixelBBox.X - aInternalBBox.X * mnXScale;
        mnYScale = aPixelBBox.Height * 1.0 / aInternalBBox.Height;
        mnYOffset = aPixelBBox.Y - aInternalBBox.Y * mnYScale;
    }
    catch (beans::UnknownPropertyException)
    {
        mnXOffset = 0;
        mnYOffset = 0;
        mnXScale = 1;
        mnYScale = 1;
    }
}


} // end of namespace accessibility
