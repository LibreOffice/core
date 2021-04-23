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

#include <stdio.h>
#include <string>

#include <sal/log.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/digitgroupingiterator.hxx>
#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <com/sun/star/i18n/KNumberFormatUsage.hpp>
#include <com/sun/star/i18n/KNumberFormatType.hpp>
#include <com/sun/star/i18n/LocaleData2.hpp>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#include <com/sun/star/i18n/NumberFormatIndex.hpp>
#include <com/sun/star/i18n/NumberFormatMapper.hpp>

#include <comphelper/processfactory.hxx>
#include <rtl/instance.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

const sal_uInt16 nCurrFormatInvalid = 0xffff;
const sal_uInt16 nCurrFormatDefault = 0;

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

namespace
{
    struct InstalledLocales
        : public rtl::Static<
            uno::Sequence< lang::Locale >, InstalledLocales >
    {};

    struct InstalledLanguageTypes
        : public rtl::Static<
            std::vector< LanguageType >, InstalledLanguageTypes >
    {};
}

bool LocaleDataWrapper::Locale_Compare::operator()(const css::lang::Locale& rLocale1, const css::lang::Locale& rLocale2) const
{
    if (rLocale1.Language < rLocale2.Language)
        return true;
    else if (rLocale1.Language > rLocale2.Language)
        return false;

    if (rLocale1.Country < rLocale2.Country)
        return true;
    else if (rLocale1.Country > rLocale2.Country)
        return false;

    return rLocale1.Variant < rLocale2.Variant;
}

sal_uInt8 LocaleDataWrapper::nLocaleDataChecking = 0;

LocaleDataWrapper::LocaleDataWrapper(
            const Reference< uno::XComponentContext > & rxContext,
            const LanguageTag& rLanguageTag
            )
        :
        m_xContext( rxContext ),
        xLD( LocaleData2::create(rxContext) ),
        maLanguageTag( rLanguageTag ),
        bLocaleDataItemValid( false ),
        bReservedWordValid( false ),
        bSecondaryCalendarValid( false )
{
    invalidateData();
}

LocaleDataWrapper::LocaleDataWrapper(
            const LanguageTag& rLanguageTag
            )
        :
        m_xContext( comphelper::getProcessComponentContext() ),
        xLD( LocaleData2::create(m_xContext) ),
        maLanguageTag( rLanguageTag ),
        bLocaleDataItemValid( false ),
        bReservedWordValid( false ),
        bSecondaryCalendarValid( false )
{
    invalidateData();
}

LocaleDataWrapper::~LocaleDataWrapper()
{
}

void LocaleDataWrapper::setLanguageTag( const LanguageTag& rLanguageTag )
{
    ::utl::ReadWriteGuard aGuard( aMutex, ReadWriteGuardMode::CriticalChange );
    maLanguageTag = rLanguageTag;
    invalidateData();
}

const LanguageTag& LocaleDataWrapper::getLanguageTag() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    return maLanguageTag;
}

const css::lang::Locale& LocaleDataWrapper::getMyLocale() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    return maLanguageTag.getLocale();
}

