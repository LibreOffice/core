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

#ifndef _CONNECTIVITY_CPOOL_ZDRIVERWRAPPER_HXX_
#define _CONNECTIVITY_CPOOL_ZDRIVERWRAPPER_HXX_

#include <com/sun/star/sdbc/XDriver.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/XAggregation.hpp>

//........................................................................
namespace connectivity
{
//........................................................................

    class OConnectionPool;
    //====================================================================
    //= ODriverWrapper
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::sdbc::XDriver
                                    >   ODriverWrapper_BASE;

    class ODriverWrapper : public ODriverWrapper_BASE
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >
                            m_xDriverAggregate;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >
                            m_xDriver;
        OConnectionPool*    m_pConnectionPool;

    public:
        /** creates a new wrapper for a driver
            @param _rxAggregateDriver
                the driver to aggregate. The object will be reset to <NULL/> when returning from the ctor.
        */
        ODriverWrapper(
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >& _rxAggregateDriver,
            OConnectionPool* _pPool
            );



        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        /// dtor
        virtual ~ODriverWrapper();
        // XDriver
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw (::com::sun::star::uno::RuntimeException);
    };

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // _CONNECTIVITY_CPOOL_ZDRIVERWRAPPER_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
