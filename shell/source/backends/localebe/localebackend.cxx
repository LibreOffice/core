/*************************************************************************
 *
 *  $RCSfile: localebackend.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-08-18 08:10:47 $
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
 *  WITHOUT LIMITATION, WARRANTIES THAT THE OOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "localebackend.hxx"
#include "localelayer.hxx"

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_COMPONENTCHANGEEVENT_HPP_
#include <com/sun/star/configuration/backend/ComponentChangeEvent.hpp>
#endif

#ifndef _UNO_CURRENT_CONTEXT_HXX_
#include <uno/current_context.hxx>
#endif

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#include <stdio.h>

#if defined(LINUX) || defined(SOLARIS) || defined(IRIX) || defined(NETBSD) || defined(FREEBSD)

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
        explicit CFGuard(T* pT) : pT_(pT) {}
        ~CFGuard() { if (pT_) CFRelease(*pT_); }
    private:
        T* pT_;
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
        CFStringGuard csRefGuard(&csPref);

        CFTypeRef ref = CFPreferencesCopyAppValue(csPref, kCFPreferencesCurrentApplication);
        CFTypeRefGuard refGuard(&ref);

        if (ref == NULL)
            return NULL;

        CFStringRef sref = (CFGetTypeID(ref) == CFArrayGetTypeID()) ? (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)ref, 0) : (CFStringRef)ref;

        CFRetain(sref); // caller is responsible for releasing reference
        return sref;
    }

    rtl::OUString ImplGetLocale(const char* pref)
    {
        CFStringRef sref = ImplGetAppPreference(pref);
        CFStringGuard srefGuard(&sref);

        rtl::OUStringBuffer aLocaleBuffer;
        aLocaleBuffer.appendAscii("en-US"); // initialize with fallback value

        if (sref != NULL)
        {
            // split the string into substrings; the first two (if there are two) substrings
            // are language and country
            CFArrayRef subs = CFStringCreateArrayBySeparatingStrings(NULL, sref, CFSTR("_"));
            CFArrayGuard subsGuard(&subs);

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

#include <windows.h>

rtl::OUString ImplGetLocale(LCID lcid)
{
    TCHAR buffer[8];
    LPTSTR cp = buffer;

    cp += GetLocaleInfo( lcid, LOCALE_SISO639LANGNAME , buffer, 4 );
    if( cp > buffer )
    {
        if( 0 < GetLocaleInfo( lcid, LOCALE_SISO3166CTRYNAME, cp, buffer + 8 - cp) )
          *cp = '-';

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
#if defined(LINUX) || defined(SOLARIS) || defined(IRIX) || defined(NETBSD) || defined(FREEBSD)
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
#if defined(LINUX) || defined(SOLARIS) || defined(IRIX) || defined(NETBSD) || defined(FREEBSD)
    return ImplGetLocale(LC_MESSAGES);
#elif defined(MACOSX)
    return ImplGetLocale("AppleLanguages");
#elif defined WNT
    return ImplGetLocale( MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT) );
#endif
}

//------------------------------------------------------------------------------

rtl::OUString LocaleBackend::createTimeStamp()
{
    // the time stamp is free text, so just returning the values here.
    return getLocale() + getUILocale();
}

//------------------------------------------------------------------------------

uno::Reference<backend::XLayer> SAL_CALL LocaleBackend::getLayer(
        const rtl::OUString& aComponent, const rtl::OUString& aTimestamp)
    throw (backend::BackendAccessException, lang::IllegalArgumentException)
{

    if( aComponent.equals( getSupportedComponents()[0]) )
    {
        if( ! m_xSystemLayer.is() )
        {
            uno::Sequence<backend::PropertyInfo> aPropInfoList(2);

            aPropInfoList[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.System/L10N/UILocale") );
            aPropInfoList[0].Type = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "string" ) );
            aPropInfoList[0].Protected = sal_False;
            aPropInfoList[0].Value = uno::makeAny( getUILocale() );

            aPropInfoList[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.System/L10N/Locale") );
            aPropInfoList[1].Type = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "string" ));
            aPropInfoList[1].Protected = sal_False;
            aPropInfoList[1].Value = uno::makeAny( getLocale() );

            m_xSystemLayer = new LocaleLayer(aPropInfoList, createTimeStamp(), m_xContext);
        }

        return m_xSystemLayer;
    }

    return uno::Reference<backend::XLayer>();
}

//------------------------------------------------------------------------------

uno::Reference<backend::XUpdatableLayer> SAL_CALL
LocaleBackend::getUpdatableLayer(const rtl::OUString& aComponent)
    throw (backend::BackendAccessException,lang::NoSupportException,
           lang::IllegalArgumentException)
{
    throw lang::NoSupportException(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
            "LocaleBackend: No Update Operation allowed, Read Only access") ),
        *this) ;

    return NULL;
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

