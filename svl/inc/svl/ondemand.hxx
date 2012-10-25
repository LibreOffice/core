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

#ifndef INCLUDED_SVTOOLS_ONDEMAND_HXX
#define INCLUDED_SVTOOLS_ONDEMAND_HXX

#include <unotools/syslocale.hxx>
#include <i18npool/lang.h>
#include <unotools/localedatawrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <unotools/transliterationwrapper.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <unotools/nativenumberwrapper.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

/*
    On demand instanciation and initialization of several i18n wrappers,
    helping the number formatter to not perform worse than it already does.
 */

/** @short
    Switch between LANGUAGE_SYSTEM and LANGUAGE_ENGLISH_US and any other
    LocaleDataWrapper.
    SvNumberformatter uses it upon switching locales.

    @descr
    Avoids reloading and analysing of locale data again and again.

    @ATTENTION
    If the default ctor is used the init() method MUST be called before
    accessing any locale data. The passed parameters Locale and LanguageType
    must match each other.
 */

class OnDemandLocaleDataWrapper
{
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
            SvtSysLocale        aSysLocale;
            LanguageType        eCurrentLanguage;
            LanguageType        eLastAnyLanguage;
    const   LocaleDataWrapper*  pSystem;
    const   LocaleDataWrapper*  pEnglish;
            LocaleDataWrapper*  pAny;
    const   LocaleDataWrapper*  pCurrent;
            bool                bInitialized;

public:
                                OnDemandLocaleDataWrapper()
                                    : eLastAnyLanguage( LANGUAGE_DONTKNOW )
                                    , pEnglish(0)
                                    , pAny(0)
                                    , bInitialized(false)
                                    {
                                        pCurrent = pSystem = aSysLocale.GetLocaleDataPtr();
                                        eCurrentLanguage = LANGUAGE_SYSTEM;
                                    }
                                OnDemandLocaleDataWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                                    ::com::sun::star::lang::Locale& rLocale,
                                    LanguageType eLang
                                    )
                                    : pEnglish(0)
                                    , pAny(0)
                                    , pCurrent(0)
                                    , bInitialized(false)
                                    {
                                        pSystem = aSysLocale.GetLocaleDataPtr();
                                        init( rxSMgr, rLocale, eLang );
                                    }
                                ~OnDemandLocaleDataWrapper()
                                    {
                                        delete pEnglish;
                                        delete pAny;
                                    }

            bool                isInitialized() const   { return bInitialized; }

            bool                is() const      { return pCurrent != NULL; }

            void                init(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                                    ::com::sun::star::lang::Locale& rLocale,
                                    LanguageType eLang
                                    )
                                    {
                                        xSMgr = rxSMgr;
                                        changeLocale( rLocale, eLang );
                                        bInitialized = true;
                                    }

            void                changeLocale( ::com::sun::star::lang::Locale& rLocale, LanguageType eLang )
                                    {
                                        switch ( eLang )
                                        {
                                            case LANGUAGE_SYSTEM :
                                                pCurrent = pSystem;
                                            break;
                                            case LANGUAGE_ENGLISH_US :
                                                if ( !pEnglish )
                                                    pEnglish = new LocaleDataWrapper( xSMgr, rLocale );
                                                pCurrent = pEnglish;
                                            break;
                                            default:
                                                if ( !pAny )
                                                {
                                                    pAny = new LocaleDataWrapper( xSMgr, rLocale );
                                                    eLastAnyLanguage = eLang;
                                                }
                                                else if ( eLastAnyLanguage != eLang )
                                                {
                                                    pAny->setLocale( rLocale );
                                                    eLastAnyLanguage = eLang;
                                                }
                                                pCurrent = pAny;
                                        }
                                        eCurrentLanguage = eLang;
                                    }

            LanguageType        getCurrentLanguage() const
                                    { return eCurrentLanguage; }

            LocaleDataWrapper*  getAnyLocale()
                                    {
                                        if ( !pAny )
                                        {
                                            pAny = new LocaleDataWrapper( xSMgr, pCurrent->getLocale() );
                                            eLastAnyLanguage = eCurrentLanguage;
                                        }
                                        else if ( pCurrent != pAny )
                                        {
                                            pAny->setLocale( pCurrent->getLocale() );
                                            eLastAnyLanguage = eCurrentLanguage;
                                        }
                                        return pAny;
                                    }

    const   LocaleDataWrapper*  get() const         { return pCurrent; }
    const   LocaleDataWrapper*  operator->() const  { return get(); }
    const   LocaleDataWrapper&  operator*() const   { return *get(); }
};

/** Load a calendar only if it's needed.
    SvNumberformatter uses it upon switching locales.
    @ATTENTION If the default ctor is used the init() method MUST be called
    before accessing the calendar.
 */
