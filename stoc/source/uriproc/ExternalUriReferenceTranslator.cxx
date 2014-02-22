/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "stocservices.hxx"

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/XExternalUriReferenceTranslator.hpp"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/supportsservice.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/thread.h"
#include "rtl/string.h"
#include "rtl/textenc.h"
#include "rtl/uri.h"
#include "rtl/uri.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace {

class Translator: public cppu::WeakImplHelper2<
    css::lang::XServiceInfo, css::uri::XExternalUriReferenceTranslator >
{
public:
    explicit Translator(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        m_context(context) {}

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(OUString const & serviceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    virtual OUString SAL_CALL
    translateToInternal(OUString const & externalUriReference)
        throw (css::uno::RuntimeException);

    virtual OUString SAL_CALL
    translateToExternal(OUString const & internalUriReference)
        throw (css::uno::RuntimeException);

private:
    Translator(Translator &); 
    void operator =(Translator); 

    virtual ~Translator() {}

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

OUString Translator::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return
        stoc_services::ExternalUriReferenceTranslator::getImplementationName();
}

sal_Bool Translator::supportsService(OUString const & serviceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > Translator::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return stoc_services::ExternalUriReferenceTranslator::
        getSupportedServiceNames();
}

OUString Translator::translateToInternal(
    OUString const & externalUriReference)
    throw (css::uno::RuntimeException)
{
    if (!externalUriReference.matchIgnoreAsciiCase("file:/"))
    {
        return externalUriReference;
    }
    sal_Int32 i = RTL_CONSTASCII_LENGTH("file:");
    OUStringBuffer buf;
    buf.append(externalUriReference.getStr(), i);
    
    
    if (!externalUriReference.match("
    {
        buf.append("
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
    throw (css::uno::RuntimeException)
{
    if (!internalUriReference.matchIgnoreAsciiCase("file:
    {
        return internalUriReference;
    }
    sal_Int32 i = RTL_CONSTASCII_LENGTH("file:
    OUStringBuffer buf;
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

namespace stoc_services  { namespace ExternalUriReferenceTranslator {

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    SAL_THROW((css::uno::Exception))
{
    return static_cast< cppu::OWeakObject * >(new Translator(context));
}

OUString getImplementationName() {
    return OUString("com.sun.star.comp.uri.ExternalUriReferenceTranslator");
}

css::uno::Sequence< OUString > getSupportedServiceNames() {
    css::uno::Sequence< OUString > s(1);
    s[0] = "com.sun.star.uri.ExternalUriReferenceTranslator";
    return s;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
