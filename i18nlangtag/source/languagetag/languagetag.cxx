/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_folders.h>

#include "i18nlangtag/languagetag.hxx"
#include "i18nlangtag/mslangid.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/file.hxx>
#include <rtl/instance.hxx>
#include <rtl/locale.h>
#include <boost/unordered_set.hpp>

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

using namespace com::sun::star;


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

typedef ::boost::unordered_set< OUString, OUStringHash > KnownTagSet;
namespace {
struct theKnowns : public rtl::Static< KnownTagSet, theKnowns > {};
struct theMutex : public rtl::Static< osl::Mutex, theMutex > {};
}

static const KnownTagSet & getKnowns()
{
    KnownTagSet & rKnowns = theKnowns::get();
    if (rKnowns.empty())
    {
        osl::MutexGuard aGuard( theMutex::get());
        if (rKnowns.empty())
        {
            ::std::vector< MsLangId::LanguagetagMapping > aDefined( MsLangId::getDefinedLanguagetags());
            for (::std::vector< MsLangId::LanguagetagMapping >::const_iterator it( aDefined.begin());
                    it != aDefined.end(); ++it)
            {
                // Do not use the BCP47 string here to initialize the
                // LanguageTag because then canonicalize() would call this
                // getKnowns() again..
                ::std::vector< OUString > aFallbacks( LanguageTag( (*it).mnLang).getFallbackStrings( true));
                for (::std::vector< OUString >::const_iterator fb( aFallbacks.begin()); fb != aFallbacks.end(); ++fb)
                {
                    rKnowns.insert( *fb);
                }
            }
        }
    }
    return rKnowns;
}


/* TODO: this is how on-the-fly LangID assignment will work, now implement
 * usage and registration. */
#if 0
static LanguageType getNextOnTheFlyLanguage()
{
    static LanguageType nOnTheFlyLanguage = 0;
    osl::MutexGuard aGuard( theMutex::get());
    if (!nOnTheFlyLanguage)
        nOnTheFlyLanguage = MsLangId::makeLangID( LANGUAGE_ON_THE_FLY_SUB_START, LANGUAGE_ON_THE_FLY_START);
    else
    {
        if (MsLangId::getPrimaryLanguage( nOnTheFlyLanguage) != LANGUAGE_ON_THE_FLY_END)
            ++nOnTheFlyLanguage;
        else
        {
            LanguageType nSub = MsLangId::getSubLanguage( nOnTheFlyLanguage);
            if (nSub != LANGUAGE_ON_THE_FLY_SUB_END)
                nOnTheFlyLanguage = MsLangId::makeLangID( ++nSub, LANGUAGE_ON_THE_FLY_START);
            else
                SAL_WARN( "i18nlangtag", "getNextOnTheFlyLanguage: none left! ("
                        << ((LANGUAGE_ON_THE_FLY_END - LANGUAGE_ON_THE_FLY_START + 1)
                            * (LANGUAGE_ON_THE_FLY_SUB_END - LANGUAGE_ON_THE_FLY_SUB_START + 1))
                        << " consumed?!?)");
        }
    }
    return nOnTheFlyLanguage;
}
#endif


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
    OString maDataPath;   // path to liblangtag data, "|" if system
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
    SAL_INFO( "i18nlangtag", "LiblantagDataRef::setup: initializing database");
    if (maDataPath.isEmpty())
        setupDataPath();
    lt_db_initialize();
    // Hold ref eternally.
    mnRef = SAL_MAX_UINT32;
}

void LiblantagDataRef::teardown()
{
    SAL_INFO( "i18nlangtag", "LiblantagDataRef::teardown: finalizing database");
    lt_db_finalize();
}

