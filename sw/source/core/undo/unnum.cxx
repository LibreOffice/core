/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unnum.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-08 17:23:35 $
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


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>           // fuer die UndoIds
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif


SV_DECL_PTRARR_DEL( _SfxPoolItems, SfxPoolItem*, 16, 16 );
SV_IMPL_PTRARR( _SfxPoolItems, SfxPoolItem* );

inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

SwUndoInsNum::SwUndoInsNum( const SwNumRule& rOldRule,
                            const SwNumRule& rNewRule )
    : SwUndo( UNDO_INSNUM ),
    aNumRule( rNewRule ), pHistory( 0 ), nLRSavePos( 0 ),
    nSttSet( ULONG_MAX ), pOldNumRule( new SwNumRule( rOldRule ))
{
    ASSERT( rOldRule.IsAutoRule(),
            "darf nur fuer AutoNumRules gerufen werden" );
}

SwUndoInsNum::SwUndoInsNum( const SwPaM& rPam, const SwNumRule& rRule )
    : SwUndo( UNDO_INSNUM ), SwUndRng( rPam ),
    aNumRule( rRule ), pHistory( 0 ), nLRSavePos( 0 ),
    nSttSet( ULONG_MAX ), pOldNumRule( 0 )
{
}

SwUndoInsNum::SwUndoInsNum( const SwPosition& rPos, const SwNumRule& rRule,
                            const String& rReplaceRule )
    : SwUndo( UNDO_INSNUM ),
    aNumRule( rRule ), pHistory( 0 ), nLRSavePos( 0 ),
    nSttSet( ULONG_MAX ), pOldNumRule( 0 ),
    sReplaceRule( rReplaceRule )
{
    // keine Selektion !!
    nEndNode = 0, nEndCntnt = USHRT_MAX;
    nSttNode = rPos.nNode.GetIndex();
    nSttCntnt = rPos.nContent.GetIndex();
}

SwUndoInsNum::~SwUndoInsNum()
{
    delete pHistory;
    delete pOldNumRule;
}

void SwUndoInsNum::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    if( nSttNode )
        SetPaM( rUndoIter );

    BOOL bUndo = rDoc.DoesUndo();
    rDoc.DoUndo( FALSE );

    if( pOldNumRule )
        rDoc.ChgNumRuleFmts( *pOldNumRule );

    if( pHistory )
    {
        SwTxtNode* pNd;
        if( ULONG_MAX != nSttSet &&
            0 != ( pNd = rDoc.GetNodes()[ nSttSet ]->GetTxtNode() ))
                pNd->SetRestart( TRUE );
        else
            pNd = 0;


        if( nLRSavePos )
        {
            // sofort Updaten, damit eventuell "alte" LRSpaces wieder
            // gueltig werden.
            // !!! Dafuer suche aber erstmal den richtigen NumRule - Namen!
            if( !pNd && nSttNode )
                pNd = rDoc.GetNodes()[ nSttNode ]->GetTxtNode();

            const SwNumRule* pNdRule;
            if( pNd )
                pNdRule = pNd->GetNumRule();
            else
                pNdRule = rDoc.FindNumRulePtr( aNumRule.GetName() );

            pHistory->TmpRollback( &rDoc, nLRSavePos );

        }
        pHistory->TmpRollback( &rDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }

    if( nSttNode )
        SetPaM( rUndoIter );
    rDoc.DoUndo( bUndo );
}


void SwUndoInsNum::Redo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();

    if( pOldNumRule )
        rDoc.ChgNumRuleFmts( aNumRule );
    else if( pHistory )
    {
        SetPaM( rUndoIter );
        if( sReplaceRule.Len() )
            rDoc.ReplaceNumRule( *rUndoIter.pAktPam->GetPoint(),
                                sReplaceRule, aNumRule.GetName() );
        else
            // --> OD 2005-02-25 #i42921# - adapt to changed signature
            rDoc.SetNumRule( *rUndoIter.pAktPam, aNumRule );
            // <--
    }
}

void SwUndoInsNum::SetLRSpaceEndPos()
{
    if( pHistory )
        nLRSavePos = pHistory->Count();
}

void SwUndoInsNum::Repeat( SwUndoIter& rUndoIter )
{
    if( nSttNode )
    {
        if( !sReplaceRule.Len() )
            // --> OD 2005-02-25 #i42921# - adapt to changed signature
            rUndoIter.GetDoc().SetNumRule( *rUndoIter.pAktPam, aNumRule );
            // <--
    }
    else
        rUndoIter.GetDoc().ChgNumRuleFmts( aNumRule );
}

SwHistory* SwUndoInsNum::GetHistory()
{
    if( !pHistory )
        pHistory = new SwHistory;
    return pHistory;
}

void SwUndoInsNum::SaveOldNumRule( const SwNumRule& rOld )
{
    if( !pOldNumRule )
        pOldNumRule = new SwNumRule( rOld );
}

