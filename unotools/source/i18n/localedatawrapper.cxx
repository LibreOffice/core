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

#include <limits>
#include <stdio.h>
#include <string>

#include <sal/log.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/digitgroupingiterator.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/debug.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/safeint.hxx>

#include <com/sun/star/i18n/KNumberFormatUsage.hpp>
#include <com/sun/star/i18n/KNumberFormatType.hpp>
#include <com/sun/star/i18n/LocaleData2.hpp>
#include <com/sun/star/i18n/NumberFormatIndex.hpp>
#include <com/sun/star/i18n/NumberFormatMapper.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/duration.hxx>
#include <o3tl/string_view.hxx>
#include <utility>

const sal_uInt16 nCurrFormatDefault = 0;

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

namespace
{
    uno::Sequence< lang::Locale > gInstalledLocales;
    std::vector< LanguageType > gInstalledLanguageTypes;
}

sal_uInt8 LocaleDataWrapper::nLocaleDataChecking = 0;

LocaleDataWrapper::LocaleDataWrapper(
            const Reference< uno::XComponentContext > & rxContext,
            LanguageTag aLanguageTag
            )
        :
        m_xContext( rxContext ),
        xLD( LocaleData2::create(rxContext) ),
        maLanguageTag(std::move( aLanguageTag ))
{
    loadData();
    loadDateAcceptancePatterns({});
}

LocaleDataWrapper::LocaleDataWrapper(
            LanguageTag aLanguageTag,
            const std::vector<OUString> & rOverrideDateAcceptancePatterns
            )
        :
        m_xContext( comphelper::getProcessComponentContext() ),
        xLD( LocaleData2::create(m_xContext) ),
        maLanguageTag(std::move( aLanguageTag ))
{
    loadData();
    loadDateAcceptancePatterns(rOverrideDateAcceptancePatterns);
}

LocaleDataWrapper::~LocaleDataWrapper()
{
}

const LanguageTag& LocaleDataWrapper::getLanguageTag() const
{
    return maLanguageTag;
}

const css::lang::Locale& LocaleDataWrapper::getMyLocale() const
{
    return maLanguageTag.getLocale();
}

void LocaleDataWrapper::loadData()
{
    const css::lang::Locale& rMyLocale = maLanguageTag.getLocale();

    {
        const Sequence< Currency2 > aCurrSeq = getAllCurrencies();
        if ( !aCurrSeq.hasElements() )
        {
            if (areChecksEnabled())
                outputCheckMessage("LocaleDataWrapper::getCurrSymbolsImpl: no currency at all, using ShellsAndPebbles");
            aCurrSymbol = "ShellsAndPebbles";
            aCurrBankSymbol = aCurrSymbol;
            nCurrPositiveFormat = nCurrNegativeFormat = nCurrFormatDefault;
            nCurrDigits = 2;
        }
        else
        {
            auto pCurr = std::find_if(aCurrSeq.begin(), aCurrSeq.end(),
                [](const Currency2& rCurr) { return rCurr.Default; });
            if ( pCurr == aCurrSeq.end() )
            {
                if (areChecksEnabled())
                {
                    outputCheckMessage( appendLocaleInfo( u"LocaleDataWrapper::getCurrSymbolsImpl: no default currency" ) );
                }
                pCurr = aCurrSeq.begin();
            }
            aCurrSymbol = pCurr->Symbol;
            aCurrBankSymbol = pCurr->BankSymbol;
            nCurrDigits = pCurr->DecimalPlaces;
        }
    }

    loadCurrencyFormats();

    {
        xDefaultCalendar.reset();
        xSecondaryCalendar.reset();
        const Sequence< Calendar2 > xCals = getAllCalendars();
        if (xCals.getLength() > 1)
        {
            auto pCal = std::find_if(xCals.begin(), xCals.end(),
                [](const Calendar2& rCal) { return !rCal.Default; });
            if (pCal != xCals.end())
                xSecondaryCalendar = std::make_shared<Calendar2>( *pCal);
        }
        auto pCal = xCals.begin();
        if (xCals.getLength() > 1)
        {
            pCal = std::find_if(xCals.begin(), xCals.end(),
                [](const Calendar2& rCal) { return rCal.Default; });
            if (pCal == xCals.end())
                pCal = xCals.begin();
        }
        xDefaultCalendar = std::make_shared<Calendar2>( *pCal);
    }

    loadDateOrders();

    try
    {
        aDateAcceptancePatterns = xLD->getDateAcceptancePatterns( rMyLocale );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getDateAcceptancePatterns" );
        aDateAcceptancePatterns = {};
    }


    loadDigitGrouping();

    try
    {
        aReservedWords = comphelper::sequenceToContainer<std::vector<OUString>>(xLD->getReservedWord( rMyLocale ));
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getReservedWord" );
    }

    try
    {
        aLocaleDataItem = xLD->getLocaleItem2( rMyLocale );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getLocaleItem" );
        static const css::i18n::LocaleDataItem2 aEmptyItem;
        aLocaleDataItem = aEmptyItem;
    }

    aLocaleItem[LocaleItem::DATE_SEPARATOR] = aLocaleDataItem.dateSeparator;
    aLocaleItem[LocaleItem::THOUSAND_SEPARATOR] = aLocaleDataItem.thousandSeparator;
    aLocaleItem[LocaleItem::DECIMAL_SEPARATOR] = aLocaleDataItem.decimalSeparator;
    aLocaleItem[LocaleItem::TIME_SEPARATOR] = aLocaleDataItem.timeSeparator;
    aLocaleItem[LocaleItem::TIME_100SEC_SEPARATOR] = aLocaleDataItem.time100SecSeparator;
    aLocaleItem[LocaleItem::LIST_SEPARATOR] = aLocaleDataItem.listSeparator;
    aLocaleItem[LocaleItem::SINGLE_QUOTATION_START] = aLocaleDataItem.quotationStart;
    aLocaleItem[LocaleItem::SINGLE_QUOTATION_END] = aLocaleDataItem.quotationEnd;
    aLocaleItem[LocaleItem::DOUBLE_QUOTATION_START] = aLocaleDataItem.doubleQuotationStart;
    aLocaleItem[LocaleItem::DOUBLE_QUOTATION_END] = aLocaleDataItem.doubleQuotationEnd;
    aLocaleItem[LocaleItem::MEASUREMENT_SYSTEM] = aLocaleDataItem.measurementSystem;
    aLocaleItem[LocaleItem::TIME_AM] = aLocaleDataItem.timeAM;
    aLocaleItem[LocaleItem::TIME_PM] = aLocaleDataItem.timePM;
    aLocaleItem[LocaleItem::LONG_DATE_DAY_OF_WEEK_SEPARATOR] = aLocaleDataItem.LongDateDayOfWeekSeparator;
    aLocaleItem[LocaleItem::LONG_DATE_DAY_SEPARATOR] = aLocaleDataItem.LongDateDaySeparator;
    aLocaleItem[LocaleItem::LONG_DATE_MONTH_SEPARATOR] = aLocaleDataItem.LongDateMonthSeparator;
    aLocaleItem[LocaleItem::LONG_DATE_YEAR_SEPARATOR] = aLocaleDataItem.LongDateYearSeparator;
    aLocaleItem[LocaleItem::DECIMAL_SEPARATOR_ALTERNATIVE] = aLocaleDataItem.decimalSeparatorAlternative;
}

