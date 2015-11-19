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

#include <IShellCursorSupplier.hxx>
#include <txtftn.hxx>
#include <fmtanchr.hxx>
#include <ftnidx.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <UndoManager.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <pamtyp.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <ndnotxt.hxx>
#include <IMark.hxx>
#include <mvsave.hxx>
#include <redline.hxx>
#include <crossrefbookmark.hxx>
#include <undo.hrc>
#include <comcore.hrc>
#include <docsh.hxx>
#include <o3tl/make_unique.hxx>

// This class saves the Pam as integers and can recompose those into a PaM
SwUndRng::SwUndRng()
    : nSttNode( 0 ), nEndNode( 0 ), nSttContent( 0 ), nEndContent( 0 )
{
}

SwUndRng::SwUndRng( const SwPaM& rPam )
{
    SetValues( rPam );
}

void SwUndRng::SetValues( const SwPaM& rPam )
{
    const SwPosition *pStt = rPam.Start();
    if( rPam.HasMark() )
    {
        const SwPosition *pEnd = rPam.GetPoint() == pStt
                        ? rPam.GetMark()
                        : rPam.GetPoint();
        nEndNode = pEnd->nNode.GetIndex();
        nEndContent = pEnd->nContent.GetIndex();
    }
    else
        // no selection !!
        nEndNode = 0, nEndContent = COMPLETE_STRING;

    nSttNode = pStt->nNode.GetIndex();
    nSttContent = pStt->nContent.GetIndex();
}

void SwUndRng::SetPaM( SwPaM & rPam, bool bCorrToContent ) const
{
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = nSttNode;
    SwNode& rNd = rPam.GetNode();
    if( rNd.IsContentNode() )
        rPam.GetPoint()->nContent.Assign( rNd.GetContentNode(), nSttContent );
    else if( bCorrToContent )
        rPam.Move( fnMoveForward, fnGoContent );
    else
        rPam.GetPoint()->nContent.Assign( nullptr, 0 );

    if( !nEndNode && COMPLETE_STRING == nEndContent )       // no selection
        return ;

    rPam.SetMark();
    if( nSttNode == nEndNode && nSttContent == nEndContent )
        return;                             // nothing left to do

    rPam.GetPoint()->nNode = nEndNode;
    if( rPam.GetNode().IsContentNode() )
        rPam.GetPoint()->nContent.Assign( rPam.GetNode().GetContentNode(), nEndContent );
    else if( bCorrToContent )
        rPam.Move( fnMoveBackward, fnGoContent );
    else
        rPam.GetPoint()->nContent.Assign( nullptr, 0 );
}

SwPaM & SwUndRng::AddUndoRedoPaM(
        ::sw::UndoRedoContext & rContext, bool const bCorrToContent) const
{
    SwPaM & rPaM( rContext.GetCursorSupplier().CreateNewShellCursor() );
    SetPaM( rPaM, bCorrToContent );
    return rPaM;
}

void SwUndo::RemoveIdxFromSection( SwDoc& rDoc, sal_uLong nSttIdx,
                                    sal_uLong* pEndIdx )
{
    SwNodeIndex aIdx( rDoc.GetNodes(), nSttIdx );
    SwNodeIndex aEndIdx( rDoc.GetNodes(), pEndIdx ? *pEndIdx
                                    : aIdx.GetNode().EndOfSectionIndex() );
    SwPosition aPos( rDoc.GetNodes().GetEndOfPostIts() );
    SwDoc::CorrAbs( aIdx, aEndIdx, aPos, true );
}

void SwUndo::RemoveIdxFromRange( SwPaM& rPam, bool bMoveNext )
{
    const SwPosition* pEnd = rPam.End();
    if( bMoveNext )
    {
        if( pEnd != rPam.GetPoint() )
            rPam.Exchange();

        SwNodeIndex aStt( rPam.GetMark()->nNode );
        SwNodeIndex aEnd( rPam.GetPoint()->nNode );

        if( !rPam.Move( fnMoveForward ) )
        {
            rPam.Exchange();
            if( !rPam.Move( fnMoveBackward ) )
            {
                rPam.GetPoint()->nNode = rPam.GetDoc()->GetNodes().GetEndOfPostIts();
                rPam.GetPoint()->nContent.Assign( nullptr, 0 );
            }
        }

        SwDoc::CorrAbs( aStt, aEnd, *rPam.GetPoint(), true );
    }
    else
        SwDoc::CorrAbs( rPam, *pEnd, true );
}

void SwUndo::RemoveIdxRel( sal_uLong nIdx, const SwPosition& rPos )
{
    // Move only the Cursor. Bookmarks/TOXMarks/etc. are done by the corresponding
    // JoinNext/JoinPrev
    SwNodeIndex aIdx( rPos.nNode.GetNode().GetNodes(), nIdx );
    ::PaMCorrRel( aIdx, rPos );
}

SwUndo::SwUndo(SwUndoId const nId)
    : m_nId(nId), nOrigRedlineMode(nsRedlineMode_t::REDLINE_NONE),
      bCacheComment(true), pComment(nullptr)
{
}

bool SwUndo::IsDelBox() const
{
    return GetId() == UNDO_COL_DELETE || GetId() == UNDO_ROW_DELETE ||
        GetId() == UNDO_TABLE_DELBOX;
}

SwUndo::~SwUndo()
{
    delete pComment;
}

