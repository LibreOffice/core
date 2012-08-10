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

//#define erDEBUG

#ifdef ENABLE_LIBLANGTAG
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
    void presetDataPath( const rtl::OUString& rPath );
private:
    rtl::OString maDataPath;   // path to liblangtag data, "|" if system
    sal_uInt32   mnRef;

    void setupDataPath();
    void setup();
    void teardown();
};

static LiblantagDataRef theDataRef;

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
    if (maDataPath.isEmpty())
        setupDataPath();
    lt_db_initialize();
    // Hold ref eternally.
    mnRef = SAL_MAX_UINT32;
}

void LiblantagDataRef::teardown()
{
    lt_db_finalize();
}

void LiblantagDataRef::presetDataPath( const rtl::OUString& rPath )
{
    if (maDataPath.isEmpty())
    {
        maDataPath = OUStringToOString( rPath, RTL_TEXTENCODING_UTF8);
        lt_db_set_datadir( maDataPath.getStr());
    }
}

void LiblantagDataRef::setupDataPath()
{
    // maDataPath is assumed to be empty here.
    OUString aPath;
    if (!rtl::Bootstrap::get( "BRAND_BASE_DIR", aPath))
        OSL_FAIL( "LiblantagDataRef: can't get BRAND_BASE_DIR");
    else
    {
        // Check if data is in our own installation, else assume system
        // installation.
        aPath += "/share/liblangtag";
        OUString aData( aPath);
        aData += "/language-subtag-registry.xml";
        osl::DirectoryItem aDirItem;
        if (osl::DirectoryItem::get( aData, aDirItem) == osl::DirectoryItem::E_None)
            maDataPath = OUStringToOString( aPath, RTL_TEXTENCODING_UTF8);
    }
    if (maDataPath.isEmpty())
        maDataPath = "|";   // assume system
    else
        lt_db_set_datadir( maDataPath.getStr());
}


// static
void LanguageTag::overrideDataPath( const rtl::OUString& rPath )
{
    theDataRef.presetDataPath( rPath);
}


