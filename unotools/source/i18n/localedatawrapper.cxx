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

#include <string.h>
#include <stdio.h>
#include <string>

#include <sal/log.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/numberformatcodewrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/digitgroupingiterator.hxx>
#include <tools/debug.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <com/sun/star/i18n/KNumberFormatUsage.hpp>
#include <com/sun/star/i18n/KNumberFormatType.hpp>
#include <com/sun/star/i18n/LocaleData.hpp>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#include <com/sun/star/i18n/NumberFormatIndex.hpp>

#include <comphelper/processfactory.hxx>
#include <rtl/instance.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <sal/macros.h>

static const int nDateFormatInvalid = -1;
static const sal_uInt16 nCurrFormatInvalid = 0xffff;
static const sal_uInt16 nCurrFormatDefault = 0;

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
            uno::Sequence< sal_uInt16 >, InstalledLanguageTypes >
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
        xLD( LocaleData::create(rxContext) ),
        maLanguageTag( rLanguageTag ),
        bLocaleDataItemValid( false ),
        bReservedWordValid( false )
{
    invalidateData();
}

LocaleDataWrapper::LocaleDataWrapper(
            const LanguageTag& rLanguageTag
            )
        :
        m_xContext( comphelper::getProcessComponentContext() ),
        xLD( LocaleData::create(m_xContext) ),
        maLanguageTag( rLanguageTag ),
        bLocaleDataItemValid( false ),
        bReservedWordValid( false )
{
    invalidateData();
}

LocaleDataWrapper::~LocaleDataWrapper()
{
}

void LocaleDataWrapper::setLanguageTag( const LanguageTag& rLanguageTag )
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nCriticalChange );
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
    nDateFormat = nLongDateFormat = nDateFormatInvalid;
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
    if (aGrouping.getLength())
        aGrouping[0] = 0;
    if (aDateAcceptancePatterns.getLength())
        aDateAcceptancePatterns = Sequence<OUString>();
    // dummies
    cCurrZeroChar = '0';
}

/* FIXME-BCP47: locale data should provide a language tag instead that could be
 * passed on. */
css::i18n::LanguageCountryInfo LocaleDataWrapper::getLanguageCountryInfo() const
{
    try
    {
        return xLD->getLanguageCountryInfo( getMyLocale() );
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getLanguageCountryInfo: Exception caught " << e.Message );
    }
    return css::i18n::LanguageCountryInfo();
}

const css::i18n::LocaleDataItem& LocaleDataWrapper::getLocaleItem() const
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
        css::i18n::LocaleDataItem aItem = xLD->getLocaleItem( rLocal );
        auto aRet = maDataItemCache.insert(std::make_pair(rLocal, aItem));
        assert(aRet.second);
        return aRet.first->second;
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getLocaleItem: Exception caught " << e.Message );
    }
    static css::i18n::LocaleDataItem aEmptyItem;
    return aEmptyItem;
}

css::uno::Sequence< css::i18n::Currency2 > LocaleDataWrapper::getAllCurrencies() const
{
    try
    {
        return xLD->getAllCurrencies2( getMyLocale() );
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getAllCurrencies: Exception caught " << e.Message );
    }
    return css::uno::Sequence< css::i18n::Currency2 >(0);
}

css::uno::Sequence< css::i18n::FormatElement > LocaleDataWrapper::getAllFormats() const
{
    try
    {
        return xLD->getAllFormats( getMyLocale() );
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getAllFormats: Exception caught " << e.Message );
    }
    return css::uno::Sequence< css::i18n::FormatElement >(0);
}

css::i18n::ForbiddenCharacters LocaleDataWrapper::getForbiddenCharacters() const
{
    try
    {
        return xLD->getForbiddenCharacters( getMyLocale() );
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getForbiddenCharacters: Exception caught " << e.Message );
    }
    return css::i18n::ForbiddenCharacters();
}

css::uno::Sequence< OUString > LocaleDataWrapper::getReservedWord() const
{
    try
    {
        return xLD->getReservedWord( getMyLocale() );
    }
    catch ( const Exception& e )
    {
        SAL_WARN( "unotools.i18n", "getReservedWord: Exception caught " << e.Message );
    }
    return css::uno::Sequence< OUString >(0);
}

