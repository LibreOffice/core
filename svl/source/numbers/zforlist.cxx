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

#include <tools/debug.hxx>
#include <unotools/charclass.hxx>
#include <i18npool/mslangid.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/numberformatcodewrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <com/sun/star/i18n/KNumberFormatUsage.hpp>
#include <com/sun/star/i18n/KNumberFormatType.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/misccfg.hxx>


#define _ZFORLIST_CXX
#include <osl/mutex.hxx>
#include <svl/zforlist.hxx>
#undef _ZFORLIST_CXX

#include "zforscan.hxx"
#include "zforfind.hxx"
#include <svl/zformat.hxx>
#include "numhead.hxx"

#include <unotools/syslocaleoptions.hxx>
#include <unotools/digitgroupingiterator.hxx>
#include <rtl/logfile.hxx>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>

#include <math.h>
#include <limits>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::std;

using ::rtl::OUString;


// Constants for type offsets per Country/Language (CL)
#define ZF_STANDARD              0
#define ZF_STANDARD_PERCENT     10
#define ZF_STANDARD_CURRENCY    20
#define ZF_STANDARD_DATE        30
#define ZF_STANDARD_TIME        40
#define ZF_STANDARD_DATETIME    50
#define ZF_STANDARD_SCIENTIFIC  60
#define ZF_STANDARD_FRACTION    70
#define ZF_STANDARD_NEWEXTENDED 75
#define ZF_STANDARD_NEWEXTENDEDMAX  SV_MAX_ANZ_STANDARD_FORMATE-2   // 98
#define ZF_STANDARD_LOGICAL     SV_MAX_ANZ_STANDARD_FORMATE-1 //  99
#define ZF_STANDARD_TEXT        SV_MAX_ANZ_STANDARD_FORMATE   // 100

/* Locale that is set if an unknown locale (from another system) is loaded of
 * legacy documents. Can not be SYSTEM because else, for example, a German "DM"
 * (old currency) is recognized as a date (#53155#). */
#define UNKNOWN_SUBSTITUTE      LANGUAGE_ENGLISH_US

static bool bIndexTableInitialized = false;
static sal_uInt32 theIndexTable[NF_INDEX_TABLE_ENTRIES];


// ====================================================================

/**
    instead of every number formatter being a listener we have a registry which
    also handles one instance of the SysLocale options
 */

typedef ::std::vector< SvNumberFormatter* > SvNumberFormatterList_impl;

class SvNumberFormatterRegistry_Impl : public utl::ConfigurationListener
{
    SvNumberFormatterList_impl  aFormatters;
    SvtSysLocaleOptions         aSysLocaleOptions;
    LanguageType                eSysLanguage;

public:
                            SvNumberFormatterRegistry_Impl();
    virtual                 ~SvNumberFormatterRegistry_Impl();

    void                    Insert( SvNumberFormatter* pThis )
                            { aFormatters.push_back( pThis ); }

    SvNumberFormatter*      Remove( SvNumberFormatter* pThis );

    size_t                  Count()
                            { return aFormatters.size(); }

    virtual void            ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );
};

SvNumberFormatterRegistry_Impl::SvNumberFormatterRegistry_Impl()
{
    eSysLanguage = MsLangId::getRealLanguage( LANGUAGE_SYSTEM );
    aSysLocaleOptions.AddListener( this );
}


SvNumberFormatterRegistry_Impl::~SvNumberFormatterRegistry_Impl()
{
    aSysLocaleOptions.RemoveListener( this );
}


SvNumberFormatter* SvNumberFormatterRegistry_Impl::Remove( SvNumberFormatter* pThis )
{
    for (SvNumberFormatterList_impl::iterator it = aFormatters.begin();
            it != aFormatters.end(); ++it)
    {
        if ( *it == pThis )
        {
            aFormatters.erase( it );
            break;
        }
    }
    return pThis;
}

void SvNumberFormatterRegistry_Impl::ConfigurationChanged(
    utl::ConfigurationBroadcaster*,
    sal_uInt32 nHint)
{
        if ( nHint & SYSLOCALEOPTIONS_HINT_LOCALE )
        {
            ::osl::MutexGuard aGuard( SvNumberFormatter::GetMutex() );
            for( size_t i = 0, n = aFormatters.size(); i < n; ++i )
                aFormatters[ i ]->ReplaceSystemCL( eSysLanguage );
            eSysLanguage = MsLangId::getRealLanguage( LANGUAGE_SYSTEM );
        }
        if ( nHint & SYSLOCALEOPTIONS_HINT_CURRENCY )
        {
            ::osl::MutexGuard aGuard( SvNumberFormatter::GetMutex() );
            for( size_t i = 0, n = aFormatters.size(); i < n; ++i )
                aFormatters[ i ]->ResetDefaultSystemCurrency();
        }
}


// ====================================================================

SvNumberFormatterRegistry_Impl* SvNumberFormatter::pFormatterRegistry = NULL;
bool SvNumberFormatter::bCurrencyTableInitialized = false;
namespace
{
    struct theCurrencyTable :
        public rtl::Static< NfCurrencyTable, theCurrencyTable > {};

    struct theLegacyOnlyCurrencyTable :
        public rtl::Static< NfCurrencyTable, theLegacyOnlyCurrencyTable > {};

    /** THE set of installed locales. */
    struct theInstalledLocales :
        public rtl::Static< NfInstalledLocales, theInstalledLocales> {};

}
sal_uInt16 SvNumberFormatter::nSystemCurrencyPosition = 0;

// Whether BankSymbol (not CurrencySymbol!) is always at the end (1 $;-1 $) or
// language dependent.
#define NF_BANKSYMBOL_FIX_POSITION 1


/***********************Funktionen SvNumberFormatter**************************/

const sal_uInt16 SvNumberFormatter::UNLIMITED_PRECISION   = ::std::numeric_limits<sal_uInt16>::max();
const sal_uInt16 SvNumberFormatter::INPUTSTRING_PRECISION = ::std::numeric_limits<sal_uInt16>::max()-1;

SvNumberFormatter::SvNumberFormatter(
            const Reference< XMultiServiceFactory >& xSMgr,
            LanguageType eLang )
        :
        xServiceManager( xSMgr )
{
    ImpConstruct( eLang );
}


SvNumberFormatter::~SvNumberFormatter()
{
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        pFormatterRegistry->Remove( this );
        if ( !pFormatterRegistry->Count() )
        {
            delete pFormatterRegistry;
            pFormatterRegistry = NULL;
        }
    }

    for (SvNumberFormatTable::iterator it = aFTable.begin(); it != aFTable.end(); ++it)
        delete it->second;
    delete pFormatTable;
    delete pCharClass;
    delete pStringScanner;
    delete pFormatScanner;
    ClearMergeTable();
    delete pMergeTable;
}


void SvNumberFormatter::ImpConstruct( LanguageType eLang )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aTimeLog, "svl", "er93726", "SvNumberFormatter::ImpConstruct" );

    if ( eLang == LANGUAGE_DONTKNOW )
        eLang = UNKNOWN_SUBSTITUTE;
    IniLnge = eLang;
    ActLnge = eLang;
    eEvalDateFormat = NF_EVALDATEFORMAT_INTL;
    nDefaultSystemCurrencyFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;

    aLocale = MsLangId::convertLanguageToLocale( eLang );
    pCharClass = new CharClass( xServiceManager, aLocale );
    xLocaleData.init( xServiceManager, aLocale, eLang );
    xCalendar.init( xServiceManager, aLocale );
    xTransliteration.init( xServiceManager, eLang,
        ::com::sun::star::i18n::TransliterationModules_IGNORE_CASE );
    xNatNum.init( xServiceManager );

    // cached locale data items
    const LocaleDataWrapper* pLoc = GetLocaleData();
    aDecimalSep = pLoc->getNumDecimalSep();
    aThousandSep = pLoc->getNumThousandSep();
    aDateSep = pLoc->getDateSep();

    pStringScanner = new ImpSvNumberInputScan( this );
    pFormatScanner = new ImpSvNumberformatScan( this );
    pFormatTable = NULL;
    MaxCLOffset = 0;
    ImpGenerateFormats( 0, false );     // 0 .. 999 for initialized language formats
    pMergeTable = NULL;
    bNoZero = false;

    ::osl::MutexGuard aGuard( GetMutex() );
    GetFormatterRegistry().Insert( this );
}


void SvNumberFormatter::ChangeIntl(LanguageType eLnge)
{
    if (ActLnge != eLnge)
    {
        ActLnge = eLnge;

        aLocale = MsLangId::convertLanguageToLocale( eLnge );
        pCharClass->setLocale( aLocale );
        xLocaleData.changeLocale( aLocale, eLnge );
        xCalendar.changeLocale( aLocale );
        xTransliteration.changeLocale( eLnge );

        // cached locale data items, initialize BEFORE calling ChangeIntl below
        const LocaleDataWrapper* pLoc = GetLocaleData();
        aDecimalSep = pLoc->getNumDecimalSep();
        aThousandSep = pLoc->getNumThousandSep();
        aDateSep = pLoc->getDateSep();

        pFormatScanner->ChangeIntl();
        pStringScanner->ChangeIntl();
    }
}


// static
::osl::Mutex& SvNumberFormatter::GetMutex()
{
    static ::osl::Mutex* pMutex = NULL;
    if( !pMutex )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pMutex )
        {
            // #i77768# Due to a static reference in the toolkit lib
            // we need a mutex that lives longer than the svl library.
            // Otherwise the dtor would use a destructed mutex!!
            pMutex = new ::osl::Mutex;
        }
    }
    return *pMutex;
}


// static
SvNumberFormatterRegistry_Impl& SvNumberFormatter::GetFormatterRegistry()
{
    ::osl::MutexGuard aGuard( GetMutex() );
    if ( !pFormatterRegistry )
        pFormatterRegistry = new SvNumberFormatterRegistry_Impl;
    return *pFormatterRegistry;
}


Color* SvNumberFormatter::GetUserDefColor(sal_uInt16 nIndex)
{
    if( aColorLink.IsSet() )
        return (Color*) ( aColorLink.Call( (void*) &nIndex ));
    else
        return NULL;
}

void SvNumberFormatter::ChangeNullDate(sal_uInt16 nDay,
                                       sal_uInt16 nMonth,
                                       sal_uInt16 nYear)
{
    pFormatScanner->ChangeNullDate(nDay, nMonth, nYear);
    pStringScanner->ChangeNullDate(nDay, nMonth, nYear);
}

Date* SvNumberFormatter::GetNullDate()
{
    return pFormatScanner->GetNullDate();
}

void SvNumberFormatter::ChangeStandardPrec(short nPrec)
{
    pFormatScanner->ChangeStandardPrec(nPrec);
}

sal_uInt16 SvNumberFormatter::GetStandardPrec()
{
    return pFormatScanner->GetStandardPrec();
}

void SvNumberFormatter::ImpChangeSysCL( LanguageType eLnge, bool bNoAdditionalFormats )
{
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = UNKNOWN_SUBSTITUTE;
    if (eLnge != IniLnge)
    {
        IniLnge = eLnge;
        ChangeIntl(eLnge);
        // delete old formats
        for (SvNumberFormatTable::iterator it = aFTable.begin(); it != aFTable.end(); ++it)
            delete it->second;
        aFTable.clear();
        ImpGenerateFormats( 0, bNoAdditionalFormats );   // new standard formats
    }
    else if ( bNoAdditionalFormats )
    {   // delete additional standard formats
        sal_uInt32 nKey;
        SvNumberFormatTable::iterator it = aFTable.find( SV_MAX_ANZ_STANDARD_FORMATE + 1 );
        while ( it != aFTable.end() && (nKey = it->first) > SV_MAX_ANZ_STANDARD_FORMATE &&
                nKey < SV_COUNTRY_LANGUAGE_OFFSET )
        {
            delete it->second;
            aFTable.erase( it++ );
        }
    }
}


void SvNumberFormatter::ReplaceSystemCL( LanguageType eOldLanguage )
{
    sal_uInt32 nCLOffset = ImpGetCLOffset( LANGUAGE_SYSTEM );
    if ( nCLOffset > MaxCLOffset )
        return ;    // no SYSTEM entries to replace

    const sal_uInt32 nMaxBuiltin = nCLOffset + SV_MAX_ANZ_STANDARD_FORMATE;
    const sal_uInt32 nNextCL = nCLOffset + SV_COUNTRY_LANGUAGE_OFFSET;
    sal_uInt32 nKey;

    // remove old builtin formats
    SvNumberFormatTable::iterator it = aFTable.find( nCLOffset );
    while ( it != aFTable.end() && (nKey = it->first) >= nCLOffset && nKey <= nMaxBuiltin )
    {
        delete it->second;
        aFTable.erase( it++ );
    }

    // move additional and user defined to temporary table
    SvNumberFormatTable aOldTable;
    while ( it != aFTable.end() && (nKey = it->first) >= nCLOffset && nKey < nNextCL )
    {
        aOldTable[ nKey ] = it->second;
        aFTable.erase( it++ );
    }

    // generate new old builtin formats
    // reset ActLnge otherwise ChangeIntl() wouldn't switch if already LANGUAGE_SYSTEM
    ActLnge = LANGUAGE_DONTKNOW;
    ChangeIntl( LANGUAGE_SYSTEM );
    ImpGenerateFormats( nCLOffset, true );

    // convert additional and user defined from old system to new system
    SvNumberformat* pStdFormat = GetFormatEntry( nCLOffset + ZF_STANDARD );
    sal_uInt32 nLastKey = nMaxBuiltin;
    pFormatScanner->SetConvertMode( eOldLanguage, LANGUAGE_SYSTEM, true );
    while ( !aOldTable.empty() )
    {
        nKey = aOldTable.begin()->first;
        if ( nLastKey < nKey )
            nLastKey = nKey;
        SvNumberformat* pOldEntry = aOldTable.begin()->second;
        aOldTable.erase( nKey );
        String aString( pOldEntry->GetFormatstring() );
        xub_StrLen nCheckPos = STRING_NOTFOUND;

        // Same as PutEntry() but assures key position even if format code is
        // a duplicate. Also won't mix up any LastInsertKey.
        ChangeIntl( eOldLanguage );
        LanguageType eLge = eOldLanguage;   // ConvertMode changes this
        bool bCheck = false;
        SvNumberformat* pNewEntry = new SvNumberformat( aString, pFormatScanner,
            pStringScanner, nCheckPos, eLge );
        if ( nCheckPos != 0 )
            delete pNewEntry;
        else
        {
            short eCheckType = pNewEntry->GetType();
            if ( eCheckType != NUMBERFORMAT_UNDEFINED )
                pNewEntry->SetType( eCheckType | NUMBERFORMAT_DEFINED );
            else
                pNewEntry->SetType( NUMBERFORMAT_DEFINED );

            if ( !aFTable.insert( make_pair( nKey, pNewEntry) ).second )
                delete pNewEntry;
            else
                bCheck = true;
        }
        DBG_ASSERT( bCheck, "SvNumberFormatter::ReplaceSystemCL: couldn't convert" );
        (void)bCheck;

        delete pOldEntry;
    }
    pFormatScanner->SetConvertMode(false);
    pStdFormat->SetLastInsertKey( sal_uInt16(nLastKey - nCLOffset) );

    // append new system additional formats
    NumberFormatCodeWrapper aNumberFormatCode( xServiceManager, GetLocale() );
    ImpGenerateAdditionalFormats( nCLOffset, aNumberFormatCode, true );
}


bool SvNumberFormatter::IsTextFormat(sal_uInt32 F_Index) const
{
    const SvNumberformat* pFormat = GetFormatEntry(F_Index);
    if (!pFormat)
        return false;
    else
        return pFormat->IsTextFormat();
}

bool SvNumberFormatter::PutEntry(String& rString,
                                 xub_StrLen& nCheckPos,
                                 short& nType,
                                 sal_uInt32& nKey,      // format key
                                 LanguageType eLnge)
{
    nKey = 0;
    if (rString.Len() == 0)                             // empty string
    {
        nCheckPos = 1;                                  // -> Error
        return false;
    }
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;

    ChangeIntl(eLnge);                                  // change locale if necessary
    LanguageType eLge = eLnge;                          // non-const for ConvertMode
    bool bCheck = false;
    SvNumberformat* p_Entry = new SvNumberformat(rString,
                                                 pFormatScanner,
                                                 pStringScanner,
                                                 nCheckPos,
                                                 eLge);
    if (nCheckPos == 0)                         // Format ok
    {                                           // Type comparison:
        short eCheckType = p_Entry->GetType();
        if ( eCheckType != NUMBERFORMAT_UNDEFINED)
        {
            p_Entry->SetType(eCheckType | NUMBERFORMAT_DEFINED);
            nType = eCheckType;
        }
        else
        {
            p_Entry->SetType(NUMBERFORMAT_DEFINED);
            nType = NUMBERFORMAT_DEFINED;
        }
        sal_uInt32 CLOffset = ImpGenerateCL(eLge);  // create new standard formats if necessary

        nKey = ImpIsEntry(p_Entry->GetFormatstring(),CLOffset, eLge);
        if (nKey != NUMBERFORMAT_ENTRY_NOT_FOUND)   // already present
            delete p_Entry;
        else
        {
            SvNumberformat* pStdFormat =
                     GetFormatEntry(CLOffset + ZF_STANDARD);
            sal_uInt32 nPos = CLOffset + pStdFormat->GetLastInsertKey();
            if (nPos - CLOffset >= SV_COUNTRY_LANGUAGE_OFFSET)
            {
                OSL_FAIL("SvNumberFormatter:: Zu viele Formate pro CL");
                delete p_Entry;
            }
            else if (!aFTable.insert(make_pair( nPos+1,p_Entry)).second)
                delete p_Entry;
            else
            {
                bCheck = true;
                nKey = nPos+1;
                pStdFormat->SetLastInsertKey((sal_uInt16) (nKey-CLOffset));
            }
        }
    }
    else
        delete p_Entry;
    return bCheck;
}

bool SvNumberFormatter::PutEntry(
    OUString& rString, xub_StrLen& nCheckPos, short& nType, sal_uInt32& nKey,
    LanguageType eLnge)
{
    // Wrapper to allow rtl::OUString to be used.
    String aStr(rString);
    bool bRet = PutEntry(aStr, nCheckPos, nType, nKey, eLnge);
    rString = aStr;
    return bRet;
}

bool SvNumberFormatter::PutandConvertEntry(String& rString,
                                           xub_StrLen& nCheckPos,
                                           short& nType,
                                           sal_uInt32& nKey,
                                           LanguageType eLnge,
                                           LanguageType eNewLnge)
{
    bool bRes;
    if (eNewLnge == LANGUAGE_DONTKNOW)
        eNewLnge = IniLnge;

    pFormatScanner->SetConvertMode(eLnge, eNewLnge);
    bRes = PutEntry(rString, nCheckPos, nType, nKey, eLnge);
    pFormatScanner->SetConvertMode(false);
    return bRes;
}