void LiblantagDataRef::setupDataPath()
{
    // maDataPath is assumed to be empty here.
    OUString aURL("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/liblangtag");
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


class LanguageTagImpl
{
public:

    explicit LanguageTagImpl( const LanguageTag & rLanguageTag );
    explicit LanguageTagImpl( const LanguageTagImpl & rLanguageTagImpl );
    ~LanguageTagImpl();
    LanguageTagImpl& operator=( const LanguageTagImpl & rLanguageTagImpl );

private:

    friend class LanguageTag;

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
    mutable OUString                        maCachedVariants;   ///< cache getVariants()
    mutable lt_tag_t*                       mpImplLangtag;      ///< liblangtag pointer
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
    mutable bool                            mbCachedVariants    : 1;

    const OUString &    getBcp47( bool bResolveSystem = true ) const;
    OUString            getLanguage() const;
    OUString            getScript() const;
    OUString            getCountry() const;
    OUString            getRegion() const;
    OUString            getVariants() const;
    bool                hasScript() const;

    bool                isIsoLocale() const;
    bool                isIsoODF() const;
    bool                isValidBcp47() const;

    void                convertLocaleToBcp47();
    void                convertLocaleToLang();
    void                convertBcp47ToLocale();
    void                convertBcp47ToLang();
    void                convertLangToLocale();
    void                convertLangToBcp47();

    void                convertFromRtlLocale();

    /** @return whether BCP 47 language tag string was changed. */
    bool                canonicalize();

    /** Canonicalize if not yet done and synchronize initialized conversions.

        @return whether BCP 47 language tag string was changed.
     */
    bool                synCanonicalize();

    OUString            getLanguageFromLangtag();
    OUString            getScriptFromLangtag();
    OUString            getRegionFromLangtag();
    OUString            getVariantsFromLangtag();

    void                resetVars();

    /** Obtain Language, Script, Country and Variants via simpleExtract() and
        assign them to the cached variables if successful.

        @return return of simpleExtract()
     */
    bool                cacheSimpleLSCV();

    enum Extraction
    {
        EXTRACTED_NONE,
        EXTRACTED_LSC,
        EXTRACTED_LV,
        EXTRACTED_X,
        EXTRACTED_X_JOKER
    };

    /** Of a language tag of the form lll[-Ssss][-CC][-vvvvvvvv] extract the
        portions.

        Does not check case or content!

        @return EXTRACTED_LSC if simple tag was detected (i.e. one that
                would fulfill the isIsoODF() condition),
                EXTRACTED_LV if a tag with variant was detected,
                EXTRACTED_X if x-... privateuse tag was detected,
                EXTRACTED_X_JOKER if "*" joker was detected,
                EXTRACTED_NONE else.
     */
    static Extraction   simpleExtract( const OUString& rBcp47,
                                       OUString& rLanguage,
                                       OUString& rScript,
                                       OUString& rCountry,
                                       OUString& rVariants );

};


LanguageTagImpl::LanguageTagImpl( const LanguageTag & rLanguageTag )
    :
        maLocale( rLanguageTag.maLocale),
        maBcp47( rLanguageTag.maBcp47),
        mpImplLangtag( NULL),
        mnLangID( rLanguageTag.mnLangID),
        meIsValid( DECISION_DONTKNOW),
        meIsIsoLocale( DECISION_DONTKNOW),
        meIsIsoODF( DECISION_DONTKNOW),
        meIsLiblangtagNeeded( DECISION_DONTKNOW),
        mbSystemLocale( rLanguageTag.mbSystemLocale),
        mbInitializedBcp47( rLanguageTag.mbInitializedBcp47),
        mbInitializedLocale( rLanguageTag.mbInitializedLocale),
        mbInitializedLangID( rLanguageTag.mbInitializedLangID),
        mbCachedLanguage( false),
        mbCachedScript( false),
        mbCachedCountry( false),
        mbCachedVariants( false)
{
}


LanguageTagImpl::LanguageTagImpl( const LanguageTagImpl & rLanguageTagImpl )
    :
        maLocale( rLanguageTagImpl.maLocale),
        maBcp47( rLanguageTagImpl.maBcp47),
        maCachedLanguage( rLanguageTagImpl.maCachedLanguage),
        maCachedScript( rLanguageTagImpl.maCachedScript),
        maCachedCountry( rLanguageTagImpl.maCachedCountry),
        maCachedVariants( rLanguageTagImpl.maCachedVariants),
        mpImplLangtag( rLanguageTagImpl.mpImplLangtag ?
                lt_tag_copy( rLanguageTagImpl.mpImplLangtag) : NULL),
        mnLangID( rLanguageTagImpl.mnLangID),
        meIsValid( rLanguageTagImpl.meIsValid),
        meIsIsoLocale( rLanguageTagImpl.meIsIsoLocale),
        meIsIsoODF( rLanguageTagImpl.meIsIsoODF),
        meIsLiblangtagNeeded( rLanguageTagImpl.meIsLiblangtagNeeded),
        mbSystemLocale( rLanguageTagImpl.mbSystemLocale),
        mbInitializedBcp47( rLanguageTagImpl.mbInitializedBcp47),
        mbInitializedLocale( rLanguageTagImpl.mbInitializedLocale),
        mbInitializedLangID( rLanguageTagImpl.mbInitializedLangID),
        mbCachedLanguage( rLanguageTagImpl.mbCachedLanguage),
        mbCachedScript( rLanguageTagImpl.mbCachedScript),
        mbCachedCountry( rLanguageTagImpl.mbCachedCountry),
        mbCachedVariants( rLanguageTagImpl.mbCachedVariants)
{
    if (mpImplLangtag)
        theDataRef::get().incRef();
}


LanguageTagImpl& LanguageTagImpl::operator=( const LanguageTagImpl & rLanguageTagImpl )
{
    maLocale            = rLanguageTagImpl.maLocale;
    maBcp47             = rLanguageTagImpl.maBcp47;
    maCachedLanguage    = rLanguageTagImpl.maCachedLanguage;
    maCachedScript      = rLanguageTagImpl.maCachedScript;
    maCachedCountry     = rLanguageTagImpl.maCachedCountry;
    maCachedVariants    = rLanguageTagImpl.maCachedVariants;
    mpImplLangtag       = rLanguageTagImpl.mpImplLangtag;
    mpImplLangtag       = rLanguageTagImpl.mpImplLangtag ?
                            lt_tag_copy( rLanguageTagImpl.mpImplLangtag) : NULL;
    mnLangID            = rLanguageTagImpl.mnLangID;
    meIsValid           = rLanguageTagImpl.meIsValid;
    meIsIsoLocale       = rLanguageTagImpl.meIsIsoLocale;
    meIsIsoODF          = rLanguageTagImpl.meIsIsoODF;
    meIsLiblangtagNeeded= rLanguageTagImpl.meIsLiblangtagNeeded;
    mbSystemLocale      = rLanguageTagImpl.mbSystemLocale;
    mbInitializedBcp47  = rLanguageTagImpl.mbInitializedBcp47;
    mbInitializedLocale = rLanguageTagImpl.mbInitializedLocale;
    mbInitializedLangID = rLanguageTagImpl.mbInitializedLangID;
    mbCachedLanguage    = rLanguageTagImpl.mbCachedLanguage;
    mbCachedScript      = rLanguageTagImpl.mbCachedScript;
    mbCachedCountry     = rLanguageTagImpl.mbCachedCountry;
    mbCachedVariants    = rLanguageTagImpl.mbCachedVariants;
    if (mpImplLangtag)
        theDataRef::get().incRef();
    return *this;
}


LanguageTagImpl::~LanguageTagImpl()
{
    if (mpImplLangtag)
    {
        lt_tag_unref( mpImplLangtag);
        theDataRef::get().decRef();
    }
}


void LanguageTagImpl::resetVars()
{
    if (mpImplLangtag)
    {
        lt_tag_unref( mpImplLangtag);
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
    if (!maCachedVariants.isEmpty())
        maCachedVariants = OUString();
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
    mbCachedVariants    = false;
}


LanguageTag::LanguageTag( const OUString & rBcp47LanguageTag, bool bCanonicalize )
    :
        maBcp47( rBcp47LanguageTag),
        mnLangID( LANGUAGE_DONTKNOW),
        mbSystemLocale( rBcp47LanguageTag.isEmpty()),
        mbInitializedBcp47( !mbSystemLocale),
        mbInitializedLocale( false),
        mbInitializedLangID( false),
        mbIsFallback( false)
{
    if (bCanonicalize)
    {
        if (getImpl()->canonicalize())
            syncFromImpl();
    }

}


LanguageTag::LanguageTag( const com::sun::star::lang::Locale & rLocale )
    :
        maLocale( rLocale),
        mnLangID( LANGUAGE_DONTKNOW),
        mbSystemLocale( rLocale.Language.isEmpty()),
        mbInitializedBcp47( false),
        mbInitializedLocale( !mbSystemLocale),
        mbInitializedLangID( false),
        mbIsFallback( false)
{
}


LanguageTag::LanguageTag( LanguageType nLanguage )
    :
        mnLangID( nLanguage),
        mbSystemLocale( nLanguage == LANGUAGE_SYSTEM),
        mbInitializedBcp47( false),
        mbInitializedLocale( false),
        mbInitializedLangID( !mbSystemLocale),
        mbIsFallback( false)
{
}


LanguageTag::LanguageTag( const OUString& rBcp47, const OUString& rLanguage,
                          const OUString& rScript, const OUString& rCountry )
    :
        maBcp47( rBcp47),
        mnLangID( LANGUAGE_DONTKNOW),
        mbSystemLocale( rBcp47.isEmpty() && rLanguage.isEmpty()),
        mbInitializedBcp47( !rBcp47.isEmpty()),
        mbInitializedLocale( false),
        mbInitializedLangID( false),
        mbIsFallback( false)
{
    if (!mbSystemLocale && !mbInitializedBcp47)
    {
        if (rScript.isEmpty())
        {
            maBcp47 = rLanguage + "-" + rCountry;
            mbInitializedBcp47 = true;
            maLocale.Language = rLanguage;
            maLocale.Country  = rCountry;
            mbInitializedLocale = true;
        }
        else
        {
            if (rCountry.isEmpty())
                maBcp47 = rLanguage + "-" + rScript;
            else
                maBcp47 = rLanguage + "-" + rScript + "-" + rCountry;
            mbInitializedBcp47 = true;
            maLocale.Language = I18NLANGTAG_QLT;
            maLocale.Country  = rCountry;
            maLocale.Variant  = maBcp47;
            mbInitializedLocale = true;
        }
    }
}


LanguageTag::LanguageTag( const rtl_Locale & rLocale )
    :
        maLocale( rLocale.Language, rLocale.Country, rLocale.Variant),
        mnLangID( LANGUAGE_DONTKNOW),
        mbSystemLocale( maLocale.Language.isEmpty()),
        mbInitializedBcp47( false),
        mbInitializedLocale( !mbSystemLocale),
        mbInitializedLangID( false),
        mbIsFallback( false)
{
    convertFromRtlLocale();
}


LanguageTag::LanguageTag( const LanguageTag & rLanguageTag )
    :
        maLocale( rLanguageTag.maLocale),
        maBcp47( rLanguageTag.maBcp47),
        mnLangID( rLanguageTag.mnLangID),
        mpImpl( rLanguageTag.mpImpl),
        mbSystemLocale( rLanguageTag.mbSystemLocale),
        mbInitializedBcp47( rLanguageTag.mbInitializedBcp47),
        mbInitializedLocale( rLanguageTag.mbInitializedLocale),
        mbInitializedLangID( rLanguageTag.mbInitializedLangID)
{
}


LanguageTag& LanguageTag::operator=( const LanguageTag & rLanguageTag )
{
    maLocale            = rLanguageTag.maLocale;
    maBcp47             = rLanguageTag.maBcp47;
    mnLangID            = rLanguageTag.mnLangID;
    mpImpl              = rLanguageTag.mpImpl;
    mbSystemLocale      = rLanguageTag.mbSystemLocale;
    mbInitializedBcp47  = rLanguageTag.mbInitializedBcp47;
    mbInitializedLocale = rLanguageTag.mbInitializedLocale;
    mbInitializedLangID = rLanguageTag.mbInitializedLangID;
    return *this;
}


LanguageTag::~LanguageTag()
{
}


LanguageTagImpl* LanguageTag::getImpl() const
{
    if (!mpImpl)
        mpImpl.reset( new LanguageTagImpl( *this));
    return mpImpl.get();
}


void LanguageTag::resetVars()
{
    mpImpl.reset();
    maLocale            = lang::Locale();
    maBcp47             = OUString();
    mnLangID            = LANGUAGE_SYSTEM;
    mbSystemLocale      = true;
    mbInitializedBcp47  = false;
    mbInitializedLocale = false;
    mbInitializedLangID = false;
    mbIsFallback        = false;
}


void LanguageTag::reset( const OUString & rBcp47LanguageTag, bool bCanonicalize )
{
    resetVars();
    maBcp47             = rBcp47LanguageTag;
    mbSystemLocale      = rBcp47LanguageTag.isEmpty();
    mbInitializedBcp47  = !mbSystemLocale;

    if (bCanonicalize)
        getImpl()->canonicalize();
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


void LanguageTag::reset( const rtl_Locale & rLocale )
{
    reset( lang::Locale( rLocale.Language, rLocale.Country, rLocale.Variant));
    convertFromRtlLocale();
}


bool LanguageTagImpl::canonicalize()
{
#ifdef erDEBUG
    // dump once
    struct dumper
    {
        lt_tag_t** mpp;
        dumper( lt_tag_t** pp ) : mpp( *pp ? NULL : pp) {}
        ~dumper() { if (mpp && *mpp) lt_tag_dump( *mpp); }
    };
    dumper aDumper( &mpImplLangtag);
#endif

    bool bChanged = false;

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
                // locale or could fall back to one.
                OUString aLanguage, aScript, aCountry, aVariants;
                Extraction eExt = simpleExtract( maBcp47, aLanguage, aScript, aCountry, aVariants);
                if (eExt != EXTRACTED_NONE)
                {
                    if (eExt == EXTRACTED_LSC || eExt == EXTRACTED_LV)
                    {
                        // Rebuild bcp47 with proper casing of tags.
                        OUStringBuffer aBuf( aLanguage.getLength() + 1 + aScript.getLength() +
                                1 + aCountry.getLength() + 1 + aVariants.getLength());
                        aBuf.append( aLanguage);
                        if (!aScript.isEmpty())
                            aBuf.append("-" + aScript);
                        if (!aCountry.isEmpty())
                            aBuf.append("-" + aCountry);
                        if (!aVariants.isEmpty())
                            aBuf.append("-" + aVariants);
                        OUString aStr( aBuf.makeStringAndClear());

                        if (maBcp47 != aStr)
                        {
                            maBcp47 = aStr;
                            bChanged = true;
                        }
                    }
                    if (eExt == EXTRACTED_LSC && aScript.isEmpty())
                    {
                        maLocale.Language = aLanguage;
                        maLocale.Country  = aCountry;
                    }
                    else
                    {
                        maLocale.Language = I18NLANGTAG_QLT;
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
                else
                {
                    const KnownTagSet& rKnowns = getKnowns();
                    if (rKnowns.find( maBcp47) != rKnowns.end())
                        meIsLiblangtagNeeded = DECISION_NO; // known fallback
                }
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
        return bChanged;            // that's it
    }
    meIsLiblangtagNeeded = DECISION_YES;
    SAL_INFO( "i18nlangtag", "LanguageTagImpl::canonicalize: using liblangtag for " << maBcp47);

    if (!mpImplLangtag)
    {
        theDataRef::get().incRef();
        mpImplLangtag = lt_tag_new();
    }

    myLtError aError;

    if (lt_tag_parse( mpImplLangtag, OUStringToOString( maBcp47, RTL_TEXTENCODING_UTF8).getStr(), &aError.p))
    {
        char* pTag = lt_tag_canonicalize( mpImplLangtag, &aError.p);
        SAL_WARN_IF( !pTag, "i18nlangtag", "LanguageTagImpl::canonicalize: could not canonicalize " << maBcp47);
        if (pTag)
        {
            OUString aOld( maBcp47);
            maBcp47 = OUString::createFromAscii( pTag);
            // Make the lt_tag_t follow the new string if different, which
            // removes default script and such.
            if (maBcp47 != aOld)
            {
                bChanged = true;
                meIsIsoLocale = DECISION_DONTKNOW;
                meIsIsoODF = DECISION_DONTKNOW;
                if (!lt_tag_parse( mpImplLangtag, pTag, &aError.p))
                {
                    SAL_WARN( "i18nlangtag", "LanguageTagImpl::canonicalize: could not reparse " << maBcp47);
                    free( pTag);
                    meIsValid = DECISION_NO;
                    return bChanged;
                }
            }
            free( pTag);
            meIsValid = DECISION_YES;
            return bChanged;
        }
    }
    else
    {
        SAL_INFO( "i18nlangtag", "LanguageTagImpl::canonicalize: could not parse " << maBcp47);
    }
    meIsValid = DECISION_NO;
    return bChanged;
}


bool LanguageTagImpl::synCanonicalize()
{
    bool bChanged = false;
    if (meIsLiblangtagNeeded != DECISION_NO && !mpImplLangtag)
    {
        bChanged = canonicalize();
        if (bChanged)
        {
            if (mbInitializedLocale)
                convertBcp47ToLocale();
            if (mbInitializedLangID)
                convertBcp47ToLang();
        }
    }
    return bChanged;
}


void LanguageTag::syncFromImpl()
{
    LanguageTagImpl* pImpl = getImpl();
    mbInitializedBcp47  = pImpl->mbInitializedBcp47;
    maBcp47             = pImpl->maBcp47;
    mbInitializedLocale = pImpl->mbInitializedLocale;
    maLocale            = pImpl->maLocale;
    mbInitializedLangID = pImpl->mbInitializedLangID;
    mnLangID            = pImpl->mnLangID;
}


bool LanguageTag::synCanonicalize()
{
    bool bChanged = getImpl()->synCanonicalize();
    if (bChanged)
        syncFromImpl();
    return bChanged;
}


void LanguageTagImpl::convertLocaleToBcp47()
{
    if (mbSystemLocale && !mbInitializedLocale)
        convertLangToLocale();

    if (maLocale.Language == I18NLANGTAG_QLT)
    {
        maBcp47 = maLocale.Variant;
        meIsIsoLocale = DECISION_NO;
    }
    else
    {
        maBcp47 = LanguageTag::convertToBcp47( maLocale, true);
    }
    mbInitializedBcp47 = true;
}


void LanguageTag::convertLocaleToBcp47()
{
    LanguageTagImpl* pImpl = getImpl();
    pImpl->convertLocaleToBcp47();
    maBcp47 = pImpl->maBcp47;
    mbInitializedBcp47 = pImpl->mbInitializedBcp47;
}


void LanguageTagImpl::convertLocaleToLang()
{
    if (mbSystemLocale)
    {
        mnLangID = MsLangId::getRealLanguage( LANGUAGE_SYSTEM);
    }
    else
    {
        mnLangID = LanguageTag::convertToLanguageType( maLocale, true);
    }
    mbInitializedLangID = true;
}


void LanguageTag::convertLocaleToLang()
{
    LanguageTagImpl* pImpl = getImpl();
    pImpl->convertLocaleToLang();
    mnLangID = pImpl->mnLangID;
    mbInitializedLangID = pImpl->mbInitializedLangID;
}


void LanguageTagImpl::convertBcp47ToLocale()
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
        maLocale.Language = I18NLANGTAG_QLT;
        maLocale.Country = getCountry();
        maLocale.Variant = maBcp47;
    }
    mbInitializedLocale = true;
}


void LanguageTag::convertBcp47ToLocale()
{
    LanguageTagImpl* pImpl = getImpl();
    pImpl->convertBcp47ToLocale();
    maLocale = pImpl->maLocale;
    mbInitializedLocale = pImpl->mbInitializedLocale;
}


void LanguageTagImpl::convertBcp47ToLang()
{
    if (mbSystemLocale)
    {
        mnLangID = MsLangId::getRealLanguage( LANGUAGE_SYSTEM);
    }
    else
    {
        if (!mbInitializedLocale)
            convertBcp47ToLocale();
        convertLocaleToLang();
    }
    mbInitializedLangID = true;
}


void LanguageTag::convertBcp47ToLang()
{
    LanguageTagImpl* pImpl = getImpl();
    pImpl->convertBcp47ToLang();
    mnLangID = pImpl->mnLangID;
    mbInitializedLangID = pImpl->mbInitializedLangID;
}


void LanguageTagImpl::convertLangToLocale()
{
    if (mbSystemLocale && !mbInitializedLangID)
    {
        mnLangID = MsLangId::getRealLanguage( LANGUAGE_SYSTEM);
        mbInitializedLangID = true;
    }
    // Resolve system here! The original is remembered as mbSystemLocale.
    maLocale = LanguageTag::convertToLocale( mnLangID, true);
    mbInitializedLocale = true;
}


void LanguageTag::convertLangToLocale()
{
    LanguageTagImpl* pImpl = getImpl();
    pImpl->convertLangToLocale();
    maLocale = pImpl->maLocale;
    mbInitializedLocale = pImpl->mbInitializedLocale;
}


void LanguageTagImpl::convertLangToBcp47()
{
    if (!mbInitializedLocale)
        convertLangToLocale();
    convertLocaleToBcp47();
    mbInitializedBcp47 = true;
}


void LanguageTag::convertLangToBcp47()
{
    LanguageTagImpl* pImpl = getImpl();
    pImpl->convertLangToBcp47();
    maBcp47 = pImpl->maBcp47;
    mbInitializedBcp47 = pImpl->mbInitializedBcp47;
}


void LanguageTag::convertFromRtlLocale()
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
        maBcp47 = OStringToOUString( lt_tag_get_string( mpImplLangtag), RTL_TEXTENCODING_UTF8);
        mbInitializedBcp47 = true;
#else
        mnLangID = MsLangId::convertUnxByteStringToLanguage( aStr);
        if (mnLangID == LANGUAGE_DONTKNOW)
        {
            SAL_WARN( "i18nlangtag", "LanguageTag(rtl_Locale) - unknown: " << aStr);
            mnLangID = LANGUAGE_ENGLISH_US;     // we need _something_ here
        }
        mbInitializedLangID = true;
#endif
        maLocale = lang::Locale();
        mbInitializedLocale = false;
    }
}


