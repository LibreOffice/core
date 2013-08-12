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
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implementationentry.hxx>
#include "cppuhelper/unourl.hxx"
#include "rtl/malformeduriexception.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/connection/XConnector.hpp>

#include "connector.hxx"

#define IMPLEMENTATION_NAME "com.sun.star.comp.io.Connector"
#define SERVICE_NAME "com.sun.star.connection.Connector"

using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;

namespace stoc_connector
{
    class OConnector : public WeakImplHelper2< XConnector, XServiceInfo >
    {
        Reference< XMultiComponentFactory > _xSMgr;
        Reference< XComponentContext > _xCtx;
    public:
        OConnector(const Reference< XComponentContext > &xCtx);
        ~OConnector();
        // Methods
        virtual Reference< XConnection > SAL_CALL connect(
            const OUString& sConnectionDescription )
            throw( NoConnectException, ConnectionSetupException, RuntimeException);

    public: // XServiceInfo
                virtual OUString              SAL_CALL getImplementationName() throw();
                virtual Sequence< OUString >  SAL_CALL getSupportedServiceNames(void) throw();
                virtual sal_Bool              SAL_CALL supportsService(const OUString& ServiceName) throw();
    };

    OConnector::OConnector(const Reference< XComponentContext > &xCtx)
        : _xSMgr( xCtx->getServiceManager() )
        , _xCtx( xCtx )
    {}

    OConnector::~OConnector() {}

    Reference< XConnection > SAL_CALL OConnector::connect( const OUString& sConnectionDescription )
        throw( NoConnectException, ConnectionSetupException, RuntimeException)
    {
        OSL_TRACE(
            "connector %s\n",
            OUStringToOString(
                sConnectionDescription, RTL_TEXTENCODING_ASCII_US).getStr());

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
                    r = Reference < XConnection > ( (XConnection * ) pConn );
                }
                else
                {
                    OUString sMessage("Connector : couldn't connect to pipe ");
                    sMessage += aName;
                    sMessage += "(";
                    sMessage += OUString::valueOf( (sal_Int32 ) pConn->m_pipe.getError() );
                    sMessage += ")";
                    delete pConn;
                    throw NoConnectException( sMessage ,Reference< XInterface > () );
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
                    throw NoConnectException( sMessage, Reference < XInterface > () );
                }
                if( bTcpNoDelay )
                {
                    sal_Int32 nTcpNoDelay = sal_True;
                    pConn->m_socket.setOption( osl_Socket_OptionTcpNoDelay , &nTcpNoDelay,
                                               sizeof( nTcpNoDelay ) , osl_Socket_LevelTcp );
                }
                pConn->completeConnectionString();
                r = Reference< XConnection > ( (XConnection * ) pConn );
            }
            else
            {
                OUString delegatee("com.sun.star.connection.Connector.");
                delegatee += aDesc.getName();

                OSL_TRACE(
                    "connector: trying to get service %s\n",
                    OUStringToOString(
                        delegatee, RTL_TEXTENCODING_ASCII_US).getStr());
                Reference<XConnector> xConnector(
                    _xSMgr->createInstanceWithContext(delegatee, _xCtx), UNO_QUERY );

                if(!xConnector.is())
                {
                    OUString message("Connector: unknown delegatee ");
                    message += delegatee;

                    throw ConnectionSetupException(message, Reference<XInterface>());
                }

                sal_Int32 index = sConnectionDescription.indexOf((sal_Unicode) ',');

                r = xConnector->connect(sConnectionDescription.copy(index + 1).trim());
            }
            return r;
        }
        catch (const rtl::MalformedUriException & rEx)
        {
            throw ConnectionSetupException(rEx.getMessage(),
                                           Reference< XInterface > ());
        }
    }

    Sequence< OUString > connector_getSupportedServiceNames()
    {
        Sequence< OUString > seqNames(1);
        seqNames.getArray()[0] = SERVICE_NAME;
        return seqNames;
    }

    OUString connector_getImplementationName()
    {
        return OUString( IMPLEMENTATION_NAME );
    }

        OUString OConnector::getImplementationName() throw()
    {
        return connector_getImplementationName();
    }

        sal_Bool OConnector::supportsService(const OUString& ServiceName) throw()
    {
        Sequence< OUString > aSNL = getSupportedServiceNames();
        const OUString * pArray = aSNL.getConstArray();

        for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
            if( pArray[i] == ServiceName )
                return sal_True;

        return sal_False;
    }

        Sequence< OUString > OConnector::getSupportedServiceNames(void) throw()
    {
        return connector_getSupportedServiceNames();
    }

    Reference< XInterface > SAL_CALL connector_CreateInstance( const Reference< XComponentContext > & xCtx)
    {
        return Reference < XInterface >( ( OWeakObject * ) new OConnector(xCtx) );
    }


}
using namespace stoc_connector;

static const struct ImplementationEntry g_entries[] =
{
    {
        connector_CreateInstance, connector_getImplementationName ,
        connector_getSupportedServiceNames, createSingleComponentFactory ,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL connector_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
