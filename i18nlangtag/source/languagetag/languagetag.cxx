/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_folders.h>
#include <config_liblangtag.h>

#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/applelangid.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include <rtl/locale.h>
#include <map>
#include <unordered_set>

//#define erDEBUG

#if LIBLANGTAG_INLINE_FIX
#define LT_HAVE_INLINE
#endif
#include <liblangtag/langtag.h>

using namespace com::sun::star;


// Helper to ensure lt_error_t is free'd
struct myLtError
{
    lt_error_t* p;
    myLtError() : p(nullptr) {}
    ~myLtError() { if (p) lt_error_unref( p); }
};

// "statics" to be returned as const reference to an empty locale and string.
namespace {
struct theEmptyLocale : public rtl::Static< lang::Locale, theEmptyLocale > {};
struct theEmptyBcp47 : public rtl::Static< OUString, theEmptyBcp47 > {};
}

typedef std::unordered_set< OUString > KnownTagSet;
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


namespace {
struct compareIgnoreAsciiCaseLess
{
    bool operator()( const OUString& r1, const OUString& r2 ) const
    {
        return r1.compareToIgnoreAsciiCase( r2) < 0;
    }
};
typedef ::std::map< OUString, LanguageTag::ImplPtr, compareIgnoreAsciiCaseLess > MapBcp47;
typedef ::std::map< LanguageType, LanguageTag::ImplPtr > MapLangID;
struct theMapBcp47 : public rtl::Static< MapBcp47, theMapBcp47 > {};
struct theMapLangID : public rtl::Static< MapLangID, theMapLangID > {};
struct theDontKnow : public rtl::Static< LanguageTag::ImplPtr, theDontKnow > {};
struct theSystemLocale : public rtl::Static< LanguageTag::ImplPtr, theSystemLocale > {};
}


static LanguageType getNextOnTheFlyLanguage()
{
    static LanguageType nOnTheFlyLanguage(0);
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
            {
                SAL_WARN( "i18nlangtag", "getNextOnTheFlyLanguage: none left! ("
                        << ((sal_uInt16(LANGUAGE_ON_THE_FLY_END) - sal_uInt16(LANGUAGE_ON_THE_FLY_START) + 1)
                            * (sal_uInt16(LANGUAGE_ON_THE_FLY_SUB_END) - sal_uInt16(LANGUAGE_ON_THE_FLY_SUB_START) + 1))
                        << " consumed?!?)");
                return LanguageType(0);
            }
        }
    }
#if OSL_DEBUG_LEVEL > 0
    static size_t nOnTheFlies = 0;
    ++nOnTheFlies;
    SAL_INFO( "i18nlangtag", "getNextOnTheFlyLanguage: number " << nOnTheFlies);
#endif
    return nOnTheFlyLanguage;
}


// static
bool LanguageTag::isOnTheFlyID( LanguageType nLang )
{
    LanguageType nPri = MsLangId::getPrimaryLanguage( nLang);
    LanguageType nSub = MsLangId::getSubLanguage( nLang);
    return
        LANGUAGE_ON_THE_FLY_START <= nPri && nPri <= LANGUAGE_ON_THE_FLY_END &&
        LANGUAGE_ON_THE_FLY_SUB_START <= nSub && nSub <= LANGUAGE_ON_THE_FLY_SUB_END;
}


/** A reference holder for liblangtag data de/initialization, one static
    instance. Currently implemented such that the first "ref" inits and dtor
    (our library deinitialized) tears down.
*/
class LiblangtagDataRef
{
public:
    LiblangtagDataRef();
    ~LiblangtagDataRef();
    void init()
    {
        if (!mbInitialized)
            setup();
    }
private:
    OString maDataPath;   // path to liblangtag data, "|" if system
    bool mbInitialized;

    void setupDataPath();
    void setup();
    static void teardown();
};

namespace {
struct theDataRef : public rtl::Static< LiblangtagDataRef, theDataRef > {};
}

LiblangtagDataRef::LiblangtagDataRef()
    :
        mbInitialized(false)
{
}

LiblangtagDataRef::~LiblangtagDataRef()
{
    if (mbInitialized)
        teardown();
}

void LiblangtagDataRef::setup()
{
    SAL_INFO( "i18nlangtag", "LiblangtagDataRef::setup: initializing database");
    if (maDataPath.isEmpty())
        setupDataPath();
    lt_db_initialize();
    mbInitialized = true;
}

void LiblangtagDataRef::teardown()
{
    SAL_INFO( "i18nlangtag", "LiblangtagDataRef::teardown: finalizing database");
    lt_db_finalize();
}

void LiblangtagDataRef::setupDataPath()
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


/* TODO: we could transform known vendor and browser-specific variants to known
 * BCP 47 if available. For now just remove them to not confuse any later
 * treatments that check for empty variants. This vendor stuff was never
 * supported anyway. */
static void handleVendorVariant( css::lang::Locale & rLocale )
{
    if (!rLocale.Variant.isEmpty() && rLocale.Language != I18NLANGTAG_QLT)
        rLocale.Variant.clear();
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

    mutable css::lang::Locale               maLocale;
    mutable OUString                        maBcp47;
    mutable OUString                        maCachedLanguage;   ///< cache getLanguage()
    mutable OUString                        maCachedScript;     ///< cache getScript()
    mutable OUString                        maCachedCountry;    ///< cache getCountry()
    mutable OUString                        maCachedVariants;   ///< cache getVariants()
    mutable lt_tag_t*                       mpImplLangtag;      ///< liblangtag pointer
    mutable LanguageType                    mnLangID;
    mutable LanguageTag::ScriptType         meScriptType;
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

    OUString const &    getBcp47() const;
    OUString const &    getLanguage() const;
    OUString const &    getScript() const;
    OUString const &    getCountry() const;
    OUString            getRegion() const;
    OUString const &    getVariants() const;
    bool                hasScript() const;
    OUString            getGlibcLocaleString() const;

    void                setScriptType(LanguageTag::ScriptType st);
    LanguageTag::ScriptType getScriptType() const;

    bool                isIsoLocale() const;
    bool                isIsoODF() const;
    bool                isValidBcp47() const;

    void                convertLocaleToBcp47();
    bool                convertLocaleToLang( bool bAllowOnTheFlyID );
    void                convertBcp47ToLocale();
    void                convertBcp47ToLang();
    void                convertLangToLocale();
    void                convertLangToBcp47();

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

    /** Generates on-the-fly LangID and registers the maBcp47,mnLangID pair.

        @param  nRegisterID
                If not 0 and not LANGUAGE_DONTKNOW, suggest (!) to use that ID
                instead of generating an on-the-fly ID. Implementation may
                still generate an ID if the suggested ID is already used for
                another language tag.

        @return NULL if no ID could be obtained or registration failed.
     */
    LanguageTag::ImplPtr registerOnTheFly( LanguageType nRegisterID );

    /** Obtain Language, Script, Country and Variants via simpleExtract() and
        assign them to the cached variables if successful.

        @return simpleExtract() successfully extracted and cached.
     */
    bool                cacheSimpleLSCV();

    enum Extraction
    {
        EXTRACTED_NONE,
        EXTRACTED_LSC,
        EXTRACTED_LV,
        EXTRACTED_C_LOCALE,
        EXTRACTED_X,
        EXTRACTED_X_JOKER,
        EXTRACTED_KNOWN_BAD
    };

    /** Of a language tag of the form lll[-Ssss][-CC][-vvvvvvvv] extract the
        portions.

        Does not check case or content!

        @return EXTRACTED_LSC if simple tag was detected (i.e. one that
                would fulfill the isIsoODF() condition),
                EXTRACTED_LV if a tag with variant was detected,
                EXTRACTED_C_LOCALE if a 'C' locale was detected,
                EXTRACTED_X if x-... privateuse tag was detected,
                EXTRACTED_X_JOKER if "*" joker was detected,
                EXTRACTED_KNOWN_BAD if a bad but known (to be remapped) tag was detected
                EXTRACTED_NONE else.
     */
    static Extraction   simpleExtract( const OUString& rBcp47,
                                       OUString& rLanguage,
                                       OUString& rScript,
                                       OUString& rCountry,
                                       OUString& rVariants );

    /** Convert Locale to BCP 47 string without resolving system and creating
        temporary LanguageTag instances. */
    static OUString     convertToBcp47( const css::lang::Locale& rLocale );

};


LanguageTagImpl::LanguageTagImpl( const LanguageTag & rLanguageTag )
    :
        maLocale( rLanguageTag.maLocale),
        maBcp47( rLanguageTag.maBcp47),
        mpImplLangtag( nullptr),
        mnLangID( rLanguageTag.mnLangID),
        meScriptType( LanguageTag::ScriptType::UNKNOWN),
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
                lt_tag_copy( rLanguageTagImpl.mpImplLangtag) : nullptr),
        mnLangID( rLanguageTagImpl.mnLangID),
        meScriptType( rLanguageTagImpl.meScriptType),
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
        theDataRef::get().init();
}


