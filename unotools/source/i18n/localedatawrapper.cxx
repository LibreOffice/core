/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#include <string.h>      // memcpy()
#include <stdio.h>       // fprintf(), stderr

#include <unotools/localedatawrapper.hxx>
#include <unotools/numberformatcodewrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/digitgroupingiterator.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>
#include <i18npool/mslangid.hxx>

#include <comphelper/componentfactory.hxx>
#include <unotools/processfactory.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/KNumberFormatUsage.hpp>
#include <com/sun/star/i18n/KNumberFormatType.hpp>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>

#include <com/sun/star/i18n/NumberFormatIndex.hdl>
#include <rtl/instance.hxx>
#include <sal/macros.h>

#define LOCALEDATA_LIBRARYNAME "i18npool"
#define LOCALEDATA_SERVICENAME "com.sun.star.i18n.LocaleData"

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

sal_uInt8 LocaleDataWrapper::nLocaleDataChecking = 0;

LocaleDataWrapper::LocaleDataWrapper(
            const Reference< lang::XMultiServiceFactory > & xSF,
            const lang::Locale& rLocale
            )
        :
        xSMgr( xSF ),
        bLocaleDataItemValid( sal_False ),
        bReservedWordValid( sal_False )
{
    setLocale( rLocale );
    if ( xSMgr.is() )
    {
        try
        {
            xLD = Reference< XLocaleData2 > ( xSMgr->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LOCALEDATA_SERVICENAME ) ) ),
                uno::UNO_QUERY );
        }
        catch ( Exception& e )
        {
#ifdef DBG_UTIL
            ByteString aMsg( "LocaleDataWrapper ctor: Exception caught\n" );
            aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
            DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
        }
    }
    else
    {   // try to get an instance somehow
        DBG_ERRORFILE( "LocaleDataWrapper: no service manager, trying own" );
        try
        {
            Reference< XInterface > xI = ::comphelper::getComponentInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LLCF_LIBNAME( LOCALEDATA_LIBRARYNAME ) ) ),
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LOCALEDATA_SERVICENAME ) ) );
            if ( xI.is() )
            {
                Any x = xI->queryInterface( ::getCppuType((const Reference< XLocaleData2 >*)0) );
                x >>= xLD;
            }
        }
        catch ( Exception& e )
        {
#ifdef DBG_UTIL
            ByteString aMsg( "getComponentInstance: Exception caught\n" );
            aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
            DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
        }
    }
}


LocaleDataWrapper::~LocaleDataWrapper()
{
}


void LocaleDataWrapper::setLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nCriticalChange );
    aLocale = rLocale;
    invalidateData();
}


const ::com::sun::star::lang::Locale& LocaleDataWrapper::getLocale() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    return aLocale;
}


void LocaleDataWrapper::invalidateData()
{
    aCurrSymbol.Erase();
    aCurrBankSymbol.Erase();
    nDateFormat = nLongDateFormat = nDateFormatInvalid;
    nCurrPositiveFormat = nCurrNegativeFormat = nCurrDigits = nCurrFormatInvalid;
    if ( bLocaleDataItemValid )
    {
        for ( sal_Int32 j=0; j<LocaleItem::COUNT; j++ )
        {
            aLocaleItem[j].Erase();
        }
        bLocaleDataItemValid = sal_False;
    }
    if ( bReservedWordValid )
    {
        for ( sal_Int16 j=0; j<reservedWords::COUNT; j++ )
        {
            aReservedWord[j].Erase();
        }
        bReservedWordValid = sal_False;
    }
    xDefaultCalendar.reset();
    if (aGrouping.getLength())
        aGrouping[0] = 0;
    // dummies
    cCurrZeroChar = '0';
}


::com::sun::star::i18n::LanguageCountryInfo LocaleDataWrapper::getLanguageCountryInfo() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getLanguageCountryInfo( getLocale() );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getLanguageCountryInfo: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::i18n::LanguageCountryInfo();
}


::com::sun::star::i18n::LocaleDataItem LocaleDataWrapper::getLocaleItem() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getLocaleItem( getLocale() );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getLocaleItem: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::i18n::LocaleDataItem();
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar > LocaleDataWrapper::getAllCalendars() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getAllCalendars( getLocale() );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getAllCalendars: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency2 > LocaleDataWrapper::getAllCurrencies() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getAllCurrencies2( getLocale() );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getAllCurrencies: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency2 >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > LocaleDataWrapper::getAllFormats() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getAllFormats( getLocale() );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getAllFormats: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Implementation > LocaleDataWrapper::getCollatorImplementations() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getCollatorImplementations( getLocale() );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getCollatorImplementations: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Implementation >(0);
}


::com::sun::star::uno::Sequence< ::rtl::OUString > LocaleDataWrapper::getTransliterations() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getTransliterations( getLocale() );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getTransliterations: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::uno::Sequence< ::rtl::OUString >(0);
}


