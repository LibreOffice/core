/*************************************************************************
 *
 *  $RCSfile: localedatawrapper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: er $ $Date: 2000-11-03 20:44:26 $
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

#include <unotools/localedatawrapper.hxx>
#include <unotools/numberformatcodewrapper.hxx>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _UNOTOOLS_COMPONENTFACTORY_HXX_
#include <unotools/componentfactory.hxx>
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

#pragma hdrstop


#define LOCALEDATA_LIBRARYNAME "int"
#define LOCALEDATA_SERVICENAME "com.sun.star.i18n.LocaleData"

static const USHORT nCurrFormatInvalid = 0xffff;
static const USHORT nCurrFormatDefault = 0;

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;


LocaleDataWrapper::LocaleDataWrapper(
            const Reference< lang::XMultiServiceFactory > & xSF,
            const lang::Locale& rLocale
            )
        :
        xSMgr( xSF ),
        nCurrPositiveFormat( nCurrFormatInvalid ),
        nCurrNegativeFormat( nCurrFormatInvalid ),
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
            DBG_ERRORFILE( "LocaleDataWrapper ctor: Exception caught!" );
        }
    }
    else
    {   // try to get an instance somehow
        DBG_ERRORFILE( "LocaleDataWrapper: no service manager, trying own" );
        try
        {
            Reference< XInterface > xI = ::utl::getComponentInstance(
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
            DBG_ERRORFILE( "getComponentInstance: Exception caught!" );
        }
    }
}


LocaleDataWrapper::~LocaleDataWrapper()
{
}


void LocaleDataWrapper::setLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    aLocale = rLocale;
    invalidateData();
}


void LocaleDataWrapper::invalidateData()
{
    aCurrSymbol.Erase();
    aCurrBankSymbol.Erase();
    nCurrPositiveFormat = nCurrNegativeFormat = nCurrFormatInvalid;
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
}


::com::sun::star::i18n::LanguageCountryInfo LocaleDataWrapper::getLanguageCountryInfo() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getLanguageCountryInfo( aLocale );
    }
    catch ( Exception& e )
    {
        DBG_ERRORFILE( "getLanguageCountryInfo: Exception caught!" );
    }
    return ::com::sun::star::i18n::LanguageCountryInfo();
}


::com::sun::star::i18n::LocaleDataItem LocaleDataWrapper::getLocaleItem() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getLocaleItem( aLocale );
    }
    catch ( Exception& e )
    {
        DBG_ERRORFILE( "getLocaleItem: Exception caught!" );
    }
    return ::com::sun::star::i18n::LocaleDataItem();
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar > LocaleDataWrapper::getAllCalendars() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getAllCalendars( aLocale );
    }
    catch ( Exception& e )
    {
        DBG_ERRORFILE( "getAllCalendars: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency > LocaleDataWrapper::getAllCurrencies() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getAllCurrencies( aLocale );
    }
    catch ( Exception& e )
    {
        DBG_ERRORFILE( "getAllCurrencies: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > LocaleDataWrapper::getAllFormats() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getAllFormats( aLocale );
    }
    catch ( Exception& e )
    {
        DBG_ERRORFILE( "getAllFormats: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Implementation > LocaleDataWrapper::getCollatorImplementations() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getCollatorImplementations( aLocale );
    }
    catch ( Exception& e )
    {
        DBG_ERRORFILE( "getCollatorImplementations: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Implementation >(0);
}


::com::sun::star::uno::Sequence< ::rtl::OUString > LocaleDataWrapper::getTransliterations() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getTransliterations( aLocale );
    }
    catch ( Exception& e )
    {
        DBG_ERRORFILE( "getTransliterations: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::rtl::OUString >(0);
}


::com::sun::star::i18n::ForbiddenCharacters LocaleDataWrapper::getForbiddenCharacters() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getForbiddenCharacters( aLocale );
    }
    catch ( Exception& e )
    {
        DBG_ERRORFILE( "getForbiddenCharacters: Exception caught!" );
    }
    return ::com::sun::star::i18n::ForbiddenCharacters();
}


::com::sun::star::uno::Sequence< ::rtl::OUString > LocaleDataWrapper::getReservedWord() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getReservedWord( aLocale );
    }
    catch ( Exception& e )
    {
        DBG_ERRORFILE( "getReservedWord: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::rtl::OUString >(0);
}


// --- Impl and helpers ----------------------------------------------------

const String& LocaleDataWrapper::getOneLocaleItem( sal_Int16 nItem ) const
{
    if ( nItem >= LocaleItem::COUNT )
    {
        DBG_ERRORFILE( "getOneLocaleItem: bounds" );
        return aLocaleItem[0];
    }
    if ( aLocaleItem[nItem].Len() == 0 )
    {   // no cached content
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
    if ( nWord >= reservedWords::COUNT )
    {
        DBG_ERRORFILE( "getOneReservedWord: bounds" );
        return aReservedWord[0];
    }
    if ( aReservedWord[nWord].Len() == 0 )
    {   // no cached content
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


const String& LocaleDataWrapper::getCurrSymbol() const
{
    if ( !aCurrSymbol.Len() )
        ((LocaleDataWrapper*)this)->getCurrSymbolsImpl();
    return aCurrSymbol;
}


const String& LocaleDataWrapper::getCurrBankSymbol() const
{
    if ( !aCurrBankSymbol.Len() )
        ((LocaleDataWrapper*)this)->getCurrSymbolsImpl();
    return aCurrBankSymbol;
}


USHORT LocaleDataWrapper::getCurrPositiveFormat() const
{
    if ( nCurrPositiveFormat == nCurrFormatInvalid )
        ((LocaleDataWrapper*)this)->getCurrFormatsImpl();
    return nCurrPositiveFormat;
}


USHORT LocaleDataWrapper::getCurrNegativeFormat() const
{
    if ( nCurrNegativeFormat == nCurrFormatInvalid )
        ((LocaleDataWrapper*)this)->getCurrFormatsImpl();
    return nCurrNegativeFormat;
}


void LocaleDataWrapper::getCurrSymbolsImpl()
{
    Sequence< Currency > aCurrSeq = getAllCurrencies();
    sal_Int32 nCnt = aCurrSeq.getLength();
    sal_Int32 nElem;
    for ( nElem = 0; nElem < nCnt; nElem++ )
    {
        if ( aCurrSeq[nElem].isDefault )
            break;
    }
    if ( nElem >= nCnt )
    {
        DBG_ERRORFILE( "getCurrFormatsImpl: no default currency" );
        nElem = 0;
        if ( nElem >= nCnt )
        {
            DBG_ERRORFILE( "getCurrFormatsImpl: no currency at all" );
            aCurrSymbol.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "ShellsAndPebbles" ) );
            aCurrBankSymbol = aCurrSymbol;
            nCurrPositiveFormat = nCurrNegativeFormat = nCurrFormatDefault;
            return ;
        }
    }
    aCurrSymbol = aCurrSeq[nElem].symbol;
    aCurrBankSymbol = aCurrSeq[nElem].bankSymbol;
}


// quasi static
void lcl_LocaleDataWrapper_scanCurrFormat( const String& rCode,
        xub_StrLen nStart, const String& rCurr, xub_StrLen& nSign,
        xub_StrLen& nPar, xub_StrLen& nNum, xub_StrLen& nBlank,
        xub_StrLen& nSym )
{
    nSign = nPar = nNum = nBlank = nSym = STRING_NOTFOUND;
    const sal_Unicode* const pStr = rCode.GetBuffer();
    const sal_Unicode* const pStop = pStr + rCode.Len();
    const sal_Unicode* p = pStr + nStart;
    int nInSection = 0;
    BOOL bQuote = FALSE;
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
                    if ( nSym == STRING_NOTFOUND && rCurr.Equals( rCode, p-pStr, rCurr.Len() ) )
                    {   // currency symbol not surrounded by [$...]
                        nSym = p - pStr;
                        if ( nBlank == STRING_NOTFOUND && pStr < p && *(p-1) == ' ' )
                            nBlank = p - pStr - 1;
                        p += rCurr.Len() - 1;
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
        ByteString aMsg( "getCurrFormatsImpl: no currency formats for " );
        aMsg += ByteString( String(aLocale.Language), RTL_TEXTENCODING_UTF8 );
        aMsg += '_';
        aMsg += ByteString( String(aLocale.Country), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
        nCurrPositiveFormat = nCurrNegativeFormat = nCurrFormatDefault;
        return ;
    }
    sal_Int32 nElem;
    sal_Int32 nNeg = -1;
    sal_Int32 nDef = -1;
    // find a negative code and a default (not necessarily the same)
    for ( nElem = 0; nElem < nCnt; nElem++ )
    {
        if ( aFormatSeq[nElem].Code.indexOf( ';' ) >= 0 )
        {
            nNeg = nElem;
            if ( nDef >= 0 )
                break;
        }
        if ( aFormatSeq[nElem].Default )
        {
            nDef = nElem;
            if ( nNeg >= 0 )
                break;
        }
    }

    getCurrSymbol();

    xub_StrLen nSign, nPar, nNum, nBlank, nSym;

    // positive format
    nElem = (nDef >= 0 ? nDef : (nNeg >= 0 ? nNeg : 0));
    lcl_LocaleDataWrapper_scanCurrFormat( aFormatSeq[nElem].Code, 0,
        aCurrSymbol, nSign, nPar, nNum, nBlank, nSym );
#ifndef PRODUCT
    if ( nNum == STRING_NOTFOUND || nSym == STRING_NOTFOUND )
    {
        ByteString aMsg( "getCurrFormatsImpl: CurrPositiveFormat? " );
        aMsg += ByteString( String(aLocale.Language), RTL_TEXTENCODING_UTF8 );
        aMsg += '_';
        aMsg += ByteString( String(aLocale.Country), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
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
    nElem = (nNeg >= 0 ? nNeg : 0);
    const ::rtl::OUString& rCode = aFormatSeq[nElem].Code;
    sal_Int32 nDelim = rCode.indexOf( ';' );
    if ( nDelim < 0 )
        nCurrNegativeFormat = nCurrFormatDefault;
    else
    {
        lcl_LocaleDataWrapper_scanCurrFormat( rCode, nDelim+1,
            aCurrSymbol, nSign, nPar, nNum, nBlank, nSym );
#ifndef PRODUCT
        if ( nNum == STRING_NOTFOUND || nSym == STRING_NOTFOUND
          || (nPar == STRING_NOTFOUND && nSign == STRING_NOTFOUND) )
        {
            ByteString aMsg( "getCurrFormatsImpl: CurrNegativeFormat? " );
            aMsg += ByteString( String(aLocale.Language), RTL_TEXTENCODING_UTF8 );
            aMsg += '_';
            aMsg += ByteString( String(aLocale.Country), RTL_TEXTENCODING_UTF8 );
            DBG_ERRORFILE( aMsg.GetBuffer() );
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


const ::com::sun::star::lang::Locale LocaleDataWrapper::getLoadedLocale() const
{
    LanguageCountryInfo aLCInfo = getLanguageCountryInfo();
    return lang::Locale( aLCInfo.Language, aLCInfo.Country, aLCInfo.Variant );
}
