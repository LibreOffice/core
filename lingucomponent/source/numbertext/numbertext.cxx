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

#include <iostream>

#include <osl/file.hxx>
#include <tools/debug.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <sal/config.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <i18nlangtag/languagetag.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XNumberText.hpp>
#include <unotools/pathoptions.hxx>
#include <osl/thread.h>

#include <sal/macros.h>

#include <Numbertext.hxx>

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

static osl::Mutex& GetNumberTextMutex()
{
    static osl::Mutex aMutex;
    return aMutex;
}

namespace
{
class NumberText_Impl : public ::cppu::WeakImplHelper<XNumberText, XServiceInfo>
{
    Numbertext m_aNumberText;
    bool m_bInitialized;

    virtual ~NumberText_Impl() override {}
    void EnsureInitialized();

public:
    NumberText_Impl();
    NumberText_Impl(const NumberText_Impl&) = delete;
    NumberText_Impl& operator=(const NumberText_Impl&) = delete;

    // XServiceInfo implementation
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XNumberText implementation
    virtual OUString SAL_CALL getNumberText(const OUString& aText,
                                            const ::css::lang::Locale& rLocale) override;
    virtual css::uno::Sequence<css::lang::Locale> SAL_CALL getAvailableLanguages() override;
};
}

NumberText_Impl::NumberText_Impl()
    : m_bInitialized(false)
{
}

void NumberText_Impl::EnsureInitialized()
{
    if (m_bInitialized)
        return;

    // set this to true at the very start to prevent loops because of
    // implicitly called functions below
    m_bInitialized = true;

    // set default numbertext path to where those get installed
    OUString aPhysPath;
    OUString aURL(SvtPathOptions().GetNumbertextPath());
    osl::FileBase::getSystemPathFromFileURL(aURL, aPhysPath);
#ifdef _WIN32
    aPhysPath += "\\";
    const rtl_TextEncoding eEnc = RTL_TEXTENCODING_UTF8;
#else
    aPhysPath += "/";
    const rtl_TextEncoding eEnc = osl_getThreadTextEncoding();
#endif
    OString path = OUStringToOString(aPhysPath, eEnc);
    m_aNumberText.set_prefix(path.getStr());
}

OUString SAL_CALL NumberText_Impl::getNumberText(const OUString& rText, const Locale& rLocale)
{
    osl::MutexGuard aGuard(GetNumberTextMutex());
    EnsureInitialized();
    // libnumbertext supports Language + Country tags (separated by "_" or "-")
    LanguageTag aLanguageTag(rLocale);
    OUString aCode(aLanguageTag.getLanguage());
    OUString aCountry(aLanguageTag.getCountry());
    OUString aScript(aLanguageTag.getScript());
    if (!aScript.isEmpty())
        aCode += "-" + aScript;
    if (!aCountry.isEmpty())
        aCode += "-" + aCountry;
    OString aLangCode(OUStringToOString(aCode, RTL_TEXTENCODING_ASCII_US));
#if defined(_WIN32)
    std::wstring sResult(o3tl::toW(rText.getStr()));
#else
    OString aInput(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
    std::wstring sResult = Numbertext::string2wstring(aInput.getStr());
#endif
    bool result = m_aNumberText.numbertext(sResult, aLangCode.getStr());
    DBG_ASSERT(result, "numbertext: false");
#if defined(_WIN32)
    OUString aResult(o3tl::toU(sResult.c_str()));
#else
    OUString aResult = OUString::fromUtf8(Numbertext::wstring2string(sResult).c_str());
#endif
    return aResult;
}

uno::Sequence<Locale> SAL_CALL NumberText_Impl::getAvailableLanguages()
{
    osl::MutexGuard aGuard(GetNumberTextMutex());
    // TODO
    Sequence<css::lang::Locale> aRes;
    return aRes;
}

OUString SAL_CALL NumberText_Impl::getImplementationName()
{
    return "com.sun.star.lingu2.NumberText";
}

sal_Bool SAL_CALL NumberText_Impl::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL NumberText_Impl::getSupportedServiceNames()
{
    return { "com.sun.star.linguistic2.NumberText" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
lingucomponent_NumberText_get_implementation(css::uno::XComponentContext*,
                                             css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new NumberText_Impl());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
