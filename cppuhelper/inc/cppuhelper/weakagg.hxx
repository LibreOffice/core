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
#ifndef _CPPUHELPER_WEAKAGG_HXX_
#define _CPPUHELPER_WEAKAGG_HXX_

#include <cppuhelper/weak.hxx>
#include <com/sun/star/uno/XAggregation.hpp>


namespace cppu
{

/** Base class to implement an UNO object supporting weak references, i.e. the object can be held
    weakly (by a ::com::sun::star::uno::WeakReference) and aggregation, i.e. the object can be
    aggregated by another (delegator).
    This implementation copes with reference counting.  Upon last release(), the virtual dtor
    is called.

    @derive
    Inherit from this class and delegate acquire()/ release() calls.  Re-implement
    XAggregation::queryInterface().
*/
class OWeakAggObject
    : public ::cppu::OWeakObject
    , public ::com::sun::star::uno::XAggregation
{
public:
    /** Constructor.  No delegator set.
    */
    inline OWeakAggObject() SAL_THROW( () )
        {}

    /** If a delegator is set, then the delegators gets acquired.  Otherwise call is delegated to
        base class ::cppu::OWeakObject.
    */
    virtual void SAL_CALL acquire() throw();
    /** If a delegator is set, then the delegators gets released.  Otherwise call is delegated to
        base class ::cppu::OWeakObject.
    */
    virtual void SAL_CALL release() throw();
    /** If a delegator is set, then the delegator is queried for the demanded interface.  If the
        delegator cannot provide the demanded interface, it calls queryAggregation() on its
        aggregated objects.

        @param rType demanded interface type
        @return demanded type or empty any
        @see queryAggregation.
    */
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
        throw(::com::sun::star::uno::RuntimeException);

    /** Set the delegator.  The delegator member reference is a weak reference.

        @param Delegator the object that delegate its queryInterface to this aggregate.
    */
    virtual void SAL_CALL setDelegator( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & Delegator )
        throw(::com::sun::star::uno::RuntimeException);
    /** Called by the delegator or queryInterface. Re-implement this method instead of
        queryInterface.

        @see queryInterface
    */
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType )
        throw(::com::sun::star::uno::RuntimeException);

protected:
    /** Virtual dtor. Called when reference count is 0.

        @attention
        Despite the fact that a RuntimeException is allowed to be thrown, you must not throw any
        exception upon destruction!
    */
    virtual ~OWeakAggObject() SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    /** weak reference to delegator.
    */
    ::com::sun::star::uno::WeakReferenceHelper xDelegator;
private:
    /** @internal */
    OWeakAggObject( const OWeakAggObject & rObj ) SAL_THROW( () );
    /** @internal */
    OWeakAggObject & operator = ( const OWeakAggObject & rObj ) SAL_THROW( () );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
