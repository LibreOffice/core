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
#ifndef _I18N_CALENDARIMPL_HXX_
#define _I18N_CALENDARIMPL_HXX_

#include <com/sun/star/i18n/XExtendedCalendar.hpp>
#include <com/sun/star/i18n/CalendarDisplayCode.hpp>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <vector>

//  ----------------------------------------------------
//  class CalendarImpl
//  ----------------------------------------------------

namespace com { namespace sun { namespace star { namespace i18n {

class CalendarImpl : public cppu::WeakImplHelper2
<
    com::sun::star::i18n::XExtendedCalendar,
    com::sun::star::lang::XServiceInfo
>
{
public:

    // Constructors
    CalendarImpl() {};
    CalendarImpl(const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF);

    /**
    * Destructor
    */
    ~CalendarImpl();


    // Methods
    virtual void SAL_CALL loadDefaultCalendar(const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL loadCalendar(const rtl::OUString& uniqueID, const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException);
    virtual Calendar SAL_CALL getLoadedCalendar() throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence < rtl::OUString > SAL_CALL getAllCalendars(const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getUniqueID() throw(com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDateTime(double nTimeInDays) throw(com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getDateTime() throw(com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setValue( sal_Int16 nFieldIndex, sal_Int16 nValue ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getValue(sal_Int16 nFieldIndex) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isValid() throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addValue(sal_Int16 nFieldIndex, sal_Int32 nAmount) throw(com::sun::star::uno::RuntimeException);
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

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence < rtl::OUString > SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException);

private:
    struct lookupTableItem {
    lookupTableItem(const rtl::OUString& _uniqueID, com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedCalendar >& _xCalendar) :
        uniqueID(_uniqueID), xCalendar(_xCalendar) {}
    rtl::OUString uniqueID;
    com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedCalendar > xCalendar;
    };
    std::vector<lookupTableItem*> lookupTable;
    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;
    com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedCalendar > xCalendar;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
