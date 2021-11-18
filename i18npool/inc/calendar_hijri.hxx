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
#ifndef INCLUDED_I18NPOOL_INC_CALENDAR_HIJRI_HXX
#define INCLUDED_I18NPOOL_INC_CALENDAR_HIJRI_HXX

#include "calendar_gregorian.hxx"




namespace i18npool {

class Calendar_hijri final : public Calendar_gregorian
{
public:

    // Constructors
    Calendar_hijri();

private:
    void mapToGregorian() override;
    void mapFromGregorian() override;

    // Synodic Period (mean time between 2 successive new moon: 29d, 12 hr, 44min, 3sec
    inline static constexpr double SynPeriod = 29.53058868;

    inline static constexpr double SynMonth = 365.25/29.53058868;           // Solar days in a year/SynPeriod

    // Julian day on Jan 1, 1900
    inline static constexpr double jd1900 = 2415020.75933;

    // Reference point: September 1984 25d 3h 10m UT. == 1405 Hijri == 1048 Synodial month from 1900
    inline static constexpr sal_Int32 SynRef = 1252;
    inline static constexpr sal_Int32 GregRef = 1422;

    // Period between 1.30pm - 6:30pm
    inline static constexpr double EveningPeriod = 6.0;

    // "Leap" years
    static constexpr sal_Int32 LeapYear[] = {
        2, 5, 7, 10, 13, 16, 18, 21, 24, 26, 29
    };


private:
    static double NewMoon(sal_Int32 n);
    static void getHijri(sal_Int32 *day, sal_Int32 *month, sal_Int32 *year);
    static void ToGregorian(sal_Int32 *day, sal_Int32 *month, sal_Int32 *year);
    static void getGregorianDay(sal_Int32 jd, sal_Int32 *pnDay, sal_Int32 *pnMonth, sal_Int32 *pnYear);
    static sal_Int32 getJulianDay(sal_Int32 day, sal_Int32 month, sal_Int32 year);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
