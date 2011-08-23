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

#ifndef _SVX_UNOFORBIDDENCHARSTABLE_HXX_
#include "UnoForbiddenCharsTable.hxx"
#endif

#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include "forbiddencharacterstable.hxx"
#endif

#ifndef _VOS_MUTEX_HXX_ 
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX 
#include <vcl/svapp.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include "unolingu.hxx"
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;

SvxUnoForbiddenCharsTable::SvxUnoForbiddenCharsTable(ORef<SvxForbiddenCharactersTable> xForbiddenChars) :
    mxForbiddenChars( xForbiddenChars )
{
}

SvxUnoForbiddenCharsTable::~SvxUnoForbiddenCharsTable()
{
}

void SvxUnoForbiddenCharsTable::onChange()
{
}

ForbiddenCharacters SvxUnoForbiddenCharsTable::getForbiddenCharacters( const Locale& rLocale )
    throw(NoSuchElementException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(!mxForbiddenChars.isValid())
        throw RuntimeException();

    const LanguageType eLang = SvxLocaleToLanguage( rLocale );
    const ForbiddenCharacters* pForbidden = mxForbiddenChars->GetForbiddenCharacters( eLang, FALSE );
    if(!pForbidden)
        throw NoSuchElementException();

    return *pForbidden;
}

sal_Bool SvxUnoForbiddenCharsTable::hasForbiddenCharacters( const Locale& rLocale )
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(!mxForbiddenChars.isValid())
        return sal_False;

    const LanguageType eLang = SvxLocaleToLanguage( rLocale );
    const ForbiddenCharacters* pForbidden = mxForbiddenChars->GetForbiddenCharacters( eLang, FALSE );

    return NULL != pForbidden;
}

void SvxUnoForbiddenCharsTable::setForbiddenCharacters(const Locale& rLocale, const ForbiddenCharacters& rForbiddenCharacters )
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(!mxForbiddenChars.isValid())
        throw RuntimeException();

    const LanguageType eLang = SvxLocaleToLanguage( rLocale );
    mxForbiddenChars->SetForbiddenCharacters( eLang, rForbiddenCharacters );

    onChange();
}

void SvxUnoForbiddenCharsTable::removeForbiddenCharacters( const Locale& rLocale )
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(!mxForbiddenChars.isValid())
        throw RuntimeException();

    const LanguageType eLang = SvxLocaleToLanguage( rLocale );
    mxForbiddenChars->ClearForbiddenCharacters( eLang );

    onChange();
}

// XSupportedLocales
Sequence< Locale > SAL_CALL SvxUnoForbiddenCharsTable::getLocales()
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const sal_Int32 nCount = mxForbiddenChars.isValid() ? mxForbiddenChars->Count() : 0;

    Sequence< Locale > aLocales( nCount );
    if( nCount )
    {
        Locale* pLocales = aLocales.getArray();

        for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
        {
            const ULONG nLanguage = mxForbiddenChars->GetObjectKey( nIndex );
            SvxLanguageToLocale ( *pLocales++, static_cast < LanguageType > (nLanguage) );
        }
    }

    return aLocales;
}

sal_Bool SAL_CALL SvxUnoForbiddenCharsTable::hasLocale( const Locale& aLocale )
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    return hasForbiddenCharacters( aLocale );
}
}
