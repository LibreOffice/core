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
#include <i18npool/i18npooldllapi.h>
#include <i18npool/lang.h>

#include <vector>

typedef struct _rtl_Locale rtl_Locale;  // as in rtl/locale.h


/** Wrapper for liblangtag BCP 47 language tags, MS-LangIDs, locales and
    conversions in between.

    Note that member variables are mutable and may change their values even in
    const methods. Getter methods return either the original value or matching
    converted values.
 */
class I18NISOLANG_DLLPUBLIC LanguageTag
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
    explicit LanguageTag( const OUString & rBcp47LanguageTag, bool bCanonicalize = false );

    /** Init LanguageTag with Locale. */
    explicit LanguageTag( const com::sun::star::lang::Locale & rLocale );

    /** Init LanguageTag with LanguageType MS-LangID. */
    explicit LanguageTag( LanguageType nLanguage );

    /** Init LanguageTag with language and country strings.

        This is a convenience ctor for places that so far use only language and
        country to replace the MsLangId::convert...IsoNames...() calls. Avoid
        use in new code.
     */
    explicit LanguageTag( const OUString& rLanguage, const OUString& rCountry );

    /** Init LanguageTag with rtl_Locale.

        This is a convenience ctor.
     */
    explicit LanguageTag( const rtl_Locale & rLocale );

    LanguageTag( const LanguageTag & rLanguageTag );
    ~LanguageTag();
    LanguageTag& operator=( const LanguageTag & rLanguageTag );

    /** Obtain BCP 47 language tag.

        @param bResolveSystem
               If TRUE, resolve an empty language tag denoting the system
               locale to the real locale used.
               If FALSE, return an empty OUString for such a tag.
     */
    const OUString &                getBcp47( bool bResolveSystem = true ) const;

    /** Obtain language tag as Locale.

        As a convention, language tags that can not be expressed as "pure"
        com::sun::star::lang::Locale content using Language and Country fields
        store "qlt" (ISO 639 reserved for local use) in the Language field and
        the entire BCP 47 language tag in the Variant field. The Country field
        contains the corresponding ISO 3166 country code _if_ there is one, or
        otherwise is empty.

        @param bResolveSystem
               If TRUE, resolve an empty language tag denoting the system
               locale to the real locale used.
               If FALSE, return an empty Locale for such a tag.
     */
    const com::sun::star::lang::Locale &    getLocale( bool bResolveSystem = true ) const;

    /** Obtain mapping to MS-LangID.

        @param bResolveSystem
               If TRUE, resolve an empty language tag denoting the system
               locale to the real locale used.
               If FALSE, return LANGUAGE_SYSTEM for such a tag.
     */
    LanguageType                    getLanguageType( bool bResolveSystem = true ) const;

    /** Obtain ISO strings for language and country.

        This is a convenience method for places that so far use only language and
        country to replace the MsLangId::convert...IsoNames...() calls. Avoid
        use in new code.

        ATTENTION! May return empty strings if the language tag is not
        expressable in valid ISO codes!

        @see isIsoLocale()

        Always resolves an empty tag to the system locale.
     */
    void                            getIsoLanguageCountry( OUString& rLanguage, OUString& rCountry ) const;

    /** Get ISO 639 language code, or BCP 47 language.

        Always resolves an empty tag to the system locale.
     */
    OUString                        getLanguage() const;

    /** Get ISO 15924 script code, if not the default script according to
        BCP 47. For default script an empty string is returned.

        @see hasScript()

        Always resolves an empty tag to the system locale.
     */
    OUString                        getScript() const;

    /** Get combined language and script code, separated by '-' if
        non-default script, if default script only language.

        @see hasScript()

        Always resolves an empty tag to the system locale.
     */
    OUString                        getLanguageAndScript() const;

    /** Get ISO 3166 country alpha code. Empty if the BCP 47 tags denote a
        region not expressable as 2 character country code.

        Always resolves an empty tag to the system locale.
     */
    OUString                        getCountry() const;

    /** Get BCP 47 region tag, which may be an ISO 3166 country alpha code or
        any other BCP 47 region tag.

        Always resolves an empty tag to the system locale.
     */
    OUString                        getRegion() const;

    /** Get a GLIBC locale string.

        Always resolves an empty tag to the system locale.

        @param  rEncoding
                An encoding to be appended to language_country, for example
                ".UTF-8" including the dot.

        @return The resulting GLIBC locale string if it could be constructed,
                if not an empty string is returned.
     */
    OUString                        getGlibcLocaleString( const OUString & rEncoding ) const;

    /** If language tag has a non-default script specified.
     */
    bool                            hasScript() const;

    /** If language tag is a locale that can be expressed using only ISO 639
        language codes and ISO 3166 country codes, thus is convertible to a
        conforming Locale struct without using extension mechanisms.

        Note that an empty language tag or empty Locale::Language field or
        LanguageType LANGUAGE_SYSTEM could be treated as a valid ISO locale in
        some context, but here is not. If you want that ask for
        aTag.isSystemLocale() || aTag.isIsoLocale()

        Always resolves an empty tag to the system locale.
     */
    bool                            isIsoLocale() const;

    /** If language tag is a locale that can be expressed using only ISO 639
        language codes and ISO 15924 script codes and ISO 3166 country codes,
        thus can be stored in an ODF document using only fo:language, fo:script
        and fo:country attributes. If this is FALSE, the locale must be stored
        as a <*:rfc-language-tag> element.

        Always resolves an empty tag to the system locale.
     */
    bool                            isIsoODF() const;

    /** If this is a valid BCP 47 language tag.

        Always resolves an empty tag to the system locale.
     */
    bool                            isValidBcp47() const;

    /** If this tag was contructed as an empty tag denoting the system locale.
      */
    bool                            isSystemLocale() const;


    /** Reset with existing BCP 47 language tag string. See ctor. */
    void                            reset( const OUString & rBcp47LanguageTag, bool bCanonicalize = false );

    /** Reset with Locale. */
    void                            reset( const com::sun::star::lang::Locale & rLocale );

    /** Reset with LanguageType MS-LangID. */
    void                            reset( LanguageType nLanguage );

    /** Reset with rtl_Locale. */
    void                            reset( const rtl_Locale & rLocale );


    /** Fall back to a known locale.

        If the current tag does not represent a known (by us) locale, fall back
        to the most likely locale possible known.
        If the current tag is known, no change occurs.
     */
    LanguageTag &                   makeFallback();

    /** Return a vector of fall-back strings.

        In order:
        full BCP 47 tag, same as getBcp47()
        lll-Ssss-CC
        lll-Ssss
        lll-CC
        lll

        Only strings that differ from a higher order are included, for example
        if there is no script the elements will be bcp47, lll-CC, lll; if the
        bcp47 string is identical to lll-CC then only lll-CC, lll.

        Note that lll is only ISO 639-1/2 alpha code and CC is only ISO 3166
        alpha code. If the region can not be expressed as ISO 3166 then no -CC
        tags are included.
     */
    ::std::vector< OUString >       getFallbackStrings() const;


    /** @short  search for an equal or at least for a similar locale in a list
                of possible ones.

        @descr  First search for a locale that is equal to the reference
                locale. (means: same BCP47 string)

                If the reference locale could not be located, check for
                "similar" locales, in the same order as obtained by
                getFallbackStrings().

                If no similar locale could be located, we search for a locale
                "en-US" inside the given locale list.

                If "en-US" could not be located, we search for a locale "en"
                inside the given list.

                If no "same" nor any "similar" locale could be found, we try
                "x-default" and "x-no-translate" explicitly. Sometimes
                variables don't use real localization. For example, in case the
                localized value is a fix product name.

                If no locale matched until then, we use any other locale that
                exists inside the set of given ones, namely the first
                encountered!

        @param  rList
                the vector of possible locales as BCP47 strings.

        @param  rReference
                the reference locale, BCP47 string.

        @return An iterator that points to the found element inside the given
                locale list. If no matching locale could be found it points to
                the end of the list.
     */
    static ::std::vector< OUString >::const_iterator getFallback( const ::std::vector< OUString > & rList,
                                                                  const OUString & rReference );


    /** Test equality of two LanguageTag, possibly resolving system locale.

        @param bResolveSystem
               If TRUE, resolve empty language tags denoting the system
               locale to the real locale used before comparing.
               If FALSE, the behavior is identical to operator==(), system
               locales are not resolved first.
      */
    bool                            equals( const LanguageTag & rLanguageTag, bool bResolveSystem = false ) const;

    /** Test equality of two LanguageTag.

        Does NOT resolve system, i.e. if the system locale is en-US
        LanguageTag("")==LanguageTag("en-US") returns false! Use
        equals(...,true) instead if system locales shall be resolved.
     */
    bool    operator==( const LanguageTag & rLanguageTag ) const;

    /** Test inequality of two LanguageTag.

        Does NOT resolve system, i.e. if the system locale is en-US
        LanguageTag("")!=LanguageTag("en-US") returns true! Use
        !equals(,...true) instead if system locales shall be resolved.
     */
    bool    operator!=( const LanguageTag & rLanguageTag ) const;

