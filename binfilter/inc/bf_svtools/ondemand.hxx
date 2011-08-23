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

#ifndef INCLUDED_SVTOOLS_ONDEMAND_HXX
#define INCLUDED_SVTOOLS_ONDEMAND_HXX

#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <bf_svtools/syslocale.hxx>
#endif

#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#include <unotools/calendarwrapper.hxx>
#endif
#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#include <unotools/collatorwrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_COLLATOROPTIONS_HPP_
#include <com/sun/star/i18n/CollatorOptions.hpp>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HPP_
#include <com/sun/star/i18n/TransliterationModules.hpp>
#endif
#ifndef _UNOTOOLS_NATIVENUMBERWRAPPER_HXX
#include <unotools/nativenumberwrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace binfilter
{

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
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
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
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                                    ::com::sun::star::lang::Locale& rLocale
                                    )
                                    : bValid(false)
                                    , bInitialized(false)
                                    {
                                        init( rxSMgr, rLocale );
                                    }
                                ~OnDemandCalendarWrapper()
                                    {
                                        delete pPtr;
                                    }

            bool                isInitialized() const   { return bInitialized; }

            bool                is() const      { return pPtr != NULL; }

            void                init(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                                    ::com::sun::star::lang::Locale& rLocale
                                    )
                                    {
                                        xSMgr = rxSMgr;
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
                                                pPtr = new CalendarWrapper( xSMgr );
                                            pPtr->loadDefaultCalendar( aLocale );
                                            bValid = true;
                                        }
                                        return pPtr;
                                    }

            CalendarWrapper*    operator->()    { return get(); }
            CalendarWrapper&    operator*()     { return *get(); }
};

/** Load a collator only if it's needed.
    SvNumberformatter uses it upon switching locales.
    @ATTENTION If the default ctor is used the init() method MUST be called
    before accessing the collator.
 */
class OnDemandCollatorWrapper
{
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
            ::com::sun::star::lang::Locale  aLocale;
    mutable CollatorWrapper*    pPtr;
    mutable bool                bValid;
            bool                bInitialized;

public:
                                OnDemandCollatorWrapper()
                                    : pPtr(0)
                                    , bValid(false)
                                    , bInitialized(false)
                                    {}
                                OnDemandCollatorWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                                    ::com::sun::star::lang::Locale& rLocale
                                    )
                                    : bValid(false)
                                    , bInitialized(false)
                                    {
                                        init( rxSMgr, rLocale );
                                    }
                                ~OnDemandCollatorWrapper()
                                    {
                                        delete pPtr;
                                    }

            bool                isInitialized() const   { return bInitialized; }

            bool                is() const      { return pPtr != NULL; }

            void                init(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                                    ::com::sun::star::lang::Locale& rLocale
                                    )
                                    {
                                        xSMgr = rxSMgr;
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

    const   CollatorWrapper*    get() const
                                    {
                                        if ( !bValid )
                                        {
                                            if ( !pPtr )
                                                pPtr = new CollatorWrapper( xSMgr );
                                            pPtr->loadDefaultCollator( aLocale, ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE );
                                            bValid = true;
                                        }
                                        return pPtr;
                                    }

    const   CollatorWrapper*    operator->() const  { return get(); }
    const   CollatorWrapper&    operator*() const   { return *get(); }
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

}

#endif // INCLUDED_SVTOOLS_ONDEMAND_HXX

