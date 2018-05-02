/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <unocrsr.hxx>
#include <doc.hxx>
#include <swtable.hxx>
#include <docary.hxx>
#include <rootfrm.hxx>

SV_IMPL_PTRARR( SwUnoCrsrTbl, SwUnoCrsrPtr )

IMPL_FIXEDMEMPOOL_NEWDEL( SwUnoCrsr, 10, 10 )

SwUnoCrsr::SwUnoCrsr( const SwPosition &rPos, SwPaM* pRing )
    : SwCursor( rPos, pRing, false ), SwModify( 0 ),
    bRemainInSection( sal_True ),
    bSkipOverHiddenSections( sal_False ),
    bSkipOverProtectSections( sal_False )

{}

SwUnoCrsr::~SwUnoCrsr()
{
    SwDoc* pDoc = GetDoc();
    if( !pDoc->IsInDtor() )
    {
        // dann muss der Cursor aus dem Array ausgetragen werden
        SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        sal_uInt16 nDelPos = rTbl.GetPos( this );

        if( USHRT_MAX != nDelPos )
            rTbl.Remove( nDelPos );
        else {
            ASSERT( sal_False, "UNO cursor no longer in the array" );
        }
    }

    // den gesamten Ring loeschen!
    while( GetNext() != this )
    {
        Ring* pNxt = GetNext();
        pNxt->MoveTo( 0 );      // ausketten
        delete pNxt;            // und loeschen
    }
}

SwUnoCrsr * SwUnoCrsr::Clone() const
{
    SwUnoCrsr * pNewCrsr = GetDoc()->CreateUnoCrsr( *GetPoint() );
    if (HasMark())
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *GetMark();
    }
    return pNewCrsr;
}

SwUnoTableCrsr * SwUnoTableCrsr::Clone() const
{
    SwUnoTableCrsr * pNewCrsr = dynamic_cast<SwUnoTableCrsr*>(
        GetDoc()->CreateUnoCrsr(
            *GetPoint(), sal_True /* create SwUnoTableCrsr */ ) );
    OSL_ENSURE(pNewCrsr, "Clone: cannot create SwUnoTableCrsr?");
    if (HasMark())
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *GetMark();
    }
    return pNewCrsr;
}


bool SwUnoCrsr::IsReadOnlyAvailable() const
{
    return true;
}

const SwCntntFrm*
SwUnoCrsr::DoSetBidiLevelLeftRight( sal_Bool &, sal_Bool, sal_Bool )
{
    return 0; // not for uno cursor
}

void SwUnoCrsr::DoSetBidiLevelUpDown()
{
    return; // not for uno cursor
}

