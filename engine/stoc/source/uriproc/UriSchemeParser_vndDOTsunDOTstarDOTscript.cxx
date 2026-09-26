/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "UriReference.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/Sequence.hxx>
#include <com/sun/star/uri/XUriSchemeParser.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrlReference.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/character.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <o3tl/safeint.hxx>
#include <o3tl/numeric.hxx>

#include <string_view>

namespace cpo::uno { class XComponentContext; }
namespace cpo::uno { class XInterface; }
namespace com::sun::star::uri { class XUriReference; }

namespace {

int parseEscaped(std::u16string_view part, sal_Int32 * index) {
    if (part.size() - *index < 3 || part[*index] != '%') {
        return -1;
    }
    int n1 = o3tl::convertToHex<int>(part[*index + 1]);
    int n2 = o3tl::convertToHex<int>(part[*index + 2]);
    if (n1 < 0 || n2 < 0) {
        return -1;
    }
    *index += 3;
    return (n1 << 4) | n2;
}

OUString parsePart(
    std::u16string_view part, bool namePart, sal_Int32 * index)
{
    OUStringBuffer buf(64);
    while (o3tl::make_unsigned(*index) < part.size()) {
        sal_Unicode c = part[*index];
        if (namePart ? c == '?' : c == '&' || c == '=') {
            break;
        } else if (c == '%') {
            sal_Int32 i = *index;
            int n = parseEscaped(part, &i);
            if (n >= 0 && n <= 0x7F) {
                buf.append(static_cast< sal_Unicode >(n));
            } else if (n >= 0xC0 && n <= 0xFC) {
                sal_Int32 encoded;
                int shift;
                sal_Int32 min;
                if (n <= 0xDF) {
                    encoded = (n & 0x1F) << 6;
                    shift = 0;
                    min = 0x80;
                } else if (n <= 0xEF) {
                    encoded = (n & 0x0F) << 12;
                    shift = 6;
                    min = 0x800;
                } else if (n <= 0xF7) {
                    encoded = (n & 0x07) << 18;
                    shift = 12;
                    min = 0x10000;
                } else if (n <= 0xFB) {
                    encoded = (n & 0x03) << 24;
                    shift = 18;
                    min = 0x200000;
                } else {
                    encoded = 0;
                    shift = 24;
                    min = 0x4000000;
                }
                bool utf8 = true;
                for (; shift >= 0; shift -= 6) {
                    n = parseEscaped(part, &i);
                    if (n < 0x80 || n > 0xBF) {
                        utf8 = false;
                        break;
                    }
                    encoded |= (n & 0x3F) << shift;
                }
                if (!utf8 || !rtl::isUnicodeScalarValue(encoded)
                    || encoded < min)
                {
                    break;
                }
                buf.appendUtf32(encoded);
            } else {
                break;
            }
            *index = i;
        } else {
            buf.append(c);
            ++*index;
        }
    }
    return buf.makeStringAndClear();
}

OUString encodeNameOrParamFragment(OUString const & fragment) {
    static constexpr auto nameOrParamFragment = rtl::createUriCharClass(
        u8"!$'()*+,-.0123456789:;@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]_abcdefghijklmnopqrstuvwxyz~");
    return rtl::Uri::encode(
        fragment, nameOrParamFragment.data(), rtl_UriEncodeIgnoreEscapes,
        RTL_TEXTENCODING_UTF8);
}

bool parseSchemeSpecificPart(std::u16string_view part) {
    size_t len = part.size();
    sal_Int32 i = 0;
    if (parsePart(part, true, &i).isEmpty() || part[0] == '/') {
        return false;
    }
    if (o3tl::make_unsigned(i) == len) {
        return true;
    }
    for (;;) {
        ++i; // skip '?' or '&'
        if (parsePart(part, false, &i).isEmpty() || o3tl::make_unsigned(i) == len
            || part[i] != '=')
        {
            return false;
        }
        ++i;
        parsePart(part, false, &i);
        if (o3tl::make_unsigned(i) == len) {
            return true;
        }
        if (part[i] != '&') {
            return false;
        }
    }
}

class UrlReference:
    public cppu::WeakImplHelper<css::uri::XVndSunStarScriptUrlReference>
{
public:
    UrlReference(OUString const & scheme, OUString const & path):
        m_base(
            scheme, false, OUString(), path, false, OUString())
    {}

    UrlReference(const UrlReference&) = delete;
    UrlReference& operator=(const UrlReference&) = delete;

    virtual OUString getUriReference() override
    { return m_base.getUriReference(); }

    virtual bool isAbsolute() override
    { return m_base.isAbsolute(); }

    virtual OUString getScheme() override
    { return m_base.getScheme(); }

    virtual OUString getSchemeSpecificPart() override
    { return m_base.getSchemeSpecificPart(); }

    virtual bool isHierarchical() override
    { return m_base.isHierarchical(); }

    virtual bool hasAuthority() override
    { return m_base.hasAuthority(); }

    virtual OUString getAuthority() override
    { return m_base.getAuthority(); }

    virtual OUString getPath() override
    { return m_base.getPath(); }

    virtual bool hasRelativePath() override
    { return m_base.hasRelativePath(); }

    virtual sal_Int32 getPathSegmentCount() override
    { return m_base.getPathSegmentCount(); }

    virtual OUString getPathSegment(sal_Int32 index) override
    { return m_base.getPathSegment(index); }

    virtual bool hasQuery() override
    { return m_base.hasQuery(); }

    virtual OUString getQuery() override
    { return m_base.getQuery(); }

    virtual bool hasFragment() override
    { return m_base.hasFragment(); }

    virtual OUString getFragment() override
    { return m_base.getFragment(); }

    virtual void setFragment(OUString const & fragment) override
    { m_base.setFragment(fragment); }

    virtual void clearFragment() override
    { m_base.clearFragment(); }

    virtual OUString getName() override;

    virtual void setName(OUString const & name) override;

    virtual bool hasParameter(OUString const & key) override;

    virtual OUString getParameter(OUString const & key) override;

    virtual void setParameter(OUString const & key, OUString const & value) override;

private:
    virtual ~UrlReference() override {}

    sal_Int32 findParameter(std::u16string_view key) const;

    stoc::uriproc::UriReference m_base;
};

OUString UrlReference::getName() {
    std::lock_guard g(m_base.m_mutex);
    sal_Int32 i = 0;
    return parsePart(m_base.m_path, true, &i);
}

void UrlReference::setName(OUString const & name)
{
    if (name.isEmpty())
        throw css::lang::IllegalArgumentException(
            OUString(), *this, 1);

    std::lock_guard g(m_base.m_mutex);
    sal_Int32 i = 0;
    parsePart(m_base.m_path, true, &i);

    m_base.m_path = encodeNameOrParamFragment(name) + m_base.m_path.subView(i);
}

bool UrlReference::hasParameter(OUString const & key)
{
    std::lock_guard g(m_base.m_mutex);
    return findParameter(key) >= 0;
}

OUString UrlReference::getParameter(OUString const & key)
{
    std::lock_guard g(m_base.m_mutex);
    sal_Int32 i = findParameter(key);
    return i >= 0 ? parsePart(m_base.m_path, false, &i) : OUString();
}

void UrlReference::setParameter(OUString const & key, OUString const & value)
{
    if (key.isEmpty())
        throw css::lang::IllegalArgumentException(
            OUString(), *this, 1);

    std::lock_guard g(m_base.m_mutex);
    sal_Int32 i = findParameter(key);
    bool bExistent = ( i>=0 );
    if (!bExistent) {
        i = m_base.m_path.getLength();
    }

    OUStringBuffer newPath(128);
    newPath.append(m_base.m_path.subView(0, i));
    if (!bExistent) {
        newPath.append( m_base.m_path.indexOf('?') < 0 ? '?' : '&' );
        newPath.append(encodeNameOrParamFragment(key) + "=");
    }
    newPath.append(encodeNameOrParamFragment(value));
    if (bExistent) {
        /*oldValue = */
        parsePart(m_base.m_path, false, &i); // skip key
        newPath.append(m_base.m_path.subView(i));
    }

    m_base.m_path = newPath.makeStringAndClear();
}

sal_Int32 UrlReference::findParameter(std::u16string_view key) const {
    sal_Int32 i = 0;
    parsePart(m_base.m_path, true, &i); // skip name
    for (;;) {
        if (i == m_base.m_path.getLength()) {
            return -1;
        }
        ++i; // skip '?' or '&'
        OUString k = parsePart(m_base.m_path, false, &i);
        ++i; // skip '='
        if (k == key) {
            return i;
        }
        parsePart(m_base.m_path, false, &i); // skip value
    }
}

class Parser:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo, css::uri::XUriSchemeParser>
{
public:
    Parser() {}

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    virtual OUString getImplementationName() override;

