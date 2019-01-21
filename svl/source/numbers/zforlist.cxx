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

#include <sal/config.h>

#include <sal/log.hxx>
#include <svl/zforlist.hxx>
#include <svl/currencytable.hxx>

#include <comphelper/string.hxx>
#include <tools/debug.hxx>
#include <unotools/charclass.hxx>
#include <unotools/configmgr.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <com/sun/star/i18n/KNumberFormatUsage.hpp>
#include <com/sun/star/i18n/KNumberFormatType.hpp>
#include <com/sun/star/i18n/FormatElement.hpp>
#include <com/sun/star/i18n/Currency2.hpp>
#include <com/sun/star/i18n/NumberFormatCode.hpp>
#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#include <com/sun/star/i18n/NumberFormatMapper.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/misccfg.hxx>


#include <osl/mutex.hxx>

#include "zforscan.hxx"
#include "zforfind.hxx"
#include <svl/zformat.hxx>

#include <unotools/syslocaleoptions.hxx>
#include <unotools/digitgroupingiterator.hxx>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>

#include <math.h>
#include <limits>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::std;

// Constants for type offsets per Country/Language (CL)
#define ZF_STANDARD              0
#define ZF_STANDARD_PERCENT     10
#define ZF_STANDARD_CURRENCY    20
#define ZF_STANDARD_DATE        30
#define ZF_STANDARD_TIME        40
#define ZF_STANDARD_DATETIME    50
#define ZF_STANDARD_SCIENTIFIC  60
#define ZF_STANDARD_FRACTION    65

// Additional builtin formats not fitting into the first 10 of a category (TLOT
// = The Legacy Of Templin; unfortunately TLOT intended only 10 builtin formats
// per category, more would overwrite the next category):
#define ZF_STANDARD_NEWEXTENDED_DATE_SYS_DMMMYYYY 75
#define ZF_STANDARD_NEWEXTENDED_DATE_SYS_DMMMMYYYY 76
#define ZF_STANDARD_NEWEXTENDED_DATE_SYS_NNDMMMYY 77
#define ZF_STANDARD_NEWEXTENDED_DATE_SYS_NNDMMMMYYYY 78
#define ZF_STANDARD_NEWEXTENDED_DATE_SYS_NNNNDMMMMYYYY 79
#define ZF_STANDARD_NEWEXTENDED_DATE_DIN_DMMMYYYY 80
#define ZF_STANDARD_NEWEXTENDED_DATE_DIN_DMMMMYYYY 81
#define ZF_STANDARD_NEWEXTENDED_DATE_DIN_MMDD 82
#define ZF_STANDARD_NEWEXTENDED_DATE_DIN_YYMMDD 83
#define ZF_STANDARD_NEWEXTENDED_DATE_DIN_YYYYMMDD 84
#define ZF_STANDARD_NEWEXTENDED_DATE_WW 85

#define ZF_STANDARD_LOGICAL     SV_MAX_COUNT_STANDARD_FORMATS-1 //  99
#define ZF_STANDARD_TEXT        SV_MAX_COUNT_STANDARD_FORMATS   // 100

static_assert( ZF_STANDARD_TEXT == NF_STANDARD_FORMAT_TEXT, "definition mismatch" );

/* Locale that is set if an unknown locale (from another system) is loaded of
 * legacy documents. Can not be SYSTEM because else, for example, a German "DM"
 * (old currency) is recognized as a date (#53155#). */
#define UNKNOWN_SUBSTITUTE      LANGUAGE_ENGLISH_US

// Same order as in include/svl/zforlist.hxx enum NfIndexTableOffset
static sal_uInt32 const indexTable[NF_INDEX_TABLE_ENTRIES] = {
    ZF_STANDARD, // NF_NUMBER_STANDARD
    ZF_STANDARD + 1, // NF_NUMBER_INT
    ZF_STANDARD + 2, // NF_NUMBER_DEC2
    ZF_STANDARD + 3, // NF_NUMBER_1000INT
    ZF_STANDARD + 4, // NF_NUMBER_1000DEC2
    ZF_STANDARD + 5, // NF_NUMBER_SYSTEM
    ZF_STANDARD_SCIENTIFIC, // NF_SCIENTIFIC_000E000
    ZF_STANDARD_SCIENTIFIC + 1, // NF_SCIENTIFIC_000E00
    ZF_STANDARD_PERCENT, // NF_PERCENT_INT
    ZF_STANDARD_PERCENT + 1, // NF_PERCENT_DEC2
    ZF_STANDARD_FRACTION, // NF_FRACTION_1D
    ZF_STANDARD_FRACTION + 1, // NF_FRACTION_2D
    ZF_STANDARD_CURRENCY, // NF_CURRENCY_1000INT
    ZF_STANDARD_CURRENCY + 1, // NF_CURRENCY_1000DEC2
    ZF_STANDARD_CURRENCY + 2, // NF_CURRENCY_1000INT_RED
    ZF_STANDARD_CURRENCY + 3, // NF_CURRENCY_1000DEC2_RED
    ZF_STANDARD_CURRENCY + 4, // NF_CURRENCY_1000DEC2_CCC
    ZF_STANDARD_CURRENCY + 5, // NF_CURRENCY_1000DEC2_DASHED
    ZF_STANDARD_DATE, // NF_DATE_SYSTEM_SHORT
    ZF_STANDARD_DATE + 8, // NF_DATE_SYSTEM_LONG
    ZF_STANDARD_DATE + 7, // NF_DATE_SYS_DDMMYY
    ZF_STANDARD_DATE + 6, // NF_DATE_SYS_DDMMYYYY
    ZF_STANDARD_DATE + 9, // NF_DATE_SYS_DMMMYY
    ZF_STANDARD_NEWEXTENDED_DATE_SYS_DMMMYYYY, // NF_DATE_SYS_DMMMYYYY
    ZF_STANDARD_NEWEXTENDED_DATE_DIN_DMMMYYYY, // NF_DATE_DIN_DMMMYYYY
    ZF_STANDARD_NEWEXTENDED_DATE_SYS_DMMMMYYYY, // NF_DATE_SYS_DMMMMYYYY
    ZF_STANDARD_NEWEXTENDED_DATE_DIN_DMMMMYYYY, // NF_DATE_DIN_DMMMMYYYY
    ZF_STANDARD_NEWEXTENDED_DATE_SYS_NNDMMMYY, // NF_DATE_SYS_NNDMMMYY
    ZF_STANDARD_DATE + 1, // NF_DATE_DEF_NNDDMMMYY
    ZF_STANDARD_NEWEXTENDED_DATE_SYS_NNDMMMMYYYY, // NF_DATE_SYS_NNDMMMMYYYY
    ZF_STANDARD_NEWEXTENDED_DATE_SYS_NNNNDMMMMYYYY, // NF_DATE_SYS_NNNNDMMMMYYYY
    ZF_STANDARD_NEWEXTENDED_DATE_DIN_MMDD, // NF_DATE_DIN_MMDD
    ZF_STANDARD_NEWEXTENDED_DATE_DIN_YYMMDD, // NF_DATE_DIN_YYMMDD
    ZF_STANDARD_NEWEXTENDED_DATE_DIN_YYYYMMDD, // NF_DATE_DIN_YYYYMMDD
    ZF_STANDARD_DATE + 2, // NF_DATE_SYS_MMYY
    ZF_STANDARD_DATE + 3, // NF_DATE_SYS_DDMMM
    ZF_STANDARD_DATE + 4, // NF_DATE_MMMM
    ZF_STANDARD_DATE + 5, // NF_DATE_QQJJ
    ZF_STANDARD_NEWEXTENDED_DATE_WW, // NF_DATE_WW
    ZF_STANDARD_TIME, // NF_TIME_HHMM
    ZF_STANDARD_TIME + 1, // NF_TIME_HHMMSS
    ZF_STANDARD_TIME + 2, // NF_TIME_HHMMAMPM
    ZF_STANDARD_TIME + 3, // NF_TIME_HHMMSSAMPM
    ZF_STANDARD_TIME + 4, // NF_TIME_HH_MMSS
    ZF_STANDARD_TIME + 5, // NF_TIME_MMSS00
    ZF_STANDARD_TIME + 6, // NF_TIME_HH_MMSS00
    ZF_STANDARD_DATETIME, // NF_DATETIME_SYSTEM_SHORT_HHMM
    ZF_STANDARD_DATETIME + 1, // NF_DATETIME_SYS_DDMMYYYY_HHMMSS
    ZF_STANDARD_LOGICAL, // NF_BOOLEAN
    ZF_STANDARD_TEXT, // NF_TEXT
    ZF_STANDARD_FRACTION + 2, // NF_FRACTION_3D
    ZF_STANDARD_FRACTION + 3, // NF_FRACTION_2
    ZF_STANDARD_FRACTION + 4, // NF_FRACTION_4
    ZF_STANDARD_FRACTION + 5, // NF_FRACTION_8
    ZF_STANDARD_FRACTION + 6, // NF_FRACTION_16
    ZF_STANDARD_FRACTION + 7, // NF_FRACTION_10
    ZF_STANDARD_FRACTION + 8, // NF_FRACTION_100
    ZF_STANDARD_DATETIME + 2 // NF_DATETIME_ISO_YYYYMMDD_HHMMSS
};

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
    virtual                 ~SvNumberFormatterRegistry_Impl() override;

    void                    Insert( SvNumberFormatter* pThis )
                                { aFormatters.push_back( pThis ); }

    void                    Remove( SvNumberFormatter const * pThis );

    size_t                  Count() const
                                { return aFormatters.size(); }

    virtual void            ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints ) override;
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


void SvNumberFormatterRegistry_Impl::Remove( SvNumberFormatter const * pThis )
{
    auto it = std::find(aFormatters.begin(), aFormatters.end(), pThis);
    if (it != aFormatters.end())
        aFormatters.erase( it );
}

void SvNumberFormatterRegistry_Impl::ConfigurationChanged( utl::ConfigurationBroadcaster*,
                                                           ConfigurationHints nHint)
{
    ::osl::MutexGuard aGuard( SvNumberFormatter::GetGlobalMutex() );

    if ( nHint & ConfigurationHints::Locale )
    {
        for(SvNumberFormatter* pFormatter : aFormatters)
            pFormatter->ReplaceSystemCL( eSysLanguage );
        eSysLanguage = MsLangId::getRealLanguage( LANGUAGE_SYSTEM );
    }
    if ( nHint & ConfigurationHints::Currency )
    {
        for(SvNumberFormatter* pFormatter : aFormatters)
            pFormatter->ResetDefaultSystemCurrency();
    }
    if ( nHint & ConfigurationHints::DatePatterns )
    {
        for(SvNumberFormatter* pFormatter : aFormatters)
            pFormatter->InvalidateDateAcceptancePatterns();
    }
}


SvNumberFormatterRegistry_Impl* SvNumberFormatter::pFormatterRegistry = nullptr;
volatile bool SvNumberFormatter::bCurrencyTableInitialized = false;
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

const sal_uInt16 SvNumberFormatter::UNLIMITED_PRECISION   = ::std::numeric_limits<sal_uInt16>::max();
const sal_uInt16 SvNumberFormatter::INPUTSTRING_PRECISION = ::std::numeric_limits<sal_uInt16>::max()-1;

SvNumberFormatter::SvNumberFormatter( const Reference< XComponentContext >& rxContext,
                                      LanguageType eLang )
    : m_xContext( rxContext )
    , maLanguageTag( eLang)
{
    ImpConstruct( eLang );
}

SvNumberFormatter::~SvNumberFormatter()
{
    {
        ::osl::MutexGuard aGuard( GetGlobalMutex() );
        pFormatterRegistry->Remove( this );
        if ( !pFormatterRegistry->Count() )
        {
            delete pFormatterRegistry;
            pFormatterRegistry = nullptr;
        }
    }

    aFTable.clear();
    ClearMergeTable();
}


void SvNumberFormatter::ImpConstruct( LanguageType eLang )
{
    if ( eLang == LANGUAGE_DONTKNOW )
    {
        eLang = UNKNOWN_SUBSTITUTE;
    }
    IniLnge = eLang;
    ActLnge = eLang;
    eEvalDateFormat = NF_EVALDATEFORMAT_INTL;
    nDefaultSystemCurrencyFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;

    maLanguageTag.reset( eLang );
    pCharClass.reset( new CharClass( m_xContext, maLanguageTag ) );
    xLocaleData.init( m_xContext, maLanguageTag );
    xCalendar.init( m_xContext, maLanguageTag.getLocale() );
    xTransliteration.init( m_xContext, eLang );
    xNatNum.init( m_xContext );

    // cached locale data items
    const LocaleDataWrapper* pLoc = GetLocaleData();
    aDecimalSep = pLoc->getNumDecimalSep();
    aDecimalSepAlt = pLoc->getNumDecimalSepAlt();
    aThousandSep = pLoc->getNumThousandSep();
    aDateSep = pLoc->getDateSep();

    pStringScanner.reset( new ImpSvNumberInputScan( this ) );
    pFormatScanner.reset( new ImpSvNumberformatScan( this ) );
    pFormatTable = nullptr;
    MaxCLOffset = 0;
    ImpGenerateFormats( 0, false );     // 0 .. 999 for initialized language formats
    pMergeTable = nullptr;
    bNoZero = false;

    ::osl::MutexGuard aGuard( GetGlobalMutex() );
    GetFormatterRegistry().Insert( this );
}


void SvNumberFormatter::ChangeIntl(LanguageType eLnge)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (ActLnge == eLnge)
        return;

    ActLnge = eLnge;

    maLanguageTag.reset( eLnge );
    pCharClass->setLanguageTag( maLanguageTag );
    xLocaleData.changeLocale( maLanguageTag );
    xCalendar.changeLocale( maLanguageTag.getLocale() );
    xTransliteration.changeLocale( eLnge );

    // cached locale data items, initialize BEFORE calling ChangeIntl below
    const LocaleDataWrapper* pLoc = GetLocaleData();
    aDecimalSep = pLoc->getNumDecimalSep();
    aDecimalSepAlt = pLoc->getNumDecimalSepAlt();
    aThousandSep = pLoc->getNumThousandSep();
    aDateSep = pLoc->getDateSep();

    pFormatScanner->ChangeIntl();
    pStringScanner->ChangeIntl();
}


// static
::osl::Mutex& SvNumberFormatter::GetGlobalMutex()
{
    // #i77768# Due to a static reference in the toolkit lib
    // we need a mutex that lives longer than the svl library.
    // Otherwise the dtor would use a destructed mutex!!
    static osl::Mutex* persistentMutex(new osl::Mutex);

    return *persistentMutex;
}


// static
SvNumberFormatterRegistry_Impl& SvNumberFormatter::GetFormatterRegistry()
{
    ::osl::MutexGuard aGuard( GetGlobalMutex() );
    if ( !pFormatterRegistry )
    {
        pFormatterRegistry = new SvNumberFormatterRegistry_Impl;
    }
    return *pFormatterRegistry;
}

void SvNumberFormatter::SetColorLink( const Link<sal_uInt16,Color*>& rColorTableCallBack )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    aColorLink = rColorTableCallBack;
}

Color* SvNumberFormatter::GetUserDefColor(sal_uInt16 nIndex)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if( aColorLink.IsSet() )
    {
        return aColorLink.Call(nIndex);
    }
    else
    {
        return nullptr;
    }
}

void SvNumberFormatter::ChangeNullDate(sal_uInt16 nDay,
                                       sal_uInt16 nMonth,
                                       sal_Int16 nYear)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    pFormatScanner->ChangeNullDate(nDay, nMonth, nYear);
    pStringScanner->ChangeNullDate(nDay, nMonth, nYear);
}

const Date& SvNumberFormatter::GetNullDate() const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    return pFormatScanner->GetNullDate();
}

void SvNumberFormatter::ChangeStandardPrec(short nPrec)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    pFormatScanner->ChangeStandardPrec(nPrec);
}

void SvNumberFormatter::SetNoZero(bool bNZ)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    bNoZero = bNZ;
}

sal_uInt16 SvNumberFormatter::GetStandardPrec() const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    return pFormatScanner->GetStandardPrec();
}

bool SvNumberFormatter::GetNoZero() const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    return bNoZero;
}

