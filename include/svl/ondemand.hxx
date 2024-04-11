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

#pragma once

#include <com/sun/star/uno/Reference.hxx>
#include <i18nlangtag/lang.h>
#include <i18nutil/transliteration.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/nativenumberwrapper.hxx>
#include <unotools/charclass.hxx>
#include <optional>

/*
    On demand instantiation and initialization of several i18n wrappers,
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
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    SvtSysLocale aSysLocale;
    LanguageType eCurrentLanguage;
    LanguageType eLastAnyLanguage;
    std::optional<LocaleDataWrapper> moEnglish;
    std::optional<LocaleDataWrapper> moAny;
    int nCurrent; // 0 == system, 1 == english, 2 == any
    bool bInitialized;

public:
    OnDemandLocaleDataWrapper()
        : eLastAnyLanguage(LANGUAGE_DONTKNOW)
        , nCurrent(0)
        , bInitialized(false)
    {
        eCurrentLanguage = LANGUAGE_SYSTEM;
    }

    bool isInitialized() const { return bInitialized; }

    void init(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
              const LanguageTag& rLanguageTag)
    {
        m_xContext = rxContext;
        changeLocale(rLanguageTag);
        bInitialized = true;
    }

    void changeLocale(const LanguageTag& rLanguageTag)
    {
        LanguageType eLang = rLanguageTag.getLanguageType(false);
        if (eLang == LANGUAGE_SYSTEM)
            nCurrent = 0;
        else if (eLang == LANGUAGE_ENGLISH_US)
        {
            if (!moEnglish)
                moEnglish.emplace(m_xContext, rLanguageTag);
            nCurrent = 1;
        }
        else
        {
            if (!moAny)
            {
                moAny.emplace(m_xContext, rLanguageTag);
                eLastAnyLanguage = eLang;
            }
            else if (eLastAnyLanguage != eLang)
            {
                moAny.emplace(m_xContext, rLanguageTag);
                eLastAnyLanguage = eLang;
            }
            nCurrent = 2;
        }
        eCurrentLanguage = eLang;
    }

    LanguageType getCurrentLanguage() const { return eCurrentLanguage; }

    const LocaleDataWrapper* get() const
    {
        switch (nCurrent)
        {
            case 0:
                return &aSysLocale.GetLocaleData();
            case 1:
                return &*moEnglish;
            case 2:
                return &*moAny;
            default:
                assert(false);
                return nullptr;
        }
    }
    const LocaleDataWrapper* operator->() const { return get(); }
    const LocaleDataWrapper& operator*() const { return *get(); }
};

/** Load a calendar only if it's needed. Keep calendar for "en-US" locale
    separately, as there can be alternation between locale dependent and
    locale independent formats.
    SvNumberformatter uses it upon switching locales.

    @ATTENTION If the default ctor is used the init() method MUST be called
    before accessing the calendar.
 */
class OnDemandCalendarWrapper
{
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    css::lang::Locale aEnglishLocale;
    css::lang::Locale aLocale;
    mutable css::lang::Locale aLastAnyLocale;
    mutable std::optional<CalendarWrapper> moEnglish;
    mutable std::optional<CalendarWrapper> moAny;

public:
    OnDemandCalendarWrapper()
    {
        LanguageTag aEnglishLanguageTag(LANGUAGE_ENGLISH_US);
        aEnglishLocale = aEnglishLanguageTag.getLocale();
        aLastAnyLocale = aEnglishLocale;
    }

    void init(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
              const css::lang::Locale& rLocale)
    {
        m_xContext = rxContext;
        changeLocale(rLocale);
        moEnglish.reset();
        moAny.reset();
    }

    void changeLocale(const css::lang::Locale& rLocale) { aLocale = rLocale; }