LanguageTagImpl& LanguageTagImpl::operator=( const LanguageTagImpl & rLanguageTagImpl )
{
    if (&rLanguageTagImpl == this)
        return *this;

    maLocale            = rLanguageTagImpl.maLocale;
    maBcp47             = rLanguageTagImpl.maBcp47;
    maCachedLanguage    = rLanguageTagImpl.maCachedLanguage;
    maCachedScript      = rLanguageTagImpl.maCachedScript;
    maCachedCountry     = rLanguageTagImpl.maCachedCountry;
    maCachedVariants    = rLanguageTagImpl.maCachedVariants;
    lt_tag_t * oldTag = mpImplLangtag;
    mpImplLangtag       = rLanguageTagImpl.mpImplLangtag ?
                            lt_tag_copy( rLanguageTagImpl.mpImplLangtag) : nullptr;
    lt_tag_unref(oldTag);
    mnLangID            = rLanguageTagImpl.mnLangID;
    meScriptType        = rLanguageTagImpl.meScriptType;
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
    if (mpImplLangtag && !oldTag)
        theDataRef::get().init();
    return *this;
}


LanguageTagImpl::~LanguageTagImpl()
{
    if (mpImplLangtag)
    {
        lt_tag_unref( mpImplLangtag);
    }
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
        getImpl()->canonicalize();
        // Registration itself may already have canonicalized, so do an
        // unconditional sync.
        syncFromImpl();
    }

}


