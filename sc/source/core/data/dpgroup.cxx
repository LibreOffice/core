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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>

#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <unotools/localedatawrapper.hxx>
#include <svl/zforlist.hxx>

#include "dpgroup.hxx"
#include "collect.hxx"
#include "global.hxx"
#include "document.hxx"
#include "dpcachetable.hxx"
#include "dptabsrc.hxx"
#include "dptabres.hxx"
#include "dpobject.hxx"
#include "dpglobal.hxx"

#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::rtl::OUString;
using ::rtl::OUStringHash;

using ::std::vector;
using ::boost::shared_ptr;

#define D_TIMEFACTOR              86400.0

const USHORT SC_DP_LEAPYEAR = 1648;     // arbitrary leap year for date calculations

// part values for the extra "<" and ">" entries (same for all parts)
const sal_Int32 SC_DP_DATE_FIRST = -1;
const sal_Int32 SC_DP_DATE_LAST = 10000;

// ============================================================================
namespace
{
    BOOL lcl_Search( SCCOL nSourceDim, const ScDPTableDataCache* pCache , const std::vector< SCROW >& vIdx, SCROW nNew , SCROW& rIndex)
    {
        rIndex = vIdx.size();
        BOOL bFound = FALSE;
        SCROW nLo = 0;
        SCROW nHi = vIdx.size() - 1;
        SCROW nIndex;
        long nCompare;
        while (nLo <= nHi)
        {
            nIndex = (nLo + nHi) / 2;

            const ScDPItemData* pData  = pCache->GetItemDataById( nSourceDim, vIdx[nIndex] );
            const ScDPItemData* pDataInsert = pCache->GetItemDataById( nSourceDim, nNew );

            nCompare = ScDPItemData::Compare( *pData, *pDataInsert );
            if (nCompare < 0)
                nLo = nIndex + 1;
            else
            {
                nHi = nIndex - 1;
                if (nCompare == 0)
                {
                    bFound = TRUE;
                    nLo = nIndex;
                }
            }
        }
        rIndex = nLo;
        return bFound;
    }

    void  lcl_Insert( SCCOL nSourceDim, const ScDPTableDataCache* pCache ,  std::vector< SCROW >& vIdx, SCROW nNew )
    {
        SCROW nIndex = 0;
        if ( !lcl_Search( nSourceDim, pCache, vIdx, nNew ,nIndex ) )
            vIdx.insert( vIdx.begin()+nIndex, nNew  );
    }

    template<bool bUpdateData>
    SCROW lcl_InsertValue(SCCOL nSourceDim, const ScDPTableDataCache* pCache, std::vector<SCROW>& vIdx, const ScDPItemData & rData);

    template<>
    SCROW lcl_InsertValue<false>(SCCOL nSourceDim, const ScDPTableDataCache* pCache, std::vector<SCROW>& vIdx, const ScDPItemData & rData)
    {
        SCROW nNewID = pCache->GetAdditionalItemID(rData);
        lcl_Insert(nSourceDim, pCache, vIdx, nNewID);
        return nNewID;
    }

    template<>
    SCROW lcl_InsertValue<true>(SCCOL nSourceDim, const ScDPTableDataCache* pCache, std::vector<SCROW>& vIdx, const ScDPItemData & rData)
    {
        SCROW nItemId = lcl_InsertValue<false>( nSourceDim, pCache, vIdx, rData );

        if( const ScDPItemData *pData = pCache->GetItemDataById( nSourceDim, nItemId ) )
            const_cast<ScDPItemData&>(*pData) = rData;

        return nItemId;
    }

    template<bool bUpdateData>
    void lcl_InsertValue ( SCCOL nSourceDim, const ScDPTableDataCache* pCache,  std::vector< SCROW >& vIdx, const String&  rString, const double& fValue )
    {
        lcl_InsertValue<bUpdateData>( nSourceDim, pCache, vIdx, ScDPItemData( rString, fValue, TRUE ) );
    }

    template<bool bUpdateData>
    void lcl_InsertValue ( SCCOL nSourceDim, const ScDPTableDataCache* pCache, std::vector< SCROW >& vIdx, const String&  rString, const double& fValue, sal_Int32 nDatePart )
    {
        lcl_InsertValue<bUpdateData>( nSourceDim, pCache, vIdx, ScDPItemData( nDatePart, rString, fValue, ScDPItemData::MK_DATA|ScDPItemData::MK_VAL|ScDPItemData::MK_DATEPART ) );
    }

    void lcl_AppendDateStr( rtl::OUStringBuffer& rBuffer, double fValue, SvNumberFormatter* pFormatter )
    {
        ULONG nFormat = pFormatter->GetStandardFormat( NUMBERFORMAT_DATE, ScGlobal::eLnge );
        String aString;
        pFormatter->GetInputLineString( fValue, nFormat, aString );
        rBuffer.append( aString );
    }

    String lcl_GetNumGroupName( double fStartValue, const ScDPNumGroupInfo& rInfo,
        bool bHasNonInteger, sal_Unicode cDecSeparator, SvNumberFormatter* pFormatter )
    {
        DBG_ASSERT( cDecSeparator != 0, "cDecSeparator not initialized" );

        double fStep = rInfo.Step;
        double fEndValue = fStartValue + fStep;
        if ( !bHasNonInteger && ( rInfo.DateValues || !rtl::math::approxEqual( fEndValue, rInfo.End ) ) )
        {
            //  The second number of the group label is
            //  (first number + size - 1) if there are only integer numbers,
            //  (first number + size) if any non-integer numbers are involved.
            //  Exception: The last group (containing the end value) is always
            //  shown as including the end value (but not for dates).

            fEndValue -= 1.0;
        }

        if ( fEndValue > rInfo.End && !rInfo.AutoEnd )
        {
            // limit the last group to the end value

            fEndValue = rInfo.End;
        }

        rtl::OUStringBuffer aBuffer;
        if ( rInfo.DateValues )
        {
            lcl_AppendDateStr( aBuffer, fStartValue, pFormatter );
            aBuffer.appendAscii( " - " );   // with spaces
            lcl_AppendDateStr( aBuffer, fEndValue, pFormatter );
        }
        else
        {
            rtl::math::doubleToUStringBuffer( aBuffer, fStartValue, rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, cDecSeparator, true );
            aBuffer.append( (sal_Unicode) '-' );
            rtl::math::doubleToUStringBuffer( aBuffer, fEndValue, rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, cDecSeparator, true );
        }

        return aBuffer.makeStringAndClear();
    }

    String lcl_GetSpecialNumGroupName( double fValue, bool bFirst, sal_Unicode cDecSeparator,
        bool bDateValues, SvNumberFormatter* pFormatter )
    {
        DBG_ASSERT( cDecSeparator != 0, "cDecSeparator not initialized" );

        rtl::OUStringBuffer aBuffer;
        aBuffer.append((sal_Unicode)( bFirst ? '<' : '>' ));
        if ( bDateValues )
            lcl_AppendDateStr( aBuffer, fValue, pFormatter );
        else
            rtl::math::doubleToUStringBuffer( aBuffer, fValue, rtl_math_StringFormat_Automatic,
            rtl_math_DecimalPlaces_Max, cDecSeparator, true );
        return aBuffer.makeStringAndClear();
    }

    inline bool IsInteger( double fValue )
    {
        return rtl::math::approxEqual( fValue, rtl::math::approxFloor(fValue) );
    }

