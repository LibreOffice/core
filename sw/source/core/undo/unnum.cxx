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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>
#include <editeng/lrspitem.hxx>
#include <doc.hxx>
#include <swundo.hxx>           // fuer die UndoIds
#include <pam.hxx>
#include <ndtxt.hxx>
#include <undobj.hxx>
#include <rolbck.hxx>


SV_DECL_PTRARR_DEL( _SfxPoolItems, SfxPoolItem*, 16, 16 )
SV_IMPL_PTRARR( _SfxPoolItems, SfxPoolItem* );

inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

SwUndoInsNum::SwUndoInsNum( const SwNumRule& rOldRule,
                            const SwNumRule& rNewRule,
                            SwUndoId nUndoId )
    : SwUndo( nUndoId ),
    aNumRule( rNewRule ), pHistory( 0 ), nSttSet( ULONG_MAX ),
    pOldNumRule( new SwNumRule( rOldRule )), nLRSavePos( 0 )
{
}

SwUndoInsNum::SwUndoInsNum( const SwPaM& rPam, const SwNumRule& rRule )
    : SwUndo( UNDO_INSNUM ), SwUndRng( rPam ),
    aNumRule( rRule ), pHistory( 0 ),
    nSttSet( ULONG_MAX ), pOldNumRule( 0 ), nLRSavePos( 0 )
{
}

SwUndoInsNum::SwUndoInsNum( const SwPosition& rPos, const SwNumRule& rRule,
                            const String& rReplaceRule )
    : SwUndo( UNDO_INSNUM ),
    aNumRule( rRule ), pHistory( 0 ),
    nSttSet( ULONG_MAX ), pOldNumRule( 0 ),
    sReplaceRule( rReplaceRule ), nLRSavePos( 0 )
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

SwRewriter SwUndoInsNum::GetRewriter() const
{
    SwRewriter aResult;
    if( UNDO_INSFMTATTR == GetId() )
        aResult.AddRule(UNDO_ARG1, aNumRule.GetName());
    return aResult;
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
                pNd->SetListRestart( TRUE );
        else
            pNd = 0;


        if( nLRSavePos )
        {
            // sofort Updaten, damit eventuell "alte" LRSpaces wieder
            // gueltig werden.
            // !!! Dafuer suche aber erstmal den richtigen NumRule - Namen!
            if( !pNd && nSttNode )
                pNd = rDoc.GetNodes()[ nSttNode ]->GetTxtNode();

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
        {
            // --> OD 2005-02-25 #i42921# - adapt to changed signature
            rDoc.SetNumRule( *rUndoIter.pAktPam, aNumRule, false );
            // <--
        }
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
        {
            // --> OD 2005-02-25 #i42921# - adapt to changed signature
            rUndoIter.GetDoc().SetNumRule( *rUndoIter.pAktPam, aNumRule, false );
            // <--
        }
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
    : SwUndo( UNDO_DELNUM ), SwUndRng( rPam )
{
    aNodes.reserve( nEndNode - nSttNode > 255 ? 255 : nEndNode - nSttNode );
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

    for( SvNode::const_iterator i = aNodes.begin(); i != aNodes.end(); ++i )
    {
        SwTxtNode* pNd = rDoc.GetNodes()[ i->first ]->GetTxtNode();
        OSL_ENSURE( pNd, "wo ist der TextNode geblieben?" );
        pNd->SetAttrListLevel( i->second );

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

void SwUndoDelNum::AddNode( const SwTxtNode& rNd, BOOL )
{
    if( rNd.GetNumRule() )
    {
        aNodes.push_back( SvNode::value_type( rNd.GetIndex(), rNd.GetActualListLevel() ) );
    }
}


/*  */


SwUndoMoveNum::SwUndoMoveNum( const SwPaM& rPam, long nOff, BOOL bIsOutlMv )
    : SwUndo( bIsOutlMv ? UNDO_OUTLINE_UD : UNDO_MOVENUM ),
    SwUndRng( rPam ),
    nNewStt( 0 ), nOffset( nOff )
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
        pTxtNd->SetCountedInList(mbOldNum);
    }
}

// #115901#, #i40034#
void SwUndoNumOrNoNum::Redo( SwUndoIter& rUndoIter )
{
    SwNodeIndex aIdx( rUndoIter.GetDoc().GetNodes(), nIdx );
    SwTxtNode * pTxtNd = aIdx.GetNode().GetTxtNode();

    if (NULL != pTxtNd)
    {
        pTxtNd->SetCountedInList(mbNewNum);
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
    nIdx( rPos.nNode.GetIndex() ), nOldStt( USHRT_MAX ),
    nNewStt( USHRT_MAX ), bSetSttValue( FALSE ), bFlag( bFlg )
{
}

SwUndoNumRuleStart::SwUndoNumRuleStart( const SwPosition& rPos, USHORT nStt )
    : SwUndo( UNDO_SETNUMRULESTART ),
    nIdx( rPos.nNode.GetIndex() ),
    nOldStt( USHRT_MAX ), nNewStt( nStt ), bSetSttValue( TRUE )
{
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if ( pTxtNd )
    {
        if ( pTxtNd->HasAttrListRestartValue() )
        {
            nOldStt = static_cast<USHORT>(pTxtNd->GetAttrListRestartValue());
        }
        else
        {
            nOldStt = USHRT_MAX; // indicating, that the list restart value is not set
        }
    }
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