const OUString & LanguageTagImpl::getBcp47( bool bResolveSystem ) const
{
    if (!bResolveSystem && mbSystemLocale)
        return theEmptyBcp47::get();
    if (!mbInitializedBcp47)
    {
        if (mbInitializedLocale)
            const_cast<LanguageTagImpl*>(this)->convertLocaleToBcp47();
        else
            const_cast<LanguageTagImpl*>(this)->convertLangToBcp47();
    }
    return maBcp47;
}


const OUString & LanguageTag::getBcp47( bool bResolveSystem ) const
{
    if (!bResolveSystem && mbSystemLocale)
        return theEmptyBcp47::get();
    if (!mbInitializedBcp47)
        const_cast<LanguageTag*>(this)->syncFromImpl();
    if (!mbInitializedBcp47)
    {
        getImpl()->getBcp47( bResolveSystem);
        const_cast<LanguageTag*>(this)->syncFromImpl();
    }
    return maBcp47;
}


OUString LanguageTagImpl::getLanguageFromLangtag()
{
    OUString aLanguage;
    synCanonicalize();
    if (maBcp47.isEmpty())
        return aLanguage;
    if (mpImplLangtag)
    {
        const lt_lang_t* pLangT = lt_tag_get_language( mpImplLangtag);
        SAL_WARN_IF( !pLangT, "i18nlangtag", "LanguageTag::getLanguageFromLangtag: pLangT==NULL");
        if (!pLangT)
            return aLanguage;
        const char* pLang = lt_lang_get_tag( pLangT);
        SAL_WARN_IF( !pLang, "i18nlangtag", "LanguageTag::getLanguageFromLangtag: pLang==NULL");
        if (pLang)
            aLanguage = OUString::createFromAscii( pLang);
    }
    else
    {
        if (mbCachedLanguage || cacheSimpleLSCV())
            aLanguage = maCachedLanguage;
    }
    return aLanguage;
}


