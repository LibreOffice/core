/*************************************************************************
 *
 *  $RCSfile: FDateFunctions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:24:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "file/FDateFunctions.hxx"
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

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
