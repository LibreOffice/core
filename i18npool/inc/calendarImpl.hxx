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
#ifndef INCLUDED_I18NPOOL_INC_CALENDARIMPL_HXX
#define INCLUDED_I18NPOOL_INC_CALENDARIMPL_HXX

#include <com/sun/star/i18n/XCalendar4.hpp>
#include <com/sun/star/i18n/CalendarDisplayCode.hpp>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vector>


//  class CalendarImpl


namespace com { namespace sun { namespace star { namespace i18n {

class CalendarImpl : public cppu::WeakImplHelper
<
    css::i18n::XCalendar4,
    css::lang::XServiceInfo
>
{
public:

    // Constructors
    CalendarImpl() {};
    CalendarImpl(const css::uno::Reference < css::uno::XComponentContext >& rxContext);

    /**
    * Destructor
    */
    virtual ~CalendarImpl();


    // Methods
    virtual void SAL_CALL loadDefaultCalendar(const css::lang::Locale& rLocale) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL loadCalendar(const OUString& uniqueID, const css::lang::Locale& rLocale) throw(css::uno::RuntimeException, std::exception) override;
    virtual Calendar SAL_CALL getLoadedCalendar() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence < OUString > SAL_CALL getAllCalendars(const css::lang::Locale& rLocale) throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getUniqueID() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDateTime(double fTimeInDays) throw(css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getDateTime() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setValue( sal_Int16 nFieldIndex, sal_Int16 nValue ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getValue(sal_Int16 nFieldIndex) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isValid() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addValue(sal_Int16 nFieldIndex, sal_Int32 nAmount) throw(css::uno::RuntimeException, std::exception) override;
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

private:
    struct lookupTableItem {
        lookupTableItem(const OUString& _uniqueID, css::uno::Reference < css::i18n::XCalendar4 >& _xCalendar)
            : uniqueID(_uniqueID), xCalendar(_xCalendar) {}
        OUString                                      uniqueID;
        css::uno::Reference < css::i18n::XCalendar4 > xCalendar;
    };
    std::vector<lookupTableItem*>                       lookupTable;
    css::uno::Reference < css::uno::XComponentContext > m_xContext;
    css::uno::Reference < css::i18n::XCalendar4 >       xCalendar;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
