/*************************************************************************
 *
 *  $RCSfile: dpgroup.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:30:50 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>

#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <unotools/localedatawrapper.hxx>
#include <svtools/zforlist.hxx>

#include "dpgroup.hxx"
#include "collect.hxx"
#include "global.hxx"
#include "document.hxx"

#define D_TIMEFACTOR              86400.0

const USHORT SC_DP_LEAPYEAR = 1648;     // arbitrary leap year for date calculations

// part values for the extra "<" and ">" entries (same for all parts)
const sal_Int32 SC_DP_DATE_FIRST = -1;
const sal_Int32 SC_DP_DATE_LAST = 10000;

// -----------------------------------------------------------------------

void lcl_AppendDateStr( rtl::OUStringBuffer& rBuffer, double fValue, SvNumberFormatter* pFormatter )
{
    ULONG nFormat = pFormatter->GetStandardFormat( NUMBERFORMAT_DATE, ScGlobal::eLnge );
    String aString;
    pFormatter->GetInputLineString( fValue, nFormat, aString );
    rBuffer.append( aString );
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
        case SC_DP_DATE_YEARS:
            aRet = String::CreateFromInt32( nValue );
            break;
        case SC_DP_DATE_QUARTERS:
            aRet = ScGlobal::pLocaleData->getQuarterAbbreviation( (sal_Int16)(nValue - 1) );    // nValue is 1-based
            break;
        case SC_DP_DATE_MONTHS:
            //! cache getMonths() result?
            aRet = ScGlobal::pCalendar->getDisplayName(
                        ::com::sun::star::i18n::CalendarDisplayIndex::MONTH,
                        sal_Int16(nValue-1), 0 );    // 0-based, get short name
            break;
        case SC_DP_DATE_DAYS:
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
        case SC_DP_DATE_HOURS:
            //! allow am/pm format?
            aRet = lcl_GetTwoDigitString( nValue );
            break;
        case SC_DP_DATE_MINUTES:
        case SC_DP_DATE_SECONDS:
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

    if ( nDatePart == SC_DP_DATE_HOURS || nDatePart == SC_DP_DATE_MINUTES || nDatePart == SC_DP_DATE_SECONDS )
    {
        // handle time
        // (as in the cell functions, ScInterpreter::ScGetHour etc.: seconds are rounded)

        double fTime = fValue - ::rtl::math::approxFloor(fValue);
        long nSeconds = (long)::rtl::math::approxFloor(fTime*D_TIMEFACTOR+0.5);

        switch ( nDatePart )
        {
            case SC_DP_DATE_HOURS:
                nResult = nSeconds / 3600;
                break;
            case SC_DP_DATE_MINUTES:
                nResult = ( nSeconds % 3600 ) / 60;
                break;
            case SC_DP_DATE_SECONDS:
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
            case SC_DP_DATE_YEARS:
                nResult = aDate.GetYear();
                break;
            case SC_DP_DATE_QUARTERS:
                nResult = 1 + ( aDate.GetMonth() - 1 ) / 3;     // 1..4
                break;
            case SC_DP_DATE_MONTHS:
                nResult = aDate.GetMonth();     // 1..12
                break;
            case SC_DP_DATE_DAYS:
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
    if ( !rGroupData.bHasValue || !rBaseData.bHasValue )
    {
        // non-numeric entries involved: only match equal entries
        return rGroupData.IsCaseInsEqual( rBaseData );
    }

    // no approxFloor needed, values were created from integers
    sal_Int32 nGroupValue = (sal_Int32) rGroupData.fValue;
    sal_Int32 nBaseValue = (sal_Int32) rBaseData.fValue;
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
        case SC_DP_DATE_MONTHS:
            // a month is only contained in its quarter
            if ( nGroupPart == SC_DP_DATE_QUARTERS )
            {
                // months and quarters are both 1-based
                bContained = ( nGroupValue - 1 == ( nBaseValue - 1 ) / 3 );
            }
            break;
        case SC_DP_DATE_DAYS:
            // a day is only contained in its quarter or month
            if ( nGroupPart == SC_DP_DATE_MONTHS || nGroupPart == SC_DP_DATE_QUARTERS )
            {
                Date aDate( 1, 1, SC_DP_LEAPYEAR );
                aDate += ( nBaseValue - 1 );            // days are 1-based
                sal_Int32 nCompare = aDate.GetMonth();
                if ( nGroupPart == SC_DP_DATE_QUARTERS )
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

void ScDPDateGroupHelper::FillColumnEntries( TypedStrCollection& rEntries, const TypedStrCollection& rOriginal,
                                            SvNumberFormatter* pFormatter ) const
{
    // auto min/max is only used for "Years" part, but the loop is always needed
    double fSourceMin = 0.0;
    double fSourceMax = 0.0;
    bool bFirst = true;

    USHORT nOriginalCount = rOriginal.GetCount();
    for (USHORT nOriginalPos=0; nOriginalPos<nOriginalCount; nOriginalPos++)
    {
        const TypedStrData& rStrData = *rOriginal[nOriginalPos];
        if ( rStrData.IsStrData() )
        {
            // string data: just copy
            TypedStrData* pNew = new TypedStrData( rStrData );
            if ( !rEntries.Insert( pNew ) )
                delete pNew;
        }
        else
        {
            double fSourceValue = rStrData.GetValue();
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

    long nStart = 0;
    long nEnd = 0;          // including

    switch ( nDatePart )
    {
        case SC_DP_DATE_YEARS:
            nStart = lcl_GetDatePartValue( fSourceMin, SC_DP_DATE_YEARS, pFormatter, NULL );
            nEnd = lcl_GetDatePartValue( fSourceMax, SC_DP_DATE_YEARS, pFormatter, NULL );
            break;
        case SC_DP_DATE_QUARTERS: nStart = 1; nEnd = 4;   break;
        case SC_DP_DATE_MONTHS:   nStart = 1; nEnd = 12;  break;
        case SC_DP_DATE_DAYS:     nStart = 1; nEnd = 366; break;
        case SC_DP_DATE_HOURS:    nStart = 0; nEnd = 23;  break;
        case SC_DP_DATE_MINUTES:  nStart = 0; nEnd = 59;  break;
        case SC_DP_DATE_SECONDS:  nStart = 0; nEnd = 59;  break;
        default:
            DBG_ERROR("invalid date part");
    }

    for ( sal_Int32 nValue = nStart; nValue <= nEnd; nValue++ )
    {
        String aName = lcl_GetDateGroupName( nDatePart, nValue, pFormatter );
        TypedStrData* pNew = new TypedStrData( aName, nValue, SC_STRTYPE_VALUE );
        if ( !rEntries.Insert( pNew ) )
            delete pNew;
    }

    // add first/last entry (min/max)

    String aFirstName = lcl_GetSpecialDateName( aNumInfo.Start, true, pFormatter );
    TypedStrData* pFirstEntry = new TypedStrData( aFirstName, SC_DP_DATE_FIRST, SC_STRTYPE_VALUE );
    if ( !rEntries.Insert( pFirstEntry ) )
        delete pFirstEntry;

    String aLastName = lcl_GetSpecialDateName( aNumInfo.End, false, pFormatter );
    TypedStrData* pLastEntry = new TypedStrData( aLastName, SC_DP_DATE_LAST, SC_STRTYPE_VALUE );
    if ( !rEntries.Insert( pLastEntry ) )
        delete pLastEntry;
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

// -----------------------------------------------------------------------

ScDPGroupDimension::ScDPGroupDimension( long nSource, const String& rNewName ) :
    nSourceDim( nSource ),
    nGroupDim( -1 ),
    aGroupName( rNewName ),
    pDateHelper( NULL ),
    pCollection( NULL )
{
}

ScDPGroupDimension::~ScDPGroupDimension()
{
    delete pDateHelper;
    delete pCollection;
}

ScDPGroupDimension::ScDPGroupDimension( const ScDPGroupDimension& rOther ) :
    nSourceDim( rOther.nSourceDim ),
    nGroupDim( rOther.nGroupDim ),
    aGroupName( rOther.aGroupName ),
    aItems( rOther.aItems ),
    pDateHelper( NULL ),
    pCollection( NULL )                 // collection isn't copied - allocated on demand
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

    delete pCollection;                 // collection isn't copied - allocated on demand
    pCollection = NULL;
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

const TypedStrCollection& ScDPGroupDimension::GetColumnEntries(
                    const TypedStrCollection& rOriginal, ScDocument* pDoc ) const
{
    if ( !pCollection )
    {
        pCollection = new TypedStrCollection();
        if ( pDateHelper )
            pDateHelper->FillColumnEntries( *pCollection, rOriginal, pDoc->GetFormatTable() );
        else
        {
            long nCount = aItems.size();
            for (long i=0; i<nCount; i++)
            {
                //! numeric entries?
                TypedStrData* pNew = new TypedStrData( aItems[i].GetName().aString );
                if ( !pCollection->Insert( pNew ) )
                    delete pNew;
            }

            USHORT nOriginalCount = rOriginal.GetCount();
            for (USHORT nOriginalPos=0; nOriginalPos<nOriginalCount; nOriginalPos++)
            {
                const TypedStrData& rStrData = *rOriginal[nOriginalPos];
                ScDPItemData aItemData( rStrData.GetString(), rStrData.GetValue(), !rStrData.IsStrData() );
                if ( !GetGroupForData( aItemData ) )
                {
                    // not in any group -> add as its own group
                    TypedStrData* pNew = new TypedStrData( rStrData );
                    if ( !pCollection->Insert( pNew ) )
                        delete pNew;
                }
            }
        }
    }
    return *pCollection;
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

void ScDPGroupDimension::DisposeData()
{
    delete pCollection;
    pCollection = NULL;
}

// -----------------------------------------------------------------------

ScDPNumGroupDimension::ScDPNumGroupDimension() :
    pCollection( NULL ),
    pDateHelper( NULL ),
    bHasNonInteger( false ),
    cDecSeparator( 0 )
{
}

ScDPNumGroupDimension::ScDPNumGroupDimension( const ScDPNumGroupInfo& rInfo ) :
    aGroupInfo( rInfo ),
    pCollection( NULL ),
    pDateHelper( NULL ),
    bHasNonInteger( false ),
    cDecSeparator( 0 )
{
}

ScDPNumGroupDimension::ScDPNumGroupDimension( const ScDPNumGroupDimension& rOther ) :
    aGroupInfo( rOther.aGroupInfo ),
    pCollection( NULL ),                // collection isn't copied - allocated on demand
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

    delete pCollection;                 // collection isn't copied - allocated on demand
    pCollection = NULL;
    bHasNonInteger = false;
    return *this;
}

void ScDPNumGroupDimension::DisposeData()
{
    delete pCollection;
    pCollection = NULL;
    bHasNonInteger = false;
}

ScDPNumGroupDimension::~ScDPNumGroupDimension()
{
    delete pDateHelper;
    delete pCollection;
}

void ScDPNumGroupDimension::MakeDateHelper( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart )
{
    delete pDateHelper;
    pDateHelper = new ScDPDateGroupHelper( rInfo, nPart );

    aGroupInfo.Enable = sal_True;   //! or query both?
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

const TypedStrCollection& ScDPNumGroupDimension::GetNumEntries(
                    const TypedStrCollection& rOriginal, ScDocument* pDoc ) const
{
    if ( !pCollection )
    {
        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

        pCollection = new TypedStrCollection();
        if ( pDateHelper )
            pDateHelper->FillColumnEntries( *pCollection, rOriginal, pFormatter );
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

            USHORT nOriginalCount = rOriginal.GetCount();
            for (USHORT nOriginalPos=0; nOriginalPos<nOriginalCount; nOriginalPos++)
            {
                const TypedStrData& rStrData = *rOriginal[nOriginalPos];
                if ( rStrData.IsStrData() )
                {
                    // string data: just copy
                    TypedStrData* pNew = new TypedStrData( rStrData );
                    if ( !pCollection->Insert( pNew ) )
                        delete pNew;
                }
                else
                {
                    double fSourceValue = rStrData.GetValue();
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
                TypedStrData* pNew = new TypedStrData( aName, fLoop, SC_STRTYPE_VALUE );
                if ( !pCollection->Insert( pNew ) )
                    delete pNew;

                ++nLoopCount;
                fLoop = aGroupInfo.Start + nLoopCount * aGroupInfo.Step;
                bFirstGroup = false;

                // ScDPItemData values are compared with approxEqual
            }

            String aFirstName = lcl_GetSpecialNumGroupName( aGroupInfo.Start, true, cDecSeparator, aGroupInfo.DateValues, pFormatter );
            TypedStrData* pFirstEntry = new TypedStrData( aFirstName, aGroupInfo.Start - aGroupInfo.Step, SC_STRTYPE_VALUE );
            if ( !pCollection->Insert( pFirstEntry ) )
                delete pFirstEntry;

            String aLastName = lcl_GetSpecialNumGroupName( aGroupInfo.End, false, cDecSeparator, aGroupInfo.DateValues, pFormatter );
            TypedStrData* pLastEntry = new TypedStrData( aLastName, aGroupInfo.End + aGroupInfo.Step, SC_STRTYPE_VALUE );
            if ( !pCollection->Insert( pLastEntry ) )
                delete pLastEntry;
        }
    }
    return *pCollection;
}

// -----------------------------------------------------------------------

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

ScDPGroupTableData::ScDPGroupTableData( ScDPTableData* pSource, ScDocument* pDocument ) :
    pSourceData( pSource ),
    pDoc( pDocument )
{
    DBG_ASSERT( pSource, "ScDPGroupTableData: pSource can't be NULL" );

    nSourceCount = pSource->GetColumnCount();               // real columns, excluding data layout
    pNumGroups = new ScDPNumGroupDimension[nSourceCount];
}

ScDPGroupTableData::~ScDPGroupTableData()
{
    delete[] pNumGroups;
    delete pSourceData;
}

void ScDPGroupTableData::AddGroupDimension( const ScDPGroupDimension& rGroup )
{
    ScDPGroupDimension aNewGroup( rGroup );
    aNewGroup.SetGroupDim( GetColumnCount() );      // new dimension will be at the end
    aGroups.push_back( aNewGroup );
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

const TypedStrCollection& ScDPGroupTableData::GetColumnEntries(long nColumn)
{
    // date handling is in ScDPGroupDimension::GetColumnEntries / ScDPNumGroupDimension::GetNumEntries
    // (to use the pCollection members)

    if ( nColumn >= nSourceCount )
    {
        if ( nColumn == nSourceCount + aGroups.size() )     // data layout dimension?
            nColumn = nSourceCount;                         // index of data layout in source data
        else
        {
            const ScDPGroupDimension& rGroupDim = aGroups[nColumn - nSourceCount];
            long nSourceDim = rGroupDim.GetSourceDim();
            // collection is cached at pSourceData, GetColumnEntries can be called every time
            const TypedStrCollection& rOriginal = pSourceData->GetColumnEntries( nSourceDim );
            return rGroupDim.GetColumnEntries( rOriginal, pDoc );
        }
    }

    if ( IsNumGroupDimension( nColumn ) )
    {
        // dimension number is unchanged for numerical groups
        const TypedStrCollection& rOriginal = pSourceData->GetColumnEntries( nColumn );
        return pNumGroups[nColumn].GetNumEntries( rOriginal, pDoc );
    }

    return pSourceData->GetColumnEntries( nColumn );
}

String ScDPGroupTableData::getDimensionName(long nColumn)
{
    if ( nColumn >= nSourceCount )
    {
        if ( nColumn == nSourceCount + aGroups.size() )     // data layout dimension?
            nColumn = nSourceCount;                         // index of data layout in source data
        else
            return aGroups[nColumn - nSourceCount].GetName();
    }

    return pSourceData->getDimensionName( nColumn );
}

BOOL ScDPGroupTableData::getIsDataLayoutDimension(long nColumn)
{
    // position of data layout dimension is moved from source data
    return ( nColumn == nSourceCount + aGroups.size() );    // data layout dimension?
}

BOOL ScDPGroupTableData::IsDateDimension(long nDim)
{
    if ( nDim >= nSourceCount )
    {
        if ( nDim == nSourceCount + aGroups.size() )        // data layout dimension?
            nDim = nSourceCount;                            // index of data layout in source data
        else
            nDim = aGroups[nDim - nSourceCount].GetSourceDim();  // look at original dimension
    }

    return pSourceData->IsDateDimension( nDim );
}

UINT32 ScDPGroupTableData::GetNumberFormat(long nDim)
{
    if ( nDim >= nSourceCount )
    {
        if ( nDim == nSourceCount + aGroups.size() )        // data layout dimension?
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

void ScDPGroupTableData::ResetIterator()
{
    pSourceData->ResetIterator();
}

long* ScDPGroupTableData::CopyFields( const long* pSourceDims, long nCount )
{
    if (!nCount)
        return NULL;

    long nGroupedColumns = aGroups.size();

    long* pNew = new long[nCount];
    for (long i=0; i<nCount; i++)
        if ( pSourceDims[i] >= nSourceCount )
        {
            if ( pSourceDims[i] == nSourceCount + nGroupedColumns )
                pNew[i] = nSourceCount;         // data layout in source
            else
                pNew[i] = aGroups[pSourceDims[i] - nSourceCount].GetSourceDim();  // original dimension
        }
        else
            pNew[i] = pSourceDims[i];

    return pNew;    // must be deleted by caller
}

void ScDPGroupTableData::FillGroupValues( ScDPItemData* pItemData, long nCount, const long* pDims )
{
    long nGroupedColumns = aGroups.size();

    for (long nDim=0; nDim<nCount; nDim++)
    {
        const ScDPDateGroupHelper* pDateHelper = NULL;

        long nColumn = pDims[nDim];
        if ( nColumn >= nSourceCount && nColumn < nSourceCount + nGroupedColumns )
        {
            const ScDPGroupDimension& rGroupDim = aGroups[nColumn - nSourceCount];
            pDateHelper = rGroupDim.GetDateHelper();
            if ( !pDateHelper )                         // date is handled below
            {
                const ScDPGroupItem* pGroupItem = rGroupDim.GetGroupForData( pItemData[nDim] );
                if ( pGroupItem )
                    pItemData[nDim] = pGroupItem->GetName();
                // if no group is found, keep the original name
            }
        }
        else if ( IsNumGroupDimension( nColumn ) )
        {
            pDateHelper = pNumGroups[nColumn].GetDateHelper();
            if ( !pDateHelper )                         // date is handled below
            {
                if ( pItemData[nDim].bHasValue )
                {
                    ScDPNumGroupInfo aNumInfo;
                    bool bHasNonInteger = false;
                    sal_Unicode cDecSeparator = 0;
                    GetNumGroupInfo( nColumn, aNumInfo, bHasNonInteger, cDecSeparator );
                    double fGroupValue;
                    String aGroupName = lcl_GetNumGroupForValue( pItemData[nDim].fValue,
                                                    aNumInfo, bHasNonInteger, cDecSeparator, fGroupValue, pDoc );

                    // consistent with TypedStrData in GetNumEntries
                    pItemData[nDim] = ScDPItemData( aGroupName, fGroupValue, TRUE );
                }
                // else (textual) keep original value
            }
        }

        if ( pDateHelper )
        {
            if ( pItemData[nDim].bHasValue )
            {
                sal_Int32 nPartValue = lcl_GetDatePartValue(
                        pItemData[nDim].fValue, pDateHelper->GetDatePart(), pDoc->GetFormatTable(),
                        &pDateHelper->GetNumInfo() );
                pItemData[nDim] = ScDPItemData( String(), nPartValue, TRUE );
            }
        }
    }
}

BOOL ScDPGroupTableData::GetNextRow( const ScDPTableIteratorParam& rParam )
{
    long nGroupedColumns = aGroups.size();

    //
    //  call source with a param containing only dimension numbers valid for source
    //

    long* pSourceCols = CopyFields( rParam.pCols, rParam.nColCount );
    long* pSourceRows = CopyFields( rParam.pRows, rParam.nRowCount );
    long* pSourcePages = CopyFields( rParam.pPages, rParam.nPageCount );
    long* pSourceDats = CopyFields( rParam.pDats, rParam.nDatCount );
    //  data fields can't be groups

    ScDPTableIteratorParam aSourceParam( rParam.nColCount, pSourceCols, rParam.pColData,
                                         rParam.nRowCount, pSourceRows, rParam.pRowData,
                                         rParam.nPageCount, pSourcePages, rParam.pPageData,
                                         rParam.nDatCount, pSourceDats, rParam.pValues );

    BOOL bRet = pSourceData->GetNextRow( aSourceParam );

    delete pSourceCols;
    delete pSourceRows;
    delete pSourcePages;
    delete pSourceDats;

    //
    //  fill values for groups
    //

    FillGroupValues( rParam.pColData, rParam.nColCount, rParam.pCols );
    FillGroupValues( rParam.pRowData, rParam.nRowCount, rParam.pRows );
    FillGroupValues( rParam.pPageData, rParam.nPageCount, rParam.pPages );
    // data is left unchanged - useful only for count

    return bRet;
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
        const ScDPGroupDimension* pDim = aIter;
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

// -----------------------------------------------------------------------

