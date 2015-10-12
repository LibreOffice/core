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
    com::sun::star::i18n::XCalendar4,
    com::sun::star::lang::XServiceInfo
>
{
public:

    // Constructors
    CalendarImpl() {};
    CalendarImpl(const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext);

    /**
    * Destructor
    */
    virtual ~CalendarImpl();


    // Methods
    virtual void SAL_CALL loadDefaultCalendar(const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL loadCalendar(const OUString& uniqueID, const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual Calendar SAL_CALL getLoadedCalendar() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence < OUString > SAL_CALL getAllCalendars(const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getUniqueID() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDateTime(double fTimeInDays) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getDateTime() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setValue( sal_Int16 nFieldIndex, sal_Int16 nValue ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getValue(sal_Int16 nFieldIndex) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isValid() throw (com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addValue(sal_Int16 nFieldIndex, sal_Int32 nAmount) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getFirstDayOfWeek() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFirstDayOfWeek(sal_Int16 nDay) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMinimumNumberOfDaysForFirstWeek(sal_Int16 nDays) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getMinimumNumberOfDaysForFirstWeek() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getNumberOfMonthsInYear() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getNumberOfDaysInWeek() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence < CalendarItem > SAL_CALL getMonths() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence < CalendarItem > SAL_CALL getDays() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getDisplayName(sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType) throw(com::sun::star::uno::RuntimeException, std::exception) override;

    // Methods in XExtendedCalendar
    virtual OUString SAL_CALL getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) throw (com::sun::star::uno::RuntimeException, std::exception) override;

    // XCalendar3
    virtual Calendar2 SAL_CALL getLoadedCalendar2() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence < CalendarItem2 > SAL_CALL getDays2() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence < CalendarItem2 > SAL_CALL getMonths2() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence < CalendarItem2 > SAL_CALL getGenitiveMonths2() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence < CalendarItem2 > SAL_CALL getPartitiveMonths2() throw(com::sun::star::uno::RuntimeException, std::exception) override;

    // XCalendar4
    virtual void SAL_CALL setLocalDateTime(double TimeInDays) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getLocalDateTime() throw(com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence < OUString > SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException, std::exception) override;

private:
    struct lookupTableItem {
    lookupTableItem(const OUString& _uniqueID, com::sun::star::uno::Reference < com::sun::star::i18n::XCalendar4 >& _xCalendar) :
        uniqueID(_uniqueID), xCalendar(_xCalendar) {}
    OUString uniqueID;
    com::sun::star::uno::Reference < com::sun::star::i18n::XCalendar4 > xCalendar;
    };
    std::vector<lookupTableItem*> lookupTable;
    com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_xContext;
    com::sun::star::uno::Reference < com::sun::star::i18n::XCalendar4 > xCalendar;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
