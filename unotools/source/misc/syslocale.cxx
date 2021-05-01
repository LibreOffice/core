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
#include <unotools/localedatawrapper.hxx>
#include <unotools/charclass.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/sequence.hxx>
#include <rtl/tencinfo.h>
#include <rtl/locale.h>
#include <osl/thread.h>
#include <osl/nlsupport.h>

#include <vector>
#include <memory>

using namespace osl;
using namespace com::sun::star;

namespace {

std::weak_ptr<SvtSysLocale_Impl> g_pSysLocale;

}

class SvtSysLocale_Impl : public utl::ConfigurationListener
{
public:
        SvtSysLocaleOptions                    aSysLocaleOptions;
        std::unique_ptr<LocaleDataWrapper>      pLocaleData;
        std::unique_ptr<CharClass>              pCharClass;

                                SvtSysLocale_Impl();
    virtual                     ~SvtSysLocale_Impl() override;

    CharClass*                  GetCharClass();
    virtual void                ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints ) override;

private:
    std::vector<OUString>       getDateAcceptancePatternsConfig();
};

SvtSysLocale_Impl::SvtSysLocale_Impl()
{
    pLocaleData.reset(new LocaleDataWrapper(
        aSysLocaleOptions.GetRealLanguageTag(),
        getDateAcceptancePatternsConfig() ));

    // listen for further changes
    aSysLocaleOptions.AddListener( this );
}

SvtSysLocale_Impl::~SvtSysLocale_Impl()
{
    aSysLocaleOptions.RemoveListener( this );
}

CharClass* SvtSysLocale_Impl::GetCharClass()
{
    if ( !pCharClass )
        pCharClass.reset(new CharClass( aSysLocaleOptions.GetRealLanguageTag() ));
    return pCharClass.get();
}

void SvtSysLocale_Impl::ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints nHint )
{
    if ( !(nHint & ConfigurationHints::Locale) &&
         !(nHint & ConfigurationHints::DatePatterns) )
        return;

    MutexGuard aGuard( SvtSysLocale::GetMutex() );

    const LanguageTag& rLanguageTag = aSysLocaleOptions.GetRealLanguageTag();
    if ( nHint & ConfigurationHints::Locale )
    {
        GetCharClass()->setLanguageTag( rLanguageTag );
    }
    pLocaleData.reset(new LocaleDataWrapper(rLanguageTag, getDateAcceptancePatternsConfig()));
}

std::vector<OUString> SvtSysLocale_Impl::getDateAcceptancePatternsConfig()
{
    OUString aStr( aSysLocaleOptions.GetDatePatternsConfigString());
    if (aStr.isEmpty())
        return {};  // reset
    ::std::vector< OUString > aVec;
    for (sal_Int32 nIndex = 0; nIndex >= 0; /*nop*/)
    {
        OUString aTok( aStr.getToken( 0, ';', nIndex));
        if (!aTok.isEmpty())
            aVec.push_back( aTok);
    }
    return aVec;
}

SvtSysLocale::SvtSysLocale()
{
    MutexGuard aGuard( GetMutex() );
    pImpl = g_pSysLocale.lock();
    if ( !pImpl )
    {
        pImpl = std::make_shared<SvtSysLocale_Impl>();
        g_pSysLocale = pImpl;
    }
}

SvtSysLocale::~SvtSysLocale()
{
    MutexGuard aGuard( GetMutex() );
    pImpl.reset();
}

// static
Mutex& SvtSysLocale::GetMutex()
{
    // #i77768# Due to a static reference in the toolkit lib
    // we need a mutex that lives longer than the svl library.
    // Otherwise the dtor would use a destructed mutex!!
    static Mutex* persistentMutex(new Mutex);

    return *persistentMutex;
}

const LocaleDataWrapper& SvtSysLocale::GetLocaleData() const
{
    return *(pImpl->pLocaleData);
}

const CharClass& SvtSysLocale::GetCharClass() const
{
    return *(pImpl->GetCharClass());
}

const CharClass* SvtSysLocale::GetCharClassPtr() const
{
    return pImpl->GetCharClass();
}

SvtSysLocaleOptions& SvtSysLocale::GetOptions() const
{
    return pImpl->aSysLocaleOptions;
}

const LanguageTag& SvtSysLocale::GetLanguageTag() const
{
    if (comphelper::LibreOfficeKit::isActive())
        return comphelper::LibreOfficeKit::getLocale();

    return pImpl->aSysLocaleOptions.GetRealLanguageTag();
}

const LanguageTag& SvtSysLocale::GetUILanguageTag() const
{
    if (comphelper::LibreOfficeKit::isActive())
        return comphelper::LibreOfficeKit::getLanguageTag();

    return pImpl->aSysLocaleOptions.GetRealUILanguageTag();
}

// static
rtl_TextEncoding SvtSysLocale::GetBestMimeEncoding()
{
    const char* pCharSet = rtl_getBestMimeCharsetFromTextEncoding(
            osl_getThreadTextEncoding() );
    if ( !pCharSet )
    {
        // If the system locale is unknown to us, e.g. LC_ALL=xx, match the UI
        // language if possible.
        SvtSysLocale aSysLocale;
        const LanguageTag& rLanguageTag = aSysLocale.GetUILanguageTag();
        // Converting blindly to Locale and then to rtl_Locale may feed the
        // 'qlt' to rtl_locale_register() and the underlying system locale
        // stuff, which doesn't know about it nor about BCP47 in the Variant
        // field. So use the real language and for non-pure ISO cases hope for
        // the best... the fallback to UTF-8 should solve these cases nowadays.
        /* FIXME-BCP47: the script needs to go in here as well, so actually
         * we'd need some variant fiddling or glibc locale string and tweak
         * rtl_locale_register() to know about it! But then again the Windows
         * implementation still wouldn't know anything about it ... */
        SAL_WARN_IF( !rLanguageTag.isIsoLocale(), "unotools.i18n",
                "SvtSysLocale::GetBestMimeEncoding - non-ISO UI locale");
        rtl_Locale * pLocale = rtl_locale_register( rLanguageTag.getLanguage().getStr(),
                rLanguageTag.getCountry().getStr(), OUString().getStr() );
        rtl_TextEncoding nEnc = osl_getTextEncodingFromLocale( pLocale );
        pCharSet = rtl_getBestMimeCharsetFromTextEncoding( nEnc );
    }
    rtl_TextEncoding nRet;
    if ( pCharSet )
        nRet = rtl_getTextEncodingFromMimeCharset( pCharSet );
    else
        nRet = RTL_TEXTENCODING_UTF8;
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