void LocaleDataWrapper::invalidateData()
{
    aCurrSymbol.clear();
    aCurrBankSymbol.clear();
    nDateOrder = nLongDateOrder = DateOrder::Invalid;
    nCurrPositiveFormat = nCurrNegativeFormat = nCurrDigits = nCurrFormatInvalid;
    if ( bLocaleDataItemValid )
    {
        for (OUString & j : aLocaleItem)
            j.clear();
        bLocaleDataItemValid = false;
    }
    if ( bReservedWordValid )
    {
        for (OUString & j : aReservedWord)
            j.clear();
        bReservedWordValid = false;
    }
    xDefaultCalendar.reset();
    xSecondaryCalendar.reset();
    bSecondaryCalendarValid = false;
    if (aGrouping.hasElements())
        aGrouping[0] = 0;
    if (aDateAcceptancePatterns.hasElements())
        aDateAcceptancePatterns = Sequence<OUString>();
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
    {
        ::utl::ReadWriteGuard aGuard( aMutex );
        const css::lang::Locale& rLocal = getMyLocale();
        auto itr = maDataItemCache.find(rLocal);
        if (itr != maDataItemCache.end())
            return itr->second;
    }

    try
    {
        ::utl::ReadWriteGuard aGuard( aMutex );

        const css::lang::Locale& rLocal = getMyLocale();
        css::i18n::LocaleDataItem2 aItem = xLD->getLocaleItem2( rLocal );
        auto aRet = maDataItemCache.insert(std::make_pair(rLocal, aItem));
        assert(aRet.second);
        return aRet.first->second;
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getLocaleItem" );
    }
    static css::i18n::LocaleDataItem2 aEmptyItem;
    return aEmptyItem;
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
    return css::uno::Sequence< css::i18n::Currency2 >(0);
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
    return css::uno::Sequence< css::i18n::FormatElement >(0);
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

css::uno::Sequence< OUString > LocaleDataWrapper::getReservedWord() const
{
    try
    {
        return xLD->getReservedWord( getMyLocale() );
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getReservedWord" );
    }
    return css::uno::Sequence< OUString >(0);
}

css::uno::Sequence< css::lang::Locale > LocaleDataWrapper::getAllInstalledLocaleNames() const
{
    uno::Sequence< lang::Locale > &rInstalledLocales = InstalledLocales::get();

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
css::uno::Sequence< css::lang::Locale > LocaleDataWrapper::getInstalledLocaleNames()
{
    const uno::Sequence< lang::Locale > &rInstalledLocales =
        InstalledLocales::get();

    if ( !rInstalledLocales.hasElements() )
    {
        LocaleDataWrapper aLDW( ::comphelper::getProcessComponentContext(), LanguageTag( LANGUAGE_SYSTEM) );
        aLDW.getAllInstalledLocaleNames();
    }
    return rInstalledLocales;
}

// static
std::vector< LanguageType > LocaleDataWrapper::getInstalledLanguageTypes()
{
    std::vector< LanguageType > &rInstalledLanguageTypes =
        InstalledLanguageTypes::get();

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
                    OUStringBuffer aMsg("ConvertIsoNamesToLanguage/ConvertLanguageToIsoNames: ambiguous locale (MS-LCID?)\n");
                    aMsg.append(aDebugLocale);
                    aMsg.append("  ->  0x");
                    aMsg.append(static_cast<sal_Int32>(static_cast<sal_uInt16>(eLang)), 16);
                    aMsg.append("  ->  ");
                    aMsg.append(aBackLanguageTag.getBcp47());
                    outputCheckMessage( aMsg.makeStringAndClear() );
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
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nItem >= LocaleItem::COUNT2 )
    {
        SAL_WARN( "unotools.i18n", "getOneLocaleItem: bounds" );
        return aLocaleItem[0];
    }
    if (aLocaleItem[nItem].isEmpty())
    {   // no cached content
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getOneLocaleItemImpl( nItem );
    }
    return aLocaleItem[nItem];
}

void LocaleDataWrapper::getOneLocaleItemImpl( sal_Int16 nItem )
{
    if ( !bLocaleDataItemValid )
    {
        aLocaleDataItem = getLocaleItem();
        bLocaleDataItemValid = true;
    }
    switch ( nItem )
    {
        case LocaleItem::DATE_SEPARATOR :
            aLocaleItem[nItem] = aLocaleDataItem.dateSeparator;
        break;
        case LocaleItem::THOUSAND_SEPARATOR :
            aLocaleItem[nItem] = aLocaleDataItem.thousandSeparator;
        break;
        case LocaleItem::DECIMAL_SEPARATOR :
            aLocaleItem[nItem] = aLocaleDataItem.decimalSeparator;
        break;
        case LocaleItem::TIME_SEPARATOR :
            aLocaleItem[nItem] = aLocaleDataItem.timeSeparator;
        break;
        case LocaleItem::TIME_100SEC_SEPARATOR :
            aLocaleItem[nItem] = aLocaleDataItem.time100SecSeparator;
        break;
        case LocaleItem::LIST_SEPARATOR :
            aLocaleItem[nItem] = aLocaleDataItem.listSeparator;
        break;
        case LocaleItem::SINGLE_QUOTATION_START :
            aLocaleItem[nItem] = aLocaleDataItem.quotationStart;
        break;
        case LocaleItem::SINGLE_QUOTATION_END :
            aLocaleItem[nItem] = aLocaleDataItem.quotationEnd;
        break;
        case LocaleItem::DOUBLE_QUOTATION_START :
            aLocaleItem[nItem] = aLocaleDataItem.doubleQuotationStart;
        break;
        case LocaleItem::DOUBLE_QUOTATION_END :
            aLocaleItem[nItem] = aLocaleDataItem.doubleQuotationEnd;
        break;
        case LocaleItem::MEASUREMENT_SYSTEM :
            aLocaleItem[nItem] = aLocaleDataItem.measurementSystem;
        break;
        case LocaleItem::TIME_AM :
            aLocaleItem[nItem] = aLocaleDataItem.timeAM;
        break;
        case LocaleItem::TIME_PM :
            aLocaleItem[nItem] = aLocaleDataItem.timePM;
        break;
        case LocaleItem::LONG_DATE_DAY_OF_WEEK_SEPARATOR :
            aLocaleItem[nItem] = aLocaleDataItem.LongDateDayOfWeekSeparator;
        break;
        case LocaleItem::LONG_DATE_DAY_SEPARATOR :
            aLocaleItem[nItem] = aLocaleDataItem.LongDateDaySeparator;
        break;
        case LocaleItem::LONG_DATE_MONTH_SEPARATOR :
            aLocaleItem[nItem] = aLocaleDataItem.LongDateMonthSeparator;
        break;
        case LocaleItem::LONG_DATE_YEAR_SEPARATOR :
            aLocaleItem[nItem] = aLocaleDataItem.LongDateYearSeparator;
        break;
        case LocaleItem::DECIMAL_SEPARATOR_ALTERNATIVE :
            aLocaleItem[nItem] = aLocaleDataItem.decimalSeparatorAlternative;
        break;
        default:
            SAL_WARN( "unotools.i18n", "getOneLocaleItemImpl: which one?" );
    }
}

void LocaleDataWrapper::getOneReservedWordImpl( sal_Int16 nWord )
{
    if ( !bReservedWordValid )
    {
        aReservedWordSeq = getReservedWord();
        bReservedWordValid = true;
    }
    DBG_ASSERT( nWord < aReservedWordSeq.getLength(), "getOneReservedWordImpl: which one?" );
    if ( nWord < aReservedWordSeq.getLength() )
        aReservedWord[nWord] = aReservedWordSeq[nWord];
}

const OUString& LocaleDataWrapper::getOneReservedWord( sal_Int16 nWord ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nWord < 0 || nWord >= reservedWords::COUNT )
    {
        SAL_WARN( "unotools.i18n", "getOneReservedWord: bounds" );
        nWord = reservedWords::FALSE_WORD;
    }
    if (aReservedWord[nWord].isEmpty())
    {   // no cached content
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getOneReservedWordImpl( nWord );
    }
    return aReservedWord[nWord];
}