void SvNumberFormatter::ReplaceSystemCL( LanguageType eOldLanguage )
{
    sal_uInt32 nCLOffset = ImpGetCLOffset( LANGUAGE_SYSTEM );
    if ( nCLOffset > MaxCLOffset )
    {
        return ;    // no SYSTEM entries to replace
    }
    const sal_uInt32 nMaxBuiltin = nCLOffset + SV_MAX_COUNT_STANDARD_FORMATS;
    const sal_uInt32 nNextCL = nCLOffset + SV_COUNTRY_LANGUAGE_OFFSET;
    sal_uInt32 nKey;

    // remove old builtin formats
    auto it = aFTable.find( nCLOffset );
    while ( it != aFTable.end() && (nKey = it->first) >= nCLOffset && nKey <= nMaxBuiltin )
    {
        it = aFTable.erase(it);
    }

    // move additional and user defined to temporary table
    SvNumberFormatTable aOldTable;
    while ( it != aFTable.end() && (nKey = it->first) >= nCLOffset && nKey < nNextCL )
    {
        aOldTable[ nKey ] = it->second.release();
        it = aFTable.erase(it);
    }

    // generate new old builtin formats
    // reset ActLnge otherwise ChangeIntl() wouldn't switch if already LANGUAGE_SYSTEM
    ActLnge = LANGUAGE_DONTKNOW;
    ChangeIntl( LANGUAGE_SYSTEM );
    ImpGenerateFormats( nCLOffset, true );

    // convert additional and user defined from old system to new system
    SvNumberformat* pStdFormat = GetFormatEntry( nCLOffset + ZF_STANDARD );
    sal_uInt32 nLastKey = nMaxBuiltin;
    pFormatScanner->SetConvertMode( eOldLanguage, LANGUAGE_SYSTEM, true , true);
    while ( !aOldTable.empty() )
    {
        nKey = aOldTable.begin()->first;
        if ( nLastKey < nKey )
        {
            nLastKey = nKey;
        }
        std::unique_ptr<SvNumberformat> pOldEntry(aOldTable.begin()->second);
        aOldTable.erase( nKey );
        OUString aString( pOldEntry->GetFormatstring() );

        // Same as PutEntry() but assures key position even if format code is
        // a duplicate. Also won't mix up any LastInsertKey.
        ChangeIntl( eOldLanguage );
        LanguageType eLge = eOldLanguage;   // ConvertMode changes this
        bool bCheck = false;
        sal_Int32 nCheckPos = -1;
        std::unique_ptr<SvNumberformat> pNewEntry(new SvNumberformat( aString, pFormatScanner.get(),
                                                                      pStringScanner.get(), nCheckPos, eLge ));
        if ( nCheckPos == 0 )
        {
            SvNumFormatType eCheckType = pNewEntry->GetType();
            if ( eCheckType != SvNumFormatType::UNDEFINED )
            {
                pNewEntry->SetType( eCheckType | SvNumFormatType::DEFINED );
            }
            else
            {
                pNewEntry->SetType( SvNumFormatType::DEFINED );
            }

            if ( aFTable.emplace( nKey, std::move(pNewEntry) ).second )
            {
                bCheck = true;
            }
        }
        DBG_ASSERT( bCheck, "SvNumberFormatter::ReplaceSystemCL: couldn't convert" );
    }
    pFormatScanner->SetConvertMode(false);
    pStdFormat->SetLastInsertKey( sal_uInt16(nLastKey - nCLOffset), SvNumberformat::FormatterPrivateAccess() );

    // append new system additional formats
    css::uno::Reference< css::i18n::XNumberFormatCode > xNFC = i18n::NumberFormatMapper::create( m_xContext );
    ImpGenerateAdditionalFormats( nCLOffset, xNFC, true );
}

const css::uno::Reference<css::uno::XComponentContext>& SvNumberFormatter::GetComponentContext() const
{
    return m_xContext;
}

const ImpSvNumberformatScan* SvNumberFormatter::GetFormatScanner() const { return pFormatScanner.get(); }

const LanguageTag& SvNumberFormatter::GetLanguageTag() const { return maLanguageTag; }

const ::utl::TransliterationWrapper* SvNumberFormatter::GetTransliteration() const
{
    return xTransliteration.get();
}

const CharClass* SvNumberFormatter::GetCharClass() const { return pCharClass.get(); }

const LocaleDataWrapper* SvNumberFormatter::GetLocaleData() const { return xLocaleData.get(); }

CalendarWrapper* SvNumberFormatter::GetCalendar() const { return xCalendar.get(); }

const NativeNumberWrapper* SvNumberFormatter::GetNatNum() const { return xNatNum.get(); }

const OUString& SvNumberFormatter::GetNumDecimalSep() const { return aDecimalSep; }

const OUString& SvNumberFormatter::GetNumDecimalSepAlt() const { return aDecimalSepAlt; }

const OUString& SvNumberFormatter::GetNumThousandSep() const { return aThousandSep; }

const OUString& SvNumberFormatter::GetDateSep() const { return aDateSep; }

bool SvNumberFormatter::IsDecimalSep( const OUString& rStr ) const
{
    if (rStr == GetNumDecimalSep())
        return true;
    if (GetNumDecimalSepAlt().isEmpty())
        return false;
    return rStr == GetNumDecimalSepAlt();
}

bool SvNumberFormatter::IsTextFormat(sal_uInt32 F_Index) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    const SvNumberformat* pFormat = GetFormatEntry(F_Index);

    return pFormat && pFormat->IsTextFormat();
}

bool SvNumberFormatter::PutEntry(OUString& rString,
                                 sal_Int32& nCheckPos,
                                 SvNumFormatType& nType,
                                 sal_uInt32& nKey,      // format key
                                 LanguageType eLnge)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    nKey = 0;
    if (rString.isEmpty())                             // empty string
    {
        nCheckPos = 1;                                  // -> Error
        return false;
    }
    if (eLnge == LANGUAGE_DONTKNOW)
    {
        eLnge = IniLnge;
    }
    ChangeIntl(eLnge);                                  // change locale if necessary
    LanguageType eLge = eLnge;                          // non-const for ConvertMode
    bool bCheck = false;
    std::unique_ptr<SvNumberformat> p_Entry(new SvNumberformat(rString,
                                                               pFormatScanner.get(),
                                                               pStringScanner.get(),
                                                               nCheckPos,
                                                               eLge));

    if (nCheckPos == 0)                         // Format ok
    {                                           // Type comparison:
        SvNumFormatType eCheckType = p_Entry->GetType();
        if ( eCheckType != SvNumFormatType::UNDEFINED)
        {
            p_Entry->SetType(eCheckType | SvNumFormatType::DEFINED);
            nType = eCheckType;
        }
        else
        {
            p_Entry->SetType(SvNumFormatType::DEFINED);
            nType = SvNumFormatType::DEFINED;
        }

        sal_uInt32 CLOffset = ImpGenerateCL(eLge);  // create new standard formats if necessary

        nKey = ImpIsEntry(p_Entry->GetFormatstring(),CLOffset, eLge);
        if (nKey == NUMBERFORMAT_ENTRY_NOT_FOUND) // only in not yet present
        {
            SvNumberformat* pStdFormat = GetFormatEntry(CLOffset + ZF_STANDARD);
            sal_uInt32 nPos = CLOffset + pStdFormat->GetLastInsertKey( SvNumberformat::FormatterPrivateAccess() );
            if (nPos+1 - CLOffset >= SV_COUNTRY_LANGUAGE_OFFSET)
            {
                SAL_WARN( "svl.numbers", "SvNumberFormatter::PutEntry: too many formats for CL");
            }
            else if (!aFTable.emplace( nPos+1, std::move(p_Entry)).second)
            {
                SAL_WARN( "svl.numbers", "SvNumberFormatter::PutEntry: dup position");
            }
            else
            {
                bCheck = true;
                nKey = nPos+1;
                pStdFormat->SetLastInsertKey(static_cast<sal_uInt16>(nKey-CLOffset), SvNumberformat::FormatterPrivateAccess());
            }
        }
    }
    return bCheck;
}

bool SvNumberFormatter::PutandConvertEntry(OUString& rString,
                                           sal_Int32& nCheckPos,
                                           SvNumFormatType& nType,
                                           sal_uInt32& nKey,
                                           LanguageType eLnge,
                                           LanguageType eNewLnge,
                                           bool bConvertDateOrder )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    bool bRes;
    if (eNewLnge == LANGUAGE_DONTKNOW)
    {
        eNewLnge = IniLnge;
    }
    pFormatScanner->SetConvertMode(eLnge, eNewLnge, false, bConvertDateOrder);
    bRes = PutEntry(rString, nCheckPos, nType, nKey, eLnge);
    pFormatScanner->SetConvertMode(false);
    return bRes;
}

bool SvNumberFormatter::PutandConvertEntrySystem(OUString& rString,
                                                 sal_Int32& nCheckPos,
                                                 SvNumFormatType& nType,
                                                 sal_uInt32& nKey,
                                                 LanguageType eLnge,
                                                 LanguageType eNewLnge)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    bool bRes;
    if (eNewLnge == LANGUAGE_DONTKNOW)
    {
        eNewLnge = IniLnge;
    }
    pFormatScanner->SetConvertMode(eLnge, eNewLnge, true, true);
    bRes = PutEntry(rString, nCheckPos, nType, nKey, eLnge);
    pFormatScanner->SetConvertMode(false);
    return bRes;
}

sal_uInt32 SvNumberFormatter::GetIndexPuttingAndConverting( OUString & rString, LanguageType eLnge,
                                                            LanguageType eSysLnge, SvNumFormatType & rType,
                                                            bool & rNewInserted, sal_Int32 & rCheckPos )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    sal_uInt32 nKey = NUMBERFORMAT_ENTRY_NOT_FOUND;
    rNewInserted = false;
    rCheckPos = 0;

    // #62389# empty format string (of Writer) => General standard format
    if (rString.isEmpty())
    {
        // nothing
    }
    else if (eLnge == LANGUAGE_SYSTEM && eSysLnge != SvtSysLocale().GetLanguageTag().getLanguageType())
    {
        sal_uInt32 nOrig = GetEntryKey( rString, eSysLnge );
        if (nOrig == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            nKey = nOrig;   // none available, maybe user-defined
        }
        else
        {
            nKey = GetFormatForLanguageIfBuiltIn( nOrig, SvtSysLocale().GetLanguageTag().getLanguageType() );
        }
        if (nKey == nOrig)
        {
            // Not a builtin format, convert.
            // The format code string may get modified and adapted to the real
            // language and wouldn't match eSysLnge anymore, do that on a copy.
            OUString aTmp( rString);
            rNewInserted = PutandConvertEntrySystem( aTmp, rCheckPos, rType,
                                                     nKey, eLnge, SvtSysLocale().GetLanguageTag().getLanguageType());
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
    {
        nKey = GetStandardIndex( eLnge);
    }
    rType = GetType( nKey);
    // Convert any (!) old "automatic" currency format to new fixed currency
    // default format.
    if (rType & SvNumFormatType::CURRENCY)
    {
        const SvNumberformat* pFormat = GetEntry( nKey);
        if (!pFormat->HasNewCurrency())
        {
            if (rNewInserted)
            {
                DeleteEntry( nKey);     // don't leave trails of rubbish
                rNewInserted = false;
            }
            nKey = GetStandardFormat( SvNumFormatType::CURRENCY, eLnge);
        }
    }
    return nKey;
}

void SvNumberFormatter::DeleteEntry(sal_uInt32 nKey)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    aFTable.erase(nKey);
}

void SvNumberFormatter::GetUsedLanguages( std::vector<LanguageType>& rList )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    rList.clear();

    sal_uInt32 nOffset = 0;
    while (nOffset <= MaxCLOffset)
    {
        SvNumberformat* pFormat = GetFormatEntry(nOffset);
        if (pFormat)
        {
            rList.push_back( pFormat->GetLanguage() );
        }
        nOffset += SV_COUNTRY_LANGUAGE_OFFSET;
    }
}


void SvNumberFormatter::FillKeywordTable( NfKeywordTable& rKeywords,
                                          LanguageType eLang )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    ChangeIntl( eLang );
    const NfKeywordTable & rTable = pFormatScanner->GetKeywords();
    for ( sal_uInt16 i = 0; i < NF_KEYWORD_ENTRIES_COUNT; ++i )
    {
        rKeywords[i] = rTable[i];
    }
}


void SvNumberFormatter::FillKeywordTableForExcel( NfKeywordTable& rKeywords )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    FillKeywordTable( rKeywords, LANGUAGE_ENGLISH_US );

    // Replace upper case "GENERAL" with proper case "General".
    rKeywords[ NF_KEY_GENERAL ] = GetStandardName( LANGUAGE_ENGLISH_US );
    // Remap codes unknown to Excel.
    rKeywords[ NF_KEY_NN ] = "DDD";
    rKeywords[ NF_KEY_NNN ] = "DDDD";
    // NNNN gets a separator appended in SvNumberformat::GetMappedFormatString()
    rKeywords[ NF_KEY_NNNN ] = "DDDD";
    // Export the Thai T NatNum modifier.
    rKeywords[ NF_KEY_THAI_T ] = "T";
}


OUString SvNumberFormatter::GetFormatStringForExcel( sal_uInt32 nKey, const NfKeywordTable& rKeywords,
        SvNumberFormatter& rTempFormatter ) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    OUString aFormatStr;
    if (const SvNumberformat* pEntry = GetEntry( nKey))
    {
        if (pEntry->GetType() == SvNumFormatType::LOGICAL)
        {
            // Build Boolean number format, which needs non-zero and zero
            // subformat codes with TRUE and FALSE strings.
            Color* pColor = nullptr;
            OUString aTemp;
            const_cast< SvNumberformat* >( pEntry )->GetOutputString( 1.0, aTemp, &pColor );
            aFormatStr += "\"" + aTemp + "\";\"" + aTemp + "\";\"";
            const_cast< SvNumberformat* >( pEntry )->GetOutputString( 0.0, aTemp, &pColor );
            aFormatStr += aTemp + "\"";
        }
        else
        {
            bool bSystemLanguage = false;
            LanguageType nLang = pEntry->GetLanguage();
            if (nLang == LANGUAGE_SYSTEM)
            {
                bSystemLanguage = true;
                nLang = SvtSysLocale().GetLanguageTag().getLanguageType();
            }
            if (nLang != LANGUAGE_ENGLISH_US)
            {
                sal_Int32 nCheckPos;
                SvNumFormatType nType = SvNumFormatType::DEFINED;
                sal_uInt32 nTempKey;
                OUString aTemp( pEntry->GetFormatstring());
                rTempFormatter.PutandConvertEntry( aTemp, nCheckPos, nType, nTempKey, nLang, LANGUAGE_ENGLISH_US, false);
                SAL_WARN_IF( nCheckPos != 0, "svl.numbers",
                        "SvNumberFormatter::GetFormatStringForExcel - format code not convertible");
                if (nTempKey != NUMBERFORMAT_ENTRY_NOT_FOUND)
                    pEntry = rTempFormatter.GetEntry( nTempKey);
            }

            if (pEntry)
            {
                // GetLocaleData() returns the current locale's data, so switch
                // before (which doesn't do anything if it was the same locale
                // already).
                rTempFormatter.ChangeIntl( LANGUAGE_ENGLISH_US);
                aFormatStr = pEntry->GetMappedFormatstring( rKeywords, *rTempFormatter.GetLocaleData(), nLang,
                        bSystemLanguage);
            }
        }
    }
    else
    {
        SAL_WARN("svl.numbers","SvNumberFormatter::GetFormatStringForExcel - format not found: " << nKey);
    }

    if (aFormatStr.isEmpty())
        aFormatStr = "General";
    return aFormatStr;
}


OUString SvNumberFormatter::GetKeyword( LanguageType eLnge, sal_uInt16 nIndex )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    ChangeIntl(eLnge);
    const NfKeywordTable & rTable = pFormatScanner->GetKeywords();
    if ( nIndex < NF_KEYWORD_ENTRIES_COUNT )
    {
        return rTable[nIndex];
    }
    SAL_WARN( "svl.numbers", "GetKeyword: invalid index");
    return OUString();
}


OUString SvNumberFormatter::GetStandardName( LanguageType eLnge )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
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
        {
            return nOffset;
        }
        nOffset += SV_COUNTRY_LANGUAGE_OFFSET;
    }
    return nOffset;
}

sal_uInt32 SvNumberFormatter::ImpIsEntry(const OUString& rString,
                                         sal_uInt32 nCLOffset,
                                         LanguageType eLnge)
{
    sal_uInt32 res = NUMBERFORMAT_ENTRY_NOT_FOUND;
    auto it = aFTable.find( nCLOffset);
    while ( res == NUMBERFORMAT_ENTRY_NOT_FOUND &&
            it != aFTable.end() && it->second->GetLanguage() == eLnge )
    {
        if ( rString == it->second->GetFormatstring() )
        {
            res = it->first;
        }
        else
        {
            ++it;
        }
    }
    return res;
}


SvNumberFormatTable& SvNumberFormatter::GetFirstEntryTable(
                                                      SvNumFormatType& eType,
                                                      sal_uInt32& FIndex,
                                                      LanguageType& rLnge)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    SvNumFormatType eTypetmp = eType;
    if (eType == SvNumFormatType::ALL)                  // empty cell or don't care
    {
        rLnge = IniLnge;
    }
    else
    {
        SvNumberformat* pFormat = GetFormatEntry(FIndex);
        if (!pFormat)
        {
            rLnge = IniLnge;
            eType = SvNumFormatType::ALL;
            eTypetmp = eType;
        }
        else
        {
            rLnge = pFormat->GetLanguage();
            eType = pFormat->GetMaskedType();
            if (eType == SvNumFormatType::ALL)
            {
                eType = SvNumFormatType::DEFINED;
                eTypetmp = eType;
            }
            else if (eType == SvNumFormatType::DATETIME)
            {
                eTypetmp = eType;
                eType = SvNumFormatType::DATE;
            }
            else
            {
                eTypetmp = eType;
            }
        }
    }
    ChangeIntl(rLnge);
    return GetEntryTable(eTypetmp, FIndex, rLnge);
}

sal_uInt32 SvNumberFormatter::ImpGenerateCL( LanguageType eLnge )
{
    ChangeIntl(eLnge);
    sal_uInt32 CLOffset = ImpGetCLOffset(ActLnge);
    if (CLOffset > MaxCLOffset)
    {
        // new CL combination
        if (LocaleDataWrapper::areChecksEnabled())
        {
            const LanguageTag& rLoadedLocale = xLocaleData->getLoadedLanguageTag();
            if ( !rLoadedLocale.equals( maLanguageTag ) )
            {
                OUString const aMsg("SvNumberFormatter::ImpGenerateCL: locales don't match:");
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
                    OUStringBuffer aDupes;
                    for ( sal_Int32 i = 0; i < xSeq.getLength(); i++ )
                    {
                        if ( i != j && xSeq[i].formatIndex == nIdx )
                        {
                            aDupes.append(OUString::number( i ));
                            aDupes.append("(");
                            aDupes.append(xSeq[i].formatKey);
                            aDupes.append( ") ");
                        }
                    }
                    if ( !aDupes.isEmpty() )
                    {
                        OUString aMsg = "XML locale data FormatElement formatindex dupe: "
                                      + OUString::number(nIdx)
                                      + "\nFormatElements: "
                                      + OUString::number( j )
                                      + "("
                                      + xSeq[j].formatKey
                                      + ") "
                                      + aDupes.makeStringAndClear();
                        LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg ));
                    }
                }
            }
        }

        MaxCLOffset += SV_COUNTRY_LANGUAGE_OFFSET;
        ImpGenerateFormats( MaxCLOffset, false/*bNoAdditionalFormats*/ );
        CLOffset = MaxCLOffset;
    }
    return CLOffset;
}

