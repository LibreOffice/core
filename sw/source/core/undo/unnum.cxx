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
#include <doc.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <utility>
#include <osl/diagnose.h>

SwUndoInsNum::SwUndoInsNum( const SwNumRule& rOldRule,
                            const SwNumRule& rNewRule,
                            const SwDoc& rDoc,
                            SwUndoId nUndoId)
    : SwUndo( nUndoId, &rDoc ),
    m_aNumRule( rNewRule ),
    m_pOldNumRule( new SwNumRule( rOldRule )), m_nLRSavePos( 0 )
{
}

SwUndoInsNum::SwUndoInsNum( const SwPaM& rPam, const SwNumRule& rRule )
    : SwUndo( SwUndoId::INSNUM, &rPam.GetDoc() ), SwUndRng( rPam ),
    m_aNumRule( rRule ),
    m_nLRSavePos( 0 )
{
}

SwUndoInsNum::SwUndoInsNum( const SwPosition& rPos, const SwNumRule& rRule,
                            OUString aReplaceRule )
    : SwUndo( SwUndoId::INSNUM, &rPos.GetNode().GetDoc() ),
    m_aNumRule( rRule ),
    m_sReplaceRule(std::move( aReplaceRule )), m_nLRSavePos( 0 )
{
    // No selection!
    m_nEndNode = SwNodeOffset(0);
    m_nEndContent = COMPLETE_STRING;
    m_nSttNode = rPos.GetNodeIndex();
    m_nSttContent = rPos.GetContentIndex();
}

SwUndoInsNum::~SwUndoInsNum()
{
    m_pHistory.reset();
    m_pOldNumRule.reset();
}

SwRewriter SwUndoInsNum::GetRewriter() const
{
    SwRewriter aResult;
    if( SwUndoId::INSFMTATTR == GetId() )
        aResult.AddRule(UndoArg1, m_aNumRule.GetName());
    return aResult;
}

void SwUndoInsNum::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    if( m_pOldNumRule )
        rDoc.ChgNumRuleFormats( *m_pOldNumRule );

    if( m_pHistory )
    {
        if( m_nLRSavePos )
        {
            // Update immediately so that potential "old" LRSpaces will be valid again.
            m_pHistory->TmpRollback( &rDoc, m_nLRSavePos );

        }
        m_pHistory->TmpRollback( &rDoc, 0 );
        m_pHistory->SetTmpEnd( m_pHistory->Count() );
    }

    if (m_nSttNode)
    {
        AddUndoRedoPaM(rContext);
    }
}

void SwUndoInsNum::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    if ( m_pOldNumRule )
        rDoc.ChgNumRuleFormats( m_aNumRule );
    else if ( m_pHistory )
    {
        SwPaM & rPam( AddUndoRedoPaM(rContext) );
        if( !m_sReplaceRule.isEmpty() )
        {
            rDoc.ReplaceNumRule( *rPam.GetPoint(), m_sReplaceRule, m_aNumRule.GetName() );
        }
        else
        {
            // #i42921# - adapt to changed signature
            rDoc.SetNumRule(rPam, m_aNumRule, false);
        }
    }
}

void SwUndoInsNum::SetLRSpaceEndPos()
{
    if( m_pHistory )
        m_nLRSavePos = m_pHistory->Count();
}

void SwUndoInsNum::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc( rContext.GetDoc() );
    if ( m_nSttNode )
    {
        if( m_sReplaceRule.isEmpty() )
        {
            // #i42921# - adapt to changed signature
            rDoc.SetNumRule(rContext.GetRepeatPaM(), m_aNumRule, false);
        }
    }
    else
    {
        rDoc.ChgNumRuleFormats( m_aNumRule );
    }
}

SwHistory* SwUndoInsNum::GetHistory()
{
    if( !m_pHistory )
        m_pHistory.reset(new SwHistory);
    return m_pHistory.get();
}

void SwUndoInsNum::SaveOldNumRule( const SwNumRule& rOld )
{
    if( !m_pOldNumRule )
        m_pOldNumRule.reset(new SwNumRule( rOld ));
}

SwUndoDelNum::SwUndoDelNum( const SwPaM& rPam )
    : SwUndo( SwUndoId::DELNUM, &rPam.GetDoc() ), SwUndRng( rPam )
{
    if (m_nEndNode > m_nSttNode)
        m_aNodes.reserve( std::min<sal_Int32>(sal_Int32(m_nEndNode - m_nSttNode), 255) );
    m_pHistory.reset( new SwHistory );
}

SwUndoDelNum::~SwUndoDelNum()
{
}

void SwUndoDelNum::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    m_pHistory->TmpRollback( &rDoc, 0 );
    m_pHistory->SetTmpEnd( m_pHistory->Count() );

    for( const auto& rNode : m_aNodes )
    {
        SwTextNode* pNd = rDoc.GetNodes()[ rNode.index ]->GetTextNode();
        assert(pNd && "Where has the TextNode gone?");
        pNd->SetAttrListLevel( rNode.level );

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

void SwUndoDelNum::AddNode( const SwTextNode& rNd )
{
    if( rNd.GetNumRule() )
    {
        m_aNodes.emplace_back( rNd.GetIndex(), rNd.GetActualListLevel() );
    }
}

SwUndoMoveNum::SwUndoMoveNum( const SwPaM& rPam, SwNodeOffset nOff, bool bIsOutlMv )
    : SwUndo( bIsOutlMv ? SwUndoId::OUTLINE_UD : SwUndoId::MOVENUM, &rPam.GetDoc() ),
    SwUndRng( rPam ),
    m_nNewStart( 0 ), m_nOffset( nOff )
{
    // nOffset: Down    =>  1
    //          Up      => -1
}

void SwUndoMoveNum::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwNodeOffset nTmpStt = m_nSttNode, nTmpEnd = m_nEndNode;

    if (m_nEndNode || m_nEndContent != COMPLETE_STRING)        // section?
    {
        if( m_nNewStart < m_nSttNode )        // moved forwards
            m_nEndNode = m_nEndNode - ( m_nSttNode - m_nNewStart );
        else
            m_nEndNode = m_nEndNode + ( m_nNewStart - m_nSttNode );
    }
    m_nSttNode = m_nNewStart;

    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    rContext.GetDoc().MoveParagraph( rPam, -m_nOffset,
                                        SwUndoId::OUTLINE_UD == GetId() );
    m_nSttNode = nTmpStt;
    m_nEndNode = nTmpEnd;
}

