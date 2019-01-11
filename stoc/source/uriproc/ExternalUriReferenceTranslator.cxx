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

#include <string_view>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/thread.h>
#include <rtl/string.h>
#include <rtl/textenc.h>
#include <rtl/uri.h>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace {

class Translator:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo, css::uri::XExternalUriReferenceTranslator>
{
public:
    Translator() {}

    Translator(const Translator&) = delete;
    Translator& operator=(const Translator&) = delete;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & serviceName) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    virtual OUString SAL_CALL
    translateToInternal(OUString const & externalUriReference) override;

    virtual OUString SAL_CALL
    translateToExternal(OUString const & internalUriReference) override;

private:
    virtual ~Translator() override {}
};

OUString Translator::getImplementationName()
{
    return OUString("com.sun.star.comp.uri.ExternalUriReferenceTranslator");
}

sal_Bool Translator::supportsService(OUString const & serviceName)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > Translator::getSupportedServiceNames()
{
    css::uno::Sequence< OUString > s { "com.sun.star.uri.ExternalUriReferenceTranslator" };
    return s;
}

OUString Translator::translateToInternal(
    OUString const & externalUriReference)
{
    if (!externalUriReference.matchIgnoreAsciiCase("file:/"))
    {
        return externalUriReference;
    }
    sal_Int32 i = RTL_CONSTASCII_LENGTH("file:");
    OUStringBuffer buf;
    buf.append(std::u16string_view(externalUriReference).substr(0, i));
    // Some environments (e.g., Java) produce illegal file URLs without an
    // authority part; treat them as having an empty authority part:
    if (!externalUriReference.match("//", i))
    {
        buf.append("//");
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
            OUString seg(
                rtl::Uri::encode(
                    rtl::Uri::decode(
                        externalUriReference.copy(i, j - i),
                        rtl_UriDecodeStrict, encoding),
                    rtl_UriCharClassPchar, rtl_UriEncodeStrict,
                    RTL_TEXTENCODING_UTF8));
            if (seg.isEmpty()) {
                return OUString();
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

OUString Translator::translateToExternal(
    OUString const & internalUriReference)
{
    if (!internalUriReference.matchIgnoreAsciiCase("file://"))
    {
        return internalUriReference;
    }
    sal_Int32 i = RTL_CONSTASCII_LENGTH("file://");
    OUStringBuffer buf;
    buf.append(std::u16string_view(internalUriReference).substr(0, i));
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
            OUString seg(
                rtl::Uri::encode(
                    rtl::Uri::decode(
                        internalUriReference.copy(i, j - i),
                        rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8),
                    rtl_UriCharClassPchar, rtl_UriEncodeStrictKeepEscapes,
                    encoding));
            if (seg.isEmpty()) {
                return OUString();
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_uri_ExternalUriReferenceTranslator_get_implementation(css::uno::XComponentContext* ,
        css::uno::Sequence<css::uno::Any> const &)
{
    return ::cppu::acquire(new Translator);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