bool SvNumberFormatter::PutandConvertEntry(rtl::OUString& rString,
                                           xub_StrLen& nCheckPos,
                                           short& nType,
                                           sal_uInt32& nKey,
                                           LanguageType eLnge,
                                           LanguageType eNewLnge)
{
    String aStr;
    bool bRet = PutandConvertEntry(aStr, nCheckPos, nType, nKey, eLnge, eNewLnge);
    rString = aStr;
    return bRet;
}

bool SvNumberFormatter::PutandConvertEntrySystem(String& rString,
                                           xub_StrLen& nCheckPos,
                                           short& nType,
                                           sal_uInt32& nKey,
                                           LanguageType eLnge,
                                           LanguageType eNewLnge)
{
    bool bRes;
    if (eNewLnge == LANGUAGE_DONTKNOW)
        eNewLnge = IniLnge;

    pFormatScanner->SetConvertMode(eLnge, eNewLnge, true);
    bRes = PutEntry(rString, nCheckPos, nType, nKey, eLnge);
    pFormatScanner->SetConvertMode(false);
    return bRes;
}


sal_uInt32 SvNumberFormatter::GetIndexPuttingAndConverting( String & rString,
        LanguageType eLnge, LanguageType eSysLnge, short & rType,
        bool & rNewInserted, xub_StrLen & rCheckPos )
{
    sal_uInt32 nKey = NUMBERFORMAT_ENTRY_NOT_FOUND;
    rNewInserted = false;
    rCheckPos = 0;

    // #62389# empty format string (of Writer) => General standard format
    if (!rString.Len())
        ;   // nothing
    else if (eLnge == LANGUAGE_SYSTEM && eSysLnge != SvtSysLocale().GetLanguage())
    {
        sal_uInt32 nOrig = GetEntryKey( rString, eSysLnge );
        if (nOrig == NUMBERFORMAT_ENTRY_NOT_FOUND)
            nKey = nOrig;   // none avaliable, maybe user-defined
        else
            nKey = GetFormatForLanguageIfBuiltIn( nOrig, SvtSysLocale().GetLanguage() );

        if (nKey == nOrig)
        {
            // Not a builtin format, convert.
            // The format code string may get modified and adapted to the real
            // language and wouldn't match eSysLnge anymore, do that on a copy.
            String aTmp( rString);
            rNewInserted = PutandConvertEntrySystem( aTmp, rCheckPos, rType,
                    nKey, eLnge, SvtSysLocale().GetLanguage());
            if (rCheckPos > 0)
            {
                SAL_WARN( "svl.numbers", "SvNumberFormatter::GetIndexPuttingAndConverting: bad format code string for current locale");
                nKey = NUMBERFORMAT_ENTRY_NOT_FOUND;
            }
        }
    }
    else
    {
        nKey = GetEntryKey( rString, eLnge);
        if (nKey == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            rNewInserted = PutEntry( rString, rCheckPos, rType, nKey, eLnge);
            if (rCheckPos > 0)
            {
                SAL_WARN( "svl.numbers", "SvNumberFormatter::GetIndexPuttingAndConverting: bad format code string for specified locale");
                nKey = NUMBERFORMAT_ENTRY_NOT_FOUND;
            }
        }
    }
    if (nKey == NUMBERFORMAT_ENTRY_NOT_FOUND)
        nKey = GetStandardIndex( eLnge);
    rType = GetType( nKey);
    // Convert any (!) old "automatic" currency format to new fixed currency
    // default format.
    if ((rType & NUMBERFORMAT_CURRENCY) != 0)
    {
        const SvNumberformat* pFormat = GetEntry( nKey);
        if (!pFormat->HasNewCurrency())
        {
            if (rNewInserted)
            {
                DeleteEntry( nKey);     // don't leave trails of rubbish
                rNewInserted = false;
            }
            nKey = GetStandardFormat( NUMBERFORMAT_CURRENCY, eLnge);
        }
    }
    return nKey;
}


void SvNumberFormatter::DeleteEntry(sal_uInt32 nKey)
{
    delete aFTable[nKey];
    aFTable.erase(nKey);
}

bool SvNumberFormatter::Load( SvStream& rStream )
{
    LanguageType eSysLang = SvtSysLocale().GetLanguage();
    SvNumberFormatter* pConverter = NULL;

    ImpSvNumMultipleReadHeader aHdr( rStream );
    sal_uInt16 nVersion;
    rStream >> nVersion;
    SvNumberformat* pEntry;
    sal_uInt32 nPos;
    sal_uInt16 nSysOnStore, eLge, eDummy;       // Dummy for compatible format
    rStream >> nSysOnStore >> eLge;             // system language from document

    SAL_WARN_IF( nVersion < SV_NUMBERFORMATTER_VERSION_CALENDAR, "svl.numbers", "SvNumberFormatter::Load: where does this unsupported old data come from?!?");

    LanguageType eSaveSysLang = (LanguageType) nSysOnStore;
    LanguageType eLnge = (LanguageType) eLge;
    ImpChangeSysCL( eLnge, true );

    rStream >> nPos;
    while (nPos != NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        rStream >> eDummy >> eLge;
        eLnge = (LanguageType) eLge;
        ImpGenerateCL( eLnge, true );           // create new standard formats if necessary

        sal_uInt32 nOffset = nPos % SV_COUNTRY_LANGUAGE_OFFSET;     // relativIndex
        bool bUserDefined = (nOffset > SV_MAX_ANZ_STANDARD_FORMATE);

        pEntry = new SvNumberformat(*pFormatScanner, eLnge);
        pEntry->Load( rStream, aHdr, NULL, *pStringScanner );
        if ( !bUserDefined )
            bUserDefined = (pEntry->GetNewStandardDefined() > SV_NUMBERFORMATTER_VERSION);
        if ( bUserDefined )
        {
            LanguageType eLoadSysLang = (eLnge == LANGUAGE_SYSTEM ? eSysLang : eSaveSysLang);
            if ( eSaveSysLang != eLoadSysLang )
            {   // different SYSTEM locale
                if ( !pConverter )
                    pConverter = new SvNumberFormatter( xServiceManager, eSysLang );
                pEntry->ConvertLanguage( *pConverter,
                        eSaveSysLang, eLoadSysLang, true );
            }
        }
        if ( nOffset == 0 )     // Standard/General format
        {
            SvNumberformat* pEnt = GetFormatEntry(nPos);
            if (pEnt)
                pEnt->SetLastInsertKey(pEntry->GetLastInsertKey());
        }
        if (!aFTable.insert(make_pair( nPos, pEntry)).second)
            delete pEntry;
        rStream >> nPos;
    }

    // as of SV_NUMBERFORMATTER_VERSION_YEAR2000
    if ( nVersion >= SV_NUMBERFORMATTER_VERSION_YEAR2000 )
    {
        aHdr.StartEntry();
        if ( aHdr.BytesLeft() >= sizeof(sal_uInt16) )
        {
            sal_uInt16 nY2k;
            rStream >> nY2k;
            if ( nVersion < SV_NUMBERFORMATTER_VERSION_TWODIGITYEAR && nY2k < 100 )
                nY2k += 1901;       // was before src513e: 29, now: 1930
            SetYear2000( nY2k );
        }
        aHdr.EndEntry();
    }

    if ( pConverter )
        delete pConverter;

    // generate additional i18n standard formats for all used locales
    LanguageType eOldLanguage = ActLnge;
    NumberFormatCodeWrapper aNumberFormatCode( xServiceManager, GetLocale() );
    std::vector<sal_uInt16> aList;
    GetUsedLanguages( aList );
    for ( std::vector<sal_uInt16>::const_iterator it(aList.begin()); it != aList.end(); ++it )
    {
        LanguageType eLang = *it;
        ChangeIntl( eLang );
        sal_uInt32 CLOffset = ImpGetCLOffset( eLang );
        ImpGenerateAdditionalFormats( CLOffset, aNumberFormatCode, true );
    }
    ChangeIntl( eOldLanguage );

    if (rStream.GetError())
        return false;
    else
        return true;
}

bool SvNumberFormatter::Save( SvStream& rStream ) const
{
    ImpSvNumMultipleWriteHeader aHdr( rStream );
    // As of 364i we store what SYSTEM locale really was, before it was hard
    // coded LANGUAGE_SYSTEM.
    rStream << (sal_uInt16) SV_NUMBERFORMATTER_VERSION;
    rStream << (sal_uInt16) SvtSysLocale().GetLanguage() << (sal_uInt16) IniLnge;
    const SvNumberFormatTable* pTable = &aFTable;
    SvNumberFormatTable::const_iterator it = pTable->begin();
    while (it != pTable->end())
    {
        SvNumberformat* pEntry = it->second;
        // Stored are all marked user defined formats and for each active
        // (selected) locale the Standard/General format and
        // NewStandardDefined.
        if ( pEntry->GetUsed() || (pEntry->GetType() & NUMBERFORMAT_DEFINED) ||
                pEntry->GetNewStandardDefined() ||
                (it->first % SV_COUNTRY_LANGUAGE_OFFSET == 0) )
        {
            rStream << it->first
                    << (sal_uInt16) LANGUAGE_SYSTEM
                    << (sal_uInt16) pEntry->GetLanguage();
            pEntry->Save(rStream, aHdr);
        }
        ++it;
    }
    rStream << NUMBERFORMAT_ENTRY_NOT_FOUND;                // end marker

    // as of SV_NUMBERFORMATTER_VERSION_YEAR2000
    aHdr.StartEntry();
    rStream << (sal_uInt16) GetYear2000();
    aHdr.EndEntry();

    if (rStream.GetError())
        return false;
    else
        return true;
}

void SvNumberFormatter::GetUsedLanguages( std::vector<sal_uInt16>& rList )
{
    rList.clear();

    sal_uInt32 nOffset = 0;
    while (nOffset <= MaxCLOffset)
    {
        SvNumberformat* pFormat = GetFormatEntry(nOffset);
        if (pFormat)
            rList.push_back( pFormat->GetLanguage() );
        nOffset += SV_COUNTRY_LANGUAGE_OFFSET;
    }
}


void SvNumberFormatter::FillKeywordTable( NfKeywordTable& rKeywords,
        LanguageType eLang )
{
    ChangeIntl( eLang );
    const NfKeywordTable & rTable = pFormatScanner->GetKeywords();
    for ( sal_uInt16 i = 0; i < NF_KEYWORD_ENTRIES_COUNT; ++i )
    {
        rKeywords[i] = rTable[i];
    }
}


String SvNumberFormatter::GetKeyword( LanguageType eLnge, sal_uInt16 nIndex )
{
    ChangeIntl(eLnge);
    const NfKeywordTable & rTable = pFormatScanner->GetKeywords();
    if ( nIndex < NF_KEYWORD_ENTRIES_COUNT )
        return rTable[nIndex];

    OSL_FAIL("GetKeyword: invalid index");
    return String();
}


String SvNumberFormatter::GetStandardName( LanguageType eLnge )
{
    ChangeIntl( eLnge );
    return pFormatScanner->GetStandardName();
}


sal_uInt32 SvNumberFormatter::ImpGetCLOffset(LanguageType eLnge) const
{
    sal_uInt32 nOffset = 0;
    while (nOffset <= MaxCLOffset)
    {
        const SvNumberformat* pFormat = GetFormatEntry(nOffset);
        if (pFormat && pFormat->GetLanguage() == eLnge)
            return nOffset;
        nOffset += SV_COUNTRY_LANGUAGE_OFFSET;
    }
    return nOffset;
}

sal_uInt32 SvNumberFormatter::ImpIsEntry(const String& rString,
                                       sal_uInt32 nCLOffset,
                                       LanguageType eLnge)
{
    sal_uInt32 res = NUMBERFORMAT_ENTRY_NOT_FOUND;
    SvNumberFormatTable::iterator it = aFTable.find( nCLOffset);
    while ( res == NUMBERFORMAT_ENTRY_NOT_FOUND &&
            it != aFTable.end() && it->second->GetLanguage() == eLnge )
    {
        if ( rString == it->second->GetFormatstring() )
            res = it->first;
        else
            ++it;
    }
    return res;
}


SvNumberFormatTable& SvNumberFormatter::GetFirstEntryTable(
                                                      short& eType,
                                                      sal_uInt32& FIndex,
                                                      LanguageType& rLnge)
{
    short eTypetmp = eType;
    if (eType == NUMBERFORMAT_ALL)                  // empty cell or don't care
        rLnge = IniLnge;
    else
    {
        SvNumberformat* pFormat = GetFormatEntry(FIndex);
        if (!pFormat)
        {
            rLnge = IniLnge;
            eType = NUMBERFORMAT_ALL;
            eTypetmp = eType;
        }
        else
        {
            rLnge = pFormat->GetLanguage();
            eType = pFormat->GetType()&~NUMBERFORMAT_DEFINED;
            if (eType == 0)
            {
                eType = NUMBERFORMAT_DEFINED;
                eTypetmp = eType;
            }
            else if (eType == NUMBERFORMAT_DATETIME)
            {
                eTypetmp = eType;
                eType = NUMBERFORMAT_DATE;
            }
            else
                eTypetmp = eType;
        }
    }
    ChangeIntl(rLnge);
    return GetEntryTable(eTypetmp, FIndex, rLnge);
}

sal_uInt32 SvNumberFormatter::ImpGenerateCL( LanguageType eLnge, bool bNoAdditionalFormats )
{
    ChangeIntl(eLnge);
    sal_uInt32 CLOffset = ImpGetCLOffset(ActLnge);
    if (CLOffset > MaxCLOffset)
    {   // new CL combination
        if (LocaleDataWrapper::areChecksEnabled())
        {
            Locale aLoadedLocale = xLocaleData->getLoadedLocale();
            if ( aLoadedLocale.Language != aLocale.Language ||
                    aLoadedLocale.Country != aLocale.Country )
            {
                rtl::OUString aMsg("SvNumerFormatter::ImpGenerateCL: locales don't match:");
                LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg ));
            }
            // test XML locale data FormatElement entries
            {
                uno::Sequence< i18n::FormatElement > xSeq = xLocaleData->getAllFormats();
                // A test for completeness of formatindex="0" ...
                // formatindex="47" is not needed here since it is done in
                // ImpGenerateFormats().

                // Test for dupes of formatindex="..."
                for ( sal_Int32 j = 0; j < xSeq.getLength(); j++ )
                {
                    sal_Int16 nIdx = xSeq[j].formatIndex;
                    rtl::OUString aDupes;
                    for ( sal_Int32 i = 0; i < xSeq.getLength(); i++ )
                    {
                        if ( i != j && xSeq[i].formatIndex == nIdx )
                        {
                            aDupes += rtl::OUString::valueOf( i );
                            aDupes += "(";
                            aDupes += rtl::OUString( xSeq[i].formatKey );
                            aDupes += ") ";
                        }
                    }
                    if ( !aDupes.isEmpty() )
                    {
                        rtl::OUString aMsg("XML locale data FormatElement formatindex dupe: ");
                        aMsg += rtl::OUString::valueOf( sal_Int32(nIdx) );
                        aMsg += "\nFormatElements: ";
                        aMsg += rtl::OUString::valueOf( j );
                        aMsg += "(";
                        aMsg += rtl::OUString( xSeq[j].formatKey );
                        aMsg += ") ";
                        aMsg += aDupes;
                        LocaleDataWrapper::outputCheckMessage(
                                xLocaleData->appendLocaleInfo( aMsg ));
                    }
                }
            }
        }

        MaxCLOffset += SV_COUNTRY_LANGUAGE_OFFSET;
        ImpGenerateFormats( MaxCLOffset, bNoAdditionalFormats );
        CLOffset = MaxCLOffset;
    }
    return CLOffset;
}

SvNumberFormatTable& SvNumberFormatter::ChangeCL(short eType,
                                                 sal_uInt32& FIndex,
                                                 LanguageType eLnge)
{
    ImpGenerateCL(eLnge);
    return GetEntryTable(eType, FIndex, ActLnge);
}

SvNumberFormatTable& SvNumberFormatter::GetEntryTable(
                                                    short eType,
                                                    sal_uInt32& FIndex,
                                                    LanguageType eLnge)
{
    if ( pFormatTable )
        pFormatTable->clear();
    else
        pFormatTable = new SvNumberFormatTable;
    ChangeIntl(eLnge);
    sal_uInt32 CLOffset = ImpGetCLOffset(ActLnge);

    // Might generate and insert a default format for the given type
    // (e.g. currency) => has to be done before collecting formats.
    sal_uInt32 nDefaultIndex = GetStandardFormat( eType, ActLnge );

    SvNumberFormatTable::iterator it = aFTable.find( CLOffset);

    if (eType == NUMBERFORMAT_ALL)
    {
        while (it != aFTable.end() && it->second->GetLanguage() == ActLnge)
        {   // copy all entries to output table
            (*pFormatTable)[ it->first ] = it->second;
            ++it;
        }
    }
    else
    {
        while (it != aFTable.end() && it->second->GetLanguage() == ActLnge)
        {   // copy entries of queried type to output table
            if ((it->second->GetType()) & eType)
                (*pFormatTable)[ it->first ] = it->second;
            ++it;
        }
    }
    if ( !pFormatTable->empty() )
    {   // select default if queried format doesn't exist or queried type or
        // language differ from existing format
        SvNumberformat* pEntry = GetFormatEntry(FIndex);
        if ( !pEntry || !(pEntry->GetType() & eType) || pEntry->GetLanguage() != ActLnge )
            FIndex = nDefaultIndex;
    }
    return *pFormatTable;
}

bool SvNumberFormatter::IsNumberFormat(const String& sString,
                                       sal_uInt32& F_Index,
                                       double& fOutNumber)
{
    short FType;
    const SvNumberformat* pFormat = GetFormatEntry(F_Index);
    if (!pFormat)
    {
        ChangeIntl(IniLnge);
        FType = NUMBERFORMAT_NUMBER;
    }
    else
    {
        FType = pFormat->GetType() &~NUMBERFORMAT_DEFINED;
        if (FType == 0)
            FType = NUMBERFORMAT_DEFINED;
        ChangeIntl(pFormat->GetLanguage());
    }
    bool res;
    short RType = FType;
    if (RType == NUMBERFORMAT_TEXT)
        res = false;        // type text preset => no conversion to number
    else
        res = pStringScanner->IsNumberFormat(sString, RType, fOutNumber, pFormat);

    if (res && !IsCompatible(FType, RType))     // non-matching type
    {
        switch ( RType )
        {
            case NUMBERFORMAT_DATE :
                // Preserve ISO 8601 input.
                if (pStringScanner->CanForceToIso8601( DMY))
                    F_Index = GetFormatIndex( NF_DATE_DIN_YYYYMMDD, ActLnge );
                else
                    F_Index = GetStandardFormat( RType, ActLnge );
                break;
            case NUMBERFORMAT_TIME :
                if ( pStringScanner->GetDecPos() )
                {   // 100th seconds
                    if ( pStringScanner->GetAnzNums() > 3 || fOutNumber < 0.0 )
                        F_Index = GetFormatIndex( NF_TIME_HH_MMSS00, ActLnge );
                    else
                        F_Index = GetFormatIndex( NF_TIME_MMSS00, ActLnge );
                }
                else if ( fOutNumber >= 1.0 || fOutNumber < 0.0 )
                    F_Index = GetFormatIndex( NF_TIME_HH_MMSS, ActLnge );
                else
                    F_Index = GetStandardFormat( RType, ActLnge );
            break;
            default:
                F_Index = GetStandardFormat( RType, ActLnge );
        }
    }
    return res;
}

