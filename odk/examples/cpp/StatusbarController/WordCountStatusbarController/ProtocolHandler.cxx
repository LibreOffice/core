/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "defines.hxx"
#include "ProtocolHandler.hxx"
#include "DispatchPool.hxx"

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <cppuhelper/compbase4.hxx>
#include <uno/current_context.hxx>

using namespace framework::statusbar_controller_wordcount;

using namespace com::sun::star;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;

using rtl::OUString;

namespace
{
    struct StaticSupportedServices
            : public rtl::StaticWithInit< Sequence< OUString >, StaticSupportedServices >
    {
        Sequence< OUString >
        operator( )( )
        {
            Sequence< OUString > aServices( 2 );
            aServices[0] = C2U( PROTOCOL_HANDLER_SERVICE_NAME );
            aServices[1] = OUSTR_SERVICENAME_PROTOCOLHANDLER;
            return aServices;
        }
    };
}

ProtocolHandler::ProtocolHandler( Reference< XComponentContext > const &rxContext )
    : ProtocolHandler_Base( m_aMutex ),
      m_xContext( rxContext ),
      m_xFrame( )
{
    OSL_TRACE( "sbctlwc::ProtocolHandler::ProtocolHandler" );
}

ProtocolHandler::~ProtocolHandler( )
{
    OSL_TRACE( "sbctlwc::ProtocolHandler::~ProtocolHandler" );
}

void
ProtocolHandler::ThrowIfDisposed( ) throw ( RuntimeException )
{
    if ( rBHelper.bInDispose || rBHelper.bDisposed )
        throw DisposedException( );
}

void SAL_CALL
ProtocolHandler::disposing()
{
}

void SAL_CALL
ProtocolHandler::initialize( const Sequence< Any > &aArguments )
throw ( Exception, RuntimeException )
{
    OSL_TRACE( "sbctlwc::ProtocolHandler::initialize" );
    Reference < XFrame > xFrame;
    if ( aArguments.getLength( ) )
    {
        aArguments[0] >>= xFrame;
        osl::MutexGuard aGuard( m_aMutex );
        m_xFrame = xFrame;
        try
        {
            Reference< XModuleManager > xModuleManager(
                m_xContext->getServiceManager()->createInstanceWithContext(
                    OUSTR_SERVICENAME_MODULEMANAGER, m_xContext ),
                UNO_QUERY );
            m_sModuleIdentifier = xModuleManager->identify( m_xFrame );
        }
        catch ( ... ) {}
    }
}

Reference< XDispatch > SAL_CALL
ProtocolHandler::queryDispatch(
    const URL &aURL,
    const OUString &sTargetFrameName,
    sal_Int32 )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlwc::ProtocolHandler::queryDispatch" );
    Reference< XDispatch > xRet;
    OUString sModuleIdentifier;

    osl::ClearableMutexGuard aGuard( m_aMutex );
    Reference< XFrame > xFrame = m_xFrame;
    Reference< XComponentContext > xContext = m_xContext;
    sModuleIdentifier = m_sModuleIdentifier;
    aGuard.clear( );

    if ( !xFrame.is( ) )
        return xRet;

    if ( aURL.Protocol.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( PROTOCOL_HANDLER_URL_PROTOCOL":" ) )
            && sTargetFrameName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "_self" ) ) )
    {
        return DispatchPool::GetDispatch( xContext, xFrame, aURL, sModuleIdentifier );
    }

    return xRet;
}

Sequence< Reference< XDispatch > >
SAL_CALL
ProtocolHandler::queryDispatches( const Sequence< DispatchDescriptor > &seqDescripts )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlwc::ProtocolHandler::queryDispatches" );
    ::sal_Int32 nCount = seqDescripts.getLength( );
    Sequence< Reference< XDispatch > > lDispatcher( nCount );

    for ( ::sal_Int32 i = 0; i < nCount; ++i )
    {
        lDispatcher[i] = queryDispatch(
                             seqDescripts[i].FeatureURL,
                             seqDescripts[i].FrameName,
                             seqDescripts[i].SearchFlags );
    }

    return lDispatcher;
}

OUString SAL_CALL
ProtocolHandler::getImplementationName( ) throw ( RuntimeException )
{
    return getImplementationName_static( );
}

::sal_Bool SAL_CALL
ProtocolHandler::supportsService( OUString const &ServiceName ) throw ( RuntimeException )
{
    const Sequence< OUString > &aServices = StaticSupportedServices::get( );

    for ( const OUString *pService = aServices.getConstArray( ),
            *pEnd = pService + aServices.getLength( );
            pService != pEnd;
            pService++ )
    {
        if ( pService->equals( ServiceName ) )
            return sal_True;
    }

    return sal_False;
}

Sequence< OUString > SAL_CALL
ProtocolHandler::getSupportedServiceNames( ) throw ( RuntimeException )
{
    return getSupportedServiceNames_static( );
}

Reference< XInterface > ProtocolHandler::Create( const Reference< XComponentContext > &rxContext )
throw( Exception )
{
    return Reference< XInterface >(
               static_cast < ::cppu::OWeakObject * > ( new ProtocolHandler( rxContext ) ),
               UNO_QUERY );
}

OUString
ProtocolHandler::getImplementationName_static( )
{
    static OUString st_ImplName = C2U( PROTOCOL_HANDLER_IMPL_NAME );
    return st_ImplName;
}

Sequence< OUString >
ProtocolHandler::getSupportedServiceNames_static( )
{
    return StaticSupportedServices::get( );
}
