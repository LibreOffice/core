/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "i18npool/languagetag.hxx"
#include "i18npool/mslangid.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/file.hxx>
#include <rtl/instance.hxx>
#include <rtl/locale.h>

//#define erDEBUG

#if defined(ENABLE_LIBLANGTAG)
#include <liblangtag/langtag.h>
#else
/* Replacement code for LGPL phobic and Android systems.
 * For iOS we could probably use NSLocale instead, that should have more or
 * less required functionality. If it is good enough, it could be used for Mac
 * OS X, too.
 */
#include "simple-langtag.cxx"
#endif

using rtl::OUString;
using rtl::OString;
using rtl::OUStringBuffer;
using namespace com::sun::star;

// The actual pointer type of mpImplLangtag that is declared void* to not
// pollute the entire code base with liblangtag.
#define LANGTAGCAST(p) (reinterpret_cast<lt_tag_t*>(p))
#define MPLANGTAG LANGTAGCAST(mpImplLangtag)

/** Convention to signal presence of BCP 47 language tag in a Locale's Variant
    field. The Locale's Language field then will contain this ISO 639-2
    reserved for local use code. */
#define ISO639_LANGUAGE_TAG "qlt"


// Helper to ensure lt_error_t is free'd
struct myLtError
{
    lt_error_t* p;
    myLtError() : p(NULL) {}
    ~myLtError() { if (p) lt_error_unref( p); }
};


// "statics" to be returned as const reference to an empty locale and string.
namespace {
struct theEmptyLocale : public rtl::Static< lang::Locale, theEmptyLocale > {};
struct theEmptyBcp47 : public rtl::Static< OUString, theEmptyBcp47 > {};
}


/** A reference holder for liblangtag data de/initialization, one static
    instance. Currently implemented such that the first "ref" inits and dtor
    (our library deinitialized) tears down.
*/
class LiblantagDataRef
{
public:
    LiblantagDataRef();
    ~LiblantagDataRef();
    inline void incRef()
    {
        if (mnRef != SAL_MAX_UINT32 && !mnRef++)
            setup();
    }
    inline void decRef()
    {
        if (mnRef != SAL_MAX_UINT32 && mnRef && !--mnRef)
            teardown();
    }
private:
    rtl::OString maDataPath;   // path to liblangtag data, "|" if system
    sal_uInt32   mnRef;

    void setupDataPath();
    void setup();
    void teardown();
};

namespace {
struct theDataRef : public rtl::Static< LiblantagDataRef, theDataRef > {};
}

LiblantagDataRef::LiblantagDataRef()
    :
        mnRef(0)
{
}

LiblantagDataRef::~LiblantagDataRef()
{
    // When destructed we're tearing down unconditionally.
    if (mnRef)
        mnRef = 1;
    decRef();
}

void LiblantagDataRef::setup()
{
    SAL_INFO( "i18npool.langtag", "LiblantagDataRef::setup: initializing database");
    if (maDataPath.isEmpty())
        setupDataPath();
    lt_db_initialize();
    // Hold ref eternally.
    mnRef = SAL_MAX_UINT32;
}

void LiblantagDataRef::teardown()
{
    SAL_INFO( "i18npool.langtag", "LiblantagDataRef::teardown: finalizing database");
    lt_db_finalize();
}

void LiblantagDataRef::setupDataPath()
{
    // maDataPath is assumed to be empty here.
    OUString aURL("$BRAND_BASE_DIR/share/liblangtag");
    rtl::Bootstrap::expandMacros(aURL); //TODO: detect failure

    // Check if data is in our own installation, else assume system
    // installation.
    OUString aData( aURL);
    aData += "/language-subtag-registry.xml";
    osl::DirectoryItem aDirItem;
    if (osl::DirectoryItem::get( aData, aDirItem) == osl::DirectoryItem::E_None)
    {
        OUString aPath;
        if (osl::FileBase::getSystemPathFromFileURL( aURL, aPath) == osl::FileBase::E_None)
            maDataPath = OUStringToOString( aPath, RTL_TEXTENCODING_UTF8);
    }
    if (maDataPath.isEmpty())
        maDataPath = "|";   // assume system
    else
        lt_db_set_datadir( maDataPath.getStr());
}

LanguageTag::LanguageTag( const rtl::OUString & rBcp47LanguageTag, bool bCanonicalize )
    :
        maBcp47( rBcp47LanguageTag),
        mpImplLangtag( NULL),
        mnLangID( LANGUAGE_DONTKNOW),
        meIsValid( DECISION_DONTKNOW),
        meIsIsoLocale( DECISION_DONTKNOW),
        meIsIsoODF( DECISION_DONTKNOW),
        meIsLiblangtagNeeded( DECISION_DONTKNOW),
        mbSystemLocale( rBcp47LanguageTag.isEmpty()),
        mbInitializedBcp47( !mbSystemLocale),
        mbInitializedLocale( false),
        mbInitializedLangID( false),
        mbCachedLanguage( false),
        mbCachedScript( false),
        mbCachedCountry( false),
        mbIsFallback( false)
{
    if (bCanonicalize)
        canonicalize();
}


