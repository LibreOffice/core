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

#include <osl/mutex.hxx>
#include <cppuhelper/implbase_ex.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/cppuhelperdllapi.h>

/// @cond INTERNAL

namespace cppu
{

/** Implementation helper base class for components. Inherits from ::cppu::OWeakObject and
    ::com::sun::star::lang::XComponent.
*/
class CPPUHELPER_DLLPUBLIC SAL_NO_VTABLE WeakComponentImplHelperBase
    : public ::cppu::OWeakObject
    , public ::com::sun::star::lang::XComponent
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
    WeakComponentImplHelperBase( ::osl::Mutex & rMutex ) SAL_THROW(());
public:
    /** Destructor
    */
    virtual ~WeakComponentImplHelperBase() SAL_THROW(());

    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW(())
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW(())
        {}

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire()
        throw () SAL_OVERRIDE;
    virtual void SAL_CALL release()
        throw () SAL_OVERRIDE;
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

/** Implementation helper base class for components. Inherits from ::cppu::OWeakAggObject and
    ::com::sun::star::lang::XComponent.
*/
class CPPUHELPER_DLLPUBLIC SAL_NO_VTABLE WeakAggComponentImplHelperBase
    : public ::cppu::OWeakAggObject
    , public ::com::sun::star::lang::XComponent
{
protected:
    ::cppu::OBroadcastHelper rBHelper;

    /** Is called upon disposing the component.
    */
    virtual void SAL_CALL disposing();

    WeakAggComponentImplHelperBase( ::osl::Mutex & rMutex ) SAL_THROW(());
public:
    virtual ~WeakAggComponentImplHelperBase() SAL_THROW(());

    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW(())
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW(())
        {}

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire()
        throw () SAL_OVERRIDE;
    virtual void SAL_CALL release()
        throw () SAL_OVERRIDE;
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

/** WeakComponentImplHelper
*/
CPPUHELPER_DLLPUBLIC ::com::sun::star::uno::Any SAL_CALL WeakComponentImplHelper_query(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::WeakComponentImplHelperBase * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakComponentImplHelper
*/
CPPUHELPER_DLLPUBLIC ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakComponentImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

/** WeakAggComponentImplHelper
*/
CPPUHELPER_DLLPUBLIC ::com::sun::star::uno::Any SAL_CALL WeakAggComponentImplHelper_queryAgg(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::WeakAggComponentImplHelperBase * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakAggComponentImplHelper
*/
CPPUHELPER_DLLPUBLIC ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakAggComponentImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

}

/// @endcond

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
