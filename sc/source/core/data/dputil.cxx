/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "dputil.hxx"
#include "global.hxx"
#include "dpitemdata.hxx"
#include "dpnumgroupinfo.hxx"
#include "globalnames.hxx"

#include "comphelper/string.hxx"
#include "unotools/localedatawrapper.hxx"
#include "unotools/calendarwrapper.hxx"
#include "svl/zforlist.hxx"
#include "rtl/math.hxx"

#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>

using namespace com::sun::star;

namespace {

const sal_uInt16 SC_DP_LEAPYEAR = 1648;     // arbitrary leap year for date calculations

OUString getTwoDigitString(sal_Int32 nValue)
{
    OUString aRet = OUString::number( nValue );
    if ( aRet.getLength() < 2 )
        aRet = "0" + aRet;
    return aRet;
}

void appendDateStr(OUStringBuffer& rBuffer, double fValue, SvNumberFormatter* pFormatter)
{
    sal_uLong nFormat = pFormatter->GetStandardFormat( NUMBERFORMAT_DATE, ScGlobal::eLnge );
    OUString aString;
    pFormatter->GetInputLineString(fValue, nFormat, aString);
    rBuffer.append(aString);
}

OUString getSpecialDateName(double fValue, bool bFirst, SvNumberFormatter* pFormatter)
{
    OUStringBuffer aBuffer;
    aBuffer.append(sal_Unicode(bFirst ? '<' : '>'));
    appendDateStr(aBuffer, fValue, pFormatter);
    return aBuffer.makeStringAndClear();
}

}

bool ScDPUtil::isDuplicateDimension(const OUString& rName)
{
    if (rName.isEmpty())
        return false;

    sal_Unicode cLast = rName[rName.getLength()-1];
    return cLast == sal_Unicode('*');
}

OUString ScDPUtil::getSourceDimensionName(const OUString& rName)
{
    return comphelper::string::stripEnd(rName, '*');
}

OUString ScDPUtil::createDuplicateDimensionName(const OUString& rOriginal, size_t nDupCount)
{
    if (!nDupCount)
        return rOriginal;

    OUStringBuffer aBuf(rOriginal);
    for (size_t i = 0; i < nDupCount; ++i)
        aBuf.append(sal_Unicode('*'));

    return aBuf.makeStringAndClear();
}

OUString ScDPUtil::getDateGroupName(
        sal_Int32 nDatePart, sal_Int32 nValue, SvNumberFormatter* pFormatter,
        double fStart, double fEnd)
{
    if (nValue == ScDPItemData::DateFirst)
        return getSpecialDateName(fStart, true, pFormatter);
    if (nValue == ScDPItemData::DateLast)
        return getSpecialDateName(fEnd, false, pFormatter);

    switch ( nDatePart )
    {
        case sheet::DataPilotFieldGroupBy::YEARS:
            return OUString::valueOf(nValue);
        case sheet::DataPilotFieldGroupBy::QUARTERS:
            return ScGlobal::pLocaleData->getQuarterAbbreviation(sal_Int16(nValue-1));    // nValue is 1-based
        case com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS:
            return ScGlobal::GetCalendar()->getDisplayName(
                        i18n::CalendarDisplayIndex::MONTH, sal_Int16(nValue-1), 0);    // 0-based, get short name
        case sheet::DataPilotFieldGroupBy::DAYS:
        {
            Date aDate(1, 1, SC_DP_LEAPYEAR);
            aDate += (nValue - 1);            // nValue is 1-based
            Date aNullDate = *pFormatter->GetNullDate();
            long nDays = aDate - aNullDate;

            sal_uLong nFormat = pFormatter->GetFormatIndex(NF_DATE_SYS_DDMMM, ScGlobal::eLnge);
            Color* pColor;
            String aStr;
            pFormatter->GetOutputString(nDays, nFormat, aStr, &pColor);
            return aStr;
        }
        case sheet::DataPilotFieldGroupBy::HOURS:
        {
            //! allow am/pm format?
            return getTwoDigitString(nValue);
        }
        break;
        case sheet::DataPilotFieldGroupBy::MINUTES:
        case sheet::DataPilotFieldGroupBy::SECONDS:
        {
            OUStringBuffer aBuf(ScGlobal::pLocaleData->getTimeSep());
            aBuf.append(getTwoDigitString(nValue));
            return aBuf.makeStringAndClear();
        }
        break;
        default:
            OSL_FAIL("invalid date part");
    }

    return OUString::createFromAscii("FIXME: unhandled value");
}