bool SvNumberFormatter::IsCompatible(short eOldType,
                                     short eNewType)
{
    if (eOldType == eNewType)
        return true;
    else if (eOldType == NUMBERFORMAT_DEFINED)
        return true;
    else
    {
        switch (eNewType)
        {
            case NUMBERFORMAT_NUMBER:
            {
                switch (eOldType)
                {
                    case NUMBERFORMAT_PERCENT:
                    case NUMBERFORMAT_CURRENCY:
                    case NUMBERFORMAT_SCIENTIFIC:
                    case NUMBERFORMAT_FRACTION:
//                  case NUMBERFORMAT_LOGICAL:
                    case NUMBERFORMAT_DEFINED:
                        return true;
                    default:
                        return false;
                }
            }
            break;
            case NUMBERFORMAT_DATE:
            {
                switch (eOldType)
                {
                    case NUMBERFORMAT_DATETIME:
                        return true;
                    default:
                        return false;
                }
            }
            break;
            case NUMBERFORMAT_TIME:
            {
                switch (eOldType)
                {
                    case NUMBERFORMAT_DATETIME:
                        return true;
                    default:
                        return false;
                }
            }
            break;
            case NUMBERFORMAT_DATETIME:
            {
                switch (eOldType)
                {
                    case NUMBERFORMAT_TIME:
                    case NUMBERFORMAT_DATE:
                        return true;
                    default:
                        return false;
                }
            }
            break;
            default:
            return false;
        }
    }
}


sal_uInt32 SvNumberFormatter::ImpGetDefaultFormat( short nType )
{
    sal_uInt32 CLOffset = ImpGetCLOffset( ActLnge );
    sal_uInt32 nSearch;
    switch( nType )
    {
        case NUMBERFORMAT_DATE      :
            nSearch = CLOffset + ZF_STANDARD_DATE;
        break;
        case NUMBERFORMAT_TIME      :
            nSearch = CLOffset + ZF_STANDARD_TIME;
        break;
        case NUMBERFORMAT_DATETIME  :
            nSearch = CLOffset + ZF_STANDARD_DATETIME;
        break;
        case NUMBERFORMAT_PERCENT   :
            nSearch = CLOffset + ZF_STANDARD_PERCENT;
        break;
        case NUMBERFORMAT_SCIENTIFIC:
            nSearch = CLOffset + ZF_STANDARD_SCIENTIFIC;
        break;
        default:
            nSearch = CLOffset + ZF_STANDARD;
    }
    DefaultFormatKeysMap::iterator it = aDefaultFormatKeys.find( nSearch);
    sal_uInt32 nDefaultFormat = (it != aDefaultFormatKeys.end() ? 
            it->second : NUMBERFORMAT_ENTRY_NOT_FOUND);
    if ( nDefaultFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {   // look for a defined standard
        sal_uInt32 nStopKey = CLOffset + SV_COUNTRY_LANGUAGE_OFFSET;
        sal_uInt32 nKey;
        SvNumberFormatTable::iterator it2 = aFTable.find( CLOffset );
        while ( it2 != aFTable.end() && (nKey = it2->first ) >= CLOffset && nKey < nStopKey )
        {
            const SvNumberformat* pEntry = it2->second;
            if ( pEntry->IsStandard() && ((pEntry->GetType() &
                            ~NUMBERFORMAT_DEFINED) == nType) )
            {
                nDefaultFormat = nKey;
                break;  // while
            }
            ++it2;
        }

        if ( nDefaultFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {   // none found, use old fixed standards
            switch( nType )
            {
                case NUMBERFORMAT_DATE      :
                    nDefaultFormat = CLOffset + ZF_STANDARD_DATE;
                break;
                case NUMBERFORMAT_TIME      :
                    nDefaultFormat = CLOffset + ZF_STANDARD_TIME+1;
                break;
                case NUMBERFORMAT_DATETIME  :
                    nDefaultFormat = CLOffset + ZF_STANDARD_DATETIME;
                break;
                case NUMBERFORMAT_PERCENT   :
                    nDefaultFormat = CLOffset + ZF_STANDARD_PERCENT+1;
                break;
                case NUMBERFORMAT_SCIENTIFIC:
                    nDefaultFormat = CLOffset + ZF_STANDARD_SCIENTIFIC;
                break;
                default:
                    nDefaultFormat = CLOffset + ZF_STANDARD;
            }
        }
        aDefaultFormatKeys[ nSearch ] = nDefaultFormat;
    }
    return nDefaultFormat;
}


sal_uInt32 SvNumberFormatter::GetStandardFormat( short eType, LanguageType eLnge )
{
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;

    sal_uInt32 CLOffset = ImpGenerateCL(eLnge);
    switch(eType)
    {
        case NUMBERFORMAT_CURRENCY  :
        {
            if ( eLnge == LANGUAGE_SYSTEM )
                return ImpGetDefaultSystemCurrencyFormat();
            else
                return ImpGetDefaultCurrencyFormat();
        }
        case NUMBERFORMAT_DATE      :
        case NUMBERFORMAT_TIME      :
        case NUMBERFORMAT_DATETIME  :
        case NUMBERFORMAT_PERCENT   :
        case NUMBERFORMAT_SCIENTIFIC:
            return ImpGetDefaultFormat( eType );

        case NUMBERFORMAT_FRACTION  : return CLOffset + ZF_STANDARD_FRACTION;
        case NUMBERFORMAT_LOGICAL   : return CLOffset + ZF_STANDARD_LOGICAL;
        case NUMBERFORMAT_TEXT      : return CLOffset + ZF_STANDARD_TEXT;
        case NUMBERFORMAT_ALL       :
        case NUMBERFORMAT_DEFINED   :
        case NUMBERFORMAT_NUMBER    :
        case NUMBERFORMAT_UNDEFINED :
        default               : return CLOffset + ZF_STANDARD;
    }
}

bool SvNumberFormatter::IsSpecialStandardFormat( sal_uInt32 nFIndex,
        LanguageType eLnge )
{
    return
        nFIndex == GetFormatIndex( NF_TIME_MMSS00, eLnge ) ||
        nFIndex == GetFormatIndex( NF_TIME_HH_MMSS00, eLnge ) ||
        nFIndex == GetFormatIndex( NF_TIME_HH_MMSS, eLnge )
        ;
}

sal_uInt32 SvNumberFormatter::GetStandardFormat( sal_uInt32 nFIndex, short eType,
        LanguageType eLnge )
{
    if ( IsSpecialStandardFormat( nFIndex, eLnge ) )
        return nFIndex;
    else
        return GetStandardFormat( eType, eLnge );
}

sal_uInt32 SvNumberFormatter::GetStandardFormat( double fNumber, sal_uInt32 nFIndex,
        short eType, LanguageType eLnge )
{
    if ( IsSpecialStandardFormat( nFIndex, eLnge ) )
        return nFIndex;

    switch( eType )
    {
        case NUMBERFORMAT_TIME :
        {
            bool bSign;
            if ( fNumber < 0.0 )
            {
                bSign = true;
                fNumber = -fNumber;
            }
            else
                bSign = false;
            double fSeconds = fNumber * 86400;
            if ( floor( fSeconds + 0.5 ) * 100 != floor( fSeconds * 100 + 0.5 ) )
            {   // with 100th seconds
                if ( bSign || fSeconds >= 3600 )
                    return GetFormatIndex( NF_TIME_HH_MMSS00, eLnge );
                else
                    return GetFormatIndex( NF_TIME_MMSS00, eLnge );
            }
            else
            {
                if ( bSign || fNumber >= 1.0 )
                    return GetFormatIndex( NF_TIME_HH_MMSS, eLnge );
                else
                    return GetStandardFormat( eType, eLnge );
            }
        }
        default:
            return GetStandardFormat( eType, eLnge );
    }
}

sal_uInt32 SvNumberFormatter::GetEditFormat( double fNumber, sal_uInt32 nFIndex,
        short eType, LanguageType eLang, SvNumberformat* pFormat )
{
    sal_uInt32 nKey = nFIndex;
    switch ( eType )
    {   // #61619# always edit using 4-digit year
        case NUMBERFORMAT_DATE :
            if (::rtl::math::approxFloor( fNumber) != fNumber)
                nKey = GetFormatIndex( NF_DATETIME_SYS_DDMMYYYY_HHMMSS, eLang );
                // fdo#34977 preserve time when editing even if only date was
                // displayed.
                /* FIXME: in case an ISO 8601 format was used, editing should
                 * also use such. Unfortunately we have no builtin combined
                 * date+time ISO format defined. Needs also locale data work.
                 * */
            else
            {
                // Preserve ISO 8601 format.
                if (    nFIndex == GetFormatIndex( NF_DATE_DIN_YYYYMMDD, eLang) ||
                        nFIndex == GetFormatIndex( NF_DATE_DIN_YYMMDD, eLang) ||
                        nFIndex == GetFormatIndex( NF_DATE_DIN_MMDD, eLang) ||
                        (pFormat && pFormat->IsIso8601( 0 )))
                    nKey = GetFormatIndex( NF_DATE_DIN_YYYYMMDD, eLang);
                else
                    nKey = GetFormatIndex( NF_DATE_SYS_DDMMYYYY, eLang );
            }
        break;
        case NUMBERFORMAT_TIME :
            if (fNumber < 0.0 || fNumber >= 1.0)
            {
                /* XXX NOTE: this is a purely arbitrary value within the limits
                 * of a signed 16-bit. 32k hours are 3.7 years ... or
                 * 1903-09-26 if date. */
                if (fabs( fNumber) * 24 < 0x7fff)
                    nKey = GetFormatIndex( NF_TIME_HH_MMSS, eLang );
                    // Preserve duration, use [HH]:MM:SS instead of time.
                else
                    nKey = GetFormatIndex( NF_DATETIME_SYS_DDMMYYYY_HHMMSS, eLang );
                    // Assume that a large value is a datetime with only time
                    // displayed.
            }
            else
                nKey = GetStandardFormat( fNumber, nFIndex, eType, eLang );
        break;
        case NUMBERFORMAT_DATETIME :
            nKey = GetFormatIndex( NF_DATETIME_SYS_DDMMYYYY_HHMMSS, eLang );
            /* FIXME: in case an ISO 8601 format was used, editing should
             * also use such. Unfortunately we have no builtin combined
             * date+time ISO format defined. Needs also locale data work. */
        break;
        default:
            nKey = GetStandardFormat( fNumber, nFIndex, eType, eLang );
    }
    return nKey;
}

void SvNumberFormatter::GetInputLineString(const double& fOutNumber,
                                           sal_uInt32 nFIndex,
                                           String& sOutString)
{
    Color* pColor;
    SvNumberformat* pFormat = GetFormatEntry( nFIndex );
    if (!pFormat)
        pFormat = GetFormatEntry(ZF_STANDARD);
    LanguageType eLang = pFormat->GetLanguage();
    ChangeIntl( eLang );
    short eType = pFormat->GetType() & ~NUMBERFORMAT_DEFINED;
    if (eType == 0)
        eType = NUMBERFORMAT_DEFINED;
    sal_uInt16 nOldPrec = pFormatScanner->GetStandardPrec();
    bool bPrecChanged = false;
    if (eType == NUMBERFORMAT_NUMBER || eType == NUMBERFORMAT_PERCENT
                                     || eType == NUMBERFORMAT_CURRENCY
                                     || eType == NUMBERFORMAT_SCIENTIFIC
                                     || eType == NUMBERFORMAT_FRACTION)
    {
        if (eType != NUMBERFORMAT_PERCENT)  // special treatment of % later
            eType = NUMBERFORMAT_NUMBER;
        ChangeStandardPrec(INPUTSTRING_PRECISION);
        bPrecChanged = true;
    }
    sal_uInt32 nKey = GetEditFormat( fOutNumber, nFIndex, eType, eLang, pFormat);
    if ( nKey != nFIndex )
        pFormat = GetFormatEntry( nKey );
    if (pFormat)
    {
        if ( eType == NUMBERFORMAT_TIME && pFormat->GetFormatPrecision() )
        {
            ChangeStandardPrec(INPUTSTRING_PRECISION);
            bPrecChanged = true;
        }
        pFormat->GetOutputString(fOutNumber, sOutString, &pColor);
    }
    if (bPrecChanged)
        ChangeStandardPrec(nOldPrec);
}

void SvNumberFormatter::GetInputLineString(const double& fOutNumber,
                                           sal_uInt32 nFIndex,
                                           rtl::OUString& rOutString)
{
    String aTmp;
    GetInputLineString(fOutNumber, nFIndex, aTmp);
    rOutString = aTmp;
}

void SvNumberFormatter::GetOutputString(const double& fOutNumber,
                                        sal_uInt32 nFIndex,
                                        String& sOutString,
                                        Color** ppColor,
                                        bool bUseStarFormat )
{
    if (bNoZero && fOutNumber == 0.0)
    {
        sOutString.Erase();
        return;
    }
    SvNumberformat* pFormat = GetFormatEntry( nFIndex );
    if (!pFormat)
        pFormat = GetFormatEntry(ZF_STANDARD);
    ChangeIntl(pFormat->GetLanguage());
    if ( bUseStarFormat )
        pFormat->SetStarFormatSupport( true );
    pFormat->GetOutputString(fOutNumber, sOutString, ppColor);
    if ( bUseStarFormat )
        pFormat->SetStarFormatSupport( false );
}

void SvNumberFormatter::GetOutputString(String& sString,
                                        sal_uInt32 nFIndex,
                                        String& sOutString,
                                        Color** ppColor,
                                        bool bUseStarFormat )
{
    SvNumberformat* pFormat = GetFormatEntry( nFIndex );
    if (!pFormat)
        pFormat = GetFormatEntry(ZF_STANDARD_TEXT);
    if (!pFormat->IsTextFormat() && !pFormat->HasTextFormat())
    {
        *ppColor = NULL;
        sOutString = sString;
    }
    else
    {
        ChangeIntl(pFormat->GetLanguage());
        if ( bUseStarFormat )
           pFormat->SetStarFormatSupport( true );
        pFormat->GetOutputString(sString, sOutString, ppColor);
        if ( bUseStarFormat )
           pFormat->SetStarFormatSupport( false );
    }
}

void SvNumberFormatter::GetOutputString(const double& fOutNumber,
                                        sal_uInt32 nFIndex,
                                        rtl::OUString& sOutString,
                                        Color** ppColor,
                                        bool bUseStarFormat )
{
    if (bNoZero && fOutNumber == 0.0)
    {
        sOutString = rtl::OUString();
        return;
    }
    SvNumberformat* pFormat = GetFormatEntry( nFIndex );
    if (!pFormat)
        pFormat = GetFormatEntry(ZF_STANDARD);
    ChangeIntl(pFormat->GetLanguage());
    String aOutString;
    if ( bUseStarFormat )
        pFormat->SetStarFormatSupport( true );
    pFormat->GetOutputString(fOutNumber, aOutString, ppColor);
    if ( bUseStarFormat )
        pFormat->SetStarFormatSupport( false );
    sOutString = aOutString;
}

void SvNumberFormatter::GetOutputString(rtl::OUString& sString,
                                        sal_uInt32 nFIndex,
                                        rtl::OUString& sOutString,
                                        Color** ppColor,
                                        bool bUseStarFormat )
{
    SvNumberformat* pFormat = GetFormatEntry( nFIndex );
    if (!pFormat)
        pFormat = GetFormatEntry(ZF_STANDARD_TEXT);
    if (!pFormat->IsTextFormat() && !pFormat->HasTextFormat())
    {
        *ppColor = NULL;
        sOutString = sString;
    }
    else
    {
        ChangeIntl(pFormat->GetLanguage());
        String aString = sString;
        String aOutString = sOutString;
        if ( bUseStarFormat )
           pFormat->SetStarFormatSupport( true );
        pFormat->GetOutputString(aString, aOutString, ppColor);
        if ( bUseStarFormat )
           pFormat->SetStarFormatSupport( false );
        sString = aString;
        sOutString = aOutString;
    }
}

bool SvNumberFormatter::GetPreviewString(const String& sFormatString,
                                         double fPreviewNumber,
                                         String& sOutString,
                                         Color** ppColor,
                                         LanguageType eLnge,
                                         bool bUseStarFormat )
{
    if (sFormatString.Len() == 0)                       // no empty string
        return false;

    xub_StrLen nCheckPos = STRING_NOTFOUND;
    sal_uInt32 nKey;
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;
    ChangeIntl(eLnge);                          // change locale if necessary
    eLnge = ActLnge;
    String sTmpString = sFormatString;
    SvNumberformat* p_Entry = new SvNumberformat(sTmpString,
                                                 pFormatScanner,
                                                 pStringScanner,
                                                 nCheckPos,
                                                 eLnge);
    if (nCheckPos == 0)                                 // String ok
    {
        sal_uInt32 CLOffset = ImpGenerateCL(eLnge);     // create new standard formats if necessary
        nKey = ImpIsEntry(p_Entry->GetFormatstring(),CLOffset, eLnge);
        if (nKey != NUMBERFORMAT_ENTRY_NOT_FOUND)       // already present
            GetOutputString(fPreviewNumber,nKey,sOutString,ppColor, bUseStarFormat);
        else
        {
            if ( bUseStarFormat )
                p_Entry->SetStarFormatSupport( true );
            p_Entry->GetOutputString(fPreviewNumber,sOutString, ppColor);
            if ( bUseStarFormat )
                p_Entry->SetStarFormatSupport( false );
        }
        delete p_Entry;
        return true;
    }
    else
    {
        delete p_Entry;
        return false;
    }
}

bool SvNumberFormatter::GetPreviewStringGuess( const String& sFormatString,
                                         double fPreviewNumber,
                                         String& sOutString,
                                         Color** ppColor,
                                         LanguageType eLnge )
{
    if (sFormatString.Len() == 0)                       // no empty string
        return false;

    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;

    ChangeIntl( eLnge );
    eLnge = ActLnge;
    bool bEnglish = (eLnge == LANGUAGE_ENGLISH_US);

    String aFormatStringUpper( pCharClass->uppercase( sFormatString ) );
    sal_uInt32 nCLOffset = ImpGenerateCL( eLnge );
    sal_uInt32 nKey = ImpIsEntry( aFormatStringUpper, nCLOffset, eLnge );
    if ( nKey != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {   // Zielformat vorhanden
        GetOutputString( fPreviewNumber, nKey, sOutString, ppColor );
        return true;
    }

    SvNumberformat *pEntry = NULL;
    xub_StrLen nCheckPos = STRING_NOTFOUND;
    String sTmpString;

    if ( bEnglish )
    {
        sTmpString = sFormatString;
        pEntry = new SvNumberformat( sTmpString, pFormatScanner,
            pStringScanner, nCheckPos, eLnge );
    }
    else
    {
        nCLOffset = ImpGenerateCL( LANGUAGE_ENGLISH_US );
        nKey = ImpIsEntry( aFormatStringUpper, nCLOffset, LANGUAGE_ENGLISH_US );
        bool bEnglishFormat = (nKey != NUMBERFORMAT_ENTRY_NOT_FOUND);

        // try english --> other bzw. english nach other konvertieren
        LanguageType eFormatLang = LANGUAGE_ENGLISH_US;
        pFormatScanner->SetConvertMode( LANGUAGE_ENGLISH_US, eLnge );
        sTmpString = sFormatString;
        pEntry = new SvNumberformat( sTmpString, pFormatScanner,
            pStringScanner, nCheckPos, eFormatLang );
        pFormatScanner->SetConvertMode( false );
        ChangeIntl( eLnge );

        if ( !bEnglishFormat )
        {
            if ( nCheckPos > 0 || xTransliteration->isEqual( sFormatString,
                    pEntry->GetFormatstring() ) )
            {   // other Format
                delete pEntry;
                sTmpString = sFormatString;
                pEntry = new SvNumberformat( sTmpString, pFormatScanner,
                    pStringScanner, nCheckPos, eLnge );
            }
            else
            {   // verify english
                xub_StrLen nCheckPos2 = STRING_NOTFOUND;
                // try other --> english
                eFormatLang = eLnge;
                pFormatScanner->SetConvertMode( eLnge, LANGUAGE_ENGLISH_US );
                sTmpString = sFormatString;
                SvNumberformat* pEntry2 = new SvNumberformat( sTmpString, pFormatScanner,
                    pStringScanner, nCheckPos2, eFormatLang );
                pFormatScanner->SetConvertMode( false );
                ChangeIntl( eLnge );
                if ( nCheckPos2 == 0 && !xTransliteration->isEqual( sFormatString,
                        pEntry2->GetFormatstring() ) )
                {   // other Format
                    delete pEntry;
                    sTmpString = sFormatString;
                    pEntry = new SvNumberformat( sTmpString, pFormatScanner,
                        pStringScanner, nCheckPos, eLnge );
                }
                delete pEntry2;
            }
        }
    }

    if (nCheckPos == 0)                                 // String ok
    {
        ImpGenerateCL( eLnge );     // create new standard formats if necessary
        pEntry->GetOutputString( fPreviewNumber, sOutString, ppColor );
        delete pEntry;
        return true;
    }
    delete pEntry;
    return false;
}

bool SvNumberFormatter::GetPreviewString( const String& sFormatString,
                                          const String& sPreviewString,
                                          String& sOutString,
                                          Color** ppColor,
                                          LanguageType eLnge )
{
    if (sFormatString.Len() == 0)               // no empty string
        return false;

    xub_StrLen nCheckPos = STRING_NOTFOUND;
    sal_uInt32 nKey;
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;
    ChangeIntl(eLnge);                          // switch if needed
    eLnge = ActLnge;
    String sTmpString = sFormatString;
    SvNumberformat* p_Entry = new SvNumberformat( sTmpString,
                                                  pFormatScanner,
                                                  pStringScanner,
                                                  nCheckPos,
                                                  eLnge);
    if (nCheckPos == 0)                          // String ok
    {
        String aNonConstPreview( sPreviewString);
        // May have to create standard formats for this locale.
        sal_uInt32 CLOffset = ImpGenerateCL(eLnge);
        nKey = ImpIsEntry( p_Entry->GetFormatstring(), CLOffset, eLnge);
        if (nKey != NUMBERFORMAT_ENTRY_NOT_FOUND)       // already present
            GetOutputString( aNonConstPreview, nKey, sOutString, ppColor);
        else
        {
            // If the format is valid but not a text format and does not
            // include a text subformat, an empty string would result. Same as
            // in SvNumberFormatter::GetOutputString()
            if (p_Entry->IsTextFormat() || p_Entry->HasTextFormat())
                p_Entry->GetOutputString( aNonConstPreview, sOutString, ppColor);
            else
            {
                *ppColor = NULL;
                sOutString = sPreviewString;
            }
        }
        delete p_Entry;
        return true;
    }
    else
    {
        delete p_Entry;
        return false;
    }
}

sal_uInt32 SvNumberFormatter::TestNewString(const String& sFormatString,
                                      LanguageType eLnge)
{
    if (sFormatString.Len() == 0)                       // no empty string
        return NUMBERFORMAT_ENTRY_NOT_FOUND;

    xub_StrLen nCheckPos = STRING_NOTFOUND;
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;
    ChangeIntl(eLnge);                                  // change locale if necessary
    eLnge = ActLnge;
    sal_uInt32 nRes;
    String sTmpString = sFormatString;
    SvNumberformat* pEntry = new SvNumberformat(sTmpString,
                                                pFormatScanner,
                                                pStringScanner,
                                                nCheckPos,
                                                eLnge);
    if (nCheckPos == 0)                                 // String ok
    {
        sal_uInt32 CLOffset = ImpGenerateCL(eLnge);     // create new standard formats if necessary
        nRes = ImpIsEntry(pEntry->GetFormatstring(),CLOffset, eLnge);
                                                        // already present?
    }
    else
        nRes = NUMBERFORMAT_ENTRY_NOT_FOUND;
    delete pEntry;
    return nRes;
}

SvNumberformat* SvNumberFormatter::ImpInsertFormat(
            const ::com::sun::star::i18n::NumberFormatCode& rCode,
            sal_uInt32 nPos, bool bAfterChangingSystemCL, sal_Int16 nOrgIndex )
{
    String aCodeStr( rCode.Code );
    if ( rCode.Index < NF_INDEX_TABLE_ENTRIES &&
            rCode.Usage == ::com::sun::star::i18n::KNumberFormatUsage::CURRENCY &&
            rCode.Index != NF_CURRENCY_1000DEC2_CCC )
    {   // strip surrounding [$...] on automatic currency
        if ( aCodeStr.SearchAscii( "[$" ) != STRING_NOTFOUND )
            aCodeStr = SvNumberformat::StripNewCurrencyDelimiters( aCodeStr, false );
        else
        {
            if (LocaleDataWrapper::areChecksEnabled() &&
                    rCode.Index != NF_CURRENCY_1000DEC2_CCC )
            {
                rtl::OUString aMsg("SvNumberFormatter::ImpInsertFormat: no [$...] on currency format code, index ");
                aMsg += rtl::OUString::valueOf( sal_Int32(rCode.Index) );
                aMsg += ":\n";
                aMsg += rCode.Code;
                LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg));
            }
        }
    }
    xub_StrLen nCheckPos = 0;
    SvNumberformat* pFormat = new SvNumberformat(aCodeStr,
                                                 pFormatScanner,
                                                 pStringScanner,
                                                 nCheckPos,
                                                 ActLnge);
    if ( !pFormat || nCheckPos > 0 )
    {
        if (LocaleDataWrapper::areChecksEnabled())
        {
            rtl::OUString aMsg( "SvNumberFormatter::ImpInsertFormat: bad format code, index " );
            aMsg += rtl::OUString::valueOf( sal_Int32(rCode.Index) );
            aMsg += "\n";
            aMsg += rCode.Code;
            LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg));
        }
        delete pFormat;
        return NULL;
    }
    if ( rCode.Index >= NF_INDEX_TABLE_ENTRIES )
    {
        sal_uInt32 nCLOffset = nPos - (nPos % SV_COUNTRY_LANGUAGE_OFFSET);
        sal_uInt32 nKey = ImpIsEntry( aCodeStr, nCLOffset, ActLnge );
        if ( nKey != NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            // If bAfterChangingSystemCL there will definitely be some dups,
            // don't cry then.
            if (LocaleDataWrapper::areChecksEnabled() && !bAfterChangingSystemCL)
            {
                // Test for duplicate indexes in locale data.
                switch ( nOrgIndex )
                {
                    // These may be dups of integer versions for locales where
                    // currencies have no decimals like Italian Lira.
                    case NF_CURRENCY_1000DEC2 :         // NF_CURRENCY_1000INT
                    case NF_CURRENCY_1000DEC2_RED :     // NF_CURRENCY_1000INT_RED
                    case NF_CURRENCY_1000DEC2_DASHED :  // NF_CURRENCY_1000INT_RED
                        break;
                    default:
                    {
                        rtl::OUString aMsg("SvNumberFormatter::ImpInsertFormat: dup format code, index ");
                        aMsg += rtl::OUString::valueOf( sal_Int32(rCode.Index) );
                        aMsg += "\n";
                        aMsg += rCode.Code;
                        LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg));
                    }
                }
            }
            delete pFormat;
            return NULL;
        }
        else if ( nPos - nCLOffset >= SV_COUNTRY_LANGUAGE_OFFSET )
        {
            if (LocaleDataWrapper::areChecksEnabled())
            {
                rtl::OUString aMsg( "SvNumberFormatter::ImpInsertFormat: too many format codes, index ");
                aMsg += rtl::OUString::valueOf( sal_Int32(rCode.Index) );
                aMsg += "\n";
                aMsg +=  rCode.Code;
                LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg));
            }
            delete pFormat;
            return NULL;
        }
    }
    if ( !aFTable.insert( make_pair( nPos, pFormat) ).second )
    {
        if (LocaleDataWrapper::areChecksEnabled())
        {
            rtl::OUString aMsg( "ImpInsertFormat: can't insert number format key pos: ");
            aMsg += rtl::OUString::valueOf( sal_Int32( nPos ) );
            aMsg += ", code index ";
            aMsg += rtl::OUString::valueOf( sal_Int32(rCode.Index) );
            aMsg += "\n";
            aMsg += rCode.Code;
            LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg));
        }
        delete pFormat;
        return NULL;
    }
    if ( rCode.Default )
        pFormat->SetStandard();
    if ( !rCode.DefaultName.isEmpty() )
        pFormat->SetComment( rCode.DefaultName );
    return pFormat;
}

