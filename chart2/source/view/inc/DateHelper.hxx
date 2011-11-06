/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