class UndoRedoRedlineGuard
{
public:
    UndoRedoRedlineGuard(::sw::UndoRedoContext & rContext, SwUndo & rUndo)
        : m_rRedlineAccess(rContext.GetDoc().getIDocumentRedlineAccess())
        , m_eMode(m_rRedlineAccess.GetRedlineMode())
    {
        RedlineMode_t const eTmpMode =
            static_cast<RedlineMode_t>(rUndo.GetRedlineMode());
        if ((nsRedlineMode_t::REDLINE_SHOW_MASK & eTmpMode) !=
            (nsRedlineMode_t::REDLINE_SHOW_MASK & m_eMode))
        {
            m_rRedlineAccess.SetRedlineMode( eTmpMode );
        }
        m_rRedlineAccess.SetRedlineMode_intern( static_cast<RedlineMode_t>(
                eTmpMode | nsRedlineMode_t::REDLINE_IGNORE) );
    }
    ~UndoRedoRedlineGuard()
    {
        m_rRedlineAccess.SetRedlineMode(m_eMode);
    }
private:
    IDocumentRedlineAccess & m_rRedlineAccess;
    RedlineMode_t const m_eMode;
};

void SwUndo::Undo()
{
    assert(false); // SwUndo::Undo(): ERROR: must call UndoWithContext instead
}

void SwUndo::Redo()
{
    assert(false); // SwUndo::Redo(): ERROR: must call RedoWithContext instead
}

void SwUndo::UndoWithContext(SfxUndoContext & rContext)
{
    ::sw::UndoRedoContext *const pContext(
            dynamic_cast< ::sw::UndoRedoContext * >(& rContext));
    assert(pContext);
    const UndoRedoRedlineGuard aUndoRedoRedlineGuard(*pContext, *this);
    UndoImpl(*pContext);
}

void SwUndo::RedoWithContext(SfxUndoContext & rContext)
{
    ::sw::UndoRedoContext *const pContext(
            dynamic_cast< ::sw::UndoRedoContext * >(& rContext));
    assert(pContext);
    const UndoRedoRedlineGuard aUndoRedoRedlineGuard(*pContext, *this);
    RedoImpl(*pContext);
}

void SwUndo::Repeat(SfxRepeatTarget & rContext)
{
    ::sw::RepeatContext *const pRepeatContext(
            dynamic_cast< ::sw::RepeatContext * >(& rContext));
    assert(pRepeatContext);
    RepeatImpl(*pRepeatContext);
}

bool SwUndo::CanRepeat(SfxRepeatTarget & rContext) const
{
    ::sw::RepeatContext *const pRepeatContext(
            dynamic_cast< ::sw::RepeatContext * >(& rContext));
    assert(pRepeatContext);
    return CanRepeatImpl(*pRepeatContext);
}

void SwUndo::RepeatImpl( ::sw::RepeatContext & )
{
}

bool SwUndo::CanRepeatImpl( ::sw::RepeatContext & ) const
{
    return ((REPEAT_START <= GetId()) && (GetId() < REPEAT_END));
}

OUString SwUndo::GetComment() const
{
    OUString aResult;

    if (bCacheComment)
    {
        if (! pComment)
        {
            pComment = new OUString(SW_RES(UNDO_BASE + GetId()));

            SwRewriter aRewriter = GetRewriter();

            *pComment = aRewriter.Apply(*pComment);
        }

        aResult = *pComment;
    }
    else
    {
        aResult = SW_RES(UNDO_BASE + GetId());

        SwRewriter aRewriter = GetRewriter();

        aResult = aRewriter.Apply(aResult);
    }

    return aResult;
}

SwRewriter SwUndo::GetRewriter() const
{
    SwRewriter aResult;

    return aResult;
}

SwUndoSaveContent::SwUndoSaveContent()
    : pHistory( nullptr )
{}

SwUndoSaveContent::~SwUndoSaveContent()
{
    delete pHistory;
}

// This is needed when deleting content. For REDO all contents will be moved
// into the UndoNodesArray. These methods always create a new node to insert
// content. As a result, the attributes will not be expanded.
// - MoveTo   moves from NodesArray into UndoNodesArray
// - MoveFrom moves from UndoNodesArray into NodesArray

// If pEndNdIdx is given, Undo/Redo calls -Ins/DelFly. In that case the whole
// section should be moved.
void SwUndoSaveContent::MoveToUndoNds( SwPaM& rPaM, SwNodeIndex* pNodeIdx,
                    sal_uLong* pEndNdIdx, sal_Int32* pEndCntIdx )
{
    SwDoc& rDoc = *rPaM.GetDoc();
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    SwNoTextNode* pCpyNd = rPaM.GetNode().GetNoTextNode();

    // here comes the actual delete (move)
    SwNodes & rNds = rDoc.GetUndoManager().GetUndoNodes();
    SwPosition aPos( pEndNdIdx ? rNds.GetEndOfPostIts()
                               : rNds.GetEndOfExtras() );

    const SwPosition* pStt = rPaM.Start(), *pEnd = rPaM.End();

    sal_uLong nTmpMvNode = aPos.nNode.GetIndex();

    if( pCpyNd || pEndNdIdx )
    {
        SwNodeRange aRg( pStt->nNode, 0, pEnd->nNode, 1 );
        rDoc.GetNodes()._MoveNodes( aRg, rNds, aPos.nNode, false );
        aPos.nContent = 0;
        --aPos.nNode;
    }
    else
    {
        rDoc.GetNodes().MoveRange( rPaM, aPos, rNds );
    }
    if( pEndNdIdx )
        *pEndNdIdx = aPos.nNode.GetIndex();
    if( pEndCntIdx )
        *pEndCntIdx = aPos.nContent.GetIndex();

    // old position
    aPos.nNode = nTmpMvNode;
    if( pNodeIdx )
        *pNodeIdx = aPos.nNode;
}

