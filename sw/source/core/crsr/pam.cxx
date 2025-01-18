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

#include <sal/config.h>

#include <tools/gen.hxx>
#include <editeng/protitem.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotools/configmgr.hxx>

#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <pamtyp.hxx>
#include <txtfrm.hxx>
#include <fmtcntnt.hxx>
#include <frmatr.hxx>
#include <flyfrm.hxx>
#include <fmteiro.hxx>
#include <section.hxx>
#include <sectfrm.hxx>
#include <ndtxt.hxx>
#include <swcrsr.hxx>

#include <IMark.hxx>
#include <DocumentSettingManager.hxx>
#include <hints.hxx>
#include <txatbase.hxx>
#include <osl/diagnose.h>
#include <utility>
#include <xmloff/odffields.hxx>
#include <rtl/ustrbuf.hxx>

#include <editsh.hxx>
#include <textcontentcontrol.hxx>

// for the dump "MSC-" compiler
static sal_Int32 GetSttOrEnd( bool bCondition, const SwContentNode& rNd )
{
    return bCondition ? 0 : rNd.Len();
}

SwPosition::SwPosition( const SwNodeIndex & rNodeIndex, const SwContentIndex & rContent )
    : nNode( rNodeIndex ), nContent( rContent )
{
    assert((!rNodeIndex.GetNode().GetContentNode() || rNodeIndex.GetNode().GetContentNode() == rContent.GetContentNode())
            && "parameters point to different nodes");
}

SwPosition::SwPosition( const SwNode & rNode, const SwContentIndex & rContent )
    : nNode( rNode ), nContent( rContent )
{
    assert((!rNode.GetContentNode() || rNode.GetContentNode() == rContent.GetContentNode())
            && "parameters point to different nodes");
}

SwPosition::SwPosition( const SwNodeIndex & rNodeIndex, const SwContentNode* pContentNode, sal_Int32 nContentOffset )
    : nNode( rNodeIndex ), nContent( pContentNode, nContentOffset )
{
    assert((!pContentNode || pContentNode == &rNodeIndex.GetNode()) &&
            "parameters point to different nodes");
}

SwPosition::SwPosition( const SwNode & rNode, const SwContentNode* pContentNode, sal_Int32 nContentOffset )
    : nNode( rNode ), nContent( pContentNode, nContentOffset )
{
    assert((!pContentNode || pContentNode == &rNode) &&
            "parameters point to different nodes");
}

SwPosition::SwPosition( const SwNodeIndex & rNodeIndex, SwNodeOffset nDiff, const SwContentNode* pContentNode, sal_Int32 nContentOffset )
    : nNode( rNodeIndex, nDiff ), nContent( pContentNode, nContentOffset )
{
    assert((!pContentNode || pContentNode == &rNodeIndex.GetNode()) &&
            "parameters point to different nodes");
}

SwPosition::SwPosition( const SwNodeIndex & rNodeIndex, SwNodeOffset nDiff )
    : nNode( rNodeIndex, nDiff ), nContent( GetNode().GetContentNode() )
{
}

SwPosition::SwPosition( const SwNode& rNode, SwNodeOffset nDiff )
    : nNode( rNode, nDiff ), nContent( GetNode().GetContentNode() )
{
}

SwPosition::SwPosition( SwNodes& rNodes, SwNodeOffset nIndex )
    : nNode( rNodes, nIndex ), nContent( GetNode().GetContentNode() )
{
}

SwPosition::SwPosition( const SwContentNode & rNode, const sal_Int32 nContentOffset )
    : nNode( rNode ), nContent( &rNode, nContentOffset )
{
}

SwPosition::SwPosition( const SwContentIndex & rContentIndex, short nDiff )
    : nNode( *rContentIndex.GetContentNode() ), nContent( rContentIndex, nDiff )
{
}

bool SwPosition::operator<(const SwPosition &rPos) const
{
    // cheaper to check for == first
    if( nNode == rPos.nNode )
    {
        // note that positions with text node but no SwContentIndex registered are
        // created for text frames anchored at para (see SwXFrame::getAnchor())
        SwContentNode const*const pThisReg(GetContentNode());
        SwContentNode const*const pOtherReg(rPos.GetContentNode());
        if (pThisReg && pOtherReg)
        {
            return (nContent < rPos.nContent);
        }
        else // by convention position with no index is smaller
        {
            return pOtherReg != nullptr;
        }
    }
    return nNode < rPos.nNode;
}

bool SwPosition::operator>(const SwPosition &rPos) const
{
    // cheaper to check for == first
    if( nNode == rPos.nNode )
    {
        // note that positions with text node but no SwContentIndex registered are
        // created for text frames anchored at para (see SwXFrame::getAnchor())
        SwContentNode const*const pThisReg(GetContentNode());
        SwContentNode const*const pOtherReg(rPos.GetContentNode());
        if (pThisReg && pOtherReg)
        {
            return (nContent > rPos.nContent);
        }
        else // by convention position with no index is smaller
        {
            return pThisReg != nullptr;
        }
    }
    return nNode > rPos.nNode;
}

bool SwPosition::operator<=(const SwPosition &rPos) const
{
    // cheaper to check for == first
    if( nNode == rPos.nNode )
    {
        // note that positions with text node but no SwContentIndex registered are
        // created for text frames anchored at para (see SwXFrame::getAnchor())
        SwContentNode const*const pThisReg(GetContentNode());
        SwContentNode const*const pOtherReg(rPos.GetContentNode());
        if (pThisReg && pOtherReg)
        {
            return (nContent <= rPos.nContent);
        }
        else // by convention position with no index is smaller
        {
            return pThisReg == nullptr;
        }
    }
    return nNode < rPos.nNode;
}

