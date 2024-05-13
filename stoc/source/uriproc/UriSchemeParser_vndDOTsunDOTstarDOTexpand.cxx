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

#include <sal/config.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uri/XUriSchemeParser.hpp>
#include <com/sun/star/uri/XVndSunStarExpandUrlReference.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/textenc.h>
#include <rtl/uri.h>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "UriReference.hxx"

namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::uno { class XInterface; }
namespace com::sun::star::uri { class XUriReference; }

namespace {

bool parseSchemeSpecificPart(OUString const & part) {
    // Liberally accepts both an empty opaque_part and an opaque_part that
    // starts with a non-escaped "/":
    return part.isEmpty()
        || (!::rtl::Uri::decode(part, ::rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8).isEmpty());
}

class UrlReference:
    public ::cppu::WeakImplHelper<css::uri::XVndSunStarExpandUrlReference>
{
public:
    UrlReference(OUString const & scheme, OUString const & path):
        base_(
            scheme, false, OUString(), path, false,
            OUString())
    {}

    UrlReference(const UrlReference&) = delete;
    UrlReference& operator=(const UrlReference&) = delete;

    virtual OUString SAL_CALL getUriReference() override
    { return base_.getUriReference(); }

    virtual sal_Bool SAL_CALL isAbsolute() override
    { return base_.isAbsolute(); }

    virtual OUString SAL_CALL getScheme() override
    { return base_.getScheme(); }

    virtual OUString SAL_CALL getSchemeSpecificPart() override
    { return base_.getSchemeSpecificPart(); }

    virtual sal_Bool SAL_CALL isHierarchical() override
    { return base_.isHierarchical(); }

    virtual sal_Bool SAL_CALL hasAuthority() override
    { return base_.hasAuthority(); }

    virtual OUString SAL_CALL getAuthority() override
    { return base_.getAuthority(); }

    virtual OUString SAL_CALL getPath() override
    { return base_.getPath(); }

    virtual sal_Bool SAL_CALL hasRelativePath() override
    { return base_.hasRelativePath(); }

    virtual ::sal_Int32 SAL_CALL getPathSegmentCount() override
    { return base_.getPathSegmentCount(); }

    virtual OUString SAL_CALL getPathSegment(sal_Int32 index) override
    { return base_.getPathSegment(index); }

    virtual sal_Bool SAL_CALL hasQuery() override
    { return base_.hasQuery(); }

    virtual OUString SAL_CALL getQuery() override
    { return base_.getQuery(); }

    virtual sal_Bool SAL_CALL hasFragment() override
    { return base_.hasFragment(); }

    virtual OUString SAL_CALL getFragment() override
    { return base_.getFragment(); }

    virtual void SAL_CALL setFragment(OUString const & fragment) override
    { base_.setFragment(fragment); }

    virtual void SAL_CALL clearFragment() override
    { base_.clearFragment(); }

    virtual OUString SAL_CALL expand(
        css::uno::Reference< css::util::XMacroExpander > const & expander) override;

private:
    virtual ~UrlReference() override {}

    stoc::uriproc::UriReference base_;
};

OUString UrlReference::expand(
    css::uno::Reference< css::util::XMacroExpander > const & expander)
{
    if (!expander.is()) {
        throw css::uno::RuntimeException(u"null expander passed to XVndSunStarExpandUrl.expand"_ustr);
    }
    return expander->expandMacros(
        ::rtl::Uri::decode(
            getPath(), ::rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8));
}

class Parser:
    public ::cppu::WeakImplHelper<
        css::lang::XServiceInfo, css::uri::XUriSchemeParser>
{
public:
    Parser() {}

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(
        OUString const & serviceName) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    parse(
        OUString const & scheme,
        OUString const & schemeSpecificPart) override;

private:
    virtual ~Parser() override {}
};

OUString Parser::getImplementationName()
{
    return u"com.sun.star.comp.uri.UriSchemeParser_vndDOTsunDOTstarDOTexpand"_ustr;
}

sal_Bool Parser::supportsService(OUString const & serviceName)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > Parser::getSupportedServiceNames()
{
    return { u"com.sun.star.uri.UriSchemeParser_vndDOTsunDOTstarDOTexpand"_ustr };
}

css::uno::Reference< css::uri::XUriReference > Parser::parse(
    OUString const & scheme, OUString const & schemeSpecificPart)
{
    if (!parseSchemeSpecificPart(schemeSpecificPart)) {
        return css::uno::Reference< css::uri::XUriReference >();
    }
    return new UrlReference(scheme, schemeSpecificPart);
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_uri_UriSchemeParser_vndDOTsunDOTstarDOTexpand_get_implementation(css::uno::XComponentContext*,
        css::uno::Sequence<css::uno::Any> const &)
{
    //TODO: single instance
    return ::cppu::acquire(new Parser());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