::com::sun::star::i18n::ForbiddenCharacters LocaleDataWrapper::getForbiddenCharacters() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getForbiddenCharacters( getLocale() );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getForbiddenCharacters: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::i18n::ForbiddenCharacters();
}


::com::sun::star::uno::Sequence< ::rtl::OUString > LocaleDataWrapper::getReservedWord() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getReservedWord( getLocale() );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getReservedWord: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::uno::Sequence< ::rtl::OUString >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > LocaleDataWrapper::getAllInstalledLocaleNames() const
{
    uno::Sequence< lang::Locale > &rInstalledLocales = InstalledLocales::get();

    if ( rInstalledLocales.getLength() )
        return rInstalledLocales;

    try
    {
        if ( xLD.is() )
            rInstalledLocales = xLD->getAllInstalledLocaleNames();
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getAllInstalledLocaleNames: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return rInstalledLocales;
}


// --- Impl and helpers ----------------------------------------------------

// static
::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > LocaleDataWrapper::getInstalledLocaleNames()
{
    const uno::Sequence< lang::Locale > &rInstalledLocales =
        InstalledLocales::get();

    if ( !rInstalledLocales.getLength() )
    {
        LocaleDataWrapper aLDW( ::comphelper::getProcessServiceFactory(), lang::Locale() );
        aLDW.getAllInstalledLocaleNames();
    }
    return rInstalledLocales;
}

// static
::com::sun::star::uno::Sequence< sal_uInt16 > LocaleDataWrapper::getInstalledLanguageTypes()
{
    uno::Sequence< sal_uInt16 > &rInstalledLanguageTypes =
        InstalledLanguageTypes::get();

    if ( rInstalledLanguageTypes.getLength() )
        return rInstalledLanguageTypes;

    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > xLoc =
        getInstalledLocaleNames();
    sal_Int32 nCount = xLoc.getLength();
    ::com::sun::star::uno::Sequence< sal_uInt16 > xLang( nCount );
    sal_Int32 nLanguages = 0;
    for ( sal_Int32 i=0; i<nCount; i++ )
    {
        String aDebugLocale;
        if (areChecksEnabled())
        {
            aDebugLocale = xLoc[i].Language;
            if ( xLoc[i].Country.getLength() )
            {
                aDebugLocale += '_';
                aDebugLocale += String( xLoc[i].Country);
                if ( xLoc[i].Variant.getLength() )
                {
                    aDebugLocale += '_';
                    aDebugLocale += String( xLoc[i].Variant);
                }
            }
        }

        if ( xLoc[i].Variant.getLength() )
        {
            if (areChecksEnabled())
            {
                String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                            "LocaleDataWrapper::getInstalledLanguageTypes: Variants not supported, locale\n"));
                aMsg += aDebugLocale;
                outputCheckMessage( aMsg );
            }
            continue;
        }
        LanguageType eLang = MsLangId::convertLocaleToLanguage( xLoc[i] );

        // In checks, exclude known problems because no MS-LCID defined.
        if (areChecksEnabled() && eLang == LANGUAGE_DONTKNOW
//              && !aDebugLocale.EqualsAscii( "br_AE" ) // ?!? Breton in United Arabic Emirates
            )
        {
            String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                        "ConvertIsoNamesToLanguage: unknown MS-LCID for locale\n"));
            aMsg += aDebugLocale;
            outputCheckMessage( aMsg );
        }

        switch ( eLang )
        {
            case LANGUAGE_NORWEGIAN :       // no_NO, not Bokmal (nb_NO), not Nynorsk (nn_NO)
                eLang = LANGUAGE_DONTKNOW;  // don't offer "Unknown" language
                break;
        }
        if ( eLang != LANGUAGE_DONTKNOW )
        {
            rtl::OUString aLanguage, aCountry;
            MsLangId::convertLanguageToIsoNames( eLang, aLanguage, aCountry );
            if ( xLoc[i].Language != aLanguage ||
                    xLoc[i].Country != aCountry )
            {
                // In checks, exclude known problems because no MS-LCID defined
                // and default for Language found.
                if ( areChecksEnabled()
                        && !aDebugLocale.EqualsAscii( "ar_SD" ) // Sudan/ar
                        && !aDebugLocale.EqualsAscii( "en_CB" ) // Carribean is not a country
//                      && !aDebugLocale.EqualsAscii( "en_BG" ) // ?!? Bulgaria/en
//                      && !aDebugLocale.EqualsAscii( "es_BR" ) // ?!? Brazil/es
                    )
                {
                    String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                                "ConvertIsoNamesToLanguage/ConvertLanguageToIsoNames: ambiguous locale (MS-LCID?)\n"));
                    aMsg += aDebugLocale;
                    aMsg.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "  ->  0x" ) );
                    aMsg += String::CreateFromInt32( eLang, 16 );
                    aMsg.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "  ->  " ) );
                    aMsg += String( aLanguage);
                    if ( aCountry.getLength() )
                    {
                        aMsg += '_';
                        aMsg += String( aCountry);
                    }
                    outputCheckMessage( aMsg );
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

