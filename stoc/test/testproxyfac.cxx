/*************************************************************************
 *
 *  $RCSfile: testproxyfac.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:36 $
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

#include <osl/diagnose.h>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/util/XProxyFactory.hpp>

#include <rtl/ustrbuf.hxx>

#include <stdio.h>


using namespace rtl;
using namespace cppu;
using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;


static sal_Int32 s_n = 0;

//==================================================================================================
class Test1 : public WeakImplHelper2< XServiceInfo, XProxyFactory >
{
public:
    virtual ~Test1()
        { ++s_n; }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException)
        { return OUString::createFromAscii( "a" ); }
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw (RuntimeException)
        { return sal_False; }
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException)
        { return Sequence< OUString >(); }
    // XProxyFactory
    virtual Reference< XAggregation > SAL_CALL createProxy( const Reference< XInterface > & xTarget ) throw (RuntimeException)
        { return Reference< XAggregation >(); }
};
//==================================================================================================
class Test2 : public WeakImplHelper1< XServiceInfo >
{
    Reference< XAggregation > _xAgg;
public:
    static Reference< XInterface > createTest2( const Reference< XAggregation > & xAgg )
    {
        Test2 * p = new Test2();
        Reference< XInterface > xRet( (XInterface *)(XServiceInfo *)p );
        p->_xAgg = xAgg;
        xAgg->release(); // ref to 1
        p->acquire(); // for xAgg dtor
        p->_xAgg->setDelegator( xRet );
        return xRet;
    }
    virtual ~Test2()
        { ++s_n; }

    virtual Any SAL_CALL queryInterface( const Type & rType ) throw (RuntimeException)
    {
        Any aRet( OWeakObject::queryInterface( rType ) );
        if (! aRet.hasValue())
        {
            aRet = cppu::queryInterface( rType, (XServiceInfo *)this );
            if (! aRet.hasValue())
                return _xAgg->queryAggregation( rType );
        }
        return aRet;
    }
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException)
        { return OUString::createFromAscii( "b" ); }
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw (RuntimeException)
        { return sal_False; }
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException)
        { return Sequence< OUString >(); }
};

static sal_Bool test_proxyfac( const Reference< XProxyFactory > & xProxyFac )
{
    {
    Reference< XServiceInfo > x1( new Test1() );
    Reference< XServiceInfo > x1p( xProxyFac->createProxy( x1 ), UNO_QUERY );
//      if (x1->getImplementationName() != x1p->getImplementationName() ||
//          !x1->getImplementationName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("a") ))
//      {
//          return sal_False;
//      }
//      if (x1 == x1p)
//          return sal_False;
    //

    Reference< XAggregation > xAgg( xProxyFac->createProxy( x1 ) );
    Reference< XInterface > xMaster( Test2::createTest2( xAgg ) );

    Reference< XServiceInfo > x2( xMaster, UNO_QUERY );
    Test2 * pt2 = static_cast< Test2 * >( x2.get() );

    if ((Test2 *)(XServiceInfo *)xMaster.get() != pt2 ||
        !pt2->getImplementationName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("b") ) ||
        !x2->getImplementationName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("b") ))
    {
        return sal_False;
    }

    if (x2 == x1 || x2 == x1p)
        return sal_False;
    if (x2 != xAgg)
        return sal_False;

    if (x1->getImplementationName() == x2->getImplementationName() ||
        x1p->getImplementationName() == x2->getImplementationName())
    {
        return sal_False;
    }

    if (s_n)
        return sal_False;
    }
    return s_n == 2;
}

#ifdef UNX
#define REG_PREFIX      "lib"
#define DLL_POSTFIX     ".so"
#else
#define REG_PREFIX      ""
#define DLL_POSTFIX     ".dll"
#endif

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int __cdecl main( int argc, char * argv[] )
#endif
{
    Reference< XMultiServiceFactory > xMgr( createRegistryServiceFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("stoctest.rdb") ) ) );

    sal_Bool bSucc = sal_False;
    try
    {
        Reference< XImplementationRegistration > xImplReg(
            xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.ImplementationRegistration") ) ), UNO_QUERY );
        OSL_ENSHURE( xImplReg.is(), "### no impl reg!" );

        OUString aLibName( OUString::createFromAscii(REG_PREFIX) );
        aLibName += OUString::createFromAscii("proxyfac");
#ifndef OS2
        aLibName += OUString::createFromAscii(DLL_POSTFIX);
#endif
        xImplReg->registerImplementation(
            OUString::createFromAscii("com.sun.star.loader.SharedLibrary"), aLibName, Reference< XSimpleRegistry >() );

        Reference< XProxyFactory > xProxyFac( xMgr->createInstance( OUString::createFromAscii("com.sun.star.util.ProxyFactory") ), UNO_QUERY );
        OSL_ENSHURE( xProxyFac.is(), "### no proxy factory!" );

        bSucc = test_proxyfac( xProxyFac );
    }
    catch (Exception & rExc)
    {
        OSL_ENSHURE( sal_False, "### exception occured!" );
        OString aMsg( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### exception occured: " );
        OSL_TRACE( aMsg.getStr() );
        OSL_TRACE( "\n" );
    }

    Reference< XComponent >( xMgr, UNO_QUERY )->dispose();

    printf( "testproxyfac %s !\n", (bSucc ? "succeeded" : "failed") );
    return (bSucc ? 0 : -1);
}
