/*************************************************************************
 *
 *  $RCSfile: connector.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 14:58:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <osl/mutex.hxx>

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

#if defined( MACOSX ) && ( __GNUC__ < 3 )
ClassData2 WeakImplHelper2< ::com::sun::star::connection::XConnection, ::com::sun::star::connection::XConnectionBroadcaster >::s_aCD(1);
ClassData2 WeakImplHelper2<com::sun::star::connection::XConnector, com::sun::star::lang::XServiceInfo>::s_aCD(1);
ClassData1 WeakImplHelper1<com::sun::star::connection::XConnection>::s_aCD(1);
#endif

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
        virtual Reference< XConnection > SAL_CALL OConnector::connect(
            const OUString& sConnectionDescription )
            throw( NoConnectException, ConnectionSetupException, RuntimeException);

    public: // XServiceInfo
                virtual OUString              SAL_CALL getImplementationName() throw();
                virtual Sequence< OUString >  SAL_CALL getSupportedServiceNames(void) throw();
                virtual sal_Bool              SAL_CALL supportsService(const OUString& ServiceName) throw();
    };

    OConnector::OConnector(const Reference< XComponentContext > &xCtx)
        : _xCtx( xCtx )
        , _xSMgr( xCtx->getServiceManager() )
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
#if OSL_DEBUG_LEVEL > 1
        OString tmp = OUStringToOString(sConnectionDescription, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("connector %s\n", tmp.getStr());
#endif

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

                PipeConnection *pConn = new PipeConnection( aName, sConnectionDescription );

                if( pConn->m_pipe.create( aName.pData, osl_Pipe_OPEN ) )
                {
                    r = Reference < XConnection > ( (XConnection * ) pConn );
                }
                else
                {
                    OUString sMessage = OUString::createFromAscii( "Connector : couldn't connect to pipe " );
                    sMessage += aName;
                    sMessage += OUString::createFromAscii( "(" );
                    sMessage += OUString::valueOf( (sal_Int32 ) pConn->m_pipe.getError() );
                    sMessage += OUString::createFromAscii( ")" );
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

                SocketConnection *pConn = new SocketConnection( aHost,
                                                                nPort,
                                                                sConnectionDescription);

                SocketAddr AddrTarget( aHost.pData, nPort );
                if(pConn->m_socket.connect(AddrTarget) != osl_Socket_Ok)
                {
                    OUString sMessage = OUString::createFromAscii( "Connector : couldn't connect to socket (" );
                    OUString sError = pConn->m_socket.getErrorAsString();
                    sMessage += sError;
                    sMessage += OUString::createFromAscii( ")" );
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

#if OSL_DEBUG_LEVEL > 1
                OString tmp = OUStringToOString(delegatee, RTL_TEXTENCODING_ASCII_US);
                OSL_TRACE("connector: trying to get service %s\n", tmp.getStr());
#endif
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
                seqNames.getArray()[0] = OUString::createFromAscii( SERVICE_NAME );
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
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}


