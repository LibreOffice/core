/*************************************************************************
 *
 *  $RCSfile: unocrsr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
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
    : SwCursor( rPos, pRing ), SwModify( 0 ),
    bRemainInSection( TRUE )
{}

SwUnoCrsr::SwUnoCrsr( SwUnoCrsr& rICrsr )
    : SwCursor( rICrsr ), SwModify( 0 ),
    bRemainInSection( rICrsr.bRemainInSection )
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

SwUnoCrsr::operator SwUnoCrsr* ()   { return this; }

/*

SwCursor* SwUnoCrsr::Create( SwPaM* pRing ) const
{
    return new SwUnoCrsr( *GetPoint(), pRing );
}

*/

FASTBOOL SwUnoCrsr::IsSelOvr( int eFlags )
{
    if( bRemainInSection )
    {
        SwDoc* pDoc = GetDoc();
        SwNodeIndex aOldIdx( *pDoc->GetNodes()[ GetSavePos()->nNode ] );
        SwNodeIndex& rPtIdx = GetPoint()->nNode;
        SwStartNode *pOldSttNd = aOldIdx.GetNode().FindStartNode(),
                    *pNewSttNd = rPtIdx.GetNode().FindStartNode();
        if( pOldSttNd != pNewSttNd )
        {
            BOOL bMoveDown = GetSavePos()->nNode < rPtIdx.GetIndex();
            BOOL bValidPos = FALSE;
            if( bMoveDown )
            {
                // ist das Ende noch nicht erreicht worden?
                while( pOldSttNd->EndOfSectionIndex() > rPtIdx.GetIndex() )
                {
                    // dann versuche auf die "Ebene" zurueck zukommen
                    rPtIdx.Assign( *pNewSttNd->EndOfSectionNode(), 1 );
                    while( pOldSttNd != rPtIdx.GetNode().FindStartNode() )
                        rPtIdx.Assign( *rPtIdx.GetNode().EndOfSectionNode(), 1 );

                    if( !rPtIdx.GetNode().IsCntntNode() &&
                        !pDoc->GetNodes().GoNextSection( &rPtIdx ))
                        break;

                    if( pOldSttNd ==
                        ( pNewSttNd = rPtIdx.GetNode().FindStartNode() ))
                    {
                        // das ist die gesuchte Position
                        bValidPos = TRUE;
                        break;
                    }
                }
            }
            else
            {
                // ist der Start noch nicht erreicht worden?
                while( pOldSttNd->GetIndex() < rPtIdx.GetIndex() )
                {
                    // dann versuche auf die "Ebene" zurueck zukommen
                    rPtIdx.Assign( *pNewSttNd, -1 );
                    while( pOldSttNd != rPtIdx.GetNode().FindStartNode() )
                        rPtIdx.Assign( *rPtIdx.GetNode().FindStartNode(), -1 );

                    if( !rPtIdx.GetNode().IsCntntNode() &&
                        !pDoc->GetNodes().GoPrevSection( &rPtIdx ))
                        break;

                    if( pOldSttNd ==
                        ( pNewSttNd = rPtIdx.GetNode().FindStartNode() ))
                    {
                        // das ist die gesuchte Position
                        bValidPos = TRUE;
                        break;
                    }
                }
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

SwUnoTableCrsr::SwUnoTableCrsr( const SwPosition& rPos )
    : SwTableCursor( rPos ), SwUnoCrsr( rPos ), SwCursor( rPos ),
    aTblSel( rPos )
{
    SetRemainInSection( FALSE );
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

FASTBOOL SwUnoTableCrsr::IsSelOvr( int eFlags )
{
    FASTBOOL bRet = SwUnoCrsr::IsSelOvr( eFlags );
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
    if( GetPoint()->nNode.GetIndex() && GetMark()->nNode.GetIndex() &&
            0 != ( pCNd = GetCntntNode() ) && pCNd->GetFrm() &&
            0 != ( pCNd = GetCntntNode(FALSE) ) && pCNd->GetFrm() )
        GetDoc()->GetRootFrm()->MakeTblCrsrs( *this );

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

/*  */

SwUnoCrsr* SwDoc::CreateUnoCrsr( const SwPosition& rPos, BOOL bTblCrsr )
{
    SwUnoCrsr* pNew;
    if( bTblCrsr )
        pNew = new SwUnoTableCrsr( rPos );
    else
        pNew = new SwUnoCrsr( rPos );

    pUnoCrsrTbl->Insert( pNew, pUnoCrsrTbl->Count() );
    return pNew;
}