SvNumberformat* SvNumberFormatter::ImpInsertNewStandardFormat(
            const ::com::sun::star::i18n::NumberFormatCode& rCode,
            sal_uInt32 nPos, sal_uInt16 nVersion, bool bAfterChangingSystemCL,
            sal_Int16 nOrgIndex )
{
    SvNumberformat* pNewFormat = ImpInsertFormat( rCode, nPos,
        bAfterChangingSystemCL, nOrgIndex );
    if (pNewFormat)
        pNewFormat->SetNewStandardDefined( nVersion );
        // so that it gets saved, displayed properly, and converted by old versions
    return pNewFormat;
}

void SvNumberFormatter::GetFormatSpecialInfo(sal_uInt32 nFormat,
                                             bool& bThousand,
                                             bool& IsRed,
                                             sal_uInt16& nPrecision,
                                             sal_uInt16& nAnzLeading)

{
    SvNumberformat* pFormat = GetFormatEntry( nFormat );
    if (pFormat)
        pFormat->GetFormatSpecialInfo(bThousand, IsRed,
                                      nPrecision, nAnzLeading);
    else
    {
        bThousand = false;
        IsRed = false;
        nPrecision = pFormatScanner->GetStandardPrec();
        nAnzLeading = 0;
    }
}

sal_uInt16 SvNumberFormatter::GetFormatPrecision( sal_uInt32 nFormat ) const
{
    const SvNumberformat* pFormat = GetFormatEntry( nFormat );
    if ( pFormat )
        return pFormat->GetFormatPrecision();
    else
        return pFormatScanner->GetStandardPrec();
}


String SvNumberFormatter::GetFormatDecimalSep( sal_uInt32 nFormat ) const
{
    const SvNumberformat* pFormat = GetFormatEntry(nFormat);
    if ( !pFormat || pFormat->GetLanguage() == ActLnge )
        return GetNumDecimalSep();

    String aRet;
    LanguageType eSaveLang = xLocaleData.getCurrentLanguage();
    if ( pFormat->GetLanguage() == eSaveLang )
        aRet = xLocaleData->getNumDecimalSep();
    else
    {
        ::com::sun::star::lang::Locale aSaveLocale( xLocaleData->getLocale() );
        ::com::sun::star::lang::Locale aTmpLocale(MsLangId::convertLanguageToLocale(pFormat->GetLanguage()));
        ((SvNumberFormatter*)this)->xLocaleData.changeLocale(aTmpLocale, pFormat->GetLanguage() );
        aRet = xLocaleData->getNumDecimalSep();
        ((SvNumberFormatter*)this)->xLocaleData.changeLocale( aSaveLocale, eSaveLang );
    }
    return aRet;
}


sal_uInt32 SvNumberFormatter::GetFormatSpecialInfo( const String& rFormatString,
            bool& bThousand, bool& IsRed, sal_uInt16& nPrecision,
            sal_uInt16& nAnzLeading, LanguageType eLnge )

{
    xub_StrLen nCheckPos = 0;
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;
    ChangeIntl(eLnge);                                  // change locale if necessary
    eLnge = ActLnge;
    String aTmpStr( rFormatString );
    SvNumberformat* pFormat = new SvNumberformat( aTmpStr,
        pFormatScanner, pStringScanner, nCheckPos, eLnge );
    if ( nCheckPos == 0 )
        pFormat->GetFormatSpecialInfo( bThousand, IsRed, nPrecision, nAnzLeading );
    else
    {
        bThousand = false;
        IsRed = false;
        nPrecision = pFormatScanner->GetStandardPrec();
        nAnzLeading = 0;
    }
    delete pFormat;
    return nCheckPos;
}


inline sal_uInt32 SetIndexTable( NfIndexTableOffset nTabOff, sal_uInt32 nIndOff )
{
    if ( !bIndexTableInitialized )
    {
        DBG_ASSERT( theIndexTable[nTabOff] == NUMBERFORMAT_ENTRY_NOT_FOUND,
            "SetIndexTable: theIndexTable[nTabOff] already occupied" );
        theIndexTable[nTabOff] = nIndOff;
    }
    return nIndOff;
}


sal_Int32 SvNumberFormatter::ImpGetFormatCodeIndex(
            ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode >& rSeq,
            const NfIndexTableOffset nTabOff )
{
    const sal_Int32 nLen = rSeq.getLength();
    for ( sal_Int32 j=0; j<nLen; j++ )
    {
        if ( rSeq[j].Index == nTabOff )
            return j;
    }
    if (LocaleDataWrapper::areChecksEnabled() && (nTabOff < NF_CURRENCY_START
                || NF_CURRENCY_END < nTabOff || nTabOff == NF_CURRENCY_1000INT
                || nTabOff == NF_CURRENCY_1000INT_RED
                || nTabOff == NF_CURRENCY_1000DEC2_CCC))
    {   // currency entries with decimals might not exist, e.g. Italian Lira
        rtl::OUString aMsg( "SvNumberFormatter::ImpGetFormatCodeIndex: not found: " );
        aMsg += rtl::OUString::valueOf( sal_Int32( nTabOff ) );
        LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo(aMsg));
    }
    if ( nLen )
    {
        sal_Int32 j;
        // look for a preset default
        for ( j=0; j<nLen; j++ )
        {
            if ( rSeq[j].Default )
                return j;
        }
        // currencies are special, not all format codes must exist, but all
        // builtin number format key index positions must have a format assigned
        if ( NF_CURRENCY_START <= nTabOff && nTabOff <= NF_CURRENCY_END )
        {
            // look for a format with decimals
            for ( j=0; j<nLen; j++ )
            {
                if ( rSeq[j].Index == NF_CURRENCY_1000DEC2 )
                    return j;
            }
            // last resort: look for a format without decimals
            for ( j=0; j<nLen; j++ )
            {
                if ( rSeq[j].Index == NF_CURRENCY_1000INT )
                    return j;
            }
        }
    }
    else
    {   // we need at least _some_ format
        rSeq.realloc(1);
        rSeq[0] = ::com::sun::star::i18n::NumberFormatCode();
        rSeq[0].Code = rtl::OUStringBuffer().
            append('0').
            append(GetNumDecimalSep()).
            appendAscii(RTL_CONSTASCII_STRINGPARAM("############")).
            makeStringAndClear();
    }
    return 0;
}


