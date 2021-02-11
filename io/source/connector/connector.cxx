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

#include <osl/security.hxx>
#include <sal/log.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/unourl.hxx>
#include <rtl/malformeduriexception.hxx>
#include <rtl/ref.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/connection/ConnectionSetupException.hpp>
#include <com/sun/star/connection/NoConnectException.hpp>
#include <com/sun/star/connection/XConnector.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "connector.hxx"

#define IMPLEMENTATION_NAME "com.sun.star.comp.io.Connector"
#define SERVICE_NAME "com.sun.star.connection.Connector"

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::connection;

namespace {

    class OConnector : public WeakImplHelper< XConnector, XServiceInfo >
    {
        Reference< XMultiComponentFactory > _xSMgr;
        Reference< XComponentContext > _xCtx;
    public:
        explicit OConnector(const Reference< XComponentContext > &xCtx);

        // Methods
        virtual Reference< XConnection > SAL_CALL connect(
            const OUString& sConnectionDescription ) override;

    public: // XServiceInfo
                virtual OUString              SAL_CALL getImplementationName() override;
                virtual Sequence< OUString >  SAL_CALL getSupportedServiceNames() override;
                virtual sal_Bool              SAL_CALL supportsService(const OUString& ServiceName) override;
    };

}

OConnector::OConnector(const Reference< XComponentContext > &xCtx)
    : _xSMgr( xCtx->getServiceManager() )
    , _xCtx( xCtx )
{}

Reference< XConnection > SAL_CALL OConnector::connect( const OUString& sConnectionDescription )
{
    // split string into tokens
    try
    {
        cppu::UnoUrlDescriptor aDesc(sConnectionDescription);

        Reference< XConnection > r;
        if ( aDesc.getName() == "pipe" )
        {
            OUString aName(aDesc.getParameter("name"));

            rtl::Reference<stoc_connector::PipeConnection> pConn(new stoc_connector::PipeConnection( sConnectionDescription ));

            if( pConn->m_pipe.create( aName.pData, osl_Pipe_OPEN, osl::Security() ) )
            {
                r = pConn;
            }
            else
            {
                OUString const sMessage(
                    "Connector : couldn't connect to pipe \"" + aName + "\": "
                    + OUString::number(pConn->m_pipe.getError()));
                SAL_WARN("io.connector", sMessage);
                throw NoConnectException( sMessage );
            }
        }
        else if ( aDesc.getName() == "socket" )
        {
            OUString aHost;
            if (aDesc.hasParameter("host"))
                aHost = aDesc.getParameter("host");
            else
                aHost = "localhost";
            sal_uInt16 nPort = static_cast< sal_uInt16 >(
                aDesc.getParameter("port").
                toInt32());
            bool bTcpNoDelay
                = aDesc.getParameter("tcpnodelay").toInt32() != 0;

            rtl::Reference<stoc_connector::SocketConnection> pConn(new stoc_connector::SocketConnection( sConnectionDescription));

            SocketAddr AddrTarget( aHost.pData, nPort );
            if(pConn->m_socket.connect(AddrTarget) != osl_Socket_Ok)
            {
                OUString sMessage("Connector : couldn't connect to socket (");
                OUString sError = pConn->m_socket.getErrorAsString();
                sMessage += sError + ")";
                throw NoConnectException( sMessage );
            }
            // we enable tcpNoDelay for loopback connections because
            // it can make a significant speed difference on linux boxes.
            if( bTcpNoDelay || aHost == "localhost" || aHost.startsWith("127.0.0.") )
            {
                sal_Int32 nTcpNoDelay = sal_Int32(true);
                pConn->m_socket.setOption( osl_Socket_OptionTcpNoDelay , &nTcpNoDelay,
                                           sizeof( nTcpNoDelay ) , osl_Socket_LevelTcp );
            }
            pConn->completeConnectionString();
            r = pConn;
        }
        else
        {
            OUString delegatee= "com.sun.star.connection.Connector." + aDesc.getName();

            Reference<XConnector> xConnector(
                _xSMgr->createInstanceWithContext(delegatee, _xCtx), UNO_QUERY );

            if(!xConnector.is())
                throw ConnectionSetupException("Connector: unknown delegatee " + delegatee);

            sal_Int32 index = sConnectionDescription.indexOf(',');

            r = xConnector->connect(sConnectionDescription.copy(index + 1).trim());
        }
        return r;
    }
    catch (const rtl::MalformedUriException & rEx)
    {
        throw ConnectionSetupException(rEx.getMessage());
    }
}

OUString OConnector::getImplementationName()
{
    return IMPLEMENTATION_NAME;
}

sal_Bool OConnector::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > OConnector::getSupportedServiceNames()
{
    return { SERVICE_NAME };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
io_OConnector_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new OConnector(context));
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