bool SwPosition::operator>=(const SwPosition &rPos) const
{
    // cheaper to check for == first
    if( nNode == rPos.nNode )
    {
        // note that positions with text node but no SwContentIndex registered are
        // created for text frames anchored at para (see SwXFrame::getAnchor())
        SwContentNode const*const pThisReg(GetContentNode());
        SwContentNode const*const pOtherReg(rPos.GetContentNode());
        if (pThisReg && pOtherReg)
        {
            return (nContent >= rPos.nContent);
        }
        else // by convention position with no index is smaller
        {
            return pOtherReg == nullptr;
        }
    }
    return nNode > rPos.nNode;
}

bool SwPosition::operator==(const SwPosition &rPos) const
{
    return (nNode == rPos.nNode)
        && (nContent == rPos.nContent);
}

bool SwPosition::operator!=(const SwPosition &rPos) const
{
    return (nNode != rPos.nNode)
        || (nContent != rPos.nContent);
}

SwDoc& SwPosition::GetDoc() const
{
    return GetNode().GetDoc();
}

void SwPosition::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwPosition"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nNode"), BAD_CAST(OString::number(sal_Int32(GetNodeIndex())).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nContent"), BAD_CAST(OString::number(GetContentIndex()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

void SwPosition::Assign( const SwNode& rNd, SwNodeOffset nDelta, sal_Int32 nContentOffset )
{
    nNode.Assign(rNd, nDelta);
    assert((nNode.GetNode().GetContentNode() || nContentOffset == 0) && "setting contentoffset, but node is not SwContentNode");
    nContent.Assign(nNode.GetNode().GetContentNode(), nContentOffset);
}
void SwPosition::Assign( SwNodeOffset nNodeOffset, sal_Int32 nContentOffset )
{
    nNode = nNodeOffset;
    nContent.Assign(nNode.GetNode().GetContentNode(), nContentOffset);
}
void SwPosition::Assign( const SwContentNode& rNode, sal_Int32 nContentOffset )
{
    nNode = rNode;
    nContent.Assign(&rNode, nContentOffset);
}
void SwPosition::Assign( const SwNode& rNd, sal_Int32 nContentOffset )
{
    nNode.Assign(rNd);
    nContent.Assign(rNd.GetContentNode(), nContentOffset);
}
void SwPosition::Assign( const SwNodeIndex& rNdIdx, sal_Int32 nContentOffset )
{
    nNode = rNdIdx;
    nContent.Assign(nNode.GetNode().GetContentNode(), nContentOffset);
}
void SwPosition::Adjust( SwNodeOffset nDelta )
{
    nNode += nDelta;
    nContent.Assign(nNode.GetNode().GetContentNode(), 0);
}
void SwPosition::AdjustContent( sal_Int32 nDelta )
{
    assert(nNode.GetNode().GetContentNode() && "only valid to call this if we point to an SwContentNode");
    nContent += nDelta;
}
void SwPosition::SetContent( sal_Int32 nContentIndex )
{
    assert(nNode.GetNode().GetContentNode() && "only valid to call this if we point to an SwContentNode");
    nContent = nContentIndex;
}
void SwPosition::AssignStartIndex( const SwContentNode& rNd )
{
    nNode = rNd;
    nContent.Assign(&rNd, 0);
}
void SwPosition::AssignEndIndex( const SwContentNode& rNd )
{
    nNode = rNd;
    nContent.Assign(&rNd, rNd.Len());
}


std::ostream &operator <<(std::ostream& s, const SwPosition& position)
{
    return s << "SwPosition (node " << position.GetNodeIndex() << ", offset " << position.GetContentIndex() << ")";
}

namespace {

enum CHKSECTION { Chk_Both, Chk_One, Chk_None };

}

static CHKSECTION lcl_TstIdx( SwNodeOffset nSttIdx, SwNodeOffset nEndIdx, const SwNode& rEndNd )
{
    SwNodeOffset nStt = rEndNd.StartOfSectionIndex(), nEnd = rEndNd.GetIndex();
    CHKSECTION eSec = nStt < nSttIdx && nEnd >= nSttIdx ? Chk_One : Chk_None;
    if( nStt < nEndIdx && nEnd >= nEndIdx )
        return( eSec == Chk_One ? Chk_Both : Chk_One );
    return eSec;
}

static bool lcl_ChkOneRange( CHKSECTION eSec, bool bChkSections,
                    const SwNode& rBaseEnd, SwNodeOffset nStt, SwNodeOffset nEnd )
{
    if( eSec != Chk_Both )
        return false;

    if( !bChkSections )
        return true;

    // search the surrounding section
    const SwNodes& rNds = rBaseEnd.GetNodes();
    const SwNode *pTmp, *pNd = rNds[ nStt ];
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();

    if( pNd == rNds[ nEnd ]->StartOfSectionNode() )
        return true; // same StartNode, same section

    // already on a base node => error
    if( !pNd->StartOfSectionIndex() )
        return false;

    for (;;)
    {
        pTmp = pNd->StartOfSectionNode();
        if (pTmp->EndOfSectionNode() == &rBaseEnd )
            break;
        pNd = pTmp;
    }

    SwNodeOffset nSttIdx = pNd->GetIndex(), nEndIdx = pNd->EndOfSectionIndex();
    return nSttIdx <= nStt && nStt <= nEndIdx &&
           nSttIdx <= nEnd && nEnd <= nEndIdx;
}

/** Check if the given range is inside one of the defined top-level sections.
 *
 * The top-level sections are Content, AutoText, PostIts, Inserts, and Redlines.
 *
 * @param bChkSection   if true, also check that the given range is inside
 *                      a single second-level section inside any of the
 *                      top-level sections, except for the Content section.
 *
 * @return <true> if valid range
 */
bool CheckNodesRange( const SwNode& rStt,
                      const SwNode& rEnd, bool bChkSection )
{
    const SwNodes& rNds = rStt.GetNodes();
    SwNodeOffset nStt = rStt.GetIndex(), nEnd = rEnd.GetIndex();
    CHKSECTION eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfContent() );
    if( Chk_None != eSec )
        return eSec == Chk_Both;

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfAutotext() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfAutotext(), nStt, nEnd );

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfPostIts() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfPostIts(), nStt, nEnd );

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfInserts() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfInserts(), nStt, nEnd );

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfRedlines() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfRedlines(), nStt, nEnd );

    return false; // somewhere in between => error
}