sal_Int32 SvNumberFormatter::ImpAdjustFormatCodeDefault(
        ::com::sun::star::i18n::NumberFormatCode * pFormatArr,
        sal_Int32 nCnt, bool bCheckCorrectness )
{
    using namespace ::com::sun::star;

    if ( !nCnt )
        return -1;
    if (bCheckCorrectness && LocaleDataWrapper::areChecksEnabled())
    {
        // check the locale data for correctness
        rtl::OStringBuffer aMsg;
        sal_Int32 nElem, nShort, nMedium, nLong, nShortDef, nMediumDef, nLongDef;
        nShort = nMedium = nLong = nShortDef = nMediumDef = nLongDef = -1;
        for ( nElem = 0; nElem < nCnt; nElem++ )
        {
            switch ( pFormatArr[nElem].Type )
            {
                case i18n::KNumberFormatType::SHORT :
                    nShort = nElem;
                break;
                case i18n::KNumberFormatType::MEDIUM :
                    nMedium = nElem;
                break;
                case i18n::KNumberFormatType::LONG :
                    nLong = nElem;
                break;
                default:
                    aMsg.append(RTL_CONSTASCII_STRINGPARAM("unknown type"));
            }
            if ( pFormatArr[nElem].Default )
            {
                switch ( pFormatArr[nElem].Type )
                {
                    case i18n::KNumberFormatType::SHORT :
                        if ( nShortDef != -1 )
                            aMsg.append(RTL_CONSTASCII_STRINGPARAM("dupe short type default"));
                        nShortDef = nElem;
                    break;
                    case i18n::KNumberFormatType::MEDIUM :
                        if ( nMediumDef != -1 )
                            aMsg.append(RTL_CONSTASCII_STRINGPARAM("dupe medium type default"));
                        nMediumDef = nElem;
                    break;
                    case i18n::KNumberFormatType::LONG :
                        if ( nLongDef != -1 )
                            aMsg.append(RTL_CONSTASCII_STRINGPARAM("dupe long type default"));
                        nLongDef = nElem;
                    break;
                }
            }
            if (aMsg.getLength())
            {
                aMsg.insert(0, RTL_CONSTASCII_STRINGPARAM("SvNumberFormatter::ImpAdjustFormatCodeDefault: "));
                aMsg.append(RTL_CONSTASCII_STRINGPARAM("\nXML locale data FormatElement formatindex: "));
                aMsg.append(static_cast<sal_Int32>(pFormatArr[nElem].Index));
                rtl::OUString aUMsg(rtl::OStringToOUString(aMsg.makeStringAndClear(),
                    RTL_TEXTENCODING_ASCII_US));
                LocaleDataWrapper::outputCheckMessage(xLocaleData->appendLocaleInfo(aUMsg));
            }
        }
        if ( nShort != -1 && nShortDef == -1 )
            aMsg.append(RTL_CONSTASCII_STRINGPARAM("no short type default  "));
        if ( nMedium != -1 && nMediumDef == -1 )
            aMsg.append(RTL_CONSTASCII_STRINGPARAM("no medium type default  "));
        if ( nLong != -1 && nLongDef == -1 )
            aMsg.append(RTL_CONSTASCII_STRINGPARAM("no long type default  "));
        if (aMsg.getLength())
        {
            aMsg.insert(0, RTL_CONSTASCII_STRINGPARAM("SvNumberFormatter::ImpAdjustFormatCodeDefault: "));
            aMsg.append(RTL_CONSTASCII_STRINGPARAM("\nXML locale data FormatElement group of: "));
            rtl::OUString aUMsg(rtl::OStringToOUString(aMsg.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US));
            LocaleDataWrapper::outputCheckMessage(
                xLocaleData->appendLocaleInfo(aUMsg + pFormatArr[0].NameID));
        }
    }
    // find the default (medium preferred, then long) and reset all other defaults
    sal_Int32 nElem, nDef, nMedium;
    nDef = nMedium = -1;
    for ( nElem = 0; nElem < nCnt; nElem++ )
    {
        if ( pFormatArr[nElem].Default )
        {
            switch ( pFormatArr[nElem].Type )
            {
                case i18n::KNumberFormatType::MEDIUM :
                    nDef = nMedium = nElem;
                break;
                case i18n::KNumberFormatType::LONG :
                    if ( nMedium == -1 )
                        nDef = nElem;
                // fallthru
                default:
                    if ( nDef == -1 )
                        nDef = nElem;
                    pFormatArr[nElem].Default = false;
            }
        }
    }
    if ( nDef == -1 )
        nDef = 0;
    pFormatArr[nDef].Default = true;
    return nDef;
}

SvNumberformat* SvNumberFormatter::GetFormatEntry( sal_uInt32 nKey )
{
    SvNumberFormatTable::iterator it = aFTable.find( nKey);
    if (it != aFTable.end())
        return it->second;
    return 0;
}

const SvNumberformat* SvNumberFormatter::GetEntry( sal_uInt32 nKey ) const
{
    SvNumberFormatTable::const_iterator it = aFTable.find( nKey);
    if (it != aFTable.end())
        return it->second;
    return 0;
}

void SvNumberFormatter::ImpGenerateFormats( sal_uInt32 CLOffset, bool bNoAdditionalFormats )
{
    using namespace ::com::sun::star;

    if ( !bIndexTableInitialized )
    {
        for ( sal_uInt16 j=0; j<NF_INDEX_TABLE_ENTRIES; j++ )
        {
            theIndexTable[j] = NUMBERFORMAT_ENTRY_NOT_FOUND;
        }
    }
    bool bOldConvertMode = pFormatScanner->GetConvertMode();
    if (bOldConvertMode)
        pFormatScanner->SetConvertMode(false);      // switch off for this function

    NumberFormatCodeWrapper aNumberFormatCode( xServiceManager, GetLocale() );

    xub_StrLen nCheckPos = 0;
    SvNumberformat* pNewFormat = NULL;
    String aFormatCode;
    sal_Int32 nIdx;
    bool bDefault;

    // Counter for additional builtin formats not fitting into the first 10
    // of a category (TLOT:=The Legacy Of Templin), altogether about 20 formats.
    // Has to be incremented on each ImpInsertNewStandardformat, new formats
    // must be appended, not inserted!
    sal_uInt16 nNewExtended = ZF_STANDARD_NEWEXTENDED;

    // Number
    uno::Sequence< i18n::NumberFormatCode > aFormatSeq
        = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::FIXED_NUMBER );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // General
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_STANDARD );
    SvNumberformat* pStdFormat = ImpInsertFormat( aFormatSeq[nIdx],
            CLOffset + SetIndexTable( NF_NUMBER_STANDARD, ZF_STANDARD ));
    if (pStdFormat)
    {
        // This is _the_ standard format.
        if (LocaleDataWrapper::areChecksEnabled() && pStdFormat->GetType() != NUMBERFORMAT_NUMBER)
                LocaleDataWrapper::outputCheckMessage( xLocaleData->
                    appendLocaleInfo( "SvNumberFormatter::ImpGenerateFormats: General format not NUMBER"));

        pStdFormat->SetType( NUMBERFORMAT_NUMBER );
        pStdFormat->SetStandard();
        pStdFormat->SetLastInsertKey( SV_MAX_ANZ_STANDARD_FORMATE );
    }
    else
    {
        if (LocaleDataWrapper::areChecksEnabled())
                LocaleDataWrapper::outputCheckMessage( xLocaleData->
                    appendLocaleInfo( "SvNumberFormatter::ImpGenerateFormats: General format not insertable, nothing will work"));
    }

    // Boolean
    aFormatCode = pFormatScanner->GetBooleanString();
    pNewFormat = new SvNumberformat( aFormatCode,
        pFormatScanner, pStringScanner, nCheckPos, ActLnge );
    pNewFormat->SetType(NUMBERFORMAT_LOGICAL);
    pNewFormat->SetStandard();
    if ( !aFTable.insert(make_pair(
            CLOffset + SetIndexTable( NF_BOOLEAN, ZF_STANDARD_LOGICAL ),
            pNewFormat)).second)
        delete pNewFormat;

    // Text
    aFormatCode = '@';
    pNewFormat = new SvNumberformat( aFormatCode,
        pFormatScanner, pStringScanner, nCheckPos, ActLnge );
    pNewFormat->SetType(NUMBERFORMAT_TEXT);
    pNewFormat->SetStandard();
    if ( !aFTable.insert(make_pair(
            CLOffset + SetIndexTable( NF_TEXT, ZF_STANDARD_TEXT ),
            pNewFormat)).second)
        delete pNewFormat;



    // 0
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_INT );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_NUMBER_INT, ZF_STANDARD+1 ));

    // 0.00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_DEC2 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_NUMBER_DEC2, ZF_STANDARD+2 ));

    // #,##0
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_1000INT );
    ImpInsertFormat( aFormatSeq[nIdx],
            CLOffset + SetIndexTable( NF_NUMBER_1000INT, ZF_STANDARD+3 ));

    // #,##0.00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_1000DEC2 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_NUMBER_1000DEC2, ZF_STANDARD+4 ));

    // #.##0,00 System country/language dependent   since number formatter version 6
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_SYSTEM );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_NUMBER_SYSTEM, ZF_STANDARD+5 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );


    // Percent number
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::PERCENT_NUMBER );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // 0%
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_PERCENT_INT );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_PERCENT_INT, ZF_STANDARD_PERCENT ));

    // 0.00%
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_PERCENT_DEC2 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_PERCENT_DEC2, ZF_STANDARD_PERCENT+1 ));



    // Currency. NO default standard option! Default is determined of locale
    // data default currency and format is generated if needed.
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::CURRENCY );
    if (LocaleDataWrapper::areChecksEnabled())
    {
        // though no default desired here, test for correctness of locale data
        ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );
    }

    // #,##0
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000INT );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = false;
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000INT, ZF_STANDARD_CURRENCY ));
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0.00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2 );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = false;
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2, ZF_STANDARD_CURRENCY+1 ));
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0 negative red
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000INT_RED );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = false;
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000INT_RED, ZF_STANDARD_CURRENCY+2 ));
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0.00 negative red
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2_RED );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = false;
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2_RED, ZF_STANDARD_CURRENCY+3 ));
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0.00 USD   since number formatter version 3
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2_CCC );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = false;
    pNewFormat = ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2_CCC, ZF_STANDARD_CURRENCY+4 ));
    if ( pNewFormat )
        pNewFormat->SetUsed(true);      // must be saved for older versions
    aFormatSeq[nIdx].Default = bDefault;

    // #.##0,--   since number formatter version 6
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2_DASHED );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = false;
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_CURRENCY_1000DEC2_DASHED, ZF_STANDARD_CURRENCY+5 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );
    aFormatSeq[nIdx].Default = bDefault;



    // Date
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::DATE );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // DD.MM.YY   System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYSTEM_SHORT );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYSTEM_SHORT, ZF_STANDARD_DATE ));

    // NN DD.MMM YY
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DEF_NNDDMMMYY );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DEF_NNDDMMMYY, ZF_STANDARD_DATE+1 ));

    // DD.MM.YY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_MMYY );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_MMYY, ZF_STANDARD_DATE+2 ));

    // DD MMM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DDMMM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DDMMM, ZF_STANDARD_DATE+3 ));

    // MMMM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_MMMM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_MMMM, ZF_STANDARD_DATE+4 ));

    // QQ YY
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_QQJJ );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_QQJJ, ZF_STANDARD_DATE+5 ));

    // DD.MM.YYYY   since number formatter version 2, was DD.MM.[YY]YY
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DDMMYYYY );
    pNewFormat = ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DDMMYYYY, ZF_STANDARD_DATE+6 ));
    if ( pNewFormat )
        pNewFormat->SetUsed(true);      // must be saved for older versions

    // DD.MM.YY   def/System, since number formatter version 6
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DDMMYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DDMMYY, ZF_STANDARD_DATE+7 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // NNN, D. MMMM YYYY   System
    // Long day of week: "NNNN" instead of "NNN," because of compatibility
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYSTEM_LONG );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYSTEM_LONG, ZF_STANDARD_DATE+8 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // Hard coded but system (regional settings) delimiters dependent long date formats
    // since numberformatter version 6

    // D. MMM YY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DMMMYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DMMMYY, ZF_STANDARD_DATE+9 ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    //! Unfortunally TLOT intended only 10 builtin formats per category, more
    //! would overwrite the next category (ZF_STANDARD_TIME) :-((
    //! Therefore they are inserted with nNewExtended++ (which is also limited)

    // D. MMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // D. MMMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DMMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_DMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // NN, D. MMM YY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_NNDMMMYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_NNDMMMYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // NN, D. MMMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_NNDMMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_NNDMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // NNN, D. MMMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_NNNNDMMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_SYS_NNNNDMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // Hard coded DIN (Deutsche Industrie Norm) and EN (European Norm) date formats

    // D. MMM. YYYY   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_DMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DIN_DMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // D. MMMM YYYY   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_DMMMMYYYY );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DIN_DMMMMYYYY, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // MM-DD   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_MMDD );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DIN_MMDD, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // YY-MM-DD   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_YYMMDD );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DIN_YYMMDD, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );

    // YYYY-MM-DD   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_YYYYMMDD );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATE_DIN_YYYYMMDD, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NEWSTANDARD );



    // Time
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::TIME );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // HH:MM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HHMM, ZF_STANDARD_TIME ));

    // HH:MM:SS
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMMSS );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HHMMSS, ZF_STANDARD_TIME+1 ));

    // HH:MM AM/PM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMMAMPM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HHMMAMPM, ZF_STANDARD_TIME+2 ));

    // HH:MM:SS AM/PM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMMSSAMPM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HHMMSSAMPM, ZF_STANDARD_TIME+3 ));

    // [HH]:MM:SS
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HH_MMSS );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HH_MMSS, ZF_STANDARD_TIME+4 ));

    // MM:SS,00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_MMSS00 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_MMSS00, ZF_STANDARD_TIME+5 ));

    // [HH]:MM:SS,00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HH_MMSS00 );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_TIME_HH_MMSS00, ZF_STANDARD_TIME+6 ),
        SV_NUMBERFORMATTER_VERSION_NF_TIME_HH_MMSS00 );



    // DateTime
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::DATE_TIME );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // DD.MM.YY HH:MM   System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATETIME_SYSTEM_SHORT_HHMM );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATETIME_SYSTEM_SHORT_HHMM, ZF_STANDARD_DATETIME ));

    // DD.MM.YYYY HH:MM:SS   System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATETIME_SYS_DDMMYYYY_HHMMSS );
    ImpInsertNewStandardFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_DATETIME_SYS_DDMMYYYY_HHMMSS, ZF_STANDARD_DATETIME+1 ),
        SV_NUMBERFORMATTER_VERSION_NF_DATETIME_SYS_DDMMYYYY_HHMMSS );



    // Scientific number
    aFormatSeq = aNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // 0.00E+000
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_SCIENTIFIC_000E000 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_SCIENTIFIC_000E000, ZF_STANDARD_SCIENTIFIC ));

    // 0.00E+00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_SCIENTIFIC_000E00 );
    ImpInsertFormat( aFormatSeq[nIdx],
        CLOffset + SetIndexTable( NF_SCIENTIFIC_000E00, ZF_STANDARD_SCIENTIFIC+1 ));



    // Fraction number (no default option)
    i18n::NumberFormatCode aSingleFormatCode;
    aSingleFormatCode.Usage = i18n::KNumberFormatUsage::FRACTION_NUMBER;

     // # ?/?
    aSingleFormatCode.Code = "# ?/?";
    ImpInsertFormat( aSingleFormatCode,
        CLOffset + SetIndexTable( NF_FRACTION_1, ZF_STANDARD_FRACTION ));

    // # ??/??
    //! "??/" would be interpreted by the compiler as a trigraph for '\'
    aSingleFormatCode.Code = "# ?\?/?\?";
    ImpInsertFormat( aSingleFormatCode,
        CLOffset + SetIndexTable( NF_FRACTION_2, ZF_STANDARD_FRACTION+1 ));

    // # ?/4
    aSingleFormatCode.Code = "# ?/4";
    ImpInsertNewStandardFormat( aSingleFormatCode,
        CLOffset + SetIndexTable( NF_FRACTION_3, ZF_STANDARD_FRACTION+2 ),
        SV_NUMBERFORMATTER_VERSION_FIXED_FRACTION );

    // # ??/100
    aSingleFormatCode.Code = "# ?\?/100";
    ImpInsertNewStandardFormat( aSingleFormatCode,
        CLOffset + SetIndexTable( NF_FRACTION_4, ZF_STANDARD_FRACTION+3 ),
        SV_NUMBERFORMATTER_VERSION_FIXED_FRACTION );



    // Week of year   must be appended here because of nNewExtended
    const NfKeywordTable & rKeyword = pFormatScanner->GetKeywords();
    aSingleFormatCode.Code = rKeyword[NF_KEY_WW];
    ImpInsertNewStandardFormat( aSingleFormatCode,
        CLOffset + SetIndexTable( NF_DATE_WW, nNewExtended++ ),
        SV_NUMBERFORMATTER_VERSION_NF_DATE_WW );


    bIndexTableInitialized = true;
    DBG_ASSERT( nNewExtended <= ZF_STANDARD_NEWEXTENDEDMAX,
        "ImpGenerateFormats: overflow of nNewExtended standard formats" );

    // Now all additional format codes provided by I18N, but only if not
    // changing SystemCL, then they are appended last after user defined.
    if ( !bNoAdditionalFormats )
        ImpGenerateAdditionalFormats( CLOffset, aNumberFormatCode, false );

    if (bOldConvertMode)
        pFormatScanner->SetConvertMode(true);
}


void SvNumberFormatter::ImpGenerateAdditionalFormats( sal_uInt32 CLOffset,
            NumberFormatCodeWrapper& rNumberFormatCode, bool bAfterChangingSystemCL )
{
    using namespace ::com::sun::star;

    SvNumberformat* pStdFormat = GetFormatEntry( CLOffset + ZF_STANDARD );
    if ( !pStdFormat )
    {
        SAL_WARN( "svl.numbers", "ImpGenerateAdditionalFormats: no GENERAL format" );
        return ;
    }
    sal_uInt32 nPos = CLOffset + pStdFormat->GetLastInsertKey();
    rNumberFormatCode.setLocale( GetLocale() );
    sal_Int32 j;

    // All currencies, this time with [$...] which was stripped in
    // ImpGenerateFormats for old "automatic" currency formats.
    uno::Sequence< i18n::NumberFormatCode > aFormatSeq =
        rNumberFormatCode.getAllFormatCode( i18n::KNumberFormatUsage::CURRENCY );
    i18n::NumberFormatCode * pFormatArr = aFormatSeq.getArray();
    sal_Int32 nCodes = aFormatSeq.getLength();
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), nCodes );
    for ( j = 0; j < nCodes; j++ )
    {
        if ( nPos - CLOffset >= SV_COUNTRY_LANGUAGE_OFFSET )
        {
            SAL_WARN( "svl.numbers", "ImpGenerateAdditionalFormats: too many formats" );
            break;  // for
        }
        if ( pFormatArr[j].Index < NF_INDEX_TABLE_ENTRIES &&
                pFormatArr[j].Index != NF_CURRENCY_1000DEC2_CCC )
        {   // Insert only if not already inserted, but internal index must be
            // above so ImpInsertFormat can distinguish it.
            sal_Int16 nOrgIndex = pFormatArr[j].Index;
            pFormatArr[j].Index = sal::static_int_cast< sal_Int16 >(
                pFormatArr[j].Index + nCodes + NF_INDEX_TABLE_ENTRIES);
            //! no default on currency
            bool bDefault = aFormatSeq[j].Default;
            aFormatSeq[j].Default = false;
            if ( ImpInsertNewStandardFormat( pFormatArr[j], nPos+1,
                    SV_NUMBERFORMATTER_VERSION_ADDITIONAL_I18N_FORMATS,
                    bAfterChangingSystemCL, nOrgIndex ) )
                nPos++;
            pFormatArr[j].Index = nOrgIndex;
            aFormatSeq[j].Default = bDefault;
        }
    }

    // all additional format codes provided by I18N that are not old standard index
    aFormatSeq = rNumberFormatCode.getAllFormatCodes();
    nCodes = aFormatSeq.getLength();
    if ( nCodes )
    {
        pFormatArr = aFormatSeq.getArray();
        // don't check ALL
        sal_Int32 nDef = ImpAdjustFormatCodeDefault( pFormatArr, nCodes, false);
        // don't have any defaults here
        pFormatArr[nDef].Default = false;
        for ( j = 0; j < nCodes; j++ )
        {
            if ( nPos - CLOffset >= SV_COUNTRY_LANGUAGE_OFFSET )
            {
                SAL_WARN( "svl.numbers", "ImpGenerateAdditionalFormats: too many formats" );
                break;  // for
            }
            if ( pFormatArr[j].Index >= NF_INDEX_TABLE_ENTRIES )
                if ( ImpInsertNewStandardFormat( pFormatArr[j], nPos+1,
                        SV_NUMBERFORMATTER_VERSION_ADDITIONAL_I18N_FORMATS,
                        bAfterChangingSystemCL ) )
                    nPos++;
        }
    }

    pStdFormat->SetLastInsertKey( (sal_uInt16)(nPos - CLOffset) );
}


