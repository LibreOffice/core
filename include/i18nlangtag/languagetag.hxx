/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_I18NLANGTAG_LANGUAGETAG_HXX
#define INCLUDED_I18NLANGTAG_LANGUAGETAG_HXX

#include <sal/config.h>
#include <rtl/locale.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <i18nlangtag/i18nlangtagdllapi.h>
#include <i18nlangtag/lang.h>

#include <memory>
#include <vector>

/** The ISO 639-2 code reserved for local use used to indicate that a
    css::Locale contains a BCP 47 string in its Variant field. The
    Locale's Language field then will contain this language code.

    @see LanguageTag::getLocale()

    Avoid use, only needed internally or if conversion from Locale to
    LanguageTag is not wanted, i.e. during ODF import. To check whether a
    LanguageTag contains a plain language/country combination or a more
    detailed BCP 47 language tag use LanguageTag::isIsoLocale() instead.
 */
#define I18NLANGTAG_QLT "qlt"


class LanguageTagImpl;


/** Wrapper for liblangtag BCP 47 language tags, MS-LangIDs, locales and
    conversions in between.

    Note that member variables are mutable and may change their values even in
    const methods. Getter methods return either the original value or matching
    converted values.

    For standalone conversions if no LanguageTag instance is at hand, static
    convertTo...() methods exist.
 */
class SAL_WARN_UNUSED I18NLANGTAG_DLLPUBLIC LanguageTag
{
    friend class LanguageTagImpl;

public:

    /** ScriptType for a language.

        Used only in onTheFly languages as a way of marking key script behaviours
        for the script of the language without having to store and analyse the
        script each time. Used primarily from msLangId.

        These need to correspond to the ExtraLanguages.ScriptType template
        property in officecfg/registry/schema/org/openoffice/VCL.xcs
     */
    enum class ScriptType
    {
        UNKNOWN = 0,
        WESTERN = 1,      // Copies css::i18n::ScriptType for strong types
        CJK = 2,
        CTL = 3,
        RTL = 4       // implies CTL
    };

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
    explicit LanguageTag( const css::lang::Locale & rLocale );

    /** Init LanguageTag with LanguageType MS-LangID. */
    explicit LanguageTag( LanguageType nLanguage );

    /** Init LanguageTag with either BCP 47 language tag (precedence if not
        empty), or a combination of language, script and country.

        This is a convenience ctor to be used in ODF import where these are
        distinct attributes.
     */
    explicit LanguageTag( const OUString& rBcp47, const OUString& rLanguage,
                          const OUString& rScript, const OUString& rCountry );

    /** Init LanguageTag with rtl_Locale.

        This is a convenience ctor.
     */
    explicit LanguageTag( const rtl_Locale & rLocale );

    ~LanguageTag();

    /** Obtain BCP 47 language tag.

        @param bResolveSystem
               If TRUE, resolve an empty language tag denoting the system
               locale to the real locale used.
               If FALSE, return an empty OUString for such a tag.
     */
    const OUString &                getBcp47( bool bResolveSystem = true ) const;

    /** Obtain BCP 47 language tag, but with MS malformed exceptions.

        To be used *only* in OOXML filter context.
        For example, es-ES-u-co-trad is stored as es-ES_tradnl which is not a
        valid BCP 47 language tag.
     */
    OUString                        getBcp47MS() const;

    /** Obtain language tag as Locale.

        As a convention, language tags that can not be expressed as "pure"
        css::lang::Locale content using Language and Country fields
        store "qlt" (ISO 639 reserved for local use) in the Language field and
        the entire BCP 47 language tag in the Variant field. The Country field
        contains the corresponding ISO 3166 country code _if_ there is one, or
        otherwise is empty.

        @param bResolveSystem
               If TRUE, resolve an empty language tag denoting the system
               locale to the real locale used.
               If FALSE, return an empty Locale for such a tag.
     */
    const css::lang::Locale &    getLocale( bool bResolveSystem = true ) const;

    /** Obtain mapping to MS-LangID.

        @param bResolveSystem
               If TRUE, resolve an empty language tag denoting the system
               locale to the real locale used.
               If FALSE, return LANGUAGE_SYSTEM for such a tag.
     */
    LanguageType                    getLanguageType( bool bResolveSystem = true ) const;

    /** Obtain ISO strings for language, script and country.

        This is a convenience method for ODF export places only. Avoid use in
        other code.

        ATTENTION! May return empty strings if the language tag is not
        expressable in valid ISO codes!

        @see isIsoODF()

        Always resolves an empty tag to the system locale.
     */
    void                            getIsoLanguageScriptCountry( OUString& rLanguage,
                                                                 OUString& rScript, OUString& rCountry ) const;

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

    /** Get BCP 47 variant subtags, of the IANA Language Subtag Registry.

        If there are multiple variant subtags they are separated by '-'.

        This is NOT related to Locale.Variant!

        Always resolves an empty tag to the system locale.
     */
    OUString                        getVariants() const;

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