LanguageTag::LanguageTag( const com::sun::star::lang::Locale & rLocale )
    :
        maLocale( rLocale),
        mpImplLangtag( NULL),
        mnLangID( LANGUAGE_DONTKNOW),
        meIsValid( DECISION_DONTKNOW),
        meIsIsoLocale( DECISION_DONTKNOW),
        meIsIsoODF( DECISION_DONTKNOW),
        meIsLiblangtagNeeded( DECISION_DONTKNOW),
        mbSystemLocale( rLocale.Language.isEmpty()),
        mbInitializedBcp47( false),
        mbInitializedLocale( !mbSystemLocale),
        mbInitializedLangID( false),
        mbCachedLanguage( false),
        mbCachedScript( false),
        mbCachedCountry( false),
        mbIsFallback( false)
{
}


LanguageTag::LanguageTag( LanguageType nLanguage )
    :
        mpImplLangtag( NULL),
        mnLangID( nLanguage),
        meIsValid( DECISION_DONTKNOW),
        meIsIsoLocale( DECISION_DONTKNOW),
        meIsIsoODF( DECISION_DONTKNOW),
        meIsLiblangtagNeeded( DECISION_DONTKNOW),
        mbSystemLocale( nLanguage == LANGUAGE_SYSTEM),
        mbInitializedBcp47( false),
        mbInitializedLocale( false),
        mbInitializedLangID( !mbSystemLocale),
        mbCachedLanguage( false),
        mbCachedScript( false),
        mbCachedCountry( false),
        mbIsFallback( false)
{
}


LanguageTag::LanguageTag( const rtl::OUString& rLanguage, const rtl::OUString& rCountry )
    :
        maLocale( rLanguage, rCountry, ""),
        mpImplLangtag( NULL),
        mnLangID( LANGUAGE_DONTKNOW),
        meIsValid( DECISION_DONTKNOW),
        meIsIsoLocale( DECISION_DONTKNOW),
        meIsIsoODF( DECISION_DONTKNOW),
        meIsLiblangtagNeeded( DECISION_DONTKNOW),
        mbSystemLocale( rLanguage.isEmpty()),
        mbInitializedBcp47( false),
        mbInitializedLocale( !mbSystemLocale),
        mbInitializedLangID( false),
        mbCachedLanguage( false),
        mbCachedScript( false),
        mbCachedCountry( false),
        mbIsFallback( false)
{
}


LanguageTag::LanguageTag( const rtl_Locale & rLocale )
    :
        maLocale( rLocale.Language, rLocale.Country, rLocale.Variant),
        mpImplLangtag( NULL),
        mnLangID( LANGUAGE_DONTKNOW),
        meIsValid( DECISION_DONTKNOW),
        meIsIsoLocale( DECISION_DONTKNOW),
        meIsIsoODF( DECISION_DONTKNOW),
        meIsLiblangtagNeeded( DECISION_DONTKNOW),
        mbSystemLocale( maLocale.Language.isEmpty()),
        mbInitializedBcp47( false),
        mbInitializedLocale( !mbSystemLocale),
        mbInitializedLangID( false),
        mbCachedLanguage( false),
        mbCachedScript( false),
        mbCachedCountry( false),
        mbIsFallback( false)
{
    // The rtl_Locale follows the Open Group Base Specification,
    // 8.2 Internationalization Variables
    // language[_territory][.codeset][@modifier]
    // On GNU/Linux systems usually being glibc locales.
    // sal/osl/unx/nlsupport.c _parse_locale() parses them into
    // Language: language               2 or 3 alpha code
    // Country: [territory]             2 alpha code
    // Variant: [.codeset][@modifier]
    // Variant effectively contains anything that follows the territory, not
    // looking for '.' dot delimiter or '@' modifier content.
    if (!maLocale.Variant.isEmpty())
    {
        OString aStr = OUStringToOString( maLocale.Language + "_" + maLocale.Country + maLocale.Variant,
                RTL_TEXTENCODING_UTF8);
        /* FIXME: let liblangtag parse this entirely with
         * lt_tag_convert_from_locale() but that needs a patch to pass the
         * string. */
#if 0
        myLtError aError;
        theDataRef::get().incRef();
        mpImplLangtag = lt_tag_convert_from_locale( aStr.getStr(), &aError.p);
        maBcp47 = OStringToOUString( lt_tag_get_string( MPLANGTAG), RTL_TEXTENCODING_UTF8);
        mbInitializedBcp47 = true;
#else
        mnLangID = MsLangId::convertUnxByteStringToLanguage( aStr);
        if (mnLangID == LANGUAGE_DONTKNOW)
        {
            SAL_WARN( "i18npool.langtag", "LanguageTag(rtl_Locale) - unknown: " << aStr);
            mnLangID = LANGUAGE_ENGLISH_US;     // we need _something_ here
        }
        mbInitializedLangID = true;
#endif
        maLocale = lang::Locale();
        mbInitializedLocale = false;
    }
}