    String lcl_GetNumGroupForValue( double fValue, const ScDPNumGroupInfo& rInfo, bool bHasNonInteger,
        sal_Unicode cDecSeparator, double& rGroupValue, ScDocument* pDoc )
    {
        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

        if ( fValue < rInfo.Start && !rtl::math::approxEqual( fValue, rInfo.Start ) )
        {
            rGroupValue = rInfo.Start - rInfo.Step;
            return lcl_GetSpecialNumGroupName( rInfo.Start, true, cDecSeparator, rInfo.DateValues, pFormatter );
        }

        if ( fValue > rInfo.End && !rtl::math::approxEqual( fValue, rInfo.End ) )
        {
            rGroupValue = rInfo.End + rInfo.Step;
            return lcl_GetSpecialNumGroupName( rInfo.End, false, cDecSeparator, rInfo.DateValues, pFormatter );
        }

        double fDiff = fValue - rInfo.Start;
        double fDiv = rtl::math::approxFloor( fDiff / rInfo.Step );
        double fGroupStart = rInfo.Start + fDiv * rInfo.Step;

        if ( rtl::math::approxEqual( fGroupStart, rInfo.End ) &&
            !rtl::math::approxEqual( fGroupStart, rInfo.Start ) )
        {
            if ( !rInfo.DateValues )
            {
                //  A group that would consist only of the end value is not created,
                //  instead the value is included in the last group before. So the
                //  previous group is used if the calculated group start value is the
                //  selected end value.

                fDiv -= 1.0;
                fGroupStart = rInfo.Start + fDiv * rInfo.Step;
            }
            else
            {
                //  For date values, the end value is instead treated as above the limit
                //  if it would be a group of its own.

                rGroupValue = rInfo.End + rInfo.Step;
                return lcl_GetSpecialNumGroupName( rInfo.End, false, cDecSeparator, rInfo.DateValues, pFormatter );
            }
        }

        rGroupValue = fGroupStart;

        return lcl_GetNumGroupName( fGroupStart, rInfo, bHasNonInteger, cDecSeparator, pFormatter );
    }
}

class ScDPGroupDateFilter : public ScDPCacheTable::FilterBase
{
public:
    ScDPGroupDateFilter(double fMatchValue, sal_Int32 nDatePart,
                        const Date* pNullDate, const ScDPNumGroupInfo* pNumInfo);

    virtual bool match(const ScDPItemData & rCellData) const;

private:
    ScDPGroupDateFilter(); // disabled

    const Date*             mpNullDate;
    const ScDPNumGroupInfo* mpNumInfo;
    double                  mfMatchValue;
    sal_Int32               mnDatePart;
};

// ----------------------------------------------------------------------------

ScDPGroupDateFilter::ScDPGroupDateFilter(double fMatchValue, sal_Int32 nDatePart,
                                 const Date* pNullDate, const ScDPNumGroupInfo* pNumInfo) :
    mpNullDate(pNullDate),
    mpNumInfo(pNumInfo),
    mfMatchValue(fMatchValue),
    mnDatePart(nDatePart)
{

}
bool ScDPGroupDateFilter::match( const ScDPItemData & rCellData ) const
{
    using namespace ::com::sun::star::sheet;
    using ::rtl::math::approxFloor;
    using ::rtl::math::approxEqual;

    if ( !rCellData.IsValue() )
        return false;

    if (!mpNumInfo)
        return false;

    // Start and end dates are inclusive.  (An end date without a time value
    // is included, while an end date with a time value is not.)

    if ( rCellData.GetValue() < mpNumInfo->Start && !approxEqual(rCellData.GetValue(), mpNumInfo->Start) )
        return static_cast<sal_Int32>(mfMatchValue) == SC_DP_DATE_FIRST;

    if ( rCellData.GetValue() > mpNumInfo->End && !approxEqual(rCellData.GetValue(), mpNumInfo->End) )
        return static_cast<sal_Int32>(mfMatchValue) == SC_DP_DATE_LAST;

    if (mnDatePart == DataPilotFieldGroupBy::HOURS || mnDatePart == DataPilotFieldGroupBy::MINUTES ||
        mnDatePart == DataPilotFieldGroupBy::SECONDS)
    {
        // handle time
        // (as in the cell functions, ScInterpreter::ScGetHour etc.: seconds are rounded)

        double time = rCellData.GetValue() - approxFloor(rCellData.GetValue());
        long seconds = static_cast<long>(approxFloor(time*D_TIMEFACTOR + 0.5));

        switch (mnDatePart)
        {
            case DataPilotFieldGroupBy::HOURS:
            {
                sal_Int32 hrs = seconds / 3600;
                sal_Int32 matchHrs = static_cast<sal_Int32>(mfMatchValue);
                return hrs == matchHrs;
            }
            case DataPilotFieldGroupBy::MINUTES:
            {
                sal_Int32 minutes = (seconds % 3600) / 60;
                sal_Int32 matchMinutes = static_cast<sal_Int32>(mfMatchValue);
                return minutes == matchMinutes;
            }
            case DataPilotFieldGroupBy::SECONDS:
            {
                sal_Int32 sec = seconds % 60;
                sal_Int32 matchSec = static_cast<sal_Int32>(mfMatchValue);
                return sec == matchSec;
            }
            default:
                DBG_ERROR("invalid time part");
        }
        return false;
    }

    Date date = *mpNullDate + static_cast<long>(approxFloor(rCellData.GetValue()));
    switch (mnDatePart)
    {
        case DataPilotFieldGroupBy::YEARS:
        {
            sal_Int32 year = static_cast<sal_Int32>(date.GetYear());
            sal_Int32 matchYear = static_cast<sal_Int32>(mfMatchValue);
            return year == matchYear;
        }
        case DataPilotFieldGroupBy::QUARTERS:
        {
            sal_Int32 qtr =  1 + (static_cast<sal_Int32>(date.GetMonth()) - 1) / 3;
            sal_Int32 matchQtr = static_cast<sal_Int32>(mfMatchValue);
            return qtr == matchQtr;
        }
        case DataPilotFieldGroupBy::MONTHS:
        {
            sal_Int32 month = static_cast<sal_Int32>(date.GetMonth());
            sal_Int32 matchMonth = static_cast<sal_Int32>(mfMatchValue);
            return month == matchMonth;
        }
        case DataPilotFieldGroupBy::DAYS:
        {
            Date yearStart(1, 1, date.GetYear());
            sal_Int32 days = (date - yearStart) + 1;       // Jan 01 has value 1
            if (days >= 60 && !date.IsLeapYear())
            {
                // This is not a leap year.  Adjust the value accordingly.
                ++days;
            }
            sal_Int32 matchDays = static_cast<sal_Int32>(mfMatchValue);
            return days == matchDays;
        }
        default:
            DBG_ERROR("invalid date part");
    }

    return false;
}
// -----------------------------------------------------------------------

ScDPDateGroupHelper::ScDPDateGroupHelper( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart ) :
    aNumInfo( rInfo ),
    nDatePart( nPart )
{
}

ScDPDateGroupHelper::~ScDPDateGroupHelper()
{
}

String lcl_GetTwoDigitString( sal_Int32 nValue )
{
    String aRet = String::CreateFromInt32( nValue );
    if ( aRet.Len() < 2 )
        aRet.Insert( (sal_Unicode)'0', 0 );
    return aRet;
}