OUString LanguageTagImpl::getScriptFromLangtag()
{
    OUString aScript;
    synCanonicalize();
    if (maBcp47.isEmpty())
        return aScript;
    if (mpImplLangtag)
    {
        const lt_script_t* pScriptT = lt_tag_get_script( mpImplLangtag);
        // pScriptT==NULL is valid for default scripts
        if (!pScriptT)
            return aScript;
        const char* pScript = lt_script_get_tag( pScriptT);
        SAL_WARN_IF( !pScript, "i18nlangtag", "LanguageTag::getScriptFromLangtag: pScript==NULL");
        if (pScript)
            aScript = OUString::createFromAscii( pScript);
    }
    else
    {
        if (mbCachedScript || cacheSimpleLSCV())
            aScript = maCachedScript;
    }
    return aScript;
}


OUString LanguageTagImpl::getRegionFromLangtag()
{
    OUString aRegion;
    synCanonicalize();
    if (maBcp47.isEmpty())
        return aRegion;
    if (mpImplLangtag)
    {
        const lt_region_t* pRegionT = lt_tag_get_region( mpImplLangtag);
        // pRegionT==NULL is valid for language only tags, rough check here
        // that does not take sophisticated tags into account that actually
        // should have a region, check for ll, lll, ll-Ssss and lll-Ssss so
        // that ll-CC and lll-CC actually fail.
        SAL_WARN_IF( !pRegionT &&
                maBcp47.getLength() != 2 && maBcp47.getLength() != 3 &&
                maBcp47.getLength() != 7 && maBcp47.getLength() != 8,
                "i18nlangtag", "LanguageTag::getRegionFromLangtag: pRegionT==NULL");
        if (!pRegionT)
            return aRegion;
        const char* pRegion = lt_region_get_tag( pRegionT);
        SAL_WARN_IF( !pRegion, "i18nlangtag", "LanguageTag::getRegionFromLangtag: pRegion==NULL");
        if (pRegion)
            aRegion = OUString::createFromAscii( pRegion);
    }
    else
    {
        if (mbCachedCountry || cacheSimpleLSCV())
            aRegion = maCachedCountry;
    }
    return aRegion;
}


OUString LanguageTagImpl::getVariantsFromLangtag()
{
    OUString aVariants;
    synCanonicalize();
    if (maBcp47.isEmpty())
        return aVariants;
    if (mpImplLangtag)
    {
        const lt_list_t* pVariantsT = lt_tag_get_variants( mpImplLangtag);
        for (const lt_list_t* pE = pVariantsT; pE; pE = lt_list_next( pE))
        {
            const lt_variant_t* pVariantT = static_cast<const lt_variant_t*>(lt_list_value( pE));
            if (pVariantT)
            {
                const char* p = lt_variant_get_tag( pVariantT);
                if (p)
                {
                    if (aVariants.isEmpty())
                        aVariants = OUString::createFromAscii( p);
                    else
                        aVariants += "-" + OUString::createFromAscii( p);
                }
            }
        }
    }
    else
    {
        if (mbCachedVariants || cacheSimpleLSCV())
            aVariants = maCachedVariants;
    }
    return aVariants;
}


