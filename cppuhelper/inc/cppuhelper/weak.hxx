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
#ifndef _CPPUHELPER_WEAK_HXX_
#define _CPPUHELPER_WEAK_HXX_

#include <osl/interlck.h>
#include <rtl/alloc.h>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/uno/XWeak.hpp>


namespace cppu
{

/** @internal */
class OWeakConnectionPoint;

/** Base class to implement an UNO object supporting weak references, i.e. the object can be held
    weakly (by a ::com::sun::star::uno::WeakReference).
    This implementation copes with reference counting.  Upon last release(), the virtual dtor
    is called.

    @derive
    Inherit from this class and delegate acquire()/ release() calls.
*/
class OWeakObject : public ::com::sun::star::uno::XWeak
{
    /** @internal */
    friend class OWeakConnectionPoint;

protected:
    /** Virtual dtor.

        @attention
        Despite the fact that a RuntimeException is allowed to be thrown, you must not throw any
        exception upon destruction!
    */
    virtual ~OWeakObject() SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    /** disposes and resets m_pWeakConnectionPoint
        @precond
            m_refCount equals 0
    */
    void    disposeWeakConnectionPoint();

    /** reference count.

        @attention
        Don't modify manually!  Use acquire() and release().
    */
    oslInterlockedCount m_refCount;

    /** Container of all weak reference listeners and the connection point from the weak reference.
        @internal
    */
    OWeakConnectionPoint * m_pWeakConnectionPoint;

    /** reserved for future use. do not use.
        @internal
    */
    void * m_pReserved;

public:
    // these are here to force memory de/allocation to sal lib.
    /** @internal */
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    /** @internal */
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    /** @internal */
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    /** @internal */
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

#ifdef _MSC_VER
    /** Default Constructor.  Sets the reference count to zero.
        Accidentally occurs in msvc mapfile = > had to be outlined.
    */
    OWeakObject() SAL_THROW( () );
#else
    /** Default Constructor.  Sets the reference count to zero.
    */
    inline OWeakObject() SAL_THROW( () )
        : m_refCount( 0 )
        , m_pWeakConnectionPoint( 0 )
        {}
#endif
    /** Dummy copy constructor.  Set the reference count to zero.

        @param rObj dummy param
    */
    inline OWeakObject( const OWeakObject & ) SAL_THROW( () )
        : com::sun::star::uno::XWeak()
        , m_refCount( 0 )
        , m_pWeakConnectionPoint( 0 )
        {}
    /** Dummy assignment operator. Does not affect reference count.

        @return this OWeakObject
    */
    inline OWeakObject & SAL_CALL operator = ( const OWeakObject &)
        SAL_THROW( () )
        { return *this; }

    /** Basic queryInterface() implementation supporting ::com::sun::star::uno::XWeak and
        ::com::sun::star::uno::XInterface.

        @param rType demanded type
        @return demanded type or empty any
    */
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type & rType )
        throw (::com::sun::star::uno::RuntimeException);
    /** increasing m_refCount
    */
    virtual void SAL_CALL acquire()
        throw ();
    /** decreasing m_refCount
    */
    virtual void SAL_CALL release()
        throw ();

    /** XWeak::queryAdapter() implementation

        @return a ::com::sun::star::uno::XAdapter reference
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAdapter > SAL_CALL queryAdapter()
        throw (::com::sun::star::uno::RuntimeException);

    /** Cast operator to XInterface reference.

        @return XInterface reference
    */
    inline SAL_CALL operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > () SAL_THROW( () )
        { return this; }
};

}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
