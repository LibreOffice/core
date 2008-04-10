/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: calendar_hijri.hxx,v $
 * $Revision: 1.7 $
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
#ifndef _I18N_CALENDAR_HIJRI_HXX_
#define _I18N_CALENDAR_HIJRI_HXX_

#include "calendar_gregorian.hxx"


//  ----------------------------------------------------
//  class Calendar_hijri
//  ----------------------------------------------------

namespace com { namespace sun { namespace star { namespace i18n {

class Calendar_hijri : public Calendar_gregorian
{
public:

    // Constructors
    Calendar_hijri();

protected:
    void SAL_CALL mapToGregorian() throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL mapFromGregorian() throw(com::sun::star::uno::RuntimeException);

    // radians per degree (pi/180)
    static const double RadPerDeg;

    // Synodic Period (mean time between 2 successive new moon: 29d, 12 hr, 44min, 3sec
    static const double SynPeriod;

    static const double SynMonth;           // Solar days in a year/SynPeriod

    // Julian day on Jan 1, 1900
    static const double jd1900;

    // Reference point: September 1984 25d 3h 10m UT. == 1405 Hijri == 1048 Synodial month from 1900
    static const sal_Int32 SynRef;
    static const sal_Int32 GregRef;

    // Local time (Saudi Arabia)
    static const double SA_TimeZone;        // Time Zone

    // Period between 1.30pm - 6:30pm
    static const double EveningPeriod;

    // "Leap" years
    static const sal_Int32 LeapYear[];

private:
    double NewMoon(sal_Int32 n);
    void getHijri(sal_Int32 *day, sal_Int32 *month, sal_Int32 *year);
    void ToGregorian(sal_Int32 *day, sal_Int32 *month, sal_Int32 *year);
    void getGregorianDay(sal_Int32 jd, sal_Int32 *pnDay, sal_Int32 *pnMonth, sal_Int32 *pnYear);
    double getJulianDay(sal_Int32 day, sal_Int32 month, sal_Int32 year);
};

} } } }

#endif