SvNumberFormatTable& SvNumberFormatter::ChangeCL(SvNumFormatType eType,
                                                 sal_uInt32& FIndex,
                                                 LanguageType eLnge)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    ImpGenerateCL(eLnge);
    return GetEntryTable(eType, FIndex, ActLnge);
}

SvNumberFormatTable& SvNumberFormatter::GetEntryTable(
                                                    SvNumFormatType eType,
                                                    sal_uInt32& FIndex,
                                                    LanguageType eLnge)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if ( pFormatTable )
    {
        pFormatTable->clear();
    }
    else
    {
        pFormatTable.reset( new SvNumberFormatTable );
    }
    ChangeIntl(eLnge);
    sal_uInt32 CLOffset = ImpGetCLOffset(ActLnge);

    // Might generate and insert a default format for the given type
    // (e.g. currency) => has to be done before collecting formats.
    sal_uInt32 nDefaultIndex = GetStandardFormat( eType, ActLnge );

    auto it = aFTable.find( CLOffset);

    if (eType == SvNumFormatType::ALL)
    {
        while (it != aFTable.end() && it->second->GetLanguage() == ActLnge)
        {   // copy all entries to output table
            (*pFormatTable)[ it->first ] = it->second.get();
            ++it;
        }
    }
    else
    {
        while (it != aFTable.end() && it->second->GetLanguage() == ActLnge)
        {   // copy entries of queried type to output table
            if ((it->second->GetType()) & eType)
                (*pFormatTable)[ it->first ] = it->second.get();
            ++it;
        }
    }
    if ( !pFormatTable->empty() )
    {   // select default if queried format doesn't exist or queried type or
        // language differ from existing format
        SvNumberformat* pEntry = GetFormatEntry(FIndex);
        if ( !pEntry || !(pEntry->GetType() & eType) || pEntry->GetLanguage() != ActLnge )
        {
            FIndex = nDefaultIndex;
        }
    }
    return *pFormatTable;
}

bool SvNumberFormatter::IsNumberFormat(const OUString& sString,
                                       sal_uInt32& F_Index,
                                       double& fOutNumber)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );

    SvNumFormatType FType;
    // For the 0 General format directly use the init/system locale and avoid
    // all overhead that is associated with a format passed to the scanner.
    const SvNumberformat* pFormat = (F_Index == 0 ? nullptr : ImpSubstituteEntry( GetFormatEntry(F_Index)));
    if (!pFormat)
    {
        ChangeIntl(IniLnge);
        FType = SvNumFormatType::NUMBER;
    }
    else
    {
        FType = pFormat->GetMaskedType();
        if (FType == SvNumFormatType::ALL)
        {
            FType = SvNumFormatType::DEFINED;
        }
        ChangeIntl(pFormat->GetLanguage());
        // Avoid scanner overhead with the General format of any locale.
        // These are never substituted above so safe to ignore.
        if ((F_Index % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        {
            assert(FType == SvNumFormatType::NUMBER);
            pFormat = nullptr;
        }
    }

    bool res;
    SvNumFormatType RType = FType;
    if (RType == SvNumFormatType::TEXT)
    {
        res = false;        // type text preset => no conversion to number
    }
    else
    {
        res = pStringScanner->IsNumberFormat(sString, RType, fOutNumber, pFormat);
    }
    if (res && !IsCompatible(FType, RType))     // non-matching type
    {
        switch ( RType )
        {
        case SvNumFormatType::DATE :
            // Preserve ISO 8601 input.
            if (pStringScanner->CanForceToIso8601( DateOrder::Invalid))
            {
                F_Index = GetFormatIndex( NF_DATE_DIN_YYYYMMDD, ActLnge );
            }
            else
            {
                F_Index = GetStandardFormat( RType, ActLnge );
            }
            break;
        case SvNumFormatType::TIME :
            if ( pStringScanner->GetDecPos() )
            {
                // 100th seconds
                if ( pStringScanner->GetNumericsCount() > 3 || fOutNumber < 0.0 )
                {
                    F_Index = GetFormatIndex( NF_TIME_HH_MMSS00, ActLnge );
                }
                else
                {
                    F_Index = GetFormatIndex( NF_TIME_MMSS00, ActLnge );
                }
            }
            else if ( fOutNumber >= 1.0 || fOutNumber < 0.0 )
            {
                F_Index = GetFormatIndex( NF_TIME_HH_MMSS, ActLnge );
            }
            else
            {
                F_Index = GetStandardFormat( RType, ActLnge );
            }
            break;
        default:
            F_Index = GetStandardFormat( RType, ActLnge );
        }
    }
    return res;
}

LanguageType SvNumberFormatter::GetLanguage() const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    return IniLnge;
}

// static
bool SvNumberFormatter::IsCompatible(SvNumFormatType eOldType, SvNumFormatType eNewType)
{
    if (eOldType == eNewType)
    {
        return true;
    }
    else if (eOldType == SvNumFormatType::DEFINED)
    {
        return true;
    }
    else
    {
        switch (eNewType)
        {
        case SvNumFormatType::NUMBER:
            switch (eOldType)
            {
            case SvNumFormatType::PERCENT:
            case SvNumFormatType::CURRENCY:
            case SvNumFormatType::SCIENTIFIC:
            case SvNumFormatType::FRACTION:
            case SvNumFormatType::DEFINED:
                return true;
            case SvNumFormatType::LOGICAL:
            default:
                return false;
            }
            break;
        case SvNumFormatType::DATE:
            switch (eOldType)
            {
            case SvNumFormatType::DATETIME:
                return true;
            default:
                return false;
            }
            break;
        case SvNumFormatType::TIME:
            switch (eOldType)
            {
            case SvNumFormatType::DATETIME:
                return true;
            default:
                return false;
            }
            break;
        case SvNumFormatType::DATETIME:
            switch (eOldType)
            {
            case SvNumFormatType::TIME:
            case SvNumFormatType::DATE:
                return true;
            default:
                return false;
            }
            break;
        default:
            return false;
        }
    }
}


sal_uInt32 SvNumberFormatter::ImpGetDefaultFormat( SvNumFormatType nType )
{
    sal_uInt32 CLOffset = ImpGetCLOffset( ActLnge );
    sal_uInt32 nSearch;
    switch( nType )
    {
    case SvNumFormatType::DATE:
        nSearch = CLOffset + ZF_STANDARD_DATE;
        break;
    case SvNumFormatType::TIME:
        nSearch = CLOffset + ZF_STANDARD_TIME;
        break;
    case SvNumFormatType::DATETIME:
        nSearch = CLOffset + ZF_STANDARD_DATETIME;
        break;
    case SvNumFormatType::PERCENT:
        nSearch = CLOffset + ZF_STANDARD_PERCENT;
        break;
    case SvNumFormatType::SCIENTIFIC:
        nSearch = CLOffset + ZF_STANDARD_SCIENTIFIC;
        break;
    default:
        nSearch = CLOffset + ZF_STANDARD;
    }

    DefaultFormatKeysMap::const_iterator it = aDefaultFormatKeys.find( nSearch);
    sal_uInt32 nDefaultFormat = (it != aDefaultFormatKeys.end() ?
                                 it->second : NUMBERFORMAT_ENTRY_NOT_FOUND);
    if ( nDefaultFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        // look for a defined standard
        sal_uInt32 nStopKey = CLOffset + SV_COUNTRY_LANGUAGE_OFFSET;
        sal_uInt32 nKey(0);
        auto it2 = aFTable.find( CLOffset );
        while ( it2 != aFTable.end() && (nKey = it2->first ) >= CLOffset && nKey < nStopKey )
        {
            const SvNumberformat* pEntry = it2->second.get();
            if ( pEntry->IsStandard() && (pEntry->GetMaskedType() == nType) )
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
            case SvNumFormatType::DATE:
                nDefaultFormat = CLOffset + ZF_STANDARD_DATE;
                break;
            case SvNumFormatType::TIME:
                nDefaultFormat = CLOffset + ZF_STANDARD_TIME+1;
                break;
            case SvNumFormatType::DATETIME:
                nDefaultFormat = CLOffset + ZF_STANDARD_DATETIME;
                break;
            case SvNumFormatType::PERCENT:
                nDefaultFormat = CLOffset + ZF_STANDARD_PERCENT+1;
                break;
            case SvNumFormatType::SCIENTIFIC:
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


sal_uInt32 SvNumberFormatter::GetStandardFormat( SvNumFormatType eType, LanguageType eLnge )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (eLnge == LANGUAGE_DONTKNOW)
    {
        eLnge = IniLnge;
    }
    sal_uInt32 CLOffset = ImpGenerateCL(eLnge);
    switch(eType)
    {
    case SvNumFormatType::CURRENCY:
        return ( eLnge == LANGUAGE_SYSTEM ) ? ImpGetDefaultSystemCurrencyFormat() : ImpGetDefaultCurrencyFormat();
    case SvNumFormatType::DATE:
    case SvNumFormatType::TIME:
    case SvNumFormatType::DATETIME:
    case SvNumFormatType::PERCENT:
    case SvNumFormatType::SCIENTIFIC:
        return ImpGetDefaultFormat( eType );
    case SvNumFormatType::FRACTION:
        return CLOffset + ZF_STANDARD_FRACTION;
    case SvNumFormatType::LOGICAL:
        return CLOffset + ZF_STANDARD_LOGICAL;
    case SvNumFormatType::TEXT:
        return CLOffset + ZF_STANDARD_TEXT;
    case SvNumFormatType::ALL:
    case SvNumFormatType::DEFINED:
    case SvNumFormatType::NUMBER:
    case SvNumFormatType::UNDEFINED:
    default:
        return CLOffset + ZF_STANDARD;
    }
}

bool SvNumberFormatter::IsSpecialStandardFormat( sal_uInt32 nFIndex,
                                                 LanguageType eLnge )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    return
        nFIndex == GetFormatIndex( NF_TIME_MMSS00, eLnge ) ||
        nFIndex == GetFormatIndex( NF_TIME_HH_MMSS00, eLnge ) ||
        nFIndex == GetFormatIndex( NF_TIME_HH_MMSS, eLnge )
        ;
}

sal_uInt32 SvNumberFormatter::GetStandardFormat( sal_uInt32 nFIndex, SvNumFormatType eType,
                                                 LanguageType eLnge )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if ( IsSpecialStandardFormat( nFIndex, eLnge ) )
        return nFIndex;
    else
        return GetStandardFormat( eType, eLnge );
}

sal_uInt32 SvNumberFormatter::GetTimeFormat( double fNumber, LanguageType eLnge )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
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
            return GetStandardFormat( SvNumFormatType::TIME, eLnge );
    }
}

sal_uInt32 SvNumberFormatter::GetStandardFormat( double fNumber, sal_uInt32 nFIndex,
                                                 SvNumFormatType eType, LanguageType eLnge )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if ( IsSpecialStandardFormat( nFIndex, eLnge ) )
        return nFIndex;

    switch( eType )
    {
        case SvNumFormatType::TIME :
            return GetTimeFormat( fNumber, eLnge);
        default:
            return GetStandardFormat( eType, eLnge );
    }
}

sal_uInt32 SvNumberFormatter::GuessDateTimeFormat( SvNumFormatType& rType, double fNumber, LanguageType eLnge )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    // Categorize the format according to the implementation of
    // SvNumberFormatter::GetEditFormat(), making assumptions about what
    // would be time only.
    sal_uInt32 nRet;
    if (0.0 <= fNumber && fNumber < 1.0)
    {
        // Clearly a time.
        rType = SvNumFormatType::TIME;
        nRet = GetTimeFormat( fNumber, eLnge);
    }
    else if (fabs( fNumber) * 24 < 0x7fff)
    {
        // Assuming time within 32k hours or 3.7 years.
        rType = SvNumFormatType::TIME;
        nRet = GetTimeFormat( fNumber, eLnge);
    }
    else if (rtl::math::approxFloor( fNumber) != fNumber)
    {
        // Date+Time.
        rType = SvNumFormatType::DATETIME;
        nRet = GetFormatIndex( NF_DATETIME_SYS_DDMMYYYY_HHMMSS, eLnge);
    }
    else
    {
        // Date only.
        rType = SvNumFormatType::DATE;
        nRet = GetFormatIndex( NF_DATE_SYS_DDMMYYYY, eLnge);
    }
    return nRet;
}

sal_uInt32 SvNumberFormatter::GetEditFormat( double fNumber, sal_uInt32 nFIndex,
                                             SvNumFormatType eType, LanguageType eLang,
                                             SvNumberformat const * pFormat )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    sal_uInt32 nKey = nFIndex;
    switch ( eType )
    {
    // #61619# always edit using 4-digit year
    case SvNumFormatType::DATE :
        {
            // Preserve ISO 8601 format.
            bool bIsoDate =
                nFIndex == GetFormatIndex( NF_DATE_DIN_YYYYMMDD, eLang) ||
                nFIndex == GetFormatIndex( NF_DATE_DIN_YYMMDD, eLang) ||
                nFIndex == GetFormatIndex( NF_DATE_DIN_MMDD, eLang) ||
                (pFormat && pFormat->IsIso8601( 0 ));
            if (rtl::math::approxFloor( fNumber) != fNumber)
            {
                // fdo#34977 preserve time when editing even if only date was
                // displayed.
                if (bIsoDate)
                    nKey = GetFormatIndex( NF_DATETIME_ISO_YYYYMMDD_HHMMSS, eLang);
                else
                    nKey = GetFormatIndex( NF_DATETIME_SYS_DDMMYYYY_HHMMSS, eLang );
            }
            else
            {
                if (bIsoDate)
                    nKey = GetFormatIndex( NF_DATE_ISO_YYYYMMDD, eLang);
                else
                    nKey = GetFormatIndex( NF_DATE_SYS_DDMMYYYY, eLang );
            }
        }
        break;
    case SvNumFormatType::TIME :
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
    case SvNumFormatType::DATETIME :
        if (nFIndex == GetFormatIndex( NF_DATETIME_ISO_YYYYMMDD_HHMMSS, eLang) || (pFormat && pFormat->IsIso8601( 0 )))
            nKey = GetFormatIndex( NF_DATETIME_ISO_YYYYMMDD_HHMMSS, eLang );
        else
            nKey = GetFormatIndex( NF_DATETIME_SYS_DDMMYYYY_HHMMSS, eLang );
        break;
    case SvNumFormatType::NUMBER:
        nKey = GetStandardFormat( eType, eLang );
        break;
    default:
        nKey = GetStandardFormat( fNumber, nFIndex, eType, eLang );
    }
    return nKey;
}

void SvNumberFormatter::GetInputLineString(const double& fOutNumber,
                                           sal_uInt32 nFIndex,
                                           OUString& sOutString)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    Color* pColor;
    sal_uInt32 nRealKey = nFIndex;
    SvNumberformat* pFormat = ImpSubstituteEntry( GetFormatEntry( nFIndex ), &nRealKey);
    if (!pFormat)
    {
        pFormat = GetFormatEntry(ZF_STANDARD);
    }

    LanguageType eLang = pFormat->GetLanguage();
    ChangeIntl( eLang );

    SvNumFormatType eType = pFormat->GetMaskedType();
    if (eType == SvNumFormatType::ALL)
    {
        // Mixed types in subformats, use first.
        /* XXX we could choose a subformat according to fOutNumber and
         * subformat conditions, but they may exist to suppress 0 or negative
         * numbers so wouldn't be a safe bet. */
        eType = pFormat->GetNumForInfoScannedType(0);
    }

    sal_uInt16 nOldPrec = pFormatScanner->GetStandardPrec();
    bool bPrecChanged = false;
    if (eType == SvNumFormatType::NUMBER ||
        eType == SvNumFormatType::PERCENT ||
        eType == SvNumFormatType::CURRENCY ||
        eType == SvNumFormatType::SCIENTIFIC ||
        eType == SvNumFormatType::FRACTION)
    {
        if (eType != SvNumFormatType::PERCENT)  // special treatment of % later
        {
            eType = SvNumFormatType::NUMBER;
        }
        ChangeStandardPrec(INPUTSTRING_PRECISION);
        bPrecChanged = true;
    }

    sal_uInt32 nKey = GetEditFormat( fOutNumber, nRealKey, eType, eLang, pFormat);
    if ( nKey != nRealKey )
    {
        pFormat = GetFormatEntry( nKey );
    }
    if (pFormat)
    {
        if ( eType == SvNumFormatType::TIME && pFormat->GetFormatPrecision() )
        {
            ChangeStandardPrec(INPUTSTRING_PRECISION);
            bPrecChanged = true;
        }
        pFormat->GetOutputString(fOutNumber, sOutString, &pColor);
    }
    if (bPrecChanged)
    {
        ChangeStandardPrec(nOldPrec);
    }
}

