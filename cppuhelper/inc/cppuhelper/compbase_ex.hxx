/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _CPPUHELPER_COMPBASE_EX_HXX_
#define _CPPUHELPER_COMPBASE_EX_HXX_

#include <osl/mutex.hxx>
#include <cppuhelper/implbase_ex.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/lang/XComponent.hpp>


namespace cppu
{

/** Implementation helper base class for components. Inherits from ::cppu::OWeakObject and
    ::com::sun::star::lang::XComponent.
    @internal
*/
class SAL_NO_VTABLE WeakComponentImplHelperBase
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
    WeakComponentImplHelperBase( ::osl::Mutex & rMutex ) SAL_THROW( () );
public:
    /** Destructor
    */
    virtual ~WeakComponentImplHelperBase() SAL_THROW( () );

    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire()
        throw ();
    virtual void SAL_CALL release()
        throw ();
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException);
};

/** Implementation helper base class for components. Inherits from ::cppu::OWeakAggObject and
    ::com::sun::star::lang::XComponent.
    @internal
*/
class SAL_NO_VTABLE WeakAggComponentImplHelperBase
    : public ::cppu::OWeakAggObject
    , public ::com::sun::star::lang::XComponent
{
protected:
    ::cppu::OBroadcastHelper rBHelper;

    /** Is called upon disposing the component.
    */
    virtual void SAL_CALL disposing();

    WeakAggComponentImplHelperBase( ::osl::Mutex & rMutex ) SAL_THROW( () );
public:
    virtual ~WeakAggComponentImplHelperBase() SAL_THROW( () );

    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire()
        throw ();
    virtual void SAL_CALL release()
        throw ();
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException);
};

/** WeakComponentImplHelper
    @internal
*/
::com::sun::star::uno::Any SAL_CALL WeakComponentImplHelper_query(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::WeakComponentImplHelperBase * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakComponentImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakComponentImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

/** WeakAggComponentImplHelper
    @internal
*/
::com::sun::star::uno::Any SAL_CALL WeakAggComponentImplHelper_queryAgg(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::WeakAggComponentImplHelperBase * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakAggComponentImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakAggComponentImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

}

#endif