LanguageTag::LanguageTag( const LanguageTag & rLanguageTag )
    :
        maLocale( rLanguageTag.maLocale),
        maBcp47( rLanguageTag.maBcp47),
        maCachedLanguage( rLanguageTag.maCachedLanguage),
        maCachedScript( rLanguageTag.maCachedScript),
        maCachedCountry( rLanguageTag.maCachedCountry),
        mpImplLangtag( rLanguageTag.mpImplLangtag ?
                lt_tag_copy( LANGTAGCAST( rLanguageTag.mpImplLangtag)) : NULL),
        mnLangID( rLanguageTag.mnLangID),
        meIsValid( rLanguageTag.meIsValid),
        meIsIsoLocale( rLanguageTag.meIsIsoLocale),
        meIsIsoODF( rLanguageTag.meIsIsoODF),
        meIsLiblangtagNeeded( rLanguageTag.meIsLiblangtagNeeded),
        mbSystemLocale( rLanguageTag.mbSystemLocale),
        mbInitializedBcp47( rLanguageTag.mbInitializedBcp47),
        mbInitializedLocale( rLanguageTag.mbInitializedLocale),
        mbInitializedLangID( rLanguageTag.mbInitializedLangID),
        mbCachedLanguage( rLanguageTag.mbCachedLanguage),
        mbCachedScript( rLanguageTag.mbCachedScript),
        mbCachedCountry( rLanguageTag.mbCachedCountry),
        mbIsFallback( rLanguageTag.mbIsFallback)
{
    if (mpImplLangtag)
        theDataRef::get().incRef();
}


LanguageTag& LanguageTag::operator=( const LanguageTag & rLanguageTag )
{
    maLocale            = rLanguageTag.maLocale;
    maBcp47             = rLanguageTag.maBcp47;
    maCachedLanguage    = rLanguageTag.maCachedLanguage;
    maCachedScript      = rLanguageTag.maCachedScript;
    maCachedCountry     = rLanguageTag.maCachedCountry;
    mpImplLangtag       = rLanguageTag.mpImplLangtag;
    mpImplLangtag       = rLanguageTag.mpImplLangtag ?
                            lt_tag_copy( LANGTAGCAST( rLanguageTag.mpImplLangtag)) : NULL;
    mnLangID            = rLanguageTag.mnLangID;
    meIsValid           = rLanguageTag.meIsValid;
    meIsIsoLocale       = rLanguageTag.meIsIsoLocale;
    meIsIsoODF          = rLanguageTag.meIsIsoODF;
    meIsLiblangtagNeeded= rLanguageTag.meIsLiblangtagNeeded;
    mbSystemLocale      = rLanguageTag.mbSystemLocale;
    mbInitializedBcp47  = rLanguageTag.mbInitializedBcp47;
    mbInitializedLocale = rLanguageTag.mbInitializedLocale;
    mbInitializedLangID = rLanguageTag.mbInitializedLangID;
    mbCachedLanguage    = rLanguageTag.mbCachedLanguage;
    mbCachedScript      = rLanguageTag.mbCachedScript;
    mbCachedCountry     = rLanguageTag.mbCachedCountry;
    mbIsFallback        = rLanguageTag.mbIsFallback;
    if (mpImplLangtag)
        theDataRef::get().incRef();
    return *this;
}


LanguageTag::~LanguageTag()
{
    if (mpImplLangtag)
    {
        lt_tag_unref( MPLANGTAG);
        theDataRef::get().decRef();
    }
}


void LanguageTag::resetVars()
{
    if (mpImplLangtag)
    {
        lt_tag_unref( MPLANGTAG);
        mpImplLangtag = NULL;
        theDataRef::get().decRef();
    }

    maLocale            = lang::Locale();
    if (!maBcp47.isEmpty())
        maBcp47         = OUString();
    if (!maCachedLanguage.isEmpty())
        maCachedLanguage= OUString();
    if (!maCachedScript.isEmpty())
        maCachedScript  = OUString();
    if (!maCachedCountry.isEmpty())
        maCachedCountry = OUString();
    mnLangID            = LANGUAGE_DONTKNOW;
    meIsValid           = DECISION_DONTKNOW;
    meIsIsoLocale       = DECISION_DONTKNOW;
    meIsIsoODF          = DECISION_DONTKNOW;
    meIsLiblangtagNeeded= DECISION_DONTKNOW;
    mbSystemLocale      = true;
    mbInitializedBcp47  = false;
    mbInitializedLocale = false;
    mbInitializedLangID = false;
    mbCachedLanguage    = false;
    mbCachedScript      = false;
    mbCachedCountry     = false;
    mbIsFallback        = false;
}


void LanguageTag::reset( const rtl::OUString & rBcp47LanguageTag, bool bCanonicalize )
{
    resetVars();
    maBcp47             = rBcp47LanguageTag;
    mbSystemLocale      = rBcp47LanguageTag.isEmpty();
    mbInitializedBcp47  = !mbSystemLocale;

    if (bCanonicalize)
        canonicalize();
}