void SvNumberFormatter::ImpGetPosCurrFormat( String& sPosStr, const String& rCurrSymbol )
{
    NfCurrencyEntry::CompletePositiveFormatString( sPosStr,
        rCurrSymbol, xLocaleData->getCurrPositiveFormat() );
}

void SvNumberFormatter::ImpGetNegCurrFormat( String& sNegStr, const String& rCurrSymbol )
{
    NfCurrencyEntry::CompleteNegativeFormatString( sNegStr,
        rCurrSymbol, xLocaleData->getCurrNegativeFormat() );
}

void SvNumberFormatter::GenerateFormat(String& sString,
                                       sal_uInt32 nIndex,
                                       LanguageType eLnge,
                                       bool bThousand,
                                       bool IsRed,
                                       sal_uInt16 nPrecision,
                                       sal_uInt16 nAnzLeading)
{
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;
    short eType = GetType(nIndex);
    sal_uInt16 i;
    ImpGenerateCL(eLnge);           // create new standard formats if necessary
    sString.Erase();

    utl::DigitGroupingIterator aGrouping( xLocaleData->getDigitGrouping());
    const xub_StrLen nDigitsInFirstGroup = static_cast<xub_StrLen>(aGrouping.get());
    const String& rThSep = GetNumThousandSep();

    SvNumberformat* pFormat = GetFormatEntry( nIndex );

    if (nAnzLeading == 0)
    {
        if (!bThousand)
            sString += '#';
        else
        {
            sString += '#';
            sString += rThSep;
            sString.Expand( sString.Len() + nDigitsInFirstGroup, '#' );
        }
    }
    else
    {
        for (i = 0; i < nAnzLeading; i++)
        {
            if (bThousand && i > 0 && i == aGrouping.getPos())
            {
                sString.Insert( rThSep, 0 );
                aGrouping.advance();
            }
            sString.Insert('0',0);
        }
        if (bThousand && nAnzLeading < nDigitsInFirstGroup + 1)
        {
            for (i = nAnzLeading; i < nDigitsInFirstGroup + 1; i++)
            {
                if (bThousand && i % nDigitsInFirstGroup == 0)
                    sString.Insert( rThSep, 0 );
                sString.Insert('#',0);
            }
        }
    }
    if (nPrecision > 0)
    {
        sString += GetNumDecimalSep();
        sString.Expand( sString.Len() + nPrecision, '0' );
    }
    if (eType == NUMBERFORMAT_PERCENT)
        sString += '%';
    else if (eType == NUMBERFORMAT_CURRENCY)
    {
        String sNegStr = sString;
        String aCurr;
        const NfCurrencyEntry* pEntry;
        bool bBank;
        if ( GetNewCurrencySymbolString( nIndex, aCurr, &pEntry, &bBank ) )
        {
            if ( pEntry )
            {
                sal_uInt16 nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat(
                    xLocaleData->getCurrPositiveFormat(),
                    pEntry->GetPositiveFormat(), bBank );
                sal_uInt16 nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat(
                    xLocaleData->getCurrNegativeFormat(),
                    pEntry->GetNegativeFormat(), bBank );
                pEntry->CompletePositiveFormatString( sString, bBank,
                    nPosiForm );
                pEntry->CompleteNegativeFormatString( sNegStr, bBank,
                    nNegaForm );
            }
            else
            {   // assume currency abbreviation (AKA banking symbol), not symbol
                sal_uInt16 nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat(
                    xLocaleData->getCurrPositiveFormat(),
                    xLocaleData->getCurrPositiveFormat(), true );
                sal_uInt16 nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat(
                    xLocaleData->getCurrNegativeFormat(),
                    xLocaleData->getCurrNegativeFormat(), true );
                NfCurrencyEntry::CompletePositiveFormatString( sString, aCurr,
                    nPosiForm );
                NfCurrencyEntry::CompleteNegativeFormatString( sNegStr, aCurr,
                    nNegaForm );
            }
        }
        else
        {   // "automatic" old style
            String aSymbol, aAbbrev;
            GetCompatibilityCurrency( aSymbol, aAbbrev );
            ImpGetPosCurrFormat( sString, aSymbol );
            ImpGetNegCurrFormat( sNegStr, aSymbol );
        }
        if (IsRed)
        {
            sString += ';';
            sString += '[';
            sString += pFormatScanner->GetRedString();
            sString += ']';
        }
        else
            sString += ';';
        sString += sNegStr;
    }
    if (eType != NUMBERFORMAT_CURRENCY)
    {
        bool insertBrackets = false;
        if ( eType != NUMBERFORMAT_UNDEFINED)
            insertBrackets = pFormat->IsNegativeInBracket();
        if (IsRed || insertBrackets)
        {
            String sTmpStr = sString;

            if ( pFormat->HasPositiveBracketPlaceholder() )
            {
                 sTmpStr += '_';
                 sTmpStr += ')';
            }
            sTmpStr += ';';

            if (IsRed)
            {
                sTmpStr += '[';
                sTmpStr += pFormatScanner->GetRedString();
                sTmpStr += ']';
            }

            if (insertBrackets)
            {
                sTmpStr += '(';
                sTmpStr += sString;
                sTmpStr += ')';
            }
            else
            {
                sTmpStr += '-';
                sTmpStr +=sString;
            }
            sString = sTmpStr;
            }
    }
}

bool SvNumberFormatter::IsUserDefined(const String& sStr,
                                      LanguageType eLnge)
{
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;
    sal_uInt32 CLOffset = ImpGenerateCL(eLnge);     // create new standard formats if necessary
    eLnge = ActLnge;
    sal_uInt32 nKey = ImpIsEntry(sStr, CLOffset, eLnge);
    if (nKey == NUMBERFORMAT_ENTRY_NOT_FOUND)
        return true;
    SvNumberformat* pEntry = GetFormatEntry( nKey );
    if ( pEntry && ((pEntry->GetType() & NUMBERFORMAT_DEFINED) != 0) )
        return true;
    return false;
}

sal_uInt32 SvNumberFormatter::GetEntryKey(const String& sStr,
                                     LanguageType eLnge)
{
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;
    sal_uInt32 CLOffset = ImpGenerateCL(eLnge);     // create new standard formats if necessary
    return ImpIsEntry(sStr, CLOffset, eLnge);
}

sal_uInt32 SvNumberFormatter::GetStandardIndex(LanguageType eLnge)
{
    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;
    return GetStandardFormat(NUMBERFORMAT_NUMBER, eLnge);
}

short SvNumberFormatter::GetType(sal_uInt32 nFIndex)
{
    short eType;
    SvNumberformat* pFormat = GetFormatEntry( nFIndex );
    if (!pFormat)
        eType = NUMBERFORMAT_UNDEFINED;
    else
    {
        eType = pFormat->GetType() &~NUMBERFORMAT_DEFINED;
        if (eType == 0)
            eType = NUMBERFORMAT_DEFINED;
    }
    return eType;
}

void SvNumberFormatter::ClearMergeTable()
{
    if ( pMergeTable )
    {
        pMergeTable->clear();
    }
}

SvNumberFormatterIndexTable* SvNumberFormatter::MergeFormatter(SvNumberFormatter& rTable)
{
    if ( pMergeTable )
        ClearMergeTable();
    else
        pMergeTable = new SvNumberFormatterIndexTable;
    sal_uInt32 nCLOffset = 0;
    sal_uInt32 nOldKey, nOffset, nNewKey;
    SvNumberformat* pNewEntry;
    SvNumberFormatTable::iterator it = rTable.aFTable.begin();
    while (it != rTable.aFTable.end())
    {
        SvNumberformat* pFormat = it->second;
        nOldKey = it->first;
        nOffset = nOldKey % SV_COUNTRY_LANGUAGE_OFFSET;     // relative index
        if (nOffset == 0)                                   // 1st format of CL
            nCLOffset = ImpGenerateCL(pFormat->GetLanguage());

        if (nOffset <= SV_MAX_ANZ_STANDARD_FORMATE)     // Std.form.
        {
            nNewKey = nCLOffset + nOffset;
            if (aFTable.find( nNewKey) == aFTable.end())    // not already present
            {
//              pNewEntry = new SvNumberformat(*pFormat);   // Copy is not sufficient!
                pNewEntry = new SvNumberformat( *pFormat, *pFormatScanner );
                if (!aFTable.insert(make_pair( nNewKey, pNewEntry)).second)
                    delete pNewEntry;
            }
            if (nNewKey != nOldKey)                     // new index
            {
                (*pMergeTable)[nOldKey] = nNewKey;
            }
        }
        else                                            // user defined
        {
//          pNewEntry = new SvNumberformat(*pFormat);   // Copy is not sufficient!
            pNewEntry = new SvNumberformat( *pFormat, *pFormatScanner );
            nNewKey = ImpIsEntry(pNewEntry->GetFormatstring(),
                              nCLOffset,
                              pFormat->GetLanguage());
            if (nNewKey != NUMBERFORMAT_ENTRY_NOT_FOUND) // already present
                delete pNewEntry;
            else
            {
                SvNumberformat* pStdFormat =
                        GetFormatEntry(nCLOffset + ZF_STANDARD);
                sal_uInt32 nPos = nCLOffset + pStdFormat->GetLastInsertKey();
                nNewKey = nPos+1;
                if (nPos - nCLOffset >= SV_COUNTRY_LANGUAGE_OFFSET)
                {
                    OSL_FAIL(
                        "SvNumberFormatter:: Zu viele Formate pro CL");
                    delete pNewEntry;
                }
                else if (!aFTable.insert(make_pair( nNewKey, pNewEntry)).second)
                    delete pNewEntry;
                else
                    pStdFormat->SetLastInsertKey((sal_uInt16) (nNewKey - nCLOffset));
            }
            if (nNewKey != nOldKey)                     // new index
            {
                (*pMergeTable)[nOldKey] = nNewKey;
            }
        }
        ++it;
    }
    return pMergeTable;
}


SvNumberFormatterMergeMap SvNumberFormatter::ConvertMergeTableToMap()
{
    if (!HasMergeFmtTbl())
        return SvNumberFormatterMergeMap();

    SvNumberFormatterMergeMap aMap;
    for (SvNumberFormatterIndexTable::iterator it = pMergeTable->begin(); it != pMergeTable->end(); ++it)
    {
        sal_uInt32 nOldKey = it->first;
        aMap[ nOldKey ] = it->second;
    }
    ClearMergeTable();
    return aMap;
}


sal_uInt32 SvNumberFormatter::GetFormatForLanguageIfBuiltIn( sal_uInt32 nFormat,
        LanguageType eLnge )
{
    if ( eLnge == LANGUAGE_DONTKNOW )
        eLnge = IniLnge;
    if ( nFormat < SV_COUNTRY_LANGUAGE_OFFSET && eLnge == IniLnge )
        return nFormat;     // it stays as it is
    sal_uInt32 nOffset = nFormat % SV_COUNTRY_LANGUAGE_OFFSET;  // relative index
    if ( nOffset > SV_MAX_ANZ_STANDARD_FORMATE )
        return nFormat;    // not a built-in format
    sal_uInt32 nCLOffset = ImpGenerateCL(eLnge);    // create new standard formats if necessary
    return nCLOffset + nOffset;
}


sal_uInt32 SvNumberFormatter::GetFormatIndex( NfIndexTableOffset nTabOff,
        LanguageType eLnge )
{
    if ( nTabOff >= NF_INDEX_TABLE_ENTRIES
            || theIndexTable[nTabOff] == NUMBERFORMAT_ENTRY_NOT_FOUND )
        return NUMBERFORMAT_ENTRY_NOT_FOUND;
    if ( eLnge == LANGUAGE_DONTKNOW )
        eLnge = IniLnge;
    sal_uInt32 nCLOffset = ImpGenerateCL(eLnge);    // create new standard formats if necessary
    return nCLOffset + theIndexTable[nTabOff];
}


NfIndexTableOffset SvNumberFormatter::GetIndexTableOffset( sal_uInt32 nFormat ) const
{
    sal_uInt32 nOffset = nFormat % SV_COUNTRY_LANGUAGE_OFFSET;      // relative index
    if ( nOffset > SV_MAX_ANZ_STANDARD_FORMATE )
        return NF_INDEX_TABLE_ENTRIES;      // not a built-in format
    for ( sal_uInt16 j = 0; j < NF_INDEX_TABLE_ENTRIES; j++ )
    {
        if ( theIndexTable[j] == nOffset )
            return (NfIndexTableOffset) j;
    }
    return NF_INDEX_TABLE_ENTRIES;      // bad luck
}


void SvNumberFormatter::SetYear2000( sal_uInt16 nVal )
{
    pStringScanner->SetYear2000( nVal );
}


sal_uInt16 SvNumberFormatter::GetYear2000() const
{
    return pStringScanner->GetYear2000();
}


sal_uInt16 SvNumberFormatter::ExpandTwoDigitYear( sal_uInt16 nYear ) const
{
    if ( nYear < 100 )
        return SvNumberFormatter::ExpandTwoDigitYear( nYear,
            pStringScanner->GetYear2000() );
    return nYear;
}


// static
sal_uInt16 SvNumberFormatter::GetYear2000Default()
{
    return (sal_uInt16) ::utl::MiscCfg().GetYear2000();
}


// static
const NfCurrencyTable& SvNumberFormatter::GetTheCurrencyTable()
{
    ::osl::MutexGuard aGuard( GetMutex() );
    while ( !bCurrencyTableInitialized )
        ImpInitCurrencyTable();
    return theCurrencyTable::get();
}


// static
const NfCurrencyEntry* SvNumberFormatter::MatchSystemCurrency()
{
    // MUST call GetTheCurrencyTable() before accessing nSystemCurrencyPosition
    const NfCurrencyTable& rTable = GetTheCurrencyTable();
    return nSystemCurrencyPosition ? &rTable[nSystemCurrencyPosition] : NULL;
}


// static
const NfCurrencyEntry& SvNumberFormatter::GetCurrencyEntry( LanguageType eLang )
{
    if ( eLang == LANGUAGE_SYSTEM )
    {
        const NfCurrencyEntry* pCurr = MatchSystemCurrency();
        return pCurr ? *pCurr : GetTheCurrencyTable()[0];
    }
    else
    {
        eLang = MsLangId::getRealLanguage( eLang );
        const NfCurrencyTable& rTable = GetTheCurrencyTable();
        sal_uInt16 nCount = rTable.size();
        for ( sal_uInt16 j = 0; j < nCount; j++ )
        {
            if ( rTable[j].GetLanguage() == eLang )
                return rTable[j];
        }
        return rTable[0];
    }
}


// static
const NfCurrencyEntry* SvNumberFormatter::GetCurrencyEntry(
        const String& rAbbrev, LanguageType eLang )
{
    eLang = MsLangId::getRealLanguage( eLang );
    const NfCurrencyTable& rTable = GetTheCurrencyTable();
    sal_uInt16 nCount = rTable.size();
    for ( sal_uInt16 j = 0; j < nCount; j++ )
    {
        if ( rTable[j].GetLanguage() == eLang &&
                rTable[j].GetBankSymbol() == rAbbrev )
            return &rTable[j];
    }
    return NULL;
}


// static
const NfCurrencyEntry* SvNumberFormatter::GetLegacyOnlyCurrencyEntry(
        const String& rSymbol, const String& rAbbrev )
{
    if (!bCurrencyTableInitialized)
        GetTheCurrencyTable();      // just for initialization
    const NfCurrencyTable& rTable = theLegacyOnlyCurrencyTable::get();
    sal_uInt16 nCount = rTable.size();
    for ( sal_uInt16 j = 0; j < nCount; j++ )
    {
        if ( rTable[j].GetSymbol() == rSymbol &&
                rTable[j].GetBankSymbol() == rAbbrev )
            return &rTable[j];
    }
    return NULL;
}


// static
IMPL_STATIC_LINK_NOINSTANCE( SvNumberFormatter, CurrencyChangeLink, SAL_UNUSED_PARAMETER void*, EMPTYARG )
{
    ::osl::MutexGuard aGuard( GetMutex() );
    String aAbbrev;
    LanguageType eLang = LANGUAGE_SYSTEM;
    SvtSysLocaleOptions().GetCurrencyAbbrevAndLanguage( aAbbrev, eLang );
    SetDefaultSystemCurrency( aAbbrev, eLang );
    return 0;
}


// static
void SvNumberFormatter::SetDefaultSystemCurrency( const String& rAbbrev, LanguageType eLang )
{
    ::osl::MutexGuard aGuard( GetMutex() );
    if ( eLang == LANGUAGE_SYSTEM )
        eLang = SvtSysLocale().GetLanguage();
    const NfCurrencyTable& rTable = GetTheCurrencyTable();
    sal_uInt16 nCount = rTable.size();
    if ( rAbbrev.Len() )
    {
        for ( sal_uInt16 j = 0; j < nCount; j++ )
        {
            if ( rTable[j].GetLanguage() == eLang && rTable[j].GetBankSymbol() == rAbbrev )
            {
                nSystemCurrencyPosition = j;
                return ;
            }
        }
    }
    else
    {
        for ( sal_uInt16 j = 0; j < nCount; j++ )
        {
            if ( rTable[j].GetLanguage() == eLang )
            {
                nSystemCurrencyPosition = j;
                return ;
            }
        }
    }
    nSystemCurrencyPosition = 0;    // not found => simple SYSTEM
}


void SvNumberFormatter::ResetDefaultSystemCurrency()
{
    nDefaultSystemCurrencyFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;
}


sal_uInt32 SvNumberFormatter::ImpGetDefaultSystemCurrencyFormat()
{
    if ( nDefaultSystemCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        xub_StrLen nCheck;
        short nType;
        NfWSStringsDtor aCurrList;
        sal_uInt16 nDefault = GetCurrencyFormatStrings( aCurrList,
            GetCurrencyEntry( LANGUAGE_SYSTEM ), false );
        DBG_ASSERT( aCurrList.size(), "where is the NewCurrency System standard format?!?" );
        // if already loaded or user defined nDefaultSystemCurrencyFormat
        // will be set to the right value
        PutEntry( aCurrList[ nDefault ], nCheck, nType,
            nDefaultSystemCurrencyFormat, LANGUAGE_SYSTEM );
        DBG_ASSERT( nCheck == 0, "NewCurrency CheckError" );
        DBG_ASSERT( nDefaultSystemCurrencyFormat != NUMBERFORMAT_ENTRY_NOT_FOUND,
            "nDefaultSystemCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND" );
    }
    return nDefaultSystemCurrencyFormat;
}