const com::sun::star::lang::Locale & LanguageTag::getLocale( bool bResolveSystem ) const
{
    if (!bResolveSystem && mbSystemLocale)
        return theEmptyLocale::get();
    if (!mbInitializedLocale)
        const_cast<LanguageTag*>(this)->syncFromImpl();
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
        const_cast<LanguageTag*>(this)->syncFromImpl();
    if (!mbInitializedLangID)
    {
        if (mbInitializedBcp47)
            const_cast<LanguageTag*>(this)->convertBcp47ToLang();
        else
        {
            const_cast<LanguageTag*>(this)->convertLocaleToLang();

            /* Resolve a locale only unknown due to some redundant information,
             * like 'de-Latn-DE' with script tag. Never call canonicalize()
             * from within convert...() methods due to possible recursion, so
             * do it here. */
            if ((!mbSystemLocale && mnLangID == LANGUAGE_SYSTEM) || mnLangID == LANGUAGE_DONTKNOW)
                const_cast<LanguageTag*>(this)->synCanonicalize();
        }
    }
    return mnLangID;
}


void LanguageTag::getIsoLanguageScriptCountry( OUString& rLanguage, OUString& rScript, OUString& rCountry ) const
{
    // Calling isIsoODF() first is a predicate for getLanguage(), getScript()
    // and getCountry() to work correctly in this context.
    if (isIsoODF())
    {
        rLanguage = getLanguage();
        rScript   = getScript();
        rCountry  = getCountry();
    }
    else
    {
        rLanguage = (LanguageTag::isIsoLanguage( getLanguage()) ? getLanguage() : OUString());
        rScript   = (LanguageTag::isIsoScript(   getScript())   ? getScript()   : OUString());
        rCountry  = (LanguageTag::isIsoCountry(  getCountry())  ? getCountry()  : OUString());
    }
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
bool LanguageTag::isIsoLanguage( const OUString& rLanguage )
{
    /* TODO: ignore case? For now let's see where rubbish is used. */
    bool b2chars;
    if (((b2chars = (rLanguage.getLength() == 2)) || rLanguage.getLength() == 3) &&
            isLowerAscii( rLanguage[0]) && isLowerAscii( rLanguage[1]) &&
            (b2chars || isLowerAscii( rLanguage[2])))
        return true;
    SAL_WARN_IF( ((rLanguage.getLength() == 2 || rLanguage.getLength() == 3) &&
                (isUpperAscii( rLanguage[0]) || isUpperAscii( rLanguage[1]))) ||
            (rLanguage.getLength() == 3 && isUpperAscii( rLanguage[2])), "i18nlangtag",
            "LanguageTag::isIsoLanguage: rejecting upper case " << rLanguage);
    return false;
}


// static
bool LanguageTag::isIsoCountry( const OUString& rRegion )
{
    /* TODO: ignore case? For now let's see where rubbish is used. */
    if (rRegion.isEmpty() ||
            (rRegion.getLength() == 2 && isUpperAscii( rRegion[0]) && isUpperAscii( rRegion[1])))
        return true;
    SAL_WARN_IF( rRegion.getLength() == 2 && (isLowerAscii( rRegion[0]) || isLowerAscii( rRegion[1])),
            "i18nlangtag", "LanguageTag::isIsoCountry: rejecting lower case " << rRegion);
    return false;
}


// static
bool LanguageTag::isIsoScript( const OUString& rScript )
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
            "i18nlangtag", "LanguageTag::isIsoScript: rejecting case mismatch " << rScript);
    return false;
}


OUString LanguageTagImpl::getLanguage() const
{
    if (!mbCachedLanguage)
    {
        maCachedLanguage = const_cast<LanguageTagImpl*>(this)->getLanguageFromLangtag();
        mbCachedLanguage = true;
    }
    return maCachedLanguage;
}


OUString LanguageTag::getLanguage() const
{
    return getImpl()->getLanguage();
}


OUString LanguageTagImpl::getScript() const
{
    if (!mbCachedScript)
    {
        maCachedScript = const_cast<LanguageTagImpl*>(this)->getScriptFromLangtag();
        mbCachedScript = true;
    }
    return maCachedScript;
}


OUString LanguageTag::getScript() const
{
    return getImpl()->getScript();
}


OUString LanguageTag::getLanguageAndScript() const
{
    OUString aLanguageScript( getLanguage());
    OUString aScript( getScript());
    if (!aScript.isEmpty())
    {
        aLanguageScript += "-" + aScript;
    }
    return aLanguageScript;
}


OUString LanguageTagImpl::getCountry() const
{
    if (!mbCachedCountry)
    {
        maCachedCountry = const_cast<LanguageTagImpl*>(this)->getRegionFromLangtag();
        if (!LanguageTag::isIsoCountry( maCachedCountry))
            maCachedCountry = OUString();
        mbCachedCountry = true;
    }
    return maCachedCountry;
}


OUString LanguageTag::getCountry() const
{
    return getImpl()->getCountry();
}


OUString LanguageTagImpl::getRegion() const
{
    return const_cast<LanguageTagImpl*>(this)->getRegionFromLangtag();
}


OUString LanguageTag::getRegion() const
{
    return getImpl()->getRegion();
}


OUString LanguageTagImpl::getVariants() const
{
    if (!mbCachedVariants)
    {
        maCachedVariants = const_cast<LanguageTagImpl*>(this)->getVariantsFromLangtag();
        mbCachedVariants = true;
    }
    return maCachedVariants;
}


OUString LanguageTag::getVariants() const
{
    return getImpl()->getVariants();
}


OUString LanguageTag::getGlibcLocaleString( const OUString & rEncoding ) const
{
    OUString aRet;
    if (isIsoLocale())
    {
        OUString aCountry( getCountry());
        if (aCountry.isEmpty())
            aRet = getLanguage() + rEncoding;
        else
            aRet = getLanguage() + "_" + aCountry + rEncoding;
    }
    else
    {
        /* FIXME: use the aImplIsoLangGLIBCModifiersEntries table from
         * i18nlangtag/source/isolang/isolang.cxx or let liblangtag handle it.
         * So far no code was prepared for anything else than a simple
         * language_country locale so we don't loose anything here right now.
         * */
    }
    return aRet;
}


bool LanguageTagImpl::hasScript() const
{
    if (!mbCachedScript)
        getScript();
    return !maCachedScript.isEmpty();
}


bool LanguageTag::hasScript() const
{
    return getImpl()->hasScript();
}


bool LanguageTagImpl::cacheSimpleLSCV()
{
    OUString aLanguage, aScript, aCountry, aVariants;
    Extraction eExt = simpleExtract( maBcp47, aLanguage, aScript, aCountry, aVariants);
    bool bRet = (eExt == EXTRACTED_LSC || eExt == EXTRACTED_LV);
    if (bRet)
    {
        maCachedLanguage = aLanguage;
        maCachedScript   = aScript;
        maCachedCountry  = aCountry;
        maCachedVariants = aVariants;
        mbCachedLanguage = mbCachedScript = mbCachedCountry = mbCachedVariants = true;
    }
    return bRet;
}


bool LanguageTagImpl::isIsoLocale() const
{
    if (meIsIsoLocale == DECISION_DONTKNOW)
    {
        const_cast<LanguageTagImpl*>(this)->synCanonicalize();
        // It must be at most ll-CC or lll-CC
        // Do not use getCountry() here, use getRegion() instead.
        meIsIsoLocale = ((maBcp47.isEmpty() ||
                    (maBcp47.getLength() <= 6 && LanguageTag::isIsoLanguage( getLanguage()) &&
                     LanguageTag::isIsoCountry( getRegion()))) ? DECISION_YES : DECISION_NO);
    }
    return meIsIsoLocale == DECISION_YES;
}


