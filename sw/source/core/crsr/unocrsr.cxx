/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
        
        SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        if( !rTbl.erase( this ) )
        {
            OSL_ENSURE( !this, "UNO Cursor nicht mehr im Array" );
        }
    }

    
    while( GetNext() != this )
    {
        Ring* pNxt = GetNext();
        pNxt->MoveTo( 0 ); 
        delete pNxt;       
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
            *GetPoint(), true /* create SwUnoTableCrsr */ ) );
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
    return 0; 
}

void SwUnoCrsr::DoSetBidiLevelUpDown()
{
    return; 
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

            
            
            while( pOldSttNd->IsSectionNode() )
                pOldSttNd = pOldSttNd->StartOfSectionNode();

            
            if( rPtIdx > *pOldSttNd &&
                rPtIdx < pOldSttNd->EndOfSectionIndex() )
            {
                
                
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
                GetPoint()->nContent.Assign( pCNd, (pCNd && !bMoveDown) ? pCNd->Len() : 0);
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
            0 != ( pCNd = GetCntntNode(false) ) && pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout() ) )
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
