/*************************************************************************
 *
 *  $RCSfile: UriSchemeParser_vndDOTsunDOTstarDOTscript.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-01-19 18:29:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "UriSchemeParser_vndDOTsunDOTstarDOTscript.hxx"

#include "UriReference.hxx"
#include "supportsService.hxx"

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/XUriReference.hpp"
#include "com/sun/star/uri/XUriSchemeParser.hpp"
#include "com/sun/star/uri/XVndSunStarScriptUrl.hpp"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/mutex.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <new>

namespace css = com::sun::star;

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
                    || encoded >= 0xD800 && encoded <= 0xDFFF
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

bool parseSchemeSpecificPart(rtl::OUString const & part) {
    sal_Int32 len = part.getLength();
    sal_Int32 i = 0;
    if (parsePart(part, true, &i).getLength() == 0 || part[0] == '/') {
        return false;
    }
    if (i == len) {
        return true;
    }
    for (;;) {
        ++i; // skip '?' or '&'
        if (parsePart(part, false, &i).getLength() == 0 || i == len
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
    public cppu::ImplInheritanceHelper2<
        stoc::uriproc::UriReference, css::lang::XServiceInfo,
        css::uri::XVndSunStarScriptUrl >
{
public:
    UrlReference(rtl::OUString const & scheme, rtl::OUString const & path);

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasParameter(rtl::OUString const & key)
        throw (css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getParameter(rtl::OUString const & key)
        throw (css::uno::RuntimeException);

private:
    UrlReference(UrlReference &); // not implemented
    void operator =(UrlReference); // not implemented

    virtual ~UrlReference() {}

    sal_Int32 findParameter(rtl::OUString const & key);
};

UrlReference::UrlReference(
    rtl::OUString const & scheme, rtl::OUString const & path)
{
    initialize(scheme, false, false, rtl::OUString(), path, false,
               rtl::OUString());
}

rtl::OUString UrlReference::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return rtl::OUString::createFromAscii(
        "com.sun.star.comp.uri.UriSchemeParser_vndDOTsunDOTstarDOTscript."
            "UrlReference"); //TODO: has this service an implementation name?
}

sal_Bool UrlReference::supportsService(rtl::OUString const & serviceName)
    throw (css::uno::RuntimeException)
{
    return stoc::uriproc::supportsService(
        getSupportedServiceNames(), serviceName);
}

css::uno::Sequence< rtl::OUString > UrlReference::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > s(1);
    s[0] = rtl::OUString::createFromAscii(
        "com.sun.star.uri.VndSunStarScriptUrlReference");
    return s;
}

rtl::OUString UrlReference::getName() throw (css::uno::RuntimeException) {
    osl::MutexGuard g(m_mutex);
    sal_Int32 i = 0;
    return parsePart(m_path, true, &i);
}

sal_Bool UrlReference::hasParameter(rtl::OUString const & key)
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard g(m_mutex);
    return findParameter(key) >= 0;
}

rtl::OUString UrlReference::getParameter(rtl::OUString const & key)
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard g(m_mutex);
    sal_Int32 i = findParameter(key);
    return i >= 0 ? parsePart(m_path, false, &i) : rtl::OUString();
}

sal_Int32 UrlReference::findParameter(rtl::OUString const & key) {
    sal_Int32 i = 0;
    parsePart(m_path, true, &i); // skip name
    for (;;) {
        if (i == m_path.getLength()) {
            return -1;
        }
        ++i; // skip '?' or '&'
        rtl::OUString k = parsePart(m_path, false, &i);
        ++i; // skip '='
        if (k == key) {
            return i;
        }
        parsePart(m_path, false, &i); // skip value
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
    return stoc::uriproc::UriSchemeParser_vndDOTsunDOTstarDOTscript::
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
    return stoc::uriproc::UriSchemeParser_vndDOTsunDOTstarDOTscript::
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
            rtl::OUString::createFromAscii("std::bad_alloc"), 0);
    }
}

}

namespace stoc { namespace uriproc {
namespace UriSchemeParser_vndDOTsunDOTstarDOTscript {

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const &)
    SAL_THROW((css::uno::Exception))
{
    //TODO: single instance
    try {
        return static_cast< cppu::OWeakObject * >(new Parser);
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii("std::bad_alloc"), 0);
    }
}

rtl::OUString getImplementationName() {
    return rtl::OUString::createFromAscii(
        "com.sun.star.comp.uri.UriSchemeParser_vndDOTsunDOTstarDOTscript");
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    css::uno::Sequence< rtl::OUString > s(1);
    s[0] = rtl::OUString::createFromAscii(
        "com.sun.star.uri.UriSchemeParser_vndDOTsunDOTstarDOTscript");
    return s;
}

} } }
