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
#ifndef _I18N_CALENDAR_GREGORIAN_HXX_
#define _I18N_CALENDAR_GREGORIAN_HXX_

#include "calendarImpl.hxx"
#include "nativenumbersupplier.hxx"

#include "warnings_guard_unicode_calendar.h"

//  ----------------------------------------------------
//  class Calendar_gregorian
//  ----------------------------------------------------

namespace com { namespace sun { namespace star { namespace i18n {

const sal_uInt8 kDisplayEraForcedLongYear = 0x01;

struct Era {
    sal_Int32 year;
    sal_Int32 month;
    sal_Int32 day;
    sal_uInt8 flags;
};

const sal_Int16 FIELD_INDEX_COUNT = CalendarFieldIndex::FIELD_COUNT2;

class Calendar_gregorian : public CalendarImpl
{
public:

    // Constructors
    Calendar_gregorian();
    Calendar_gregorian(Era *_eraArray);
    void SAL_CALL init(Era *_eraArray);

    /**
    * Destructor
    */
    ~Calendar_gregorian();

    // Methods in XCalendar
    virtual void SAL_CALL loadCalendar(const rtl::OUString& uniqueID, const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDateTime(double nTimeInDays) throw(com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getDateTime() throw(com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setValue( sal_Int16 nFieldIndex, sal_Int16 nValue ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getValue(sal_Int16 nFieldIndex) throw(com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addValue(sal_Int16 nFieldIndex, sal_Int32 nAmount) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isValid() throw (com::sun::star::uno::RuntimeException);
    virtual Calendar SAL_CALL getLoadedCalendar() throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getUniqueID() throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getFirstDayOfWeek() throw(com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFirstDayOfWeek(sal_Int16 nDay) throw(com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMinimumNumberOfDaysForFirstWeek(sal_Int16 nDays) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getMinimumNumberOfDaysForFirstWeek() throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getNumberOfMonthsInYear() throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getNumberOfDaysInWeek() throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence < CalendarItem > SAL_CALL getMonths() throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence < CalendarItem > SAL_CALL getDays() throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getDisplayName(sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType) throw(com::sun::star::uno::RuntimeException);

    // Methods in XExtendedCalendar
    virtual rtl::OUString SAL_CALL getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) throw (com::sun::star::uno::RuntimeException);

    // XCalendar3
    virtual Calendar2 SAL_CALL getLoadedCalendar2() throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence < CalendarItem2 > SAL_CALL getDays2() throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence < CalendarItem2 > SAL_CALL getMonths2() throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence < CalendarItem2 > SAL_CALL getGenitiveMonths2() throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence < CalendarItem2 > SAL_CALL getPartitiveMonths2() throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence < rtl::OUString > SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException);

protected:
    Era *eraArray;
    icu::Calendar *body;
    NativeNumberSupplier aNatNum;
    const sal_Char* cCalendar;
    com::sun::star::lang::Locale aLocale;
    sal_uInt32 fieldSet;
    sal_Int16 fieldValue[FIELD_INDEX_COUNT];
    sal_Int16 fieldSetValue[FIELD_INDEX_COUNT];

    virtual void mapToGregorian() throw(com::sun::star::uno::RuntimeException);
    virtual void mapFromGregorian() throw(com::sun::star::uno::RuntimeException);
    void getValue() throw(com::sun::star::uno::RuntimeException);

    rtl::OUString getDisplayStringImpl( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode, bool bEraMode ) throw (com::sun::star::uno::RuntimeException);

private:
    Calendar2 aCalendar;

    /** Submit fieldSetValue array according to fieldSet. */
    void submitFields() throw(com::sun::star::uno::RuntimeException);
    /** Submit fieldSetValue array according to fieldSet, plus YMDhms if >=0,
        plus zone and DST if != 0 */
    void submitValues( sal_Int32 nYear, sal_Int32 nMonth, sal_Int32 nDay, sal_Int32 nHour, sal_Int32 nMinute, sal_Int32 nSecond, sal_Int32 nMilliSecond, sal_Int32 nZone, sal_Int32 nDST) throw(com::sun::star::uno::RuntimeException);
    /** Set fields internally. */
    void setValue() throw(com::sun::star::uno::RuntimeException);
    /** Obtain combined field values for timezone offset (minutes+secondmillis)
        in milliseconds and whether fields were set. */
    bool getZoneOffset( sal_Int32 & o_nOffset ) const;
    /** Obtain combined field values for DST offset (minutes+secondmillis) in
        milliseconds and whether fields were set. */
    bool getDSTOffset( sal_Int32 & o_nOffset ) const;
    /** Used by getZoneOffset() and getDSTOffset(). Parent is
        CalendarFieldIndex for offset in minutes, child is CalendarFieldIndex
        for offset in milliseconds. */
    bool getCombinedOffset( sal_Int32 & o_nOffset, sal_Int16 nParentFieldIndex, sal_Int16 nChildFieldIndex ) const;
};

//  ----------------------------------------------------
//  class Calendar_hanja
//  ----------------------------------------------------
class Calendar_hanja : public Calendar_gregorian
{
public:
    // Constructors
    Calendar_hanja();
    virtual void SAL_CALL loadCalendar(const rtl::OUString& uniqueID, const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getDisplayName(sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType) throw(com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class Calendar_gengou
//  ----------------------------------------------------
class Calendar_gengou : public Calendar_gregorian
{
public:
    // Constructors
    Calendar_gengou();
};

//  ----------------------------------------------------
//  class Calendar_ROC
//  ----------------------------------------------------
class Calendar_ROC : public Calendar_gregorian
{
public:
    // Constructors
    Calendar_ROC();
};

//  ----------------------------------------------------
//  class Calendar_buddhist
//  ----------------------------------------------------
class Calendar_buddhist : public Calendar_gregorian
{
public:
    // Constructors
    Calendar_buddhist();

    // Methods in XExtendedCalendar
    virtual rtl::OUString SAL_CALL getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) throw (com::sun::star::uno::RuntimeException);
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
