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

#include "UriReference.hxx"
#include "supportsService.hxx"

#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/RelativeUriExcessParentSegments.hpp"
#include "com/sun/star/uri/XUriReference.hpp"
#include "com/sun/star/uri/XUriReferenceFactory.hpp"
#include "com/sun/star/uri/XUriSchemeParser.hpp"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <algorithm>
#include /*MSVC trouble: <cstdlib>*/ <stdlib.h>
#include <new>
#include <vector>

namespace {

//TODO: move comphelper::string::misc into something like
//sal/salhelper and use those instead

bool isDigit(sal_Unicode c) {
    return c >= '0' && c <= '9';
}

bool isUpperCase(sal_Unicode c) {
    return c >= 'A' && c <= 'Z';
}

bool isLowerCase(sal_Unicode c) {
    return c >= 'a' && c <= 'z';
}

bool isAlpha(sal_Unicode c) {
    return isUpperCase(c) || isLowerCase(c);
}

bool isHexDigit(sal_Unicode c) {
    return isDigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

sal_Unicode toLowerCase(sal_Unicode c) {
    return isUpperCase(c) ? c + ('a' - 'A') : c;
}

bool equalIgnoreCase(sal_Unicode c1, sal_Unicode c2) {
    return toLowerCase(c1) == toLowerCase(c2);
}

bool equalIgnoreEscapeCase(rtl::OUString const & s1, rtl::OUString const & s2) {
    if (s1.getLength() == s2.getLength()) {
        for (sal_Int32 i = 0; i < s1.getLength();) {
            if (s1[i] == '%' && s2[i] == '%' && s1.getLength() - i > 2
                && isHexDigit(s1[i + 1]) && isHexDigit(s1[i + 2])
                && isHexDigit(s2[i + 1]) && isHexDigit(s2[i + 2])
                && equalIgnoreCase(s1[i + 1], s2[i + 1])
                && equalIgnoreCase(s1[i + 2], s2[i + 2]))
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

sal_Int32 parseScheme(rtl::OUString const & uriReference) {
    if (uriReference.getLength() >= 2 && isAlpha(uriReference[0])) {
        for (sal_Int32 i = 0; i < uriReference.getLength(); ++i) {
            sal_Unicode c = uriReference[i];
            if (c == ':') {
                return i;
            } else if (!isAlpha(c) && !isDigit(c) && c != '+' && c != '-'
                       && c != '.')
            {
                break;
            }
        }
    }
    return -1;
}

class UriReference: public cppu::WeakImplHelper1< css::uri::XUriReference > {
public:
    UriReference(
        rtl::OUString const & scheme, bool bIsHierarchical, bool bHasAuthority,
        rtl::OUString const & authority, rtl::OUString const & path,
        bool bHasQuery, rtl::OUString const & query):
        m_base(
            scheme, bIsHierarchical, bHasAuthority, authority, path, bHasQuery,
            query)
    {}

    virtual rtl::OUString SAL_CALL getUriReference()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.getUriReference(); }

    virtual sal_Bool SAL_CALL isAbsolute()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.isAbsolute(); }

    virtual rtl::OUString SAL_CALL getScheme()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.getScheme(); }

    virtual rtl::OUString SAL_CALL getSchemeSpecificPart()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.getSchemeSpecificPart(); }

    virtual sal_Bool SAL_CALL isHierarchical()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.isHierarchical(); }