private:

    enum Decision
    {
        DECISION_DONTKNOW,
        DECISION_NO,
        DECISION_YES
    };

    mutable com::sun::star::lang::Locale    maLocale;
    mutable OUString                        maBcp47;
    mutable OUString                        maCachedLanguage;   ///< cache getLanguage()
    mutable OUString                        maCachedScript;     ///< cache getScript()
    mutable OUString                        maCachedCountry;    ///< cache getCountry()
    mutable void*                           mpImplLangtag;      ///< actually lt_tag_t pointer, encapsulated
    mutable LanguageType                    mnLangID;
    mutable Decision                        meIsValid;
    mutable Decision                        meIsIsoLocale;
    mutable Decision                        meIsIsoODF;
    mutable Decision                        meIsLiblangtagNeeded;   ///< whether processing with liblangtag needed
            bool                            mbSystemLocale      : 1;
    mutable bool                            mbInitializedBcp47  : 1;
    mutable bool                            mbInitializedLocale : 1;
    mutable bool                            mbInitializedLangID : 1;
    mutable bool                            mbCachedLanguage    : 1;
    mutable bool                            mbCachedScript      : 1;
    mutable bool                            mbCachedCountry     : 1;
            bool                            mbIsFallback        : 1;

    void    convertLocaleToBcp47();
    void    convertLocaleToLang();
    void    convertBcp47ToLocale();
    void    convertBcp47ToLang();
    void    convertLangToLocale();
    void    convertLangToBcp47();

    void    convertFromRtlLocale();

    bool    canonicalize();

    OUString    getLanguageFromLangtag();
    OUString    getScriptFromLangtag();
    OUString    getRegionFromLangtag();

    void            resetVars();

    /** Obtain Language, Script and Country via simpleExtract() and assign them
        to the cached variables if successful.

        @return return of simpleExtract()
     */
    bool            cacheSimpleLSC();

    static bool     isIsoLanguage( const OUString& rLanguage );
    static bool     isIsoScript( const OUString& rScript );
    static bool     isIsoCountry( const OUString& rRegion );

    enum Extraction
    {
        EXTRACTED_NONE,
        EXTRACTED_LSC,
        EXTRACTED_X,
        EXTRACTED_X_JOKER
    };

    /** Of a simple language tag of the form lll[-Ssss][-CC] (i.e. one that
        would fulfill the isIsoODF() condition) extract the portions.

        Does not check case or content!

        @return EXTRACTED_LSC if simple tag was detected, EXTRACTED_X if x-...
        privateuse tag was detected, EXTRACTED_X_JOKER if "*" joker was
        detected, else EXTRACTED_NONE.
     */
    static Extraction   simpleExtract( const OUString& rBcp47,
                                       OUString& rLanguage,
                                       OUString& rScript,
                                       OUString& rCountry );
};

#endif  // INCLUDED_I18NPOOL_LANGUAGETAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