void SvNumberFormatter::GetOutputString(const OUString& sString,
                                        sal_uInt32 nFIndex,
                                        OUString& sOutString,
                                        Color** ppColor,
                                        bool bUseStarFormat )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    SvNumberformat* pFormat = GetFormatEntry( nFIndex );
    // ImpSubstituteEntry() is unnecessary here because so far only numeric
    // (time and date) are substituted.
    if (!pFormat)
    {
        pFormat = GetFormatEntry(ZF_STANDARD_TEXT);
    }
    if (!pFormat->IsTextFormat() && !pFormat->HasTextFormat())
    {
        *ppColor = nullptr;
        sOutString = sString;
    }
    else
    {
        ChangeIntl(pFormat->GetLanguage());
        if ( bUseStarFormat )
        {
           pFormat->SetStarFormatSupport( true );
        }
        pFormat->GetOutputString(sString, sOutString, ppColor);
        if ( bUseStarFormat )
        {
           pFormat->SetStarFormatSupport( false );
        }
    }
}

void SvNumberFormatter::GetOutputString(const double& fOutNumber,
                                        sal_uInt32 nFIndex,
                                        OUString& sOutString,
                                        Color** ppColor,
                                        bool bUseStarFormat )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (bNoZero && fOutNumber == 0.0)
    {
        sOutString.clear();
        return;
    }
    SvNumberformat* pFormat = ImpSubstituteEntry( GetFormatEntry( nFIndex ));
    if (!pFormat)
        pFormat = GetFormatEntry(ZF_STANDARD);
    ChangeIntl(pFormat->GetLanguage());
    if ( bUseStarFormat )
        pFormat->SetStarFormatSupport( true );
    pFormat->GetOutputString(fOutNumber, sOutString, ppColor);
    if ( bUseStarFormat )
        pFormat->SetStarFormatSupport( false );
}

bool SvNumberFormatter::GetPreviewString(const OUString& sFormatString,
                                         double fPreviewNumber,
                                         OUString& sOutString,
                                         Color** ppColor,
                                         LanguageType eLnge,
                                         bool bUseStarFormat )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (sFormatString.isEmpty())                       // no empty string
    {
        return false;
    }
    sal_uInt32 nKey;
    if (eLnge == LANGUAGE_DONTKNOW)
    {
        eLnge = IniLnge;
    }
    ChangeIntl(eLnge);                          // change locale if necessary
    eLnge = ActLnge;
    sal_Int32 nCheckPos = -1;
    OUString sTmpString = sFormatString;
    std::unique_ptr<SvNumberformat> p_Entry(new SvNumberformat(sTmpString,
                                                 pFormatScanner.get(),
                                                 pStringScanner.get(),
                                                 nCheckPos,
                                                 eLnge));
    if (nCheckPos == 0)                                 // String ok
    {
        sal_uInt32 CLOffset = ImpGenerateCL(eLnge);     // create new standard formats if necessary
        nKey = ImpIsEntry(p_Entry->GetFormatstring(),CLOffset, eLnge);
        if (nKey != NUMBERFORMAT_ENTRY_NOT_FOUND)       // already present
        {
            GetOutputString(fPreviewNumber, nKey, sOutString, ppColor, bUseStarFormat);
        }
        else
        {
            if ( bUseStarFormat )
            {
                p_Entry->SetStarFormatSupport( true );
            }
            p_Entry->GetOutputString(fPreviewNumber, sOutString, ppColor);
            if ( bUseStarFormat )
            {
                p_Entry->SetStarFormatSupport( false );
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool SvNumberFormatter::GetPreviewStringGuess( const OUString& sFormatString,
                                               double fPreviewNumber,
                                               OUString& sOutString,
                                               Color** ppColor,
                                               LanguageType eLnge )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (sFormatString.isEmpty())                       // no empty string
    {
        return false;
    }
    if (eLnge == LANGUAGE_DONTKNOW)
    {
        eLnge = IniLnge;
    }
    ChangeIntl( eLnge );
    eLnge = ActLnge;
    bool bEnglish = (eLnge == LANGUAGE_ENGLISH_US);

    OUString aFormatStringUpper( pCharClass->uppercase( sFormatString ) );
    sal_uInt32 nCLOffset = ImpGenerateCL( eLnge );
    sal_uInt32 nKey = ImpIsEntry( aFormatStringUpper, nCLOffset, eLnge );
    if ( nKey != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        // Target format present
        GetOutputString( fPreviewNumber, nKey, sOutString, ppColor );
        return true;
    }

    std::unique_ptr<SvNumberformat> pEntry;
    sal_Int32 nCheckPos = -1;
    OUString sTmpString;

    if ( bEnglish )
    {
        sTmpString = sFormatString;
        pEntry.reset(new SvNumberformat( sTmpString, pFormatScanner.get(),
                                     pStringScanner.get(), nCheckPos, eLnge ));
    }
    else
    {
        nCLOffset = ImpGenerateCL( LANGUAGE_ENGLISH_US );
        nKey = ImpIsEntry( aFormatStringUpper, nCLOffset, LANGUAGE_ENGLISH_US );
        bool bEnglishFormat = (nKey != NUMBERFORMAT_ENTRY_NOT_FOUND);

        // Try English -> other or convert english to other
        LanguageType eFormatLang = LANGUAGE_ENGLISH_US;
        pFormatScanner->SetConvertMode( LANGUAGE_ENGLISH_US, eLnge, false, false);
        sTmpString = sFormatString;
        pEntry.reset(new SvNumberformat( sTmpString, pFormatScanner.get(),
                                     pStringScanner.get(), nCheckPos, eFormatLang ));
        pFormatScanner->SetConvertMode( false );
        ChangeIntl( eLnge );

        if ( !bEnglishFormat )
        {
            if ( nCheckPos != 0 || xTransliteration->isEqual( sFormatString,
                                                              pEntry->GetFormatstring() ) )
            {
                // other Format
                // Force locale's keywords.
                pFormatScanner->ChangeIntl( ImpSvNumberformatScan::KeywordLocalization::LocaleLegacy );
                sTmpString = sFormatString;
                pEntry.reset(new SvNumberformat( sTmpString, pFormatScanner.get(),
                                             pStringScanner.get(), nCheckPos, eLnge ));
            }
            else
            {
                // verify english
                sal_Int32 nCheckPos2 = -1;
                // try other --> english
                eFormatLang = eLnge;
                pFormatScanner->SetConvertMode( eLnge, LANGUAGE_ENGLISH_US, false, false);
                sTmpString = sFormatString;
                std::unique_ptr<SvNumberformat> pEntry2(new SvNumberformat( sTmpString, pFormatScanner.get(),
                                                              pStringScanner.get(), nCheckPos2, eFormatLang ));
                pFormatScanner->SetConvertMode( false );
                ChangeIntl( eLnge );
                if ( nCheckPos2 == 0 && !xTransliteration->isEqual( sFormatString,
                                                                    pEntry2->GetFormatstring() ) )
                {
                    // other Format
                    // Force locale's keywords.
                    pFormatScanner->ChangeIntl( ImpSvNumberformatScan::KeywordLocalization::LocaleLegacy );
                    sTmpString = sFormatString;
                    pEntry.reset(new SvNumberformat( sTmpString, pFormatScanner.get(),
                                                 pStringScanner.get(), nCheckPos, eLnge ));
                }
            }
        }
    }

    if (nCheckPos == 0)                                 // String ok
    {
        ImpGenerateCL( eLnge );     // create new standard formats if necessary
        pEntry->GetOutputString( fPreviewNumber, sOutString, ppColor );
        return true;
    }
    return false;
}

bool SvNumberFormatter::GetPreviewString( const OUString& sFormatString,
                                          const OUString& sPreviewString,
                                          OUString& sOutString,
                                          Color** ppColor,
                                          LanguageType eLnge )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (sFormatString.isEmpty())               // no empty string
    {
        return false;
    }
    sal_uInt32 nKey;
    if (eLnge == LANGUAGE_DONTKNOW)
    {
        eLnge = IniLnge;
    }
    ChangeIntl(eLnge);                          // switch if needed
    eLnge = ActLnge;
    sal_Int32 nCheckPos = -1;
    OUString sTmpString = sFormatString;
    std::unique_ptr<SvNumberformat> p_Entry(new SvNumberformat( sTmpString,
                                                  pFormatScanner.get(),
                                                  pStringScanner.get(),
                                                  nCheckPos,
                                                  eLnge));
    if (nCheckPos == 0)                          // String ok
    {
        // May have to create standard formats for this locale.
        sal_uInt32 CLOffset = ImpGenerateCL(eLnge);
        nKey = ImpIsEntry( p_Entry->GetFormatstring(), CLOffset, eLnge);
        if (nKey != NUMBERFORMAT_ENTRY_NOT_FOUND)       // already present
        {
            GetOutputString( sPreviewString, nKey, sOutString, ppColor);
        }
        else
        {
            // If the format is valid but not a text format and does not
            // include a text subformat, an empty string would result. Same as
            // in SvNumberFormatter::GetOutputString()
            if (p_Entry->IsTextFormat() || p_Entry->HasTextFormat())
            {
                p_Entry->GetOutputString( sPreviewString, sOutString, ppColor);
            }
            else
            {
                *ppColor = nullptr;
                sOutString = sPreviewString;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

sal_uInt32 SvNumberFormatter::TestNewString(const OUString& sFormatString,
                                            LanguageType eLnge)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (sFormatString.isEmpty())                       // no empty string
    {
        return NUMBERFORMAT_ENTRY_NOT_FOUND;
    }
    if (eLnge == LANGUAGE_DONTKNOW)
    {
        eLnge = IniLnge;
    }
    ChangeIntl(eLnge);                                  // change locale if necessary
    eLnge = ActLnge;
    sal_uInt32 nRes;
    sal_Int32 nCheckPos = -1;
    OUString sTmpString = sFormatString;
    std::unique_ptr<SvNumberformat> pEntry(new SvNumberformat(sTmpString,
                                                pFormatScanner.get(),
                                                pStringScanner.get(),
                                                nCheckPos,
                                                eLnge));
    if (nCheckPos == 0)                                 // String ok
    {
        sal_uInt32 CLOffset = ImpGenerateCL(eLnge);     // create new standard formats if necessary
        nRes = ImpIsEntry(pEntry->GetFormatstring(),CLOffset, eLnge);
                                                        // already present?
    }
    else
    {
        nRes = NUMBERFORMAT_ENTRY_NOT_FOUND;
    }
    return nRes;
}

SvNumberformat* SvNumberFormatter::ImpInsertFormat( const css::i18n::NumberFormatCode& rCode,
                                                    sal_uInt32 nPos, bool bAfterChangingSystemCL,
                                                    sal_Int16 nOrgIndex )
{
    SAL_WARN_IF( NF_INDEX_TABLE_LOCALE_DATA_DEFAULTS <= rCode.Index && rCode.Index < NF_INDEX_TABLE_ENTRIES,
            "svl.numbers", "i18npool locale '" << maLanguageTag.getBcp47() <<
            "' uses reserved formatIndex value " << rCode.Index << ", next free: " << NF_INDEX_TABLE_ENTRIES <<
            "  Please see description in include/svl/zforlist.hxx at end of enum NfIndexTableOffset");
    assert( (rCode.Index < NF_INDEX_TABLE_LOCALE_DATA_DEFAULTS || NF_INDEX_TABLE_ENTRIES <= rCode.Index) &&
            "reserved formatIndex, see warning above");

    OUString aCodeStr( rCode.Code );
    if ( rCode.Index < NF_INDEX_TABLE_LOCALE_DATA_DEFAULTS &&
            rCode.Usage == css::i18n::KNumberFormatUsage::CURRENCY &&
            rCode.Index != NF_CURRENCY_1000DEC2_CCC )
    {   // strip surrounding [$...] on automatic currency
        if ( aCodeStr.indexOf( "[$" ) >= 0)
            aCodeStr = SvNumberformat::StripNewCurrencyDelimiters( aCodeStr );
        else
        {
            if (LocaleDataWrapper::areChecksEnabled() &&
                    rCode.Index != NF_CURRENCY_1000DEC2_CCC )
            {
                OUString aMsg = "SvNumberFormatter::ImpInsertFormat: no [$...] on currency format code, index " +
                                OUString::number( rCode.Index) +
                                ":\n" +
                                rCode.Code;
                LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg));
            }
        }
    }
    sal_Int32 nCheckPos = 0;
    std::unique_ptr<SvNumberformat> pFormat(new SvNumberformat(aCodeStr,
                                                               pFormatScanner.get(),
                                                               pStringScanner.get(),
                                                               nCheckPos,
                                                               ActLnge));
    if (nCheckPos != 0)
    {
        if (LocaleDataWrapper::areChecksEnabled())
        {
            OUString aMsg = "SvNumberFormatter::ImpInsertFormat: bad format code, index " +
                            OUString::number( rCode.Index ) +
                            "\n" +
                            rCode.Code;
            LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg));
        }
        return nullptr;
    }
    if ( rCode.Index >= NF_INDEX_TABLE_LOCALE_DATA_DEFAULTS )
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
                    OUString aMsg = "SvNumberFormatter::ImpInsertFormat: dup format code, index "
                                  + OUString::number( rCode.Index )
                                  + "\n"
                                  + rCode.Code;
                    LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg));
                }
                }
            }
            return nullptr;
        }
        else if ( nPos - nCLOffset >= SV_COUNTRY_LANGUAGE_OFFSET )
        {
            if (LocaleDataWrapper::areChecksEnabled())
            {
                OUString aMsg =  "SvNumberFormatter::ImpInsertFormat: too many format codes, index "
                              + OUString::number( rCode.Index )
                              + "\n"
                              +  rCode.Code;
                LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg));
            }
            return nullptr;
        }
    }
    auto pFormat2 = pFormat.get();
    if ( !aFTable.emplace( nPos, std::move(pFormat) ).second )
    {
        if (LocaleDataWrapper::areChecksEnabled())
        {
            OUString aMsg = "ImpInsertFormat: can't insert number format key pos: "
                          + OUString::number( nPos )
                          + ", code index "
                          + OUString::number( rCode.Index )
                          + "\n"
                          + rCode.Code;
            LocaleDataWrapper::outputCheckMessage( xLocaleData->appendLocaleInfo( aMsg));
        }
        else
        {
            SAL_WARN( "svl.numbers", "SvNumberFormatter::ImpInsertFormat: dup position");
        }
        return nullptr;
    }
    if ( rCode.Default )
        pFormat2->SetStandard();
    if ( !rCode.DefaultName.isEmpty() )
        pFormat2->SetComment( rCode.DefaultName );
    return pFormat2;
}

void SvNumberFormatter::GetFormatSpecialInfo(sal_uInt32 nFormat,
                                             bool& bThousand,
                                             bool& IsRed,
                                             sal_uInt16& nPrecision,
                                             sal_uInt16& nLeadingCnt)

{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    SvNumberformat* pFormat = GetFormatEntry( nFormat );
    if (pFormat)
        pFormat->GetFormatSpecialInfo(bThousand, IsRed,
                                      nPrecision, nLeadingCnt);
    else
    {
        bThousand = false;
        IsRed = false;
        nPrecision = pFormatScanner->GetStandardPrec();
        nLeadingCnt = 0;
    }
}

sal_uInt16 SvNumberFormatter::GetFormatPrecision( sal_uInt32 nFormat ) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    const SvNumberformat* pFormat = GetFormatEntry( nFormat );
    if ( pFormat )
        return pFormat->GetFormatPrecision();
    else
        return pFormatScanner->GetStandardPrec();
}

sal_uInt16 SvNumberFormatter::GetFormatIntegerDigits( sal_uInt32 nFormat ) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    const SvNumberformat* pFormat = GetFormatEntry( nFormat );
    if ( pFormat )
        return pFormat->GetFormatIntegerDigits();
    else
        return 1;
}

OUString SvNumberFormatter::GetFormatDecimalSep( sal_uInt32 nFormat ) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    const SvNumberformat* pFormat = GetFormatEntry(nFormat);
    if ( !pFormat || pFormat->GetLanguage() == ActLnge )
    {
        return GetNumDecimalSep();
    }
    OUString aRet;
    LanguageType eSaveLang = xLocaleData.getCurrentLanguage();
    if ( pFormat->GetLanguage() == eSaveLang )
    {
        aRet = xLocaleData->getNumDecimalSep();
    }
    else
    {
        LanguageTag aSaveLocale( xLocaleData->getLanguageTag() );
        const_cast<SvNumberFormatter*>(this)->xLocaleData.changeLocale( LanguageTag( pFormat->GetLanguage()) );
        aRet = xLocaleData->getNumDecimalSep();
        const_cast<SvNumberFormatter*>(this)->xLocaleData.changeLocale( aSaveLocale );
    }
    return aRet;
}


sal_uInt32 SvNumberFormatter::GetFormatSpecialInfo( const OUString& rFormatString,
                                                    bool& bThousand, bool& IsRed, sal_uInt16& nPrecision,
                                                    sal_uInt16& nLeadingCnt, LanguageType eLnge )

{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (eLnge == LANGUAGE_DONTKNOW)
    {
        eLnge = IniLnge;
    }
    ChangeIntl(eLnge);                                  // change locale if necessary
    eLnge = ActLnge;
    OUString aTmpStr( rFormatString );
    sal_Int32 nCheckPos = 0;
    std::unique_ptr<SvNumberformat> pFormat(new SvNumberformat( aTmpStr, pFormatScanner.get(),
                                                  pStringScanner.get(), nCheckPos, eLnge ));
    if ( nCheckPos == 0 )
    {
        pFormat->GetFormatSpecialInfo( bThousand, IsRed, nPrecision, nLeadingCnt );
    }
    else
    {
        bThousand = false;
        IsRed = false;
        nPrecision = pFormatScanner->GetStandardPrec();
        nLeadingCnt = 0;
    }
    return nCheckPos;
}

