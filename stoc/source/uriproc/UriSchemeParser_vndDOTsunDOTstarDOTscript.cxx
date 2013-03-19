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

#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/XUriReference.hpp"
#include "com/sun/star/uri/XUriSchemeParser.hpp"
#include "com/sun/star/uri/XVndSunStarScriptUrlReference.hpp"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/mutex.hxx"
#include "rtl/uri.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <new>

namespace {

int getHexWeight(sal_Unicode c) {
    return c >= '0' && c <= '9' ? static_cast< int >(c - '0')
        : c >= 'A' && c <= 'F' ? static_cast< int >(c - 'A' + 10)
        : c >= 'a' && c <= 'f' ? static_cast< int >(c - 'a' + 10)
        : -1;
}

int parseEscaped(rtl::OUString const & part, sal_Int32 * index) {
    if (part.getLength() - *index < 3 || part[*index] != '%') {
        return -1;
    }
    int n1 = getHexWeight(part[*index + 1]);
    int n2 = getHexWeight(part[*index + 2]);
    if (n1 < 0 || n2 < 0) {
        return -1;
    }
    *index += 3;
    return (n1 << 4) | n2;
}

rtl::OUString parsePart(
    rtl::OUString const & part, bool namePart, sal_Int32 * index)
{
    rtl::OUStringBuffer buf;
    while (*index < part.getLength()) {
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
                if (!utf8 || encoded < min
                    || (encoded >= 0xD800 && encoded <= 0xDFFF)
                    || encoded > 0x10FFFF)
                {
                    break;
                }
                if (encoded <= 0xFFFF) {
                    buf.append(static_cast< sal_Unicode >(encoded));
                } else {
                    buf.append(static_cast< sal_Unicode >(
                        (encoded >> 10) | 0xD800));
                    buf.append(static_cast< sal_Unicode >(
                        (encoded & 0x3FF) | 0xDC00));
                }
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

namespace
{
    static rtl::OUString encodeNameOrParamFragment( rtl::OUString const & fragment )
    {
        static sal_Bool const aCharClass[] =
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* NameOrParamFragment */
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, /* !"#$%&'()*+,-./*/
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, /*0123456789:;<=>?*/
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, /*PQRSTUVWXYZ[\]^_*/
          0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
        };

        return rtl::Uri::encode(
            fragment,
            aCharClass,
            rtl_UriEncodeIgnoreEscapes,
            RTL_TEXTENCODING_UTF8
        );
    }
}

bool parseSchemeSpecificPart(rtl::OUString const & part) {
    sal_Int32 len = part.getLength();
    sal_Int32 i = 0;
    if (parsePart(part, true, &i).isEmpty() || part[0] == '/') {
        return false;
    }
    if (i == len) {
        return true;
    }
    for (;;) {
        ++i; // skip '?' or '&'
        if (parsePart(part, false, &i).isEmpty() || i == len
            || part[i] != '=')
        {
            return false;
        }
        ++i;
        parsePart(part, false, &i);
        if (i == len) {
            return true;
        }
        if (part[i] != '&') {
            return false;
        }
    }
}

class UrlReference:
    public cppu::WeakImplHelper1< css::uri::XVndSunStarScriptUrlReference >
{
public:
    UrlReference(rtl::OUString const & scheme, rtl::OUString const & path):
        m_base(
            scheme, false, false, rtl::OUString(), path, false, rtl::OUString())
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

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);

    virtual void SAL_CALL setName(rtl::OUString const & name)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);

    virtual sal_Bool SAL_CALL hasParameter(rtl::OUString const & key)
        throw (css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getParameter(rtl::OUString const & key)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setParameter(rtl::OUString const & key, rtl::OUString const & value)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);

private:
    UrlReference(UrlReference &); // not implemented
    void operator =(UrlReference); // not implemented

    virtual ~UrlReference() {}

    sal_Int32 findParameter(rtl::OUString const & key);

    stoc::uriproc::UriReference m_base;
};

rtl::OUString UrlReference::getName() throw (css::uno::RuntimeException) {
    osl::MutexGuard g(m_base.m_mutex);
    sal_Int32 i = 0;
    return parsePart(m_base.m_path, true, &i);
}

void SAL_CALL UrlReference::setName(rtl::OUString const & name) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException)
{
    if (name.isEmpty())
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString(), *this, 1);

    osl::MutexGuard g(m_base.m_mutex);
    sal_Int32 i = 0;
    parsePart(m_base.m_path, true, &i);

    rtl::OUStringBuffer newPath;
    newPath.append(encodeNameOrParamFragment(name));
    newPath.append(m_base.m_path.copy(i));
    m_base.m_path = newPath.makeStringAndClear();
}

