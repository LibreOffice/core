/*************************************************************************
 *
 *  $RCSfile: acceptor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 13:52:40 $
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

#include "acceptor.hxx"

#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMEINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif



namespace desktop
{

extern "C" void workerfunc (void * acc)
{
    ((Acceptor*)acc)->run();
}

static Reference<XInterface> getComponentContext( const Reference<XMultiServiceFactory>& rFactory)
{
    Reference<XInterface> rContext;
    Reference< XPropertySet > rPropSet( rFactory, UNO_QUERY );
    Any a = rPropSet->getPropertyValue(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) );
    a >>= rContext;
    return rContext;
}

Mutex Acceptor::m_aMutex;

Acceptor::Acceptor( const Reference< XMultiServiceFactory >& rFactory )
    : m_aAcceptString()
    , m_aConnectString()
    , m_aProtocol()
    , m_bInit(sal_False)
{
    m_rSMgr = rFactory;
    m_rAcceptor = Reference< XAcceptor > (m_rSMgr->createInstance(
        rtl::OUString::createFromAscii( "com.sun.star.connection.Acceptor" )),
        UNO_QUERY );
    m_rBridgeFactory = Reference < XBridgeFactory > (m_rSMgr->createInstance(
        rtl::OUString::createFromAscii( "com.sun.star.bridge.BridgeFactory" )),
        UNO_QUERY );
    // get component context
    m_rContext = getComponentContext(m_rSMgr);
}


Acceptor::~Acceptor()
{
    // at destruction time, m_rAcceptor will be destoyed, aborting any
    // pending accept calls in the accept thread thus leaving the run method
    // and ending the accept thread. Hence no expicit join here.
    m_rAcceptor->stopAccepting();

}

void SAL_CALL Acceptor::run()
{
    while ( m_rAcceptor.is() && m_rBridgeFactory.is()  )
    {
        RTL_LOGFILE_CONTEXT( aLog, "desktop (lo119109) Acceptor::run" );
        try
        {
            // wait until we get enabled
            RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109)"\
                "Acceptor::run waiting for office to come up");
            m_cEnable.wait();
            RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109)"\
                "Acceptor::run now enabled and continuing");

            // accept connection
            Reference< XConnection > rConnection = m_rAcceptor->accept( m_aConnectString );
            // if we return without a valid connection we mus assume that the acceptor
            // is destructed so we break out of the run method terminating the thread
            if (! rConnection.is()) break;
            OUString aDescription = rConnection->getDescription();
            RTL_LOGFILE_CONTEXT_TRACE1( aLog, "desktop (lo119109) Acceptor::run connection %s",
                OUStringToOString(aDescription, RTL_TEXTENCODING_ASCII_US).getStr());

            // create instanceprovider for this connection
            Reference< XInstanceProvider > rInstanceProvider(
                (XInstanceProvider*)new AccInstanceProvider(m_rSMgr, rConnection));
            // create the bridge. The remote end will have a reference to this bridge
            // thus preventing the bridge from being disposed. When the remote end releases
            // the bridge, it will be destructed.
            Reference< XBridge > rBridge = m_rBridgeFactory->createBridge(
                rtl::OUString() ,m_aProtocol ,rConnection ,rInstanceProvider );

        } catch (Exception&) {
            // connection failed...
            // something went wrong during connection setup.
            // just wait for a new connection to accept
        }
    }
}

// XInitialize
void SAL_CALL Acceptor::initialize( const Sequence<Any>& aArguments )
    throw( Exception )
{
    // prevent multiple initialization
    ClearableMutexGuard aGuard( m_aMutex );
    RTL_LOGFILE_CONTEXT( aLog, "destop (lo119109) Acceptor::initialize()" );

    sal_Bool bOk = sal_False;

    // arg count
    int nArgs = aArguments.getLength();

    // not yet initialized and acceptstring
    if (!m_bInit && nArgs > 0 && (aArguments[0] >>= m_aAcceptString))
    {
        RTL_LOGFILE_CONTEXT_TRACE1( aLog, "desktop (lo119109) Acceptor::initialize string=%s",
            OUStringToOString(m_aAcceptString, RTL_TEXTENCODING_ASCII_US).getStr());

        // get connect string and protocol from accept string
        // "<connectString>;<protocol>"
        sal_Int32 nIndex1 = m_aAcceptString.indexOf( (sal_Unicode) ';' );
        if (nIndex1 < 0) throw IllegalArgumentException(
            OUString::createFromAscii("Invalid accept-string format"), m_rContext, 1);
        m_aConnectString = m_aAcceptString.copy( 0 , nIndex1 ).trim();
        nIndex1++;
        sal_Int32 nIndex2 = m_aAcceptString.indexOf( (sal_Unicode) ';' , nIndex1 );
        if (nIndex2 < 0) nIndex2 = m_aAcceptString.getLength();
        m_aProtocol = m_aAcceptString.copy( nIndex1, nIndex2 - nIndex1 );

        // start accepting in new thread...
        oslThread m_aThread = osl_createThread(workerfunc, this);
        m_bInit = sal_True;
        bOk = sal_True;
    }

    // do we want to enable accepting?
    sal_Bool bEnable = sal_False;
    if ((nArgs == 1 &&  (aArguments[0] >>= bEnable) )
        || (nArgs == 2 && (aArguments[1] >>= bEnable))
        && bEnable )
    {
        m_cEnable.set();
        bOk = sal_True;
    }


    if (!bOk)
    {
        throw IllegalArgumentException(
            OUString::createFromAscii("invalid initialization"), m_rContext, 1);
    }
}

// XServiceInfo
const sal_Char *Acceptor::serviceName = "com.sun.star.office.Acceptor";
const sal_Char *Acceptor::implementationName = "com.sun.star.office.comp.Acceptor";
const sal_Char *Acceptor::supportedServiceNames[] = {"com.sun.star.office.Acceptor", NULL};
OUString Acceptor::impl_getImplementationName()
{
    return OUString::createFromAscii( implementationName );
}
OUString SAL_CALL Acceptor::getImplementationName()
    throw (RuntimeException)
{
    return Acceptor::impl_getImplementationName();
}
Sequence<OUString> Acceptor::impl_getSupportedServiceNames()
{
    Sequence<OUString> aSequence;
    for (int i=0; supportedServiceNames[i]!=NULL; i++) {
        aSequence.realloc(i+1);
        aSequence[i]=(OUString::createFromAscii(supportedServiceNames[i]));
    }
    return aSequence;
}
Sequence<OUString> SAL_CALL Acceptor::getSupportedServiceNames()
    throw (RuntimeException)
{
    return Acceptor::impl_getSupportedServiceNames();
}
sal_Bool Acceptor::impl_supportsService( const OUString& aServiceName)
{
    return sal_False;
}
sal_Bool SAL_CALL Acceptor::supportsService( const OUString& aServiceName)
    throw (RuntimeException)
{
    return Acceptor::impl_supportsService( aServiceName );
}

// Factory
Reference< XInterface > Acceptor::impl_getInstance( const Reference< XMultiServiceFactory >& aFactory )
{
    try {
        return (XComponent*) new Acceptor( aFactory );
    } catch ( Exception& ) {
        return (XComponent*) NULL;
    }
}

// InstanceProvider
AccInstanceProvider::AccInstanceProvider(const Reference<XMultiServiceFactory>& aFactory, const Reference<XConnection>& rConnection)
{
    m_rSMgr = aFactory;
    m_rConnection = rConnection;
}

AccInstanceProvider::~AccInstanceProvider()
{
}

Reference<XInterface> SAL_CALL AccInstanceProvider::getInstance (const OUString& aName )
        throw ( NoSuchElementException )
{

    Reference<XInterface> rInstance;

    if ( aName.compareToAscii( "StarOffice.Startup" ) == 0)
    {
        // rvp client wants to connect
        rInstance = Application::GetUnoInstance( m_rConnection, aName );
    }
    else if ( aName.compareToAscii( "StarOffice.ServiceManager" ) == 0)
    {
        rInstance = Reference< XInterface >( m_rSMgr );
    }
    else if(aName.compareToAscii( "StarOffice.ComponentContext" ) == 0 )
    {
        rInstance = getComponentContext( m_rSMgr );
    }
    else if ( aName.compareToAscii("StarOffice.NamingService" ) == 0 )
    {
        Reference< XNamingService > rNamingService(
            m_rSMgr->createInstance( OUString::createFromAscii( "com.sun.star.uno.NamingService" )),
            UNO_QUERY );
        if ( rNamingService.is() )
        {
            rNamingService->registerObject(
                OUString::createFromAscii( "StarOffice.ServiceManager" ), m_rSMgr );
            rNamingService->registerObject(
                OUString::createFromAscii( "StarOffice.ComponentContext" ), getComponentContext( m_rSMgr ));
            rInstance = rNamingService;
        }
    }
    /*
    else if ( aName.compareToAscii("com.sun.star.ucb.RemoteContentProviderAcceptor" ))
    {
        Reference< XMultiServiceFactory > rSMgr = ::comphelper::getProcessServiceFactory();
        if ( rSMgr.is() ) {
            try {
                rInstance = rSMgr->createInstance( sObjectName );
            }
            catch (Exception const &) {}
        }
    }
    */
    return rInstance;
}

}

