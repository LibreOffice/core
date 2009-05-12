/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: component.hxx,v $
 * $Revision: 1.8 $
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
#ifndef _CPPUHELPER_COMPONENT_HXX_
#define _CPPUHELPER_COMPONENT_HXX_

#include <osl/mutex.hxx>
#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#ifndef _CPPUHELPER_IMPLBASE1_HXX
#include <cppuhelper/implbase1.hxx>
#endif

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>


namespace cppu
{

/** Deprecated.  Helper for implementing ::com::sun::star::lang::XComponent.
    Upon disposing objects of this class, sub-classes receive a disposing() call.  Objects of
    this class can be held weakly, i.e. by a ::com::sun::star::uno::WeakReference.  Object of
    this class can be aggregated, i.e. incoming queryInterface() calls are delegated.

    @attention
    The life-cycle of the passed mutex reference has to be longer than objects of this class.
    @deprecated
*/
class OComponentHelper
    : public ::cppu::OWeakAggObject
    , public ::com::sun::star::lang::XTypeProvider
    , public ::com::sun::star::lang::XComponent
{
public:
    /** Constructor.

        @param rMutex
        the mutex used to protect multi-threaded access;
        lifetime must be longer than the lifetime of this object.
    */
    OComponentHelper( ::osl::Mutex & rMutex ) SAL_THROW( () );
    /** Dewstructor. If this object was not disposed previously, object will be disposed manually.
    */
    virtual ~OComponentHelper() SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    // XAggregation
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

    /** @attention
        XTypeProvider::getImplementationId() has to be implemented separately!
    */
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw(::com::sun::star::uno::RuntimeException) = 0;
    /** @attention
        XTypeProvider::getTypes() has to be re-implemented!
    */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose()
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
        throw(::com::sun::star::uno::RuntimeException);

protected:
    /** Called in dispose method after the listeners were notified.
    */
    virtual void SAL_CALL disposing();

    /** @internal */
    OBroadcastHelper    rBHelper;
private:
    /** @internal */
    inline OComponentHelper( const OComponentHelper & ) SAL_THROW( () );
    /** @internal */
    inline OComponentHelper & operator = ( const OComponentHelper & ) SAL_THROW( () );
};

}

#endif
