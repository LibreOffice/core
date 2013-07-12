/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "dptabres.hxx"

#include "dptabdat.hxx"
#include "dptabsrc.hxx"
#include "global.hxx"
#include "subtotal.hxx"
#include "globstr.hrc"
#include "dpitemdata.hxx"

#include "document.hxx"     // for DumpState only!
#include "stlalgorithm.hxx"
#include "dpresfilter.hxx"
#include "dputil.hxx"

#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <rtl/strbuf.hxx>

#include <math.h>
#include <float.h>          //! Test !!!
#include <algorithm>
#include <boost/unordered_map.hpp>
#include <boost/scoped_ptr.hpp>

#include <com/sun/star/sheet/DataResultFlags.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#include <com/sun/star/sheet/DataPilotFieldShowItemsMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

using namespace com::sun::star;
using ::std::vector;
using ::std::pair;
using ::com::sun::star::uno::Sequence;

namespace {

sal_uInt16 nFuncStrIds[12] =     // passend zum enum ScSubTotalFunc
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

bool lcl_SearchMember( const std::vector <ScDPResultMember *>& list, SCROW nOrder, SCROW& rIndex)
{
    rIndex = list.size();
    bool bFound = false;
    SCROW  nLo = 0;
    SCROW nHi = list.size() - 1;
    SCROW nIndex;
    while (nLo <= nHi)
    {
        nIndex = (nLo + nHi) / 2;
        if ( list[nIndex]->GetOrder() < nOrder )
            nLo = nIndex + 1;
        else
        {
            nHi = nIndex - 1;
            if ( list[nIndex]->GetOrder() == nOrder )
            {
                bFound = true;
                nLo = nIndex;
            }
        }
    }
    rIndex = nLo;
    return bFound;
}

class FilterStack
{
    std::vector<ScDPResultFilter>& mrFilters;
public:
    FilterStack(std::vector<ScDPResultFilter>& rFilters) : mrFilters(rFilters) {}

    void pushDimName(const OUString& rName, bool bDataLayout)
    {
        mrFilters.push_back(ScDPResultFilter(rName, bDataLayout));
    }

    void pushDimValue(const OUString& rValue)
    {
        ScDPResultFilter& rFilter = mrFilters.back();
        rFilter.maValue = rValue;
        rFilter.mbHasValue = true;
    }

    ~FilterStack()
    {
        ScDPResultFilter& rFilter = mrFilters.back();
        if (rFilter.mbHasValue)
            rFilter.mbHasValue = false;
        else
            mrFilters.pop_back();
    }
};

}

//
// function objects for sorting of the column and row members:
//

class ScDPRowMembersOrder
{
    ScDPResultDimension& rDimension;
    long                 nMeasure;
    bool                 bAscending;

public:
            ScDPRowMembersOrder( ScDPResultDimension& rDim, long nM, bool bAsc ) :
                rDimension(rDim),
                nMeasure(nM),
                bAscending(bAsc)
            {}
            ~ScDPRowMembersOrder() {}

    bool operator()( sal_Int32 nIndex1, sal_Int32 nIndex2 ) const;
};

class ScDPColMembersOrder
{
    ScDPDataDimension& rDimension;
    long               nMeasure;
    bool               bAscending;

public:
            ScDPColMembersOrder( ScDPDataDimension& rDim, long nM, bool bAsc ) :
                rDimension(rDim),
                nMeasure(nM),
                bAscending(bAsc)
            {}
            ~ScDPColMembersOrder() {}

    bool operator()( sal_Int32 nIndex1, sal_Int32 nIndex2 ) const;
};

static bool lcl_IsLess( const ScDPDataMember* pDataMember1, const ScDPDataMember* pDataMember2, long nMeasure, bool bAscending )
{
    // members can be NULL if used for rows

    ScDPSubTotalState aEmptyState;
    const ScDPAggData* pAgg1 = pDataMember1 ? pDataMember1->GetConstAggData( nMeasure, aEmptyState ) : NULL;
    const ScDPAggData* pAgg2 = pDataMember2 ? pDataMember2->GetConstAggData( nMeasure, aEmptyState ) : NULL;

    bool bError1 = pAgg1 && pAgg1->HasError();
    bool bError2 = pAgg2 && pAgg2->HasError();
    if ( bError1 )
        return false;       // errors are always sorted at the end
    else if ( bError2 )
        return true;            // errors are always sorted at the end
    else
    {
        double fVal1 = ( pAgg1 && pAgg1->HasData() ) ? pAgg1->GetResult() : 0.0;    // no data is sorted as 0
        double fVal2 = ( pAgg2 && pAgg2->HasData() ) ? pAgg2->GetResult() : 0.0;

        // compare values
        // don't have to check approxEqual, as this is the only sort criterion

        return bAscending ? ( fVal1 < fVal2 ) : ( fVal1 > fVal2 );
    }
}

static bool lcl_IsEqual( const ScDPDataMember* pDataMember1, const ScDPDataMember* pDataMember2, long nMeasure )
{
    // members can be NULL if used for rows

    ScDPSubTotalState aEmptyState;
    const ScDPAggData* pAgg1 = pDataMember1 ? pDataMember1->GetConstAggData( nMeasure, aEmptyState ) : NULL;
    const ScDPAggData* pAgg2 = pDataMember2 ? pDataMember2->GetConstAggData( nMeasure, aEmptyState ) : NULL;

    bool bError1 = pAgg1 && pAgg1->HasError();
    bool bError2 = pAgg2 && pAgg2->HasError();
    if ( bError1 )
    {
        if ( bError2 )
            return true;        // equal
        else
            return false;
    }
    else if ( bError2 )
        return false;
    else
    {
        double fVal1 = ( pAgg1 && pAgg1->HasData() ) ? pAgg1->GetResult() : 0.0;    // no data is sorted as 0
        double fVal2 = ( pAgg2 && pAgg2->HasData() ) ? pAgg2->GetResult() : 0.0;

        // compare values
        // this is used to find equal data at the end of the AutoShow range, so approxEqual must be used

        return rtl::math::approxEqual( fVal1, fVal2 );
    }
}

bool ScDPRowMembersOrder::operator()( sal_Int32 nIndex1, sal_Int32 nIndex2 ) const
{
    const ScDPResultMember* pMember1 = rDimension.GetMember(nIndex1);
    const ScDPResultMember* pMember2 = rDimension.GetMember(nIndex2);

// make the hide item to the largest order.
    if ( !pMember1->IsVisible() || !pMember2->IsVisible() )
        return pMember1->IsVisible();
    const ScDPDataMember* pDataMember1 =  pMember1->GetDataRoot() ;
    const ScDPDataMember* pDataMember2 =  pMember2->GetDataRoot();
    //  GetDataRoot can be NULL if there was no data.
    //  IsVisible == false can happen after AutoShow.
    return lcl_IsLess( pDataMember1, pDataMember2, nMeasure, bAscending );
}

bool ScDPColMembersOrder::operator()( sal_Int32 nIndex1, sal_Int32 nIndex2 ) const
{
    const ScDPDataMember* pDataMember1 = rDimension.GetMember(nIndex1);
    const ScDPDataMember* pDataMember2 = rDimension.GetMember(nIndex2);
        bool bHide1 = pDataMember1 && !pDataMember1->IsVisible();
        bool bHide2 =  pDataMember2 && !pDataMember2->IsVisible();
        if ( bHide1 || bHide2 )
            return !bHide1;
    return lcl_IsLess( pDataMember1, pDataMember2, nMeasure, bAscending );
}

ScDPInitState::Member::Member(long nSrcIndex, SCROW nNameIndex) :
    mnSrcIndex(nSrcIndex), mnNameIndex(nNameIndex) {}

void ScDPInitState::AddMember( long nSourceIndex, SCROW nMember )
{
    maMembers.push_back(Member(nSourceIndex, nMember));
}

void ScDPInitState::RemoveMember()
{
    OSL_ENSURE(!maMembers.empty(), "ScDPInitState::RemoveMember: Attempt to remmove member while empty.");
    if (!maMembers.empty())
        maMembers.pop_back();
}

namespace {

#if DEBUG_PIVOT_TABLE
void lcl_DumpRow(
    const OUString& rType, const OUString& rName, const ScDPAggData* pAggData,
    ScDocument* pDoc, ScAddress& rPos )
{
    SCCOL nCol = rPos.Col();
    SCROW nRow = rPos.Row();
    SCTAB nTab = rPos.Tab();
    pDoc->SetString( nCol++, nRow, nTab, rType );
    pDoc->SetString( nCol++, nRow, nTab, rName );
    while ( pAggData )
    {
        pDoc->SetValue( nCol++, nRow, nTab, pAggData->GetResult() );
        pAggData = pAggData->GetExistingChild();
    }
    rPos.SetRow( nRow + 1 );
}

void lcl_Indent( ScDocument* pDoc, SCROW nStartRow, const ScAddress& rPos )
{
    SCCOL nCol = rPos.Col();
    SCTAB nTab = rPos.Tab();

    OUString aString;
    for (SCROW nRow = nStartRow; nRow < rPos.Row(); nRow++)
    {
        aString = pDoc->GetString(nCol, nRow, nTab);
        if (!aString.isEmpty())
        {
            aString = " " + aString;
            pDoc->SetString( nCol, nRow, nTab, aString );
        }
    }
}
#endif

}

ScDPRunningTotalState::ScDPRunningTotalState( ScDPResultMember* pColRoot, ScDPResultMember* pRowRoot ) :
    pColResRoot(pColRoot), pRowResRoot(pRowRoot)
{
    // These arrays should never be empty as the terminating value must be present at all times.
    maColVisible.push_back(-1);
    maColSorted.push_back(-1);
    maRowVisible.push_back(-1);
    maRowSorted.push_back(-1);
}

const ScDPRunningTotalState::IndexArray& ScDPRunningTotalState::GetColVisible() const
{
    return maColVisible;
}

const ScDPRunningTotalState::IndexArray& ScDPRunningTotalState::GetColSorted() const
{
    return maColSorted;
}

const ScDPRunningTotalState::IndexArray& ScDPRunningTotalState::GetRowVisible() const
{
    return maRowVisible;
}

const ScDPRunningTotalState::IndexArray& ScDPRunningTotalState::GetRowSorted() const
{
    return maRowSorted;
}

void ScDPRunningTotalState::AddColIndex( long nVisible, long nSorted )
{
    maColVisible.back() = nVisible;
    maColVisible.push_back(-1);

    maColSorted.back() = nSorted;
    maColSorted.push_back(-1);
}

void ScDPRunningTotalState::AddRowIndex( long nVisible, long nSorted )
{
    maRowVisible.back() = nVisible;
    maRowVisible.push_back(-1);

    maRowSorted.back() = nSorted;
    maRowSorted.push_back(-1);
}

void ScDPRunningTotalState::RemoveColIndex()
{
    OSL_ENSURE(!maColVisible.empty() && !maColSorted.empty(), "ScDPRunningTotalState::RemoveColIndex: array is already empty!");
    if (maColVisible.size() >= 2)
    {
        maColVisible.pop_back();
        maColVisible.back() = -1;
    }

    if (maColSorted.size() >= 2)
    {
        maColSorted.pop_back();
        maColSorted.back() = -1;
    }
}

void ScDPRunningTotalState::RemoveRowIndex()
{
    OSL_ENSURE(!maRowVisible.empty() && !maRowSorted.empty(), "ScDPRunningTotalState::RemoveRowIndex: array is already empty!");
    if (maRowVisible.size() >= 2)
    {
        maRowVisible.pop_back();
        maRowVisible.back() = -1;
    }

    if (maRowSorted.size() >= 2)
    {
        maRowSorted.pop_back();
        maRowSorted.back() = -1;
    }
}

// -----------------------------------------------------------------------

ScDPRelativePos::ScDPRelativePos( long nBase, long nDir ) :
    nBasePos( nBase ),
    nDirection( nDir )
{
}

// -----------------------------------------------------------------------

void ScDPAggData::Update( const ScDPValue& rNext, ScSubTotalFunc eFunc, const ScDPSubTotalState& rSubState )
{
    if (nCount<0)       // error?
        return;         // nothing more...

    if (rNext.meType == ScDPValue::Empty)
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
        if (rNext.meType == ScDPValue::Error)
        {
            nCount = -1;        // -1 for error (not for CNT2)
            return;
        }
        if (rNext.meType == ScDPValue::String)
            return;             // ignore
    }

    ++nCount;           // for all functions

    switch (eFunc)
    {
        case SUBTOTAL_FUNC_SUM:
        case SUBTOTAL_FUNC_AVE:
            if ( !SubTotal::SafePlus( fVal, rNext.mfValue ) )
                nCount = -1;                            // -1 for error
            break;
        case SUBTOTAL_FUNC_PROD:
            if ( nCount == 1 )          // copy first value (fVal is initialized to 0)
                fVal = rNext.mfValue;
            else if ( !SubTotal::SafeMult( fVal, rNext.mfValue ) )
                nCount = -1;                            // -1 for error
            break;
        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:
            //  nothing more than incrementing nCount
            break;
        case SUBTOTAL_FUNC_MAX:
            if ( nCount == 1 || rNext.mfValue > fVal )
                fVal = rNext.mfValue;
            break;
        case SUBTOTAL_FUNC_MIN:
            if ( nCount == 1 || rNext.mfValue < fVal )
                fVal = rNext.mfValue;
            break;
        case SUBTOTAL_FUNC_STD:
        case SUBTOTAL_FUNC_STDP:
        case SUBTOTAL_FUNC_VAR:
        case SUBTOTAL_FUNC_VARP:
            {
                // fAux is used to sum up squares
                if ( !SubTotal::SafePlus( fVal, rNext.mfValue ) )
                    nCount = -1;                            // -1 for error
                double fAdd = rNext.mfValue;
                if ( !SubTotal::SafeMult( fAdd, rNext.mfValue ) ||
                     !SubTotal::SafePlus( fAux, fAdd ) )
                    nCount = -1;                            // -1 for error
            }
            break;
        default:
            OSL_FAIL("invalid function");
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

    bool bError = false;
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
            OSL_FAIL("invalid function");
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
                OSL_FAIL("invalid function");
        }
    }

    bool bEmpty = ( nCount == 0 );          // no data

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

bool ScDPAggData::IsCalculated() const
{
    return ( nCount <= SC_DPAGG_RESULT_EMPTY );
}

double ScDPAggData::GetResult() const
{
    OSL_ENSURE( IsCalculated(), "ScDPAggData not calculated" );

    return fVal;        // use calculated value
}

