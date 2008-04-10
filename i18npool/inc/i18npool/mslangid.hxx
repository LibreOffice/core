/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mslangid.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#define INCLUDED_I18NPOOL_MSLANGID_HXX

#include <sal/config.h>

#ifndef INCLUDED_I18NPOOL_DLLAPI_H
#include "i18npool/i18npooldllapi.h"
#endif
#include "i18npool/lang.h"
#include <com/sun/star/lang/Locale.hpp>


/** Methods related to Microsoft language IDs. For details about MS-LANGIDs
    please see lang.h */
class I18NPOOL_DLLPUBLIC MsLangId
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
                application's configuration. If you need to access the system
                values use <method>getRealLanguageWithoutConfig()</method>
                instead.

        @returns
            case LANGUAGE_PROCESS_OR_USER_DEFAULT : configured or system language
            case LANGUAGE_SYSTEM_DEFAULT :          configured or system language
            case LANGUAGE_SYSTEM :                  configured or system language
            case LANGUAGE_NONE :                    configured or system UI language
            case LANGUAGE_DONTKNOW :                LANGUAGE_ENGLISH_US
            else: nLang

            In case the configured language is LANGUAGE_SYSTEM, which is also
            the initial default, the system language is obtained. In case the
            configured or resulting system language is LANGUAGE_DONTKNOW,
            LANGUAGE_ENGLISH_US is returned instead.
      */
    static LanguageType getRealLanguage( LanguageType nLang );


    /** @short: Convert a LanguageType to a Locale, resolving LANGUAGE_SYSTEM.

        @ATTENTION: A round trip convertLanguageToLocale(
                convertLocaleToLanguage( ...)) is NOT possible because this
                method substitutes LANGUAGE_SYSTEM and the like. If round-trip
                is desired, you MUST use convertLanguageToLocale( ..., false)
                instead.
     */
    static void convertLanguageToLocale( LanguageType nLang,
            ::com::sun::star::lang::Locale & rLocale );


    /** @short: Convert a LanguageType to a Locale with handling of
                getRealLanguage().

        @descr: If bResolveSystem==true don't use to convert a Language to a
                Locale for file storage because it substitutes LANGUAGE_SYSTEM
                and LANGUAGE_NONE and similar, use only at runtime! If
                bResolveSystem==false a LANGUAGE_SYSTEM results in an empty
                Locale.

        @ATTENTION: A round trip convertLanguageToLocale(
                convertLocaleToLanguage( ...)) using the default parameter is
                NOT possible because this method
                substitutes LANGUAGE_SYSTEM and the like. If round-trip is
                desired, you MUST use convertLanguageToLocale( ..., false)
                instead.
      */
    static ::com::sun::star::lang::Locale convertLanguageToLocale(
            LanguageType nLang, bool bResolveSystem = true );


    /** Convert a Locale to a LanguageType with handling of an empty language
        name designating the SYSTEM language.
      */
    static LanguageType convertLocaleToLanguage( const ::com::sun::star::lang::Locale & rLocale );


    /** Convert a LanguageType to a Locale, resolving LANGUAGE_SYSTEM, falling
        back to a default locale if no exact match was found.
     */
    static ::com::sun::star::lang::Locale convertLanguageToLocaleWithFallback( LanguageType nLang );


    /** Convert a Locale to a LanguageType with handling of an empty language
        name designating the SYSTEM language, falling back to a default locale
        if no exact match was found.
      */
    static LanguageType convertLocaleToLanguageWithFallback(
            const ::com::sun::star::lang::Locale & rLocale );


    /** Get fall-back Locale for Locale with handling of an empty language name
        designating the SYSTEM language. Returns the same Locale if an exact
        match was found.
      */
    static ::com::sun::star::lang::Locale getFallbackLocale(
            const ::com::sun::star::lang::Locale & rLocale );


    /** Get fall-back LanguageType for LanguageType, resolving LANGUAGE_SYSTEM.
        Returns the same LanguageType if an exact match was found.
      */
    static LanguageType getFallbackLanguage( LanguageType nLang );


    // -----------------------------
    // - ConvertLanguageToIsoNames -
    // -----------------------------

    static void convertLanguageToIsoNames( LanguageType nLang,
            rtl::OUString& rLangStr, rtl::OUString& rCountry );
    static void convertLanguageToIsoNames( LanguageType nLang,
            rtl::OString& rLangStr, rtl::OString& rCountry );
    static rtl::OUString convertLanguageToIsoString( LanguageType nLang,
            sal_Unicode cSep = '-' );
    static rtl::OString convertLanguageToIsoByteString( LanguageType nLang,
            sal_Char cSep = '-' );

    // -----------------------------
    // - ConvertIsoNamesToLanguage -
    // -----------------------------

    static LanguageType convertIsoNamesToLanguage( const rtl::OUString& rLang,
            const rtl::OUString& rCountry );
    static LanguageType convertIsoNamesToLanguage( const rtl::OString& rLang,
            const rtl::OString& rCountry );
    static LanguageType convertIsoStringToLanguage(
            const rtl::OUString& rString, sal_Unicode cSep = '-' );
    static LanguageType convertIsoByteStringToLanguage(
            const rtl::OString& rString, sal_Char cSep = '-' );
    static LanguageType convertUnxByteStringToLanguage(
            const rtl::OString& rString );


    /** @short: A real language/locale if the nLang parameter designates some
                special value.

        @descr: NOTE: This is a raw interface to the system and does not take
                any application configuration into account. If that is wanted,
                which is most likely, use <method>getRealLanguage()</method>
                instead.

        @returns
            case LANGUAGE_PROCESS_OR_USER_DEFAULT : getSystemLanguage()
            case LANGUAGE_SYSTEM_DEFAULT :          getSystemLanguage()
            case LANGUAGE_SYSTEM :                  getSystemLanguage()
            case LANGUAGE_NONE :                    getSystemUILanguage()
            case LANGUAGE_DONTKNOW :                LANGUAGE_ENGLISH_US
            else: nLang

            In case getSystemLanguage() or getSystemUILanguage() returned
            LANGUAGE_DONTKNOW, LANGUAGE_ENGLISH_US is returned instead.
      */
    static LanguageType getRealLanguageWithoutConfig( LanguageType nLang );


    /** Whether locale has a Right-To-Left orientation. */
    static bool isRightToLeft( LanguageType nLang );


    /** Whether there are "forbidden characters at start or end of line" in
        this locale. CJK locales.

        @see offapi/com/sun/star/i18n/ForbiddenCharacters.idl
      */
    static bool hasForbiddenCharacters( LanguageType nLang );


    /** Get ::com::sun::star::i18n::ScriptType of locale. */
    static sal_Int16 getScriptType( LanguageType nLang );


    /** Map an obsolete user defined LANGID (see lang.h
        LANGUAGE_OBSOLETE_USER_...) to the new value defined by MS in the
        meantime. */
    static LanguageType getReplacementForObsoleteLanguage( LanguageType nLang );


    /** @ATTENTION: these are _ONLY_ to be called by the application's
        configuration! */
    static void setConfiguredSystemLanguage( LanguageType nLang );
    static void setConfiguredSystemUILanguage( LanguageType nLang );