LanguageTag::LanguageTag( const css::lang::Locale & rLocale )
    :
        maLocale( rLocale),
        mnLangID( LANGUAGE_DONTKNOW),
        mbSystemLocale( rLocale.Language.isEmpty()),
        mbInitializedBcp47( false),
        mbInitializedLocale( false),    // we do not know which mess we got passed in
        mbInitializedLangID( false),
        mbIsFallback( false)
{
    handleVendorVariant( maLocale);
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


LanguageTag::~LanguageTag()
{
}


LanguageTag::ImplPtr LanguageTagImpl::registerOnTheFly( LanguageType nRegisterID )
{
    LanguageTag::ImplPtr pImpl;

    if (!mbInitializedBcp47)
    {
        if (mbInitializedLocale)
        {
            maBcp47 = LanguageTagImpl::convertToBcp47( maLocale);
            mbInitializedBcp47 = !maBcp47.isEmpty();
        }
    }
    if (maBcp47.isEmpty())
    {
        SAL_WARN( "i18nlangtag", "LanguageTagImpl::registerOnTheFly: no Bcp47 string, no registering");
        return pImpl;
    }

    osl::MutexGuard aGuard( theMutex::get());

    MapBcp47& rMapBcp47 = theMapBcp47::get();
    MapBcp47::const_iterator it( rMapBcp47.find( maBcp47));
    bool bOtherImpl = false;
    if (it != rMapBcp47.end())
    {
        SAL_INFO( "i18nlangtag", "LanguageTag::registerOnTheFly: found impl for '" << maBcp47 << "'");
        pImpl = (*it).second;
        if (pImpl.get() != this)
        {
            // Could happen for example if during registerImpl() the tag was
            // changed via canonicalize() and the result was already present in
            // the map before, for example 'bn-Beng' => 'bn'. This specific
            // case is now taken care of in registerImpl() and doesn't reach
            // here. However, use the already existing impl if it matches.
            SAL_WARN( "i18nlangtag", "LanguageTag::registerOnTheFly: using other impl for this '" << maBcp47 << "'");
            *this = *pImpl;     // ensure consistency
            bOtherImpl = true;
        }
    }
    else
    {
        SAL_INFO( "i18nlangtag", "LanguageTag::registerOnTheFly: new impl for '" << maBcp47 << "'");
        pImpl.reset( new LanguageTagImpl( *this));
        rMapBcp47.insert( ::std::make_pair( maBcp47, pImpl));
    }

    if (!bOtherImpl || !pImpl->mbInitializedLangID)
    {
        if (nRegisterID == LanguageType(0) || nRegisterID == LANGUAGE_DONTKNOW)
            nRegisterID = getNextOnTheFlyLanguage();
        else
        {
            // Accept a suggested ID only if it is not mapped yet to something
            // different, otherwise we would end up with ambiguous assignments
            // of different language tags, for example for the same primary
            // LangID with "no", "nb" and "nn".
            const MapLangID& rMapLangID = theMapLangID::get();
            MapLangID::const_iterator itID( rMapLangID.find( nRegisterID));
            if (itID != rMapLangID.end())
            {
                if ((*itID).second->maBcp47 != maBcp47)
                {
                    SAL_INFO( "i18nlangtag", "LanguageTag::registerOnTheFly: not using suggested 0x"
                            << ::std::hex << nRegisterID << " for '" << maBcp47 << "' have '"
                            << (*itID).second->maBcp47 << "'");
                    nRegisterID = getNextOnTheFlyLanguage();
                }
                else
                {
                    SAL_WARN( "i18nlangtag", "LanguageTag::registerOnTheFly: suggested 0x"
                            << ::std::hex << nRegisterID << " for '" << maBcp47 << "' already registered");
                }
            }
        }
        if (!nRegisterID)
        {
            // out of IDs, nothing to register
            return pImpl;
        }
        pImpl->mnLangID = nRegisterID;
        pImpl->mbInitializedLangID = true;
        if (pImpl.get() != this)
        {
            mnLangID = nRegisterID;
            mbInitializedLangID = true;
        }
    }

    ::std::pair< MapLangID::const_iterator, bool > res(
            theMapLangID::get().insert( ::std::make_pair( pImpl->mnLangID, pImpl)));
    if (res.second)
    {
        SAL_INFO( "i18nlangtag", "LanguageTag::registerOnTheFly: cross-inserted 0x"
                << ::std::hex << pImpl->mnLangID << " for '" << maBcp47 << "'");
    }
    else
    {
        SAL_WARN( "i18nlangtag", "LanguageTag::registerOnTheFly: not cross-inserted 0x"
                << ::std::hex << pImpl->mnLangID << " for '" << maBcp47 << "' have '"
                << (*res.first).second->maBcp47 << "'");
    }

    return pImpl;
}


LanguageTag::ScriptType LanguageTag::getOnTheFlyScriptType( LanguageType nRegisterID )
{
    const MapLangID& rMapLangID = theMapLangID::get();
    MapLangID::const_iterator itID( rMapLangID.find( nRegisterID));
    if (itID != rMapLangID.end())
        return (*itID).second->getScriptType();
    else
        return UNKNOWN;
}


// static
void LanguageTag::setConfiguredSystemLanguage( LanguageType nLang )
{
    if (nLang == LANGUAGE_DONTKNOW || nLang == LANGUAGE_SYSTEM)
    {
        SAL_WARN( "i18nlangtag",
                "LanguageTag::setConfiguredSystemLanguage: refusing to set unresolved system locale 0x" <<
                ::std::hex << nLang);
        return;
    }
    SAL_INFO( "i18nlangtag", "LanguageTag::setConfiguredSystemLanguage: setting to 0x" << ::std::hex << nLang);
    MsLangId::LanguageTagAccess::setConfiguredSystemLanguage( nLang);
    // Reset system locale to none and let registerImpl() do the rest to
    // initialize a new one.
    theSystemLocale::get().reset();
    LanguageTag aLanguageTag( LANGUAGE_SYSTEM);
    aLanguageTag.registerImpl();
}

static bool lcl_isKnownOnTheFlyID( LanguageType nLang )
{
    return nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM &&
        (LanguageTag::isOnTheFlyID( nLang) || (nLang == MsLangId::getPrimaryLanguage( nLang)));
}


LanguageTag::ImplPtr LanguageTag::registerImpl() const
{
    // XXX NOTE: Do not use non-static LanguageTag::convert...() member methods
    // here as they access getImpl() and syncFromImpl() and would lead to
    // recursion. Also do not use the static LanguageTag::convertTo...()
    // methods as they may create temporary LanguageTag instances. Only
    // LanguageTagImpl::convertToBcp47(Locale) is ok.

    ImplPtr pImpl;

#if OSL_DEBUG_LEVEL > 0
    static size_t nCalls = 0;
    ++nCalls;
    SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: " << nCalls << " calls");
#endif

    // Do not register unresolved system locale, also force LangID if system
    // and take the system locale shortcut if possible.
    if (mbSystemLocale)
    {
        pImpl = theSystemLocale::get();
        if (pImpl)
        {
#if OSL_DEBUG_LEVEL > 0
            static size_t nCallsSystem = 0;
            ++nCallsSystem;
            SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: " << nCallsSystem << " system calls");
#endif
            return pImpl;
        }
        if (!mbInitializedLangID)
        {
            mnLangID = MsLangId::getRealLanguage( LANGUAGE_SYSTEM);
            mbInitializedLangID = (mnLangID != LANGUAGE_SYSTEM);
            SAL_WARN_IF( !mbInitializedLangID, "i18nlangtag", "LanguageTag::registerImpl: can't resolve system!");
        }
    }

    if (mbInitializedLangID)
    {
        if (mnLangID == LANGUAGE_DONTKNOW)
        {
            // Heavy usage of LANGUAGE_DONTKNOW, make it an own Impl for all the
            // conversion attempts. At the same time provide a central breakpoint
            // to inspect such places.
            LanguageTag::ImplPtr& rDontKnow = theDontKnow::get();
            if (!rDontKnow)
                rDontKnow.reset( new LanguageTagImpl( *this));
            pImpl = rDontKnow;
#if OSL_DEBUG_LEVEL > 0
            static size_t nCallsDontKnow = 0;
            ++nCallsDontKnow;
            SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: " << nCallsDontKnow << " DontKnow calls");
#endif
            return pImpl;
        }
        else
        {
            // A great share are calls for a system equal locale.
            pImpl = theSystemLocale::get();
            if (pImpl && pImpl->mnLangID == mnLangID)
            {
#if OSL_DEBUG_LEVEL > 0
                static size_t nCallsSystemEqual = 0;
                ++nCallsSystemEqual;
                SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: " << nCallsSystemEqual
                        << " system equal LangID calls");
#endif
                return pImpl;
            }
        }
    }

    // Force Bcp47 if not LangID.
    if (!mbInitializedLangID && !mbInitializedBcp47)
    {
        // The one central point to set mbInitializedLocale=true if a
        // LanguageTag was initialized with a Locale. We will now convert and
        // possibly later resolve it.
        if (!mbInitializedLocale && (mbSystemLocale || !maLocale.Language.isEmpty()))
            mbInitializedLocale = true;
        SAL_WARN_IF( !mbInitializedLocale, "i18nlangtag", "LanguageTag::registerImpl: still not mbInitializedLocale");

        maBcp47 = LanguageTagImpl::convertToBcp47( maLocale);
        mbInitializedBcp47 = !maBcp47.isEmpty();
    }

    if (mbInitializedBcp47)
    {
        // A great share are calls for a system equal locale.
        pImpl = theSystemLocale::get();
        if (pImpl && pImpl->maBcp47 == maBcp47)
        {
#if OSL_DEBUG_LEVEL > 0
            static size_t nCallsSystemEqual = 0;
            ++nCallsSystemEqual;
            SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: " << nCallsSystemEqual << " system equal BCP47 calls");
#endif
            return pImpl;
        }
    }

#if OSL_DEBUG_LEVEL > 0
    static size_t nCallsNonSystem = 0;
    ++nCallsNonSystem;
    SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: " << nCallsNonSystem << " non-system calls");
#endif

    osl::MutexGuard aGuard( theMutex::get());

#if OSL_DEBUG_LEVEL > 0
    static long nRunning = 0;
    // Entering twice here is ok, which is needed for fallback init in
    // getKnowns() in canonicalize() via pImpl->convertBcp47ToLocale() below,
    // everything else is suspicious.
    SAL_WARN_IF( nRunning > 1, "i18nlangtag", "LanguageTag::registerImpl: re-entered for '"
            << maBcp47 << "' 0x" << ::std::hex << mnLangID );
    struct Runner { Runner() { ++nRunning; } ~Runner() { --nRunning; } } aRunner;
#endif

    // Prefer LangID map as find+insert needs less comparison work.
    if (mbInitializedLangID)
    {
        MapLangID& rMap = theMapLangID::get();
        MapLangID::const_iterator it( rMap.find( mnLangID));
        if (it != rMap.end())
        {
            SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: found impl for 0x" << ::std::hex << mnLangID);
            pImpl = (*it).second;
        }
        else
        {
            SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: new impl for 0x" << ::std::hex << mnLangID);
            pImpl.reset( new LanguageTagImpl( *this));
            rMap.insert( ::std::make_pair( mnLangID, pImpl));
            // Try round-trip.
            if (!pImpl->mbInitializedLocale)
                pImpl->convertLangToLocale();
            LanguageType nLang = MsLangId::Conversion::convertLocaleToLanguage( pImpl->maLocale);
            // If round-trip is identical cross-insert to Bcp47 map.
            if (nLang == pImpl->mnLangID)
            {
                if (!pImpl->mbInitializedBcp47)
                    pImpl->convertLocaleToBcp47();
                ::std::pair< MapBcp47::const_iterator, bool > res(
                        theMapBcp47::get().insert( ::std::make_pair( pImpl->maBcp47, pImpl)));
                if (res.second)
                {
                    SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: cross-inserted '" << pImpl->maBcp47 << "' for 0x" << ::std::hex << mnLangID);
                }
                else
                {
                    SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: not cross-inserted '" << pImpl->maBcp47 << "' for 0x" << ::std::hex << mnLangID << " have 0x"
                            << ::std::hex << (*res.first).second->mnLangID);
                }
            }
            else
            {
                if (!pImpl->mbInitializedBcp47)
                    pImpl->convertLocaleToBcp47();
                SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: not cross-inserted '" << pImpl->maBcp47 << "' for 0x" << ::std::hex << mnLangID << " round-trip to 0x" << ::std::hex << nLang);
            }
        }
    }
    else if (!maBcp47.isEmpty())
    {
        MapBcp47& rMap = theMapBcp47::get();
        MapBcp47::const_iterator it( rMap.find( maBcp47));
        if (it != rMap.end())
        {
            SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: found impl for '" << maBcp47 << "'");
            pImpl = (*it).second;
        }
        else
        {
            SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: new impl for '" << maBcp47 << "'");
            pImpl.reset( new LanguageTagImpl( *this));
            ::std::pair< MapBcp47::iterator, bool > insOrig( rMap.insert( ::std::make_pair( maBcp47, pImpl)));
            // If changed after canonicalize() also add the resulting tag to
            // the map.
            if (pImpl->synCanonicalize())
            {
                SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: canonicalized to '" << pImpl->maBcp47 << "'");
                ::std::pair< MapBcp47::const_iterator, bool > insCanon(
                        rMap.insert( ::std::make_pair( pImpl->maBcp47, pImpl)));
                SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: " << (insCanon.second ? "" : "not ")
                        << "inserted '" << pImpl->maBcp47 << "'");
                // If the canonicalized tag already existed (was not inserted)
                // and impls are different, make this impl that impl and skip
                // the rest if that LangID is present as well. The existing
                // entry may or may not be different, it may even be strictly
                // identical to this if it differs only in case (e.g. ko-kr =>
                // ko-KR) which was corrected in canonicalize() hence also in
                // the map entry but comparison is case insensitive and found
                // it again.
                if (!insCanon.second && (*insCanon.first).second != pImpl)
                {
                    (*insOrig.first).second = pImpl = (*insCanon.first).second;
                    SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: share impl with 0x"
                            << ::std::hex << pImpl->mnLangID);
                }
            }
            if (!pImpl->mbInitializedLangID)
            {
                // Try round-trip Bcp47->Locale->LangID->Locale->Bcp47.
                if (!pImpl->mbInitializedLocale)
                    pImpl->convertBcp47ToLocale();
                if (!pImpl->mbInitializedLangID)
                    pImpl->convertLocaleToLang( true);
                // Unconditionally insert (round-trip is possible) for
                // on-the-fly IDs and (generated or not) suggested IDs.
                bool bInsert = lcl_isKnownOnTheFlyID( pImpl->mnLangID);
                OUString aBcp47;
                if (!bInsert)
                {
                    if (pImpl->mnLangID != LANGUAGE_DONTKNOW)
                    {
                        // May have involved canonicalize(), so compare with
                        // pImpl->maBcp47 instead of maBcp47!
                        aBcp47 = LanguageTagImpl::convertToBcp47(
                                MsLangId::Conversion::convertLanguageToLocale( pImpl->mnLangID ));
                        bInsert = (aBcp47 == pImpl->maBcp47);
                    }
                }
                // If round-trip is identical cross-insert to Bcp47 map.
                if (bInsert)
                {
                    ::std::pair< MapLangID::const_iterator, bool > res(
                            theMapLangID::get().insert( ::std::make_pair( pImpl->mnLangID, pImpl)));
                    if (res.second)
                    {
                        SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: cross-inserted 0x"
                                << ::std::hex << pImpl->mnLangID << " for '" << maBcp47 << "'");
                    }
                    else
                    {
                        SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: not cross-inserted 0x"
                                << ::std::hex << pImpl->mnLangID << " for '" << maBcp47 << "' have '"
                                << (*res.first).second->maBcp47 << "'");
                    }
                }
                else
                {
                    SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: not cross-inserted 0x"
                            << ::std::hex << pImpl->mnLangID << " for '" << maBcp47 << "' round-trip to '"
                            << aBcp47 << "'");
                }
            }
        }
    }
    else
    {
        SAL_WARN( "i18nlangtag", "LanguageTag::registerImpl: can't register for 0x" << ::std::hex << mnLangID );
        pImpl.reset( new LanguageTagImpl( *this));
    }

    // If we reach here for mbSystemLocale we didn't have theSystemLocale
    // above, so add it.
    if (mbSystemLocale && mbInitializedLangID)
    {
        theSystemLocale::get() = pImpl;
        SAL_INFO( "i18nlangtag", "LanguageTag::registerImpl: added system locale 0x"
                << ::std::hex << pImpl->mnLangID << " '" << pImpl->maBcp47 << "'");
    }

    return pImpl;
}


LanguageTagImpl const * LanguageTag::getImpl() const
{
    if (!mpImpl)
    {
        mpImpl = registerImpl();
        syncVarsFromRawImpl();
    }
    return mpImpl.get();
}

LanguageTagImpl * LanguageTag::getImpl()
{
    if (!mpImpl)
    {
        mpImpl = registerImpl();
        syncVarsFromRawImpl();
    }
    return mpImpl.get();
}