String lcl_GetDateGroupName( sal_Int32 nDatePart, sal_Int32 nValue, SvNumberFormatter* pFormatter )
{
    String aRet;
    switch ( nDatePart )
    {
        case com::sun::star::sheet::DataPilotFieldGroupBy::YEARS:
            aRet = String::CreateFromInt32( nValue );
            break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS:
            aRet = ScGlobal::pLocaleData->getQuarterAbbreviation( (sal_Int16)(nValue - 1) );    // nValue is 1-based
            break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS:
            //! cache getMonths() result?
            aRet = ScGlobal::GetCalendar()->getDisplayName(
                        ::com::sun::star::i18n::CalendarDisplayIndex::MONTH,
                        sal_Int16(nValue-1), 0 );    // 0-based, get short name
            break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::DAYS:
            {
                Date aDate( 1, 1, SC_DP_LEAPYEAR );
                aDate += ( nValue - 1 );            // nValue is 1-based
                Date aNullDate = *(pFormatter->GetNullDate());
                long nDays = aDate - aNullDate;

                ULONG nFormat = pFormatter->GetFormatIndex( NF_DATE_SYS_DDMMM, ScGlobal::eLnge );
                Color* pColor;
                pFormatter->GetOutputString( nDays, nFormat, aRet, &pColor );
            }
            break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::HOURS:
            //! allow am/pm format?
            aRet = lcl_GetTwoDigitString( nValue );
            break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::MINUTES:
        case com::sun::star::sheet::DataPilotFieldGroupBy::SECONDS:
            aRet = ScGlobal::pLocaleData->getTimeSep();
            aRet.Append( lcl_GetTwoDigitString( nValue ) );
            break;
        default:
            DBG_ERROR("invalid date part");
    }
    return aRet;
}

sal_Int32 lcl_GetDatePartValue( double fValue, sal_Int32 nDatePart, SvNumberFormatter* pFormatter,
                                const ScDPNumGroupInfo* pNumInfo )
{
    // Start and end are inclusive
    // (End date without a time value is included, with a time value it's not)

    if ( pNumInfo )
    {
        if ( fValue < pNumInfo->Start && !rtl::math::approxEqual( fValue, pNumInfo->Start ) )
            return SC_DP_DATE_FIRST;
        if ( fValue > pNumInfo->End && !rtl::math::approxEqual( fValue, pNumInfo->End ) )
            return SC_DP_DATE_LAST;
    }

    sal_Int32 nResult = 0;

    if ( nDatePart == com::sun::star::sheet::DataPilotFieldGroupBy::HOURS || nDatePart == com::sun::star::sheet::DataPilotFieldGroupBy::MINUTES || nDatePart == com::sun::star::sheet::DataPilotFieldGroupBy::SECONDS )
    {
        // handle time
        // (as in the cell functions, ScInterpreter::ScGetHour etc.: seconds are rounded)

        double fTime = fValue - ::rtl::math::approxFloor(fValue);
        long nSeconds = (long)::rtl::math::approxFloor(fTime*D_TIMEFACTOR+0.5);

        switch ( nDatePart )
        {
            case com::sun::star::sheet::DataPilotFieldGroupBy::HOURS:
                nResult = nSeconds / 3600;
                break;
            case com::sun::star::sheet::DataPilotFieldGroupBy::MINUTES:
                nResult = ( nSeconds % 3600 ) / 60;
                break;
            case com::sun::star::sheet::DataPilotFieldGroupBy::SECONDS:
                nResult = nSeconds % 60;
                break;
        }
    }
    else
    {
        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long)::rtl::math::approxFloor( fValue );

        switch ( nDatePart )
        {
            case com::sun::star::sheet::DataPilotFieldGroupBy::YEARS:
                nResult = aDate.GetYear();
                break;
            case com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS:
                nResult = 1 + ( aDate.GetMonth() - 1 ) / 3;     // 1..4
                break;
            case com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS:
                nResult = aDate.GetMonth();     // 1..12
                break;
            case com::sun::star::sheet::DataPilotFieldGroupBy::DAYS:
                {
                    Date aYearStart( 1, 1, aDate.GetYear() );
                    nResult = ( aDate - aYearStart ) + 1;       // Jan 01 has value 1
                    if ( nResult >= 60 && !aDate.IsLeapYear() )
                    {
                        // days are counted from 1 to 366 - if not from a leap year, adjust
                        ++nResult;
                    }
                }
                break;
            default:
                DBG_ERROR("invalid date part");
        }
    }

    return nResult;
}

BOOL lcl_DateContained( sal_Int32 nGroupPart, const ScDPItemData& rGroupData,
                        sal_Int32 nBasePart, const ScDPItemData& rBaseData )
{
    if ( !rGroupData.IsValue() || !rBaseData.IsValue() )
    {
        // non-numeric entries involved: only match equal entries
        return rGroupData.IsCaseInsEqual( rBaseData );
    }

    // no approxFloor needed, values were created from integers
    sal_Int32 nGroupValue = (sal_Int32) rGroupData.GetValue();
    sal_Int32 nBaseValue = (sal_Int32) rBaseData.GetValue();
    if ( nBasePart > nGroupPart )
    {
        // switch, so the base part is the smaller (inner) part

        ::std::swap( nGroupPart, nBasePart );
        ::std::swap( nGroupValue, nBaseValue );
    }

    if ( nGroupValue == SC_DP_DATE_FIRST || nGroupValue == SC_DP_DATE_LAST ||
         nBaseValue == SC_DP_DATE_FIRST || nBaseValue == SC_DP_DATE_LAST )
    {
        // first/last entry matches only itself
        return ( nGroupValue == nBaseValue );
    }

    BOOL bContained = TRUE;
    switch ( nBasePart )        // inner part
    {
        case com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS:
            // a month is only contained in its quarter
            if ( nGroupPart == com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS )
            {
                // months and quarters are both 1-based
                bContained = ( nGroupValue - 1 == ( nBaseValue - 1 ) / 3 );
            }
            break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::DAYS:
            // a day is only contained in its quarter or month
            if ( nGroupPart == com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS || nGroupPart == com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS )
            {
                Date aDate( 1, 1, SC_DP_LEAPYEAR );
                aDate += ( nBaseValue - 1 );            // days are 1-based
                sal_Int32 nCompare = aDate.GetMonth();
                if ( nGroupPart == com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS )
                    nCompare = ( ( nCompare - 1 ) / 3 ) + 1;    // get quarter from date

                bContained = ( nGroupValue == nCompare );
            }
            break;

        // other parts: everything is contained
    }

    return bContained;
}

String lcl_GetSpecialDateName( double fValue, bool bFirst, SvNumberFormatter* pFormatter )
{
    rtl::OUStringBuffer aBuffer;
    aBuffer.append((sal_Unicode)( bFirst ? '<' : '>' ));
    lcl_AppendDateStr( aBuffer, fValue, pFormatter );
    return aBuffer.makeStringAndClear();
}