    virtual bool supportsService(OUString const & serviceName) override;

    virtual cpo::uno::Sequence< OUString >
    getSupportedServiceNames() override;

    virtual cpo::uno::Reference< css::uri::XUriReference >
    parse(
        OUString const & scheme, OUString const & schemeSpecificPart) override;

private:
    virtual ~Parser() override {}
};

OUString Parser::getImplementationName()
{
    return u"com.sun.star.comp.uri.UriSchemeParser_vndDOTsunDOTstarDOTscript"_ustr;
}

bool Parser::supportsService(OUString const & serviceName)
{
    return cppu::supportsService(this, serviceName);
}

cpo::uno::Sequence< OUString > Parser::getSupportedServiceNames()
{
    return { u"com.sun.star.uri.UriSchemeParser_vndDOTsunDOTstarDOTscript"_ustr };
}

cpo::uno::Reference< css::uri::XUriReference >
Parser::parse(
    OUString const & scheme, OUString const & schemeSpecificPart)
{
    if (!parseSchemeSpecificPart(schemeSpecificPart)) {
        return nullptr;
    }
    return new UrlReference(scheme, schemeSpecificPart);
}

}

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
com_sun_star_comp_uri_UriSchemeParser_vndDOTsunDOTstarDOTscript_get_implementation(cpo::uno::XComponentContext*,
        cpo::uno::Sequence<cpo::uno::Any> const &)
{
    //TODO: single instance
    return ::cppu::acquire(new Parser());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