/*  */


SwUndoDelNum::SwUndoDelNum( const SwPaM& rPam )
    : SwUndo( UNDO_DELNUM ), SwUndRng( rPam ),
    aNodeIdx( BYTE( nEndNode - nSttNode > 255 ? 255 : nEndNode - nSttNode )),
    aLevels( BYTE( nEndNode - nSttNode > 255 ? 255 : nEndNode - nSttNode ))
#ifndef NUM_RELSPACE
    , aRstLRSpaces( BYTE( nEndNode - nSttNode > 255 ? 255 : nEndNode - nSttNode ))
#endif
{
    pHistory = new SwHistory;
}


SwUndoDelNum::~SwUndoDelNum()
{
    delete pHistory;
}


void SwUndoDelNum::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SetPaM( rUndoIter );

    BOOL bUndo = rDoc.DoesUndo();
    rDoc.DoUndo( FALSE );

    pHistory->TmpRollback( &rDoc, 0 );
    pHistory->SetTmpEnd( pHistory->Count() );

    for( USHORT n = 0; n < aNodeIdx.Count(); ++n )
    {
        SwTxtNode* pNd = rDoc.GetNodes()[ aNodeIdx[ n ] ]->GetTxtNode();
        ASSERT( pNd, "wo ist der TextNode geblieben?" );
        pNd->SetLevel(aLevels[ n ] );

#ifndef NUM_RELSPACE
        pNd->SetNumLSpace( aRstLRSpaces[ n ] );
#endif

        if( pNd->GetCondFmtColl() )
            pNd->ChkCondColl();
    }

    SetPaM( rUndoIter );
    rDoc.DoUndo( bUndo );
}


void SwUndoDelNum::Redo( SwUndoIter& rUndoIter )
{
    SetPaM( rUndoIter );
    rUndoIter.GetDoc().DelNumRules( *rUndoIter.pAktPam );
}


void SwUndoDelNum::Repeat( SwUndoIter& rUndoIter )
{
    SetPaM( rUndoIter );
    rUndoIter.GetDoc().DelNumRules( *rUndoIter.pAktPam );
}

void SwUndoDelNum::AddNode( const SwTxtNode& rNd, BOOL bFlag )
{
    if( rNd.GetNumRule() )
    {
        register USHORT nIns = aNodeIdx.Count();
        aNodeIdx.Insert( rNd.GetIndex(), nIns );
        aLevels.Insert( rNd.GetLevel(), nIns );
#ifndef NUM_RELSPACE
        aRstLRSpaces.Insert( bFlag, nIns );
#endif
    }
}


/*  */


SwUndoMoveNum::SwUndoMoveNum( const SwPaM& rPam, long nOff, BOOL bIsOutlMv )
    : SwUndo( bIsOutlMv ? UNDO_OUTLINE_UD : UNDO_MOVENUM ),
    SwUndRng( rPam ), nOffset( nOff ),
    nNewStt( 0 )
{
    // nOffset: nach unten  =>  1
    //          nach oben   => -1
}


void SwUndoMoveNum::Undo( SwUndoIter& rUndoIter )
{
    ULONG nTmpStt = nSttNode, nTmpEnd = nEndNode;

    if( nEndNode || USHRT_MAX != nEndCntnt )        // Bereich ?
    {
        if( nNewStt < nSttNode )        // nach vorne verschoben
            nEndNode = nEndNode - ( nSttNode - nNewStt );
        else
            nEndNode = nEndNode + ( nNewStt - nSttNode );
    }
    nSttNode = nNewStt;

//JP 22.06.95: wird wollen die Bookmarks/Verzeichnisse behalten, oder?
//  SetPaM( rUndoIter );
//  RemoveIdxFromRange( *rUndoIter.pAktPam, TRUE );

    SetPaM( rUndoIter );
    rUndoIter.GetDoc().MoveParagraph( *rUndoIter.pAktPam, -nOffset,
                                        UNDO_OUTLINE_UD == GetId() );
    nSttNode = nTmpStt;
    nEndNode = nTmpEnd;
}


void SwUndoMoveNum::Redo( SwUndoIter& rUndoIter )
{
//JP 22.06.95: wird wollen die Bookmarks/Verzeichnisse behalten, oder?
//  SetPaM( rUndoIter );
//  RemoveIdxFromRange( *rUndoIter.pAktPam, TRUE );

    SetPaM( rUndoIter );
    rUndoIter.GetDoc().MoveParagraph( *rUndoIter.pAktPam, nOffset,
                                        UNDO_OUTLINE_UD == GetId() );
}


void SwUndoMoveNum::Repeat( SwUndoIter& rUndoIter )
{
    if( UNDO_OUTLINE_UD == GetId() )
        rUndoIter.GetDoc().MoveOutlinePara( *rUndoIter.pAktPam,
                                            0 < nOffset ? 1 : -1 );
    else
        rUndoIter.GetDoc().MoveParagraph( *rUndoIter.pAktPam, nOffset, FALSE );
}

