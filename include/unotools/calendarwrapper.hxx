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

#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#define _UNOTOOLS_CALENDARWRAPPER_HXX

#include <tools/datetime.hxx>
#include <tools/string.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/i18n/Calendar2.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include "unotools/unotoolsdllapi.h"

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
    }
}}}

namespace com { namespace sun { namespace star {
    namespace i18n {
        class XCalendar3;
    }
}}}


class UNOTOOLS_DLLPUBLIC CalendarWrapper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCalendar3 >   xC;

            DateTime            aEpochStart;        // 1Jan1970

public:
                                CalendarWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & rxContext
                                    );
                                ~CalendarWrapper();


    // wrapper implementations of XCalendar

    void loadDefaultCalendar( const ::com::sun::star::lang::Locale& rLocale );
    void loadCalendar( const OUString& rUniqueID, const ::com::sun::star::lang::Locale& rLocale );
    ::com::sun::star::uno::Sequence< OUString > getAllCalendars( const ::com::sun::star::lang::Locale& rLocale ) const;
    OUString getUniqueID() const;
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
    sal_Int16 getNumberOfMonthsInYear() const;
    sal_Int16 getNumberOfDaysInWeek() const;
    String getDisplayName( sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType ) const;

    /** Convenience method to get timezone offset in milliseconds, taking both
        fields ZONE_OFFSET and ZONE_OFFSET_SECOND_MILLIS into account. */
    sal_Int32 getZoneOffsetInMillis() const;
    /** Convenience method to get DST offset in milliseconds, taking both
        fields DST_OFFSET and DST_OFFSET_SECOND_MILLIS into account. */
    sal_Int32 getDSTOffsetInMillis() const;


    // wrapper implementations of XExtendedCalendar

    String getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) const;


    // wrapper implementations of XCalendar3

    ::com::sun::star::i18n::Calendar2 getLoadedCalendar() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > getDays() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > getMonths() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > getGenitiveMonths() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > getPartitiveMonths() const;


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
