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

#ifndef _CONNECTIVITY_ZCONNECTIONWRAPPER_HXX_
#define _CONNECTIVITY_ZCONNECTIONWRAPPER_HXX_

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/CommonTools.hxx"
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{

    //==========================================================================
    //= OConnectionWrapper - wraps all methods to the real connection from the driver
    //= but when disposed it doesn't dispose the real connection
    //==========================================================================
    typedef ::cppu::ImplHelper2<        ::com::sun::star::lang::XServiceInfo,
                                        ::com::sun::star::lang::XUnoTunnel
                                > OConnection_BASE;

    class OOO_DLLPUBLIC_DBTOOLS OConnectionWrapper :     public OConnection_BASE
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation > m_xProxyConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider > m_xTypeProvider;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > m_xUnoTunnel;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo > m_xServiceInfo;

        virtual ~OConnectionWrapper();
        void setDelegation(::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >& _rxProxyConnection,oslInterlockedCount& _rRefCount);
        void setDelegation(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _xORB
            ,oslInterlockedCount& _rRefCount);
        // must be called from derived classes
        void disposing();
    public:
        OConnectionWrapper( );

        // XServiceInfo
        DECLARE_SERVICE_INFO();
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);

        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
        /** method to create unique ids
            @param  _rURL
                The URL.
            @param  _rInfo
                The info property of the datasource. It will be resorted if needed.
            @param  _pBuffer
                Here we store the digest. Must not NULL.
            @param  _rUserName
                The user name.
            @param  _rPassword
                The password.
        */
        static void createUniqueId( const ::rtl::OUString& _rURL
                    ,::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo
                    ,sal_uInt8* _pBuffer
                    ,const ::rtl::OUString& _rUserName = ::rtl::OUString()
                    ,const ::rtl::OUString& _rPassword = ::rtl::OUString());
    };
}
#endif // _CONNECTIVITY_ZCONNECTIONWRAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