// ---------------------------------------------------------------------------

    /** @internal - Access to fields of an element of the simple conversion table.
        For resource compiler build environment usage only! */
    struct IsoLangEntry
    {
        LanguageType  mnLang;
        sal_Char      maLangStr[4];
        sal_Char      maCountry[3];
    };

    /** @internal - Return a pointer to the IsoLangEntry of the underlying table,
        matching the offset passed by nIndex. Only meaningful for the resource
        compiler to build a list of known languages.

        @returns address of IsoLangEntry, or NULL pointer if nIndex exceeds the
        table elements' count.
     */
    static const IsoLangEntry* getIsoLangEntry( size_t nIndex );

// ---------------------------------------------------------------------------

private:

    static LanguageType         nConfiguredSystemLanguage;
    static LanguageType         nConfiguredSystemUILanguage;

    static LanguageType getPlatformSystemLanguage();
    static LanguageType getPlatformSystemUILanguage();

    // Substitute LANGUAGE_SYSTEM for LANGUAGE_SYSTEM_DEFAULT and
    // LANGUAGE_PROCESS_OR_USER_DEFAULT, other values aren't touched.
    I18NPOOL_DLLPRIVATE static inline LanguageType simplifySystemLanguages( LanguageType nLang );

    // Several locale lookups with fall-back
    I18NPOOL_DLLPRIVATE static LanguageType lookupFallbackLanguage( LanguageType nLang );
    I18NPOOL_DLLPRIVATE static LanguageType lookupFallbackLanguage(
            const ::com::sun::star::lang::Locale & rLocale );
    I18NPOOL_DLLPRIVATE static ::com::sun::star::lang::Locale lookupFallbackLocale( LanguageType nLang );
    I18NPOOL_DLLPRIVATE static ::com::sun::star::lang::Locale lookupFallbackLocale(
            const ::com::sun::star::lang::Locale & rLocale );
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

#endif // INCLUDED_I18NPOOL_MSLANGID_HXX