/* FIXME-BCP47: locale data should provide a language tag instead that could be
 * passed on. */
css::i18n::LanguageCountryInfo LocaleDataWrapper::getLanguageCountryInfo() const
{
    try
    {
        return xLD->getLanguageCountryInfo( getMyLocale() );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getLanguageCountryInfo" );
    }
    return css::i18n::LanguageCountryInfo();
}

const css::i18n::LocaleDataItem2& LocaleDataWrapper::getLocaleItem() const
{
    return aLocaleDataItem;
}

css::uno::Sequence< css::i18n::Currency2 > LocaleDataWrapper::getAllCurrencies() const
{
    try
    {
        return xLD->getAllCurrencies2( getMyLocale() );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getAllCurrencies" );
    }
    return {};
}

css::uno::Sequence< css::i18n::FormatElement > LocaleDataWrapper::getAllFormats() const
{
    try
    {
        return xLD->getAllFormats( getMyLocale() );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getAllFormats" );
    }
    return {};
}

css::i18n::ForbiddenCharacters LocaleDataWrapper::getForbiddenCharacters() const
{
    try
    {
        return xLD->getForbiddenCharacters( getMyLocale() );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getForbiddenCharacters" );
    }
    return css::i18n::ForbiddenCharacters();
}

const css::uno::Sequence< css::lang::Locale > & LocaleDataWrapper::getAllInstalledLocaleNames() const
{
    uno::Sequence< lang::Locale > &rInstalledLocales = gInstalledLocales;

    if ( rInstalledLocales.hasElements() )
        return rInstalledLocales;

    try
    {
        rInstalledLocales = xLD->getAllInstalledLocaleNames();
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getAllInstalledLocaleNames" );
    }
    return rInstalledLocales;
}

// --- Impl and helpers ----------------------------------------------------

// static
const css::uno::Sequence< css::lang::Locale >& LocaleDataWrapper::getInstalledLocaleNames()
{
    const uno::Sequence< lang::Locale > &rInstalledLocales = gInstalledLocales;

    if ( !rInstalledLocales.hasElements() )
    {
        LocaleDataWrapper aLDW( ::comphelper::getProcessComponentContext(), LanguageTag( LANGUAGE_SYSTEM) );
        aLDW.getAllInstalledLocaleNames();
    }
    return rInstalledLocales;
}

// static
const std::vector< LanguageType >& LocaleDataWrapper::getInstalledLanguageTypes()
{
    std::vector< LanguageType > &rInstalledLanguageTypes = gInstalledLanguageTypes;

    if ( !rInstalledLanguageTypes.empty() )
        return rInstalledLanguageTypes;

    const css::uno::Sequence< css::lang::Locale > xLoc =  getInstalledLocaleNames();
    sal_Int32 nCount = xLoc.getLength();
    std::vector< LanguageType > xLang;
    xLang.reserve(nCount);
    for ( const auto& rLoc : xLoc )
    {
        LanguageTag aLanguageTag( rLoc );
        OUString aDebugLocale;
        if (areChecksEnabled())
        {
            aDebugLocale = aLanguageTag.getBcp47( false);
        }

        LanguageType eLang = aLanguageTag.getLanguageType( false);
        if (areChecksEnabled() && eLang == LANGUAGE_DONTKNOW)
        {
            OUString aMsg = "ConvertIsoNamesToLanguage: unknown MS-LCID for locale\n" +
                aDebugLocale;
            outputCheckMessage(aMsg);
        }

        if ( eLang == LANGUAGE_NORWEGIAN)       // no_NO, not Bokmal (nb_NO), not Nynorsk (nn_NO)
            eLang = LANGUAGE_DONTKNOW;  // don't offer "Unknown" language
        if ( eLang != LANGUAGE_DONTKNOW )
        {
            LanguageTag aBackLanguageTag( eLang);
            if ( aLanguageTag != aBackLanguageTag )
            {
                // In checks, exclude known problems because no MS-LCID defined
                // and default for Language found.
                if ( areChecksEnabled()
                        && aDebugLocale != "ar-SD"  // Sudan/ar
                        && aDebugLocale != "en-CB"  // Caribbean is not a country
//                      && aDebugLocale != "en-BG"  // ?!? Bulgaria/en
//                      && aDebugLocale != "es-BR"  // ?!? Brazil/es
                    )
                {
                    outputCheckMessage(Concat2View(
                        "ConvertIsoNamesToLanguage/ConvertLanguageToIsoNames: ambiguous locale (MS-LCID?)\n"
                        + aDebugLocale
                        + "  ->  0x"
                        + OUString::number(static_cast<sal_Int32>(static_cast<sal_uInt16>(eLang)), 16)
                        + "  ->  "
                        + aBackLanguageTag.getBcp47() ));
                }
                eLang = LANGUAGE_DONTKNOW;
            }
        }
        if ( eLang != LANGUAGE_DONTKNOW )
            xLang.push_back(eLang);
    }
    rInstalledLanguageTypes = xLang;

    return rInstalledLanguageTypes;
}

const OUString& LocaleDataWrapper::getOneLocaleItem( sal_Int16 nItem ) const
{
    if ( nItem >= LocaleItem::COUNT2 )
    {
        SAL_WARN( "unotools.i18n", "getOneLocaleItem: bounds" );
        return aLocaleItem[0];
    }
    return aLocaleItem[nItem];
}

const OUString& LocaleDataWrapper::getOneReservedWord( sal_Int16 nWord ) const
{
    if ( nWord < 0 || o3tl::make_unsigned(nWord) >= aReservedWords.size() )
    {
        SAL_WARN( "unotools.i18n", "getOneReservedWord: bounds" );
        static const OUString EMPTY;
        return EMPTY;
    }
    return aReservedWords[nWord];
}