// component management stuff...
// ----------------------------------------------------------------------------
extern "C"
{
using namespace desktop;

void SAL_CALL
component_getImplementationEnvironment(const sal_Char **ppEnvironmentTypeName, uno_Environment **ppEnvironment)
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

sal_Bool SAL_CALL
component_writeInfo(void *pServiceManager, void *pRegistryKey)
{
    Reference< XMultiServiceFactory > xMan(reinterpret_cast< XMultiServiceFactory* >(pServiceManager));
    Reference< XRegistryKey > xKey(reinterpret_cast< XRegistryKey* >(pRegistryKey));

    // register service
    ::rtl::OUString aTempStr;
    ::rtl::OUString aImpl(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += Acceptor::impl_getImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    Reference< XRegistryKey > xNewKey = xKey->createKey(aImpl);
    xNewKey->createKey(Acceptor::impl_getSupportedServiceNames()[0]);

    return sal_True;
}

void * SAL_CALL
component_getFactory(const sal_Char *pImplementationName, void *pServiceManager, void *pRegistryKey)
{
    void* pReturn = NULL ;
    if  ( pImplementationName && pServiceManager )
    {
        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory > xFactory;
        Reference< XMultiServiceFactory >  xServiceManager(
            reinterpret_cast< XMultiServiceFactory* >(pServiceManager));

        if (Acceptor::impl_getImplementationName().compareToAscii( pImplementationName ) == COMPARE_EQUAL )
        {
            xFactory = Reference< XSingleServiceFactory >( cppu::createSingleFactory(
                xServiceManager, Acceptor::impl_getImplementationName(),
                Acceptor::impl_getInstance, Acceptor::impl_getSupportedServiceNames()) );
        }

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    // Return with result of this operation.
    return pReturn ;
}

} // extern "C"
