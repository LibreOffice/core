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
#include <sal/log.hxx>

#include <cassert>
#include <limits>

#include "localebackend.hxx"
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/character.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>

#ifdef _WIN32
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static css::beans::Optional<css::uno::Any> ImplGetLocale(LCID lcid)
{
    WCHAR buffer[8];
    PWSTR cp = buffer;

    cp += GetLocaleInfoW( lcid, LOCALE_SISO639LANGNAME, buffer, 4 );
    if( cp > buffer )
    {
        if( 0 < GetLocaleInfoW( lcid, LOCALE_SISO3166CTRYNAME, cp, buffer + 8 - cp) )
            // #i50822# minus character must be written before cp
            *(cp - 1) = '-';

        return {true, css::uno::Any(OUString(o3tl::toU(buffer)))};
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

        // NOTE: this API is only available with macOS >=10.3. We need to use it because
        // Apple used non-ISO values on systems <10.2 like "German" for instance but didn't
        // upgrade those values during upgrade to newer macOS versions. See also #i54337#
        return CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorDefault, sref);
    }

    css::beans::Optional<css::uno::Any> ImplGetLocale(const char* pref)
    {
        CFStringRef sref = ImplGetAppPreference(pref);
        CFStringGuard srefGuard(sref);

        OUStringBuffer aLocaleBuffer("en-US"); // initialize with fallback value

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
        return {true, css::uno::Any(u"en-US"_ustr)};


    const char *cp;
    const char *uscore = nullptr;
    const char *end = nullptr;

    // locale string have the format lang[_ctry][.encoding][@modifier]
    // Let LanguageTag handle all conversion, but do a sanity and length check
    // first.
    // For the fallback we are only interested in the first two items, so we
    // handle '.' and '@' as string end for that.
    for (cp = locale; *cp; cp++)
    {
        if (*cp == '_' && !uscore)
            uscore = cp;
        if ((*cp == '.' || *cp == '@') && !end)
            end = cp;
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

    // This is a tad awkward... but the easiest way to obtain what we're
    // actually interested in. For example this also converts
    // "ca_ES.UTF-8@valencia" to "ca-ES-valencia".
    const OString aLocaleStr(locale);
    const LanguageType nLang = MsLangId::convertUnxByteStringToLanguage( aLocaleStr);
    if (nLang != LANGUAGE_DONTKNOW)
    {
        const OUString aLangTagStr( LanguageTag::convertToBcp47( nLang));
        return {true, css::uno::Any(aLangTagStr)};
    }

    // As a fallback, strip encoding and modifier and return just a
    // language-country combination and let the caller handle unknowns.
    OUStringBuffer aLocaleBuffer;
    if (!end)
        end = cp;
    if( uscore != nullptr )
    {
        aLocaleBuffer.appendAscii(locale, uscore++ - locale);
        aLocaleBuffer.append("-");
        aLocaleBuffer.appendAscii(uscore, end - uscore);
    }
    else
    {
        aLocaleBuffer.appendAscii(locale, end - locale);
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
        u"setPropertyValue not supported"_ustr,
        getXWeak(), -1);
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
            PropertyName, getXWeak());
    }
}


OUString SAL_CALL LocaleBackend::getImplementationName()
{
    return u"com.sun.star.comp.configuration.backend.LocaleBackend"_ustr ;
}

sal_Bool SAL_CALL LocaleBackend::supportsService(const OUString& aServiceName)
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence<OUString> SAL_CALL LocaleBackend::getSupportedServiceNames()
{
    return { u"com.sun.star.configuration.backend.LocaleBackend"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
shell_LocaleBackend_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new LocaleBackend());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