// static
MeasurementSystem LocaleDataWrapper::mapMeasurementStringToEnum( std::u16string_view rMS )
{
//! TODO: could be cached too
    if ( o3tl::equalsIgnoreAsciiCase( rMS, u"metric" ) )
        return MeasurementSystem::Metric;
//! TODO: other measurement systems? => extend enum MeasurementSystem
    return MeasurementSystem::US;
}

bool LocaleDataWrapper::doesSecondaryCalendarUseEC( std::u16string_view rName ) const
{
    if (rName.empty())
        return false;

    // Check language tag first to avoid loading all calendars of this locale.
    LanguageTag aLoaded( getLoadedLanguageTag());
    const OUString& aBcp47( aLoaded.getBcp47());
    // So far determine only by locale, we know for a few.
    /* TODO: check date format codes? or add to locale data? */
    if (    aBcp47 != "ja-JP" &&
            aBcp47 != "lo-LA" &&
            aBcp47 != "zh-TW")
        return false;

    if (!xSecondaryCalendar)
        return false;
    if (!xSecondaryCalendar->Name.equalsIgnoreAsciiCase( rName))
        return false;

    return true;
}

const std::shared_ptr< css::i18n::Calendar2 >& LocaleDataWrapper::getDefaultCalendar() const
{
    return xDefaultCalendar;
}

css::uno::Sequence< css::i18n::CalendarItem2 > const & LocaleDataWrapper::getDefaultCalendarDays() const
{
    return getDefaultCalendar()->Days;
}

css::uno::Sequence< css::i18n::CalendarItem2 > const & LocaleDataWrapper::getDefaultCalendarMonths() const
{
    return getDefaultCalendar()->Months;
}

// --- currencies -----------------------------------------------------

const OUString& LocaleDataWrapper::getCurrSymbol() const
{
    return aCurrSymbol;
}

const OUString& LocaleDataWrapper::getCurrBankSymbol() const
{
    return aCurrBankSymbol;
}

sal_uInt16 LocaleDataWrapper::getCurrPositiveFormat() const
{
    return nCurrPositiveFormat;
}

sal_uInt16 LocaleDataWrapper::getCurrNegativeFormat() const
{
    return nCurrNegativeFormat;
}

sal_uInt16 LocaleDataWrapper::getCurrDigits() const
{
    return nCurrDigits;
}

void LocaleDataWrapper::scanCurrFormatImpl( std::u16string_view rCode,
        sal_Int32 nStart, sal_Int32& nSign, sal_Int32& nPar,
        sal_Int32& nNum, sal_Int32& nBlank, sal_Int32& nSym ) const
{
    nSign = nPar = nNum = nBlank = nSym = -1;
    const sal_Unicode* const pStr = rCode.data();
    const sal_Unicode* const pStop = pStr + rCode.size();
    const sal_Unicode* p = pStr + nStart;
    int nInSection = 0;
    bool bQuote = false;
    while ( p < pStop )
    {
        if ( bQuote )
        {
            if ( *p == '"' && *(p-1) != '\\' )
                bQuote = false;
        }
        else
        {
            switch ( *p )
            {
                case '"' :
                    if ( pStr == p || *(p-1) != '\\' )
                        bQuote = true;
                break;
                case '-' :
                    if (!nInSection && nSign == -1)
                        nSign = p - pStr;
                break;
                case '(' :
                    if (!nInSection && nPar == -1)
                        nPar = p - pStr;
                break;
                case '0' :
                case '#' :
                    if (!nInSection && nNum == -1)
                        nNum = p - pStr;
                break;
                case '[' :
                    nInSection++;
                break;
                case ']' :
                    if ( nInSection )
                    {
                        nInSection--;
                        if (!nInSection && nBlank == -1
                          && nSym != -1 && p < pStop-1 && *(p+1) == ' ' )
                            nBlank = p - pStr + 1;
                    }
                break;
                case '$' :
                    if (nSym == -1 && nInSection && *(p-1) == '[')
                    {
                        nSym = p - pStr + 1;
                        if (nNum != -1 && *(p-2) == ' ')
                            nBlank = p - pStr - 2;
                    }
                break;
                case ';' :
                    if ( !nInSection )
                        p = pStop;
                break;
                default:
                    if (!nInSection && nSym == -1 && o3tl::starts_with(rCode.substr(static_cast<sal_Int32>(p - pStr)), aCurrSymbol))
                    {   // currency symbol not surrounded by [$...]
                        nSym = p - pStr;
                        if (nBlank == -1 && pStr < p && *(p-1) == ' ')
                            nBlank = p - pStr - 1;
                        p += aCurrSymbol.getLength() - 1;
                        if (nBlank == -1 && p < pStop-2 && *(p+2) == ' ')
                            nBlank = p - pStr + 2;
                    }
            }
        }
        p++;
    }
}