void LanguageTag::reset( const com::sun::star::lang::Locale & rLocale )
{
    resetVars();
    maLocale            = rLocale;
    mbSystemLocale      = rLocale.Language.isEmpty();
    mbInitializedLocale = !mbSystemLocale;
}


void LanguageTag::reset( LanguageType nLanguage )
{
    resetVars();
    mnLangID            = nLanguage;
    mbSystemLocale      = nLanguage == LANGUAGE_SYSTEM;
    mbInitializedLangID = !mbSystemLocale;
}


bool LanguageTag::canonicalize()
{
#ifdef erDEBUG
    // dump once
    struct dumper
    {
        void** mpp;
        dumper( void** pp ) : mpp( *pp ? NULL : pp) {}
        ~dumper() { if (mpp && *mpp) lt_tag_dump( LANGTAGCAST( *mpp)); }
    };
    dumper aDumper( &mpImplLangtag);
#endif

    // Side effect: have maBcp47 in any case, resolved system.
    // Some methods calling canonicalize() (or not calling it due to
    // meIsLiblangtagNeeded==DECISION_NO) rely on this! Hence do not set
    // meIsLiblangtagNeeded anywhere else than hereafter.
    getBcp47( true );

    // The simple cases and known locales don't need liblangtag processing,
    // which also avoids loading liblangtag data on startup.
    if (meIsLiblangtagNeeded == DECISION_DONTKNOW)
    {
        bool bTemporaryLocale = false;
        bool bTemporaryLangID = false;
        if (!mbInitializedLocale && !mbInitializedLangID)
        {
            if (mbSystemLocale)
            {
                mnLangID = MsLangId::getRealLanguage( LANGUAGE_SYSTEM);
                mbInitializedLangID = true;
            }
            else
            {
                // Now this is getting funny.. we only have some BCP47 string
                // and want to determine if parsing it would be possible
                // without using liblangtag just to see if it is a simple known
                // locale.
                OUString aLanguage, aScript, aCountry;
                if (simpleExtract( maBcp47, aLanguage, aScript, aCountry))
                {
                    if (aScript.isEmpty())
                    {
                        maLocale.Language = aLanguage;
                        maLocale.Country  = aCountry;
                    }
                    else
                    {
                        maLocale.Language = ISO639_LANGUAGE_TAG;
                        maLocale.Country  = aCountry;
                        maLocale.Variant  = maBcp47;
                    }
                    bTemporaryLocale = mbInitializedLocale = true;
                }
            }
        }
        if (mbInitializedLangID && !mbInitializedLocale)
        {
            // Do not call getLocale() here because that prefers
            // convertBcp47ToLocale() which would end up in recursion via
            // isIsoLocale()!

            // Prepare to verify that we have a known locale, not just an
            // arbitrary MS-LangID.
            convertLangToLocale();
        }
        if (mbInitializedLocale)
        {
            if (maLocale.Variant.isEmpty())
                meIsLiblangtagNeeded = DECISION_NO;     // per definition ll[l][-CC]
            else
            {
                if (!mbInitializedLangID)
                {
                    convertLocaleToLang();
                    if (bTemporaryLocale)
                        bTemporaryLangID = true;
                }
                if (mnLangID != LANGUAGE_DONTKNOW && mnLangID != LANGUAGE_SYSTEM)
                    meIsLiblangtagNeeded = DECISION_NO; // known locale
            }
        }
        if (bTemporaryLocale)
        {
            mbInitializedLocale = false;
            maLocale = lang::Locale();
        }
        if (bTemporaryLangID)
        {
            mbInitializedLangID = false;
            mnLangID = LANGUAGE_DONTKNOW;
        }
    }
    if (meIsLiblangtagNeeded == DECISION_NO)
    {
        meIsValid = DECISION_YES;   // really, known must be valid ...
        return true;                // that's it
    }
    meIsLiblangtagNeeded = DECISION_YES;
    SAL_INFO( "i18npool.langtag", "LanguageTag::canonicalize: using liblangtag for " << maBcp47);

    if (!mpImplLangtag)
    {
        theDataRef::get().incRef();
        mpImplLangtag = lt_tag_new();
    }

    myLtError aError;

    if (lt_tag_parse( MPLANGTAG, OUStringToOString( maBcp47, RTL_TEXTENCODING_UTF8).getStr(), &aError.p))
    {
        char* pTag = lt_tag_canonicalize( MPLANGTAG, &aError.p);
        SAL_WARN_IF( !pTag, "i18npool.langtag", "LanguageTag::canonicalize: could not canonicalize " << maBcp47);
        if (pTag)
        {
            OUString aOld( maBcp47);
            maBcp47 = OUString::createFromAscii( pTag);
            // Make the lt_tag_t follow the new string if different, which
            // removes default script and such.
            if (maBcp47 != aOld)
            {
                if (!lt_tag_parse( MPLANGTAG, pTag, &aError.p))
                {
                    SAL_WARN( "i18npool.langtag", "LanguageTag::canonicalize: could not reparse " << maBcp47);
                    free( pTag);
                    meIsValid = DECISION_NO;
                    return false;
                }
            }
            free( pTag);
            meIsValid = DECISION_YES;
            return true;
        }
    }
    else
    {
        SAL_INFO( "i18npool.langtag", "LanguageTag::canonicalize: could not parse " << maBcp47);
    }
    meIsValid = DECISION_NO;
    return false;
}


