/*************************************************************************
 *
 *  $RCSfile: testacquire.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-09 09:21:00 $
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

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/uno/Any.hxx"
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
#include "test/java_uno/acquire/XBase.hpp"
#include "test/java_uno/acquire/XDerived.hpp"
#include "test/java_uno/acquire/XTest.hpp"
#include "uno/environment.h"
#include "uno/lbnames.h"

namespace css = com::sun::star;

namespace {

class Service: public cppu::WeakImplHelper2<
    css::lang::XServiceInfo, test::java_uno::acquire::XTest >
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

    virtual inline void SAL_CALL setInterfaceToInterface(
        css::uno::Reference< css::uno::XInterface > const & rObj)
        throw (css::uno::RuntimeException)
    { m_xInterface = rObj; }

    virtual inline void SAL_CALL setBaseToInterface(
        css::uno::Reference< test::java_uno::acquire::XBase > const & rObj)
        throw (css::uno::RuntimeException)
    { m_xInterface = rObj; }

    virtual inline void SAL_CALL setDerivedToInterface(
        css::uno::Reference< test::java_uno::acquire::XDerived > const & rObj)
        throw (css::uno::RuntimeException)
    { m_xInterface = rObj; }

    virtual inline void SAL_CALL setAnyToInterface(
        css::uno::Any const & rObj)
        throw (css::uno::RuntimeException)
    { rObj >>= m_xInterface; }

    virtual inline css::uno::Reference< css::uno::XInterface >
    SAL_CALL getInterfaceFromInterface() throw (css::uno::RuntimeException)
    { return m_xInterface; }

    virtual inline void SAL_CALL setBaseToBase(
        css::uno::Reference< test::java_uno::acquire::XBase > const & rObj)
        throw (css::uno::RuntimeException)
    { m_xBase = rObj; }

    virtual inline void SAL_CALL setDerivedToBase(
        css::uno::Reference< test::java_uno::acquire::XDerived > const & rObj)
        throw (css::uno::RuntimeException)
    { m_xBase = rObj.get(); }

    virtual inline void SAL_CALL setAnyToBase(
        css::uno::Any const & rObj)
        throw (css::uno::RuntimeException)
    { rObj >>= m_xBase; }

    virtual inline css::uno::Reference< css::uno::XInterface >
    SAL_CALL getInterfaceFromBase() throw (css::uno::RuntimeException)
    { return m_xBase; }

    virtual inline css::uno::Reference< test::java_uno::acquire::XBase >
    SAL_CALL getBaseFromBase() throw (css::uno::RuntimeException)
    { return m_xBase; }

    virtual inline void SAL_CALL setDerivedToDerived(
        css::uno::Reference< test::java_uno::acquire::XDerived > const & rObj)
        throw (css::uno::RuntimeException)
    { m_xDerived = rObj; }

    virtual inline void SAL_CALL setAnyToDerived(
        css::uno::Any const & rObj)
        throw (css::uno::RuntimeException)
    { rObj >>= m_xDerived; }

    virtual inline css::uno::Reference< css::uno::XInterface >
    SAL_CALL getInterfaceFromDerived() throw (css::uno::RuntimeException)
    { return m_xBase; }

    virtual inline css::uno::Reference< test::java_uno::acquire::XBase >
    SAL_CALL getBaseFromDerived() throw (css::uno::RuntimeException)
    { return m_xBase; }

    virtual inline css::uno::Reference< test::java_uno::acquire::XDerived >
    SAL_CALL getDerivedFromDerived() throw (css::uno::RuntimeException)
    { return m_xDerived; }

    virtual inline void SAL_CALL setAnyToAny(
        css::uno::Any const & rObj)
        throw (css::uno::RuntimeException)
    { m_aAny = rObj; }

    virtual inline css::uno::Any
    SAL_CALL getAnyFromAny() throw (css::uno::RuntimeException)
    { return m_aAny; }

    virtual inline css::uno::Reference< css::uno::XInterface >
    SAL_CALL roundTripInterfaceToInterface(
        css::uno::Reference< css::uno::XInterface > const & rObj)
        throw (css::uno::RuntimeException)
    { return rObj; }

    virtual inline css::uno::Reference< css::uno::XInterface >
    SAL_CALL roundTripBaseToInterface(
        css::uno::Reference< test::java_uno::acquire::XBase > const & rObj)
        throw (css::uno::RuntimeException)
    { return rObj; }

    virtual inline css::uno::Reference< css::uno::XInterface >
    SAL_CALL roundTripDerivedToInterface(
        css::uno::Reference< test::java_uno::acquire::XDerived > const & rObj)
        throw (css::uno::RuntimeException)
    { return rObj; }

    virtual inline css::uno::Reference< css::uno::XInterface >
    SAL_CALL roundTripAnyToInterface(
        css::uno::Any const & rObj)
        throw (css::uno::RuntimeException)
    { return css::uno::Reference< css::uno::XInterface >(
        rObj, css::uno::UNO_QUERY_THROW); }

    virtual inline css::uno::Reference< test::java_uno::acquire::XBase >
    SAL_CALL roundTripBaseToBase(
        css::uno::Reference< test::java_uno::acquire::XBase > const & rObj)
        throw (css::uno::RuntimeException)
    { return rObj; }

    virtual inline css::uno::Reference< test::java_uno::acquire::XBase >
    SAL_CALL roundTripDerivedToBase(
        css::uno::Reference< test::java_uno::acquire::XDerived > const & rObj)
        throw (css::uno::RuntimeException)
    { return rObj.get(); }

    virtual inline css::uno::Reference< test::java_uno::acquire::XBase >
    SAL_CALL roundTripAnyToBase(
        css::uno::Any const & rObj)
        throw (css::uno::RuntimeException)
    { return css::uno::Reference< test::java_uno::acquire::XBase >(
        rObj, css::uno::UNO_QUERY_THROW); }

    virtual inline css::uno::Reference< test::java_uno::acquire::XDerived >
    SAL_CALL roundTripDerivedToDerived(
        css::uno::Reference< test::java_uno::acquire::XDerived > const & rObj)
        throw (css::uno::RuntimeException)
    { return rObj; }

    virtual inline css::uno::Reference< test::java_uno::acquire::XDerived >
    SAL_CALL roundTripAnyToDerived(
        css::uno::Any const & rObj)
        throw (css::uno::RuntimeException)
    { return css::uno::Reference< test::java_uno::acquire::XDerived >(
        rObj, css::uno::UNO_QUERY_THROW); }

    virtual inline css::uno::Any
    SAL_CALL roundTripAnyToAny(
        css::uno::Any const & rObj)
        throw (css::uno::RuntimeException)
    { return rObj; }

    static inline sal_Char const * getImplementationName_static()
    { return "com.sun.star.test.bridges.testacquire.impl"; }

    static css::uno::Sequence< rtl::OUString >
    getSupportedServiceNames_static();

    static css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(
        css::uno::Reference< css::uno::XComponentContext > const &)
        throw (css::uno::Exception);

private:
    inline Service() {}

    css::uno::Reference< css::uno::XInterface > m_xInterface;
    css::uno::Reference< test::java_uno::acquire::XBase > m_xBase;
    css::uno::Reference< test::java_uno::acquire::XDerived > m_xDerived;
    css::uno::Any m_aAny;
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

css::uno::Sequence< rtl::OUString > Service::getSupportedServiceNames_static()
{
    css::uno::Sequence< rtl::OUString > aNames(1);
    aNames[0] = rtl::OUString::createFromAscii(
        "com.sun.star.test.bridges.testacquire");
    return aNames;
}

css::uno::Reference< css::uno::XInterface > Service::createInstance(
    css::uno::Reference< css::uno::XComponentContext > const &)
    throw (css::uno::Exception)
{
    return static_cast< cppu::OWeakObject * >(new Service);
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