void ScDPDateGroupHelper::FillColumnEntries(
    SCCOL nSourceDim, const ScDPTableDataCache* pCache, std::vector<SCROW>& rEntries, const std::vector<SCROW>& rOriginal) const
{
    // auto min/max is only used for "Years" part, but the loop is always needed
    double fSourceMin = 0.0;
    double fSourceMax = 0.0;
    bool bFirst = true;

    size_t  nOriginalCount = rOriginal.size();
    for (size_t nOriginalPos=0; nOriginalPos<nOriginalCount; nOriginalPos++)
    {
        const  ScDPItemData* pItemData = pCache->GetItemDataById( nSourceDim, rOriginal[nOriginalPos] );
        if ( pItemData->HasStringData() )
        {
            // string data: just copy
            lcl_Insert( nSourceDim, pCache , rEntries,  rOriginal[nOriginalPos] );
        }
        else
        {
            double fSourceValue = pItemData->GetValue();
            if ( bFirst )
            {
                fSourceMin = fSourceMax = fSourceValue;
                bFirst = false;
            }
            else
            {
                if ( fSourceValue < fSourceMin )
                    fSourceMin = fSourceValue;
                if ( fSourceValue > fSourceMax )
                    fSourceMax = fSourceValue;
            }
        }
    }

    // For the start/end values, use the same date rounding as in ScDPNumGroupDimension::GetNumEntries
    // (but not for the list of available years):
    if ( aNumInfo.AutoStart )
        const_cast<ScDPDateGroupHelper*>(this)->aNumInfo.Start = rtl::math::approxFloor( fSourceMin );
    if ( aNumInfo.AutoEnd )
        const_cast<ScDPDateGroupHelper*>(this)->aNumInfo.End = rtl::math::approxFloor( fSourceMax ) + 1;

    //! if not automatic, limit fSourceMin/fSourceMax for list of year values?
    SvNumberFormatter* pFormatter = pCache->GetDoc()->GetFormatTable();

    long nStart = 0;
    long nEnd = 0;          // including

    switch ( nDatePart )
    {
        case com::sun::star::sheet::DataPilotFieldGroupBy::YEARS:
            nStart = lcl_GetDatePartValue( fSourceMin, com::sun::star::sheet::DataPilotFieldGroupBy::YEARS, pFormatter, NULL );
            nEnd = lcl_GetDatePartValue( fSourceMax, com::sun::star::sheet::DataPilotFieldGroupBy::YEARS, pFormatter, NULL );
            break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS: nStart = 1; nEnd = 4;   break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS:   nStart = 1; nEnd = 12;  break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::DAYS:     nStart = 1; nEnd = 366; break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::HOURS:    nStart = 0; nEnd = 23;  break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::MINUTES:  nStart = 0; nEnd = 59;  break;
        case com::sun::star::sheet::DataPilotFieldGroupBy::SECONDS:  nStart = 0; nEnd = 59;  break;
        default:
            DBG_ERROR("invalid date part");
    }

    for ( sal_Int32 nValue = nStart; nValue <= nEnd; nValue++ )
    {
        String aName = lcl_GetDateGroupName( nDatePart, nValue, pFormatter );
      lcl_InsertValue<false>( nSourceDim, pCache, rEntries, aName, nValue, nDatePart );
    }

    // add first/last entry (min/max)
    String aFirstName = lcl_GetSpecialDateName( aNumInfo.Start, true, pFormatter );
    lcl_InsertValue<true>( nSourceDim, pCache, rEntries, aFirstName, SC_DP_DATE_FIRST, nDatePart );

    String aLastName = lcl_GetSpecialDateName( aNumInfo.End, false, pFormatter );
    lcl_InsertValue<true>( nSourceDim, pCache, rEntries, aLastName, SC_DP_DATE_LAST, nDatePart );
}

// -----------------------------------------------------------------------

ScDPGroupItem::ScDPGroupItem( const ScDPItemData& rName ) :
    aGroupName( rName )
{
}

ScDPGroupItem::~ScDPGroupItem()
{
}

void ScDPGroupItem::AddElement( const ScDPItemData& rName )
{
    aElements.push_back( rName );
}

bool ScDPGroupItem::HasElement( const ScDPItemData& rData ) const
{
    for ( ScDPItemDataVec::const_iterator aIter(aElements.begin()); aIter != aElements.end(); aIter++ )
        if ( aIter->IsCaseInsEqual( rData ) )
            return true;

    return false;
}

bool ScDPGroupItem::HasCommonElement( const ScDPGroupItem& rOther ) const
{
    for ( ScDPItemDataVec::const_iterator aIter(aElements.begin()); aIter != aElements.end(); aIter++ )
        if ( rOther.HasElement( *aIter ) )
            return true;

    return false;
}

void ScDPGroupItem::FillGroupFilter( ScDPCacheTable::GroupFilter& rFilter ) const
{
    ScDPItemDataVec::const_iterator itrEnd = aElements.end();
    for (ScDPItemDataVec::const_iterator itr = aElements.begin(); itr != itrEnd; ++itr)
        rFilter.addMatchItem(itr->GetString(), itr->GetValue(), itr->IsValue());
}

// -----------------------------------------------------------------------

ScDPGroupDimension::ScDPGroupDimension( long nSource, const String& rNewName ) :
    nSourceDim( nSource ),
    nGroupDim( -1 ),
    aGroupName( rNewName ),
    pDateHelper( NULL )
{
}

ScDPGroupDimension::~ScDPGroupDimension()
{
    delete pDateHelper;
    maMemberEntries.clear();
}

ScDPGroupDimension::ScDPGroupDimension( const ScDPGroupDimension& rOther ) :
    nSourceDim( rOther.nSourceDim ),
    nGroupDim( rOther.nGroupDim ),
    aGroupName( rOther.aGroupName ),
    pDateHelper( NULL ),
   aItems( rOther.aItems )
{
    if ( rOther.pDateHelper )
        pDateHelper = new ScDPDateGroupHelper( *rOther.pDateHelper );
}

ScDPGroupDimension& ScDPGroupDimension::operator=( const ScDPGroupDimension& rOther )
{
    nSourceDim = rOther.nSourceDim;
    nGroupDim  = rOther.nGroupDim;
    aGroupName = rOther.aGroupName;
    aItems     = rOther.aItems;

    delete pDateHelper;
    if ( rOther.pDateHelper )
        pDateHelper = new ScDPDateGroupHelper( *rOther.pDateHelper );
    else
        pDateHelper = NULL;

    return *this;
}

void ScDPGroupDimension::MakeDateHelper( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart )
{
    delete pDateHelper;
    pDateHelper = new ScDPDateGroupHelper( rInfo, nPart );
}

void ScDPGroupDimension::AddItem( const ScDPGroupItem& rItem )
{
    aItems.push_back( rItem );
}

void ScDPGroupDimension::SetGroupDim( long nDim )
{
    nGroupDim = nDim;
}
const std::vector< SCROW >&  ScDPGroupDimension::GetColumnEntries( const ScDPCacheTable&  rCacheTable, const std::vector< SCROW >& rOriginal  )  const
{
    if ( maMemberEntries.empty() )
    {
        if ( pDateHelper )
        {
            pDateHelper->FillColumnEntries(  (SCCOL)GetSourceDim(), rCacheTable.getCache(), maMemberEntries,  rOriginal  );
        }
        else
        {
            for (size_t  i =0; i < rOriginal.size( );  i ++)
            {
                const  ScDPItemData* pItemData = rCacheTable.getCache()->GetItemDataById( (SCCOL)GetSourceDim(), rOriginal[i] );
                if ( !pItemData || !GetGroupForData( *pItemData ) )
                {
                    // not in any group -> add as its own group
                    maMemberEntries.push_back( rOriginal[i] );
                }
            }

            long nCount = aItems.size();
            for (long i=0; i<nCount; i++)
            {
                SCROW nNew = rCacheTable.getCache()->GetAdditionalItemID(  aItems[i].GetName() );
                lcl_Insert ( (SCCOL)GetSourceDim(), rCacheTable.getCache(), maMemberEntries, nNew  );
            }
        }
    }
    return maMemberEntries;
}



