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
#include <com/sun/star/beans/XPropertySet.hpp>
#include "FastTokenHandlerService.hxx"

namespace oox {
namespace shape {

using namespace ::com::sun::star;

FastTokenHandlerService::FastTokenHandlerService(uno::Reference< uno::XComponentContext > const & context) :
    m_xContext(context)
{}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL FastTokenHandlerService::getImplementationName() throw (uno::RuntimeException)
{
    return FastTokenHandlerService_getImplementationName();
}

::sal_Bool SAL_CALL FastTokenHandlerService::supportsService(::rtl::OUString const & serviceName) throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > serviceNames = FastTokenHandlerService_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }
    return sal_False;
}

uno::Sequence< ::rtl::OUString > SAL_CALL FastTokenHandlerService::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return FastTokenHandlerService_getSupportedServiceNames();
}

::sal_Int32 SAL_CALL FastTokenHandlerService::getToken(const ::rtl::OUString & Identifier) throw (::com::sun::star::uno::RuntimeException)
{
    return mFastTokenHandler.getToken(Identifier);
}

::rtl::OUString SAL_CALL FastTokenHandlerService::getIdentifier(::sal_Int32 Token) throw (::com::sun::star::uno::RuntimeException)
{
    return mFastTokenHandler.getIdentifier(Token);
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL FastTokenHandlerService::getUTF8Identifier( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException)
{
    return mFastTokenHandler.getUTF8Identifier(Token);
}

::sal_Int32 SAL_CALL FastTokenHandlerService::getTokenFromUTF8(const ::com::sun::star::uno::Sequence< ::sal_Int8 > & Identifier) throw (::com::sun::star::uno::RuntimeException)
{
    return mFastTokenHandler.getTokenFromUTF8(Identifier);
}

::rtl::OUString SAL_CALL FastTokenHandlerService_getImplementationName() {
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.oox.FastTokenHandlerService"));
}

uno::Sequence< ::rtl::OUString > SAL_CALL FastTokenHandlerService_getSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.xml.sax.FastTokenHandler"));
    return s;
}

uno::Reference< uno::XInterface > SAL_CALL FastTokenHandlerService_create(
    const uno::Reference< uno::XComponentContext > & context)
        SAL_THROW((uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new FastTokenHandlerService(context));
}

uno::Reference< uno::XInterface > SAL_CALL 
FastTokenHandlerService_createInstance
( const uno::Reference< lang::XMultiServiceFactory > & rSMgr) 
throw( uno::Exception )
{
    uno::Reference<beans::XPropertySet> 
        xPropertySet(rSMgr, uno::UNO_QUERY_THROW);
    uno::Any aDefaultContext = xPropertySet->getPropertyValue
        (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")));
    
    uno::Reference<uno::XComponentContext> xContext;
    aDefaultContext >>= xContext;

    return FastTokenHandlerService_create(xContext);
}

}}
