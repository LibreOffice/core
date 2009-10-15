/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: syslocale.cxx,v $
 * $Revision: 1.11 $
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
#ifndef GCC
#endif

#include <unotools/syslocale.hxx>
#include <tools/string.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <i18npool/mslangid.hxx>

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
        com::sun::star::lang::Locale maLocale;
        com::sun::star::lang::Locale maUILocale;
        LanguageType            meLanguage;
        LanguageType            meUILanguage;

                                    SvtSysLocale_Impl();
    virtual                         ~SvtSysLocale_Impl();

    CharClass*                      GetCharClass();
    virtual void                    ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );
    void                            GetLocale();
    void                            GetUILocale();
};

void SvtSysLocale_Impl::GetLocale()
{
    // ask configuration
    maLocale = aSysLocaleOptions.GetLocale();
    if ( maLocale.Language.getLength() )
    {
        meLanguage = MsLangId::convertLocaleToLanguage( maLocale );
    }
    else
    {
        meLanguage = MsLangId::getSystemLanguage();
        MsLangId::convertLanguageToLocale( meLanguage, maLocale );
    }
}

void SvtSysLocale_Impl::GetUILocale()
{
    maLocale = aSysLocaleOptions.GetLocale();
    if ( maUILocale.Language.getLength() )
    {
        meUILanguage = MsLangId::convertLocaleToLanguage( maUILocale );
    }
    else
    {
        meUILanguage = MsLangId::getSystemUILanguage();
        MsLangId::convertLanguageToLocale( meUILanguage, maUILocale );
    }
}

// -----------------------------------------------------------------------

SvtSysLocale_Impl::SvtSysLocale_Impl() : pCharClass(NULL)
{
    GetLocale();
    GetUILocale();

    pLocaleData = new LocaleDataWrapper( ::comphelper::getProcessServiceFactory(), maLocale );

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
        pCharClass = new CharClass(::comphelper::getProcessServiceFactory(), maLocale );
    return pCharClass;
}

void SvtSysLocale_Impl::ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 nHint )
{
    MutexGuard aGuard( SvtSysLocale::GetMutex() );
    if ( nHint & SYSLOCALEOPTIONS_HINT_LOCALE )
    {
        GetLocale();
        pLocaleData->setLocale( maLocale );
        GetCharClass()->setLocale( maLocale );
    }

    if ( nHint & SYSLOCALEOPTIONS_HINT_UILOCALE )
        GetUILocale();
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
    return pImpl->maLocale;
}

LanguageType SvtSysLocale::GetLanguage() const
{
    return pImpl->meLanguage;
}

com::sun::star::lang::Locale SvtSysLocale::GetUILocale() const
{
    return pImpl->maUILocale;
}

LanguageType SvtSysLocale::GetUILanguage() const
{
    return pImpl->meUILanguage;
}


