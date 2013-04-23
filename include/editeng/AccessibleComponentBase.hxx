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
    virtual OUString SAL_CALL getTitledBorderText (void)
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getToolTipText (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
        getTypes (void)
        throw (::com::sun::star::uno::RuntimeException);

};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
