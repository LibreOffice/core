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

#include <cassert>
#include <limits>

#include "localebackend.hxx"
#include <com/sun/star/beans/Optional.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/time.h>
#include <rtl/character.hxx>

#include <stdio.h>

#ifdef _WIN32
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

css::beans::Optional<css::uno::Any> ImplGetLocale(LCID lcid)
{
    WCHAR buffer[8];
    PWSTR cp = buffer;

    cp += GetLocaleInfoW( lcid, LOCALE_SISO639LANGNAME, buffer, 4 );
    if( cp > buffer )
    {
        if( 0 < GetLocaleInfoW( lcid, LOCALE_SISO3166CTRYNAME, cp, buffer + 8 - cp) )
            // #i50822# minus character must be written before cp
            *(cp - 1) = '-';

        return {true, css::uno::Any(OUString(SAL_U(buffer)))};
    }

    return {false, {}};
}

#elif defined(MACOSX)

#include <rtl/ustrbuf.hxx>
#include <locale.h>
#include <string.h>

#include <premac.h>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>

namespace /* private */
{

    void OUStringBufferAppendCFString(OUStringBuffer& buffer, const CFStringRef s)
    {
        CFIndex lstr = CFStringGetLength(s);
        for (CFIndex i = 0; i < lstr; i++)
            buffer.append(sal_Unicode(CFStringGetCharacterAtIndex(s, i)));
    }

    template <typename T>
    class CFGuard
    {
    public:
        explicit CFGuard(T& rT) : rT_(rT) {}
        ~CFGuard() { if (rT_) CFRelease(rT_); }
    private:
        T& rT_;
    };

    typedef CFGuard<CFArrayRef> CFArrayGuard;
    typedef CFGuard<CFStringRef> CFStringGuard;
    typedef CFGuard<CFTypeRef> CFTypeRefGuard;

    /* For more information on the Apple locale concept please refer to
    http://developer.apple.com/documentation/CoreFoundation/Conceptual/CFLocales/Articles/CFLocaleConcepts.html
    According to this documentation a locale identifier has the format: language[_country][_variant]*
    e.g. es_ES_PREEURO -> spain prior Euro support
    Note: The calling code should be able to handle locales with only language information e.g. 'en' for certain
    UI languages just the language code will be returned.
    */

    CFStringRef ImplGetAppPreference(const char* pref)
    {
        CFStringRef csPref = CFStringCreateWithCString(nullptr, pref, kCFStringEncodingASCII);
        CFStringGuard csRefGuard(csPref);

        CFTypeRef ref = CFPreferencesCopyAppValue(csPref, kCFPreferencesCurrentApplication);
        CFTypeRefGuard refGuard(ref);

        if (ref == nullptr)
            return nullptr;

        CFStringRef sref = (CFGetTypeID(ref) == CFArrayGetTypeID()) ? static_cast<CFStringRef>(CFArrayGetValueAtIndex(static_cast<CFArrayRef>(ref), 0)) : static_cast<CFStringRef>(ref);

        // NOTE: this API is only available with Mac OS X >=10.3. We need to use it because
        // Apple used non-ISO values on systems <10.2 like "German" for instance but didn't
        // upgrade those values during upgrade to newer Mac OS X versions. See also #i54337#
        return CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorDefault, sref);
    }

    css::beans::Optional<css::uno::Any> ImplGetLocale(const char* pref)
    {
        CFStringRef sref = ImplGetAppPreference(pref);
        CFStringGuard srefGuard(sref);

        OUStringBuffer aLocaleBuffer;
        aLocaleBuffer.append("en-US"); // initialize with fallback value

        if (sref != nullptr)
        {
            // split the string into substrings; the first two (if there are two) substrings
            // are language and country
            CFArrayRef subs = CFStringCreateArrayBySeparatingStrings(nullptr, sref, CFSTR("_"));
            CFArrayGuard subsGuard(subs);

            if (subs != nullptr)
            {
                aLocaleBuffer.setLength(0); // clear buffer which still contains fallback value

                CFStringRef lang = static_cast<CFStringRef>(CFArrayGetValueAtIndex(subs, 0));
                OUStringBufferAppendCFString(aLocaleBuffer, lang);

                // country also available? Assumption: if the array contains more than one
                // value the second value is always the country!
                if (CFArrayGetCount(subs) > 1)
                {
                    aLocaleBuffer.append("-");
                    CFStringRef country = static_cast<CFStringRef>(CFArrayGetValueAtIndex(subs, 1));
                    OUStringBufferAppendCFString(aLocaleBuffer, country);
                }
            }
        }
        return {true, css::uno::Any(aLocaleBuffer.makeStringAndClear())};
    }

} // namespace /* private */