bool GoNext(SwNode& rNd, SwContentIndex& rIdx, SwCursorSkipMode nMode )
{
    if( rNd.IsContentNode() )
        return static_cast<SwContentNode&>(rNd).GoNext( rIdx, nMode );
    return false;
}

bool GoPrevious( SwNode& rNd, SwContentIndex& rIdx, SwCursorSkipMode nMode )
{
    if( rNd.IsContentNode() )
        return static_cast<SwContentNode&>(rNd).GoPrevious( rIdx, nMode );
    return false;
}

SwContentNode* GoNextNds( SwNodeIndex* pIdx, bool bChk )
{
    SwNodeIndex aIdx( *pIdx );
    SwContentNode* pNd = aIdx.GetNodes().GoNext( &aIdx );
    if( pNd )
    {
        if( bChk && SwNodeOffset(1) != aIdx.GetIndex() - pIdx->GetIndex() &&
            !CheckNodesRange( pIdx->GetNode(), aIdx.GetNode(), true ) )
                pNd = nullptr;
        else
            *pIdx = aIdx;
    }
    return pNd;
}

SwContentNode* GoPreviousNds( SwNodeIndex * pIdx, bool bChk )
{
    SwNodeIndex aIdx( *pIdx );
    SwContentNode* pNd = SwNodes::GoPrevious( &aIdx );
    if( pNd )
    {
        if( bChk && SwNodeOffset(1) != pIdx->GetIndex() - aIdx.GetIndex() &&
            !CheckNodesRange( pIdx->GetNode(), aIdx.GetNode(), true ) )
                pNd = nullptr;
        else
            *pIdx = aIdx;
    }
    return pNd;
}

SwContentNode* GoNextPos( SwPosition* pIdx, bool bChk )
{
    SwNodeIndex aIdx( pIdx->GetNode() );
    SwContentNode* pNd = aIdx.GetNodes().GoNext( &aIdx );
    if( pNd )
    {
        if( bChk && SwNodeOffset(1) != aIdx.GetIndex() - pIdx->GetNodeIndex() &&
            !CheckNodesRange( pIdx->GetNode(), aIdx.GetNode(), true ) )
                pNd = nullptr;
        else
            pIdx->Assign(aIdx);
    }
    return pNd;
}

SwContentNode* GoPreviousPos( SwPosition * pIdx, bool bChk )
{
    SwNodeIndex aIdx( pIdx->GetNode() );
    SwContentNode* pNd = SwNodes::GoPrevious( &aIdx );
    if( pNd )
    {
        if( bChk && SwNodeOffset(1) != pIdx->GetNodeIndex() - aIdx.GetIndex() &&
            !CheckNodesRange( pIdx->GetNode(), aIdx.GetNode(), true ) )
                pNd = nullptr;
        else
            pIdx->Assign(aIdx);
    }
    return pNd;
}

SwPaM::SwPaM( const SwPosition& rPos, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rPos )
    , m_Bound2( rPos.GetNode().GetNodes() ) // default initialize
    , m_pPoint( &m_Bound1 )
    , m_pMark( m_pPoint )
    , m_bIsInFrontOfLabel( false )
{
}

SwPaM::SwPaM( const SwPosition& rMark, const SwPosition& rPoint, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
}

SwPaM::SwPaM( const SwNodeIndex& rMark, const SwNodeIndex& rPoint,
              SwNodeOffset nMarkOffset, SwNodeOffset nPointOffset, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    if ( nMarkOffset )
    {
        m_pMark->nNode += nMarkOffset;
    }
    if ( nPointOffset )
    {
        m_pPoint->nNode += nPointOffset;
    }
    m_Bound1.nContent.Assign( m_Bound1.GetNode().GetContentNode(), 0 );
    m_Bound2.nContent.Assign( m_Bound2.GetNode().GetContentNode(), 0 );
}

SwPaM::SwPaM( const SwNode& rMark, const SwNode& rPoint,
              SwNodeOffset nMarkOffset, SwNodeOffset nPointOffset, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    if ( nMarkOffset )
    {
        m_pMark->nNode += nMarkOffset;
    }
    if ( nPointOffset )
    {
        m_pPoint->nNode += nPointOffset;
    }
    m_Bound1.nContent.Assign( m_Bound1.GetNode().GetContentNode(), 0 );
    m_Bound2.nContent.Assign( m_Bound2.GetNode().GetContentNode(), 0 );
}

SwPaM::SwPaM( const SwNodeIndex& rMark, sal_Int32 nMarkContent,
              const SwNodeIndex& rPoint, sal_Int32 nPointContent, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( rPoint.GetNode().GetContentNode(), nPointContent);
    m_pMark ->nContent.Assign( rMark .GetNode().GetContentNode(), nMarkContent );
}

SwPaM::SwPaM( const SwNode& rMark, sal_Int32 nMarkContent,
              const SwNode& rPoint, sal_Int32 nPointContent, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( m_pPoint->GetNode().GetContentNode(),
        nPointContent);
    m_pMark ->nContent.Assign( m_pMark ->GetNode().GetContentNode(),
        nMarkContent );
}

