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

#ifndef INCLUDED_VCL_OSX_DOCUMENTFOCUSLISTENER_HXX
#define INCLUDED_VCL_OSX_DOCUMENTFOCUSLISTENER_HXX

#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>

#include <cppuhelper/implbase.hxx>

#include "osx/a11yfocustracker.hxx"

#include <set>

// - DocumentFocusListener -

class DocumentFocusListener :
    public ::cppu::WeakImplHelper< ::com::sun::star::accessibility::XAccessibleEventListener >
{

public:

    explicit DocumentFocusListener(AquaA11yFocusTracker& rTracker);

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
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

private:
    std::set< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > m_aRefList;

    AquaA11yFocusTracker& m_aFocusTracker;
};

#endif // INCLUDED_VCL_OSX_DOCUMENTFOCUSLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
