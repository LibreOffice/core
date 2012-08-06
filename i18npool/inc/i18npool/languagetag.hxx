/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_I18NPOOL_LANGUAGETAG_HXX
#define INCLUDED_I18NPOOL_LANGUAGETAG_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <i18npool/lang.h>


/** Wrapper for liblangtag BCP 47 language tags, MS-LangIDs, locales and
    conversions in between.

    Note that member variables are mutable and may change their values even in
    const methods. Getter methods return either the original value or matching
    converted values.
 */
class LanguageTag
{
public:

    /** Init LanguageTag with existing BCP 47 language tag string.

        @param bCanonicalize
            If TRUE, canonicalize tag and reparse, the resulting tag string may
            be different.
            IF FALSE, the tag is simply stored and can be retrieved with
            getBcp47().

        Note that conversions to ISO codes, locales or LanguageType or
        obtaining language or script will canonicalize the tag string anyway,
        so specifying bCanonicalize=false is not a guarantee that the tag will
        stay identical to what was passed.
     */
    explicit LanguageTag( const rtl::OUString & rBcp47LanguageTag, bool bCanonicalize = false );

    /** Init LanguageTag with Locale. */
    explicit LanguageTag( const com::sun::star::lang::Locale & rLocale );

    /** Init LanguageTag with LanguageType MS-LangID. */
    explicit LanguageTag( LanguageType nLanguage );

    /** Init LanguageTag with language and country strings.

        This is a convenience ctor for places that so far use only language and
        country to replace the MsLangId::convert...IsoNames...() calls. Avoid
        use in new code.
     */
    explicit LanguageTag( const rtl::OUString& rLanguage, const rtl::OUString& rCountry );

    explicit LanguageTag( const LanguageTag & rLanguageTag );
    ~LanguageTag();
    LanguageTag& operator=( const LanguageTag & rLanguageTag );

    /** Obtain BCP 47 language tag. */
    rtl::OUString                   getBcp47() const;

    /** Obtain language tag as Locale.

        As a convention, language tags that can not be expressed as "pure"
        com::sun::star::lang::Locale content using Language and Country fields
        store "qlt" (ISO 639 reserved for local use) in the Language field and
        the entire BCP 47 language tag in the Variant field. The Country field
        contains the corresponding ISO 3166 country code _if_ there is one, or
        otherwise is empty.
     */
    com::sun::star::lang::Locale    getLocale() const;

    /** Obtain mapping to MS-LangID. */
    LanguageType                    getLanguageType() const;

    /** Get ISO 639 language code, or BCP 47 language. */
    rtl::OUString                   getLanguage() const;

    /** Get ISO 15924 script code, if not the default script according to
        BCP 47. For default script an empty string is returned.
     */
    rtl::OUString                   getScript() const;

    /** Get combined language and script code, separated by '-' if
        non-default script, if default script only language.
     */
    rtl::OUString                   getLanguageAndScript() const;

    /** Get ISO 3166 country alpha code. Empty if the BCP 47 tags denote a
        region not expressable as 2 character country code.
     */
    rtl::OUString                   getCountry() const;

    /** Get BCP 47 region tag, which may be an ISO 3166 country alpha code or
        any other BCP 47 region tag.
     */
    rtl::OUString                   getRegion() const;

    /** If language tag is a locale that can be expressed using only ISO 639
        language codes and ISO 3166 country codes, thus is convertible to a
        conforming Locale struct without using extension mechanisms. Note that
        an empty language tag or empty Locale::Language field or LanguageType
        LANGUAGE_SYSTEM is treated as a valid ISO locale.
     */
    bool                            isIsoLocale() const;

    /** If language tag is a locale that can be expressed using only ISO 639
        language codes and ISO 15924 script codes and ISO 3166 country codes,
        thus can be stored in an ODF document using only fo:language, fo:script
        and fo:country attributes. If this is FALSE, the locale must be stored
        as a <*:rfc-language-tag> element.
     */
    bool                            isIsoODF() const;

    /** If this is a valid BCP 47 language tag. */
    bool                            isValidBcp47() const;

private:

    enum Decision
    {
        DECISION_DONTKNOW,
        DECISION_NO,
        DECISION_YES
    };

    mutable com::sun::star::lang::Locale    maLocale;
    mutable rtl::OUString                   maBcp47;
    mutable rtl::OUString                   maCachedLanguage;   ///< cache getLanguage()
    mutable rtl::OUString                   maCachedScript;     ///< cache getScript()
    mutable rtl::OUString                   maCachedCountry;    ///< cache getCountry()
    mutable void*                           mpImplLangtag;      ///< actually lt_tag_t pointer, encapsulated
    mutable LanguageType                    mnLangID;
    mutable Decision                        meIsValid;
    mutable Decision                        meIsIsoLocale;
    mutable Decision                        meIsIsoODF;
    mutable bool                            mbInitializedBcp47  : 1;
    mutable bool                            mbInitializedLocale : 1;
    mutable bool                            mbInitializedLangID : 1;
    mutable bool                            mbCachedLanguage    : 1;
    mutable bool                            mbCachedScript      : 1;
    mutable bool                            mbCachedCountry     : 1;

    void    convertLocaleToBcp47();
    void    convertLocaleToLang();
    void    convertBcp47ToLocale();
    void    convertBcp47ToLang();
    void    convertLangToLocale();
    void    convertLangToBcp47();

    bool    canonicalize() const;

    rtl::OUString   getLanguageFromLangtag() const;
    rtl::OUString   getScriptFromLangtag() const;
    rtl::OUString   getRegionFromLangtag() const;

    static bool     isIsoLanguage( const rtl::OUString& rLanguage );
    static bool     isIsoScript( const rtl::OUString& rScript );
    static bool     isIsoCountry( const rtl::OUString& rRegion );
};

#endif  // INCLUDED_I18NPOOL_LANGUAGETAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
