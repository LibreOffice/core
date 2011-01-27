/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "AccessiblePageShape.hxx"
#include <svx/AccessibleShapeInfo.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

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
    AccessiblePageShape::getAccessibleChild( sal_Int32 )
    throw (::com::sun::star::uno::RuntimeException)
{
    throw lang::IndexOutOfBoundsException (
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("page shape has no children") ),
        static_cast<uno::XWeak*>(this));
}




//=====  XAccessibleComponent  ================================================

awt::Rectangle SAL_CALL AccessiblePageShape::getBounds (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();

    awt::Rectangle aBoundingBox;

    if (maShapeTreeInfo.GetViewForwarder() != NULL)
    {
        uno::Reference<beans::XPropertySet> xSet (mxPage, uno::UNO_QUERY);
        if (xSet.is())
        {
            uno::Any aValue;
            awt::Point aPosition;
            awt::Size aSize;

            aValue = xSet->getPropertyValue (
                ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("BorderLeft")));
            aValue >>= aBoundingBox.X;
            aValue = xSet->getPropertyValue (
                ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("BorderTop")));
            aValue >>= aBoundingBox.Y;

            aValue = xSet->getPropertyValue (
                ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("Width")));
            aValue >>= aBoundingBox.Width;
            aValue = xSet->getPropertyValue (
                ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("Height")));
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
    }

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
            aColor = aSet->getPropertyValue (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LineColor")));
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
                ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("Background")));
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
                        ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("Background")));
                    xBGSet = Reference<beans::XPropertySet> (aBGSet, uno::UNO_QUERY);
                }
            }
            // Fetch the fill color.  Has to be extended to cope with
            // gradients, hashes, and bitmaps.
            if (xBGSet.is())
            {
                uno::Any aColor;
                aColor = xBGSet->getPropertyValue (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FillColor")));
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
    ThrowIfDisposed ();
    return AccessibleShape::getSupportedServiceNames();
}




//=====  lang::XEventListener  ================================================

void SAL_CALL
    AccessiblePageShape::disposing (const ::com::sun::star::lang::EventObject& aEvent)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
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
    Reference<beans::XPropertySet> xPageProperties (mxPage, UNO_QUERY);

    // Get name of the current slide.
    OUString sCurrentSlideName;
    try
    {
        if (xPageProperties.is())
        {
            xPageProperties->getPropertyValue(A2S("LinkDisplayName")) >>= sCurrentSlideName;
        }
    }
    catch (beans::UnknownPropertyException&)
    {
    }

    return CreateAccessibleBaseName()+A2S(": ")+sCurrentSlideName;
}




::rtl::OUString
    AccessiblePageShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("Page Shape"));
}


} // end of namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