    virtual sal_Bool SAL_CALL hasAuthority()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.hasAuthority(); }

    virtual rtl::OUString SAL_CALL getAuthority()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.getAuthority(); }

    virtual rtl::OUString SAL_CALL getPath()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.getPath(); }

    virtual sal_Bool SAL_CALL hasRelativePath()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.hasRelativePath(); }

    virtual sal_Int32 SAL_CALL getPathSegmentCount()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.getPathSegmentCount(); }

    virtual rtl::OUString SAL_CALL getPathSegment(sal_Int32 index)
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.getPathSegment(index); }

    virtual sal_Bool SAL_CALL hasQuery()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.hasQuery(); }

    virtual rtl::OUString SAL_CALL getQuery()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.getQuery(); }

    virtual sal_Bool SAL_CALL hasFragment()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.hasFragment(); }

    virtual rtl::OUString SAL_CALL getFragment()
        throw (com::sun::star::uno::RuntimeException)
    { return m_base.getFragment(); }

    virtual void SAL_CALL setFragment(rtl::OUString const & fragment)
        throw (com::sun::star::uno::RuntimeException)
    { m_base.setFragment(fragment); }

    virtual void SAL_CALL clearFragment()
        throw (com::sun::star::uno::RuntimeException)
    { m_base.clearFragment(); }

private:
    UriReference(UriReference &); // not implemented
    void operator =(UriReference); // not implemented

    virtual ~UriReference() {}

    stoc::uriproc::UriReference m_base;
};