const ScDPGroupItem* ScDPGroupDimension::GetGroupForData( const ScDPItemData& rData ) const
{
    for ( ScDPGroupItemVec::const_iterator aIter(aItems.begin()); aIter != aItems.end(); aIter++ )
        if ( aIter->HasElement( rData ) )
            return &*aIter;

    return NULL;
}

const ScDPGroupItem* ScDPGroupDimension::GetGroupForName( const ScDPItemData& rName ) const
{
    for ( ScDPGroupItemVec::const_iterator aIter(aItems.begin()); aIter != aItems.end(); aIter++ )
        if ( aIter->GetName().IsCaseInsEqual( rName ) )
            return &*aIter;

    return NULL;
}

const ScDPGroupItem* ScDPGroupDimension::GetGroupByIndex( size_t nIndex ) const
{
    if (nIndex >= aItems.size())
        return NULL;

    return &aItems[nIndex];
}

void ScDPGroupDimension::DisposeData()
{
    maMemberEntries.clear();
}

// -----------------------------------------------------------------------

ScDPNumGroupDimension::ScDPNumGroupDimension() :
    pDateHelper( NULL ),
    bHasNonInteger( false ),
    cDecSeparator( 0 )
{
}

ScDPNumGroupDimension::ScDPNumGroupDimension( const ScDPNumGroupInfo& rInfo ) :
    aGroupInfo( rInfo ),
    pDateHelper( NULL ),
    bHasNonInteger( false ),
    cDecSeparator( 0 )
{
}

ScDPNumGroupDimension::ScDPNumGroupDimension( const ScDPNumGroupDimension& rOther ) :
    aGroupInfo( rOther.aGroupInfo ),
    pDateHelper( NULL ),
    bHasNonInteger( false ),
    cDecSeparator( 0 )
{
    if ( rOther.pDateHelper )
        pDateHelper = new ScDPDateGroupHelper( *rOther.pDateHelper );
}

ScDPNumGroupDimension& ScDPNumGroupDimension::operator=( const ScDPNumGroupDimension& rOther )
{
    aGroupInfo = rOther.aGroupInfo;

    delete pDateHelper;
    if ( rOther.pDateHelper )
        pDateHelper = new ScDPDateGroupHelper( *rOther.pDateHelper );
    else
        pDateHelper = NULL;

    bHasNonInteger = false;
    return *this;
}

void ScDPNumGroupDimension::DisposeData()
{
    bHasNonInteger = false;
    maMemberEntries.clear();
}

ScDPNumGroupDimension::~ScDPNumGroupDimension()
{
    delete pDateHelper;
}

void ScDPNumGroupDimension::MakeDateHelper( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart )
{
    delete pDateHelper;
    pDateHelper = new ScDPDateGroupHelper( rInfo, nPart );

    aGroupInfo.Enable = sal_True;   //! or query both?
}

const std::vector<SCROW>& ScDPNumGroupDimension::GetNumEntries(
    SCCOL nSourceDim, const ScDPTableDataCache* pCache, const std::vector<SCROW>& rOriginal) const
{
    if ( maMemberEntries.empty() )
    {
        SvNumberFormatter* pFormatter = pCache->GetDoc()->GetFormatTable();

        if ( pDateHelper )
            pDateHelper->FillColumnEntries( nSourceDim, pCache, maMemberEntries,rOriginal );
        else
        {
            // Copy textual entries.
            // Also look through the source entries for non-integer numbers, minimum and maximum.
            // GetNumEntries (GetColumnEntries) must be called before accessing the groups
            // (this in ensured by calling ScDPLevel::GetMembersObject for all column/row/page
            // dimensions before iterating over the values).

            cDecSeparator = ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0);

            // non-integer GroupInfo values count, too
            bHasNonInteger = ( !aGroupInfo.AutoStart && !IsInteger( aGroupInfo.Start ) ) ||
                             ( !aGroupInfo.AutoEnd   && !IsInteger( aGroupInfo.End   ) ) ||
                             !IsInteger( aGroupInfo.Step );
            double fSourceMin = 0.0;
            double fSourceMax = 0.0;
            bool bFirst = true;

            size_t  nOriginalCount = rOriginal.size();
            for (size_t  nOriginalPos=0; nOriginalPos<nOriginalCount; nOriginalPos++)
            {
               const  ScDPItemData* pItemData = pCache->GetItemDataById( nSourceDim , rOriginal[nOriginalPos] );

               if ( pItemData && pItemData ->HasStringData() )
               {
                   lcl_Insert( nSourceDim, pCache, maMemberEntries,  rOriginal[nOriginalPos] );
               }
               else
               {
                   double fSourceValue = pItemData->GetValue();
                   if ( bFirst )
                   {
                       fSourceMin = fSourceMax = fSourceValue;
                       bFirst = false;
                   }
                   else
                   {
                       if ( fSourceValue < fSourceMin )
                           fSourceMin = fSourceValue;
                       if ( fSourceValue > fSourceMax )
                           fSourceMax = fSourceValue;
                   }
                   if ( !bHasNonInteger && !IsInteger( fSourceValue ) )
                   {
                       // if any non-integer numbers are involved, the group labels are
                       // shown including their upper limit
                       bHasNonInteger = true;
                   }
               }
            }

            if ( aGroupInfo.DateValues )
            {
                // special handling for dates: always integer, round down limits
                bHasNonInteger = false;
                fSourceMin = rtl::math::approxFloor( fSourceMin );
                fSourceMax = rtl::math::approxFloor( fSourceMax ) + 1;
            }

            if ( aGroupInfo.AutoStart )
                const_cast<ScDPNumGroupDimension*>(this)->aGroupInfo.Start = fSourceMin;
            if ( aGroupInfo.AutoEnd )
                const_cast<ScDPNumGroupDimension*>(this)->aGroupInfo.End = fSourceMax;

            //! limit number of entries?

            long nLoopCount = 0;
            double fLoop = aGroupInfo.Start;

            // Use "less than" instead of "less or equal" for the loop - don't create a group
            // that consists only of the end value. Instead, the end value is then included
            // in the last group (last group is bigger than the others).
            // The first group has to be created nonetheless. GetNumGroupForValue has corresponding logic.

            bool bFirstGroup = true;
            while ( bFirstGroup || ( fLoop < aGroupInfo.End && !rtl::math::approxEqual( fLoop, aGroupInfo.End ) ) )
            {
                String aName = lcl_GetNumGroupName( fLoop, aGroupInfo, bHasNonInteger, cDecSeparator, pFormatter );
                // create a numerical entry to ensure proper sorting
                // (in FillMemberResults this needs special handling)
                lcl_InsertValue<true>( nSourceDim,  pCache,  maMemberEntries, aName, fLoop );
                ++nLoopCount;
                fLoop = aGroupInfo.Start + nLoopCount * aGroupInfo.Step;
                bFirstGroup = false;

                // ScDPItemData values are compared with approxEqual
            }

            String aFirstName = lcl_GetSpecialNumGroupName( aGroupInfo.Start, true, cDecSeparator, aGroupInfo.DateValues, pFormatter );
            lcl_InsertValue<true>( nSourceDim,  pCache,  maMemberEntries, aFirstName,  aGroupInfo.Start - aGroupInfo.Step );

            String aLastName = lcl_GetSpecialNumGroupName( aGroupInfo.End, false, cDecSeparator, aGroupInfo.DateValues, pFormatter );
            lcl_InsertValue<true>( nSourceDim,  pCache,  maMemberEntries, aLastName,  aGroupInfo.End + aGroupInfo.Step );
        }
    }
    return maMemberEntries;
}

