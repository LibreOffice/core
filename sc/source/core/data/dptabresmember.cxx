/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dptabresmember.cxx,v $
 * $Revision: 1.0 $
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
#include "dptabresmember.hxx"
// -----------------------------------------------------------------------
ScDPResultMember(  const ScDPResultData* pData, const ScDPParentDimData& rParentDimData ,
                                    BOOL bForceSub ) :
    pResultData( pData ),
       aParentDimData( rParentDimData ),
        /* pParentDim( pDim ),
        pParentLevel( pLev ),
        pMemberDesc( pDesc ),*/
        pChildDimension( NULL ),
    pDataRoot( NULL ),
    bHasElements( FALSE ),
    bForceSubTotal( bForceSub ),
    bHasHiddenDetails( FALSE ),
    bInitialized( FALSE ),
    nMemberStep( 1 ),
    bAutoHidden( FALSE )
{
    // pParentLevel/pMemberDesc is 0 for root members
}

ScDPNormalResultMember::ScDPNormalResultMember(  const ScDPResultData* pData,
                                    BOOL bForceSub ) :
    pResultData( pData ),
        pChildDimension( NULL ),
    pDataRoot( NULL ),
    bHasElements( FALSE ),
    bForceSubTotal( bForceSub ),
    bHasHiddenDetails( FALSE ),
    bInitialized( FALSE ),
    nMemberStep( 1 ),
    bAutoHidden( FALSE )
{
}

ScDPNormalResultMember::~ScDPNormalResultMember()
{
    delete pChildDimension;
    delete pDataRoot;
}

String ScDPNormalResultMember::GetName() const
{
// Wang Xu Ming -- 2009-6-10
// DataPilot Migration
  const ScDPMember*   pMemberDesc = GetDPMember();
 // End Comments
  if (pMemberDesc)
        return pMemberDesc->GetNameStr();
    else
        return ScGlobal::GetRscString(STR_PIVOT_TOTAL);         // root member
}

void ScDPNormalResultMember::FillItemData( ScDPItemData& rData ) const
{
// Wang Xu Ming -- 2009-6-10
// DataPilot Migration
    const ScDPMember*   pMemberDesc = GetDPMember();
// End Comments
    if (pMemberDesc)
        pMemberDesc->FillItemData( rData );
    else
        rData.SetString( ScGlobal::GetRscString(STR_PIVOT_TOTAL) );     // root member
}

BOOL ScDPNormalResultMember::IsNamedItem( /*const ScDPItemData& r */SCROW nIndex ) const
{
    //! store ScDPMember pointer instead of ScDPMember ???
  const ScDPMember*   pMemberDesc = GetDPMember();
  if (pMemberDesc)
        return ((ScDPMember*)pMemberDesc)->IsNamedItem(/* r*/ nIndex  );
    return FALSE;
}

// Wang Xu Ming -- 2009-5-27
// DataPilot Migration
bool ScDPNormalResultMember::IsValidEntry( const vector< SCROW >& aMembers ) const
{
        return GetEntryStatus( aMembers ) != ENTRY_INVALID;
}
// End Comments

ENTRYSTATUS ScDPNormalResultMember::GetEntryStatus( const vector< SCROW >& aMembers ) const
{
    if ( !IsValid() )
        return ENTRY_INVALID;

    const ScDPResultDimension* pChildDim = GetChildDimension();
    if (pChildDim)
    {
        if (aMembers.size() < 2)
            return ENTRY_INVALID;

        vector</*ScDPItemData*/SCROW>::const_iterator itr = aMembers.begin();
        vector</*ScDPItemData*/SCROW> aChildMembers(++itr, aMembers.end());
        return pChildDim->GetEntryStatus(aChildMembers);
    }
    else if( bHasHiddenDetails )
        return ENTRY_HASHIDDENDETAIL;
    else
        return ENTRY_VALID;
}