void LocaleDataWrapper::loadCurrencyFormats()
{
    css::uno::Reference< css::i18n::XNumberFormatCode > xNFC = i18n::NumberFormatMapper::create( m_xContext );
    uno::Sequence< NumberFormatCode > aFormatSeq = xNFC->getAllFormatCode( KNumberFormatUsage::CURRENCY, maLanguageTag.getLocale() );
    sal_Int32 nCnt = aFormatSeq.getLength();
    if ( !nCnt )
    {   // bad luck
        if (areChecksEnabled())
        {
            outputCheckMessage( appendLocaleInfo( u"LocaleDataWrapper::getCurrFormatsImpl: no currency formats" ) );
        }
        nCurrPositiveFormat = nCurrNegativeFormat = nCurrFormatDefault;
        return;
    }
    // find a negative code (medium preferred) and a default (medium preferred) (not necessarily the same)
    NumberFormatCode const * const pFormatArr = aFormatSeq.getArray();
    sal_Int32 nElem, nDef, nNeg, nMedium;
    nDef = nNeg = nMedium = -1;
    for ( nElem = 0; nElem < nCnt; nElem++ )
    {
        if ( pFormatArr[nElem].Type == KNumberFormatType::MEDIUM )
        {
            if ( pFormatArr[nElem].Default )
            {
                nDef = nElem;
                nMedium = nElem;
                if ( pFormatArr[nElem].Code.indexOf( ';' ) >= 0 )
                    nNeg = nElem;
            }
            else
            {
                if ( (nNeg == -1 || nMedium == -1) && pFormatArr[nElem].Code.indexOf( ';' ) >= 0 )
                    nNeg = nElem;
                if ( nMedium == -1 )
                    nMedium = nElem;
            }
        }
        else
        {
            if ( nDef == -1 && pFormatArr[nElem].Default )
                nDef = nElem;
            if ( nNeg == -1 && pFormatArr[nElem].Code.indexOf( ';' ) >= 0 )
                nNeg = nElem;
        }
    }

    sal_Int32 nSign, nPar, nNum, nBlank, nSym;

    // positive format
    nElem = (nDef >= 0 ? nDef : (nNeg >= 0 ? nNeg : 0));
    scanCurrFormatImpl( pFormatArr[nElem].Code, 0, nSign, nPar, nNum, nBlank, nSym );
    if (areChecksEnabled() && (nNum == -1 || nSym == -1))
    {
        outputCheckMessage( appendLocaleInfo( u"LocaleDataWrapper::getCurrFormatsImpl: CurrPositiveFormat?" ) );
    }
    if (nBlank == -1)
    {
        if ( nSym < nNum )
            nCurrPositiveFormat = 0;    // $1
        else
            nCurrPositiveFormat = 1;    // 1$
    }
    else
    {
        if ( nSym < nNum )
            nCurrPositiveFormat = 2;    // $ 1
        else
            nCurrPositiveFormat = 3;    // 1 $
    }

    // negative format
    if ( nNeg < 0 )
        nCurrNegativeFormat = nCurrFormatDefault;
    else
    {
        const OUString& rCode = pFormatArr[nNeg].Code;
        sal_Int32 nDelim = rCode.indexOf(';');
        scanCurrFormatImpl( rCode, nDelim+1, nSign, nPar, nNum, nBlank, nSym );
        if (areChecksEnabled() && (nNum == -1 || nSym == -1 || (nPar == -1 && nSign == -1)))
        {
            outputCheckMessage( appendLocaleInfo( u"LocaleDataWrapper::getCurrFormatsImpl: CurrNegativeFormat?" ) );
        }
        // NOTE: one of nPar or nSign are allowed to be -1
        if (nBlank == -1)
        {
            if ( nSym < nNum )
            {
                if ( -1 < nPar && nPar < nSym )
                    nCurrNegativeFormat = 0;    // ($1)
                else if ( -1 < nSign && nSign < nSym )
                    nCurrNegativeFormat = 1;    // -$1
                else if ( nNum < nSign )
                    nCurrNegativeFormat = 3;    // $1-
                else
                    nCurrNegativeFormat = 2;    // $-1
            }
            else
            {
                if ( -1 < nPar && nPar < nNum )
                    nCurrNegativeFormat = 4;    // (1$)
                else if ( -1 < nSign && nSign < nNum )
                    nCurrNegativeFormat = 5;    // -1$
                else if ( nSym < nSign )
                    nCurrNegativeFormat = 7;    // 1$-
                else
                    nCurrNegativeFormat = 6;    // 1-$
            }
        }
        else
        {
            if ( nSym < nNum )
            {
                if ( -1 < nPar && nPar < nSym )
                    nCurrNegativeFormat = 14;   // ($ 1)
                else if ( -1 < nSign && nSign < nSym )
                    nCurrNegativeFormat = 9;    // -$ 1
                else if ( nNum < nSign )
                    nCurrNegativeFormat = 12;   // $ 1-
                else
                    nCurrNegativeFormat = 11;   // $ -1
            }
            else
            {
                if ( -1 < nPar && nPar < nNum )
                    nCurrNegativeFormat = 15;   // (1 $)
                else if ( -1 < nSign && nSign < nNum )
                    nCurrNegativeFormat = 8;    // -1 $
                else if ( nSym < nSign )
                    nCurrNegativeFormat = 10;   // 1 $-
                else
                    nCurrNegativeFormat = 13;   // 1- $
            }
        }
    }
}

// --- date -----------------------------------------------------------

DateOrder LocaleDataWrapper::getDateOrder() const
{
    return nDateOrder;
}

LongDateOrder LocaleDataWrapper::getLongDateOrder() const
{
    return nLongDateOrder;
}

LongDateOrder LocaleDataWrapper::scanDateOrderImpl( std::u16string_view rCode ) const
{
    // Only some european versions were translated, the ones with different
    // keyword combinations are:
    // English DMY, German TMJ, Spanish DMA, French JMA, Italian GMA,
    // Dutch DMJ, Finnish PKV

    // default is English keywords for every other language
    size_t nDay = rCode.find('D');
    size_t nMonth = rCode.find('M');
    size_t nYear = rCode.find('Y');
    if (nDay == std::u16string_view::npos || nMonth == std::u16string_view::npos || nYear == std::u16string_view::npos)
    {   // This algorithm assumes that all three parts (DMY) are present
        if (nMonth == std::u16string_view::npos)
        {   // only Finnish has something else than 'M' for month
            nMonth = rCode.find('K');
            if (nMonth != std::u16string_view::npos)
            {
                nDay = rCode.find('P');
                nYear = rCode.find('V');
            }
        }
        else if (nDay == std::u16string_view::npos)
        {   // We have a month 'M' if we reach this branch.
            // Possible languages containing 'M' but no 'D':
            // German, French, Italian
            nDay = rCode.find('T');         // German
            if (nDay != std::u16string_view::npos)
                nYear = rCode.find('J');
            else
            {
                nYear = rCode.find('A');    // French, Italian
                if (nYear != std::u16string_view::npos)
                {
                    nDay = rCode.find('J'); // French
                    if (nDay == std::u16string_view::npos)
                        nDay = rCode.find('G'); // Italian
                }
            }
        }
        else
        {   // We have a month 'M' and a day 'D'.
            // Possible languages containing 'D' and 'M' but not 'Y':
            // Spanish, Dutch
            nYear = rCode.find('A');        // Spanish
            if (nYear == std::u16string_view::npos)
                nYear = rCode.find('J');    // Dutch
        }
        if (nDay == std::u16string_view::npos || nMonth == std::u16string_view::npos || nYear == std::u16string_view::npos)
        {
            if (areChecksEnabled())
            {
                outputCheckMessage( appendLocaleInfo( u"LocaleDataWrapper::scanDateOrder: not all DMY present" ) );
            }
            if (nDay == std::u16string_view::npos)
                nDay = rCode.size();
            if (nMonth == std::u16string_view::npos)
                nMonth = rCode.size();
            if (nYear == std::u16string_view::npos)
                nYear = rCode.size();
        }
    }
    // compare with <= because each position may equal rCode.getLength()
    if ( nDay <= nMonth && nMonth <= nYear )
        return LongDateOrder::DMY;     // also if every position equals rCode.getLength()
    else if ( nMonth <= nDay && nDay <= nYear )
        return LongDateOrder::MDY;
    else if ( nYear <= nMonth && nMonth <= nDay )
        return LongDateOrder::YMD;
    else if ( nYear <= nDay && nDay <= nMonth )
        return LongDateOrder::YDM;
    else
    {
        if (areChecksEnabled())
        {
            outputCheckMessage( appendLocaleInfo( u"LocaleDataWrapper::scanDateOrder: no magic applicable" ) );
        }
        return LongDateOrder::DMY;
    }
}