void LanguageTag::convertLocaleToBcp47()
{
    if (mbSystemLocale && !mbInitializedLocale)
        convertLangToLocale();

    if (maLocale.Language == ISO639_LANGUAGE_TAG)
    {
        maBcp47 = maLocale.Variant;
        meIsIsoLocale = DECISION_NO;
    }
    else
    {
        /* XXX NOTE: most legacy code never evaluated the Variant field, so for
         * now just concatenate language and country. In case we stumbled over
         * variant aware code we'd have to take care of that. */
        if (maLocale.Country.isEmpty())
            maBcp47 = maLocale.Language;
        else
        {
            OUStringBuffer aBuf( maLocale.Language.getLength() + 1 + maLocale.Country.getLength());
            aBuf.append( maLocale.Language).append( '-').append( maLocale.Country);
            maBcp47 = aBuf.makeStringAndClear();
        }
    }
    mbInitializedBcp47 = true;
}


void LanguageTag::convertLocaleToLang()
{
    if (mbSystemLocale)
    {
        mnLangID = MsLangId::getRealLanguage( LANGUAGE_SYSTEM);
    }
    else
    {
        /* FIXME: this is temporary until code base is converted to not use
         * MsLangId::convert...() anymore. After that, proper new method has to
         * be implemented to allow ISO639_LANGUAGE_TAG and sript tag and such. */
        mnLangID = MsLangId::Conversion::convertLocaleToLanguage( maLocale);
    }
    mbInitializedLangID = true;
}


void LanguageTag::convertBcp47ToLocale()
{
    bool bIso = isIsoLocale();
    if (bIso)
    {
        maLocale.Language = getLanguageFromLangtag();
        maLocale.Country = getRegionFromLangtag();
        maLocale.Variant = OUString();
    }
    else
    {
        maLocale.Language = ISO639_LANGUAGE_TAG;
        maLocale.Country = getCountry();
        maLocale.Variant = maBcp47;
    }
    mbInitializedLocale = true;
}


void LanguageTag::convertBcp47ToLang()
{
    if (mbSystemLocale)
    {
        mnLangID = MsLangId::getRealLanguage( LANGUAGE_SYSTEM);
    }
    else
    {
        /* FIXME: this is temporary. If we support locales that consist not
         * only of language and country, e.g. added script, this probably needs
         * to be adapted. */
        if (!mbInitializedLocale)
            convertBcp47ToLocale();
        convertLocaleToLang();
    }
    mbInitializedLangID = true;
}


void LanguageTag::convertLangToLocale()
{
    if (mbSystemLocale && !mbInitializedLangID)
    {
        mnLangID = MsLangId::getRealLanguage( LANGUAGE_SYSTEM);
        mbInitializedLangID = true;
    }
    /* FIXME: this is temporary until code base is converted to not use
     * MsLangId::convert...() anymore. After that, proper new method has to be
     * implemented to allow ISO639_LANGUAGE_TAG and script tag and such. */
    // Resolve system here!
    maLocale = MsLangId::Conversion::convertLanguageToLocale( mnLangID, true);
    mbInitializedLocale = true;
}


void LanguageTag::convertLangToBcp47()
{
    /* FIXME: this is temporary. If we support locales that consist not only of
     * language and country, e.g. added script, this probably needs to be
     * adapted. */
    if (!mbInitializedLocale)
        convertLangToLocale();
    convertLocaleToBcp47();
    mbInitializedBcp47 = true;
}


const rtl::OUString & LanguageTag::getBcp47( bool bResolveSystem ) const
{
    if (!bResolveSystem && mbSystemLocale)
        return theEmptyBcp47::get();
    if (!mbInitializedBcp47)
    {
        if (mbInitializedLocale)
            const_cast<LanguageTag*>(this)->convertLocaleToBcp47();
        else
            const_cast<LanguageTag*>(this)->convertLangToBcp47();
    }
    return maBcp47;
}


rtl::OUString LanguageTag::getLanguageFromLangtag()
{
    OUString aLanguage;
    if (meIsLiblangtagNeeded != DECISION_NO && !mpImplLangtag)
        canonicalize();
    if (maBcp47.isEmpty())
        return aLanguage;
    if (mpImplLangtag)
    {
        const lt_lang_t* pLangT = lt_tag_get_language( MPLANGTAG);
        SAL_WARN_IF( !pLangT, "i18npool.langtag", "LanguageTag::getLanguageFromLangtag: pLangT==NULL");
        if (!pLangT)
            return aLanguage;
        const char* pLang = lt_lang_get_tag( pLangT);
        SAL_WARN_IF( !pLang, "i18npool.langtag", "LanguageTag::getLanguageFromLangtag: pLang==NULL");
        if (pLang)
            aLanguage = OUString::createFromAscii( pLang);
    }
    else
    {
        if (mbCachedLanguage || cacheSimpleLSC())
            aLanguage = maCachedLanguage;
    }
    return aLanguage;
}


