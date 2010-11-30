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
#include "precompiled_editeng.hxx"


#include <editeng/AccessibleComponentBase.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <tools/color.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessibleComponentBase::AccessibleComponentBase (void)
{
}




AccessibleComponentBase::~AccessibleComponentBase (void)
{
}




//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL AccessibleComponentBase::containsPoint (
        const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    return uno::Reference<XAccessible>();
}




awt::Rectangle SAL_CALL AccessibleComponentBase::getBounds (void)
    throw (uno::RuntimeException)
{
    return awt::Rectangle();
}




awt::Point SAL_CALL AccessibleComponentBase::getLocation (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Rectangle aBBox (getBounds());
    return awt::Point (aBBox.X, aBBox.Y);
}




awt::Point SAL_CALL AccessibleComponentBase::getLocationOnScreen (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return awt::Point();
}




::com::sun::star::awt::Size SAL_CALL AccessibleComponentBase::getSize (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Rectangle aBBox (getBounds());
    return awt::Size (aBBox.Width, aBBox.Height);
}




void SAL_CALL AccessibleComponentBase::addFocusListener (
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XFocusListener >& /*xListener*/)
    throw (::com::sun::star::uno::RuntimeException)
{
    // Ignored
}




void SAL_CALL AccessibleComponentBase::removeFocusListener (const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XFocusListener >& /*xListener*/ )
    throw (::com::sun::star::uno::RuntimeException)
{
    // Ignored
}




void SAL_CALL AccessibleComponentBase::grabFocus (void)
    throw (::com::sun::star::uno::RuntimeException)
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




sal_Int32 SAL_CALL AccessibleComponentBase::getForeground (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return Color(COL_BLACK).GetColor();
}




sal_Int32 SAL_CALL AccessibleComponentBase::getBackground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return Color(COL_WHITE).GetColor();
}




//=====  XAccessibleExtendedComponent  ========================================

::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL
        AccessibleComponentBase::getFont (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return uno::Reference<awt::XFont>();
}




::rtl::OUString SAL_CALL AccessibleComponentBase::getTitledBorderText (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString ();
}




::rtl::OUString SAL_CALL AccessibleComponentBase::getToolTipText (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString ();
}




//=====  XTypeProvider  ===================================================

uno::Sequence<uno::Type> SAL_CALL
    AccessibleComponentBase::getTypes (void)
    throw (uno::RuntimeException)
{
    // Get list of types from the context base implementation...
    uno::Sequence<uno::Type> aTypeList (2);
    // ...and add the additional type for the component.
    const uno::Type aComponentType =
         ::getCppuType((const uno::Reference<XAccessibleComponent>*)0);
    const uno::Type aExtendedComponentType =
        ::getCppuType((const uno::Reference<XAccessibleExtendedComponent>*)0);
    aTypeList[0] = aComponentType;
    aTypeList[1] = aExtendedComponentType;

    return aTypeList;
}


} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