void LanguageTag::resetVars()
{
    mpImpl.reset();
    maLocale            = lang::Locale();
    maBcp47.clear();
    mnLangID            = LANGUAGE_SYSTEM;
    mbSystemLocale      = true;
    mbInitializedBcp47  = false;
    mbInitializedLocale = false;
    mbInitializedLangID = false;
    mbIsFallback        = false;
}


LanguageTag & LanguageTag::reset( const OUString & rBcp47LanguageTag )
{
    resetVars();
    maBcp47             = rBcp47LanguageTag;
    mbSystemLocale      = rBcp47LanguageTag.isEmpty();
    mbInitializedBcp47  = !mbSystemLocale;

    return *this;
}


LanguageTag & LanguageTag::reset( const css::lang::Locale & rLocale )
{
    resetVars();
    maLocale            = rLocale;
    mbSystemLocale      = rLocale.Language.isEmpty();
    mbInitializedLocale = !mbSystemLocale;
    handleVendorVariant( maLocale);
    return *this;
}


LanguageTag & LanguageTag::reset( LanguageType nLanguage )
{
    resetVars();
    mnLangID            = nLanguage;
    mbSystemLocale      = nLanguage == LANGUAGE_SYSTEM;
    mbInitializedLangID = !mbSystemLocale;
    return *this;
}


