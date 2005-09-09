/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: calendarwrapper.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:27:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#define _UNOTOOLS_CALENDARWRAPPER_HXX

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_CALENDAR_HPP_
#include <com/sun/star/i18n/Calendar.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}

namespace com { namespace sun { namespace star {
    namespace i18n {
        class XExtendedCalendar;
    }
}}}


class UNOTOOLS_DLLPUBLIC CalendarWrapper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XExtendedCalendar >   xC;

            DateTime            aEpochStart;        // 1Jan1970

public:
                                CalendarWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF
                                    );
                                ~CalendarWrapper();


    // wrapper implementations of XCalendar

    void loadDefaultCalendar( const ::com::sun::star::lang::Locale& rLocale );
    void loadCalendar( const ::rtl::OUString& rUniqueID, const ::com::sun::star::lang::Locale& rLocale );
    ::com::sun::star::i18n::Calendar getLoadedCalendar() const;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > getAllCalendars( const ::com::sun::star::lang::Locale& rLocale ) const;
    ::rtl::OUString getUniqueID() const;
    /// set UTC date/time
    void setDateTime( double nTimeInDays );
    /// get UTC date/time
    double getDateTime() const;
    /// convenience method to set local date/time
    void setLocalDateTime( double nTimeInDays );
    /// convenience method to get local date/time
    double getLocalDateTime() const;
    void setValue( sal_Int16 nFieldIndex, sal_Int16 nValue );
    sal_Bool isValid() const;
    sal_Int16 getValue( sal_Int16 nFieldIndex ) const;
    void addValue( sal_Int16 nFieldIndex, sal_Int32 nAmount );
    sal_Int16 getFirstDayOfWeek() const;
    void setFirstDayOfWeek( sal_Int16 nDay );
    void setMinimumNumberOfDaysForFirstWeek( sal_Int16 nDays );
    sal_Int16 getMinimumNumberOfDaysForFirstWeek() const;
    sal_Int16 getNumberOfMonthsInYear() const;
    sal_Int16 getNumberOfDaysInWeek() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > getMonths() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > getDays() const;
    String getDisplayName( sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType ) const;

    // wrapper implementations of XExtendedCalendar

    String getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) const;


    // convenience methods

    /// get epoch start (should be 01Jan1970)
    inline  const DateTime&     getEpochStart() const
                                    { return aEpochStart; }

    /// set a local (!) Gregorian DateTime
    inline  void                setGregorianDateTime( const DateTime& rDateTime )
                                    { setLocalDateTime( rDateTime - aEpochStart ); }

    /// get the DateTime as a local (!) Gregorian DateTime
    inline  DateTime            getGregorianDateTime() const
                                    { return aEpochStart + getLocalDateTime(); }

};

#endif