css::uno::Sequence< css::lang::Locale > LocaleDataWrapper::getAllInstalledLocaleNames() const
{
    uno::Sequence< lang::Locale > &rInstalledLocales = InstalledLocales::get();

    if ( rInstalledLocales.getLength() )
        return rInstalledLocales;

    try
    {
        rInstalledLocales = xLD->getAllInstalledLocaleNames();
    }
    catch ( const Exception& e )
    {
        SAL_WARN( "unotools.i18n", "getAllInstalledLocaleNames: Exception caught " << e.Message );
    }
    return rInstalledLocales;
}

// --- Impl and helpers ----------------------------------------------------

// static
css::uno::Sequence< css::lang::Locale > LocaleDataWrapper::getInstalledLocaleNames()
{
    const uno::Sequence< lang::Locale > &rInstalledLocales =
        InstalledLocales::get();

    if ( !rInstalledLocales.getLength() )
    {
        LocaleDataWrapper aLDW( ::comphelper::getProcessComponentContext(), LanguageTag( LANGUAGE_SYSTEM) );
        aLDW.getAllInstalledLocaleNames();
    }
    return rInstalledLocales;
}

// static
css::uno::Sequence< sal_uInt16 > LocaleDataWrapper::getInstalledLanguageTypes()
{
    uno::Sequence< sal_uInt16 > &rInstalledLanguageTypes =
        InstalledLanguageTypes::get();

    if ( rInstalledLanguageTypes.getLength() )
        return rInstalledLanguageTypes;

    css::uno::Sequence< css::lang::Locale > xLoc =  getInstalledLocaleNames();
    sal_Int32 nCount = xLoc.getLength();
    css::uno::Sequence< sal_uInt16 > xLang( nCount );
    sal_Int32 nLanguages = 0;
    for ( sal_Int32 i=0; i<nCount; i++ )
    {
        LanguageTag aLanguageTag( xLoc[i] );
        OUString aDebugLocale;
        if (areChecksEnabled())
        {
            aDebugLocale = aLanguageTag.getBcp47( false);
        }

        LanguageType eLang = aLanguageTag.getLanguageType( false);
        if (areChecksEnabled() && eLang == LANGUAGE_DONTKNOW)
        {
            OUStringBuffer aMsg("ConvertIsoNamesToLanguage: unknown MS-LCID for locale\n");
            aMsg.append(aDebugLocale);
            outputCheckMessage(aMsg.makeStringAndClear());
        }

        switch ( eLang )
        {
            case LANGUAGE_NORWEGIAN :       // no_NO, not Bokmal (nb_NO), not Nynorsk (nn_NO)
                eLang = LANGUAGE_DONTKNOW;  // don't offer "Unknown" language
                break;
        }
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
                    aMsg.append(static_cast<sal_Int32>(eLang), 16);
                    aMsg.append("  ->  ");
                    aMsg.append(aBackLanguageTag.getBcp47());
                    outputCheckMessage( aMsg.makeStringAndClear() );
                }
                eLang = LANGUAGE_DONTKNOW;
            }
        }
        if ( eLang != LANGUAGE_DONTKNOW )
            xLang[ nLanguages++ ] = eLang;
    }
    if ( nLanguages < nCount )
        xLang.realloc( nLanguages );
    rInstalledLanguageTypes = xLang;

    return rInstalledLanguageTypes;
}

const OUString& LocaleDataWrapper::getOneLocaleItem( sal_Int16 nItem ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nItem >= LocaleItem::COUNT )
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
        return MEASURE_METRIC;
//! TODO: other measurement systems? => extend enum MeasurementSystem
    return MEASURE_US;
}