// throws std::bad_alloc
css::uno::Reference< css::uri::XUriReference > parseGeneric(
    rtl::OUString const & scheme, rtl::OUString const & schemeSpecificPart)
{
    bool isAbsolute = !scheme.isEmpty();
    bool isHierarchical
        = !isAbsolute
        || (!schemeSpecificPart.isEmpty() && schemeSpecificPart[0] == '/');
    bool hasAuthority = false;
    rtl::OUString authority;
    rtl::OUString path;
    bool hasQuery = false;
    rtl::OUString query;
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
            return 0;
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
    OSL_ASSERT(count <= SAL_MAX_INT32 - 1 && -count >= SAL_MIN_INT32 + 1);
    for (sal_Int32 i = 0; i < count; ++i) {
        if (processSpecialSegments) {
            rtl::OUString segment(uriReference->getPathSegment(i));
            if ( segment == "." ) {
                if (!base && i == count - 1) {
                    segments.push_back(0);
                }
                continue;
            } else if ( segment == ".." ) {
                if (segments.empty()
                    || /*MSVC trouble: std::*/abs(segments.back()) == 1)
                {
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

class Factory: public cppu::WeakImplHelper2<
    css::lang::XServiceInfo, css::uri::XUriReferenceFactory >
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
    parse(rtl::OUString const & uriReference)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    makeAbsolute(
        css::uno::Reference< css::uri::XUriReference > const & baseUriReference,
        css::uno::Reference< css::uri::XUriReference > const & uriReference,
        sal_Bool processSpecialBaseSegments,
        css::uri::RelativeUriExcessParentSegments excessParentSegments)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    makeRelative(
        css::uno::Reference< css::uri::XUriReference > const & baseUriReference,
        css::uno::Reference< css::uri::XUriReference > const & uriReference,
        sal_Bool preferAuthorityOverRelativePath,
        sal_Bool preferAbsoluteOverRelativePath,
        sal_Bool encodeRetainedSpecialSegments)
        throw (css::uno::RuntimeException);

private:
    Factory(Factory &); // not implemented
    void operator =(Factory); // not implemented

    virtual ~Factory() {}

    css::uno::Reference< css::uri::XUriReference > clone(
        css::uno::Reference< css::uri::XUriReference > const & uriReference)
    { return parse(uriReference->getUriReference()); }

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

rtl::OUString Factory::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return stoc_services::UriReferenceFactory::getImplementationName();
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
    return stoc_services::UriReferenceFactory::getSupportedServiceNames();
}

css::uno::Reference< css::uri::XUriReference > Factory::parse(
    rtl::OUString const & uriReference) throw (css::uno::RuntimeException)
{
    sal_Int32 fragment = uriReference.indexOf('#');
    if (fragment == -1) {
        fragment = uriReference.getLength();
    }
    rtl::OUString scheme;
    rtl::OUString schemeSpecificPart;
    rtl::OUString serviceName;
    sal_Int32 n = parseScheme(uriReference);
    OSL_ASSERT(n < fragment);
    if (n >= 0) {
        scheme = uriReference.copy(0, n);
        schemeSpecificPart = uriReference.copy(n + 1, fragment - (n + 1));
        rtl::OUStringBuffer buf;
        buf.appendAscii(
            RTL_CONSTASCII_STRINGPARAM("com.sun.star.uri.UriSchemeParser_"));
        for (sal_Int32 i = 0; i < scheme.getLength(); ++i) {
            sal_Unicode c = scheme[i];
            if (isUpperCase(c)) {
                buf.append(toLowerCase(c));
            } else if (c == '+') {
                buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("PLUS"));
            } else if (c == '-') {
                buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("HYPHEN"));
            } else if (c == '.') {
                buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("DOT"));
            } else {
                OSL_ASSERT(isLowerCase(c) || isDigit(c));
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
                    rtl::OUString("creating service ")
                        + serviceName,
                    static_cast< cppu::OWeakObject * >(this),
                    css::uno::makeAny(e)); //TODO: preserve type of e
            }
            if (service.is()) {
                parser = css::uno::Reference< css::uri::XUriSchemeParser >(
                    service, css::uno::UNO_QUERY_THROW);
            }
        }
    }
    css::uno::Reference< css::uri::XUriReference > uriRef;
    if (parser.is()) {
        uriRef = parser->parse(scheme, schemeSpecificPart);
    } else {
        try {
            uriRef = parseGeneric(scheme, schemeSpecificPart);
        } catch (std::bad_alloc &) {
            throw css::uno::RuntimeException(
                rtl::OUString("std::bad_alloc"),
                static_cast< cppu::OWeakObject * >(this));
        }
    }
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
    throw (css::uno::RuntimeException)
{
    if (!baseUriReference.is() || !baseUriReference->isAbsolute()
        || !baseUriReference->isHierarchical() || !uriReference.is()) {
        return 0;
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
        rtl::OUStringBuffer abs(baseUriReference->getScheme());
        abs.append(static_cast< sal_Unicode >(':'));
        if (uriReference->hasAuthority()) {
            abs.appendAscii(RTL_CONSTASCII_STRINGPARAM("//"));
            abs.append(uriReference->getAuthority());
        } else if (baseUriReference->hasAuthority()) {
            abs.appendAscii(RTL_CONSTASCII_STRINGPARAM("//"));
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
                abs.append(static_cast< sal_Unicode >('/'));
            }
            for (Segments::iterator i(segments.begin()); i != segments.end();
                 ++i)
            {
                if (*i < -1) {
                    rtl::OUString segment(
                        baseUriReference->getPathSegment(-(*i + 2)));
                    if (!segment.isEmpty() || segments.size() > 1) {
                        if (!slash) {
                            abs.append(static_cast< sal_Unicode >('/'));
                        }
                        abs.append(segment);
                        slash = true;
                        abs.append(static_cast< sal_Unicode >('/'));
                    }
                } else if (*i > 1) {
                    rtl::OUString segment(uriReference->getPathSegment(*i - 2));
                    if (!segment.isEmpty() || segments.size() > 1) {
                        if (!slash) {
                            abs.append(static_cast< sal_Unicode >('/'));
                        }
                        abs.append(segment);
                        slash = false;
                    }
                } else if (*i == 0) {
                    if (segments.size() > 1 && !slash) {
                        abs.append(static_cast< sal_Unicode >('/'));
                    }
                } else {
                    switch (excessParentSegments) {
                    case css::uri::RelativeUriExcessParentSegments_ERROR:
                        return 0;

                    case css::uri::RelativeUriExcessParentSegments_RETAIN:
                        if (!slash) {
                            abs.append(static_cast< sal_Unicode >('/'));
                        }
                        abs.appendAscii(RTL_CONSTASCII_STRINGPARAM(".."));
                        slash = *i < 0;
                        if (slash) {
                            abs.append(static_cast< sal_Unicode >('/'));
                        }
                        break;

                    case css::uri::RelativeUriExcessParentSegments_REMOVE:
                        break;

                    default:
                        OSL_ASSERT(false);
                        break;
                    }
                }
            }
        } else {
            abs.append(uriReference->getPath());
        }
        if (uriReference->hasQuery()) {
            abs.append(static_cast< sal_Unicode >('?'));
            abs.append(uriReference->getQuery());
        }
        if (uriReference->hasFragment()) {
            abs.append(static_cast< sal_Unicode >('#'));
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
    throw (css::uno::RuntimeException)
{
    if (!baseUriReference.is() || !baseUriReference->isAbsolute()
        || !baseUriReference->isHierarchical() || !uriReference.is()) {
        return 0;
    } else if (!uriReference->isAbsolute() || !uriReference->isHierarchical()
               || !baseUriReference->getScheme().equalsIgnoreAsciiCase(
                   uriReference->getScheme())) {
        return clone(uriReference);
    } else {
        rtl::OUStringBuffer rel;
        bool omitQuery = false;
        if ((baseUriReference->hasAuthority() != uriReference->hasAuthority())
            || !equalIgnoreEscapeCase(
                baseUriReference->getAuthority(),
                uriReference->getAuthority()))
        {
            if (uriReference->hasAuthority()) {
                rel.appendAscii(RTL_CONSTASCII_STRINGPARAM("//"));
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
                    || !uriReference->getPath().matchAsciiL(
                        RTL_CONSTASCII_STRINGPARAM("//"))))
            {
                if (baseUriReference->getPath().getLength() > 1
                    || uriReference->getPath().getLength() > 1)
                {
                    if (uriReference->getPath().isEmpty()) {
                        rel.append(static_cast< sal_Unicode >('/'));
                    } else {
                        OSL_ASSERT(uriReference->getPath()[0] == '/');
                        if (uriReference->getPath().matchAsciiL(
                                RTL_CONSTASCII_STRINGPARAM("//"))) {
                            OSL_ASSERT(uriReference->hasAuthority());
                            rel.appendAscii(RTL_CONSTASCII_STRINGPARAM("//"));
                            rel.append(uriReference->getAuthority());
                        }
                        rel.append(uriReference->getPath());
                    }
                }
            } else {
                bool segments = false;
                for (sal_Int32 j = i; j < count1 - 1; ++j) {
                    if (segments) {
                        rel.append(static_cast< sal_Unicode >('/'));
                    }
                    rel.appendAscii(RTL_CONSTASCII_STRINGPARAM(".."));
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
                        rel.append(static_cast< sal_Unicode >('.'));
                        segments = true;
                    }
                    for (; i < count2; ++i) {
                        if (segments) {
                            rel.append(static_cast< sal_Unicode >('/'));
                        }
                        rtl::OUString s(uriReference->getPathSegment(i));
                        if (encodeRetainedSpecialSegments
                            && s.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(".")))
                        {
                            rel.appendAscii(RTL_CONSTASCII_STRINGPARAM("%2E"));
                        } else if (encodeRetainedSpecialSegments
                                   && s.equalsAsciiL(
                                       RTL_CONSTASCII_STRINGPARAM("..")))
                        {
                            rel.appendAscii(
                                RTL_CONSTASCII_STRINGPARAM("%2E%2E"));
                        } else {
                            rel.append(s);
                        }
                        segments = true;
                    }
                }
            }
        }
        if (!omitQuery && uriReference->hasQuery()) {
            rel.append(static_cast< sal_Unicode >('?'));
            rel.append(uriReference->getQuery());
        }
        if (uriReference->hasFragment()) {
            rel.append(static_cast< sal_Unicode >('#'));
            rel.append(uriReference->getFragment());
        }
        return parse(rel.makeStringAndClear());
    }
}

}

namespace stoc_services { namespace UriReferenceFactory {

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
    return rtl::OUString("com.sun.star.comp.uri.UriReferenceFactory");
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    css::uno::Sequence< rtl::OUString > s(1);
    s[0] = rtl::OUString("com.sun.star.uri.UriReferenceFactory");
    return s;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
