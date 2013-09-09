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

#ifndef _SBDATE_HXX
#define _SBDATE_HXX

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>

bool implDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay, double& rdRet );
double implTimeSerial( sal_Int16 nHour, sal_Int16 nMinute, sal_Int16 nSecond);
bool implDateTimeSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay,
                         sal_Int16 nHour, sal_Int16 nMinute, sal_Int16 nSecond,
                         double& rdRet );

sal_Int16 implGetWeekDay( double aDate, bool bFirstDayParam = false, sal_Int16 nFirstDay = 0 );

sal_Int16 implGetDateYear( double aDate );
sal_Int16 implGetDateMonth( double aDate );
sal_Int16 implGetDateDay( double aDate );

sal_Int16 implGetHour( double dDate );
sal_Int16 implGetMinute( double dDate );
sal_Int16 implGetSecond( double dDate );

::com::sun::star::util::Date SbxDateToUNODate( const SbxValue* );
void SbxDateFromUNODate( SbxValue*, const ::com::sun::star::util::Date& );
::com::sun::star::util::Time SbxDateToUNOTime( const SbxValue* );
void SbxDateFromUNOTime( SbxValue*, const ::com::sun::star::util::Time& );
::com::sun::star::util::DateTime SbxDateToUNODateTime( const SbxValue* );
void SbxDateFromUNODateTime( SbxValue*, const ::com::sun::star::util::DateTime& );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