double ScDPUtil::getNumGroupStartValue(double fValue, const ScDPNumGroupInfo& rInfo)
{
    if (fValue < rInfo.mfStart && !rtl::math::approxEqual(fValue, rInfo.mfStart))
    {
        rtl::math::setInf(&fValue, true);
        return fValue;
    }

    if (fValue > rInfo.mfEnd && !rtl::math::approxEqual(fValue, rInfo.mfEnd))
    {
        rtl::math::setInf(&fValue, false);
        return fValue;
    }

    double fDiff = fValue - rInfo.mfStart;
    double fDiv = rtl::math::approxFloor( fDiff / rInfo.mfStep );
    double fGroupStart = rInfo.mfStart + fDiv * rInfo.mfStep;

    if (rtl::math::approxEqual(fGroupStart, rInfo.mfEnd) &&
        !rtl::math::approxEqual(fGroupStart, rInfo.mfStart))
    {
        if (!rInfo.mbDateValues)
        {
            // A group that would consist only of the end value is not
            // created, instead the value is included in the last group
            // before. So the previous group is used if the calculated group
            // start value is the selected end value.

            fDiv -= 1.0;
            return rInfo.mfStart + fDiv * rInfo.mfStep;
        }

        // For date values, the end value is instead treated as above the
        // limit if it would be a group of its own.

        return rInfo.mfEnd + rInfo.mfStep;
    }

    return fGroupStart;
}

namespace {

void lcl_AppendDateStr( OUStringBuffer& rBuffer, double fValue, SvNumberFormatter* pFormatter )
{
    sal_uLong nFormat = pFormatter->GetStandardFormat( NUMBERFORMAT_DATE, ScGlobal::eLnge );
    OUString aString;
    pFormatter->GetInputLineString( fValue, nFormat, aString );
    rBuffer.append( aString );
}

OUString lcl_GetSpecialNumGroupName( double fValue, bool bFirst, sal_Unicode cDecSeparator,
    bool bDateValues, SvNumberFormatter* pFormatter )
{
    OSL_ENSURE( cDecSeparator != 0, "cDecSeparator not initialized" );

    OUStringBuffer aBuffer;
    aBuffer.append((sal_Unicode)( bFirst ? '<' : '>' ));
    if ( bDateValues )
        lcl_AppendDateStr( aBuffer, fValue, pFormatter );
    else
        rtl::math::doubleToUStringBuffer( aBuffer, fValue, rtl_math_StringFormat_Automatic,
        rtl_math_DecimalPlaces_Max, cDecSeparator, true );
    return aBuffer.makeStringAndClear();
}

OUString lcl_GetNumGroupName(
    double fStartValue, const ScDPNumGroupInfo& rInfo, sal_Unicode cDecSep,
    SvNumberFormatter* pFormatter)
{
    OSL_ENSURE( cDecSep != 0, "cDecSeparator not initialized" );

    double fStep = rInfo.mfStep;
    double fEndValue = fStartValue + fStep;
    if (rInfo.mbIntegerOnly && (rInfo.mbDateValues || !rtl::math::approxEqual(fEndValue, rInfo.mfEnd)))
    {
        //  The second number of the group label is
        //  (first number + size - 1) if there are only integer numbers,
        //  (first number + size) if any non-integer numbers are involved.
        //  Exception: The last group (containing the end value) is always
        //  shown as including the end value (but not for dates).

        fEndValue -= 1.0;
    }

    if ( fEndValue > rInfo.mfEnd && !rInfo.mbAutoEnd )
    {
        // limit the last group to the end value

        fEndValue = rInfo.mfEnd;
    }

    OUStringBuffer aBuffer;
    if ( rInfo.mbDateValues )
    {
        lcl_AppendDateStr( aBuffer, fStartValue, pFormatter );
        aBuffer.appendAscii( " - " );   // with spaces
        lcl_AppendDateStr( aBuffer, fEndValue, pFormatter );
    }
    else
    {
        rtl::math::doubleToUStringBuffer( aBuffer, fStartValue, rtl_math_StringFormat_Automatic,
            rtl_math_DecimalPlaces_Max, cDecSep, true );
        aBuffer.append( (sal_Unicode) '-' );
        rtl::math::doubleToUStringBuffer( aBuffer, fEndValue, rtl_math_StringFormat_Automatic,
            rtl_math_DecimalPlaces_Max, cDecSep, true );
    }

    return aBuffer.makeStringAndClear();
}

}

