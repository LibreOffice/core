/*************************************************************************
 *
 *  $RCSfile: localedatawrapper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: er $ $Date: 2000-10-13 19:52:04 $
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

#pragma hdrstop


#define LOCALEDATA_LIBRARYNAME "int"
#define LOCALEDATA_SERVICENAME "com.sun.star.lang.LocaleData"

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;


LocaleDataWrapper::LocaleDataWrapper(
            const Reference< XMultiServiceFactory > & xSF,
            const Locale& rLocale
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
    if ( bLocaleDataItemValid )
    {
        for ( sal_Int32 j=0; j<LocaleItem_LocaleItemTotalCount; j++ )
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


::com::sun::star::lang::LCInfo LocaleDataWrapper::getLCInfo() const
{
    try
    {
        if ( xLD.is() )
            return xLD->getLCInfo( aLocale );
    }
    catch ( Exception& e )
    {
        DBG_ERRORFILE( "getLCInfo: Exception caught!" );
    }
    return ::com::sun::star::lang::LCInfo();
}


::com::sun::star::lang::LocaleDataItem LocaleDataWrapper::getLocaleItem() const
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
    return ::com::sun::star::lang::LocaleDataItem();
}


::com::sun::star::uno::Sequence< ::com::sun::star::lang::Calendar > LocaleDataWrapper::getAllCalendars() const
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
    return ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Calendar >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::lang::Currency > LocaleDataWrapper::getAllCurrencies() const
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
    return ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Currency >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::lang::FormatElement > LocaleDataWrapper::getAllFormats() const
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
    return ::com::sun::star::uno::Sequence< ::com::sun::star::lang::FormatElement >(0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::lang::Implementation > LocaleDataWrapper::getCollatorImplementations() const
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
    return ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Implementation >(0);
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


::com::sun::star::lang::ForbiddenCharacters LocaleDataWrapper::getForbiddenCharacters() const
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
    return ::com::sun::star::lang::ForbiddenCharacters();
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

const String& LocaleDataWrapper::getOneLocaleItem( LocaleItem eItem ) const
{
    if ( eItem >= LocaleItem_LocaleItemTotalCount )
    {
        DBG_ERRORFILE( "getOneLocaleItem: bounds" );
        return aLocaleItem[0];
    }
    if ( aLocaleItem[eItem].Len() == 0 )
    {   // no cached content
        ((LocaleDataWrapper*)this)->getOneLocaleItemImpl( eItem );
    }
    return aLocaleItem[eItem];
}


void LocaleDataWrapper::getOneLocaleItemImpl( LocaleItem eItem )
{
    if ( !bLocaleDataItemValid )
    {
        aLocaleDataItem = getLocaleItem();
        bLocaleDataItemValid = TRUE;
    }
    switch ( eItem )
    {
        case LocaleItem_DateSeparator :
            aLocaleItem[eItem] = aLocaleDataItem.dateSeparator;
        break;
        case LocaleItem_ThousandSeparator :
            aLocaleItem[eItem] = aLocaleDataItem.thousandSeparator;
        break;
        case LocaleItem_DecimalSeparator :
            aLocaleItem[eItem] = aLocaleDataItem.decimalSeparator;
        break;
        case LocaleItem_TimeSeparator :
            aLocaleItem[eItem] = aLocaleDataItem.timeSeparator;
        break;
        case LocaleItem_Time100SecSeparator :
            aLocaleItem[eItem] = aLocaleDataItem.time100SecSeparator;
        break;
        case LocaleItem_ListSeparator :
            aLocaleItem[eItem] = aLocaleDataItem.listSeparator;
        break;
        case LocaleItem_QuotationStart :
            aLocaleItem[eItem] = aLocaleDataItem.quotationStart;
        break;
        case LocaleItem_QuotationEnd :
            aLocaleItem[eItem] = aLocaleDataItem.quotationEnd;
        break;
        case LocaleItem_DoubleQuotationStart :
            aLocaleItem[eItem] = aLocaleDataItem.doubleQuotationStart;
        break;
        case LocaleItem_DoubleQuotationEnd :
            aLocaleItem[eItem] = aLocaleDataItem.doubleQuotationEnd;
        break;
        case LocaleItem_MeasurementSystem :
            aLocaleItem[eItem] = aLocaleDataItem.measurementSystem;
        break;
        case LocaleItem_TimeAM :
            aLocaleItem[eItem] = aLocaleDataItem.timeAM;
        break;
        case LocaleItem_TimePM :
            aLocaleItem[eItem] = aLocaleDataItem.timePM;
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