void LocaleDataWrapper::getDefaultCalendarImpl()
{
    if (!xDefaultCalendar)
    {
        Sequence< Calendar2 > xCals = getAllCalendars();
        sal_Int32 nCount = xCals.getLength();
        sal_Int32 nDef = 0;
        if (nCount > 1)
        {
            const Calendar2* pArr = xCals.getArray();
            for (sal_Int32 i=0; i<nCount; ++i)
            {
                if (pArr[i].Default)
                {
                    nDef = i;
                    break;
                }
            }
        }
        xDefaultCalendar.reset( new Calendar2( xCals[nDef]));
    }
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

const css::uno::Sequence< css::i18n::CalendarItem2 > LocaleDataWrapper::getDefaultCalendarDays() const
{
    return getDefaultCalendar()->Days;
}

const css::uno::Sequence< css::i18n::CalendarItem2 > LocaleDataWrapper::getDefaultCalendarMonths() const
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
    sal_Int32 nCnt = aCurrSeq.getLength();
    Currency2 const * const pCurrArr = aCurrSeq.getArray();
    sal_Int32 nElem;
    for ( nElem = 0; nElem < nCnt; nElem++ )
    {
        if ( pCurrArr[nElem].Default )
            break;
    }
    if ( nElem >= nCnt )
    {
        if (areChecksEnabled())
        {
            OUString aMsg( "LocaleDataWrapper::getCurrSymbolsImpl: no default currency" );
            outputCheckMessage( appendLocaleInfo( aMsg ) );
        }
        nElem = 0;
        if ( nElem >= nCnt )
        {
            if (areChecksEnabled())
                outputCheckMessage(OUString("LocaleDataWrapper::getCurrSymbolsImpl: no currency at all, using ShellsAndPebbles"));
            aCurrSymbol = "ShellsAndPebbles";
            aCurrBankSymbol = aCurrSymbol;
            nCurrPositiveFormat = nCurrNegativeFormat = nCurrFormatDefault;
            nCurrDigits = 2;
            return;
        }
    }
    aCurrSymbol = pCurrArr[nElem].Symbol;
    aCurrBankSymbol = pCurrArr[nElem].BankSymbol;
    nCurrDigits = pCurrArr[nElem].DecimalPlaces;
}

void LocaleDataWrapper::scanCurrFormatImpl( const OUString& rCode,
        sal_Int32 nStart, sal_Int32& nSign, sal_Int32& nPar,
        sal_Int32& nNum, sal_Int32& nBlank, sal_Int32& nSym )
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
                    if (!nInSection && nSym == -1 && rCode.match(aCurrSymbol, (sal_Int32)(p - pStr)))
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
    NumberFormatCodeWrapper aNumberFormatCode( m_xContext, getMyLocale() );
    uno::Sequence< NumberFormatCode > aFormatSeq
        = aNumberFormatCode.getAllFormatCode( KNumberFormatUsage::CURRENCY );
    sal_Int32 nCnt = aFormatSeq.getLength();
    if ( !nCnt )
    {   // bad luck
        if (areChecksEnabled())
        {
            OUString aMsg( "LocaleDataWrapper::getCurrFormatsImpl: no currency formats" );
            outputCheckMessage( appendLocaleInfo( aMsg ) );
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
        OUString aMsg( "LocaleDataWrapper::getCurrFormatsImpl: CurrPositiveFormat?" );
        outputCheckMessage( appendLocaleInfo( aMsg ) );
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
            OUString aMsg( "LocaleDataWrapper::getCurrFormatsImpl: CurrNegativeFormat?" );
            outputCheckMessage( appendLocaleInfo( aMsg ) );
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

DateFormat LocaleDataWrapper::getDateFormat() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nDateFormat == nDateFormatInvalid )
    {
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getDateFormatsImpl();
    }
    return (DateFormat) nDateFormat;
}

DateFormat LocaleDataWrapper::getLongDateFormat() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nLongDateFormat == nDateFormatInvalid )
    {
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getDateFormatsImpl();
    }
    return (DateFormat) nLongDateFormat;
}

DateFormat LocaleDataWrapper::scanDateFormatImpl( const OUString& rCode )
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
                OUString aMsg( "LocaleDataWrapper::scanDateFormat: not all DMY present" );
                outputCheckMessage( appendLocaleInfo( aMsg ) );
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
        return DMY;     // also if every position equals rCode.getLength()
    else if ( nMonth <= nDay && nDay <= nYear )
        return MDY;
    else if ( nYear <= nMonth && nMonth <= nDay )
        return YMD;
    else
    {
        if (areChecksEnabled())
        {
            OUString aMsg( "LocaleDataWrapper::scanDateFormat: no magic applicable" );
            outputCheckMessage( appendLocaleInfo( aMsg ) );
        }
        return DMY;
    }
}

