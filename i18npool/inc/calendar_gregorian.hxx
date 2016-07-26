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
#ifndef INCLUDED_I18NPOOL_INC_CALENDAR_GREGORIAN_HXX
#define INCLUDED_I18NPOOL_INC_CALENDAR_GREGORIAN_HXX

#include "calendarImpl.hxx"
#include "nativenumbersupplier.hxx"

#include <unicode/calendar.h>
#include <rtl/ref.hxx>


//  class Calendar_gregorian


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
    Calendar_gregorian(const Era *_eraArray);
    void SAL_CALL init(const Era *_eraArray);

    /**
    * Destructor
    */
    virtual ~Calendar_gregorian();

    // Methods in XCalendar
    virtual void SAL_CALL loadCalendar(const OUString& uniqueID, const css::lang::Locale& rLocale) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDateTime(double fTimeInDays) throw(css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getDateTime() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setValue( sal_Int16 nFieldIndex, sal_Int16 nValue ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getValue(sal_Int16 nFieldIndex) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addValue(sal_Int16 nFieldIndex, sal_Int32 nAmount) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isValid() throw (css::uno::RuntimeException, std::exception) override;
    virtual Calendar SAL_CALL getLoadedCalendar() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getUniqueID() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getFirstDayOfWeek() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFirstDayOfWeek(sal_Int16 nDay) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMinimumNumberOfDaysForFirstWeek(sal_Int16 nDays) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getMinimumNumberOfDaysForFirstWeek() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getNumberOfMonthsInYear() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getNumberOfDaysInWeek() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence < CalendarItem > SAL_CALL getMonths() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence < CalendarItem > SAL_CALL getDays() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getDisplayName(sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType) throw(css::uno::RuntimeException, std::exception) override;

    // Methods in XExtendedCalendar
    virtual OUString SAL_CALL getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) throw (css::uno::RuntimeException, std::exception) override;

    // XCalendar3
    virtual Calendar2 SAL_CALL getLoadedCalendar2() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence < CalendarItem2 > SAL_CALL getDays2() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence < CalendarItem2 > SAL_CALL getMonths2() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence < CalendarItem2 > SAL_CALL getGenitiveMonths2() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence < CalendarItem2 > SAL_CALL getPartitiveMonths2() throw(css::uno::RuntimeException, std::exception) override;

    // XCalendar4
    virtual void SAL_CALL setLocalDateTime(double TimeInDays) throw(css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getLocalDateTime() throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence < OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

protected:
    const Era *eraArray;
    icu::Calendar *body;
    rtl::Reference<NativeNumberSupplierService> mxNatNum;
    const sal_Char* cCalendar;
    css::lang::Locale aLocale;
    sal_uInt32 fieldSet;
    sal_Int16 fieldValue[FIELD_INDEX_COUNT];
    sal_Int16 fieldSetValue[FIELD_INDEX_COUNT];

    virtual void mapToGregorian() throw(css::uno::RuntimeException);
    virtual void mapFromGregorian() throw(css::uno::RuntimeException);
    void getValue() throw(css::uno::RuntimeException);

    OUString getDisplayStringImpl( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode, bool bEraMode ) throw (css::uno::RuntimeException);

private:
    Calendar2 aCalendar;

    /** Submit fieldSetValue array according to fieldSet. */
    void submitFields() throw(css::uno::RuntimeException);
    /** Set fields internally. */
    void setValue() throw(css::uno::RuntimeException);
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


//  class Calendar_hanja

class Calendar_hanja : public Calendar_gregorian
{
public:
    // Constructors
    Calendar_hanja();
    virtual void SAL_CALL loadCalendar(const OUString& uniqueID, const css::lang::Locale& rLocale) throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getDisplayName(sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType) throw(css::uno::RuntimeException, std::exception) override;
};


//  class Calendar_gengou

class Calendar_gengou : public Calendar_gregorian
{
public:
    // Constructors
    Calendar_gengou();
};


//  class Calendar_ROC

class Calendar_ROC : public Calendar_gregorian
{
public:
    // Constructors
    Calendar_ROC();
};


//  class Calendar_buddhist

class Calendar_buddhist : public Calendar_gregorian
{
public:
    // Constructors
    Calendar_buddhist();

    // Methods in XExtendedCalendar
    virtual OUString SAL_CALL getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) throw (css::uno::RuntimeException, std::exception) override;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