void ScDPNormalResultMember::InitFrom( const vector<ScDPDimension*>& ppDim, const vector<ScDPLevel*>& ppLev,
                                 size_t nPos, ScDPInitState& rInitState ,
                                 BOOL bInitChild /*= TRUE */)
    {
    //  with LateInit, initialize only those members that have data
    if ( pResultData->IsLateInit() )
        return;

    bInitialized = TRUE;

    if (nPos >= ppDim.size()  )
        return;

    //  skip child dimension if details are not shown
    if ( GetDPMember() && !GetDPMember()->getShowDetails() )
    {
         // Wang Xu Ming -- 2009-6-16
        // Show DataLayout dimention
        nMemberStep = 1;
        while ( nPos < ppDim.size() )
        {
            if (  ppDim[nPos] ->getIsDataLayoutDimension() )
            {
                 if ( !pChildDimension )
                        pChildDimension = new ScDPResultDimension( pResultData );
                    pChildDimension->InitFrom( ppDim, ppLev, nPos, rInitState , FALSE );
                    return;
            }
            else
            { //find next dim
                nPos ++;
                nMemberStep ++;
            }
        }
        // End Comments
        bHasHiddenDetails = TRUE;   // only if there is a next dimension
        return;
    }

    if ( bInitChild )
    {
        pChildDimension = new ScDPResultDimension( pResultData );
        pChildDimension->InitFrom( ppDim, ppLev, nPos, rInitState, TRUE  );
    }
}

// Wang Xu Ming -- 2009-6-10
// DataPilot Migration
void ScDPNormalResultMember::LateInitFrom( LateInitParams& rParams/*const vector<ScDPDimension*>& ppDim, const vector<ScDPLevel*>& ppLev*/,
                                     const vector< SCROW >& pItemData,   size_t nPos,
                                     ScDPInitState& rInitState )
// End Comments
{
    //  without LateInit, everything has already been initialized
    if ( !pResultData->IsLateInit() )
        return;

    bInitialized = TRUE;

    if ( rParams.IsEnd( nPos )  /*nPos >= ppDim.size()*/)
        // No next dimension.  Bail out.
        return;

    //  skip child dimension if details are not shown
    if ( GetDPMember() && !GetDPMember()->getShowDetails() )
    {
        // Wang Xu Ming -- 2009-6-16
        // DataPilot Migration
        // Show DataLayout dimention
        nMemberStep = 1;
        while ( !rParams.IsEnd( nPos ) )
        {
            if (  rParams.GetDim( nPos ) ->getIsDataLayoutDimension() )
            {
                 if ( !pChildDimension )
                        pChildDimension = new ScDPResultDimension( pResultData );
                    rParams.SetInitChild( FALSE );
                    pChildDimension->LateInitFrom( rParams, pItemData, nPos, rInitState );
                    return;
            }
            else
            { //find next dim
                nPos ++;
                nMemberStep ++;
            }
        }
        // End Comments
        bHasHiddenDetails = TRUE;   // only if there is a next dimension
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

BOOL ScDPNormalResultMember::IsSubTotalInTitle(long nMeasure) const
{
    BOOL bRet = FALSE;
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
                bRet = TRUE;
        }
    }
    return bRet;
}

long ScDPNormalResultMember::GetSize(long nMeasure) const
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


BOOL ScDPNormalResultMember::IsVisible() const
{
    //  not initialized -> shouldn't be there at all
    //  (allocated only to preserve ordering)
   const ScDPLevel* pParentLevel = GetParentLevel();
    return ( bHasElements || ( pParentLevel && pParentLevel->getShowEmpty() ) ) && IsValid() && bInitialized;
}

BOOL ScDPNormalResultMember::IsValid() const
{
    //  non-Valid members are left out of calculation

    //  was member set no invisible at the DataPilotSource?
  const ScDPMember*     pMemberDesc =GetDPMember();
    if ( pMemberDesc && !pMemberDesc->getIsVisible() )
        return FALSE;

    if ( bAutoHidden )
        return FALSE;

    return TRUE;
}

BOOL ScDPNormalResultMember::HasHiddenDetails() const
{
    // bHasHiddenDetails is set only if the "show details" flag is off,
    // and there was a child dimension to skip

    return bHasHiddenDetails;
}

long ScDPNormalResultMember::GetSubTotalCount( long* pUserSubStart ) const
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

void ScDPNormalResultMember::ProcessData( const vector< SCROW >& aChildMembers, const ScDPResultDimension* pDataDim,
                                    const vector< SCROW >& aDataMembers, const vector<ScDPValueData>& aValues )
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