SwPaM::SwPaM( const SwNode& rMark, SwNodeOffset nMarkOffset, sal_Int32 nMarkContent,
              const SwNode& rPoint, SwNodeOffset nPointOffset, sal_Int32 nPointContent, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    if ( nMarkOffset )
    {
        m_pMark->nNode += nMarkOffset;
    }
    if ( nPointOffset )
    {
        m_pPoint->nNode += nPointOffset;
    }
    m_pPoint->nContent.Assign( m_pPoint->GetNode().GetContentNode(),
        nPointContent);
    m_pMark ->nContent.Assign( m_pMark ->GetNode().GetContentNode(),
        nMarkContent );
}

SwPaM::SwPaM( const SwNode& rNode, sal_Int32 nContent, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rNode )
    , m_Bound2( m_Bound1.GetNode().GetNodes() ) // default initialize
    , m_pPoint( &m_Bound1 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( m_pPoint->GetNode().GetContentNode(),
        nContent );
}

SwPaM::SwPaM( const SwNode& rNode, SwNodeOffset nNdOffset, sal_Int32 nContent, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rNode, nNdOffset )
    , m_Bound2( m_Bound1.GetNode().GetNodes() ) // default initialize
    , m_pPoint( &m_Bound1 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( m_pPoint->GetNode().GetContentNode(),
        nContent );
}

SwPaM::SwPaM( const SwNodeIndex& rNodeIdx, sal_Int32 nContent, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rNodeIdx )
    , m_Bound2( rNodeIdx.GetNode().GetNodes() ) // default initialize
    , m_pPoint( &m_Bound1 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( rNodeIdx.GetNode().GetContentNode(), nContent );
}

SwPaM::SwPaM( SwNodes& rNodes, SwNodeOffset nNdOffset, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rNodes, nNdOffset )
    , m_Bound2( rNodes ) // default initialize
    , m_pPoint( &m_Bound1 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
}

SwPaM::~SwPaM() {}

SwPaM::SwPaM(SwPaM const& rPam, SwPaM *const pRing)
    : Ring(pRing)
    , m_Bound1( *(rPam.m_pPoint) )
    , m_Bound2( *(rPam.m_pMark)  )
    , m_pPoint( &m_Bound1 ), m_pMark( rPam.HasMark() ? &m_Bound2 : m_pPoint )
    , m_bIsInFrontOfLabel( false )
{
}

// @@@ semantic: no copy assignment for super class Ring.
SwPaM &SwPaM::operator=( const SwPaM &rPam )
{
    if(this == &rPam)
        return *this;

    *m_pPoint = *( rPam.m_pPoint );
    if ( rPam.HasMark() )
    {
        SetMark();
        *m_pMark = *( rPam.m_pMark );
    }
    else
    {
        DeleteMark();
    }
    return *this;
}

void SwPaM::SetMark()
{
    if (m_pPoint == &m_Bound1)
    {
        m_pMark = &m_Bound2;
    }
    else
    {
        m_pMark = &m_Bound1;
    }
    (*m_pMark) = *m_pPoint;
}

/// movement of cursor
bool SwPaM::Move( SwMoveFnCollection const & fnMove, SwGoInDoc fnGo )
{
    const bool bRet = (*fnGo)( *this, fnMove );

    m_bIsInFrontOfLabel = false;
    return bRet;
}

namespace sw {

/** make a new region

    Sets the first SwPaM onto the given SwPaM, or to the beginning or end of a
    document. SPoint stays at its position, GetMark will be changed respectively.

    @param fnMove  Contains information if beginning or end of document.
    @param pOrigRg The given region.
    @param rPam    returns newly created range, in Ring with parameter pOrigRg.
*/
void MakeRegion(SwMoveFnCollection const & fnMove,
        const SwPaM & rOrigRg, std::optional<SwPaM>& rPam)
{
    rPam.emplace(rOrigRg, const_cast<SwPaM*>(&rOrigRg)); // given search range
    // make sure that SPoint is on the "real" start position
    // FORWARD: SPoint always smaller than GetMark
    // BACKWARD: SPoint always bigger than GetMark
    if( (rPam->GetMark()->*fnMove.fnCmpOp)( *rPam->GetPoint() ) )
        rPam->Exchange();
}

} // namespace sw

void SwPaM::Normalize(bool bPointFirst)
{
    if (HasMark())
        if ( ( bPointFirst && *m_pPoint > *m_pMark) ||
             (!bPointFirst && *m_pPoint < *m_pMark) )
        {
            Exchange();
        }
}

/// return page number at cursor (for reader and page bound frames)
sal_uInt16 SwPaM::GetPageNum( bool bAtPoint, const Point* pLayPos )
{
    const SwContentFrame* pCFrame;
    const SwPageFrame *pPg;
    const SwContentNode *pNd ;
    const SwPosition* pPos = bAtPoint ? m_pPoint : m_pMark;

    std::pair<Point, bool> tmp;
    if (pLayPos)
    {
        tmp.first = *pLayPos;
        tmp.second = false;
    }
    if( nullptr != ( pNd = pPos->GetNode().GetContentNode() ) &&
        nullptr != (pCFrame = pNd->getLayoutFrame(pNd->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(), pPos, pLayPos ? &tmp : nullptr)) &&
        nullptr != ( pPg = pCFrame->FindPageFrame() ))
        return pPg->GetPhyPageNum();
    return 0;
}

// form view - see also SwCursorShell::IsCursorReadonly()
static const SwFrame* lcl_FindEditInReadonlyFrame( const SwFrame& rFrame )
{
    const SwFrame* pRet = nullptr;

    const SwFlyFrame* pFly;
    const SwSectionFrame* pSectionFrame;

    if( rFrame.IsInFly() &&
       (pFly = rFrame.FindFlyFrame())->GetFormat()->GetEditInReadonly().GetValue() &&
        pFly->Lower() &&
       !pFly->Lower()->IsNoTextFrame() )
    {
       pRet = pFly;
    }
    else if ( rFrame.IsInSct() &&
              nullptr != ( pSectionFrame = rFrame.FindSctFrame() )->GetSection() &&
              pSectionFrame->GetSection()->IsEditInReadonlyFlag() )
    {
        pRet = pSectionFrame;
    }

    return pRet;
}

