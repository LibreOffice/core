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

#include "osl/security.hxx"

#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "cppuhelper/unourl.hxx"
#include "rtl/malformeduriexception.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/connection/ConnectionSetupException.hpp>
#include <com/sun/star/connection/NoConnectException.hpp>
#include <com/sun/star/connection/XConnector.hpp>

#include "services.hxx"
#include "connector.hxx"

#define IMPLEMENTATION_NAME "com.sun.star.comp.io.Connector"
#define SERVICE_NAME "com.sun.star.connection.Connector"

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;

namespace stoc_connector
{
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

                PipeConnection *pConn = new PipeConnection( sConnectionDescription );

                if( pConn->m_pipe.create( aName.pData, osl_Pipe_OPEN, osl::Security() ) )
                {
                    r.set( static_cast<XConnection *>(pConn) );
                }
                else
                {
                    OUString const sMessage(
                        "Connector : couldn't connect to pipe " + aName + "("
                        + OUString::number(pConn->m_pipe.getError()) + ")");
                    SAL_WARN("io.connector", sMessage);
                    delete pConn;
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

                SocketConnection *pConn = new SocketConnection( sConnectionDescription);

                SocketAddr AddrTarget( aHost.pData, nPort );
                if(pConn->m_socket.connect(AddrTarget) != osl_Socket_Ok)
                {
                    OUString sMessage("Connector : couldn't connect to socket (");
                    OUString sError = pConn->m_socket.getErrorAsString();
                    sMessage += sError;
                    sMessage += ")";
                    delete pConn;
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
                r.set( static_cast<XConnection *>(pConn) );
            }
            else
            {
                OUString delegatee= "com.sun.star.connection.Connector." + aDesc.getName();

                Reference<XConnector> xConnector(
                    _xSMgr->createInstanceWithContext(delegatee, _xCtx), UNO_QUERY );

                if(!xConnector.is())
                {
                    OUString message("Connector: unknown delegatee ");
                    message += delegatee;

                    throw ConnectionSetupException(message);
                }

                sal_Int32 index = sConnectionDescription.indexOf((sal_Unicode) ',');

                r = xConnector->connect(sConnectionDescription.copy(index + 1).trim());
            }
            return r;
        }
        catch (const rtl::MalformedUriException & rEx)
        {
            throw ConnectionSetupException(rEx.getMessage());
        }
    }

    Sequence< OUString > connector_getSupportedServiceNames()
    {
        Sequence< OUString > seqNames { SERVICE_NAME };
        return seqNames;
    }

    OUString connector_getImplementationName()
    {
        return OUString( IMPLEMENTATION_NAME );
    }

        OUString OConnector::getImplementationName()
    {
        return connector_getImplementationName();
    }

        sal_Bool OConnector::supportsService(const OUString& ServiceName)
    {
        return cppu::supportsService(this, ServiceName);
    }

        Sequence< OUString > OConnector::getSupportedServiceNames()
    {
        return connector_getSupportedServiceNames();
    }

    Reference< XInterface > SAL_CALL connector_CreateInstance( const Reference< XComponentContext > & xCtx)
    {
        return Reference < XInterface >( static_cast<OWeakObject *>(new OConnector(xCtx)) );
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