OUString ScDPUtil::getNumGroupName(
    double fValue, const ScDPNumGroupInfo& rInfo, sal_Unicode cDecSep, SvNumberFormatter* pFormatter)
{
    if ( fValue < rInfo.mfStart && !rtl::math::approxEqual( fValue, rInfo.mfStart ) )
        return lcl_GetSpecialNumGroupName( rInfo.mfStart, true, cDecSep, rInfo.mbDateValues, pFormatter );

    if ( fValue > rInfo.mfEnd && !rtl::math::approxEqual( fValue, rInfo.mfEnd ) )
        return lcl_GetSpecialNumGroupName( rInfo.mfEnd, false, cDecSep, rInfo.mbDateValues, pFormatter );

    double fDiff = fValue - rInfo.mfStart;
    double fDiv = rtl::math::approxFloor( fDiff / rInfo.mfStep );
    double fGroupStart = rInfo.mfStart + fDiv * rInfo.mfStep;

    if ( rtl::math::approxEqual( fGroupStart, rInfo.mfEnd ) &&
        !rtl::math::approxEqual( fGroupStart, rInfo.mfStart ) )
    {
        if (rInfo.mbDateValues)
        {
            //  For date values, the end value is instead treated as above the limit
            //  if it would be a group of its own.
            return lcl_GetSpecialNumGroupName( rInfo.mfEnd, false, cDecSep, rInfo.mbDateValues, pFormatter );
        }
    }

    return lcl_GetNumGroupName(fGroupStart, rInfo, cDecSep, pFormatter);
}

sal_Int32 ScDPUtil::getDatePartValue(
    double fValue, const ScDPNumGroupInfo& rInfo, sal_Int32 nDatePart,
    SvNumberFormatter* pFormatter)
{
    // Start and end are inclusive
    // (End date without a time value is included, with a time value it's not)

    if (fValue < rInfo.mfStart && !rtl::math::approxEqual(fValue, rInfo.mfStart))
        return ScDPItemData::DateFirst;
    if (fValue > rInfo.mfEnd && !rtl::math::approxEqual(fValue, rInfo.mfEnd))
        return ScDPItemData::DateLast;

    sal_Int32 nResult = 0;

    if (nDatePart == sheet::DataPilotFieldGroupBy::HOURS ||
        nDatePart == sheet::DataPilotFieldGroupBy::MINUTES ||
        nDatePart == sheet::DataPilotFieldGroupBy::SECONDS)
    {
        // handle time
        // (as in the cell functions, ScInterpreter::ScGetHour etc.: seconds are rounded)

        double fTime = fValue - rtl::math::approxFloor(fValue);
        long nSeconds = (long)rtl::math::approxFloor(fTime*DATE_TIME_FACTOR+0.5);

        switch (nDatePart)
        {
            case sheet::DataPilotFieldGroupBy::HOURS:
                nResult = nSeconds / 3600;
                break;
            case sheet::DataPilotFieldGroupBy::MINUTES:
                nResult = ( nSeconds % 3600 ) / 60;
                break;
            case sheet::DataPilotFieldGroupBy::SECONDS:
                nResult = nSeconds % 60;
                break;
        }
    }
    else
    {
        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long)::rtl::math::approxFloor(fValue);

        switch ( nDatePart )
        {
            case com::sun::star::sheet::DataPilotFieldGroupBy::YEARS:
                nResult = aDate.GetYear();
                break;
            case com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS:
                nResult = 1 + (aDate.GetMonth() - 1) / 3;     // 1..4
                break;
            case com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS:
                nResult = aDate.GetMonth();     // 1..12
                break;
            case com::sun::star::sheet::DataPilotFieldGroupBy::DAYS:
                {
                    Date aYearStart(1, 1, aDate.GetYear());
                    nResult = (aDate - aYearStart) + 1;       // Jan 01 has value 1
                    if (nResult >= 60 && !aDate.IsLeapYear())
                    {
                        // days are counted from 1 to 366 - if not from a leap year, adjust
                        ++nResult;
                    }
                }
                break;
            default:
                OSL_FAIL("invalid date part");
        }
    }

    return nResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
