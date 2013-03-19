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

#include "sal/config.h"

#include "stocservices.hxx"

#include <new>

#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/XUriReference.hpp"
#include "com/sun/star/uri/XUriSchemeParser.hpp"
#include "com/sun/star/uri/XVndSunStarExpandUrlReference.hpp"
#include "com/sun/star/util/XMacroExpander.hpp"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/textenc.h"
#include "rtl/uri.h"
#include "rtl/uri.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "UriReference.hxx"
#include "supportsService.hxx"

namespace {

bool parseSchemeSpecificPart(::rtl::OUString const & part) {
    // Liberally accepts both an empty opaque_part and an opaque_part that
    // starts with a non-escaped "/":
    return part.isEmpty()
        || (!::rtl::Uri::decode(part, ::rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8).isEmpty());
}

class UrlReference:
    public ::cppu::WeakImplHelper1< css::uri::XVndSunStarExpandUrlReference >
{
public:
    UrlReference(::rtl::OUString const & scheme, ::rtl::OUString const & path):
        base_(
            scheme, false, false, ::rtl::OUString(), path, false,
            ::rtl::OUString())
    {}

    virtual ::rtl::OUString SAL_CALL getUriReference()
        throw (css::uno::RuntimeException)
    { return base_.getUriReference(); }

    virtual ::sal_Bool SAL_CALL isAbsolute() throw (css::uno::RuntimeException)
    { return base_.isAbsolute(); }

    virtual ::rtl::OUString SAL_CALL getScheme()
        throw (css::uno::RuntimeException)
    { return base_.getScheme(); }

    virtual ::rtl::OUString SAL_CALL getSchemeSpecificPart()
        throw (css::uno::RuntimeException)
    { return base_.getSchemeSpecificPart(); }

    virtual ::sal_Bool SAL_CALL isHierarchical()
        throw (css::uno::RuntimeException)
    { return base_.isHierarchical(); }

    virtual ::sal_Bool SAL_CALL hasAuthority()
        throw (css::uno::RuntimeException)
    { return base_.hasAuthority(); }

    virtual ::rtl::OUString SAL_CALL getAuthority()
        throw (css::uno::RuntimeException)
    { return base_.getAuthority(); }

    virtual ::rtl::OUString SAL_CALL getPath()
        throw (css::uno::RuntimeException)
    { return base_.getPath(); }

    virtual ::sal_Bool SAL_CALL hasRelativePath()
        throw (css::uno::RuntimeException)
    { return base_.hasRelativePath(); }

    virtual ::sal_Int32 SAL_CALL getPathSegmentCount()
        throw (css::uno::RuntimeException)
    { return base_.getPathSegmentCount(); }

    virtual ::rtl::OUString SAL_CALL getPathSegment(sal_Int32 index)
        throw (css::uno::RuntimeException)
    { return base_.getPathSegment(index); }

    virtual ::sal_Bool SAL_CALL hasQuery() throw (css::uno::RuntimeException)
    { return base_.hasQuery(); }

    virtual ::rtl::OUString SAL_CALL getQuery()
        throw (css::uno::RuntimeException)
    { return base_.getQuery(); }

    virtual ::sal_Bool SAL_CALL hasFragment() throw (css::uno::RuntimeException)
    { return base_.hasFragment(); }

    virtual ::rtl::OUString SAL_CALL getFragment()
        throw (css::uno::RuntimeException)
    { return base_.getFragment(); }

    virtual void SAL_CALL setFragment(::rtl::OUString const & fragment)
        throw (css::uno::RuntimeException)
    { base_.setFragment(fragment); }

    virtual void SAL_CALL clearFragment() throw (css::uno::RuntimeException)
    { base_.clearFragment(); }

    virtual ::rtl::OUString SAL_CALL expand(
        css::uno::Reference< css::util::XMacroExpander > const & expander)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

private:
    UrlReference(UrlReference &); // not defined
    void operator =(UrlReference); // not defined

    virtual ~UrlReference() {}

    stoc::uriproc::UriReference base_;
};

::rtl::OUString UrlReference::expand(
    css::uno::Reference< css::util::XMacroExpander > const & expander)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    OSL_ASSERT(expander.is());
    return expander->expandMacros(
        ::rtl::Uri::decode(
            getPath(), ::rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8));
}

class Parser: public ::cppu::WeakImplHelper2<
    css::lang::XServiceInfo, css::uri::XUriSchemeParser >
{
public:
    Parser() {}

    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL supportsService(
        ::rtl::OUString const & serviceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    parse(
        ::rtl::OUString const & scheme,
        ::rtl::OUString const & schemeSpecificPart)
        throw (css::uno::RuntimeException);

private:
    Parser(Parser &); // not defined
    void operator =(Parser); // not defined

    virtual ~Parser() {}
};

::rtl::OUString Parser::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return ::stoc_services::UriSchemeParser_vndDOTsunDOTstarDOTexpand::
        getImplementationName();
}

::sal_Bool Parser::supportsService(::rtl::OUString const & serviceName)
    throw (css::uno::RuntimeException)
{
    return ::stoc::uriproc::supportsService(
        getSupportedServiceNames(), serviceName);
}

css::uno::Sequence< ::rtl::OUString > Parser::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return ::stoc_services::UriSchemeParser_vndDOTsunDOTstarDOTexpand::
        getSupportedServiceNames();
}

css::uno::Reference< css::uri::XUriReference > Parser::parse(
    ::rtl::OUString const & scheme, ::rtl::OUString const & schemeSpecificPart)
    throw (css::uno::RuntimeException)
{
    if (!parseSchemeSpecificPart(schemeSpecificPart)) {
        return css::uno::Reference< css::uri::XUriReference >();
    }
    try {
        return new UrlReference(scheme, schemeSpecificPart);
    } catch (::std::bad_alloc &) {
        throw css::uno::RuntimeException(
            ::rtl::OUString("std::bad_alloc"),
            css::uno::Reference< css::uno::XInterface >());
    }
}

}

namespace stoc_services { namespace UriSchemeParser_vndDOTsunDOTstarDOTexpand {

css::uno::Reference< css::uno::XInterface > create(
    SAL_UNUSED_PARAMETER css::uno::Reference< css::uno::XComponentContext >
        const &)
    SAL_THROW((css::uno::Exception))
{
    //TODO: single instance
    try {
        return static_cast< ::cppu::OWeakObject * >(new Parser);
    } catch (::std::bad_alloc &) {
        throw css::uno::RuntimeException(
            ::rtl::OUString("std::bad_alloc"),
            css::uno::Reference< css::uno::XInterface >());
    }
}

::rtl::OUString getImplementationName() {
    return ::rtl::OUString(
            "com.sun.star.comp.uri.UriSchemeParser_vndDOTsunDOTstarDOTexpand");
}

css::uno::Sequence< ::rtl::OUString > getSupportedServiceNames() {
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(
            "com.sun.star.uri.UriSchemeParser_vndDOTsunDOTstarDOTexpand");
    return s;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