sal_Bool UrlReference::hasParameter(rtl::OUString const & key)
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard g(m_base.m_mutex);
    return findParameter(key) >= 0;
}

rtl::OUString UrlReference::getParameter(rtl::OUString const & key)
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard g(m_base.m_mutex);
    sal_Int32 i = findParameter(key);
    return i >= 0 ? parsePart(m_base.m_path, false, &i) : rtl::OUString();
}

void UrlReference::setParameter(rtl::OUString const & key, rtl::OUString const & value)
    throw (css::uno::RuntimeException, css::lang::IllegalArgumentException)
{
    if (key.isEmpty())
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString(), *this, 1);

    osl::MutexGuard g(m_base.m_mutex);
    sal_Int32 i = findParameter(key);
    bool bExistent = ( i>=0 );
    if (!bExistent) {
        i = m_base.m_path.getLength();
    }

    rtl::OUStringBuffer newPath;
    newPath.append(m_base.m_path.copy(0, i));
    if (!bExistent) {
        newPath.append(sal_Unicode(m_base.m_path.indexOf('?') < 0 ? '?' : '&'));
        newPath.append(encodeNameOrParamFragment(key));
        newPath.append(sal_Unicode('='));
    }
    newPath.append(encodeNameOrParamFragment(value));
    if (bExistent) {
        /*oldValue = */
        parsePart(m_base.m_path, false, &i); // skip key
        newPath.append(m_base.m_path.copy(i));
    }

    m_base.m_path = newPath.makeStringAndClear();
}

sal_Int32 UrlReference::findParameter(rtl::OUString const & key) {
    sal_Int32 i = 0;
    parsePart(m_base.m_path, true, &i); // skip name
    for (;;) {
        if (i == m_base.m_path.getLength()) {
            return -1;
        }
        ++i; // skip '?' or '&'
        rtl::OUString k = parsePart(m_base.m_path, false, &i);
        ++i; // skip '='
        if (k == key) {
            return i;
        }
        parsePart(m_base.m_path, false, &i); // skip value
    }
}

class Parser: public cppu::WeakImplHelper2<
    css::lang::XServiceInfo, css::uri::XUriSchemeParser >
{
public:
    Parser() {}

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    parse(
        rtl::OUString const & scheme, rtl::OUString const & schemeSpecificPart)
        throw (css::uno::RuntimeException);

private:
    Parser(Parser &); // not implemented
    void operator =(Parser); // not implemented

    virtual ~Parser() {}
};

rtl::OUString Parser::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return stoc_services::UriSchemeParser_vndDOTsunDOTstarDOTscript::
        getImplementationName();
}

sal_Bool Parser::supportsService(rtl::OUString const & serviceName)
    throw (css::uno::RuntimeException)
{
    return stoc::uriproc::supportsService(
        getSupportedServiceNames(), serviceName);
}

css::uno::Sequence< rtl::OUString > Parser::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return stoc_services::UriSchemeParser_vndDOTsunDOTstarDOTscript::
        getSupportedServiceNames();
}

css::uno::Reference< css::uri::XUriReference >
Parser::parse(
    rtl::OUString const & scheme, rtl::OUString const & schemeSpecificPart)
    throw (css::uno::RuntimeException)
{
    if (!parseSchemeSpecificPart(schemeSpecificPart)) {
        return 0;
    }
    try {
        return new UrlReference(scheme, schemeSpecificPart);
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException(
            rtl::OUString("std::bad_alloc"), 0);
    }
}

}

namespace stoc_services {
namespace UriSchemeParser_vndDOTsunDOTstarDOTscript {

css::uno::Reference< css::uno::XInterface > create(
    SAL_UNUSED_PARAMETER css::uno::Reference< css::uno::XComponentContext >
        const &)
    SAL_THROW((css::uno::Exception))
{
    //TODO: single instance
    try {
        return static_cast< cppu::OWeakObject * >(new Parser);
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException(
            rtl::OUString("std::bad_alloc"), 0);
    }
}

rtl::OUString getImplementationName() {
    return rtl::OUString("com.sun.star.comp.uri.UriSchemeParser_vndDOTsunDOTstarDOTscript");
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    css::uno::Sequence< rtl::OUString > s(1);
    s[0] = rtl::OUString("com.sun.star.uri.UriSchemeParser_vndDOTsunDOTstarDOTscript");
    return s;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