MeasurementSystem LocaleDataWrapper::mapMeasurementStringToEnum( const OUString& rMS ) const
{
//! TODO: could be cached too
    if ( rMS.equalsIgnoreAsciiCase( "metric" ) )
        return MeasurementSystem::Metric;
//! TODO: other measurement systems? => extend enum MeasurementSystem
    return MeasurementSystem::US;
}

void LocaleDataWrapper::getSecondaryCalendarImpl()
{
    if (!xSecondaryCalendar && !bSecondaryCalendarValid)
    {
        Sequence< Calendar2 > xCals = getAllCalendars();
        if (xCals.getLength() > 1)
        {
            auto pCal = std::find_if(xCals.begin(), xCals.end(),
                [](const Calendar2& rCal) { return !rCal.Default; });
            if (pCal != xCals.end())
                xSecondaryCalendar = std::make_shared<Calendar2>( *pCal);
        }
        bSecondaryCalendarValid = true;
    }
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

    ::utl::ReadWriteGuard aGuard( aMutex );

    if (!bSecondaryCalendarValid)
    {   // no cached content
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getSecondaryCalendarImpl();
    }
    if (!xSecondaryCalendar)
        return false;
    if (!xSecondaryCalendar->Name.equalsIgnoreAsciiCase( rName))
        return false;

    return true;
}

void LocaleDataWrapper::getDefaultCalendarImpl()
{
    if (xDefaultCalendar)
        return;

    Sequence< Calendar2 > xCals = getAllCalendars();
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

const std::shared_ptr< css::i18n::Calendar2 >& LocaleDataWrapper::getDefaultCalendar() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if (!xDefaultCalendar)
    {   // no cached content
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getDefaultCalendarImpl();
    }
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
    ::utl::ReadWriteGuard aGuard( aMutex );
    if (aCurrSymbol.isEmpty())
    {
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getCurrSymbolsImpl();
    }
    return aCurrSymbol;
}

const OUString& LocaleDataWrapper::getCurrBankSymbol() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if (aCurrBankSymbol.isEmpty())
    {
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getCurrSymbolsImpl();
    }
    return aCurrBankSymbol;
}

sal_uInt16 LocaleDataWrapper::getCurrPositiveFormat() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nCurrPositiveFormat == nCurrFormatInvalid )
    {
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getCurrFormatsImpl();
    }
    return nCurrPositiveFormat;
}

sal_uInt16 LocaleDataWrapper::getCurrNegativeFormat() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nCurrNegativeFormat == nCurrFormatInvalid )
    {
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getCurrFormatsImpl();
    }
    return nCurrNegativeFormat;
}

sal_uInt16 LocaleDataWrapper::getCurrDigits() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nCurrDigits == nCurrFormatInvalid )
    {
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getCurrSymbolsImpl();
    }
    return nCurrDigits;
}

