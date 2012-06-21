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
    inline explicit WeakReference() SAL_THROW(()) : mpObject( 0 ) {}

    /** Initializes this weak reference with the passed reference to an object.
     */
    inline explicit WeakReference( const ::rtl::Reference< ObjectType >& rxObject ) SAL_THROW(()) :
        mxWeakRef( rxObject.get() ), mpObject( rxObject.get() ) {}

    /** Releases this weak reference and takes over the passed reference.
     */
    inline WeakReference& SAL_CALL operator=( const ::rtl::Reference< ObjectType >& rxObject ) SAL_THROW(())
    {
        mxWeakRef = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XWeak >( rxObject.get() );
        mpObject = rxObject.get();
        return *this;
    }

    /** Gets an RTL reference to the referenced object.

        @return  Reference or null, if the weakly referenced object is gone.
     */
    inline SAL_CALL operator ::rtl::Reference< ObjectType >() SAL_THROW(())
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
