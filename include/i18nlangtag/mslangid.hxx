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

#ifndef INCLUDED_I18NLANGTAG_MSLANGID_HXX
#define INCLUDED_I18NLANGTAG_MSLANGID_HXX

#include <sal/config.h>

#include "i18nlangtag/i18nlangtagdllapi.h"
#include "i18nlangtag/lang.h"
#include <com/sun/star/lang/Locale.hpp>

/** Methods related to Microsoft language IDs. For details about MS-LANGIDs
    please see lang.h */
class I18NLANGTAG_DLLPUBLIC MsLangId
{
public:

    /// Create a LangID from a primary and a sublanguage.
    static inline LanguageType makeLangID( LanguageType nSubLangId, LanguageType nPriLangId)
    {
        return (nSubLangId << 10) | nPriLangId;
    }

    /// Get the primary language of a LangID.
    static inline LanguageType getPrimaryLanguage( LanguageType nLangID)
    {
        return nLangID & LANGUAGE_MASK_PRIMARY;
    }

    /// Get the sublanguage of a LangID.
    static inline LanguageType getSubLanguage( LanguageType nLangID)
    {
        return (nLangID & ~LANGUAGE_MASK_PRIMARY) >> 10;
    }

    /** Language/locale of category LC_CTYPE (on Unix, else the system
        language).
        Evaluation order: LC_ALL, LC_CTYPE, LANG */
    static LanguageType getSystemLanguage();

    /** Language/locale of category LC_MESSAGES (on Unix, else same as
        GetSystemLanguage()).
        Evaluation order: LANGUAGE, LC_ALL, LC_MESSAGES, LANG */
    static LanguageType getSystemUILanguage();


    /** @short: A proper language/locale if the nLang parameter designates some
                special value.

        @descr: NOTE: The "system" values may be overridden by the
                application's configuration.

        @returns
            case LANGUAGE_PROCESS_OR_USER_DEFAULT :     configured or system language
            case LANGUAGE_SYSTEM_DEFAULT :              configured or system language
            case LANGUAGE_SYSTEM :                      configured or system language
            case LANGUAGE_HID_HUMAN_INTERFACE_DEVICE :  configured or system UI language
            case LANGUAGE_DONTKNOW :                    LANGUAGE_ENGLISH_US
            else: nLang

            In case the configured language is LANGUAGE_SYSTEM, which is also
            the initial default, the system language is obtained. In case the
            configured or resulting system language is LANGUAGE_DONTKNOW,
            LANGUAGE_ENGLISH_US is returned instead.
      */
    static LanguageType getRealLanguage( LanguageType nLang );


    // TODO: refactor to LanguageTag? Used only in
    // i18npool/source/localedata/localedata.cxx

    /** Get fall-back Locale for Locale with handling of an empty language name
        designating the SYSTEM language. Returns the same Locale if an exact
        match was found.
      */
    static ::com::sun::star::lang::Locale getFallbackLocale(
            const ::com::sun::star::lang::Locale & rLocale );


    // TODO: refactor to LanguageTag, used only in
    // i18npool/source/isolang/inunx.cxx to convert Unix locale string

    static LanguageType convertUnxByteStringToLanguage( const OString& rString );


    static LanguageType resolveSystemLanguageByScriptType( LanguageType nLang, sal_Int16 nType );


    /** Whether locale has a Right-To-Left orientation. */
    static bool isRightToLeft( LanguageType nLang );

    /** Whether locale is a CJK locale */
    static bool isCJK( LanguageType nLang );

    /** Whether locale is a chinese locale */
    static bool isChinese( LanguageType nLang );

    /** Whether locale is a simplified chinese locale */
    static bool isSimplifiedChinese( LanguageType nLang );

    /** Whether locale is a traditional chinese locale */
    static bool isTraditionalChinese( LanguageType nLang );

    /** Whether locale is a korean locale */
    static bool isKorean( LanguageType nLang );

    /** Whether locale is a simplified chinese locale */
    static bool isSimplifiedChinese( const ::com::sun::star::lang::Locale & rLocale );

    /** Whether locale is a traditional chinese locale */
    static bool isTraditionalChinese( const ::com::sun::star::lang::Locale & rLocale );

    /** Whether locale is one where family name comes first, e.g. Japan, Hungary, Samoa */
    static bool isFamilyNameFirst( LanguageType nLang );

    /** Whether there are "forbidden characters at start or end of line" in
        this locale. CJK locales.

        @see offapi/com/sun/star/i18n/ForbiddenCharacters.idl
      */
    static bool hasForbiddenCharacters( LanguageType nLang );


    /** Whether locale needs input sequence checking. CTL locales. */
    static bool needsSequenceChecking( LanguageType nLang );


    /** Get ::com::sun::star::i18n::ScriptType of locale. */
    static sal_Int16 getScriptType( LanguageType nLang );


