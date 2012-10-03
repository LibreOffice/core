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

#include <unocrsr.hxx>
#include <doc.hxx>
#include <swtable.hxx>
#include <docary.hxx>
#include <rootfrm.hxx>

IMPL_FIXEDMEMPOOL_NEWDEL( SwUnoCrsr )

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
        // then remove cursor from array
        SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        if( !rTbl.erase( this ) )
        {
            OSL_ENSURE( !this, "UNO Cursor nicht mehr im Array" );
        }
    }

    // delete the whole ring
    while( GetNext() != this )
    {
        Ring* pNxt = GetNext();
        pNxt->MoveTo( 0 ); // remove from chain
        delete pNxt;       // and delete
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
            bool bMoveDown = GetSavePos()->nNode < rPtIdx.GetIndex();
            bool bValidPos = false;

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
                        bValidPos = true;
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



SwUnoTableCrsr::SwUnoTableCrsr(const SwPosition& rPos)
    : SwCursor(rPos,0,false), SwUnoCrsr(rPos), SwTableCursor(rPos), aTblSel(rPos,0,false)
{
    SetRemainInSection(sal_False);
}

SwUnoTableCrsr::~SwUnoTableCrsr()
{
    while( aTblSel.GetNext() != &aTblSel )
        delete aTblSel.GetNext();
}

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
        SwSelBoxes const& rTmpBoxes = GetSelectedBoxes();
        while (!rTmpBoxes.empty())
        {
            DeleteBox(0);
        }
    }

    if( IsChgd() )
    {
        SwTableCursor::MakeBoxSels( &aTblSel );
        if (!GetSelectedBoxesCount())
        {
            const SwTableBox* pBox;
            const SwNode* pBoxNd = GetPoint()->nNode.GetNode().FindTableBoxStartNode();
            const SwTableNode* pTblNd = pBoxNd ? pBoxNd->FindTableNode() : 0;
            if( pTblNd && 0 != ( pBox = pTblNd->GetTable().GetTblBox( pBoxNd->GetIndex() )) )
                InsertBox( *pBox );
        }
    }
}

SwUnoCrsrTbl::~SwUnoCrsrTbl()
{
    while (!empty())
    {
        delete *begin();
        erase( begin() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
