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
#ifndef _CPPUHELPER_WEAKREF_HXX_
#define _CPPUHELPER_WEAKREF_HXX_

#include <com/sun/star/uno/XInterface.hpp>


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** @internal */
class OWeakRefListener;

/** The WeakReferenceHelper holds a weak reference to an object. This object must implement
    the ::com::sun::star::uno::XWeak interface.  The implementation is thread safe.
*/
class WeakReferenceHelper
{
public:
    /** Default ctor.  Creates an empty weak reference.
    */
    inline WeakReferenceHelper() SAL_THROW( () )
        : m_pImpl( 0 )
        {}

    /** Copy ctor.  Initialize this reference with the same interface as in rWeakRef.

        @param rWeakRef another weak ref
    */
    WeakReferenceHelper( const WeakReferenceHelper & rWeakRef ) SAL_THROW( () );
    /** Initialize this reference with the hard interface reference xInt. If the implementation
        behind xInt does not support XWeak or XInt is null then this reference will be null.

        @param xInt another hard interface reference
    */
    WeakReferenceHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xInt )
        SAL_THROW( () );
    /** Releases this reference.
    */
    ~WeakReferenceHelper() SAL_THROW( () );

    /** Releases this reference and takes over rWeakRef.

        @param rWeakRef another weak ref
    */
    WeakReferenceHelper & SAL_CALL operator = ( const WeakReferenceHelper & rWeakRef ) SAL_THROW( () );

    /** Releases this reference and takes over hard reference xInt.
        If the implementation behind xInt does not support XWeak
        or XInt is null, then this reference is null.

        @param xInt another hard reference
    */
    WeakReferenceHelper & SAL_CALL operator = (
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > & xInt ) SAL_THROW( () );

    /** Returns true if both weak refs reference to the same object.

        @param rObj another weak ref
        @return true, if both weak refs reference to the same object.
    */
    inline sal_Bool SAL_CALL operator == ( const WeakReferenceHelper & rObj ) const SAL_THROW( () )
        { return (get() == rObj.get()); }

    /**  Gets a hard reference to the object.

         @return hard reference or null, if the weakly referenced interface has gone
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL get() const SAL_THROW( () );
    /**  Gets a hard reference to the object.

         @return hard reference or null, if the weakly referenced interface has gone
    */
    inline SAL_CALL operator Reference< XInterface > () const SAL_THROW( () )
        { return get(); }

    /** Releases this reference.

        @since UDK 3.2.12
    */
    void SAL_CALL clear() SAL_THROW( () );

protected:
    /** @internal */
    OWeakRefListener * m_pImpl;
};

/** The WeakReference<> holds a weak reference to an object. This object must implement
    the ::com::sun::star::uno::XWeak interface.  The implementation is thread safe.

    @tplparam interface_type type of interface
*/
template< class interface_type >
class WeakReference : public WeakReferenceHelper
{
public:
    /** Default ctor.  Creates an empty weak reference.
    */
    inline WeakReference() SAL_THROW( () )
        : WeakReferenceHelper()
        {}

    /** Copy ctor.  Initialize this reference with a hard reference.

        @param rRef another hard ref
    */
    inline WeakReference( const Reference< interface_type > & rRef ) SAL_THROW( () )
        : WeakReferenceHelper( rRef )
        {}

    /** Releases this reference and takes over hard reference xInt.
        If the implementation behind xInt does not support XWeak
        or XInt is null, then this reference is null.

        @param xInt another hard reference

        @since UDK 3.2.12
    */
    WeakReference & SAL_CALL operator = (
            const ::com::sun::star::uno::Reference< interface_type > & xInt )
        SAL_THROW( () )
        { WeakReferenceHelper::operator=(xInt); return *this; }

    /**  Gets a hard reference to the object.

         @return hard reference or null, if the weakly referenced interface has gone
    */
    inline SAL_CALL operator Reference< interface_type > () const SAL_THROW( () )
        { return Reference< interface_type >::query( get() ); }
};

}
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
