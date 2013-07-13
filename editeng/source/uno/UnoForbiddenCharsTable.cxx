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

#include <editeng/UnoForbiddenCharsTable.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <editeng/unolingu.hxx> // LocalToLanguage, LanguageToLocale

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::rtl;
using namespace ::cppu;

SvxUnoForbiddenCharsTable::SvxUnoForbiddenCharsTable(::rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars) :
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

    const LanguageType eLang = LanguageTag::convertToLanguageType( rLocale );
    const ForbiddenCharacters* pForbidden = mxForbiddenChars->GetForbiddenCharacters( eLang, sal_False );
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

    const LanguageType eLang = LanguageTag::convertToLanguageType( rLocale );
    const ForbiddenCharacters* pForbidden = mxForbiddenChars->GetForbiddenCharacters( eLang, sal_False );

    return NULL != pForbidden;
}

void SvxUnoForbiddenCharsTable::setForbiddenCharacters(const Locale& rLocale, const ForbiddenCharacters& rForbiddenCharacters )
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!mxForbiddenChars.is())
        throw RuntimeException();

    const LanguageType eLang = LanguageTag::convertToLanguageType( rLocale );
    mxForbiddenChars->SetForbiddenCharacters( eLang, rForbiddenCharacters );

    onChange();
}

void SvxUnoForbiddenCharsTable::removeForbiddenCharacters( const Locale& rLocale )
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!mxForbiddenChars.is())
        throw RuntimeException();

    const LanguageType eLang = LanguageTag::convertToLanguageType( rLocale );
    mxForbiddenChars->ClearForbiddenCharacters( eLang );

    onChange();
}

// XSupportedLocales
Sequence< Locale > SAL_CALL SvxUnoForbiddenCharsTable::getLocales()
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Int32 nCount = mxForbiddenChars.is() ? mxForbiddenChars->GetMap().size() : 0;

    Sequence< Locale > aLocales( nCount );
    if( nCount )
    {
        Locale* pLocales = aLocales.getArray();

        for( SvxForbiddenCharactersTable::Map::iterator it = mxForbiddenChars->GetMap().begin();
             it != mxForbiddenChars->GetMap().end(); ++it )
        {
            const sal_uLong nLanguage = it->first;
            *pLocales++ = LanguageTag( static_cast < LanguageType > (nLanguage) ).getLocale();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