const String& LocaleDataWrapper::getOneLocaleItem( sal_Int16 nItem ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nItem >= LocaleItem::COUNT )
    {
        DBG_ERRORFILE( "getOneLocaleItem: bounds" );
        return aLocaleItem[0];
    }
    if ( aLocaleItem[nItem].Len() == 0 )
    {   // no cached content
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getOneLocaleItemImpl( nItem );
    }
    return aLocaleItem[nItem];
}


void LocaleDataWrapper::getOneLocaleItemImpl( sal_Int16 nItem )
{
    if ( !bLocaleDataItemValid )
    {
        aLocaleDataItem = getLocaleItem();
        bLocaleDataItemValid = sal_True;
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
            DBG_ERRORFILE( "getOneLocaleItemImpl: which one?" );
    }
}


void LocaleDataWrapper::getOneReservedWordImpl( sal_Int16 nWord )
{
    if ( !bReservedWordValid )
    {
        aReservedWordSeq = getReservedWord();
        bReservedWordValid = sal_True;
    }
    DBG_ASSERT( nWord < aReservedWordSeq.getLength(), "getOneReservedWordImpl: which one?" );
    if ( nWord < aReservedWordSeq.getLength() )
        aReservedWord[nWord] = aReservedWordSeq[nWord];
}


const String& LocaleDataWrapper::getOneReservedWord( sal_Int16 nWord ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nWord < 0 || nWord >= reservedWords::COUNT )
    {
        DBG_ERRORFILE( "getOneReservedWord: bounds" );
        nWord = reservedWords::FALSE_WORD;
    }
    if ( aReservedWord[nWord].Len() == 0 )
    {   // no cached content
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getOneReservedWordImpl( nWord );
    }
    return aReservedWord[nWord];
}


MeasurementSystem LocaleDataWrapper::mapMeasurementStringToEnum( const String& rMS ) const
{
//! TODO: could be cached too
    if ( rMS.EqualsIgnoreCaseAscii( "metric" ) )
        return MEASURE_METRIC;
//! TODO: other measurement systems? => extend enum MeasurementSystem
    return MEASURE_US;
}


void LocaleDataWrapper::getDefaultCalendarImpl()
{
    if (!xDefaultCalendar)
    {
        Sequence< Calendar > xCals = getAllCalendars();
        sal_Int32 nCount = xCals.getLength();
        sal_Int32 nDef = 0;
        if (nCount > 1)
        {
            const Calendar* pArr = xCals.getArray();
            for (sal_Int32 i=0; i<nCount; ++i)
            {
                if (pArr[i].Default)
                {
                    nDef = i;
                    break;
                }
            }
        }
        xDefaultCalendar.reset( new Calendar( xCals[nDef]));
    }
}


const ::boost::shared_ptr< ::com::sun::star::i18n::Calendar > LocaleDataWrapper::getDefaultCalendar() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if (!xDefaultCalendar)
    {   // no cached content
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getDefaultCalendarImpl();
    }
    return xDefaultCalendar;
}


const ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > LocaleDataWrapper::getDefaultCalendarDays() const
{
    return getDefaultCalendar()->Days;
}


const ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > LocaleDataWrapper::getDefaultCalendarMonths() const
{
    return getDefaultCalendar()->Months;
}


// --- currencies -----------------------------------------------------

const String& LocaleDataWrapper::getCurrSymbol() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( !aCurrSymbol.Len() )
    {
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getCurrSymbolsImpl();
    }
    return aCurrSymbol;
}


const String& LocaleDataWrapper::getCurrBankSymbol() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( !aCurrBankSymbol.Len() )
    {
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getCurrSymbolsImpl();
    }
    return aCurrBankSymbol;
}


sal_uInt16 LocaleDataWrapper::getCurrPositiveFormat() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nCurrPositiveFormat == nCurrFormatInvalid )
    {
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getCurrFormatsImpl();
    }
    return nCurrPositiveFormat;
}


sal_uInt16 LocaleDataWrapper::getCurrNegativeFormat() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nCurrNegativeFormat == nCurrFormatInvalid )
    {
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getCurrFormatsImpl();
    }
    return nCurrNegativeFormat;
}


sal_uInt16 LocaleDataWrapper::getCurrDigits() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nCurrDigits == nCurrFormatInvalid )
    {
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getCurrSymbolsImpl();
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
            String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                        "LocaleDataWrapper::getCurrSymbolsImpl: no default currency"));
            outputCheckMessage( appendLocaleInfo( aMsg ) );
        }
        nElem = 0;
        if ( nElem >= nCnt )
        {
            if (areChecksEnabled())
                outputCheckMessage( String( RTL_CONSTASCII_USTRINGPARAM(
                                "LocaleDataWrapper::getCurrSymbolsImpl: no currency at all, using ShellsAndPebbles")));
            aCurrSymbol.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "ShellsAndPebbles" ) );
            aCurrBankSymbol = aCurrSymbol;
            nCurrPositiveFormat = nCurrNegativeFormat = nCurrFormatDefault;
            nCurrDigits = 2;
            return ;
        }
    }
    aCurrSymbol = pCurrArr[nElem].Symbol;
    aCurrBankSymbol = pCurrArr[nElem].BankSymbol;
    nCurrDigits = pCurrArr[nElem].DecimalPlaces;
}