bool LanguageTag::isIsoLocale() const
{
    return getImpl()->isIsoLocale();
}


bool LanguageTagImpl::isIsoODF() const
{
    if (meIsIsoODF == DECISION_DONTKNOW)
    {
        const_cast<LanguageTagImpl*>(this)->synCanonicalize();
        if (!LanguageTag::isIsoScript( getScript()))
            return ((meIsIsoODF = DECISION_NO) == DECISION_YES);
        // The usual case is lll-CC so simply check that first.
        if (isIsoLocale())
            return ((meIsIsoODF = DECISION_YES) == DECISION_YES);
        // If this is not ISO locale for which script must not exist it can
        // still be ISO locale plus ISO script lll-Ssss-CC, but not ll-vvvv ...
        // ll-vvvvvvvv
        meIsIsoODF = ((maBcp47.getLength() <= 11 && LanguageTag::isIsoLanguage( getLanguage()) &&
                    LanguageTag::isIsoCountry( getRegion()) && LanguageTag::isIsoScript( getScript()) &&
                    getVariants().isEmpty()) ? DECISION_YES : DECISION_NO);
    }
    return meIsIsoODF == DECISION_YES;
}


bool LanguageTag::isIsoODF() const
{
    return getImpl()->isIsoODF();
}


bool LanguageTagImpl::isValidBcp47() const
{
    if (meIsValid == DECISION_DONTKNOW)
    {
        const_cast<LanguageTagImpl*>(this)->synCanonicalize();
        SAL_WARN_IF( meIsValid == DECISION_DONTKNOW, "i18nlangtag",
                "LanguageTag::isValidBcp47: canonicalize() didn't set meIsValid");
    }
    return meIsValid == DECISION_YES;
}


bool LanguageTag::isValidBcp47() const
{
    return getImpl()->isValidBcp47();
}


bool LanguageTag::isSystemLocale() const
{
    return mbSystemLocale;
}


LanguageTag & LanguageTag::makeFallback()
{
    if (!mbIsFallback)
    {
        const lang::Locale& rLocale1 = getLocale( true);
        lang::Locale aLocale2( MsLangId::Conversion::lookupFallbackLocale( rLocale1));
        if (    rLocale1.Language != aLocale2.Language ||
                rLocale1.Country  != aLocale2.Country ||
                rLocale1.Variant  != aLocale2.Variant)
        {
            if (rLocale1.Language != "en" && aLocale2.Language == "en" && aLocale2.Country == "US")
            {
                // "en-US" is the last resort fallback, try if we get a better
                // one for the fallback hierarchy of a non-"en" locale.
                ::std::vector< OUString > aFallbacks( getFallbackStrings( false));
                for (::std::vector< OUString >::const_iterator it( aFallbacks.begin()); it != aFallbacks.end(); ++it)
                {
                    lang::Locale aLocale3( LanguageTag( *it).getLocale());
                    aLocale2 = MsLangId::Conversion::lookupFallbackLocale( aLocale3);
                    if (aLocale2.Language != "en" || aLocale2.Country != "US")
                        break;  // for, success
                }
            }
            SAL_INFO( "i18nlangtag", "LanguageTag::makeFallback - for (" <<
                    rLocale1.Language << "," << rLocale1.Country << "," << rLocale1.Variant << ") to (" <<
                    aLocale2.Language << "," << aLocale2.Country << "," << aLocale2.Variant << ")");
            reset( aLocale2);
        }
        mbIsFallback = true;
    }
    return *this;
}


::std::vector< OUString > LanguageTag::getFallbackStrings( bool bIncludeFullBcp47 ) const
{
    ::std::vector< OUString > aVec;
    OUString aLanguage( getLanguage());
    OUString aCountry( getCountry());
    if (isIsoLocale())
    {
        if (!aCountry.isEmpty())
        {
            if (bIncludeFullBcp47)
                aVec.push_back( aLanguage + "-" + aCountry);
            if (aLanguage == "zh")
            {
                // For zh-HK or zh-MO also list zh-TW, for all other zh-XX also
                // list zh-CN.
                if (aCountry == "HK" || aCountry == "MO")
                    aVec.push_back( aLanguage + "-TW");
                else if (aCountry != "CN")
                    aVec.push_back( aLanguage + "-CN");
                aVec.push_back( aLanguage);
            }
            else if (aLanguage == "sh")
            {
                // Manual list instead of calling
                // LanguageTag( "sr-Latn-" + aCountry).getFallbackStrings( true)
                // that would also include "sh-*" again.
                aVec.push_back( "sr-Latn-" + aCountry);
                aVec.push_back( "sr-Latn");
                aVec.push_back( "sh");  // legacy with script, before default script with country
                aVec.push_back( "sr-" + aCountry);
                aVec.push_back( "sr");
            }
            else if (aLanguage == "ca" && aCountry == "XV")
            {
                ::std::vector< OUString > aRep( LanguageTag( "ca-ES-valencia").getFallbackStrings( true));
                aVec.insert( aVec.end(), aRep.begin(), aRep.end());
                // Already includes 'ca' language fallback.
            }
            else
                aVec.push_back( aLanguage);
        }
        else
        {
            if (bIncludeFullBcp47)
                aVec.push_back( aLanguage);
            if (aLanguage == "sh")
            {
                aVec.push_back( "sr-Latn");
                aVec.push_back( "sr");
            }
            else if (aLanguage == "pli")
            {
                // a special case for Pali dictionary, see fdo#41599
                aVec.push_back( "pi-Latn");
                aVec.push_back( "pi");
            }
        }
        return aVec;
    }

    getBcp47();     // have maBcp47 now
    if (bIncludeFullBcp47)
        aVec.push_back( maBcp47);
    OUString aScript;
    OUString aVariants( getVariants());
    OUString aTmp;
    if (hasScript())
    {
        aScript = getScript();
        bool bHaveLanguageScriptVariant = false;
        if (!aCountry.isEmpty())
        {
            if (!aVariants.isEmpty())
            {
                aTmp = aLanguage + "-" + aScript + "-" + aCountry + "-" + aVariants;
                if (aTmp != maBcp47)
                    aVec.push_back( aTmp);
                // Language with variant but without country before language
                // without variant but with country.
                aTmp = aLanguage + "-" + aScript + "-" + aVariants;
                if (aTmp != maBcp47)
                    aVec.push_back( aTmp);
                bHaveLanguageScriptVariant = true;
            }
            aTmp = aLanguage + "-" + aScript + "-" + aCountry;
            if (aTmp != maBcp47)
                aVec.push_back( aTmp);
            if (aLanguage == "sr" && aScript == "Latn")
            {
                // sr-Latn-CS => sr-Latn-YU, sh-CS, sh-YU
                if (aCountry == "CS")
                {
                    aVec.push_back( "sr-Latn-YU");
                    aVec.push_back( "sh-CS");
                    aVec.push_back( "sh-YU");
                }
                else
                    aVec.push_back( "sh-" + aCountry);
            }
            else if (aLanguage == "pi" && aScript == "Latn")
                aVec.push_back( "pli");     // a special case for Pali dictionary, see fdo#41599
        }
        if (!aVariants.isEmpty() && !bHaveLanguageScriptVariant)
        {
            aTmp = aLanguage + "-" + aScript + "-" + aVariants;
            if (aTmp != maBcp47)
                aVec.push_back( aTmp);
        }
        aTmp = aLanguage + "-" + aScript;
        if (aTmp != maBcp47)
            aVec.push_back( aTmp);
        // 'sh' actually denoted a script, so have it here instead of appended
        // at the end as language-only.
        if (aLanguage == "sr" && aScript == "Latn")
            aVec.push_back( "sh");
    }
    bool bHaveLanguageVariant = false;
    if (!aCountry.isEmpty())
    {
        if (!aVariants.isEmpty())
        {
            aTmp = aLanguage + "-" + aCountry + "-" + aVariants;
            if (aTmp != maBcp47)
                aVec.push_back( aTmp);
            // Language with variant but without country before language
            // without variant but with country.
            aTmp = aLanguage + "-" + aVariants;
            if (aTmp != maBcp47)
                aVec.push_back( aTmp);
            bHaveLanguageVariant = true;
        }
        aTmp = aLanguage + "-" + aCountry;
        if (aTmp != maBcp47)
            aVec.push_back( aTmp);
    }
    if (!aVariants.isEmpty() && !bHaveLanguageVariant)
    {
        aTmp = aLanguage + "-" + aVariants;
        if (aTmp != maBcp47)
            aVec.push_back( aTmp);
    }

    // Insert legacy fallbacks with country before language-only, but only
    // default script, script was handled already above.
    if (!aCountry.isEmpty())
    {
        if (aLanguage == "sr" && aCountry == "CS")
            aVec.push_back( "sr-YU");
    }

    // Original language-only.
    if (aLanguage != maBcp47)
        aVec.push_back( aLanguage);

    return aVec;
}


