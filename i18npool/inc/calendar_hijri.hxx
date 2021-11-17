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
#pragma once

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

private:
    static double NewMoon(sal_Int32 n);
    static void getHijri(sal_Int32 *day, sal_Int32 *month, sal_Int32 *year);
    static void ToGregorian(sal_Int32 *day, sal_Int32 *month, sal_Int32 *year);
    static void getGregorianDay(sal_Int32 jd, sal_Int32 *pnDay, sal_Int32 *pnMonth, sal_Int32 *pnYear);
    static sal_Int32 getJulianDay(sal_Int32 day, sal_Int32 month, sal_Int32 year);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