void LocaleDataWrapper::scanCurrFormatImpl( const String& rCode,
        xub_StrLen nStart, xub_StrLen& nSign, xub_StrLen& nPar,
        xub_StrLen& nNum, xub_StrLen& nBlank, xub_StrLen& nSym )
{
    nSign = nPar = nNum = nBlank = nSym = STRING_NOTFOUND;
    const sal_Unicode* const pStr = rCode.GetBuffer();
    const sal_Unicode* const pStop = pStr + rCode.Len();
    const sal_Unicode* p = pStr + nStart;
    int nInSection = 0;
    sal_Bool bQuote = sal_False;
    while ( p < pStop )
    {
        if ( bQuote )
        {
            if ( *p == '"' && *(p-1) != '\\' )
                bQuote = sal_False;
        }
        else
        {
            switch ( *p )
            {
                case '"' :
                    if ( pStr == p || *(p-1) != '\\' )
                        bQuote = sal_True;
                break;
                case '-' :
                    if ( !nInSection && nSign == STRING_NOTFOUND )
                        nSign = (xub_StrLen)(p - pStr);
                break;
                case '(' :
                    if ( !nInSection && nPar == STRING_NOTFOUND )
                        nPar = (xub_StrLen)(p - pStr);
                break;
                case '0' :
                case '#' :
                    if ( !nInSection && nNum == STRING_NOTFOUND )
                        nNum = (xub_StrLen)(p - pStr);
                break;
                case '[' :
                    nInSection++;
                break;
                case ']' :
                    if ( nInSection )
                    {
                        nInSection--;
                        if ( !nInSection && nBlank == STRING_NOTFOUND
                          && nSym != STRING_NOTFOUND && p < pStop-1 && *(p+1) == ' ' )
                            nBlank = (xub_StrLen)(p - pStr + 1);
                    }
                break;
                case '$' :
                    if ( nSym == STRING_NOTFOUND && nInSection && *(p-1) == '[' )
                    {
                        nSym = (xub_StrLen)(p - pStr + 1);
                        if ( nNum != STRING_NOTFOUND && *(p-2) == ' ' )
                            nBlank = (xub_StrLen)(p - pStr - 2);
                    }
                break;
                case ';' :
                    if ( !nInSection )
                        p = pStop;
                break;
                default:
                    if ( !nInSection && nSym == STRING_NOTFOUND && rCode.Equals( aCurrSymbol, (xub_StrLen)(p-pStr), aCurrSymbol.Len() ) )
                    {   // currency symbol not surrounded by [$...]
                        nSym = (xub_StrLen)(p - pStr);
                        if ( nBlank == STRING_NOTFOUND && pStr < p && *(p-1) == ' ' )
                            nBlank = (xub_StrLen)(p - pStr - 1);
                        p += aCurrSymbol.Len() - 1;
                        if ( nBlank == STRING_NOTFOUND && p < pStop-2 && *(p+2) == ' ' )
                            nBlank = (xub_StrLen)(p - pStr + 2);
                    }
            }
        }
        p++;
    }
}


