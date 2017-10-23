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
#ifndef INCLUDED_CPPUHELPER_WEAKREF_HXX
#define INCLUDED_CPPUHELPER_WEAKREF_HXX

#include "sal/config.h"

#include <cstddef>

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/cppuhelperdllapi.h"


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

class OWeakRefListener;

/** The WeakReferenceHelper holds a weak reference to an object.

    That object must implement the css::uno::XWeak interface.

    The WeakReferenceHelper itself is *not* thread safe, just as
    Reference itself isn't, but the implementation of the listeners etc.
    behind it *is* thread-safe, so multiple threads can have their own
    WeakReferences to the same XWeak object.
*/
class CPPUHELPER_DLLPUBLIC WeakReferenceHelper
{
public:
    /** Default ctor.  Creates an empty weak reference.
    */
    WeakReferenceHelper()
        : m_pImpl( NULL )
        {}

    /** Copy ctor.  Initialize this reference with the same interface as in rWeakRef.

        @param rWeakRef another weak ref
    */
    WeakReferenceHelper( const WeakReferenceHelper & rWeakRef );

#if defined LIBO_INTERNAL_ONLY
    WeakReferenceHelper(WeakReferenceHelper && other): m_pImpl(other.m_pImpl)
    { other.m_pImpl = nullptr; }
#endif

    /** Initialize this reference with the hard interface reference xInt. If the implementation
        behind xInt does not support XWeak or xInt is null then this reference will be null.

        @param xInt another hard interface reference
    */
    WeakReferenceHelper( const css::uno::Reference< css::uno::XInterface > & xInt );

    /** Releases this reference.
    */
    ~WeakReferenceHelper();

    /** Releases this reference and takes over rWeakRef.

        @param rWeakRef another weak ref
    */
    WeakReferenceHelper & SAL_CALL operator = ( const WeakReferenceHelper & rWeakRef );

#if defined LIBO_INTERNAL_ONLY
    WeakReferenceHelper & SAL_CALL operator =(WeakReferenceHelper && other);
#endif

    /** Releases this reference and takes over hard reference xInt.
        If the implementation behind xInt does not support XWeak
        or XInt is null, then this reference is null.

        @param xInt another hard reference
    */
    WeakReferenceHelper & SAL_CALL operator = (
            const css::uno::Reference< css::uno::XInterface > & xInt );

    /** Returns true if both weak refs reference to the same object.

        @param rObj another weak ref
        @return true, if both weak refs reference to the same object.
    */
    bool SAL_CALL operator == ( const WeakReferenceHelper & rObj ) const
        { return (get() == rObj.get()); }

    /**  Gets a hard reference to the object.

         @return hard reference or null, if the weakly referenced interface has gone
    */
    css::uno::Reference< css::uno::XInterface > SAL_CALL get() const;

    /**  Gets a hard reference to the object.

         @return hard reference or null, if the weakly referenced interface has gone
    */
    SAL_CALL operator Reference< XInterface > () const
        { return get(); }

    /** Releases this reference.

        @since UDK 3.2.12
    */
    void SAL_CALL clear();

protected:
    /// @cond INTERNAL
    OWeakRefListener * m_pImpl;
    /// @endcond
};

/** The WeakReference<> holds a weak reference to an object.

    That object must implement the css::uno::XWeak interface.

    The WeakReference itself is *not* thread safe, just as
    Reference itself isn't, but the implementation of the listeners etc.
    behind it *is* thread-safe, so multiple threads can have their own
    WeakReferences to the same XWeak object.

    @tparam interface_type type of interface
*/
template< class interface_type >
class SAL_WARN_UNUSED WeakReference : public WeakReferenceHelper
{
public:
    /** Default ctor.  Creates an empty weak reference.
    */
    WeakReference()
        : WeakReferenceHelper()
        {}

    /** Copy ctor.  Initialize this reference with a hard reference.

        @param rRef another hard ref
    */
    WeakReference( const Reference< interface_type > & rRef )
        : WeakReferenceHelper( rRef )
        {}

    /** Releases this reference and takes over hard reference xInt.
        If the implementation behind xInt does not support XWeak
        or XInt is null, then this reference is null.

        @param xInt another hard reference

        @since UDK 3.2.12
    */
    WeakReference & SAL_CALL operator = (
            const css::uno::Reference< interface_type > & xInt )
        { WeakReferenceHelper::operator=(xInt); return *this; }

    /**  Gets a hard reference to the object.

         @return hard reference or null, if the weakly referenced interface has gone
    */
    SAL_CALL operator Reference< interface_type > () const
        { return Reference< interface_type >::query( get() ); }
};

}
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
