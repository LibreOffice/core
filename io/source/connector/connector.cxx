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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_io.hxx"
#include <osl/mutex.hxx>
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
    rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

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
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    }

    OConnector::~OConnector()
    {
        g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
    }

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
            if (aDesc.getName().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                                 "pipe")))
            {
                rtl::OUString aName(
                    aDesc.getParameter(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("name"))));

                PipeConnection *pConn = new PipeConnection( sConnectionDescription );

                if( pConn->m_pipe.create( aName.pData, osl_Pipe_OPEN, osl::Security() ) )
                {
                    r = Reference < XConnection > ( (XConnection * ) pConn );
                }
                else
                {
                    OUString sMessage = OUString(RTL_CONSTASCII_USTRINGPARAM("Connector : couldn't connect to pipe "));
                    sMessage += aName;
                    sMessage += OUString(RTL_CONSTASCII_USTRINGPARAM("("));
                    sMessage += OUString::valueOf( (sal_Int32 ) pConn->m_pipe.getError() );
                    sMessage += OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
                    delete pConn;
                    throw NoConnectException( sMessage ,Reference< XInterface > () );
                }
            }
            else if (aDesc.getName().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                                      "socket")))
            {
                rtl::OUString aHost;
                if (aDesc.hasParameter(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("host"))))
                    aHost = aDesc.getParameter(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("host")));
                else
                    aHost = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                              "localhost"));
                sal_uInt16 nPort = static_cast< sal_uInt16 >(
                    aDesc.getParameter(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("port"))).
                    toInt32());
                bool bTcpNoDelay
                    = aDesc.getParameter(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                          "tcpnodelay"))).toInt32() != 0;

                SocketConnection *pConn = new SocketConnection( sConnectionDescription);

                SocketAddr AddrTarget( aHost.pData, nPort );
                if(pConn->m_socket.connect(AddrTarget) != osl_Socket_Ok)
                {
                    OUString sMessage = OUString(RTL_CONSTASCII_USTRINGPARAM("Connector : couldn't connect to socket ("));
                    OUString sError = pConn->m_socket.getErrorAsString();
                    sMessage += sError;
                    sMessage += OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
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
                OUString delegatee = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Connector."));
                delegatee += aDesc.getName();

                OSL_TRACE(
                    "connector: trying to get service %s\n",
                    OUStringToOString(
                        delegatee, RTL_TEXTENCODING_ASCII_US).getStr());
                Reference<XConnector> xConnector(
                    _xSMgr->createInstanceWithContext(delegatee, _xCtx), UNO_QUERY );

                if(!xConnector.is())
                {
                    OUString message(RTL_CONSTASCII_USTRINGPARAM("Connector: unknown delegatee "));
                    message += delegatee;

                    throw ConnectionSetupException(message, Reference<XInterface>());
                }

                sal_Int32 index = sConnectionDescription.indexOf((sal_Unicode) ',');

                r = xConnector->connect(sConnectionDescription.copy(index + 1).trim());
            }
            return r;
        }
        catch (rtl::MalformedUriException & rEx)
        {
            throw ConnectionSetupException(rEx.getMessage(),
                                           Reference< XInterface > ());
        }
    }

    Sequence< OUString > connector_getSupportedServiceNames()
    {
        static Sequence < OUString > *pNames = 0;
        if( ! pNames )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( !pNames )
            {
                static Sequence< OUString > seqNames(1);
                seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICE_NAME));
                pNames = &seqNames;
            }
        }
        return *pNames;
    }

    OUString connector_getImplementationName()
    {
        return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
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

static struct ImplementationEntry g_entries[] =
{
    {
        connector_CreateInstance, connector_getImplementationName ,
        connector_getSupportedServiceNames, createSingleComponentFactory ,
        &g_moduleCount.modCnt , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{

sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
