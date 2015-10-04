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
#ifndef INCLUDED_CPPUHELPER_COMPONENT_HXX
#define INCLUDED_CPPUHELPER_COMPONENT_HXX

#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/cppuhelperdllapi.h>

namespace osl { class Mutex; }

namespace cppu
{

/** Deprecated.  Helper for implementing css::lang::XComponent.
    Upon disposing objects of this class, sub-classes receive a disposing() call.  Objects of
    this class can be held weakly, i.e. by a css::uno::WeakReference.  Object of
    this class can be aggregated, i.e. incoming queryInterface() calls are delegated.

    @attention
    The life-cycle of the passed mutex reference has to be longer than objects of this class.
    @deprecated
*/
class CPPUHELPER_DLLPUBLIC OComponentHelper
    : public ::cppu::OWeakAggObject
    , public css::lang::XTypeProvider
    , public css::lang::XComponent
{
public:
    /** Constructor.

        @param rMutex
        the mutex used to protect multi-threaded access;
        lifetime must be longer than the lifetime of this object.
    */
    OComponentHelper( ::osl::Mutex & rMutex );
    /** Destructor. If this object was not disposed previously, object will be disposed manually.
    */
    virtual ~OComponentHelper() SAL_OVERRIDE;

    // XAggregation
    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL queryAggregation(
        css::uno::Type const & rType )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire()
        throw () SAL_OVERRIDE;
    virtual void SAL_CALL release()
        throw () SAL_OVERRIDE;

    /** @attention
        XTypeProvider::getImplementationId() has to be implemented separately!
    */
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE = 0;
    /** @attention
        XTypeProvider::getTypes() has to be re-implemented!
    */
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose()
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener(
        const css::uno::Reference< css::lang::XEventListener >& aListener )
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener(
        const css::uno::Reference< css::lang::XEventListener >& aListener )
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    /** Called in dispose method after the listeners were notified.
    */
    virtual void SAL_CALL disposing();

    /// @cond INTERNAL
    OBroadcastHelper    rBHelper;
    /// @endcond

private:
    OComponentHelper( const OComponentHelper & ) SAL_DELETED_FUNCTION;
    OComponentHelper & operator = ( const OComponentHelper & )
        SAL_DELETED_FUNCTION;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
