/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"
#include <editeng/UnoForbiddenCharsTable.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <editeng/unolingu.hxx> // LocalToLanguage, LanguageToLocale

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
    const ForbiddenCharacters* pForbidden = mxForbiddenChars->GetForbiddenCharacters( eLang, sal_False );
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
    const ForbiddenCharacters* pForbidden = mxForbiddenChars->GetForbiddenCharacters( eLang, sal_False );

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
            const sal_uLong nLanguage = mxForbiddenChars->GetObjectKey( nIndex );
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