void SwUndoSaveContent::MoveFromUndoNds( SwDoc& rDoc, sal_uLong nNodeIdx,
                            SwPosition& rInsPos,
                            sal_uLong* pEndNdIdx, sal_Int32* pEndCntIdx )
{
    // here comes the recovery
    SwNodes & rNds = rDoc.GetUndoManager().GetUndoNodes();
    if( nNodeIdx == rNds.GetEndOfPostIts().GetIndex() )
        return;     // nothing saved

    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    SwPaM aPaM( rInsPos );
    if( pEndNdIdx )         // than get the section from it
        aPaM.GetPoint()->nNode.Assign( rNds, *pEndNdIdx );
    else
    {
        aPaM.GetPoint()->nNode = rNds.GetEndOfExtras();
        GoInContent( aPaM, fnMoveBackward );
    }

    SwTextNode* pTextNd = aPaM.GetNode().GetTextNode();
    if (!pEndNdIdx && pTextNd)
    {
        if( pEndCntIdx )
            aPaM.GetPoint()->nContent.Assign( pTextNd, *pEndCntIdx );

        aPaM.SetMark();
        aPaM.GetPoint()->nNode = nNodeIdx;
        aPaM.GetPoint()->nContent.Assign(aPaM.GetContentNode(), 0);

        _SaveRedlEndPosForRestore aRedlRest( rInsPos.nNode, rInsPos.nContent.GetIndex() );

        rNds.MoveRange( aPaM, rInsPos, rDoc.GetNodes() );

        // delete the last Node as well
        if( !aPaM.GetPoint()->nContent.GetIndex() ||
            ( aPaM.GetPoint()->nNode++ &&       // still empty Nodes at the end?
            &rNds.GetEndOfExtras() != &aPaM.GetPoint()->nNode.GetNode() ))
        {
            aPaM.GetPoint()->nContent.Assign( nullptr, 0 );
            aPaM.SetMark();
            rNds.Delete( aPaM.GetPoint()->nNode,
                        rNds.GetEndOfExtras().GetIndex() -
                        aPaM.GetPoint()->nNode.GetIndex() );
        }

        aRedlRest.Restore();
    }
    else if( pEndNdIdx || !pTextNd )
    {
        SwNodeRange aRg( rNds, nNodeIdx, rNds, (pEndNdIdx
                        ? ((*pEndNdIdx) + 1)
                        : rNds.GetEndOfExtras().GetIndex() ) );
        rNds._MoveNodes( aRg, rDoc.GetNodes(), rInsPos.nNode, nullptr == pEndNdIdx );

    }
    else {
        assert(false); // wtf?
    }
}

// These two methods move the Point of Pam backwards/forwards. With that, one
// can span an area for a Undo/Redo. (The Point is then positioned in front of
// the area to manipulate!)
// The flag indicates if there is still content in front of Point.
bool SwUndoSaveContent::MovePtBackward( SwPaM& rPam )
{
    rPam.SetMark();
    if( rPam.Move( fnMoveBackward ))
        return true;

    // If there is no content onwards, set Point simply to the previous position
    // (Node and Content, so that Content will be detached!)
    --rPam.GetPoint()->nNode;
    rPam.GetPoint()->nContent.Assign( nullptr, 0 );
    return false;
}

void SwUndoSaveContent::MovePtForward( SwPaM& rPam, bool bMvBkwrd )
{
    // Was there content before this position?
    if( bMvBkwrd )
        rPam.Move( fnMoveForward );
    else
    {
        ++rPam.GetPoint()->nNode;
        SwContentNode* pCNd = rPam.GetContentNode();
        if( pCNd )
            pCNd->MakeStartIndex( &rPam.GetPoint()->nContent );
        else
            rPam.Move( fnMoveForward );
    }
}

// Delete all objects that have ContentIndices to the given area.
// Currently (1994) these exist:
//                  - Footnotes
//                  - Flys
//                  - Bookmarks
//                  - Directories

