/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#define _UNOTOOLS_CALENDARWRAPPER_HXX

#include <tools/datetime.hxx>
#include <tools/string.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/i18n/Calendar.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include "unotools/unotoolsdllapi.h"

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

    // wrapper implementations of XCalendar

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

    /** Convenience method to get timezone offset in milliseconds, taking both
        fields ZONE_OFFSET and ZONE_OFFSET_SECOND_MILLIS into account. */
    sal_Int32 getZoneOffsetInMillis() const;
    /** Convenience method to get DST offset in milliseconds, taking both
        fields DST_OFFSET and DST_OFFSET_SECOND_MILLIS into account. */
    sal_Int32 getDSTOffsetInMillis() const;

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

private:

    /** get timezone or DST offset in milliseconds, fields are
        CalendarFieldIndex ZONE_OFFSET and ZONE_OFFSET_SECOND_MILLIS
        respectively DST_OFFSET and DST_OFFSET_SECOND_MILLIS.
     */
    sal_Int32 getCombinedOffsetInMillis( sal_Int16 nParentFieldIndex, sal_Int16 nChildFieldIndex ) const;
};

#endif
