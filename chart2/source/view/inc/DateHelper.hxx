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
#ifndef _CHART2_DATEHELPER_HXX
#define _CHART2_DATEHELPER_HXX

#include <com/sun/star/chart2/XScaling.hpp>
#include <tools/date.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class DateHelper
{
public:
    static bool IsInSameYear( const Date& rD1, const Date& rD2 );
    static bool IsInSameMonth( const Date& rD1, const Date& rD2 );

    static long GetMonthsBetweenDates( Date aD1, Date aD2 );
    static Date GetDateSomeMonthsAway( const Date& rD, long nMonthDistance );
    static Date GetDateSomeYearsAway( const Date& rD, long nYearDistance );
    static bool IsLessThanOneMonthAway( const Date& rD1, const Date& rD2 );
    static bool IsLessThanOneYearAway( const Date& rD1, const Date& rD2 );

    static double RasterizeDateValue( double fValue, const Date& rNullDate, long TimeResolution );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