// #i81002# - extending method
// delete certain (not all) cross-reference bookmarks at text node of <rMark>
// and at text node of <rPoint>, if these text nodes aren't the same.
void SwUndoSaveContent::DelContentIndex( const SwPosition& rMark,
                                     const SwPosition& rPoint,
                                     DelContentType nDelContentType )
{
    const SwPosition *pStt = rMark < rPoint ? &rMark : &rPoint,
                    *pEnd = &rMark == pStt ? &rPoint : &rMark;

    SwDoc* pDoc = rMark.nNode.GetNode().GetDoc();

    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    // 1. Footnotes
    if( nsDelContentType::DELCNT_FTN & nDelContentType )
    {
        SwFootnoteIdxs& rFootnoteArr = pDoc->GetFootnoteIdxs();
        if( !rFootnoteArr.empty() )
        {
            const SwNode* pFootnoteNd;
            size_t nPos = 0;
            rFootnoteArr.SeekEntry( pStt->nNode, &nPos );
            SwTextFootnote* pSrch;

            // for now delete all that come afterwards
            while( nPos < rFootnoteArr.size() && ( pFootnoteNd =
                &( pSrch = rFootnoteArr[ nPos ] )->GetTextNode())->GetIndex()
                        <= pEnd->nNode.GetIndex() )
            {
                const sal_Int32 nFootnoteSttIdx = pSrch->GetStart();
                if( (nsDelContentType::DELCNT_CHKNOCNTNT & nDelContentType )
                    ? (&pEnd->nNode.GetNode() == pFootnoteNd )
                    : (( &pStt->nNode.GetNode() == pFootnoteNd &&
                    pStt->nContent.GetIndex() > nFootnoteSttIdx) ||
                    ( &pEnd->nNode.GetNode() == pFootnoteNd &&
                    nFootnoteSttIdx >= pEnd->nContent.GetIndex() )) )
                {
                    ++nPos;     // continue searching
                    continue;
                }

// FIXME: duplicated code here and below -> refactor?
                // Unfortunately an index needs to be created. Otherwise there
                // will be problems with TextNode because the index will be
                // deleted in the DTOR of SwFootnote!
                SwTextNode* pTextNd = const_cast<SwTextNode*>(static_cast<const SwTextNode*>(pFootnoteNd));
                if( !pHistory )
                    pHistory = new SwHistory;
                SwTextAttr* const pFootnoteHint =
                    pTextNd->GetTextAttrForCharAt( nFootnoteSttIdx );
                assert(pFootnoteHint);
                SwIndex aIdx( pTextNd, nFootnoteSttIdx );
                pHistory->Add( pFootnoteHint, pTextNd->GetIndex(), false );
                pTextNd->EraseText( aIdx, 1 );
            }

            while( nPos-- && ( pFootnoteNd = &( pSrch = rFootnoteArr[ nPos ] )->
                    GetTextNode())->GetIndex() >= pStt->nNode.GetIndex() )
            {
                const sal_Int32 nFootnoteSttIdx = pSrch->GetStart();
                if( !(nsDelContentType::DELCNT_CHKNOCNTNT & nDelContentType) && (
                    ( &pStt->nNode.GetNode() == pFootnoteNd &&
                    pStt->nContent.GetIndex() > nFootnoteSttIdx ) ||
                    ( &pEnd->nNode.GetNode() == pFootnoteNd &&
                    nFootnoteSttIdx >= pEnd->nContent.GetIndex() )))
                    continue;               // continue searching

                // Unfortunately an index needs to be created. Otherwise there
                // will be problems with TextNode because the index will be
                // deleted in the DTOR of SwFootnote!
                SwTextNode* pTextNd = const_cast<SwTextNode*>(static_cast<const SwTextNode*>(pFootnoteNd));
                if( !pHistory )
                    pHistory = new SwHistory;
                SwTextAttr* const pFootnoteHint =
                    pTextNd->GetTextAttrForCharAt( nFootnoteSttIdx );
                assert(pFootnoteHint);
                SwIndex aIdx( pTextNd, nFootnoteSttIdx );
                pHistory->Add( pFootnoteHint, pTextNd->GetIndex(), false );
                pTextNd->EraseText( aIdx, 1 );
            }
        }
    }

    // 2. Flys
    if( nsDelContentType::DELCNT_FLY & nDelContentType )
    {
        sal_uInt16 nChainInsPos = pHistory ? pHistory->Count() : 0;
        const SwFrameFormats& rSpzArr = *pDoc->GetSpzFrameFormats();
        if( !rSpzArr.empty() )
        {
            const bool bDelFwrd = rMark.nNode.GetIndex() <= rPoint.nNode.GetIndex();
            SwFrameFormat* pFormat;
            const SwFormatAnchor* pAnchor;
            size_t n = rSpzArr.size();
            const SwPosition* pAPos;

            while( n && !rSpzArr.empty() )
            {
                pFormat = rSpzArr[--n];
                pAnchor = &pFormat->GetAnchor();
                switch( pAnchor->GetAnchorId() )
                {
                case FLY_AS_CHAR:
                    if( nullptr != (pAPos = pAnchor->GetContentAnchor() ) &&
                        (( nsDelContentType::DELCNT_CHKNOCNTNT & nDelContentType )
                        ? ( pStt->nNode <= pAPos->nNode &&
                            pAPos->nNode < pEnd->nNode )
                        : ( *pStt <= *pAPos && *pAPos < *pEnd )) )
                    {
                        if( !pHistory )
                            pHistory = new SwHistory;
                        SwTextNode *const pTextNd =
                            pAPos->nNode.GetNode().GetTextNode();
                        SwTextAttr* const pFlyHint = pTextNd->GetTextAttrForCharAt(
                            pAPos->nContent.GetIndex());
                        assert(pFlyHint);
                        pHistory->Add( pFlyHint, 0, false );
                        // reset n so that no Format is skipped
                        n = n >= rSpzArr.size() ? rSpzArr.size() : n+1;
                    }
                    break;
                case FLY_AT_PARA:
                    {
                        pAPos =  pAnchor->GetContentAnchor();
                        if( pAPos )
                        {
                            bool bTmp;
                            if( nsDelContentType::DELCNT_CHKNOCNTNT & nDelContentType )
                                bTmp = pStt->nNode <= pAPos->nNode && pAPos->nNode < pEnd->nNode;
                            else
                            {
                                if (bDelFwrd)
                                    bTmp = rMark.nNode < pAPos->nNode &&
                                        pAPos->nNode <= rPoint.nNode;
                                else
                                    bTmp = rPoint.nNode <= pAPos->nNode &&
                                        pAPos->nNode < rMark.nNode;
                            }

                            if (bTmp)
                            {
                                if( !pHistory )
                                    pHistory = new SwHistory;

                                // Moving the anchor?
                                if( !( nsDelContentType::DELCNT_CHKNOCNTNT & nDelContentType ) &&
                                    ( rPoint.nNode.GetIndex() == pAPos->nNode.GetIndex() ) )
                                {
                                    // Do not try to move the anchor to a table!
                                    if( rMark.nNode.GetNode().GetTextNode() )
                                    {
                                        pHistory->Add( *pFormat );
                                        SwFormatAnchor aAnch( *pAnchor );
                                        SwPosition aPos( rMark.nNode );
                                        aAnch.SetAnchor( &aPos );
                                        pFormat->SetFormatAttr( aAnch );
                                    }
                                }
                                else
                                {
                                    pHistory->Add( *static_cast<SwFlyFrameFormat *>(pFormat), nChainInsPos );
                                    // reset n so that no Format is skipped
                                    n = n >= rSpzArr.size() ?
                                        rSpzArr.size() : n+1;
                                }
                            }
                        }
                    }
                    break;
                case FLY_AT_CHAR:
                    if( nullptr != (pAPos = pAnchor->GetContentAnchor() ) &&
                        ( pStt->nNode <= pAPos->nNode && pAPos->nNode <= pEnd->nNode ) )
                    {
                        if( !pHistory )
                            pHistory = new SwHistory;
                        if (IsDestroyFrameAnchoredAtChar(
                                *pAPos, *pStt, *pEnd, pDoc, nDelContentType))
                        {
                            pHistory->Add( *static_cast<SwFlyFrameFormat *>(pFormat), nChainInsPos );
                            n = n >= rSpzArr.size() ? rSpzArr.size() : n+1;
                        }
                        else if( !( nsDelContentType::DELCNT_CHKNOCNTNT & nDelContentType ) )
                        {
                            if( *pStt <= *pAPos && *pAPos < *pEnd )
                            {
                                // These are the objects anchored
                                // between section start and end position
                                // Do not try to move the anchor to a table!
                                if( rMark.nNode.GetNode().GetTextNode() )
                                {
                                    pHistory->Add( *pFormat );
                                    SwFormatAnchor aAnch( *pAnchor );
                                    aAnch.SetAnchor( &rMark );
                                    pFormat->SetFormatAttr( aAnch );
                                }
                            }
                        }
                    }
                    break;
                case FLY_AT_FLY:

                    if( nullptr != (pAPos = pAnchor->GetContentAnchor() ) &&
                        pStt->nNode == pAPos->nNode )
                    {
                        if( !pHistory )
                            pHistory = new SwHistory;

                        pHistory->Add( *static_cast<SwFlyFrameFormat *>(pFormat), nChainInsPos );

                        // reset n so that no Format is skipped
                        n = n >= rSpzArr.size() ? rSpzArr.size() : n+1;
                    }
                    break;
                default: break;
                }
            }
        }
    }

    // 3. Bookmarks
    if( nsDelContentType::DELCNT_BKM & nDelContentType )
    {
        IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
        if( pMarkAccess->getAllMarksCount() )
        {

            for( sal_Int32 n = 0; n < pMarkAccess->getAllMarksCount(); ++n )
            {
                // #i81002#
                bool bSavePos = false;
                bool bSaveOtherPos = false;
                const ::sw::mark::IMark* pBkmk = (pMarkAccess->getAllMarksBegin() + n)->get();

                if( nsDelContentType::DELCNT_CHKNOCNTNT & nDelContentType )
                {
                    if ( pStt->nNode <= pBkmk->GetMarkPos().nNode
                         && pBkmk->GetMarkPos().nNode < pEnd->nNode )
                    {
                        bSavePos = true;
                    }
                    if ( pBkmk->IsExpanded()
                         && pStt->nNode <= pBkmk->GetOtherMarkPos().nNode
                         && pBkmk->GetOtherMarkPos().nNode < pEnd->nNode )
                    {
                        bSaveOtherPos = true;
                    }
                }
                else
                {
                    // #i92125#
                    // keep cross-reference bookmarks, if content inside one paragraph is deleted.
                    if ( rMark.nNode == rPoint.nNode
                         && ( IDocumentMarkAccess::GetType(*pBkmk) == IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK
                              || IDocumentMarkAccess::GetType(*pBkmk) == IDocumentMarkAccess::MarkType::CROSSREF_NUMITEM_BOOKMARK ) )
                    {
                        continue;
                    }

                    bool bMaybe = false;
                    if ( *pStt <= pBkmk->GetMarkPos() && pBkmk->GetMarkPos() <= *pEnd )
                    {
                        if ( pBkmk->GetMarkPos() == *pEnd
                             || ( *pStt == pBkmk->GetMarkPos() && pBkmk->IsExpanded() ) )
                            bMaybe = true;
                        else
                            bSavePos = true;
                    }
                    if( pBkmk->IsExpanded() &&
                        *pStt <= pBkmk->GetOtherMarkPos() && pBkmk->GetOtherMarkPos() <= *pEnd )
                    {
                        if ( bSavePos || bSaveOtherPos
                             || ( pBkmk->GetOtherMarkPos() < *pEnd && pBkmk->GetOtherMarkPos() > *pStt ) )
                        {
                            if( bMaybe )
                                bSavePos = true;
                            bSaveOtherPos = true;
                        }
                    }

                    if ( !bSavePos && !bSaveOtherPos
                         && dynamic_cast< const ::sw::mark::CrossRefBookmark* >(pBkmk) )
                    {
                        // certain special handling for cross-reference bookmarks
                        const bool bDifferentTextNodesAtMarkAndPoint =
                            rMark.nNode != rPoint.nNode
                            && rMark.nNode.GetNode().GetTextNode()
                            && rPoint.nNode.GetNode().GetTextNode();
                        if ( bDifferentTextNodesAtMarkAndPoint )
                        {
                            // delete cross-reference bookmark at <pStt>, if only part of
                            // <pEnd> text node content is deleted.
                            if( pStt->nNode == pBkmk->GetMarkPos().nNode
                                && pEnd->nContent.GetIndex() != pEnd->nNode.GetNode().GetTextNode()->Len() )
                            {
                                bSavePos = true;
                                bSaveOtherPos = false; // cross-reference bookmarks are not expanded
                            }
                            // delete cross-reference bookmark at <pEnd>, if only part of
                            // <pStt> text node content is deleted.
                            else if( pEnd->nNode == pBkmk->GetMarkPos().nNode &&
                                pStt->nContent.GetIndex() != 0 )
                            {
                                bSavePos = true;
                                bSaveOtherPos = false; // cross-reference bookmarks are not expanded
                            }
                        }
                    }
                    else if ( IDocumentMarkAccess::GetType(*pBkmk) == IDocumentMarkAccess::MarkType::ANNOTATIONMARK )
                    {
                        // delete annotation marks, if its end position is covered by the deletion
                        const SwPosition& rAnnotationEndPos = pBkmk->GetMarkEnd();
                        if ( *pStt < rAnnotationEndPos && rAnnotationEndPos <= *pEnd )
                        {
                            bSavePos = true;
                            bSaveOtherPos = pBkmk->IsExpanded(); //tdf#90138, only save the other pos if there is one
                        }
                    }
                }

                if ( bSavePos || bSaveOtherPos )
                {
                    if( IDocumentMarkAccess::GetType(*pBkmk) != IDocumentMarkAccess::MarkType::UNO_BOOKMARK )
                    {
                        if( !pHistory )
                            pHistory = new SwHistory;
                        pHistory->Add( *pBkmk, bSavePos, bSaveOtherPos );
                    }
                    if ( bSavePos
                         && ( bSaveOtherPos
                              || !pBkmk->IsExpanded() ) )
                    {
                        pMarkAccess->deleteMark(pMarkAccess->getAllMarksBegin()+n);
                        n--;
                    }
                }
            }
        }
    }
}

