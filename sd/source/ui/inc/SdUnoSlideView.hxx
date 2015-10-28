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

#ifndef INCLUDED_SD_SOURCE_UI_INC_SDUNOSLIDEVIEW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_SDUNOSLIDEVIEW_HXX

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
    virtual ~SdUnoSlideView() throw();

    // XSelectionSupplier

    virtual sal_Bool SAL_CALL select (const css::uno::Any& aSelection)
        throw(css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getSelection()
        throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addSelectionChangeListener (
        const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
        throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeSelectionChangeListener (
        const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
        throw(css::uno::RuntimeException, std::exception) override;

    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference< css::drawing::XDrawPage >& xPage)
        throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL
        getCurrentPage()
        throw(css::uno::RuntimeException, std::exception) override;

    // XFastPropertySet

    virtual void SAL_CALL setFastPropertyValue (
        sal_Int32 nHandle,
        const css::uno::Any& rValue)
        throw(css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getFastPropertyValue (
        sal_Int32 nHandle)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

private:
    slidesorter::SlideSorter& mrSlideSorter;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
