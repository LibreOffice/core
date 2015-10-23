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

#ifndef INCLUDED_UNOTOOLS_CALENDARWRAPPER_HXX
#define INCLUDED_UNOTOOLS_CALENDARWRAPPER_HXX

#include <tools/datetime.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/i18n/Calendar2.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <unotools/unotoolsdllapi.h>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
    }
}}}

namespace com { namespace sun { namespace star {
    namespace i18n {
        class XCalendar4;
    }
}}}

class UNOTOOLS_DLLPUBLIC CalendarWrapper
{
    css::uno::Reference< css::i18n::XCalendar4 >   xC;

            DateTime            aEpochStart;        // 1Jan1970

public:
                                CalendarWrapper(
                                    const css::uno::Reference< css::uno::XComponentContext > & rxContext
                                    );
                                ~CalendarWrapper();

    // wrapper implementations of XCalendar

    void loadDefaultCalendar( const css::lang::Locale& rLocale );
    void loadCalendar( const OUString& rUniqueID, const css::lang::Locale& rLocale );
    css::uno::Sequence< OUString > getAllCalendars( const css::lang::Locale& rLocale ) const;
    OUString getUniqueID() const;
    /// set UTC date/time
    void setDateTime( double fTimeInDays );
    /// get UTC date/time
    double getDateTime() const;

    // For local setDateTime() and getDateTime() see further down at wrapper
    // implementations of XCalendar4.

    // wrapper implementations of XCalendar

    void setValue( sal_Int16 nFieldIndex, sal_Int16 nValue );
    bool isValid() const;
    sal_Int16 getValue( sal_Int16 nFieldIndex ) const;
    void addValue( sal_Int16 nFieldIndex, sal_Int32 nAmount );
    sal_Int16 getFirstDayOfWeek() const;
    sal_Int16 getNumberOfMonthsInYear() const;
    sal_Int16 getNumberOfDaysInWeek() const;
    OUString getDisplayName( sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType ) const;

    // wrapper implementations of XExtendedCalendar

    OUString getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) const;

    // wrapper implementations of XCalendar3

    css::i18n::Calendar2 getLoadedCalendar() const;
    css::uno::Sequence< css::i18n::CalendarItem2 > getDays() const;
    css::uno::Sequence< css::i18n::CalendarItem2 > getMonths() const;
    css::uno::Sequence< css::i18n::CalendarItem2 > getGenitiveMonths() const;
    css::uno::Sequence< css::i18n::CalendarItem2 > getPartitiveMonths() const;

    // wrapper implementations of XCalendar4

    /// set local date/time
    void setLocalDateTime( double fTimeInDays );
    /// get local date/time
    double getLocalDateTime() const;

    // convenience methods

    /// get epoch start (should be 01Jan1970)
    inline  const DateTime&     getEpochStart() const
                                    { return aEpochStart; }

    /// set a local (!) Gregorian DateTime
    inline  void                setGregorianDateTime( const DateTime& rDateTime )
                                    { setLocalDateTime( rDateTime - aEpochStart ); }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
