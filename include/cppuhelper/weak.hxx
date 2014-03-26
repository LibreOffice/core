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
#ifndef INCLUDED_CPPUHELPER_WEAK_HXX
#define INCLUDED_CPPUHELPER_WEAK_HXX

#include <osl/interlck.h>
#include <rtl/alloc.h>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/uno/XWeak.hpp>
#include <cppuhelper/cppuhelperdllapi.h>


namespace cppu
{

class OWeakConnectionPoint;

/** Base class to implement an UNO object supporting weak references, i.e. the object can be held
    weakly (by a ::com::sun::star::uno::WeakReference).
    This implementation copes with reference counting.  Upon last release(), the virtual dtor
    is called.

    @derive
    Inherit from this class and delegate acquire()/ release() calls.
*/
class CPPUHELPER_DLLPUBLIC OWeakObject : public ::com::sun::star::uno::XWeak
{
    friend class OWeakConnectionPoint;

protected:
    /** Virtual dtor.

        @attention
        Despite the fact that a RuntimeException is allowed to be thrown, you must not throw any
        exception upon destruction!
    */
    virtual ~OWeakObject() SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    /** disposes and resets m_pWeakConnectionPoint
        @pre
            m_refCount equals 0
    */
    void    disposeWeakConnectionPoint();

    /** reference count.

        @attention
        Don't modify manually!  Use acquire() and release().
    */
    oslInterlockedCount m_refCount;

    /// @cond INTERNAL

    /** Container of all weak reference listeners and the connection point from the weak reference.
    */
    OWeakConnectionPoint * m_pWeakConnectionPoint;

    /** reserved for future use. do not use.
    */
    void * m_pReserved;

    /// @endcond

public:
    /// @cond INTERNAL
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW(())
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW(())
        {}
    /// @endcond

#ifdef _MSC_VER
    /** Default Constructor.  Sets the reference count to zero.
        Accidentally occurs in msvc mapfile = > had to be outlined.
    */
    OWeakObject() SAL_THROW(());
#else
    /** Default Constructor.  Sets the reference count to zero.
    */
    inline OWeakObject() SAL_THROW(())
        : m_refCount( 0 )
        , m_pWeakConnectionPoint( 0 )
        {}
#endif
    /** Dummy copy constructor.  Set the reference count to zero.

        @param rObj dummy param
    */
    inline OWeakObject( const OWeakObject & rObj ) SAL_THROW(())
        : com::sun::star::uno::XWeak()
        , m_refCount( 0 )
        , m_pWeakConnectionPoint( 0 )
        , m_pReserved(0)
        {
            (void) rObj;
        }
    /** Dummy assignment operator. Does not affect reference count.

        @return this OWeakObject
    */
    inline OWeakObject & SAL_CALL operator = ( const OWeakObject &)
        SAL_THROW(())
        { return *this; }

    /** Basic queryInterface() implementation supporting \::com::sun::star::uno::XWeak and
        \::com::sun::star::uno::XInterface.

        @param rType demanded type
        @return demanded type or empty any
    */
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type & rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    /** increasing m_refCount
    */
    virtual void SAL_CALL acquire()
        throw () SAL_OVERRIDE;
    /** decreasing m_refCount
    */
    virtual void SAL_CALL release()
        throw () SAL_OVERRIDE;

    /** XWeak::queryAdapter() implementation

        @return a \::com::sun::star::uno::XAdapter reference
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAdapter > SAL_CALL queryAdapter()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /** Cast operator to XInterface reference.

        @return XInterface reference
    */
    inline SAL_CALL operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > () SAL_THROW(())
        { return this; }
};

/// @cond INTERNAL
/** Convenience function for constructor-based service implementations.

    To be used like:

    css::uno::XInterface * FOO_constructor_function(...) {
        return cppu::acquire(new FOO(...));
    }

    @param instance
    Newly created instance that should be acquired.
*/
static inline ::com::sun::star::uno::XInterface * acquire(OWeakObject * instance)
{
    assert(instance != 0);
    instance->acquire();
    return instance;
}
/// @endcond

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