ScDPGroupTableData::ScDPGroupTableData( const shared_ptr<ScDPTableData>& pSource, ScDocument* pDocument ) :
    ScDPTableData(pDocument),
    pSourceData( pSource ),
    pDoc( pDocument )
{
    DBG_ASSERT( pSource, "ScDPGroupTableData: pSource can't be NULL" );

    CreateCacheTable();
    nSourceCount = pSource->GetColumnCount();               // real columns, excluding data layout
    pNumGroups = new ScDPNumGroupDimension[nSourceCount];
}

ScDPGroupTableData::~ScDPGroupTableData()
{
    delete[] pNumGroups;
}

void ScDPGroupTableData::AddGroupDimension( const ScDPGroupDimension& rGroup )
{
    ScDPGroupDimension aNewGroup( rGroup );
    aNewGroup.SetGroupDim( GetColumnCount() );      // new dimension will be at the end
    aGroups.push_back( aNewGroup );
    aGroupNames.insert( OUString(aNewGroup.GetName()) );
}

void ScDPGroupTableData::SetNumGroupDimension( long nIndex, const ScDPNumGroupDimension& rGroup )
{
    if ( nIndex < nSourceCount )
    {
        pNumGroups[nIndex] = rGroup;

        // automatic minimum / maximum is handled in GetNumEntries
    }
}

long ScDPGroupTableData::GetDimensionIndex( const String& rName )
{
    for (long i=0; i<nSourceCount; i++)                         // nSourceCount excludes data layout
        if ( pSourceData->getDimensionName(i) == rName )        //! ignore case?
            return i;
    return -1;  // none
}

long ScDPGroupTableData::GetColumnCount()
{
    return nSourceCount + aGroups.size();
}

bool ScDPGroupTableData::IsNumGroupDimension( long nDimension ) const
{
    return ( nDimension < nSourceCount && pNumGroups[nDimension].GetInfo().Enable );
}

void ScDPGroupTableData::GetNumGroupInfo( long nDimension, ScDPNumGroupInfo& rInfo,
                                        bool& rNonInteger, sal_Unicode& rDecimal )
{
    if ( nDimension < nSourceCount )
    {
        rInfo       = pNumGroups[nDimension].GetInfo();
        rNonInteger = pNumGroups[nDimension].HasNonInteger();
        rDecimal    = pNumGroups[nDimension].GetDecSeparator();
    }
}
long  ScDPGroupTableData::GetMembersCount( long nDim )
{
    const std::vector< SCROW >&  members = GetColumnEntries( nDim );
    return members.size();
}
const std::vector< SCROW >& ScDPGroupTableData::GetColumnEntries( long  nColumn )
{
    if ( nColumn >= nSourceCount )
    {
        if ( getIsDataLayoutDimension( nColumn) )     // data layout dimension?
            nColumn = nSourceCount;                         // index of data layout in source data
        else
        {
            const ScDPGroupDimension& rGroupDim = aGroups[nColumn - nSourceCount];
            long nSourceDim = rGroupDim.GetSourceDim();
            // collection is cached at pSourceData, GetColumnEntries can be called every time
            const  std::vector< SCROW >& rOriginal = pSourceData->GetColumnEntries( nSourceDim );
            return rGroupDim.GetColumnEntries( GetCacheTable(), rOriginal );
        }
    }

    if ( IsNumGroupDimension( nColumn ) )
    {
        // dimension number is unchanged for numerical groups
        const  std::vector< SCROW >& rOriginal = pSourceData->GetColumnEntries( nColumn );
        return pNumGroups[nColumn].GetNumEntries( (SCCOL)nColumn, GetCacheTable().getCache(), rOriginal );
    }

    return pSourceData->GetColumnEntries( nColumn );
}

const ScDPItemData* ScDPGroupTableData::GetMemberById( long nDim, long nId )
{
    if ( nDim >= nSourceCount )
    {
        if ( getIsDataLayoutDimension( nDim) )
            nDim    = nSourceCount;
        else
        {
            const ScDPGroupDimension& rGroupDim = aGroups[nDim - nSourceCount];
            nDim = rGroupDim.GetSourceDim();
        }
    }
    return pSourceData->GetMemberById( nDim, nId );
}

String ScDPGroupTableData::getDimensionName(long nColumn)
{
    if ( nColumn >= nSourceCount )
    {
        if ( nColumn == sal::static_int_cast<long>( nSourceCount + aGroups.size() ) )     // data layout dimension?
            nColumn = nSourceCount;                         // index of data layout in source data
        else
            return aGroups[nColumn - nSourceCount].GetName();
    }

    return pSourceData->getDimensionName( nColumn );
}

BOOL ScDPGroupTableData::getIsDataLayoutDimension(long nColumn)
{
    // position of data layout dimension is moved from source data
    return ( nColumn == sal::static_int_cast<long>( nSourceCount + aGroups.size() ) );    // data layout dimension?
}

BOOL ScDPGroupTableData::IsDateDimension(long nDim)
{
    if ( nDim >= nSourceCount )
    {
        if ( nDim == sal::static_int_cast<long>( nSourceCount + aGroups.size() ) )        // data layout dimension?
            nDim = nSourceCount;                            // index of data layout in source data
        else
            nDim = aGroups[nDim - nSourceCount].GetSourceDim();  // look at original dimension
    }

    return pSourceData->IsDateDimension( nDim );
}

ULONG ScDPGroupTableData::GetNumberFormat(long nDim)
{
    if ( nDim >= nSourceCount )
    {
        if ( nDim == sal::static_int_cast<long>( nSourceCount + aGroups.size() ) )        // data layout dimension?
            nDim = nSourceCount;                            // index of data layout in source data
        else
            nDim = aGroups[nDim - nSourceCount].GetSourceDim();  // look at original dimension
    }

    return pSourceData->GetNumberFormat( nDim );
}

void ScDPGroupTableData::DisposeData()
{
    for ( ScDPGroupDimensionVec::iterator aIter(aGroups.begin()); aIter != aGroups.end(); aIter++ )
        aIter->DisposeData();

    for ( long i=0; i<nSourceCount; i++ )
        pNumGroups[i].DisposeData();

    pSourceData->DisposeData();
}

void ScDPGroupTableData::SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty )
{
    pSourceData->SetEmptyFlags( bIgnoreEmptyRows, bRepeatIfEmpty );
}

bool ScDPGroupTableData::IsRepeatIfEmpty()
{
    return pSourceData->IsRepeatIfEmpty();
}

void ScDPGroupTableData::CreateCacheTable()
{
    pSourceData->CreateCacheTable();
}