/*  */


SwUndoNumUpDown::SwUndoNumUpDown( const SwPaM& rPam, short nOff )
    : SwUndo( nOff > 0 ? UNDO_NUMUP : UNDO_NUMDOWN ), SwUndRng( rPam ),
      nOffset( nOff )
{
    // nOffset: Down    =>  1
    //          Up      => -1
}


void SwUndoNumUpDown::Undo( SwUndoIter& rUndoIter )
{
    SetPaM( rUndoIter );
    rUndoIter.GetDoc().NumUpDown( *rUndoIter.pAktPam, 1 != nOffset );
}


void SwUndoNumUpDown::Redo( SwUndoIter& rUndoIter )
{
    SetPaM( rUndoIter );
    rUndoIter.GetDoc().NumUpDown( *rUndoIter.pAktPam, 1 == nOffset );
}


void SwUndoNumUpDown::Repeat( SwUndoIter& rUndoIter )
{
    rUndoIter.GetDoc().NumUpDown( *rUndoIter.pAktPam, 1 == nOffset );
}

/*  */

// #115901#
SwUndoNumOrNoNum::SwUndoNumOrNoNum( const SwNodeIndex& rIdx, BOOL bOldNum,
                                    BOOL bNewNum)
    : SwUndo( UNDO_NUMORNONUM ), nIdx( rIdx.GetIndex() ), mbNewNum(bNewNum),
      mbOldNum(bOldNum)
{
}

// #115901#, #i40034#
void SwUndoNumOrNoNum::Undo( SwUndoIter& rUndoIter )
{
    SwNodeIndex aIdx( rUndoIter.GetDoc().GetNodes(), nIdx );
    SwTxtNode * pTxtNd = aIdx.GetNode().GetTxtNode();

    if (NULL != pTxtNd)
    {
        pTxtNd->SetCounted(mbOldNum);
    }
}

// #115901#, #i40034#
void SwUndoNumOrNoNum::Redo( SwUndoIter& rUndoIter )
{
    SwNodeIndex aIdx( rUndoIter.GetDoc().GetNodes(), nIdx );
    SwTxtNode * pTxtNd = aIdx.GetNode().GetTxtNode();

    if (NULL != pTxtNd)
    {
        pTxtNd->SetCounted(mbNewNum);
    }
}

// #115901#
void SwUndoNumOrNoNum::Repeat( SwUndoIter& rUndoIter )
{

    if (mbOldNum && ! mbNewNum)
        rUndoIter.GetDoc().NumOrNoNum( rUndoIter.pAktPam->GetPoint()->nNode,
                                       FALSE);
    else if ( ! mbOldNum && mbNewNum )
        rUndoIter.GetDoc().NumOrNoNum( rUndoIter.pAktPam->GetPoint()->nNode,
                                       TRUE);
}

/*  */

SwUndoNumRuleStart::SwUndoNumRuleStart( const SwPosition& rPos, BOOL bFlg )
    : SwUndo( UNDO_SETNUMRULESTART ),
    nIdx( rPos.nNode.GetIndex() ), bFlag( bFlg ), bSetSttValue( FALSE ),
    nNewStt( USHRT_MAX ), nOldStt( USHRT_MAX )
{
}

SwUndoNumRuleStart::SwUndoNumRuleStart( const SwPosition& rPos, USHORT nStt )
    : SwUndo( UNDO_SETNUMRULESTART ),
    nIdx( rPos.nNode.GetIndex() ), bSetSttValue( TRUE ),
    nNewStt( nStt ), nOldStt( USHRT_MAX )
{
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd )
        nOldStt = pTxtNd->GetStart();
}


void SwUndoNumRuleStart::Undo( SwUndoIter& rUndoIter )
{
    SwPosition aPos( *rUndoIter.GetDoc().GetNodes()[ nIdx ] );
    if( bSetSttValue )
        rUndoIter.GetDoc().SetNodeNumStart( aPos, nOldStt );
    else
        rUndoIter.GetDoc().SetNumRuleStart( aPos, !bFlag );
}


void SwUndoNumRuleStart::Redo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();

    SwPosition aPos( *rDoc.GetNodes()[ nIdx ] );
    if( bSetSttValue )
        rDoc.SetNodeNumStart( aPos, nNewStt );
    else
        rDoc.SetNumRuleStart( aPos, bFlag );
}


void SwUndoNumRuleStart::Repeat( SwUndoIter& rUndoIter )
{
    if( bSetSttValue )
        rUndoIter.GetDoc().SetNodeNumStart( *rUndoIter.pAktPam->GetPoint(), nNewStt );
    else
        rUndoIter.GetDoc().SetNumRuleStart( *rUndoIter.pAktPam->GetPoint(), bFlag );
}