sal_uInt32 SvNumberFormatter::ImpGetDefaultCurrencyFormat()
{
    sal_uInt32 CLOffset = ImpGetCLOffset( ActLnge );
    DefaultFormatKeysMap::iterator it = aDefaultFormatKeys.find( CLOffset + ZF_STANDARD_CURRENCY );
    sal_uInt32 nDefaultCurrencyFormat = (it != aDefaultFormatKeys.end() ? 
            it->second : NUMBERFORMAT_ENTRY_NOT_FOUND);
    if ( nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        // look for a defined standard
        sal_uInt32 nStopKey = CLOffset + SV_COUNTRY_LANGUAGE_OFFSET;
        sal_uInt32 nKey;
        SvNumberFormatTable::iterator it2 = aFTable.lower_bound( CLOffset );
        while ( it2 != aFTable.end() && (nKey = it2->first) >= CLOffset && nKey < nStopKey )
        {
            const SvNumberformat* pEntry = it2->second;
            if ( pEntry->IsStandard() && (pEntry->GetType() & NUMBERFORMAT_CURRENCY) )
            {
                nDefaultCurrencyFormat = nKey;
                break;  // while
            }
            ++it2;
        }

        if ( nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {   // none found, create one
            xub_StrLen nCheck;
            NfWSStringsDtor aCurrList;
            sal_uInt16 nDefault = GetCurrencyFormatStrings( aCurrList,
                GetCurrencyEntry( ActLnge ), false );
            DBG_ASSERT( aCurrList.size(), "where is the NewCurrency standard format?" );
            if ( !aCurrList.empty() )
            {
                // if already loaded or user defined nDefaultSystemCurrencyFormat
                // will be set to the right value
                short nType;
                PutEntry( aCurrList[ nDefault ], nCheck, nType,
                    nDefaultCurrencyFormat, ActLnge );
                DBG_ASSERT( nCheck == 0, "NewCurrency CheckError" );
                DBG_ASSERT( nDefaultCurrencyFormat != NUMBERFORMAT_ENTRY_NOT_FOUND,
                    "nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND" );
            }
            // old automatic currency format as a last resort
            if ( nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
                nDefaultCurrencyFormat = CLOffset + ZF_STANDARD_CURRENCY+3;
            else
            {   // mark as standard so that it is found next time
                SvNumberformat* pEntry = GetFormatEntry( nDefaultCurrencyFormat );
                if ( pEntry )
                    pEntry->SetStandard();
            }
        }
        aDefaultFormatKeys[ CLOffset + ZF_STANDARD_CURRENCY ] = nDefaultCurrencyFormat;
    }
    return nDefaultCurrencyFormat;
}


// static
// try to make it inline if possible since this a loop body
// true: continue; false: break loop, if pFoundEntry==NULL dupe found
#ifndef DBG_UTIL
inline
#endif
    bool SvNumberFormatter::ImpLookupCurrencyEntryLoopBody(
        const NfCurrencyEntry*& pFoundEntry, bool& bFoundBank,
        const NfCurrencyEntry* pData, sal_uInt16 nPos, const String& rSymbol )
{
    bool bFound;
    if ( pData->GetSymbol() == rSymbol )
    {
        bFound = true;
        bFoundBank = false;
    }
    else if ( pData->GetBankSymbol() == rSymbol )
    {
        bFound = true;
        bFoundBank = true;
    }
    else
        bFound = false;
    if ( bFound )
    {
        if ( pFoundEntry && pFoundEntry != pData )
        {
            pFoundEntry = NULL;
            return false;   // break loop, not unique
        }
        if ( nPos == 0 )
        {   // first entry is SYSTEM
            pFoundEntry = MatchSystemCurrency();
            if ( pFoundEntry )
                return false;   // break loop
                // even if there are more matching entries
                // this one is propably the one we are looking for
            else
                pFoundEntry = pData;
        }
        else
            pFoundEntry = pData;
    }
    return true;
}


bool SvNumberFormatter::GetNewCurrencySymbolString( sal_uInt32 nFormat,
            String& rStr, const NfCurrencyEntry** ppEntry /* = NULL */,
            bool* pBank /* = NULL */ ) const
{
    rStr.Erase();
    if ( ppEntry )
        *ppEntry = NULL;
    if ( pBank )
        *pBank = false;
    const SvNumberformat* pFormat = GetFormatEntry(nFormat);
    if ( pFormat )
    {
        String aSymbol, aExtension;
        if ( pFormat->GetNewCurrencySymbol( aSymbol, aExtension ) )
        {
            if ( ppEntry )
            {
                bool bFoundBank = false;
                // we definiteley need an entry matching the format code string
                const NfCurrencyEntry* pFoundEntry = GetCurrencyEntry(
                    bFoundBank, aSymbol, aExtension, pFormat->GetLanguage(),
                    true );
                if ( pFoundEntry )
                {
                    *ppEntry = pFoundEntry;
                    if ( pBank )
                        *pBank = bFoundBank;
                    pFoundEntry->BuildSymbolString( rStr, bFoundBank );
                }
            }
            if ( !rStr.Len() )
            {   // analog to BuildSymbolString
                rStr  = '[';
                rStr += '$';
                if ( aSymbol.Search( '-' ) != STRING_NOTFOUND ||
                        aSymbol.Search( ']' ) != STRING_NOTFOUND )
                {
                    rStr += '"';
                    rStr += aSymbol;
                    rStr += '"';
                }
                else
                    rStr += aSymbol;
                if ( aExtension.Len() )
                    rStr += aExtension;
                rStr += ']';
            }
            return true;
        }
    }
    return false;
}


// static
const NfCurrencyEntry* SvNumberFormatter::GetCurrencyEntry( bool & bFoundBank,
            const String& rSymbol, const String& rExtension,
            LanguageType eFormatLanguage, bool bOnlyStringLanguage )
{
    xub_StrLen nExtLen = rExtension.Len();
    LanguageType eExtLang;
    if ( nExtLen )
    {
        sal_Int32 nExtLang = ::rtl::OUString( rExtension ).toInt32( 16 );
        if ( !nExtLang )
            eExtLang = LANGUAGE_DONTKNOW;
        else
            eExtLang = (LanguageType) ((nExtLang < 0) ?
                -nExtLang : nExtLang);
    }
    else
        eExtLang = LANGUAGE_DONTKNOW;
    const NfCurrencyEntry* pFoundEntry = NULL;
    const NfCurrencyTable& rTable = GetTheCurrencyTable();
    sal_uInt16 nCount = rTable.size();
    bool bCont = true;

    // first try with given extension language/country
    if ( nExtLen )
    {
        for ( sal_uInt16 j = 0; j < nCount && bCont; j++ )
        {
            LanguageType eLang = rTable[j].GetLanguage();
            if ( eLang == eExtLang ||
                    ((eExtLang == LANGUAGE_DONTKNOW) &&
                    (eLang == LANGUAGE_SYSTEM))
                )
            {
                bCont = ImpLookupCurrencyEntryLoopBody( pFoundEntry, bFoundBank,
                    &rTable[j], j, rSymbol );
            }
        }
    }

    // ok?
    if ( pFoundEntry || !bCont || (bOnlyStringLanguage && nExtLen) )
        return pFoundEntry;

    if ( !bOnlyStringLanguage )
    {
        // now try the language/country of the number format
        for ( sal_uInt16 j = 0; j < nCount && bCont; j++ )
        {
            LanguageType eLang = rTable[j].GetLanguage();
            if ( eLang == eFormatLanguage ||
                    ((eFormatLanguage == LANGUAGE_DONTKNOW) &&
                    (eLang == LANGUAGE_SYSTEM))
                )
            {
                bCont = ImpLookupCurrencyEntryLoopBody( pFoundEntry, bFoundBank,
                    &rTable[j], j, rSymbol );
            }
        }

        // ok?
        if ( pFoundEntry || !bCont )
            return pFoundEntry;
    }

    // then try without language/country if no extension specified
    if ( !nExtLen )
    {
        for ( sal_uInt16 j = 0; j < nCount && bCont; j++ )
        {
            bCont = ImpLookupCurrencyEntryLoopBody( pFoundEntry, bFoundBank,
                &rTable[j], j, rSymbol );
        }
    }

    return pFoundEntry;
}


void SvNumberFormatter::GetCompatibilityCurrency( String& rSymbol, String& rAbbrev ) const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency2 >
        xCurrencies( xLocaleData->getAllCurrencies() );

    const ::com::sun::star::i18n::Currency2 *pCurrencies = xCurrencies.getConstArray();
    sal_Int32 nCurrencies = xCurrencies.getLength();

    sal_Int32 j;
    for ( j=0; j < nCurrencies; ++j )
    {
        if ( pCurrencies[j].UsedInCompatibleFormatCodes )
        {
            rSymbol = pCurrencies[j].Symbol;
            rAbbrev = pCurrencies[j].BankSymbol;
            break;
        }
    }
    if ( j >= nCurrencies )
    {
        if (LocaleDataWrapper::areChecksEnabled())
               LocaleDataWrapper::outputCheckMessage( xLocaleData->
                   appendLocaleInfo( "GetCompatibilityCurrency: none?"));

        rSymbol = xLocaleData->getCurrSymbol();
        rAbbrev = xLocaleData->getCurrBankSymbol();
    }
}


void lcl_CheckCurrencySymbolPosition( const NfCurrencyEntry& rCurr )
{
    switch ( rCurr.GetPositiveFormat() )
    {
        case 0:                                         // $1
        case 1:                                         // 1$
        case 2:                                         // $ 1
        case 3:                                         // 1 $
        break;
        default:
            LocaleDataWrapper::outputCheckMessage(
                    "lcl_CheckCurrencySymbolPosition: unknown PositiveFormat");
        break;
    }
    switch ( rCurr.GetNegativeFormat() )
    {
        case 0:                                         // ($1)
        case 1:                                         // -$1
        case 2:                                         // $-1
        case 3:                                         // $1-
        case 4:                                         // (1$)
        case 5:                                         // -1$
        case 6:                                         // 1-$
        case 7:                                         // 1$-
        case 8:                                         // -1 $
        case 9:                                         // -$ 1
        case 10:                                        // 1 $-
        case 11:                                        // $ -1
        case 12 :                                       // $ 1-
        case 13 :                                       // 1- $
        case 14 :                                       // ($ 1)
        case 15 :                                       // (1 $)
        break;
        default:
            LocaleDataWrapper::outputCheckMessage(
                    "lcl_CheckCurrencySymbolPosition: unknown NegativeFormat");
        break;
    }
}

// static
bool SvNumberFormatter::IsLocaleInstalled( LanguageType eLang )
{
    // The set is initialized as a side effect of the currency table
    // created, make sure that exists, which usually is the case unless a
    // SvNumberFormatter was never instanciated.
    GetTheCurrencyTable();
    const NfInstalledLocales &rInstalledLocales = theInstalledLocales::get();
    return rInstalledLocales.find( eLang) != rInstalledLocales.end();
}

// static
void SvNumberFormatter::ImpInitCurrencyTable()
{
    // racing condition possible:
    // ::osl::MutexGuard aGuard( GetMutex() );
    // while ( !bCurrencyTableInitialized )
    //      ImpInitCurrencyTable();
    static bool bInitializing = false;
    if ( bCurrencyTableInitialized || bInitializing )
        return ;
    bInitializing = true;

    RTL_LOGFILE_CONTEXT_AUTHOR( aTimeLog, "svl", "er93726", "SvNumberFormatter::ImpInitCurrencyTable" );

    LanguageType eSysLang = SvtSysLocale().GetLanguage();
    LocaleDataWrapper* pLocaleData = new LocaleDataWrapper(
        ::comphelper::getProcessServiceFactory(),
        MsLangId::convertLanguageToLocale( eSysLang ) );
    // get user configured currency
    String aConfiguredCurrencyAbbrev;
    LanguageType eConfiguredCurrencyLanguage = LANGUAGE_SYSTEM;
    SvtSysLocaleOptions().GetCurrencyAbbrevAndLanguage(
        aConfiguredCurrencyAbbrev, eConfiguredCurrencyLanguage );
    sal_uInt16 nSecondarySystemCurrencyPosition = 0;
    sal_uInt16 nMatchingSystemCurrencyPosition = 0;
    NfCurrencyEntry* pEntry;

    // first entry is SYSTEM
    pEntry = new NfCurrencyEntry( *pLocaleData, LANGUAGE_SYSTEM );
    theCurrencyTable::get().insert( theCurrencyTable::get().begin(), pEntry );
    sal_uInt16 nCurrencyPos = 1;

    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > xLoc =
        LocaleDataWrapper::getInstalledLocaleNames();
    sal_Int32 nLocaleCount = xLoc.getLength();
    RTL_LOGFILE_CONTEXT_TRACE1( aTimeLog, "number of locales: %ld", nLocaleCount );
    Locale const * const pLocales = xLoc.getConstArray();
    NfCurrencyTable &rCurrencyTable = theCurrencyTable::get();
    NfCurrencyTable &rLegacyOnlyCurrencyTable = theLegacyOnlyCurrencyTable::get();
    NfInstalledLocales &rInstalledLocales = theInstalledLocales::get();
    sal_uInt16 nLegacyOnlyCurrencyPos = 0;
    for ( sal_Int32 nLocale = 0; nLocale < nLocaleCount; nLocale++ )
    {
        LanguageType eLang = MsLangId::convertLocaleToLanguage(
                pLocales[nLocale]);
        rInstalledLocales.insert( eLang);
        pLocaleData->setLocale( pLocales[nLocale] );
        Sequence< Currency2 > aCurrSeq = pLocaleData->getAllCurrencies();
        sal_Int32 nCurrencyCount = aCurrSeq.getLength();
        Currency2 const * const pCurrencies = aCurrSeq.getConstArray();

        // one default currency for each locale, insert first so it is found first
        sal_Int32 nDefault;
        for ( nDefault = 0; nDefault < nCurrencyCount; nDefault++ )
        {
            if ( pCurrencies[nDefault].Default )
                break;
        }
        if ( nDefault < nCurrencyCount )
            pEntry = new NfCurrencyEntry( pCurrencies[nDefault], *pLocaleData, eLang );
        else
            pEntry = new NfCurrencyEntry( *pLocaleData, eLang );    // first or ShellsAndPebbles

        if (LocaleDataWrapper::areChecksEnabled())
            lcl_CheckCurrencySymbolPosition( *pEntry );

        rCurrencyTable.insert( rCurrencyTable.begin() + nCurrencyPos++, pEntry );
        if ( !nSystemCurrencyPosition && (aConfiguredCurrencyAbbrev.Len() ?
                pEntry->GetBankSymbol() == aConfiguredCurrencyAbbrev &&
                pEntry->GetLanguage() == eConfiguredCurrencyLanguage : false) )
            nSystemCurrencyPosition = nCurrencyPos-1;
        if ( !nMatchingSystemCurrencyPosition &&
                pEntry->GetLanguage() == eSysLang )
            nMatchingSystemCurrencyPosition = nCurrencyPos-1;

        // all remaining currencies for each locale
        if ( nCurrencyCount > 1 )
        {
            sal_Int32 nCurrency;
            for ( nCurrency = 0; nCurrency < nCurrencyCount; nCurrency++ )
            {
                if (pCurrencies[nCurrency].LegacyOnly)
                {
                    pEntry = new NfCurrencyEntry( pCurrencies[nCurrency], *pLocaleData, eLang );
                    rLegacyOnlyCurrencyTable.insert( rLegacyOnlyCurrencyTable.begin() + nLegacyOnlyCurrencyPos++, pEntry );
                }
                else if ( nCurrency != nDefault )
                {
                    pEntry = new NfCurrencyEntry( pCurrencies[nCurrency], *pLocaleData, eLang );
                    // no dupes
                    bool bInsert = true;
                    sal_uInt16 n = rCurrencyTable.size();
                    sal_uInt16 aCurrencyIndex = 1; // skip first SYSTEM entry
                    for ( sal_uInt16 j=1; j<n; j++ )
                    {
                        if ( rCurrencyTable[aCurrencyIndex++] == *pEntry )
                        {
                            bInsert = false;
                            break;  // for
                        }
                    }
                    if ( !bInsert )
                        delete pEntry;
                    else
                    {
                        rCurrencyTable.insert( rCurrencyTable.begin() + nCurrencyPos++, pEntry );
                        if ( !nSecondarySystemCurrencyPosition &&
                                (aConfiguredCurrencyAbbrev.Len() ?
                                pEntry->GetBankSymbol() == aConfiguredCurrencyAbbrev :
                                pEntry->GetLanguage() == eConfiguredCurrencyLanguage) )
                            nSecondarySystemCurrencyPosition = nCurrencyPos-1;
                        if ( !nMatchingSystemCurrencyPosition &&
                                pEntry->GetLanguage() ==  eSysLang )
                            nMatchingSystemCurrencyPosition = nCurrencyPos-1;
                    }
                }
            }
        }
    }
    if ( !nSystemCurrencyPosition )
        nSystemCurrencyPosition = nSecondarySystemCurrencyPosition;
    if ((aConfiguredCurrencyAbbrev.Len() && !nSystemCurrencyPosition) &&
            LocaleDataWrapper::areChecksEnabled())
        LocaleDataWrapper::outputCheckMessage(
                "SvNumberFormatter::ImpInitCurrencyTable: configured currency not in I18N locale data.");
    // match SYSTEM if no configured currency found
    if ( !nSystemCurrencyPosition )
        nSystemCurrencyPosition = nMatchingSystemCurrencyPosition;
    if ((!aConfiguredCurrencyAbbrev.Len() && !nSystemCurrencyPosition) &&
            LocaleDataWrapper::areChecksEnabled())
        LocaleDataWrapper::outputCheckMessage(
                "SvNumberFormatter::ImpInitCurrencyTable: system currency not in I18N locale data.");
    delete pLocaleData;
    SvtSysLocaleOptions::SetCurrencyChangeLink(
        STATIC_LINK( NULL, SvNumberFormatter, CurrencyChangeLink ) );
    bInitializing = false;
    bCurrencyTableInitialized = true;
}


sal_uInt16 SvNumberFormatter::GetCurrencyFormatStrings( NfWSStringsDtor& rStrArr,
            const NfCurrencyEntry& rCurr, bool bBank ) const
{
    rtl::OUString aRed = rtl::OUStringBuffer().
        append('[').
        append(pFormatScanner->GetRedString()).
        append(']').makeStringAndClear();

    sal_uInt16 nDefault = 0;
    if ( bBank )
    {
        // Only bank symbols.
        String aPositiveBank, aNegativeBank;
        rCurr.BuildPositiveFormatString( aPositiveBank, true, *xLocaleData, 1 );
        rCurr.BuildNegativeFormatString( aNegativeBank, true, *xLocaleData, 1 );

        ::rtl::OUStringBuffer format1(aPositiveBank);
        format1.append(';');
        format1.append(aNegativeBank);
        rStrArr.push_back(format1.makeStringAndClear());

        ::rtl::OUStringBuffer format2(aPositiveBank);
        format2.append(';');

        format2.append(aRed);

        format2.append(aNegativeBank);
        rStrArr.push_back(format2.makeStringAndClear());

        nDefault = rStrArr.size() - 1;
    }
    else
    {
        // Mixed formats like in SvNumberFormatter::ImpGenerateFormats() but no
        // duplicates if no decimals in currency.
        String aPositive, aNegative, aPositiveNoDec, aNegativeNoDec,
            aPositiveDashed, aNegativeDashed;

        rCurr.BuildPositiveFormatString( aPositive, false, *xLocaleData, 1 );
        rCurr.BuildNegativeFormatString( aNegative, false, *xLocaleData, 1 );
        ::rtl::OUStringBuffer format1;
        ::rtl::OUStringBuffer format2;
        ::rtl::OUStringBuffer format3;
        ::rtl::OUStringBuffer format4;
        ::rtl::OUStringBuffer format5;
        if ( rCurr.GetDigits() )
        {
            rCurr.BuildPositiveFormatString( aPositiveNoDec, false, *xLocaleData, 0 );
            rCurr.BuildNegativeFormatString( aNegativeNoDec, false, *xLocaleData, 0 );
            rCurr.BuildPositiveFormatString( aPositiveDashed, false, *xLocaleData, 2 );
            rCurr.BuildNegativeFormatString( aNegativeDashed, false, *xLocaleData, 2 );

            format1.append(aPositiveNoDec);
            format1.append(';');
            format1.append(aNegativeNoDec);

            format3.append(aPositiveNoDec);
            format3.append(';');
            format3.append(aRed);
            format3.append(aNegativeNoDec);

            format5.append(aPositiveDashed);
            format5.append(';');
            format5.append(aRed);
            format5.append(aNegativeDashed);
        }

        format2.append(aPositive);
        format2.append(';');
        format2.append(aNegative);

        format4.append(aPositive);
        format4.append(';');
        format4.append(aRed);
        format4.append(aNegative);

        if (rCurr.GetDigits()) {
            rStrArr.push_back(format1.makeStringAndClear());
        }
        rStrArr.push_back(format2.makeStringAndClear());
        if (rCurr.GetDigits()) {
            rStrArr.push_back(format3.makeStringAndClear());
        }
        rStrArr.push_back(format4.makeStringAndClear());
        nDefault = rStrArr.size() - 1;
        if (rCurr.GetDigits()) {
            rStrArr.push_back(format5.makeStringAndClear());
        }
    }
    return nDefault;
}


//--- NfCurrencyEntry ----------------------------------------------------


NfCurrencyEntry::NfCurrencyEntry( const LocaleDataWrapper& rLocaleData, LanguageType eLang )
{
    aSymbol         = rLocaleData.getCurrSymbol();
    aBankSymbol     = rLocaleData.getCurrBankSymbol();
    eLanguage       = eLang;
    nPositiveFormat = rLocaleData.getCurrPositiveFormat();
    nNegativeFormat = rLocaleData.getCurrNegativeFormat();
    nDigits         = rLocaleData.getCurrDigits();
    cZeroChar       = rLocaleData.getCurrZeroChar();
}


NfCurrencyEntry::NfCurrencyEntry( const ::com::sun::star::i18n::Currency & rCurr,
            const LocaleDataWrapper& rLocaleData, LanguageType eLang )
{
    aSymbol         = rCurr.Symbol;
    aBankSymbol     = rCurr.BankSymbol;
    eLanguage       = eLang;
    nPositiveFormat = rLocaleData.getCurrPositiveFormat();
    nNegativeFormat = rLocaleData.getCurrNegativeFormat();
    nDigits         = rCurr.DecimalPlaces;
    cZeroChar       = rLocaleData.getCurrZeroChar();
}


bool NfCurrencyEntry::operator==( const NfCurrencyEntry& r ) const
{
    return aSymbol      == r.aSymbol
        && aBankSymbol  == r.aBankSymbol
        && eLanguage    == r.eLanguage
        ;
}

void NfCurrencyEntry::BuildSymbolString( String& rStr, bool bBank,
            bool bWithoutExtension ) const
{
    rStr  = '[';
    rStr += '$';
    if ( bBank )
        rStr += aBankSymbol;
    else
    {
        if ( aSymbol.Search( '-' ) != STRING_NOTFOUND || aSymbol.Search( ']' ) != STRING_NOTFOUND )
        {
            rStr += '"';
            rStr += aSymbol;
            rStr += '"';
        }
        else
            rStr += aSymbol;
        if ( !bWithoutExtension && eLanguage != LANGUAGE_DONTKNOW && eLanguage != LANGUAGE_SYSTEM )
        {
            rStr += '-';
            rStr += String::CreateFromInt32( sal_Int32( eLanguage ), 16 ).ToUpperAscii();
        }
    }
    rStr += ']';
}


void NfCurrencyEntry::Impl_BuildFormatStringNumChars( String& rStr,
            const LocaleDataWrapper& rLoc, sal_uInt16 nDecimalFormat ) const
{
    rStr.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "###0" ) );
    rStr.Insert( rLoc.getNumThousandSep(), 1 );
    if ( nDecimalFormat && nDigits )
    {
        rStr += rLoc.getNumDecimalSep();
        rStr.Expand( rStr.Len() + nDigits, (nDecimalFormat == 2 ? '-' : cZeroChar) );
    }
}


