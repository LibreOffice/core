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

#include <cassert>
#include <cstddef>
#include "osl/interlck.h"
#include "rtl/alloc.h"
#include "com/sun/star/uno/XWeak.hpp"
#include "cppuhelper/cppuhelperdllapi.h"


namespace cppu
{

class OWeakConnectionPoint;

/** Base class to implement a UNO object supporting weak references, i.e. the object can be held
    weakly (by a css::uno::WeakReference).
    This implementation copes with reference counting.  Upon last release(), the virtual dtor
    is called.

    @derive
    Inherit from this class and delegate acquire()/ release() calls.
*/
class CPPUHELPER_DLLPUBLIC OWeakObject : public css::uno::XWeak
{
    friend class OWeakConnectionPoint;

protected:
    /** Virtual dtor.

        @attention
        Despite the fact that a RuntimeException is allowed to be thrown, you must not throw any
        exception upon destruction!
    */
    virtual ~OWeakObject() COVERITY_NOEXCEPT_FALSE;

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
    static void * SAL_CALL operator new( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * SAL_CALL operator new( size_t, void * pMem )
        { return pMem; }
    static void SAL_CALL operator delete( void *, void * )
        {}
    /// @endcond

#ifdef _MSC_VER
    /** Default Constructor.  Sets the reference count to zero.
        Accidentally occurs in msvc mapfile = > had to be outlined.
    */
    OWeakObject();
#else
    /** Default Constructor.  Sets the reference count to zero.
    */
    OWeakObject()
        : m_refCount( 0 )
        , m_pWeakConnectionPoint( NULL )
        , m_pReserved(NULL)
        {}
#endif
    /** Dummy copy constructor.  Set the reference count to zero.
    */
    OWeakObject( const OWeakObject & )
        : css::uno::XWeak()
        , m_refCount( 0 )
        , m_pWeakConnectionPoint( NULL )
        , m_pReserved(NULL)
        {}
    /** Dummy assignment operator. Does not affect reference count.

        @return this OWeakObject
    */
    OWeakObject & SAL_CALL operator = ( const OWeakObject &)
        { return *this; }

    /** Basic queryInterface() implementation supporting com::sun::star::uno::XWeak and
        com::sun::star::uno::XInterface.

        @param rType demanded type
        @return demanded type or empty any
    */
    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type & rType ) SAL_OVERRIDE;
    /** increasing m_refCount
    */
    virtual void SAL_CALL acquire()
        throw () SAL_OVERRIDE;
    /** decreasing m_refCount
    */
    virtual void SAL_CALL release()
        throw () SAL_OVERRIDE;

    /** XWeak::queryAdapter() implementation

        @return a com::sun::star::uno::XAdapter reference
    */
    virtual css::uno::Reference< css::uno::XAdapter > SAL_CALL queryAdapter() SAL_OVERRIDE;

    /** Cast operator to XInterface reference.

        @return XInterface reference
    */
    SAL_CALL operator css::uno::Reference< css::uno::XInterface > ()
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
static inline css::uno::XInterface * acquire(OWeakObject * instance)
{
    assert(instance != NULL);
    instance->acquire();
    return instance;
}
/// @endcond

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