        @seealso    static bool isValidBcp47(const OUString&)
     */
    bool                            isValidBcp47() const;

    /** If this tag was constructed as an empty tag denoting the system locale.
      */
    bool                            isSystemLocale() const { return mbSystemLocale;}

    /** Returns the script type for this language, UNKNOWN if not set */
    ScriptType                      getScriptType() const;

    /** Sets the script type for this language */
    void                            setScriptType(ScriptType st);

    /** Reset with existing BCP 47 language tag string. See ctor. */
    LanguageTag &                   reset( const OUString & rBcp47LanguageTag );

    /** Reset with Locale. */
    LanguageTag &                   reset( const css::lang::Locale & rLocale );

    /** Reset with LanguageType MS-LangID. */
    LanguageTag &                   reset( LanguageType nLanguage );


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

        If the tag includes variants the order is:
        full BCP 47 tag, same as getBcp47()
        lll-Ssss-CC-vvvvvvvv
        lll-Ssss-vvvvvvvv
        lll-Ssss-CC
        lll-Ssss
        lll-CC-vvvvvvvv
        lll-vvvvvvvv
        lll-CC
        lll

        Only strings that differ from a higher order are included, for example
        if there is no script the elements will be bcp47, lll-CC, lll; if the
        bcp47 string is identical to lll-CC then only lll-CC, lll.

        Note that lll is only ISO 639-1/2 alpha code and CC is only ISO 3166
        alpha code. If the region can not be expressed as ISO 3166 then no -CC
        tags are included.

        @param  bIncludeFullBcp47
                If TRUE, the full BCP 47 tag is included as first element.
                If FALSE, the full tag is not included; used if the caller
                obtains the fallbacks only if the full tag did not lead to a
                match, so subsequent tries need not to include it again.
     */
    ::std::vector< OUString >       getFallbackStrings( bool bIncludeFullBcp47 ) const;


    /** @short  Search for an equal or at least for a similar locale in a list
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
                the beginning of the list.
     */
    static ::std::vector< OUString >::const_iterator getFallback( const ::std::vector< OUString > & rList,
                                                                  const OUString & rReference );


    /** @short  Search for an equal or for a similar locale in a list
                of possible ones where at least the language matches.

        @descr  First search for a locale that is equal to the reference
                locale.

                If the reference locale could not be located, check for
                "similar" locales, in the same order as obtained by
                getFallbackStrings().

                If no locale matches, rList.end() is returned.

        @param  rList
                the vector of possible locales.

        @param  rReference
                the reference locale.

        @return An iterator that points to the found element inside the given
                locale list. If no matching locale could be found it points to
                the end of the list.
     */
    static ::std::vector< css::lang::Locale >::const_iterator getMatchingFallback(
            const ::std::vector< css::lang::Locale > & rList,
            const css::lang::Locale & rReference );


    /** Test equality of two LanguageTag, possibly resolving system locale.

        Resolve empty language tags denoting the system
        locale to the real locale used before comparing.
      */
    bool    equals( const LanguageTag & rLanguageTag ) const;

    /** Test equality of two LanguageTag.

        Does NOT resolve system, i.e. if the system locale is en-US
        LanguageTag("")==LanguageTag("en-US") returns false! Use
        equals(...) instead if system locales shall be resolved.
     */
    bool    operator==( const LanguageTag & rLanguageTag ) const;

    /** Test inequality of two LanguageTag.

        Does NOT resolve system, i.e. if the system locale is en-US
        LanguageTag("")!=LanguageTag("en-US") returns true! Use
        !equals(,..) instead if system locales shall be resolved.
     */
    bool    operator!=( const LanguageTag & rLanguageTag ) const;

    /** Test this LanguageTag less than that LanguageTag.

        For sorted containers. Does NOT resolve system.
     */
    bool    operator<( const LanguageTag & rLanguageTag ) const;

    /** Convert MS-LangID to Locale.

        @param bResolveSystem
               If TRUE, resolve an empty language tag denoting the system
               locale to the real locale used.
               If FALSE, return an empty Locale for such a tag.
     */
    static css::lang::Locale convertToLocale( LanguageType nLangID, bool bResolveSystem = true );

    /** Convert Locale to MS-LangID.

        @param bResolveSystem
               If TRUE, resolve an empty language tag denoting the system
               locale to the real locale used.
               If FALSE, return LANGUAGE_SYSTEM for such a tag.
     */
    static LanguageType convertToLanguageType( const css::lang::Locale& rLocale, bool bResolveSystem = true );

    /** Convert MS-LangID to BCP 47 string.

        Resolve an empty language tag denoting the system
       locale to the real locale used.
     */
    static OUString convertToBcp47( LanguageType nLangID );

