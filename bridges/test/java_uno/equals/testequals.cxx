/*************************************************************************
 *
 *  $RCSfile: testequals.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:51:10 $
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

#include "com/sun/star/bridge/XBridge.hpp"
#include "com/sun/star/bridge/XBridgeFactory.hpp"
#include "com/sun/star/connection/Connector.hpp"
#include "com/sun/star/connection/XConnection.hpp"
#include "com/sun/star/connection/XConnector.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/string.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "test/java_uno/equals/XBase.hpp"
#include "test/java_uno/equals/XDerived.hpp"
#include "test/java_uno/equals/XTestInterface.hpp"
#include "uno/environment.h"
#include "uno/lbnames.h"

namespace css = com::sun::star;

namespace {

class Service: public cppu::WeakImplHelper2<
    css::lang::XServiceInfo, test::java_uno::equals::XTestInterface >
{
public:
    virtual inline rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    { return rtl::OUString::createFromAscii(getImplementationName_static()); }

    virtual sal_Bool SAL_CALL supportsService(
        rtl::OUString const & rServiceName) throw (css::uno::RuntimeException);

    virtual inline css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames()  throw (css::uno::RuntimeException)
    { return getSupportedServiceNames_static(); }

    virtual void SAL_CALL connect(rtl::OUString const & rConnection,
                                  rtl::OUString const & rProtocol)
        throw (css::uno::Exception);

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL get(
        rtl::OUString const & rName) throw (css::uno::RuntimeException);

    static inline sal_Char const * getImplementationName_static()
    { return "com.sun.star.test.bridges.testequals.impl"; }

    static css::uno::Sequence< rtl::OUString >
    getSupportedServiceNames_static();

    static css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(
        css::uno::Reference< css::uno::XComponentContext > const & rContext)
        throw (css::uno::Exception);

private:
    explicit inline Service(
        css::uno::Reference< css::uno::XComponentContext > const & rContext):
        m_xContext(rContext) {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::bridge::XBridge > m_xBridge;
};

}

sal_Bool Service::supportsService(rtl::OUString const & rServiceName)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > aNames(
        getSupportedServiceNames_static());
    for (sal_Int32 i = 0; i< aNames.getLength(); ++i)
        if (aNames[i] == rServiceName)
            return true;
    return false;
}

void Service::connect(rtl::OUString const & rConnection,
                      rtl::OUString const & rProtocol)
    throw (css::uno::Exception)
{
    css::uno::Reference< css::connection::XConnection > xConnection(
        css::connection::Connector::create(m_xContext)->connect(rConnection));
    css::uno::Reference< css::bridge::XBridgeFactory > xBridgeFactory(
        m_xContext->getServiceManager()->createInstanceWithContext(
            rtl::OUString::createFromAscii("com.sun.star.bridge.BridgeFactory"),
            m_xContext),
        css::uno::UNO_QUERY);
    m_xBridge = xBridgeFactory->createBridge(rtl::OUString(), rProtocol,
                                             xConnection, 0);
}

css::uno::Reference< css::uno::XInterface >
Service::get(rtl::OUString const & rName) throw (css::uno::RuntimeException)
{
    return m_xBridge->getInstance(rName);
}

css::uno::Sequence< rtl::OUString > Service::getSupportedServiceNames_static()
{
    css::uno::Sequence< rtl::OUString > aNames(1);
    aNames[0] = rtl::OUString::createFromAscii(
        "com.sun.star.test.bridges.testequals");
    return aNames;
}

css::uno::Reference< css::uno::XInterface > Service::createInstance(
    css::uno::Reference< css::uno::XComponentContext > const & rContext)
    throw (css::uno::Exception)
{
    // Make types known:
    getCppuType(
        static_cast<
        css::uno::Reference< test::java_uno::equals::XBase > const * >(0));
    getCppuType(
        static_cast<
        css::uno::Reference< test::java_uno::equals::XDerived > const * >(0));
    getCppuType(
        static_cast<
        css::uno::Reference< test::java_uno::equals::XTestInterface > const * >(
            0));

    return static_cast< cppu::OWeakObject * >(new Service(rContext));
}

extern "C" void SAL_CALL component_getImplementationEnvironment(
    sal_Char const ** pEnvTypeName, uno_Environment **)
{
    *pEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

extern "C" void * SAL_CALL component_getFactory(sal_Char const * pImplName,
                                                void * pServiceManager, void *)
{
    void * pFactory = 0;
    if (pServiceManager)
        if (rtl_str_compare(pImplName, Service::getImplementationName_static())
            == 0)
        {
            css::uno::Reference< css::lang::XSingleComponentFactory >
                xFactory(cppu::createSingleComponentFactory(
                             &Service::createInstance,
                             rtl::OUString::createFromAscii(
                                 Service::getImplementationName_static()),
                             Service::getSupportedServiceNames_static()));
            if (xFactory.is())
            {
                xFactory->acquire();
                pFactory = xFactory.get();
            }
        }
    return pFactory;
}

namespace {

bool writeInfo(void * pRegistryKey, sal_Char const * pImplementationName,
               css::uno::Sequence< rtl::OUString > const & rServiceNames)
{
    rtl::OUString aKeyName(rtl::OUString::createFromAscii("/"));
    aKeyName += rtl::OUString::createFromAscii(pImplementationName);
    aKeyName += rtl::OUString::createFromAscii("/UNO/SERVICES");
    css::uno::Reference< css::registry::XRegistryKey > xKey;
    try
    {
        xKey = static_cast< css::registry::XRegistryKey * >(pRegistryKey)->
            createKey(aKeyName);
    }
    catch (css::registry::InvalidRegistryException &) {}
    if (!xKey.is())
        return false;
    bool bSuccess = true;
    for (sal_Int32 i = 0; i < rServiceNames.getLength(); ++i)
        try
        {
            xKey->createKey(rServiceNames[i]);
        }
        catch (css::registry::InvalidRegistryException &)
        {
            bSuccess = false;
            break;
        }
    return bSuccess;
}

}

extern "C" sal_Bool SAL_CALL component_writeInfo(void *, void * pRegistryKey)
{
    return pRegistryKey
        && writeInfo(pRegistryKey, Service::getImplementationName_static(),
                     Service::getSupportedServiceNames_static());
}
