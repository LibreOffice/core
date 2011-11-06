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
#include "precompiled_unotools.hxx"

#include "unotools/intlwrapper.hxx"
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <i18npool/mslangid.hxx>

IntlWrapper::IntlWrapper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
            const ::com::sun::star::lang::Locale& rLocale )
        :
        aLocale( rLocale ),
        xSMgr( xSF ),
        pCharClass( NULL ),
        pLocaleData( NULL ),
        pCalendar( NULL ),
        pCollator( NULL ),
        pCaseCollator( NULL )
{
    eLanguage = MsLangId::convertLocaleToLanguage( aLocale );
}


IntlWrapper::IntlWrapper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
            LanguageType eLang )
        :
        xSMgr( xSF ),
        pCharClass( NULL ),
        pLocaleData( NULL ),
        pCalendar( NULL ),
        pCollator( NULL ),
        pCaseCollator( NULL ),
        eLanguage( eLang )
{
    MsLangId::convertLanguageToLocale( eLanguage, aLocale );
}


IntlWrapper::~IntlWrapper()
{
    delete pCharClass;
    delete pLocaleData;
    delete pCalendar;
    delete pCollator;
    delete pCaseCollator;
}


void IntlWrapper::ImplNewCharClass() const
{
    ((IntlWrapper*)this)->pCharClass = new CharClass( xSMgr, aLocale );
}


void IntlWrapper::ImplNewLocaleData() const
{
    ((IntlWrapper*)this)->pLocaleData = new LocaleDataWrapper( xSMgr, aLocale );
}


void IntlWrapper::ImplNewCalendar() const
{
    CalendarWrapper* p = new CalendarWrapper( xSMgr );
    p->loadDefaultCalendar( aLocale );
    ((IntlWrapper*)this)->pCalendar = p;
}


void IntlWrapper::ImplNewCollator( sal_Bool bCaseSensitive ) const
{
    CollatorWrapper* p = new CollatorWrapper( xSMgr );
    if ( bCaseSensitive )
    {
        p->loadDefaultCollator( aLocale, 0 );
        ((IntlWrapper*)this)->pCaseCollator = p;
    }
    else
    {
        p->loadDefaultCollator( aLocale, ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE );
        ((IntlWrapper*)this)->pCollator = p;
    }
}