LanguageTag::LanguageTag( const rtl::OUString & rBcp47LanguageTag, bool bCanonicalize )
    :
        maBcp47( rBcp47LanguageTag),
        mpImplLangtag( NULL),
        mnLangID( LANGUAGE_DONTKNOW),
        meIsValid( DECISION_DONTKNOW),
        meIsIsoLocale( DECISION_DONTKNOW),
        meIsIsoODF( DECISION_DONTKNOW),
        mbInitializedBcp47( true),
        mbInitializedLocale( false),
        mbInitializedLangID( false),
        mbCachedLanguage( false),
        mbCachedScript( false),
        mbCachedCountry( false)
{
    theDataRef.incRef();

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
        mbInitializedBcp47( false),
        mbInitializedLocale( true),
        mbInitializedLangID( false),
        mbCachedLanguage( false),
        mbCachedScript( false),
        mbCachedCountry( false)
{
    theDataRef.incRef();
}


LanguageTag::LanguageTag( LanguageType nLanguage )
    :
        mpImplLangtag( NULL),
        mnLangID( nLanguage),
        meIsValid( DECISION_DONTKNOW),
        meIsIsoLocale( DECISION_DONTKNOW),
        meIsIsoODF( DECISION_DONTKNOW),
        mbInitializedBcp47( false),
        mbInitializedLocale( false),
        mbInitializedLangID( true),
        mbCachedLanguage( false),
        mbCachedScript( false),
        mbCachedCountry( false)
{
    theDataRef.incRef();
}


LanguageTag::LanguageTag( const rtl::OUString& rLanguage, const rtl::OUString& rCountry )
    :
        maLocale( rLanguage, rCountry, ""),
        mpImplLangtag( NULL),
        mnLangID( LANGUAGE_DONTKNOW),
        meIsValid( DECISION_DONTKNOW),
        meIsIsoLocale( DECISION_DONTKNOW),
        meIsIsoODF( DECISION_DONTKNOW),
        mbInitializedBcp47( false),
        mbInitializedLocale( true),
        mbInitializedLangID( false),
        mbCachedLanguage( false),
        mbCachedScript( false),
        mbCachedCountry( false)
{
    theDataRef.incRef();
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
        mbInitializedBcp47( rLanguageTag.mbInitializedBcp47),
        mbInitializedLocale( rLanguageTag.mbInitializedLocale),
        mbInitializedLangID( rLanguageTag.mbInitializedLangID),
        mbCachedLanguage( rLanguageTag.mbCachedLanguage),
        mbCachedScript( rLanguageTag.mbCachedScript),
        mbCachedCountry( rLanguageTag.mbCachedCountry)
{
    theDataRef.incRef();
}


LanguageTag& LanguageTag::operator=( const LanguageTag & rLanguageTag )
{
    theDataRef.incRef();

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
    mbInitializedBcp47  = rLanguageTag.mbInitializedBcp47;
    mbInitializedLocale = rLanguageTag.mbInitializedLocale;
    mbInitializedLangID = rLanguageTag.mbInitializedLangID;
    mbCachedLanguage    = rLanguageTag.mbCachedLanguage;
    mbCachedScript      = rLanguageTag.mbCachedScript;
    mbCachedCountry     = rLanguageTag.mbCachedCountry;
    return *this;
}


LanguageTag::~LanguageTag()
{
    lt_tag_unref( MPLANGTAG);

    theDataRef.decRef();
}


bool LanguageTag::canonicalize() const
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

    // g_error_free() mocks about NULL, so ...
    struct myerror
    {
        GError* p;
        myerror() : p(NULL) {}
        ~myerror() { if (p) g_error_free( p); }
    } aError;

    getBcp47();     // side effect: have maBcp47 in any case
    // Checking empty for system locale before having allocated mpImplLangtag
    // may result in multiple calls of this method because that serves as flag
    // whether this was canonicalized, but that's better than allocating
    // lt_tag_t for all those system locales.
    if (maBcp47.isEmpty())
    {
        meIsValid = DECISION_YES;
        return true;
    }
    if (!mpImplLangtag)
        mpImplLangtag = lt_tag_new();
    if (lt_tag_parse( MPLANGTAG, OUStringToOString( maBcp47, RTL_TEXTENCODING_UTF8).getStr(), &aError.p))
    {
        gchar* pTag = lt_tag_canonicalize( MPLANGTAG, &aError.p);
        SAL_WARN_IF( !pTag || aError.p, "i18npool.langtag", "LanguageTag::canonicalize: could not canonicalize, " <<
                (aError.p ? aError.p->message : ""));
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
                    SAL_WARN( "i18npool.langtag", "LanguageTag::canonicalize: could not reparse, " <<
                            (aError.p ? aError.p->message : ""));
                    g_free( pTag);
                    meIsValid = DECISION_NO;
                    return false;
                }
            }
            g_free( pTag);
            meIsValid = DECISION_YES;
            return true;
        }
    }
    else
    {
        SAL_WARN( "i18npool.langtag", "LanguageTag::canonicalize: could not parse, " <<
                (aError.p ? aError.p->message : ""));
    }
    meIsValid = DECISION_NO;
    return false;
}


void LanguageTag::convertLocaleToBcp47()
{
    if (maLocale.Language.isEmpty())
    {
        // Special case system locale.
        maBcp47 = OUString();
        meIsIsoLocale = DECISION_YES;
    }
    else if (maLocale.Language == ISO639_LANGUAGE_TAG)
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
    /* FIXME: this is temporary until code base is converted to not use
     * MsLangId::convert...() anymore. After that, proper new method has to be
     * implemented to allow ISO639_LANGUAGE_TAG and sript tag and such. */
    mnLangID = MsLangId::convertLocaleToLanguage( maLocale);
    mbInitializedLangID = true;
}


void LanguageTag::convertBcp47ToLocale()
{
    if (maBcp47.isEmpty())
    {
        // Special case system locale.
        maLocale = lang::Locale();
        meIsIsoLocale = DECISION_YES;
    }
    else
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
    }
    mbInitializedLocale = true;
}


void LanguageTag::convertBcp47ToLang()
{
    /* FIXME: this is temporary. If we support locales that consist not only of
     * language and country, e.g. added script, this probably needs to be
     * adapted. */
    if (!mbInitializedLocale)
        convertBcp47ToLocale();
    convertLocaleToLang();
    mbInitializedLangID = true;
}


