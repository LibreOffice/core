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


#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_EXTENDED_COMPONENT_BASE_HXX
#define _SVX_ACCESSIBILITY_ACCESSIBLE_EXTENDED_COMPONENT_BASE_HXX


#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include "editeng/editengdllapi.h"

namespace accessibility {

/** @descr
        This base class provides (will provice) a base implementation of the
        <type>XAccessibleComponent</type> and the
        <type>XAccessibleExtendedComponent</type> for all shapes.  For
        more detailed documentation about the methods refer to the interface
        descriptions of <type>XAccessible</type>,
        <type>XAccessibleContext</type>, <type>XInterface</type>,
        <type>XServiceInfo</type>, and <type>XTypeProvider</type>.
*/
class EDITENG_DLLPUBLIC AccessibleComponentBase
    :   public ::com::sun::star::accessibility::XAccessibleExtendedComponent
{
public:
    //=====  internal  ========================================================
    AccessibleComponentBase (void);
    virtual ~AccessibleComponentBase (void);


    //=====  XAccessibleComponent  ================================================

    /** The default implementation uses the result of
        <member>getBounds</member> to determine whether the given point lies
        inside this object.
    */
    virtual sal_Bool SAL_CALL containsPoint (
        const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation returns an empty reference.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint (
            const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation returns an empty rectangle.
    */
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation uses the result of
        <member>getBounds</member> to determine the location.
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation returns an empty position, i.e. the
    * result of the default constructor of <type>com::sun::star::awt::Point</type>.
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation uses the result of
        <member>getBounds</member> to determine the size.
    */
    virtual ::com::sun::star::awt::Size SAL_CALL getSize (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation ignores this call.
    */
    virtual void SAL_CALL addFocusListener (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XFocusListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation ignores this call.
    */
    virtual void SAL_CALL removeFocusListener (const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XFocusListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation does nothing.
    */
    virtual void SAL_CALL grabFocus (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns black as the default foreground color.
    */
    virtual sal_Int32 SAL_CALL getForeground (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns white as the default background color.
    */
    virtual sal_Int32 SAL_CALL getBackground (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleExtendedComponent  ====================================
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL
        getFont (void)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitledBorderText (void)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getToolTipText (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
        getTypes (void)
        throw (::com::sun::star::uno::RuntimeException);

};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
