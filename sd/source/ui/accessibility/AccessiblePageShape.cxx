/*************************************************************************
 *
 *  $RCSfile: AccessiblePageShape.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:03:34 $
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

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_PAGE_SHAPE_HXX
#include "AccessiblePageShape.hxx"
#endif

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX
#include <svx/AccessibleShapeInfo.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLE_ROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLE_STATE_TYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEDESCRIPTOR_HPP_
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGETARGET_HPP_
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

//=====  internal  ============================================================

AccessiblePageShape::AccessiblePageShape (
    const uno::Reference<drawing::XDrawPage>& rxPage,
    const uno::Reference<XAccessible>& rxParent,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    long nIndex)
    : AccessibleShape (AccessibleShapeInfo (NULL, rxParent, nIndex), rShapeTreeInfo),
      mxPage (rxPage)
{
    // The main part of the initialization is done in the init method which
    // has to be called from this constructor's caller.
}




AccessiblePageShape::~AccessiblePageShape (void)
{
    OSL_TRACE ("~AccessiblePageShape");
}




void AccessiblePageShape::Init (void)
{
    AccessibleShape::Init ();
}




//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL
       AccessiblePageShape::getAccessibleChildCount (void)
    throw ()
{
    return 0;
}




/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
uno::Reference<XAccessible> SAL_CALL
    AccessiblePageShape::getAccessibleChild (long nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    throw lang::IndexOutOfBoundsException (
        ::rtl::OUString::createFromAscii ("page shape has no children"),
        static_cast<uno::XWeak*>(this));
}




//=====  XAccessibleComponent  ================================================

awt::Rectangle SAL_CALL AccessiblePageShape::getBounds (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    awt::Rectangle aBoundingBox;

    uno::Reference<beans::XPropertySet> xSet (mxPage, uno::UNO_QUERY);
    if (xSet.is())
    {
        uno::Any aValue;
        awt::Point aPosition;
        awt::Size aSize;

        aValue = xSet->getPropertyValue (
            OUString (RTL_CONSTASCII_USTRINGPARAM("BorderLeft")));
        aValue >>= aBoundingBox.X;
        aValue = xSet->getPropertyValue (
            OUString (RTL_CONSTASCII_USTRINGPARAM("BorderTop")));
        aValue >>= aBoundingBox.Y;

        aValue = xSet->getPropertyValue (
            OUString (RTL_CONSTASCII_USTRINGPARAM("Width")));
        aValue >>= aBoundingBox.Width;
        aValue = xSet->getPropertyValue (
            OUString (RTL_CONSTASCII_USTRINGPARAM("Height")));
        aValue >>= aBoundingBox.Height;
    }

    // Transform coordinates from internal to pixel.
    ::Size aPixelSize = maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
        ::Size (aBoundingBox.Width, aBoundingBox.Height));
    ::Point aPixelPosition = maShapeTreeInfo.GetViewForwarder()->LogicToPixel (
        ::Point (aBoundingBox.X, aBoundingBox.Y));

    // Clip the shape's bounding box with the bounding box of its parent.
    Reference<XAccessibleComponent> xParentComponent (
        getAccessibleParent(), uno::UNO_QUERY);
    if (xParentComponent.is())
    {
        // Make the coordinates relative to the parent.
        awt::Point aParentLocation (xParentComponent->getLocationOnScreen());
        int x = aPixelPosition.getX() - aParentLocation.X;
        int y = aPixelPosition.getY() - aParentLocation.Y;


        // Clip with parent (with coordinates relative to itself).
        ::Rectangle aBBox (
            x, y, x + aPixelSize.getWidth(), y + aPixelSize.getHeight());
        awt::Size aParentSize (xParentComponent->getSize());
        ::Rectangle aParentBBox (0,0, aParentSize.Width, aParentSize.Height);
        aBBox = aBBox.GetIntersection (aParentBBox);
        aBoundingBox = awt::Rectangle (
            aBBox.getX(),
            aBBox.getY(),
            aBBox.getWidth(),
            aBBox.getHeight());
    }
    else
        aBoundingBox = awt::Rectangle (
            aPixelPosition.getX(), aPixelPosition.getY(),
            aPixelSize.getWidth(), aPixelSize.getHeight());

    return aBoundingBox;
}




sal_Int32 SAL_CALL AccessiblePageShape::getForeground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    sal_Int32 nColor (0x0ffffffL);

    try
    {
        uno::Reference<beans::XPropertySet> aSet (mxPage, uno::UNO_QUERY);
        if (aSet.is())
        {
            uno::Any aColor;
            aColor = aSet->getPropertyValue (OUString::createFromAscii ("LineColor"));
            aColor >>= nColor;
        }
    }
    catch (::com::sun::star::beans::UnknownPropertyException)
    {
        // Ignore exception and return default color.
    }
    return nColor;
}




/** Extract the background color from the Background property of eithe the
    draw page or its master page.
*/
sal_Int32 SAL_CALL AccessiblePageShape::getBackground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    sal_Int32 nColor (0x01020ffL);

    try
    {
        uno::Reference<beans::XPropertySet> xSet (mxPage, uno::UNO_QUERY);
        if (xSet.is())
        {
            uno::Any aBGSet;
            aBGSet = xSet->getPropertyValue (
                OUString (RTL_CONSTASCII_USTRINGPARAM("Background")));
            Reference<beans::XPropertySet> xBGSet (aBGSet, uno::UNO_QUERY);
            if ( ! xBGSet.is())
            {
                // Draw page has no Background property.  Try the master
                // page instead.
                Reference<drawing::XMasterPageTarget> xTarget (mxPage, uno::UNO_QUERY);
                if (xTarget.is())
                {
                    xSet = Reference<beans::XPropertySet> (xTarget->getMasterPage(),
                        uno::UNO_QUERY);
                    aBGSet = xSet->getPropertyValue (
                        OUString (RTL_CONSTASCII_USTRINGPARAM("Background")));
                    xBGSet = Reference<beans::XPropertySet> (aBGSet, uno::UNO_QUERY);
                }
            }
            // Fetch the fill color.  Has to be extended to cope with
            // gradients, hashes, and bitmaps.
            if (xBGSet.is())
            {
                uno::Any aColor;
                aColor = xBGSet->getPropertyValue (OUString::createFromAscii ("FillColor"));
                aColor >>= nColor;
            }
            else
                OSL_TRACE ("no Background property in page");
        }
    }
    catch (::com::sun::star::beans::UnknownPropertyException)
    {
        OSL_TRACE ("caught excption due to unknown property");
        // Ignore exception and return default color.
    }
    return nColor;
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessiblePageShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessiblePageShape"));
}




::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
    AccessiblePageShape::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return AccessibleShape::getSupportedServiceNames();
}




//=====  lang::XEventListener  ================================================

void SAL_CALL
    AccessiblePageShape::disposing (const ::com::sun::star::lang::EventObject& aEvent)
    throw (::com::sun::star::uno::RuntimeException)
{
    AccessibleShape::disposing (aEvent);
}




//=====  XComponent  ==========================================================

void AccessiblePageShape::dispose (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    OSL_TRACE ("AccessiblePageShape::dispose");

    // Unregister listeners.
    Reference<lang::XComponent> xComponent (mxShape, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener (this);

    // Cleanup.
    mxShape = NULL;

    // Call base classes.
    AccessibleContextBase::dispose ();
}




//=====  protected internal  ==================================================

::rtl::OUString
    AccessiblePageShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("PageShape"));
}




::rtl::OUString
    AccessiblePageShape::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return CreateAccessibleBaseName();
}




::rtl::OUString
    AccessiblePageShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("Page Shape"));
}


} // end of namespace accessibility