void LocaleDataWrapper::getCurrSymbolsImpl()
{
    Sequence< Currency2 > aCurrSeq = getAllCurrencies();
    if ( !aCurrSeq.hasElements() )
    {
        if (areChecksEnabled())
            outputCheckMessage("LocaleDataWrapper::getCurrSymbolsImpl: no currency at all, using ShellsAndPebbles");
        aCurrSymbol = "ShellsAndPebbles";
        aCurrBankSymbol = aCurrSymbol;
        nCurrPositiveFormat = nCurrNegativeFormat = nCurrFormatDefault;
        nCurrDigits = 2;
        return;
    }
    auto pCurr = std::find_if(aCurrSeq.begin(), aCurrSeq.end(),
        [](const Currency2& rCurr) { return rCurr.Default; });
    if ( pCurr == aCurrSeq.end() )
    {
        if (areChecksEnabled())
        {
            outputCheckMessage( appendLocaleInfo( "LocaleDataWrapper::getCurrSymbolsImpl: no default currency" ) );
        }
        pCurr = aCurrSeq.begin();
    }
    aCurrSymbol = pCurr->Symbol;
    aCurrBankSymbol = pCurr->BankSymbol;
    nCurrDigits = pCurr->DecimalPlaces;
}

void LocaleDataWrapper::scanCurrFormatImpl( const OUString& rCode,
        sal_Int32 nStart, sal_Int32& nSign, sal_Int32& nPar,
        sal_Int32& nNum, sal_Int32& nBlank, sal_Int32& nSym ) const
{
    nSign = nPar = nNum = nBlank = nSym = -1;
    const sal_Unicode* const pStr = rCode.getStr();
    const sal_Unicode* const pStop = pStr + rCode.getLength();
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
                    if (!nInSection && nSym == -1 && rCode.match(aCurrSymbol, static_cast<sal_Int32>(p - pStr)))
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

void LocaleDataWrapper::getCurrFormatsImpl()
{
    css::uno::Reference< css::i18n::XNumberFormatCode > xNFC = i18n::NumberFormatMapper::create( m_xContext );
    uno::Sequence< NumberFormatCode > aFormatSeq = xNFC->getAllFormatCode( KNumberFormatUsage::CURRENCY, getMyLocale() );
    sal_Int32 nCnt = aFormatSeq.getLength();
    if ( !nCnt )
    {   // bad luck
        if (areChecksEnabled())
        {
            outputCheckMessage( appendLocaleInfo( "LocaleDataWrapper::getCurrFormatsImpl: no currency formats" ) );
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

    // make sure it's loaded
    getCurrSymbol();

    sal_Int32 nSign, nPar, nNum, nBlank, nSym;

    // positive format
    nElem = (nDef >= 0 ? nDef : (nNeg >= 0 ? nNeg : 0));
    scanCurrFormatImpl( pFormatArr[nElem].Code, 0, nSign, nPar, nNum, nBlank, nSym );
    if (areChecksEnabled() && (nNum == -1 || nSym == -1))
    {
        outputCheckMessage( appendLocaleInfo( "LocaleDataWrapper::getCurrFormatsImpl: CurrPositiveFormat?" ) );
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
            outputCheckMessage( appendLocaleInfo( "LocaleDataWrapper::getCurrFormatsImpl: CurrNegativeFormat?" ) );
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
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nDateOrder == DateOrder::Invalid )
    {
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getDateOrdersImpl();
    }
    return nDateOrder;
}

DateOrder LocaleDataWrapper::getLongDateOrder() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nLongDateOrder == DateOrder::Invalid )
    {
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getDateOrdersImpl();
    }
    return nLongDateOrder;
}

DateOrder LocaleDataWrapper::scanDateOrderImpl( const OUString& rCode ) const
{
    // Only some european versions were translated, the ones with different
    // keyword combinations are:
    // English DMY, German TMJ, Spanish DMA, French JMA, Italian GMA,
    // Dutch DMJ, Finnish PKV

    // default is English keywords for every other language
    sal_Int32 nDay = rCode.indexOf('D');
    sal_Int32 nMonth = rCode.indexOf('M');
    sal_Int32 nYear = rCode.indexOf('Y');
    if (nDay == -1 || nMonth == -1 || nYear == -1)
    {   // This algorithm assumes that all three parts (DMY) are present
        if (nMonth == -1)
        {   // only Finnish has something else than 'M' for month
            nMonth = rCode.indexOf('K');
            if (nMonth != -1)
            {
                nDay = rCode.indexOf('P');
                nYear = rCode.indexOf('V');
            }
        }
        else if (nDay == -1)
        {   // We have a month 'M' if we reach this branch.
            // Possible languages containing 'M' but no 'D':
            // German, French, Italian
            nDay = rCode.indexOf('T');         // German
            if (nDay != -1)
                nYear = rCode.indexOf('J');
            else
            {
                nYear = rCode.indexOf('A');    // French, Italian
                if (nYear != -1)
                {
                    nDay = rCode.indexOf('J'); // French
                    if (nDay == -1)
                        nDay = rCode.indexOf('G'); // Italian
                }
            }
        }
        else
        {   // We have a month 'M' and a day 'D'.
            // Possible languages containing 'D' and 'M' but not 'Y':
            // Spanish, Dutch
            nYear = rCode.indexOf('A');        // Spanish
            if (nYear == -1)
                nYear = rCode.indexOf('J');    // Dutch
        }
        if (nDay == -1 || nMonth == -1 || nYear == -1)
        {
            if (areChecksEnabled())
            {
                outputCheckMessage( appendLocaleInfo( "LocaleDataWrapper::scanDateOrder: not all DMY present" ) );
            }
            if (nDay == -1)
                nDay = rCode.getLength();
            if (nMonth == -1)
                nMonth = rCode.getLength();
            if (nYear == -1)
                nYear = rCode.getLength();
        }
    }
    // compare with <= because each position may equal rCode.getLength()
    if ( nDay <= nMonth && nMonth <= nYear )
        return DateOrder::DMY;     // also if every position equals rCode.getLength()
    else if ( nMonth <= nDay && nDay <= nYear )
        return DateOrder::MDY;
    else if ( nYear <= nMonth && nMonth <= nDay )
        return DateOrder::YMD;
    else
    {
        if (areChecksEnabled())
        {
            outputCheckMessage( appendLocaleInfo( "LocaleDataWrapper::scanDateOrder: no magic applicable" ) );
        }
        return DateOrder::DMY;
    }
}

void LocaleDataWrapper::getDateOrdersImpl()
{
    css::uno::Reference< css::i18n::XNumberFormatCode > xNFC = i18n::NumberFormatMapper::create( m_xContext );
    uno::Sequence< NumberFormatCode > aFormatSeq = xNFC->getAllFormatCode( KNumberFormatUsage::DATE, getMyLocale() );
    sal_Int32 nCnt = aFormatSeq.getLength();
    if ( !nCnt )
    {   // bad luck
        if (areChecksEnabled())
        {
            outputCheckMessage( appendLocaleInfo( "LocaleDataWrapper::getDateOrdersImpl: no date formats" ) );
        }
        nDateOrder = nLongDateOrder = DateOrder::DMY;
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
            outputCheckMessage( appendLocaleInfo( "LocaleDataWrapper::getDateOrdersImpl: no edit" ) );
        }
        if ( nDef == -1 )
        {
            if (areChecksEnabled())
            {
                outputCheckMessage( appendLocaleInfo( "LocaleDataWrapper::getDateOrdersImpl: no default" ) );
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
    DateOrder nDF = scanDateOrderImpl( pFormatArr[nEdit].Code );
    if ( pFormatArr[nEdit].Type == KNumberFormatType::LONG )
    {   // normally this is not the case
        nLongDateOrder = nDateOrder = nDF;
    }
    else
    {
        nDateOrder = nDF;
        if ( nLong == -1 )
            nLongDateOrder = nDF;
        else
            nLongDateOrder = scanDateOrderImpl( pFormatArr[nLong].Code );
    }
}

// --- digit grouping -------------------------------------------------

void LocaleDataWrapper::getDigitGroupingImpl()
{
    /* TODO: This is a very simplified grouping setup that only serves its
     * current purpose for Indian locales. A free-form flexible one would
     * obtain grouping from locale data where it could be specified using, for
     * example, codes like #,### and #,##,### that would generate the integer
     * sequence. Needed additional API and a locale data element.
     */

    if (!aGrouping.hasElements())
    {
        aGrouping.realloc(3);   // room for {3,2,0}
        aGrouping[0] = 0;       // invalidate
    }
    if (aGrouping[0])
        return;

    i18n::LanguageCountryInfo aLCInfo( getLanguageCountryInfo());
    if (aLCInfo.Country.equalsIgnoreAsciiCase("IN") || // India
        aLCInfo.Country.equalsIgnoreAsciiCase("BT") )  // Bhutan
    {
        aGrouping[0] = 3;
        aGrouping[1] = 2;
        aGrouping[2] = 0;
    }
    else
    {
        aGrouping[0] = 3;
        aGrouping[1] = 0;
    }
}

css::uno::Sequence< sal_Int32 > LocaleDataWrapper::getDigitGrouping() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if (!aGrouping.hasElements() || aGrouping[0] == 0)
    {   // no cached content
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getDigitGroupingImpl();
    }
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

static void ImplAdd2UNum( OUStringBuffer& rBuf, sal_uInt16 nNumber, bool bLeading )
{
    DBG_ASSERT( nNumber < 100, "ImplAdd2UNum() - Number >= 100" );

    if ( nNumber < 10 )
    {
        if ( bLeading )
        {
            rBuf.append('0');
        }
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
    if ( nNumber < 0 )
    {
        nNumber *= -1;
        rBuf.append('-');
    }

    // convert number
    ImplAddUNum( aNumBuf, static_cast<sal_uInt64>(nNumber) );
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
            rBuf.append( getNumDecimalSep() );

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
        const OUString& rThoSep = getNumThousandSep();

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
            rBuf.append( getNumDecimalSep() );

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
    ::utl::ReadWriteGuard aGuard( aMutex, ReadWriteGuardMode::BlockCritical );
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
            ImplAdd2UNum( aBuf, nDay, true /* IsDateDayLeadingZero() */ );
            aBuf.append( getDateSep() );
            ImplAdd2UNum( aBuf, nMonth, true /* IsDateMonthLeadingZero() */ );
            aBuf.append( getDateSep() );
            ImplAddNum( aBuf, nYear, nYearLen );
        break;
        case DateOrder::MDY :
            ImplAdd2UNum( aBuf, nMonth, true /* IsDateMonthLeadingZero() */ );
            aBuf.append( getDateSep() );
            ImplAdd2UNum( aBuf, nDay, true /* IsDateDayLeadingZero() */ );
            aBuf.append( getDateSep() );
            ImplAddNum( aBuf, nYear, nYearLen );
        break;
        default:
            ImplAddNum( aBuf, nYear, nYearLen );
            aBuf.append( getDateSep() );
            ImplAdd2UNum( aBuf, nMonth, true /* IsDateMonthLeadingZero() */ );
            aBuf.append( getDateSep() );
            ImplAdd2UNum( aBuf, nDay, true /* IsDateDayLeadingZero() */ );
    }

    return aBuf.makeStringAndClear();
}

OUString LocaleDataWrapper::getTime( const tools::Time& rTime, bool bSec, bool b100Sec ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ReadWriteGuardMode::BlockCritical );
//!TODO: leading zeros et al
    OUStringBuffer aBuf(128);
    sal_uInt16  nHour = rTime.GetHour();

    nHour %= 24;

    ImplAdd2UNum( aBuf, nHour, true /* IsTimeLeadingZero() */ );
    aBuf.append( getTimeSep() );
    ImplAdd2UNum( aBuf, rTime.GetMin(), true );
    if ( bSec )
    {
        aBuf.append( getTimeSep() );
        ImplAdd2UNum( aBuf, rTime.GetSec(), true );

        if ( b100Sec )
        {
            aBuf.append( getTime100SecSep() );
            ImplAdd9UNum( aBuf, rTime.GetNanoSec() );
        }
    }

    return aBuf.makeStringAndClear();
}

OUString LocaleDataWrapper::getLongDate( const Date& rDate, CalendarWrapper& rCal,
        bool bTwoDigitYear ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ReadWriteGuardMode::BlockCritical );
    OUStringBuffer aBuf(20);
    OUStringBuffer aStr(120); // complete guess
    sal_Int16 nVal;
    rCal.setGregorianDateTime( rDate );
    // day of week
    nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_WEEK );
    aStr.append(rCal.getDisplayName( CalendarDisplayIndex::DAY, nVal, 1 ));
    aStr.append(getLongDateDayOfWeekSep());
    // day of month
    nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_MONTH );
    ImplAdd2UNum( aBuf, nVal, false/*bDayOfMonthWithLeadingZero*/ );
    OUString aDay = aBuf.makeStringAndClear();
    // month of year
    nVal = rCal.getValue( CalendarFieldIndex::MONTH );
    OUString aMonth( rCal.getDisplayName( CalendarDisplayIndex::MONTH, nVal, 1 ) );
    // year
    nVal = rCal.getValue( CalendarFieldIndex::YEAR );
    if ( bTwoDigitYear )
        ImplAddUNum( aBuf, nVal % 100, 2 );
    else
        ImplAddUNum( aBuf, nVal );
    OUString aYear = aBuf.makeStringAndClear();
    // concatenate
    switch ( getLongDateOrder() )
    {
        case DateOrder::DMY :
            aStr.append(aDay + getLongDateDaySep() + aMonth + getLongDateMonthSep() + aYear);
        break;
        case DateOrder::MDY :
            aStr.append(aMonth + getLongDateMonthSep() + aDay + getLongDateDaySep() + aYear);
        break;
        default:    // YMD
            aStr.append(aYear + getLongDateYearSep() + aMonth + getLongDateMonthSep() + aDay);
    }
    return aStr.makeStringAndClear();
}