bool LanguageTagImpl::canonicalize()
{
#ifdef erDEBUG
    // dump once
    struct dumper
    {
        lt_tag_t** mpp;
        explicit dumper( lt_tag_t** pp ) : mpp( *pp ? NULL : pp) {}
        ~dumper() { if (mpp && *mpp) lt_tag_dump( *mpp); }
    };
    dumper aDumper( &mpImplLangtag);
#endif

    bool bChanged = false;

    // Side effect: have maBcp47 in any case, resolved system.
    // Some methods calling canonicalize() (or not calling it due to
    // meIsLiblangtagNeeded==DECISION_NO) rely on this! Hence do not set
    // meIsLiblangtagNeeded anywhere else than hereafter.
    getBcp47();

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
                    else if (eExt == EXTRACTED_C_LOCALE)
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
            if (!mbInitializedLangID)
            {
                if (convertLocaleToLang( false))
                    bChanged = true;
                if (bTemporaryLocale || mnLangID == LANGUAGE_DONTKNOW)
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
            // We may have an internal override "canonicalization".
            lang::Locale aNew( MsLangId::Conversion::getOverride( maLocale));
            if (!aNew.Language.isEmpty() &&
                    (aNew.Language != maLocale.Language ||
                     aNew.Country  != maLocale.Country ||
                     aNew.Variant  != maLocale.Variant))
            {
                maBcp47 = LanguageTagImpl::convertToBcp47( aNew);
                bChanged = true;
                meIsIsoLocale = DECISION_DONTKNOW;
                meIsIsoODF = DECISION_DONTKNOW;
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
        return bChanged;            // that's it
    }

    meIsLiblangtagNeeded = DECISION_YES;
    SAL_INFO( "i18nlangtag", "LanguageTagImpl::canonicalize: using liblangtag for '" << maBcp47 << "'");

    if (!mpImplLangtag)
    {
        theDataRef::get().init();
        mpImplLangtag = lt_tag_new();
    }

    myLtError aError;

    if (lt_tag_parse( mpImplLangtag, OUStringToOString( maBcp47, RTL_TEXTENCODING_UTF8).getStr(), &aError.p))
    {
        char* pTag = lt_tag_canonicalize( mpImplLangtag, &aError.p);
        SAL_WARN_IF( !pTag, "i18nlangtag", "LanguageTagImpl::canonicalize: could not canonicalize '" << maBcp47 << "'");
        if (pTag)
        {
            OUString aNew( OUString::createFromAscii( pTag));
            // Make the lt_tag_t follow the new string if different, which
            // removes default script and such.
            if (maBcp47 != aNew)
            {
                maBcp47 = aNew;
                bChanged = true;
                meIsIsoLocale = DECISION_DONTKNOW;
                meIsIsoODF = DECISION_DONTKNOW;
                if (!lt_tag_parse( mpImplLangtag, pTag, &aError.p))
                {
                    SAL_WARN( "i18nlangtag", "LanguageTagImpl::canonicalize: could not reparse '" << maBcp47 << "'");
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
        SAL_INFO( "i18nlangtag", "LanguageTagImpl::canonicalize: could not parse '" << maBcp47 << "'");
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
    bool bRegister = ((mbInitializedBcp47 && maBcp47 != pImpl->maBcp47) ||
            (mbInitializedLangID && mnLangID != pImpl->mnLangID));
    SAL_INFO_IF( bRegister, "i18nlangtag",
            "LanguageTag::syncFromImpl: re-registering, '" << pImpl->maBcp47 << "' vs '" << maBcp47 <<
            " and 0x" << ::std::hex << pImpl->mnLangID << " vs 0x" << ::std::hex << mnLangID);
    syncVarsFromRawImpl();
    if (bRegister)
        mpImpl = registerImpl();
}


void LanguageTag::syncVarsFromImpl() const
{
    if (!mpImpl)
        getImpl();      // with side effect syncVarsFromRawImpl()
    else
        syncVarsFromRawImpl();
}


void LanguageTag::syncVarsFromRawImpl() const
{
    // Do not use getImpl() here.
    LanguageTagImpl* pImpl = mpImpl.get();
    if (!pImpl)
        return;

    // Obviously only mutable variables.
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

    if (maLocale.Language.isEmpty())
    {
        // Do not call LanguageTag::convertToBcp47(Locale) that for an empty
        // locale via LanguageTag::convertToBcp47(LanguageType) and
        // LanguageTag::convertToLocale(LanguageType) would instantiate another
        // LanguageTag.
        maLocale = MsLangId::Conversion::convertLanguageToLocale( LANGUAGE_SYSTEM );
    }
    if (maLocale.Language.isEmpty())
    {
        maBcp47.clear();   // bad luck
    }
    else if (maLocale.Language == I18NLANGTAG_QLT)
    {
        maBcp47 = maLocale.Variant;
        meIsIsoLocale = DECISION_NO;
    }
    else
    {
        maBcp47 = LanguageTag::convertToBcp47( maLocale );
    }
    mbInitializedBcp47 = true;
}


bool LanguageTagImpl::convertLocaleToLang( bool bAllowOnTheFlyID )
{
    bool bRemapped = false;
    if (mbSystemLocale)
    {
        mnLangID = MsLangId::getRealLanguage( LANGUAGE_SYSTEM);
    }
    else
    {
        mnLangID = MsLangId::Conversion::convertLocaleToLanguage( maLocale);
        if (mnLangID == LANGUAGE_DONTKNOW)
        {
            // convertLocaleToLanguage() only searches in ISO and private
            // definitions, search in remaining definitions, i.e. for the "C"
            // locale and non-standard things like "sr-latin" or "german" to
            // resolve to a known locale, skipping ISO lll-CC that were already
            // searched.
            mnLangID = MsLangId::Conversion::convertIsoNamesToLanguage( maLocale.Language, maLocale.Country, true);
            if (mnLangID != LANGUAGE_DONTKNOW)
            {
                // If one found, convert back and adapt Locale and Bcp47
                // strings so we have a matching entry.
                OUString aOrgBcp47( maBcp47);
                convertLangToLocale();
                convertLocaleToBcp47();
                bRemapped = (maBcp47 != aOrgBcp47);
            }
        }
        if (mnLangID == LANGUAGE_DONTKNOW && bAllowOnTheFlyID)
        {
            if (isValidBcp47())
            {
                // For language-only (including script) look if we know some
                // locale of that language and if so try to use the primary
                // language ID of that instead of generating an on-the-fly ID.
                if (getCountry().isEmpty() && isIsoODF())
                {
                    lang::Locale aLoc( MsLangId::Conversion::lookupFallbackLocale( maLocale));
                    // 'en-US' is last resort, do not use except when looking
                    // for 'en'.
                    if (aLoc.Language != "en" || getLanguage() == "en")
                    {
                        mnLangID = MsLangId::Conversion::convertLocaleToLanguage( aLoc);
                        if (mnLangID != LANGUAGE_DONTKNOW)
                            mnLangID = MsLangId::getPrimaryLanguage( mnLangID);
                    }
                }
                registerOnTheFly( mnLangID);
            }
            else
            {
                SAL_WARN( "i18nlangtag", "LanguageTagImpl::convertLocaleToLang: with bAllowOnTheFlyID invalid '"
                        << maBcp47 << "'");
            }
        }
    }
    mbInitializedLangID = true;
    return bRemapped;
}


void LanguageTag::convertLocaleToLang()
{
    getImpl()->convertLocaleToLang( true);
    syncFromImpl();
}


void LanguageTagImpl::convertBcp47ToLocale()
{
    bool bIso = isIsoLocale();
    if (bIso)
    {
        maLocale.Language = getLanguageFromLangtag();
        maLocale.Country = getRegionFromLangtag();
        maLocale.Variant.clear();
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
    getImpl()->convertBcp47ToLocale();
    syncFromImpl();
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
        convertLocaleToLang( true);
    }
    mbInitializedLangID = true;
}


void LanguageTag::convertBcp47ToLang()
{
    getImpl()->convertBcp47ToLang();
    syncFromImpl();
}


void LanguageTagImpl::convertLangToLocale()
{
    if (mbSystemLocale && !mbInitializedLangID)
    {
        mnLangID = MsLangId::getRealLanguage( LANGUAGE_SYSTEM);
        mbInitializedLangID = true;
    }
    // Resolve system here! The original is remembered as mbSystemLocale.
    maLocale = MsLangId::Conversion::convertLanguageToLocale( mnLangID );
    mbInitializedLocale = true;
}


void LanguageTag::convertLangToLocale()
{
    getImpl()->convertLangToLocale();
    syncFromImpl();
}


void LanguageTagImpl::convertLangToBcp47()
{
    if (!mbInitializedLocale)
        convertLangToLocale();
    convertLocaleToBcp47();
    mbInitializedBcp47 = true;
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
        theDataRef::get().init();
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


const OUString & LanguageTagImpl::getBcp47() const
{
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
        syncVarsFromImpl();
    if (!mbInitializedBcp47)
    {
        getImpl()->getBcp47();
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
        SAL_WARN_IF( !pLangT, "i18nlangtag",
                "LanguageTag::getLanguageFromLangtag: pLangT==NULL for '" << maBcp47 << "'");
        if (!pLangT)
            return aLanguage;
        const char* pLang = lt_lang_get_tag( pLangT);
        SAL_WARN_IF( !pLang, "i18nlangtag",
                "LanguageTag::getLanguageFromLangtag: pLang==NULL for '" << maBcp47 << "'");
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
                "i18nlangtag", "LanguageTag::getRegionFromLangtag: pRegionT==NULL for '" << maBcp47 << "'");
        if (!pRegionT)
            return aRegion;
        const char* pRegion = lt_region_get_tag( pRegionT);
        SAL_WARN_IF( !pRegion, "i18nlangtag",
                "LanguageTag::getRegionFromLangtag: pRegion==NULL for'" << maBcp47 << "'");
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


const css::lang::Locale & LanguageTag::getLocale( bool bResolveSystem ) const
{
    if (!bResolveSystem && mbSystemLocale)
        return theEmptyLocale::get();
    if (!mbInitializedLocale)
        syncVarsFromImpl();
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
        syncVarsFromImpl();
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

inline bool isLowerAscii( sal_Unicode c )
{
    return 'a' <= c && c <= 'z';
}

inline bool isUpperAscii( sal_Unicode c )
{
    return 'A' <= c && c <= 'Z';
}

}


// static
bool LanguageTag::isIsoLanguage( const OUString& rLanguage )
{
    /* TODO: ignore case? For now let's see where rubbish is used. */
    bool b2chars = rLanguage.getLength() == 2;
    if ((b2chars || rLanguage.getLength() == 3) &&
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


OUString const & LanguageTagImpl::getLanguage() const
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
    LanguageTagImpl const* pImpl = getImpl();
    if (pImpl->mbCachedLanguage)
        return pImpl->maCachedLanguage;
    OUString aRet( pImpl->getLanguage());
    const_cast<LanguageTag*>(this)->syncFromImpl();
    return aRet;
}


OUString const & LanguageTagImpl::getScript() const
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
    LanguageTagImpl const* pImpl = getImpl();
    if (pImpl->mbCachedScript)
        return pImpl->maCachedScript;
    OUString aRet( pImpl->getScript());
    const_cast<LanguageTag*>(this)->syncFromImpl();
    return aRet;
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


OUString const & LanguageTagImpl::getCountry() const
{
    if (!mbCachedCountry)
    {
        maCachedCountry = const_cast<LanguageTagImpl*>(this)->getRegionFromLangtag();
        if (!LanguageTag::isIsoCountry( maCachedCountry))
            maCachedCountry.clear();
        mbCachedCountry = true;
    }
    return maCachedCountry;
}


OUString LanguageTag::getCountry() const
{
    LanguageTagImpl const* pImpl = getImpl();
    if (pImpl->mbCachedCountry)
        return pImpl->maCachedCountry;
    OUString aRet( pImpl->getCountry());
    const_cast<LanguageTag*>(this)->syncFromImpl();
    return aRet;
}


OUString LanguageTagImpl::getRegion() const
{
    return const_cast<LanguageTagImpl*>(this)->getRegionFromLangtag();
}


OUString const & LanguageTagImpl::getVariants() const
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
    LanguageTagImpl const * pImpl = getImpl();
    if (pImpl->mbCachedVariants)
        return pImpl->maCachedVariants;
    OUString aRet( pImpl->getVariants());
    const_cast<LanguageTag*>(this)->syncFromImpl();
    return aRet;
}

OUString LanguageTagImpl::getGlibcLocaleString() const
{
    OUString sLocale;
    if (!mpImplLangtag)
    {
        meIsLiblangtagNeeded = DECISION_YES;
        const_cast<LanguageTagImpl*>(this)->synCanonicalize();
    }
    if (mpImplLangtag)
    {
        char* pLang = lt_tag_convert_to_locale(mpImplLangtag, nullptr);
        if (pLang)
        {
            sLocale = OUString::createFromAscii( pLang);
            free(pLang);
        }
    }
    return sLocale;
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
        aRet = getImpl()->getGlibcLocaleString();
        sal_Int32 nAt = aRet.indexOf('@');
        if (nAt != -1)
            aRet = aRet.copy(0, nAt) + rEncoding + aRet.copy(nAt);
        else
            aRet += rEncoding;
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
    bool bRet = getImpl()->hasScript();
    const_cast<LanguageTag*>(this)->syncFromImpl();
    return bRet;
}


LanguageTag::ScriptType LanguageTagImpl::getScriptType() const
{
    return meScriptType;
}


LanguageTag::ScriptType LanguageTag::getScriptType() const
{
    return getImpl()->getScriptType();
}


void LanguageTagImpl::setScriptType(LanguageTag::ScriptType st)
{
    if (meScriptType == LanguageTag::ScriptType::UNKNOWN)  // poor man's clash resolution
        meScriptType = st;
}


void LanguageTag::setScriptType(LanguageTag::ScriptType st)
{
    getImpl()->setScriptType(st);
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
    bool bRet = getImpl()->isIsoLocale();
    const_cast<LanguageTag*>(this)->syncFromImpl();
    return bRet;
}


bool LanguageTagImpl::isIsoODF() const
{
    if (meIsIsoODF == DECISION_DONTKNOW)
    {
        const_cast<LanguageTagImpl*>(this)->synCanonicalize();
        if (!LanguageTag::isIsoScript( getScript()))
        {
            meIsIsoODF = DECISION_NO;
            return false;
        }
        // The usual case is lll-CC so simply check that first.
        if (isIsoLocale())
        {
            meIsIsoODF = DECISION_YES;
            return true;
        }
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
    bool bRet = getImpl()->isIsoODF();
    const_cast<LanguageTag*>(this)->syncFromImpl();
    return bRet;
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
    bool bRet = getImpl()->isValidBcp47();
    const_cast<LanguageTag*>(this)->syncFromImpl();
    return bRet;
}


LanguageTag & LanguageTag::makeFallback()
{
    if (!mbIsFallback)
    {
        const lang::Locale& rLocale1 = getLocale();
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


/* TODO: maybe this now could take advantage of the mnOverride field in
 * isolang.cxx entries and search for kSAME instead of hardcoded special
 * fallbacks. Though iterating through those tables would be slower and even
 * then there would be some special cases, but we wouldn't lack entries that
 * were missed out. */
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
                aVec.emplace_back(aLanguage + "-" + aCountry);
            if (aLanguage == "zh")
            {
                // For zh-HK or zh-MO also list zh-TW, for all other zh-XX also
                // list zh-CN.
                if (aCountry == "HK" || aCountry == "MO")
                    aVec.emplace_back(aLanguage + "-TW");
                else if (aCountry != "CN")
                    aVec.emplace_back(aLanguage + "-CN");
                aVec.push_back( aLanguage);
            }
            else if (aLanguage == "sh")
            {
                // Manual list instead of calling
                // LanguageTag( "sr-Latn-" + aCountry).getFallbackStrings( true)
                // that would also include "sh-*" again.
                aVec.emplace_back("sr-Latn-" + aCountry);
                aVec.emplace_back("sr-Latn");
                aVec.emplace_back("sh");  // legacy with script, before default script with country
                aVec.emplace_back("sr-" + aCountry);
                aVec.emplace_back("sr");
            }
            else if (aLanguage == "ca" && aCountry == "XV")
            {
                ::std::vector< OUString > aRep( LanguageTag( "ca-ES-valencia").getFallbackStrings( true));
                aVec.insert( aVec.end(), aRep.begin(), aRep.end());
                // Already includes 'ca' language fallback.
            }
            else if (aLanguage == "ku")
            {
                if (aCountry == "TR" || aCountry == "SY")
                {
                    aVec.emplace_back("kmr-Latn-" + aCountry);
                    aVec.emplace_back("kmr-" + aCountry);
                    aVec.emplace_back("kmr-Latn");
                    aVec.emplace_back("kmr");
                    aVec.push_back( aLanguage);
                }
                else if (aCountry == "IQ" || aCountry == "IR")
                {
                    aVec.emplace_back("ckb-" + aCountry);
                    aVec.emplace_back("ckb");
                }
            }
            else if (aLanguage == "kmr" && (aCountry == "TR" || aCountry == "SY"))
            {
                aVec.emplace_back("ku-Latn-" + aCountry);
                aVec.emplace_back("ku-" + aCountry);
                aVec.push_back( aLanguage);
                aVec.emplace_back("ku");
            }
            else if (aLanguage == "ckb" && (aCountry == "IQ" || aCountry == "IR"))
            {
                aVec.emplace_back("ku-Arab-" + aCountry);
                aVec.emplace_back("ku-" + aCountry);
                aVec.push_back( aLanguage);
                // not 'ku' only, that was used for Latin script
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
                aVec.emplace_back("sr-Latn");
                aVec.emplace_back("sr");
            }
            else if (aLanguage == "pli")
            {
                // a special case for Pali dictionary, see fdo#41599
                aVec.emplace_back("pi-Latn");
                aVec.emplace_back("pi");
            }
        }
        return aVec;
    }

    getBcp47();     // have maBcp47 now
    if (bIncludeFullBcp47)
        aVec.push_back( maBcp47);

    // Special cases for deprecated tags and their replacements, include both
    // in fallbacks in a sensible order.
    /* TODO: could such things be generalized and automated with liblangtag? */
    if (maBcp47 == "en-GB-oed")
        aVec.emplace_back("en-GB-oxendict");
    else if (maBcp47 == "en-GB-oxendict")
        aVec.emplace_back("en-GB-oed");

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
                    aVec.emplace_back("sr-Latn-YU");
                    aVec.emplace_back("sh-CS");
                    aVec.emplace_back("sh-YU");
                }
                else
                    aVec.emplace_back("sh-" + aCountry);
            }
            else if (aLanguage == "pi" && aScript == "Latn")
                aVec.emplace_back("pli");     // a special case for Pali dictionary, see fdo#41599
            else if (aLanguage == "krm" && aScript == "Latn" && (aCountry == "TR" || aCountry == "SY"))
                aVec.emplace_back("ku-" + aCountry);
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
            aVec.emplace_back("sh");
        else if (aLanguage == "ku" && aScript == "Arab")
            aVec.emplace_back("ckb");
        // 'ku' only denoted Latin script
        else if (aLanguage == "krm" && aScript == "Latn" && aCountry.isEmpty())
            aVec.emplace_back("ku");
    }
    bool bHaveLanguageVariant = false;
    if (!aCountry.isEmpty())
    {
        if (!aVariants.isEmpty())
        {
            aTmp = aLanguage + "-" + aCountry + "-" + aVariants;
            if (aTmp != maBcp47)
                aVec.push_back( aTmp);
            if (maBcp47 == "ca-ES-valencia")
                aVec.emplace_back("ca-XV");
            // Language with variant but without country before language
            // without variant but with country.
            // But only if variant is not from a grandfathered tag that
            // wouldn't match the rules, i.e. "de-1901" is fine but "en-oed" is
            // not.
            if (aVariants.getLength() >= 5 ||
                    (aVariants.getLength() == 4 && '0' <= aVariants[0] && aVariants[0] <= '9'))
            {
                aTmp = aLanguage + "-" + aVariants;
                if (aTmp != maBcp47)
                    aVec.push_back( aTmp);
                bHaveLanguageVariant = true;
            }
        }
        aTmp = aLanguage + "-" + aCountry;
        if (aTmp != maBcp47)
            aVec.push_back( aTmp);
    }
    if (!aVariants.isEmpty() && !bHaveLanguageVariant)
    {
        // Only if variant is not from a grandfathered tag that wouldn't match
        // the rules, i.e. "de-1901" is fine but "en-oed" is not.
        if (aVariants.getLength() >= 5 ||
                (aVariants.getLength() == 4 && '0' <= aVariants[0] && aVariants[0] <= '9'))
        {
            aTmp = aLanguage + "-" + aVariants;
            if (aTmp != maBcp47)
                aVec.push_back( aTmp);
        }
    }

    // Insert legacy fallbacks with country before language-only, but only
    // default script, script was handled already above.
    if (!aCountry.isEmpty())
    {
        if (aLanguage == "sr" && aCountry == "CS")
            aVec.emplace_back("sr-YU");
    }

    // Original language-only.
    if (aLanguage != maBcp47)
        aVec.push_back( aLanguage);

    return aVec;
}


OUString LanguageTag::getBcp47MS() const
{
    if (getLanguageType() == LANGUAGE_SPANISH_DATED)
        return OUString("es-ES_tradnl");
    return getBcp47();
}


bool LanguageTag::equals( const LanguageTag & rLanguageTag ) const
{
    // If SYSTEM is not to be resolved or either both are SYSTEM or none, we
    // can use the operator==() optimization.
    if (isSystemLocale() == rLanguageTag.isSystemLocale())
        return operator==( rLanguageTag);

    // Compare full language tag strings.
    return getBcp47() == rLanguageTag.getBcp47();
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


bool LanguageTag::operator<( const LanguageTag & rLanguageTag ) const
{
    return getBcp47( false).compareToIgnoreAsciiCase( rLanguageTag.getBcp47( false)) < 0;
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
    else if (nLen == 1 && rBcp47[0] == 'C')         // the 'C' locale
    {
        eRet = EXTRACTED_C_LOCALE;
        rLanguage = "C";
        rScript.clear();
        rCountry.clear();
        rVariants.clear();
    }
    else if (nLen == 2 || nLen == 3)                // ll or lll
    {
        if (nHyph1 < 0)
        {
            rLanguage = rBcp47.toAsciiLowerCase();
            rScript.clear();
            rCountry.clear();
            rVariants.clear();
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
            rScript.clear();
            rVariants.clear();
            eRet = EXTRACTED_LSC;
        }
    }
    else if (  (nHyph1 == 2 && nLen ==  7)          // ll-Ssss or ll-vvvv
            || (nHyph1 == 3 && nLen ==  8))         // lll-Ssss or lll-vvvv
    {
        if (nHyph2 < 0)
        {
            sal_Unicode c = rBcp47[nHyph1+1];
            if ('0' <= c && c <= '9')
            {
                // (DIGIT 3ALNUM) vvvv variant instead of Ssss script
                rLanguage = rBcp47.copy( 0, nHyph1).toAsciiLowerCase();
                rScript.clear();
                rCountry.clear();
                rVariants = rBcp47.copy( nHyph1 + 1);
                eRet = EXTRACTED_LV;
            }
            else
            {
                rLanguage = rBcp47.copy( 0, nHyph1).toAsciiLowerCase();
                rScript   = rBcp47.copy( nHyph1 + 1, 1).toAsciiUpperCase() +
                            rBcp47.copy( nHyph1 + 2, 3).toAsciiLowerCase();
                rCountry.clear();
                rVariants.clear();
                eRet = EXTRACTED_LSC;
            }
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
            rVariants.clear();
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
    else if (  (nHyph1 == 2 && nHyph2 == 5 && nHyph3 == 7)      // ll-CC-u-...
            || (nHyph1 == 3 && nHyph2 == 6 && nHyph3 == 8))     // lll-CC-u-...
    {
        if (rBcp47[nHyph3-1] == 'u')
        {
            // Need to recognize as known, otherwise getLanguage() and
            // getCountry() return empty string because mpImplLangtag is not
            // used with a known mapping.
            /* TODO: if there were more this would get ugly and needed some
             * table driven approach via isolang.cxx instead. */
            if (rBcp47.equalsIgnoreAsciiCase( "es-ES-u-co-trad"))
            {
                rLanguage = "es";
                rScript.clear();
                rCountry  = "ES";
                rVariants = "u-co-trad";    // not strictly a variant, but used to reconstruct the tag.
                eRet = EXTRACTED_LV;
            }
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
            rScript.clear();
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
            rScript.clear();
            rCountry.clear();
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
                rScript.clear();
                rCountry  = "GB";
                rVariants = "oed";
                eRet = EXTRACTED_LV;
            }
            // Other known and handled odd cases.
            else if (rBcp47.equalsIgnoreAsciiCase( "es-ES_tradnl"))
            {
                // Will get overridden, but needs to be recognized as known.
                rLanguage = "es";
                rScript.clear();
                rCountry  = "ES";
                rVariants = "tradnl";   // this is nonsense, but.. ignored.
                eRet = EXTRACTED_KNOWN_BAD;
            }
        }
    }
    if (eRet == EXTRACTED_NONE)
    {
        SAL_INFO( "i18nlangtag", "LanguageTagImpl::simpleExtract: did not extract '" << rBcp47 << "'");
        rLanguage.clear();
        rScript.clear();
        rCountry.clear();
        rVariants.clear();
    }
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
        aFallbacks.emplace_back("en-US");
        if (rReference != "en")
            aFallbacks.emplace_back("en");
    }
    if (rReference != "x-default")
        aFallbacks.emplace_back("x-default");
    if (rReference != "x-no-translate")
        aFallbacks.emplace_back("x-no-translate");
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
::std::vector< css::lang::Locale >::const_iterator LanguageTag::getMatchingFallback(
        const ::std::vector< css::lang::Locale > & rList,
        const css::lang::Locale & rReference )
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
    return nNewLangID != nLangID;
}


// static
css::lang::Locale LanguageTag::convertToLocale( LanguageType nLangID, bool bResolveSystem )
{
    if (!bResolveSystem && lcl_isSystem( nLangID))
        return lang::Locale();

    return LanguageTag( nLangID).getLocale( bResolveSystem);
}


// static
LanguageType LanguageTag::convertToLanguageType( const css::lang::Locale& rLocale, bool bResolveSystem )
{
    if (rLocale.Language.isEmpty() && !bResolveSystem)
        return LANGUAGE_SYSTEM;

    return LanguageTag( rLocale).getLanguageType( bResolveSystem);
}


// static
OUString LanguageTagImpl::convertToBcp47( const css::lang::Locale& rLocale )
{
    OUString aBcp47;
    if (rLocale.Language.isEmpty())
    {
        // aBcp47 stays empty
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
OUString LanguageTag::convertToBcp47( const css::lang::Locale& rLocale, bool bResolveSystem )
{
    OUString aBcp47;
    if (rLocale.Language.isEmpty())
    {
        if (bResolveSystem)
            aBcp47 = LanguageTag::convertToBcp47( LANGUAGE_SYSTEM );
        // else aBcp47 stays empty
    }
    else
    {
        aBcp47 = LanguageTagImpl::convertToBcp47( rLocale);
    }
    return aBcp47;
}


// static
OUString LanguageTag::convertToBcp47( LanguageType nLangID )
{
    lang::Locale aLocale( LanguageTag::convertToLocale( nLangID ));
    // If system for some reason (should not happen.. haha) could not be
    // resolved DO NOT CALL LanguageTag::convertToBcp47(Locale) because that
    // would recurse into this method here!
    if (aLocale.Language.isEmpty())
        return OUString();      // bad luck, bail out
    return LanguageTagImpl::convertToBcp47( aLocale);
}


// static
css::lang::Locale LanguageTag::convertToLocale( const OUString& rBcp47, bool bResolveSystem )
{
    if (rBcp47.isEmpty() && !bResolveSystem)
        return lang::Locale();

    return LanguageTag( rBcp47).getLocale( bResolveSystem);
}


// static
LanguageType LanguageTag::convertToLanguageType( const OUString& rBcp47 )
{
    return LanguageTag( rBcp47).getLanguageType();
}


// static
LanguageType LanguageTag::convertToLanguageTypeWithFallback( const OUString& rBcp47 )
{
    return LanguageTag( rBcp47).makeFallback().getLanguageType();
}


// static
css::lang::Locale LanguageTag::convertToLocaleWithFallback( const OUString& rBcp47 )
{
    return LanguageTag( rBcp47).makeFallback().getLocale();
}


// static
bool LanguageTag::isValidBcp47( const OUString& rString, OUString* o_pCanonicalized, bool bDisallowPrivate )
{
    bool bValid = false;

    struct guard
    {
        lt_tag_t* mpLangtag;
        guard()
        {
            theDataRef::get().init();
            mpLangtag = lt_tag_new();
        }
        ~guard()
        {
            lt_tag_unref( mpLangtag);
        }
    } aVar;

    myLtError aError;

    if (lt_tag_parse( aVar.mpLangtag, OUStringToOString( rString, RTL_TEXTENCODING_UTF8).getStr(), &aError.p))
    {
        char* pTag = lt_tag_canonicalize( aVar.mpLangtag, &aError.p);
        SAL_WARN_IF( !pTag, "i18nlangtag", "LanguageTag:isValidBcp47: could not canonicalize '" << rString << "'");
        if (pTag)
        {
            bValid = true;
            if (bDisallowPrivate)
            {
                const lt_string_t* pPrivate = lt_tag_get_privateuse( aVar.mpLangtag);
                if (pPrivate && lt_string_length( pPrivate) > 0)
                    bValid = false;
                else
                {
                    const lt_lang_t* pLangT = lt_tag_get_language( aVar.mpLangtag);
                    if (pLangT)
                    {
                        const char* pLang = lt_lang_get_tag( pLangT);
                        if (pLang && strcmp( pLang, I18NLANGTAG_QLT) == 0)
                        {
                            // Disallow 'qlt' privateuse code to prevent
                            // confusion with our internal usage.
                            bValid = false;
                        }
                    }
                }
            }
            if (o_pCanonicalized)
                *o_pCanonicalized = OUString::createFromAscii( pTag);
            free( pTag);
            return bValid;
        }
    }
    else
    {
        SAL_INFO( "i18nlangtag", "LanguageTag:isValidBcp47: could not parse '" << rString << "'");
    }
    return bValid;
}

LanguageTag makeLanguageTagFromAppleLanguageId(AppleLanguageId nLanguage)
{
    //map the simple ones via LanguageTypes, and the hard ones explicitly
    LanguageType nLang(LANGUAGE_DONTKNOW);

    switch (nLanguage)
    {
        case AppleLanguageId::ENGLISH:
            nLang = LANGUAGE_ENGLISH;
            break;
        case AppleLanguageId::FRENCH:
            nLang = LANGUAGE_FRENCH;
            break;
        case AppleLanguageId::GERMAN:
            nLang = LANGUAGE_GERMAN;
            break;
        case AppleLanguageId::ITALIAN:
            nLang = LANGUAGE_ITALIAN;
            break;
        case AppleLanguageId::DUTCH:
            nLang = LANGUAGE_DUTCH;
            break;
        case AppleLanguageId::SWEDISH:
            nLang = LANGUAGE_SWEDISH;
            break;
        case AppleLanguageId::SPANISH:
            nLang = LANGUAGE_SPANISH;
            break;
        case AppleLanguageId::DANISH:
            nLang = LANGUAGE_DANISH;
            break;
        case AppleLanguageId::PORTUGUESE:
            nLang = LANGUAGE_PORTUGUESE;
            break;
        case AppleLanguageId::NORWEGIAN:
            nLang = LANGUAGE_NORWEGIAN;
            break;
        case AppleLanguageId::HEBREW:
            nLang = LANGUAGE_HEBREW;
            break;
        case AppleLanguageId::JAPANESE:
            nLang = LANGUAGE_JAPANESE;
            break;
        case AppleLanguageId::ARABIC:
            nLang = LANGUAGE_ARABIC_PRIMARY_ONLY;
            break;
        case AppleLanguageId::FINNISH:
            nLang = LANGUAGE_FINNISH;
            break;
        case AppleLanguageId::GREEK:
            nLang = LANGUAGE_GREEK;
            break;
        case AppleLanguageId::ICELANDIC:
            nLang = LANGUAGE_ICELANDIC;
            break;
        case AppleLanguageId::MALTESE:
            nLang = LANGUAGE_MALTESE;
            break;
        case AppleLanguageId::TURKISH:
            nLang = LANGUAGE_TURKISH;
            break;
        case AppleLanguageId::CROATIAN:
            nLang = LANGUAGE_CROATIAN;
            break;
        case AppleLanguageId::CHINESE_TRADITIONAL:
            nLang = LANGUAGE_CHINESE_TRADITIONAL;
            break;
        case AppleLanguageId::URDU:
            nLang = LANGUAGE_URDU_PAKISTAN; //probably, otherwise we need a LANGUAGE_URDU_PRIMARY_ONLY
            break;
        case AppleLanguageId::HINDI:
            nLang = LANGUAGE_HINDI;
            break;
        case AppleLanguageId::THAI:
            nLang = LANGUAGE_THAI;
            break;
        case AppleLanguageId::KOREAN:
            nLang = LANGUAGE_KOREAN;
            break;
        case AppleLanguageId::LITHUANIAN:
            nLang = LANGUAGE_LITHUANIAN;
            break;
        case AppleLanguageId::POLISH:
            nLang = LANGUAGE_POLISH;
            break;
        case AppleLanguageId::HUNGARIAN:
            nLang = LANGUAGE_HUNGARIAN;
            break;
        case AppleLanguageId::ESTONIAN:
            nLang = LANGUAGE_ESTONIAN;
            break;
        case AppleLanguageId::LATVIAN:
            nLang = LANGUAGE_LATVIAN;
            break;
        case AppleLanguageId::SAMI:
            nLang = LANGUAGE_SAMI_NORTHERN_NORWAY; //maybe
            break;
        case AppleLanguageId::FAROESE:
            nLang = LANGUAGE_FAEROESE;
            break;
        case AppleLanguageId::FARSI:
            nLang = LANGUAGE_FARSI;
            break;
        case AppleLanguageId::RUSSIAN:
            nLang = LANGUAGE_RUSSIAN;
            break;
        case AppleLanguageId::CHINESE_SIMPLIFIED:
            nLang = LANGUAGE_CHINESE_SIMPLIFIED;
            break;
        case AppleLanguageId::FLEMISH:
            nLang = LANGUAGE_DUTCH_BELGIAN;
            break;
        case AppleLanguageId::IRISH_GAELIC:
            nLang = LANGUAGE_GAELIC_IRELAND;
            break;
        case AppleLanguageId::ALBANIAN:
            nLang = LANGUAGE_ALBANIAN;
            break;
        case AppleLanguageId::ROMANIAN:
            nLang = LANGUAGE_ROMANIAN;
            break;
        case AppleLanguageId::CZECH:
            nLang = LANGUAGE_CZECH;
            break;
        case AppleLanguageId::SLOVAK:
            nLang = LANGUAGE_SLOVAK;
            break;
        case AppleLanguageId::SLOVENIAN:
            nLang = LANGUAGE_SLOVENIAN;
            break;
        case AppleLanguageId::YIDDISH:
            nLang = LANGUAGE_YIDDISH;
            break;
        case AppleLanguageId::SERBIAN:
            nLang = LANGUAGE_SERBIAN_CYRILLIC_SERBIA;   //maybe
            break;
        case AppleLanguageId::MACEDONIAN:
            nLang = LANGUAGE_MACEDONIAN;
            break;
        case AppleLanguageId::BULGARIAN:
            nLang = LANGUAGE_BULGARIAN;
            break;
        case AppleLanguageId::UKRAINIAN:
            nLang = LANGUAGE_UKRAINIAN;
            break;
        case AppleLanguageId::BYELORUSSIAN:
            nLang = LANGUAGE_BELARUSIAN;
            break;
        case AppleLanguageId::UZBEK:
            nLang = LANGUAGE_UZBEK_CYRILLIC; //maybe
            break;
        case AppleLanguageId::KAZAKH:
            nLang = LANGUAGE_KAZAKH;
            break;
        case AppleLanguageId::AZERI_CYRILLIC:
            nLang = LANGUAGE_AZERI_CYRILLIC;
            break;
        case AppleLanguageId::AZERI_ARABIC:
            return LanguageTag("az-Arab");
            break;
        case AppleLanguageId::ARMENIAN:
            nLang = LANGUAGE_ARMENIAN;
            break;
        case AppleLanguageId::GEORGIAN:
            nLang = LANGUAGE_GEORGIAN;
            break;
        case AppleLanguageId::MOLDAVIAN:
            nLang = LANGUAGE_ROMANIAN_MOLDOVA;
            break;
        case AppleLanguageId::KIRGHIZ:
            nLang = LANGUAGE_KIRGHIZ;
            break;
        case AppleLanguageId::TAJIKI:
            nLang = LANGUAGE_TAJIK;
            break;
        case AppleLanguageId::TURKMEN:
            nLang = LANGUAGE_TURKMEN;
            break;
        case AppleLanguageId::MONGOLIAN_MONGOLIAN:
            nLang = LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA;
            break;
        case AppleLanguageId::MONGOLIAN_CYRILLIC:
            nLang = LANGUAGE_MONGOLIAN_CYRILLIC_MONGOLIA;
            break;
        case AppleLanguageId::PASHTO:
            nLang = LANGUAGE_PASHTO;
            break;
        case AppleLanguageId::KURDISH:
            nLang = LANGUAGE_USER_KURDISH_TURKEY; //maybe
            break;
        case AppleLanguageId::KASHMIRI:
            nLang = LANGUAGE_KASHMIRI;
            break;
        case AppleLanguageId::SINDHI:
            nLang = LANGUAGE_SINDHI;
            break;
        case AppleLanguageId::TIBETAN:
            nLang = LANGUAGE_TIBETAN;
            break;
        case AppleLanguageId::NEPALI:
            nLang = LANGUAGE_NEPALI;
            break;
        case AppleLanguageId::SANSKRIT:
            nLang = LANGUAGE_SANSKRIT;
            break;
        case AppleLanguageId::MARATHI:
            nLang = LANGUAGE_MARATHI;
            break;
        case AppleLanguageId::BENGALI:
            nLang = LANGUAGE_BENGALI;
            break;
        case AppleLanguageId::ASSAMESE:
            nLang = LANGUAGE_ASSAMESE;
            break;
        case AppleLanguageId::GUJARATI:
            nLang = LANGUAGE_GUJARATI;
            break;
        case AppleLanguageId::PUNJABI:
            nLang = LANGUAGE_PUNJABI;
            break;
        case AppleLanguageId::ORIYA:
            nLang = LANGUAGE_ODIA;
            break;
        case AppleLanguageId::MALAYALAM:
            nLang = LANGUAGE_MALAYALAM;
            break;
        case AppleLanguageId::KANNADA:
            nLang = LANGUAGE_KANNADA;
            break;
        case AppleLanguageId::TAMIL:
            nLang = LANGUAGE_TAMIL;
            break;
        case AppleLanguageId::TELUGU:
            nLang = LANGUAGE_TELUGU;
            break;
        case AppleLanguageId::SINHALESE:
            nLang = LANGUAGE_SINHALESE_SRI_LANKA;
            break;
        case AppleLanguageId::BURMESE:
            nLang = LANGUAGE_BURMESE;
            break;
        case AppleLanguageId::KHMER:
            nLang = LANGUAGE_KHMER;
            break;
        case AppleLanguageId::LAO:
            nLang = LANGUAGE_LAO;
            break;
        case AppleLanguageId::VIETNAMESE:
            nLang = LANGUAGE_VIETNAMESE;
            break;
        case AppleLanguageId::INDONESIAN:
            nLang = LANGUAGE_INDONESIAN;
            break;
        case AppleLanguageId::TAGALONG:
            nLang = LANGUAGE_USER_TAGALOG;
            break;
        case AppleLanguageId::MALAY_LATIN:
            nLang = LANGUAGE_MALAY_MALAYSIA;
            break;
        case AppleLanguageId::MALAY_ARABIC:
            return LanguageTag("ms-Arab");
            break;
        case AppleLanguageId::AMHARIC:
            nLang = LANGUAGE_AMHARIC_ETHIOPIA;
            break;
        case AppleLanguageId::TIGRINYA:
            nLang = LANGUAGE_TIGRIGNA_ETHIOPIA;
            break;
        case AppleLanguageId::GALLA:
            nLang = LANGUAGE_OROMO;
            break;
        case AppleLanguageId::SOMALI:
            nLang = LANGUAGE_SOMALI;
            break;
        case AppleLanguageId::SWAHILI:
            nLang = LANGUAGE_SWAHILI;
            break;
        case AppleLanguageId::KINYARWANDA:
            nLang = LANGUAGE_KINYARWANDA_RWANDA;
            break;
        case AppleLanguageId::RUNDI:
            return LanguageTag("rn");
            break;
        case AppleLanguageId::NYANJA:
            nLang = LANGUAGE_USER_NYANJA;
            break;
        case AppleLanguageId::MALAGASY:
            nLang = LANGUAGE_MALAGASY_PLATEAU;
            break;
        case AppleLanguageId::ESPERANTO:
            nLang = LANGUAGE_USER_ESPERANTO;
            break;
        case AppleLanguageId::WELSH:
            nLang = LANGUAGE_WELSH;
            break;
        case AppleLanguageId::BASQUE:
            nLang = LANGUAGE_BASQUE;
            break;
        case AppleLanguageId::CATALAN:
            nLang = LANGUAGE_CATALAN;
            break;
        case AppleLanguageId::LATIN:
            nLang = LANGUAGE_USER_LATIN;
            break;
        case AppleLanguageId::QUENCHUA:
            nLang = LANGUAGE_QUECHUA_BOLIVIA; //maybe
            break;
        case AppleLanguageId::GUARANI:
            nLang = LANGUAGE_GUARANI_PARAGUAY;
            break;
        case AppleLanguageId::AYMARA:
            return LanguageTag("ay");
            break;
        case AppleLanguageId::TATAR:
            nLang = LANGUAGE_TATAR;
            break;
        case AppleLanguageId::UIGHUR:
            nLang = LANGUAGE_UIGHUR_CHINA;
            break;
        case AppleLanguageId::DZONGKHA:
            nLang = LANGUAGE_DZONGKHA_BHUTAN;
            break;
        case AppleLanguageId::JAVANESE_LATIN:
            return LanguageTag("jv-Latn");
            break;
        case AppleLanguageId::SUNDANESE_LATIN:
            return LanguageTag("su-Latn");
            break;
        case AppleLanguageId::GALICIAN:
            nLang = LANGUAGE_GALICIAN;
            break;
        case AppleLanguageId::AFRIKAANS:
            nLang = LANGUAGE_AFRIKAANS;
            break;
        case AppleLanguageId::BRETON:
            nLang = LANGUAGE_BRETON_FRANCE;
            break;
        case AppleLanguageId::INUKTITUT:
            nLang = LANGUAGE_INUKTITUT_LATIN_CANADA; //probably
            break;
        case AppleLanguageId::SCOTTISH_GAELIC:
            nLang = LANGUAGE_GAELIC_SCOTLAND;
            break;
        case AppleLanguageId::MANX_GAELIC:
            nLang = LANGUAGE_USER_MANX;
            break;
        case AppleLanguageId::IRISH_GAELIC_WITH_DOT_ABOVE:
            return LanguageTag("ga-Latg");
            break;
        case AppleLanguageId::TONGAN:
            return LanguageTag("to");
            break;
        case AppleLanguageId::GREEK_POLYTONIC:
            nLang = LANGUAGE_USER_ANCIENT_GREEK;
            break;
        case AppleLanguageId::GREENLANDIC:
            nLang = LANGUAGE_KALAALLISUT_GREENLAND;
            break;
        case AppleLanguageId::AZERI_LATIN:
            nLang = LANGUAGE_AZERI_LATIN;
            break;
    }

    return LanguageTag(nLang);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
