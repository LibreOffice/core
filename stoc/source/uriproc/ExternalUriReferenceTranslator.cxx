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
            if (seg.isEmpty()) {
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
            if (seg.isEmpty()) {
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
            rtl::OUString("std::bad_alloc"), 0);
    }
}

rtl::OUString getImplementationName() {
    return rtl::OUString("com.sun.star.comp.uri.ExternalUriReferenceTranslator");
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    css::uno::Sequence< rtl::OUString > s(1);
    s[0] = rtl::OUString("com.sun.star.uri.ExternalUriReferenceTranslator");
    return s;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