void ScDPGroupTableData::ModifyFilterCriteria(vector<ScDPCacheTable::Criterion>& rCriteria)
{
    typedef boost::unordered_map<long, const ScDPGroupDimension*> GroupFieldMapType;
    GroupFieldMapType aGroupFieldIds;
    {
        ScDPGroupDimensionVec::const_iterator itr = aGroups.begin(), itrEnd = aGroups.end();
        for (; itr != itrEnd; ++itr)
            aGroupFieldIds.insert( boost::unordered_map<long, const ScDPGroupDimension*>::value_type(itr->GetGroupDim(), &(*itr)) );
    }

    vector<ScDPCacheTable::Criterion> aNewCriteria;
    aNewCriteria.reserve(rCriteria.size() + aGroups.size());

    // Go through all the filtered field names and process them appropriately.

    vector<ScDPCacheTable::Criterion>::const_iterator itrEnd = rCriteria.end();
    GroupFieldMapType::const_iterator itrGrpEnd = aGroupFieldIds.end();
    for (vector<ScDPCacheTable::Criterion>::const_iterator itr = rCriteria.begin(); itr != itrEnd; ++itr)
    {
        ScDPCacheTable::SingleFilter* pFilter = dynamic_cast<ScDPCacheTable::SingleFilter*>(itr->mpFilter.get());
        if (!pFilter)
            // We expect this to be a single filter.
            continue;

        GroupFieldMapType::const_iterator itrGrp = aGroupFieldIds.find(itr->mnFieldIndex);
        if (itrGrp == itrGrpEnd)
        {
            if (IsNumGroupDimension(itr->mnFieldIndex))
            {
                // internal number group field
                const ScDPNumGroupDimension& rNumGrpDim = pNumGroups[itr->mnFieldIndex];
                const ScDPDateGroupHelper* pDateHelper = rNumGrpDim.GetDateHelper();
                if (!pDateHelper)
                {
                    // What do we do here !?
                    continue;
                }

                ScDPCacheTable::Criterion aCri;
                aCri.mnFieldIndex = itr->mnFieldIndex;
                aCri.mpFilter.reset(new ScDPGroupDateFilter(
                    pFilter->getMatchValue(), pDateHelper->GetDatePart(),
                    pDoc->GetFormatTable()->GetNullDate(), &pDateHelper->GetNumInfo()));

                aNewCriteria.push_back(aCri);
            }
            else
            {
                // This is a regular source field.
                aNewCriteria.push_back(*itr);
            }
        }
        else
        {
            // This is an ordinary group field or external number group field.

            const ScDPGroupDimension* pGrpDim = itrGrp->second;
            long nSrcDim = pGrpDim->GetSourceDim();
            const ScDPDateGroupHelper* pDateHelper = pGrpDim->GetDateHelper();

            if (pDateHelper)
            {
                // external number group
                ScDPCacheTable::Criterion aCri;
                aCri.mnFieldIndex = nSrcDim;  // use the source dimension, not the group dimension.
                aCri.mpFilter.reset(new ScDPGroupDateFilter(
                    pFilter->getMatchValue(), pDateHelper->GetDatePart(),
                    pDoc->GetFormatTable()->GetNullDate(), &pDateHelper->GetNumInfo()));

                aNewCriteria.push_back(aCri);
            }
            else
            {
                // normal group

                // Note that each group dimension may have multiple group names!
                size_t nGroupItemCount = pGrpDim->GetItemCount();
                for (size_t i = 0; i < nGroupItemCount; ++i)
                {
                    const ScDPGroupItem* pGrpItem = pGrpDim->GetGroupByIndex(i);
                    ScDPItemData aName( pFilter->getMatchString(),pFilter->getMatchValue(),pFilter->hasValue()) ;

                                       if (!pGrpItem || !pGrpItem->GetName().IsCaseInsEqual(aName))
                        continue;

                    ScDPCacheTable::Criterion aCri;
                    aCri.mnFieldIndex = nSrcDim;
                    aCri.mpFilter.reset(new ScDPCacheTable::GroupFilter());
                    ScDPCacheTable::GroupFilter* pGrpFilter =
                        static_cast<ScDPCacheTable::GroupFilter*>(aCri.mpFilter.get());

                    pGrpItem->FillGroupFilter(*pGrpFilter);
                    aNewCriteria.push_back(aCri);
                }
            }
        }
    }
    rCriteria.swap(aNewCriteria);
}

void ScDPGroupTableData::FilterCacheTable(const vector<ScDPCacheTable::Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rCatDims)
{
    vector<ScDPCacheTable::Criterion> aNewCriteria(rCriteria);
    ModifyFilterCriteria(aNewCriteria);
    pSourceData->FilterCacheTable(aNewCriteria, rCatDims);
}

void ScDPGroupTableData::GetDrillDownData(const vector<ScDPCacheTable::Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rCatDims, Sequence< Sequence<Any> >& rData)
{
    vector<ScDPCacheTable::Criterion> aNewCriteria(rCriteria);
    ModifyFilterCriteria(aNewCriteria);
    pSourceData->GetDrillDownData(aNewCriteria, rCatDims, rData);
}

void ScDPGroupTableData::CalcResults(CalcInfo& rInfo, bool bAutoShow)
{
    // #i111435# Inside FillRowDataFromCacheTable/GetItemData, virtual methods
    // getIsDataLayoutDimension and GetSourceDim are used, so it has to be called
    // with original rInfo, containing dimension indexes of the grouped data.

    const ScDPCacheTable& rCacheTable = pSourceData->GetCacheTable();
    sal_Int32 nRowSize = rCacheTable.getRowSize();
    for (sal_Int32 nRow = 0; nRow < nRowSize; ++nRow)
    {
        if (!rCacheTable.isRowActive(nRow))
            continue;

        CalcRowData aData;
        FillRowDataFromCacheTable(nRow, rCacheTable, rInfo, aData);

        if ( !rInfo.aColLevelDims.empty() )
            FillGroupValues(&aData.aColData[0], rInfo.aColLevelDims.size(), &rInfo.aColLevelDims[0]);
        if ( !rInfo.aRowLevelDims.empty() )
            FillGroupValues(&aData.aRowData[0], rInfo.aRowLevelDims.size(), &rInfo.aRowLevelDims[0]);
        if ( !rInfo.aPageDims.empty() )
            FillGroupValues(&aData.aPageData[0], rInfo.aPageDims.size(), &rInfo.aPageDims[0]);

        ProcessRowData(rInfo, aData, bAutoShow);
    }
}

const ScDPCacheTable& ScDPGroupTableData::GetCacheTable() const
{
    return pSourceData->GetCacheTable();
}

void ScDPGroupTableData::FillGroupValues( /*ScDPItemData* pItemData*/ SCROW* pItemDataIndex, long nCount, const long* pDims )
{
    long nGroupedColumns = aGroups.size();

    const ScDPTableDataCache* pCache = GetCacheTable().getCache();
    for (long nDim=0; nDim<nCount; nDim++)
    {
        const ScDPDateGroupHelper* pDateHelper = NULL;

        long nColumn = pDims[nDim];
        long nSourceDim = nColumn;
        if ( nColumn >= nSourceCount && nColumn < nSourceCount + nGroupedColumns )
        {
            const ScDPGroupDimension& rGroupDim = aGroups[nColumn - nSourceCount];
            nSourceDim= rGroupDim.GetSourceDim();
            pDateHelper = rGroupDim.GetDateHelper();
            if ( !pDateHelper )                         // date is handled below
            {
                 const ScDPGroupItem* pGroupItem = rGroupDim.GetGroupForData( *GetMemberById( nSourceDim, pItemDataIndex[nDim] ));
              if ( pGroupItem )
                      pItemDataIndex[nDim] = pCache->GetAdditionalItemID(  pGroupItem->GetName() );
            }
        }
        else if ( IsNumGroupDimension( nColumn ) )
        {
            pDateHelper = pNumGroups[nColumn].GetDateHelper();
            if ( !pDateHelper )                         // date is handled below
            {
                 const ScDPItemData* pData = pCache->GetItemDataById( (SCCOL)nSourceDim, pItemDataIndex[nDim]);
                if ( pData ->IsValue() )
                {
                    ScDPNumGroupInfo aNumInfo;
                    bool bHasNonInteger = false;
                    sal_Unicode cDecSeparator = 0;
                    GetNumGroupInfo( nColumn, aNumInfo, bHasNonInteger, cDecSeparator );
                    double fGroupValue;
                    String aGroupName = lcl_GetNumGroupForValue( pData->GetValue(),
                             aNumInfo, bHasNonInteger, cDecSeparator, fGroupValue, pDoc );
                    ScDPItemData  aItemData ( aGroupName, fGroupValue, TRUE ) ;
                    pItemDataIndex[nDim] = pCache->GetAdditionalItemID( aItemData  );
                }
                // else (textual) keep original value
            }
        }

        if ( pDateHelper )
        {
            const ScDPItemData* pData  = GetCacheTable().getCache()->GetItemDataById( (SCCOL)nSourceDim, pItemDataIndex[nDim]);
              if ( pData ->IsValue() )
            {
                sal_Int32 nPartValue = lcl_GetDatePartValue(
                    pData->GetValue(), pDateHelper->GetDatePart(), pDoc->GetFormatTable(),
                    &pDateHelper->GetNumInfo() );
                ScDPItemData  aItemData( pDateHelper->GetDatePart(), String(), nPartValue, ScDPItemData::MK_DATA|ScDPItemData::MK_VAL|ScDPItemData::MK_DATEPART );
                pItemDataIndex[nDim] = GetCacheTable().getCache()->GetAdditionalItemID( aItemData );
            }
        }
    }
}

