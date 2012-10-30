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

#include <UndoNumbering.hxx>
#include <hintids.hxx>
#include <editeng/lrspitem.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>

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
    // No selection!
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
        aResult.AddRule(UndoArg1, aNumRule.GetName());
    return aResult;
}

void SwUndoInsNum::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    if( pOldNumRule )
        rDoc.ChgNumRuleFmts( *pOldNumRule );

    if( pHistory )
    {
        SwTxtNode* pNd;
        if( ULONG_MAX != nSttSet &&
            0 != ( pNd = rDoc.GetNodes()[ nSttSet ]->GetTxtNode() ))
                pNd->SetListRestart( sal_True );
        else
            pNd = 0;


        if( nLRSavePos )
        {
            // Update immediately so that potential "old" LRSpaces will be valid again.
            // For that search firstly the correct NumRule names!
            if( !pNd && nSttNode )
                pNd = rDoc.GetNodes()[ nSttNode ]->GetTxtNode();

            pHistory->TmpRollback( &rDoc, nLRSavePos );

        }
        pHistory->TmpRollback( &rDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }

    if (nSttNode)
    {
        AddUndoRedoPaM(rContext);
    }
}

void SwUndoInsNum::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    if( pOldNumRule )
        rDoc.ChgNumRuleFmts( aNumRule );
    else if( pHistory )
    {
        SwPaM & rPam( AddUndoRedoPaM(rContext) );
        if( sReplaceRule.Len() )
        {
            rDoc.ReplaceNumRule(*rPam.GetPoint(),
                                sReplaceRule, aNumRule.GetName() );
        }
        else
        {
            // #i42921# - adapt to changed signature
            rDoc.SetNumRule(rPam, aNumRule, false);
        }
    }
}

void SwUndoInsNum::SetLRSpaceEndPos()
{
    if( pHistory )
        nLRSavePos = pHistory->Count();
}

void SwUndoInsNum::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    if( nSttNode )
    {
        if( !sReplaceRule.Len() )
        {
            // #i42921# - adapt to changed signature
            rDoc.SetNumRule(rContext.GetRepeatPaM(), aNumRule, false);
        }
    }
    else
    {
        rDoc.ChgNumRuleFmts( aNumRule );
    }
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

void SwUndoDelNum::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    pHistory->TmpRollback( &rDoc, 0 );
    pHistory->SetTmpEnd( pHistory->Count() );

    for( std::vector<NodeLevel>::const_iterator i = aNodes.begin(); i != aNodes.end(); ++i )
    {
        SwTxtNode* pNd = rDoc.GetNodes()[ i->index ]->GetTxtNode();
        OSL_ENSURE( pNd, "Where has the TextNode gone?" );
        pNd->SetAttrListLevel( i->level );

        if( pNd->GetCondFmtColl() )
            pNd->ChkCondColl();
    }

    AddUndoRedoPaM(rContext);
}

void SwUndoDelNum::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    rContext.GetDoc().DelNumRules(rPam);
}

void SwUndoDelNum::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().DelNumRules(rContext.GetRepeatPaM());
}

void SwUndoDelNum::AddNode( const SwTxtNode& rNd, sal_Bool )
{
    if( rNd.GetNumRule() )
    {
        aNodes.push_back( NodeLevel( rNd.GetIndex(), rNd.GetActualListLevel() ) );
    }
}

SwUndoMoveNum::SwUndoMoveNum( const SwPaM& rPam, long nOff, bool bIsOutlMv )
    : SwUndo( bIsOutlMv ? UNDO_OUTLINE_UD : UNDO_MOVENUM ),
    SwUndRng( rPam ),
    nNewStt( 0 ), nOffset( nOff )
{
    // nOffset: Down    =>  1
    //          Up      => -1
}

void SwUndoMoveNum::UndoImpl(::sw::UndoRedoContext & rContext)
{
    sal_uLong nTmpStt = nSttNode, nTmpEnd = nEndNode;

    if( nEndNode || USHRT_MAX != nEndCntnt )        // section?
    {
        if( nNewStt < nSttNode )        // moved forwards
            nEndNode = nEndNode - ( nSttNode - nNewStt );
        else
            nEndNode = nEndNode + ( nNewStt - nSttNode );
    }
    nSttNode = nNewStt;

    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    rContext.GetDoc().MoveParagraph( rPam, -nOffset,
                                        UNDO_OUTLINE_UD == GetId() );
    nSttNode = nTmpStt;
    nEndNode = nTmpEnd;
}

void SwUndoMoveNum::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    rContext.GetDoc().MoveParagraph(rPam, nOffset, UNDO_OUTLINE_UD == GetId());
}

