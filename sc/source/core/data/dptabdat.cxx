/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dptabdat.cxx,v $
 * $Revision: 1.13 $
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

#include <rtl/math.hxx>
#include <tools/debug.hxx>
#include <tools/date.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/collatorwrapper.hxx>

#include "dptabdat.hxx"
#include "global.hxx"

// -----------------------------------------------------------------------

BOOL ScDPItemData::IsCaseInsEqual( const ScDPItemData& r ) const
{
    //! pass Transliteration?
    //! inline?
    return bHasValue ? ( r.bHasValue && rtl::math::approxEqual( fValue, r.fValue ) ) :
                       ( !r.bHasValue &&
                        ScGlobal::pTransliteration->isEqual( aString, r.aString ) );
}

size_t ScDPItemData::Hash() const
{
    if ( bHasValue )
        return (size_t) rtl::math::approxFloor( fValue );
    else
        // If we do unicode safe case insensitive hash we can drop
        // ScDPItemData::operator== and use ::IsCasInsEqual
        return rtl_ustr_hashCode_WithLength( aString.GetBuffer(), aString.Len() );
}

BOOL ScDPItemData::operator==( const ScDPItemData& r ) const
{
    if ( bHasValue )
    {
        if ( r.bHasValue )
            return rtl::math::approxEqual( fValue, r.fValue );
        else
            return FALSE;
    }
    else if ( r.bHasValue )
        return FALSE;
    else
        // need exact equality until we have a safe case insensitive string hash
        return aString == r.aString;
}

sal_Int32 ScDPItemData::Compare( const ScDPItemData& rA,
                                 const ScDPItemData& rB )
{
    if ( rA.bHasValue )
    {
        if ( rB.bHasValue )
        {
            if ( rtl::math::approxEqual( rA.fValue, rB.fValue ) )
                return 0;
            else if ( rA.fValue < rB.fValue )
                return -1;
            else
                return 1;
        }
        else
            return -1;           // values first
    }
    else if ( rB.bHasValue )
        return 1;                // values first
    else
        return ScGlobal::pCollator->compareString( rA.aString, rB.aString );
}

// -----------------------------------------------------------------------

ScDPTableIteratorParam::ScDPTableIteratorParam(
                            long nCCount, const long* pC, ScDPItemData* pCDat,
                            long nRCount, const long* pR, ScDPItemData* pRDat,
                            long nPCount, const long* pP, ScDPItemData* pPDat,
                            long nDCount, const long* pD, ScDPValueData* pV ) :
    nColCount( (SCSIZE)nCCount ),
    pCols    ( pC ),
    pColData ( pCDat ),
    nRowCount( (SCSIZE)nRCount ),
    pRows    ( pR ),
    pRowData ( pRDat ),
    nPageCount( (SCSIZE)nPCount ),
    pPages   ( pP ),
    pPageData( pPDat ),
    nDatCount( (SCSIZE)nDCount ),
    pDats    ( pD ),
    pValues  ( pV )
{
}

// -----------------------------------------------------------------------

ScDPTableData::ScDPTableData()
{
    nLastDateVal = nLastHier = nLastLevel = nLastRet = -1;      // invalid

    //! reset before new calculation (in case the base date is changed)
}

ScDPTableData::~ScDPTableData()
{
}

long ScDPTableData::GetDatePart( long nDateVal, long nHierarchy, long nLevel )
{
    if ( nDateVal == nLastDateVal && nHierarchy == nLastHier && nLevel == nLastLevel )
        return nLastRet;

    Date aDate( 30,12,1899 );                   //! get from source data (and cache here)
    aDate += nDateVal;

    long nRet = 0;
    switch (nHierarchy)
    {
        case SC_DAPI_HIERARCHY_QUARTER:
            switch (nLevel)
            {
                case 0: nRet = aDate.GetYear();                 break;
                case 1: nRet = (aDate.GetMonth()-1) / 3 + 1;    break;
                case 2: nRet = aDate.GetMonth();                break;
                case 3: nRet = aDate.GetDay();                  break;
                default:
                    DBG_ERROR("GetDatePart: wrong level");
            }
            break;
        case SC_DAPI_HIERARCHY_WEEK:
            switch (nLevel)
            {
                //! use settings for different definitions
                case 0: nRet = aDate.GetYear();                 break;      //!...
                case 1: nRet = aDate.GetWeekOfYear();           break;
                case 2: nRet = (long)aDate.GetDayOfWeek();      break;
                default:
                    DBG_ERROR("GetDatePart: wrong level");
            }
            break;
        default:
            DBG_ERROR("GetDatePart: wrong hierarchy");
    }

    nLastDateVal = nDateVal;
    nLastHier    = nHierarchy;
    nLastLevel   = nLevel;
    nLastRet     = nRet;

    return nRet;
}

UINT32 ScDPTableData::GetNumberFormat(long)
{
    return 0;           // default format
}

BOOL ScDPTableData::IsBaseForGroup(long) const
{
    return FALSE;       // always false
}

long ScDPTableData::GetGroupBase(long) const
{
    return -1;          // always none
}

BOOL ScDPTableData::IsNumOrDateGroup(long) const
{
    return FALSE;       // always false
}

BOOL ScDPTableData::IsInGroup( const ScDPItemData&, long,
                               const ScDPItemData&, long ) const
{
    DBG_ERROR("IsInGroup shouldn't be called for non-group data");
    return FALSE;
}

BOOL ScDPTableData::HasCommonElement( const ScDPItemData&, long,
                                      const ScDPItemData&, long ) const
{
    DBG_ERROR("HasCommonElement shouldn't be called for non-group data");
    return FALSE;
}

// -----------------------------------------------------------------------