bool ScDPAggData::HasError() const
{
    OSL_ENSURE( IsCalculated(), "ScDPAggData not calculated" );

    return ( nCount == SC_DPAGG_RESULT_ERROR );
}

bool ScDPAggData::HasData() const
{
    OSL_ENSURE( IsCalculated(), "ScDPAggData not calculated" );

    return ( nCount != SC_DPAGG_RESULT_EMPTY );     // values or error
}

void ScDPAggData::SetResult( double fNew )
{
    OSL_ENSURE( IsCalculated(), "ScDPAggData not calculated" );

    fVal = fNew;        // don't reset error flag
}

void ScDPAggData::SetError()
{
    OSL_ENSURE( IsCalculated(), "ScDPAggData not calculated" );

    nCount = SC_DPAGG_RESULT_ERROR;
}

void ScDPAggData::SetEmpty( bool bSet )
{
    OSL_ENSURE( IsCalculated(), "ScDPAggData not calculated" );

    if ( bSet )
        nCount = SC_DPAGG_RESULT_EMPTY;
    else
        nCount = SC_DPAGG_RESULT_VALID;
}

double ScDPAggData::GetAuxiliary() const
{
    // after Calculate, fAux is used as auxiliary value for running totals and reference values
    OSL_ENSURE( IsCalculated(), "ScDPAggData not calculated" );

    return fAux;
}

void ScDPAggData::SetAuxiliary( double fNew )
{
    // after Calculate, fAux is used as auxiliary value for running totals and reference values
    OSL_ENSURE( IsCalculated(), "ScDPAggData not calculated" );

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

#if DEBUG_PIVOT_TABLE
void ScDPAggData::Dump(int nIndent) const
{
    std::string aIndent(nIndent*2, ' ');
    std::cout << aIndent << "* ";
    if (IsCalculated())
        std::cout << GetResult();
    else
        std::cout << "not calculated";

    std::cout << "  [val=" << fVal << "; aux=" << fAux << "; count=" << nCount << "]" << std::endl;
}
#endif

// -----------------------------------------------------------------------

ScDPRowTotals::ScDPRowTotals() :
    bIsInColRoot( false )
{
}

ScDPRowTotals::~ScDPRowTotals()
{
}

static ScDPAggData* lcl_GetChildTotal( ScDPAggData* pFirst, long nMeasure )
{
    OSL_ENSURE( nMeasure >= 0, "GetColTotal: no measure" );

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

static ScSubTotalFunc lcl_GetForceFunc( const ScDPLevel* pLevel, long nFuncNo )
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
                eRet = ScDPUtil::toSubTotalFunc(eUser);
        }
    }
    return eRet;
}

// -----------------------------------------------------------------------

ScDPResultData::ScDPResultData( ScDPSource& rSrc ) :
    mrSource(rSrc),
    bLateInit( false ),
    bDataAtCol( false ),
    bDataAtRow( false )
{
}

ScDPResultData::~ScDPResultData()
{
    std::for_each(maDimMembers.begin(), maDimMembers.end(), ScDeleteObjectByPtr<ResultMembers>());
}

void ScDPResultData::SetMeasureData(
    std::vector<ScSubTotalFunc>& rFunctions, std::vector<sheet::DataPilotFieldReference>& rRefs,
    std::vector<sal_uInt16>& rRefOrient, std::vector<OUString>& rNames )
{
    // We need to have at least one measure data at all times.

    maMeasureFuncs.swap(rFunctions);
    if (maMeasureFuncs.empty())
        maMeasureFuncs.push_back(SUBTOTAL_FUNC_NONE);

    maMeasureRefs.swap(rRefs);
    if (maMeasureRefs.empty())
        maMeasureRefs.push_back(sheet::DataPilotFieldReference()); // default ctor is ok.

    maMeasureRefOrients.swap(rRefOrient);
    if (maMeasureRefOrients.empty())
        maMeasureRefOrients.push_back(sheet::DataPilotFieldOrientation_HIDDEN);

    maMeasureNames.swap(rNames);
    if (maMeasureNames.empty())
        maMeasureNames.push_back(ScGlobal::GetRscString(STR_EMPTYDATA));
}

void ScDPResultData::SetDataLayoutOrientation( sal_uInt16 nOrient )
{
    bDataAtCol = ( nOrient == sheet::DataPilotFieldOrientation_COLUMN );
    bDataAtRow = ( nOrient == sheet::DataPilotFieldOrientation_ROW );
}

void ScDPResultData::SetLateInit( bool bSet )
{
    bLateInit = bSet;
}

long ScDPResultData::GetColStartMeasure() const
{
    if (maMeasureFuncs.size() == 1)
        return 0;

    return bDataAtCol ? SC_DPMEASURE_ALL : SC_DPMEASURE_ANY;
}

long ScDPResultData::GetRowStartMeasure() const
{
    if (maMeasureFuncs.size() == 1)
        return 0;

    return bDataAtRow ? SC_DPMEASURE_ALL : SC_DPMEASURE_ANY;
}

ScSubTotalFunc ScDPResultData::GetMeasureFunction(long nMeasure) const
{
    OSL_ENSURE((size_t) nMeasure < maMeasureFuncs.size(), "bumm");
    return maMeasureFuncs[nMeasure];
}

const sheet::DataPilotFieldReference& ScDPResultData::GetMeasureRefVal(long nMeasure) const
{
    OSL_ENSURE((size_t) nMeasure < maMeasureRefs.size(), "bumm");
    return maMeasureRefs[nMeasure];
}

sal_uInt16 ScDPResultData::GetMeasureRefOrient(long nMeasure) const
{
    OSL_ENSURE((size_t) nMeasure < maMeasureRefOrients.size(), "bumm");
    return maMeasureRefOrients[nMeasure];
}

OUString ScDPResultData::GetMeasureString(long nMeasure, bool bForce, ScSubTotalFunc eForceFunc, bool& rbTotalResult) const
{
    //  with bForce==true, return function instead of "result" for single measure
    //  with eForceFunc != SUBTOTAL_FUNC_NONE, always use eForceFunc
    rbTotalResult = false;
    if ( nMeasure < 0 || (maMeasureFuncs.size() == 1 && !bForce && eForceFunc == SUBTOTAL_FUNC_NONE) )
    {
        //  for user-specified subtotal function with all measures,
        //  display only function name
        if ( eForceFunc != SUBTOTAL_FUNC_NONE )
            return ScGlobal::GetRscString(nFuncStrIds[eForceFunc]);

        rbTotalResult = true;
        return ScGlobal::GetRscString(STR_TABLE_ERGEBNIS);
    }
    else
    {
        OSL_ENSURE((size_t) nMeasure < maMeasureFuncs.size(), "bumm");
        const ScDPDimension* pDataDim = mrSource.GetDataDimension(nMeasure);
        if (pDataDim)
        {
            const OUString* pLayoutName = pDataDim->GetLayoutName();
            if (pLayoutName)
                return *pLayoutName;
        }

        ScSubTotalFunc eFunc = ( eForceFunc == SUBTOTAL_FUNC_NONE ) ?
                                    GetMeasureFunction(nMeasure) : eForceFunc;

        return ScDPUtil::getDisplayedMeasureName(maMeasureNames[nMeasure], eFunc);
    }
}

OUString ScDPResultData::GetMeasureDimensionName(long nMeasure) const
{
    if ( nMeasure < 0 )
    {
        OSL_FAIL("GetMeasureDimensionName: negative");
        return OUString("***");
    }

    return mrSource.GetDataDimName(nMeasure);
}

bool ScDPResultData::IsBaseForGroup( long nDim ) const
{
    return mrSource.GetData()->IsBaseForGroup(nDim);
}

long ScDPResultData::GetGroupBase( long nGroupDim ) const
{
    return mrSource.GetData()->GetGroupBase(nGroupDim);
}

bool ScDPResultData::IsNumOrDateGroup( long nDim ) const
{
    return mrSource.GetData()->IsNumOrDateGroup(nDim);
}

bool ScDPResultData::IsInGroup( SCROW nGroupDataId, long nGroupIndex,
                                const ScDPItemData& rBaseData, long nBaseIndex ) const
{
    const ScDPItemData* pGroupData = mrSource.GetItemDataById(nGroupIndex , nGroupDataId);
    if ( pGroupData )
        return mrSource.GetData()->IsInGroup(*pGroupData, nGroupIndex, rBaseData, nBaseIndex);
    else
        return false;
}

bool ScDPResultData::HasCommonElement( SCROW nFirstDataId, long nFirstIndex,
                                       const ScDPItemData& rSecondData, long nSecondIndex ) const
{
    const ScDPItemData* pFirstData = mrSource.GetItemDataById(nFirstIndex , nFirstDataId);
    if ( pFirstData )
        return mrSource.GetData()->HasCommonElement(*pFirstData, nFirstIndex, rSecondData, nSecondIndex);
    else
        return false;
}

const ScDPSource& ScDPResultData::GetSource() const
{
    return mrSource;
}

ResultMembers* ScDPResultData::GetDimResultMembers(long nDim, ScDPDimension* pDim, ScDPLevel* pLevel) const
{
    if (nDim < static_cast<long>(maDimMembers.size()) && maDimMembers[nDim])
        return maDimMembers[nDim];

    maDimMembers.resize(nDim+1, NULL);

    ResultMembers* pResultMembers = new ResultMembers();
    // global order is used to initialize aMembers, so it doesn't have to be looked at later
    const ScMemberSortOrder& rGlobalOrder = pLevel->GetGlobalOrder();

    ScDPMembers* pMembers = pLevel->GetMembersObject();
    long nMembCount = pMembers->getCount();
    for (long i = 0; i < nMembCount; ++i)
    {
        long nSorted = rGlobalOrder.empty() ? i : rGlobalOrder[i];
        ScDPMember* pMember = pMembers->getByIndex(nSorted);
        if (!pResultMembers->FindMember(pMember->GetItemDataId()))
        {
            ScDPParentDimData* pNew = new ScDPParentDimData(i, pDim, pLevel, pMember);
            pResultMembers->InsertMember(pNew);
        }
    }

    maDimMembers[nDim] = pResultMembers;
    return maDimMembers[nDim];
}

// -----------------------------------------------------------------------


ScDPResultMember::ScDPResultMember(
    const ScDPResultData* pData, const ScDPParentDimData& rParentDimData, bool bForceSub ) :
    pResultData( pData ),
       aParentDimData( rParentDimData ),
    pChildDimension( NULL ),
    pDataRoot( NULL ),
    bHasElements( false ),
    bForceSubTotal( bForceSub ),
    bHasHiddenDetails( false ),
    bInitialized( false ),
    bAutoHidden( false ),
    nMemberStep( 1 )
{
    // pParentLevel/pMemberDesc is 0 for root members
}

ScDPResultMember::ScDPResultMember(
    const ScDPResultData* pData, bool bForceSub ) :
    pResultData( pData ),
        pChildDimension( NULL ),
    pDataRoot( NULL ),
    bHasElements( false ),
    bForceSubTotal( bForceSub ),
    bHasHiddenDetails( false ),
    bInitialized( false ),
    bAutoHidden( false ),
    nMemberStep( 1 )
{
}
ScDPResultMember::~ScDPResultMember()
{
    delete pChildDimension;
    delete pDataRoot;
}

OUString ScDPResultMember::GetName() const
{
  const ScDPMember* pMemberDesc = GetDPMember();
    if (pMemberDesc)
        return pMemberDesc->GetNameStr();
    else
        return ScGlobal::GetRscString(STR_PIVOT_TOTAL);         // root member
}

OUString ScDPResultMember::GetDisplayName() const
{
    const ScDPMember* pDPMember = GetDPMember();
    if (!pDPMember)
        return OUString();

    ScDPItemData aItem;
    pDPMember->FillItemData(aItem);
    if (aParentDimData.mpParentDim)
    {
        long nDim = aParentDimData.mpParentDim->GetDimension();
        return pResultData->GetSource().GetData()->GetFormattedString(nDim, aItem);
    }

    return aItem.GetString();
}

void ScDPResultMember::FillItemData( ScDPItemData& rData ) const
{
    const ScDPMember*   pMemberDesc = GetDPMember();
    if (pMemberDesc)
        pMemberDesc->FillItemData( rData );
    else
        rData.SetString( ScGlobal::GetRscString(STR_PIVOT_TOTAL) );     // root member
}

bool ScDPResultMember::IsNamedItem( SCROW nIndex ) const
{
    //! store ScDPMember pointer instead of ScDPMember ???
    const ScDPMember* pMemberDesc = GetDPMember();
    if (pMemberDesc)
        return pMemberDesc->IsNamedItem(nIndex);
    return false;
}

bool ScDPResultMember::IsValidEntry( const vector< SCROW >& aMembers ) const
{
    if ( !IsValid() )
        return false;

    const ScDPResultDimension* pChildDim = GetChildDimension();
    if (pChildDim)
    {
        if (aMembers.size() < 2)
            return false;

        vector<SCROW>::const_iterator itr = aMembers.begin();
        vector<SCROW> aChildMembers(++itr, aMembers.end());
        return pChildDim->IsValidEntry(aChildMembers);
    }
    else
        return true;
}

void ScDPResultMember::InitFrom( const vector<ScDPDimension*>& ppDim, const vector<ScDPLevel*>& ppLev,
                                 size_t nPos, ScDPInitState& rInitState ,
                                 bool bInitChild )
{
    //  with LateInit, initialize only those members that have data
    if ( pResultData->IsLateInit() )
        return;

    bInitialized = true;

    if (nPos >= ppDim.size())
        return;

    //  skip child dimension if details are not shown
    if ( GetDPMember() && !GetDPMember()->getShowDetails() )
    {
        // Show DataLayout dimention
        nMemberStep = 1;
        while ( nPos < ppDim.size() )
        {
            if (  ppDim[nPos] ->getIsDataLayoutDimension() )
            {
                 if ( !pChildDimension )
                        pChildDimension = new ScDPResultDimension( pResultData );
                    pChildDimension->InitFrom( ppDim, ppLev, nPos, rInitState , false );
                    return;
            }
            else
            { //find next dim
                nPos ++;
                nMemberStep ++;
            }
        }
        bHasHiddenDetails = true;   // only if there is a next dimension
        return;
    }

    if ( bInitChild )
    {
        pChildDimension = new ScDPResultDimension( pResultData );
        pChildDimension->InitFrom(ppDim, ppLev, nPos, rInitState, true);
    }
}