void ScDPNormalResultMember::FillMemberResults( uno::Sequence<sheet::MemberResult>* pSequences,
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
        aName = aItemData.GetString();
        bIsNumeric = aItemData.IsValue();
    }
    const ScDPDimension*        pParentDim = GetParentDim();
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

    const ScDPLevel*    pParentLevel = GetParentLevel();
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
            // Wang Xu Ming -- 2009-6-16
            // DataPilot Migration
            // for show details
            pChildDimension->FillMemberResults( pSequences + nMemberStep/*1*/, rPos, nMeasure );
             // End Comments

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

void ScDPNormalResultMember::FillDataResults( const ScDPResultMember* pRefMember,
                            uno::Sequence< uno::Sequence<sheet::DataResult> >& rSequence,
                            long& rRow, long nMeasure ) const
{
    //  IsVisible() test is in ScDPResultDimension::FillDataResults
    //  (not on data layout dimension)
    const ScDPLevel*     pParentLevel = GetParentLevel();
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

void ScDPNormalResultMember::UpdateDataResults( const ScDPResultMember* pRefMember, long nMeasure ) const
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

void ScDPNormalResultMember::SortMembers( ScDPResultMember* pRefMember )
{
    BOOL bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
        pChildDimension->SortMembers( pRefMember );     // sorting is done at the dimension

    if ( IsRoot() && pDataRoot )
    {
        // use the row root member to sort columns
        // sub total count is always 1

        pDataRoot->SortMembers( pRefMember );
    }
}

void ScDPNormalResultMember::DoAutoShow( ScDPResultMember* pRefMember )
{
    BOOL bHasChild = ( pChildDimension != NULL );
    if (bHasChild)
        pChildDimension->DoAutoShow( pRefMember );     // sorting is done at the dimension

    if ( IsRoot()&& pDataRoot )
    {
        // use the row root member to sort columns
        // sub total count is always 1

        pDataRoot->DoAutoShow( pRefMember );
    }
}

void ScDPNormalResultMember::ResetResults( BOOL bRoot )
{
    if (pDataRoot)
        pDataRoot->ResetResults();

    if (pChildDimension)
        pChildDimension->ResetResults();

// Wang Xu Ming -- 3/4/2009
// Dim refresh and filter. SODC_19023
 //   if (!bRoot)
 //       bHasElements = FALSE;
// End Comments
}

void ScDPNormalResultMember::UpdateRunningTotals( const ScDPResultMember* pRefMember, long nMeasure,
                                            ScDPRunningTotalState& rRunning, ScDPRowTotals& rTotals ) const
{
    //  IsVisible() test is in ScDPResultDimension::FillDataResults
    //  (not on data layout dimension)

    rTotals.SetInColRoot( IsRoot() );

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
                if ( bHasChild && nUserSubCount > 1 )
                {
                    aSubState.nRowSubTotalFunc = nUserPos;
                    aSubState.eRowForce = lcl_GetForceFunc( /*pParentLevel*/GetParentLevel(), nUserPos );
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

void ScDPNormalResultMember::DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const
{
    lcl_DumpRow( String::CreateFromAscii("ScDPResultMember"), GetName(), NULL, pDoc, rPos );
    SCROW nStartRow = rPos.Row();

    if (pDataRoot)
        pDataRoot->DumpState( pRefMember, pDoc, rPos );

    if (pChildDimension)
        pChildDimension->DumpState( pRefMember, pDoc, rPos );

    lcl_Indent( pDoc, nStartRow, rPos );
}

ScDPAggData* ScDPNormalResultMember::GetColTotal( long nMeasure ) const
{
    return lcl_GetChildTotal( const_cast<ScDPAggData*>(&aColTotal), nMeasure );
}

void ScDPNormalResultMember::FillVisibilityData(ScDPResultVisibilityData& rData) const
{
    if (pChildDimension)
        pChildDimension->FillVisibilityData(rData);
}
// Wang Xu Ming -- 2009-6-10
// DataPilot Migration
SCROW ScDPNormalResultMember::GetDataId( ) const
{
// TODO:
 const ScDPMember*   pMemberDesc = GetDPMember();
  if (pMemberDesc)
        return  pMemberDesc->GetItemDataId();
    return -1;
}

// -----------------------------------------------------------------------
ScDPHideDetailsMember:: ScDPHideDetailsMember(  const ScDPResultData* pData,  const ScDPParentDimData& rParentDimData,
                                              BOOL bForceSub ):ScDPResultMember(pData,rParentDimData, bForceSub)
{
    pOrigMember = new ScDPNormalResultMember(pData,rParentDimData, bForceSub);

}