sal_Int32 SvNumberFormatter::ImpGetFormatCodeIndex(
            css::uno::Sequence< css::i18n::NumberFormatCode >& rSeq,
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
        OUString aMsg = "SvNumberFormatter::ImpGetFormatCodeIndex: not found: "
                      + OUString::number( nTabOff );
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
        rSeq[0] = css::i18n::NumberFormatCode();
        rSeq[0].Code = "0" + GetNumDecimalSep() + "############";
    }
    return 0;
}


void SvNumberFormatter::ImpAdjustFormatCodeDefault(
        css::i18n::NumberFormatCode * pFormatArr,
        sal_Int32 nCnt )
{
    if ( !nCnt )
        return;
    if (LocaleDataWrapper::areChecksEnabled())
    {
        // check the locale data for correctness
        OStringBuffer aMsg;
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
                aMsg.append("unknown type");
            }
            if ( pFormatArr[nElem].Default )
            {
                switch ( pFormatArr[nElem].Type )
                {
                case i18n::KNumberFormatType::SHORT :
                    if ( nShortDef != -1 )
                        aMsg.append("dupe short type default");
                    nShortDef = nElem;
                    break;
                case i18n::KNumberFormatType::MEDIUM :
                    if ( nMediumDef != -1 )
                        aMsg.append("dupe medium type default");
                    nMediumDef = nElem;
                    break;
                case i18n::KNumberFormatType::LONG :
                    if ( nLongDef != -1 )
                        aMsg.append("dupe long type default");
                    nLongDef = nElem;
                    break;
                }
            }
            if (!aMsg.isEmpty())
            {
                aMsg.insert(0, "SvNumberFormatter::ImpAdjustFormatCodeDefault: ");
                aMsg.append("\nXML locale data FormatElement formatindex: ");
                aMsg.append(static_cast<sal_Int32>(pFormatArr[nElem].Index));
                OUString aUMsg(OStringToOUString(aMsg.makeStringAndClear(),
                    RTL_TEXTENCODING_ASCII_US));
                LocaleDataWrapper::outputCheckMessage(xLocaleData->appendLocaleInfo(aUMsg));
            }
        }
        if ( nShort != -1 && nShortDef == -1 )
            aMsg.append("no short type default  ");
        if ( nMedium != -1 && nMediumDef == -1 )
            aMsg.append("no medium type default  ");
        if ( nLong != -1 && nLongDef == -1 )
            aMsg.append("no long type default  ");
        if (!aMsg.isEmpty())
        {
            aMsg.insert(0, "SvNumberFormatter::ImpAdjustFormatCodeDefault: ");
            aMsg.append("\nXML locale data FormatElement group of: ");
            OUString aUMsg(OStringToOUString(aMsg.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US));
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
                [[fallthrough]];
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
}

SvNumberformat* SvNumberFormatter::GetFormatEntry( sal_uInt32 nKey )
{
    auto it = aFTable.find( nKey);
    if (it != aFTable.end())
        return it->second.get();
    return nullptr;
}

const SvNumberformat* SvNumberFormatter::GetFormatEntry( sal_uInt32 nKey ) const
{
    return GetEntry( nKey);
}

const SvNumberformat* SvNumberFormatter::GetEntry( sal_uInt32 nKey ) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    auto it = aFTable.find( nKey);
    if (it != aFTable.end())
        return it->second.get();
    return nullptr;
}

const SvNumberformat* SvNumberFormatter::GetSubstitutedEntry( sal_uInt32 nKey, sal_uInt32 & o_rNewKey ) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    // A tad ugly, but GetStandardFormat() and GetFormatIndex() in
    // ImpSubstituteEntry() may have to add the LANGUAGE_SYSTEM formats if not
    // already present (which in practice most times they are).
    SvNumberFormatter* pThis = const_cast<SvNumberFormatter*>(this);
    return pThis->ImpSubstituteEntry( pThis->GetFormatEntry( nKey), &o_rNewKey);
}

SvNumberformat* SvNumberFormatter::ImpSubstituteEntry( SvNumberformat* pFormat, sal_uInt32 * o_pRealKey )
{
    if (!pFormat || !pFormat->IsSubstituted())
        return pFormat;

    // XXX NOTE: substitution can not be done in GetFormatEntry() as otherwise
    // to be substituted formats would "vanish", i.e. from the number formatter
    // dialog or when exporting to Excel.

    sal_uInt32 nKey;
    if (pFormat->IsSystemTimeFormat())
        /* TODO: should we have NF_TIME_SYSTEM for consistency? */
        nKey = GetStandardFormat( SvNumFormatType::TIME, LANGUAGE_SYSTEM);
    else if (pFormat->IsSystemLongDateFormat())
        /* TODO: either that above, or have a long option for GetStandardFormat() */
        nKey = GetFormatIndex( NF_DATE_SYSTEM_LONG, LANGUAGE_SYSTEM);
    else
        return pFormat;

    if (o_pRealKey)
        *o_pRealKey = nKey;
    auto it = aFTable.find( nKey);
    return it == aFTable.end() ? nullptr : it->second.get();
}

void SvNumberFormatter::ImpGenerateFormats( sal_uInt32 CLOffset, bool bNoAdditionalFormats )
{
    bool bOldConvertMode = pFormatScanner->GetConvertMode();
    if (bOldConvertMode)
    {
        pFormatScanner->SetConvertMode(false);      // switch off for this function
    }

    css::lang::Locale aLocale = GetLanguageTag().getLocale();
    css::uno::Reference< css::i18n::XNumberFormatCode > xNFC = i18n::NumberFormatMapper::create( m_xContext );
    sal_Int32 nIdx;
    bool bDefault;

    // Number
    uno::Sequence< i18n::NumberFormatCode > aFormatSeq = xNFC->getAllFormatCode( i18n::KNumberFormatUsage::FIXED_NUMBER, aLocale );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // General
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_STANDARD );
    SvNumberformat* pStdFormat = ImpInsertFormat( aFormatSeq[nIdx],
            CLOffset + ZF_STANDARD /* NF_NUMBER_STANDARD */ );
    if (pStdFormat)
    {
        // This is _the_ standard format.
        if (LocaleDataWrapper::areChecksEnabled() && pStdFormat->GetType() != SvNumFormatType::NUMBER)
        {
            LocaleDataWrapper::outputCheckMessage( xLocaleData->
                                                   appendLocaleInfo( "SvNumberFormatter::ImpGenerateFormats: General format not NUMBER"));
        }
        pStdFormat->SetType( SvNumFormatType::NUMBER );
        pStdFormat->SetStandard();
        pStdFormat->SetLastInsertKey( SV_MAX_COUNT_STANDARD_FORMATS, SvNumberformat::FormatterPrivateAccess() );
    }
    else
    {
        if (LocaleDataWrapper::areChecksEnabled())
        {
            LocaleDataWrapper::outputCheckMessage( xLocaleData->
                                                   appendLocaleInfo( "SvNumberFormatter::ImpGenerateFormats: General format not insertable, nothing will work"));
        }
    }

    {
        // Boolean
        OUString aFormatCode = pFormatScanner->GetBooleanString();
        sal_Int32 nCheckPos = 0;

        std::unique_ptr<SvNumberformat> pNewFormat(new SvNumberformat( aFormatCode, pFormatScanner.get(),
                                                                       pStringScanner.get(), nCheckPos, ActLnge ));
        pNewFormat->SetType(SvNumFormatType::LOGICAL);
        pNewFormat->SetStandard();
        if ( !aFTable.emplace(CLOffset + ZF_STANDARD_LOGICAL /* NF_BOOLEAN */,
                            std::move(pNewFormat)).second )
        {
            SAL_WARN( "svl.numbers", "SvNumberFormatter::ImpGenerateFormats: dup position Boolean");
        }

        // Text
        aFormatCode = "@";
        pNewFormat.reset(new SvNumberformat( aFormatCode, pFormatScanner.get(),
                                             pStringScanner.get(), nCheckPos, ActLnge ));
        pNewFormat->SetType(SvNumFormatType::TEXT);
        pNewFormat->SetStandard();
        if ( !aFTable.emplace( CLOffset + ZF_STANDARD_TEXT /* NF_TEXT */,
                               std::move(pNewFormat)).second )
        {
            SAL_WARN( "svl.numbers", "SvNumberFormatter::ImpGenerateFormats: dup position Text");
        }
    }

    // 0
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_INT );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD+1 /* NF_NUMBER_INT */ );

    // 0.00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_DEC2 );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD+2 /* NF_NUMBER_DEC2 */ );

    // #,##0
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_1000INT );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD+3 /* NF_NUMBER_1000INT */ );

    // #,##0.00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_1000DEC2 );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD+4 /* NF_NUMBER_1000DEC2 */ );

    // #.##0,00 System country/language dependent
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_NUMBER_SYSTEM );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD+5 /* NF_NUMBER_SYSTEM */ );


    // Percent number
    aFormatSeq = xNFC->getAllFormatCode( i18n::KNumberFormatUsage::PERCENT_NUMBER, aLocale );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // 0%
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_PERCENT_INT );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_PERCENT /* NF_PERCENT_INT */ );

    // 0.00%
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_PERCENT_DEC2 );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_PERCENT+1 /* NF_PERCENT_DEC2 */ );


    // Currency. NO default standard option! Default is determined of locale
    // data default currency and format is generated if needed.
    aFormatSeq = xNFC->getAllFormatCode( i18n::KNumberFormatUsage::CURRENCY, aLocale );
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
                     CLOffset + ZF_STANDARD_CURRENCY /* NF_CURRENCY_1000INT */ );
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0.00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2 );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = false;
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_CURRENCY+1 /* NF_CURRENCY_1000DEC2 */ );
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0 negative red
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000INT_RED );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = false;
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_CURRENCY+2 /* NF_CURRENCY_1000INT_RED */ );
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0.00 negative red
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2_RED );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = false;
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_CURRENCY+3 /* NF_CURRENCY_1000DEC2_RED */ );
    aFormatSeq[nIdx].Default = bDefault;

    // #,##0.00 USD
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2_CCC );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = false;
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_CURRENCY+4 /* NF_CURRENCY_1000DEC2_CCC */ );
    aFormatSeq[nIdx].Default = bDefault;

    // #.##0,--
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_CURRENCY_1000DEC2_DASHED );
    bDefault = aFormatSeq[nIdx].Default;
    aFormatSeq[nIdx].Default = false;
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_CURRENCY+5 /* NF_CURRENCY_1000DEC2_DASHED */ );
    aFormatSeq[nIdx].Default = bDefault;


    // Date
    aFormatSeq = xNFC->getAllFormatCode( i18n::KNumberFormatUsage::DATE, aLocale );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // DD.MM.YY   System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYSTEM_SHORT );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATE /* NF_DATE_SYSTEM_SHORT */ );

    // NN DD.MMM YY
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DEF_NNDDMMMYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATE+1 /* NF_DATE_DEF_NNDDMMMYY */ );

    // DD.MM.YY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_MMYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATE+2 /* NF_DATE_SYS_MMYY */ );

    // DD MMM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DDMMM );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATE+3 /* NF_DATE_SYS_DDMMM */ );

    // MMMM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_MMMM );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATE+4 /* NF_DATE_MMMM */ );

    // QQ YY
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_QQJJ );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATE+5 /* NF_DATE_QQJJ */ );

    // DD.MM.YYYY   was DD.MM.[YY]YY
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DDMMYYYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATE+6 /* NF_DATE_SYS_DDMMYYYY */ );

    // DD.MM.YY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DDMMYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATE+7 /* NF_DATE_SYS_DDMMYY */ );

    // NNN, D. MMMM YYYY   System
    // Long day of week: "NNNN" instead of "NNN," because of compatibility
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYSTEM_LONG );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATE+8 /* NF_DATE_SYSTEM_LONG */ );

    // Hard coded but system (regional settings) delimiters dependent long date formats

    // D. MMM YY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DMMMYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATE+9 /* NF_DATE_SYS_DMMMYY */ );

    // D. MMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DMMMYYYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_NEWEXTENDED_DATE_SYS_DMMMYYYY /* NF_DATE_SYS_DMMMYYYY */ );

    // D. MMMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_DMMMMYYYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_NEWEXTENDED_DATE_SYS_DMMMMYYYY /* NF_DATE_SYS_DMMMMYYYY */ );

    // NN, D. MMM YY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_NNDMMMYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_NEWEXTENDED_DATE_SYS_NNDMMMYY /* NF_DATE_SYS_NNDMMMYY */ );

    // NN, D. MMMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_NNDMMMMYYYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_NEWEXTENDED_DATE_SYS_NNDMMMMYYYY /*  NF_DATE_SYS_NNDMMMMYYYY */ );

    // NNN, D. MMMM YYYY   def/System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_SYS_NNNNDMMMMYYYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_NEWEXTENDED_DATE_SYS_NNNNDMMMMYYYY /* NF_DATE_SYS_NNNNDMMMMYYYY */ );

    // Hard coded DIN (Deutsche Industrie Norm) and EN (European Norm) date formats

    // D. MMM. YYYY   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_DMMMYYYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_NEWEXTENDED_DATE_DIN_DMMMYYYY /* NF_DATE_DIN_DMMMYYYY */ );

    // D. MMMM YYYY   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_DMMMMYYYY );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_NEWEXTENDED_DATE_DIN_DMMMMYYYY /* NF_DATE_DIN_DMMMMYYYY */ );

    // MM-DD   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_MMDD );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_NEWEXTENDED_DATE_DIN_MMDD /* NF_DATE_DIN_MMDD */ );

    // YY-MM-DD   DIN/EN
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_YYMMDD );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_NEWEXTENDED_DATE_DIN_YYMMDD /* NF_DATE_DIN_YYMMDD */ );

    // YYYY-MM-DD   DIN/EN/ISO
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATE_DIN_YYYYMMDD );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_NEWEXTENDED_DATE_DIN_YYYYMMDD /* NF_DATE_DIN_YYYYMMDD */ );


    // Time
    aFormatSeq = xNFC->getAllFormatCode( i18n::KNumberFormatUsage::TIME, aLocale );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // HH:MM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMM );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_TIME /* NF_TIME_HHMM */ );

    // HH:MM:SS
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMMSS );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_TIME+1 /* NF_TIME_HHMMSS */ );

    // HH:MM AM/PM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMMAMPM );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_TIME+2 /* NF_TIME_HHMMAMPM */ );

    // HH:MM:SS AM/PM
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HHMMSSAMPM );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_TIME+3 /* NF_TIME_HHMMSSAMPM */ );

    // [HH]:MM:SS
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HH_MMSS );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_TIME+4 /* NF_TIME_HH_MMSS */ );

    // MM:SS,00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_MMSS00 );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_TIME+5 /* NF_TIME_MMSS00 */ );

    // [HH]:MM:SS,00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_TIME_HH_MMSS00 );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_TIME+6 /* NF_TIME_HH_MMSS00 */ );


    // DateTime
    aFormatSeq = xNFC->getAllFormatCode( i18n::KNumberFormatUsage::DATE_TIME, aLocale );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // DD.MM.YY HH:MM   System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATETIME_SYSTEM_SHORT_HHMM );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATETIME /* NF_DATETIME_SYSTEM_SHORT_HHMM */ );

    // DD.MM.YYYY HH:MM:SS   System
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_DATETIME_SYS_DDMMYYYY_HHMMSS );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_DATETIME+1 /* NF_DATETIME_SYS_DDMMYYYY_HHMMSS */ );

    const NfKeywordTable & rKeyword = pFormatScanner->GetKeywords();
    i18n::NumberFormatCode aSingleFormatCode;
    OUStringBuffer aBuf;
    aSingleFormatCode.Usage = i18n::KNumberFormatUsage::DATE_TIME;

    // YYYY-MM-DD HH:MM:SS   ISO
    aBuf.append( rKeyword[NF_KEY_YYYY]).append('-').
        append( rKeyword[NF_KEY_MM]).append('-').
        append( rKeyword[NF_KEY_DD]).append(' ').
        append( rKeyword[NF_KEY_HH]).append(':').
        append( rKeyword[NF_KEY_MMI]).append(':').
        append( rKeyword[NF_KEY_SS]);
    aSingleFormatCode.Code = aBuf.makeStringAndClear();
    ImpInsertFormat( aSingleFormatCode,
                     CLOffset + ZF_STANDARD_DATETIME+2 /* NF_DATETIME_ISO_YYYYMMDD_HHMMSS */ );


    // Scientific number
    aFormatSeq = xNFC->getAllFormatCode( i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER, aLocale );
    ImpAdjustFormatCodeDefault( aFormatSeq.getArray(), aFormatSeq.getLength() );

    // 0.00E+000
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_SCIENTIFIC_000E000 );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_SCIENTIFIC /* NF_SCIENTIFIC_000E000 */ );

    // 0.00E+00
    nIdx = ImpGetFormatCodeIndex( aFormatSeq, NF_SCIENTIFIC_000E00 );
    ImpInsertFormat( aFormatSeq[nIdx],
                     CLOffset + ZF_STANDARD_SCIENTIFIC+1 /* NF_SCIENTIFIC_000E00 */ );


    // Fraction number (no default option)
    aSingleFormatCode.Usage = i18n::KNumberFormatUsage::FRACTION_NUMBER;

     // # ?/?
    aSingleFormatCode.Code = "# ?/?";
    ImpInsertFormat( aSingleFormatCode,
                     CLOffset + ZF_STANDARD_FRACTION /* NF_FRACTION_1D */ );

    // # ??/??
    //! "??/" would be interpreted by the compiler as a trigraph for '\'
    aSingleFormatCode.Code = "# ?\?/?\?";
    ImpInsertFormat( aSingleFormatCode,
                     CLOffset + ZF_STANDARD_FRACTION+1 /* NF_FRACTION_2D */ );

    // # ???/???
    //! "??/" would be interpreted by the compiler as a trigraph for '\'
    aSingleFormatCode.Code = "# ?\?\?/?\?\?";
    ImpInsertFormat( aSingleFormatCode,
                     CLOffset + ZF_STANDARD_FRACTION+2 /* NF_FRACTION_3D */ );

    // # ?/2
    aSingleFormatCode.Code = "# ?/2";
    ImpInsertFormat( aSingleFormatCode,
                     CLOffset + ZF_STANDARD_FRACTION+3 /* NF_FRACTION_2 */ );

    // # ?/4
    aSingleFormatCode.Code = "# ?/4";
    ImpInsertFormat( aSingleFormatCode,
                     CLOffset + ZF_STANDARD_FRACTION+4 /* NF_FRACTION_4 */ );

    // # ?/8
    aSingleFormatCode.Code = "# ?/8";
    ImpInsertFormat( aSingleFormatCode,
                     CLOffset + ZF_STANDARD_FRACTION+5 /* NF_FRACTION_8 */ );

    // # ??/16
    aSingleFormatCode.Code = "# ?\?/16";
    ImpInsertFormat( aSingleFormatCode,
                     CLOffset + ZF_STANDARD_FRACTION+6 /* NF_FRACTION_16 */ );

    // # ??/10
    aSingleFormatCode.Code = "# ?\?/10";
    ImpInsertFormat( aSingleFormatCode,
                     CLOffset + ZF_STANDARD_FRACTION+7 /* NF_FRACTION_10 */ );

    // # ??/100
    aSingleFormatCode.Code = "# ?\?/100";
    ImpInsertFormat( aSingleFormatCode,
                     CLOffset + ZF_STANDARD_FRACTION+8 /* NF_FRACTION_100 */ );


    // Week of year
    aSingleFormatCode.Code = rKeyword[NF_KEY_WW];
    ImpInsertFormat( aSingleFormatCode,
                     CLOffset + ZF_STANDARD_NEWEXTENDED_DATE_WW /* NF_DATE_WW */ );

    // Now all additional format codes provided by I18N, but only if not
    // changing SystemCL, then they are appended last after user defined.
    if ( !bNoAdditionalFormats )
    {
        ImpGenerateAdditionalFormats( CLOffset, xNFC, false );
    }
    if (bOldConvertMode)
    {
        pFormatScanner->SetConvertMode(true);
    }
}