BOOL ScDPGroupTableData::IsBaseForGroup(long nDim) const
{
    for ( ScDPGroupDimensionVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); aIter++ )
    {
        const ScDPGroupDimension& rDim = *aIter;
        if ( rDim.GetSourceDim() == nDim )
            return TRUE;
    }

    return FALSE;
}

long ScDPGroupTableData::GetGroupBase(long nGroupDim) const
{
    for ( ScDPGroupDimensionVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); aIter++ )
    {
        const ScDPGroupDimension& rDim = *aIter;
        if ( rDim.GetGroupDim() == nGroupDim )
            return rDim.GetSourceDim();
    }

    return -1;      // none
}

BOOL ScDPGroupTableData::IsNumOrDateGroup(long nDimension) const
{
    // Virtual method from ScDPTableData, used in result data to force text labels.

    if ( nDimension < nSourceCount )
    {
        return pNumGroups[nDimension].GetInfo().Enable ||
               pNumGroups[nDimension].GetDateHelper();
    }

    for ( ScDPGroupDimensionVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); aIter++ )
    {
        const ScDPGroupDimension& rDim = *aIter;
        if ( rDim.GetGroupDim() == nDimension )
            return ( rDim.GetDateHelper() != NULL );
    }

    return FALSE;
}

BOOL ScDPGroupTableData::IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                    const ScDPItemData& rBaseData, long nBaseIndex ) const
{
    for ( ScDPGroupDimensionVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); aIter++ )
    {
        const ScDPGroupDimension& rDim = *aIter;
        if ( rDim.GetGroupDim() == nGroupIndex && rDim.GetSourceDim() == nBaseIndex )
        {
            const ScDPDateGroupHelper* pGroupDateHelper = rDim.GetDateHelper();
            if ( pGroupDateHelper )
            {
                //! transform rBaseData (innermost date part)
                //! -> always do "HasCommonElement" style comparison
                //! (only Quarter, Month, Day affected)

                const ScDPDateGroupHelper* pBaseDateHelper = NULL;
                if ( nBaseIndex < nSourceCount )
                    pBaseDateHelper = pNumGroups[nBaseIndex].GetDateHelper();

                // If there's a date group dimension, the base dimension must have
                // date group information, too.
                if ( !pBaseDateHelper )
                {
                    DBG_ERROR( "mix of date and non-date groups" );
                    return TRUE;
                }

                sal_Int32 nGroupPart = pGroupDateHelper->GetDatePart();
                sal_Int32 nBasePart = pBaseDateHelper->GetDatePart();
                return lcl_DateContained( nGroupPart, rGroupData, nBasePart, rBaseData );
            }
            else
            {
                // If the item is in a group, only that group is valid.
                // If the item is not in any group, its own name is valid.

                const ScDPGroupItem* pGroup = rDim.GetGroupForData( rBaseData );
                return pGroup ? pGroup->GetName().IsCaseInsEqual( rGroupData ) :
                                rGroupData.IsCaseInsEqual( rBaseData );
            }
        }
    }

    DBG_ERROR("IsInGroup: no group dimension found");
    return TRUE;
}

BOOL ScDPGroupTableData::HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                         const ScDPItemData& rSecondData, long nSecondIndex ) const
{
    const ScDPGroupDimension* pFirstDim = NULL;
    const ScDPGroupDimension* pSecondDim = NULL;
    for ( ScDPGroupDimensionVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); aIter++ )
    {
        const ScDPGroupDimension* pDim = &(*aIter);
        if ( pDim->GetGroupDim() == nFirstIndex )
            pFirstDim = pDim;
        else if ( pDim->GetGroupDim() == nSecondIndex )
            pSecondDim = pDim;
    }
    if ( pFirstDim && pSecondDim )
    {
        const ScDPDateGroupHelper* pFirstDateHelper = pFirstDim->GetDateHelper();
        const ScDPDateGroupHelper* pSecondDateHelper = pSecondDim->GetDateHelper();
        if ( pFirstDateHelper || pSecondDateHelper )
        {
            // If one is a date group dimension, the other one must be, too.
            if ( !pFirstDateHelper || !pSecondDateHelper )
            {
                DBG_ERROR( "mix of date and non-date groups" );
                return TRUE;
            }

            sal_Int32 nFirstPart = pFirstDateHelper->GetDatePart();
            sal_Int32 nSecondPart = pSecondDateHelper->GetDatePart();
            return lcl_DateContained( nFirstPart, rFirstData, nSecondPart, rSecondData );
        }

        const ScDPGroupItem* pFirstItem = pFirstDim->GetGroupForName( rFirstData );
        const ScDPGroupItem* pSecondItem = pSecondDim->GetGroupForName( rSecondData );
        if ( pFirstItem && pSecondItem )
        {
            // two existing groups -> TRUE if they have a common element
            return pFirstItem->HasCommonElement( *pSecondItem );
        }
        else if ( pFirstItem )
        {
            // "automatic" group contains only its own name
            return pFirstItem->HasElement( rSecondData );
        }
        else if ( pSecondItem )
        {
            // "automatic" group contains only its own name
            return pSecondItem->HasElement( rFirstData );
        }
        else
        {
            // no groups -> TRUE if equal
            return rFirstData.IsCaseInsEqual( rSecondData );
        }
    }

    DBG_ERROR("HasCommonElement: no group dimension found");
    return TRUE;
}

long ScDPGroupTableData::GetSourceDim( long nDim )
{
    if ( getIsDataLayoutDimension( nDim ) )
        return nSourceCount;
    if (  nDim >= nSourceCount && nDim < nSourceCount +(long) aGroups.size()  )
    {
         const ScDPGroupDimension& rGroupDim = aGroups[nDim - nSourceCount];
            return  rGroupDim.GetSourceDim();
    }
    return nDim;
}
 long ScDPGroupTableData::Compare( long nDim, long nDataId1, long nDataId2)
{
    if ( getIsDataLayoutDimension(nDim) )
        return 0;
    return ScDPItemData::Compare( *GetMemberById(nDim,  nDataId1),*GetMemberById(nDim,  nDataId2) );
}
// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
