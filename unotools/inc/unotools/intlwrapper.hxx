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
#include "unotools/unotoolsdllapi.h"

#ifndef _UNOTOOLS_INTLWRAPPER_HXX
#define _UNOTOOLS_INTLWRAPPER_HXX
#include <unotools/charclass.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <i18npool/lang.h>


/**
    A wrapper of I18N wrappers. Using this is more expensive than using some
    single wrapper classes so use it only if you must pass a single pointer
    without knowing in advance what is needed, e.g. for
    SfxPoolItem::GetPresentation(). Remember that this wrapper was only created
    for convenience to bypass some oddities, if possible don't use it. <p>

    Implemented are only the const get...() methods of the wrappers, which are
    loaded on demand, for consistency reasons no change of locale is possible.
    Only default calendar and default collator are supported. <p>

    One exception though is the calendar wrapper: to be able to set a value and
    retrieve calendar values it is not const, so methods using this should
    reset the calendar to the previous value if it isn't sure where the
    IntlWrapper did come from. <p>
 */
class UNOTOOLS_DLLPUBLIC IntlWrapper
{
private:

    ::com::sun::star::lang::Locale  aLocale;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;

            CharClass*          pCharClass;
            LocaleDataWrapper*  pLocaleData;
            CalendarWrapper*    pCalendar;
            CollatorWrapper*    pCollator;
            CollatorWrapper*    pCaseCollator;

            LanguageType        eLanguage;

            void                ImplNewCharClass() const;
            void                ImplNewLocaleData() const;
            void                ImplNewCalendar() const;
            void                ImplNewCollator( sal_Bool bCaseSensitive ) const;


public:
                                IntlWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
                                    const ::com::sun::star::lang::Locale& rLocale
                                    );
                                IntlWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
                                    LanguageType eLang
                                    );
                                ~IntlWrapper();

    LanguageType                getLanguage() const { return eLanguage; }
    const ::com::sun::star::lang::Locale&   getLocale() const { return aLocale; }

    const CharClass*            getCharClass() const
                                    {
                                        if ( !pCharClass )
                                            ImplNewCharClass();
                                        return pCharClass;
                                    }
    const LocaleDataWrapper*    getLocaleData() const
                                    {
                                        if ( !pLocaleData )
                                            ImplNewLocaleData();
                                        return pLocaleData;
                                    }
    CalendarWrapper*            getCalendar() const
                                    {
                                        if ( !pCalendar )
                                            ImplNewCalendar();
                                        return pCalendar;
                                    }
    /// case insensitive collator, simple IGNORE_CASE
    const CollatorWrapper*      getCollator() const
                                    {
                                        if ( !pCollator )
                                            ImplNewCollator( sal_False );
                                        return pCollator;
                                    }
    /// case sensitive collator
    const CollatorWrapper*      getCaseCollator() const
                                    {
                                        if ( !pCaseCollator )
                                            ImplNewCollator( sal_True );
                                        return pCaseCollator;
                                    }
};

#endif // _UNOTOOLS_INTLWRAPPER_HXX
