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

#ifndef SD_UNO_SLIDE_VIEW_HXX
#define SD_UNO_SLIDE_VIEW_HXX

#include "DrawSubController.hxx"
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>

namespace css = ::com::sun::star;

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