bool LanguageTag::equals( const LanguageTag & rLanguageTag, bool bResolveSystem ) const
{
    // If SYSTEM is not to be resolved or either both are SYSTEM or none, we
    // can use the operator==() optimization.
    if (!bResolveSystem || isSystemLocale() == rLanguageTag.isSystemLocale())
        return operator==( rLanguageTag);

    // Compare full language tag strings.
    return getBcp47( bResolveSystem) == rLanguageTag.getBcp47( bResolveSystem);
}


bool LanguageTag::operator==( const LanguageTag & rLanguageTag ) const
{
    if (isSystemLocale() && rLanguageTag.isSystemLocale())
        return true;    // both SYSTEM

    // No need to convert to BCP47 if both Lang-IDs are available.
    if (mbInitializedLangID && rLanguageTag.mbInitializedLangID)
    {
        // Equal if same ID and no SYSTEM is involved or both are SYSTEM.
        return mnLangID == rLanguageTag.mnLangID && isSystemLocale() == rLanguageTag.isSystemLocale();
    }

    // Compare full language tag strings but SYSTEM unresolved.
    return getBcp47( false) == rLanguageTag.getBcp47( false);
}


bool LanguageTag::operator!=( const LanguageTag & rLanguageTag ) const
{
    return !operator==( rLanguageTag);
}


// static
LanguageTagImpl::Extraction LanguageTagImpl::simpleExtract( const OUString& rBcp47,
        OUString& rLanguage, OUString& rScript, OUString& rCountry, OUString& rVariants )
{
    Extraction eRet = EXTRACTED_NONE;
    const sal_Int32 nLen = rBcp47.getLength();
    const sal_Int32 nHyph1 = rBcp47.indexOf( '-');
    sal_Int32 nHyph2 = (nHyph1 < 0 ? -1 : rBcp47.indexOf( '-', nHyph1 + 1));
    sal_Int32 nHyph3 = (nHyph2 < 0 ? -1 : rBcp47.indexOf( '-', nHyph2 + 1));
    sal_Int32 nHyph4 = (nHyph3 < 0 ? -1 : rBcp47.indexOf( '-', nHyph3 + 1));
    if (nLen == 1 && rBcp47[0] == '*')              // * the dreaded jolly joker
    {
        // It's f*d up but we need to recognize this.
        eRet = EXTRACTED_X_JOKER;
    }
    else if (nHyph1 == 1 && rBcp47[0] == 'x')       // x-... privateuse
    {
        // x-... privateuse tags MUST be known to us by definition.
        eRet = EXTRACTED_X;
    }
    else if (nLen == 2 || nLen == 3)                // ll or lll
    {
        if (nHyph1 < 0)
        {
            rLanguage = rBcp47.toAsciiLowerCase();
            rScript = rCountry = rVariants = OUString();
            eRet = EXTRACTED_LSC;
        }
    }
    else if (  (nHyph1 == 2 && nLen == 5)           // ll-CC
            || (nHyph1 == 3 && nLen == 6))          // lll-CC
    {
        if (nHyph2 < 0)
        {
            rLanguage = rBcp47.copy( 0, nHyph1).toAsciiLowerCase();
            rCountry  = rBcp47.copy( nHyph1 + 1, 2).toAsciiUpperCase();
            rScript = rVariants = OUString();
            eRet = EXTRACTED_LSC;
        }
    }
    else if (  (nHyph1 == 2 && nLen ==  7)          // ll-Ssss
            || (nHyph1 == 3 && nLen ==  8))         // lll-Ssss
    {
        /* TODO: also accept a (DIGIT 3*ALNUM) vvvv variant instead of Ssss */
        if (nHyph2 < 0)
        {
            rLanguage = rBcp47.copy( 0, nHyph1).toAsciiLowerCase();
            rScript   = rBcp47.copy( nHyph1 + 1, 1).toAsciiUpperCase() + rBcp47.copy( nHyph1 + 2, 3).toAsciiLowerCase();
            rCountry  = rVariants = OUString();
            eRet = EXTRACTED_LSC;
        }
    }
    else if (  (nHyph1 == 2 && nHyph2 == 7 && nLen == 10)   // ll-Ssss-CC
            || (nHyph1 == 3 && nHyph2 == 8 && nLen == 11))  // lll-Ssss-CC
    {
        if (nHyph3 < 0)
        {
            rLanguage = rBcp47.copy( 0, nHyph1).toAsciiLowerCase();
            rScript   = rBcp47.copy( nHyph1 + 1, 1).toAsciiUpperCase() + rBcp47.copy( nHyph1 + 2, 3).toAsciiLowerCase();
            rCountry  = rBcp47.copy( nHyph2 + 1, 2).toAsciiUpperCase();
            rVariants = OUString();
            eRet = EXTRACTED_LSC;
        }
    }
    else if (  (nHyph1 == 2 && nHyph2 == 7 && nHyph3 == 10 && nLen >= 15)   // ll-Ssss-CC-vvvv[vvvv][-...]
            || (nHyph1 == 3 && nHyph2 == 8 && nHyph3 == 11 && nLen >= 16))  // lll-Ssss-CC-vvvv[vvvv][-...]
    {
        if (nHyph4 < 0)
            nHyph4 = rBcp47.getLength();
        if (nHyph4 - nHyph3 > 4 && nHyph4 - nHyph3 <= 9)
        {
            rLanguage = rBcp47.copy( 0, nHyph1).toAsciiLowerCase();
            rScript   = rBcp47.copy( nHyph1 + 1, 1).toAsciiUpperCase() + rBcp47.copy( nHyph1 + 2, 3).toAsciiLowerCase();
            rCountry  = rBcp47.copy( nHyph2 + 1, 2).toAsciiUpperCase();
            rVariants = rBcp47.copy( nHyph3 + 1);
            eRet = EXTRACTED_LV;
        }
    }
    else if (  (nHyph1 == 2 && nHyph2 == 5 && nLen >= 10)   // ll-CC-vvvv[vvvv][-...]
            || (nHyph1 == 3 && nHyph2 == 6 && nLen >= 11))  // lll-CC-vvvv[vvvv][-...]
    {
        if (nHyph3 < 0)
            nHyph3 = rBcp47.getLength();
        if (nHyph3 - nHyph2 > 4 && nHyph3 - nHyph2 <= 9)
        {
            rLanguage = rBcp47.copy( 0, nHyph1).toAsciiLowerCase();
            rScript   = OUString();
            rCountry  = rBcp47.copy( nHyph1 + 1, 2).toAsciiUpperCase();
            rVariants = rBcp47.copy( nHyph2 + 1);
            eRet = EXTRACTED_LV;
        }
    }
    else if (  (nHyph1 == 2 && nLen >= 8)                   // ll-vvvvv[vvv][-...]
            || (nHyph1 == 3 && nLen >= 9))                  // lll-vvvvv[vvv][-...]
    {
        if (nHyph2 < 0)
            nHyph2 = rBcp47.getLength();
        if (nHyph2 - nHyph1 > 5 && nHyph2 - nHyph1 <= 9)
        {
            rLanguage = rBcp47.copy( 0, nHyph1).toAsciiLowerCase();
            rScript   = rCountry = OUString();
            rVariants = rBcp47.copy( nHyph1 + 1);
            eRet = EXTRACTED_LV;
        }
        else
        {
            // Known and handled grandfathered; ugly but effective ...
            // Note that nLen must have matched above.
            // Strictly not a variant, but so far we treat it as such.
            if (rBcp47.equalsIgnoreAsciiCase( "en-GB-oed"))
            {
                rLanguage = "en";
                rScript   = OUString();
                rCountry  = "GB";
                rVariants = "oed";
                eRet = EXTRACTED_LV;
            }
        }
    }
    if (eRet == EXTRACTED_NONE)
        rLanguage = rScript = rCountry = rVariants = OUString();
    return eRet;
}