OUString LocaleDataWrapper::getDuration( const tools::Time& rTime, bool bSec, bool b100Sec ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ReadWriteGuardMode::BlockCritical );
    OUStringBuffer aBuf(128);

    if ( rTime < tools::Time( 0 ) )
        aBuf.append(' ' );

    if ( (true) /* IsTimeLeadingZero() */ )
        ImplAddUNum( aBuf, rTime.GetHour(), 2 );
    else
        ImplAddUNum( aBuf, rTime.GetHour() );
    aBuf.append( getTimeSep() );
    ImplAdd2UNum( aBuf, rTime.GetMin(), true );
    if ( bSec )
    {
        aBuf.append( getTimeSep() );
        ImplAdd2UNum( aBuf, rTime.GetSec(), true );

        if ( b100Sec )
        {
            aBuf.append( getTime100SecSep() );
            ImplAdd9UNum( aBuf, rTime.GetNanoSec() );
        }
    }

    return aBuf.makeStringAndClear();
}

// --- simple number formatting ---------------------------------------

static size_t ImplGetNumberStringLengthGuess( const LocaleDataWrapper& rLoc, sal_uInt16 nDecimals )
{
    // approximately 3.2 bits per digit
    const size_t nDig = ((sizeof(sal_Int64) * 8) / 3) + 1;
    // digits, separators (pessimized for insane "every digit may be grouped"), leading zero, sign
    size_t nGuess = ((nDecimals < nDig) ?
        (((nDig - nDecimals) * rLoc.getNumThousandSep().getLength()) + nDig) :
        nDecimals) + rLoc.getNumDecimalSep().getLength() + 3;
    return nGuess;
}

