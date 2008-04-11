/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localebackend.cxx,v $
 * $Revision: 1.12 $
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
#include "precompiled_shell.hxx"

#include "localebackend.hxx"
#include "localelayer.hxx"
#include <com/sun/star/configuration/backend/ComponentChangeEvent.hpp>
#include <uno/current_context.hxx>
#include <osl/time.h>

#include <stdio.h>

#if defined(LINUX) || defined(SOLARIS) || defined(IRIX) || defined(NETBSD) || defined(FREEBSD) || defined(OS2)

#include <rtl/ustrbuf.hxx>
#include <locale.h>
#include <string.h>

/*
 * Note: setlocale is not at all thread safe, so is this code. It could
 * especially interfere with the stuff VCL is doing, so make sure this
 * is called from the main thread only.
 */

static rtl::OUString ImplGetLocale(int category)
{
    const char *locale = setlocale(category, "");

    // Return "en-US" for C locales
    if( (locale == NULL) || ( locale[0] == 'C' && locale[1] == '\0' ) )
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "en-US" ) );


    const char *cp;
    const char *uscore = NULL;

    // locale string have the format lang[_ctry][.encoding][@modifier]
    // we are only interested in the first two items, so we handle
    // '.' and '@' as string end.
    for (cp = locale; *cp; cp++)
    {
        if (*cp == '_')
            uscore = cp;
        if (*cp == '.' || *cp == '@')
            break;
    }

    rtl::OUStringBuffer aLocaleBuffer;
    if( uscore != NULL )
    {
        aLocaleBuffer.appendAscii(locale, uscore++ - locale);
        aLocaleBuffer.appendAscii("-");
        aLocaleBuffer.appendAscii(uscore, cp - uscore);
    }
    else
    {
        aLocaleBuffer.appendAscii(locale, cp - locale);
    }

    return aLocaleBuffer.makeStringAndClear();
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

    void OUStringBufferAppendCFString(rtl::OUStringBuffer& buffer, const CFStringRef s)
    {
        CFIndex lstr = CFStringGetLength(s);
        for (CFIndex i = 0; i < lstr; i++)
            buffer.append(CFStringGetCharacterAtIndex(s, i));
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
        CFStringRef csPref = CFStringCreateWithCString(NULL, pref, kCFStringEncodingASCII);
        CFStringGuard csRefGuard(csPref);

        CFTypeRef ref = CFPreferencesCopyAppValue(csPref, kCFPreferencesCurrentApplication);
        CFTypeRefGuard refGuard(ref);

        if (ref == NULL)
            return NULL;

        CFStringRef sref = (CFGetTypeID(ref) == CFArrayGetTypeID()) ? (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)ref, 0) : (CFStringRef)ref;

        // NOTE: this API is only available with Mac OS X >=10.3. We need to use it because
        // Apple used non-ISO values on systems <10.2 like "German" for instance but didn't
        // upgrade those values during upgrade to newer Mac OS X versions. See also #i54337#
        return CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorDefault, sref);
    }

    rtl::OUString ImplGetLocale(const char* pref)
    {
        CFStringRef sref = ImplGetAppPreference(pref);
        CFStringGuard srefGuard(sref);

        rtl::OUStringBuffer aLocaleBuffer;
        aLocaleBuffer.appendAscii("en-US"); // initialize with fallback value

        if (sref != NULL)
        {
            // split the string into substrings; the first two (if there are two) substrings
            // are language and country
            CFArrayRef subs = CFStringCreateArrayBySeparatingStrings(NULL, sref, CFSTR("_"));
            CFArrayGuard subsGuard(subs);

            if (subs != NULL)
            {
                aLocaleBuffer.setLength(0); // clear buffer which still contains fallback value

                CFStringRef lang = (CFStringRef)CFArrayGetValueAtIndex(subs, 0);
                OUStringBufferAppendCFString(aLocaleBuffer, lang);

                // country also available? Assumption: if the array contains more than one
                // value the second value is always the country!
                if (CFArrayGetCount(subs) > 1)
                {
                    aLocaleBuffer.appendAscii("-");
                    CFStringRef country = (CFStringRef)CFArrayGetValueAtIndex(subs, 1);
                    OUStringBufferAppendCFString(aLocaleBuffer, country);
                }
            }
        }
        return aLocaleBuffer.makeStringAndClear();
    }

} // namespace /* private */

#endif

// -------------------------------------------------------------------------------

#ifdef WNT

#ifdef WINVER
#undef WINVER
#endif
#define WINVER 0x0501

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

rtl::OUString ImplGetLocale(LCID lcid)
{
    TCHAR buffer[8];
    LPTSTR cp = buffer;

    cp += GetLocaleInfo( lcid, LOCALE_SISO639LANGNAME , buffer, 4 );
    if( cp > buffer )
    {
        if( 0 < GetLocaleInfo( lcid, LOCALE_SISO3166CTRYNAME, cp, buffer + 8 - cp) )
            // #i50822# minus character must be written before cp
            *(cp - 1) = '-';

        return rtl::OUString::createFromAscii(buffer);
    }

    return rtl::OUString();
}

