/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dputil.hxx"
#include "dpitemdata.hxx"
#include "dpnumgroupinfo.hxx"
#include "globalnames.hxx"
#include "globstr.hrc"

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
            return OUString::number(nValue);
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
            OUString aStr;
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

    return OUString("FIXME: unhandled value");
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

namespace {

sal_uInt16 nFuncStrIds[12] = {
    0,                              // SUBTOTAL_FUNC_NONE
    STR_FUN_TEXT_AVG,               // SUBTOTAL_FUNC_AVE
    STR_FUN_TEXT_COUNT,             // SUBTOTAL_FUNC_CNT
    STR_FUN_TEXT_COUNT,             // SUBTOTAL_FUNC_CNT2
    STR_FUN_TEXT_MAX,               // SUBTOTAL_FUNC_MAX
    STR_FUN_TEXT_MIN,               // SUBTOTAL_FUNC_MIN
    STR_FUN_TEXT_PRODUCT,           // SUBTOTAL_FUNC_PROD
    STR_FUN_TEXT_STDDEV,            // SUBTOTAL_FUNC_STD
    STR_FUN_TEXT_STDDEV,            // SUBTOTAL_FUNC_STDP
    STR_FUN_TEXT_SUM,               // SUBTOTAL_FUNC_SUM
    STR_FUN_TEXT_VAR,               // SUBTOTAL_FUNC_VAR
    STR_FUN_TEXT_VAR                // SUBTOTAL_FUNC_VARP
};

}

OUString ScDPUtil::getDisplayedMeasureName(const OUString& rName, ScSubTotalFunc eFunc)
{
    OUStringBuffer aRet;
    sal_uInt16 nId = nFuncStrIds[eFunc];
    if (nId)
    {
        aRet.append(ScGlobal::GetRscString(nId));        // function name
        aRet.appendAscii(RTL_CONSTASCII_STRINGPARAM(" - "));
    }
    aRet.append(rName);                   // field name

    return aRet.makeStringAndClear();
}

ScSubTotalFunc ScDPUtil::toSubTotalFunc(com::sun::star::sheet::GeneralFunction eGenFunc)
{
    ScSubTotalFunc eSubTotal;
    switch (eGenFunc)
    {
        case sheet::GeneralFunction_NONE:       eSubTotal = SUBTOTAL_FUNC_NONE; break;
        case sheet::GeneralFunction_SUM:        eSubTotal = SUBTOTAL_FUNC_SUM;  break;
        case sheet::GeneralFunction_COUNT:      eSubTotal = SUBTOTAL_FUNC_CNT2; break;
        case sheet::GeneralFunction_AVERAGE:    eSubTotal = SUBTOTAL_FUNC_AVE;  break;
        case sheet::GeneralFunction_MAX:        eSubTotal = SUBTOTAL_FUNC_MAX;  break;
        case sheet::GeneralFunction_MIN:        eSubTotal = SUBTOTAL_FUNC_MIN;  break;
        case sheet::GeneralFunction_PRODUCT:    eSubTotal = SUBTOTAL_FUNC_PROD; break;
        case sheet::GeneralFunction_COUNTNUMS:  eSubTotal = SUBTOTAL_FUNC_CNT;  break;
        case sheet::GeneralFunction_STDEV:      eSubTotal = SUBTOTAL_FUNC_STD;  break;
        case sheet::GeneralFunction_STDEVP:     eSubTotal = SUBTOTAL_FUNC_STDP; break;
        case sheet::GeneralFunction_VAR:        eSubTotal = SUBTOTAL_FUNC_VAR;  break;
        case sheet::GeneralFunction_VARP:       eSubTotal = SUBTOTAL_FUNC_VARP; break;
        case sheet::GeneralFunction_AUTO:
        default:
            eSubTotal = SUBTOTAL_FUNC_NONE;
    }
    return eSubTotal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