// static
::std::vector< OUString >::const_iterator LanguageTag::getFallback(
        const ::std::vector< OUString > & rList, const OUString & rReference )
{
    if (rList.empty())
        return rList.end();

    ::std::vector< OUString >::const_iterator it;

    // Try the simple case first without constructing fallbacks.
    for (it = rList.begin(); it != rList.end(); ++it)
    {
        if (*it == rReference)
            return it;  // exact match
    }

    ::std::vector< OUString > aFallbacks( LanguageTag( rReference).getFallbackStrings( false));
    if (rReference != "en-US")
    {
        aFallbacks.push_back( "en-US");
        if (rReference != "en")
            aFallbacks.push_back( "en");
    }
    if (rReference != "x-default")
        aFallbacks.push_back( "x-default");
    if (rReference != "x-no-translate")
        aFallbacks.push_back( "x-no-translate");
    /* TODO: the original comphelper::Locale::getFallback() code had
     * "x-notranslate" instead of "x-no-translate", but all .xcu files use
     * "x-no-translate" and "x-notranslate" apparently was never used anywhere.
     * Did that ever work? Was it supposed to work at all like this? */

    for (::std::vector< OUString >::const_iterator fb = aFallbacks.begin(); fb != aFallbacks.end(); ++fb)
    {
        for (it = rList.begin(); it != rList.end(); ++it)
        {
            if (*it == *fb)
                return it;  // fallback found
        }
    }

    // Did not find anything so return something of the list, the first value
    // will do as well as any other as none did match any of the possible
    // fallbacks.
    return rList.begin();
}


// static
::std::vector< com::sun::star::lang::Locale >::const_iterator LanguageTag::getMatchingFallback(
        const ::std::vector< com::sun::star::lang::Locale > & rList,
        const com::sun::star::lang::Locale & rReference )
{
    if (rList.empty())
        return rList.end();

    ::std::vector< lang::Locale >::const_iterator it;

    // Try the simple case first without constructing fallbacks.
    for (it = rList.begin(); it != rList.end(); ++it)
    {
        if (    (*it).Language == rReference.Language &&
                (*it).Country  == rReference.Country  &&
                (*it).Variant  == rReference.Variant)
            return it;  // exact match
    }

    // Now for each reference fallback test the fallbacks of the list in order.
    ::std::vector< OUString > aFallbacks( LanguageTag( rReference).getFallbackStrings( false));
    ::std::vector< ::std::vector< OUString > > aListFallbacks( rList.size());
    size_t i = 0;
    for (it = rList.begin(); it != rList.end(); ++it, ++i)
    {
        ::std::vector< OUString > aTmp( LanguageTag( *it).getFallbackStrings( true));
        aListFallbacks[i] = aTmp;
    }
    for (::std::vector< OUString >::const_iterator rfb( aFallbacks.begin()); rfb != aFallbacks.end(); ++rfb)
    {
        for (::std::vector< ::std::vector< OUString > >::const_iterator lfb( aListFallbacks.begin());
                lfb != aListFallbacks.end(); ++lfb)
        {
            for (::std::vector< OUString >::const_iterator fb( (*lfb).begin()); fb != (*lfb).end(); ++fb)
            {
                if (*rfb == *fb)
                    return rList.begin() + (lfb - aListFallbacks.begin());
            }
        }
    }

    // No match found.
    return rList.end();
}


static bool lcl_isSystem( LanguageType nLangID )
{
    if (nLangID == LANGUAGE_SYSTEM)
        return true;
    // There are some special values that simplify to SYSTEM,
    // getRealLanguage() catches and resolves them.
    LanguageType nNewLangID = MsLangId::getRealLanguage( nLangID);
    if (nNewLangID != nLangID)
        return true;
    return false;
}


// static
com::sun::star::lang::Locale LanguageTag::convertToLocale( LanguageType nLangID, bool bResolveSystem )
{
    if (!bResolveSystem && lcl_isSystem( nLangID))
        return lang::Locale();

    return MsLangId::Conversion::convertLanguageToLocale( nLangID, bResolveSystem);
}


// static
LanguageType LanguageTag::convertToLanguageType( const com::sun::star::lang::Locale& rLocale, bool bResolveSystem )
{
    if (rLocale.Language.isEmpty() && !bResolveSystem)
        return LANGUAGE_SYSTEM;

    return MsLangId::Conversion::convertLocaleToLanguage( rLocale);
}


// static
OUString LanguageTag::convertToBcp47( const com::sun::star::lang::Locale& rLocale, bool bResolveSystem )
{
    OUString aBcp47;
    if (rLocale.Language.isEmpty())
    {
        if (bResolveSystem)
            aBcp47 = convertToBcp47( LANGUAGE_SYSTEM, true);
        // else aBcp47 stays empty
    }
    else if (rLocale.Language == I18NLANGTAG_QLT)
    {
        aBcp47 = rLocale.Variant;
    }
    else
    {
        /* XXX NOTE: most legacy code never evaluated the Variant field, so for
         * now just concatenate language and country. In case we stumbled over
         * variant aware code we'd have to take care of that. */
        if (rLocale.Country.isEmpty())
            aBcp47 = rLocale.Language;
        else
        {
            aBcp47 = rLocale.Language + "-" + rLocale.Country;
        }
    }
    return aBcp47;
}


// static
OUString LanguageTag::convertToBcp47( LanguageType nLangID, bool bResolveSystem )
{
    // Catch this first so we don't need the rest.
    if (!bResolveSystem && lcl_isSystem( nLangID))
        return OUString();

    lang::Locale aLocale( convertToLocale( nLangID, bResolveSystem));
    // If system for some reason (should not happen.. haha) could not be
    // resolved DO NOT CALL convertToBcp47() because that would recurse into
    // this method here!
    if (aLocale.Language.isEmpty() && bResolveSystem)
        return OUString();      // bad luck, bail out
    return convertToBcp47( aLocale, bResolveSystem);
}


// static
com::sun::star::lang::Locale LanguageTag::convertToLocale( const OUString& rBcp47, bool bResolveSystem )
{
    return LanguageTag( rBcp47).getLocale( bResolveSystem);
}


// static
LanguageType LanguageTag::convertToLanguageType( const OUString& rBcp47, bool bResolveSystem )
{
    return LanguageTag( rBcp47).getLanguageType( bResolveSystem);
}


// static
LanguageType LanguageTag::convertToLanguageTypeWithFallback( const OUString& rBcp47 )
{
    return LanguageTag( rBcp47).makeFallback().getLanguageType( true);
}


// static
com::sun::star::lang::Locale LanguageTag::convertToLocaleWithFallback( const OUString& rBcp47 )
{
    return LanguageTag( rBcp47).makeFallback().getLocale( true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