sal_Bool SwUnoCrsr::IsSelOvr( int eFlags )
{
    if( bRemainInSection )
    {
        SwDoc* pDoc = GetDoc();
        SwNodeIndex aOldIdx( *pDoc->GetNodes()[ GetSavePos()->nNode ] );
        SwNodeIndex& rPtIdx = GetPoint()->nNode;
        SwStartNode *pOldSttNd = aOldIdx.GetNode().StartOfSectionNode(),
                    *pNewSttNd = rPtIdx.GetNode().StartOfSectionNode();
        if( pOldSttNd != pNewSttNd )
        {
            sal_Bool bMoveDown = GetSavePos()->nNode < rPtIdx.GetIndex();
            sal_Bool bValidPos = sal_False;

            // search the correct surrounded start node - which the index
            // can't leave.
            while( pOldSttNd->IsSectionNode() )
                pOldSttNd = pOldSttNd->StartOfSectionNode();

            // is the new index inside this surrounded section?
            if( rPtIdx > *pOldSttNd &&
                rPtIdx < pOldSttNd->EndOfSectionIndex() )
            {
                // check if it a valid move inside this section
                // (only over SwSection's !)
                const SwStartNode* pInvalidNode;
                do {
                    pInvalidNode = 0;
                    pNewSttNd = rPtIdx.GetNode().StartOfSectionNode();

                    const SwStartNode *pSttNd = pNewSttNd, *pEndNd = pOldSttNd;
                    if( pSttNd->EndOfSectionIndex() >
                        pEndNd->EndOfSectionIndex() )
                    {
                        pEndNd = pNewSttNd;
                        pSttNd = pOldSttNd;
                    }

                    while( pSttNd->GetIndex() > pEndNd->GetIndex() )
                    {
                        if( !pSttNd->IsSectionNode() )
                            pInvalidNode = pSttNd;
                        pSttNd = pSttNd->StartOfSectionNode();
                    }
                    if( pInvalidNode )
                    {
                        if( bMoveDown )
                        {
                            rPtIdx.Assign( *pInvalidNode->EndOfSectionNode(), 1 );

                            if( !rPtIdx.GetNode().IsCntntNode() &&
                                ( !pDoc->GetNodes().GoNextSection( &rPtIdx ) ||
                                  rPtIdx > pOldSttNd->EndOfSectionIndex() ) )
                                break;
                        }
                        else
                        {
                            rPtIdx.Assign( *pInvalidNode, -1 );

                            if( !rPtIdx.GetNode().IsCntntNode() &&
                                ( !pDoc->GetNodes().GoPrevSection( &rPtIdx ) ||
                                  rPtIdx < *pOldSttNd ) )
                                break;
                        }
                    }
                    else
                        bValidPos = sal_True;
                } while ( pInvalidNode );
            }

            if( bValidPos )
            {
                SwCntntNode* pCNd = GetCntntNode();
                sal_uInt16 nCnt = 0;
                if( pCNd && !bMoveDown )
                    nCnt = pCNd->Len();
                GetPoint()->nContent.Assign( pCNd, nCnt );
            }
            else
            {
                rPtIdx = GetSavePos()->nNode;
                GetPoint()->nContent.Assign( GetCntntNode(), GetSavePos()->nCntnt );
                return sal_True;
            }
        }
    }
    return SwCursor::IsSelOvr( eFlags );
}


/*  */

SwUnoTableCrsr::SwUnoTableCrsr(const SwPosition& rPos)
    : SwCursor(rPos,0,false), SwUnoCrsr(rPos), SwTableCursor(rPos), aTblSel(rPos,0,false)
{
    SetRemainInSection(sal_False);
}

SwUnoTableCrsr::~SwUnoTableCrsr()
{
    while( aTblSel.GetNext() != &aTblSel )
        delete aTblSel.GetNext();           // und loeschen
}


/*
SwCursor* SwUnoTableCrsr::Create( SwPaM* pRing ) const
{
    return SwUnoCrsr::Create( pRing );
}
*/

sal_Bool SwUnoTableCrsr::IsSelOvr( int eFlags )
{
    sal_Bool bRet = SwUnoCrsr::IsSelOvr( eFlags );
    if( !bRet )
    {
        const SwTableNode* pTNd = GetPoint()->nNode.GetNode().FindTableNode();
        bRet = !(pTNd == GetDoc()->GetNodes()[ GetSavePos()->nNode ]->
                FindTableNode() && (!HasMark() ||
                pTNd == GetMark()->nNode.GetNode().FindTableNode() ));
    }
    return bRet;
}

void SwUnoTableCrsr::MakeBoxSels()
{
    const SwCntntNode* pCNd;
    bool bMakeTblCrsrs = true;
    if( GetPoint()->nNode.GetIndex() && GetMark()->nNode.GetIndex() &&
            0 != ( pCNd = GetCntntNode() ) && pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout() ) &&
            0 != ( pCNd = GetCntntNode(sal_False) ) && pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout() ) )
        bMakeTblCrsrs = GetDoc()->GetCurrentLayout()->MakeTblCrsrs( *this );

    if ( !bMakeTblCrsrs )
    {
        SwSelBoxes& rTmpBoxes = (SwSelBoxes&)GetBoxes();
        sal_uInt16 nCount = 0;
        while( nCount < rTmpBoxes.Count() )
            DeleteBox( nCount );
    }

    if( IsChgd() )
    {
        SwTableCursor::MakeBoxSels( &aTblSel );
        if( !GetBoxesCount() )
        {
            const SwTableBox* pBox;
            const SwNode* pBoxNd = GetPoint()->nNode.GetNode().FindTableBoxStartNode();
            const SwTableNode* pTblNd = pBoxNd ? pBoxNd->FindTableNode() : 0;
            if( pTblNd && 0 != ( pBox = pTblNd->GetTable().GetTblBox( pBoxNd->GetIndex() )) )
                InsertBox( *pBox );
        }
    }
}