    /** Map an obsolete user defined LANGID (see lang.h
        LANGUAGE_OBSOLETE_USER_...) to the new value defined by MS in the
        meantime.

        Also used to map UI localizations using reserved ISO codes to something
        "official" but not identical in order to not pollute documents with
        invalid ISO codes.

        @param bUserInterfaceSelection
            If TRUE, don't replace such UI-only locale. Only use for
                     Tools->Options->LanguageSettings->UserInterface listbox.
            If FALSE, do replace.
     */
    static LanguageType getReplacementForObsoleteLanguage( LanguageType nLang,
            bool bUserInterfaceSelection = false );


    /** @ATTENTION: these are _ONLY_ to be called by the application's
        configuration! */
    static void setConfiguredSystemLanguage( LanguageType nLang );
    static void setConfiguredSystemUILanguage( LanguageType nLang );
    static void setConfiguredWesternFallback( LanguageType nLang );
    static void setConfiguredComplexFallback( LanguageType nLang );
    static void setConfiguredAsianFallback( LanguageType nLang );

// ---------------------------------------------------------------------------

    /** @internal - Access to fields of an element of the simple conversion table.
        For resource compiler build environment usage only! */
    struct IsoLangEntry
    {
        LanguageType  mnLang;
        sal_Char      maLangStr[4];
        sal_Char      maCountry[3];

        /** Obtain a language tag string with '-' separator. */
        I18NLANGTAG_DLLPUBLIC OUString getTagString() const;
    };

    /** @internal - Return a pointer to the IsoLangEntry of the underlying table,
        matching the offset passed by nIndex. Only meaningful for the resource
        compiler to build a list of known languages.

        @returns address of IsoLangEntry, or NULL pointer if nIndex exceeds the
        table elements' count.
     */
    static const IsoLangEntry* getIsoLangEntry( size_t nIndex );

// ---------------------------------------------------------------------------

    /** Encapsulated conversion methods used by LanguageTag and conversions,
        not to be used by anything else.
     */
    class Conversion
    {
    private:

        friend class LanguageTag;

        friend ::com::sun::star::lang::Locale MsLangId::getFallbackLocale(
                const ::com::sun::star::lang::Locale & rLocale );

        friend LanguageType MsLangId::convertUnxByteStringToLanguage(
                const OString& rString );


        /** Convert a Locale to a LanguageType with handling of an empty
            language name designating LANGUAGE_SYSTEM.
          */
        I18NLANGTAG_DLLPRIVATE static LanguageType convertLocaleToLanguage(
                const ::com::sun::star::lang::Locale & rLocale );

        /** Convert x-... privateuse, used by convertLocaleToLanguage(Locale) */
        I18NLANGTAG_DLLPRIVATE static LanguageType convertPrivateUseToLanguage(
                const OUString& rPriv );

        /** Used by convertLocaleToLanguage(Locale) */
        I18NLANGTAG_DLLPRIVATE static LanguageType convertIsoNamesToLanguage(
                const OUString& rLang, const OUString& rCountry );


        /** Used by convertUnxByteStringToLanguage(OString) */
        I18NLANGTAG_DLLPRIVATE static LanguageType convertIsoNamesToLanguage(
                const OString& rLang, const OString& rCountry );


        /** Convert a LanguageType to a Locale.

            @param bResolveSystem
                   If bResolveSystem==true, a LANGUAGE_SYSTEM is resolved.
                   If bResolveSystem==false, a LANGUAGE_SYSTEM results in an
                   empty Locale.
          */
        I18NLANGTAG_DLLPRIVATE static ::com::sun::star::lang::Locale convertLanguageToLocale(
                LanguageType nLang, bool bResolveSystem );

        /** Used by convertLanguageToLocale(LanguageType,bool) */
        I18NLANGTAG_DLLPRIVATE static void convertLanguageToIsoNames(
                LanguageType nLang, OUString& rLangStr, OUString& rCountry );


        I18NLANGTAG_DLLPRIVATE static ::com::sun::star::lang::Locale lookupFallbackLocale(
                const ::com::sun::star::lang::Locale & rLocale );

        /** Used by getFallbackLocale(Locale) */
        I18NLANGTAG_DLLPRIVATE static ::com::sun::star::lang::Locale lookupFallbackLocale(
                LanguageType nLang );
    };

private:

    static LanguageType         nConfiguredSystemLanguage;
    static LanguageType         nConfiguredSystemUILanguage;

    static LanguageType         nConfiguredWesternFallback;
    static LanguageType         nConfiguredAsianFallback;
    static LanguageType         nConfiguredComplexFallback;

    static LanguageType getPlatformSystemLanguage();
    static LanguageType getPlatformSystemUILanguage();

    // Substitute LANGUAGE_SYSTEM for LANGUAGE_SYSTEM_DEFAULT and
    // LANGUAGE_PROCESS_OR_USER_DEFAULT, other values aren't touched.
    I18NLANGTAG_DLLPRIVATE static inline LanguageType simplifySystemLanguages( LanguageType nLang );
};


// static
inline LanguageType MsLangId::getSystemLanguage()
{
    return getPlatformSystemLanguage();
}


// static
inline LanguageType MsLangId::getSystemUILanguage()
{
    return getPlatformSystemUILanguage();
}

#endif // INCLUDED_I18NLANGTAG_MSLANGID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
