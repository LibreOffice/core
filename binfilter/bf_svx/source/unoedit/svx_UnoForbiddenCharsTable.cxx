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

#include "UnoForbiddenCharsTable.hxx"

#include "forbiddencharacterstable.hxx"

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ref.hxx>

#include "unolingu.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::cppu;

SvxUnoForbiddenCharsTable::SvxUnoForbiddenCharsTable(rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars) :
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
    SolarMutexGuard aGuard;

    if(!mxForbiddenChars.is())
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
    SolarMutexGuard aGuard;

    if(!mxForbiddenChars.is())
        return sal_False;

    const LanguageType eLang = SvxLocaleToLanguage( rLocale );
    const ForbiddenCharacters* pForbidden = mxForbiddenChars->GetForbiddenCharacters( eLang, FALSE );

    return NULL != pForbidden;
}

void SvxUnoForbiddenCharsTable::setForbiddenCharacters(const Locale& rLocale, const ForbiddenCharacters& rForbiddenCharacters )
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!mxForbiddenChars.is())
        throw RuntimeException();

    const LanguageType eLang = SvxLocaleToLanguage( rLocale );
    mxForbiddenChars->SetForbiddenCharacters( eLang, rForbiddenCharacters );

    onChange();
}

void SvxUnoForbiddenCharsTable::removeForbiddenCharacters( const Locale& rLocale )
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!mxForbiddenChars.is())
        throw RuntimeException();

    const LanguageType eLang = SvxLocaleToLanguage( rLocale );
    mxForbiddenChars->ClearForbiddenCharacters( eLang );

    onChange();
}

// XSupportedLocales
Sequence< Locale > SAL_CALL SvxUnoForbiddenCharsTable::getLocales()
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Int32 nCount = mxForbiddenChars.is() ? mxForbiddenChars->Count() : 0;

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
    SolarMutexGuard aGuard;

    return hasForbiddenCharacters( aLocale );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
