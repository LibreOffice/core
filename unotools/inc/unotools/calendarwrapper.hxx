/*************************************************************************
 *
 *  $RCSfile: calendarwrapper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2000-11-18 20:02:42 $
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

#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#define _UNOTOOLS_CALENDARWRAPPER_HXX

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
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

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace i18n {
        class XCalendar;
    }
}}}


class CalendarWrapper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCalendar >   xC;

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
    void setDateTime( double nTimeInDays );
    double getDateTime() const;
    void setValue( sal_Int16 nFieldIndex, sal_Int16 nValue );
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


    // convenience methods

    /// set a gregorian DateTime
    inline  void                setGregorianDateTime( const DateTime& rDateTime )
                                    { setDateTime( rDateTime - aEpochStart ); }

    /// get the DateTime as a gregorian DateTime
    inline  DateTime            getGregorianDateTime() const
                                    { return aEpochStart + getDateTime(); }

};

#endif
