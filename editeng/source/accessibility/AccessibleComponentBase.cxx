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

#include <editeng/AccessibleComponentBase.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <tools/color.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

// internal

AccessibleComponentBase::AccessibleComponentBase()
{
}




AccessibleComponentBase::~AccessibleComponentBase()
{
}

// XAccessibleComponent

sal_Bool SAL_CALL AccessibleComponentBase::containsPoint (
        const css::awt::Point& aPoint)
        throw (css::uno::RuntimeException, std::exception)
{
    awt::Size aSize (getSize());
    return (aPoint.X >= 0)
        && (aPoint.X < aSize.Width)
        && (aPoint.Y >= 0)
        && (aPoint.Y < aSize.Height);
}




uno::Reference<XAccessible > SAL_CALL
    AccessibleComponentBase::getAccessibleAtPoint (
        const awt::Point& /*aPoint*/)
    throw (uno::RuntimeException, std::exception)
{
    return uno::Reference<XAccessible>();
}




awt::Rectangle SAL_CALL AccessibleComponentBase::getBounds()
    throw (uno::RuntimeException, std::exception)
{
    return awt::Rectangle();
}




awt::Point SAL_CALL AccessibleComponentBase::getLocation()
    throw (css::uno::RuntimeException, std::exception)
{
    awt::Rectangle aBBox (getBounds());
    return awt::Point (aBBox.X, aBBox.Y);
}




awt::Point SAL_CALL AccessibleComponentBase::getLocationOnScreen()
    throw (css::uno::RuntimeException, std::exception)
{
    return awt::Point();
}




css::awt::Size SAL_CALL AccessibleComponentBase::getSize()
    throw (css::uno::RuntimeException, std::exception)
{
    awt::Rectangle aBBox (getBounds());
    return awt::Size (aBBox.Width, aBBox.Height);
}




void SAL_CALL AccessibleComponentBase::addFocusListener (
    const css::uno::Reference<
    css::awt::XFocusListener >& /*xListener*/)
    throw (css::uno::RuntimeException)
{
    // Ignored
}




void SAL_CALL AccessibleComponentBase::removeFocusListener (const css::uno::Reference<
    css::awt::XFocusListener >& /*xListener*/ )
    throw (css::uno::RuntimeException)
{
    // Ignored
}




void SAL_CALL AccessibleComponentBase::grabFocus()
    throw (css::uno::RuntimeException, std::exception)
{
    uno::Reference<XAccessibleContext> xContext (this, uno::UNO_QUERY);
    uno::Reference<XAccessibleSelection> xSelection (
        xContext->getAccessibleParent(), uno::UNO_QUERY);
    if (xSelection.is())
    {
        // Do a single selection on this object.
        xSelection->clearAccessibleSelection();
        xSelection->selectAccessibleChild (xContext->getAccessibleIndexInParent());
    }
}




sal_Int32 SAL_CALL AccessibleComponentBase::getForeground()
        throw (css::uno::RuntimeException, std::exception)
{
    return Color(COL_BLACK).GetColor();
}




sal_Int32 SAL_CALL AccessibleComponentBase::getBackground()
    throw (css::uno::RuntimeException, std::exception)
{
    return Color(COL_WHITE).GetColor();
}




// XAccessibleExtendedComponent

css::uno::Reference< css::awt::XFont > SAL_CALL
        AccessibleComponentBase::getFont()
        throw (css::uno::RuntimeException, std::exception)
{
    return uno::Reference<awt::XFont>();
}




OUString SAL_CALL AccessibleComponentBase::getTitledBorderText()
        throw (css::uno::RuntimeException, std::exception)
{
    return OUString();
}


OUString SAL_CALL AccessibleComponentBase::getToolTipText()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString();
}

// XTypeProvider

uno::Sequence<uno::Type> SAL_CALL
    AccessibleComponentBase::getTypes()
    throw (uno::RuntimeException, std::exception)
{
    // Get list of types from the context base implementation...
    uno::Sequence<uno::Type> aTypeList (2);
    // ...and add the additional type for the component.
    const uno::Type aComponentType =
         cppu::UnoType<XAccessibleComponent>::get();
    const uno::Type aExtendedComponentType =
        cppu::UnoType<XAccessibleExtendedComponent>::get();
    aTypeList[0] = aComponentType;
    aTypeList[1] = aExtendedComponentType;

    return aTypeList;
}


} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
