/*************************************************************************
 *
 *  $RCSfile: localedatawrapper.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:39:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include <string.h>     // memcpy()

#include <unotools/localedatawrapper.hxx>
#include <unotools/numberformatcodewrapper.hxx>
#include <unotools/calendarwrapper.hxx>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#ifndef _COMPHELPER_COMPONENTFACTORY_HXX_
#include <comphelper/componentfactory.hxx>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_KNUMBERFORMATUSAGE_HPP_
#include <com/sun/star/i18n/KNumberFormatUsage.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_KNUMBERFORMATTYPE_HPP_
#include <com/sun/star/i18n/KNumberFormatType.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_CALENDARFIELDINDEX_HPP_
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_CALENDARDISPLAYINDEX_HPP_
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_NUMBERFORMATINDEX_HPP_
#include <com/sun/star/i18n/NumberFormatIndex.hdl>
#endif

#define LOCALEDATA_LIBRARYNAME "i18npool"
#define LOCALEDATA_SERVICENAME "com.sun.star.i18n.LocaleData"

static const int nDateFormatInvalid = -1;
static const USHORT nCurrFormatInvalid = 0xffff;
static const USHORT nCurrFormatDefault = 0;

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

uno::Sequence< lang::Locale > LocaleDataWrapper::xInstalledLocales =
    uno::Sequence< lang::Locale >(0);
uno::Sequence< sal_uInt16 > LocaleDataWrapper::xInstalledLanguageTypes =
    uno::Sequence< sal_uInt16 >(0);


LocaleDataWrapper::LocaleDataWrapper(
            const Reference< lang::XMultiServiceFactory > & xSF,
            const lang::Locale& rLocale
            )
        :
        xSMgr( xSF ),
        bLocaleDataItemValid( FALSE ),
        bReservedWordValid( FALSE )
{
    setLocale( rLocale );
    if ( xSMgr.is() )
    {
        try
        {
            xLD = Reference< XLocaleData > ( xSMgr->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LOCALEDATA_SERVICENAME ) ) ),
                uno::UNO_QUERY );
        }
        catch ( Exception& e )
        {
#ifndef PRODUCT
            ByteString aMsg( "LocaleDataWrapper ctor: Exception caught\n" );
            aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
            DBG_ERRORFILE( aMsg.GetBuffer() );
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
                Any x = xI->queryInterface( ::getCppuType((const Reference< XLocaleData >*)0) );
                x >>= xLD;
            }
        }
        catch ( Exception& e )
        {
#ifndef PRODUCT
            ByteString aMsg( "getComponentInstance: Exception caught\n" );
            aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
            DBG_ERRORFILE( aMsg.GetBuffer() );
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
        bLocaleDataItemValid = FALSE;
    }
    if ( bReservedWordValid )
    {
        for ( sal_Int16 j=0; j<reservedWords::COUNT; j++ )
        {
            aReservedWord[j].Erase();
        }
        bReservedWordValid = FALSE;
    }
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
#ifndef PRODUCT
        ByteString aMsg( "getLanguageCountryInfo: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
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
#ifndef PRODUCT
        ByteString aMsg( "getLocaleItem: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
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
#ifndef PRODUCT
        ByteString aMsg( "getAllCalendars: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency > LocaleDataWrapper::getAllCurrencies() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getAllCurrencies( getLocale() );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getAllCurrencies: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency >(0);
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
#ifndef PRODUCT
        ByteString aMsg( "getAllFormats: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
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
#ifndef PRODUCT
        ByteString aMsg( "getCollatorImplementations: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
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
#ifndef PRODUCT
        ByteString aMsg( "getTransliterations: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
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
#ifndef PRODUCT
        ByteString aMsg( "getForbiddenCharacters: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
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
#ifndef PRODUCT
        ByteString aMsg( "getReservedWord: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return ::com::sun::star::uno::Sequence< ::rtl::OUString >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > LocaleDataWrapper::getAllInstalledLocaleNames() const
{
    if ( xInstalledLocales.getLength() )
        return xInstalledLocales;

    try
    {
        if ( xLD.is() )
            xInstalledLocales = xLD->getAllInstalledLocaleNames();
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getAllInstalledLocaleNames: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return xInstalledLocales;
}


// --- Impl and helpers ----------------------------------------------------

// static
::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > LocaleDataWrapper::getInstalledLocaleNames()
{
    if ( !xInstalledLocales.getLength() )
    {
        LocaleDataWrapper aLDW( ::comphelper::getProcessServiceFactory(), lang::Locale() );
        aLDW.getAllInstalledLocaleNames();
    }
    return xInstalledLocales;
}


// static
::com::sun::star::uno::Sequence< sal_uInt16 > LocaleDataWrapper::getInstalledLanguageTypes()
{
    if ( xInstalledLanguageTypes.getLength() )
        return xInstalledLanguageTypes;

    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > xLoc =
        getInstalledLocaleNames();
    sal_Int32 nCount = xLoc.getLength();
    ::com::sun::star::uno::Sequence< sal_uInt16 > xLang( nCount );
    sal_Int32 nLanguages = 0;
    for ( sal_Int32 i=0; i<nCount; i++ )
    {
#ifndef PRODUCT
        String aDebugLocale( xLoc[i].Language );
        if ( xLoc[i].Country.getLength() )
        {
            aDebugLocale += '_';
            aDebugLocale += String( xLoc[i].Country );
            if ( xLoc[i].Variant.getLength() )
            {
                aDebugLocale += '_';
                aDebugLocale += String( xLoc[i].Variant );
            }
        }
#endif
        if ( xLoc[i].Variant.getLength() )
        {
#ifndef PRODUCT
            ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "ConvertIsoNamesToLanguage: Variants not supported, locale\n" ) );
            aMsg += ByteString( aDebugLocale, RTL_TEXTENCODING_UTF8  );
            DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
            continue;
        }
        LanguageType eLang = ConvertIsoNamesToLanguage( xLoc[i].Language,
            xLoc[i].Country );

#ifndef PRODUCT
        // Exclude known problems because no MS-LCID defined
        if ( eLang == LANGUAGE_DONTKNOW
//              && !aDebugLocale.EqualsAscii( "br_AE" ) // ?!? Breton in United Arabic Emirates
            )
        {
            ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "ConvertIsoNamesToLanguage: unknown MS-LCID for locale\n" ) );
            aMsg += ByteString( aDebugLocale, RTL_TEXTENCODING_UTF8  );
            DBG_ERRORFILE( aMsg.GetBuffer() );
        }
#endif
        switch ( eLang )
        {
            case LANGUAGE_NORWEGIAN :       // no_NO, not Bokmal (nb_NO), not Nynorsk (nn_NO)
                eLang = LANGUAGE_DONTKNOW;  // don't offer "Unknown" language
                break;
        }
        if ( eLang != LANGUAGE_DONTKNOW )
        {
            String aLanguage, aCountry;
            ConvertLanguageToIsoNames( eLang, aLanguage, aCountry );
            if ( String( xLoc[i].Language ) != aLanguage ||
                    String( xLoc[i].Country ) != aCountry )
            {
#ifndef PRODUCT
                // Exclude known problems because no MS-LCID defined and
                // default for Language found.
                if ( TRUE
                        && !aDebugLocale.EqualsAscii( "ar_SD" ) // Sudan/ar
                        && !aDebugLocale.EqualsAscii( "en_CB" ) // Carribean is not a country
//                      && !aDebugLocale.EqualsAscii( "en_BG" ) // ?!? Bulgaria/en
//                      && !aDebugLocale.EqualsAscii( "es_BR" ) // ?!? Brazil/es
                    )
                {
                    ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "ConvertIsoNamesToLanguage/ConvertLanguageToIsoNames: ambiguous locale (MS-LCID?)\n" ) );
                    aMsg += ByteString( aDebugLocale, RTL_TEXTENCODING_UTF8  );
                    aMsg.Append( RTL_CONSTASCII_STRINGPARAM( "  ->  0x" ) );
                    aMsg += ByteString::CreateFromInt32( eLang, 16 );
                    aMsg.Append( RTL_CONSTASCII_STRINGPARAM( "  ->  " ) );
                    aMsg += ByteString( aLanguage, RTL_TEXTENCODING_UTF8  );
                    if ( aCountry.Len() )
                    {
                        aMsg += '_';
                        aMsg += ByteString( aCountry, RTL_TEXTENCODING_UTF8  );
                    }
                    DBG_ERRORFILE( aMsg.GetBuffer() );
                }
#endif
                eLang = LANGUAGE_DONTKNOW;
            }
        }
        if ( eLang != LANGUAGE_DONTKNOW )
            xLang[ nLanguages++ ] = eLang;
    }
    if ( nLanguages < nCount )
        xLang.realloc( nLanguages );
    xInstalledLanguageTypes = xLang;

    return xInstalledLanguageTypes;
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
        bLocaleDataItemValid = TRUE;
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
        bReservedWordValid = TRUE;
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
//! TODO: other measurement systems? => extend enum MeasurementSystem in tools/intn.hxx
    return MEASURE_US;
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


USHORT LocaleDataWrapper::getCurrPositiveFormat() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nCurrPositiveFormat == nCurrFormatInvalid )
    {
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getCurrFormatsImpl();
    }
    return nCurrPositiveFormat;
}


USHORT LocaleDataWrapper::getCurrNegativeFormat() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
    if ( nCurrNegativeFormat == nCurrFormatInvalid )
    {
        aGuard.changeReadToWrite();
        ((LocaleDataWrapper*)this)->getCurrFormatsImpl();
    }
    return nCurrNegativeFormat;
}


USHORT LocaleDataWrapper::getCurrDigits() const
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
    Sequence< Currency > aCurrSeq = getAllCurrencies();
    sal_Int32 nCnt = aCurrSeq.getLength();
    Currency const * const pCurrArr = aCurrSeq.getArray();
    sal_Int32 nElem;
    for ( nElem = 0; nElem < nCnt; nElem++ )
    {
        if ( pCurrArr[nElem].Default )
            break;
    }
    if ( nElem >= nCnt )
    {
#ifndef PRODUCT
        ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "getCurrSymbolsImpl: no default currency" ) );
        DBG_ERRORFILE( AppendLocaleInfo( aMsg ).GetBuffer() );
#endif
        nElem = 0;
        if ( nElem >= nCnt )
        {
            DBG_ERRORFILE( "getCurrSymbolsImpl: no currency at all" );
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
    BOOL bQuote = FALSE;
    const String& rDecSep = getNumDecimalSep();
    while ( p < pStop )
    {
        if ( bQuote )
        {
            if ( *p == '"' && *(p-1) != '\\' )
                bQuote = FALSE;
        }
        else
        {
            switch ( *p )
            {
                case '"' :
                    if ( pStr == p || *(p-1) != '\\' )
                        bQuote = TRUE;
                break;
                case '-' :
                    if ( !nInSection && nSign == STRING_NOTFOUND )
                        nSign = p - pStr;
                break;
                case '(' :
                    if ( !nInSection && nPar == STRING_NOTFOUND )
                        nPar = p - pStr;
                break;
                case '0' :
                case '#' :
                    if ( !nInSection && nNum == STRING_NOTFOUND )
                        nNum = p - pStr;
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
                            nBlank = p - pStr + 1;
                    }
                break;
                case '$' :
                    if ( nSym == STRING_NOTFOUND && nInSection && *(p-1) == '[' )
                    {
                        nSym = p - pStr + 1;
                        if ( nNum != STRING_NOTFOUND && *(p-2) == ' ' )
                            nBlank = p - pStr - 2;
                    }
                break;
                case ';' :
                    if ( !nInSection )
                        p = pStop;
                break;
                default:
                    if ( nSym == STRING_NOTFOUND && rCode.Equals( aCurrSymbol, p-pStr, aCurrSymbol.Len() ) )
                    {   // currency symbol not surrounded by [$...]
                        nSym = p - pStr;
                        if ( nBlank == STRING_NOTFOUND && pStr < p && *(p-1) == ' ' )
                            nBlank = p - pStr - 1;
                        p += aCurrSymbol.Len() - 1;
                        if ( nBlank == STRING_NOTFOUND && p < pStop-2 && *(p+2) == ' ' )
                            nBlank = p - pStr + 2;
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
#ifndef PRODUCT
        ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "getCurrFormatsImpl: no currency formats" ) );
        DBG_ERRORFILE( AppendLocaleInfo( aMsg ).GetBuffer() );
#endif
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
#ifndef PRODUCT
    if ( nNum == STRING_NOTFOUND || nSym == STRING_NOTFOUND )
    {
        ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "getCurrFormatsImpl: CurrPositiveFormat?" ) );
        DBG_ERRORFILE( AppendLocaleInfo( aMsg ).GetBuffer() );
    }
#endif
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
        sal_Int32 nDelim = rCode.indexOf( ';' );
        scanCurrFormatImpl( rCode, nDelim+1, nSign, nPar, nNum, nBlank, nSym );
#ifndef PRODUCT
        if ( nNum == STRING_NOTFOUND || nSym == STRING_NOTFOUND
          || (nPar == STRING_NOTFOUND && nSign == STRING_NOTFOUND) )
        {
            ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "getCurrFormatsImpl: CurrNegativeFormat?" ) );
            DBG_ERRORFILE( AppendLocaleInfo( aMsg ).GetBuffer() );
        }
#endif
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
#ifndef PRODUCT
            ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "scanDateFormat: not all DMY present" ) );
            DBG_ERRORFILE( AppendLocaleInfo( aMsg ).GetBuffer() );
#endif
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
#ifndef PRODUCT
        ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "scanDateFormat: no magic applyable" ) );
        DBG_ERRORFILE( AppendLocaleInfo( aMsg ).GetBuffer() );
#endif
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
#ifndef PRODUCT
        ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "getDateFormatsImpl: no date formats" ) );
        DBG_ERRORFILE( AppendLocaleInfo( aMsg ).GetBuffer() );
#endif
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
#ifndef PRODUCT
        ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "getDateFormatsImpl: no edit" ) );
        DBG_ERRORFILE( AppendLocaleInfo( aMsg ).GetBuffer() );
#endif
        if ( nDef == -1 )
        {
#ifndef PRODUCT
            ByteString aMsg( RTL_CONSTASCII_STRINGPARAM( "getDateFormatsImpl: no default" ) );
            DBG_ERRORFILE( AppendLocaleInfo( aMsg ).GetBuffer() );
#endif
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


// --- simple number formatting helpers -------------------------------

// The ImplAdd... methods are taken from class International and modified to
// suit the needs.

static sal_Unicode* ImplAddUNum( sal_Unicode* pBuf, ULONG nNumber )
{
    // fill temp buffer with digits
    sal_Unicode aTempBuf[30];
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


static sal_Unicode* ImplAddUNum( sal_Unicode* pBuf, ULONG nNumber, int nMinLen )
{
    // fill temp buffer with digits
    sal_Unicode aTempBuf[30];
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


static sal_Unicode* ImplAdd2UNum( sal_Unicode* pBuf, USHORT nNumber, int bLeading )
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
        USHORT nTemp = nNumber % 10;
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
        long nNumber, USHORT nDecimals, BOOL bUseThousandSep,
        BOOL bTrailingZeros ) const
{
    sal_Unicode aNumBuf[32];
    sal_Unicode* pNumBuf;
    USHORT  nNumLen;
    USHORT  i = 0;
    BOOL    bNeg;

    // negative number
    if ( nNumber < 0 )
    {
        nNumber *= -1;
        bNeg = TRUE;
        *pBuf = '-';
        pBuf++;
    }
    else
        bNeg = FALSE;

    // convert number
    pNumBuf = ImplAddUNum( aNumBuf, (ULONG)nNumber );
    nNumLen = (USHORT)(ULONG)(pNumBuf-aNumBuf);
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
        USHORT nNumLen2 = nNumLen-nDecimals;
        while ( i < nNumLen2 )
        {
            *pBuf = *pNumBuf;
            pBuf++;
            pNumBuf++;
            i++;

            // add thousand separator?
            if ( bUseThousandSep && !((nNumLen2-i)%3) && (i < nNumLen2) )
                pBuf = ImplAddString( pBuf, rThoSep );
        }

        // append decimals
        if ( nDecimals )
        {
            pBuf = ImplAddString( pBuf, getNumDecimalSep() );

            BOOL bNullEnd = TRUE;
            while ( i < nNumLen )
            {
                if ( *pNumBuf != '0' )
                    bNullEnd = FALSE;

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
    USHORT  nDay    = rDate.GetDay();
    USHORT  nMonth  = rDate.GetMonth();
    USHORT  nYear   = rDate.GetYear();
    USHORT  nYearLen;

    if ( TRUE /* IsDateCentury() */ )
        nYearLen = 4;
    else
    {
        nYearLen = 2;
        nYear %= 100;
    }

    switch ( getDateFormat() )
    {
        case DMY :
            pBuf = ImplAdd2UNum( pBuf, nDay, TRUE /* IsDateDayLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nMonth, TRUE /* IsDateMonthLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAddUNum( pBuf, nYear, nYearLen );
        break;
        case MDY :
            pBuf = ImplAdd2UNum( pBuf, nMonth, TRUE /* IsDateMonthLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nDay, TRUE /* IsDateDayLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAddUNum( pBuf, nYear, nYearLen );
        break;
        default:
            pBuf = ImplAddUNum( pBuf, nYear, nYearLen );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nMonth, TRUE /* IsDateMonthLeadingZero() */ );
            pBuf = ImplAddString( pBuf, getDateSep() );
            pBuf = ImplAdd2UNum( pBuf, nDay, TRUE /* IsDateDayLeadingZero() */ );
    }

    return String( aBuf, (xub_StrLen)(ULONG)(pBuf-aBuf) );
}


