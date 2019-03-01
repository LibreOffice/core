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

#include <AccessiblePageShape.hxx>
#include <svx/AccessibleShapeInfo.hxx>
#include <svx/IAccessibleViewForwarder.hxx>
#include <tools/diagnose_ex.h>
#include <tools/gen.hxx>
#include <sal/log.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

//=====  internal  ============================================================

AccessiblePageShape::AccessiblePageShape (
    const uno::Reference<drawing::XDrawPage>& rxPage,
    const uno::Reference<XAccessible>& rxParent,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleShape (AccessibleShapeInfo (nullptr, rxParent), rShapeTreeInfo),
      mxPage (rxPage)
{
    // The main part of the initialization is done in the init method which
    // has to be called from this constructor's caller.
}

AccessiblePageShape::~AccessiblePageShape()
{
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL
       AccessiblePageShape::getAccessibleChildCount()
{
    return 0;
}

/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
uno::Reference<XAccessible> SAL_CALL
    AccessiblePageShape::getAccessibleChild( sal_Int32 )
{
    throw lang::IndexOutOfBoundsException ("page shape has no children",
        static_cast<uno::XWeak*>(this));
}

//=====  XAccessibleComponent  ================================================

awt::Rectangle SAL_CALL AccessiblePageShape::getBounds()
{
    ThrowIfDisposed ();

    awt::Rectangle aBoundingBox;

    if (maShapeTreeInfo.GetViewForwarder() != nullptr)
    {
        uno::Reference<beans::XPropertySet> xSet (mxPage, uno::UNO_QUERY);
        if (xSet.is())
        {
            uno::Any aValue;

            aValue = xSet->getPropertyValue ("BorderLeft");
            aValue >>= aBoundingBox.X;
            aValue = xSet->getPropertyValue ("BorderTop");
            aValue >>= aBoundingBox.Y;

            aValue = xSet->getPropertyValue ("Width");
            aValue >>= aBoundingBox.Width;
            aValue = xSet->getPropertyValue ("Height");
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
            ::tools::Rectangle aBBox (
                x, y, x + aPixelSize.getWidth(), y + aPixelSize.getHeight());
            awt::Size aParentSize (xParentComponent->getSize());
            ::tools::Rectangle aParentBBox (0,0, aParentSize.Width, aParentSize.Height);
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

sal_Int32 SAL_CALL AccessiblePageShape::getForeground()
{
    ThrowIfDisposed ();
    sal_Int32 nColor (0x0ffffffL);

    try
    {
        uno::Reference<beans::XPropertySet> aSet (mxPage, uno::UNO_QUERY);
        if (aSet.is())
        {
            uno::Any aColor;
            aColor = aSet->getPropertyValue ("LineColor");
            aColor >>= nColor;
        }
    }
    catch (const css::beans::UnknownPropertyException&)
    {
        // Ignore exception and return default color.
    }
    return nColor;
}

/** Extract the background color from the Background property of the
    draw page or its master page.
*/
sal_Int32 SAL_CALL AccessiblePageShape::getBackground()
{
    ThrowIfDisposed ();
    sal_Int32 nColor (0x01020ffL);

    try
    {
        uno::Reference<beans::XPropertySet> xSet (mxPage, uno::UNO_QUERY);
        if (xSet.is())
        {
            uno::Any aBGSet;
            aBGSet = xSet->getPropertyValue ("Background");
            Reference<beans::XPropertySet> xBGSet (aBGSet, uno::UNO_QUERY);
            if ( ! xBGSet.is())
            {
                // Draw page has no Background property.  Try the master
                // page instead.
                Reference<drawing::XMasterPageTarget> xTarget (mxPage, uno::UNO_QUERY);
                if (xTarget.is())
                {
                    xSet.set(xTarget->getMasterPage(), uno::UNO_QUERY);
                    aBGSet = xSet->getPropertyValue ("Background");
                    xBGSet.set(aBGSet, uno::UNO_QUERY);
                }
            }
            // Fetch the fill color.  Has to be extended to cope with
            // gradients, hashes, and bitmaps.
            if (xBGSet.is())
            {
                uno::Any aColor;
                aColor = xBGSet->getPropertyValue ("FillColor");
                aColor >>= nColor;
            }
            else
                SAL_WARN("sd", "no Background property in page");
        }
    }
    catch (const css::beans::UnknownPropertyException&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("sd", "caught exception due to unknown property " << exceptionToString(ex));
        // Ignore exception and return default color.
    }
    return nColor;
}

// XServiceInfo

OUString SAL_CALL
    AccessiblePageShape::getImplementationName()
{
    ThrowIfDisposed ();
    return OUString("AccessiblePageShape");
}

css::uno::Sequence< OUString> SAL_CALL
    AccessiblePageShape::getSupportedServiceNames()
{
    ThrowIfDisposed ();
    return AccessibleShape::getSupportedServiceNames();
}

//=====  lang::XEventListener  ================================================

void SAL_CALL
    AccessiblePageShape::disposing (const css::lang::EventObject& aEvent)
{
    ThrowIfDisposed ();
    AccessibleShape::disposing (aEvent);
}

//=====  XComponent  ==========================================================

void AccessiblePageShape::dispose()
{
    // Unregister listeners.
    Reference<lang::XComponent> xComponent (mxShape, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener (this);

    // Cleanup.
    mxShape = nullptr;

    // Call base classes.
    AccessibleContextBase::dispose ();
}

//=====  protected internal  ==================================================

OUString
    AccessiblePageShape::CreateAccessibleBaseName()
{
    return OUString ("PageShape");
}

OUString
    AccessiblePageShape::CreateAccessibleName()
{
    Reference<beans::XPropertySet> xPageProperties (mxPage, UNO_QUERY);

    // Get name of the current slide.
    OUString sCurrentSlideName;
    try
    {
        if (xPageProperties.is())
        {
            xPageProperties->getPropertyValue( "LinkDisplayName" ) >>= sCurrentSlideName;
        }
    }
    catch (const beans::UnknownPropertyException&)
    {
    }

    return CreateAccessibleBaseName()+": "+sCurrentSlideName;
}

OUString
    AccessiblePageShape::CreateAccessibleDescription()
{
    return OUString ("Page Shape");
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
