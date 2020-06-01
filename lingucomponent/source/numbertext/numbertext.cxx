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

#include <config_libnumbertext.h>
#include <iostream>

#include <osl/file.hxx>
#include <tools/debug.hxx>

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

#if ENABLE_LIBNUMBERTEXT
#include <Numbertext.hxx>
#endif

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

#define SERVICENAME "com.sun.star.linguistic2.NumberText"
#define IMPLNAME "com.sun.star.lingu2.NumberText"

static Sequence<OUString> getSupportedServiceNames_NumberText_Impl()
{
    Sequence<OUString> names{ SERVICENAME };
    return names;
}

static OUString getImplementationName_NumberText_Impl() { return IMPLNAME; }

static osl::Mutex& GetNumberTextMutex()
{
    static osl::Mutex aMutex;
    return aMutex;
}

namespace
{
class NumberText_Impl : public ::cppu::WeakImplHelper<XNumberText, XServiceInfo>
{
#if ENABLE_LIBNUMBERTEXT
    Numbertext m_aNumberText;
#endif
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
#else
    aPhysPath += "/";
#endif
#if ENABLE_LIBNUMBERTEXT
    OString path = OUStringToOString(aPhysPath, osl_getThreadTextEncoding());
    m_aNumberText.set_prefix(path.getStr());
#endif
}

OUString SAL_CALL NumberText_Impl::getNumberText(const OUString& rText, const Locale&
#if ENABLE_LIBNUMBERTEXT
                                                                            rLocale)
#else
)
#endif
{
    osl::MutexGuard aGuard(GetNumberTextMutex());
    EnsureInitialized();
#if ENABLE_LIBNUMBERTEXT
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
    OString aInput(OUStringToOString(rText, RTL_TEXTENCODING_UTF8));
    std::wstring aResult = Numbertext::string2wstring(aInput.getStr());
    bool result = m_aNumberText.numbertext(aResult, aLangCode.getStr());
    DBG_ASSERT(result, "numbertext: false");
    OString aResult2(Numbertext::wstring2string(aResult).c_str());
    return OUString::fromUtf8(aResult2);
#else
    return rText;
#endif
}

uno::Sequence<Locale> SAL_CALL NumberText_Impl::getAvailableLanguages()
{
    osl::MutexGuard aGuard(GetNumberTextMutex());
    // TODO
    Sequence<css::lang::Locale> aRes;
    return aRes;
}

OUString SAL_CALL NumberText_Impl::getImplementationName() { return IMPLNAME; }

sal_Bool SAL_CALL NumberText_Impl::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL NumberText_Impl::getSupportedServiceNames() { return { SERVICENAME }; }

/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
static Reference<XInterface> NumberText_Impl_create(Reference<XComponentContext> const&)
{
    return static_cast<::cppu::OWeakObject*>(new NumberText_Impl);
}

//#### EXPORTED ### functions to allow for registration and creation of the UNO component
static const struct ::cppu::ImplementationEntry s_component_entries[]
    = { { NumberText_Impl_create, getImplementationName_NumberText_Impl,
          getSupportedServiceNames_NumberText_Impl, ::cppu::createSingleComponentFactory, nullptr,
          0 },
        { nullptr, nullptr, nullptr, nullptr, nullptr, 0 } };

extern "C" {

SAL_DLLPUBLIC_EXPORT void* numbertext_component_getFactory(char const* implName, void* xMgr,
                                                           void* xRegistry)
{
    return ::cppu::component_getFactoryHelper(implName, xMgr, xRegistry, s_component_entries);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
