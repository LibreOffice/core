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