// save a complete section into UndoNodes array
SwUndoSaveSection::SwUndoSaveSection()
    : pMvStt( nullptr ), pRedlSaveData( nullptr ), nMvLen( 0 ), nStartPos( ULONG_MAX )
{
}

SwUndoSaveSection::~SwUndoSaveSection()
{
    if( pMvStt )        // delete also the section from UndoNodes array
    {
        // SaveSection saves the content in the PostIt section.
        SwNodes& rUNds = pMvStt->GetNode().GetNodes();
        rUNds.Delete( *pMvStt, nMvLen );

        delete pMvStt;
    }
    delete pRedlSaveData;
}

void SwUndoSaveSection::SaveSection( const SwNodeIndex& rSttIdx )
{
    SwNodeRange aRg( rSttIdx.GetNode(), *rSttIdx.GetNode().EndOfSectionNode() );
    SaveSection( aRg );
}

void SwUndoSaveSection::SaveSection(
    const SwNodeRange& rRange )
{
    SwPaM aPam( rRange.aStart, rRange.aEnd );

    // delete all footnotes, fly frames, bookmarks and indexes
    DelContentIndex( *aPam.GetMark(), *aPam.GetPoint() );
    {
        // move certain indexes out of deleted range
        SwNodeIndex aSttIdx( aPam.Start()->nNode.GetNode() );
        SwNodeIndex aEndIdx( aPam.End()->nNode.GetNode() );
        SwNodeIndex aMvStt( aEndIdx, 1 );
        SwDoc::CorrAbs( aSttIdx, aEndIdx, SwPosition( aMvStt ), true );
    }

    pRedlSaveData = new SwRedlineSaveDatas;
    if( !SwUndo::FillSaveData( aPam, *pRedlSaveData ))
        delete pRedlSaveData, pRedlSaveData = nullptr;

    nStartPos = rRange.aStart.GetIndex();

    --aPam.GetPoint()->nNode;
    ++aPam.GetMark()->nNode;

    SwContentNode* pCNd = aPam.GetContentNode( false );
    if( pCNd )
        aPam.GetMark()->nContent.Assign( pCNd, 0 );
    if( nullptr != ( pCNd = aPam.GetContentNode()) )
        aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

    // Keep positions as SwIndex so that this section can be deleted in DTOR
    sal_uLong nEnd;
    pMvStt = new SwNodeIndex( rRange.aStart );
    MoveToUndoNds(aPam, pMvStt, &nEnd);
    nMvLen = nEnd - pMvStt->GetIndex() + 1;
}