    CalendarWrapper* get() const
    {
        CalendarWrapper* pPtr;
        if (aLocale == aEnglishLocale)
        {
            if (!moEnglish)
            {
                moEnglish.emplace(m_xContext);
                moEnglish->loadDefaultCalendar(aEnglishLocale);
            }
            pPtr = &*moEnglish;
        }
        else
        {
            if (!moAny)
            {
                moAny.emplace(m_xContext);
                moAny->loadDefaultCalendar(aLocale);
                aLastAnyLocale = aLocale;
            }
            else if (aLocale != aLastAnyLocale)
            {
                moAny->loadDefaultCalendar(aLocale);
                aLastAnyLocale = aLocale;
            }
            pPtr = &*moAny;
        }
        return pPtr;
    }
};

/** Load a transliteration only if it's needed.
    SvNumberformatter uses it upon switching locales.
    @ATTENTION If the default ctor is used the init() method MUST be called
    before accessing the transliteration.
 */
class OnDemandTransliterationWrapper
{
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    LanguageType eLanguage;
    TransliterationFlags nType;
    mutable std::optional<::utl::TransliterationWrapper> moTransliterate;
    mutable bool bValid;
    bool bInitialized;

public:
    OnDemandTransliterationWrapper()
        : eLanguage(LANGUAGE_SYSTEM)
        , nType(TransliterationFlags::NONE)
        , bValid(false)
        , bInitialized(false)
    {
    }

    bool isInitialized() const { return bInitialized; }

    void init(const css::uno::Reference<css::uno::XComponentContext>& rxContext, LanguageType eLang)
    {
        m_xContext = rxContext;
        nType = TransliterationFlags::IGNORE_CASE;
        changeLocale(eLang);
        moTransliterate.reset();
        bInitialized = true;
    }

    void changeLocale(LanguageType eLang)
    {
        bValid = false;
        eLanguage = eLang;
    }

    const ::utl::TransliterationWrapper* get() const
    {
        if (!bValid)
        {
            if (!moTransliterate)
                moTransliterate.emplace(m_xContext, nType);
            moTransliterate->loadModuleIfNeeded(eLanguage);
            bValid = true;
        }
        return &*moTransliterate;
    }

    const ::utl::TransliterationWrapper* operator->() const { return get(); }
};

/** Load a native number service wrapper only if it's needed.
    SvNumberformatter uses it.
 */
class OnDemandNativeNumberWrapper
{
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    mutable std::optional<NativeNumberWrapper> moNativeNumber;

public:
    OnDemandNativeNumberWrapper(const css::uno::Reference<css::uno::XComponentContext>& rContext)
        : m_xContext(rContext)
    {
    }

    NativeNumberWrapper& get() const
    {
        if (!moNativeNumber)
            moNativeNumber.emplace(m_xContext);
        return *moNativeNumber;
    }
};

/** @short
    SvNumberformatter uses it upon switching locales.

    @descr
    Avoids reloading and analysing of locale data again and again.

    @ATTENTION
    If the default ctor is used the init() method MUST be called before
    accessing any locale data.
 */

class OnDemandCharClass
{
    std::optional<CharClass> moCharClass1;
    std::optional<CharClass> moCharClass2;
    int nCurrent; // -1 == uninitialised, 0 == class1, 1 == class2

public:
    OnDemandCharClass()
        : nCurrent(-1)
    {
    }

    void changeLocale(const css::uno::Reference<css::uno::XComponentContext>& xContext,
                      const LanguageTag& rLanguageTag)
    {
        // check for existing match
        if (moCharClass1 && moCharClass1->getLanguageTag() == rLanguageTag)
        {
            nCurrent = 0;
            return;
        }
        if (moCharClass2 && moCharClass2->getLanguageTag() == rLanguageTag)
        {
            nCurrent = 1;
            return;
        }
        // no match - update one the current entries
        if (nCurrent == -1 || nCurrent == 1)
        {
            moCharClass1.emplace(xContext, rLanguageTag);
            nCurrent = 0;
        }
        else
        {
            moCharClass2.emplace(xContext, rLanguageTag);
            nCurrent = 1;
        }
    }

    const CharClass* get() const
    {
        switch (nCurrent)
        {
            case 0:
                return &*moCharClass1;
            case 1:
                return &*moCharClass2;
            default:
                assert(false);
                return nullptr;
        }
    }
    const CharClass* operator->() const { return get(); }
    const CharClass& operator*() const { return *get(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