static DateOrder getDateOrderFromLongDateOrder( LongDateOrder eLong )
{
    switch (eLong)
    {
        case LongDateOrder::YMD:
            return DateOrder::YMD;
        break;
        case LongDateOrder::DMY:
            return DateOrder::DMY;
        break;
        case LongDateOrder::MDY:
            return DateOrder::MDY;
        break;
        case LongDateOrder::YDM:
        default:
            assert(!"unhandled LongDateOrder to DateOrder");
            return DateOrder::DMY;
    }
}

void LocaleDataWrapper::loadDateOrders()
{
    css::uno::Reference< css::i18n::XNumberFormatCode > xNFC = i18n::NumberFormatMapper::create( m_xContext );
    uno::Sequence< NumberFormatCode > aFormatSeq = xNFC->getAllFormatCode( KNumberFormatUsage::DATE, maLanguageTag.getLocale() );
    sal_Int32 nCnt = aFormatSeq.getLength();
    if ( !nCnt )
    {   // bad luck
        if (areChecksEnabled())
        {
            outputCheckMessage( appendLocaleInfo( u"LocaleDataWrapper::getDateOrdersImpl: no date formats" ) );
        }
        nDateOrder = DateOrder::DMY;
        nLongDateOrder = LongDateOrder::DMY;
        return;
    }
    // find the edit (21), a default (medium preferred),
    // a medium (default preferred), and a long (default preferred)
    NumberFormatCode const * const pFormatArr = aFormatSeq.getArray();
    sal_Int32 nEdit, nDef, nMedium, nLong;
    nEdit = nDef = nMedium = nLong = -1;
    for ( sal_Int32 nElem = 0; nElem < nCnt; nElem++ )
    {
        if ( nEdit == -1 && pFormatArr[nElem].Index == NumberFormatIndex::DATE_SYS_DDMMYYYY )
            nEdit = nElem;
        if ( nDef == -1 && pFormatArr[nElem].Default )
            nDef = nElem;
        switch ( pFormatArr[nElem].Type )
        {
            case KNumberFormatType::MEDIUM :
            {
                if ( pFormatArr[nElem].Default )
                {
                    nDef = nElem;
                    nMedium = nElem;
                }
                else if ( nMedium == -1 )
                    nMedium = nElem;
            }
            break;
            case KNumberFormatType::LONG :
            {
                if ( pFormatArr[nElem].Default )
                    nLong = nElem;
                else if ( nLong == -1 )
                    nLong = nElem;
            }
            break;
        }
    }
    if ( nEdit == -1 )
    {
        if (areChecksEnabled())
        {
            outputCheckMessage( appendLocaleInfo( u"LocaleDataWrapper::getDateOrdersImpl: no edit" ) );
        }
        if ( nDef == -1 )
        {
            if (areChecksEnabled())
            {
                outputCheckMessage( appendLocaleInfo( u"LocaleDataWrapper::getDateOrdersImpl: no default" ) );
            }
            if ( nMedium != -1 )
                nDef = nMedium;
            else if ( nLong != -1 )
                nDef = nLong;
            else
                nDef = 0;
        }
        nEdit = nDef;
    }
    LongDateOrder nDO = scanDateOrderImpl( pFormatArr[nEdit].Code );
    if ( pFormatArr[nEdit].Type == KNumberFormatType::LONG )
    {   // normally this is not the case
        nLongDateOrder = nDO;
        nDateOrder = getDateOrderFromLongDateOrder(nDO);
    }
    else
    {
        // YDM should not occur in a short/medium date (i.e. no locale has
        // that) and is nowhere handled.
        nDateOrder = getDateOrderFromLongDateOrder(nDO);
        if ( nLong == -1 )
            nLongDateOrder = nDO;
        else
            nLongDateOrder = scanDateOrderImpl( pFormatArr[nLong].Code );
    }
}

// --- digit grouping -------------------------------------------------

void LocaleDataWrapper::loadDigitGrouping()
{
    /* TODO: This is a very simplified grouping setup that only serves its
     * current purpose for Indian locales. A free-form flexible one would
     * obtain grouping from locale data where it could be specified using, for
     * example, codes like #,### and #,##,### that would generate the integer
     * sequence. Needed additional API and a locale data element.
     */

    if (aGrouping.hasElements() && aGrouping[0])
        return;

    i18n::LanguageCountryInfo aLCInfo( getLanguageCountryInfo());
    if (aLCInfo.Country.equalsIgnoreAsciiCase("IN") || // India
        aLCInfo.Country.equalsIgnoreAsciiCase("BT") )  // Bhutan
    {
        aGrouping = { 3, 2, 0 };
    }
    else
    {
        aGrouping = { 3, 0, 0 };
    }
}

const css::uno::Sequence< sal_Int32 >& LocaleDataWrapper::getDigitGrouping() const
{
    return aGrouping;
}

// --- simple number formatting helpers -------------------------------

// The ImplAdd... methods are taken from class International and modified to
// suit the needs.

static void ImplAddUNum( OUStringBuffer& rBuf, sal_uInt64 nNumber )
{
    // fill temp buffer with digits
    sal_Unicode aTempBuf[64];
    sal_Unicode* pTempBuf = aTempBuf;
    do
    {
        *pTempBuf = static_cast<sal_Unicode>(nNumber % 10) + '0';
        pTempBuf++;
        nNumber /= 10;
    }
    while ( nNumber );

    // copy temp buffer to buffer passed
    do
    {
        pTempBuf--;
        rBuf.append(*pTempBuf);
    }
    while ( pTempBuf != aTempBuf );
}