void LocaleDataWrapper::getDateFormatsImpl()
{
    NumberFormatCodeWrapper aNumberFormatCode( m_xContext, getMyLocale() );
    uno::Sequence< NumberFormatCode > aFormatSeq
        = aNumberFormatCode.getAllFormatCode( KNumberFormatUsage::DATE );
    sal_Int32 nCnt = aFormatSeq.getLength();
    if ( !nCnt )
    {   // bad luck
        if (areChecksEnabled())
        {
            OUString aMsg( "LocaleDataWrapper::getDateFormatsImpl: no date formats" );
            outputCheckMessage( appendLocaleInfo( aMsg ) );
        }
        nDateFormat = nLongDateFormat = DMY;
        return;
    }
    // find the edit (21), a default (medium preferred),
    // a medium (default preferred), and a long (default preferred)
    NumberFormatCode const * const pFormatArr = aFormatSeq.getArray();
    sal_Int32 nElem, nEdit, nDef, nMedium, nLong;
    nEdit = nDef = nMedium = nLong = -1;
    for ( nElem = 0; nElem < nCnt; nElem++ )
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
            OUString aMsg( "LocaleDataWrapper::getDateFormatsImpl: no edit" );
            outputCheckMessage( appendLocaleInfo( aMsg ) );
        }
        if ( nDef == -1 )
        {
            if (areChecksEnabled())
            {
                OUString aMsg( "LocaleDataWrapper::getDateFormatsImpl: no default" );
                outputCheckMessage( appendLocaleInfo( aMsg ) );
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
    DateFormat nDF = scanDateFormatImpl( pFormatArr[nEdit].Code );
    if ( pFormatArr[nEdit].Type == KNumberFormatType::LONG )
    {   // normally this is not the case
        nLongDateFormat = nDateFormat = nDF;
    }
    else
    {
        nDateFormat = nDF;
        if ( nLong == -1 )
            nLongDateFormat = nDF;
        else
            nLongDateFormat = scanDateFormatImpl( pFormatArr[nLong].Code );
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

    if (!aGrouping.getLength())
    {
        aGrouping.realloc(3);   // room for {3,2,0}
        aGrouping[0] = 0;       // invalidate
    }
    if (!aGrouping[0])
    {
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
}

const css::uno::Sequence< sal_Int32 > LocaleDataWrapper::getDigitGrouping() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if (!aGrouping.getLength() || aGrouping[0] == 0)
    {   // no cached content
        aGuard.changeReadToWrite();
        const_cast<LocaleDataWrapper*>(this)->getDigitGroupingImpl();
    }
    return aGrouping;
}

// --- simple number formatting helpers -------------------------------

// The ImplAdd... methods are taken from class International and modified to
// suit the needs.

static sal_Unicode* ImplAddUNum( sal_Unicode* pBuf, sal_uInt64 nNumber )
{
    // fill temp buffer with digits
    sal_Unicode aTempBuf[64];
    sal_Unicode* pTempBuf = aTempBuf;
    do
    {
        *pTempBuf = (sal_Unicode)(nNumber % 10) + '0';
        pTempBuf++;
        nNumber /= 10;
    }
    while ( nNumber );

    // copy temp buffer to buffer passed
    do
    {
        pTempBuf--;
        *pBuf = *pTempBuf;
        pBuf++;
    }
    while ( pTempBuf != aTempBuf );

    return pBuf;
}

static sal_Unicode* ImplAddUNum( sal_Unicode* pBuf, sal_uInt64 nNumber, int nMinLen )
{
    // fill temp buffer with digits
    sal_Unicode aTempBuf[64];
    sal_Unicode* pTempBuf = aTempBuf;
    do
    {
        *pTempBuf = (sal_Unicode)(nNumber % 10) + '0';
        pTempBuf++;
        nNumber /= 10;
        nMinLen--;
    }
    while ( nNumber );

    // fill with zeros up to the minimal length
    while ( nMinLen > 0 )
    {
        *pBuf = '0';
        pBuf++;
        nMinLen--;
    }

    // copy temp buffer to real buffer
    do
    {
        pTempBuf--;
        *pBuf = *pTempBuf;
        pBuf++;
    }
    while ( pTempBuf != aTempBuf );

    return pBuf;
}

static sal_Unicode* ImplAddNum( sal_Unicode* pBuf, sal_Int64 nNumber, int nMinLen )
{
    if (nNumber < 0)
    {
        *pBuf++ = '-';
        nNumber = -nNumber;
    }
    return ImplAddUNum( pBuf, nNumber, nMinLen);
}

static sal_Unicode* ImplAdd2UNum( sal_Unicode* pBuf, sal_uInt16 nNumber, bool bLeading )
{
    DBG_ASSERT( nNumber < 100, "ImplAdd2UNum() - Number >= 100" );

    if ( nNumber < 10 )
    {
        if ( bLeading )
        {
            *pBuf = '0';
            pBuf++;
        }
        *pBuf = nNumber + '0';
    }
    else
    {
        sal_uInt16 nTemp = nNumber % 10;
        nNumber /= 10;
        *pBuf = nNumber + '0';
        pBuf++;
        *pBuf = nTemp + '0';
    }

    pBuf++;
    return pBuf;
}

static sal_Unicode* ImplAdd9UNum( sal_Unicode* pBuf, sal_uInt32 nNumber, bool bLeading )
{
    DBG_ASSERT( nNumber < 1000000000, "ImplAdd9UNum() - Number >= 1000000000" );

    std::ostringstream ostr;
    if (bLeading)
    {
        ostr.fill('0');
        ostr.width(9);
    }
    ostr << nNumber;
    std::string aStr = ostr.str();
    for(const char *pAB= aStr.c_str(); *pAB != '\0'; ++pAB, ++pBuf)
    {
        *pBuf = *pAB;
    }

    return pBuf;
}

inline sal_Unicode* ImplAddString( sal_Unicode* pBuf, const OUString& rStr )
{
    if ( rStr.getLength() == 1 )
        *pBuf++ = rStr[0];
    else if (rStr.isEmpty())
;
    else
    {
        memcpy( pBuf, rStr.getStr(), rStr.getLength() * sizeof(sal_Unicode) );
        pBuf += rStr.getLength();
    }
    return pBuf;
}

inline sal_Unicode* ImplAddString( sal_Unicode* pBuf, sal_Unicode c )
{
    *pBuf = c;
    pBuf++;
    return pBuf;
}

inline sal_Unicode* ImplAddString( sal_Unicode* pBuf, const sal_Unicode* pCopyBuf, sal_Int32 nLen )
{
    memcpy( pBuf, pCopyBuf, nLen * sizeof(sal_Unicode) );
    return pBuf + nLen;
}

sal_Unicode* LocaleDataWrapper::ImplAddFormatNum( sal_Unicode* pBuf,
        sal_Int64 nNumber, sal_uInt16 nDecimals, bool bUseThousandSep,
        bool bTrailingZeros ) const
{
    sal_Unicode aNumBuf[64];
    sal_Unicode* pNumBuf;
    sal_uInt16  nNumLen;
    sal_uInt16  i = 0;

    // negative number
    if ( nNumber < 0 )
    {
        nNumber *= -1;
        *pBuf = '-';
        pBuf++;
    }

    // convert number
    pNumBuf = ImplAddUNum( aNumBuf, (sal_uInt64)nNumber );
    nNumLen = (sal_uInt16)(sal_uLong)(pNumBuf-aNumBuf);
    pNumBuf = aNumBuf;

    if ( nNumLen <= nDecimals )
    {
        // strip .0 in decimals?
        if ( !nNumber && !bTrailingZeros )
        {
            *pBuf = '0';
            pBuf++;
        }
        else
        {
            // LeadingZero, insert 0
            if ( isNumLeadingZero() )
            {
                *pBuf = '0';
                pBuf++;
            }

            // append decimal separator
            pBuf = ImplAddString( pBuf, getNumDecimalSep() );

            // fill with zeros
            while ( i < (nDecimals-nNumLen) )
            {
                *pBuf = '0';
                pBuf++;
                i++;
            }

            // append decimals
            while ( nNumLen )
            {
                *pBuf = *pNumBuf;
                pBuf++;
                pNumBuf++;
                nNumLen--;
            }
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
        for (; i < nNumLen2; ++i )
        {
            *pBuf = *pNumBuf;
            pBuf++;
            pNumBuf++;

            // add thousand separator?
            if ( bUseThousandSep && aGroupPos[i] )
                pBuf = ImplAddString( pBuf, rThoSep );
        }

        // append decimals
        if ( nDecimals )
        {
            pBuf = ImplAddString( pBuf, getNumDecimalSep() );

            bool bNullEnd = true;
            while ( i < nNumLen )
            {
                if ( *pNumBuf != '0' )
                    bNullEnd = false;

                *pBuf = *pNumBuf;
                pBuf++;
                pNumBuf++;
                i++;
            }

            // strip .0 in decimals?
            if ( bNullEnd && !bTrailingZeros )
                pBuf -= nDecimals+1;
        }
    }

    return pBuf;
}

// --- simple date and time formatting --------------------------------

OUString LocaleDataWrapper::getDate( const Date& rDate ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
//!TODO: leading zeros et al
    sal_Unicode aBuf[128];
    sal_Unicode* pBuf = aBuf;
    sal_uInt16  nDay    = rDate.GetDay();
    sal_uInt16  nMonth  = rDate.GetMonth();
    sal_Int16   nYear   = rDate.GetYear();
    sal_uInt16  nYearLen;

    if ( true /* IsDateCentury() */ )
        nYearLen = 4;
    else
    {
        nYearLen = 2;
        nYear %= 100;
    }

    switch ( getDateFormat() )
    {
        case DMY :
            pBuf = ImplAdd2UNum( pBuf, nDay, true /* IsDateDayLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nMonth, true /* IsDateMonthLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAddNum( pBuf, nYear, nYearLen );
        break;
        case MDY :
            pBuf = ImplAdd2UNum( pBuf, nMonth, true /* IsDateMonthLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nDay, true /* IsDateDayLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAddNum( pBuf, nYear, nYearLen );
        break;
        default:
            pBuf = ImplAddNum( pBuf, nYear, nYearLen );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nMonth, true /* IsDateMonthLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nDay, true /* IsDateDayLeadingZero() */ );
    }

    return OUString(aBuf, pBuf-aBuf);
}

OUString LocaleDataWrapper::getTime( const tools::Time& rTime, bool bSec, bool b100Sec ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
//!TODO: leading zeros et al
    sal_Unicode aBuf[128];
    sal_Unicode* pBuf = aBuf;
    sal_uInt16  nHour = rTime.GetHour();

    nHour %= 24;

    pBuf = ImplAdd2UNum( pBuf, nHour, true /* IsTimeLeadingZero() */ );
    pBuf = ImplAddString( pBuf, getTimeSep() );
    pBuf = ImplAdd2UNum( pBuf, rTime.GetMin(), true );
    if ( bSec )
    {
        pBuf = ImplAddString( pBuf, getTimeSep() );
        pBuf = ImplAdd2UNum( pBuf, rTime.GetSec(), true );

        if ( b100Sec )
        {
            pBuf = ImplAddString( pBuf, getTime100SecSep() );
            pBuf = ImplAdd9UNum( pBuf, rTime.GetNanoSec(), true );
        }
    }

    return OUString(aBuf, pBuf - aBuf);
}

OUString LocaleDataWrapper::getLongDate( const Date& rDate, CalendarWrapper& rCal,
        bool bTwoDigitYear ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    using namespace css::i18n;
    sal_Unicode     aBuf[20];
    sal_Unicode*    pBuf;
    OUString aStr;
    sal_Int16 nVal;
    rCal.setGregorianDateTime( rDate );
    // day of week
    nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_WEEK );
    aStr += rCal.getDisplayName( CalendarDisplayIndex::DAY, nVal, 1 );
    aStr += getLongDateDayOfWeekSep();
    // day of month
    nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_MONTH );
    pBuf = ImplAdd2UNum( aBuf, nVal, false/*bDayOfMonthWithLeadingZero*/ );
    OUString aDay(aBuf, pBuf-aBuf);
    // month of year
    nVal = rCal.getValue( CalendarFieldIndex::MONTH );
    OUString aMonth( rCal.getDisplayName( CalendarDisplayIndex::MONTH, nVal, 1 ) );
    // year
    nVal = rCal.getValue( CalendarFieldIndex::YEAR );
    if ( bTwoDigitYear )
        pBuf = ImplAddUNum( aBuf, nVal % 100, 2 );
    else
        pBuf = ImplAddUNum( aBuf, nVal );
    OUString aYear(aBuf, pBuf-aBuf);
    // concatenate
    switch ( getLongDateFormat() )
    {
        case DMY :
            aStr += aDay + getLongDateDaySep() + aMonth + getLongDateMonthSep() + aYear;
        break;
        case MDY :
            aStr += aMonth + getLongDateMonthSep() + aDay + getLongDateDaySep() + aYear;
        break;
        default:    // YMD
            aStr += aYear + getLongDateYearSep() +  aMonth +  getLongDateMonthSep() + aDay;
    }
    return aStr;
}

OUString LocaleDataWrapper::getDuration( const tools::Time& rTime, bool bSec, bool b100Sec ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    sal_Unicode aBuf[128];
    sal_Unicode* pBuf = aBuf;

    if ( rTime < tools::Time( 0 ) )
        pBuf = ImplAddString( pBuf, ' ' );

    if ( true /* IsTimeLeadingZero() */ )
        pBuf = ImplAddUNum( pBuf, rTime.GetHour(), 2 );
    else
        pBuf = ImplAddUNum( pBuf, rTime.GetHour() );
    pBuf = ImplAddString( pBuf, getTimeSep() );
    pBuf = ImplAdd2UNum( pBuf, rTime.GetMin(), true );
    if ( bSec )
    {
        pBuf = ImplAddString( pBuf, getTimeSep() );
        pBuf = ImplAdd2UNum( pBuf, rTime.GetSec(), true );

        if ( b100Sec )
        {
            pBuf = ImplAddString( pBuf, getTime100SecSep() );
            pBuf = ImplAdd9UNum( pBuf, rTime.GetNanoSec(), true );
        }
    }

    return OUString(aBuf, pBuf-aBuf);
}

// --- simple number formatting ---------------------------------------

inline size_t ImplGetNumberStringLengthGuess( const LocaleDataWrapper& rLoc, sal_uInt16 nDecimals )
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
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    sal_Unicode aBuf[128];      // big enough for 64-bit long and crazy grouping
    // check if digits and separators will fit into fixed buffer or allocate
    size_t nGuess = ImplGetNumberStringLengthGuess( *this, nDecimals );
    sal_Unicode* const pBuffer = (nGuess < 118 ? aBuf :
        new sal_Unicode[nGuess + 16]);

    sal_Unicode* pBuf = ImplAddFormatNum( pBuffer, nNumber, nDecimals,
        bUseThousandSep, bTrailingZeros );
    OUString aStr(pBuffer, pBuf-pBuffer);

    if ( pBuffer != aBuf )
        delete [] pBuffer;
    return aStr;
}

