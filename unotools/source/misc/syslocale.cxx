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
#include <rtl/tencinfo.h>
#include <rtl/locale.h>
#include <osl/thread.h>
#include <osl/nlsupport.h>

#include <memory>
#include <mutex>
#include <optional>
#include <vector>

using namespace com::sun::star;

namespace {

std::weak_ptr<SvtSysLocale_Impl> g_pSysLocale;

// static
std::mutex& GetMutex()
{
    // #i77768# Due to a static reference in the toolkit lib
    // we need a mutex that lives longer than the svl library.
    // Otherwise the dtor would use a destructed mutex!!
    static std::mutex* persistentMutex(new std::mutex);

    return *persistentMutex;
}


}

class SvtSysLocale_Impl : public utl::ConfigurationListener
{
public:
        SvtSysLocaleOptions                    aSysLocaleOptions;
        std::optional<LocaleDataWrapper>       moLocaleData;
        std::optional<CharClass>               moCharClass;

                                SvtSysLocale_Impl();
    virtual                     ~SvtSysLocale_Impl() override;

    CharClass&                  GetCharClass();
    virtual void                ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints ) override;

private:
    std::vector<OUString>       getDateAcceptancePatternsConfig() const;
};

SvtSysLocale_Impl::SvtSysLocale_Impl()
{
    moLocaleData.emplace(
        aSysLocaleOptions.GetRealLanguageTag(),
        getDateAcceptancePatternsConfig() );

    // listen for further changes
    aSysLocaleOptions.AddListener( this );
}

SvtSysLocale_Impl::~SvtSysLocale_Impl()
{
    aSysLocaleOptions.RemoveListener( this );
}

CharClass& SvtSysLocale_Impl::GetCharClass()
{
    if ( !moCharClass )
        moCharClass.emplace( aSysLocaleOptions.GetRealLanguageTag() );
    return *moCharClass;
}

void SvtSysLocale_Impl::ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints nHint )
{
    if ( !(nHint & ConfigurationHints::Locale) &&
         !(nHint & ConfigurationHints::DatePatterns) )
        return;

    std::unique_lock aGuard( GetMutex() );

    const LanguageTag& rLanguageTag = aSysLocaleOptions.GetRealLanguageTag();
    if ( nHint & ConfigurationHints::Locale )
    {
        moCharClass.emplace( rLanguageTag );
    }
    moLocaleData.emplace(rLanguageTag, getDateAcceptancePatternsConfig());
}

std::vector<OUString> SvtSysLocale_Impl::getDateAcceptancePatternsConfig() const
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
    std::unique_lock aGuard( GetMutex() );
    pImpl = g_pSysLocale.lock();
    if ( !pImpl )
    {
        pImpl = std::make_shared<SvtSysLocale_Impl>();
        g_pSysLocale = pImpl;
    }
}

SvtSysLocale::~SvtSysLocale()
{
    std::unique_lock aGuard( GetMutex() );
    pImpl.reset();
}

const LocaleDataWrapper& SvtSysLocale::GetLocaleData() const
{
    return *(pImpl->moLocaleData);
}

const CharClass& SvtSysLocale::GetCharClass() const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