static void ImplAddUNum( OUStringBuffer& rBuf, sal_uInt64 nNumber, int nMinLen )
{
    // fill temp buffer with digits
    sal_Unicode aTempBuf[64];
    sal_Unicode* pTempBuf = aTempBuf;
    do
    {
        *pTempBuf = static_cast<sal_Unicode>(nNumber % 10) + '0';
        pTempBuf++;
        nNumber /= 10;
        nMinLen--;
    }
    while ( nNumber );

    // fill with zeros up to the minimal length
    while ( nMinLen > 0 )
    {
        rBuf.append('0');
        nMinLen--;
    }

    // copy temp buffer to real buffer
    do
    {
        pTempBuf--;
        rBuf.append(*pTempBuf);
    }
    while ( pTempBuf != aTempBuf );
}

static void ImplAddNum( OUStringBuffer& rBuf, sal_Int64 nNumber, int nMinLen )
{
    if (nNumber < 0)
    {
        rBuf.append('-');
        nNumber = -nNumber;
    }
    return ImplAddUNum( rBuf, nNumber, nMinLen);
}

static void ImplAdd2UNum( OUStringBuffer& rBuf, sal_uInt16 nNumber )
{
    DBG_ASSERT( nNumber < 100, "ImplAdd2UNum() - Number >= 100" );

    if ( nNumber < 10 )
    {
        rBuf.append('0');
        rBuf.append(static_cast<char>(nNumber + '0'));
    }
    else
    {
        sal_uInt16 nTemp = nNumber % 10;
        nNumber /= 10;
        rBuf.append(static_cast<char>(nNumber + '0'));
        rBuf.append(static_cast<char>(nTemp + '0'));
    }
}

static void ImplAdd9UNum( OUStringBuffer& rBuf, sal_uInt32 nNumber )
{
    DBG_ASSERT( nNumber < 1000000000, "ImplAdd9UNum() - Number >= 1000000000" );

    std::ostringstream ostr;
    ostr.fill('0');
    ostr.width(9);
    ostr << nNumber;
    std::string aStr = ostr.str();
    rBuf.appendAscii(aStr.c_str(), aStr.size());
}

void LocaleDataWrapper::ImplAddFormatNum( OUStringBuffer& rBuf,
        sal_Int64 nNumber, sal_uInt16 nDecimals, bool bUseThousandSep,
        bool bTrailingZeros ) const
{
    OUStringBuffer aNumBuf(64);
    sal_uInt16  nNumLen;

    // negative number
    sal_uInt64 abs;
    if ( nNumber < 0 )
    {
        // Avoid overflow, map -2^63 -> 2^63 explicitly:
        abs = nNumber == std::numeric_limits<sal_Int64>::min()
            ? static_cast<sal_uInt64>(std::numeric_limits<sal_Int64>::min()) : nNumber * -1;
        rBuf.append('-');
    }
    else
    {
        abs = nNumber;
    }

    // convert number
    ImplAddUNum( aNumBuf, abs );
    nNumLen = static_cast<sal_uInt16>(aNumBuf.getLength());

    if ( nNumLen <= nDecimals )
    {
        // strip .0 in decimals?
        if ( !nNumber && !bTrailingZeros )
        {
            rBuf.append('0');
        }
        else
        {
            // LeadingZero, insert 0
            if ( isNumLeadingZero() )
            {
                rBuf.append('0');
            }

            // append decimal separator
            rBuf.append( aLocaleDataItem.decimalSeparator );

            // fill with zeros
            sal_uInt16 i = 0;
            while ( i < (nDecimals-nNumLen) )
            {
                rBuf.append('0');
                i++;
            }

            // append decimals
            rBuf.append(aNumBuf);
        }
    }
    else
    {
        const OUString& rThoSep = aLocaleDataItem.thousandSeparator;

        // copy number to buffer (excluding decimals)
        sal_uInt16 nNumLen2 = nNumLen-nDecimals;
        uno::Sequence< sal_Bool > aGroupPos;
        if (bUseThousandSep)
            aGroupPos = utl::DigitGroupingIterator::createForwardSequence(
                    nNumLen2, getDigitGrouping());
        sal_uInt16 i = 0;
        for (; i < nNumLen2; ++i )
        {
            rBuf.append(aNumBuf[i]);

            // add thousand separator?
            if ( bUseThousandSep && aGroupPos[i] )
                rBuf.append( rThoSep );
        }

        // append decimals
        if ( nDecimals )
        {
            rBuf.append( aLocaleDataItem.decimalSeparator );

            bool bNullEnd = true;
            while ( i < nNumLen )
            {
                if ( aNumBuf[i] != '0' )
                    bNullEnd = false;

                rBuf.append(aNumBuf[i]);
                i++;
            }

            // strip .0 in decimals?
            if ( bNullEnd && !bTrailingZeros )
                rBuf.setLength( rBuf.getLength() - (nDecimals + 1) );
        }
    }
}

// --- simple date and time formatting --------------------------------

OUString LocaleDataWrapper::getDate( const Date& rDate ) const
{
//!TODO: leading zeros et al
    OUStringBuffer aBuf(128);
    sal_uInt16  nDay    = rDate.GetDay();
    sal_uInt16  nMonth  = rDate.GetMonth();
    sal_Int16   nYear   = rDate.GetYear();
    sal_uInt16  nYearLen;

    if ( (true) /* IsDateCentury() */ )
        nYearLen = 4;
    else
    {
        nYearLen = 2;
        nYear %= 100;
    }

    switch ( getDateOrder() )
    {
        case DateOrder::DMY :
            ImplAdd2UNum( aBuf, nDay );
            aBuf.append( aLocaleDataItem.dateSeparator );
            ImplAdd2UNum( aBuf, nMonth );
            aBuf.append( aLocaleDataItem.dateSeparator );
            ImplAddNum( aBuf, nYear, nYearLen );
        break;
        case DateOrder::MDY :
            ImplAdd2UNum( aBuf, nMonth );
            aBuf.append( aLocaleDataItem.dateSeparator );
            ImplAdd2UNum( aBuf, nDay );
            aBuf.append( aLocaleDataItem.dateSeparator );
            ImplAddNum( aBuf, nYear, nYearLen );
        break;
        default:
            ImplAddNum( aBuf, nYear, nYearLen );
            aBuf.append( aLocaleDataItem.dateSeparator );
            ImplAdd2UNum( aBuf, nMonth );
            aBuf.append( aLocaleDataItem.dateSeparator );
            ImplAdd2UNum( aBuf, nDay );
    }

    return aBuf.makeStringAndClear();
}