class OnDemandCalendarWrapper
{
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
            ::com::sun::star::lang::Locale  aLocale;
    mutable CalendarWrapper*    pPtr;
    mutable bool                bValid;
            bool                bInitialized;

public:
                                OnDemandCalendarWrapper()
                                    : pPtr(0)
                                    , bValid(false)
                                    , bInitialized(false)
                                    {}
                                OnDemandCalendarWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                                    ::com::sun::star::lang::Locale& rLocale
                                    )
                                    : bValid(false)
                                    , bInitialized(false)
                                    {
                                        init( rxContext, rLocale );
                                    }
                                ~OnDemandCalendarWrapper()
                                    {
                                        delete pPtr;
                                    }

            bool                isInitialized() const   { return bInitialized; }

            bool                is() const      { return pPtr != NULL; }

            void                init(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                                    ::com::sun::star::lang::Locale& rLocale
                                    )
                                    {
                                        m_xContext = rxContext;
                                        changeLocale( rLocale );
                                        if ( pPtr )
                                        {
                                            delete pPtr;
                                            pPtr = NULL;
                                        }
                                        bInitialized = true;
                                    }

            void                changeLocale( ::com::sun::star::lang::Locale& rLocale )
                                    {
                                        bValid = false;
                                        aLocale = rLocale;
                                    }

            CalendarWrapper*    get() const
                                    {
                                        if ( !bValid )
                                        {
                                            if ( !pPtr )
                                                pPtr = new CalendarWrapper( m_xContext );
                                            pPtr->loadDefaultCalendar( aLocale );
                                            bValid = true;
                                        }
                                        return pPtr;
                                    }

            CalendarWrapper*    operator->()    { return get(); }
            CalendarWrapper&    operator*()     { return *get(); }
};

/** Load a transliteration only if it's needed.
    SvNumberformatter uses it upon switching locales.
    @ATTENTION If the default ctor is used the init() method MUST be called
    before accessing the transliteration.
 */
class OnDemandTransliterationWrapper
{
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
            LanguageType        eLanguage;
            ::com::sun::star::i18n::TransliterationModules  nType;
    mutable ::utl::TransliterationWrapper*  pPtr;
    mutable bool                bValid;
            bool                bInitialized;

public:
                                OnDemandTransliterationWrapper()
                                    : eLanguage( LANGUAGE_SYSTEM )
                                    , pPtr(0)
                                    , bValid(false)
                                    , bInitialized(false)
                                    {}
                                OnDemandTransliterationWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                                    LanguageType eLang,
                                    ::com::sun::star::i18n::TransliterationModules nTypeP
                                    )
                                    : bValid(false)
                                    , bInitialized(false)
                                    {
                                        init( rxSMgr, eLang, nTypeP );
                                    }
                                ~OnDemandTransliterationWrapper()
                                    {
                                        delete pPtr;
                                    }

            bool                isInitialized() const   { return bInitialized; }

            bool                is() const      { return pPtr != NULL; }

            void                init(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                                    LanguageType eLang,
                                    ::com::sun::star::i18n::TransliterationModules nTypeP
                                    )
                                    {
                                        xSMgr = rxSMgr;
                                        nType = nTypeP;
                                        changeLocale( eLang );
                                        if ( pPtr )
                                        {
                                            delete pPtr;
                                            pPtr = NULL;
                                        }
                                        bInitialized = true;
                                    }

            void                changeLocale( LanguageType eLang )
                                    {
                                        bValid = false;
                                        eLanguage = eLang;
                                    }

    const   ::utl::TransliterationWrapper*  get() const
                                    {
                                        if ( !bValid )
                                        {
                                            if ( !pPtr )
                                                pPtr = new ::utl::TransliterationWrapper( xSMgr, nType );
                                            pPtr->loadModuleIfNeeded( eLanguage );
                                            bValid = true;
                                        }
                                        return pPtr;
                                    }

    const   ::utl::TransliterationWrapper*  getForModule( const String& rModule, LanguageType eLang ) const
                                    {
                                        if ( !pPtr )
                                            pPtr = new ::utl::TransliterationWrapper( xSMgr, nType );
                                        pPtr->loadModuleByImplName( rModule, eLang );
                                        bValid = false; // reforce settings change in get()
                                        return pPtr;
                                    }

    const   ::utl::TransliterationWrapper*  operator->() const  { return get(); }
    const   ::utl::TransliterationWrapper&  operator*() const   { return *get(); }
};

/** Load a native number service wrapper only if it's needed.
    SvNumberformatter uses it.

    @ATTENTION
    If the default ctor is used the init() method MUST be called
    before accessing the native number supplier.
 */
class OnDemandNativeNumberWrapper
{
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    mutable NativeNumberWrapper*    pPtr;
            bool                bInitialized;

public:
                                OnDemandNativeNumberWrapper()
                                    : pPtr(0)
                                    , bInitialized(false)
                                    {}
                                OnDemandNativeNumberWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr
                                    )
                                    : pPtr(0)
                                    , bInitialized(false)
                                    {
                                        init( rxSMgr );
                                    }
                                ~OnDemandNativeNumberWrapper()
                                    {
                                        delete pPtr;
                                    }

            bool                isInitialized() const   { return bInitialized; }

            void                init(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr
                                    )
                                    {
                                        xSMgr = rxSMgr;
                                        if ( pPtr )
                                        {
                                            delete pPtr;
                                            pPtr = NULL;
                                        }
                                        bInitialized = true;
                                    }

            bool                is() const      { return pPtr != NULL; }

            NativeNumberWrapper*    get() const
                                    {
                                        if ( !pPtr )
                                            pPtr = new NativeNumberWrapper( xSMgr );
                                        return pPtr;
                                    }

            NativeNumberWrapper*    operator->()    { return get(); }
            NativeNumberWrapper&    operator*()     { return *get(); }
};

#endif // INCLUDED_SVTOOLS_ONDEMAND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
