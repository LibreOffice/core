/*************************************************************************
 *
 *  $RCSfile: dptabres.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:31:51 $
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

#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <math.h>
#include <float.h>          //! Test !!!

#include <algorithm>

#include "dptabres.hxx"
#include "dptabsrc.hxx"
#include "dptabdat.hxx"
#include "global.hxx"
#include "subtotal.hxx"
#include "globstr.hrc"
#include "datauno.hxx"      // ScDataUnoConversion

#include "document.hxx"     // for DumpState only!

#include <com/sun/star/sheet/DataResultFlags.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#include <com/sun/star/sheet/DataPilotFieldShowItemsMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>

using namespace com::sun::star;

// -----------------------------------------------------------------------

SV_IMPL_PTRARR( ScDPResultMembers, ScDPResultMemberPtr );
SV_IMPL_PTRARR( ScDPDataMembers, ScDPDataMemberPtr );

// -----------------------------------------------------------------------

static USHORT nFuncStrIds[12] =     // passend zum enum ScSubTotalFunc
{
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

// -----------------------------------------------------------------------

//
// function objects for sorting of the column and row members:
//

class ScDPRowMembersOrder
{
    ScDPResultDimension& rDimension;
    long                 nMeasure;
    BOOL                 bAscending;

public:
            ScDPRowMembersOrder( ScDPResultDimension& rDim, long nM, BOOL bAsc ) :
                rDimension(rDim),
                nMeasure(nM),
                bAscending(bAsc)
            {}
            ~ScDPRowMembersOrder() {}

    BOOL operator()( sal_Int32 nIndex1, sal_Int32 nIndex2 ) const;
};

class ScDPColMembersOrder
{
    ScDPDataDimension& rDimension;
    long               nMeasure;
    BOOL               bAscending;

public:
            ScDPColMembersOrder( ScDPDataDimension& rDim, long nM, BOOL bAsc ) :
                rDimension(rDim),
                nMeasure(nM),
                bAscending(bAsc)
            {}
            ~ScDPColMembersOrder() {}

    BOOL operator()( sal_Int32 nIndex1, sal_Int32 nIndex2 ) const;
};

BOOL lcl_IsLess( ScDPDataMember* pDataMember1, ScDPDataMember* pDataMember2, long nMeasure, BOOL bAscending )
{
    // members can be NULL if used for rows

    ScDPSubTotalState aEmptyState;
    const ScDPAggData* pAgg1 = pDataMember1 ? pDataMember1->GetConstAggData( nMeasure, aEmptyState ) : NULL;
    const ScDPAggData* pAgg2 = pDataMember2 ? pDataMember2->GetConstAggData( nMeasure, aEmptyState ) : NULL;

    BOOL bError1 = pAgg1 && pAgg1->HasError();
    BOOL bError2 = pAgg2 && pAgg2->HasError();
    if ( bError1 )
    {
        if ( bError2 )
            return FALSE;       // equal
        else
            return FALSE;       // errors are always sorted at the end
    }
    else if ( bError2 )
        return TRUE;            // errors are always sorted at the end
    else
    {
        double fVal1 = ( pAgg1 && pAgg1->HasData() ) ? pAgg1->GetResult() : 0.0;    // no data is sorted as 0
        double fVal2 = ( pAgg2 && pAgg2->HasData() ) ? pAgg2->GetResult() : 0.0;

        // compare values
        // don't have to check approxEqual, as this is the only sort criterion

        return bAscending ? ( fVal1 < fVal2 ) : ( fVal1 > fVal2 );
    }
}

BOOL lcl_IsEqual( ScDPDataMember* pDataMember1, ScDPDataMember* pDataMember2, long nMeasure )
{
    // members can be NULL if used for rows

    ScDPSubTotalState aEmptyState;
    const ScDPAggData* pAgg1 = pDataMember1 ? pDataMember1->GetConstAggData( nMeasure, aEmptyState ) : NULL;
    const ScDPAggData* pAgg2 = pDataMember2 ? pDataMember2->GetConstAggData( nMeasure, aEmptyState ) : NULL;

    BOOL bError1 = pAgg1 && pAgg1->HasError();
    BOOL bError2 = pAgg2 && pAgg2->HasError();
    if ( bError1 )
    {
        if ( bError2 )
            return TRUE;        // equal
        else
            return FALSE;
    }
    else if ( bError2 )
        return FALSE;
    else
    {
        double fVal1 = ( pAgg1 && pAgg1->HasData() ) ? pAgg1->GetResult() : 0.0;    // no data is sorted as 0
        double fVal2 = ( pAgg2 && pAgg2->HasData() ) ? pAgg2->GetResult() : 0.0;

        // compare values
        // this is used to find equal data at the end of the AutoShow range, so approxEqual must be used

        return rtl::math::approxEqual( fVal1, fVal2 );
    }
}

BOOL ScDPRowMembersOrder::operator()( sal_Int32 nIndex1, sal_Int32 nIndex2 ) const
{
    ScDPResultMember* pMember1 = rDimension.GetMember(nIndex1);
    ScDPResultMember* pMember2 = rDimension.GetMember(nIndex2);

    //  GetDataRoot can be NULL if there was no data.
    //  IsVisible == FALSE can happen after AutoShow.
    ScDPDataMember* pDataMember1 = pMember1->IsVisible() ? pMember1->GetDataRoot() : NULL;
    ScDPDataMember* pDataMember2 = pMember2->IsVisible() ? pMember2->GetDataRoot() : NULL;

    return lcl_IsLess( pDataMember1, pDataMember2, nMeasure, bAscending );
}

BOOL ScDPColMembersOrder::operator()( sal_Int32 nIndex1, sal_Int32 nIndex2 ) const
{
    ScDPDataMember* pDataMember1 = rDimension.GetMember(nIndex1);
    ScDPDataMember* pDataMember2 = rDimension.GetMember(nIndex2);

    if ( pDataMember1 && !pDataMember1->IsVisible() )   //! IsColVisible?
        pDataMember1 = NULL;
    if ( pDataMember2 && !pDataMember2->IsVisible() )
        pDataMember2 = NULL;

    return lcl_IsLess( pDataMember1, pDataMember2, nMeasure, bAscending );
}

// -----------------------------------------------------------------------

ScDPInitState::ScDPInitState() :
    nCount( 0 )
{
    pIndex = new long[SC_DAPI_MAXFIELDS];
    pData = new ScDPItemData[SC_DAPI_MAXFIELDS];
}

ScDPInitState::~ScDPInitState()
{
    delete[] pIndex;
    delete[] pData;
}

void ScDPInitState::AddMember( long nSourceIndex, const ScDPItemData& rName )
{
    DBG_ASSERT( nCount < SC_DAPI_MAXFIELDS, "too many InitState members" )
    if ( nCount < SC_DAPI_MAXFIELDS )
    {
        pIndex[nCount] = nSourceIndex;
        pData[nCount] = rName;
        ++nCount;
    }
}

void ScDPInitState::RemoveMember()
{
    DBG_ASSERT( nCount > 0, "RemoveColIndex without index" )
    if ( nCount > 0 )
        --nCount;
}

const ScDPItemData* ScDPInitState::GetNameForIndex( long nIndexValue ) const
{
    for (long i=0; i<nCount; i++)
        if ( pIndex[i] == nIndexValue )
            return &pData[i];

    return NULL;    // not found
}

// -----------------------------------------------------------------------

void lcl_DumpStrings( const String& rType, const String& rName, const String& rValue,
                    ScDocument* pDoc, ScAddress& rPos )
{
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    pDoc->SetString( nCol,   nRow, nTab, rType );
    pDoc->SetString( nCol+1, nRow, nTab, rName );
    pDoc->SetString( nCol+2, nRow, nTab, rValue );
    rPos.SetRow( nRow + 1 );
}

void lcl_Indent( ScDocument* pDoc, SCROW nStartRow, const ScAddress& rPos )
{
    SCCOL nCol = rPos.Col();
    SCTAB nTab = rPos.Tab();

    String aString;
    for (SCROW nRow = nStartRow; nRow < rPos.Row(); nRow++)
    {
        pDoc->GetString( nCol, nRow, nTab, aString );
        if ( aString.Len() )
        {
            aString.InsertAscii( "  ", 0 );
            pDoc->SetString( nCol, nRow, nTab, aString );
        }
    }
}

// -----------------------------------------------------------------------

ScDPRunningTotalState::ScDPRunningTotalState( ScDPResultMember* pColRoot, ScDPResultMember* pRowRoot ) :
    pColResRoot( pColRoot ),
    pRowResRoot( pRowRoot ),
    nColIndexPos( 0 ),
    nRowIndexPos( 0 )
{
    pColVisible = new long[SC_DAPI_MAXFIELDS+1];
    pColIndexes = new long[SC_DAPI_MAXFIELDS+1];
    pRowVisible = new long[SC_DAPI_MAXFIELDS+1];
    pRowIndexes = new long[SC_DAPI_MAXFIELDS+1];
    pColIndexes[0] = -1;
    pRowIndexes[0] = -1;
}

ScDPRunningTotalState::~ScDPRunningTotalState()
{
    delete[] pColVisible;
    delete[] pColIndexes;
    delete[] pRowVisible;
    delete[] pRowIndexes;
}

void ScDPRunningTotalState::AddColIndex( long nVisible, long nSorted )
{
    DBG_ASSERT( nColIndexPos < SC_DAPI_MAXFIELDS, "too many column indexes" )
    if ( nColIndexPos < SC_DAPI_MAXFIELDS )
    {
        pColVisible[nColIndexPos] = nVisible;
        pColIndexes[nColIndexPos] = nSorted;
        pColVisible[nColIndexPos+1] = -1;
        pColIndexes[nColIndexPos+1] = -1;
        ++nColIndexPos;
    }
}

void ScDPRunningTotalState::AddRowIndex( long nVisible, long nSorted )
{
    DBG_ASSERT( nRowIndexPos < SC_DAPI_MAXFIELDS, "too many row indexes" )
    if ( nRowIndexPos < SC_DAPI_MAXFIELDS )
    {
        pRowVisible[nRowIndexPos] = nVisible;
        pRowIndexes[nRowIndexPos] = nSorted;
        pRowVisible[nRowIndexPos+1] = -1;
        pRowIndexes[nRowIndexPos+1] = -1;
        ++nRowIndexPos;
    }
}

void ScDPRunningTotalState::RemoveColIndex()
{
    DBG_ASSERT( nColIndexPos > 0, "RemoveColIndex without index" )
    if ( nColIndexPos > 0 )
    {
        --nColIndexPos;
        pColVisible[nColIndexPos] = -1;
        pColIndexes[nColIndexPos] = -1;
    }
}

void ScDPRunningTotalState::RemoveRowIndex()
{
    DBG_ASSERT( nRowIndexPos > 0, "RemoveRowIndex without index" )
    if ( nRowIndexPos > 0 )
    {
        --nRowIndexPos;
        pRowVisible[nRowIndexPos] = -1;
        pRowIndexes[nRowIndexPos] = -1;
    }
}

// -----------------------------------------------------------------------

ScDPRelativePos::ScDPRelativePos( long nBase, long nDir ) :
    nBasePos( nBase ),
    nDirection( nDir )
{
}

// -----------------------------------------------------------------------

void ScDPAggData::Update( const ScDPValueData& rNext, ScSubTotalFunc eFunc, const ScDPSubTotalState& rSubState )
{
    if (nCount<0)       // error?
        return;         // nothing more...

    if ( rNext.nType == SC_VALTYPE_EMPTY )
        return;

    if ( rSubState.eColForce != SUBTOTAL_FUNC_NONE && rSubState.eRowForce != SUBTOTAL_FUNC_NONE &&
                                                        rSubState.eColForce != rSubState.eRowForce )
        return;
    if ( rSubState.eColForce != SUBTOTAL_FUNC_NONE ) eFunc = rSubState.eColForce;
    if ( rSubState.eRowForce != SUBTOTAL_FUNC_NONE ) eFunc = rSubState.eRowForce;

    if ( eFunc == SUBTOTAL_FUNC_NONE )
        return;

    if ( eFunc != SUBTOTAL_FUNC_CNT2 )          // CNT2 counts everything, incl. strings and errors
    {
        if ( rNext.nType == SC_VALTYPE_ERROR )
        {
            nCount = -1;        // -1 for error (not for CNT2)
            return;
        }
        if ( rNext.nType == SC_VALTYPE_STRING )
            return;             // ignore
    }

    ++nCount;           // for all functions

    switch (eFunc)
    {
        case SUBTOTAL_FUNC_SUM:
        case SUBTOTAL_FUNC_AVE:
            if ( !SubTotal::SafePlus( fVal, rNext.fValue ) )
                nCount = -1;                            // -1 for error
            break;
        case SUBTOTAL_FUNC_PROD:
            if ( nCount == 1 )          // copy first value (fVal is initialized to 0)
                fVal = rNext.fValue;
            else if ( !SubTotal::SafeMult( fVal, rNext.fValue ) )
                nCount = -1;                            // -1 for error
            break;
        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:
            //  nothing more than incrementing nCount
            break;
        case SUBTOTAL_FUNC_MAX:
            if ( nCount == 1 || rNext.fValue > fVal )
                fVal = rNext.fValue;
            break;
        case SUBTOTAL_FUNC_MIN:
            if ( nCount == 1 || rNext.fValue < fVal )
                fVal = rNext.fValue;
            break;
        case SUBTOTAL_FUNC_STD:
        case SUBTOTAL_FUNC_STDP:
        case SUBTOTAL_FUNC_VAR:
        case SUBTOTAL_FUNC_VARP:
            {
                // fAux is used to sum up squares
                if ( !SubTotal::SafePlus( fVal, rNext.fValue ) )
                    nCount = -1;                            // -1 for error
                double fAdd = rNext.fValue;
                if ( !SubTotal::SafeMult( fAdd, rNext.fValue ) ||
                     !SubTotal::SafePlus( fAux, fAdd ) )
                    nCount = -1;                            // -1 for error
            }
            break;
        default:
            DBG_ERROR("invalid function");
    }
}

void ScDPAggData::Calculate( ScSubTotalFunc eFunc, const ScDPSubTotalState& rSubState )
{
    //  calculate the original result
    //  (without reference value, used as the basis for reference value calculation)

    //  called several times at the cross-section of several subtotals - don't calculate twice then
    if ( IsCalculated() )
        return;

    if ( rSubState.eColForce != SUBTOTAL_FUNC_NONE ) eFunc = rSubState.eColForce;
    if ( rSubState.eRowForce != SUBTOTAL_FUNC_NONE ) eFunc = rSubState.eRowForce;

    if ( eFunc == SUBTOTAL_FUNC_NONE )      // this happens when there is no data dimension
    {
        nCount = SC_DPAGG_RESULT_EMPTY;     // make sure there's a valid state for HasData etc.
        return;
    }

    //  check the error conditions for the selected function

    BOOL bError = FALSE;
    switch (eFunc)
    {
        case SUBTOTAL_FUNC_SUM:
        case SUBTOTAL_FUNC_PROD:
        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:
            bError = ( nCount < 0 );        // only real errors
            break;

        case SUBTOTAL_FUNC_AVE:
        case SUBTOTAL_FUNC_MAX:
        case SUBTOTAL_FUNC_MIN:
        case SUBTOTAL_FUNC_STDP:
        case SUBTOTAL_FUNC_VARP:
            bError = ( nCount <= 0 );       // no data is an error
            break;

        case SUBTOTAL_FUNC_STD:
        case SUBTOTAL_FUNC_VAR:
            bError = ( nCount < 2 );        // need at least 2 values
            break;

        default:
            DBG_ERROR("invalid function");
    }

    //  calculate the selected function

    double fResult = 0.0;
    if ( !bError )
    {
        switch (eFunc)
        {
            case SUBTOTAL_FUNC_MAX:
            case SUBTOTAL_FUNC_MIN:
            case SUBTOTAL_FUNC_SUM:
            case SUBTOTAL_FUNC_PROD:
                //  different error conditions are handled above
                fResult = fVal;
                break;

            case SUBTOTAL_FUNC_CNT:
            case SUBTOTAL_FUNC_CNT2:
                fResult = nCount;
                break;

            case SUBTOTAL_FUNC_AVE:
                if ( nCount > 0 )
                    fResult = fVal / (double) nCount;
                break;

            //! use safe mul for fVal * fVal

            case SUBTOTAL_FUNC_STD:
                if ( nCount >= 2 )
                    fResult = sqrt((fAux - fVal*fVal/(double)(nCount)) / (double)(nCount-1));
                break;
            case SUBTOTAL_FUNC_VAR:
                if ( nCount >= 2 )
                    fResult = (fAux - fVal*fVal/(double)(nCount)) / (double)(nCount-1);
                break;
            case SUBTOTAL_FUNC_STDP:
                if ( nCount > 0 )
                    fResult = sqrt((fAux - fVal*fVal/(double)(nCount)) / (double)nCount);
                break;
            case SUBTOTAL_FUNC_VARP:
                if ( nCount > 0 )
                    fResult = (fAux - fVal*fVal/(double)(nCount)) / (double)nCount;
                break;
            default:
                DBG_ERROR("invalid function");
        }
    }

    BOOL bEmpty = ( nCount == 0 );          // no data

    //  store the result
    //  Empty is checked first, so empty results are shown empty even for "average" etc.
    //  If these results should be treated as errors in reference value calculations,
    //  a separate state value (EMPTY_ERROR) is needed.
    //  Now, for compatibility, empty "average" results are counted as 0.

    if ( bEmpty )
        nCount = SC_DPAGG_RESULT_EMPTY;
    else if ( bError )
        nCount = SC_DPAGG_RESULT_ERROR;
    else
        nCount = SC_DPAGG_RESULT_VALID;

    if ( bEmpty || bError )
        fResult = 0.0;      // default, in case the state is later modified

    fVal = fResult;         // used directly from now on
    fAux = 0.0;             // used for running total or original result of reference value
}

BOOL ScDPAggData::IsCalculated() const
{
    return ( nCount <= SC_DPAGG_RESULT_EMPTY );
}

double ScDPAggData::GetResult() const
{
    DBG_ASSERT( IsCalculated(), "ScDPAggData not calculated" );

    return fVal;        // use calculated value
}

BOOL ScDPAggData::HasError() const
{
    DBG_ASSERT( IsCalculated(), "ScDPAggData not calculated" );

    return ( nCount == SC_DPAGG_RESULT_ERROR );
}

BOOL ScDPAggData::HasData() const
{
    DBG_ASSERT( IsCalculated(), "ScDPAggData not calculated" );

    return ( nCount != SC_DPAGG_RESULT_EMPTY );     // values or error
}

void ScDPAggData::SetResult( double fNew )
{
    DBG_ASSERT( IsCalculated(), "ScDPAggData not calculated" );

    fVal = fNew;        // don't reset error flag
}

void ScDPAggData::SetError()
{
    DBG_ASSERT( IsCalculated(), "ScDPAggData not calculated" );

    nCount = SC_DPAGG_RESULT_ERROR;
}

void ScDPAggData::SetEmpty( BOOL bSet )
{
    DBG_ASSERT( IsCalculated(), "ScDPAggData not calculated" );

    if ( bSet )
        nCount = SC_DPAGG_RESULT_EMPTY;
    else
        nCount = SC_DPAGG_RESULT_VALID;
}

double ScDPAggData::GetAuxiliary() const
{
    // after Calculate, fAux is used as auxiliary value for running totals and reference values
    DBG_ASSERT( IsCalculated(), "ScDPAggData not calculated" );

    return fAux;
}

void ScDPAggData::SetAuxiliary( double fNew )
{
    // after Calculate, fAux is used as auxiliary value for running totals and reference values
    DBG_ASSERT( IsCalculated(), "ScDPAggData not calculated" );

    fAux = fNew;
}

ScDPAggData* ScDPAggData::GetChild()
{
    if (!pChild)
        pChild = new ScDPAggData;
    return pChild;
}

void ScDPAggData::Reset()
{
    fVal = 0.0;
    fAux = 0.0;
    nCount = SC_DPAGG_EMPTY;
    delete pChild;
    pChild = NULL;
}

// -----------------------------------------------------------------------

ScDPRowTotals::ScDPRowTotals() :
    bIsInColRoot( FALSE )
{
}

ScDPRowTotals::~ScDPRowTotals()
{
}

ScDPAggData* lcl_GetChildTotal( ScDPAggData* pFirst, long nMeasure )
{
    DBG_ASSERT( nMeasure >= 0, "GetColTotal: no measure" );

    ScDPAggData* pAgg = pFirst;
    long nSkip = nMeasure;

    // subtotal settings are ignored - colum/row totals exist once per measure

    for ( long nPos=0; nPos<nSkip; nPos++ )
        pAgg = pAgg->GetChild();    // column total is constructed empty - children need to be created

    if ( !pAgg->IsCalculated() )
    {
        // for first use, simulate an empty calculation
        ScDPSubTotalState aEmptyState;
        pAgg->Calculate( SUBTOTAL_FUNC_SUM, aEmptyState );
    }

    return pAgg;
}

ScDPAggData* ScDPRowTotals::GetRowTotal( long nMeasure )
{
    return lcl_GetChildTotal( &aRowTotal, nMeasure );
}

ScDPAggData* ScDPRowTotals::GetGrandTotal( long nMeasure )
{
    return lcl_GetChildTotal( &aGrandTotal, nMeasure );
}

// -----------------------------------------------------------------------

ScSubTotalFunc lcl_GetForceFunc( ScDPLevel* pLevel, long nFuncNo )
{
    ScSubTotalFunc eRet = SUBTOTAL_FUNC_NONE;
    if ( pLevel )
    {
        //! direct access via ScDPLevel

        uno::Sequence<sheet::GeneralFunction> aSeq = pLevel->getSubTotals();
        long nSequence = aSeq.getLength();
        if ( nSequence && aSeq[0] != sheet::GeneralFunction_AUTO )
        {
            // For manual subtotals, "automatic" is added as first function.
            // ScDPResultMember::GetSubTotalCount adds to the count, here NONE has to be
            // returned as the first function then.

            --nFuncNo;      // keep NONE for first (check below), move the other entries
        }

        if ( nFuncNo >= 0 && nFuncNo < nSequence )
        {
            sheet::GeneralFunction eUser = aSeq.getConstArray()[nFuncNo];
            if (eUser != sheet::GeneralFunction_AUTO)
                eRet = ScDataUnoConversion::GeneralToSubTotal( eUser );
        }
    }
    return eRet;
}

// -----------------------------------------------------------------------

ScDPResultData::ScDPResultData( ScDPSource* pSrc ) :        //! Ref
    pSource( pSrc ),
    nMeasCount( 0 ),
    pMeasFuncs( NULL ),
    pMeasRefs( NULL ),
    pMeasRefOrient( NULL ),
    pMeasNames( NULL ),
    bLateInit( FALSE ),
    bDataAtCol( FALSE ),
    bDataAtRow( FALSE )
{
}

ScDPResultData::~ScDPResultData()
{
    delete[] pMeasFuncs;
    delete[] pMeasRefs;
    delete[] pMeasRefOrient;
    delete[] pMeasNames;
}

void ScDPResultData::SetMeasureData( long nCount, const ScSubTotalFunc* pFunctions,
                                    const sheet::DataPilotFieldReference* pRefs, const USHORT* pRefOrient,
                                    const String* pNames )
{
    delete[] pMeasFuncs;
    delete[] pMeasRefs;
    delete[] pMeasRefOrient;
    delete[] pMeasNames;
    if ( nCount )
    {
        nMeasCount = nCount;
        pMeasFuncs = new ScSubTotalFunc[nCount];
        pMeasRefs  = new sheet::DataPilotFieldReference[nCount];
        pMeasRefOrient = new USHORT[nCount];
        pMeasNames = new String[nCount];
        for (long i=0; i<nCount; i++)
        {
            pMeasFuncs[i] = pFunctions[i];
            pMeasRefs[i]  = pRefs[i];
            pMeasRefOrient[i] = pRefOrient[i];
            pMeasNames[i] = pNames[i];
        }
    }
    else
    {
        //  use one dummy measure
        nMeasCount = 1;
        pMeasFuncs = new ScSubTotalFunc[1];
        pMeasFuncs[0] = SUBTOTAL_FUNC_NONE;
        pMeasRefs  = new sheet::DataPilotFieldReference[1]; // default ctor is ok
        pMeasRefOrient = new USHORT[1];
        pMeasRefOrient[0] = sheet::DataPilotFieldOrientation_HIDDEN;
        pMeasNames = new String[1];
        pMeasNames[0] = ScGlobal::GetRscString( STR_EMPTYDATA );
    }
}

void ScDPResultData::SetDataLayoutOrientation( USHORT nOrient )
{
    bDataAtCol = ( nOrient == sheet::DataPilotFieldOrientation_COLUMN );
    bDataAtRow = ( nOrient == sheet::DataPilotFieldOrientation_ROW );
}

void ScDPResultData::SetLateInit( BOOL bSet )
{
    bLateInit = bSet;
}

long ScDPResultData::GetColStartMeasure() const
{
    if ( nMeasCount == 1 ) return 0;
    return bDataAtCol ? SC_DPMEASURE_ALL : SC_DPMEASURE_ANY;
}

long ScDPResultData::GetRowStartMeasure() const
{
    if ( nMeasCount == 1 ) return 0;
    return bDataAtRow ? SC_DPMEASURE_ALL : SC_DPMEASURE_ANY;
}

ScSubTotalFunc ScDPResultData::GetMeasureFunction(long nMeasure) const
{
    DBG_ASSERT( pMeasFuncs && nMeasure < nMeasCount, "bumm" );
    return pMeasFuncs[nMeasure];
}

const sheet::DataPilotFieldReference& ScDPResultData::GetMeasureRefVal(long nMeasure) const
{
    DBG_ASSERT( pMeasRefs && nMeasure < nMeasCount, "bumm" );
    return pMeasRefs[nMeasure];
}

USHORT ScDPResultData::GetMeasureRefOrient(long nMeasure) const
{
    DBG_ASSERT( pMeasRefOrient && nMeasure < nMeasCount, "bumm" );
    return pMeasRefOrient[nMeasure];
}

String ScDPResultData::GetMeasureString(long nMeasure, BOOL bForce, ScSubTotalFunc eForceFunc) const
{
    //  with bForce==TRUE, return function instead of "result" for single measure
    //  with eForceFunc != SUBTOTAL_FUNC_NONE, always use eForceFunc

    if ( nMeasure < 0 || ( nMeasCount == 1 && !bForce && eForceFunc == SUBTOTAL_FUNC_NONE ) )
    {
        //  for user-specified subtotal function with all measures,
        //  display only function name
        if ( eForceFunc != SUBTOTAL_FUNC_NONE )
            return ScGlobal::GetRscString(nFuncStrIds[eForceFunc]);

        return ScGlobal::GetRscString(STR_TABLE_ERGEBNIS);
    }
    else
    {
        DBG_ASSERT( pMeasNames && nMeasure < nMeasCount, "bumm" );

        String aRet;
        ScSubTotalFunc eFunc = ( eForceFunc == SUBTOTAL_FUNC_NONE ) ?
                                    GetMeasureFunction(nMeasure) : eForceFunc;
        USHORT nId = nFuncStrIds[eFunc];
        if (nId)
        {
            aRet += ScGlobal::GetRscString(nId);        // function name
            aRet.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " - " ));
        }
        aRet += pMeasNames[nMeasure];                   // field name

        return aRet;
    }
}

String ScDPResultData::GetMeasureDimensionName(long nMeasure) const
{
    if ( nMeasure < 0 )
    {
        DBG_ERROR("GetMeasureDimensionName: negative");
        return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("***"));
    }

    return pSource->GetDataDimName( nMeasure );
}

BOOL ScDPResultData::IsBaseForGroup( long nDim ) const
{
    return pSource->GetData()->IsBaseForGroup( nDim );
}

long ScDPResultData::GetGroupBase( long nGroupDim ) const
{
    return pSource->GetData()->GetGroupBase( nGroupDim );
}

BOOL ScDPResultData::IsNumOrDateGroup( long nDim ) const
{
    return pSource->GetData()->IsNumOrDateGroup( nDim );
}

BOOL ScDPResultData::IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                const ScDPItemData& rBaseData, long nBaseIndex ) const
{
    return pSource->GetData()->IsInGroup( rGroupData, nGroupIndex, rBaseData, nBaseIndex );
}

BOOL ScDPResultData::HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                       const ScDPItemData& rSecondData, long nSecondIndex ) const
{
    return pSource->GetData()->HasCommonElement( rFirstData, nFirstIndex, rSecondData, nSecondIndex );
}

// -----------------------------------------------------------------------


ScDPResultMember::ScDPResultMember( ScDPResultData* pData, ScDPDimension* pDim,
                                    ScDPLevel* pLev, ScDPMember* pDesc,
                                    BOOL bForceSub ) :
    pResultData( pData ),
    pParentDim( pDim ),
    pParentLevel( pLev ),
    pMemberDesc( pDesc ),
    pChildDimension( NULL ),
    pDataRoot( NULL ),
    bHasElements( FALSE ),
    bForceSubTotal( bForceSub ),
    bHasHiddenDetails( FALSE ),
    bInitialized( FALSE ),
    bAutoHidden( FALSE )
{
    // pParentLevel/pMemberDesc is 0 for root members
}

ScDPResultMember::~ScDPResultMember()
{
    delete pChildDimension;
    delete pDataRoot;
}

String ScDPResultMember::GetName() const
{
    if (pMemberDesc)
        return pMemberDesc->GetNameStr();
    else
        return ScGlobal::GetRscString(STR_PIVOT_TOTAL);         // root member
}

void ScDPResultMember::FillItemData( ScDPItemData& rData ) const
{
    if (pMemberDesc)
        pMemberDesc->FillItemData( rData );
    else
        rData.SetString( ScGlobal::GetRscString(STR_PIVOT_TOTAL) );     // root member
}

BOOL ScDPResultMember::IsNamedItem( const ScDPItemData& r ) const
{
    //! store ScDPMember pointer instead of ScDPMember ???

    if (pMemberDesc)
        return ((ScDPMember*)pMemberDesc)->IsNamedItem( r );
    return FALSE;
}

void ScDPResultMember::InitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev, ScDPInitState& rInitState )
{
    //  with LateInit, initialize only those members that have data
    if ( pResultData->IsLateInit() )
        return;

    bInitialized = TRUE;

    //  skip child dimension if details are not shown
    if ( pMemberDesc && !pMemberDesc->getShowDetails() )
    {
        if ( *ppDim )
            bHasHiddenDetails = TRUE;   // only if there is a next dimension
        return;
    }

    if ( *ppDim )
    {
        pChildDimension = new ScDPResultDimension( pResultData );
        pChildDimension->InitFrom( ppDim, ppLev, rInitState );
    }
}

void ScDPResultMember::LateInitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev, ScDPItemData* pItemData,
                                    ScDPInitState& rInitState )
{
    //  without LateInit, everything has already been initialized
    if ( !pResultData->IsLateInit() )
        return;

    bInitialized = TRUE;

    //  skip child dimension if details are not shown
    if ( pMemberDesc && !pMemberDesc->getShowDetails() )
    {
        if ( *ppDim )
            bHasHiddenDetails = TRUE;   // only if there is a next dimension
        return;
    }

    if ( *ppDim )
    {
        //  LateInitFrom is called several times...
        if ( !pChildDimension )
            pChildDimension = new ScDPResultDimension( pResultData );
        pChildDimension->LateInitFrom( ppDim, ppLev, pItemData, rInitState );
    }
}

BOOL ScDPResultMember::IsSubTotalInTitle(long nMeasure) const
{
    BOOL bRet = FALSE;
    if ( pChildDimension && pParentLevel &&
         pParentLevel->IsOutlineLayout() && pParentLevel->IsSubtotalsAtTop() )
    {
        long nUserSubStart;
        long nSubTotals = GetSubTotalCount( &nUserSubStart );
        nSubTotals -= nUserSubStart;            // visible count
        if ( nSubTotals )
        {
            if ( nMeasure == SC_DPMEASURE_ALL )
                nSubTotals *= pResultData->GetMeasureCount();   // number of subtotals that will be inserted

            // only a single subtotal row will be shown in the outline title row
            if ( nSubTotals == 1 )
                bRet = TRUE;
        }
    }
    return bRet;
}

long ScDPResultMember::GetSize(long nMeasure) const
{
    if ( !IsVisible() )
        return 0;

    long nExtraSpace = 0;
    if ( pParentLevel && pParentLevel->IsAddEmpty() )
        ++nExtraSpace;

    if ( pChildDimension )
    {
        //  outline layout takes up an extra row for the title only if subtotals aren't shown in that row
        if ( pParentLevel && pParentLevel->IsOutlineLayout() && !IsSubTotalInTitle( nMeasure ) )
            ++nExtraSpace;

        long nSize = pChildDimension->GetSize(nMeasure);
        long nUserSubStart;
        long nUserSubCount = GetSubTotalCount( &nUserSubStart );
        nUserSubCount -= nUserSubStart;     // for output size, use visible count
        if ( nUserSubCount )
        {
            if ( nMeasure == SC_DPMEASURE_ALL )
                nSize += pResultData->GetMeasureCount() * nUserSubCount;
            else
                nSize += nUserSubCount;
        }
        return nSize + nExtraSpace;
    }
    else
    {
        if ( nMeasure == SC_DPMEASURE_ALL )
            return pResultData->GetMeasureCount() + nExtraSpace;
        else
            return 1 + nExtraSpace;
    }
}

BOOL ScDPResultMember::IsVisible() const
{
    //  not initialized -> shouldn't be there at all
    //  (allocated only to preserve ordering)

    return ( bHasElements || ( pParentLevel && pParentLevel->getShowEmpty() ) ) && IsValid() && bInitialized;
}

BOOL ScDPResultMember::IsValid() const
{
    //  non-Valid members are left out of calculation

    //  was member set no invisible at the DataPilotSource?
    if ( pMemberDesc && !pMemberDesc->getIsVisible() )
        return FALSE;

    if ( bAutoHidden )
        return FALSE;

    return TRUE;
}

BOOL ScDPResultMember::HasHiddenDetails() const
{
    // bHasHiddenDetails is set only if the "show details" flag is off,
    // and there was a child dimension to skip

    return bHasHiddenDetails;
}

long ScDPResultMember::GetSubTotalCount( long* pUserSubStart ) const
{
    if ( pUserSubStart )
        *pUserSubStart = 0;     // default

    if ( bForceSubTotal )       // set if needed for root members
        return 1;               // grand total is always "automatic"
    else if ( pParentLevel )
    {
        //! direct access via ScDPLevel

        uno::Sequence<sheet::GeneralFunction> aSeq = pParentLevel->getSubTotals();
        long nSequence = aSeq.getLength();
        if ( nSequence && aSeq[0] != sheet::GeneralFunction_AUTO )
        {
            // For manual subtotals, always add "automatic" as first function
            // (used for calculation, but not for display, needed for sorting, see lcl_GetForceFunc)

            ++nSequence;
            if ( pUserSubStart )
                *pUserSubStart = 1;     // visible subtotals start at 1
        }
        return nSequence;
    }
    else
        return 0;
}

void ScDPResultMember::ProcessData( const ScDPItemData* pChildMembers, ScDPResultDimension* pDataDim,
                                        const ScDPItemData* pDataMembers, const ScDPValueData* pValues )
{
    SetHasElements();

    if (pChildDimension)
        pChildDimension->ProcessData( pChildMembers, pDataDim, pDataMembers, pValues );

    if ( pDataMembers )
    {
        if ( !pDataRoot )
        {
            pDataRoot = new ScDPDataMember( pResultData, NULL );
            if ( pDataDim )
                pDataRoot->InitFrom( pDataDim );            // recursive
        }

        ScDPSubTotalState aSubState;        // initial state

        long nUserSubCount = GetSubTotalCount();

        // Calculate at least automatic if no subtotals are selected,
        // show only own values if there's no child dimension (innermost).
        if ( !nUserSubCount || !pChildDimension )
            nUserSubCount = 1;

        for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)   // including hidden "automatic"
        {
            if ( pChildDimension )
            {
                aSubState.nRowSubTotalFunc = nUserPos;
                aSubState.eRowForce = lcl_GetForceFunc( pParentLevel, nUserPos );
            }

            pDataRoot->ProcessData( pDataMembers, pValues, aSubState );
        }
    }
}

void ScDPResultMember::FillMemberResults( uno::Sequence<sheet::MemberResult>* pSequences,
                                            long& rPos, long nMeasure, BOOL bRoot,
                                            const String* pMemberName,
                                            const String* pMemberCaption )
{
    //  IsVisible() test is in ScDPResultDimension::FillMemberResults
    //  (not on data layout dimension)

    long nSize = GetSize(nMeasure);
    sheet::MemberResult* pArray = pSequences->getArray();
    DBG_ASSERT( rPos+nSize <= pSequences->getLength(), "bumm" );

    BOOL bIsNumeric = FALSE;
    String aName;
    if ( pMemberName )          // if pMemberName != NULL, use instead of real member name
        aName = *pMemberName;
    else
    {
        ScDPItemData aItemData;
        FillItemData( aItemData );
        aName = aItemData.aString;
        bIsNumeric = aItemData.bHasValue;
    }

    if ( bIsNumeric && pParentDim && pResultData->IsNumOrDateGroup( pParentDim->GetDimension() ) )
    {
        // Numeric group dimensions use numeric entries for proper sorting,
        // but the group titles must be output as text.
        bIsNumeric = FALSE;
    }

    String aCaption = aName;
    if ( pMemberCaption )                   // use pMemberCaption if != NULL
        aCaption = *pMemberCaption;
    if (!aCaption.Len())
        aCaption = ScGlobal::GetRscString(STR_EMPTYDATA);

    if ( !bIsNumeric )
    {
        // add a "'" character so a string isn't parsed as value in the output cell
        //! have a separate bit in Flags (MemberResultFlags) instead?
        aCaption.Insert( (sal_Unicode) '\'', 0 );
    }

    if ( nSize && !bRoot )                  // root is overwritten by first dimension
    {
        pArray[rPos].Name    = rtl::OUString(aName);
        pArray[rPos].Caption = rtl::OUString(aCaption);
        pArray[rPos].Flags  |= sheet::MemberResultFlags::HASMEMBER;

        //  set "continue" flag (removed for subtotals later)
        for (long i=1; i<nSize; i++)
            pArray[rPos+i].Flags |= sheet::MemberResultFlags::CONTINUE;
    }

    long nExtraSpace = 0;
    if ( pParentLevel && pParentLevel->IsAddEmpty() )
        ++nExtraSpace;

    BOOL bTitleLine = FALSE;
    if ( pParentLevel && pParentLevel->IsOutlineLayout() )
        bTitleLine = TRUE;

    // if the subtotals are shown at the top (title row) in outline layout,
    // no extra row for the subtotals is needed
    BOOL bSubTotalInTitle = IsSubTotalInTitle( nMeasure );

    BOOL bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
    {
        if ( bTitleLine )           // in tabular layout the title is on a separate row
            ++rPos;                 // -> fill child dimension one row below

        if (bRoot)      // same sequence for root member
            pChildDimension->FillMemberResults( pSequences, rPos, nMeasure );
        else
            pChildDimension->FillMemberResults( pSequences + 1, rPos, nMeasure );

        if ( bTitleLine )           // title row is included in GetSize, so the following
            --rPos;                 // positions are calculated with the normal values
    }

    rPos += nSize;

    long nUserSubStart;
    long nUserSubCount = GetSubTotalCount(&nUserSubStart);
    if ( nUserSubCount && pChildDimension && !bSubTotalInTitle )
    {
        long nMemberMeasure = nMeasure;
        long nSubSize = pResultData->GetCountForMeasure(nMeasure);

        rPos -= nSubSize * (nUserSubCount - nUserSubStart);     // GetSize includes space for SubTotal
        rPos -= nExtraSpace;                                    // GetSize includes the empty line

        for (long nUserPos=nUserSubStart; nUserPos<nUserSubCount; nUserPos++)
        {
            for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
            {
                if ( nMeasure == SC_DPMEASURE_ALL )
                    nMemberMeasure = nSubCount;

                ScSubTotalFunc eForce = SUBTOTAL_FUNC_NONE;
                if (bHasChild)
                    eForce = lcl_GetForceFunc( pParentLevel, nUserPos );

                String aSubStr = aName;     //! caption?
                aSubStr += ' ';
                aSubStr += pResultData->GetMeasureString(nMemberMeasure, FALSE, eForce);

                pArray[rPos].Name    = rtl::OUString(aName);
                pArray[rPos].Caption = rtl::OUString(aSubStr);
                pArray[rPos].Flags = ( pArray[rPos].Flags |
                                    ( sheet::MemberResultFlags::HASMEMBER | sheet::MemberResultFlags::SUBTOTAL) ) &
                                    ~sheet::MemberResultFlags::CONTINUE;

                if ( nMeasure == SC_DPMEASURE_ALL )
                {
                    //  data layout dimension is (direct/indirect) child of this.
                    //  data layout dimension must have name for all entries.

                    uno::Sequence<sheet::MemberResult>* pLayoutSeq = pSequences;
                    if (!bRoot)
                        ++pLayoutSeq;
                    ScDPResultDimension* pLayoutDim = pChildDimension;
                    while ( pLayoutDim && !pLayoutDim->IsDataLayout() )
                    {
                        pLayoutDim = pLayoutDim->GetFirstChildDimension();
                        ++pLayoutSeq;
                    }
                    if ( pLayoutDim )
                    {
                        sheet::MemberResult* pLayoutArray = pLayoutSeq->getArray();
                        String aDataName = pResultData->GetMeasureDimensionName(nMemberMeasure);
                        pLayoutArray[rPos].Name = rtl::OUString(aDataName);
                    }
                }

                rPos += 1;
            }
        }

        rPos += nExtraSpace;                                    // add again (subtracted above)
    }
}

void ScDPResultMember::FillDataResults( const ScDPResultMember* pRefMember,
                            uno::Sequence< uno::Sequence<sheet::DataResult> >& rSequence,
                            long& rRow, long nMeasure ) const
{
    //  IsVisible() test is in ScDPResultDimension::FillDataResults
    //  (not on data layout dimension)

    long nStartRow = rRow;

    long nExtraSpace = 0;
    if ( pParentLevel && pParentLevel->IsAddEmpty() )
        ++nExtraSpace;

    BOOL bTitleLine = FALSE;
    if ( pParentLevel && pParentLevel->IsOutlineLayout() )
        bTitleLine = TRUE;

    BOOL bSubTotalInTitle = IsSubTotalInTitle( nMeasure );

    BOOL bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
    {
        if ( bTitleLine )           // in tabular layout the title is on a separate row
            ++rRow;                 // -> fill child dimension one row below

        pChildDimension->FillDataResults( pRefMember, rSequence, rRow, nMeasure );  // doesn't modify rRow
        rRow += (USHORT) GetSize( nMeasure );

        if ( bTitleLine )           // title row is included in GetSize, so the following
            --rRow;                 // positions are calculated with the normal values
    }

    long nUserSubStart;
    long nUserSubCount = GetSubTotalCount(&nUserSubStart);
    if ( nUserSubCount || !bHasChild )
    {
        // Calculate at least automatic if no subtotals are selected,
        // show only own values if there's no child dimension (innermost).
        if ( !nUserSubCount || !bHasChild )
        {
            nUserSubCount = 1;
            nUserSubStart = 0;
        }

        long nMemberMeasure = nMeasure;
        long nSubSize = pResultData->GetCountForMeasure(nMeasure);
        if (bHasChild)
        {
            rRow -= nSubSize * ( nUserSubCount - nUserSubStart );   // GetSize includes space for SubTotal
            rRow -= nExtraSpace;                                    // GetSize includes the empty line
        }

        long nMoveSubTotal = 0;
        if ( bSubTotalInTitle )
        {
            nMoveSubTotal = rRow - nStartRow;   // force to first (title) row
            rRow = nStartRow;
        }

        if ( pDataRoot )
        {
            ScDPSubTotalState aSubState;        // initial state

            for (long nUserPos=nUserSubStart; nUserPos<nUserSubCount; nUserPos++)
            {
                if ( bHasChild )
                {
                    aSubState.nRowSubTotalFunc = nUserPos;
                    aSubState.eRowForce = lcl_GetForceFunc( pParentLevel, nUserPos );
                }

                for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
                {
                    if ( nMeasure == SC_DPMEASURE_ALL )
                        nMemberMeasure = nSubCount;
                    else if ( pResultData->GetColStartMeasure() == SC_DPMEASURE_ALL )
                        nMemberMeasure = SC_DPMEASURE_ALL;

                    DBG_ASSERT( rRow < rSequence.getLength(), "bumm" );
                    uno::Sequence<sheet::DataResult>& rSubSeq = rSequence.getArray()[rRow];
                    long nSeqCol = 0;
                    pDataRoot->FillDataRow( pRefMember, rSubSeq, nSeqCol, nMemberMeasure, bHasChild, aSubState );

                    rRow += 1;
                }
            }
        }
        else
            rRow += nSubSize * ( nUserSubCount - nUserSubStart );   // empty rows occur when ShowEmpty is true

        // add extra space again if subtracted from GetSize above,
        // add to own size if no children
        rRow += nExtraSpace;

        rRow += nMoveSubTotal;
    }
}

void ScDPResultMember::UpdateDataResults( const ScDPResultMember* pRefMember, long nMeasure ) const
{
    //  IsVisible() test is in ScDPResultDimension::FillDataResults
    //  (not on data layout dimension)

    BOOL bHasChild = ( pChildDimension != NULL );

    long nUserSubCount = GetSubTotalCount();
    // process subtotals even if not shown
//  if ( nUserSubCount || !bHasChild )
    {
        // Calculate at least automatic if no subtotals are selected,
        // show only own values if there's no child dimension (innermost).
        if ( !nUserSubCount || !bHasChild )
            nUserSubCount = 1;

        long nMemberMeasure = nMeasure;
        long nSubSize = pResultData->GetCountForMeasure(nMeasure);

        if ( pDataRoot )
        {
            ScDPSubTotalState aSubState;        // initial state

            for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)   // including hidden "automatic"
            {
                if ( bHasChild )
                {
                    aSubState.nRowSubTotalFunc = nUserPos;
                    aSubState.eRowForce = lcl_GetForceFunc( pParentLevel, nUserPos );
                }

                for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
                {
                    if ( nMeasure == SC_DPMEASURE_ALL )
                        nMemberMeasure = nSubCount;
                    else if ( pResultData->GetColStartMeasure() == SC_DPMEASURE_ALL )
                        nMemberMeasure = SC_DPMEASURE_ALL;

                    pDataRoot->UpdateDataRow( pRefMember, nMemberMeasure, bHasChild, aSubState );
                }
            }
        }
    }

    if (bHasChild)  // child dimension must be processed last, so the column total is known
    {
        pChildDimension->UpdateDataResults( pRefMember, nMeasure );
    }
}

void ScDPResultMember::SortMembers( ScDPResultMember* pRefMember )
{
    BOOL bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
        pChildDimension->SortMembers( pRefMember );     // sorting is done at the dimension

    BOOL bIsRoot = ( pParentLevel == NULL );
    if ( bIsRoot && pDataRoot )
    {
        // use the row root member to sort columns
        // sub total count is always 1

        pDataRoot->SortMembers( pRefMember );
    }
}

void ScDPResultMember::DoAutoShow( ScDPResultMember* pRefMember )
{
    BOOL bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
        pChildDimension->DoAutoShow( pRefMember );     // sorting is done at the dimension

    BOOL bIsRoot = ( pParentLevel == NULL );
    if ( bIsRoot && pDataRoot )
    {
        // use the row root member to sort columns
        // sub total count is always 1

        pDataRoot->DoAutoShow( pRefMember );
    }
}

void ScDPResultMember::ResetResults( BOOL bRoot )
{
    if (pDataRoot)
        pDataRoot->ResetResults();

    if (pChildDimension)
        pChildDimension->ResetResults();

    if (!bRoot)
        bHasElements = FALSE;
}

void ScDPResultMember::UpdateRunningTotals( const ScDPResultMember* pRefMember, long nMeasure,
                                            ScDPRunningTotalState& rRunning, ScDPRowTotals& rTotals ) const
{
    //  IsVisible() test is in ScDPResultDimension::FillDataResults
    //  (not on data layout dimension)

    BOOL bIsRoot = ( pParentLevel == NULL );
    rTotals.SetInColRoot( bIsRoot );

    BOOL bHasChild = ( pChildDimension != NULL );

    long nUserSubCount = GetSubTotalCount();
    if ( nUserSubCount || !bHasChild )
    {
        // Calculate at least automatic if no subtotals are selected,
        // show only own values if there's no child dimension (innermost).
        if ( !nUserSubCount || !bHasChild )
            nUserSubCount = 1;

        long nMemberMeasure = nMeasure;
        long nSubSize = pResultData->GetCountForMeasure(nMeasure);

        if ( pDataRoot )
        {
            ScDPSubTotalState aSubState;        // initial state

            for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)   // including hidden "automatic"
            {
                if ( bHasChild )
                {
                    aSubState.nRowSubTotalFunc = nUserPos;
                    aSubState.eRowForce = lcl_GetForceFunc( pParentLevel, nUserPos );
                }

                for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
                {
                    if ( nMeasure == SC_DPMEASURE_ALL )
                        nMemberMeasure = nSubCount;
                    else if ( pResultData->GetColStartMeasure() == SC_DPMEASURE_ALL )
                        nMemberMeasure = SC_DPMEASURE_ALL;

                    pDataRoot->UpdateRunningTotals( pRefMember, nMemberMeasure,
                                        bHasChild, aSubState, rRunning, rTotals, *this );
                }
            }
        }
    }

    if (bHasChild)  // child dimension must be processed last, so the column total is known
    {
        pChildDimension->UpdateRunningTotals( pRefMember, nMeasure, rRunning, rTotals );
    }
}

void ScDPResultMember::DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const
{
    lcl_DumpStrings( String::CreateFromAscii("ScDPResultMember"), GetName(), String(), pDoc, rPos );
    SCROW nStartRow = rPos.Row();

    if (pDataRoot)
        pDataRoot->DumpState( pRefMember, pDoc, rPos );

    if (pChildDimension)
        pChildDimension->DumpState( pRefMember, pDoc, rPos );

    lcl_Indent( pDoc, nStartRow, rPos );
}

ScDPAggData* ScDPResultMember::GetColTotal( long nMeasure ) const
{
    return lcl_GetChildTotal( const_cast<ScDPAggData*>(&aColTotal), nMeasure );
}

// -----------------------------------------------------------------------

ScDPDataMember::ScDPDataMember( ScDPResultData* pData, ScDPResultMember* pRes ) :
    pResultData( pData ),
    pResultMember( pRes ),
    pChildDimension( NULL )
{
    // pResultMember is 0 for root members
}

ScDPDataMember::~ScDPDataMember()
{
    delete pChildDimension;
}

String ScDPDataMember::GetName() const
{
    if (pResultMember)
        return pResultMember->GetName();
    else
        return EMPTY_STRING;
}

BOOL ScDPDataMember::IsVisible() const
{
    if (pResultMember)
        return pResultMember->IsVisible();
    else
        return FALSE;
}

BOOL ScDPDataMember::IsNamedItem( const ScDPItemData& r ) const
{
    if (pResultMember)
        return pResultMember->IsNamedItem(r);
    else
        return FALSE;
}

BOOL ScDPDataMember::HasHiddenDetails() const
{
    if (pResultMember)
        return pResultMember->HasHiddenDetails();
    else
        return FALSE;
}

void ScDPDataMember::InitFrom( ScDPResultDimension* pDim )
{
    if ( !pChildDimension )
        pChildDimension = new ScDPDataDimension(pResultData);
    pChildDimension->InitFrom(pDim);
}

long lcl_GetSubTotalPos( const ScDPSubTotalState& rSubState )
{
    long nRet = -1;
    if ( rSubState.nColSubTotalFunc >= 0 ) nRet = rSubState.nColSubTotalFunc;
    if ( rSubState.nRowSubTotalFunc >= 0 ) nRet = rSubState.nRowSubTotalFunc;
    return nRet;
}

void ScDPDataMember::UpdateValues( const ScDPValueData* pValues, const ScDPSubTotalState& rSubState )
{
    //! find out how many and which subtotals are used

    ScDPAggData* pAgg = &aAggregate;

    long nSubPos = lcl_GetSubTotalPos(rSubState);
    if (nSubPos > 0)
    {
        long nSkip = nSubPos * pResultData->GetMeasureCount();
        for (long i=0; i<nSkip; i++)
            pAgg = pAgg->GetChild();        // created if not there
    }

    long nCount = pResultData->GetMeasureCount();
    for (long nPos=0; nPos<nCount; nPos++)
    {
        pAgg->Update( *pValues, pResultData->GetMeasureFunction(nPos), rSubState );
        if ( nPos+1 < nCount )
        {
            pAgg = pAgg->GetChild();        // created if not there
            ++pValues;                      // next value entry
        }
    }
}

void ScDPDataMember::ProcessData( const ScDPItemData* pChildMembers, const ScDPValueData* pValues,
                                    const ScDPSubTotalState& rSubState )
{
    if ( pResultData->IsLateInit() && !pChildDimension && pResultMember && pResultMember->GetChildDimension() )
    {
        //  if this DataMember doesn't have a child dimension because the ResultMember's
        //  child dimension wasn't there yet during this DataMembers's creation,
        //  create the child dimension now
        InitFrom( pResultMember->GetChildDimension() );
    }

    ScDPSubTotalState aLocalSubState(rSubState);        // keep row state, modify column

    long nUserSubCount = pResultMember ? pResultMember->GetSubTotalCount() : 0;

    // Calculate at least automatic if no subtotals are selected,
    // show only own values if there's no child dimension (innermost).
    if ( !nUserSubCount || !pChildDimension )
        nUserSubCount = 1;

    for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)   // including hidden "automatic"
    {
        if ( pChildDimension )
        {
            ScDPLevel* pForceLevel = pResultMember ? pResultMember->GetParentLevel() : NULL;
            aLocalSubState.nColSubTotalFunc = nUserPos;
            aLocalSubState.eColForce = lcl_GetForceFunc( pForceLevel, nUserPos );
        }

        UpdateValues( pValues, aLocalSubState );
    }

    if (pChildDimension)
        pChildDimension->ProcessData( pChildMembers, pValues, rSubState );      // with unmodified subtotal state
}

BOOL ScDPDataMember::HasData( long nMeasure, const ScDPSubTotalState& rSubState ) const
{
    if ( rSubState.eColForce != SUBTOTAL_FUNC_NONE && rSubState.eRowForce != SUBTOTAL_FUNC_NONE &&
                                                        rSubState.eColForce != rSubState.eRowForce )
        return FALSE;

    //  #74542# HasData can be different between measures!

    const ScDPAggData* pAgg = GetConstAggData( nMeasure, rSubState );
    if (!pAgg)
        return FALSE;           //! error?

    return pAgg->HasData();
}

BOOL ScDPDataMember::HasError( long nMeasure, const ScDPSubTotalState& rSubState ) const
{
    const ScDPAggData* pAgg = GetConstAggData( nMeasure, rSubState );
    if (!pAgg)
        return TRUE;

    return pAgg->HasError();
}

double ScDPDataMember::GetAggregate( long nMeasure, const ScDPSubTotalState& rSubState ) const
{
    const ScDPAggData* pAgg = GetConstAggData( nMeasure, rSubState );
    if (!pAgg)
        return DBL_MAX;         //! error?

    return pAgg->GetResult();
}

ScDPAggData* ScDPDataMember::GetAggData( long nMeasure, const ScDPSubTotalState& rSubState )
{
    DBG_ASSERT( nMeasure >= 0, "GetAggData: no measure" );

    ScDPAggData* pAgg = &aAggregate;
    long nSkip = nMeasure;
    long nSubPos = lcl_GetSubTotalPos(rSubState);
    if (nSubPos > 0)
        nSkip += nSubPos * pResultData->GetMeasureCount();

    for ( long nPos=0; nPos<nSkip; nPos++ )
        pAgg = pAgg->GetChild();        //! need to create children here?

    return pAgg;
}

const ScDPAggData* ScDPDataMember::GetConstAggData( long nMeasure, const ScDPSubTotalState& rSubState ) const
{
    DBG_ASSERT( nMeasure >= 0, "GetConstAggData: no measure" );

    const ScDPAggData* pAgg = &aAggregate;
    long nSkip = nMeasure;
    long nSubPos = lcl_GetSubTotalPos(rSubState);
    if (nSubPos > 0)
        nSkip += nSubPos * pResultData->GetMeasureCount();

    for ( long nPos=0; nPos<nSkip; nPos++ )
    {
        pAgg = pAgg->GetExistingChild();
        if (!pAgg)
            return NULL;
    }

    return pAgg;
}

void ScDPDataMember::FillDataRow( const ScDPResultMember* pRefMember,
                                    uno::Sequence<sheet::DataResult>& rSequence,
                                    long& rCol, long nMeasure, BOOL bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState ) const
{
    DBG_ASSERT( pRefMember == pResultMember || !pResultMember, "bla" );

    if ( pRefMember->IsVisible() )  //! here or in ScDPDataDimension::FillDataRow ???
    {
        long nStartCol = rCol;

        const ScDPDataDimension* pDataChild = GetChildDimension();
        const ScDPResultDimension* pRefChild = pRefMember->GetChildDimension();

        const ScDPLevel* pRefParentLevel = const_cast<ScDPResultMember*>(pRefMember)->GetParentLevel();

        long nExtraSpace = 0;
        if ( pRefParentLevel && pRefParentLevel->IsAddEmpty() )
            ++nExtraSpace;

        BOOL bTitleLine = FALSE;
        if ( pRefParentLevel && pRefParentLevel->IsOutlineLayout() )
            bTitleLine = TRUE;

        BOOL bSubTotalInTitle = pRefMember->IsSubTotalInTitle( nMeasure );

        //  leave space for children even if the DataMember hasn't been initialized
        //  (pDataChild is null then, this happens when no values for it are in this row)
        BOOL bHasChild = ( pRefChild != NULL );

        if ( bHasChild )
        {
            if ( bTitleLine )           // in tabular layout the title is on a separate column
                ++rCol;                 // -> fill child dimension one column below

            if ( pDataChild )
                pDataChild->FillDataRow( pRefChild, rSequence, rCol, nMeasure, bIsSubTotalRow, rSubState );
            rCol += (USHORT)pRefMember->GetSize( nMeasure );

            if ( bTitleLine )           // title column is included in GetSize, so the following
                --rCol;                 // positions are calculated with the normal values
        }

        long nUserSubStart;
        long nUserSubCount = pRefMember->GetSubTotalCount(&nUserSubStart);
        if ( nUserSubCount || !bHasChild )
        {
            // Calculate at least automatic if no subtotals are selected,
            // show only own values if there's no child dimension (innermost).
            if ( !nUserSubCount || !bHasChild )
            {
                nUserSubCount = 1;
                nUserSubStart = 0;
            }

            ScDPSubTotalState aLocalSubState(rSubState);        // keep row state, modify column

            long nMemberMeasure = nMeasure;
            long nSubSize = pResultData->GetCountForMeasure(nMeasure);
            if (bHasChild)
            {
                rCol -= nSubSize * ( nUserSubCount - nUserSubStart );   // GetSize includes space for SubTotal
                rCol -= nExtraSpace;                                    // GetSize includes the empty line
            }

            long nMoveSubTotal = 0;
            if ( bSubTotalInTitle )
            {
                nMoveSubTotal = rCol - nStartCol;   // force to first (title) column
                rCol = nStartCol;
            }

            for (long nUserPos=nUserSubStart; nUserPos<nUserSubCount; nUserPos++)
            {
                if ( pChildDimension )
                {
                    ScDPLevel* pForceLevel = pResultMember ? pResultMember->GetParentLevel() : NULL;
                    aLocalSubState.nColSubTotalFunc = nUserPos;
                    aLocalSubState.eColForce = lcl_GetForceFunc( pForceLevel, nUserPos );
                }

                for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
                {
                    if ( nMeasure == SC_DPMEASURE_ALL )
                        nMemberMeasure = nSubCount;

                    DBG_ASSERT( rCol < rSequence.getLength(), "bumm" );
                    sheet::DataResult& rRes = rSequence.getArray()[rCol];

                    if ( HasData( nMemberMeasure, aLocalSubState ) )
                    {
                        if ( HasError( nMemberMeasure, aLocalSubState ) )
                        {
                            rRes.Value = 0;
                            rRes.Flags |= sheet::DataResultFlags::ERROR;
                        }
                        else
                        {
                            rRes.Value = GetAggregate( nMemberMeasure, aLocalSubState );
                            rRes.Flags |= sheet::DataResultFlags::HASDATA;
                        }
                    }

                    if ( bHasChild || bIsSubTotalRow )
                        rRes.Flags |= sheet::DataResultFlags::SUBTOTAL;

                    rCol += 1;
                }
            }

            // add extra space again if subtracted from GetSize above,
            // add to own size if no children
            rCol += nExtraSpace;

            rCol += nMoveSubTotal;
        }
    }
}

void ScDPDataMember::UpdateDataRow( const ScDPResultMember* pRefMember,
                                long nMeasure, BOOL bIsSubTotalRow,
                                const ScDPSubTotalState& rSubState )
{
    DBG_ASSERT( pRefMember == pResultMember || !pResultMember, "bla" );

    // Calculate must be called even if not visible (for use as reference value)
    const ScDPDataDimension* pDataChild = GetChildDimension();
    const ScDPResultDimension* pRefChild = pRefMember->GetChildDimension();

    BOOL bIsRoot = ( pResultMember == NULL || pResultMember->GetParentLevel() == NULL );

    //  leave space for children even if the DataMember hasn't been initialized
    //  (pDataChild is null then, this happens when no values for it are in this row)
    BOOL bHasChild = ( pRefChild != NULL );

    // process subtotals even if not shown
    long nUserSubCount = pRefMember->GetSubTotalCount();

    // Calculate at least automatic if no subtotals are selected,
    // show only own values if there's no child dimension (innermost).
    if ( !nUserSubCount || !bHasChild )
        nUserSubCount = 1;

    ScDPSubTotalState aLocalSubState(rSubState);        // keep row state, modify column

    long nMemberMeasure = nMeasure;
    long nSubSize = pResultData->GetCountForMeasure(nMeasure);

    for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)   // including hidden "automatic"
    {
        if ( pChildDimension )
        {
            ScDPLevel* pForceLevel = pResultMember ? pResultMember->GetParentLevel() : NULL;
            aLocalSubState.nColSubTotalFunc = nUserPos;
            aLocalSubState.eColForce = lcl_GetForceFunc( pForceLevel, nUserPos );
        }

        for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
        {
            if ( nMeasure == SC_DPMEASURE_ALL )
                nMemberMeasure = nSubCount;

            // update data...
            ScDPAggData* pAggData = GetAggData( nMemberMeasure, aLocalSubState );
            if (pAggData)
            {
                //! aLocalSubState?
                ScSubTotalFunc eFunc = pResultData->GetMeasureFunction( nMemberMeasure );
                sheet::DataPilotFieldReference aReferenceValue = pResultData->GetMeasureRefVal( nMemberMeasure );
                sal_Int32 eRefType = aReferenceValue.ReferenceType;

                // calculate the result first - for all members, regardless of reference value
                pAggData->Calculate( eFunc, aLocalSubState );

                if ( eRefType == sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE ||
                     eRefType == sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE ||
                     eRefType == sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE )
                {
                    // copy the result into auxiliary value, so differences can be
                    // calculated in any order
                    pAggData->SetAuxiliary( pAggData->GetResult() );
                }
                // column/row percentage/index is now in UpdateRunningTotals, so it doesn't disturb sorting
            }
        }
    }

    if ( bHasChild )    // child dimension must be processed last, so the row total is known
    {
        if ( pDataChild )
            pDataChild->UpdateDataRow( pRefChild, nMeasure, bIsSubTotalRow, rSubState );
    }
}

void ScDPDataMember::SortMembers( ScDPResultMember* pRefMember )
{
    DBG_ASSERT( pRefMember == pResultMember || !pResultMember, "bla" );

    if ( pRefMember->IsVisible() )  //! here or in ScDPDataDimension ???
    {
        ScDPDataDimension* pDataChild = GetChildDimension();
        ScDPResultDimension* pRefChild = pRefMember->GetChildDimension();
        if ( pRefChild && pDataChild )
            pDataChild->SortMembers( pRefChild );       // sorting is done at the dimension
    }
}

void ScDPDataMember::DoAutoShow( ScDPResultMember* pRefMember )
{
    DBG_ASSERT( pRefMember == pResultMember || !pResultMember, "bla" );

    if ( pRefMember->IsVisible() )  //! here or in ScDPDataDimension ???
    {
        ScDPDataDimension* pDataChild = GetChildDimension();
        ScDPResultDimension* pRefChild = pRefMember->GetChildDimension();
        if ( pRefChild && pDataChild )
            pDataChild->DoAutoShow( pRefChild );       // sorting is done at the dimension
    }
}

void ScDPDataMember::ResetResults()
{
    aAggregate.Reset();

    ScDPDataDimension* pDataChild = GetChildDimension();
    if ( pDataChild )
        pDataChild->ResetResults();
}

void ScDPDataMember::UpdateRunningTotals( const ScDPResultMember* pRefMember,
                                long nMeasure, BOOL bIsSubTotalRow,
                                const ScDPSubTotalState& rSubState, ScDPRunningTotalState& rRunning,
                                ScDPRowTotals& rTotals, const ScDPResultMember& rRowParent )
{
    DBG_ASSERT( pRefMember == pResultMember || !pResultMember, "bla" );

    if ( pRefMember->IsVisible() )  //! here or in ScDPDataDimension::UpdateRunningTotals ???
    {
        const ScDPDataDimension* pDataChild = GetChildDimension();
        const ScDPResultDimension* pRefChild = pRefMember->GetChildDimension();

        BOOL bIsRoot = ( pResultMember == NULL || pResultMember->GetParentLevel() == NULL );

        //  leave space for children even if the DataMember hasn't been initialized
        //  (pDataChild is null then, this happens when no values for it are in this row)
        BOOL bHasChild = ( pRefChild != NULL );

        long nUserSubCount = pRefMember->GetSubTotalCount();
        if ( nUserSubCount || !bHasChild )
        {
            // Calculate at least automatic if no subtotals are selected,
            // show only own values if there's no child dimension (innermost).
            if ( !nUserSubCount || !bHasChild )
                nUserSubCount = 1;

            ScDPSubTotalState aLocalSubState(rSubState);        // keep row state, modify column

            long nMemberMeasure = nMeasure;
            long nSubSize = pResultData->GetCountForMeasure(nMeasure);

            for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)   // including hidden "automatic"
            {
                if ( pChildDimension )
                {
                    ScDPLevel* pForceLevel = pResultMember ? pResultMember->GetParentLevel() : NULL;
                    aLocalSubState.nColSubTotalFunc = nUserPos;
                    aLocalSubState.eColForce = lcl_GetForceFunc( pForceLevel, nUserPos );
                }

                for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
                {
                    if ( nMeasure == SC_DPMEASURE_ALL )
                        nMemberMeasure = nSubCount;

                    // update data...
                    ScDPAggData* pAggData = GetAggData( nMemberMeasure, aLocalSubState );
                    if (pAggData)
                    {
                        //! aLocalSubState?
                        sheet::DataPilotFieldReference aReferenceValue = pResultData->GetMeasureRefVal( nMemberMeasure );
                        sal_Int32 eRefType = aReferenceValue.ReferenceType;

                        if ( eRefType == sheet::DataPilotFieldReferenceType::RUNNING_TOTAL ||
                             eRefType == sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE ||
                             eRefType == sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE ||
                             eRefType == sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE )
                        {
                            BOOL bRunningTotal = ( eRefType == sheet::DataPilotFieldReferenceType::RUNNING_TOTAL );
                            BOOL bRelative =
                                ( aReferenceValue.ReferenceItemType != sheet::DataPilotFieldReferenceItemType::NAMED && !bRunningTotal );
                            long nRelativeDir = bRelative ?
                                ( ( aReferenceValue.ReferenceItemType == sheet::DataPilotFieldReferenceItemType::PREVIOUS ) ? -1 : 1 ) : 0;

                            const long* pColVisible = rRunning.GetColVisible();
                            const long* pColIndexes = rRunning.GetColIndexes();
                            const long* pRowVisible = rRunning.GetRowVisible();
                            const long* pRowIndexes = rRunning.GetRowIndexes();

                            String aRefFieldName = aReferenceValue.ReferenceField;

                            //! aLocalSubState?
                            USHORT nRefOrient = pResultData->GetMeasureRefOrient( nMemberMeasure );
                            BOOL bRefDimInCol = ( nRefOrient == sheet::DataPilotFieldOrientation_COLUMN );
                            BOOL bRefDimInRow = ( nRefOrient == sheet::DataPilotFieldOrientation_ROW );

                            ScDPResultDimension* pSelectDim = NULL;
                            long nRowPos = 0;
                            long nColPos = 0;

                            //
                            //  find the reference field in column or row dimensions
                            //

                            if ( bRefDimInRow )     //  look in row dimensions
                            {
                                pSelectDim = rRunning.GetRowResRoot()->GetChildDimension();
                                while ( pSelectDim && pSelectDim->GetName() != aRefFieldName )
                                {
                                    long nIndex = pRowIndexes[nRowPos];
                                    if ( nIndex >= 0 && nIndex < pSelectDim->GetMemberCount() )
                                        pSelectDim = pSelectDim->GetMember(nIndex)->GetChildDimension();
                                    else
                                        pSelectDim = NULL;
                                    ++nRowPos;
                                }
                                // child dimension of innermost member?
                                if ( pSelectDim && pRowIndexes[nRowPos] < 0 )
                                    pSelectDim = NULL;
                            }

                            if ( bRefDimInCol )     //  look in column dimensions
                            {
                                pSelectDim = rRunning.GetColResRoot()->GetChildDimension();
                                while ( pSelectDim && pSelectDim->GetName() != aRefFieldName )
                                {
                                    long nIndex = pColIndexes[nColPos];
                                    if ( nIndex >= 0 && nIndex < pSelectDim->GetMemberCount() )
                                        pSelectDim = pSelectDim->GetMember(nIndex)->GetChildDimension();
                                    else
                                        pSelectDim = NULL;
                                    ++nColPos;
                                }
                                // child dimension of innermost member?
                                if ( pSelectDim && pColIndexes[nColPos] < 0 )
                                    pSelectDim = NULL;
                            }

                            BOOL bNoDetailsInRef = FALSE;
                            if ( pSelectDim && bRunningTotal )
                            {
                                //  Running totals:
                                //  If details are hidden for this member in the reference dimension,
                                //  don't show or sum up the value. Otherwise, for following members,
                                //  the running totals of details and subtotals wouldn't match.

                                long nMyIndex = bRefDimInCol ? pColIndexes[nColPos] : pRowIndexes[nRowPos];
                                if ( nMyIndex >= 0 && nMyIndex < pSelectDim->GetMemberCount() )
                                {
                                    ScDPResultMember* pMyRefMember = pSelectDim->GetMember(nMyIndex);
                                    if ( pMyRefMember && pMyRefMember->HasHiddenDetails() )
                                    {
                                        pSelectDim = NULL;          // don't calculate
                                        bNoDetailsInRef = TRUE;     // show error, not empty
                                    }
                                }
                            }

                            if ( bRelative )
                            {
                                //  Difference/Percentage from previous/next:
                                //  If details are hidden for this member in the innermost column/row
                                //  dimension (the orientation of the reference dimension), show an
                                //  error value.
                                //  - If the no-details dimension is the reference dimension, its
                                //    members will be skipped when finding the previous/next member,
                                //    so there must be no results for its members.
                                //  - If the no-details dimension is outside of the reference dimension,
                                //    no calculation in the reference dimension is possible.
                                //  - Otherwise, the error isn't strictly necessary, but shown for
                                //    consistency.

                                BOOL bInnerNoDetails = bRefDimInCol ? HasHiddenDetails() :
                                                     ( bRefDimInRow ? rRowParent.HasHiddenDetails() : TRUE );
                                if ( bInnerNoDetails )
                                {
                                    pSelectDim = NULL;
                                    bNoDetailsInRef = TRUE;         // show error, not empty
                                }
                            }

                            if ( !bRefDimInCol && !bRefDimInRow )   // invalid dimension specified
                                bNoDetailsInRef = TRUE;             // pSelectDim is then already NULL

                            //
                            //  get the member for the reference item and do the calculation
                            //

                            if ( bRunningTotal )
                            {
                                // running total in (dimension) -> find first existing member

                                if ( pSelectDim )
                                {
                                    ScDPDataMember* pSelectMember;
                                    if ( bRefDimInCol )
                                        pSelectMember = ScDPResultDimension::GetColReferenceMember( NULL, NULL,
                                                                        nColPos, rRunning );
                                    else
                                    {
                                        long nSkip = nRowPos + 1;   // including the reference dimension
                                        pSelectMember = pSelectDim->GetRowReferenceMember( NULL, NULL,
                                                                        pRowIndexes+nSkip, pColIndexes );
                                    }

                                    if ( pSelectMember )
                                    {
                                        // The running total is kept as the auxiliary value in
                                        // the first available member for the reference dimension.
                                        // Members are visited in final order, so each one's result
                                        // can be used and then modified.

                                        ScDPAggData* pSelectData = pSelectMember->
                                                        GetAggData( nMemberMeasure, aLocalSubState );
                                        if ( pSelectData )
                                        {
                                            double fTotal = pSelectData->GetAuxiliary();
                                            fTotal += pAggData->GetResult();
                                            pSelectData->SetAuxiliary( fTotal );
                                            pAggData->SetResult( fTotal );
                                            pAggData->SetEmpty(FALSE);              // always display
                                        }
                                    }
                                    else
                                        pAggData->SetError();
                                }
                                else if (bNoDetailsInRef)
                                    pAggData->SetError();
                                else
                                    pAggData->SetEmpty(TRUE);                       // empty (dim set to 0 above)
                            }
                            else
                            {
                                // difference/percentage -> find specified member

                                if ( pSelectDim )
                                {
                                    String aRefItemName = aReferenceValue.ReferenceItemName;
                                    ScDPRelativePos aRefItemPos( 0, nRelativeDir );     // nBasePos is modified later

                                    const String* pRefName = NULL;
                                    const ScDPRelativePos* pRefPos = NULL;
                                    if ( bRelative )
                                        pRefPos = &aRefItemPos;
                                    else
                                        pRefName = &aRefItemName;

                                    ScDPDataMember* pSelectMember;
                                    if ( bRefDimInCol )
                                    {
                                        aRefItemPos.nBasePos = pColVisible[nColPos];    // without sort order applied
                                        pSelectMember = ScDPResultDimension::GetColReferenceMember( pRefPos, pRefName,
                                                                        nColPos, rRunning );
                                    }
                                    else
                                    {
                                        aRefItemPos.nBasePos = pRowVisible[nRowPos];    // without sort order applied
                                        long nSkip = nRowPos + 1;   // including the reference dimension
                                        pSelectMember = pSelectDim->GetRowReferenceMember( pRefPos, pRefName,
                                                                        pRowIndexes+nSkip, pColIndexes );
                                    }

                                    // difference or perc.difference is empty for the reference item itself
                                    if ( pSelectMember == this &&
                                         eRefType != sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE )
                                    {
                                        pAggData->SetEmpty(TRUE);
                                    }
                                    else if ( pSelectMember )
                                    {
                                        const ScDPAggData* pOtherAggData = pSelectMember->
                                                            GetConstAggData( nMemberMeasure, aLocalSubState );
                                        DBG_ASSERT( pOtherAggData, "no agg data" );
                                        if ( pOtherAggData )
                                        {
                                            // Reference member may be visited before or after this one,
                                            // so the auxiliary value is used for the original result.

                                            double fOtherResult = pOtherAggData->GetAuxiliary();
                                            double fThisResult = pAggData->GetResult();
                                            BOOL bError = FALSE;
                                            switch ( eRefType )
                                            {
                                                case sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE:
                                                    fThisResult = fThisResult - fOtherResult;
                                                    break;
                                                case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE:
                                                    if ( fOtherResult == 0.0 )
                                                        bError = TRUE;
                                                    else
                                                        fThisResult = fThisResult / fOtherResult;
                                                    break;
                                                case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE:
                                                    if ( fOtherResult == 0.0 )
                                                        bError = TRUE;
                                                    else
                                                        fThisResult = ( fThisResult - fOtherResult ) / fOtherResult;
                                                    break;
                                                default:
                                                    DBG_ERROR("invalid calculation type");
                                            }
                                            if ( bError )
                                            {
                                                pAggData->SetError();
                                            }
                                            else
                                            {
                                                pAggData->SetResult(fThisResult);
                                                pAggData->SetEmpty(FALSE);              // always display
                                            }
                                            //! errors in data?
                                        }
                                    }
                                    else if (bRelative && !bNoDetailsInRef)
                                        pAggData->SetEmpty(TRUE);                   // empty
                                    else
                                        pAggData->SetError();                       // error
                                }
                                else if (bNoDetailsInRef)
                                    pAggData->SetError();                           // error
                                else
                                    pAggData->SetEmpty(TRUE);                       // empty
                            }
                        }
                        else if ( eRefType == sheet::DataPilotFieldReferenceType::ROW_PERCENTAGE ||
                                  eRefType == sheet::DataPilotFieldReferenceType::COLUMN_PERCENTAGE ||
                                  eRefType == sheet::DataPilotFieldReferenceType::TOTAL_PERCENTAGE ||
                                  eRefType == sheet::DataPilotFieldReferenceType::INDEX )
                        {
                            //
                            //  set total values when they are encountered (always before their use)
                            //

                            ScDPAggData* pColTotalData = pRefMember->GetColTotal( nMemberMeasure );
                            ScDPAggData* pRowTotalData = rTotals.GetRowTotal( nMemberMeasure );
                            ScDPAggData* pGrandTotalData = rTotals.GetGrandTotal( nMemberMeasure );

                            double fTotalValue = pAggData->HasError() ? 0 : pAggData->GetResult();

                            if ( bIsRoot && rTotals.IsInColRoot() && pGrandTotalData )
                                pGrandTotalData->SetAuxiliary( fTotalValue );

                            if ( bIsRoot && pRowTotalData )
                                pRowTotalData->SetAuxiliary( fTotalValue );

                            if ( rTotals.IsInColRoot() && pColTotalData )
                                pColTotalData->SetAuxiliary( fTotalValue );

                            //
                            //  find relation to total values
                            //

                            switch ( eRefType )
                            {
                                case sheet::DataPilotFieldReferenceType::ROW_PERCENTAGE:
                                case sheet::DataPilotFieldReferenceType::COLUMN_PERCENTAGE:
                                case sheet::DataPilotFieldReferenceType::TOTAL_PERCENTAGE:
                                    {
                                        double nTotal;
                                        if ( eRefType == sheet::DataPilotFieldReferenceType::ROW_PERCENTAGE )
                                            nTotal = pRowTotalData->GetAuxiliary();
                                        else if ( eRefType == sheet::DataPilotFieldReferenceType::COLUMN_PERCENTAGE )
                                            nTotal = pColTotalData->GetAuxiliary();
                                        else
                                            nTotal = pGrandTotalData->GetAuxiliary();

                                        if ( nTotal == 0.0 )
                                            pAggData->SetError();
                                        else
                                            pAggData->SetResult( pAggData->GetResult() / nTotal );
                                    }
                                    break;
                                case sheet::DataPilotFieldReferenceType::INDEX:
                                    {
                                        double nColTotal = pColTotalData->GetAuxiliary();
                                        double nRowTotal = pRowTotalData->GetAuxiliary();
                                        double nGrandTotal = pGrandTotalData->GetAuxiliary();
                                        if ( nRowTotal == 0.0 || nColTotal == 0.0 )
                                            pAggData->SetError();
                                        else
                                            pAggData->SetResult(
                                                ( pAggData->GetResult() * nGrandTotal ) /
                                                ( nRowTotal * nColTotal ) );
                                    }
                                    break;
                            }
                        }
                    }
                }
            }
        }

        if ( bHasChild )    // child dimension must be processed last, so the row total is known
        {
            if ( pDataChild )
                pDataChild->UpdateRunningTotals( pRefChild, nMeasure,
                                                bIsSubTotalRow, rSubState, rRunning, rTotals, rRowParent );
        }
    }
}

void ScDPDataMember::DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const
{
    String aValue;
    const ScDPAggData* pAggData = GetConstAggData( 0, ScDPSubTotalState() );
    if ( pAggData )
        aValue = String::CreateFromDouble( pAggData->GetResult() );

    lcl_DumpStrings( String::CreateFromAscii("ScDPDataMember"), GetName(), aValue, pDoc, rPos );
    SCROW nStartRow = rPos.Row();

    const ScDPDataDimension* pDataChild = GetChildDimension();
    const ScDPResultDimension* pRefChild = pRefMember->GetChildDimension();
    if ( pDataChild && pRefChild )
        pDataChild->DumpState( pRefChild, pDoc, rPos );

    lcl_Indent( pDoc, nStartRow, rPos );
}

// -----------------------------------------------------------------------

//  Helper class to select the members to include in
//  ScDPResultDimension::InitFrom or LateInitFrom if groups are used

class ScDPGroupCompare
{
private:
    ScDPResultData*      pResultData;
    const ScDPInitState& rInitState;
    long                 nDimSource;
    BOOL                 bIncludeAll;
    BOOL                 bIsBase;
    long                 nGroupBase;
    const ScDPItemData*  pBaseData;

public:
            ScDPGroupCompare( ScDPResultData* pData, const ScDPInitState& rState, long nDimension );
            ~ScDPGroupCompare() {}

    BOOL    IsIncluded( const ScDPMember& rMember )     { return bIncludeAll || TestIncluded( rMember ); }
    BOOL    TestIncluded( const ScDPMember& rMember );
};

ScDPGroupCompare::ScDPGroupCompare( ScDPResultData* pData, const ScDPInitState& rState, long nDimension ) :
    pResultData( pData ),
    rInitState( rState ),
    nDimSource( nDimension ),
    pBaseData( NULL )
{
    bIsBase = pResultData->IsBaseForGroup( nDimSource );
    nGroupBase = pResultData->GetGroupBase( nDimSource );      //! get together in one call?
    if ( nGroupBase >= 0 )
        pBaseData = rInitState.GetNameForIndex( nGroupBase );

    // if bIncludeAll is set, TestIncluded doesn't need to be called
    bIncludeAll = !( bIsBase || nGroupBase >= 0 );
}

BOOL ScDPGroupCompare::TestIncluded( const ScDPMember& rMember )
{
    BOOL bInclude = TRUE;
    if ( pBaseData )
    {
        ScDPItemData aMemberData;
        rMember.FillItemData( aMemberData );
        bInclude = pResultData->IsInGroup( aMemberData, nDimSource, *pBaseData, nGroupBase );
    }
    else if ( bIsBase )
    {
        // need to check all previous groups
        //! get array of groups (or indexes) before loop?
        ScDPItemData aMemberData;
        rMember.FillItemData( aMemberData );
        long nInitCount = rInitState.GetCount();
        const long* pInitSource = rInitState.GetSource();
        const ScDPItemData* pInitNames = rInitState.GetNames();
        for (long nInitPos=0; nInitPos<nInitCount && bInclude; nInitPos++)
            if ( pResultData->GetGroupBase( pInitSource[nInitPos] ) == nDimSource )
            {
                bInclude = pResultData->IsInGroup( pInitNames[nInitPos], pInitSource[nInitPos],
                                                    aMemberData, nDimSource );
            }
    }
    else if ( nGroupBase >= 0 )
    {
        // base isn't used in preceding fields
        // -> look for other groups using the same base

        //! get array of groups (or indexes) before loop?
        ScDPItemData aMemberData;
        rMember.FillItemData( aMemberData );
        long nInitCount = rInitState.GetCount();
        const long* pInitSource = rInitState.GetSource();
        const ScDPItemData* pInitNames = rInitState.GetNames();
        for (long nInitPos=0; nInitPos<nInitCount && bInclude; nInitPos++)
            if ( pResultData->GetGroupBase( pInitSource[nInitPos] ) == nGroupBase )
            {
                // same base (hierarchy between the two groups is irrelevant)
                bInclude = pResultData->HasCommonElement( pInitNames[nInitPos], pInitSource[nInitPos],
                                                        aMemberData, nDimSource );
            }
    }

    return bInclude;
}

// -----------------------------------------------------------------------

ScDPResultDimension::ScDPResultDimension( ScDPResultData* pData ) :
    pResultData( pData ),
    bInitialized( FALSE ),
    bIsDataLayout( FALSE ),
    bSortByData( FALSE ),
    bSortAscending( FALSE ),
    nSortMeasure( 0 ),
    bAutoShow( FALSE ),
    bAutoTopItems( FALSE ),
    nAutoMeasure( 0 ),
    nAutoCount( 0 )
{
}

ScDPResultDimension::~ScDPResultDimension()
{
}

void ScDPResultDimension::InitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev, ScDPInitState& rInitState )
{
    ScDPDimension* pThisDim = *ppDim;
    ScDPLevel* pThisLevel = *ppLev;
    if (pThisDim && pThisLevel)
    {
        ScDPDimension** ppChildDim = ppDim + 1;
        ScDPLevel** ppChildLev = ppLev + 1;

        bIsDataLayout = pThisDim->getIsDataLayoutDimension();
        aDimensionName = pThisDim->getName();

        const sheet::DataPilotFieldAutoShowInfo& rAutoInfo = pThisLevel->GetAutoShow();
        if ( rAutoInfo.IsEnabled )
        {
            bAutoShow     = TRUE;
            bAutoTopItems = ( rAutoInfo.ShowItemsMode == sheet::DataPilotFieldShowItemsMode::FROM_TOP );
            nAutoMeasure  = pThisLevel->GetAutoMeasure();
            nAutoCount    = rAutoInfo.ItemCount;
        }

        const sheet::DataPilotFieldSortInfo& rSortInfo = pThisLevel->GetSortInfo();
        if ( rSortInfo.Mode == sheet::DataPilotFieldSortMode::DATA )
        {
            bSortByData = TRUE;
            bSortAscending = rSortInfo.IsAscending;
            nSortMeasure = pThisLevel->GetSortMeasure();
        }

        // global order is used to initialize aMembers, so it doesn't have to be looked at later
        const ScMemberSortOrder& rGlobalOrder = pThisLevel->GetGlobalOrder();

        long nDimSource = pThisDim->GetDimension();     //! check GetSourceDim?
        ScDPGroupCompare aCompare( pResultData, rInitState, nDimSource );

        ScDPMembers* pMembers = pThisLevel->GetMembersObject();
        long nMembCount = pMembers->getCount();
        for ( long i=0; i<nMembCount; i++ )
        {
            long nSorted = rGlobalOrder.empty() ? i : rGlobalOrder[i];

            ScDPMember* pMember = pMembers->getByIndex(nSorted);
            if ( aCompare.IsIncluded( *pMember ) )
            {
                ScDPResultMember* pNew = new ScDPResultMember( pResultData, pThisDim,
                                                pThisLevel, pMember, FALSE );
                aMembers.Insert( pNew, aMembers.Count() );

                ScDPItemData aMemberData;
                pMember->FillItemData( aMemberData );
                rInitState.AddMember( nDimSource, ScDPItemData( aMemberData ) );
                pNew->InitFrom( ppChildDim, ppChildLev, rInitState );
                rInitState.RemoveMember();
            }
        }
    }
    bInitialized = TRUE;
}

void ScDPResultDimension::LateInitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev, ScDPItemData* pItemData,
                                        ScDPInitState& rInitState )
{
    ScDPDimension* pThisDim = *ppDim;
    ScDPLevel* pThisLevel = *ppLev;
    ScDPItemData& rThisData = *pItemData;
    if (pThisDim && pThisLevel)
    {
        ScDPDimension** ppChildDim = ppDim + 1;
        ScDPLevel** ppChildLev = ppLev + 1;
        ScDPItemData* pChildData = pItemData + 1;

        long nDimSource = pThisDim->GetDimension();     //! check GetSourceDim?

        if ( !bInitialized )
        {
            //  create all members at the first call (preserve order)

            bIsDataLayout = pThisDim->getIsDataLayoutDimension();
            aDimensionName = pThisDim->getName();

            const sheet::DataPilotFieldAutoShowInfo& rAutoInfo = pThisLevel->GetAutoShow();
            if ( rAutoInfo.IsEnabled )
            {
                bAutoShow     = TRUE;
                bAutoTopItems = ( rAutoInfo.ShowItemsMode == sheet::DataPilotFieldShowItemsMode::FROM_TOP );
                nAutoMeasure  = pThisLevel->GetAutoMeasure();
                nAutoCount    = rAutoInfo.ItemCount;
            }

            const sheet::DataPilotFieldSortInfo& rSortInfo = pThisLevel->GetSortInfo();
            if ( rSortInfo.Mode == sheet::DataPilotFieldSortMode::DATA )
            {
                bSortByData = TRUE;
                bSortAscending = rSortInfo.IsAscending;
                nSortMeasure = pThisLevel->GetSortMeasure();
            }

            // global order is used to initialize aMembers, so it doesn't have to be looked at later
            const ScMemberSortOrder& rGlobalOrder = pThisLevel->GetGlobalOrder();

            ScDPGroupCompare aCompare( pResultData, rInitState, nDimSource );

            ScDPMembers* pMembers = pThisLevel->GetMembersObject();
            long nMembCount = pMembers->getCount();
            for ( long i=0; i<nMembCount; i++ )
            {
                long nSorted = rGlobalOrder.empty() ? i : rGlobalOrder[i];

                ScDPMember* pMember = pMembers->getByIndex(nSorted);
                if ( aCompare.IsIncluded( *pMember ) )
                {
                    ScDPResultMember* pNew = new ScDPResultMember( pResultData, pThisDim,
                                                    pThisLevel, pMember, FALSE );
                    aMembers.Insert( pNew, aMembers.Count() );
                }
            }
            bInitialized = TRUE;    // don't call again, even if no members were included
        }

        //  initialize only specific member (or all if "show empty" flag is set)

        BOOL bShowEmpty = pThisLevel->getShowEmpty();
        long nCount = aMembers.Count();
        for (long i=0; i<nCount; i++)
        {
            ScDPResultMember* pResultMember = aMembers[(USHORT)i];
            if ( bIsDataLayout || bShowEmpty || pResultMember->IsNamedItem( rThisData ) )
            {
                ScDPItemData aMemberData;
                pResultMember->FillItemData( aMemberData );
                rInitState.AddMember( nDimSource, aMemberData );
                pResultMember->LateInitFrom( ppChildDim, ppChildLev, pChildData, rInitState );
                rInitState.RemoveMember();

                if ( !bIsDataLayout && !bShowEmpty )
                    break;
            }
        }
    }
}

long ScDPResultDimension::GetSize(long nMeasure) const
{
    long nTotal = 0;
    long nMemberCount = aMembers.Count();
    if (bIsDataLayout)
    {
        DBG_ASSERT(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                    "DataLayout dimension twice?");
        //  repeat first member...
        nTotal = nMemberCount * aMembers[0]->GetSize(0);    // all measures have equal size
    }
    else
    {
        //  add all members
        for (long nMem=0; nMem<nMemberCount; nMem++)
            nTotal += aMembers[(USHORT)nMem]->GetSize(nMeasure);
    }
    return nTotal;
}

BOOL ScDPResultDimension::IsValidEntry( const ScDPItemData* pMembers ) const
{
    // the ScDPItemData array must contain enough entries for all dimensions - this isn't checked

    long nCount = aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPResultMember* pMember = aMembers[(USHORT)i];
        if ( bIsDataLayout || pMember->IsNamedItem( *pMembers ) )
        {
            if ( !pMember->IsValid() )
                return FALSE;

            ScDPResultDimension* pChildDim = pMember->GetChildDimension();
            if (pChildDim)
                return pChildDim->IsValidEntry( pMembers + 1 );
            else
                return TRUE;
        }
    }

    DBG_ERROR("IsValidEntry: Member not found");
    return FALSE;
}

void ScDPResultDimension::ProcessData( const ScDPItemData* pMembers,
                            ScDPResultDimension* pDataDim, const ScDPItemData* pDataMembers,
                            const ScDPValueData* pValues )
{
    // the ScDPItemData array must contain enough entries for all dimensions - this isn't checked

    long nCount = aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPResultMember* pMember = aMembers[(USHORT)i];

        // always first member for data layout dim
        if ( bIsDataLayout || pMember->IsNamedItem( *pMembers ) )
        {
            pMember->ProcessData( pMembers + 1, pDataDim, pDataMembers, pValues );
            return;
        }
    }

    DBG_ERROR("ProcessData: Member not found");
}

void ScDPResultDimension::FillMemberResults( uno::Sequence<sheet::MemberResult>* pSequences,
                                                long nStart, long nMeasure )
{
    long nPos = nStart;
    long nCount = aMembers.Count();

    for (long i=0; i<nCount; i++)
    {
        long nSorted = aMemberOrder.empty() ? i : aMemberOrder[i];

        ScDPResultMember* pMember = aMembers[(USHORT)nSorted];
        //  in data layout dimension, use first member with different measures/names
        if ( bIsDataLayout )
        {
            String aMbrName = pResultData->GetMeasureDimensionName( nSorted );
            String aMbrCapt = pResultData->GetMeasureString( nSorted, FALSE, SUBTOTAL_FUNC_NONE );
            aMembers[0]->FillMemberResults( pSequences, nPos, nSorted, FALSE, &aMbrName, &aMbrCapt );
        }
        else if ( pMember->IsVisible() )
            pMember->FillMemberResults( pSequences, nPos, nMeasure, FALSE, NULL, NULL );
        // nPos is modified
    }
}

void ScDPResultDimension::FillDataResults( const ScDPResultMember* pRefMember,
                            uno::Sequence< uno::Sequence<sheet::DataResult> >& rSequence,
                            long nRow, long nMeasure ) const
{
    long nMemberRow = nRow;
    long nMemberMeasure = nMeasure;
    long nCount = aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        long nSorted = aMemberOrder.empty() ? i : aMemberOrder[i];

        const ScDPResultMember* pMember;
        if (bIsDataLayout)
        {
            DBG_ASSERT(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            pMember = aMembers[0];
            nMemberMeasure = nSorted;
        }
        else
            pMember = aMembers[(USHORT)nSorted];

        if ( pMember->IsVisible() )
            pMember->FillDataResults( pRefMember, rSequence, nMemberRow, nMemberMeasure );
            // nMemberRow is modified
    }
}

void ScDPResultDimension::UpdateDataResults( const ScDPResultMember* pRefMember, long nMeasure ) const
{
    long nMemberMeasure = nMeasure;
    long nCount = aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        const ScDPResultMember* pMember;
        if (bIsDataLayout)
        {
            DBG_ASSERT(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            pMember = aMembers[0];
            nMemberMeasure = i;
        }
        else
            pMember = aMembers[(USHORT)i];

        if ( pMember->IsVisible() )
            pMember->UpdateDataResults( pRefMember, nMemberMeasure );
    }
}

void ScDPResultDimension::SortMembers( ScDPResultMember* pRefMember )
{
    long nCount = aMembers.Count();

    if ( bSortByData )
    {
        // sort members

        DBG_ASSERT( aMemberOrder.empty(), "sort twice?" );
        aMemberOrder.resize( nCount );
        for (long nPos=0; nPos<nCount; nPos++)
            aMemberOrder[nPos] = nPos;

        ScDPRowMembersOrder aComp( *this, nSortMeasure, bSortAscending );
        ::std::sort( aMemberOrder.begin(), aMemberOrder.end(), aComp );
    }

    // handle children

    // for data layout, call only once - sorting measure is always taken from settings
    long nLoopCount = bIsDataLayout ? 1 : nCount;
    for (long i=0; i<nLoopCount; i++)
    {
        ScDPResultMember* pMember = aMembers[(USHORT)i];
        if ( pMember->IsVisible() )
            pMember->SortMembers( pRefMember );
    }
}

void ScDPResultDimension::DoAutoShow( ScDPResultMember* pRefMember )
{
    long nCount = aMembers.Count();

    // handle children first, before changing the visible state

    // for data layout, call only once - sorting measure is always taken from settings
    long nLoopCount = bIsDataLayout ? 1 : nCount;
    for (long i=0; i<nLoopCount; i++)
    {
        ScDPResultMember* pMember = aMembers[(USHORT)i];
        if ( pMember->IsVisible() )
            pMember->DoAutoShow( pRefMember );
    }

    if ( bAutoShow && nAutoCount > 0 && nAutoCount < nCount )
    {
        // establish temporary order, hide remaining members

        ScMemberSortOrder aAutoOrder;
        aAutoOrder.resize( nCount );
        long nPos;
        for (nPos=0; nPos<nCount; nPos++)
            aAutoOrder[nPos] = nPos;

        ScDPRowMembersOrder aComp( *this, nAutoMeasure, !bAutoTopItems );
        ::std::sort( aAutoOrder.begin(), aAutoOrder.end(), aComp );

        // look for equal values to the last included one

        long nIncluded = nAutoCount;
        ScDPResultMember* pMember1 = aMembers[(USHORT)aAutoOrder[nIncluded - 1]];
        ScDPDataMember* pDataMember1 = pMember1->IsVisible() ? pMember1->GetDataRoot() : NULL;
        BOOL bContinue = TRUE;
        while ( bContinue )
        {
            bContinue = FALSE;
            if ( nIncluded < nCount )
            {
                ScDPResultMember* pMember2 = aMembers[(USHORT)aAutoOrder[nIncluded]];
                ScDPDataMember* pDataMember2 = pMember2->IsVisible() ? pMember2->GetDataRoot() : NULL;

                if ( lcl_IsEqual( pDataMember1, pDataMember2, nAutoMeasure ) )
                {
                    ++nIncluded;                // include more members if values are equal
                    bContinue = TRUE;
                }
            }
        }

        // hide the remaining members

        for (nPos = nIncluded; nPos < nCount; nPos++)
        {
            ScDPResultMember* pMember = aMembers[(USHORT)aAutoOrder[nPos]];
            pMember->SetAutoHidden();
        }
    }
}

void ScDPResultDimension::ResetResults()
{
    long nCount = aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        // sort order doesn't matter
        ScDPResultMember* pMember = aMembers[(USHORT)( bIsDataLayout ? 0 : i )];
        pMember->ResetResults( FALSE );
    }
}

long ScDPResultDimension::GetSortedIndex( long nUnsorted ) const
{
    return aMemberOrder.empty() ? nUnsorted : aMemberOrder[nUnsorted];
}

void ScDPResultDimension::UpdateRunningTotals( const ScDPResultMember* pRefMember, long nMeasure,
                                                ScDPRunningTotalState& rRunning, ScDPRowTotals& rTotals ) const
{
    long nMemberMeasure = nMeasure;
    long nCount = aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        long nSorted = aMemberOrder.empty() ? i : aMemberOrder[i];

        const ScDPResultMember* pMember;
        if (bIsDataLayout)
        {
            DBG_ASSERT(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            pMember = aMembers[0];
            nMemberMeasure = nSorted;
        }
        else
            pMember = aMembers[(USHORT)nSorted];

        if ( pMember->IsVisible() )
        {
            if ( bIsDataLayout )
                rRunning.AddRowIndex( 0, 0 );
            else
                rRunning.AddRowIndex( i, nSorted );
            pMember->UpdateRunningTotals( pRefMember, nMemberMeasure, rRunning, rTotals );
            rRunning.RemoveRowIndex();
        }
    }
}

ScDPDataMember* ScDPResultDimension::GetRowReferenceMember( const ScDPRelativePos* pRelativePos, const String* pName,
                                    const long* pRowIndexes, const long* pColIndexes ) const
{
    // get named, previous/next, or first member of this dimension (first existing if pRelativePos and pName are NULL)

    DBG_ASSERT( pRelativePos == NULL || pName == NULL, "can't use position and name" );

    ScDPDataMember* pColMember = NULL;

    BOOL bFirstExisting = ( pRelativePos == NULL && pName == NULL );
    long nMemberCount = aMembers.Count();
    long nMemberIndex = 0;      // unsorted
    long nDirection = 1;        // forward if no relative position is used
    if ( pRelativePos )
    {
        nDirection = pRelativePos->nDirection;
        nMemberIndex = pRelativePos->nBasePos + nDirection;     // bounds are handled below

        DBG_ASSERT( nDirection == 1 || nDirection == -1, "Direction must be 1 or -1" );
    }
    else if ( pName )
    {
        // search for named member

        ScDPResultMember* pRowMember = aMembers[(USHORT)GetSortedIndex(nMemberIndex)];

        //! use ScDPItemData, as in ScDPDimension::IsValidPage?
        while ( pRowMember && pRowMember->GetName() != *pName )
        {
            ++nMemberIndex;
            if ( nMemberIndex < nMemberCount )
                pRowMember = aMembers[(USHORT)GetSortedIndex(nMemberIndex)];
            else
                pRowMember = NULL;
        }
    }

    BOOL bContinue = TRUE;
    while ( bContinue && nMemberIndex >= 0 && nMemberIndex < nMemberCount )
    {
        ScDPResultMember* pRowMember = aMembers[(USHORT)GetSortedIndex(nMemberIndex)];

        // get child members by given indexes

        const long* pNextRowIndex = pRowIndexes;
        while ( *pNextRowIndex >= 0 && pRowMember )
        {
            const ScDPResultDimension* pRowChild = pRowMember->GetChildDimension();
            if ( pRowChild && *pNextRowIndex < pRowChild->GetMemberCount() )
                pRowMember = pRowChild->GetMember( *pNextRowIndex );
            else
                pRowMember = NULL;
            ++pNextRowIndex;
        }

        if ( pRowMember && pRelativePos )
        {
            //  Skip the member if it has hidden details
            //  (because when looking for the details, it is skipped, too).
            //  Also skip if the member is invisible because it has no data,
            //  for consistent ordering.
            if ( pRowMember->HasHiddenDetails() || !pRowMember->IsVisible() )
                pRowMember = NULL;
        }

        if ( pRowMember )
        {
            pColMember = pRowMember->GetDataRoot();

            const long* pNextColIndex = pColIndexes;
            while ( *pNextColIndex >= 0 && pColMember )
            {
                const ScDPDataDimension* pColChild = pColMember->GetChildDimension();
                if ( pColChild && *pNextColIndex < pColChild->GetMemberCount() )
                    pColMember = pColChild->GetMember( *pNextColIndex );
                else
                    pColMember = NULL;
                ++pNextColIndex;
            }
        }

        // continue searching only if looking for first existing or relative position
        bContinue = ( pColMember == NULL && ( bFirstExisting || pRelativePos ) );
        nMemberIndex += nDirection;
    }

    return pColMember;
}

// static
ScDPDataMember* ScDPResultDimension::GetColReferenceMember( const ScDPRelativePos* pRelativePos, const String* pName,
                            long nRefDimPos, const ScDPRunningTotalState& rRunning )
{
    DBG_ASSERT( pRelativePos == NULL || pName == NULL, "can't use position and name" );

    const long* pColIndexes = rRunning.GetColIndexes();
    const long* pRowIndexes = rRunning.GetRowIndexes();

    // get own row member using all indexes

    ScDPResultMember* pRowMember = rRunning.GetRowResRoot();
    ScDPDataMember* pColMember = NULL;

    const long* pNextRowIndex = pRowIndexes;
    while ( *pNextRowIndex >= 0 && pRowMember )
    {
        const ScDPResultDimension* pRowChild = pRowMember->GetChildDimension();
        if ( pRowChild && *pNextRowIndex < pRowChild->GetMemberCount() )
            pRowMember = pRowChild->GetMember( *pNextRowIndex );
        else
            pRowMember = NULL;
        ++pNextRowIndex;
    }

    // get column (data) members before the reference field
    //! pass rRowParent from ScDPDataMember::UpdateRunningTotals instead

    if ( pRowMember )
    {
        pColMember = pRowMember->GetDataRoot();

        const long* pNextColIndex = pColIndexes;
        long nColSkipped = 0;
        while ( *pNextColIndex >= 0 && pColMember && nColSkipped < nRefDimPos )
        {
            const ScDPDataDimension* pColChild = pColMember->GetChildDimension();
            if ( pColChild && *pNextColIndex < pColChild->GetMemberCount() )
                pColMember = pColChild->GetMember( *pNextColIndex );
            else
                pColMember = NULL;
            ++pNextColIndex;
            ++nColSkipped;
        }
    }

    // get column member for the reference field

    if ( pColMember )
    {
        const ScDPDataDimension* pReferenceDim = pColMember->GetChildDimension();
        if ( pReferenceDim )
        {
            long nReferenceCount = pReferenceDim->GetMemberCount();

            BOOL bFirstExisting = ( pRelativePos == NULL && pName == NULL );
            long nMemberIndex = 0;      // unsorted
            long nDirection = 1;        // forward if no relative position is used
            pColMember = NULL;          // don't use parent dimension's member if none found
            if ( pRelativePos )
            {
                nDirection = pRelativePos->nDirection;
                nMemberIndex = pRelativePos->nBasePos + nDirection;     // bounds are handled below
            }
            else if ( pName )
            {
                // search for named member

                pColMember = pReferenceDim->GetMember( pReferenceDim->GetSortedIndex( nMemberIndex ) );

                //! use ScDPItemData, as in ScDPDimension::IsValidPage?
                while ( pColMember && pColMember->GetName() != *pName )
                {
                    ++nMemberIndex;
                    if ( nMemberIndex < nReferenceCount )
                        pColMember = pReferenceDim->GetMember( pReferenceDim->GetSortedIndex( nMemberIndex ) );
                    else
                        pColMember = NULL;
                }
            }

            BOOL bContinue = TRUE;
            while ( bContinue && nMemberIndex >= 0 && nMemberIndex < nReferenceCount )
            {
                pColMember = pReferenceDim->GetMember( pReferenceDim->GetSortedIndex( nMemberIndex ) );

                // get column members below the reference field

                const long* pNextColIndex = pColIndexes + nRefDimPos + 1;
                while ( *pNextColIndex >= 0 && pColMember )
                {
                    const ScDPDataDimension* pColChild = pColMember->GetChildDimension();
                    if ( pColChild && *pNextColIndex < pColChild->GetMemberCount() )
                        pColMember = pColChild->GetMember( *pNextColIndex );
                    else
                        pColMember = NULL;
                    ++pNextColIndex;
                }

                if ( pColMember && pRelativePos )
                {
                    //  Skip the member if it has hidden details
                    //  (because when looking for the details, it is skipped, too).
                    //  Also skip if the member is invisible because it has no data,
                    //  for consistent ordering.
                    if ( pColMember->HasHiddenDetails() || !pColMember->IsVisible() )
                        pColMember = NULL;
                }

                // continue searching only if looking for first existing or relative position
                bContinue = ( pColMember == NULL && ( bFirstExisting || pRelativePos ) );
                nMemberIndex += nDirection;
            }
        }
        else
            pColMember = NULL;
    }

    return pColMember;
}

void ScDPResultDimension::DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const
{
    String aDimName = bIsDataLayout ? String::CreateFromAscii("(data layout)") : GetName();
    lcl_DumpStrings( String::CreateFromAscii("ScDPResultDimension"), aDimName, String(), pDoc, rPos );

    SCROW nStartRow = rPos.Row();

    long nCount = bIsDataLayout ? 1 : aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        const ScDPResultMember* pMember = aMembers[(USHORT)i];
        pMember->DumpState( pRefMember, pDoc, rPos );
    }

    lcl_Indent( pDoc, nStartRow, rPos );
}

long ScDPResultDimension::GetMemberCount() const
{
    return aMembers.Count();
}

ScDPResultMember* ScDPResultDimension::GetMember(long n) const
{
    return aMembers[(USHORT)n];
}

ScDPResultDimension* ScDPResultDimension::GetFirstChildDimension() const
{
    if ( aMembers.Count() > 0 )
        return aMembers[0]->GetChildDimension();
    else
        return NULL;
}

// -----------------------------------------------------------------------

ScDPDataDimension::ScDPDataDimension( ScDPResultData* pData ) :
    pResultData( pData ),
    pResultDimension( NULL ),
    bIsDataLayout( FALSE )
{
}

ScDPDataDimension::~ScDPDataDimension()
{
}

void ScDPDataDimension::InitFrom( ScDPResultDimension* pDim )
{
    if (!pDim)
        return;

    pResultDimension = pDim;
    bIsDataLayout = pDim->IsDataLayout();

    long nCount = pDim->GetMemberCount();
    for (long i=0; i<nCount; i++)
    {
        ScDPResultMember* pResMem = pDim->GetMember(i);

        ScDPDataMember* pNew = new ScDPDataMember( pResultData, pResMem );
        aMembers.Insert( pNew, aMembers.Count() );

        if ( !pResultData->IsLateInit() )
        {
            //  with LateInit, pResMem hasn't necessarily been initialized yet,
            //  so InitFrom for the new result member is called from its ProcessData method

            ScDPResultDimension* pChildDim = pResMem->GetChildDimension();
            if ( pChildDim )
                pNew->InitFrom( pChildDim );
        }
    }
}

void ScDPDataDimension::ProcessData( const ScDPItemData* pDataMembers, const ScDPValueData* pValues,
                                    const ScDPSubTotalState& rSubState )
{
    // the ScDPItemData array must contain enough entries for all dimensions - this isn't checked

    long nCount = aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        ScDPDataMember* pMember = aMembers[(USHORT)i];

        // always first member for data layout dim
        if ( bIsDataLayout || pMember->IsNamedItem( *pDataMembers ) )
        {
            pMember->ProcessData( pDataMembers + 1, pValues, rSubState );
            return;
        }
    }

    DBG_ERROR("ProcessData: Member not found");
}

void ScDPDataDimension::FillDataRow( const ScDPResultDimension* pRefDim,
                                    uno::Sequence<sheet::DataResult>& rSequence,
                                    long nCol, long nMeasure, BOOL bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState ) const
{
    DBG_ASSERT( pRefDim && pRefDim->GetMemberCount() == aMembers.Count(), "dimensions don't match" );
    DBG_ASSERT( pRefDim == pResultDimension, "wrong dim" );

    const ScMemberSortOrder& rMemberOrder = pRefDim->GetMemberOrder();

    long nMemberMeasure = nMeasure;
    long nMemberCol = nCol;
    long nCount = aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        long nSorted = rMemberOrder.empty() ? i : rMemberOrder[i];

        long nMemberPos = nSorted;
        if (bIsDataLayout)
        {
            DBG_ASSERT(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            nMemberPos = 0;
            nMemberMeasure = nSorted;
        }

        const ScDPResultMember* pRefMember = pRefDim->GetMember(nMemberPos);
        if ( pRefMember->IsVisible() )  //! here or in ScDPDataMember::FillDataRow ???
        {
            const ScDPDataMember* pDataMember = aMembers[(USHORT)nMemberPos];
            pDataMember->FillDataRow( pRefMember, rSequence, nMemberCol, nMemberMeasure, bIsSubTotalRow, rSubState );
            // nMemberCol is modified
        }
    }
}

void ScDPDataDimension::UpdateDataRow( const ScDPResultDimension* pRefDim,
                                    long nMeasure, BOOL bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState ) const
{
    DBG_ASSERT( pRefDim && pRefDim->GetMemberCount() == aMembers.Count(), "dimensions don't match" );
    DBG_ASSERT( pRefDim == pResultDimension, "wrong dim" );

    long nMemberMeasure = nMeasure;
    long nCount = aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        long nMemberPos = i;
        if (bIsDataLayout)
        {
            DBG_ASSERT(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            nMemberPos = 0;
            nMemberMeasure = i;
        }

        // Calculate must be called even if the member is not visible (for use as reference value)
        const ScDPResultMember* pRefMember = pRefDim->GetMember(nMemberPos);
        ScDPDataMember* pDataMember = aMembers[(USHORT)nMemberPos];
        pDataMember->UpdateDataRow( pRefMember, nMemberMeasure, bIsSubTotalRow, rSubState );
    }
}

void ScDPDataDimension::SortMembers( ScDPResultDimension* pRefDim )
{
    long nCount = aMembers.Count();

    if ( pRefDim->IsSortByData() )
    {
        // sort members

        ScMemberSortOrder& rMemberOrder = pRefDim->GetMemberOrder();
        DBG_ASSERT( rMemberOrder.empty(), "sort twice?" );
        rMemberOrder.resize( nCount );
        for (long nPos=0; nPos<nCount; nPos++)
            rMemberOrder[nPos] = nPos;

        ScDPColMembersOrder aComp( *this, pRefDim->GetSortMeasure(), pRefDim->IsSortAscending() );
        ::std::sort( rMemberOrder.begin(), rMemberOrder.end(), aComp );
    }

    // handle children

    DBG_ASSERT( pRefDim && pRefDim->GetMemberCount() == aMembers.Count(), "dimensions don't match" );
    DBG_ASSERT( pRefDim == pResultDimension, "wrong dim" );

    // for data layout, call only once - sorting measure is always taken from settings
    long nLoopCount = bIsDataLayout ? 1 : nCount;
    for (long i=0; i<nLoopCount; i++)
    {
        ScDPResultMember* pRefMember = pRefDim->GetMember(i);
        if ( pRefMember->IsVisible() )  //! here or in ScDPDataMember ???
        {
            ScDPDataMember* pDataMember = aMembers[(USHORT)i];
            pDataMember->SortMembers( pRefMember );
        }
    }
}

void ScDPDataDimension::DoAutoShow( ScDPResultDimension* pRefDim )
{
    long nCount = aMembers.Count();

    // handle children first, before changing the visible state

    DBG_ASSERT( pRefDim && pRefDim->GetMemberCount() == aMembers.Count(), "dimensions don't match" );
    DBG_ASSERT( pRefDim == pResultDimension, "wrong dim" );

    // for data layout, call only once - sorting measure is always taken from settings
    long nLoopCount = bIsDataLayout ? 1 : nCount;
    for (long i=0; i<nLoopCount; i++)
    {
        ScDPResultMember* pRefMember = pRefDim->GetMember(i);
        if ( pRefMember->IsVisible() )  //! here or in ScDPDataMember ???
        {
            ScDPDataMember* pDataMember = aMembers[(USHORT)i];
            pDataMember->DoAutoShow( pRefMember );
        }
    }

    if ( pRefDim->IsAutoShow() && pRefDim->GetAutoCount() > 0 && pRefDim->GetAutoCount() < nCount )
    {
        // establish temporary order, hide remaining members

        ScMemberSortOrder aAutoOrder;
        aAutoOrder.resize( nCount );
        long nPos;
        for (nPos=0; nPos<nCount; nPos++)
            aAutoOrder[nPos] = nPos;

        ScDPColMembersOrder aComp( *this, pRefDim->GetAutoMeasure(), !pRefDim->IsAutoTopItems() );
        ::std::sort( aAutoOrder.begin(), aAutoOrder.end(), aComp );

        // look for equal values to the last included one

        long nIncluded = pRefDim->GetAutoCount();
        ScDPDataMember* pDataMember1 = aMembers[(USHORT)aAutoOrder[nIncluded - 1]];
        if ( !pDataMember1->IsVisible() )
            pDataMember1 = NULL;
        BOOL bContinue = TRUE;
        while ( bContinue )
        {
            bContinue = FALSE;
            if ( nIncluded < nCount )
            {
                ScDPDataMember* pDataMember2 = aMembers[(USHORT)aAutoOrder[nIncluded]];
                if ( !pDataMember2->IsVisible() )
                    pDataMember2 = NULL;

                if ( lcl_IsEqual( pDataMember1, pDataMember2, pRefDim->GetAutoMeasure() ) )
                {
                    ++nIncluded;                // include more members if values are equal
                    bContinue = TRUE;
                }
            }
        }

        // hide the remaining members

        for (nPos = nIncluded; nPos < nCount; nPos++)
        {
            ScDPResultMember* pMember = pRefDim->GetMember(aAutoOrder[nPos]);
            pMember->SetAutoHidden();
        }
    }
}

void ScDPDataDimension::ResetResults()
{
    long nCount = aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        //  sort order doesn't matter

        long nMemberPos = bIsDataLayout ? 0 : i;
        ScDPDataMember* pDataMember = aMembers[(USHORT)nMemberPos];
        pDataMember->ResetResults();
    }
}

long ScDPDataDimension::GetSortedIndex( long nUnsorted ) const
{
    if (!pResultDimension)
       return nUnsorted;

    const ScMemberSortOrder& rMemberOrder = pResultDimension->GetMemberOrder();
    return rMemberOrder.empty() ? nUnsorted : rMemberOrder[nUnsorted];
}

void ScDPDataDimension::UpdateRunningTotals( const ScDPResultDimension* pRefDim,
                                    long nMeasure, BOOL bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState, ScDPRunningTotalState& rRunning,
                                    ScDPRowTotals& rTotals, const ScDPResultMember& rRowParent ) const
{
    DBG_ASSERT( pRefDim && pRefDim->GetMemberCount() == aMembers.Count(), "dimensions don't match" );
    DBG_ASSERT( pRefDim == pResultDimension, "wrong dim" );

    long nMemberMeasure = nMeasure;
    long nCount = aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        const ScMemberSortOrder& rMemberOrder = pRefDim->GetMemberOrder();
        long nSorted = rMemberOrder.empty() ? i : rMemberOrder[i];

        long nMemberPos = nSorted;
        if (bIsDataLayout)
        {
            DBG_ASSERT(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            nMemberPos = 0;
            nMemberMeasure = nSorted;
        }

        const ScDPResultMember* pRefMember = pRefDim->GetMember(nMemberPos);
        if ( pRefMember->IsVisible() )  //! here or in ScDPDataMember::UpdateRunningTotals ???
        {
            if ( bIsDataLayout )
                rRunning.AddColIndex( 0, 0 );
            else
                rRunning.AddColIndex( i, nSorted );

            ScDPDataMember* pDataMember = aMembers[(USHORT)nMemberPos];
            pDataMember->UpdateRunningTotals( pRefMember, nMemberMeasure,
                                            bIsSubTotalRow, rSubState, rRunning, rTotals, rRowParent );

            rRunning.RemoveColIndex();
        }
    }
}

void ScDPDataDimension::DumpState( const ScDPResultDimension* pRefDim, ScDocument* pDoc, ScAddress& rPos ) const
{
    String aDimName = String::CreateFromAscii( bIsDataLayout ? "(data layout)" : "(unknown)" );
    lcl_DumpStrings( String::CreateFromAscii("ScDPDataDimension"), aDimName, String(), pDoc, rPos );

    SCROW nStartRow = rPos.Row();

    long nCount = bIsDataLayout ? 1 : aMembers.Count();
    for (long i=0; i<nCount; i++)
    {
        const ScDPResultMember* pRefMember = pRefDim->GetMember(i);
        const ScDPDataMember* pDataMember = aMembers[(USHORT)i];
        pDataMember->DumpState( pRefMember, pDoc, rPos );
    }

    lcl_Indent( pDoc, nStartRow, rPos );
}

long ScDPDataDimension::GetMemberCount() const
{
    return aMembers.Count();
}

ScDPDataMember* ScDPDataDimension::GetMember(long n) const
{
    return aMembers[(USHORT)n];
}

