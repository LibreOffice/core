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

#ifndef _DOCUMENTFOCUSLISTENER_HXX_
#define _DOCUMENTFOCUSLISTENER_HXX_

#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>

#include <cppuhelper/implbase1.hxx>

#include "aqua/aqua11yfocustracker.hxx"

#include <set>

// -------------------------
// - DocumentFocusListener -
// -------------------------

class DocumentFocusListener :
    public ::cppu::WeakImplHelper1< ::com::sun::star::accessibility::XAccessibleEventListener >
{

public:

    DocumentFocusListener(AquaA11yFocusTracker& rTracker);

    void attachRecursive(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& xAccessible
    ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    void attachRecursive(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& xAccessible,
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& xContext
    ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    void attachRecursive(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& xAccessible,
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& xContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet >& xStateSet
    ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    void detachRecursive(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& xAccessible
    ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    void detachRecursive(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& xAccessible,
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& xContext
    ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    void detachRecursive(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& xAccessible,
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& xContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet >& xStateSet
    ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > getAccessible(const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException );

private:
    std::set< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > m_aRefList;

    AquaA11yFocusTracker& m_aFocusTracker;
};

#endif // _DOCUMENTFOCUSLISTENER_HXX_