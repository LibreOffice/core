/*************************************************************************
 *
 *  $RCSfile: dptabres.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 18:05:33 $
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
#include <math.h>
#include <float.h>          //! Test !!!

#include "dptabres.hxx"
#include "dptabsrc.hxx"
#include "dptabdat.hxx"
#include "global.hxx"
#include "subtotal.hxx"
#include "globstr.hrc"
#include "datauno.hxx"      // ScDataUnoConversion

#include <com/sun/star/sheet/DataResultFlags.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

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


//! member of something or pass as arguments !!!!!!!!!!
static ScSubTotalFunc eColForce = SUBTOTAL_FUNC_NONE;
static ScSubTotalFunc eRowForce = SUBTOTAL_FUNC_NONE;

static long nColSubTotalFunc = -1;
static long nRowSubTotalFunc = -1;

static BOOL bLateInit = TRUE;

// -----------------------------------------------------------------------

void ScDPAggData::Update( const ScDPValueData& rNext, ScSubTotalFunc eFunc )
{
    if (nCount<0)       // error?
        return;         // nothing more...

    if ( rNext.nType == SC_VALTYPE_EMPTY )
        return;

    //! Test !!!
    if ( eColForce != SUBTOTAL_FUNC_NONE && eRowForce != SUBTOTAL_FUNC_NONE && eColForce != eRowForce )
        return;
    if ( eColForce != SUBTOTAL_FUNC_NONE ) eFunc = eColForce;
    if ( eRowForce != SUBTOTAL_FUNC_NONE ) eFunc = eRowForce;
    //! Test !!!

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
                if ( !SubTotal::SafePlus( fVal, rNext.fValue ) )
                    nCount = -1;                            // -1 for error
                double fAdd = rNext.fValue;
                if ( !SubTotal::SafeMult( fAdd, rNext.fValue ) ||
                     !SubTotal::SafePlus( fSquare, fAdd ) )
                    nCount = -1;                            // -1 for error
            }
            break;
        default:
            DBG_ERROR("invalid function");
    }
}

BOOL ScDPAggData::HasError( ScSubTotalFunc eFunc ) const
{
    //! Test !!!
    if ( eColForce != SUBTOTAL_FUNC_NONE ) eFunc = eColForce;
    if ( eRowForce != SUBTOTAL_FUNC_NONE ) eFunc = eRowForce;
    //! Test !!!

    switch (eFunc)
    {
        case SUBTOTAL_FUNC_SUM:
        case SUBTOTAL_FUNC_PROD:
        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:
            return ( nCount < 0 );          // only real errors

        case SUBTOTAL_FUNC_AVE:
        case SUBTOTAL_FUNC_MAX:
        case SUBTOTAL_FUNC_MIN:
        case SUBTOTAL_FUNC_STDP:
        case SUBTOTAL_FUNC_VARP:
            return ( nCount <= 0 );         // no data is an error

        case SUBTOTAL_FUNC_STD:
        case SUBTOTAL_FUNC_VAR:
            return ( nCount < 2 );          // need at least 2 values

        default:
            DBG_ERROR("invalid function");
    }
    return TRUE;
}

double ScDPAggData::GetResult( ScSubTotalFunc eFunc ) const
{
    //! Test !!!
    if ( eColForce != SUBTOTAL_FUNC_NONE ) eFunc = eColForce;
    if ( eRowForce != SUBTOTAL_FUNC_NONE ) eFunc = eRowForce;
    //! Test !!!

    switch (eFunc)
    {
        case SUBTOTAL_FUNC_MAX:
        case SUBTOTAL_FUNC_MIN:
        case SUBTOTAL_FUNC_SUM:
        case SUBTOTAL_FUNC_PROD:
            //  different error conditions are handled in HasError
            return fVal;

        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:
            return nCount;

        case SUBTOTAL_FUNC_AVE:
            if ( nCount > 0 )
                return fVal / (double) nCount;
            DBG_ERROR("empty average");
            break;

        //! use safe mul for fVal * fVal

        case SUBTOTAL_FUNC_STD:
            if ( nCount >= 2 )
                return sqrt((fSquare - fVal*fVal/(double)(nCount)) / (double)(nCount-1));
            DBG_ERROR("empty dev");
            break;
        case SUBTOTAL_FUNC_VAR:
            if ( nCount >= 2 )
                return (fSquare - fVal*fVal/(double)(nCount)) / (double)(nCount-1);
            DBG_ERROR("empty var");
            break;
        case SUBTOTAL_FUNC_STDP:
            if ( nCount > 0 )
                return sqrt((fSquare - fVal*fVal/(double)(nCount)) / (double)nCount);
            DBG_ERROR("empty devp")
            break;
        case SUBTOTAL_FUNC_VARP:
            if ( nCount > 0 )
                return (fSquare - fVal*fVal/(double)(nCount)) / (double)nCount;
            DBG_ERROR("empty devp")
            break;
        default:
            DBG_ERROR("invalid function");
    }
    return 0.0;
}

ScDPAggData* ScDPAggData::GetChild()
{
    if (!pChild)
        pChild = new ScDPAggData;
    return pChild;
}

// -----------------------------------------------------------------------

ScSubTotalFunc lcl_GetForceFunc( ScDPLevel* pLevel, long nFuncNo )
{
    ScSubTotalFunc eRet = SUBTOTAL_FUNC_NONE;
    if ( pLevel )
    {
        //! direct access via ScDPLevel

        uno::Sequence<sheet::GeneralFunction> aSeq = pLevel->getSubTotals();
        if ( nFuncNo < aSeq.getLength() )
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
    pMeasNames( NULL ),
    bDataAtCol( FALSE ),
    bDataAtRow( FALSE )
{
}

ScDPResultData::~ScDPResultData()
{
    delete[] pMeasFuncs;
    delete[] pMeasNames;
}

void ScDPResultData::SetMeasureData( long nCount, const ScSubTotalFunc* pFunctions,
                                        const String* pNames )
{
    delete[] pMeasFuncs;
    delete[] pMeasNames;
    if ( nCount )
    {
        nMeasCount = nCount;
        pMeasFuncs = new ScSubTotalFunc[nCount];
        pMeasNames = new String[nCount];
        for (long i=0; i<nCount; i++)
        {
            pMeasFuncs[i] = pFunctions[i];
            pMeasNames[i] = pNames[i];
        }
    }
    else
    {
        //  use one dummy measure
        nMeasCount = 1;
        pMeasFuncs = new ScSubTotalFunc[1];
        pMeasFuncs[0] = SUBTOTAL_FUNC_NONE;
        pMeasNames = new String[1];
        pMeasNames[0] = ScGlobal::GetRscString( STR_EMPTYDATA );
    }
}

void ScDPResultData::SetDataLayoutOrientation( USHORT nOrient )
{
    bDataAtCol = ( nOrient == sheet::DataPilotFieldOrientation_COLUMN );
    bDataAtRow = ( nOrient == sheet::DataPilotFieldOrientation_ROW );
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
    bInitialized( FALSE )
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

BOOL ScDPResultMember::IsNamedItem( const ScDPItemData& r ) const
{
    //! store ScDPMember pointer instead of ScDPMember ???

    if (pMemberDesc)
        return ((ScDPMember*)pMemberDesc)->IsNamedItem( r );
    return FALSE;
}

void ScDPResultMember::InitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev )
{
    //  with bLateInit, initialize only those members that have data
    if ( bLateInit )
        return;

    bInitialized = TRUE;

    //  skip child dimension if details are not shown
    if ( pMemberDesc && !pMemberDesc->getShowDetails() )
        return;

    if ( *ppDim )
    {
        pChildDimension = new ScDPResultDimension( pResultData );
        pChildDimension->InitFrom( ppDim, ppLev );
    }
}

void ScDPResultMember::LateInitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev, ScDPItemData* pItemData )
{
    //  without bLateInit, everything has already been initialized
    if ( !bLateInit )
        return;

    bInitialized = TRUE;

    //  skip child dimension if details are not shown
    if ( pMemberDesc && !pMemberDesc->getShowDetails() )
        return;

    if ( *ppDim )
    {
        //  LateInitFrom is called several times...
        if ( !pChildDimension )
            pChildDimension = new ScDPResultDimension( pResultData );
        pChildDimension->LateInitFrom( ppDim, ppLev, pItemData );
    }
}

long ScDPResultMember::GetSize(long nMeasure) const
{
    if ( !IsVisible() )
        return 0;

    if ( pChildDimension )
    {
        long nSize = pChildDimension->GetSize(nMeasure);
        long nUserSubCount = GetSubTotalCount();
        if ( nUserSubCount )
        {
            if ( nMeasure == SC_DPMEASURE_ALL )
                nSize += pResultData->GetMeasureCount() * nUserSubCount;
            else
                nSize += nUserSubCount;
        }
        return nSize;
    }
    else
    {
        if ( nMeasure == SC_DPMEASURE_ALL )
            return pResultData->GetMeasureCount();
        else
            return 1;
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

    return TRUE;
}

long ScDPResultMember::GetSubTotalCount() const
{
    if ( bForceSubTotal )       // set if needed for root members
        return 1;               // grand total is always "automatic"
    else if ( pParentLevel )
    {
        //! direct access via ScDPLevel
        return pParentLevel->getSubTotals().getLength();
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

        //! Test !!!!
        ScSubTotalFunc eOld = eRowForce;
        long nOldSub = nRowSubTotalFunc;
        //! Test !!!!

        long nUserSubCount = GetSubTotalCount();
        if ( !nUserSubCount || !pChildDimension )           //! Test
            nUserSubCount = 1;                              //! Test

        for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)
        {
            //! Test !!!!
            if ( pChildDimension )
            {
                nRowSubTotalFunc = nUserPos;
                eRowForce = lcl_GetForceFunc( pParentLevel, nUserPos );
            }
            //! Test !!!!

            pDataRoot->ProcessData( pDataMembers, pValues );

            //! Test !!!!
            eRowForce = eOld;
            nRowSubTotalFunc = nOldSub;
            //! Test !!!!
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

    String aName;
    if ( pMemberName )          // if pMemberName != NULL, use instead of real member name
        aName = *pMemberName;
    else
        aName = GetName();

    String aCaption = aName;
    if ( pMemberCaption )                   // use pMemberCaption if != NULL
        aCaption = *pMemberCaption;
    if (!aCaption.Len())
        aCaption = ScGlobal::GetRscString(STR_EMPTYDATA);

    if ( nSize && !bRoot )                  // root is overwritten by first dimension
    {
        pArray[rPos].Name    = rtl::OUString(aName);
        pArray[rPos].Caption = rtl::OUString(aCaption);
        pArray[rPos].Flags  |= sheet::MemberResultFlags::HASMEMBER;

        //  set "continue" flag (removed for subtotals later)
        for (long i=1; i<nSize; i++)
            pArray[rPos+i].Flags |= sheet::MemberResultFlags::CONTINUE;
    }

    BOOL bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
    {
        if (bRoot)      // same sequence for root member
            pChildDimension->FillMemberResults( pSequences, rPos, nMeasure );
        else
            pChildDimension->FillMemberResults( pSequences + 1, rPos, nMeasure );
    }

    rPos += nSize;

    long nUserSubCount = GetSubTotalCount();
    if ( nUserSubCount && pChildDimension )
    {
        long nMemberMeasure = nMeasure;
        long nSubSize = pResultData->GetCountForMeasure(nMeasure);

        rPos -= nSubSize * nUserSubCount;                   //  GetSize includes space for SubTotal

        for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)
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
    }
}

void ScDPResultMember::FillDataResults( const ScDPResultMember* pRefMember,
                            uno::Sequence< uno::Sequence<sheet::DataResult> >& rSequence,
                            long& rRow, long nMeasure ) const
{
    //  IsVisible() test is in ScDPResultDimension::FillDataResults
    //  (not on data layout dimension)

    BOOL bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
    {
        pChildDimension->FillDataResults( pRefMember, rSequence, rRow, nMeasure );
        rRow += (USHORT) GetSize( nMeasure );
    }

    long nUserSubCount = GetSubTotalCount();
    if ( nUserSubCount || !bHasChild )
    {
        if ( !nUserSubCount || !bHasChild )             //! Test
            nUserSubCount = 1;                          //! Test

        long nMemberMeasure = nMeasure;
        long nSubSize = pResultData->GetCountForMeasure(nMeasure);
        if (bHasChild)
            rRow -= nSubSize * nUserSubCount;       // GetSize includes space for SubTotal

        //! Test !!!!
        ScSubTotalFunc eOld = eRowForce;
        long nOldSub = nRowSubTotalFunc;
        //! Test !!!!

        if ( pDataRoot )
        {
            for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)
            {
                //! Test !!!!
                if ( bHasChild )
                {
                    nRowSubTotalFunc = nUserPos;
                    eRowForce = lcl_GetForceFunc( pParentLevel, nUserPos );
                }
                //! Test !!!!

                for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
                {
                    if ( nMeasure == SC_DPMEASURE_ALL )
                        nMemberMeasure = nSubCount;
                    else if ( pResultData->GetColStartMeasure() == SC_DPMEASURE_ALL )
                        nMemberMeasure = SC_DPMEASURE_ALL;

                    DBG_ASSERT( rRow < rSequence.getLength(), "bumm" );
                    uno::Sequence<sheet::DataResult>& rSubSeq = rSequence.getArray()[rRow];
                    long nSeqCol = 0;
                    pDataRoot->FillDataRow( pRefMember, rSubSeq, nSeqCol, nMemberMeasure, bHasChild );

                    rRow += 1;
                }
            }
        }
        else
            rRow += nSubSize * nUserSubCount;       // empty rows occur when ShowEmpty is true

        //! Test !!!!
        eRowForce = eOld;
        nRowSubTotalFunc = nOldSub;
        //! Test !!!!
    }
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

BOOL ScDPDataMember::IsNamedItem( const ScDPItemData& r ) const
{
    if (pResultMember)
        return pResultMember->IsNamedItem(r);
    else
        return FALSE;
}

void ScDPDataMember::InitFrom( ScDPResultDimension* pDim )
{
    if ( !pChildDimension )
        pChildDimension = new ScDPDataDimension(pResultData);
    pChildDimension->InitFrom(pDim);
}

long lcl_GetSubTotalPos()
{
//  return Max( nColSubTotalFunc, nRowSubTotalFunc );

    long nRet = -1;
    if ( nColSubTotalFunc >= 0 ) nRet = nColSubTotalFunc;
    if ( nRowSubTotalFunc >= 0 ) nRet = nRowSubTotalFunc;
    return nRet;
}

void ScDPDataMember::UpdateValues(const ScDPValueData* pValues)
{
    //! find out how many and which subtotals are used

    ScDPAggData* pAgg = &aAggregate;

    long nSubPos = lcl_GetSubTotalPos();
    if (nSubPos > 0)
    {
        long nSkip = nSubPos * pResultData->GetMeasureCount();
        for (long i=0; i<nSkip; i++)
            pAgg = pAgg->GetChild();        // created if not there
    }

    long nCount = pResultData->GetMeasureCount();
    for (long nPos=0; nPos<nCount; nPos++)
    {
        pAgg->Update( *pValues, pResultData->GetMeasureFunction(nPos) );
        if ( nPos+1 < nCount )
        {
            pAgg = pAgg->GetChild();        // created if not there
            ++pValues;                      // next value entry
        }
    }
}

void ScDPDataMember::ProcessData( const ScDPItemData* pChildMembers, const ScDPValueData* pValues )
{
    //! Test !!!!
    ScSubTotalFunc eOld = eColForce;
    long nOldSub = nColSubTotalFunc;
    //! Test !!!!

    long nUserSubCount = pResultMember ? pResultMember->GetSubTotalCount() : 0;
    if ( !nUserSubCount || !pChildDimension )           //! Test
        nUserSubCount = 1;                              //! Test

    for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)
    {
        //! Test !!!!
        if ( pChildDimension )
        {
            ScDPLevel* pForceLevel = pResultMember ? pResultMember->GetParentLevel() : NULL;
            nColSubTotalFunc = nUserPos;
            eColForce = lcl_GetForceFunc( pForceLevel, nUserPos );
        }
        //! Test !!!!

        UpdateValues(pValues);
    }

    //! Test !!!!
    eColForce = eOld;
    nColSubTotalFunc = nOldSub;
    //! Test !!!!

    if (pChildDimension)
        pChildDimension->ProcessData( pChildMembers, pValues );
}

BOOL ScDPDataMember::HasData(long nMeasure) const
{
    //! Test !!!
    if ( eColForce != SUBTOTAL_FUNC_NONE && eRowForce != SUBTOTAL_FUNC_NONE && eColForce != eRowForce )
        return FALSE;
    //! Test !!!

    //  #74542# HasData can be different between measures!

    DBG_ASSERT( nMeasure >= 0, "HasData: no measure" );

    const ScDPAggData* pAgg = &aAggregate;
    long nSkip = nMeasure;
    long nSubPos = lcl_GetSubTotalPos();
    if (nSubPos > 0)
        nSkip += nSubPos * pResultData->GetMeasureCount();

    for (long i=0; i<nSkip; i++)
    {
        pAgg = pAgg->GetExistingChild();
//!     DBG_ASSERT( pAgg, "AggData not found" );
        if (!pAgg)
            return FALSE;           //! error?
    }

    return pAgg->HasData();
}

BOOL ScDPDataMember::HasError(long nMeasure) const
{
    //! merge with GetAggregate?

    DBG_ASSERT( nMeasure >= 0, "GetAggregate: no measure" );

    const ScDPAggData* pAgg = &aAggregate;
    long nSkip = nMeasure;
    long nSubPos = lcl_GetSubTotalPos();
    if (nSubPos > 0)
        nSkip += nSubPos * pResultData->GetMeasureCount();

    for ( long nPos=0; nPos<nSkip; nPos++ )
    {
        pAgg = pAgg->GetExistingChild();
//!     DBG_ASSERT( pAgg, "AggData not found" );
        if (!pAgg)
            return TRUE;
    }

    return pAgg->HasError( pResultData->GetMeasureFunction(nMeasure) );
}

double ScDPDataMember::GetAggregate( long nMeasure ) const
{
    //! allow to return errors

    DBG_ASSERT( nMeasure >= 0, "GetAggregate: no measure" );

    const ScDPAggData* pAgg = &aAggregate;
    long nSkip = nMeasure;
    long nSubPos = lcl_GetSubTotalPos();
    if (nSubPos > 0)
        nSkip += nSubPos * pResultData->GetMeasureCount();

    for ( long nPos=0; nPos<nSkip; nPos++ )
    {
        pAgg = pAgg->GetExistingChild();
//!     DBG_ASSERT( pAgg, "AggData not found" );
        if (!pAgg)
            return DBL_MAX;         //! error!!!
    }

    return pAgg->GetResult( pResultData->GetMeasureFunction(nMeasure) );
}

void ScDPDataMember::FillDataRow( const ScDPResultMember* pRefMember,
                                    uno::Sequence<sheet::DataResult>& rSequence,
                                    long& rCol, long nMeasure, BOOL bIsSubTotalRow ) const
{
    DBG_ASSERT( pRefMember == pResultMember || !pResultMember, "bla" );

    if ( pRefMember->IsVisible() )  //! here or in ScDPDataDimension::FillDataRow ???
    {
        const ScDPDataMember* pDataMember = this;       //! Test

        const ScDPDataDimension* pDataChild = pDataMember->GetChildDimension();
        const ScDPResultDimension* pRefChild = pRefMember->GetChildDimension();
        BOOL bHasChild = ( pDataChild && pRefChild );

        if ( bHasChild )
        {
            pDataChild->FillDataRow( pRefChild, rSequence, rCol, nMeasure, bIsSubTotalRow );
            rCol += (USHORT)pRefMember->GetSize( nMeasure );
        }

        long nUserSubCount = pRefMember->GetSubTotalCount();
        if ( nUserSubCount || !bHasChild )
        {
            if ( !nUserSubCount || !bHasChild )     //! Test
                nUserSubCount = 1;                  //! Test

            //! Test !!!!
            ScSubTotalFunc eOld = eColForce;
            long nOldSub = nColSubTotalFunc;
            //! Test !!!!

            long nMemberMeasure = nMeasure;
            long nSubSize = pResultData->GetCountForMeasure(nMeasure);
            if (bHasChild)
                rCol -= nSubSize * nUserSubCount;       // GetSize includes space for SubTotal

            for (long nUserPos=0; nUserPos<nUserSubCount; nUserPos++)
            {
                //! Test !!!!
                if ( pChildDimension )
                {
                    ScDPLevel* pForceLevel = pResultMember ? pResultMember->GetParentLevel() : NULL;
                    nColSubTotalFunc = nUserPos;
                    eColForce = lcl_GetForceFunc( pForceLevel, nUserPos );
                }
                //! Test !!!!

                for ( long nSubCount=0; nSubCount<nSubSize; nSubCount++ )
                {
                    if ( nMeasure == SC_DPMEASURE_ALL )
                        nMemberMeasure = nSubCount;

                    DBG_ASSERT( rCol < rSequence.getLength(), "bumm" );
                    sheet::DataResult& rRes = rSequence.getArray()[rCol];

                    if ( pDataMember->HasData(nMemberMeasure) )
                    {
                        if ( pDataMember->HasError(nMemberMeasure) )
                        {
                            rRes.Value = 0;
                            rRes.Flags |= sheet::DataResultFlags::ERROR;
                        }
                        else
                        {
                            rRes.Value = pDataMember->GetAggregate(nMemberMeasure);
                            rRes.Flags |= sheet::DataResultFlags::HASDATA;
                        }
                    }

                    if ( bHasChild || bIsSubTotalRow )
                        rRes.Flags |= sheet::DataResultFlags::SUBTOTAL;

                    rCol += 1;
                }
            }

            //! Test !!!!
            eColForce = eOld;
            nColSubTotalFunc = nOldSub;
            //! Test !!!!
        }
    }
}


// -----------------------------------------------------------------------

ScDPResultDimension::ScDPResultDimension( ScDPResultData* pData ) :
    pResultData( pData ),
    bIsDataLayout( FALSE )
{
}

ScDPResultDimension::~ScDPResultDimension()
{
}

void ScDPResultDimension::InitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev )
{
    ScDPDimension* pThisDim = *ppDim;
    ScDPLevel* pThisLevel = *ppLev;
    if (pThisDim && pThisLevel)
    {
        ScDPDimension** ppChildDim = ppDim + 1;
        ScDPLevel** ppChildLev = ppLev + 1;

        bIsDataLayout = pThisDim->getIsDataLayoutDimension();

        ScDPMembers* pMembers = pThisLevel->GetMembersObject();
        long nMembCount = pMembers->getCount();
        for ( long i=0; i<nMembCount; i++ )
        {
            ScDPMember* pMember = pMembers->getByIndex(i);
            ScDPResultMember* pNew = new ScDPResultMember( pResultData, pThisDim,
                                            pThisLevel, pMember, FALSE );
            aMembers.Insert( pNew, aMembers.Count() );

            pNew->InitFrom( ppChildDim, ppChildLev );
        }
    }
}

void ScDPResultDimension::LateInitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev, ScDPItemData* pItemData )
{
    ScDPDimension* pThisDim = *ppDim;
    ScDPLevel* pThisLevel = *ppLev;
    ScDPItemData& rThisData = *pItemData;
    if (pThisDim && pThisLevel)
    {
        ScDPDimension** ppChildDim = ppDim + 1;
        ScDPLevel** ppChildLev = ppLev + 1;
        ScDPItemData* pChildData = pItemData + 1;

        if ( aMembers.Count() == 0 )
        {
            //  create all members at the first call (preserve order)

            bIsDataLayout = pThisDim->getIsDataLayoutDimension();

            ScDPMembers* pMembers = pThisLevel->GetMembersObject();
            long nMembCount = pMembers->getCount();
            for ( long i=0; i<nMembCount; i++ )
            {
                ScDPMember* pMember = pMembers->getByIndex(i);
                ScDPResultMember* pNew = new ScDPResultMember( pResultData, pThisDim,
                                                pThisLevel, pMember, FALSE );
                aMembers.Insert( pNew, aMembers.Count() );
            }
        }

        //  initialize only specific member (or all if "show empty" flag is set)

        BOOL bShowEmpty = pThisLevel->getShowEmpty();
        long nCount = aMembers.Count();
        for (long i=0; i<nCount; i++)
        {
            ScDPResultMember* pResultMember = aMembers[(USHORT)i];
            if ( bIsDataLayout || bShowEmpty || pResultMember->IsNamedItem( rThisData ) )
            {
                pResultMember->LateInitFrom( ppChildDim, ppChildLev, pChildData );
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
    //! pass end-marker behind valid members?

//  if (!pMembers->IsEmpty())               //! non-empty sequence..
    {
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
    }
    DBG_ERROR("IsValidEntry: Member not found");
    return FALSE;
}

void ScDPResultDimension::ProcessData( const ScDPItemData* pMembers,
                            ScDPResultDimension* pDataDim, const ScDPItemData* pDataMembers,
                            const ScDPValueData* pValues )
{
    //! pass end-marker behind valid members?

//  if (!pMembers->IsEmpty())               //! non-empty sequence..
    {
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
        ScDPResultMember* pMember = aMembers[(USHORT)i];
        //  in data layout dimension, use first member with different measures/names
        if ( bIsDataLayout )
        {
            String aMbrName = pResultData->GetMeasureDimensionName( i );
            String aMbrCapt = pResultData->GetMeasureString( i, FALSE, SUBTOTAL_FUNC_NONE );
            aMembers[0]->FillMemberResults( pSequences, nPos, i, FALSE, &aMbrName, &aMbrCapt );
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
            pMember->FillDataResults( pRefMember, rSequence, nMemberRow, nMemberMeasure );
            // nMemberRow is modified
    }
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

    bIsDataLayout = pDim->IsDataLayout();

    long nCount = pDim->GetMemberCount();
    for (long i=0; i<nCount; i++)
    {
        ScDPResultMember* pResMem = pDim->GetMember(i);

        ScDPDataMember* pNew = new ScDPDataMember( pResultData, pResMem );
        aMembers.Insert( pNew, aMembers.Count() );

        ScDPResultDimension* pChildDim = pResMem->GetChildDimension();
        if ( pChildDim )
            pNew->InitFrom( pChildDim );
    }
}

void ScDPDataDimension::ProcessData( const ScDPItemData* pDataMembers, const ScDPValueData* pValues )
{
    //! pass end-marker behind valid members?

//  if (!pDataMembers->IsEmpty())               //! non-empty sequence..
    {
        long nCount = aMembers.Count();
        for (long i=0; i<nCount; i++)
        {
            ScDPDataMember* pMember = aMembers[(USHORT)i];

            // always first member for data layout dim
            if ( bIsDataLayout || pMember->IsNamedItem( *pDataMembers ) )
            {
                pMember->ProcessData( pDataMembers + 1, pValues );
                return;
            }
        }
    }
    DBG_ERROR("ProcessData: Member not found");
}

void ScDPDataDimension::FillDataRow( const ScDPResultDimension* pRefDim,
                                    uno::Sequence<sheet::DataResult>& rSequence,
                                    long nCol, long nMeasure, BOOL bIsSubTotalRow ) const
{
    DBG_ASSERT( pRefDim && pRefDim->GetMemberCount() == aMembers.Count(), "dimensions don't match" );

    long nMemberMeasure = nMeasure;
    long nMemberCol = nCol;
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

        const ScDPResultMember* pRefMember = pRefDim->GetMember(nMemberPos);
        if ( pRefMember->IsVisible() )  //! here or in ScDPDataMember::FillDataRow ???
        {
            const ScDPDataMember* pDataMember = aMembers[(USHORT)nMemberPos];
            pDataMember->FillDataRow( pRefMember, rSequence, nMemberCol, nMemberMeasure, bIsSubTotalRow );
            // nMemberCol is modified
        }
    }
}