rtl::OUString LanguageTag::getScriptFromLangtag()
{
    OUString aScript;
    if (meIsLiblangtagNeeded != DECISION_NO && !mpImplLangtag)
        canonicalize();
    if (maBcp47.isEmpty())
        return aScript;
    if (mpImplLangtag)
    {
        const lt_script_t* pScriptT = lt_tag_get_script( MPLANGTAG);
        // pScriptT==NULL is valid for default scripts
        if (!pScriptT)
            return aScript;
        const char* pScript = lt_script_get_tag( pScriptT);
        SAL_WARN_IF( !pScript, "i18npool.langtag", "LanguageTag::getScriptFromLangtag: pScript==NULL");
        if (pScript)
            aScript = OUString::createFromAscii( pScript);
    }
    else
    {
        if (mbCachedScript || cacheSimpleLSC())
            aScript = maCachedScript;
    }
    return aScript;
}


rtl::OUString LanguageTag::getRegionFromLangtag()
{
    OUString aRegion;
    if (meIsLiblangtagNeeded != DECISION_NO && !mpImplLangtag)
        canonicalize();
    if (maBcp47.isEmpty())
        return aRegion;
    if (mpImplLangtag)
    {
        const lt_region_t* pRegionT = lt_tag_get_region( MPLANGTAG);
        // pRegionT==NULL is valid for language only tags, rough check here
        // that does not take sophisticated tags into account that actually
        // should have a region, check for ll, lll, ll-Ssss and lll-Ssss so
        // that ll-CC and lll-CC actually fail.
        SAL_WARN_IF( !pRegionT &&
                maBcp47.getLength() != 2 && maBcp47.getLength() != 3 &&
                maBcp47.getLength() != 7 && maBcp47.getLength() != 8,
                "i18npool.langtag", "LanguageTag::getRegionFromLangtag: pRegionT==NULL");
        if (!pRegionT)
            return aRegion;
        const char* pRegion = lt_region_get_tag( pRegionT);
        SAL_WARN_IF( !pRegion, "i18npool.langtag", "LanguageTag::getRegionFromLangtag: pRegion==NULL");
        if (pRegion)
            aRegion = OUString::createFromAscii( pRegion);
    }
    else
    {
        if (mbCachedCountry || cacheSimpleLSC())
            aRegion = maCachedCountry;
    }
    return aRegion;
}


const com::sun::star::lang::Locale & LanguageTag::getLocale( bool bResolveSystem ) const
{
    if (!bResolveSystem && mbSystemLocale)
        return theEmptyLocale::get();
    if (!mbInitializedLocale)
    {
        if (mbInitializedBcp47)
            const_cast<LanguageTag*>(this)->convertBcp47ToLocale();
        else
            const_cast<LanguageTag*>(this)->convertLangToLocale();
    }
    return maLocale;
}


LanguageType LanguageTag::getLanguageType( bool bResolveSystem ) const
{
    if (!bResolveSystem && mbSystemLocale)
        return LANGUAGE_SYSTEM;
    if (!mbInitializedLangID)
    {
        if (mbInitializedBcp47)
            const_cast<LanguageTag*>(this)->convertBcp47ToLang();
        else
            const_cast<LanguageTag*>(this)->convertLocaleToLang();
    }
    return mnLangID;
}


void LanguageTag::getIsoLanguageCountry( rtl::OUString& rLanguage, rtl::OUString& rCountry ) const
{
    if (!isIsoLocale())
    {
        rLanguage = OUString();
        rCountry = OUString();
        return;
    }
    // After isIsoLocale() it's safe to call getLanguage() for ISO code.
    rLanguage = getLanguage();
    rCountry = getCountry();
}


namespace
{

bool isLowerAscii( sal_Unicode c )
{
    return 'a' <= c && c <= 'z';
}

bool isUpperAscii( sal_Unicode c )
{
    return 'A' <= c && c <= 'Z';
}

}


// static
bool LanguageTag::isIsoLanguage( const rtl::OUString& rLanguage )
{
    /* TODO: ignore case? For now let's see where rubbish is used. */
    bool b2chars;
    if (((b2chars = (rLanguage.getLength() == 2)) || rLanguage.getLength() == 3) &&
            isLowerAscii( rLanguage[0]) && isLowerAscii( rLanguage[1]) &&
            (b2chars || isLowerAscii( rLanguage[2])))
        return true;
    SAL_WARN_IF( ((rLanguage.getLength() == 2 || rLanguage.getLength() == 3) &&
                (isUpperAscii( rLanguage[0]) || isUpperAscii( rLanguage[1]))) ||
            (rLanguage.getLength() == 3 && isUpperAscii( rLanguage[2])), "i18npool.langtag",
            "LanguageTag::isIsoLanguage: rejecting upper case " << rLanguage);
    return false;
}