OUString LocaleDataWrapper::getTime( const tools::Time& rTime, bool bSec, bool b100Sec ) const
{
//!TODO: leading zeros et al
    OUStringBuffer aBuf(128);
    sal_uInt16  nHour = rTime.GetHour();

    nHour %= 24;

    ImplAdd2UNum( aBuf, nHour );
    aBuf.append( aLocaleDataItem.timeSeparator );
    ImplAdd2UNum( aBuf, rTime.GetMin() );
    if ( bSec )
    {
        aBuf.append( aLocaleDataItem.timeSeparator );
        ImplAdd2UNum( aBuf, rTime.GetSec() );

        if ( b100Sec )
        {
            aBuf.append( aLocaleDataItem.time100SecSeparator );
            ImplAdd9UNum( aBuf, rTime.GetNanoSec() );
        }
    }

    return aBuf.makeStringAndClear();
}

OUString LocaleDataWrapper::getDuration( const tools::Duration& rDuration, bool bSec, bool b100Sec ) const
{
    OUStringBuffer aBuf(128);

    if ( rDuration.IsNegative() )
        aBuf.append(' ');

    sal_Int64 nHours = static_cast<sal_Int64>(rDuration.GetDays()) * 24 +
        (rDuration.IsNegative() ?
         -static_cast<sal_Int64>(rDuration.GetTime().GetHour()) :
         rDuration.GetTime().GetHour());
    if ( (true) /* IsTimeLeadingZero() */ )
        ImplAddNum( aBuf, nHours, 2 );
    else
        ImplAddNum( aBuf, nHours, 1 );
    aBuf.append( aLocaleDataItem.timeSeparator );
    ImplAdd2UNum( aBuf, rDuration.GetTime().GetMin() );
    if ( bSec )
    {
        aBuf.append( aLocaleDataItem.timeSeparator );
        ImplAdd2UNum( aBuf, rDuration.GetTime().GetSec() );

        if ( b100Sec )
        {
            aBuf.append( aLocaleDataItem.time100SecSeparator );
            ImplAdd9UNum( aBuf, rDuration.GetTime().GetNanoSec() );
        }
    }

    return aBuf.makeStringAndClear();
}

// --- simple number formatting ---------------------------------------

static size_t ImplGetNumberStringLengthGuess( const css::i18n::LocaleDataItem2& rLocaleDataItem, sal_uInt16 nDecimals )
{
    // approximately 3.2 bits per digit
    const size_t nDig = ((sizeof(sal_Int64) * 8) / 3) + 1;
    // digits, separators (pessimized for insane "every digit may be grouped"), leading zero, sign
    size_t nGuess = ((nDecimals < nDig) ?
        (((nDig - nDecimals) * rLocaleDataItem.thousandSeparator.getLength()) + nDig) :
        nDecimals) + rLocaleDataItem.decimalSeparator.getLength() + 3;
    return nGuess;
}

OUString LocaleDataWrapper::getNum( sal_Int64 nNumber, sal_uInt16 nDecimals,
        bool bUseThousandSep, bool bTrailingZeros ) const
{
    // check if digits and separators will fit into fixed buffer or allocate
    size_t nGuess = ImplGetNumberStringLengthGuess( aLocaleDataItem, nDecimals );
    OUStringBuffer aBuf(int(nGuess + 16));

    ImplAddFormatNum( aBuf, nNumber, nDecimals,
        bUseThousandSep, bTrailingZeros );

    return aBuf.makeStringAndClear();
}

OUString LocaleDataWrapper::getCurr( sal_Int64 nNumber, sal_uInt16 nDecimals,
        std::u16string_view rCurrencySymbol, bool bUseThousandSep ) const
{
    sal_Unicode cZeroChar = getCurrZeroChar();

    // check if digits and separators will fit into fixed buffer or allocate
    size_t nGuess = ImplGetNumberStringLengthGuess( aLocaleDataItem, nDecimals );
    OUStringBuffer aNumBuf(sal_Int32(nGuess + 16));

    bool bNeg;
    if ( nNumber < 0 )
    {
        bNeg = true;
        nNumber *= -1;
    }
    else
        bNeg = false;

    // convert number
    ImplAddFormatNum( aNumBuf, nNumber, nDecimals,
        bUseThousandSep, true );
    const sal_Int32 nNumLen = aNumBuf.getLength();

    // replace zeros with zero character
    if ( (cZeroChar != '0') && nDecimals /* && IsNumTrailingZeros() */ )
    {
        sal_uInt16  i;
        bool    bZero = true;

        sal_uInt16 nNumBufIndex = nNumLen-nDecimals;
        i = 0;
        do
        {
            if ( aNumBuf[nNumBufIndex] != '0' )
            {
                bZero = false;
                break;
            }

            nNumBufIndex++;
            i++;
        }
        while ( i < nDecimals );

        if ( bZero )
        {
            nNumBufIndex = nNumLen-nDecimals;
            i = 0;
            do
            {
                aNumBuf[nNumBufIndex] = cZeroChar;
                nNumBufIndex++;
                i++;
            }
            while ( i < nDecimals );
        }
    }

    OUString aCur;
    if ( !bNeg )
    {
        switch( getCurrPositiveFormat() )
        {
            case 0:
                aCur = rCurrencySymbol + aNumBuf;
                break;
            case 1:
                aCur = aNumBuf + rCurrencySymbol;
                break;
            case 2:
                aCur = OUString::Concat(rCurrencySymbol) + " " + aNumBuf;
                break;
            case 3:
                aCur = aNumBuf + " " + rCurrencySymbol;
                break;
        }
    }
    else
    {
        switch( getCurrNegativeFormat() )
        {
            case 0:
                 aCur = OUString::Concat("(") + rCurrencySymbol + aNumBuf + ")";
                break;
            case 1:
                 aCur = OUString::Concat("-") + rCurrencySymbol + aNumBuf;
                break;
            case 2:
                 aCur = OUString::Concat(rCurrencySymbol) + "-" + aNumBuf;
                break;
            case 3:
                 aCur = rCurrencySymbol + aNumBuf + "-";
                break;
            case 4:
                 aCur = "(" + aNumBuf + rCurrencySymbol + ")";
                break;
            case 5:
                 aCur = "-" + aNumBuf + rCurrencySymbol;
                break;
            case 6:
                 aCur = aNumBuf + "-" + rCurrencySymbol;
                break;
            case 7:
                 aCur = aNumBuf + rCurrencySymbol + "-";
                break;
            case 8:
                 aCur = "-" + aNumBuf + " " + rCurrencySymbol;
                break;
            case 9:
                 aCur = OUString::Concat("-") + rCurrencySymbol + " " + aNumBuf;
                break;
            case 10:
                 aCur = aNumBuf + " " + rCurrencySymbol + "-";
                break;
            case 11:
                 aCur = OUString::Concat(rCurrencySymbol) + " -" + aNumBuf;
                break;
            case 12:
                 aCur = OUString::Concat(rCurrencySymbol) + " " + aNumBuf + "-";
                break;
            case 13:
                 aCur = aNumBuf + "- " + rCurrencySymbol;
                break;
            case 14:
                 aCur = OUString::Concat("(") + rCurrencySymbol + " " + aNumBuf + ")";
                break;
            case 15:
                 aCur = "(" + aNumBuf + " " + rCurrencySymbol + ")";
                break;
        }
    }

    return aCur;
}

