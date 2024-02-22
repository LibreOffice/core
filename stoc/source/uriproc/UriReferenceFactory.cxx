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
#include <cstddef>
#include <string_view>
#include <utility>
#include <vector>

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
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "UriReference.hxx"

namespace {

bool equalIgnoreEscapeCase(std::u16string_view s1, std::u16string_view s2) {
    if (s1.size() == s2.size()) {
        for (size_t i = 0; i < s1.size();) {
            if (s1[i] == '%' && s2[i] == '%' && s1.size() - i > 2
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

sal_Int32 parseScheme(std::u16string_view uriReference) {
    if (uriReference.size() >= 2 && rtl::isAsciiAlpha(uriReference[0])) {
        for (size_t i = 0; i < uriReference.size(); ++i) {
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
    public cppu::WeakImplHelper<css::uri::XUriReference>
{
public:
    UriReference(
        OUString const & scheme, bool bHasAuthority,
        OUString const & authority, OUString const & path,
        bool bHasQuery, OUString const & query):
        m_base(
            scheme, bHasAuthority, authority, path, bHasQuery,
            query)
    {}

    UriReference(const UriReference&) = delete;
    UriReference& operator=(const UriReference&) = delete;

    virtual OUString SAL_CALL getUriReference() override
    { return m_base.getUriReference(); }

    virtual sal_Bool SAL_CALL isAbsolute() override
    { return m_base.isAbsolute(); }

    virtual OUString SAL_CALL getScheme() override
    { return m_base.getScheme(); }

    virtual OUString SAL_CALL getSchemeSpecificPart() override
    { return m_base.getSchemeSpecificPart(); }

    virtual sal_Bool SAL_CALL isHierarchical() override
    { return m_base.isHierarchical(); }

    virtual sal_Bool SAL_CALL hasAuthority() override
    { return m_base.hasAuthority(); }

    virtual OUString SAL_CALL getAuthority() override
    { return m_base.getAuthority(); }

    virtual OUString SAL_CALL getPath() override
    { return m_base.getPath(); }

    virtual sal_Bool SAL_CALL hasRelativePath() override
    { return m_base.hasRelativePath(); }

    virtual sal_Int32 SAL_CALL getPathSegmentCount() override
    { return m_base.getPathSegmentCount(); }

    virtual OUString SAL_CALL getPathSegment(sal_Int32 index) override
    { return m_base.getPathSegment(index); }

    virtual sal_Bool SAL_CALL hasQuery() override
    { return m_base.hasQuery(); }

    virtual OUString SAL_CALL getQuery() override
    { return m_base.getQuery(); }

    virtual sal_Bool SAL_CALL hasFragment() override
    { return m_base.hasFragment(); }

    virtual OUString SAL_CALL getFragment() override
    { return m_base.getFragment(); }

    virtual void SAL_CALL setFragment(OUString const & fragment) override
    { m_base.setFragment(fragment); }

    virtual void SAL_CALL clearFragment() override
    { m_base.clearFragment(); }

private:
    virtual ~UriReference() override {}

    stoc::uriproc::UriReference m_base;
};

css::uno::Reference< css::uri::XUriReference > parseGeneric(
    OUString const & scheme, std::u16string_view schemeSpecificPart)
{
    size_t len = schemeSpecificPart.size();
    size_t i = 0;
    bool hasAuthority = false;
    OUString authority;
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
        authority = schemeSpecificPart.substr(n, i - n);
    }
    sal_Int32 n = i;
    i = schemeSpecificPart.find('?', i);
    if (i == std::u16string_view::npos) {
        i = len;
    }
    OUString path( schemeSpecificPart.substr(n, i - n) );
    bool hasQuery = false;
    OUString query;
    if (i != len) {
        hasQuery = true;
        query = schemeSpecificPart.substr(i + 1);
    }
    return new UriReference(
        scheme, hasAuthority, authority, path, hasQuery, query);
}

struct Segment {
    bool leadingSlash;
    bool excessParent;
    std::u16string_view segment;

    Segment(bool theLeadingSlash, bool theExcessParent, std::u16string_view theSegment):
        leadingSlash(theLeadingSlash), excessParent(theExcessParent), segment(theSegment) {}
};

std::pair<std::vector<Segment>, bool> processSegments(
    std::u16string_view first, std::u16string_view second, bool processSpecialSegments)
{
    std::vector<Segment> segments;
    bool processed = false;
    std::u16string_view const * half = &first;
        // later checks for `half == &first` and `half == &second` rely on the fact that `first` and
        // `second` are passed by value, in case a caller passes the same object for both arguments
    std::size_t index = 0;
    bool slash = false;
    if (index == half->length()) {
        half = &second;
        index = 0;
    }
    if (index != half->length()) {
        if ((*half)[index] == u'/') {
            slash = true;
            ++index;
        }
        for (;;) {
            if (index == half->length() && half == &first) {
                half = &second;
                index = 0;
            }
            if (index == half->length()) {
                if (slash) {
                    segments.emplace_back(true, false, std::u16string_view());
                }
                break;
            }
            auto const n = std::min(half->find(u'/', index), half->length());
            auto const leadingSlash = slash;
            auto const segment = half->substr(index, n - index);
            auto const process = processSpecialSegments || half == &second;
            index = n;
            slash = false;
            if (index == half->length() && half == &first) {
                half = &second;
                index = 0;
            }
            if (index != half->length() && (*half)[index] == u'/') {
                slash = true;
                ++index;
            }
            if (process) {
                if (segment == u".") {
                    slash = leadingSlash;
                    processed = true;
                    continue;
                } else if (segment == u"..") {
                    if (segments.empty() || segments.back().excessParent) {
                        segments.emplace_back(leadingSlash, true, segment);
                    } else {
                        if (leadingSlash) {
                            segments.pop_back();
                        }
                        slash = leadingSlash;
                    }
                    processed = true;
                    continue;
                }
            }
            segments.emplace_back(leadingSlash, false, segment);
        }
    }
    return {segments, processed};
}

class Factory:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo, css::uri::XUriReferenceFactory>
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
    parse(OUString const & uriReference) override;

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    makeAbsolute(
        css::uno::Reference< css::uri::XUriReference > const & baseUriReference,
        css::uno::Reference< css::uri::XUriReference > const & uriReference,
        sal_Bool processAdditionalSpecialSegments,
        css::uri::RelativeUriExcessParentSegments excessParentSegments) override;

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    makeRelative(
        css::uno::Reference< css::uri::XUriReference > const & baseUriReference,
        css::uno::Reference< css::uri::XUriReference > const & uriReference,
        sal_Bool preferAuthorityOverRelativePath,
        sal_Bool preferAbsoluteOverRelativePath,
        sal_Bool encodeRetainedSpecialSegments) override;

private:
    virtual ~Factory() override {}

    css::uno::Reference< css::uri::XUriReference > clone(
        css::uno::Reference< css::uri::XUriReference > const & uriReference)
    { return parse(uriReference->getUriReference()); }

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

OUString Factory::getImplementationName()
{
    return "com.sun.star.comp.uri.UriReferenceFactory";
}

sal_Bool Factory::supportsService(OUString const & serviceName)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > Factory::getSupportedServiceNames()
{
    css::uno::Sequence< OUString > s { "com.sun.star.uri.UriReferenceFactory" };
    return s;
}

css::uno::Reference< css::uri::XUriReference > Factory::parse(
    OUString const & uriReference)
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
        OUStringBuffer buf(128);
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
            } catch (const css::uno::Exception &) {
                css::uno::Any anyEx = cppu::getCaughtException();
                throw css::lang::WrappedTargetRuntimeException(
                    "creating service " + serviceName,
                    getXWeak(),
                    anyEx);
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
    sal_Bool processAdditionalSpecialSegments,
    css::uri::RelativeUriExcessParentSegments excessParentSegments)
{
    if (!baseUriReference.is() || !baseUriReference->isAbsolute()
        || !uriReference.is()) {
        return nullptr;
    } else if (uriReference->isAbsolute()) {
        if (processAdditionalSpecialSegments) {
            auto const path = uriReference->getPath();
            auto [segments, proc] = processSegments(path, {}, true);
            if (proc) {
                OUStringBuffer abs(uriReference->getScheme() + ":");
                if (uriReference->hasAuthority()) {
                    abs.append("//" + uriReference->getAuthority());
                }
                for (auto const & i : segments)
                {
                    if (i.excessParent) {
                        switch (excessParentSegments) {
                        case css::uri::RelativeUriExcessParentSegments_ERROR:
                            return nullptr;

                        case css::uri::RelativeUriExcessParentSegments_RETAIN:
                            assert(i.segment == u"..");
                            break;

                        case css::uri::RelativeUriExcessParentSegments_REMOVE:
                            continue;

                        default:
                            assert(false);
                            break;
                        }
                    }
                    if (i.leadingSlash) {
                        abs.append('/');
                    }
                    abs.append(i.segment);
                }
                if (uriReference->hasQuery()) {
                    abs.append("?" + uriReference->getQuery());
                }
                if (uriReference->hasFragment()) {
                    abs.append("#" + uriReference->getFragment());
                }
                return parse(abs.makeStringAndClear());
            }
        }
        return clone(uriReference);
    } else if (!uriReference->hasAuthority()
               && uriReference->getPath().isEmpty()) {
        OUStringBuffer abs(baseUriReference->getScheme() + ":");
        if (baseUriReference->hasAuthority()) {
            abs.append("//" + baseUriReference->getAuthority());
        }
        abs.append(baseUriReference->getPath());
        if (uriReference->hasQuery()) {
            abs.append("?" + uriReference->getQuery());
        } else if (baseUriReference->hasQuery()) {
            abs.append("?" + baseUriReference->getQuery());
        }
        if (uriReference->hasFragment()) {
            abs.append("#" + uriReference->getFragment());
        }
        return parse(abs.makeStringAndClear());
    } else {
        OUStringBuffer abs(128);
        abs.append(baseUriReference->getScheme() + ":");
        if (uriReference->hasAuthority()) {
            abs.append("//" + uriReference->getAuthority());
        } else if (baseUriReference->hasAuthority()) {
            abs.append("//" + baseUriReference->getAuthority());
        }
        if (uriReference->hasRelativePath()) {
            auto path1 = baseUriReference->getPath();
            if (path1.isEmpty()) {
                if (baseUriReference->hasAuthority()) {
                    path1 = "/";
                }
            } else {
                path1 = path1.copy(0, path1.lastIndexOf('/') + 1);
            }
            auto const path2 = uriReference->getPath();
            auto [segments, _] = processSegments(path1, path2, processAdditionalSpecialSegments);
            (void)_;
            for (auto const & i : segments)
            {
                if (i.excessParent) {
                    switch (excessParentSegments) {
                    case css::uri::RelativeUriExcessParentSegments_ERROR:
                        return nullptr;

                    case css::uri::RelativeUriExcessParentSegments_RETAIN:
                        assert(i.segment == u"..");
                        break;

                    case css::uri::RelativeUriExcessParentSegments_REMOVE:
                        continue;

                    default:
                        assert(false);
                        break;
                    }
                }
                if (i.leadingSlash) {
                    abs.append('/');
                }
                abs.append(i.segment);
            }
        } else {
            bool processed = false;
            if (processAdditionalSpecialSegments) {
                auto const path = uriReference->getPath();
                auto [segments, proc] = processSegments(path, {}, true);
                if (proc) {
                    for (auto const & i : segments)
                    {
                        if (i.excessParent) {
                            switch (excessParentSegments) {
                            case css::uri::RelativeUriExcessParentSegments_ERROR:
                                return nullptr;

                            case css::uri::RelativeUriExcessParentSegments_RETAIN:
                                assert(i.segment == u"..");
                                break;

                            case css::uri::RelativeUriExcessParentSegments_REMOVE:
                                continue;

                            default:
                                assert(false);
                                break;
                            }
                        }
                        if (i.leadingSlash) {
                            abs.append('/');
                        }
                        abs.append(i.segment);
                    }
                    processed = true;
                }
            }
            if (!processed) {
                abs.append(uriReference->getPath());
            }
        }
        if (uriReference->hasQuery()) {
            abs.append("?" + uriReference->getQuery());
        }
        if (uriReference->hasFragment()) {
            abs.append("#" + uriReference->getFragment());
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
{
    if (!baseUriReference.is() || !baseUriReference->isAbsolute()
        || !uriReference.is()) {
        return nullptr;
    } else if (!uriReference->isAbsolute() || uriReference->hasRelativePath()
               || !baseUriReference->getScheme().equalsIgnoreAsciiCase(
                   uriReference->getScheme())) {
        return clone(uriReference);
    } else {
        OUStringBuffer rel(128);
        bool omitQuery = false;
        if ((baseUriReference->hasAuthority() != uriReference->hasAuthority())
            || !equalIgnoreEscapeCase(
                baseUriReference->getAuthority(),
                uriReference->getAuthority()))
        {
            if (uriReference->hasAuthority()) {
                rel.append("//" + uriReference->getAuthority());
            }
            rel.append(uriReference->getPath());
        } else if ((equalIgnoreEscapeCase(
                        baseUriReference->getPath(), uriReference->getPath())
                    || (baseUriReference->getPath() == "/"
                        && uriReference->getPath().isEmpty()))
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
            if (i == 0
                && (preferAbsoluteOverRelativePath || uriReference->hasQuery())
                && (preferAuthorityOverRelativePath
                    || !uriReference->getPath().startsWith("//")))
            {
                if (uriReference->getPath().isEmpty()) {
                    if (!baseUriReference->getPath().isEmpty()
                        && baseUriReference->getPath() != "/")
                    {
                        rel.append('/');
                    }
                } else if (uriReference->getPath() == "/") {
                    if (baseUriReference->getPath().isEmpty()
                        || baseUriReference->getPath() != "/")
                    {
                        rel.append('/');
                    }
                } else {
                    if (uriReference->getPath().startsWith("//")) {
                        assert(uriReference->hasAuthority());
                        rel.append("//" + uriReference->getAuthority());
                    }
                    rel.append(uriReference->getPath());
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
            rel.append("?" + uriReference->getQuery());
        }
        if (uriReference->hasFragment()) {
            rel.append("#" + uriReference->getFragment());
        }
        return parse(rel.makeStringAndClear());
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_uri_UriReferenceFactory_get_implementation(css::uno::XComponentContext* rxContext,
        css::uno::Sequence<css::uno::Any> const &)
{
    return ::cppu::acquire(new Factory(rxContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