#else

#include <rtl/ustrbuf.hxx>
#include <cstdlib>
#include <cstring>

static css::beans::Optional<css::uno::Any> ImplGetLocale(char const * category)
{
    const char *locale = std::getenv("LC_ALL");
    if (locale == nullptr || *locale == '\0') {
        locale = std::getenv(category);
        if (locale == nullptr || *locale == '\0') {
            locale = std::getenv("LANG");
        }
    }

    // Return "en-US" for C locales
    if( (locale == nullptr) || *locale == '\0' || std::strcmp(locale, "C") == 0
        || std::strcmp(locale, "POSIX") == 0 )
        return {true, css::uno::Any(OUString("en-US"))};


    const char *cp;
    const char *uscore = nullptr;

    // locale string have the format lang[_ctry][.encoding][@modifier]
    // we are only interested in the first two items, so we handle
    // '.' and '@' as string end.
    for (cp = locale; *cp; cp++)
    {
        if (*cp == '_')
            uscore = cp;
        if (*cp == '.' || *cp == '@')
            break;
        if (!rtl::isAscii(static_cast<unsigned char>(*cp))) {
            SAL_INFO("shell", "locale env var with non-ASCII content");
            return {false, {}};
        }
    }
    assert(cp >= locale);
    if (cp - locale > std::numeric_limits<sal_Int32>::max()) {
        SAL_INFO("shell", "locale env var content too long");
        return {false, {}};
    }

    OUStringBuffer aLocaleBuffer;
    if( uscore != nullptr )
    {
        aLocaleBuffer.appendAscii(locale, uscore++ - locale);
        aLocaleBuffer.append("-");
        aLocaleBuffer.appendAscii(uscore, cp - uscore);
    }
    else
    {
        aLocaleBuffer.appendAscii(locale, cp - locale);
    }

    return {true, css::uno::Any(aLocaleBuffer.makeStringAndClear())};
}

#endif


LocaleBackend::LocaleBackend()
{
}


LocaleBackend::~LocaleBackend()
{
}


LocaleBackend* LocaleBackend::createInstance()
{
    return new LocaleBackend;
}


css::beans::Optional<css::uno::Any> LocaleBackend::getLocale()
{
#if defined(_WIN32)
    return ImplGetLocale( GetUserDefaultLCID() );
#elif defined (MACOSX)
    return ImplGetLocale("AppleLocale");
#else
    return ImplGetLocale("LC_CTYPE");
#endif
}


css::beans::Optional<css::uno::Any> LocaleBackend::getUILocale()
{
#if defined(_WIN32)
    return ImplGetLocale( MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT) );
#elif defined(MACOSX)
    return ImplGetLocale("AppleLanguages");
#else
    return ImplGetLocale("LC_MESSAGES");
#endif
}


css::beans::Optional<css::uno::Any> LocaleBackend::getSystemLocale()
{
// note: the implementation differs from getLocale() only on Windows
#if defined(_WIN32)
    return ImplGetLocale( GetSystemDefaultLCID() );
#else
    return getLocale();
#endif
}


void LocaleBackend::setPropertyValue(
    OUString const &, css::uno::Any const &)
{
    throw css::lang::IllegalArgumentException(
        "setPropertyValue not supported",
        static_cast< cppu::OWeakObject * >(this), -1);
}

css::uno::Any LocaleBackend::getPropertyValue(
    OUString const & PropertyName)
{
    if ( PropertyName == "Locale" ) {
        return css::uno::Any(getLocale());
    } else if (PropertyName == "SystemLocale")
    {
        return css::uno::Any(getSystemLocale());
    } else if (PropertyName == "UILocale")
    {
        return css::uno::Any(getUILocale());
    } else {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
}


OUString SAL_CALL LocaleBackend::getBackendName() {
    return OUString("com.sun.star.comp.configuration.backend.LocaleBackend") ;
}

OUString SAL_CALL LocaleBackend::getImplementationName()
{
    return getBackendName() ;
}

uno::Sequence<OUString> SAL_CALL LocaleBackend::getBackendServiceNames()
{
    uno::Sequence<OUString> aServiceNameList { "com.sun.star.configuration.backend.LocaleBackend" };
    return aServiceNameList ;
}

sal_Bool SAL_CALL LocaleBackend::supportsService(const OUString& aServiceName)
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence<OUString> SAL_CALL LocaleBackend::getSupportedServiceNames()
{
    return getBackendServiceNames() ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