void SwUndoMoveNum::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    rContext.GetDoc().MoveParagraph(rPam, m_nOffset, SwUndoId::OUTLINE_UD == GetId());
}

void SwUndoMoveNum::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if( SwUndoId::OUTLINE_UD == GetId() )
    {
        rDoc.MoveOutlinePara(rContext.GetRepeatPaM(),
                                            SwNodeOffset(0) < m_nOffset ? 1 : -1 );
    }
    else
    {
        rDoc.MoveParagraph(rContext.GetRepeatPaM(), m_nOffset);
    }
}

SwUndoNumUpDown::SwUndoNumUpDown( const SwPaM& rPam, short nOff )
    : SwUndo( nOff > 0 ? SwUndoId::NUMUP : SwUndoId::NUMDOWN, &rPam.GetDoc() ),
      SwUndRng( rPam ),
      m_nOffset( nOff )
{
    // nOffset: Down    =>  1
    //          Up      => -1
}

void SwUndoNumUpDown::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    rContext.GetDoc().NumUpDown(rPam, 1 != m_nOffset );
}

void SwUndoNumUpDown::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    rContext.GetDoc().NumUpDown(rPam, 1 == m_nOffset);
}

void SwUndoNumUpDown::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().NumUpDown(rContext.GetRepeatPaM(), 1 == m_nOffset);
}

SwUndoNumOrNoNum::SwUndoNumOrNoNum( const SwNode& rIdx, bool bOldNum,
                                    bool bNewNum)
    : SwUndo( SwUndoId::NUMORNONUM, &rIdx.GetDoc() ),
      m_nIndex( rIdx.GetIndex() ), mbNewNum(bNewNum),
      mbOldNum(bOldNum)
{
}

// #115901#, #i40034#
void SwUndoNumOrNoNum::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwNodeIndex aIdx( rContext.GetDoc().GetNodes(), m_nIndex );
    SwTextNode * pTextNd = aIdx.GetNode().GetTextNode();

    if (nullptr != pTextNd)
    {
        pTextNd->SetCountedInList(mbOldNum);
    }
}

// #115901#, #i40034#
void SwUndoNumOrNoNum::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwNodeIndex aIdx( rContext.GetDoc().GetNodes(), m_nIndex );
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
        rDoc.NumOrNoNum(rContext.GetRepeatPaM().GetPoint()->GetNode());
    }
    else if ( ! mbOldNum && mbNewNum )
    {
        rDoc.NumOrNoNum(rContext.GetRepeatPaM().GetPoint()->GetNode(), true);
    }
}

SwUndoNumRuleStart::SwUndoNumRuleStart( const SwPosition& rPos, bool bFlg )
    : SwUndo( SwUndoId::SETNUMRULESTART, &rPos.GetDoc() ),
    m_nIndex( rPos.GetNodeIndex() ), m_nOldStart( USHRT_MAX ),
    m_nNewStart( USHRT_MAX ), m_bSetStartValue( false ), m_bFlag( bFlg )
{
}

SwUndoNumRuleStart::SwUndoNumRuleStart( const SwPosition& rPos, sal_uInt16 nStt )
    : SwUndo(SwUndoId::SETNUMRULESTART, &rPos.GetDoc())
    , m_nIndex(rPos.GetNodeIndex())
    , m_nOldStart(USHRT_MAX)
    , m_nNewStart(nStt)
    , m_bSetStartValue(true)
    , m_bFlag(false)
{
    SwTextNode* pTextNd = rPos.GetNode().GetTextNode();
    if ( pTextNd )
    {
        if ( pTextNd->HasAttrListRestartValue() )
        {
            m_nOldStart = o3tl::narrowing<sal_uInt16>(pTextNd->GetAttrListRestartValue());
        }
        else
        {
            m_nOldStart = USHRT_MAX; // indicating, that the list restart value is not set
        }
    }
}

void SwUndoNumRuleStart::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPosition const aPos( *rDoc.GetNodes()[ m_nIndex ] );
    if( m_bSetStartValue )
    {
        rDoc.SetNodeNumStart( aPos, m_nOldStart );
    }
    else
    {
        rDoc.SetNumRuleStart( aPos, !m_bFlag );
    }
}

void SwUndoNumRuleStart::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPosition const aPos( *rDoc.GetNodes()[ m_nIndex ] );
    if( m_bSetStartValue )
    {
        rDoc.SetNodeNumStart( aPos, m_nNewStart );
    }
    else
    {
        rDoc.SetNumRuleStart( aPos, m_bFlag );
    }
}

void SwUndoNumRuleStart::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if( m_bSetStartValue )
    {
        rDoc.SetNodeNumStart(*rContext.GetRepeatPaM().GetPoint(), m_nNewStart);
    }
    else
    {
        rDoc.SetNumRuleStart(*rContext.GetRepeatPaM().GetPoint(), m_bFlag);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
