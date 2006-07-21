/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dptabdat.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 10:58:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <rtl/math.hxx>
#include <tools/debug.hxx>
#include <tools/date.hxx>
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

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

UINT32 ScDPTableData::GetNumberFormat(long nDim)
{
    return 0;           // default format
}

BOOL ScDPTableData::IsBaseForGroup(long nDim) const
{
    return FALSE;       // always false
}

long ScDPTableData::GetGroupBase(long nGroupDim) const
{
    return -1;          // always none
}

BOOL ScDPTableData::IsNumOrDateGroup(long nDim) const
{
    return FALSE;       // always false
}

BOOL ScDPTableData::IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                               const ScDPItemData& rBaseData, long nBaseIndex ) const
{
    DBG_ERROR("IsInGroup shouldn't be called for non-group data");
    return FALSE;
}

BOOL ScDPTableData::HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                      const ScDPItemData& rSecondData, long nSecondIndex ) const
{
    DBG_ERROR("HasCommonElement shouldn't be called for non-group data");
    return FALSE;
}

// -----------------------------------------------------------------------




