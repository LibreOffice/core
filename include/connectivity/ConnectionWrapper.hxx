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

#ifndef INCLUDED_CONNECTIVITY_CONNECTIONWRAPPER_HXX
#define INCLUDED_CONNECTIVITY_CONNECTIONWRAPPER_HXX

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/broadcasthelper.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{


    //= OConnectionWrapper - wraps all methods to the real connection from the driver
    //= but when disposed it doesn't dispose the real connection

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
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext
            ,oslInterlockedCount& _rRefCount);
        // must be called from derived classes
        void disposing();
    public:
        OConnectionWrapper( );

        // XServiceInfo
        DECLARE_SERVICE_INFO();
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
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
        static void createUniqueId( const OUString& _rURL
                    ,::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo
                    ,sal_uInt8* _pBuffer
                    ,const OUString& _rUserName = OUString()
                    ,const OUString& _rPassword = OUString());
    };
}
#endif // INCLUDED_CONNECTIVITY_CONNECTIONWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