void ScDPResultMember::LateInitFrom(
    LateInitParams& rParams, const vector<SCROW>& pItemData, size_t nPos, ScDPInitState& rInitState)
{
    //  without LateInit, everything has already been initialized
    if ( !pResultData->IsLateInit() )
        return;

    bInitialized = true;

    if ( rParams.IsEnd( nPos )  /*nPos >= ppDim.size()*/)
        // No next dimension.  Bail out.
        return;

    //  skip child dimension if details are not shown
    if ( GetDPMember() && !GetDPMember()->getShowDetails() )
    {
        // Show DataLayout dimention
        nMemberStep = 1;
        while ( !rParams.IsEnd( nPos ) )
        {
            if (  rParams.GetDim( nPos ) ->getIsDataLayoutDimension() )
            {
                if ( !pChildDimension )
                    pChildDimension = new ScDPResultDimension( pResultData );

                // #i111462# reset InitChild flag only for this child dimension's LateInitFrom call,
                // not for following members of parent dimensions
                bool bWasInitChild = rParams.GetInitChild();
                rParams.SetInitChild( false );
                pChildDimension->LateInitFrom( rParams, pItemData, nPos, rInitState );
                rParams.SetInitChild( bWasInitChild );
                return;
            }
            else
            { //find next dim
                nPos ++;
                nMemberStep ++;
            }
        }
        bHasHiddenDetails = true;   // only if there is a next dimension
        return;
    }

    //  LateInitFrom is called several times...
    if ( rParams.GetInitChild() )
    {
        if ( !pChildDimension )
            pChildDimension = new ScDPResultDimension( pResultData );
        pChildDimension->LateInitFrom( rParams, pItemData, nPos, rInitState );
    }
}

bool ScDPResultMember::IsSubTotalInTitle(long nMeasure) const
{
    bool bRet = false;
    if ( pChildDimension && /*pParentLevel*/GetParentLevel() &&
         /*pParentLevel*/GetParentLevel()->IsOutlineLayout() && /*pParentLevel*/GetParentLevel()->IsSubtotalsAtTop() )
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
                bRet = true;
        }
    }
    return bRet;
}

long ScDPResultMember::GetSize(long nMeasure) const
{
    if ( !IsVisible() )
        return 0;
    const ScDPLevel*       pParentLevel = GetParentLevel();
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

bool ScDPResultMember::IsVisible() const
{
    if (!bInitialized)
        return false;

    if (!IsValid())
        return false;

    if (bHasElements)
        return true;

    //  not initialized -> shouldn't be there at all
    //  (allocated only to preserve ordering)
    const ScDPLevel* pParentLevel = GetParentLevel();

    return (pParentLevel && pParentLevel->getShowEmpty());
}

bool ScDPResultMember::IsValid() const
{
    //  non-Valid members are left out of calculation

    //  was member set no invisible at the DataPilotSource?
    const ScDPMember* pMemberDesc = GetDPMember();
    if ( pMemberDesc && !pMemberDesc->isVisible() )
        return false;

    if ( bAutoHidden )
        return false;

    return true;
}

bool ScDPResultMember::HasHiddenDetails() const
{
    // bHasHiddenDetails is set only if the "show details" flag is off,
    // and there was a child dimension to skip

    return bHasHiddenDetails;
}

long ScDPResultMember::GetSubTotalCount( long* pUserSubStart ) const
{
    if ( pUserSubStart )
        *pUserSubStart = 0;     // default

   const ScDPLevel* pParentLevel = GetParentLevel();

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

void ScDPResultMember::ProcessData( const vector< SCROW >& aChildMembers, const ScDPResultDimension* pDataDim,
                                    const vector< SCROW >& aDataMembers, const vector<ScDPValue>& aValues )
{
    SetHasElements();

    if (pChildDimension)
        pChildDimension->ProcessData( aChildMembers, pDataDim, aDataMembers, aValues );

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

    const ScDPLevel*    pParentLevel = GetParentLevel();

    for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)   // including hidden "automatic"
    {
        // #i68338# if nUserSubCount is 1 (automatic only), don't set nRowSubTotalFunc
        if ( pChildDimension && nUserSubCount > 1 )
        {
            aSubState.nRowSubTotalFunc = nUserPos;
            aSubState.eRowForce = lcl_GetForceFunc( pParentLevel, nUserPos );
        }

        pDataRoot->ProcessData( aDataMembers, aValues, aSubState );
    }
}

/**
 * Parse subtotal string and replace all occurrences of '?' with the caption
 * string.  Do ensure that escaped characters are not translated.
 */
static String lcl_parseSubtotalName(const String& rSubStr, const String& rCaption)
{
    String aNewStr;
    xub_StrLen n = rSubStr.Len();
    bool bEscaped = false;
    for (xub_StrLen i = 0; i < n; ++i)
    {
        sal_Unicode c = rSubStr.GetChar(i);
        if (!bEscaped && c == sal_Unicode('\\'))
        {
            bEscaped = true;
            continue;
        }

        if (!bEscaped && c == sal_Unicode('?'))
            aNewStr.Append(rCaption);
        else
            aNewStr.Append(c);
        bEscaped = false;
    }
    return aNewStr;
}

void ScDPResultMember::FillMemberResults(
    uno::Sequence<sheet::MemberResult>* pSequences, long& rPos, long nMeasure, bool bRoot,
    const OUString* pMemberName, const OUString* pMemberCaption )
{
    //  IsVisible() test is in ScDPResultDimension::FillMemberResults
    //  (not on data layout dimension)

    if (!pSequences->getLength())
        // empty sequence.  Bail out.
        return;

    long nSize = GetSize(nMeasure);
    sheet::MemberResult* pArray = pSequences->getArray();
    OSL_ENSURE( rPos+nSize <= pSequences->getLength(), "bumm" );

    bool bIsNumeric = false;
    OUString aName;
    if ( pMemberName )          // if pMemberName != NULL, use instead of real member name
    {
        aName = *pMemberName;
    }
    else
    {
        ScDPItemData aItemData;
        FillItemData( aItemData );
        if (aParentDimData.mpParentDim)
        {
            long nDim = aParentDimData.mpParentDim->GetDimension();
            aName = pResultData->GetSource().GetData()->GetFormattedString(nDim, aItemData);
        }
        else
        {
            long nDim = -1;
            const ScDPMember* pMem = GetDPMember();
            if (pMem)
                nDim = pMem->GetDim();
            aName = pResultData->GetSource().GetData()->GetFormattedString(nDim, aItemData);
        }

        ScDPItemData::Type eType = aItemData.GetType();
        bIsNumeric = eType == ScDPItemData::Value || ScDPItemData::GroupValue;
    }

    const ScDPDimension*        pParentDim = GetParentDim();
    if ( bIsNumeric && pParentDim && pResultData->IsNumOrDateGroup( pParentDim->GetDimension() ) )
    {
        // Numeric group dimensions use numeric entries for proper sorting,
        // but the group titles must be output as text.
        bIsNumeric = false;
    }

    OUString aCaption = aName;
    const ScDPMember* pMemberDesc = GetDPMember();
    if (pMemberDesc)
    {
        const OUString* pLayoutName = pMemberDesc->GetLayoutName();
        if (pLayoutName)
        {
            aCaption = *pLayoutName;
            bIsNumeric = false; // layout name is always non-numeric.
        }
    }

    if ( pMemberCaption )                   // use pMemberCaption if != NULL
        aCaption = *pMemberCaption;
    if (aCaption.isEmpty())
        aCaption = ScGlobal::GetRscString(STR_EMPTYDATA);

    if (bIsNumeric)
        pArray[rPos].Flags |= sheet::MemberResultFlags::NUMERIC;
    else
        pArray[rPos].Flags &= ~sheet::MemberResultFlags::NUMERIC;

    if ( nSize && !bRoot )                  // root is overwritten by first dimension
    {
        pArray[rPos].Name    = aName;
        pArray[rPos].Caption = aCaption;
        pArray[rPos].Flags  |= sheet::MemberResultFlags::HASMEMBER;

        //  set "continue" flag (removed for subtotals later)
        for (long i=1; i<nSize; i++)
            pArray[rPos+i].Flags |= sheet::MemberResultFlags::CONTINUE;
    }

    const ScDPLevel*    pParentLevel = GetParentLevel();
    long nExtraSpace = 0;
    if ( pParentLevel && pParentLevel->IsAddEmpty() )
        ++nExtraSpace;

    bool bTitleLine = false;
    if ( pParentLevel && pParentLevel->IsOutlineLayout() )
        bTitleLine = true;

    // if the subtotals are shown at the top (title row) in outline layout,
    // no extra row for the subtotals is needed
    bool bSubTotalInTitle = IsSubTotalInTitle( nMeasure );

    bool bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
    {
        if ( bTitleLine )           // in tabular layout the title is on a separate row
            ++rPos;                 // -> fill child dimension one row below

        if (bRoot)      // same sequence for root member
            pChildDimension->FillMemberResults( pSequences, rPos, nMeasure );
        else
            pChildDimension->FillMemberResults( pSequences + nMemberStep/*1*/, rPos, nMeasure );

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

                bool bTotalResult = false;
                OUString aSubStr = aCaption + " " + pResultData->GetMeasureString(nMemberMeasure, false, eForce, bTotalResult);

                if (bTotalResult)
                {
                    if (pMemberDesc)
                    {
                        // single data field layout.
                        const OUString* pSubtotalName = pParentDim->GetSubtotalName();
                        if (pSubtotalName)
                            aSubStr = lcl_parseSubtotalName(*pSubtotalName, aCaption);
                        pArray[rPos].Flags &= ~sheet::MemberResultFlags::GRANDTOTAL;
                    }
                    else
                    {
                        // root member - subtotal (grand total?) for multi-data field layout.
                        const OUString* pGrandTotalName = pResultData->GetSource().GetGrandTotalName();
                        if (pGrandTotalName)
                            aSubStr = *pGrandTotalName;
                        pArray[rPos].Flags |= sheet::MemberResultFlags::GRANDTOTAL;
                    }
                }

                pArray[rPos].Name    = aName;
                pArray[rPos].Caption = aSubStr;
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
                        pLayoutArray[rPos].Name = pResultData->GetMeasureDimensionName(nMemberMeasure);
                    }
                }

                rPos += 1;
            }
        }

        rPos += nExtraSpace;                                    // add again (subtracted above)
    }
}

void ScDPResultMember::FillDataResults(
    const ScDPResultMember* pRefMember,
    ScDPResultFilterContext& rFilterCxt, uno::Sequence<uno::Sequence<sheet::DataResult> >& rSequence,
    long nMeasure) const
{
    boost::scoped_ptr<FilterStack> pFilterStack;
    const ScDPMember* pDPMember = GetDPMember();
    if (pDPMember)
    {
        // Root result has no corresponding DP member. Only take the non-root results.
        OUString aMemStr = GetDisplayName();
        pFilterStack.reset(new FilterStack(rFilterCxt.maFilters));
        pFilterStack->pushDimValue(aMemStr);
    }

    //  IsVisible() test is in ScDPResultDimension::FillDataResults
    //  (not on data layout dimension)
    const ScDPLevel*     pParentLevel = GetParentLevel();
    long nStartRow = rFilterCxt.mnRow;

    long nExtraSpace = 0;
    if ( pParentLevel && pParentLevel->IsAddEmpty() )
        ++nExtraSpace;

    bool bTitleLine = false;
    if ( pParentLevel && pParentLevel->IsOutlineLayout() )
        bTitleLine = true;

    bool bSubTotalInTitle = IsSubTotalInTitle( nMeasure );

    bool bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
    {
        if ( bTitleLine )           // in tabular layout the title is on a separate row
            ++rFilterCxt.mnRow;                 // -> fill child dimension one row below

        long nOldRow = rFilterCxt.mnRow;
        pChildDimension->FillDataResults(pRefMember, rFilterCxt, rSequence, nMeasure);
        rFilterCxt.mnRow = nOldRow; // Revert to the original row before the call.

        rFilterCxt.mnRow += GetSize( nMeasure );

        if ( bTitleLine )           // title row is included in GetSize, so the following
            --rFilterCxt.mnRow;                 // positions are calculated with the normal values
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
            rFilterCxt.mnRow -= nSubSize * ( nUserSubCount - nUserSubStart );   // GetSize includes space for SubTotal
            rFilterCxt.mnRow -= nExtraSpace;                                    // GetSize includes the empty line
        }

        long nMoveSubTotal = 0;
        if ( bSubTotalInTitle )
        {
            nMoveSubTotal = rFilterCxt.mnRow - nStartRow;   // force to first (title) row
            rFilterCxt.mnRow = nStartRow;
        }

        if ( pDataRoot )
        {
            ScDPSubTotalState aSubState;        // initial state

            for (long nUserPos=nUserSubStart; nUserPos<nUserSubCount; nUserPos++)
            {
                if ( bHasChild && nUserSubCount > 1 )
                {
                    aSubState.nRowSubTotalFunc = nUserPos;
                    aSubState.eRowForce = lcl_GetForceFunc( /*pParentLevel*/GetParentLevel() , nUserPos );
                }

                for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
                {
                    if ( nMeasure == SC_DPMEASURE_ALL )
                        nMemberMeasure = nSubCount;
                    else if ( pResultData->GetColStartMeasure() == SC_DPMEASURE_ALL )
                        nMemberMeasure = SC_DPMEASURE_ALL;

                    OSL_ENSURE( rFilterCxt.mnRow < rSequence.getLength(), "bumm" );
                    uno::Sequence<sheet::DataResult>& rSubSeq = rSequence.getArray()[rFilterCxt.mnRow];
                    rFilterCxt.mnCol = 0;
                    if (pRefMember->IsVisible())
                        pDataRoot->FillDataRow(pRefMember, rFilterCxt, rSubSeq, nMemberMeasure, bHasChild, aSubState);

                    rFilterCxt.mnRow += 1;
                }
            }
        }
        else
            rFilterCxt.mnRow += nSubSize * ( nUserSubCount - nUserSubStart );   // empty rows occur when ShowEmpty is true

        // add extra space again if subtracted from GetSize above,
        // add to own size if no children
        rFilterCxt.mnRow += nExtraSpace;
        rFilterCxt.mnRow += nMoveSubTotal;
    }
}

