/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    aNumRule( rNewRule ), pHistory( nullptr ), nSttSet( ULONG_MAX ),
    pOldNumRule( new SwNumRule( rOldRule )), nLRSavePos( 0 )
{
}

SwUndoInsNum::SwUndoInsNum( const SwPaM& rPam, const SwNumRule& rRule )
    : SwUndo( UNDO_INSNUM ), SwUndRng( rPam ),
    aNumRule( rRule ), pHistory( nullptr ),
    nSttSet( ULONG_MAX ), pOldNumRule( nullptr ), nLRSavePos( 0 )
{
}

SwUndoInsNum::SwUndoInsNum( const SwPosition& rPos, const SwNumRule& rRule,
                            const OUString& rReplaceRule )
    : SwUndo( UNDO_INSNUM ),
    aNumRule( rRule ), pHistory( nullptr ),
    nSttSet( ULONG_MAX ), pOldNumRule( nullptr ),
    sReplaceRule( rReplaceRule ), nLRSavePos( 0 )
{
    // No selection!
    nEndNode = 0;
    nEndContent = COMPLETE_STRING;
    nSttNode = rPos.nNode.GetIndex();
    nSttContent = rPos.nContent.GetIndex();
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
        rDoc.ChgNumRuleFormats( *pOldNumRule );

    if( pHistory )
    {
        SwTextNode* pNd;
        if( ULONG_MAX != nSttSet &&
            nullptr != ( pNd = rDoc.GetNodes()[ nSttSet ]->GetTextNode() ))
                pNd->SetListRestart( true );
        else
            pNd = nullptr;

        if( nLRSavePos )
        {
            // Update immediately so that potential "old" LRSpaces will be valid again.
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

    if ( pOldNumRule )
        rDoc.ChgNumRuleFormats( aNumRule );
    else if ( pHistory )
    {
        SwPaM & rPam( AddUndoRedoPaM(rContext) );
        if( !sReplaceRule.isEmpty() )
        {
            rDoc.ReplaceNumRule( *rPam.GetPoint(), sReplaceRule, aNumRule.GetName() );
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
    SwDoc & rDoc( rContext.GetDoc() );
    if ( nSttNode )
    {
        if( sReplaceRule.isEmpty() )
        {
            // #i42921# - adapt to changed signature
            rDoc.SetNumRule(rContext.GetRepeatPaM(), aNumRule, false);
        }
    }
    else
    {
        rDoc.ChgNumRuleFormats( aNumRule );
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
        SwTextNode* pNd = rDoc.GetNodes()[ i->index ]->GetTextNode();
        OSL_ENSURE( pNd, "Where has the TextNode gone?" );
        pNd->SetAttrListLevel( i->level );

        if( pNd->GetCondFormatColl() )
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

void SwUndoDelNum::AddNode( const SwTextNode& rNd, bool )
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

    if (nEndNode || nEndContent != COMPLETE_STRING)        // section?
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
        rDoc.MoveParagraph(rContext.GetRepeatPaM(), nOffset);
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

SwUndoNumOrNoNum::SwUndoNumOrNoNum( const SwNodeIndex& rIdx, bool bOldNum,
                                    bool bNewNum)
    : SwUndo( UNDO_NUMORNONUM ), nIdx( rIdx.GetIndex() ), mbNewNum(bNewNum),
      mbOldNum(bOldNum)
{
}

// #115901#, #i40034#
void SwUndoNumOrNoNum::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwNodeIndex aIdx( rContext.GetDoc().GetNodes(), nIdx );
    SwTextNode * pTextNd = aIdx.GetNode().GetTextNode();

    if (nullptr != pTextNd)
    {
        pTextNd->SetCountedInList(mbOldNum);
    }
}

// #115901#, #i40034#
void SwUndoNumOrNoNum::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwNodeIndex aIdx( rContext.GetDoc().GetNodes(), nIdx );
    SwTextNode * pTextNd = aIdx.GetNode().GetTextNode();

    if (nullptr != pTextNd)
    {
        pTextNd->SetCountedInList(mbNewNum);
    }
}

void SwUndoNumOrNoNum::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if (mbOldNum && ! mbNewNum)
    {
        rDoc.NumOrNoNum(rContext.GetRepeatPaM().GetPoint()->nNode);
    }
    else if ( ! mbOldNum && mbNewNum )
    {
        rDoc.NumOrNoNum(rContext.GetRepeatPaM().GetPoint()->nNode, true);
    }
}

SwUndoNumRuleStart::SwUndoNumRuleStart( const SwPosition& rPos, bool bFlg )
    : SwUndo( UNDO_SETNUMRULESTART ),
    nIdx( rPos.nNode.GetIndex() ), nOldStt( USHRT_MAX ),
    nNewStt( USHRT_MAX ), bSetSttValue( false ), bFlag( bFlg )
{
}

SwUndoNumRuleStart::SwUndoNumRuleStart( const SwPosition& rPos, sal_uInt16 nStt )
    : SwUndo(UNDO_SETNUMRULESTART)
    , nIdx(rPos.nNode.GetIndex())
    , nOldStt(USHRT_MAX)
    , nNewStt(nStt)
    , bSetSttValue(true)
    , bFlag(false)
{
    SwTextNode* pTextNd = rPos.nNode.GetNode().GetTextNode();
    if ( pTextNd )
    {
        if ( pTextNd->HasAttrListRestartValue() )
        {
            nOldStt = static_cast<sal_uInt16>(pTextNd->GetAttrListRestartValue());
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
