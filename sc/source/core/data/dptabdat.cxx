/*************************************************************************
 *
 *  $RCSfile: dptabdat.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:31:37 $
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