void SwUndoMoveNum::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if( UNDO_OUTLINE_UD == GetId() )
    {
        rDoc.MoveOutlinePara(rContext.GetRepeatPaM(),
                                            0 < nOffset ? 1 : -1 );
    }
    else
    {
        rDoc.MoveParagraph(rContext.GetRepeatPaM(), nOffset, false);
    }
}

SwUndoNumUpDown::SwUndoNumUpDown( const SwPaM& rPam, short nOff )
    : SwUndo( nOff > 0 ? UNDO_NUMUP : UNDO_NUMDOWN ), SwUndRng( rPam ),
      nOffset( nOff )
{
    // nOffset: Down    =>  1
    //          Up      => -1
}

void SwUndoNumUpDown::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    rContext.GetDoc().NumUpDown(rPam, 1 != nOffset );
}

void SwUndoNumUpDown::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    rContext.GetDoc().NumUpDown(rPam, 1 == nOffset);
}

void SwUndoNumUpDown::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().NumUpDown(rContext.GetRepeatPaM(), 1 == nOffset);
}

// #115901#
SwUndoNumOrNoNum::SwUndoNumOrNoNum( const SwNodeIndex& rIdx, sal_Bool bOldNum,
                                    sal_Bool bNewNum)
    : SwUndo( UNDO_NUMORNONUM ), nIdx( rIdx.GetIndex() ), mbNewNum(bNewNum),
      mbOldNum(bOldNum)
{
}

// #115901#, #i40034#
void SwUndoNumOrNoNum::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwNodeIndex aIdx( rContext.GetDoc().GetNodes(), nIdx );
    SwTxtNode * pTxtNd = aIdx.GetNode().GetTxtNode();

    if (NULL != pTxtNd)
    {
        pTxtNd->SetCountedInList(mbOldNum);
    }
}

// #115901#, #i40034#
void SwUndoNumOrNoNum::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwNodeIndex aIdx( rContext.GetDoc().GetNodes(), nIdx );
    SwTxtNode * pTxtNd = aIdx.GetNode().GetTxtNode();

    if (NULL != pTxtNd)
    {
        pTxtNd->SetCountedInList(mbNewNum);
    }
}

// #115901#
void SwUndoNumOrNoNum::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if (mbOldNum && ! mbNewNum)
    {
        rDoc.NumOrNoNum(rContext.GetRepeatPaM().GetPoint()->nNode, sal_False);
    }
    else if ( ! mbOldNum && mbNewNum )
    {
        rDoc.NumOrNoNum(rContext.GetRepeatPaM().GetPoint()->nNode, sal_True);
    }
}

SwUndoNumRuleStart::SwUndoNumRuleStart( const SwPosition& rPos, sal_Bool bFlg )
    : SwUndo( UNDO_SETNUMRULESTART ),
    nIdx( rPos.nNode.GetIndex() ), nOldStt( USHRT_MAX ),
    nNewStt( USHRT_MAX ), bSetSttValue( sal_False ), bFlag( bFlg )
{
}

SwUndoNumRuleStart::SwUndoNumRuleStart( const SwPosition& rPos, sal_uInt16 nStt )
    : SwUndo( UNDO_SETNUMRULESTART ),
    nIdx( rPos.nNode.GetIndex() ),
    nOldStt( USHRT_MAX ), nNewStt( nStt ), bSetSttValue( sal_True )
{
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if ( pTxtNd )
    {
        if ( pTxtNd->HasAttrListRestartValue() )
        {
            nOldStt = static_cast<sal_uInt16>(pTxtNd->GetAttrListRestartValue());
        }
        else
        {
            nOldStt = USHRT_MAX; // indicating, that the list restart value is not set
        }
    }
}

void SwUndoNumRuleStart::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPosition const aPos( *rDoc.GetNodes()[ nIdx ] );
    if( bSetSttValue )
    {
        rDoc.SetNodeNumStart( aPos, nOldStt );
    }
    else
    {
        rDoc.SetNumRuleStart( aPos, !bFlag );
    }
}

void SwUndoNumRuleStart::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPosition const aPos( *rDoc.GetNodes()[ nIdx ] );
    if( bSetSttValue )
    {
        rDoc.SetNodeNumStart( aPos, nNewStt );
    }
    else
    {
        rDoc.SetNumRuleStart( aPos, bFlag );
    }
}

void SwUndoNumRuleStart::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if( bSetSttValue )
    {
        rDoc.SetNodeNumStart(*rContext.GetRepeatPaM().GetPoint(), nNewStt);
    }
    else
    {
        rDoc.SetNumRuleStart(*rContext.GetRepeatPaM().GetPoint(), bFlag);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