    /** Convert Locale to BCP 47 string.

        @param bResolveSystem
               If TRUE, resolve an empty language tag denoting the system
               locale to the real locale used.
               If FALSE, return an empty OUString for such a tag.
     */
    static OUString convertToBcp47( const css::lang::Locale& rLocale, bool bResolveSystem = true );

    /** Convert BCP 47 string to Locale, convenience method.

        NOTE: exists only for consistency with the other convertTo...()
        methods, internally uses a temporary LanguageTag instance for
        conversion so does not save anything compared to
        LanguageTag(rBcp47).getLocale(bResolveSystem).

        @param bResolveSystem
               If TRUE, resolve an empty language tag denoting the system
               locale to the real locale used.
               If FALSE, return an empty Locale for such a tag.
     */
    static css::lang::Locale convertToLocale( const OUString& rBcp47, bool bResolveSystem = true );

    /** Convert BCP 47 string to MS-LangID, convenience method.

        NOTE: exists only for consistency with the other convertTo...()
        methods, internally uses a temporary LanguageTag instance for
        conversion so does not save anything compared to
        LanguageTag(rBcp47).getLanguageType(bResolveSystem).

        Resolve an empty language tag denoting the system
        locale to the real locale used.
     */
    static LanguageType convertToLanguageType( const OUString& rBcp47 );

    /** Convert BCP 47 string to MS-LangID with fallback, convenience method.

        NOTE: exists only for consistency with the other convertTo...()
        methods, internally uses a temporary LanguageTag instance for
        conversion so does not save anything compared to
        LanguageTag(rBcp47).makeFallback().getLanguageType(bResolveSystem).

        @see    makeFallback()

        Always resolves an empty tag to the system locale.
     */
    static LanguageType convertToLanguageTypeWithFallback( const OUString& rBcp47 );

    /** Convert BCP 47 string to Locale with fallback, convenience method.

        NOTE: exists only for consistency with the other convertTo...()
        methods, internally uses a temporary LanguageTag instance for
        conversion so does not save anything compared to
        LanguageTag(rBcp47).makeFallback().getLocale(bResolveSystem).

        @see    makeFallback()

        Always resolves an empty tag to the system locale.
     */
    static css::lang::Locale convertToLocaleWithFallback( const OUString& rBcp47 );

    /** If rString represents a valid BCP 47 language tag.

        Never resolves an empty tag to the system locale, in fact an empty
        string is invalid here. Does not create an instance to be registered
        with a conversion to Locale or LanguageType.

        @param  o_pCanonicalized
                If given and rString is a valid BCP 47 language tag, the
                canonicalized form is assigned, which may differ from the
                original string even if that was a valid tag. If rString is not
                a valid tag, nothing is assigned.

        @param  bDisallowPrivate
                If TRUE, valid tags according to BCP 47 but reserved for
                private use, like 'x-...', are not allowed and FALSE is
                returned in this case.
     */
    static bool         isValidBcp47( const OUString& rString, OUString* o_pCanonicalized,
                                      bool bDisallowPrivate = false );

    /** If nLang is a generated on-the-fly LangID */
    static bool         isOnTheFlyID( LanguageType nLang );
    static ScriptType   getOnTheFlyScriptType( LanguageType nLang );

    /** @ATTENTION: _ONLY_ to be called by the application's configuration! */
    static void setConfiguredSystemLanguage( LanguageType nLang );

    /** @ATTENTION: _ONLY_ to be called by fuzzing setup */
    static void disable_lt_tag_parse();

    typedef std::shared_ptr< LanguageTagImpl > ImplPtr;

private:

    mutable css::lang::Locale               maLocale;
    mutable OUString                        maBcp47;
    mutable LanguageType                    mnLangID;
    mutable ImplPtr                         mpImpl;
            bool                            mbSystemLocale      : 1;
    mutable bool                            mbInitializedBcp47  : 1;
    mutable bool                            mbInitializedLocale : 1;
    mutable bool                            mbInitializedLangID : 1;
            bool                            mbIsFallback        : 1;

    LanguageTagImpl*    getImpl();
    LanguageTagImpl const* getImpl() const;
    ImplPtr             registerImpl() const;
    void                syncFromImpl();
    void                syncVarsFromRawImpl() const;
    void                syncVarsFromImpl() const;

    void                convertLocaleToLang();
    void                convertBcp47ToLocale();
    void                convertBcp47ToLang();
    void                convertLangToLocale();

    void                convertFromRtlLocale();

    /** Canonicalize if not yet done and synchronize initialized conversions.

        @return whether BCP 47 language tag string was changed.
     */
    bool                synCanonicalize();

    void                resetVars();

    static bool         isIsoLanguage( const OUString& rLanguage );
    static bool         isIsoScript( const OUString& rScript );
    static bool         isIsoCountry( const OUString& rRegion );

};

#endif  // INCLUDED_I18NLANGTAG_LANGUAGETAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