OUString LocaleDataWrapper::getCurr( sal_Int64 nNumber, sal_uInt16 nDecimals,
        const OUString& rCurrencySymbol, bool bUseThousandSep ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    sal_Unicode aBuf[192];
    sal_Unicode aNumBuf[128];    // big enough for 64-bit long and crazy grouping
    sal_Unicode cZeroChar = getCurrZeroChar();

    // check if digits and separators will fit into fixed buffer or allocate
    size_t nGuess = ImplGetNumberStringLengthGuess( *this, nDecimals );
    sal_Unicode* const pNumBuffer = (nGuess < 118 ? aNumBuf :
        new sal_Unicode[nGuess + 16]);

    sal_Unicode* const pBuffer =
        ((size_t(rCurrencySymbol.getLength()) + nGuess + 20) < SAL_N_ELEMENTS(aBuf) ? aBuf :
        new sal_Unicode[ rCurrencySymbol.getLength() + nGuess + 20 ]);
    sal_Unicode* pBuf = pBuffer;

    bool bNeg;
    if ( nNumber < 0 )
    {
        bNeg = true;
        nNumber *= -1;
    }
    else
        bNeg = false;

    // convert number
    sal_Unicode* pEndNumBuf = ImplAddFormatNum( pNumBuffer, nNumber, nDecimals,
        bUseThousandSep, true );
    sal_Int32 nNumLen = (sal_Int32)(sal_uLong)(pEndNumBuf-pNumBuffer);

    // replace zeros with zero character
    if ( (cZeroChar != '0') && nDecimals /* && IsNumTrailingZeros() */ )
    {
        sal_Unicode* pTempBuf;
        sal_uInt16  i;
        bool    bZero = true;

        pTempBuf = pNumBuffer+nNumLen-nDecimals;
        i = 0;
        do
        {
            if ( *pTempBuf != '0' )
            {
                bZero = false;
                break;
            }

            pTempBuf++;
            i++;
        }
        while ( i < nDecimals );

        if ( bZero )
        {
            pTempBuf = pNumBuffer+nNumLen-nDecimals;
            i = 0;
            do
            {
                *pTempBuf = cZeroChar;
                pTempBuf++;
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
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                break;
            case 1:
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                break;
            case 2:
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, ' ' );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                break;
            case 3:
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, ' ' );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                break;
        }
    }
    else
    {
        switch( getCurrNegativeFormat() )
        {
            case 0:
                pBuf = ImplAddString( pBuf, '(' );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, ')' );
                break;
            case 1:
                pBuf = ImplAddString( pBuf, '-' );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                break;
            case 2:
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, '-' );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                break;
            case 3:
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, '-' );
                break;
            case 4:
                pBuf = ImplAddString( pBuf, '(' );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, ')' );
                break;
            case 5:
                pBuf = ImplAddString( pBuf, '-' );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                break;
            case 6:
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, '-' );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                break;
            case 7:
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, '-' );
                break;
            case 8:
                pBuf = ImplAddString( pBuf, '-' );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, ' ' );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                break;
            case 9:
                pBuf = ImplAddString( pBuf, '-' );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, ' ' );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                break;
            case 10:
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, ' ' );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, '-' );
                break;
            case 11:
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, ' ' );
                pBuf = ImplAddString( pBuf, '-' );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                break;
            case 12:
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, ' ' );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, '-' );
                break;
            case 13:
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, '-' );
                pBuf = ImplAddString( pBuf, ' ' );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                break;
            case 14:
                pBuf = ImplAddString( pBuf, '(' );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, ' ' );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, ')' );
                break;
            case 15:
                pBuf = ImplAddString( pBuf, '(' );
                pBuf = ImplAddString( pBuf, pNumBuffer, nNumLen );
                pBuf = ImplAddString( pBuf, ' ' );
                pBuf = ImplAddString( pBuf, rCurrencySymbol );
                pBuf = ImplAddString( pBuf, ')' );
                break;
        }
    }

    OUString aNumber(pBuffer, pBuf-pBuffer);

    if ( pBuffer != aBuf )
        delete [] pBuffer;
    if ( pNumBuffer != aNumBuf )
        delete [] pNumBuffer;

    return aNumber;
}

