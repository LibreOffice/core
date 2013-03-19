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


#include "stocservices.hxx"

#include "supportsService.hxx"

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/UriReferenceFactory.hpp"
#include "com/sun/star/uri/XUriReference.hpp"
#include "com/sun/star/uri/XUriReferenceFactory.hpp"
#include "com/sun/star/uri/XVndSunStarPkgUrlReferenceFactory.hpp"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/string.h"
#include "rtl/textenc.h"
#include "rtl/uri.h"
#include "rtl/uri.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <new>

namespace {

class Factory: public cppu::WeakImplHelper2<
    css::lang::XServiceInfo, css::uri::XVndSunStarPkgUrlReferenceFactory >
{
public:
    explicit Factory(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        m_context(context) {}

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    createVndSunStarPkgUrlReference(
        css::uno::Reference< css::uri::XUriReference > const & authority)
        throw (css::uno::RuntimeException);

private:
    Factory(Factory &); // not implemented
    void operator =(Factory); // not implemented

    virtual ~Factory() {}

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

rtl::OUString Factory::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return
        stoc_services::VndSunStarPkgUrlReferenceFactory::
        getImplementationName();
}

sal_Bool Factory::supportsService(rtl::OUString const & serviceName)
    throw (css::uno::RuntimeException)
{
    return stoc::uriproc::supportsService(
        getSupportedServiceNames(), serviceName);
}

css::uno::Sequence< rtl::OUString > Factory::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return stoc_services::VndSunStarPkgUrlReferenceFactory::
        getSupportedServiceNames();
}

css::uno::Reference< css::uri::XUriReference >
Factory::createVndSunStarPkgUrlReference(
    css::uno::Reference< css::uri::XUriReference > const & authority)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(authority.is());
    if (authority->isAbsolute() && !authority->hasFragment()) {
        rtl::OUStringBuffer buf;
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg://"));
        buf.append(
            rtl::Uri::encode(
                authority->getUriReference(), rtl_UriCharClassRegName,
                rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8));
        css::uno::Reference< css::uri::XUriReference > uriRef(
            css::uri::UriReferenceFactory::create(m_context)->parse(
                buf.makeStringAndClear()));
        OSL_ASSERT(uriRef.is());
        return uriRef;
    } else {
        return css::uno::Reference< css::uri::XUriReference >();
    }
}

}

namespace stoc_services { namespace VndSunStarPkgUrlReferenceFactory
{

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    SAL_THROW((css::uno::Exception))
{
    try {
        return static_cast< cppu::OWeakObject * >(new Factory(context));
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException(
            rtl::OUString("std::bad_alloc"), 0);
    }
}

rtl::OUString getImplementationName() {
    return rtl::OUString("com.sun.star.comp.uri.VndSunStarPkgUrlReferenceFactory");
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    css::uno::Sequence< rtl::OUString > s(1);
    s[0] = rtl::OUString("com.sun.star.uri.VndSunStarPkgUrlReferenceFactory");
    return s;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