// --- number parsing -------------------------------------------------

double LocaleDataWrapper::stringToDouble( std::u16string_view aString, bool bUseGroupSep,
        rtl_math_ConversionStatus* pStatus, sal_Int32* pParseEnd ) const
{
    const sal_Unicode* pParseEndChar;
    double fValue = stringToDouble(aString.data(), aString.data() + aString.size(), bUseGroupSep, pStatus, &pParseEndChar);
    if (pParseEnd)
        *pParseEnd = pParseEndChar - aString.data();
    return fValue;
}

double LocaleDataWrapper::stringToDouble( const sal_Unicode* pBegin, const sal_Unicode* pEnd, bool bUseGroupSep,
        rtl_math_ConversionStatus* pStatus, const sal_Unicode** ppParseEnd ) const
{
    const sal_Unicode cGroupSep = (bUseGroupSep ? aLocaleDataItem.thousandSeparator[0] : 0);
    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    const sal_Unicode* pParseEnd = nullptr;
    double fValue = rtl_math_uStringToDouble( pBegin, pEnd, aLocaleDataItem.decimalSeparator[0], cGroupSep, &eStatus, &pParseEnd);
    bool bTryAlt = (pParseEnd < pEnd && !aLocaleDataItem.decimalSeparatorAlternative.isEmpty() &&
            *pParseEnd == aLocaleDataItem.decimalSeparatorAlternative.toChar());
    // Try re-parsing with alternative if that was the reason to stop.
    if (bTryAlt)
        fValue = rtl_math_uStringToDouble( pBegin, pEnd, aLocaleDataItem.decimalSeparatorAlternative.toChar(), cGroupSep, &eStatus, &pParseEnd);
    if (pStatus)
        *pStatus = eStatus;
    if (ppParseEnd)
        *ppParseEnd = pParseEnd;
    return fValue;
}

// --- mixed ----------------------------------------------------------

LanguageTag LocaleDataWrapper::getLoadedLanguageTag() const
{
    LanguageCountryInfo aLCInfo = getLanguageCountryInfo();
    return LanguageTag( lang::Locale( aLCInfo.Language, aLCInfo.Country, aLCInfo.Variant ));
}

OUString LocaleDataWrapper::appendLocaleInfo(std::u16string_view rDebugMsg) const
{
    LanguageTag aLoaded = getLoadedLanguageTag();
    return OUString::Concat(rDebugMsg) + "\n" + maLanguageTag.getBcp47() + " requested\n"
        + aLoaded.getBcp47() + " loaded";
}

// static
void LocaleDataWrapper::outputCheckMessage( std::u16string_view rMsg )
{
    outputCheckMessage(OUStringToOString(rMsg, RTL_TEXTENCODING_UTF8).getStr());
}

// static
void LocaleDataWrapper::outputCheckMessage( const char* pStr )
{
    fprintf( stderr, "\n%s\n", pStr);
    fflush( stderr);
    SAL_WARN("unotools.i18n", pStr);
}

// static
void LocaleDataWrapper::evaluateLocaleDataChecking()
{
    // Using the rtl_Instance template here wouldn't solve all threaded write
    // accesses, since we want to assign the result to the static member
    // variable and would need to dereference the pointer returned and assign
    // the value unguarded. This is the same pattern manually coded.
    sal_uInt8 nCheck = nLocaleDataChecking;
    if (!nCheck)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex());
        nCheck = nLocaleDataChecking;
        if (!nCheck)
        {
#ifdef DBG_UTIL
            nCheck = 1;
#else
            const char* pEnv = getenv( "OOO_ENABLE_LOCALE_DATA_CHECKS");
            if (pEnv && (pEnv[0] == 'Y' || pEnv[0] == 'y' || pEnv[0] == '1'))
                nCheck = 1;
            else
                nCheck = 2;
#endif
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            nLocaleDataChecking = nCheck;
        }
    }
    else {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
}

// --- XLocaleData3 ----------------------------------------------------------

css::uno::Sequence< css::i18n::Calendar2 > LocaleDataWrapper::getAllCalendars() const
{
    try
    {
        return xLD->getAllCalendars2( getMyLocale() );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getAllCalendars" );
    }
    return {};
}

// --- XLocaleData4 ----------------------------------------------------------

const css::uno::Sequence< OUString > & LocaleDataWrapper::getDateAcceptancePatterns() const
{
    return aDateAcceptancePatterns;
}

// --- Override layer --------------------------------------------------------

void LocaleDataWrapper::loadDateAcceptancePatterns(
        const std::vector<OUString> & rPatterns )
{
    if (!aDateAcceptancePatterns.hasElements() || rPatterns.empty())
    {
        try
        {
            aDateAcceptancePatterns = xLD->getDateAcceptancePatterns( maLanguageTag.getLocale() );
        }
        catch (const Exception&)
        {
            TOOLS_WARN_EXCEPTION( "unotools.i18n", "setDateAcceptancePatterns" );
        }
        if (rPatterns.empty())
            return;     // just a reset
        if (!aDateAcceptancePatterns.hasElements())
        {
            aDateAcceptancePatterns = comphelper::containerToSequence(rPatterns);
            return;
        }
    }

    // Earlier versions checked for presence of the full date pattern with
    // aDateAcceptancePatterns[0] == rPatterns[0] and prepended that if not.
    // This lead to confusion if the patterns were intentionally specified
    // without, giving entirely a different DMY order, see tdf#150288.
    // Not checking this and accepting the given patterns as is may result in
    // the user shooting themself in the foot, but we can't have both.
    aDateAcceptancePatterns = comphelper::containerToSequence(rPatterns);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