// --- mixed ----------------------------------------------------------

LanguageTag LocaleDataWrapper::getLoadedLanguageTag() const
{
    LanguageCountryInfo aLCInfo = getLanguageCountryInfo();
    return LanguageTag( lang::Locale( aLCInfo.Language, aLCInfo.Country, aLCInfo.Variant ));
}

OUString LocaleDataWrapper::appendLocaleInfo(const OUString& rDebugMsg) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
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
void LocaleDataWrapper::outputCheckMessage( const OUString& rMsg )
{
    outputCheckMessage(OUStringToOString(rMsg, RTL_TEXTENCODING_UTF8).getStr());
}

// static
void LocaleDataWrapper::outputCheckMessage( const char* pStr )
{
    fprintf( stderr, "\n%s\n", pStr);
    fflush( stderr);
    OSL_TRACE("%s", pStr);
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getAllCalendars: Exception caught " << e.Message );
    }
    return css::uno::Sequence< css::i18n::Calendar2 >(0);
}

// --- XLocaleData4 ----------------------------------------------------------

css::uno::Sequence< OUString > LocaleDataWrapper::getDateAcceptancePatterns() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );

    if (aDateAcceptancePatterns.getLength())
        return aDateAcceptancePatterns;

    aGuard.changeReadToWrite();

    try
    {
        const_cast<LocaleDataWrapper*>(this)->aDateAcceptancePatterns =
            xLD->getDateAcceptancePatterns( getMyLocale() );
        return aDateAcceptancePatterns;
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getDateAcceptancePatterns: Exception caught " << e.Message );
    }
    return css::uno::Sequence< OUString >(0);
}

// --- Override layer --------------------------------------------------------

void LocaleDataWrapper::setDateAcceptancePatterns(
        const css::uno::Sequence< OUString > & rPatterns )
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nWrite );

    if (!aDateAcceptancePatterns.getLength() || !rPatterns.getLength())
    {
        try
        {
            aDateAcceptancePatterns = xLD->getDateAcceptancePatterns( getMyLocale() );
        }
        catch (const Exception& e)
        {
            SAL_WARN( "unotools.i18n", "setDateAcceptancePatterns: Exception caught " << e.Message );
        }
        if (!rPatterns.getLength())
            return;     // just a reset
        if (!aDateAcceptancePatterns.getLength())
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
        OUString* pArray1 = aTmp.getArray();
        const OUString* pArray2 = rPatterns.getConstArray();
        pArray1[0] = aDateAcceptancePatterns[0];
        for (sal_Int32 i=0; i < rPatterns.getLength(); ++i)
            pArray1[i+1] = pArray2[i];
        aDateAcceptancePatterns = aTmp;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
