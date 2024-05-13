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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XVndSunStarPkgUrlReferenceFactory.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/textenc.h>
#include <rtl/uri.h>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include <utility>
#include <sal/types.h>

namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::uno { class XInterface; }

namespace {

class Factory:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo, css::uri::XVndSunStarPkgUrlReferenceFactory>
{
public:
    explicit Factory(
        css::uno::Reference< css::uno::XComponentContext > context):
        m_context(std::move(context)) {}

    Factory(const Factory&) = delete;
    Factory& operator=(const Factory&) = delete;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & serviceName) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    createVndSunStarPkgUrlReference(
        css::uno::Reference< css::uri::XUriReference > const & authority) override;

private:
    virtual ~Factory() override {}

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

OUString Factory::getImplementationName()
{
    return u"com.sun.star.comp.uri.VndSunStarPkgUrlReferenceFactory"_ustr;
}

sal_Bool Factory::supportsService(OUString const & serviceName)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > Factory::getSupportedServiceNames()
{
    css::uno::Sequence< OUString > s { u"com.sun.star.uri.VndSunStarPkgUrlReferenceFactory"_ustr };
    return s;
}

css::uno::Reference< css::uri::XUriReference >
Factory::createVndSunStarPkgUrlReference(
    css::uno::Reference< css::uri::XUriReference > const & authority)
{
    if (!authority.is()) {
        throw css::uno::RuntimeException(
            u"null authority passed to"
            " XVndSunStarPkgUrlReferenceFactory.createVndSunStarPkgUrlReference"_ustr);
    }
    if (authority->isAbsolute() && !authority->hasFragment()) {
        OUString buf =
            "vnd.sun.star.pkg://" +
            rtl::Uri::encode(
                authority->getUriReference(), rtl_UriCharClassRegName,
                rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8);
        css::uno::Reference< css::uri::XUriReference > uriRef(
            css::uri::UriReferenceFactory::create(m_context)->parse(
                buf));
        return uriRef;
    } else {
        return css::uno::Reference< css::uri::XUriReference >();
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_uri_VndSunStarPkgUrlReferenceFactory_get_implementation(css::uno::XComponentContext* rxContext,
        css::uno::Sequence<css::uno::Any> const &)
{
    return ::cppu::acquire(new Factory(rxContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
