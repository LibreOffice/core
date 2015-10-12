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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_ACCESSIBLETEXTHELPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_ACCESSIBLETEXTHELPER_HXX

#include "MutexContainer.hxx"
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

// forward declaration of helper class from svx
namespace accessibility
{
class AccessibleTextHelper;
}

namespace chart
{

class DrawViewWrapper;

namespace impl
{
typedef ::cppu::WeakComponentImplHelper<
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::accessibility::XAccessibleContext >
    AccessibleTextHelper_Base;
}

class AccessibleTextHelper :
        public MutexContainer,
        public impl::AccessibleTextHelper_Base
{
public:
    explicit AccessibleTextHelper( DrawViewWrapper * pDrawViewWrapper );
    virtual ~AccessibleTextHelper();

    // ____ XInitialization ____
    /** Must be called at least once for this helper class to work.

        mandatory parameter 0: type string. This is the CID that is used to find
            the corresponding drawing object that contains the text that should
            be handled by this helper class.
1
        mandatory parameter 1: type XAccessible.  Is used as EventSource for the
            ::accessibility::AccessibleTextHelper (svx)

        mandatory parameter 2: type awt::XWindow.  The Window that shows the
            text currently.
     */
    virtual void SAL_CALL initialize(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw (::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XAccessibleContext ____
    virtual ::sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild(
        ::sal_Int32 i )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL getAccessibleRole()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale()
        throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    ::accessibility::AccessibleTextHelper * m_pTextHelper;
    DrawViewWrapper *                       m_pDrawViewWrapper;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_INC_ACCESSIBLETEXTHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
