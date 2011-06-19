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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