void SwUndoSaveSection::RestoreSection( SwDoc* pDoc, SwNodeIndex* pIdx,
                                        sal_uInt16 nSectType )
{
    if( ULONG_MAX != nStartPos )        // was there any content?
    {
        // check if the content is at the old position
        SwNodeIndex aSttIdx( pDoc->GetNodes(), nStartPos );

        // move the content from UndoNodes array into Fly
        SwStartNode* pSttNd = SwNodes::MakeEmptySection( aSttIdx,
                                                (SwStartNodeType)nSectType );

        RestoreSection( pDoc, SwNodeIndex( *pSttNd->EndOfSectionNode() ));

        if( pIdx )
            *pIdx = *pSttNd;
    }
}

void SwUndoSaveSection::RestoreSection( SwDoc* pDoc, const SwNodeIndex& rInsPos )
{
    if( ULONG_MAX != nStartPos )        // was there any content?
    {
        SwPosition aInsPos( rInsPos );
        sal_uLong nEnd = pMvStt->GetIndex() + nMvLen - 1;
        MoveFromUndoNds(*pDoc, pMvStt->GetIndex(), aInsPos, &nEnd);

        // destroy indices again, content was deleted from UndoNodes array
        DELETEZ( pMvStt );
        nMvLen = 0;

        if( pRedlSaveData )
        {
            SwUndo::SetSaveData( *pDoc, *pRedlSaveData );
            delete pRedlSaveData, pRedlSaveData = nullptr;
        }
    }
}

