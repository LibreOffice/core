/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_unotools.hxx"

#include <unotools/syslocale.hxx>
#include <tools/string.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <i18npool/mslangid.hxx>
#include <rtl/tencinfo.h>
#include <rtl/locale.h>
#include <osl/nlsupport.h>

using namespace osl;
using namespace com::sun::star;


SvtSysLocale_Impl*  SvtSysLocale::pImpl = NULL;
sal_Int32           SvtSysLocale::nRefCount = 0;


class SvtSysLocale_Impl : public utl::ConfigurationListener
{
public:
        SvtSysLocaleOptions     aSysLocaleOptions;
        LocaleDataWrapper*      pLocaleData;
        CharClass*              pCharClass;

                                    SvtSysLocale_Impl();
    virtual                         ~SvtSysLocale_Impl();

    CharClass*                      GetCharClass();
    virtual void                    ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );
};

// -----------------------------------------------------------------------

SvtSysLocale_Impl::SvtSysLocale_Impl() : pCharClass(NULL)
{
    pLocaleData = new LocaleDataWrapper( ::comphelper::getProcessServiceFactory(), aSysLocaleOptions.GetRealLocale() );

    // listen for further changes
    aSysLocaleOptions.AddListener( this );
}


SvtSysLocale_Impl::~SvtSysLocale_Impl()
{
    aSysLocaleOptions.RemoveListener( this );
    delete pCharClass;
    delete pLocaleData;
}

CharClass* SvtSysLocale_Impl::GetCharClass()
{
    if ( !pCharClass )
        pCharClass = new CharClass(::comphelper::getProcessServiceFactory(), aSysLocaleOptions.GetRealLocale() );
    return pCharClass;
}

void SvtSysLocale_Impl::ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 nHint )
{
    MutexGuard aGuard( SvtSysLocale::GetMutex() );
    if ( nHint & SYSLOCALEOPTIONS_HINT_LOCALE )
    {
        com::sun::star::lang::Locale aLocale( aSysLocaleOptions.GetRealLocale() );
        pLocaleData->setLocale( aLocale );
        GetCharClass()->setLocale( aLocale );
    }
}

// ====================================================================

SvtSysLocale::SvtSysLocale()
{
    MutexGuard aGuard( GetMutex() );
    if ( !pImpl )
        pImpl = new SvtSysLocale_Impl;
    ++nRefCount;
}


SvtSysLocale::~SvtSysLocale()
{
    MutexGuard aGuard( GetMutex() );
    if ( !--nRefCount )
    {
        delete pImpl;
        pImpl = NULL;
    }
}


// static
Mutex& SvtSysLocale::GetMutex()
{
    static Mutex* pMutex = NULL;
    if( !pMutex )
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if( !pMutex )
        {
            // #i77768# Due to a static reference in the toolkit lib
            // we need a mutex that lives longer than the svl library.
            // Otherwise the dtor would use a destructed mutex!!
            pMutex = new Mutex;
        }
    }
    return *pMutex;
}


const LocaleDataWrapper& SvtSysLocale::GetLocaleData() const
{
    return *(pImpl->pLocaleData);
}


const LocaleDataWrapper* SvtSysLocale::GetLocaleDataPtr() const
{
    return pImpl->pLocaleData;
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

com::sun::star::lang::Locale SvtSysLocale::GetLocale() const
{
    return pImpl->aSysLocaleOptions.GetRealLocale();
}

LanguageType SvtSysLocale::GetLanguage() const
{
    return pImpl->aSysLocaleOptions.GetRealLanguage();
}

com::sun::star::lang::Locale SvtSysLocale::GetUILocale() const
{
    return pImpl->aSysLocaleOptions.GetRealUILocale();
}

LanguageType SvtSysLocale::GetUILanguage() const
{
    return pImpl->aSysLocaleOptions.GetRealUILanguage();
}

//------------------------------------------------------------------------

// static
rtl_TextEncoding SvtSysLocale::GetBestMimeEncoding()
{
    const sal_Char* pCharSet = rtl_getBestMimeCharsetFromTextEncoding(
            gsl_getSystemTextEncoding() );
    if ( !pCharSet )
    {
        // If the system locale is unknown to us, e.g. LC_ALL=xx, match the UI
        // language if possible.
        ::com::sun::star::lang::Locale aLocale( SvtSysLocale().GetUILocale() );
        rtl_Locale * pLocale = rtl_locale_register( aLocale.Language.getStr(),
                aLocale.Country.getStr(), aLocale.Variant.getStr() );
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