void NfCurrencyEntry::BuildPositiveFormatString( String& rStr, bool bBank,
            const LocaleDataWrapper& rLoc, sal_uInt16 nDecimalFormat ) const
{
    Impl_BuildFormatStringNumChars( rStr, rLoc, nDecimalFormat );
    sal_uInt16 nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat(
        rLoc.getCurrPositiveFormat(), nPositiveFormat, bBank );
    CompletePositiveFormatString( rStr, bBank, nPosiForm );
}


void NfCurrencyEntry::BuildNegativeFormatString( String& rStr, bool bBank,
            const LocaleDataWrapper& rLoc, sal_uInt16 nDecimalFormat ) const
{
    Impl_BuildFormatStringNumChars( rStr, rLoc, nDecimalFormat );
    sal_uInt16 nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat(
        rLoc.getCurrNegativeFormat(), nNegativeFormat, bBank );
    CompleteNegativeFormatString( rStr, bBank, nNegaForm );
}


void NfCurrencyEntry::CompletePositiveFormatString( String& rStr, bool bBank,
            sal_uInt16 nPosiForm ) const
{
    String aSymStr;
    BuildSymbolString( aSymStr, bBank );
    NfCurrencyEntry::CompletePositiveFormatString( rStr, aSymStr, nPosiForm );
}


void NfCurrencyEntry::CompleteNegativeFormatString( String& rStr, bool bBank,
            sal_uInt16 nNegaForm ) const
{
    String aSymStr;
    BuildSymbolString( aSymStr, bBank );
    NfCurrencyEntry::CompleteNegativeFormatString( rStr, aSymStr, nNegaForm );
}


// static
void NfCurrencyEntry::CompletePositiveFormatString( String& rStr,
        const String& rSymStr, sal_uInt16 nPositiveFormat )
{
    switch( nPositiveFormat )
    {
        case 0:                                         // $1
            rStr.Insert( rSymStr , 0 );
        break;
        case 1:                                         // 1$
            rStr += rSymStr;
        break;
        case 2:                                         // $ 1
        {
            rStr.Insert( ' ', 0 );
            rStr.Insert( rSymStr, 0 );
        }
        break;
        case 3:                                         // 1 $
        {
            rStr += ' ';
            rStr += rSymStr;
        }
        break;
        default:
            OSL_FAIL("NfCurrencyEntry::CompletePositiveFormatString: unknown option");
        break;
    }
}


// static
void NfCurrencyEntry::CompleteNegativeFormatString( String& rStr,
        const String& rSymStr, sal_uInt16 nNegativeFormat )
{
    switch( nNegativeFormat )
    {
        case 0:                                         // ($1)
        {
            rStr.Insert( rSymStr, 0);
            rStr.Insert('(',0);
            rStr += ')';
        }
        break;
        case 1:                                         // -$1
        {
            rStr.Insert( rSymStr, 0);
            rStr.Insert('-',0);
        }
        break;
        case 2:                                         // $-1
        {
            rStr.Insert('-',0);
            rStr.Insert( rSymStr, 0);
        }
        break;
        case 3:                                         // $1-
        {
            rStr.Insert( rSymStr, 0);
            rStr += '-';
        }
        break;
        case 4:                                         // (1$)
        {
            rStr.Insert('(',0);
            rStr += rSymStr;
            rStr += ')';
        }
        break;
        case 5:                                         // -1$
        {
            rStr += rSymStr;
            rStr.Insert('-',0);
        }
        break;
        case 6:                                         // 1-$
        {
            rStr += '-';
            rStr += rSymStr;
        }
        break;
        case 7:                                         // 1$-
        {
            rStr += rSymStr;
            rStr += '-';
        }
        break;
        case 8:                                         // -1 $
        {
            rStr += ' ';
            rStr += rSymStr;
            rStr.Insert('-',0);
        }
        break;
        case 9:                                         // -$ 1
        {
            rStr.Insert(' ',0);
            rStr.Insert( rSymStr, 0);
            rStr.Insert('-',0);
        }
        break;
        case 10:                                        // 1 $-
        {
            rStr += ' ';
            rStr += rSymStr;
            rStr += '-';
        }
        break;
        case 11:                                        // $ -1
        {
            String aTmp( rSymStr );
            aTmp += ' ';
            aTmp += '-';
            rStr.Insert( aTmp, 0 );
        }
        break;
        case 12 :                                       // $ 1-
        {
            rStr.Insert(' ', 0);
            rStr.Insert( rSymStr, 0);
            rStr += '-';
        }
        break;
        case 13 :                                       // 1- $
        {
            rStr += '-';
            rStr += ' ';
            rStr += rSymStr;
        }
        break;
        case 14 :                                       // ($ 1)
        {
            rStr.Insert(' ',0);
            rStr.Insert( rSymStr, 0);
            rStr.Insert('(',0);
            rStr += ')';
        }
        break;
        case 15 :                                       // (1 $)
        {
            rStr.Insert('(',0);
            rStr += ' ';
            rStr += rSymStr;
            rStr += ')';
        }
        break;
        default:
            OSL_FAIL("NfCurrencyEntry::CompleteNegativeFormatString: unknown option");
        break;
    }
}


// static
sal_uInt16 NfCurrencyEntry::GetEffectivePositiveFormat(
    sal_uInt16 nIntlFormat, sal_uInt16 nCurrFormat, bool bBank )
{
    if ( bBank )
    {
#if NF_BANKSYMBOL_FIX_POSITION
        (void) nIntlFormat; // avoid warnings
        return 3;
#else
        switch ( nIntlFormat )
        {
            case 0:                                         // $1
                nIntlFormat = 2;                            // $ 1
            break;
            case 1:                                         // 1$
                nIntlFormat = 3;                            // 1 $
            break;
            case 2:                                         // $ 1
            break;
            case 3:                                         // 1 $
            break;
            default:
                OSL_FAIL("NfCurrencyEntry::GetEffectivePositiveFormat: unknown option");
            break;
        }
        return nIntlFormat;
#endif
    }
    else
        return nCurrFormat;
}


//! Call this only if nCurrFormat is really with parentheses!
sal_uInt16 lcl_MergeNegativeParenthesisFormat( sal_uInt16 nIntlFormat, sal_uInt16 nCurrFormat )
{
    short nSign = 0;        // -1:=Klammer 0:=links, 1:=mitte, 2:=rechts
    switch ( nIntlFormat )
    {
        case 0:                                         // ($1)
        case 4:                                         // (1$)
        case 14 :                                       // ($ 1)
        case 15 :                                       // (1 $)
            return nCurrFormat;
        case 1:                                         // -$1
        case 5:                                         // -1$
        case 8:                                         // -1 $
        case 9:                                         // -$ 1
            nSign = 0;
        break;
        case 2:                                         // $-1
        case 6:                                         // 1-$
        case 11 :                                       // $ -1
        case 13 :                                       // 1- $
            nSign = 1;
        break;
        case 3:                                         // $1-
        case 7:                                         // 1$-
        case 10:                                        // 1 $-
        case 12 :                                       // $ 1-
            nSign = 2;
        break;
        default:
            OSL_FAIL("lcl_MergeNegativeParenthesisFormat: unknown option");
        break;
    }

    switch ( nCurrFormat )
    {
        case 0:                                         // ($1)
            switch ( nSign )
            {
                case 0:
                    return 1;                           // -$1
                case 1:
                    return 2;                           // $-1
                case 2:
                    return 3;                           // $1-
            }
        break;
        case 4:                                         // (1$)
            switch ( nSign )
            {
                case 0:
                    return 5;                           // -1$
                case 1:
                    return 6;                           // 1-$
                case 2:
                    return 7;                           // 1$-
            }
        break;
        case 14 :                                       // ($ 1)
            switch ( nSign )
            {
                case 0:
                    return 9;                           // -$ 1
                case 1:
                    return 11;                          // $ -1
                case 2:
                    return 12;                          // $ 1-
            }
        break;
        case 15 :                                       // (1 $)
            switch ( nSign )
            {
                case 0:
                    return 8;                           // -1 $
                case 1:
                    return 13;                          // 1- $
                case 2:
                    return 10;                          // 1 $-
            }
        break;
    }
    return nCurrFormat;
}


// static
sal_uInt16 NfCurrencyEntry::GetEffectiveNegativeFormat( sal_uInt16 nIntlFormat,
            sal_uInt16 nCurrFormat, bool bBank )
{
    if ( bBank )
    {
#if NF_BANKSYMBOL_FIX_POSITION
        return 8;
#else
        switch ( nIntlFormat )
        {
            case 0:                                         // ($1)
//              nIntlFormat = 14;                           // ($ 1)
                nIntlFormat = 9;                            // -$ 1
            break;
            case 1:                                         // -$1
                nIntlFormat = 9;                            // -$ 1
            break;
            case 2:                                         // $-1
                nIntlFormat = 11;                           // $ -1
            break;
            case 3:                                         // $1-
                nIntlFormat = 12;                           // $ 1-
            break;
            case 4:                                         // (1$)
//              nIntlFormat = 15;                           // (1 $)
                nIntlFormat = 8;                            // -1 $
            break;
            case 5:                                         // -1$
                nIntlFormat = 8;                            // -1 $
            break;
            case 6:                                         // 1-$
                nIntlFormat = 13;                           // 1- $
            break;
            case 7:                                         // 1$-
                nIntlFormat = 10;                           // 1 $-
            break;
            case 8:                                         // -1 $
            break;
            case 9:                                         // -$ 1
            break;
            case 10:                                        // 1 $-
            break;
            case 11:                                        // $ -1
            break;
            case 12 :                                       // $ 1-
            break;
            case 13 :                                       // 1- $
            break;
            case 14 :                                       // ($ 1)
//              nIntlFormat = 14;                           // ($ 1)
                nIntlFormat = 9;                            // -$ 1
            break;
            case 15 :                                       // (1 $)
//              nIntlFormat = 15;                           // (1 $)
                nIntlFormat = 8;                            // -1 $
            break;
            default:
                OSL_FAIL("NfCurrencyEntry::GetEffectiveNegativeFormat: unknown option");
            break;
        }
#endif
    }
    else if ( nIntlFormat != nCurrFormat )
    {
        switch ( nCurrFormat )
        {
            case 0:                                         // ($1)
                nIntlFormat = lcl_MergeNegativeParenthesisFormat(
                    nIntlFormat, nCurrFormat );
            break;
            case 1:                                         // -$1
                nIntlFormat = nCurrFormat;
            break;
            case 2:                                         // $-1
                nIntlFormat = nCurrFormat;
            break;
            case 3:                                         // $1-
                nIntlFormat = nCurrFormat;
            break;
            case 4:                                         // (1$)
                nIntlFormat = lcl_MergeNegativeParenthesisFormat(
                    nIntlFormat, nCurrFormat );
            break;
            case 5:                                         // -1$
                nIntlFormat = nCurrFormat;
            break;
            case 6:                                         // 1-$
                nIntlFormat = nCurrFormat;
            break;
            case 7:                                         // 1$-
                nIntlFormat = nCurrFormat;
            break;
            case 8:                                         // -1 $
                nIntlFormat = nCurrFormat;
            break;
            case 9:                                         // -$ 1
                nIntlFormat = nCurrFormat;
            break;
            case 10:                                        // 1 $-
                nIntlFormat = nCurrFormat;
            break;
            case 11:                                        // $ -1
                nIntlFormat = nCurrFormat;
            break;
            case 12 :                                       // $ 1-
                nIntlFormat = nCurrFormat;
            break;
            case 13 :                                       // 1- $
                nIntlFormat = nCurrFormat;
            break;
            case 14 :                                       // ($ 1)
                nIntlFormat = lcl_MergeNegativeParenthesisFormat(
                    nIntlFormat, nCurrFormat );
            break;
            case 15 :                                       // (1 $)
                nIntlFormat = lcl_MergeNegativeParenthesisFormat(
                    nIntlFormat, nCurrFormat );
            break;
            default:
                OSL_FAIL("NfCurrencyEntry::GetEffectiveNegativeFormat: unknown option");
            break;
        }
    }
    return nIntlFormat;
}


// we only support default encodings here
// static
sal_Char NfCurrencyEntry::GetEuroSymbol( rtl_TextEncoding eTextEncoding )
{
    switch ( eTextEncoding )
    {
        case RTL_TEXTENCODING_MS_1252 :         // WNT Ansi
        case RTL_TEXTENCODING_ISO_8859_1 :      // UNX for use with TrueType fonts
            return '\x80';
        case RTL_TEXTENCODING_ISO_8859_15 :     // UNX real
            return '\xA4';
        case RTL_TEXTENCODING_IBM_850 :         // OS2
            return '\xD5';
        case RTL_TEXTENCODING_APPLE_ROMAN :     // MAC
            return '\xDB';
        default:                                // default system
#if WNT
            return '\x80';
#elif UNX
//          return '\xA4';      // #56121# 0xA4 would be correct for iso-8859-15
            return '\x80';      // but Windows code for the converted TrueType fonts
#else
#error EuroSymbol is what?
            return '\x80';
#endif
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