String LocaleDataWrapper::getTime( const Time& rTime, BOOL bSec, BOOL b100Sec ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
//!TODO: leading zeros et al
    sal_Unicode aBuf[128];
    sal_Unicode* pBuf = aBuf;
    USHORT  nHour = rTime.GetHour();
    BOOL bHour12 = FALSE;   //!TODO: AM/PM from default time format code

    if ( bHour12 )
    {
        nHour %= 12;
        // 0:00 -> 12:00
        if ( !nHour )
            nHour = 12;
    }
    else
        nHour %= 24;

    pBuf = ImplAdd2UNum( pBuf, nHour, TRUE /* IsTimeLeadingZero() */ );
    pBuf = ImplAddString( pBuf, getTimeSep() );
    pBuf = ImplAdd2UNum( pBuf, rTime.GetMin(), TRUE );
    if ( bSec )
    {
        pBuf = ImplAddString( pBuf, getTimeSep() );
        pBuf = ImplAdd2UNum( pBuf, rTime.GetSec(), TRUE );

        if ( b100Sec )
        {
            pBuf = ImplAddString( pBuf, getTime100SecSep() );
            pBuf = ImplAdd2UNum( pBuf, rTime.Get100Sec(), TRUE );
        }
    }

    String aStr( aBuf, (xub_StrLen)(ULONG)(pBuf-aBuf) );

    if ( bHour12 )
    {
        if ( (rTime.GetHour() % 24) >= 12 )
            aStr += getTimePM();
        else
            aStr += getTimeAM();
    }
#if 0
//!TODO: do we need a time string? like "o'clock" or "Uhr" or similar
    else
        aStr += getTimeStr();
#endif

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
    String aDay( aBuf, (xub_StrLen)(ULONG)(pBuf-aBuf) );
    // month of year
    nVal = rCal.getValue( CalendarFieldIndex::MONTH );
    String aMonth( rCal.getDisplayName( CalendarDisplayIndex::MONTH, nVal, nDisplayMonth ) );
    // year
    nVal = rCal.getValue( CalendarFieldIndex::YEAR );
    if ( bTwoDigitYear )
        pBuf = ImplAddUNum( aBuf, nVal % 100, 2 );
    else
        pBuf = ImplAddUNum( aBuf, nVal );
    String aYear( aBuf, (xub_StrLen)(ULONG)(pBuf-aBuf) );
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


String LocaleDataWrapper::getDuration( const Time& rTime, BOOL bSec, BOOL b100Sec ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    sal_Unicode aBuf[128];
    sal_Unicode* pBuf = aBuf;

    if ( rTime < Time( 0 ) )
        pBuf = ImplAddString( pBuf, ' ' );

    if ( TRUE /* IsTimeLeadingZero() */ )
        pBuf = ImplAddUNum( pBuf, rTime.GetHour(), 2 );
    else
        pBuf = ImplAddUNum( pBuf, rTime.GetHour() );
    pBuf = ImplAddString( pBuf, getTimeSep() );
    pBuf = ImplAdd2UNum( pBuf, rTime.GetMin(), TRUE );
    if ( bSec )
    {
        pBuf = ImplAddString( pBuf, getTimeSep() );
        pBuf = ImplAdd2UNum( pBuf, rTime.GetSec(), TRUE );

        if ( b100Sec )
        {
            pBuf = ImplAddString( pBuf, getTime100SecSep() );
            pBuf = ImplAdd2UNum( pBuf, rTime.Get100Sec(), TRUE );
        }
    }

    return String( aBuf, (xub_StrLen)(ULONG)(pBuf-aBuf) );
}


// --- simple number formatting ---------------------------------------

inline long ImplGetNumberStringLengthGuess( const LocaleDataWrapper& rLoc, USHORT nDecimals )
{
    // approximately 3.2 bits per digit
    const long nDig = ((sizeof(long) * 8) / 3) + 1;
    // digits, separators, leading zero, sign
    long nGuess = ((nDecimals < nDig) ?
        ((((nDig - nDecimals) / 3) * rLoc.getNumThousandSep().Len()) + nDig) :
        nDecimals) + rLoc.getNumDecimalSep().Len() + 3;
    return nGuess;
}


String LocaleDataWrapper::getNum( long nNumber, USHORT nDecimals,
        BOOL bUseThousandSep, BOOL bTrailingZeros ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    sal_Unicode aBuf[48];       // big enough for 64-bit long
    // check if digits and separators will fit into fixed buffer or allocate
    long nGuess = ImplGetNumberStringLengthGuess( *this, nDecimals );
    sal_Unicode* const pBuffer = (nGuess < 42 ? aBuf :
        new sal_Unicode[nGuess + 16]);

    sal_Unicode* pBuf = ImplAddFormatNum( pBuffer, nNumber, nDecimals,
        bUseThousandSep, bTrailingZeros );
    String aStr( pBuffer, (xub_StrLen)(ULONG)(pBuf-pBuffer) );

    if ( pBuffer != aBuf )
        delete [] pBuffer;
    return aStr;
}


#if SUPD < 638
String LocaleDataWrapper::getNum( long nNumber, USHORT nDecimals, BOOL bUseThousandSep ) const
{
    return getNum( nNumber, nDecimals, bUseThousandSep, TRUE );
}
#endif


String LocaleDataWrapper::getCurr( long nNumber, USHORT nDecimals,
        const String& rCurrencySymbol, BOOL bUseThousandSep ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    sal_Unicode aBuf[75];
    sal_Unicode aNumBuf[48];    // big enough for 64-bit long
    sal_Unicode cZeroChar = getCurrZeroChar();

    // check if digits and separators will fit into fixed buffer or allocate
    long nGuess = ImplGetNumberStringLengthGuess( *this, nDecimals );
    sal_Unicode* const pNumBuffer = (nGuess < 42 ? aNumBuf :
        new sal_Unicode[nGuess + 16]);

    sal_Unicode* const pBuffer =
        ((rCurrencySymbol.Len() + nGuess + 20) < sizeof(aBuf) ? aBuf :
        new sal_Unicode[ rCurrencySymbol.Len() + nGuess + 20 ]);
    sal_Unicode* pBuf = pBuffer;

    BOOL bNeg;
    if ( nNumber < 0 )
    {
        bNeg = TRUE;
        nNumber *= -1;
    }
    else
        bNeg = FALSE;

    // convert number
    sal_Unicode* pEndNumBuf = ImplAddFormatNum( pNumBuffer, nNumber, nDecimals,
        bUseThousandSep, TRUE );
    xub_StrLen nNumLen = (xub_StrLen)(ULONG)(pEndNumBuf-pNumBuffer);

    // replace zeros with zero character
    if ( (cZeroChar != '0') && nDecimals /* && IsNumTrailingZeros() */ )
    {
        sal_Unicode* pTempBuf;
        USHORT  i;
        BOOL    bZero = TRUE;

        pTempBuf = pNumBuffer+nNumLen-nDecimals;
        i = 0;
        do
        {
            if ( *pTempBuf != '0' )
            {
                bZero = FALSE;
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

    String aNumber( pBuffer, (xub_StrLen)(ULONG)(pBuf-pBuffer) );

    if ( pBuffer != aBuf )
        delete pBuffer;
    if ( pNumBuffer != aNumBuf )
        delete pNumBuffer;

    return aNumber;
}


// --- mixed ----------------------------------------------------------

::com::sun::star::lang::Locale LocaleDataWrapper::getLoadedLocale() const
{
    LanguageCountryInfo aLCInfo = getLanguageCountryInfo();
    return lang::Locale( aLCInfo.Language, aLCInfo.Country, aLCInfo.Variant );
}


#ifndef PRODUCT
ByteString& LocaleDataWrapper::AppendLocaleInfo( ByteString& rDebugMsg ) const
{
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nBlockCritical );
    rDebugMsg += '\n';
    rDebugMsg += ByteString( String( aLocale.Language ), RTL_TEXTENCODING_UTF8 );
    rDebugMsg += '_';
    rDebugMsg += ByteString( String( aLocale.Country ), RTL_TEXTENCODING_UTF8 );
    rDebugMsg.Append( RTL_CONSTASCII_STRINGPARAM( " requested\n" ) );
    lang::Locale aLoaded = getLoadedLocale();
    rDebugMsg += ByteString( String( aLoaded.Language ), RTL_TEXTENCODING_UTF8 );
    rDebugMsg += '_';
    rDebugMsg += ByteString( String( aLoaded.Country ), RTL_TEXTENCODING_UTF8 );
    rDebugMsg.Append( RTL_CONSTASCII_STRINGPARAM( " loaded" ) );
    return rDebugMsg;
}
#endif
