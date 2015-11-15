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

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <exception>
#include <vector>

#include <boost/noncopyable.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uri/RelativeUriExcessParentSegments.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XUriSchemeParser.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "UriReference.hxx"

namespace {

bool equalIgnoreEscapeCase(OUString const & s1, OUString const & s2) {
    if (s1.getLength() == s2.getLength()) {
        for (sal_Int32 i = 0; i < s1.getLength();) {
            if (s1[i] == '%' && s2[i] == '%' && s1.getLength() - i > 2
                && rtl::isAsciiHexDigit(s1[i + 1])
                && rtl::isAsciiHexDigit(s1[i + 2])
                && rtl::isAsciiHexDigit(s2[i + 1])
                && rtl::isAsciiHexDigit(s2[i + 2])
                && rtl::compareIgnoreAsciiCase(s1[i + 1], s2[i + 1]) == 0
                && rtl::compareIgnoreAsciiCase(s1[i + 2], s2[i + 2]) == 0)
            {
                i += 3;
            } else if (s1[i] != s2[i]) {
                return false;
            } else {
                ++i;
            }
        }
        return true;
    } else {
        return false;
    }
}

sal_Int32 parseScheme(OUString const & uriReference) {
    if (uriReference.getLength() >= 2 && rtl::isAsciiAlpha(uriReference[0])) {
        for (sal_Int32 i = 0; i < uriReference.getLength(); ++i) {
            sal_Unicode c = uriReference[i];
            if (c == ':') {
                return i;
            } else if (!rtl::isAsciiAlpha(c) && !rtl::isAsciiDigit(c)
                       && c != '+' && c != '-' && c != '.')
            {
                break;
            }
        }
    }
    return -1;
}

class UriReference:
    public cppu::WeakImplHelper<css::uri::XUriReference>,
    private boost::noncopyable
{
public:
    UriReference(
        OUString const & scheme, bool bIsHierarchical, bool bHasAuthority,
        OUString const & authority, OUString const & path,
        bool bHasQuery, OUString const & query):
        m_base(
            scheme, bIsHierarchical, bHasAuthority, authority, path, bHasQuery,
            query)
    {}

    virtual OUString SAL_CALL getUriReference()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.getUriReference(); }

    virtual sal_Bool SAL_CALL isAbsolute()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.isAbsolute(); }

    virtual OUString SAL_CALL getScheme()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.getScheme(); }

    virtual OUString SAL_CALL getSchemeSpecificPart()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.getSchemeSpecificPart(); }

    virtual sal_Bool SAL_CALL isHierarchical()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.isHierarchical(); }

    virtual sal_Bool SAL_CALL hasAuthority()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.hasAuthority(); }

    virtual OUString SAL_CALL getAuthority()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.getAuthority(); }

    virtual OUString SAL_CALL getPath()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.getPath(); }

    virtual sal_Bool SAL_CALL hasRelativePath()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.hasRelativePath(); }

    virtual sal_Int32 SAL_CALL getPathSegmentCount()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.getPathSegmentCount(); }

    virtual OUString SAL_CALL getPathSegment(sal_Int32 index)
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.getPathSegment(index); }

    virtual sal_Bool SAL_CALL hasQuery()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.hasQuery(); }

    virtual OUString SAL_CALL getQuery()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.getQuery(); }

    virtual sal_Bool SAL_CALL hasFragment()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.hasFragment(); }

    virtual OUString SAL_CALL getFragment()
        throw (css::uno::RuntimeException, std::exception) override
    { return m_base.getFragment(); }

    virtual void SAL_CALL setFragment(OUString const & fragment)
        throw (css::uno::RuntimeException, std::exception) override
    { m_base.setFragment(fragment); }

    virtual void SAL_CALL clearFragment()
        throw (css::uno::RuntimeException, std::exception) override
    { m_base.clearFragment(); }

private:
    virtual ~UriReference() {}

    stoc::uriproc::UriReference m_base;
};

