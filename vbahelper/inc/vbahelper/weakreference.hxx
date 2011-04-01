/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2011 Oracle and/or its affiliates.
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

#ifndef VBAHELPER_WEAKREFERENCE_HXX
#define VBAHELPER_WEAKREFERENCE_HXX

#include <cppuhelper/weakref.hxx>
#include <rtl/ref.hxx>

namespace vbahelper {

// ============================================================================

/** A weak reference holding any UNO implementation object.

    The held object must implement the ::com::sun::star::uno::XWeak interface.

    In difference to the ::com::sun::star::uno::WeakReference<> implementation
    from cppuhelper/weakref.hxx, the class type of this weak reference is not
    restricted to UNO interface types, but can be used for any C++ class type
    implementing the XWeak interface somehow (e.g. ::cppu::WeakImplHelperN<>,
    ::cppu::ImplInheritanceHelperN<>, etc.).
 */
template< typename ObjectType >
class WeakReference
{
public:
    /** Default constructor. Creates an empty weak reference.
     */
    inline explicit WeakReference() SAL_THROW( () ) : mpObject( 0 ) {}

    /** Initializes this weak reference with the passed reference to an object.
     */
    inline explicit WeakReference( const ::rtl::Reference< ObjectType >& rxObject ) SAL_THROW( () ) :
        mxWeakRef( rxObject.get() ), mpObject( rxObject.get() ) {}

    /** Releases this weak reference and takes over the passed reference.
     */
    inline WeakReference& SAL_CALL operator=( const ::rtl::Reference< ObjectType >& rxObject ) SAL_THROW( () )
    {
        mxWeakRef = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XWeak >( rxObject.get() );
        mpObject = rxObject.get();
        return *this;
    }

    /** Gets an RTL reference to the referenced object.

        @return  Reference or null, if the weakly referenced object is gone.
     */
    inline SAL_CALL operator ::rtl::Reference< ObjectType >() SAL_THROW( () )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XWeak > xRef = mxWeakRef;
        ::rtl::Reference< ObjectType > xObject;
        if( xRef.is() )
            xObject = mpObject;
        else
            mpObject = 0;
        return xObject;
    }

private:
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XWeak > mxWeakRef;
    ObjectType* mpObject;
};

// ============================================================================

} // namespace vbahelper

#endif