// static
bool LanguageTag::isIsoCountry( const rtl::OUString& rRegion )
{
    /* TODO: ignore case? For now let's see where rubbish is used. */
    if (rRegion.isEmpty() ||
            (rRegion.getLength() == 2 && isUpperAscii( rRegion[0]) && isUpperAscii( rRegion[1])))
        return true;
    SAL_WARN_IF( rRegion.getLength() == 2 && (isLowerAscii( rRegion[0]) || isLowerAscii( rRegion[1])),
            "i18npool.langtag", "LanguageTag::isIsoCountry: rejecting lower case " << rRegion);
    return false;
}


// static
bool LanguageTag::isIsoScript( const rtl::OUString& rScript )
{
    /* TODO: ignore case? For now let's see where rubbish is used. */
    if (rScript.isEmpty() ||
            (rScript.getLength() == 4 &&
             isUpperAscii( rScript[0]) && isLowerAscii( rScript[1]) &&
             isLowerAscii( rScript[2]) && isLowerAscii( rScript[3])))
        return true;
    SAL_WARN_IF( rScript.getLength() == 4 &&
            (isLowerAscii( rScript[0]) || isUpperAscii( rScript[1]) ||
             isUpperAscii( rScript[2]) || isUpperAscii( rScript[3])),
            "i18npool.langtag", "LanguageTag::isIsoScript: rejecting case mismatch " << rScript);
    return false;
}


rtl::OUString LanguageTag::getLanguage() const
{
    if (!mbCachedLanguage)
    {
        maCachedLanguage = const_cast<LanguageTag*>(this)->getLanguageFromLangtag();
        mbCachedLanguage = true;
    }
    return maCachedLanguage;
}


rtl::OUString LanguageTag::getScript() const
{
    if (!mbCachedScript)
    {
        maCachedScript = const_cast<LanguageTag*>(this)->getScriptFromLangtag();
        mbCachedScript = true;
    }
    return maCachedScript;
}


rtl::OUString LanguageTag::getLanguageAndScript() const
{
    OUString aLanguageScript( getLanguage());
    OUString aScript( getScript());
    if (!aScript.isEmpty())
    {
        OUStringBuffer aBuf( aLanguageScript.getLength() + 1 + aScript.getLength());
        aBuf.append( aLanguageScript).append( '-').append( aScript);
        aLanguageScript = aBuf.makeStringAndClear();
    }
    return aLanguageScript;
}


rtl::OUString LanguageTag::getCountry() const
{
    if (!mbCachedCountry)
    {
        maCachedCountry = const_cast<LanguageTag*>(this)->getRegionFromLangtag();
        if (!isIsoCountry( maCachedCountry))
            maCachedCountry = OUString();
        mbCachedCountry = true;
    }
    return maCachedCountry;
}


rtl::OUString LanguageTag::getRegion() const
{
    return const_cast<LanguageTag*>(this)->getRegionFromLangtag();
}


bool LanguageTag::hasScript() const
{
    if (!mbCachedScript)
        getScript();
    return !maCachedScript.isEmpty();
}


bool LanguageTag::cacheSimpleLSC()
{
    OUString aLanguage, aScript, aCountry;
    bool bRet = simpleExtract( maBcp47, aLanguage, aScript, aCountry);
    if (bRet)
    {
        maCachedLanguage = aLanguage;
        maCachedScript   = aScript;
        maCachedCountry  = aCountry;
        mbCachedLanguage = mbCachedScript = mbCachedCountry = true;
    }
    return bRet;
}


bool LanguageTag::isIsoLocale() const
{
    if (meIsIsoLocale == DECISION_DONTKNOW)
    {
        if (meIsLiblangtagNeeded != DECISION_NO && !mpImplLangtag)
            const_cast<LanguageTag*>(this)->canonicalize();
        // It must be at most ll-CC or lll-CC
        // Do not use getCountry() here, use getRegion() instead.
        meIsIsoLocale = ((maBcp47.isEmpty() ||
                    (maBcp47.getLength() <= 6 && isIsoLanguage( getLanguage()) && isIsoCountry( getRegion()))) ?
                DECISION_YES : DECISION_NO);
    }
    return meIsIsoLocale == DECISION_YES;
}


bool LanguageTag::isIsoODF() const
{
    if (meIsIsoODF == DECISION_DONTKNOW)
    {
        if (meIsLiblangtagNeeded != DECISION_NO && !mpImplLangtag)
            const_cast<LanguageTag*>(this)->canonicalize();
        if (!isIsoScript( getScript()))
            return ((meIsIsoODF = DECISION_NO) == DECISION_YES);
        // The usual case is lll-CC so simply check that first.
        if (isIsoLocale())
            return ((meIsIsoODF = DECISION_YES) == DECISION_YES);
        // If this is not ISO locale for which script must not exist it can
        // still be ISO locale plus ISO script lll-Ssss-CC
        meIsIsoODF = ((maBcp47.getLength() <= 11 &&
                    isIsoLanguage( getLanguage()) && isIsoCountry( getRegion()) && isIsoScript( getScript())) ?
                DECISION_YES : DECISION_NO);
    }
    return meIsIsoODF == DECISION_YES;
}


