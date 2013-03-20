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

#pragma once
#if 1

#include "DrawSubController.hxx"
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd {

/** This class implements the SlideSorter specific part of the
    controller.
 */
class SdUnoSlideView
    : private cppu::BaseMutex,
      public DrawSubControllerInterfaceBase
{
public:
    SdUnoSlideView (
        slidesorter::SlideSorter& rSlideSorter) throw();
    virtual ~SdUnoSlideView (void) throw();

    // XSelectionSupplier

    virtual sal_Bool SAL_CALL select (const ::com::sun::star::uno::Any& aSelection)
        throw(::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getSelection (void)
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addSelectionChangeListener (
        const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL removeSelectionChangeListener (
        const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
        throw(css::uno::RuntimeException);


    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage)
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL
        getCurrentPage (void)
        throw(::com::sun::star::uno::RuntimeException);


    // XFastPropertySet

    virtual void SAL_CALL setFastPropertyValue (
        sal_Int32 nHandle,
        const css::uno::Any& rValue)
        throw(css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL getFastPropertyValue (
        sal_Int32 nHandle)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

private:
    slidesorter::SlideSorter& mrSlideSorter;
    /*
    virtual void FillPropertyTable (
        ::std::vector< ::com::sun::star::beans::Property>& rProperties);
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);
    virtual void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const;
    */
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