void ScDPResultMember::UpdateDataResults( const ScDPResultMember* pRefMember, long nMeasure ) const
{
    //  IsVisible() test is in ScDPResultDimension::FillDataResults
    //  (not on data layout dimension)

    bool bHasChild = ( pChildDimension != NULL );

    long nUserSubCount = GetSubTotalCount();

    // process subtotals even if not shown

    // Calculate at least automatic if no subtotals are selected,
    // show only own values if there's no child dimension (innermost).
    if (!nUserSubCount || !bHasChild)
        nUserSubCount = 1;

    long nMemberMeasure = nMeasure;
    long nSubSize = pResultData->GetCountForMeasure(nMeasure);

    if (pDataRoot)
    {
        ScDPSubTotalState aSubState;        // initial state

        for (long nUserPos = 0; nUserPos < nUserSubCount; ++nUserPos)   // including hidden "automatic"
        {
            if (bHasChild && nUserSubCount > 1)
            {
                aSubState.nRowSubTotalFunc = nUserPos;
                aSubState.eRowForce = lcl_GetForceFunc(GetParentLevel(), nUserPos);
            }

            for (long nSubCount = 0; nSubCount < nSubSize; ++nSubCount)
            {
                if (nMeasure == SC_DPMEASURE_ALL)
                    nMemberMeasure = nSubCount;
                else if (pResultData->GetColStartMeasure() == SC_DPMEASURE_ALL)
                    nMemberMeasure = SC_DPMEASURE_ALL;

                pDataRoot->UpdateDataRow(pRefMember, nMemberMeasure, bHasChild, aSubState);
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
    bool bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
        pChildDimension->SortMembers( pRefMember );     // sorting is done at the dimension

    if ( IsRoot() && pDataRoot )
    {
        // use the row root member to sort columns
        // sub total count is always 1

        pDataRoot->SortMembers( pRefMember );
    }
}

void ScDPResultMember::DoAutoShow( ScDPResultMember* pRefMember )
{
    bool bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
        pChildDimension->DoAutoShow( pRefMember );     // sorting is done at the dimension

    if ( IsRoot()&& pDataRoot )
    {
        // use the row root member to sort columns
        // sub total count is always 1

        pDataRoot->DoAutoShow( pRefMember );
    }
}

void ScDPResultMember::ResetResults()
{
    if (pDataRoot)
        pDataRoot->ResetResults();

    if (pChildDimension)
        pChildDimension->ResetResults();
}

void ScDPResultMember::UpdateRunningTotals( const ScDPResultMember* pRefMember, long nMeasure,
                                            ScDPRunningTotalState& rRunning, ScDPRowTotals& rTotals ) const
{
    //  IsVisible() test is in ScDPResultDimension::FillDataResults
    //  (not on data layout dimension)

    rTotals.SetInColRoot( IsRoot() );

    bool bHasChild = ( pChildDimension != NULL );

    long nUserSubCount = GetSubTotalCount();
    //if ( nUserSubCount || !bHasChild )
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
                if ( bHasChild && nUserSubCount > 1 )
                {
                    aSubState.nRowSubTotalFunc = nUserPos;
                    aSubState.eRowForce = lcl_GetForceFunc(GetParentLevel(), nUserPos);
                }

                for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
                {
                    if ( nMeasure == SC_DPMEASURE_ALL )
                        nMemberMeasure = nSubCount;
                    else if ( pResultData->GetColStartMeasure() == SC_DPMEASURE_ALL )
                        nMemberMeasure = SC_DPMEASURE_ALL;

                    if (pRefMember->IsVisible())
                        pDataRoot->UpdateRunningTotals(
                            pRefMember, nMemberMeasure, bHasChild, aSubState, rRunning, rTotals, *this);
                }
            }
        }
    }

    if (bHasChild)  // child dimension must be processed last, so the column total is known
    {
        pChildDimension->UpdateRunningTotals( pRefMember, nMeasure, rRunning, rTotals );
    }
}

#if DEBUG_PIVOT_TABLE
void ScDPResultMember::DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const
{
    lcl_DumpRow( OUString("ScDPResultMember"), GetName(), NULL, pDoc, rPos );
    SCROW nStartRow = rPos.Row();

    if (pDataRoot)
        pDataRoot->DumpState( pRefMember, pDoc, rPos );

    if (pChildDimension)
        pChildDimension->DumpState( pRefMember, pDoc, rPos );

    lcl_Indent( pDoc, nStartRow, rPos );
}

void ScDPResultMember::Dump(int nIndent) const
{
    std::string aIndent(nIndent*2, ' ');
    std::cout << aIndent << "-- result member '" << GetName() << "'" << std::endl;

    std::cout << aIndent << " column totals" << std::endl;
    for (const ScDPAggData* p = &aColTotal; p; p = p->GetExistingChild())
        p->Dump(nIndent+1);

    if (pChildDimension)
        pChildDimension->Dump(nIndent+1);

    if (pDataRoot)
    {
        std::cout << aIndent << " data root" << std::endl;
        pDataRoot->Dump(nIndent+1);
    }
}
#endif

ScDPAggData* ScDPResultMember::GetColTotal( long nMeasure ) const
{
    return lcl_GetChildTotal( const_cast<ScDPAggData*>(&aColTotal), nMeasure );
}

void ScDPResultMember::FillVisibilityData(ScDPResultVisibilityData& rData) const
{
    if (pChildDimension)
        pChildDimension->FillVisibilityData(rData);
}

// -----------------------------------------------------------------------

ScDPDataMember::ScDPDataMember( const ScDPResultData* pData, const ScDPResultMember* pRes ) :
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

OUString ScDPDataMember::GetName() const
{
    if (pResultMember)
        return pResultMember->GetName();
    else
        return EMPTY_OUSTRING;
}

bool ScDPDataMember::IsVisible() const
{
    if (pResultMember)
        return pResultMember->IsVisible();
    else
        return false;
}

bool ScDPDataMember::IsNamedItem( SCROW nRow ) const
{
    if (pResultMember)
        return pResultMember->IsNamedItem(nRow);
    else
        return false;
}

bool ScDPDataMember::HasHiddenDetails() const
{
    if (pResultMember)
        return pResultMember->HasHiddenDetails();
    else
        return false;
}

void ScDPDataMember::InitFrom( const ScDPResultDimension* pDim )
{
    if ( !pChildDimension )
        pChildDimension = new ScDPDataDimension(pResultData);
    pChildDimension->InitFrom(pDim);
}

const long SC_SUBTOTALPOS_AUTO = -1;    // default
const long SC_SUBTOTALPOS_SKIP = -2;    // don't use

static long lcl_GetSubTotalPos( const ScDPSubTotalState& rSubState )
{
    if ( rSubState.nColSubTotalFunc >= 0 && rSubState.nRowSubTotalFunc >= 0 &&
         rSubState.nColSubTotalFunc != rSubState.nRowSubTotalFunc )
    {
        // #i68338# don't return the same index for different combinations (leading to repeated updates),
        // return a "don't use" value instead

        return SC_SUBTOTALPOS_SKIP;
    }

    long nRet = SC_SUBTOTALPOS_AUTO;
    if ( rSubState.nColSubTotalFunc >= 0 ) nRet = rSubState.nColSubTotalFunc;
    if ( rSubState.nRowSubTotalFunc >= 0 ) nRet = rSubState.nRowSubTotalFunc;
    return nRet;
}

void ScDPDataMember::UpdateValues( const vector<ScDPValue>& aValues, const ScDPSubTotalState& rSubState )
{
    //! find out how many and which subtotals are used

    ScDPAggData* pAgg = &aAggregate;

    long nSubPos = lcl_GetSubTotalPos(rSubState);
    if (nSubPos == SC_SUBTOTALPOS_SKIP)
        return;
    if (nSubPos > 0)
    {
        long nSkip = nSubPos * pResultData->GetMeasureCount();
        for (long i=0; i<nSkip; i++)
            pAgg = pAgg->GetChild();        // created if not there
    }

    size_t nCount = aValues.size();
    for (size_t nPos = 0; nPos < nCount; ++nPos)
    {
        pAgg->Update(aValues[nPos], pResultData->GetMeasureFunction(nPos), rSubState);
        pAgg = pAgg->GetChild();
    }
}

void ScDPDataMember::ProcessData( const vector< SCROW >& aChildMembers, const vector<ScDPValue>& aValues,
                                    const ScDPSubTotalState& rSubState )
{
    if ( pResultData->IsLateInit() && !pChildDimension && pResultMember && pResultMember->GetChildDimension() )
    {
        //  if this DataMember doesn't have a child dimension because the ResultMember's
        //  child dimension wasn't there yet during this DataMembers's creation,
        //  create the child dimension now
        InitFrom( pResultMember->GetChildDimension() );
    }

    long nUserSubCount = pResultMember ? pResultMember->GetSubTotalCount() : 0;

    // Calculate at least automatic if no subtotals are selected,
    // show only own values if there's no child dimension (innermost).
    if ( !nUserSubCount || !pChildDimension )
        nUserSubCount = 1;

    ScDPSubTotalState aLocalSubState = rSubState;        // keep row state, modify column
    for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)   // including hidden "automatic"
    {
        if ( pChildDimension && nUserSubCount > 1 )
        {
            const ScDPLevel* pForceLevel = pResultMember ? pResultMember->GetParentLevel() : NULL;
            aLocalSubState.nColSubTotalFunc = nUserPos;
            aLocalSubState.eColForce = lcl_GetForceFunc( pForceLevel, nUserPos );
        }

        UpdateValues( aValues, aLocalSubState );
    }

    if (pChildDimension)
        pChildDimension->ProcessData( aChildMembers, aValues, rSubState );      // with unmodified subtotal state
}

bool ScDPDataMember::HasData( long nMeasure, const ScDPSubTotalState& rSubState ) const
{
    if ( rSubState.eColForce != SUBTOTAL_FUNC_NONE && rSubState.eRowForce != SUBTOTAL_FUNC_NONE &&
                                                        rSubState.eColForce != rSubState.eRowForce )
        return false;

    //  HasData can be different between measures!

    const ScDPAggData* pAgg = GetConstAggData( nMeasure, rSubState );
    if (!pAgg)
        return false;           //! error?

    return pAgg->HasData();
}

bool ScDPDataMember::HasError( long nMeasure, const ScDPSubTotalState& rSubState ) const
{
    const ScDPAggData* pAgg = GetConstAggData( nMeasure, rSubState );
    if (!pAgg)
        return true;

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
    OSL_ENSURE( nMeasure >= 0, "GetAggData: no measure" );

    ScDPAggData* pAgg = &aAggregate;
    long nSkip = nMeasure;
    long nSubPos = lcl_GetSubTotalPos(rSubState);
    if (nSubPos == SC_SUBTOTALPOS_SKIP)
        return NULL;
    if (nSubPos > 0)
        nSkip += nSubPos * pResultData->GetMeasureCount();

    for ( long nPos=0; nPos<nSkip; nPos++ )
        pAgg = pAgg->GetChild();        //! need to create children here?

    return pAgg;
}

const ScDPAggData* ScDPDataMember::GetConstAggData( long nMeasure, const ScDPSubTotalState& rSubState ) const
{
    OSL_ENSURE( nMeasure >= 0, "GetConstAggData: no measure" );

    const ScDPAggData* pAgg = &aAggregate;
    long nSkip = nMeasure;
    long nSubPos = lcl_GetSubTotalPos(rSubState);
    if (nSubPos == SC_SUBTOTALPOS_SKIP)
        return NULL;
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

void ScDPDataMember::FillDataRow(
    const ScDPResultMember* pRefMember, ScDPResultFilterContext& rFilterCxt,
    uno::Sequence<sheet::DataResult>& rSequence, long nMeasure, bool bIsSubTotalRow,
    const ScDPSubTotalState& rSubState) const
{
    boost::scoped_ptr<FilterStack> pFilterStack;
    if (pResultMember)
    {
        // Topmost data member (pResultMember=NULL) doesn't need to be handled
        // since its immediate parent result member is linked to the same
        // dimension member.
        pFilterStack.reset(new FilterStack(rFilterCxt.maFilters));
        pFilterStack->pushDimValue(pResultMember->GetDisplayName());
    }

    OSL_ENSURE( pRefMember == pResultMember || !pResultMember, "bla" );

    long nStartCol = rFilterCxt.mnCol;

    const ScDPDataDimension* pDataChild = GetChildDimension();
    const ScDPResultDimension* pRefChild = pRefMember->GetChildDimension();

    const ScDPLevel* pRefParentLevel = const_cast<ScDPResultMember*>(pRefMember)->GetParentLevel();

    long nExtraSpace = 0;
    if ( pRefParentLevel && pRefParentLevel->IsAddEmpty() )
        ++nExtraSpace;

    bool bTitleLine = false;
    if ( pRefParentLevel && pRefParentLevel->IsOutlineLayout() )
        bTitleLine = true;

    bool bSubTotalInTitle = pRefMember->IsSubTotalInTitle( nMeasure );

    //  leave space for children even if the DataMember hasn't been initialized
    //  (pDataChild is null then, this happens when no values for it are in this row)
    bool bHasChild = ( pRefChild != NULL );

    if ( bHasChild )
    {
        if ( bTitleLine )           // in tabular layout the title is on a separate column
            ++rFilterCxt.mnCol;                 // -> fill child dimension one column below

        if ( pDataChild )
        {
            long nOldCol = rFilterCxt.mnCol;
            pDataChild->FillDataRow(pRefChild, rFilterCxt, rSequence, nMeasure, bIsSubTotalRow, rSubState);
            rFilterCxt.mnCol = nOldCol; // Revert to the old column value before the call.
        }
        rFilterCxt.mnCol += (sal_uInt16)pRefMember->GetSize( nMeasure );

        if ( bTitleLine )           // title column is included in GetSize, so the following
            --rFilterCxt.mnCol;                 // positions are calculated with the normal values
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
            rFilterCxt.mnCol -= nSubSize * ( nUserSubCount - nUserSubStart );   // GetSize includes space for SubTotal
            rFilterCxt.mnCol -= nExtraSpace;                                    // GetSize includes the empty line
        }

        long nMoveSubTotal = 0;
        if ( bSubTotalInTitle )
        {
            nMoveSubTotal = rFilterCxt.mnCol - nStartCol;   // force to first (title) column
            rFilterCxt.mnCol = nStartCol;
        }

        for (long nUserPos=nUserSubStart; nUserPos<nUserSubCount; nUserPos++)
        {
            if ( pChildDimension && nUserSubCount > 1 )
            {
                const ScDPLevel* pForceLevel = pResultMember ? pResultMember->GetParentLevel() : NULL;
                aLocalSubState.nColSubTotalFunc = nUserPos;
                aLocalSubState.eColForce = lcl_GetForceFunc( pForceLevel, nUserPos );
            }

            for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
            {
                if ( nMeasure == SC_DPMEASURE_ALL )
                    nMemberMeasure = nSubCount;

                OSL_ENSURE( rFilterCxt.mnCol < rSequence.getLength(), "bumm" );
                sheet::DataResult& rRes = rSequence.getArray()[rFilterCxt.mnCol];

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

                rFilterCxt.maFilterSet.add(rFilterCxt.maFilters, rFilterCxt.mnCol, rFilterCxt.mnRow, rRes.Value);
                rFilterCxt.mnCol += 1;
            }
        }

        // add extra space again if subtracted from GetSize above,
        // add to own size if no children
        rFilterCxt.mnCol += nExtraSpace;
        rFilterCxt.mnCol += nMoveSubTotal;
    }
}

