/*************************************************************************
 *
 *  $RCSfile: unourl_resolver.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:57 $
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

//  #define TRACE(x) OSL_TRACE(x)
#define TRACE(x)

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/connection/XConnector.hpp>
#include <com/sun/star/bridge/XBridgeFactory.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>

using namespace cppu;
using namespace rtl;
using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::connection;
using namespace com::sun::star::bridge;
using namespace com::sun::star::registry;

#define SERVICENAME     "com.sun.star.bridge.UnoUrlResolver"
#define IMPLNAME        "com.sun.star.comp.bridge.UnoUrlResolver"

namespace unourl_resolver
{

//--------------------------------------------------------------------------------------------------
inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
}

//==================================================================================================
class ResolverImpl : public WeakImplHelper2< XServiceInfo, XUnoUrlResolver >
{
    Reference< XMultiServiceFactory > _xSMgr;

public:
    ResolverImpl( const Reference< XMultiServiceFactory > & xSMgr );
    virtual ~ResolverImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XUnoUrlResolver
    virtual Reference< XInterface > SAL_CALL resolve( const OUString & rUnoUrl )
        throw (NoConnectException, ConnectionSetupException, RuntimeException);
};

//##################################################################################################

//__________________________________________________________________________________________________
ResolverImpl::ResolverImpl( const Reference< XMultiServiceFactory > & xSMgr )
    : _xSMgr( xSMgr )
{
}
//__________________________________________________________________________________________________
ResolverImpl::~ResolverImpl()
{
}

// XServiceInfo
//__________________________________________________________________________________________________
OUString ResolverImpl::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}
//__________________________________________________________________________________________________
sal_Bool ResolverImpl::supportsService( const OUString & rServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    const Sequence< OUString > & rSNL = getSupportedServiceNames();
    const OUString * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (pArray[nPos] == rServiceName)
            return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > ResolverImpl::getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    return unourl_resolver::getSupportedServiceNames();
}

// XUnoUrlResolver
//__________________________________________________________________________________________________
Reference< XInterface > ResolverImpl::resolve( const OUString & rUnoUrl )
    throw (NoConnectException, ConnectionSetupException, RuntimeException)
{
    if (rUnoUrl.getTokenCount( ';' ) != 3 || rUnoUrl.getLength() < 10 ||
        !rUnoUrl.copy( 0, 4 ).equalsIgnoreCase( OUString( RTL_CONSTASCII_USTRINGPARAM("uno:") ) ))
    {
        throw ConnectionSetupException( OUString( RTL_CONSTASCII_USTRINGPARAM("illegal uno url given!" ) ), Reference< XInterface >() );
    }

    Reference< XConnector > xConnector( _xSMgr->createInstance(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Connector") ) ), UNO_QUERY );
    if (! xConnector.is())
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("no connector!" ) ), Reference< XInterface >() );

    OUString aConnectDescr( rUnoUrl.getToken( 0 ).copy( 4 ) ); // uno:CONNECTDESCR;iiop;InstanceName
    OUString aInstanceName( rUnoUrl.getToken( 2 ) );

    Reference< XConnection > xConnection( xConnector->connect( aConnectDescr ) );

    Reference< XBridgeFactory > xBridgeFactory( _xSMgr->createInstance(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory") ) ), UNO_QUERY );
    if (! xBridgeFactory.is())
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("no bridge factory!" ) ), Reference< XInterface >() );

    // bridge
    Reference< XBridge > xBridge( xBridgeFactory->createBridge(
        OUString(), rUnoUrl.getToken( 1 ),
        xConnection, Reference< XInstanceProvider >() ) );

    Reference< XInterface > xRet( xBridge->getInstance( aInstanceName ) );

//      if (! xRet.is()) // dispose bridge manually
//      {
//          Reference< XComponent > xComp( xBridge, UNO_QUERY );
//          if (xComp.is())
//              xComp->dispose();
//      }

    return xRet;
}

//==================================================================================================
static Reference< XInterface > SAL_CALL ResolverImpl_create( const Reference< XMultiServiceFactory > & xSMgr )
{
    return Reference< XInterface >( *new ResolverImpl( xSMgr ) );
}

}


//##################################################################################################
//##################################################################################################
//##################################################################################################


extern "C"
{
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
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );
            xNewKey->createKey( OUString( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) ) );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            unourl_resolver::ResolverImpl_create,
            unourl_resolver::getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


