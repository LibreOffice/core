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
#ifndef INCLUDED_CPPUHELPER_COMPBASE_EX_HXX
#define INCLUDED_CPPUHELPER_COMPBASE_EX_HXX

#include "cppuhelper/interfacecontainer.h"
#include "com/sun/star/lang/XComponent.hpp"
#include "cppuhelper/cppuhelperdllapi.h"
#include "cppuhelper/weak.hxx"
#include "cppuhelper/weakagg.hxx"

/// @cond INTERNAL

namespace osl { class Mutex; }
namespace cppu { struct class_data; }

namespace cppu
{

/** Implementation helper base class for components. Inherits from ::cppu::OWeakObject and
    css::lang::XComponent.
*/
class CPPUHELPER_DLLPUBLIC SAL_NO_VTABLE WeakComponentImplHelperBase
    : public ::cppu::OWeakObject
    , public css::lang::XComponent
{
protected:
    /** broadcast helper for disposing events
    */
    ::cppu::OBroadcastHelper rBHelper;

    /** this function is called upon disposing the component
    */
    virtual void SAL_CALL disposing();

    /** This is the one and only constructor that is called from derived implementations.

        @param rMutex mutex to sync upon disposing
    */
    WeakComponentImplHelperBase( ::osl::Mutex & rMutex );
public:
    /** Destructor
    */
    virtual ~WeakComponentImplHelperBase() SAL_OVERRIDE;

    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * SAL_CALL operator new( size_t, void * pMem )
        { return pMem; }
    static void SAL_CALL operator delete( void *, void * )
        {}

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType ) SAL_OVERRIDE;
    virtual void SAL_CALL acquire()
        throw () SAL_OVERRIDE;
    virtual void SAL_CALL release()
        throw () SAL_OVERRIDE;
    virtual void SAL_CALL dispose() SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener(
        css::uno::Reference< css::lang::XEventListener > const & xListener ) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener(
        css::uno::Reference< css::lang::XEventListener > const & xListener ) SAL_OVERRIDE;
};

/** Implementation helper base class for components. Inherits from ::cppu::OWeakAggObject and
    css::lang::XComponent.
*/
class CPPUHELPER_DLLPUBLIC SAL_NO_VTABLE WeakAggComponentImplHelperBase
    : public ::cppu::OWeakAggObject
    , public css::lang::XComponent
{
protected:
    ::cppu::OBroadcastHelper rBHelper;

    /** Is called upon disposing the component.
    */
    virtual void SAL_CALL disposing();

    WeakAggComponentImplHelperBase( ::osl::Mutex & rMutex );
public:
    virtual ~WeakAggComponentImplHelperBase() SAL_OVERRIDE;

    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * SAL_CALL operator new( size_t, void * pMem )
        { return pMem; }
    static void SAL_CALL operator delete( void *, void * )
        {}

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType ) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL queryAggregation(
        css::uno::Type const & rType ) SAL_OVERRIDE;
    virtual void SAL_CALL acquire()
        throw () SAL_OVERRIDE;
    virtual void SAL_CALL release()
        throw () SAL_OVERRIDE;
    virtual void SAL_CALL dispose() SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener(
        css::uno::Reference< css::lang::XEventListener > const & xListener ) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener(
        css::uno::Reference< css::lang::XEventListener > const & xListener ) SAL_OVERRIDE;
};

/** WeakComponentImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Any SAL_CALL WeakComponentImplHelper_query(
    css::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::WeakComponentImplHelperBase * pBase );
/** WeakComponentImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Sequence< css::uno::Type > SAL_CALL WeakComponentImplHelper_getTypes(
    class_data * cd );

/** WeakAggComponentImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Any SAL_CALL WeakAggComponentImplHelper_queryAgg(
    css::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::WeakAggComponentImplHelperBase * pBase );
/** WeakAggComponentImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Sequence< css::uno::Type > SAL_CALL WeakAggComponentImplHelper_getTypes(
    class_data * cd );

}

/// @endcond

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