// save and set the RedlineData
SwRedlineSaveData::SwRedlineSaveData(
    SwComparePosition eCmpPos,
    const SwPosition& rSttPos,
    const SwPosition& rEndPos,
    SwRangeRedline& rRedl,
    bool bCopyNext )
    : SwUndRng( rRedl )
    , SwRedlineData( rRedl.GetRedlineData(), bCopyNext )
{
    assert( POS_OUTSIDE == eCmpPos ||
            !rRedl.GetContentIdx() ); // "Redline with Content"

    switch (eCmpPos)
    {
    case POS_OVERLAP_BEFORE:        // Pos1 overlaps Pos2 at the beginning
        nEndNode = rEndPos.nNode.GetIndex();
        nEndContent = rEndPos.nContent.GetIndex();
        break;

    case POS_OVERLAP_BEHIND:        // Pos1 overlaps Pos2 at the end
        nSttNode = rSttPos.nNode.GetIndex();
        nSttContent = rSttPos.nContent.GetIndex();
        break;

    case POS_INSIDE:                // Pos1 lays completely in Pos2
        nSttNode = rSttPos.nNode.GetIndex();
        nSttContent = rSttPos.nContent.GetIndex();
        nEndNode = rEndPos.nNode.GetIndex();
        nEndContent = rEndPos.nContent.GetIndex();
        break;

    case POS_OUTSIDE:               // Pos2 lays completely in Pos1
        if ( rRedl.GetContentIdx() )
        {
            // than move section into UndoArray and memorize it
            SaveSection( *rRedl.GetContentIdx() );
            rRedl.SetContentIdx( nullptr );
        }
        break;

    case POS_EQUAL:                 // Pos1 ist exactly as big as Pos2
        break;

    default:
        assert(false);
    }

#if OSL_DEBUG_LEVEL > 0
    nRedlineCount = rSttPos.nNode.GetNode().GetDoc()->getIDocumentRedlineAccess().GetRedlineTable().size();
#endif
}

SwRedlineSaveData::~SwRedlineSaveData()
{
}

void SwRedlineSaveData::RedlineToDoc( SwPaM& rPam )
{
    SwDoc& rDoc = *rPam.GetDoc();
    SwRangeRedline* pRedl = new SwRangeRedline( *this, rPam );

    if( GetMvSttIdx() )
    {
        SwNodeIndex aIdx( rDoc.GetNodes() );
        RestoreSection( &rDoc, &aIdx, SwNormalStartNode );
        if( GetHistory() )
            GetHistory()->Rollback( &rDoc );
        pRedl->SetContentIdx( &aIdx );
    }
    SetPaM( *pRedl );
    // First, delete the "old" so that in an Append no unexpected things will
    // happen, e.g. a delete in an insert. In the latter case the just restored
    // content will be deleted and not the one you originally wanted.
    rDoc.getIDocumentRedlineAccess().DeleteRedline( *pRedl, false, USHRT_MAX );

    RedlineMode_t eOld = rDoc.getIDocumentRedlineAccess().GetRedlineMode();
    rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_DONTCOMBINE_REDLINES));
    //#i92154# let UI know about a new redline with comment
    if (rDoc.GetDocShell() && (!pRedl->GetComment().isEmpty()) )
        rDoc.GetDocShell()->Broadcast(SwRedlineHint());

    bool const bSuccess = rDoc.getIDocumentRedlineAccess().AppendRedline( pRedl, true );
    assert(bSuccess); // SwRedlineSaveData::RedlineToDoc: insert redline failed
    (void) bSuccess; // unused in non-debug
    rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
}

bool SwUndo::FillSaveData(
    const SwPaM& rRange,
    SwRedlineSaveDatas& rSData,
    bool bDelRange,
    bool bCopyNext )
{
    rSData.clear();

    const SwPosition* pStt = rRange.Start();
    const SwPosition* pEnd = rRange.End();
    const SwRedlineTable& rTable = rRange.GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
    sal_uInt16 n = 0;
    rRange.GetDoc()->getIDocumentRedlineAccess().GetRedline( *pStt, &n );
    for ( ; n < rTable.size(); ++n )
    {
        SwRangeRedline* pRedl = rTable[n];

        const SwComparePosition eCmpPos =
            ComparePosition( *pStt, *pEnd, *pRedl->Start(), *pRedl->End() );
        if ( eCmpPos != POS_BEFORE
             && eCmpPos != POS_BEHIND
             && eCmpPos != POS_COLLIDE_END
             && eCmpPos != POS_COLLIDE_START )
        {
            std::unique_ptr<SwRedlineSaveData> pNewData(
                new SwRedlineSaveData(eCmpPos, *pStt, *pEnd, *pRedl, bCopyNext));
            rSData.push_back(std::move(pNewData));
        }
    }
    if( !rSData.empty() && bDelRange )
    {
        rRange.GetDoc()->getIDocumentRedlineAccess().DeleteRedline( rRange, false, USHRT_MAX );
    }
    return !rSData.empty();
}