css::uno::Reference< css::uri::XUriReference > parseGeneric(
    OUString const & scheme, OUString const & schemeSpecificPart)
{
    bool isAbsolute = !scheme.isEmpty();
    bool isHierarchical = !isAbsolute || schemeSpecificPart.startsWith("/");
    bool hasAuthority = false;
    OUString authority;
    OUString path;
    bool hasQuery = false;
    OUString query;
    if (isHierarchical) {
        sal_Int32 len = schemeSpecificPart.getLength();
        sal_Int32 i = 0;
        if (len - i >= 2 && schemeSpecificPart[i] == '/'
            && schemeSpecificPart[i + 1] == '/')
        {
            i += 2;
            sal_Int32 n = i;
            while (i < len && schemeSpecificPart[i] != '/'
                   && schemeSpecificPart[i] != '?') {
                ++i;
            }
            hasAuthority = true;
            authority = schemeSpecificPart.copy(n, i - n);
        }
        sal_Int32 n = i;
        i = schemeSpecificPart.indexOf('?', i);
        if (i == -1) {
            i = len;
        }
        path = schemeSpecificPart.copy(n, i - n);
        if (i != len) {
            hasQuery = true;
            query = schemeSpecificPart.copy(i + 1);
        }
    } else {
        if (schemeSpecificPart.isEmpty()) {
            // The scheme-specific part of an opaque URI must not be empty:
            return nullptr;
        }
        path = schemeSpecificPart;
    }
    return new UriReference(
        scheme, isHierarchical, hasAuthority, authority, path, hasQuery, query);
}

typedef std::vector< sal_Int32 > Segments;

void processSegments(
    Segments & segments,
    css::uno::Reference< css::uri::XUriReference > const & uriReference,
    bool base, bool processSpecialSegments)
{
    sal_Int32 count = uriReference->getPathSegmentCount() - (base ? 1 : 0);
    assert(count <= SAL_MAX_INT32 - 1 && -count >= SAL_MIN_INT32 + 1);
    for (sal_Int32 i = 0; i < count; ++i) {
        if (processSpecialSegments) {
            OUString segment(uriReference->getPathSegment(i));
            if ( segment == "." ) {
                if (!base && i == count - 1) {
                    segments.push_back(0);
                }
                continue;
            } else if ( segment == ".." ) {
                if (segments.empty() || std::abs(segments.back()) == 1) {
                    segments.push_back(base ? -1 : 1);
                } else {
                    segments.pop_back();
                }
                continue;
            }
        }
        segments.push_back(base ? -(i + 2) : i + 2);
    }
}