void SvNumberFormatter::ImpGenerateAdditionalFormats( sal_uInt32 CLOffset,
            css::uno::Reference< css::i18n::XNumberFormatCode > const & rNumberFormatCode,
            bool bAfterChangingSystemCL )
{
    SvNumberformat* pStdFormat = GetFormatEntry( CLOffset + ZF_STANDARD );
    if ( !pStdFormat )
    {
        SAL_WARN( "svl.numbers", "ImpGenerateAdditionalFormats: no GENERAL format" );
        return ;
    }
    sal_uInt32 nPos = CLOffset + pStdFormat->GetLastInsertKey( SvNumberformat::FormatterPrivateAccess() );
    css::lang::Locale aLocale = GetLanguageTag().getLocale();
    sal_Int32 j;

    // All currencies, this time with [$...] which was stripped in
    // ImpGenerateFormats for old "automatic" currency formats.
    uno::Sequence< i18n::NumberFormatCode > aFormatSeq = rNumberFormatCode->getAllFormatCode( i18n::KNumberFormatUsage::CURRENCY, aLocale );
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
        if ( pFormatArr[j].Index < NF_INDEX_TABLE_LOCALE_DATA_DEFAULTS &&
                pFormatArr[j].Index != NF_CURRENCY_1000DEC2_CCC )
        {   // Insert only if not already inserted, but internal index must be
            // above so ImpInsertFormat can distinguish it.
            sal_Int16 nOrgIndex = pFormatArr[j].Index;
            pFormatArr[j].Index = sal::static_int_cast< sal_Int16 >(
                pFormatArr[j].Index + nCodes + NF_INDEX_TABLE_ENTRIES);
            //! no default on currency
            bool bDefault = aFormatSeq[j].Default;
            aFormatSeq[j].Default = false;
            if ( SvNumberformat* pNewFormat = ImpInsertFormat( pFormatArr[j], nPos+1,
                        bAfterChangingSystemCL, nOrgIndex ) )
            {
                pNewFormat->SetAdditionalBuiltin();
                nPos++;
            }
            pFormatArr[j].Index = nOrgIndex;
            aFormatSeq[j].Default = bDefault;
        }
    }

    // All additional format codes provided by I18N that are not old standard
    // index. Additional formats may define defaults, currently there is no
    // check if more than one default of a usage/type combination is provided,
    // like it is done for usage groups with ImpAdjustFormatCodeDefault().
    // There is no harm though, on first invocation ImpGetDefaultFormat() will
    // use the first default encountered.
    aFormatSeq = rNumberFormatCode->getAllFormatCodes( aLocale );
    nCodes = aFormatSeq.getLength();
    if ( nCodes )
    {
        pFormatArr = aFormatSeq.getArray();
        for ( j = 0; j < nCodes; j++ )
        {
            if ( nPos - CLOffset >= SV_COUNTRY_LANGUAGE_OFFSET )
            {
                SAL_WARN( "svl.numbers", "ImpGenerateAdditionalFormats: too many formats" );
                break;  // for
            }
            if ( pFormatArr[j].Index >= NF_INDEX_TABLE_LOCALE_DATA_DEFAULTS )
            {
                if ( SvNumberformat* pNewFormat = ImpInsertFormat( pFormatArr[j], nPos+1,
                            bAfterChangingSystemCL ) )
                {
                    pNewFormat->SetAdditionalBuiltin();
                    nPos++;
                }
            }
        }
    }

    pStdFormat->SetLastInsertKey( static_cast<sal_uInt16>(nPos - CLOffset), SvNumberformat::FormatterPrivateAccess() );
}


sal_Int32 SvNumberFormatter::ImpPosToken ( const OUStringBuffer & sFormat, sal_Unicode token, sal_Int32 nStartPos /* = 0*/ ) const
{
    sal_Int32 nLength = sFormat.getLength();
    for ( sal_Int32 i=nStartPos; i<nLength && i>=0 ; i++ )
    {
        switch(sFormat[i])
        {
            case '\"' : // skip text
                i = sFormat.indexOf('\"',i+1);
                break;
            case '['  : // skip condition
                i = sFormat.indexOf(']',i+1);
                break;
            case '\\' : // skip escaped character
                i++;
                break;
            case ';'  :
                if (token == ';')
                    return i;
                break;
            case 'e'  :
            case 'E'  :
                if (token == 'E')
                   return i; // if 'E' is outside "" and [] it must be the 'E' exponent
                break;
            default : break;
        }
        if ( i<0 )
            i--;
    }
    return -2;
}

OUString SvNumberFormatter::GenerateFormat(sal_uInt32 nIndex,
                                           LanguageType eLnge,
                                           bool bThousand,
                                           bool IsRed,
                                           sal_uInt16 nPrecision,
                                           sal_uInt16 nLeadingZeros)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (eLnge == LANGUAGE_DONTKNOW)
    {
        eLnge = IniLnge;
    }

    const SvNumberformat* pFormat = GetFormatEntry( nIndex );
    const SvNumFormatType eType = (pFormat ? pFormat->GetMaskedType() : SvNumFormatType::UNDEFINED);

    ImpGenerateCL(eLnge);           // create new standard formats if necessary

    utl::DigitGroupingIterator aGrouping( xLocaleData->getDigitGrouping());
    // always group of 3 for Engineering notation
    const sal_Int32 nDigitsInFirstGroup = ( bThousand && (eType == SvNumFormatType::SCIENTIFIC) ) ? 3 : aGrouping.get();
    const OUString& rThSep = GetNumThousandSep();

    OUStringBuffer sString;
    using comphelper::string::padToLength;

    if (nLeadingZeros == 0)
    {
        if (!bThousand)
            sString.append('#');
        else
        {
            if (eType == SvNumFormatType::SCIENTIFIC)
            {  // for scientific, bThousand is used for Engineering notation
                sString.append("###");
            }
            else
            {
                sString.append('#');
                sString.append(rThSep);
                padToLength(sString, sString.getLength() + nDigitsInFirstGroup, '#');
            }
        }
    }
    else
    {
        for (sal_uInt16 i = 0; i < nLeadingZeros; i++)
        {
            if (bThousand && i > 0 && i == aGrouping.getPos())
            {
                sString.insert(0, rThSep);
                aGrouping.advance();
            }
            sString.insert(0, '0');
        }
        if ( bThousand )
        {
            sal_Int32 nDigits = (eType == SvNumFormatType::SCIENTIFIC) ?  3*((nLeadingZeros-1)/3 + 1) : nDigitsInFirstGroup + 1;
            for (sal_Int32 i = nLeadingZeros; i < nDigits; i++)
            {
                if ( i % nDigitsInFirstGroup == 0 )
                    sString.insert(0, rThSep);
                sString.insert(0, '#');
            }
        }
    }
    if (nPrecision > 0 && eType != SvNumFormatType::FRACTION )
    {
        sString.append(GetNumDecimalSep());
        padToLength(sString, sString.getLength() + nPrecision, '0');
    }
    if (eType == SvNumFormatType::PERCENT)
    {
        sString.append('%');
    }
    else if (eType == SvNumFormatType::SCIENTIFIC)
    {
      OUStringBuffer sOldFormatString = pFormat->GetFormatstring();
      sal_Int32 nIndexE = ImpPosToken( sOldFormatString, 'E' );
      if (nIndexE > -1)
      {
        sal_Int32 nIndexSep = ImpPosToken( sOldFormatString, ';', nIndexE );
        if (nIndexSep > nIndexE)
            sString.append( sOldFormatString.copy(nIndexE, nIndexSep - nIndexE) );
        else
            sString.append( sOldFormatString.copy(nIndexE) );
      }
    }
    else if (eType == SvNumFormatType::CURRENCY)
    {
        OUStringBuffer sNegStr(sString);
        OUString aCurr;
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
                pEntry->CompletePositiveFormatString( sString, bBank, nPosiForm );
                pEntry->CompleteNegativeFormatString( sNegStr, bBank, nNegaForm );
            }
            else
            {   // assume currency abbreviation (AKA banking symbol), not symbol
                sal_uInt16 nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat(
                    xLocaleData->getCurrPositiveFormat(),
                    xLocaleData->getCurrPositiveFormat(), true );
                sal_uInt16 nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat(
                    xLocaleData->getCurrNegativeFormat(),
                    xLocaleData->getCurrNegativeFormat(), true );
                NfCurrencyEntry::CompletePositiveFormatString( sString, aCurr, nPosiForm );
                NfCurrencyEntry::CompleteNegativeFormatString( sNegStr, aCurr, nNegaForm );
            }
        }
        else
        {   // "automatic" old style
            OUString aSymbol, aAbbrev;
            GetCompatibilityCurrency( aSymbol, aAbbrev );
            NfCurrencyEntry::CompletePositiveFormatString( sString,
                                aSymbol, xLocaleData->getCurrPositiveFormat() );
            NfCurrencyEntry::CompleteNegativeFormatString( sNegStr,
                                aSymbol, xLocaleData->getCurrNegativeFormat() );
        }
        if (IsRed)
        {
            sString.append(';');
            sString.append('[');
            sString.append(pFormatScanner->GetRedString());
            sString.append(']');
        }
        else
        {
            sString.append(';');
        }
        sString.append(sNegStr.makeStringAndClear());
    }
    else if (eType == SvNumFormatType::FRACTION)
    {
        OUString aIntegerFractionDelimiterString = pFormat->GetIntegerFractionDelimiterString( 0 );
        if ( aIntegerFractionDelimiterString == " " )
            sString.append( aIntegerFractionDelimiterString );
        else
        {
            sString.append( '"' );
            sString.append( aIntegerFractionDelimiterString );
            sString.append( '"' );
        }
        sString.append( pFormat->GetNumeratorString( 0 ) );
        sString.append( '/' );
        if ( nPrecision > 0 )
            padToLength(sString, sString.getLength() + nPrecision, '?');
        else
            sString.append( '#' );
    }
    if (eType != SvNumFormatType::CURRENCY)
    {
        bool insertBrackets = false;
        if ( eType != SvNumFormatType::UNDEFINED)
        {
            insertBrackets = pFormat->IsNegativeInBracket();
        }
        if (IsRed || insertBrackets)
        {
            OUStringBuffer sTmpStr(sString);

            if (pFormat && pFormat->HasPositiveBracketPlaceholder())
            {
                 sTmpStr.append('_');
                 sTmpStr.append(')');
            }
            sTmpStr.append(';');

            if (IsRed)
            {
                sTmpStr.append('[');
                sTmpStr.append(pFormatScanner->GetRedString());
                sTmpStr.append(']');
            }

            if (insertBrackets)
            {
                sTmpStr.append('(');
                sTmpStr.append(sString.toString());
                sTmpStr.append(')');
            }
            else
            {
                sTmpStr.append('-');
                sTmpStr.append(sString.toString());
            }
            sString = sTmpStr;
        }
    }
    return sString.makeStringAndClear();
}

bool SvNumberFormatter::IsUserDefined(const OUString& sStr,
                                      LanguageType eLnge)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (eLnge == LANGUAGE_DONTKNOW)
    {
        eLnge = IniLnge;
    }
    sal_uInt32 CLOffset = ImpGenerateCL(eLnge);     // create new standard formats if necessary
    eLnge = ActLnge;

    sal_uInt32 nKey = ImpIsEntry(sStr, CLOffset, eLnge);
    if (nKey == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        return true;
    }
    SvNumberformat* pEntry = GetFormatEntry( nKey );
    return pEntry && (pEntry->GetType() & SvNumFormatType::DEFINED);
}

sal_uInt32 SvNumberFormatter::GetEntryKey(const OUString& sStr,
                                          LanguageType eLnge)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (eLnge == LANGUAGE_DONTKNOW)
    {
        eLnge = IniLnge;
    }
    sal_uInt32 CLOffset = ImpGenerateCL(eLnge);     // create new standard formats if necessary
    return ImpIsEntry(sStr, CLOffset, eLnge);
}

sal_uInt32 SvNumberFormatter::GetStandardIndex(LanguageType eLnge)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (eLnge == LANGUAGE_DONTKNOW)
    {
        eLnge = IniLnge;
    }
    return GetStandardFormat(SvNumFormatType::NUMBER, eLnge);
}

SvNumFormatType SvNumberFormatter::GetType(sal_uInt32 nFIndex) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    SvNumFormatType eType;
    const SvNumberformat* pFormat = GetFormatEntry( nFIndex );
    if (!pFormat)
    {
        eType = SvNumFormatType::UNDEFINED;
    }
    else
    {
        eType = pFormat->GetMaskedType();
        if (eType == SvNumFormatType::ALL)
        {
            eType = SvNumFormatType::DEFINED;
        }
    }
    return eType;
}

void SvNumberFormatter::ClearMergeTable()
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if ( pMergeTable )
    {
        pMergeTable->clear();
    }
}

SvNumberFormatterIndexTable* SvNumberFormatter::MergeFormatter(SvNumberFormatter& rTable)
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if ( pMergeTable )
    {
        ClearMergeTable();
    }
    else
    {
        pMergeTable.reset( new SvNumberFormatterIndexTable );
    }

    sal_uInt32 nCLOffset = 0;
    sal_uInt32 nOldKey, nOffset, nNewKey;

    for (const auto& rEntry : rTable.aFTable)
    {
        SvNumberformat* pFormat = rEntry.second.get();
        nOldKey = rEntry.first;
        nOffset = nOldKey % SV_COUNTRY_LANGUAGE_OFFSET;     // relative index
        if (nOffset == 0)                                   // 1st format of CL
        {
            nCLOffset = ImpGenerateCL(pFormat->GetLanguage());
        }
        if (nOffset <= SV_MAX_COUNT_STANDARD_FORMATS)     // Std.form.
        {
            nNewKey = nCLOffset + nOffset;
            if (aFTable.find( nNewKey) == aFTable.end())    // not already present
            {
                std::unique_ptr<SvNumberformat> pNewEntry(new SvNumberformat( *pFormat, *pFormatScanner ));
                if (!aFTable.emplace( nNewKey, std::move(pNewEntry)).second)
                {
                    SAL_WARN( "svl.numbers", "SvNumberFormatter::MergeFormatter: dup position");
                }
            }
            if (nNewKey != nOldKey)                     // new index
            {
                (*pMergeTable)[nOldKey] = nNewKey;
            }
        }
        else                                            // user defined
        {
            std::unique_ptr<SvNumberformat> pNewEntry(new SvNumberformat( *pFormat, *pFormatScanner ));
            nNewKey = ImpIsEntry(pNewEntry->GetFormatstring(),
                                 nCLOffset,
                                 pFormat->GetLanguage());
            if (nNewKey == NUMBERFORMAT_ENTRY_NOT_FOUND) // only if not present yet
            {
                SvNumberformat* pStdFormat = GetFormatEntry(nCLOffset + ZF_STANDARD);
                sal_uInt32 nPos = nCLOffset + pStdFormat->GetLastInsertKey( SvNumberformat::FormatterPrivateAccess() );
                nNewKey = nPos+1;
                if (nNewKey - nCLOffset >= SV_COUNTRY_LANGUAGE_OFFSET)
                {
                    SAL_WARN( "svl.numbers", "SvNumberFormatter::MergeFormatter: too many formats for CL");
                }
                else if (!aFTable.emplace( nNewKey, std::move(pNewEntry)).second)
                {
                    SAL_WARN( "svl.numbers", "SvNumberFormatter::MergeFormatter: dup position");
                }
                else
                {
                    pStdFormat->SetLastInsertKey(static_cast<sal_uInt16>(nNewKey - nCLOffset),
                            SvNumberformat::FormatterPrivateAccess());
                }
            }
            if (nNewKey != nOldKey)                     // new index
            {
                (*pMergeTable)[nOldKey] = nNewKey;
            }
        }
    }
    return pMergeTable.get();
}