/// is in protected section or selection surrounds something protected
bool SwPaM::HasReadonlySel(bool bFormView, bool const isReplace) const
{
    bool bRet = false;

    const SwContentNode* pNd = GetPoint()->GetNode().GetContentNode();
    const SwContentFrame *pFrame = nullptr;
    if ( pNd != nullptr )
    {
        Point aTmpPt;
        std::pair<Point, bool> const tmp(aTmpPt, false);
        pFrame = pNd->getLayoutFrame(
            pNd->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(),
            GetPoint(), &tmp);
    }

    // Will be set if point are inside edit-in-readonly environment
    const SwFrame* pPointEditInReadonlyFrame = nullptr;
    if ( pFrame != nullptr
         && ( pFrame->IsProtected()
              || ( bFormView
                   && nullptr == ( pPointEditInReadonlyFrame = lcl_FindEditInReadonlyFrame( *pFrame ) ) ) ) )
    {
        bRet = true;
    }
    else if( pNd != nullptr )
    {
        const SwSectionNode* pSNd = pNd->GetSectionNode();
        if ( pSNd != nullptr
             && ( pSNd->GetSection().IsProtectFlag()
                  || ( bFormView
                       && !pSNd->GetSection().IsEditInReadonlyFlag()) ) )
        {
            bRet = true;
        }
        else
        {
            const SwSectionNode* pParentSectionNd = pNd->FindSectionNode();
            if ( pParentSectionNd != nullptr
                 && ( pParentSectionNd->GetSection().IsProtectFlag()
                      || ( bFormView && !pParentSectionNd->GetSection().IsEditInReadonlyFlag()) ) )
            {
                bRet = true;
            }
        }
    }

    if ( !bRet
         && HasMark()
         && GetPoint()->nNode != GetMark()->nNode )
    {
        pNd = GetMark()->GetNode().GetContentNode();
        pFrame = nullptr;
        if ( pNd != nullptr )
        {
            Point aTmpPt;
            std::pair<Point, bool> const tmp(aTmpPt, false);
            pFrame = pNd->getLayoutFrame(
                pNd->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(),
                GetMark(), &tmp);
        }

        const SwFrame* pMarkEditInReadonlyFrame = nullptr;
        if ( pFrame != nullptr
             && ( pFrame->IsProtected()
                  || ( bFormView
                       && nullptr == ( pMarkEditInReadonlyFrame = lcl_FindEditInReadonlyFrame( *pFrame ) ) ) ) )
        {
            bRet = true;
        }
        else if( pNd != nullptr )
        {
            const SwSectionNode* pSNd = pNd->GetSectionNode();
            if ( pSNd != nullptr
                 && ( pSNd->GetSection().IsProtectFlag()
                      || ( bFormView
                           && !pSNd->GetSection().IsEditInReadonlyFlag()) ) )
            {
                bRet = true;
            }
        }

        if ( !bRet && bFormView )
        {
           // Check if start and end frame are inside the _same_
           // edit-in-readonly-environment. Otherwise we better return 'true'
           if ( pPointEditInReadonlyFrame != pMarkEditInReadonlyFrame )
                bRet = true;
        }

        // check for protected section inside the selection
        if( !bRet )
        {
            SwNodeOffset nSttIdx = GetMark()->GetNodeIndex(),
                    nEndIdx = GetPoint()->GetNodeIndex();
            if( nEndIdx < nSttIdx )
                std::swap( nSttIdx, nEndIdx );

            // If a protected section should be between nodes, then the
            // selection needs to contain already x nodes.
            // (TextNd, SectNd, TextNd, EndNd, TextNd )
            if( nSttIdx + SwNodeOffset(3) < nEndIdx )
            {
                const SwSectionFormats& rFormats = GetDoc().GetSections();
                for( SwSectionFormats::size_type n = rFormats.size(); n;  )
                {
                    const SwSectionFormat* pFormat = rFormats[ --n ];
                    if( pFormat->GetProtect().IsContentProtected() )
                    {
                        const SwFormatContent& rContent = pFormat->GetContent(false);
                        OSL_ENSURE( rContent.GetContentIdx(), "where is the SectionNode?" );
                        SwNodeOffset nIdx = rContent.GetContentIdx()->GetIndex();
                        if( nSttIdx <= nIdx && nEndIdx >= nIdx &&
                            rContent.GetContentIdx()->GetNode().GetNodes().IsDocNodes() )
                        {
                            bRet = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    const SwDoc& rDoc = GetDoc();
    // Legacy text/combo/checkbox: never return read-only when inside these form fields.
    const IDocumentMarkAccess* pMarksAccess = rDoc.getIDocumentMarkAccess();
    sw::mark::IFieldmark* pA = GetPoint() ? pMarksAccess->getInnerFieldmarkFor(*GetPoint()) : nullptr;
    sw::mark::IFieldmark* pB = GetMark()  ? pMarksAccess->getInnerFieldmarkFor(*GetMark()) : pA;
    // prevent the user from accidentally deleting the field itself when modifying the text.
    const bool bAtStartA = (pA != nullptr) && (pA->GetMarkStart() == *GetPoint());
    const bool bAtStartB = (pB != nullptr) && (pB->GetMarkStart() == *GetMark());

    if (!utl::ConfigManager::IsFuzzing() && officecfg::Office::Common::Filter::Microsoft::Import::ForceImportWWFieldsAsGenericFields::get())
    {
        ; // allow editing all fields in generic mode
    }
    else if (!bRet)
    {
        bool bUnhandledMark = pA && pA->GetFieldname( ) == ODF_UNHANDLED;
        // Unhandled fieldmarks case shouldn't be edited manually to avoid breaking anything
        if ( ( pA == pB ) && bUnhandledMark )
            bRet = true;
        else
        {
            if ((pA == pB) && (bAtStartA != bAtStartB))
                bRet = true;
            else if (pA != pB)
            {
                // If both points are either outside or at marks edges (i.e. selection either
                // touches fields, or fully encloses it), then don't disable editing
                bRet = !( ( !pA || bAtStartA ) && ( !pB || bAtStartB ) );
            }
            if( !bRet && rDoc.GetDocumentSettingManager().get( DocumentSettingId::PROTECT_FORM ) && (pA || pB) )
            {
                // Form protection case
                bRet = ( pA == nullptr ) || ( pB == nullptr ) || bAtStartA || bAtStartB;
            }
        }
    }
    else
    {
        // Allow editing when the cursor/selection is fully inside of a legacy form field.
        bRet = !( pA != nullptr && !bAtStartA && !bAtStartB && pA == pB );

        if (bRet)
        {
            // Also allow editing inside content controls in general, similar to form fields.
            // Specific types will be disabled below.
            if (const SwEditShell* pEditShell = rDoc.GetEditShell())
                bRet = !pEditShell->CursorInsideContentControl();
        }
    }

    if (!bRet)
    {
        // Paragraph Signatures and Classification fields are read-only.
        if (const SwEditShell* pEditShell = rDoc.GetEditShell())
            bRet = pEditShell->IsCursorInParagraphMetadataField();
    }

    if (!bRet &&
        rDoc.getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_BOOKMARKS))
    {
        if (rDoc.getIDocumentMarkAccess()->isBookmarkDeleted(*this, isReplace))
        {
            return true;
        }
    }
    if (!bRet &&
        rDoc.getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_FIELDS))
    {
        SwPosition const& rStart(*Start());
        SwPosition const& rEnd(*End());
        for (SwNodeIndex n(rStart.GetNode()); n <= rEnd.GetNode(); ++n)
        {
            if (SwTextNode const*const pNode = n.GetNode().GetTextNode())
            {
                if (SwpHints const*const pHints = pNode->GetpSwpHints())
                {
                    for (size_t i = 0; i < pHints->Count(); ++i)
                    {
                        SwTextAttr const*const pHint(pHints->Get(i));
                        if (n == rStart.GetNode() && pHint->GetStart() < rStart.GetContentIndex())
                        {
                            continue; // before selection
                        }
                        if (n == rEnd.GetNode() && rEnd.GetContentIndex() <= pHint->GetStart())
                        {
                            break; // after selection
                        }
                        if (pHint->Which() == RES_TXTATR_FIELD
                            // placeholders don't work if you can't delete them
                            && pHint->GetFormatField().GetField()->GetTyp()->Which() != SwFieldIds::JumpEdit)
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }

    if (!bRet)
    {
        // See if we're inside a read-only content control.
        const SwPosition* pStart = Start();
        SwTextNode* pTextNode = pStart->GetNode().GetTextNode();
        if (pTextNode)
        {
            sal_Int32 nIndex = pStart->GetContentIndex();
            SwTextAttr* pAttr
                = pTextNode->GetTextAttrAt(nIndex, RES_TXTATR_CONTENTCONTROL, ::sw::GetTextAttrMode::Parent);
            auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
            if (pTextContentControl)
            {
                const SwFormatContentControl& rFormatContentControl
                    = pTextContentControl->GetContentControl();
                std::shared_ptr<SwContentControl> pContentControl
                    = rFormatContentControl.GetContentControl();
                if (pContentControl && !pContentControl->GetReadWrite())
                {
                    switch (pContentControl->GetType())
                    {
                        case SwContentControlType::CHECKBOX:
                        case SwContentControlType::PICTURE:
                        case SwContentControlType::DROP_DOWN_LIST:
                            bRet = true;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    return bRet;
}

bool SwPaM::HasHiddenSections() const
{
    bool bRet = false;

    if (HasMark() && GetPoint()->nNode != GetMark()->nNode)
    {
        // check for hidden section inside the selection
        SwNodeOffset nSttIdx = Start()->GetNodeIndex(), nEndIdx = End()->GetNodeIndex();

        if (nSttIdx + SwNodeOffset(3) < nEndIdx)
        {
            const SwSectionFormats& rFormats = GetDoc().GetSections();
            for (SwSectionFormats::size_type n = rFormats.size(); n;)
            {
                const SwSectionFormat* pFormat = rFormats[--n];
                if (pFormat->GetSection()->IsHidden())
                {
                    const SwFormatContent& rContent = pFormat->GetContent(false);
                    OSL_ENSURE(rContent.GetContentIdx(), "where is the SectionNode?");
                    SwNodeOffset nIdx = rContent.GetContentIdx()->GetIndex();
                    if (nSttIdx <= nIdx && nEndIdx >= nIdx
                        && rContent.GetContentIdx()->GetNode().GetNodes().IsDocNodes())
                    {
                        bRet = true;
                        break;
                    }
                }
            }
        }
    }

    return bRet;
}

/// This function returns the next node in direction of search. If there is no
/// left or the next is out of the area, then a null-pointer is returned.
/// @param rbFirst If <true> then first time request. If so than the position of
///        the PaM must not be changed!
SwContentNode* GetNode( SwPaM & rPam, bool& rbFirst, SwMoveFnCollection const & fnMove,
        bool const bInReadOnly, SwRootFrame const*const i_pLayout)
{
    SwRootFrame const*const pLayout(i_pLayout ? i_pLayout :
        rPam.GetDoc().getIDocumentLayoutAccess().GetCurrentLayout());
    SwContentNode * pNd = nullptr;
    if( ((*rPam.GetPoint()).*fnMove.fnCmpOp)( *rPam.GetMark() ) ||
        ( *rPam.GetPoint() == *rPam.GetMark() && rbFirst ) )
    {
        if( rbFirst )
        {
            rbFirst = false;
            pNd = rPam.GetPointContentNode();
            if( pNd )
            {
                SwContentFrame const*const pFrame(pNd->getLayoutFrame(pLayout));
                if(
                    (
                        nullptr == pFrame ||
                        ( !bInReadOnly && pFrame->IsProtected() ) ||
                        pFrame->IsHiddenNow()
                    ) ||
                    ( !bInReadOnly && pNd->FindSectionNode() &&
                        pNd->FindSectionNode()->GetSection().IsProtect()
                    )
                  )
                {
                    pNd = nullptr;
                }
            }
        }

        if( !pNd ) // is the cursor not on a ContentNode?
        {
            SwPosition aPos( *rPam.GetPoint() );
            bool bSrchForward = &fnMove == &fnMoveForward;
            SwNodes& rNodes = aPos.GetNodes();

            // go to next/previous ContentNode
            while( true )
            {
                if (i_pLayout && aPos.GetNode().IsTextNode())
                {
                    auto const fal(sw::GetFirstAndLastNode(*pLayout, aPos.GetNode()));
                    aPos.Assign( bSrchForward ? *fal.second : *fal.first );
                }

                pNd = bSrchForward
                        ? rNodes.GoNextSection( &aPos, true, !bInReadOnly )
                        : SwNodes::GoPrevSection( &aPos, true, !bInReadOnly );
                if( pNd )
                {
                    if (!bSrchForward)
                        aPos.AssignEndIndex( *pNd );
                    // is the position still in the area
                    if( (aPos.*fnMove.fnCmpOp)( *rPam.GetMark() ) )
                    {
                        // only in AutoTextSection can be nodes that are hidden
                        SwContentFrame const*const pFrame(pNd->getLayoutFrame(pLayout));
                        if (nullptr == pFrame ||
                            ( !bInReadOnly && pFrame->IsProtected() ) ||
                            pFrame->IsHiddenNow())
                        {
                            pNd = nullptr;
                            continue;
                        }
                        *rPam.GetPoint() = aPos;
                    }
                    else
                        pNd = nullptr; // no valid node
                    break;
                }
                break;
            }
        }
    }
    return pNd;
}

void GoStartDoc( SwPosition * pPos )
{
    SwNodes& rNodes = pPos->GetNodes();
    pPos->Assign( *rNodes.GetEndOfContent().StartOfSectionNode() );
    // we always need to find a ContentNode!
    rNodes.GoNext( pPos );
}

void GoEndDoc( SwPosition * pPos )
{
    SwNodes& rNodes = pPos->GetNodes();
    pPos->Assign( rNodes.GetEndOfContent() );
    SwContentNode* pCNd = GoPreviousPos( pPos, true );
    if( pCNd )
        pPos->AssignEndIndex(*pCNd);
}

void GoStartSection( SwPosition * pPos )
{
    // jump to section's beginning
    SwNodes& rNodes = pPos->GetNodes();
    sal_uInt16 nLevel = SwNodes::GetSectionLevel( pPos->GetNode() );
    if( pPos->GetNode() < *rNodes.GetEndOfContent().StartOfSectionNode() )
        nLevel--;
    do { SwNodes::GoStartOfSection( &pPos->nNode ); } while( nLevel-- );

    // already on a ContentNode
    pPos->AssignStartIndex(*pPos->GetNode().GetContentNode());
}

void GoStartOfSection( SwPosition * pPos )
{
    // jump to section's beginning
    SwNodes::GoStartOfSection( &pPos->nNode );
    pPos->nContent.Assign(pPos->GetNode().GetContentNode(), 0);
}

/// go to the end of the current base section
void GoEndSection( SwPosition * pPos )
{
    // jump to section's beginning/end
    SwNodes& rNodes = pPos->GetNodes();
    sal_uInt16 nLevel = SwNodes::GetSectionLevel( pPos->GetNode() );
    if( pPos->GetNode() < *rNodes.GetEndOfContent().StartOfSectionNode() )
        nLevel--;
    do { SwNodes::GoEndOfSection( &pPos->nNode ); } while( nLevel-- );

    // now on an EndNode, thus to the previous ContentNode
    if( SwContentNode* pCNd = GoPreviousNds( &pPos->nNode, true ) )
        pPos->AssignEndIndex(*pCNd);
}

void GoEndOfSection( SwPosition * pPos )
{
    SwNodes::GoEndOfSection( &pPos->nNode );
    SwContentNode* pCNd = pPos->nNode.GetNode().GetContentNode();
    pPos->nContent.Assign(pCNd, pCNd ? pCNd->Len() : 0);
}

bool GoInDoc( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    (*fnMove.fnDoc)( rPam.GetPoint() );
    return true;
}

bool GoInSection( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    (*fnMove.fnSections)( rPam.GetPoint() );
    return true;
}

bool GoInNode( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    SwContentNode *pNd = (*fnMove.fnPos)( rPam.GetPoint(), true );
    if( pNd )
        rPam.GetPoint()->SetContent(
                        ::GetSttOrEnd( &fnMove == &fnMoveForward, *pNd ) );
    return pNd;
}

bool GoInContent( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    if( (*fnMove.fnNd)( rPam.GetPoint()->GetNode(),
                        rPam.GetPoint()->nContent, SwCursorSkipMode::Chars ))
        return true;
    return GoInNode( rPam, fnMove );
}

bool GoInContentCells( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    if( (*fnMove.fnNd)( rPam.GetPoint()->GetNode(),
                        rPam.GetPoint()->nContent, SwCursorSkipMode::Cells ))
        return true;
    return GoInNode( rPam, fnMove );
}

bool GoInContentSkipHidden( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    if( (*fnMove.fnNd)( rPam.GetPoint()->GetNode(),
                        rPam.GetPoint()->nContent, SwCursorSkipMode::Chars | SwCursorSkipMode::Hidden ) )
        return true;
    return GoInNode( rPam, fnMove );
}

bool GoInContentCellsSkipHidden( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    if( (*fnMove.fnNd)( rPam.GetPoint()->GetNode(),
                        rPam.GetPoint()->nContent, SwCursorSkipMode::Cells | SwCursorSkipMode::Hidden ) )
        return true;
    return GoInNode( rPam, fnMove );
}

bool GoPrevPara( SwPaM & rPam, SwMoveFnCollection const & aPosPara )
{
    if( rPam.Move( fnMoveBackward, GoInNode ) )
    {
        // always on a ContentNode
        SwPosition& rPos = *rPam.GetPoint();
        SwContentNode * pNd = rPos.GetNode().GetContentNode();
        rPos.SetContent( ::GetSttOrEnd( &aPosPara == &fnMoveForward, *pNd ) );
        return true;
    }
    return false;
}

bool GoCurrPara( SwPaM & rPam, SwMoveFnCollection const & aPosPara )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwContentNode * pNd = rPos.GetNode().GetContentNode();
    if( pNd )
    {
        const sal_Int32 nOld = rPos.GetContentIndex();
        const sal_Int32 nNew = &aPosPara == &fnMoveForward ? 0 : pNd->Len();
        // if already at beginning/end then to the next/previous
        if( nOld != nNew )
        {
            rPos.SetContent( nNew );
            return true;
        }
    }
    // move node to next/previous ContentNode
    if( ( &aPosPara==&fnParaStart && nullptr != ( pNd =
            GoPreviousPos( &rPos, true ))) ||
        ( &aPosPara==&fnParaEnd && nullptr != ( pNd =
            GoNextPos( &rPos, true ))) )
    {
        rPos.SetContent( ::GetSttOrEnd( &aPosPara == &fnMoveForward, *pNd ));
        return true;
    }
    return false;
}

bool GoNextPara( SwPaM & rPam, SwMoveFnCollection const & aPosPara )
{
    if( rPam.Move( fnMoveForward, GoInNode ) )
    {
        // always on a ContentNode
        SwPosition& rPos = *rPam.GetPoint();
        SwContentNode * pNd = rPos.GetNode().GetContentNode();
        rPos.SetContent( ::GetSttOrEnd( &aPosPara == &fnMoveForward, *pNd ) );
        return true;
    }
    return false;
}

bool GoCurrSection( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwPosition aSavePos( rPos ); // position for comparison
    (fnMove.fnSection)( &rPos );
    SwContentNode *pNd;
    if( nullptr == ( pNd = rPos.GetNode().GetContentNode()) &&
        nullptr == ( pNd = (*fnMove.fnPos)( &rPos, true )) )
    {
        rPos = aSavePos; // do not change cursor
        return false;
    }

    rPos.SetContent( ::GetSttOrEnd( &fnMove == &fnMoveForward, *pNd ) );
    return aSavePos != rPos;
}

OUString SwPaM::GetText() const
{
    OUStringBuffer aResult;

    SwNodeIndex aNodeIndex = Start()->nNode;

    // The first node can be already the end node.
    // Use a "forever" loop with an exit condition in the middle
    // of its body, in order to correctly handle all cases.
    bool bIsStartNode = true;
    for (;;)
    {
        const bool bIsEndNode = aNodeIndex == End()->nNode;
        SwTextNode * pTextNode = aNodeIndex.GetNode().GetTextNode();

        if (pTextNode != nullptr)
        {
            if (!bIsStartNode)
            {
                aResult.append(CH_TXTATR_NEWLINE); // use newline for para break
            }
            const OUString& aTmpStr = pTextNode->GetText();

            if (aTmpStr.getLength() > 0 && (bIsStartNode || bIsEndNode))
            {
                // Handle corner cases of start/end node(s)
                const sal_Int32 nStart = bIsStartNode
                    ? Start()->GetContentIndex()
                    : 0;
                const sal_Int32 nEnd = bIsEndNode
                    ? End()->GetContentIndex()
                    : aTmpStr.getLength();

                aResult.append(aTmpStr.subView(nStart, nEnd-nStart));
            }
            else
            {
                aResult.append(aTmpStr);
            }
        }

        if (bIsEndNode)
        {
            break;
        }

        ++aNodeIndex;
        bIsStartNode = false;
    }

    return aResult.makeStringAndClear();
}

void SwPaM::InvalidatePaM()
{
    for (SwNodeIndex index(Start()->GetNode()); index <= End()->GetNode(); ++index)
    {
        if (SwTextNode *const pTextNode = index.GetNode().GetTextNode())
        {
            // pretend that the PaM marks changed formatting to reformat...
            sal_Int32 const nStart(
                index == Start()->nNode ? Start()->GetContentIndex() : 0);
            // this should work even for length of 0
            SwUpdateAttr const aHint(
                nStart,
                index == End()->nNode
                    ? End()->GetContentIndex() - nStart
                    : pTextNode->Len() - nStart,
                0);
            pTextNode->TriggerNodeUpdate(sw::LegacyModifyHint(&aHint, &aHint));
        }
        // other node types not invalidated
    }
}

void SwPaM::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwPaM"));

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("point"));
    GetPoint()->dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);

    if (HasMark())
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mark"));
        GetMark()->dumpAsXml(pWriter);
        (void)xmlTextWriterEndElement(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

std::ostream &operator <<(std::ostream& s, const SwPaM& pam)
{
    if( pam.HasMark())
        return s << "SwPaM (point " << *pam.GetPoint() << ", mark " << *pam.GetMark() << ")";
    else
        return s << "SwPaM (point " << *pam.GetPoint() << ")";
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