void LocaleDataWrapper::getCurrFormatsImpl()
{
    NumberFormatCodeWrapper aNumberFormatCode( xSMgr, getLocale() );
    uno::Sequence< NumberFormatCode > aFormatSeq
        = aNumberFormatCode.getAllFormatCode( KNumberFormatUsage::CURRENCY );
    sal_Int32 nCnt = aFormatSeq.getLength();
    if ( !nCnt )
    {   // bad luck
        if (areChecksEnabled())
        {
            String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                        "LocaleDataWrapper::getCurrFormatsImpl: no currency formats"));
            outputCheckMessage( appendLocaleInfo( aMsg ) );
        }
        nCurrPositiveFormat = nCurrNegativeFormat = nCurrFormatDefault;
        return ;
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

    xub_StrLen nSign, nPar, nNum, nBlank, nSym;

    // positive format
    nElem = (nDef >= 0 ? nDef : (nNeg >= 0 ? nNeg : 0));
    scanCurrFormatImpl( pFormatArr[nElem].Code, 0, nSign, nPar, nNum, nBlank, nSym );
    if (areChecksEnabled() && (nNum == STRING_NOTFOUND || nSym == STRING_NOTFOUND))
    {
        String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                    "LocaleDataWrapper::getCurrFormatsImpl: CurrPositiveFormat?"));
        outputCheckMessage( appendLocaleInfo( aMsg ) );
    }
    if ( nBlank == STRING_NOTFOUND )
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
        const ::rtl::OUString& rCode = pFormatArr[nNeg].Code;
        xub_StrLen nDelim = (xub_StrLen)rCode.indexOf( ';' );
        scanCurrFormatImpl( rCode, nDelim+1, nSign, nPar, nNum, nBlank, nSym );
        if (areChecksEnabled() && (nNum == STRING_NOTFOUND ||
                    nSym == STRING_NOTFOUND || (nPar == STRING_NOTFOUND &&
                        nSign == STRING_NOTFOUND)))
        {
            String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                        "LocaleDataWrapper::getCurrFormatsImpl: CurrNegativeFormat?"));
            outputCheckMessage( appendLocaleInfo( aMsg ) );
        }
        if ( nBlank == STRING_NOTFOUND )
        {
            if ( nSym < nNum )
            {
                if ( nPar < nSym )
                    nCurrNegativeFormat = 0;    // ($1)
                else if ( nSign < nSym )
                    nCurrNegativeFormat = 1;    // -$1
                else if ( nNum < nSign )
                    nCurrNegativeFormat = 3;    // $1-
                else
                    nCurrNegativeFormat = 2;    // $-1
            }
            else
            {
                if ( nPar < nNum )
                    nCurrNegativeFormat = 4;    // (1$)
                else if ( nSign < nNum )
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
                if ( nPar < nSym )
                    nCurrNegativeFormat = 14;   // ($ 1)
                else if ( nSign < nSym )
                    nCurrNegativeFormat = 9;    // -$ 1
                else if ( nNum < nSign )
                    nCurrNegativeFormat = 12;   // $ 1-
                else
                    nCurrNegativeFormat = 11;   // $ -1
            }
            else
            {
                if ( nPar < nNum )
                    nCurrNegativeFormat = 15;   // (1 $)
                else if ( nSign < nNum )
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
        ((LocaleDataWrapper*)this)->getDateFormatsImpl();
    }
    return (DateFormat) nDateFormat;
}


DateFormat LocaleDataWrapper::getLongDateFormat() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nLongDateFormat == nDateFormatInvalid )
    {
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getDateFormatsImpl();
    }
    return (DateFormat) nLongDateFormat;
}


DateFormat LocaleDataWrapper::scanDateFormatImpl( const String& rCode )
{
    // Only some european versions were translated, the ones with different
    // keyword combinations are:
    // English DMY, German TMJ, Spanish DMA, French JMA, Italian GMA,
    // Dutch DMJ, Finnish PKV

    // default is English keywords for every other language
    xub_StrLen nDay = rCode.Search( 'D' );
    xub_StrLen nMonth = rCode.Search( 'M' );
    xub_StrLen nYear = rCode.Search( 'Y' );
    if ( nDay == STRING_NOTFOUND || nMonth == STRING_NOTFOUND || nYear == STRING_NOTFOUND )
    {   // This algorithm assumes that all three parts (DMY) are present
        if ( nMonth == STRING_NOTFOUND )
        {   // only Finnish has something else than 'M' for month
            nMonth = rCode.Search( 'K' );
            if ( nMonth != STRING_NOTFOUND )
            {
                nDay = rCode.Search( 'P' );
                nYear = rCode.Search( 'V' );
            }
        }
        else if ( nDay == STRING_NOTFOUND )
        {   // We have a month 'M' if we reach this branch.
            // Possible languages containing 'M' but no 'D':
            // German, French, Italian
            nDay = rCode.Search( 'T' );         // German
            if ( nDay != STRING_NOTFOUND )
                nYear = rCode.Search( 'J' );
            else
            {
                nYear = rCode.Search( 'A' );    // French, Italian
                if ( nYear != STRING_NOTFOUND )
                {
                    nDay = rCode.Search( 'J' ); // French
                    if ( nDay == STRING_NOTFOUND )
                        nDay = rCode.Search( 'G' ); // Italian
                }
            }
        }
        else
        {   // We have a month 'M' and a day 'D'.
            // Possible languages containing 'D' and 'M' but not 'Y':
            // Spanish, Dutch
            nYear = rCode.Search( 'A' );        // Spanish
            if ( nYear == STRING_NOTFOUND )
                nYear = rCode.Search( 'J' );    // Dutch
        }
        if ( nDay == STRING_NOTFOUND || nMonth == STRING_NOTFOUND || nYear == STRING_NOTFOUND )
        {
            if (areChecksEnabled())
            {
                String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                            "LocaleDataWrapper::scanDateFormat: not all DMY present"));
                outputCheckMessage( appendLocaleInfo( aMsg ) );
            }
            if ( nDay == STRING_NOTFOUND )
                nDay = rCode.Len();
            if ( nMonth == STRING_NOTFOUND )
                nMonth = rCode.Len();
            if ( nYear == STRING_NOTFOUND )
                nYear = rCode.Len();
        }
    }
    // compare with <= because each position may equal rCode.Len()
    if ( nDay <= nMonth && nMonth <= nYear )
        return DMY;     // also if every position equals rCode.Len()
    else if ( nMonth <= nDay && nDay <= nYear )
        return MDY;
    else if ( nYear <= nMonth && nMonth <= nDay )
        return YMD;
    else
    {
        if (areChecksEnabled())
        {
            String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                        "LocaleDataWrapper::scanDateFormat: no magic applyable"));
            outputCheckMessage( appendLocaleInfo( aMsg ) );
        }
        return DMY;
    }
}