SvNumberFormatterMergeMap SvNumberFormatter::ConvertMergeTableToMap()
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (!HasMergeFormatTable())
    {
        return SvNumberFormatterMergeMap();
    }
    SvNumberFormatterMergeMap aMap;
    for (const auto& rEntry : *pMergeTable)
    {
        sal_uInt32 nOldKey = rEntry.first;
        aMap[ nOldKey ] = rEntry.second;
    }
    ClearMergeTable();
    return aMap;
}


sal_uInt32 SvNumberFormatter::GetFormatForLanguageIfBuiltIn( sal_uInt32 nFormat,
                                                             LanguageType eLnge )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if ( eLnge == LANGUAGE_DONTKNOW )
    {
        eLnge = IniLnge;
    }
    if ( nFormat < SV_COUNTRY_LANGUAGE_OFFSET && eLnge == IniLnge )
    {
        return nFormat;     // it stays as it is
    }
    sal_uInt32 nOffset = nFormat % SV_COUNTRY_LANGUAGE_OFFSET;  // relative index
    if ( nOffset > SV_MAX_COUNT_STANDARD_FORMATS )
    {
        return nFormat;    // not a built-in format
    }
    sal_uInt32 nCLOffset = ImpGenerateCL(eLnge);    // create new standard formats if necessary
    return nCLOffset + nOffset;
}


sal_uInt32 SvNumberFormatter::GetFormatIndex( NfIndexTableOffset nTabOff,
                                              LanguageType eLnge )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (nTabOff >= NF_INDEX_TABLE_ENTRIES)
        return NUMBERFORMAT_ENTRY_NOT_FOUND;

    if (eLnge == LANGUAGE_DONTKNOW)
        eLnge = IniLnge;

    if (indexTable[nTabOff] == NUMBERFORMAT_ENTRY_NOT_FOUND)
        return NUMBERFORMAT_ENTRY_NOT_FOUND;

    sal_uInt32 nCLOffset = ImpGenerateCL(eLnge);    // create new standard formats if necessary

    return nCLOffset + indexTable[nTabOff];
}


NfIndexTableOffset SvNumberFormatter::GetIndexTableOffset( sal_uInt32 nFormat ) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    sal_uInt32 nOffset = nFormat % SV_COUNTRY_LANGUAGE_OFFSET;      // relative index
    if ( nOffset > SV_MAX_COUNT_STANDARD_FORMATS )
    {
        return NF_INDEX_TABLE_ENTRIES;      // not a built-in format
    }

    for ( sal_uInt16 j = 0; j < NF_INDEX_TABLE_ENTRIES; j++ )
    {
        if (indexTable[j] == nOffset)
            return static_cast<NfIndexTableOffset>(j);
    }
    return NF_INDEX_TABLE_ENTRIES;      // bad luck
}

void SvNumberFormatter::SetEvalDateFormat( NfEvalDateFormat eEDF )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    eEvalDateFormat = eEDF;
}

NfEvalDateFormat SvNumberFormatter::GetEvalDateFormat() const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    return eEvalDateFormat;
}

void SvNumberFormatter::SetYear2000( sal_uInt16 nVal )
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    pStringScanner->SetYear2000( nVal );
}


sal_uInt16 SvNumberFormatter::GetYear2000() const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
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
    if (!utl::ConfigManager::IsFuzzing())
        return static_cast<sal_uInt16>(::utl::MiscCfg().GetYear2000());
    return 1930;
}


// static
const NfCurrencyTable& SvNumberFormatter::GetTheCurrencyTable()
{
    while ( !bCurrencyTableInitialized )
        ImpInitCurrencyTable();
    return theCurrencyTable::get();
}


// static
const NfCurrencyEntry* SvNumberFormatter::MatchSystemCurrency()
{
    // MUST call GetTheCurrencyTable() before accessing nSystemCurrencyPosition
    const NfCurrencyTable& rTable = GetTheCurrencyTable();
    return nSystemCurrencyPosition ? &rTable[nSystemCurrencyPosition] : nullptr;
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
const NfCurrencyEntry* SvNumberFormatter::GetCurrencyEntry(const OUString& rAbbrev, LanguageType eLang )
{
    eLang = MsLangId::getRealLanguage( eLang );
    const NfCurrencyTable& rTable = GetTheCurrencyTable();
    sal_uInt16 nCount = rTable.size();
    for ( sal_uInt16 j = 0; j < nCount; j++ )
    {
        if ( rTable[j].GetLanguage() == eLang &&
             rTable[j].GetBankSymbol() == rAbbrev )
        {
            return &rTable[j];
        }
    }
    return nullptr;
}


// static
const NfCurrencyEntry* SvNumberFormatter::GetLegacyOnlyCurrencyEntry( const OUString& rSymbol,
                                                                      const OUString& rAbbrev )
{
    GetTheCurrencyTable();      // just for initialization
    const NfCurrencyTable& rTable = theLegacyOnlyCurrencyTable::get();
    sal_uInt16 nCount = rTable.size();
    for ( sal_uInt16 j = 0; j < nCount; j++ )
    {
        if ( rTable[j].GetSymbol() == rSymbol &&
             rTable[j].GetBankSymbol() == rAbbrev )
        {
            return &rTable[j];
        }
    }
    return nullptr;
}


// static
IMPL_STATIC_LINK_NOARG( SvNumberFormatter, CurrencyChangeLink, LinkParamNone*, void )
{
    OUString aAbbrev;
    LanguageType eLang = LANGUAGE_SYSTEM;
    SvtSysLocaleOptions().GetCurrencyAbbrevAndLanguage( aAbbrev, eLang );
    SetDefaultSystemCurrency( aAbbrev, eLang );
}


// static
void SvNumberFormatter::SetDefaultSystemCurrency( const OUString& rAbbrev, LanguageType eLang )
{
    ::osl::MutexGuard aGuard( GetGlobalMutex() );
    if ( eLang == LANGUAGE_SYSTEM )
    {
        eLang = SvtSysLocale().GetLanguageTag().getLanguageType();
    }
    const NfCurrencyTable& rTable = GetTheCurrencyTable();
    sal_uInt16 nCount = rTable.size();
    if ( !rAbbrev.isEmpty() )
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


void SvNumberFormatter::InvalidateDateAcceptancePatterns()
{
    pStringScanner->InvalidateDateAcceptancePatterns();
}


sal_uInt32 SvNumberFormatter::ImpGetDefaultSystemCurrencyFormat()
{
    if ( nDefaultSystemCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        sal_Int32 nCheck;
        SvNumFormatType nType;
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
    DefaultFormatKeysMap::const_iterator it = aDefaultFormatKeys.find( CLOffset + ZF_STANDARD_CURRENCY );
    sal_uInt32 nDefaultCurrencyFormat = (it != aDefaultFormatKeys.end() ?
            it->second : NUMBERFORMAT_ENTRY_NOT_FOUND);
    if ( nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        // look for a defined standard
        sal_uInt32 nStopKey = CLOffset + SV_COUNTRY_LANGUAGE_OFFSET;
        sal_uInt32 nKey(0);
        auto it2 = aFTable.lower_bound( CLOffset );
        while ( it2 != aFTable.end() && (nKey = it2->first) >= CLOffset && nKey < nStopKey )
        {
            const SvNumberformat* pEntry = it2->second.get();
            if ( pEntry->IsStandard() && (pEntry->GetType() & SvNumFormatType::CURRENCY) )
            {
                nDefaultCurrencyFormat = nKey;
                break;  // while
            }
            ++it2;
        }

        if ( nDefaultCurrencyFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {   // none found, create one
            sal_Int32 nCheck;
            NfWSStringsDtor aCurrList;
            sal_uInt16 nDefault = GetCurrencyFormatStrings( aCurrList,
                GetCurrencyEntry( ActLnge ), false );
            DBG_ASSERT( aCurrList.size(), "where is the NewCurrency standard format?" );
            if ( !aCurrList.empty() )
            {
                // if already loaded or user defined nDefaultSystemCurrencyFormat
                // will be set to the right value
                SvNumFormatType nType;
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
// true: continue; false: break loop, if pFoundEntry==NULL dupe found
bool SvNumberFormatter::ImpLookupCurrencyEntryLoopBody(
    const NfCurrencyEntry*& pFoundEntry, bool& bFoundBank, const NfCurrencyEntry* pData,
    sal_uInt16 nPos, const OUString& rSymbol )
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
            pFoundEntry = nullptr;
            return false;   // break loop, not unique
        }
        if ( nPos == 0 )
        {   // first entry is SYSTEM
            pFoundEntry = MatchSystemCurrency();
            if ( pFoundEntry )
            {
                return false;   // break loop
                // even if there are more matching entries
                // this one is probably the one we are looking for
            }
            else
            {
                pFoundEntry = pData;
            }
        }
        else
        {
            pFoundEntry = pData;
        }
    }
    return true;
}


bool SvNumberFormatter::GetNewCurrencySymbolString( sal_uInt32 nFormat, OUString& rStr,
                                                    const NfCurrencyEntry** ppEntry /* = NULL */,
                                                    bool* pBank /* = NULL */ ) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if ( ppEntry )
        *ppEntry = nullptr;
    if ( pBank )
        *pBank = false;

    const SvNumberformat* pFormat = GetFormatEntry(nFormat);
    if ( pFormat )
    {
        OUStringBuffer sBuff(128); // guess-estimate of a value that will pretty much guarantee no re-alloc
        OUString aSymbol, aExtension;
        if ( pFormat->GetNewCurrencySymbol( aSymbol, aExtension ) )
        {
            if ( ppEntry )
            {
                bool bFoundBank = false;
                // we definitely need an entry matching the format code string
                const NfCurrencyEntry* pFoundEntry = GetCurrencyEntry(
                    bFoundBank, aSymbol, aExtension, pFormat->GetLanguage(),
                    true );
                if ( pFoundEntry )
                {
                    *ppEntry = pFoundEntry;
                    if ( pBank )
                        *pBank = bFoundBank;
                    rStr = pFoundEntry->BuildSymbolString(bFoundBank);
                }
            }
            if ( rStr.isEmpty() )
            {   // analog to BuildSymbolString
                sBuff.append("[$");
                if ( aSymbol.indexOf( '-' ) != -1 ||
                        aSymbol.indexOf( ']' ) != -1 )
                {
                    sBuff.append('"');
                    sBuff.append( aSymbol);
                    sBuff.append('"');
                }
                else
                {
                    sBuff.append(aSymbol);
                }
                if ( !aExtension.isEmpty() )
                {
                    sBuff.append(aExtension);
                }
                sBuff.append(']');
            }
            rStr = sBuff.toString();
            return true;
        }
    }
    rStr.clear();
    return false;
}


// static
const NfCurrencyEntry* SvNumberFormatter::GetCurrencyEntry( bool & bFoundBank,
                                                            const OUString& rSymbol,
                                                            const OUString& rExtension,
                                                            LanguageType eFormatLanguage,
                                                            bool bOnlyStringLanguage )
{
    sal_Int32 nExtLen = rExtension.getLength();
    LanguageType eExtLang;
    if ( nExtLen )
    {
        // rExtension should be a 16-bit hex value max FFFF which may contain a
        // leading "-" separator (that is not a minus sign, but toInt32 can be
        // used to parse it, with post-processing as necessary):
        sal_Int32 nExtLang = rExtension.toInt32( 16 );
        if ( !nExtLang )
        {
            eExtLang = LANGUAGE_DONTKNOW;
        }
        else
        {
            eExtLang = LanguageType((nExtLang < 0) ? -nExtLang : nExtLang);
        }
    }
    else
    {
        eExtLang = LANGUAGE_DONTKNOW;
    }
    const NfCurrencyEntry* pFoundEntry = nullptr;
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
                  (eLang == LANGUAGE_SYSTEM)))
            {
                bCont = ImpLookupCurrencyEntryLoopBody( pFoundEntry, bFoundBank,
                                                        &rTable[j], j, rSymbol );
            }
        }
    }

    // ok?
    if ( pFoundEntry || !bCont || (bOnlyStringLanguage && nExtLen) )
    {
        return pFoundEntry;
    }
    if ( !bOnlyStringLanguage )
    {
        // now try the language/country of the number format
        for ( sal_uInt16 j = 0; j < nCount && bCont; j++ )
        {
            LanguageType eLang = rTable[j].GetLanguage();
            if ( eLang == eFormatLanguage ||
                 ((eFormatLanguage == LANGUAGE_DONTKNOW) &&
                  (eLang == LANGUAGE_SYSTEM)))
            {
                bCont = ImpLookupCurrencyEntryLoopBody( pFoundEntry, bFoundBank,
                                                        &rTable[j], j, rSymbol );
            }
        }

        // ok?
        if ( pFoundEntry || !bCont )
        {
            return pFoundEntry;
        }
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


void SvNumberFormatter::GetCompatibilityCurrency( OUString& rSymbol, OUString& rAbbrev ) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    css::uno::Sequence< css::i18n::Currency2 >
        xCurrencies( xLocaleData->getAllCurrencies() );

    const css::i18n::Currency2 *pCurrencies = xCurrencies.getConstArray();
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
        {
            LocaleDataWrapper::outputCheckMessage( xLocaleData->
                                                   appendLocaleInfo( "GetCompatibilityCurrency: none?"));
        }
        rSymbol = xLocaleData->getCurrSymbol();
        rAbbrev = xLocaleData->getCurrBankSymbol();
    }
}


static void lcl_CheckCurrencySymbolPosition( const NfCurrencyEntry& rCurr )
{
    switch ( rCurr.GetPositiveFormat() )
    {
    case 0:                                         // $1
    case 1:                                         // 1$
    case 2:                                         // $ 1
    case 3:                                         // 1 $
        break;
    default:
        LocaleDataWrapper::outputCheckMessage( "lcl_CheckCurrencySymbolPosition: unknown PositiveFormat");
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
        LocaleDataWrapper::outputCheckMessage( "lcl_CheckCurrencySymbolPosition: unknown NegativeFormat");
        break;
    }
}

// static
bool SvNumberFormatter::IsLocaleInstalled( LanguageType eLang )
{
    // The set is initialized as a side effect of the currency table
    // created, make sure that exists, which usually is the case unless a
    // SvNumberFormatter was never instantiated.
    GetTheCurrencyTable();
    const NfInstalledLocales &rInstalledLocales = theInstalledLocales::get();
    return rInstalledLocales.find( eLang) != rInstalledLocales.end();
}

