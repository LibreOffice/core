/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unocrsr.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:31:42 $
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
#include "precompiled_sw.hxx"


#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif

SV_IMPL_PTRARR( SwUnoCrsrTbl, SwUnoCrsrPtr )

IMPL_FIXEDMEMPOOL_NEWDEL( SwUnoCrsr, 10, 10 )

SwUnoCrsr::SwUnoCrsr( const SwPosition &rPos, SwPaM* pRing )
    : SwCursor( rPos, pRing, false ), SwModify( 0 ),
    bRemainInSection( TRUE ),
    bSkipOverHiddenSections( FALSE ),
    bSkipOverProtectSections( FALSE )

{}

// @@@ semantic: no copy ctor.
SwUnoCrsr::SwUnoCrsr( SwUnoCrsr& rICrsr )
    : SwCursor( rICrsr ), SwModify( 0 ),
    bRemainInSection( rICrsr.bRemainInSection ),
    bSkipOverHiddenSections( rICrsr.bSkipOverHiddenSections ),
    bSkipOverProtectSections( rICrsr.bSkipOverProtectSections )
{}

SwUnoCrsr::~SwUnoCrsr()
{
    SwDoc* pDoc = GetDoc();
    if( !pDoc->IsInDtor() )
    {
        // dann muss der Cursor aus dem Array ausgetragen werden
        SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        USHORT nDelPos = rTbl.GetPos( this );

        if( USHRT_MAX != nDelPos )
            rTbl.Remove( nDelPos );
        else
            ASSERT( !this, "UNO Cursor nicht mehr im Array" );
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
    SwUnoCrsr * pNewCrsr = 0;

    // check if the cursor is a SwUnoTableCrsr, if so clone that type
    const SwUnoTableCrsr * pUnoTableCrsr = *this;
    if (pUnoTableCrsr)
        pNewCrsr = GetDoc()->CreateUnoCrsr( *GetPoint(), sal_True /* create SwUnoTableCrsr */ );
    else
        pNewCrsr = GetDoc()->CreateUnoCrsr( *GetPoint() );
    if (HasMark())
    {
        pNewCrsr->SetMark();
        *pNewCrsr->GetMark() = *GetMark();
    }
    return pNewCrsr;
}

SwUnoCrsr::operator SwUnoCrsr* ()   { return this; }


BOOL SwUnoCrsr::IsSelOvr( int eFlags )
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
            BOOL bMoveDown = GetSavePos()->nNode < rPtIdx.GetIndex();
            BOOL bValidPos = FALSE;

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
                                !pDoc->GetNodes().GoNextSection( &rPtIdx ))
                                break;
                        }
                        else
                        {
                            rPtIdx.Assign( *pInvalidNode, -1 );

                            if( !rPtIdx.GetNode().IsCntntNode() &&
                                !pDoc->GetNodes().GoPrevSection( &rPtIdx ))
                                break;
                        }
                    }
                    else
                        bValidPos = TRUE;
                } while ( pInvalidNode );
            }

            if( bValidPos )
            {
                SwCntntNode* pCNd = GetCntntNode();
                USHORT nCnt = 0;
                if( pCNd && !bMoveDown )
                    nCnt = pCNd->Len();
                GetPoint()->nContent.Assign( pCNd, nCnt );
            }
            else
            {
                rPtIdx = GetSavePos()->nNode;
                GetPoint()->nContent.Assign( GetCntntNode(), GetSavePos()->nCntnt );
                return TRUE;
            }
        }
    }
    return SwCursor::IsSelOvr( eFlags );
}


/*  */

SwUnoTableCrsr::SwUnoTableCrsr(const SwPosition& rPos)
    : SwCursor(rPos,0,false), SwUnoCrsr(rPos), SwTableCursor(rPos), aTblSel(rPos,0,false)
{
    SetRemainInSection(FALSE);
}

SwUnoTableCrsr::~SwUnoTableCrsr()
{
    while( aTblSel.GetNext() != &aTblSel )
        delete aTblSel.GetNext();           // und loeschen
}

SwUnoTableCrsr::operator SwUnoCrsr* ()      { return this; }
SwUnoTableCrsr::operator SwTableCursor* ()  { return this; }
SwUnoTableCrsr::operator SwUnoTableCrsr* () { return this; }

/*
SwCursor* SwUnoTableCrsr::Create( SwPaM* pRing ) const
{
    return SwUnoCrsr::Create( pRing );
}
*/

BOOL SwUnoTableCrsr::IsSelOvr( int eFlags )
{
    BOOL bRet = SwUnoCrsr::IsSelOvr( eFlags );
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
            0 != ( pCNd = GetCntntNode() ) && pCNd->GetFrm() &&
            0 != ( pCNd = GetCntntNode(FALSE) ) && pCNd->GetFrm() )
        bMakeTblCrsrs = GetDoc()->GetRootFrm()->MakeTblCrsrs( *this );

    if ( !bMakeTblCrsrs )
    {
        SwSelBoxes& rTmpBoxes = (SwSelBoxes&)GetBoxes();
        USHORT nCount = 0;
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

