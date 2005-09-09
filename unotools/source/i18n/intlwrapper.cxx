/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: intlwrapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:44:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#pragma hdrstop

#include "unotools/intlwrapper.hxx"

#ifndef _COM_SUN_STAR_I18N_COLLATOROPTIONS_HPP_
#include <com/sun/star/i18n/CollatorOptions.hpp>
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

IntlWrapper::IntlWrapper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
            const ::com::sun::star::lang::Locale& rLocale )
        :
        xSMgr( xSF ),
        aLocale( rLocale ),
        pCharClass( NULL ),
        pLocaleData( NULL ),
        pCalendar( NULL ),
        pCollator( NULL ),
        pCaseCollator( NULL )
{
    eLanguage = ConvertIsoNamesToLanguage( aLocale.Language, aLocale.Country );
}


IntlWrapper::IntlWrapper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
            LanguageType eLang )
        :
        xSMgr( xSF ),
        eLanguage( eLang ),
        pCharClass( NULL ),
        pLocaleData( NULL ),
        pCalendar( NULL ),
        pCollator( NULL ),
        pCaseCollator( NULL )
{
    String aLanguage, aCountry;
    ConvertLanguageToIsoNames( eLanguage, aLanguage, aCountry );
    aLocale.Language = aLanguage;
    aLocale.Country = aCountry;
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


void IntlWrapper::ImplNewCollator( BOOL bCaseSensitive ) const
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