void LocaleDataWrapper::getDateFormatsImpl()
{
    NumberFormatCodeWrapper aNumberFormatCode( xSMgr, getLocale() );
    uno::Sequence< NumberFormatCode > aFormatSeq
        = aNumberFormatCode.getAllFormatCode( KNumberFormatUsage::DATE );
    sal_Int32 nCnt = aFormatSeq.getLength();
    if ( !nCnt )
    {   // bad luck
        if (areChecksEnabled())
        {
            String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                        "LocaleDataWrapper::getDateFormatsImpl: no date formats"));
            outputCheckMessage( appendLocaleInfo( aMsg ) );
        }
        nDateFormat = nLongDateFormat = DMY;
        return ;
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
            String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                        "LocaleDataWrapper::getDateFormatsImpl: no edit"));
            outputCheckMessage( appendLocaleInfo( aMsg ) );
        }
        if ( nDef == -1 )
        {
            if (areChecksEnabled())
            {
                String aMsg( RTL_CONSTASCII_USTRINGPARAM(
                            "LocaleDataWrapper::getDateFormatsImpl: no default"));
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
        if (aLCInfo.Country.equalsIgnoreAsciiCaseAscii( "IN") ||    // India
                aLCInfo.Country.equalsIgnoreAsciiCaseAscii( "BT"))  // Bhutan
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


const ::com::sun::star::uno::Sequence< sal_Int32 > LocaleDataWrapper::getDigitGrouping() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if (!aGrouping.getLength() || aGrouping[0] == 0)
    {   // no cached content
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getDigitGroupingImpl();
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


static sal_Unicode* ImplAdd2UNum( sal_Unicode* pBuf, sal_uInt16 nNumber, int bLeading )
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


inline sal_Unicode* ImplAddString( sal_Unicode* pBuf, const String& rStr )
{
    if ( rStr.Len() == 1 )
        *pBuf++ = rStr.GetChar(0);
    else if ( rStr.Len() == 0 )
        ;
    else
    {
        memcpy( pBuf, rStr.GetBuffer(), rStr.Len() * sizeof(sal_Unicode) );
        pBuf += rStr.Len();
    }
    return pBuf;
}


inline sal_Unicode* ImplAddString( sal_Unicode* pBuf, sal_Unicode c )
{
    *pBuf = c;
    pBuf++;
    return pBuf;
}


inline sal_Unicode* ImplAddString( sal_Unicode* pBuf, const sal_Unicode* pCopyBuf, xub_StrLen nLen )
{
    memcpy( pBuf, pCopyBuf, nLen * sizeof(sal_Unicode) );
    return pBuf + nLen;
}


sal_Unicode* LocaleDataWrapper::ImplAddFormatNum( sal_Unicode* pBuf,
        sal_Int64 nNumber, sal_uInt16 nDecimals, sal_Bool bUseThousandSep,
        sal_Bool bTrailingZeros ) const
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
        const String& rThoSep = getNumThousandSep();

        // copy number to buffer (excluding decimals)
        sal_uInt16 nNumLen2 = nNumLen-nDecimals;
        uno::Sequence< sal_Bool > aGroupPos;
        if (bUseThousandSep)
            aGroupPos = utl::DigitGroupingIterator::createForwardSequence(
                    nNumLen2, getDigitGrouping());
        for ( ; i < nNumLen2; ++i )
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

            sal_Bool bNullEnd = sal_True;
            while ( i < nNumLen )
            {
                if ( *pNumBuf != '0' )
                    bNullEnd = sal_False;

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

String LocaleDataWrapper::getDate( const Date& rDate ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
//!TODO: leading zeros et al
    sal_Unicode aBuf[128];
    sal_Unicode* pBuf = aBuf;
    sal_uInt16  nDay    = rDate.GetDay();
    sal_uInt16  nMonth  = rDate.GetMonth();
    sal_uInt16  nYear   = rDate.GetYear();
    sal_uInt16  nYearLen;

    if ( sal_True /* IsDateCentury() */ )
        nYearLen = 4;
    else
    {
        nYearLen = 2;
        nYear %= 100;
    }

    switch ( getDateFormat() )
    {
        case DMY :
            pBuf = ImplAdd2UNum( pBuf, nDay, sal_True /* IsDateDayLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nMonth, sal_True /* IsDateMonthLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAddUNum( pBuf, nYear, nYearLen );
        break;
        case MDY :
            pBuf = ImplAdd2UNum( pBuf, nMonth, sal_True /* IsDateMonthLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nDay, sal_True /* IsDateDayLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAddUNum( pBuf, nYear, nYearLen );
        break;
        default:
            pBuf = ImplAddUNum( pBuf, nYear, nYearLen );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nMonth, sal_True /* IsDateMonthLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nDay, sal_True /* IsDateDayLeadingZero() */ );
    }

    return String( aBuf, (xub_StrLen)(sal_uLong)(pBuf-aBuf) );
}


String LocaleDataWrapper::getTime( const Time& rTime, sal_Bool bSec, sal_Bool b100Sec ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
//!TODO: leading zeros et al
    sal_Unicode aBuf[128];
    sal_Unicode* pBuf = aBuf;
    sal_uInt16  nHour = rTime.GetHour();
    sal_Bool bHour12 = sal_False;   //!TODO: AM/PM from default time format code

    if ( bHour12 )
    {
        nHour %= 12;
        // 0:00 -> 12:00
        if ( !nHour )
            nHour = 12;
    }
    else
        nHour %= 24;

    pBuf = ImplAdd2UNum( pBuf, nHour, sal_True /* IsTimeLeadingZero() */ );
    pBuf = ImplAddString( pBuf, getTimeSep() );
    pBuf = ImplAdd2UNum( pBuf, rTime.GetMin(), sal_True );
    if ( bSec )
    {
        pBuf = ImplAddString( pBuf, getTimeSep() );
        pBuf = ImplAdd2UNum( pBuf, rTime.GetSec(), sal_True );

        if ( b100Sec )
        {
            pBuf = ImplAddString( pBuf, getTime100SecSep() );
            pBuf = ImplAdd2UNum( pBuf, rTime.Get100Sec(), sal_True );
        }
    }

    String aStr( aBuf, (xub_StrLen)(sal_uLong)(pBuf-aBuf) );

    if ( bHour12 )
    {
        if ( (rTime.GetHour() % 24) >= 12 )
            aStr += getTimePM();
        else
            aStr += getTimeAM();
    }

    return aStr;
}


String LocaleDataWrapper::getLongDate( const Date& rDate, CalendarWrapper& rCal,
        sal_Int16 nDisplayDayOfWeek, sal_Bool bDayOfMonthWithLeadingZero,
        sal_Int16 nDisplayMonth, sal_Bool bTwoDigitYear ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    using namespace ::com::sun::star::i18n;
    sal_Unicode     aBuf[20];
    sal_Unicode*    pBuf;
    String aStr;
    sal_Int16 nVal;
    rCal.setGregorianDateTime( rDate );
    // day of week
    nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_WEEK );
    aStr += rCal.getDisplayName( CalendarDisplayIndex::DAY, nVal, nDisplayDayOfWeek );
    aStr += getLongDateDayOfWeekSep();
    // day of month
    nVal = rCal.getValue( CalendarFieldIndex::DAY_OF_MONTH );
    pBuf = ImplAdd2UNum( aBuf, nVal, bDayOfMonthWithLeadingZero );
    String aDay( aBuf, (xub_StrLen)(sal_uLong)(pBuf-aBuf) );
    // month of year
    nVal = rCal.getValue( CalendarFieldIndex::MONTH );
    String aMonth( rCal.getDisplayName( CalendarDisplayIndex::MONTH, nVal, nDisplayMonth ) );
    // year
    nVal = rCal.getValue( CalendarFieldIndex::YEAR );
    if ( bTwoDigitYear )
        pBuf = ImplAddUNum( aBuf, nVal % 100, 2 );
    else
        pBuf = ImplAddUNum( aBuf, nVal );
    String aYear( aBuf, (xub_StrLen)(sal_uLong)(pBuf-aBuf) );
    // concatenate
    switch ( getLongDateFormat() )
    {
        case DMY :
            aStr += aDay;
            aStr += getLongDateDaySep();
            aStr += aMonth;
            aStr += getLongDateMonthSep();
            aStr += aYear;
        break;
        case MDY :
            aStr += aMonth;
            aStr += getLongDateMonthSep();
            aStr += aDay;
            aStr += getLongDateDaySep();
            aStr += aYear;
        break;
        default:    // YMD
            aStr += aYear;
            aStr += getLongDateYearSep();
            aStr += aMonth;
            aStr += getLongDateMonthSep();
            aStr += aDay;
    }
    return aStr;
}


String LocaleDataWrapper::getDuration( const Time& rTime, sal_Bool bSec, sal_Bool b100Sec ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    sal_Unicode aBuf[128];
    sal_Unicode* pBuf = aBuf;

    if ( rTime < Time( 0 ) )
        pBuf = ImplAddString( pBuf, ' ' );

    if ( sal_True /* IsTimeLeadingZero() */ )
        pBuf = ImplAddUNum( pBuf, rTime.GetHour(), 2 );
    else
        pBuf = ImplAddUNum( pBuf, rTime.GetHour() );
    pBuf = ImplAddString( pBuf, getTimeSep() );
    pBuf = ImplAdd2UNum( pBuf, rTime.GetMin(), sal_True );
    if ( bSec )
    {
        pBuf = ImplAddString( pBuf, getTimeSep() );
        pBuf = ImplAdd2UNum( pBuf, rTime.GetSec(), sal_True );

        if ( b100Sec )
        {
            pBuf = ImplAddString( pBuf, getTime100SecSep() );
            pBuf = ImplAdd2UNum( pBuf, rTime.Get100Sec(), sal_True );
        }
    }

    return String( aBuf, (xub_StrLen)(sal_uLong)(pBuf-aBuf) );
}


// --- simple number formatting ---------------------------------------

inline size_t ImplGetNumberStringLengthGuess( const LocaleDataWrapper& rLoc, sal_uInt16 nDecimals )
{
    // approximately 3.2 bits per digit
    const size_t nDig = ((sizeof(sal_Int64) * 8) / 3) + 1;
    // digits, separators (pessimized for insane "every digit may be grouped"), leading zero, sign
    size_t nGuess = ((nDecimals < nDig) ?
        (((nDig - nDecimals) * rLoc.getNumThousandSep().Len()) + nDig) :
        nDecimals) + rLoc.getNumDecimalSep().Len() + 3;
    return nGuess;
}


String LocaleDataWrapper::getNum( sal_Int64 nNumber, sal_uInt16 nDecimals,
        sal_Bool bUseThousandSep, sal_Bool bTrailingZeros ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    sal_Unicode aBuf[128];      // big enough for 64-bit long and crazy grouping
    // check if digits and separators will fit into fixed buffer or allocate
    size_t nGuess = ImplGetNumberStringLengthGuess( *this, nDecimals );
    sal_Unicode* const pBuffer = (nGuess < 118 ? aBuf :
        new sal_Unicode[nGuess + 16]);

    sal_Unicode* pBuf = ImplAddFormatNum( pBuffer, nNumber, nDecimals,
        bUseThousandSep, bTrailingZeros );
    String aStr( pBuffer, (xub_StrLen)(sal_uLong)(pBuf-pBuffer) );

    if ( pBuffer != aBuf )
        delete [] pBuffer;
    return aStr;
}


String LocaleDataWrapper::getCurr( sal_Int64 nNumber, sal_uInt16 nDecimals,
        const String& rCurrencySymbol, sal_Bool bUseThousandSep ) const
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
        ((size_t(rCurrencySymbol.Len()) + nGuess + 20) < SAL_N_ELEMENTS(aBuf) ? aBuf :
        new sal_Unicode[ rCurrencySymbol.Len() + nGuess + 20 ]);
    sal_Unicode* pBuf = pBuffer;

    sal_Bool bNeg;
    if ( nNumber < 0 )
    {
        bNeg = sal_True;
        nNumber *= -1;
    }
    else
        bNeg = sal_False;

    // convert number
    sal_Unicode* pEndNumBuf = ImplAddFormatNum( pNumBuffer, nNumber, nDecimals,
        bUseThousandSep, sal_True );
    xub_StrLen nNumLen = (xub_StrLen)(sal_uLong)(pEndNumBuf-pNumBuffer);

    // replace zeros with zero character
    if ( (cZeroChar != '0') && nDecimals /* && IsNumTrailingZeros() */ )
    {
        sal_Unicode* pTempBuf;
        sal_uInt16  i;
        sal_Bool    bZero = sal_True;

        pTempBuf = pNumBuffer+nNumLen-nDecimals;
        i = 0;
        do
        {
            if ( *pTempBuf != '0' )
            {
                bZero = sal_False;
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

    String aNumber( pBuffer, (xub_StrLen)(sal_uLong)(pBuf-pBuffer) );

    if ( pBuffer != aBuf )
        delete [] pBuffer;
    if ( pNumBuffer != aNumBuf )
        delete [] pNumBuffer;

    return aNumber;
}


// --- mixed ----------------------------------------------------------

::com::sun::star::lang::Locale LocaleDataWrapper::getLoadedLocale() const
{
    LanguageCountryInfo aLCInfo = getLanguageCountryInfo();
    return lang::Locale( aLCInfo.Language, aLCInfo.Country, aLCInfo.Variant );
}


String& LocaleDataWrapper::appendLocaleInfo( String& rDebugMsg ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    rDebugMsg += '\n';
    rDebugMsg += String( aLocale.Language);
    rDebugMsg += '_';
    rDebugMsg += String( aLocale.Country);
    rDebugMsg.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " requested\n" ) );
    lang::Locale aLoaded = getLoadedLocale();
    rDebugMsg += String( aLoaded.Language);
    rDebugMsg += '_';
    rDebugMsg += String( aLoaded.Country);
    rDebugMsg.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " loaded" ) );
    return rDebugMsg;
}


// static
void LocaleDataWrapper::outputCheckMessage( const String& rMsg )
{
    outputCheckMessage( ByteString( rMsg, RTL_TEXTENCODING_UTF8).GetBuffer());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