void LanguageTag::convertLangToLocale()
{
    /* FIXME: this is temporary until code base is converted to not use
     * MsLangId::convert...() anymore. After that, proper new method has to be
     * implemented to allow ISO639_LANGUAGE_TAG and script tag and such. */
    // Do not resolve system here!
    maLocale = MsLangId::convertLanguageToLocale( mnLangID, false);
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


rtl::OUString LanguageTag::getBcp47() const
{
    if (!mbInitializedBcp47)
    {
        if (mbInitializedLocale)
            const_cast<LanguageTag*>(this)->convertLocaleToBcp47();
        else
            const_cast<LanguageTag*>(this)->convertLangToBcp47();
    }
    return maBcp47;
}


rtl::OUString LanguageTag::getLanguageFromLangtag() const
{
    rtl::OUString aLanguage;
    if (!mpImplLangtag)
        canonicalize();
    if (maBcp47.isEmpty())
        return aLanguage;
    const lt_lang_t* pLangT = lt_tag_get_language( MPLANGTAG);
    SAL_WARN_IF( !pLangT, "i18npool.langtag", "LanguageTag::getLanguageFromLangtag: pLangT==NULL");
    if (!pLangT)
        return aLanguage;
    const gchar* pLang = lt_lang_get_tag( pLangT);
    SAL_WARN_IF( !pLang, "i18npool.langtag", "LanguageTag::getLanguageFromLangtag: pLang==NULL");
    if (pLang)
        aLanguage = OUString::createFromAscii( pLang);
    return aLanguage;
}


rtl::OUString LanguageTag::getScriptFromLangtag() const
{
    rtl::OUString aScript;
    if (!mpImplLangtag)
        canonicalize();
    if (maBcp47.isEmpty())
        return aScript;
    const lt_script_t* pScriptT = lt_tag_get_script( MPLANGTAG);
    // pScriptT==NULL is valid for default scripts
    if (!pScriptT)
        return aScript;
    const gchar* pScript = lt_script_get_tag( pScriptT);
    SAL_WARN_IF( !pScript, "i18npool.langtag", "LanguageTag::getScriptFromLangtag: pScript==NULL");
    if (pScript)
        aScript = OUString::createFromAscii( pScript);
    return aScript;
}


rtl::OUString LanguageTag::getRegionFromLangtag() const
{
    rtl::OUString aRegion;
    if (!mpImplLangtag)
        canonicalize();
    if (maBcp47.isEmpty())
        return aRegion;
    const lt_region_t* pRegionT = lt_tag_get_region( MPLANGTAG);
    SAL_WARN_IF( !pRegionT, "i18npool.langtag", "LanguageTag::getRegionFromLangtag: pRegionT==NULL");
    if (!pRegionT)
        return aRegion;
    const gchar* pRegion = lt_region_get_tag( pRegionT);
    SAL_WARN_IF( !pRegion, "i18npool.langtag", "LanguageTag::getRegionFromLangtag: pRegion==NULL");
    if (pRegion)
        aRegion = OUString::createFromAscii( pRegion);
    return aRegion;
}


com::sun::star::lang::Locale LanguageTag::getLocale() const
{
    if (!mbInitializedLocale)
    {
        if (mbInitializedBcp47)
            const_cast<LanguageTag*>(this)->convertBcp47ToLocale();
        else
            const_cast<LanguageTag*>(this)->convertLangToLocale();
    }
    return maLocale;
}


LanguageType LanguageTag::getLanguageType() const
{
    if (!mbInitializedLangID)
    {
        if (mbInitializedBcp47)
            const_cast<LanguageTag*>(this)->convertBcp47ToLang();
        else
            const_cast<LanguageTag*>(this)->convertLocaleToLang();
    }
    return mnLangID;
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
            "LanguageTag::isIsoLanguage: rejecting upper case");
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
            "i18npool.langtag", "LanguageTag::isIsoCountry: rejecting lower case");
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
            "i18npool.langtag", "LanguageTag::isIsoScript: rejecting case mismatch");
    return false;
}


rtl::OUString LanguageTag::getLanguage() const
{
    if (!mbCachedLanguage)
    {
        maCachedLanguage = getLanguageFromLangtag();
        mbCachedLanguage = true;
    }
    return maCachedLanguage;
}


rtl::OUString LanguageTag::getScript() const
{
    if (!mbCachedScript)
    {
        maCachedScript = getScriptFromLangtag();
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
        maCachedCountry = getRegionFromLangtag();
        if (!isIsoCountry( maCachedCountry))
            maCachedCountry = OUString();
        mbCachedCountry = true;
    }
    return maCachedCountry;
}


rtl::OUString LanguageTag::getRegion() const
{
    return getRegionFromLangtag();
}


bool LanguageTag::isIsoLocale() const
{
    if (meIsIsoLocale == DECISION_DONTKNOW)
    {
        if (!mpImplLangtag)
            canonicalize();
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
        if (!mpImplLangtag)
            canonicalize();
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
        if (!mpImplLangtag)
           canonicalize();
        SAL_WARN_IF( meIsValid == DECISION_DONTKNOW, "i18npool.langtag",
                "LanguageTag::isValidBcp47: canonicalize() doesn't set meIsValid");
    }
    return meIsValid == DECISION_YES;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
