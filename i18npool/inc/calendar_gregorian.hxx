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

#include <memory>


//  class Calendar_gregorian


namespace i18npool {

const sal_uInt8 kDisplayEraForcedLongYear = 0x01;

struct Era {
    sal_Int32 year;
    sal_Int32 month;
    sal_Int32 day;
    sal_uInt8 flags;
};

const sal_Int16 FIELD_INDEX_COUNT = css::i18n::CalendarFieldIndex::FIELD_COUNT2;

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
    virtual ~Calendar_gregorian() override;

    // Methods in XCalendar
    virtual void SAL_CALL loadCalendar(const OUString& uniqueID, const css::lang::Locale& rLocale) override;
    virtual void SAL_CALL setDateTime(double fTimeInDays) override;
    virtual double SAL_CALL getDateTime() override;
    virtual void SAL_CALL setValue( sal_Int16 nFieldIndex, sal_Int16 nValue ) override;
    virtual sal_Int16 SAL_CALL getValue(sal_Int16 nFieldIndex) override;
    virtual void SAL_CALL addValue(sal_Int16 nFieldIndex, sal_Int32 nAmount) override;
    virtual sal_Bool SAL_CALL isValid() override;
    virtual css::i18n:: Calendar SAL_CALL getLoadedCalendar() override;
    virtual OUString SAL_CALL getUniqueID() override;
    virtual sal_Int16 SAL_CALL getFirstDayOfWeek() override;
    virtual void SAL_CALL setFirstDayOfWeek(sal_Int16 nDay) override;
    virtual void SAL_CALL setMinimumNumberOfDaysForFirstWeek(sal_Int16 nDays) override;
    virtual sal_Int16 SAL_CALL getMinimumNumberOfDaysForFirstWeek() override;
    virtual sal_Int16 SAL_CALL getNumberOfMonthsInYear() override;
    virtual sal_Int16 SAL_CALL getNumberOfDaysInWeek() override;
    virtual css::uno::Sequence < css::i18n::CalendarItem > SAL_CALL getMonths() override;
    virtual css::uno::Sequence < css::i18n::CalendarItem > SAL_CALL getDays() override;
    virtual OUString SAL_CALL getDisplayName(sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType) override;

    // Methods in XExtendedCalendar
    virtual OUString SAL_CALL getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) override;

    // XCalendar3
    virtual css::i18n::Calendar2 SAL_CALL getLoadedCalendar2() override;
    virtual css::uno::Sequence < css::i18n::CalendarItem2 > SAL_CALL getDays2() override;
    virtual css::uno::Sequence < css::i18n::CalendarItem2 > SAL_CALL getMonths2() override;
    virtual css::uno::Sequence < css::i18n::CalendarItem2 > SAL_CALL getGenitiveMonths2() override;
    virtual css::uno::Sequence < css::i18n::CalendarItem2 > SAL_CALL getPartitiveMonths2() override;

    // XCalendar4
    virtual void SAL_CALL setLocalDateTime(double TimeInDays) override;
    virtual double SAL_CALL getLocalDateTime() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence < OUString > SAL_CALL getSupportedServiceNames() override;

protected:
    const Era *eraArray;
    std::unique_ptr<icu::Calendar> body;
    rtl::Reference<NativeNumberSupplierService> mxNatNum;
    const sal_Char* cCalendar;
    css::lang::Locale aLocale;
    sal_uInt32 fieldSet;
    sal_Int16 fieldValue[FIELD_INDEX_COUNT];
    sal_Int16 fieldSetValue[FIELD_INDEX_COUNT];

    /// @throws css::uno::RuntimeException
    virtual void mapToGregorian();
    /// @throws css::uno::RuntimeException
    virtual void mapFromGregorian();
    /// @throws css::uno::RuntimeException
    void getValue();

    /// @throws css::uno::RuntimeException
    OUString getDisplayStringImpl( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode, bool bEraMode );

private:
    css::i18n::Calendar2 aCalendar;

    /** Submit fieldSetValue array according to fieldSet.

        @throws css::uno::RuntimeException
    */
    void submitFields();
    /** Set fields internally.

        @throws css::uno::RuntimeException
    */
    void setValue();
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
    virtual void SAL_CALL loadCalendar(const OUString& uniqueID, const css::lang::Locale& rLocale) override;
    virtual OUString SAL_CALL getDisplayName(sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType) override;
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
    virtual OUString SAL_CALL getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
