/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

#include "stocservices.hxx"

#include "supportsService.hxx"

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/XExternalUriReferenceTranslator.hpp"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/thread.h"
#include "rtl/string.h"
#include "rtl/textenc.h"
#include "rtl/uri.h"
#include "rtl/uri.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <new>

namespace css = com::sun::star;

namespace {

class Translator: public cppu::WeakImplHelper2<
    css::lang::XServiceInfo, css::uri::XExternalUriReferenceTranslator >
{
public:
    explicit Translator(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        m_context(context) {}

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL
    translateToInternal(rtl::OUString const & externalUriReference)
        throw (css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL
    translateToExternal(rtl::OUString const & internalUriReference)
        throw (css::uno::RuntimeException);

private:
    Translator(Translator &); // not implemented
    void operator =(Translator); // not implemented

    virtual ~Translator() {}

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

rtl::OUString Translator::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return
        stoc_services::ExternalUriReferenceTranslator::getImplementationName();
}

sal_Bool Translator::supportsService(rtl::OUString const & serviceName)
    throw (css::uno::RuntimeException)
{
    return stoc::uriproc::supportsService(
        getSupportedServiceNames(), serviceName);
}

css::uno::Sequence< rtl::OUString > Translator::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return stoc_services::ExternalUriReferenceTranslator::
        getSupportedServiceNames();
}

rtl::OUString Translator::translateToInternal(
    rtl::OUString const & externalUriReference)
    throw (css::uno::RuntimeException)
{
    if (!externalUriReference.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("file:/")))
    {
        return externalUriReference;
    }
    sal_Int32 i = RTL_CONSTASCII_LENGTH("file:");
    rtl::OUStringBuffer buf;
    buf.append(externalUriReference.getStr(), i);
    // Some environments (e.g., Java) produce illegal file URLs without an
    // authority part; treat them as having an empty authority part:
    if (!externalUriReference.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("//"), i))
    {
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("//"));
    }
    rtl_TextEncoding encoding = osl_getThreadTextEncoding();
    for (bool path = true;;) {
        sal_Int32 j = i;
        while (j != externalUriReference.getLength()
               && externalUriReference[j] != '#'
               && (!path || externalUriReference[j] != '/'))
        {
            ++j;
        }
        if (j != i) {
            rtl::OUString seg(
                rtl::Uri::encode(
                    rtl::Uri::decode(
                        externalUriReference.copy(i, j - i),
                        rtl_UriDecodeStrict, encoding),
                    rtl_UriCharClassPchar, rtl_UriEncodeStrict,
                    RTL_TEXTENCODING_UTF8));
            if (seg.getLength() == 0) {
                return rtl::OUString();
            }
            buf.append(seg);
        }
        if (j == externalUriReference.getLength()) {
            break;
        }
        buf.append(externalUriReference[j]);
        path = externalUriReference[j] == '/';
        i = j + 1;
    }
    return buf.makeStringAndClear();
}

rtl::OUString Translator::translateToExternal(
    rtl::OUString const & internalUriReference)
    throw (css::uno::RuntimeException)
{
    if (!internalUriReference.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("file://")))
    {
        return internalUriReference;
    }
    sal_Int32 i = RTL_CONSTASCII_LENGTH("file://");
    rtl::OUStringBuffer buf;
    buf.append(internalUriReference.getStr(), i);
    rtl_TextEncoding encoding = osl_getThreadTextEncoding();
    for (bool path = true;;) {
        sal_Int32 j = i;
        while (j != internalUriReference.getLength()
               && internalUriReference[j] != '#'
               && (!path || internalUriReference[j] != '/'))
        {
            ++j;
        }
        if (j != i) {
            // Use rtl_UriDecodeToIuri -> rtl_UriEncodeStrictKeepEscapes instead
            // of rtl_UriDecodeStrict -> rtl_UriEncodeStrict, so that spurious
            // non--UTF-8 octets like "%FE" are copied verbatim:
            rtl::OUString seg(
                rtl::Uri::encode(
                    rtl::Uri::decode(
                        internalUriReference.copy(i, j - i),
                        rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8),
                    rtl_UriCharClassPchar, rtl_UriEncodeStrictKeepEscapes,
                    encoding));
            if (seg.getLength() == 0) {
                return rtl::OUString();
            }
            buf.append(seg);
        }
        if (j == internalUriReference.getLength()) {
            break;
        }
        buf.append(internalUriReference[j]);
        path = internalUriReference[j] == '/';
        i = j + 1;
    }
    return buf.makeStringAndClear();
}

}

namespace stoc_services  { namespace ExternalUriReferenceTranslator {

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    SAL_THROW((css::uno::Exception))
{
    try {
        return static_cast< cppu::OWeakObject * >(new Translator(context));
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("std::bad_alloc")), 0);
    }
}

rtl::OUString getImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.uri.ExternalUriReferenceTranslator"));
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    css::uno::Sequence< rtl::OUString > s(1);
    s[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uri.ExternalUriReferenceTranslator"));
    return s;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