bool LanguageTag::isValidBcp47() const
{
    if (meIsValid == DECISION_DONTKNOW)
    {
        if (meIsLiblangtagNeeded != DECISION_NO && !mpImplLangtag)
           const_cast<LanguageTag*>(this)->canonicalize();
        SAL_WARN_IF( meIsValid == DECISION_DONTKNOW, "i18npool.langtag",
                "LanguageTag::isValidBcp47: canonicalize() didn't set meIsValid");
    }
    return meIsValid == DECISION_YES;
}


bool LanguageTag::isSystemLocale() const
{
    return mbSystemLocale;
}


LanguageTag & LanguageTag::makeFallback()
{
    if (!mbIsFallback)
    {
        if (mbInitializedLangID)
        {
            LanguageType nLang1 = getLanguageType();
            LanguageType nLang2 = MsLangId::Conversion::lookupFallbackLanguage( nLang1);
            if (nLang1 != nLang2)
                reset( nLang2);
        }
        else
        {
            const lang::Locale& rLocale1 = getLocale();
            lang::Locale aLocale2( MsLangId::Conversion::lookupFallbackLocale( rLocale1));
            if (    rLocale1.Language != aLocale2.Language ||
                    rLocale1.Country  != aLocale2.Country ||
                    rLocale1.Variant  != aLocale2.Variant)
                reset( aLocale2);
        }
        mbIsFallback = true;
    }
    return *this;
}


::std::vector< OUString > LanguageTag::getFallbackStrings() const
{
    ::std::vector< OUString > aVec;
    OUString aLanguage( getLanguage());
    OUString aCountry( getCountry());
    if (isIsoLocale())
    {
        if (!aCountry.isEmpty())
            aVec.push_back( aLanguage + "-" + aCountry);
        aVec.push_back( aLanguage);
        return aVec;
    }
    aVec.push_back( getBcp47());
    OUString aTmp;
    if (hasScript())
    {
        OUString aScript( getScript());
        if (!aCountry.isEmpty())
        {
            aTmp = aLanguage + "-" + aScript + "-" + aCountry;
            if (aTmp != aVec[0])
                aVec.push_back( aTmp);
        }
        aTmp = aLanguage + "-" + aScript;
        if (aTmp != aVec[0])
            aVec.push_back( aTmp);
    }
    if (!aCountry.isEmpty())
    {
        aTmp = aLanguage + "-" + aCountry;
        if (aTmp != aVec[0])
            aVec.push_back( aTmp);
    }
    aTmp = aLanguage;
    if (aTmp != aVec[0])
        aVec.push_back( aTmp);
    return aVec;
}


bool LanguageTag::operator==( const LanguageTag & rLanguageTag ) const
{
    // Compare full language tag strings but SYSTEM unresolved.
    return getBcp47( false) == rLanguageTag.getBcp47( false);
}


bool LanguageTag::operator!=( const LanguageTag & rLanguageTag ) const
{
    return !operator==( rLanguageTag);
}


// static
bool LanguageTag::simpleExtract( const rtl::OUString& rBcp47,
                                 rtl::OUString& rLanguage,
                                 rtl::OUString& rScript,
                                 rtl::OUString& rCountry )
{
    bool bRet = false;
    const sal_Int32 nLen = rBcp47.getLength();
    const sal_Int32 nHyph1 = rBcp47.indexOf( '-');
    if ((nLen == 2 || nLen == 3) && nHyph1 < 0)     // ll or lll
    {
        rLanguage = rBcp47;
        rScript = rCountry = OUString();
        bRet = true;
    }
    else if (  (nLen == 5 && nHyph1 == 2)           // ll-CC
            || (nLen == 6 && nHyph1 == 3))          // lll-CC
    {
        rLanguage = rBcp47.copy( 0, nHyph1);
        rCountry  = rBcp47.copy( nHyph1 + 1, 2);
        rScript = OUString();
        bRet = true;
    }
    else if (  (nHyph1 == 2 && nLen == 10)          // ll-Ssss-CC check
            || (nHyph1 == 3 && nLen == 11))         // lll-Ssss-CC check
    {
        const sal_Int32 nHyph2 = rBcp47.indexOf( '-', nHyph1 + 1);
        if (nHyph2 == nHyph1 + 5)
        {
            rLanguage = rBcp47.copy( 0, nHyph1);
            rScript   = rBcp47.copy( nHyph1 + 1, 4);
            rCountry  = rBcp47.copy( nHyph2 + 1, 2);
            bRet = true;
        }
    }
    if (!bRet)
        rLanguage = rScript = rCountry = OUString();
    return bRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