#endif // WNT

// -------------------------------------------------------------------------------

LocaleBackend::LocaleBackend(const uno::Reference<uno::XComponentContext>& xContext)
    throw (backend::BackendAccessException) :
    ::cppu::WeakImplHelper2 < backend::XSingleLayerStratum, lang::XServiceInfo > (),
    m_xContext(xContext)

{
}

//------------------------------------------------------------------------------

LocaleBackend::~LocaleBackend(void)
{
}

//------------------------------------------------------------------------------

LocaleBackend* LocaleBackend::createInstance(
    const uno::Reference<uno::XComponentContext>& xContext
)
{
    return new LocaleBackend(xContext);
}

// ---------------------------------------------------------------------------------------

rtl::OUString LocaleBackend::getLocale(void)
{
#if defined(LINUX) || defined(SOLARIS) || defined(IRIX) || defined(NETBSD) || defined(FREEBSD) || defined(OS2)
    return ImplGetLocale(LC_CTYPE);
#elif defined (MACOSX)
    return ImplGetLocale("AppleLocale");
#elif defined WNT
    return ImplGetLocale( GetUserDefaultLCID() );
#endif
}

//------------------------------------------------------------------------------

rtl::OUString LocaleBackend::getUILocale(void)
{
#if defined(LINUX) || defined(SOLARIS) || defined(IRIX) || defined(NETBSD) || defined(FREEBSD) || defined(OS2)
    return ImplGetLocale(LC_MESSAGES);
#elif defined(MACOSX)
    return ImplGetLocale("AppleLanguages");
#elif defined WNT
    return ImplGetLocale( MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT) );
#endif
}

// ---------------------------------------------------------------------------------------

rtl::OUString LocaleBackend::getSystemLocale(void)
{
// note: the implementation differs from getLocale() only on Windows
#if defined WNT
    return ImplGetLocale( GetSystemDefaultLCID() );
#else
    return getLocale();
#endif
}
//------------------------------------------------------------------------------

rtl::OUString LocaleBackend::createTimeStamp()
{
    // the time stamp is free text, so just returning the values here.
    return getLocale() + getUILocale() + getSystemLocale();
}

//------------------------------------------------------------------------------

uno::Reference<backend::XLayer> SAL_CALL LocaleBackend::getLayer(
        const rtl::OUString& aComponent, const rtl::OUString& /*aTimestamp*/)
    throw (backend::BackendAccessException, lang::IllegalArgumentException)
{

    uno::Sequence<rtl::OUString> aComps( getSupportedComponents() );
    if( aComponent.equals( aComps[0]) )
    {
        if( ! m_xSystemLayer.is() )
        {
            uno::Sequence<backend::PropertyInfo> aPropInfoList(3);

            aPropInfoList[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.System/L10N/UILocale") );
            aPropInfoList[0].Type = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "string" ) );
            aPropInfoList[0].Protected = sal_False;
            aPropInfoList[0].Value = uno::makeAny( getUILocale() );

            aPropInfoList[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.System/L10N/Locale") );
            aPropInfoList[1].Type = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "string" ));
            aPropInfoList[1].Protected = sal_False;
            aPropInfoList[1].Value = uno::makeAny( getLocale() );

            aPropInfoList[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.System/L10N/SystemLocale") );
            aPropInfoList[2].Type = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "string" ));
            aPropInfoList[2].Protected = sal_False;
            aPropInfoList[2].Value = uno::makeAny( getSystemLocale() );

            m_xSystemLayer = new LocaleLayer(aPropInfoList, createTimeStamp(), m_xContext);
        }

        return m_xSystemLayer;
    }

    return uno::Reference<backend::XLayer>();
}

//------------------------------------------------------------------------------

uno::Reference<backend::XUpdatableLayer> SAL_CALL
LocaleBackend::getUpdatableLayer(const rtl::OUString& /*aComponent*/)
    throw (backend::BackendAccessException,lang::NoSupportException,
           lang::IllegalArgumentException)
{
    throw lang::NoSupportException(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
            "LocaleBackend: No Update Operation allowed, Read Only access") ),
        *this) ;
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocaleBackend::getBackendName(void) {
    return rtl::OUString::createFromAscii("com.sun.star.comp.configuration.backend.LocaleBackend") ;
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocaleBackend::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getBackendName() ;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL LocaleBackend::getBackendServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServiceNameList(2);
    aServiceNameList[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.LocaleBackend")) ;
    aServiceNameList[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.PlatformBackend")) ;

    return aServiceNameList ;
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocaleBackend::supportsService(const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getBackendServiceNames();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;

    return false;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL LocaleBackend::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getBackendServiceNames() ;
}

// ---------------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL LocaleBackend::getSupportedComponents(void)
{
    uno::Sequence<rtl::OUString> aSupportedComponentList(1);
    aSupportedComponentList[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.System" )
    );

    return aSupportedComponentList;
}