OUString LocaleDataWrapper::getNum( sal_Int64 nNumber, sal_uInt16 nDecimals,
        bool bUseThousandSep, bool bTrailingZeros ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ReadWriteGuardMode::BlockCritical );
    // check if digits and separators will fit into fixed buffer or allocate
    size_t nGuess = ImplGetNumberStringLengthGuess( *this, nDecimals );
    OUStringBuffer aBuf(int(nGuess + 16));

    ImplAddFormatNum( aBuf, nNumber, nDecimals,
        bUseThousandSep, bTrailingZeros );

    return aBuf.makeStringAndClear();
}

OUString LocaleDataWrapper::getCurr( sal_Int64 nNumber, sal_uInt16 nDecimals,
        const OUString& rCurrencySymbol, bool bUseThousandSep ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ReadWriteGuardMode::BlockCritical );
    sal_Unicode cZeroChar = getCurrZeroChar();

    // check if digits and separators will fit into fixed buffer or allocate
    size_t nGuess = ImplGetNumberStringLengthGuess( *this, nDecimals );
    OUStringBuffer aNumBuf(int(nGuess + 16));
    OUStringBuffer aBuf(int(rCurrencySymbol.getLength() + nGuess + 20 ));

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

    if ( !bNeg )
    {
        switch( getCurrPositiveFormat() )
        {
            case 0:
                aBuf.append( rCurrencySymbol );
                aBuf.append( aNumBuf );
                break;
            case 1:
                aBuf.append( aNumBuf );
                aBuf.append( rCurrencySymbol );
                break;
            case 2:
                aBuf.append( rCurrencySymbol );
                aBuf.append( ' ' );
                aBuf.append( aNumBuf );
                break;
            case 3:
                aBuf.append( aNumBuf );
                aBuf.append( ' ' );
                aBuf.append( rCurrencySymbol );
                break;
        }
    }
    else
    {
        switch( getCurrNegativeFormat() )
        {
            case 0:
                 aBuf.append( '(' );
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( aNumBuf );
                 aBuf.append( ')' );
                break;
            case 1:
                 aBuf.append( '-' );
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( aNumBuf );
                break;
            case 2:
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( '-' );
                 aBuf.append( aNumBuf );
                break;
            case 3:
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( aNumBuf );
                 aBuf.append( '-' );
                break;
            case 4:
                 aBuf.append( '(' );
                 aBuf.append( aNumBuf );
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( ')' );
                break;
            case 5:
                 aBuf.append( '-' );
                 aBuf.append( aNumBuf );
                 aBuf.append( rCurrencySymbol );
                break;
            case 6:
                 aBuf.append( aNumBuf );
                 aBuf.append( '-' );
                 aBuf.append( rCurrencySymbol );
                break;
            case 7:
                 aBuf.append( aNumBuf );
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( '-' );
                break;
            case 8:
                 aBuf.append( '-' );
                 aBuf.append( aNumBuf );
                 aBuf.append( ' ' );
                 aBuf.append( rCurrencySymbol );
                break;
            case 9:
                 aBuf.append( '-' );
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( ' ' );
                 aBuf.append( aNumBuf );
                break;
            case 10:
                 aBuf.append( aNumBuf );
                 aBuf.append( ' ' );
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( '-' );
                break;
            case 11:
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( ' ' );
                 aBuf.append( '-' );
                 aBuf.append( aNumBuf );
                break;
            case 12:
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( ' ' );
                 aBuf.append( aNumBuf );
                 aBuf.append( '-' );
                break;
            case 13:
                 aBuf.append( aNumBuf );
                 aBuf.append( '-' );
                 aBuf.append( ' ' );
                 aBuf.append( rCurrencySymbol );
                break;
            case 14:
                 aBuf.append( '(' );
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( ' ' );
                 aBuf.append( aNumBuf );
                 aBuf.append( ')' );
                break;
            case 15:
                 aBuf.append( '(' );
                 aBuf.append( aNumBuf );
                 aBuf.append( ' ' );
                 aBuf.append( rCurrencySymbol );
                 aBuf.append( ')' );
                break;
        }
    }

    return aBuf.makeStringAndClear();
}