bool SwUndo::FillSaveDataForFormat(
    const SwPaM& rRange,
    SwRedlineSaveDatas& rSData )
{
    rSData.clear();

    const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
    const SwRedlineTable& rTable = rRange.GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
    sal_uInt16 n = 0;
    rRange.GetDoc()->getIDocumentRedlineAccess().GetRedline( *pStt, &n );
    for ( ; n < rTable.size(); ++n )
    {
        SwRangeRedline* pRedl = rTable[n];
        if ( nsRedlineType_t::REDLINE_FORMAT == pRedl->GetType() )
        {
            const SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRedl->Start(), *pRedl->End() );
            if ( eCmpPos != POS_BEFORE
                 && eCmpPos != POS_BEHIND
                 && eCmpPos != POS_COLLIDE_END
                 && eCmpPos != POS_COLLIDE_START )
            {
                rSData.push_back(o3tl::make_unique<SwRedlineSaveData>(eCmpPos, *pStt, *pEnd, *pRedl, true));
            }

        }
    }
    return !rSData.empty();
}


void SwUndo::SetSaveData( SwDoc& rDoc, SwRedlineSaveDatas& rSData )
{
    RedlineMode_t eOld = rDoc.getIDocumentRedlineAccess().GetRedlineMode();
    rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( (RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON ));
    SwPaM aPam( rDoc.GetNodes().GetEndOfContent() );

    for( size_t n = rSData.size(); n; )
        rSData[ --n ].RedlineToDoc( aPam );

#if OSL_DEBUG_LEVEL > 0
    // check redline count against count saved in RedlineSaveData object
    assert(rSData.empty() ||
           (rSData[0].nRedlineCount == rDoc.getIDocumentRedlineAccess().GetRedlineTable().size()));
            // "redline count not restored properly"
#endif

    rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
}

bool SwUndo::HasHiddenRedlines( const SwRedlineSaveDatas& rSData )
{
    for( size_t n = rSData.size(); n; )
        if( rSData[ --n ].GetMvSttIdx() )
            return true;
    return false;
}

bool SwUndo::CanRedlineGroup( SwRedlineSaveDatas& rCurr,
                        const SwRedlineSaveDatas& rCheck, bool bCurrIsEnd )
{
    if( rCurr.size() != rCheck.size() )
        return false;

    for( size_t n = 0; n < rCurr.size(); ++n )
    {
        const SwRedlineSaveData& rSet = rCurr[ n ];
        const SwRedlineSaveData& rGet = rCheck[ n ];
        if( rSet.nSttNode != rGet.nSttNode ||
            rSet.GetMvSttIdx() || rGet.GetMvSttIdx() ||
            ( bCurrIsEnd ? rSet.nSttContent != rGet.nEndContent
                            : rSet.nEndContent != rGet.nSttContent ) ||
            !rGet.CanCombine( rSet ) )
        {
            return false;
        }
    }

    for( size_t n = 0; n < rCurr.size(); ++n )
    {
        SwRedlineSaveData& rSet = rCurr[ n ];
        const SwRedlineSaveData& rGet = rCheck[ n ];
        if( bCurrIsEnd )
            rSet.nSttContent = rGet.nSttContent;
        else
            rSet.nEndContent = rGet.nEndContent;
    }
    return true;
}

OUString ShortenString(const OUString & rStr, sal_Int32 nLength, const OUString & rFillStr)
{
    assert(nLength - rFillStr.getLength() >= 2);

    if (rStr.getLength() <= nLength)
        return rStr;

    nLength -= rFillStr.getLength();
    if ( nLength < 2 )
        nLength = 2;

    const sal_Int32 nFrontLen = nLength - nLength / 2;
    const sal_Int32 nBackLen = nLength - nFrontLen;

    return rStr.copy(0, nFrontLen)
           + rFillStr
           + rStr.copy(rStr.getLength() - nBackLen);
}

bool IsDestroyFrameAnchoredAtChar(SwPosition const & rAnchorPos,
        SwPosition const & rStart, SwPosition const & rEnd, const SwDoc* doc,
        DelContentType const nDelContentType)
{
    bool inSelection = rAnchorPos < rEnd;
    if( rAnchorPos == rEnd )
    {
        const SwNodes& nodes = doc->GetNodes();
        if( rEnd == SwPosition( nodes.GetEndOfContent()))
            inSelection = true;
        else
        {
            SwNodeIndex idx( nodes.GetEndOfContent());
         if( SwContentNode* last = SwNodes::GoPrevious( &idx ))
            inSelection = rEnd == SwPosition( *last, last->Len());
        }
    }
    // Here we identified the objects to destroy:
    // - anchored between start and end of the selection
    // - anchored in start of the selection with "CheckNoContent"
    // - anchored in start of sel. and the selection start at pos 0
    return  inSelection
         && (   (nsDelContentType::DELCNT_CHKNOCNTNT & nDelContentType)
            ||  (rStart.nNode < rAnchorPos.nNode)
            ||  !rStart.nContent.GetIndex()
            );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