// static
void SvNumberFormatter::ImpInitCurrencyTable()
{
    // Race condition possible:
    // ::osl::MutexGuard aGuard( GetMutex() );
    // while ( !bCurrencyTableInitialized )
    //      ImpInitCurrencyTable();
    static bool bInitializing = false;
    if ( bCurrencyTableInitialized || bInitializing )
    {
        return ;
    }
    bInitializing = true;

    LanguageType eSysLang = SvtSysLocale().GetLanguageTag().getLanguageType();
    std::unique_ptr<LocaleDataWrapper> pLocaleData(new LocaleDataWrapper(
        ::comphelper::getProcessComponentContext(),
        SvtSysLocale().GetLanguageTag() ));
    // get user configured currency
    OUString aConfiguredCurrencyAbbrev;
    LanguageType eConfiguredCurrencyLanguage = LANGUAGE_SYSTEM;
    SvtSysLocaleOptions().GetCurrencyAbbrevAndLanguage(
        aConfiguredCurrencyAbbrev, eConfiguredCurrencyLanguage );
    sal_uInt16 nSecondarySystemCurrencyPosition = 0;
    sal_uInt16 nMatchingSystemCurrencyPosition = 0;

    // First entry is SYSTEM:
    theCurrencyTable::get().insert(
        theCurrencyTable::get().begin(),
        std::make_unique<NfCurrencyEntry>(*pLocaleData, LANGUAGE_SYSTEM));
    sal_uInt16 nCurrencyPos = 1;

    css::uno::Sequence< css::lang::Locale > xLoc = LocaleDataWrapper::getInstalledLocaleNames();
    sal_Int32 nLocaleCount = xLoc.getLength();
    SAL_INFO( "svl.numbers", "number of locales: \"" << nLocaleCount << "\"" );
    css::lang::Locale const * const pLocales = xLoc.getConstArray();
    NfCurrencyTable &rCurrencyTable = theCurrencyTable::get();
    NfCurrencyTable &rLegacyOnlyCurrencyTable = theLegacyOnlyCurrencyTable::get();
    NfInstalledLocales &rInstalledLocales = theInstalledLocales::get();
    sal_uInt16 nLegacyOnlyCurrencyPos = 0;
    for ( sal_Int32 nLocale = 0; nLocale < nLocaleCount; nLocale++ )
    {
        LanguageType eLang = LanguageTag::convertToLanguageType( pLocales[nLocale], false);
        rInstalledLocales.insert( eLang);
        pLocaleData->setLanguageTag( LanguageTag( pLocales[nLocale]) );
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
        std::unique_ptr<NfCurrencyEntry> pEntry;
        if ( nDefault < nCurrencyCount )
        {
            pEntry.reset(new NfCurrencyEntry(pCurrencies[nDefault], *pLocaleData, eLang));
        }
        else
        {   // first or ShellsAndPebbles
            pEntry.reset(new NfCurrencyEntry(*pLocaleData, eLang));
        }
        if (LocaleDataWrapper::areChecksEnabled())
        {
            lcl_CheckCurrencySymbolPosition( *pEntry );
        }
        if ( !nSystemCurrencyPosition && !aConfiguredCurrencyAbbrev.isEmpty() &&
             pEntry->GetBankSymbol() == aConfiguredCurrencyAbbrev &&
             pEntry->GetLanguage() == eConfiguredCurrencyLanguage )
        {
            nSystemCurrencyPosition = nCurrencyPos;
        }
        if ( !nMatchingSystemCurrencyPosition &&
             pEntry->GetLanguage() == eSysLang )
        {
            nMatchingSystemCurrencyPosition = nCurrencyPos;
        }
        rCurrencyTable.insert(
                rCurrencyTable.begin() + nCurrencyPos++, std::move(pEntry));
        // all remaining currencies for each locale
        if ( nCurrencyCount > 1 )
        {
            sal_Int32 nCurrency;
            for ( nCurrency = 0; nCurrency < nCurrencyCount; nCurrency++ )
            {
                if (pCurrencies[nCurrency].LegacyOnly)
                {
                    rLegacyOnlyCurrencyTable.insert(
                        rLegacyOnlyCurrencyTable.begin() + nLegacyOnlyCurrencyPos++,
                        std::make_unique<NfCurrencyEntry>(
                            pCurrencies[nCurrency], *pLocaleData, eLang));
                }
                else if ( nCurrency != nDefault )
                {
                    pEntry.reset(new NfCurrencyEntry(pCurrencies[nCurrency], *pLocaleData, eLang));
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
                    {
                        pEntry.reset();
                    }
                    else
                    {
                        if ( !nSecondarySystemCurrencyPosition &&
                             (!aConfiguredCurrencyAbbrev.isEmpty() ?
                              pEntry->GetBankSymbol() == aConfiguredCurrencyAbbrev :
                              pEntry->GetLanguage() == eConfiguredCurrencyLanguage) )
                        {
                            nSecondarySystemCurrencyPosition = nCurrencyPos;
                        }
                        if ( !nMatchingSystemCurrencyPosition &&
                             pEntry->GetLanguage() ==  eSysLang )
                        {
                            nMatchingSystemCurrencyPosition = nCurrencyPos;
                        }
                        rCurrencyTable.insert(
                            rCurrencyTable.begin() + nCurrencyPos++, std::move(pEntry));
                    }
                }
            }
        }
    }
    if ( !nSystemCurrencyPosition )
    {
        nSystemCurrencyPosition = nSecondarySystemCurrencyPosition;
    }
    if ((!aConfiguredCurrencyAbbrev.isEmpty() && !nSystemCurrencyPosition) &&
        LocaleDataWrapper::areChecksEnabled())
    {
        LocaleDataWrapper::outputCheckMessage(
                "SvNumberFormatter::ImpInitCurrencyTable: configured currency not in I18N locale data.");
    }
    // match SYSTEM if no configured currency found
    if ( !nSystemCurrencyPosition )
    {
        nSystemCurrencyPosition = nMatchingSystemCurrencyPosition;
    }
    if ((aConfiguredCurrencyAbbrev.isEmpty() && !nSystemCurrencyPosition) &&
        LocaleDataWrapper::areChecksEnabled())
    {
        LocaleDataWrapper::outputCheckMessage(
                "SvNumberFormatter::ImpInitCurrencyTable: system currency not in I18N locale data.");
    }
    pLocaleData.reset();
    SvtSysLocaleOptions::SetCurrencyChangeLink( LINK( nullptr, SvNumberFormatter, CurrencyChangeLink ) );
    bInitializing = false;
    bCurrencyTableInitialized = true;
}


static void addToCurrencyFormatsList( NfWSStringsDtor& rStrArr, const OUString& rFormat )
{
    // Prevent duplicates even over subsequent calls of
    // GetCurrencyFormatStrings() with the same vector.
    if (std::find( rStrArr.begin(), rStrArr.end(), rFormat) == rStrArr.end())
        rStrArr.push_back( rFormat);
}


sal_uInt16 SvNumberFormatter::GetCurrencyFormatStrings( NfWSStringsDtor& rStrArr,
                                                        const NfCurrencyEntry& rCurr,
                                                        bool bBank ) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    OUString aRed = "["
                  + pFormatScanner->GetRedString()
                  + "]";

    sal_uInt16 nDefault = 0;
    if ( bBank )
    {
        // Only bank symbols.
        OUString aPositiveBank = rCurr.BuildPositiveFormatString(true, *xLocaleData);
        OUString aNegativeBank = rCurr.BuildNegativeFormatString(true, *xLocaleData );

        OUString format1 = aPositiveBank
                         + ";"
                         + aNegativeBank;
        addToCurrencyFormatsList( rStrArr, format1);

        OUString format2 = aPositiveBank
                         + ";"
                         + aRed
                         + aNegativeBank;
        addToCurrencyFormatsList( rStrArr, format2);

        nDefault = rStrArr.size() - 1;
    }
    else
    {
        // Mixed formats like in SvNumberFormatter::ImpGenerateFormats() but no
        // duplicates if no decimals in currency.
        OUString aPositive = rCurr.BuildPositiveFormatString(false, *xLocaleData );
        OUString aNegative = rCurr.BuildNegativeFormatString(false, *xLocaleData );
        OUString format1;
        OUString format2;
        OUString format3;
        OUString format4;
        OUString format5;
        if (rCurr.GetDigits())
        {
            OUString aPositiveNoDec = rCurr.BuildPositiveFormatString(false, *xLocaleData, 0);
            OUString aNegativeNoDec = rCurr.BuildNegativeFormatString(false, *xLocaleData, 0 );
            OUString aPositiveDashed = rCurr.BuildPositiveFormatString(false, *xLocaleData, 2);
            OUString aNegativeDashed = rCurr.BuildNegativeFormatString(false, *xLocaleData, 2);

            format1 = aPositiveNoDec
                    + ";"
                    + aNegativeNoDec;

            format3 = aPositiveNoDec
                    + ";"
                    + aRed
                    + aNegativeNoDec;

            format5 = aPositiveDashed
                    + ";"
                    + aRed
                    + aNegativeDashed;
        }

        format2 = aPositive
                + ";"
                + aNegative;

        format4 = aPositive
                + ";"
                + aRed
                + aNegative;

        if (rCurr.GetDigits())
        {
            addToCurrencyFormatsList( rStrArr, format1);
        }
        addToCurrencyFormatsList( rStrArr, format2);
        if (rCurr.GetDigits())
        {
            addToCurrencyFormatsList( rStrArr, format3);
        }
        addToCurrencyFormatsList( rStrArr, format4);
        nDefault = rStrArr.size() - 1;
        if (rCurr.GetDigits())
        {
            addToCurrencyFormatsList( rStrArr, format5);
        }
    }
    return nDefault;
}

sal_uInt32 SvNumberFormatter::GetMergeFormatIndex( sal_uInt32 nOldFmt ) const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    if (pMergeTable)
    {
        SvNumberFormatterIndexTable::const_iterator it = pMergeTable->find(nOldFmt);
        if (it != pMergeTable->end())
        {
            return it->second;
        }
    }
    return nOldFmt;
}

bool SvNumberFormatter::HasMergeFormatTable() const
{
    ::osl::MutexGuard aGuard( GetInstanceMutex() );
    return pMergeTable && !pMergeTable->empty();
}

// static
sal_uInt16 SvNumberFormatter::ExpandTwoDigitYear( sal_uInt16 nYear, sal_uInt16 nTwoDigitYearStart )
{
    if ( nYear < 100 )
    {
        if ( nYear < (nTwoDigitYearStart % 100) )
        {
            return nYear + (((nTwoDigitYearStart / 100) + 1) * 100);
        }
        else
        {
            return nYear + ((nTwoDigitYearStart / 100) * 100);
        }
    }
    return nYear;
}

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


NfCurrencyEntry::NfCurrencyEntry( const css::i18n::Currency & rCurr,
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

OUString NfCurrencyEntry::BuildSymbolString(bool bBank,
                                            bool bWithoutExtension) const
{
    OUStringBuffer aBuf("[$");
    if (bBank)
    {
        aBuf.append(aBankSymbol);
    }
    else
    {
        if ( aSymbol.indexOf( '-' ) >= 0 ||
             aSymbol.indexOf( ']' ) >= 0)
        {
            aBuf.append('"').append(aSymbol).append('"');
        }
        else
        {
            aBuf.append(aSymbol);
        }
        if ( !bWithoutExtension && eLanguage != LANGUAGE_DONTKNOW && eLanguage != LANGUAGE_SYSTEM )
        {
            sal_Int32 nLang = static_cast<sal_uInt16>(eLanguage);
            aBuf.append('-').append( OUString::number(nLang, 16).toAsciiUpperCase());
        }
    }
    aBuf.append(']');
    return aBuf.makeStringAndClear();
}

OUString NfCurrencyEntry::Impl_BuildFormatStringNumChars( const LocaleDataWrapper& rLoc,
                                                          sal_uInt16 nDecimalFormat) const
{
    OUStringBuffer aBuf;
    aBuf.append('#').append(rLoc.getNumThousandSep()).append("##0");
    if (nDecimalFormat && nDigits)
    {
        aBuf.append(rLoc.getNumDecimalSep());
        sal_Unicode cDecimalChar = nDecimalFormat == 2 ? '-' : cZeroChar;
        for (sal_uInt16 i = 0; i < nDigits; ++i)
        {
            aBuf.append(cDecimalChar);
        }
    }
    return aBuf.makeStringAndClear();
}


OUString NfCurrencyEntry::BuildPositiveFormatString(bool bBank, const LocaleDataWrapper& rLoc,
                                                    sal_uInt16 nDecimalFormat) const
{
    OUStringBuffer sBuf(Impl_BuildFormatStringNumChars(rLoc, nDecimalFormat));
    sal_uInt16 nPosiForm = NfCurrencyEntry::GetEffectivePositiveFormat( rLoc.getCurrPositiveFormat(),
                                                                        nPositiveFormat, bBank );
    CompletePositiveFormatString(sBuf, bBank, nPosiForm);
    return sBuf.makeStringAndClear();
}


OUString NfCurrencyEntry::BuildNegativeFormatString(bool bBank,
            const LocaleDataWrapper& rLoc, sal_uInt16 nDecimalFormat ) const
{
    OUStringBuffer sBuf(Impl_BuildFormatStringNumChars(rLoc, nDecimalFormat));
    sal_uInt16 nNegaForm = NfCurrencyEntry::GetEffectiveNegativeFormat( rLoc.getCurrNegativeFormat(),
                                                                        nNegativeFormat, bBank );
    CompleteNegativeFormatString(sBuf, bBank, nNegaForm);
    return sBuf.makeStringAndClear();
}


void NfCurrencyEntry::CompletePositiveFormatString(OUStringBuffer& rStr, bool bBank,
                                                   sal_uInt16 nPosiForm) const
{
    OUString aSymStr = BuildSymbolString(bBank);
    NfCurrencyEntry::CompletePositiveFormatString( rStr, aSymStr, nPosiForm );
}


void NfCurrencyEntry::CompleteNegativeFormatString(OUStringBuffer& rStr, bool bBank,
                                                   sal_uInt16 nNegaForm) const
{
    OUString aSymStr = BuildSymbolString(bBank);
    NfCurrencyEntry::CompleteNegativeFormatString( rStr, aSymStr, nNegaForm );
}


// static
void NfCurrencyEntry::CompletePositiveFormatString(OUStringBuffer& rStr, const OUString& rSymStr,
                                                   sal_uInt16 nPositiveFormat)
{
    switch( nPositiveFormat )
    {
        case 0:                                         // $1
            rStr.insert(0, rSymStr);
        break;
        case 1:                                         // 1$
            rStr.append(rSymStr);
        break;
        case 2:                                         // $ 1
        {
            rStr.insert(0, ' ');
            rStr.insert(0, rSymStr);
        }
        break;
        case 3:                                         // 1 $
        {
            rStr.append(' ');
            rStr.append(rSymStr);
        }
        break;
        default:
            SAL_WARN( "svl.numbers", "NfCurrencyEntry::CompletePositiveFormatString: unknown option");
        break;
    }
}


// static
void NfCurrencyEntry::CompleteNegativeFormatString(OUStringBuffer& rStr,
                                                   const OUString& rSymStr,
                                                   sal_uInt16 nNegativeFormat)
{
    switch( nNegativeFormat )
    {
        case 0:                                         // ($1)
        {
            rStr.insert(0, rSymStr);
            rStr.insert(0, '(');
            rStr.append(')');
        }
        break;
        case 1:                                         // -$1
        {
            rStr.insert(0, rSymStr);
            rStr.insert(0, '-');
        }
        break;
        case 2:                                         // $-1
        {
            rStr.insert(0, '-');
            rStr.insert(0, rSymStr);
        }
        break;
        case 3:                                         // $1-
        {
            rStr.insert(0, rSymStr);
            rStr.append('-');
        }
        break;
        case 4:                                         // (1$)
        {
            rStr.insert(0, '(');
            rStr.append(rSymStr);
            rStr.append(')');
        }
        break;
        case 5:                                         // -1$
        {
            rStr.append(rSymStr);
            rStr.insert(0, '-');
        }
        break;
        case 6:                                         // 1-$
        {
            rStr.append('-');
            rStr.append(rSymStr);
        }
        break;
        case 7:                                         // 1$-
        {
            rStr.append(rSymStr);
            rStr.append('-');
        }
        break;
        case 8:                                         // -1 $
        {
            rStr.append(' ');
            rStr.append(rSymStr);
            rStr.insert(0, '-');
        }
        break;
        case 9:                                         // -$ 1
        {
            rStr.insert(0, ' ');
            rStr.insert(0, rSymStr);
            rStr.insert(0, '-');
        }
        break;
        case 10:                                        // 1 $-
        {
            rStr.append(' ');
            rStr.append(rSymStr);
            rStr.append('-');
        }
        break;
        case 11:                                        // $ -1
        {
            rStr.insert(0, " -");
            rStr.insert(0, rSymStr);
        }
        break;
        case 12 :                                       // $ 1-
        {
            rStr.insert(0, ' ');
            rStr.insert(0, rSymStr);
            rStr.append('-');
        }
        break;
        case 13 :                                       // 1- $
        {
            rStr.append('-');
            rStr.append(' ');
            rStr.append(rSymStr);
        }
        break;
        case 14 :                                       // ($ 1)
        {
            rStr.insert(0, ' ');
            rStr.insert(0, rSymStr);
            rStr.insert(0, '(');
            rStr.append(')');
        }
        break;
        case 15 :                                       // (1 $)
        {
            rStr.insert(0, '(');
            rStr.append(' ');
            rStr.append(rSymStr);
            rStr.append(')');
        }
        break;
        default:
            SAL_WARN( "svl.numbers", "NfCurrencyEntry::CompleteNegativeFormatString: unknown option");
        break;
    }
}


// static
sal_uInt16 NfCurrencyEntry::GetEffectivePositiveFormat( sal_uInt16 nIntlFormat,
                                                        sal_uInt16 nCurrFormat, bool bBank )
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
            SAL_WARN( "svl.numbers", "NfCurrencyEntry::GetEffectivePositiveFormat: unknown option");
            break;
        }
        return nIntlFormat;
#endif
    }
    else
        return nCurrFormat;
}


//! Call this only if nCurrFormat is really with parentheses!
static sal_uInt16 lcl_MergeNegativeParenthesisFormat( sal_uInt16 nIntlFormat, sal_uInt16 nCurrFormat )
{
    short nSign = 0;        // -1:=bracket 0:=left, 1:=middle, 2:=right
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
        SAL_WARN( "svl.numbers", "lcl_MergeNegativeParenthesisFormat: unknown option");
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
//          nIntlFormat = 14;                           // ($ 1)
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
//          nIntlFormat = 15;                           // (1 $)
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
//          nIntlFormat = 14;                           // ($ 1)
            nIntlFormat = 9;                            // -$ 1
            break;
        case 15 :                                       // (1 $)
//          nIntlFormat = 15;                           // (1 $)
            nIntlFormat = 8;                            // -1 $
            break;
        default:
            SAL_WARN( "svl.numbers", "NfCurrencyEntry::GetEffectiveNegativeFormat: unknown option");
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
            SAL_WARN( "svl.numbers", "NfCurrencyEntry::GetEffectiveNegativeFormat: unknown option");
            break;
        }
    }
    return nIntlFormat;
}

const NfKeywordTable & SvNumberFormatter::GetKeywords( sal_uInt32 nKey )
{
    osl::MutexGuard aGuard( GetInstanceMutex() );
    const SvNumberformat* pFormat = GetFormatEntry( nKey);
    if (pFormat)
        ChangeIntl( pFormat->GetLanguage());
    else
        ChangeIntl( IniLnge);
    return pFormatScanner->GetKeywords();
}

const NfKeywordTable & SvNumberFormatter::GetEnglishKeywords() const
{
    return ImpSvNumberformatScan::GetEnglishKeywords();
}

const std::vector<Color> & SvNumberFormatter::GetStandardColors() const
{
    return ImpSvNumberformatScan::GetStandardColors();
}

size_t SvNumberFormatter::GetMaxDefaultColors() const
{
    return ImpSvNumberformatScan::GetMaxDefaultColors();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