// --- number parsing -------------------------------------------------

double LocaleDataWrapper::stringToDouble( const OUString& rString, bool bUseGroupSep,
        rtl_math_ConversionStatus* pStatus, sal_Int32* pParseEnd ) const
{
    const sal_Unicode cGroupSep = (bUseGroupSep ? getNumThousandSep()[0] : 0);
    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    sal_Int32 nParseEnd = 0;
    double fValue = rtl::math::stringToDouble( rString, getNumDecimalSep()[0], cGroupSep, &eStatus, &nParseEnd);
    bool bTryAlt = (nParseEnd < rString.getLength() && !getNumDecimalSepAlt().isEmpty() &&
            rString[nParseEnd] == getNumDecimalSepAlt().toChar());
    // Try re-parsing with alternative if that was the reason to stop.
    if (bTryAlt)
        fValue = rtl::math::stringToDouble( rString, getNumDecimalSepAlt().toChar(), cGroupSep, &eStatus, &nParseEnd);
    if (pStatus)
        *pStatus = eStatus;
    if (pParseEnd)
        *pParseEnd = nParseEnd;
    return fValue;
}

double LocaleDataWrapper::stringToDouble( const sal_Unicode* pBegin, const sal_Unicode* pEnd, bool bUseGroupSep,
        rtl_math_ConversionStatus* pStatus, const sal_Unicode** ppParseEnd ) const
{
    const sal_Unicode cGroupSep = (bUseGroupSep ? getNumThousandSep()[0] : 0);
    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    const sal_Unicode* pParseEnd = nullptr;
    double fValue = rtl_math_uStringToDouble( pBegin, pEnd, getNumDecimalSep()[0], cGroupSep, &eStatus, &pParseEnd);
    bool bTryAlt = (pParseEnd < pEnd && !getNumDecimalSepAlt().isEmpty() &&
            *pParseEnd == getNumDecimalSepAlt().toChar());
    // Try re-parsing with alternative if that was the reason to stop.
    if (bTryAlt)
        fValue = rtl_math_uStringToDouble( pBegin, pEnd, getNumDecimalSepAlt().toChar(), cGroupSep, &eStatus, &pParseEnd);
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

OUString LocaleDataWrapper::appendLocaleInfo(const OUString& rDebugMsg) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ReadWriteGuardMode::BlockCritical );
    OUStringBuffer aDebugMsg(rDebugMsg);
    aDebugMsg.append('\n');
    aDebugMsg.append(maLanguageTag.getBcp47());
    aDebugMsg.append(" requested\n");
    LanguageTag aLoaded = getLoadedLanguageTag();
    aDebugMsg.append(aLoaded.getBcp47());
    aDebugMsg.append(" loaded");
    return aDebugMsg.makeStringAndClear();
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
    return css::uno::Sequence< css::i18n::Calendar2 >(0);
}

