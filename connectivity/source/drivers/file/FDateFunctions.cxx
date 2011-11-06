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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "file/FDateFunctions.hxx"
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>

using namespace connectivity;
using namespace connectivity::file;
//------------------------------------------------------------------
ORowSetValue OOp_DayOfWeek::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    sal_Int32 nRet = 0;
    ::com::sun::star::util::Date aD = lhs;
    Date aDate(aD.Day,aD.Month,aD.Year);
    DayOfWeek eDayOfWeek = aDate.GetDayOfWeek();
    switch(eDayOfWeek)
    {
        case MONDAY:
            nRet = 2;
            break;
        case TUESDAY:
            nRet = 3;
            break;
        case WEDNESDAY:
            nRet = 4;
            break;
        case THURSDAY:
            nRet = 5;
            break;
        case FRIDAY:
            nRet = 6;
            break;
        case SATURDAY:
            nRet = 7;
            break;
        case SUNDAY:
            nRet = 1;
            break;
        default:
            OSL_ENSURE(0,"Error in enum values for date");
    }
    return nRet;
}
//------------------------------------------------------------------
ORowSetValue OOp_DayOfMonth::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    ::com::sun::star::util::Date aD = lhs;
    return static_cast<sal_Int16>(aD.Day);
}
//------------------------------------------------------------------
ORowSetValue OOp_DayOfYear::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    ::com::sun::star::util::Date aD = lhs;
    Date aDate(aD.Day,aD.Month,aD.Year);
    return static_cast<sal_Int16>(aDate.GetDayOfYear());
}
//------------------------------------------------------------------
ORowSetValue OOp_Month::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    ::com::sun::star::util::Date aD = lhs;
    return static_cast<sal_Int16>(aD.Month);
}
//------------------------------------------------------------------
ORowSetValue OOp_DayName::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    ::rtl::OUString sRet;
    ::com::sun::star::util::Date aD = lhs;
    Date aDate(aD.Day,aD.Month,aD.Year);
    DayOfWeek eDayOfWeek = aDate.GetDayOfWeek();
    switch(eDayOfWeek)
    {
        case MONDAY:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Monday"));
            break;
        case TUESDAY:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Tuesday"));
            break;
        case WEDNESDAY:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Wednesday"));
            break;
        case THURSDAY:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Thursday"));
            break;
        case FRIDAY:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Friday"));
            break;
        case SATURDAY:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Saturday"));
            break;
        case SUNDAY:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Sunday"));
            break;
        default:
            OSL_ENSURE(0,"Error in enum values for date");
    }
    return sRet;
}
//------------------------------------------------------------------
ORowSetValue OOp_MonthName::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    ::rtl::OUString sRet;
    ::com::sun::star::util::Date aD = lhs;
    switch(aD.Month)
    {
        case 1:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("January"));
            break;
        case 2:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("February"));
            break;
        case 3:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("March"));
            break;
        case 4:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("April"));
            break;
        case 5:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("May"));
            break;
        case 6:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("June"));
            break;
        case 7:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("July"));
            break;
        case 8:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("August"));
            break;
        case 9:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("September"));
            break;
        case 10:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("October"));
            break;
        case 11:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("November"));
            break;
        case 12:
            sRet = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("December"));
            break;
    }
    return sRet;
}
//------------------------------------------------------------------
ORowSetValue OOp_Quarter::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    sal_Int32 nRet = 1;
    ::com::sun::star::util::Date aD = lhs;
    Date aDate(aD.Day,aD.Month,aD.Year);
    if ( aD.Month >= 4 && aD.Month < 7 )
        nRet = 2;
    else if ( aD.Month >= 7 && aD.Month < 10 )
        nRet = 3;
    else if ( aD.Month >= 10 && aD.Month <= 12 )
        nRet = 4;
    return nRet;
}
//------------------------------------------------------------------
ORowSetValue OOp_Week::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    if ( lhs.empty() || lhs.size() > 2 )
        return ORowSetValue();

    size_t nSize = lhs.size();

    ::com::sun::star::util::Date aD = lhs[nSize-1];
    Date aDate(aD.Day,aD.Month,aD.Year);

    sal_Int16 nStartDay = SUNDAY;
    if ( nSize == 2 && !lhs[0].isNull() )
        nStartDay = lhs[0];

    return static_cast<sal_Int16>(aDate.GetWeekOfYear(static_cast<DayOfWeek>(nStartDay)));
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Year::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    ::com::sun::star::util::Date aD = lhs;
    return static_cast<sal_Int16>(aD.Year);
}
//------------------------------------------------------------------
ORowSetValue OOp_Hour::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    ::com::sun::star::util::Time aT = lhs;
    return static_cast<sal_Int16>(aT.Hours);
}
//------------------------------------------------------------------
ORowSetValue OOp_Minute::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    ::com::sun::star::util::Time aT = lhs;
    return static_cast<sal_Int16>(aT.Minutes);
}
//------------------------------------------------------------------
ORowSetValue OOp_Second::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    ::com::sun::star::util::Time aT = lhs;
    return static_cast<sal_Int16>(aT.Seconds);
}
//------------------------------------------------------------------
ORowSetValue OOp_CurDate::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    if ( !lhs.empty() )
        return ORowSetValue();

    Date aCurDate;
    return ::com::sun::star::util::Date(aCurDate.GetDay(),aCurDate.GetMonth(),aCurDate.GetYear());
}
//------------------------------------------------------------------
ORowSetValue OOp_CurTime::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    if ( !lhs.empty() )
        return ORowSetValue();

    Time aCurTime;
    return ::com::sun::star::util::Time(aCurTime.Get100Sec(),aCurTime.GetSec(),aCurTime.GetMin(),aCurTime.GetHour());
}
//------------------------------------------------------------------
ORowSetValue OOp_Now::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    if ( !lhs.empty() )
        return ORowSetValue();

    DateTime aCurTime;
    return ::com::sun::star::util::DateTime(aCurTime.Get100Sec(),aCurTime.GetSec(),aCurTime.GetMin(),aCurTime.GetHour(),
                                            aCurTime.GetDay(),aCurTime.GetMonth(),aCurTime.GetYear());
}
//------------------------------------------------------------------
