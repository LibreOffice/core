/*************************************************************************
 *
 *  $RCSfile: calendarwrapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2000-11-23 20:19:32 $
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

#include <unotools/calendarwrapper.hxx>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COMPHELPER_COMPONENTFACTORY_HXX_
#include <comphelper/componentfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_XCALENDAR_HPP_
#include <com/sun/star/i18n/XCalendar.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#pragma hdrstop


#define CALENDAR_LIBRARYNAME "int"
#define CALENDAR_SERVICENAME "com.sun.star.i18n.Calendar"


using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;


CalendarWrapper::CalendarWrapper(
            const Reference< lang::XMultiServiceFactory > & xSF
            )
        :
        xSMgr( xSF ),
        aEpochStart( Date( 1, 1, 1970 ) )
{
    if ( xSMgr.is() )
    {
        try
        {
            xC = Reference< XCalendar > ( xSMgr->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CALENDAR_SERVICENAME ) ) ),
                uno::UNO_QUERY );
        }
        catch ( Exception& e )
        {
#ifndef PRODUCT
            ByteString aMsg( "CalendarWrapper ctor: Exception caught\n" );
            aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
            DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
        }
    }
    else
    {   // try to get an instance somehow
        DBG_ERRORFILE( "CalendarWrapper: no service manager, trying own" );
        try
        {
            Reference< XInterface > xI = ::comphelper::getComponentInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LLCF_LIBNAME( CALENDAR_LIBRARYNAME ) ) ),
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CALENDAR_SERVICENAME ) ) );
            if ( xI.is() )
            {
                Any x = xI->queryInterface( ::getCppuType((const Reference< XCalendar >*)0) );
                x >>= xC;
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


CalendarWrapper::~CalendarWrapper()
{
}


void CalendarWrapper::loadDefaultCalendar( const ::com::sun::star::lang::Locale& rLocale )
{
    try
    {
        if ( xC.is() )
            xC->loadDefaultCalendar( rLocale );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "loadDefaultCalendar: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
}


void CalendarWrapper::loadCalendar( const ::rtl::OUString& rUniqueID, const ::com::sun::star::lang::Locale& rLocale )
{
    try
    {
        if ( xC.is() )
            xC->loadCalendar( rUniqueID, rLocale );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "loadCalendar: Exception caught\nrequested: " );
        aMsg += ByteString( String( rUniqueID ), RTL_TEXTENCODING_UTF8 );
        aMsg += "   Locale: ";
        aMsg += ByteString( String( rLocale.Language ), RTL_TEXTENCODING_UTF8 );
        aMsg += '_';
        aMsg += ByteString( String( rLocale.Country ), RTL_TEXTENCODING_UTF8 );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
}


::com::sun::star::i18n::Calendar CalendarWrapper::getLoadedCalendar() const
{
    try
    {
        if ( xC.is() )
            return xC->getLoadedCalendar();
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getLoadedCalendar: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return ::com::sun::star::i18n::Calendar();
}


::com::sun::star::uno::Sequence< ::rtl::OUString > CalendarWrapper::getAllCalendars( const ::com::sun::star::lang::Locale& rLocale ) const
{
    try
    {
        if ( xC.is() )
            return xC->getAllCalendars( rLocale );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getAllCalendars: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return ::com::sun::star::uno::Sequence< ::rtl::OUString > (0);
}


::rtl::OUString CalendarWrapper::getUniqueID() const
{
    try
    {
        if ( xC.is() )
            return xC->getUniqueID();
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getUniqueID: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return ::rtl::OUString();
}


void CalendarWrapper::setDateTime( double nTimeInDays )
{
    try
    {
        if ( xC.is() )
            xC->setDateTime( nTimeInDays );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "setDateTime: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
}


double CalendarWrapper::getDateTime() const
{
    try
    {
        if ( xC.is() )
            return xC->getDateTime();
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getDateTime: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return 0.0;
}


void CalendarWrapper::setValue( sal_Int16 nFieldIndex, sal_Int16 nValue )
{
    try
    {
        if ( xC.is() )
            xC->setValue( nFieldIndex, nValue );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "setValue: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
}


sal_Int16 CalendarWrapper::getValue( sal_Int16 nFieldIndex ) const
{
    try
    {
        if ( xC.is() )
            return xC->getValue( nFieldIndex );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getValue: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return 0;
}


void CalendarWrapper::addValue( sal_Int16 nFieldIndex, sal_Int32 nAmount )
{
    try
    {
        if ( xC.is() )
            xC->addValue( nFieldIndex, nAmount );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "addValue: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
}


sal_Int16 CalendarWrapper::getFirstDayOfWeek() const
{
    try
    {
        if ( xC.is() )
            return xC->getFirstDayOfWeek();
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getFirstDayOfWeek: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return 0;
}


void CalendarWrapper::setFirstDayOfWeek( sal_Int16 nDay )
{
    try
    {
        if ( xC.is() )
            xC->setFirstDayOfWeek( nDay );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "setFirstDayOfWeek: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
}


void CalendarWrapper::setMinimumNumberOfDaysForFirstWeek( sal_Int16 nDays )
{
    try
    {
        if ( xC.is() )
            xC->setMinimumNumberOfDaysForFirstWeek( nDays );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "setMinimumNumberOfDaysForFirstWeek: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
}


sal_Int16 CalendarWrapper::getMinimumNumberOfDaysForFirstWeek() const
{
    try
    {
        if ( xC.is() )
            return xC->getMinimumNumberOfDaysForFirstWeek();
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getMinimumNumberOfDaysForFirstWeek: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return 0;
}


sal_Int16 CalendarWrapper::getNumberOfMonthsInYear() const
{
    try
    {
        if ( xC.is() )
            return xC->getNumberOfMonthsInYear();
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getNumberOfMonthsInYear: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return 0;
}


sal_Int16 CalendarWrapper::getNumberOfDaysInWeek() const
{
    try
    {
        if ( xC.is() )
            return xC->getNumberOfDaysInWeek();
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getNumberOfDaysInWeek: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return 0;
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > CalendarWrapper::getMonths() const
{
    try
    {
        if ( xC.is() )
            return xC->getMonths();
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getMonths: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > (0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > CalendarWrapper::getDays() const
{
    try
    {
        if ( xC.is() )
            return xC->getDays();
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getDays: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > (0);
}


String CalendarWrapper::getDisplayName( sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType ) const
{
    try
    {
        if ( xC.is() )
            return xC->getDisplayName( nCalendarDisplayIndex, nIdx, nNameType );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getDisplayName: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return String();
}