// --- XLocaleData4 ----------------------------------------------------------

css::uno::Sequence< OUString > LocaleDataWrapper::getDateAcceptancePatterns() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );

    if (aDateAcceptancePatterns.hasElements())
        return aDateAcceptancePatterns;

    aGuard.changeReadToWrite();

    try
    {
        const_cast<LocaleDataWrapper*>(this)->aDateAcceptancePatterns =
            xLD->getDateAcceptancePatterns( getMyLocale() );
        return aDateAcceptancePatterns;
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getDateAcceptancePatterns" );
    }
    return css::uno::Sequence< OUString >(0);
}

// --- Override layer --------------------------------------------------------

void LocaleDataWrapper::setDateAcceptancePatterns(
        const css::uno::Sequence< OUString > & rPatterns )
{
    ::utl::ReadWriteGuard aGuard( aMutex, ReadWriteGuardMode::Write );

    if (!aDateAcceptancePatterns.hasElements() || !rPatterns.hasElements())
    {
        try
        {
            aDateAcceptancePatterns = xLD->getDateAcceptancePatterns( getMyLocale() );
        }
        catch (const Exception&)
        {
            TOOLS_WARN_EXCEPTION( "unotools.i18n", "setDateAcceptancePatterns" );
        }
        if (!rPatterns.hasElements())
            return;     // just a reset
        if (!aDateAcceptancePatterns.hasElements())
        {
            aDateAcceptancePatterns = rPatterns;
            return;
        }
    }

    // Never overwrite the locale's full date pattern! The first.
    if (aDateAcceptancePatterns[0] == rPatterns[0])
        aDateAcceptancePatterns = rPatterns;    // sane
    else
    {
        // Copy existing full date pattern and append the sequence passed.
        /* TODO: could check for duplicates and shrink target sequence */
        Sequence< OUString > aTmp( rPatterns.getLength() + 1 );
        aTmp[0] = aDateAcceptancePatterns[0];
        std::copy(rPatterns.begin(), rPatterns.end(), std::next(aTmp.begin()));
        aDateAcceptancePatterns = aTmp;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