class Factory:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo, css::uri::XUriReferenceFactory>,
    private boost::noncopyable
{
public:
    explicit Factory(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        m_context(context) {}

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & serviceName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    parse(OUString const & uriReference)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    makeAbsolute(
        css::uno::Reference< css::uri::XUriReference > const & baseUriReference,
        css::uno::Reference< css::uri::XUriReference > const & uriReference,
        sal_Bool processSpecialBaseSegments,
        css::uri::RelativeUriExcessParentSegments excessParentSegments)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    makeRelative(
        css::uno::Reference< css::uri::XUriReference > const & baseUriReference,
        css::uno::Reference< css::uri::XUriReference > const & uriReference,
        sal_Bool preferAuthorityOverRelativePath,
        sal_Bool preferAbsoluteOverRelativePath,
        sal_Bool encodeRetainedSpecialSegments)
        throw (css::uno::RuntimeException, std::exception) override;

private:
    virtual ~Factory() {}

    css::uno::Reference< css::uri::XUriReference > clone(
        css::uno::Reference< css::uri::XUriReference > const & uriReference)
    { return parse(uriReference->getUriReference()); }

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

OUString Factory::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.uri.UriReferenceFactory");
}

sal_Bool Factory::supportsService(OUString const & serviceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > Factory::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence< OUString > s { "com.sun.star.uri.UriReferenceFactory" };
    return s;
}

css::uno::Reference< css::uri::XUriReference > Factory::parse(
    OUString const & uriReference)
    throw (css::uno::RuntimeException, std::exception)
{
    sal_Int32 fragment = uriReference.indexOf('#');
    if (fragment == -1) {
        fragment = uriReference.getLength();
    }
    OUString scheme;
    OUString schemeSpecificPart;
    OUString serviceName;
    sal_Int32 n = parseScheme(uriReference);
    assert(n < fragment);
    if (n >= 0) {
        scheme = uriReference.copy(0, n);
        schemeSpecificPart = uriReference.copy(n + 1, fragment - (n + 1));
        OUStringBuffer buf;
        buf.append("com.sun.star.uri.UriSchemeParser_");
        for (sal_Int32 i = 0; i < scheme.getLength(); ++i) {
            sal_Unicode c = scheme[i];
            if (rtl::isAsciiUpperCase(c)) {
                buf.append(static_cast<sal_Unicode>(rtl::toAsciiLowerCase(c)));
            } else if (c == '+') {
                buf.append("PLUS");
            } else if (c == '-') {
                buf.append("HYPHEN");
            } else if (c == '.') {
                buf.append("DOT");
            } else {
                assert(rtl::isAsciiLowerCase(c) || rtl::isAsciiDigit(c));
                buf.append(c);
            }
        }
        serviceName = buf.makeStringAndClear();
    } else {
        schemeSpecificPart = uriReference.copy(0, fragment);
    }
    css::uno::Reference< css::uri::XUriSchemeParser > parser;
    if (!serviceName.isEmpty()) {
        css::uno::Reference< css::lang::XMultiComponentFactory > factory(
            m_context->getServiceManager());
        if (factory.is()) {
            css::uno::Reference< css::uno::XInterface > service;
            try {
                service = factory->createInstanceWithContext(
                    serviceName, m_context);
            } catch (css::uno::RuntimeException &) {
                throw;
            } catch (const css::uno::Exception & e) {
                throw css::lang::WrappedTargetRuntimeException(
                    "creating service " + serviceName,
                    static_cast< cppu::OWeakObject * >(this),
                    css::uno::makeAny(e)); //TODO: preserve type of e
            }
            if (service.is()) {
                parser.set( service, css::uno::UNO_QUERY_THROW);
            }
        }
    }
    css::uno::Reference< css::uri::XUriReference > uriRef(
        parser.is()
        ? parser->parse(scheme, schemeSpecificPart)
        : parseGeneric(scheme, schemeSpecificPart));
    if (uriRef.is() && fragment != uriReference.getLength()) {
        uriRef->setFragment(uriReference.copy(fragment + 1));
    }
    return uriRef;
}

css::uno::Reference< css::uri::XUriReference > Factory::makeAbsolute(
    css::uno::Reference< css::uri::XUriReference > const & baseUriReference,
    css::uno::Reference< css::uri::XUriReference > const & uriReference,
    sal_Bool processSpecialBaseSegments,
    css::uri::RelativeUriExcessParentSegments excessParentSegments)
    throw (css::uno::RuntimeException, std::exception)
{
    if (!baseUriReference.is() || !baseUriReference->isAbsolute()
        || !baseUriReference->isHierarchical() || !uriReference.is()) {
        return nullptr;
    } else if (uriReference->isAbsolute()) {
        return clone(uriReference);
    } else if (!uriReference->hasAuthority()
               && uriReference->getPath().isEmpty()
               && !uriReference->hasQuery()) {
        css::uno::Reference< css::uri::XUriReference > abs(
            clone(baseUriReference));
        if (uriReference->hasFragment()) {
            abs->setFragment(uriReference->getFragment());
        } else {
            abs->clearFragment();
        }
        return abs;
    } else {
        OUStringBuffer abs(baseUriReference->getScheme());
        abs.append(':');
        if (uriReference->hasAuthority()) {
            abs.append("//");
            abs.append(uriReference->getAuthority());
        } else if (baseUriReference->hasAuthority()) {
            abs.append("//");
            abs.append(baseUriReference->getAuthority());
        }
        if (uriReference->hasRelativePath()) {
            Segments segments;
            processSegments(
                segments, baseUriReference, true, processSpecialBaseSegments);
            processSegments(segments, uriReference, false, true);
            // If the path component of the base URI reference is empty (which
            // implies that the base URI reference denotes a "root entity"), and
            // the resulting URI reference denotes the same root entity, make
            // sure the path component of the resulting URI reference is also
            // empty (and not "/").  RFC 2396 is unclear about this, and I chose
            // these rules for consistent results.
            bool slash = !baseUriReference->getPath().isEmpty();
            if (slash) {
                abs.append('/');
            }
            for (Segments::iterator i(segments.begin()); i != segments.end();
                 ++i)
            {
                if (*i < -1) {
                    OUString segment(
                        baseUriReference->getPathSegment(-(*i + 2)));
                    if (!segment.isEmpty() || segments.size() > 1) {
                        if (!slash) {
                            abs.append('/');
                        }
                        abs.append(segment);
                        slash = true;
                        abs.append('/');
                    }
                } else if (*i > 1) {
                    OUString segment(uriReference->getPathSegment(*i - 2));
                    if (!segment.isEmpty() || segments.size() > 1) {
                        if (!slash) {
                            abs.append('/');
                        }
                        abs.append(segment);
                        slash = false;
                    }
                } else if (*i == 0) {
                    if (segments.size() > 1 && !slash) {
                        abs.append('/');
                    }
                } else {
                    switch (excessParentSegments) {
                    case css::uri::RelativeUriExcessParentSegments_ERROR:
                        return nullptr;

                    case css::uri::RelativeUriExcessParentSegments_RETAIN:
                        if (!slash) {
                            abs.append('/');
                        }
                        abs.append("..");
                        slash = *i < 0;
                        if (slash) {
                            abs.append('/');
                        }
                        break;

                    case css::uri::RelativeUriExcessParentSegments_REMOVE:
                        break;

                    default:
                        assert(false);
                        break;
                    }
                }
            }
        } else {
            abs.append(uriReference->getPath());
        }
        if (uriReference->hasQuery()) {
            abs.append('?');
            abs.append(uriReference->getQuery());
        }
        if (uriReference->hasFragment()) {
            abs.append('#');
            abs.append(uriReference->getFragment());
        }
        return parse(abs.makeStringAndClear());
    }
}

css::uno::Reference< css::uri::XUriReference > Factory::makeRelative(
    css::uno::Reference< css::uri::XUriReference > const & baseUriReference,
    css::uno::Reference< css::uri::XUriReference > const & uriReference,
    sal_Bool preferAuthorityOverRelativePath,
    sal_Bool preferAbsoluteOverRelativePath,
    sal_Bool encodeRetainedSpecialSegments)
    throw (css::uno::RuntimeException, std::exception)
{
    if (!baseUriReference.is() || !baseUriReference->isAbsolute()
        || !baseUriReference->isHierarchical() || !uriReference.is()) {
        return nullptr;
    } else if (!uriReference->isAbsolute() || !uriReference->isHierarchical()
               || !baseUriReference->getScheme().equalsIgnoreAsciiCase(
                   uriReference->getScheme())) {
        return clone(uriReference);
    } else {
        OUStringBuffer rel;
        bool omitQuery = false;
        if ((baseUriReference->hasAuthority() != uriReference->hasAuthority())
            || !equalIgnoreEscapeCase(
                baseUriReference->getAuthority(),
                uriReference->getAuthority()))
        {
            if (uriReference->hasAuthority()) {
                rel.append("//");
                rel.append(uriReference->getAuthority());
            }
            rel.append(uriReference->getPath());
        } else if ((equalIgnoreEscapeCase(
                        baseUriReference->getPath(), uriReference->getPath())
                    || (baseUriReference->getPath().getLength() <= 1
                        && uriReference->getPath().getLength() <= 1))
                   && baseUriReference->hasQuery() == uriReference->hasQuery()
                   && equalIgnoreEscapeCase(
                       baseUriReference->getQuery(), uriReference->getQuery()))
        {
            omitQuery = true;
        } else {
            sal_Int32 count1 = std::max< sal_Int32 >(
                baseUriReference->getPathSegmentCount(), 1);
            sal_Int32 count2 = std::max< sal_Int32 >(
                uriReference->getPathSegmentCount(), 1);
            sal_Int32 i = 0;
            for (; i < std::min(count1, count2) - 1; ++i) {
                if (!equalIgnoreEscapeCase(
                        baseUriReference->getPathSegment(i),
                        uriReference->getPathSegment(i)))
                {
                    break;
                }
            }
            if (i == 0 && preferAbsoluteOverRelativePath
                && (preferAuthorityOverRelativePath
                    || !uriReference->getPath().startsWith("//")))
            {
                if (baseUriReference->getPath().getLength() > 1
                    || uriReference->getPath().getLength() > 1)
                {
                    if (uriReference->getPath().isEmpty()) {
                        rel.append('/');
                    } else {
                        assert(uriReference->getPath()[0] == '/');
                        if (uriReference->getPath().startsWith("//")) {
                            assert(uriReference->hasAuthority());
                            rel.append("//");
                            rel.append(uriReference->getAuthority());
                        }
                        rel.append(uriReference->getPath());
                    }
                }
            } else {
                bool segments = false;
                for (sal_Int32 j = i; j < count1 - 1; ++j) {
                    if (segments) {
                        rel.append('/');
                    }
                    rel.append("..");
                    segments = true;
                }
                if (i < count2 - 1
                    || (!uriReference->getPathSegment(count2 - 1).isEmpty()))
                {
                    if (!segments
                        && (uriReference->getPathSegment(i).isEmpty()
                            || (parseScheme(uriReference->getPathSegment(i))
                                >= 0)))
                    {
                        rel.append('.');
                        segments = true;
                    }
                    for (; i < count2; ++i) {
                        if (segments) {
                            rel.append('/');
                        }
                        OUString s(uriReference->getPathSegment(i));
                        if (encodeRetainedSpecialSegments && s == ".") {
                            rel.append("%2E");
                        } else if (encodeRetainedSpecialSegments && s == "..") {
                            rel.append("%2E%2E");
                        } else {
                            rel.append(s);
                        }
                        segments = true;
                    }
                }
            }
        }
        if (!omitQuery && uriReference->hasQuery()) {
            rel.append('?');
            rel.append(uriReference->getQuery());
        }
        if (uriReference->hasFragment()) {
            rel.append('#');
            rel.append(uriReference->getFragment());
        }
        return parse(rel.makeStringAndClear());
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_uri_UriReferenceFactory_get_implementation(css::uno::XComponentContext* rxContext,
        css::uno::Sequence<css::uno::Any> const &)
{
    return ::cppu::acquire(new Factory(rxContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