void ScDPDataMember::UpdateDataRow(
    const ScDPResultMember* pRefMember, long nMeasure, bool bIsSubTotalRow,
    const ScDPSubTotalState& rSubState )
{
    OSL_ENSURE( pRefMember == pResultMember || !pResultMember, "bla" );

    // Calculate must be called even if not visible (for use as reference value)
    const ScDPDataDimension* pDataChild = GetChildDimension();
    const ScDPResultDimension* pRefChild = pRefMember->GetChildDimension();

    //  leave space for children even if the DataMember hasn't been initialized
    //  (pDataChild is null then, this happens when no values for it are in this row)
    bool bHasChild = ( pRefChild != NULL );

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
        if ( pChildDimension && nUserSubCount > 1 )
        {
            const ScDPLevel* pForceLevel = pResultMember ? pResultMember->GetParentLevel() : NULL;
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
    OSL_ENSURE( pRefMember == pResultMember || !pResultMember, "bla" );

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
    OSL_ENSURE( pRefMember == pResultMember || !pResultMember, "bla" );

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

void ScDPDataMember::UpdateRunningTotals(
    const ScDPResultMember* pRefMember, long nMeasure, bool bIsSubTotalRow,
    const ScDPSubTotalState& rSubState, ScDPRunningTotalState& rRunning,
    ScDPRowTotals& rTotals, const ScDPResultMember& rRowParent )
{
    OSL_ENSURE( pRefMember == pResultMember || !pResultMember, "bla" );

    const ScDPDataDimension* pDataChild = GetChildDimension();
    const ScDPResultDimension* pRefChild = pRefMember->GetChildDimension();

    bool bIsRoot = ( pResultMember == NULL || pResultMember->GetParentLevel() == NULL );

    //  leave space for children even if the DataMember hasn't been initialized
    //  (pDataChild is null then, this happens when no values for it are in this row)
    bool bHasChild = ( pRefChild != NULL );

    long nUserSubCount = pRefMember->GetSubTotalCount();
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
            if ( pChildDimension && nUserSubCount > 1 )
            {
                const ScDPLevel* pForceLevel = pResultMember ? pResultMember->GetParentLevel() : NULL;
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
                        bool bRunningTotal = ( eRefType == sheet::DataPilotFieldReferenceType::RUNNING_TOTAL );
                        bool bRelative =
                            ( aReferenceValue.ReferenceItemType != sheet::DataPilotFieldReferenceItemType::NAMED && !bRunningTotal );
                        long nRelativeDir = bRelative ?
                            ( ( aReferenceValue.ReferenceItemType == sheet::DataPilotFieldReferenceItemType::PREVIOUS ) ? -1 : 1 ) : 0;

                        const ScDPRunningTotalState::IndexArray& rColVisible = rRunning.GetColVisible();
                        const ScDPRunningTotalState::IndexArray& rColSorted = rRunning.GetColSorted();
                        const ScDPRunningTotalState::IndexArray& rRowVisible = rRunning.GetRowVisible();
                        const ScDPRunningTotalState::IndexArray& rRowSorted = rRunning.GetRowSorted();

                        String aRefFieldName = aReferenceValue.ReferenceField;

                        //! aLocalSubState?
                        sal_uInt16 nRefOrient = pResultData->GetMeasureRefOrient( nMemberMeasure );
                        bool bRefDimInCol = ( nRefOrient == sheet::DataPilotFieldOrientation_COLUMN );
                        bool bRefDimInRow = ( nRefOrient == sheet::DataPilotFieldOrientation_ROW );

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
                                long nIndex = rRowSorted[nRowPos];
                                if ( nIndex >= 0 && nIndex < pSelectDim->GetMemberCount() )
                                    pSelectDim = pSelectDim->GetMember(nIndex)->GetChildDimension();
                                else
                                    pSelectDim = NULL;
                                ++nRowPos;
                            }
                            // child dimension of innermost member?
                            if ( pSelectDim && rRowSorted[nRowPos] < 0 )
                                pSelectDim = NULL;
                        }

                        if ( bRefDimInCol )     //  look in column dimensions
                        {
                            pSelectDim = rRunning.GetColResRoot()->GetChildDimension();
                            while ( pSelectDim && pSelectDim->GetName() != aRefFieldName )
                            {
                                long nIndex = rColSorted[nColPos];
                                if ( nIndex >= 0 && nIndex < pSelectDim->GetMemberCount() )
                                    pSelectDim = pSelectDim->GetMember(nIndex)->GetChildDimension();
                                else
                                    pSelectDim = NULL;
                                ++nColPos;
                            }
                            // child dimension of innermost member?
                            if ( pSelectDim && rColSorted[nColPos] < 0 )
                                pSelectDim = NULL;
                        }

                        bool bNoDetailsInRef = false;
                        if ( pSelectDim && bRunningTotal )
                        {
                            //  Running totals:
                            //  If details are hidden for this member in the reference dimension,
                            //  don't show or sum up the value. Otherwise, for following members,
                            //  the running totals of details and subtotals wouldn't match.

                            long nMyIndex = bRefDimInCol ? rColSorted[nColPos] : rRowSorted[nRowPos];
                            if ( nMyIndex >= 0 && nMyIndex < pSelectDim->GetMemberCount() )
                            {
                                const ScDPResultMember* pMyRefMember = pSelectDim->GetMember(nMyIndex);
                                if ( pMyRefMember && pMyRefMember->HasHiddenDetails() )
                                {
                                    pSelectDim = NULL;          // don't calculate
                                    bNoDetailsInRef = true;     // show error, not empty
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

                            bool bInnerNoDetails = bRefDimInCol ? HasHiddenDetails() :
                                                 ( bRefDimInRow ? rRowParent.HasHiddenDetails() : true );
                            if ( bInnerNoDetails )
                            {
                                pSelectDim = NULL;
                                bNoDetailsInRef = true;         // show error, not empty
                            }
                        }

                        if ( !bRefDimInCol && !bRefDimInRow )   // invalid dimension specified
                            bNoDetailsInRef = true;             // pSelectDim is then already NULL

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
                                    const long* pRowSorted = &rRowSorted[0];
                                    const long* pColSorted = &rColSorted[0];
                                    pRowSorted += nRowPos + 1; // including the reference dimension
                                    pSelectMember = pSelectDim->GetRowReferenceMember(
                                        NULL, NULL, pRowSorted, pColSorted);
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
                                        pAggData->SetEmpty(false);              // always display
                                    }
                                }
                                else
                                    pAggData->SetError();
                            }
                            else if (bNoDetailsInRef)
                                pAggData->SetError();
                            else
                                pAggData->SetEmpty(true);                       // empty (dim set to 0 above)
                        }
                        else
                        {
                            // difference/percentage -> find specified member

                            if ( pSelectDim )
                            {
                                OUString aRefItemName = aReferenceValue.ReferenceItemName;
                                ScDPRelativePos aRefItemPos( 0, nRelativeDir );     // nBasePos is modified later

                                const OUString* pRefName = NULL;
                                const ScDPRelativePos* pRefPos = NULL;
                                if ( bRelative )
                                    pRefPos = &aRefItemPos;
                                else
                                    pRefName = &aRefItemName;

                                ScDPDataMember* pSelectMember;
                                if ( bRefDimInCol )
                                {
                                    aRefItemPos.nBasePos = rColVisible[nColPos];    // without sort order applied
                                    pSelectMember = ScDPResultDimension::GetColReferenceMember( pRefPos, pRefName,
                                                                    nColPos, rRunning );
                                }
                                else
                                {
                                    aRefItemPos.nBasePos = rRowVisible[nRowPos];    // without sort order applied
                                    const long* pRowSorted = &rRowSorted[0];
                                    const long* pColSorted = &rColSorted[0];
                                    pRowSorted += nRowPos + 1; // including the reference dimension
                                    pSelectMember = pSelectDim->GetRowReferenceMember(
                                        pRefPos, pRefName, pRowSorted, pColSorted);
                                }

                                // difference or perc.difference is empty for the reference item itself
                                if ( pSelectMember == this &&
                                     eRefType != sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE )
                                {
                                    pAggData->SetEmpty(true);
                                }
                                else if ( pSelectMember )
                                {
                                    const ScDPAggData* pOtherAggData = pSelectMember->
                                                        GetConstAggData( nMemberMeasure, aLocalSubState );
                                    OSL_ENSURE( pOtherAggData, "no agg data" );
                                    if ( pOtherAggData )
                                    {
                                        // Reference member may be visited before or after this one,
                                        // so the auxiliary value is used for the original result.

                                        double fOtherResult = pOtherAggData->GetAuxiliary();
                                        double fThisResult = pAggData->GetResult();
                                        bool bError = false;
                                        switch ( eRefType )
                                        {
                                            case sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE:
                                                fThisResult = fThisResult - fOtherResult;
                                                break;
                                            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE:
                                                if ( fOtherResult == 0.0 )
                                                    bError = true;
                                                else
                                                    fThisResult = fThisResult / fOtherResult;
                                                break;
                                            case sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE:
                                                if ( fOtherResult == 0.0 )
                                                    bError = true;
                                                else
                                                    fThisResult = ( fThisResult - fOtherResult ) / fOtherResult;
                                                break;
                                            default:
                                                OSL_FAIL("invalid calculation type");
                                        }
                                        if ( bError )
                                        {
                                            pAggData->SetError();
                                        }
                                        else
                                        {
                                            pAggData->SetResult(fThisResult);
                                            pAggData->SetEmpty(false);              // always display
                                        }
                                        //! errors in data?
                                    }
                                }
                                else if (bRelative && !bNoDetailsInRef)
                                    pAggData->SetEmpty(true);                   // empty
                                else
                                    pAggData->SetError();                       // error
                            }
                            else if (bNoDetailsInRef)
                                pAggData->SetError();                           // error
                            else
                                pAggData->SetEmpty(true);                       // empty
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

#if DEBUG_PIVOT_TABLE
void ScDPDataMember::DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const
{
    lcl_DumpRow( OUString("ScDPDataMember"), GetName(), &aAggregate, pDoc, rPos );
    SCROW nStartRow = rPos.Row();

    const ScDPDataDimension* pDataChild = GetChildDimension();
    const ScDPResultDimension* pRefChild = pRefMember->GetChildDimension();
    if ( pDataChild && pRefChild )
        pDataChild->DumpState( pRefChild, pDoc, rPos );

    lcl_Indent( pDoc, nStartRow, rPos );
}

void ScDPDataMember::Dump(int nIndent) const
{
    std::string aIndent(nIndent*2, ' ');
    std::cout << aIndent << "-- data member '"
        << (pResultMember ? pResultMember->GetName() : OUString()) << "'" << std::endl;
    for (const ScDPAggData* pAgg = &aAggregate; pAgg; pAgg = pAgg->GetExistingChild())
        pAgg->Dump(nIndent+1);

    if (pChildDimension)
        pChildDimension->Dump(nIndent+1);
}
#endif

// -----------------------------------------------------------------------

//  Helper class to select the members to include in
//  ScDPResultDimension::InitFrom or LateInitFrom if groups are used

class ScDPGroupCompare
{
private:
    const ScDPResultData* pResultData;
    const ScDPInitState& rInitState;
    long                 nDimSource;
    bool                 bIncludeAll;
    bool                 bIsBase;
    long                 nGroupBase;
public:
            ScDPGroupCompare( const ScDPResultData* pData, const ScDPInitState& rState, long nDimension );
            ~ScDPGroupCompare() {}

    bool    IsIncluded( const ScDPMember& rMember )     { return bIncludeAll || TestIncluded( rMember ); }
    bool    TestIncluded( const ScDPMember& rMember );
};

ScDPGroupCompare::ScDPGroupCompare( const ScDPResultData* pData, const ScDPInitState& rState, long nDimension ) :
    pResultData( pData ),
    rInitState( rState ),
    nDimSource( nDimension )
{
    bIsBase = pResultData->IsBaseForGroup( nDimSource );
    nGroupBase = pResultData->GetGroupBase( nDimSource );      //! get together in one call?

    // if bIncludeAll is set, TestIncluded doesn't need to be called
    bIncludeAll = !( bIsBase || nGroupBase >= 0 );
}

bool ScDPGroupCompare::TestIncluded( const ScDPMember& rMember )
{
    bool bInclude = true;
    if ( bIsBase )
    {
        // need to check all previous groups
        //! get array of groups (or indexes) before loop?
        ScDPItemData aMemberData;
        rMember.FillItemData( aMemberData );

        const std::vector<ScDPInitState::Member>& rMemStates = rInitState.GetMembers();
        std::vector<ScDPInitState::Member>::const_iterator it = rMemStates.begin(), itEnd = rMemStates.end();
        for (; it != itEnd && bInclude; ++it)
        {
            if (pResultData->GetGroupBase(it->mnSrcIndex) == nDimSource)
            {
                bInclude = pResultData->IsInGroup(
                    it->mnNameIndex, it->mnSrcIndex, aMemberData, nDimSource);
            }
        }
    }
    else if ( nGroupBase >= 0 )
    {
        // base isn't used in preceding fields
        // -> look for other groups using the same base

        //! get array of groups (or indexes) before loop?
        ScDPItemData aMemberData;
        rMember.FillItemData( aMemberData );
        const std::vector<ScDPInitState::Member>& rMemStates = rInitState.GetMembers();
        std::vector<ScDPInitState::Member>::const_iterator it = rMemStates.begin(), itEnd = rMemStates.end();
        for (; it != itEnd && bInclude; ++it)
        {
            if (pResultData->GetGroupBase(it->mnSrcIndex) == nGroupBase)
            {
                // same base (hierarchy between the two groups is irrelevant)
                bInclude = pResultData->HasCommonElement(
                    it->mnNameIndex, it->mnSrcIndex, aMemberData, nDimSource);
            }

        }
    }

    return bInclude;
}

// -----------------------------------------------------------------------

ScDPResultDimension::ScDPResultDimension( const ScDPResultData* pData ) :
    pResultData( pData ),
    nSortMeasure( 0 ),
    bIsDataLayout( false ),
    bSortByData( false ),
    bSortAscending( false ),
    bAutoShow( false ),
    bAutoTopItems( false ),
    bInitialized( false ),
    nAutoMeasure( 0 ),
    nAutoCount( 0 )
{
}

ScDPResultDimension::~ScDPResultDimension()
{
    for( int i = maMemberArray.size () ; i-- > 0 ; )
        delete maMemberArray[i];
}

ScDPResultMember *ScDPResultDimension::FindMember(  SCROW  iData ) const
{
    if( bIsDataLayout )
        return maMemberArray[0];

    MemberHash::const_iterator aRes = maMemberHash.find( iData );
    if( aRes != maMemberHash.end()) {
           if ( aRes->second->IsNamedItem( iData ) )
            return aRes->second;
        OSL_FAIL("problem!  hash result is not the same as IsNamedItem");
    }

    unsigned int i;
    unsigned int nCount = maMemberArray.size();
    ScDPResultMember* pResultMember;
    for( i = 0; i < nCount ; i++ )
    {
        pResultMember = maMemberArray[i];
        if ( pResultMember->IsNamedItem( iData ) )
            return pResultMember;
    }
    return NULL;
}

void ScDPResultDimension::InitFrom(
    const vector<ScDPDimension*>& ppDim, const vector<ScDPLevel*>& ppLev,
    size_t nPos, ScDPInitState& rInitState,  bool bInitChild )
{
    if (nPos >= ppDim.size() || nPos >= ppLev.size())
    {
        bInitialized = true;
        return;
    }

    ScDPDimension* pThisDim = ppDim[nPos];
    ScDPLevel* pThisLevel = ppLev[nPos];

    if (!pThisDim || !pThisLevel)
    {
        bInitialized = true;
        return;
    }

    bIsDataLayout = pThisDim->getIsDataLayoutDimension();   // member
    aDimensionName = pThisDim->getName();                   // member

    // Check the autoshow setting.  If it's enabled, store the settings.
    const sheet::DataPilotFieldAutoShowInfo& rAutoInfo = pThisLevel->GetAutoShow();
    if ( rAutoInfo.IsEnabled )
    {
        bAutoShow     = true;
        bAutoTopItems = ( rAutoInfo.ShowItemsMode == sheet::DataPilotFieldShowItemsMode::FROM_TOP );
        nAutoMeasure  = pThisLevel->GetAutoMeasure();
        nAutoCount    = rAutoInfo.ItemCount;
    }

    // Check the sort info, and store the settings if appropriate.
    const sheet::DataPilotFieldSortInfo& rSortInfo = pThisLevel->GetSortInfo();
    if ( rSortInfo.Mode == sheet::DataPilotFieldSortMode::DATA )
    {
        bSortByData = true;
        bSortAscending = rSortInfo.IsAscending;
        nSortMeasure = pThisLevel->GetSortMeasure();
    }

    // global order is used to initialize aMembers, so it doesn't have to be looked at later
    const ScMemberSortOrder& rGlobalOrder = pThisLevel->GetGlobalOrder();

    long nDimSource = pThisDim->GetDimension();     //! check GetSourceDim?
    ScDPGroupCompare aCompare( pResultData, rInitState, nDimSource );

    // Now, go through all members and initialize them.
    ScDPMembers* pMembers = pThisLevel->GetMembersObject();
    long nMembCount = pMembers->getCount();
    for ( long i=0; i<nMembCount; i++ )
    {
        long nSorted = rGlobalOrder.empty() ? i : rGlobalOrder[i];

        ScDPMember* pMember = pMembers->getByIndex(nSorted);
        if ( aCompare.IsIncluded( *pMember ) )
        {
            ScDPParentDimData aData( i, pThisDim, pThisLevel, pMember);
            ScDPResultMember* pNew = AddMember( aData );

            rInitState.AddMember(nDimSource, pNew->GetDataId());
            pNew->InitFrom( ppDim, ppLev, nPos+1, rInitState, bInitChild  );
            rInitState.RemoveMember();
        }
    }
    bInitialized = true;
}

void ScDPResultDimension::LateInitFrom(
    LateInitParams& rParams, const vector<SCROW>& pItemData, size_t nPos, ScDPInitState& rInitState)
{
    if ( rParams.IsEnd( nPos ) )
        return;
    OSL_ENSURE( nPos <= pItemData.size(), OString::number(pItemData.size()).getStr() );
    ScDPDimension* pThisDim = rParams.GetDim( nPos );
    ScDPLevel* pThisLevel = rParams.GetLevel( nPos );
    SCROW rThisData = pItemData[nPos];

    if (!pThisDim || !pThisLevel)
        return;

    long nDimSource = pThisDim->GetDimension();     //! check GetSourceDim?

    bool bShowEmpty = pThisLevel->getShowEmpty();

    if ( !bInitialized )
    { // init some values
        //  create all members at the first call (preserve order)
        bIsDataLayout = pThisDim->getIsDataLayoutDimension();
        aDimensionName = pThisDim->getName();

        const sheet::DataPilotFieldAutoShowInfo& rAutoInfo = pThisLevel->GetAutoShow();
        if ( rAutoInfo.IsEnabled )
        {
            bAutoShow     = true;
            bAutoTopItems = ( rAutoInfo.ShowItemsMode == sheet::DataPilotFieldShowItemsMode::FROM_TOP );
            nAutoMeasure  = pThisLevel->GetAutoMeasure();
            nAutoCount    = rAutoInfo.ItemCount;
        }

        const sheet::DataPilotFieldSortInfo& rSortInfo = pThisLevel->GetSortInfo();
        if ( rSortInfo.Mode == sheet::DataPilotFieldSortMode::DATA )
        {
            bSortByData = true;
            bSortAscending = rSortInfo.IsAscending;
            nSortMeasure = pThisLevel->GetSortMeasure();
        }
    }

    bool bLateInitAllMembers=  bIsDataLayout || rParams.GetInitAllChild() || bShowEmpty;

    if ( !bLateInitAllMembers )
    {
        ResultMembers* pMembers = pResultData->GetDimResultMembers(nDimSource, pThisDim, pThisLevel);
        bLateInitAllMembers = pMembers->IsHasHideDetailsMembers();
#if OSL_DEBUG_LEVEL > 1
        OSL_TRACE( "%s", aDimensionName.getStr() );
        if ( pMembers->IsHasHideDetailsMembers() )
            OSL_TRACE( "HasHideDetailsMembers" );
#endif
        pMembers->SetHasHideDetailsMembers( false );
    }

    bool bNewAllMembers = (!rParams.IsRow()) ||  nPos == 0 || bLateInitAllMembers;

    if (bNewAllMembers )
    {
      // global order is used to initialize aMembers, so it doesn't have to be looked at later
        if ( !bInitialized )
        { //init all members
            const ScMemberSortOrder& rGlobalOrder = pThisLevel->GetGlobalOrder();

            ScDPGroupCompare aCompare( pResultData, rInitState, nDimSource );
            ScDPMembers* pMembers = pThisLevel->GetMembersObject();
            long nMembCount = pMembers->getCount();
            for ( long i=0; i<nMembCount; i++ )
            {
                long nSorted = rGlobalOrder.empty() ? i : rGlobalOrder[i];

                ScDPMember* pMember = pMembers->getByIndex(nSorted);
                if ( aCompare.IsIncluded( *pMember ) )
                { // add all members
                    ScDPParentDimData aData( i, pThisDim, pThisLevel, pMember );
                    AddMember( aData );
                }
            }
            bInitialized = true;    // don't call again, even if no members were included
        }
        //  initialize only specific member (or all if "show empty" flag is set)
        if ( bLateInitAllMembers  )
        {
            long nCount = maMemberArray.size();
            for (long i=0; i<nCount; i++)
            {
                ScDPResultMember* pResultMember = maMemberArray[i];

                // check show empty
                bool bAllChildren = false;
                if( bShowEmpty )
                {
                    if (  pResultMember->IsNamedItem( rThisData ) )
                        bAllChildren = false;
                    else
                        bAllChildren = true;
                }
                rParams.SetInitAllChildren( bAllChildren );
                rInitState.AddMember( nDimSource,  pResultMember->GetDataId() );
                pResultMember->LateInitFrom( rParams, pItemData, nPos+1, rInitState );
                rInitState.RemoveMember();
            }
        }
        else
        {
            ScDPResultMember* pResultMember = FindMember( rThisData );
            if( NULL != pResultMember )
            {
                rInitState.AddMember( nDimSource,  pResultMember->GetDataId() );
                pResultMember->LateInitFrom( rParams, pItemData, nPos+1, rInitState );
                rInitState.RemoveMember();
            }
        }
    }
    else
        InitWithMembers( rParams, pItemData, nPos, rInitState );
}

long ScDPResultDimension::GetSize(long nMeasure) const
{
    long nTotal = 0;
    long nMemberCount = maMemberArray.size();
    if (bIsDataLayout)
    {
        OSL_ENSURE(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                    "DataLayout dimension twice?");
        //  repeat first member...
        nTotal = nMemberCount * maMemberArray[0]->GetSize(0);   // all measures have equal size
    }
    else
    {
        //  add all members
        for (long nMem=0; nMem<nMemberCount; nMem++)
            nTotal += maMemberArray[nMem]->GetSize(nMeasure);
    }
    return nTotal;
}

bool ScDPResultDimension::IsValidEntry( const vector< SCROW >& aMembers ) const
{
    if (aMembers.empty())
        return false;

    const ScDPResultMember* pMember = FindMember( aMembers[0] );
    if ( NULL != pMember )
        return pMember->IsValidEntry( aMembers );
#if OSL_DEBUG_LEVEL > 1
    OStringBuffer strTemp(RTL_CONSTASCII_STRINGPARAM(
        "IsValidEntry: Member not found, DimName = "));
    strTemp.append(OUStringToOString(GetName(), RTL_TEXTENCODING_UTF8));
    OSL_TRACE("%s", strTemp.getStr());
#endif
    return false;
}

void ScDPResultDimension::ProcessData( const vector< SCROW >& aMembers,
                                       const ScDPResultDimension* pDataDim,
                                       const vector< SCROW >& aDataMembers,
                                       const vector<ScDPValue>& aValues ) const
{
    if (aMembers.empty())
        return;

    ScDPResultMember* pMember = FindMember( aMembers[0] );
    if ( NULL != pMember )
    {
        vector<SCROW> aChildMembers;
        if (aMembers.size() > 1)
        {
            vector<SCROW>::const_iterator itr = aMembers.begin();
            aChildMembers.insert(aChildMembers.begin(), ++itr, aMembers.end());
        }
        pMember->ProcessData( aChildMembers, pDataDim, aDataMembers, aValues );
        return;
    }

    OSL_FAIL("ProcessData: Member not found");
}

void ScDPResultDimension::FillMemberResults( uno::Sequence<sheet::MemberResult>* pSequences,
                                                long nStart, long nMeasure )
{
    long nPos = nStart;
    long nCount = maMemberArray.size();

    for (long i=0; i<nCount; i++)
    {
        long nSorted = aMemberOrder.empty() ? i : aMemberOrder[i];

        ScDPResultMember* pMember = maMemberArray[nSorted];
        //  in data layout dimension, use first member with different measures/names
        if ( bIsDataLayout )
        {
            bool bTotalResult = false;
            OUString aMbrName = pResultData->GetMeasureDimensionName( nSorted );
            OUString aMbrCapt = pResultData->GetMeasureString( nSorted, false, SUBTOTAL_FUNC_NONE, bTotalResult );
            maMemberArray[0]->FillMemberResults( pSequences, nPos, nSorted, false, &aMbrName, &aMbrCapt );
        }
        else if ( pMember->IsVisible() )
        {
            pMember->FillMemberResults( pSequences, nPos, nMeasure, false, NULL, NULL );
        }
        // nPos is modified
    }
}

void ScDPResultDimension::FillDataResults(
    const ScDPResultMember* pRefMember, ScDPResultFilterContext& rFilterCxt,
    uno::Sequence< uno::Sequence<sheet::DataResult> >& rSequence, long nMeasure) const
{
    FilterStack aFilterStack(rFilterCxt.maFilters);
    aFilterStack.pushDimName(GetName(), bIsDataLayout);

    long nMemberMeasure = nMeasure;
    long nCount = maMemberArray.size();
    for (long i=0; i<nCount; i++)
    {
        long nSorted = aMemberOrder.empty() ? i : aMemberOrder[i];

        const ScDPResultMember* pMember;
        if (bIsDataLayout)
        {
            OSL_ENSURE(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            pMember = maMemberArray[0];
            nMemberMeasure = nSorted;
        }
        else
            pMember = maMemberArray[nSorted];

        if ( pMember->IsVisible() )
            pMember->FillDataResults(pRefMember, rFilterCxt, rSequence, nMemberMeasure);
    }
}

void ScDPResultDimension::UpdateDataResults( const ScDPResultMember* pRefMember, long nMeasure ) const
{
    long nMemberMeasure = nMeasure;
    long nCount = maMemberArray.size();
    for (long i=0; i<nCount; i++)
    {
        const ScDPResultMember* pMember;
        if (bIsDataLayout)
        {
            OSL_ENSURE(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            pMember = maMemberArray[0];
            nMemberMeasure = i;
        }
        else
            pMember = maMemberArray[i];

        if ( pMember->IsVisible() )
            pMember->UpdateDataResults( pRefMember, nMemberMeasure );
    }
}

void ScDPResultDimension::SortMembers( ScDPResultMember* pRefMember )
{
    long nCount = maMemberArray.size();

    if ( bSortByData )
    {
        // sort members

        OSL_ENSURE( aMemberOrder.empty(), "sort twice?" );
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
        ScDPResultMember* pMember = maMemberArray[i];
        if ( pMember->IsVisible() )
            pMember->SortMembers( pRefMember );
    }
}

void ScDPResultDimension::DoAutoShow( ScDPResultMember* pRefMember )
{
    long nCount = maMemberArray.size();

    // handle children first, before changing the visible state

    // for data layout, call only once - sorting measure is always taken from settings
    long nLoopCount = bIsDataLayout ? 1 : nCount;
    for (long i=0; i<nLoopCount; i++)
    {
        ScDPResultMember* pMember = maMemberArray[i];
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
        const ScDPResultMember* pMember1 = maMemberArray[aAutoOrder[nIncluded - 1]];
        const ScDPDataMember* pDataMember1 = pMember1->IsVisible() ? pMember1->GetDataRoot() : NULL;
        bool bContinue = true;
        while ( bContinue )
        {
            bContinue = false;
            if ( nIncluded < nCount )
            {
                const ScDPResultMember* pMember2 = maMemberArray[aAutoOrder[nIncluded]];
                const ScDPDataMember* pDataMember2 = pMember2->IsVisible() ? pMember2->GetDataRoot() : NULL;

                if ( lcl_IsEqual( pDataMember1, pDataMember2, nAutoMeasure ) )
                {
                    ++nIncluded;                // include more members if values are equal
                    bContinue = true;
                }
            }
        }

        // hide the remaining members

        for (nPos = nIncluded; nPos < nCount; nPos++)
        {
            ScDPResultMember* pMember = maMemberArray[aAutoOrder[nPos]];
            pMember->SetAutoHidden();
        }
    }
}

void ScDPResultDimension::ResetResults()
{
    long nCount = maMemberArray.size();
    for (long i=0; i<nCount; i++)
    {
        // sort order doesn't matter
        ScDPResultMember* pMember = maMemberArray[bIsDataLayout ? 0 : i];
        pMember->ResetResults();
    }
}

long ScDPResultDimension::GetSortedIndex( long nUnsorted ) const
{
    return aMemberOrder.empty() ? nUnsorted : aMemberOrder[nUnsorted];
}

void ScDPResultDimension::UpdateRunningTotals( const ScDPResultMember* pRefMember, long nMeasure,
                                                ScDPRunningTotalState& rRunning, ScDPRowTotals& rTotals ) const
{
    const ScDPResultMember* pMember;
    long nMemberMeasure = nMeasure;
    long nCount = maMemberArray.size();
    for (long i=0; i<nCount; i++)
    {
        long nSorted = aMemberOrder.empty() ? i : aMemberOrder[i];

        if (bIsDataLayout)
        {
            OSL_ENSURE(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            pMember = maMemberArray[0];
            nMemberMeasure = nSorted;
        }
        else
            pMember = maMemberArray[nSorted];

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

ScDPDataMember* ScDPResultDimension::GetRowReferenceMember(
    const ScDPRelativePos* pRelativePos, const OUString* pName,
    const long* pRowIndexes, const long* pColIndexes ) const
{
    // get named, previous/next, or first member of this dimension (first existing if pRelativePos and pName are NULL)

    OSL_ENSURE( pRelativePos == NULL || pName == NULL, "can't use position and name" );

    ScDPDataMember* pColMember = NULL;

    bool bFirstExisting = ( pRelativePos == NULL && pName == NULL );
    long nMemberCount = maMemberArray.size();
    long nMemberIndex = 0;      // unsorted
    long nDirection = 1;        // forward if no relative position is used
    if ( pRelativePos )
    {
        nDirection = pRelativePos->nDirection;
        nMemberIndex = pRelativePos->nBasePos + nDirection;     // bounds are handled below

        OSL_ENSURE( nDirection == 1 || nDirection == -1, "Direction must be 1 or -1" );
    }
    else if ( pName )
    {
        // search for named member

        const ScDPResultMember* pRowMember = maMemberArray[GetSortedIndex(nMemberIndex)];

        //! use ScDPItemData, as in ScDPDimension::IsValidPage?
        while ( pRowMember && pRowMember->GetName() != *pName )
        {
            ++nMemberIndex;
            if ( nMemberIndex < nMemberCount )
                pRowMember = maMemberArray[GetSortedIndex(nMemberIndex)];
            else
                pRowMember = NULL;
        }
    }

    bool bContinue = true;
    while ( bContinue && nMemberIndex >= 0 && nMemberIndex < nMemberCount )
    {
        const ScDPResultMember* pRowMember = maMemberArray[GetSortedIndex(nMemberIndex)];

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
                ScDPDataDimension* pColChild = pColMember->GetChildDimension();
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

ScDPDataMember* ScDPResultDimension::GetColReferenceMember(
    const ScDPRelativePos* pRelativePos, const OUString* pName,
    long nRefDimPos, const ScDPRunningTotalState& rRunning )
{
    OSL_ENSURE( pRelativePos == NULL || pName == NULL, "can't use position and name" );

    const long* pColIndexes = &rRunning.GetColSorted()[0];
    const long* pRowIndexes = &rRunning.GetRowSorted()[0];

    // get own row member using all indexes

    const ScDPResultMember* pRowMember = rRunning.GetRowResRoot();
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
            ScDPDataDimension* pColChild = pColMember->GetChildDimension();
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
        ScDPDataDimension* pReferenceDim = pColMember->GetChildDimension();
        if ( pReferenceDim )
        {
            long nReferenceCount = pReferenceDim->GetMemberCount();

            bool bFirstExisting = ( pRelativePos == NULL && pName == NULL );
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

            bool bContinue = true;
            while ( bContinue && nMemberIndex >= 0 && nMemberIndex < nReferenceCount )
            {
                pColMember = pReferenceDim->GetMember( pReferenceDim->GetSortedIndex( nMemberIndex ) );

                // get column members below the reference field

                const long* pNextColIndex = pColIndexes + nRefDimPos + 1;
                while ( *pNextColIndex >= 0 && pColMember )
                {
                    ScDPDataDimension* pColChild = pColMember->GetChildDimension();
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

#if DEBUG_PIVOT_TABLE
void ScDPResultDimension::DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const
{
    OUString aDimName = bIsDataLayout ? OUString("(data layout)") : OUString(GetName());
    lcl_DumpRow( OUString("ScDPResultDimension"), aDimName, NULL, pDoc, rPos );

    SCROW nStartRow = rPos.Row();

    long nCount = bIsDataLayout ? 1 : maMemberArray.size();
    for (long i=0; i<nCount; i++)
    {
        const ScDPResultMember* pMember = maMemberArray[i];
        pMember->DumpState( pRefMember, pDoc, rPos );
    }

    lcl_Indent( pDoc, nStartRow, rPos );
}

void ScDPResultDimension::Dump(int nIndent) const
{
    std::string aIndent(nIndent*2, ' ');
    std::cout << aIndent << "-- dimension '" << GetName() << "'" << std::endl;
    MemberArray::const_iterator it = maMemberArray.begin(), itEnd = maMemberArray.end();
    for (; it != itEnd; ++it)
    {
        const ScDPResultMember* p = *it;
        p->Dump(nIndent+1);
    }
}
#endif

long ScDPResultDimension::GetMemberCount() const
{
    return maMemberArray.size();
}

const ScDPResultMember* ScDPResultDimension::GetMember(long n) const
{
    return maMemberArray[n];
}
ScDPResultMember* ScDPResultDimension::GetMember(long n)
{
    return maMemberArray[n];
}

ScDPResultDimension* ScDPResultDimension::GetFirstChildDimension() const
{
    if ( maMemberArray.size() > 0 )
        return maMemberArray[0]->GetChildDimension();
    else
        return NULL;
}

void ScDPResultDimension::FillVisibilityData(ScDPResultVisibilityData& rData) const
{
    if (IsDataLayout())
        return;

    MemberArray::const_iterator itr = maMemberArray.begin(), itrEnd = maMemberArray.end();

    for (;itr != itrEnd; ++itr)
    {
        ScDPResultMember* pMember = *itr;
        if (pMember->IsValid())
        {
            ScDPItemData aItem;
            pMember->FillItemData(aItem);
            rData.addVisibleMember(GetName(), aItem);
            pMember->FillVisibilityData(rData);
        }
    }
}

// -----------------------------------------------------------------------

ScDPDataDimension::ScDPDataDimension( const ScDPResultData* pData ) :
    pResultData( pData ),
    pResultDimension( NULL ),
    bIsDataLayout( false )
{
}

ScDPDataDimension::~ScDPDataDimension()
{
    std::for_each(maMembers.begin(), maMembers.end(), ScDeleteObjectByPtr<ScDPDataMember>());
}

void ScDPDataDimension::InitFrom( const ScDPResultDimension* pDim )
{
    if (!pDim)
        return;

    pResultDimension = pDim;
    bIsDataLayout = pDim->IsDataLayout();

    // Go through all result members under the given result dimension, and
    // create a new data member instance for each result member.
    long nCount = pDim->GetMemberCount();
    for (long i=0; i<nCount; i++)
    {
        const ScDPResultMember* pResMem = pDim->GetMember(i);

        ScDPDataMember* pNew = new ScDPDataMember( pResultData, pResMem );
        maMembers.push_back( pNew);

        if ( !pResultData->IsLateInit() )
        {
            //  with LateInit, pResMem hasn't necessarily been initialized yet,
            //  so InitFrom for the new result member is called from its ProcessData method

            const ScDPResultDimension* pChildDim = pResMem->GetChildDimension();
            if ( pChildDim )
                pNew->InitFrom( pChildDim );
        }
    }
}

void ScDPDataDimension::ProcessData( const vector< SCROW >& aDataMembers, const vector<ScDPValue>& aValues,
                                     const ScDPSubTotalState& rSubState )
{
    // the ScDPItemData array must contain enough entries for all dimensions - this isn't checked

    long nCount = maMembers.size();
    for (long i=0; i<nCount; i++)
    {
        ScDPDataMember* pMember = maMembers[(sal_uInt16)i];

        // always first member for data layout dim
        if ( bIsDataLayout || ( !aDataMembers.empty() && pMember->IsNamedItem(aDataMembers[0]) ) )
        {
            vector<SCROW> aChildDataMembers;
            if (aDataMembers.size() > 1)
            {
                vector<SCROW>::const_iterator itr = aDataMembers.begin();
                aChildDataMembers.insert(aChildDataMembers.begin(), ++itr, aDataMembers.end());
            }
            pMember->ProcessData( aChildDataMembers, aValues, rSubState );
            return;
        }
    }

    OSL_FAIL("ProcessData: Member not found");
}

void ScDPDataDimension::FillDataRow(
    const ScDPResultDimension* pRefDim, ScDPResultFilterContext& rFilterCxt,
    uno::Sequence<sheet::DataResult>& rSequence, long nMeasure, bool bIsSubTotalRow,
    const ScDPSubTotalState& rSubState) const
{
    OUString aDimName;
    bool bDataLayout = false;
    if (pResultDimension)
    {
        aDimName = pResultDimension->GetName();
        bDataLayout = pResultDimension->IsDataLayout();
    }

    FilterStack aFilterStack(rFilterCxt.maFilters);
    aFilterStack.pushDimName(aDimName, bDataLayout);

    OSL_ENSURE( pRefDim && static_cast<size_t>(pRefDim->GetMemberCount()) == maMembers.size(), "dimensions don't match" );
    OSL_ENSURE( pRefDim == pResultDimension, "wrong dim" );

    const ScMemberSortOrder& rMemberOrder = pRefDim->GetMemberOrder();

    long nMemberMeasure = nMeasure;
    long nCount = maMembers.size();
    for (long i=0; i<nCount; i++)
    {
        long nSorted = rMemberOrder.empty() ? i : rMemberOrder[i];

        long nMemberPos = nSorted;
        if (bIsDataLayout)
        {
            OSL_ENSURE(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            nMemberPos = 0;
            nMemberMeasure = nSorted;
        }

        const ScDPResultMember* pRefMember = pRefDim->GetMember(nMemberPos);
        if ( pRefMember->IsVisible() )  //! here or in ScDPDataMember::FillDataRow ???
        {
            const ScDPDataMember* pDataMember = maMembers[(sal_uInt16)nMemberPos];
            pDataMember->FillDataRow(pRefMember, rFilterCxt, rSequence, nMemberMeasure, bIsSubTotalRow, rSubState);
        }
    }
}

void ScDPDataDimension::UpdateDataRow( const ScDPResultDimension* pRefDim,
                                    long nMeasure, bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState ) const
{
    OSL_ENSURE( pRefDim && static_cast<size_t>(pRefDim->GetMemberCount()) == maMembers.size(), "dimensions don't match" );
    OSL_ENSURE( pRefDim == pResultDimension, "wrong dim" );

    long nMemberMeasure = nMeasure;
    long nCount = maMembers.size();
    for (long i=0; i<nCount; i++)
    {
        long nMemberPos = i;
        if (bIsDataLayout)
        {
            OSL_ENSURE(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            nMemberPos = 0;
            nMemberMeasure = i;
        }

        // Calculate must be called even if the member is not visible (for use as reference value)
        const ScDPResultMember* pRefMember = pRefDim->GetMember(nMemberPos);
        ScDPDataMember* pDataMember = maMembers[(sal_uInt16)nMemberPos];
        pDataMember->UpdateDataRow( pRefMember, nMemberMeasure, bIsSubTotalRow, rSubState );
    }
}

void ScDPDataDimension::SortMembers( ScDPResultDimension* pRefDim )
{
    long nCount = maMembers.size();

    if ( pRefDim->IsSortByData() )
    {
        // sort members

        ScMemberSortOrder& rMemberOrder = pRefDim->GetMemberOrder();
        OSL_ENSURE( rMemberOrder.empty(), "sort twice?" );
        rMemberOrder.resize( nCount );
        for (long nPos=0; nPos<nCount; nPos++)
            rMemberOrder[nPos] = nPos;

        ScDPColMembersOrder aComp( *this, pRefDim->GetSortMeasure(), pRefDim->IsSortAscending() );
        ::std::sort( rMemberOrder.begin(), rMemberOrder.end(), aComp );
    }

    // handle children

    OSL_ENSURE( pRefDim && static_cast<size_t>(pRefDim->GetMemberCount()) == maMembers.size(), "dimensions don't match" );
    OSL_ENSURE( pRefDim == pResultDimension, "wrong dim" );

    // for data layout, call only once - sorting measure is always taken from settings
    long nLoopCount = bIsDataLayout ? 1 : nCount;
    for (long i=0; i<nLoopCount; i++)
    {
        ScDPResultMember* pRefMember = pRefDim->GetMember(i);
        if ( pRefMember->IsVisible() )  //! here or in ScDPDataMember ???
        {
            ScDPDataMember* pDataMember = maMembers[(sal_uInt16)i];
            pDataMember->SortMembers( pRefMember );
        }
    }
}

void ScDPDataDimension::DoAutoShow( ScDPResultDimension* pRefDim )
{
    long nCount = maMembers.size();

    // handle children first, before changing the visible state

    OSL_ENSURE( pRefDim && static_cast<size_t>(pRefDim->GetMemberCount()) == maMembers.size(), "dimensions don't match" );
    OSL_ENSURE( pRefDim == pResultDimension, "wrong dim" );

    // for data layout, call only once - sorting measure is always taken from settings
    long nLoopCount = bIsDataLayout ? 1 : nCount;
    for (long i=0; i<nLoopCount; i++)
    {
        ScDPResultMember* pRefMember = pRefDim->GetMember(i);
        if ( pRefMember->IsVisible() )  //! here or in ScDPDataMember ???
        {
            ScDPDataMember* pDataMember = maMembers[i];
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
        ScDPDataMember* pDataMember1 = maMembers[aAutoOrder[nIncluded - 1]];
        if ( !pDataMember1->IsVisible() )
            pDataMember1 = NULL;
        bool bContinue = true;
        while ( bContinue )
        {
            bContinue = false;
            if ( nIncluded < nCount )
            {
                ScDPDataMember* pDataMember2 = maMembers[aAutoOrder[nIncluded]];
                if ( !pDataMember2->IsVisible() )
                    pDataMember2 = NULL;

                if ( lcl_IsEqual( pDataMember1, pDataMember2, pRefDim->GetAutoMeasure() ) )
                {
                    ++nIncluded;                // include more members if values are equal
                    bContinue = true;
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
    long nCount = maMembers.size();
    for (long i=0; i<nCount; i++)
    {
        //  sort order doesn't matter

        long nMemberPos = bIsDataLayout ? 0 : i;
        ScDPDataMember* pDataMember = maMembers[nMemberPos];
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
                                    long nMeasure, bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState, ScDPRunningTotalState& rRunning,
                                    ScDPRowTotals& rTotals, const ScDPResultMember& rRowParent ) const
{
    OSL_ENSURE( pRefDim && static_cast<size_t>(pRefDim->GetMemberCount()) == maMembers.size(), "dimensions don't match" );
    OSL_ENSURE( pRefDim == pResultDimension, "wrong dim" );

    long nMemberMeasure = nMeasure;
    long nCount = maMembers.size();
    for (long i=0; i<nCount; i++)
    {
        const ScMemberSortOrder& rMemberOrder = pRefDim->GetMemberOrder();
        long nSorted = rMemberOrder.empty() ? i : rMemberOrder[i];

        long nMemberPos = nSorted;
        if (bIsDataLayout)
        {
            OSL_ENSURE(nMeasure == SC_DPMEASURE_ALL || pResultData->GetMeasureCount() == 1,
                        "DataLayout dimension twice?");
            nMemberPos = 0;
            nMemberMeasure = nSorted;
        }

        const ScDPResultMember* pRefMember = pRefDim->GetMember(nMemberPos);
        if ( pRefMember->IsVisible() )
        {
            if ( bIsDataLayout )
                rRunning.AddColIndex( 0, 0 );
            else
                rRunning.AddColIndex( i, nSorted );

            ScDPDataMember* pDataMember = maMembers[nMemberPos];
            pDataMember->UpdateRunningTotals(
                pRefMember, nMemberMeasure, bIsSubTotalRow, rSubState, rRunning, rTotals, rRowParent);

            rRunning.RemoveColIndex();
        }
    }
}

#if DEBUG_PIVOT_TABLE
void ScDPDataDimension::DumpState( const ScDPResultDimension* pRefDim, ScDocument* pDoc, ScAddress& rPos ) const
{
    OUString aDimName = bIsDataLayout ? OUString("(data layout)") : OUString("(unknown)");
    lcl_DumpRow( OUString("ScDPDataDimension"), aDimName, NULL, pDoc, rPos );

    SCROW nStartRow = rPos.Row();

    long nCount = bIsDataLayout ? 1 : maMembers.size();
    for (long i=0; i<nCount; i++)
    {
        const ScDPResultMember* pRefMember = pRefDim->GetMember(i);
        const ScDPDataMember* pDataMember = maMembers[i];
        pDataMember->DumpState( pRefMember, pDoc, rPos );
    }

    lcl_Indent( pDoc, nStartRow, rPos );
}

void ScDPDataDimension::Dump(int nIndent) const
{
    std::string aIndent(nIndent*2, ' ');
    std::cout << aIndent << "-- data dimension '"
        << (pResultDimension ? pResultDimension->GetName() : OUString()) << "'" << std::endl;
    ScDPDataMembers::const_iterator it = maMembers.begin(), itEnd = maMembers.end();
    for (; it != itEnd; ++it)
        (*it)->Dump(nIndent+1);
}
#endif

long ScDPDataDimension::GetMemberCount() const
{
    return maMembers.size();
}

const ScDPDataMember* ScDPDataDimension::GetMember(long n) const
{
    return maMembers[n];
}

ScDPDataMember* ScDPDataDimension::GetMember(long n)
{
    return maMembers[n];
}

// ----------------------------------------------------------------------------

ScDPResultVisibilityData::ScDPResultVisibilityData(
 ScDPSource* pSource) :
    mpSource(pSource)
{
}

ScDPResultVisibilityData::~ScDPResultVisibilityData()
{
}

void ScDPResultVisibilityData::addVisibleMember(const OUString& rDimName, const ScDPItemData& rMemberItem)
{
    DimMemberType::iterator itr = maDimensions.find(rDimName);
    if (itr == maDimensions.end())
    {
        pair<DimMemberType::iterator, bool> r = maDimensions.insert(
            DimMemberType::value_type(rDimName, VisibleMemberType()));

        if (!r.second)
            // insertion failed.
            return;

        itr = r.first;
    }
    VisibleMemberType& rMem = itr->second;
    VisibleMemberType::iterator itrMem = rMem.find(rMemberItem);
    if (itrMem == rMem.end())
        rMem.insert(rMemberItem);
}

void ScDPResultVisibilityData::fillFieldFilters(vector<ScDPFilteredCache::Criterion>& rFilters) const
{
    typedef boost::unordered_map<String, long, ScStringHashCode> FieldNameMapType;
    FieldNameMapType aFieldNames;
    ScDPTableData* pData = mpSource->GetData();
    long nColumnCount = pData->GetColumnCount();
    for (long i = 0; i < nColumnCount; ++i)
    {
        aFieldNames.insert(
            FieldNameMapType::value_type(pData->getDimensionName(i), i));
    }

    const ScDPDimensions* pDims = mpSource->GetDimensionsObject();
    for (DimMemberType::const_iterator itr = maDimensions.begin(), itrEnd = maDimensions.end();
          itr != itrEnd; ++itr)
    {
        const String& rDimName = itr->first;
        ScDPFilteredCache::Criterion aCri;
        FieldNameMapType::const_iterator itrField = aFieldNames.find(rDimName);
        if (itrField == aFieldNames.end())
            // This should never happen!
            continue;

        long nDimIndex = itrField->second;
        aCri.mnFieldIndex = static_cast<sal_Int32>(nDimIndex);
        aCri.mpFilter.reset(new ScDPFilteredCache::GroupFilter);

        ScDPFilteredCache::GroupFilter* pGrpFilter =
            static_cast<ScDPFilteredCache::GroupFilter*>(aCri.mpFilter.get());

        const VisibleMemberType& rMem = itr->second;
        for (VisibleMemberType::const_iterator itrMem = rMem.begin(), itrMemEnd = rMem.end();
              itrMem != itrMemEnd; ++itrMem)
        {
            const ScDPItemData& rMemItem = *itrMem;
            pGrpFilter->addMatchItem(rMemItem);
        }

        ScDPDimension* pDim = pDims->getByIndex(nDimIndex);
        ScDPMembers* pMembers = pDim->GetHierarchiesObject()->getByIndex(0)->
            GetLevelsObject()->getByIndex(0)->GetMembersObject();
        if (pGrpFilter->getMatchItemCount() < static_cast<size_t>(pMembers->getCount()))
            rFilters.push_back(aCri);
    }
}

size_t ScDPResultVisibilityData::MemberHash::operator() (const ScDPItemData& r) const
{
    if (r.IsValue())
        return static_cast<size_t>(::rtl::math::approxFloor(r.GetValue()));
    else
        return rtl_ustr_hashCode_WithLength(r.GetString().getStr(), r.GetString().getLength());
}
SCROW ScDPResultMember::GetDataId( ) const
{
 const ScDPMember*   pMemberDesc = GetDPMember();
  if (pMemberDesc)
        return  pMemberDesc->GetItemDataId();
    return -1;
}

ScDPResultMember* ScDPResultDimension::AddMember(const ScDPParentDimData &aData )
{
    ScDPResultMember* pMember = new ScDPResultMember( pResultData, aData, false );
    SCROW   nDataIndex = pMember->GetDataId();
    maMemberArray.push_back( pMember );

    if ( maMemberHash.end() == maMemberHash.find( nDataIndex ) )
        maMemberHash.insert( std::pair< SCROW, ScDPResultMember *>( nDataIndex, pMember ) );
    return pMember;
}

ScDPResultMember* ScDPResultDimension::InsertMember(ScDPParentDimData *pMemberData)
{
    SCROW  nInsert = 0;
    if ( !lcl_SearchMember( maMemberArray, pMemberData->mnOrder , nInsert ) )
    {
        ScDPResultMember* pNew = new ScDPResultMember( pResultData, *pMemberData, false );
        maMemberArray.insert( maMemberArray.begin()+nInsert, pNew );

        SCROW   nDataIndex = pMemberData->mpMemberDesc->GetItemDataId();
        if ( maMemberHash.end() == maMemberHash.find( nDataIndex ) )
            maMemberHash.insert( std::pair< SCROW, ScDPResultMember *>( nDataIndex, pNew ) );
        return pNew;
    }
    return maMemberArray[ nInsert ];
}

void ScDPResultDimension::InitWithMembers(
    LateInitParams& rParams, const std::vector<SCROW>& pItemData, size_t nPos,
    ScDPInitState& rInitState)
{
    if ( rParams.IsEnd( nPos ) )
        return;
    ScDPDimension* pThisDim        = rParams.GetDim( nPos );
    ScDPLevel*        pThisLevel      = rParams.GetLevel( nPos );
    SCROW             nDataID         = pItemData[nPos];

    if (pThisDim && pThisLevel)
    {
        long nDimSource = pThisDim->GetDimension();     //! check GetSourceDim?

        //  create all members at the first call (preserve order)
        ResultMembers* pMembers = pResultData->GetDimResultMembers(nDimSource, pThisDim, pThisLevel);
        ScDPGroupCompare aCompare( pResultData, rInitState, nDimSource );
        //  initialize only specific member (or all if "show empty" flag is set)
        ScDPResultMember* pResultMember = NULL;
        if ( bInitialized  )
            pResultMember = FindMember( nDataID );
        else
            bInitialized = true;

        if ( pResultMember == NULL )
        { //only insert found item
            ScDPParentDimData* pMemberData = pMembers->FindMember( nDataID );
            if ( pMemberData && aCompare.IsIncluded( *( pMemberData->mpMemberDesc ) ) )
                pResultMember = InsertMember( pMemberData );
        }
        if ( pResultMember )
        {
            rInitState.AddMember( nDimSource, pResultMember->GetDataId()  );
            pResultMember->LateInitFrom(rParams, pItemData, nPos+1, rInitState);
            rInitState.RemoveMember();
        }
    }
}

ScDPParentDimData::ScDPParentDimData() :
    mnOrder(-1), mpParentDim(NULL), mpParentLevel(NULL), mpMemberDesc(NULL) {}

ScDPParentDimData::ScDPParentDimData(
    SCROW nIndex, const ScDPDimension* pDim, const ScDPLevel* pLev, const ScDPMember* pMember) :
    mnOrder(nIndex), mpParentDim(pDim), mpParentLevel(pLev), mpMemberDesc(pMember) {}

ScDPParentDimData* ResultMembers::FindMember( SCROW nIndex ) const
{
    DimMemberHash::const_iterator aRes = maMemberHash.find( nIndex );
    if( aRes != maMemberHash.end()) {
           if (  aRes->second->mpMemberDesc && aRes->second->mpMemberDesc->GetItemDataId()==nIndex )
            return aRes->second;
    }
    return NULL;
}
void  ResultMembers::InsertMember(  ScDPParentDimData* pNew )
{
    if ( !pNew->mpMemberDesc->getShowDetails() )
        mbHasHideDetailsMember = true;
    maMemberHash.insert( std::pair< const SCROW, ScDPParentDimData *>( pNew->mpMemberDesc->GetItemDataId(), pNew ) );
}

ResultMembers::ResultMembers():
    mbHasHideDetailsMember( false )
{
}
ResultMembers::~ResultMembers()
{
    for ( DimMemberHash::const_iterator iter = maMemberHash.begin(); iter != maMemberHash.end(); ++iter )
        delete iter->second;
}
// -----------------------------------------------------------------------
LateInitParams::LateInitParams(
    const vector<ScDPDimension*>& ppDim, const vector<ScDPLevel*>& ppLev, bool bRow, bool bInitChild, bool bAllChildren ) :
    mppDim( ppDim ),
    mppLev( ppLev ),
    mbRow( bRow ),
    mbInitChild( bInitChild ),
    mbAllChildren( bAllChildren )
{
}

LateInitParams::~LateInitParams()
{
}

bool LateInitParams::IsEnd( size_t nPos ) const
{
    return nPos >= mppDim.size();
}

void ScDPResultDimension::CheckShowEmpty( bool bShow )
{
    long nCount = maMemberArray.size();

    ScDPResultMember* pMember = NULL;
    for (long i=0; i<nCount; i++)
    {
        pMember = maMemberArray.at(i);
        pMember->CheckShowEmpty(bShow);
    }

}

void ScDPResultMember::CheckShowEmpty( bool bShow )
{
    if (bHasElements)
    {
        ScDPResultDimension* pChildDim = GetChildDimension();
        if (pChildDim)
            pChildDim->CheckShowEmpty();
    }
    else if (IsValid() && bInitialized)
    {
        bShow = bShow || (GetParentLevel() && GetParentLevel()->getShowEmpty());
        if (bShow)
        {
            SetHasElements();
            ScDPResultDimension* pChildDim = GetChildDimension();
            if (pChildDim)
                pChildDim->CheckShowEmpty(true);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
