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

#include <string_view>

#include <tools/gen.hxx>
#include <hintids.hxx>
#include <editeng/protitem.hxx>
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
#include <swtable.hxx>
#include <flyfrm.hxx>
#include <fmteiro.hxx>
#include <section.hxx>
#include <sectfrm.hxx>
#include <ndtxt.hxx>
#include <swcrsr.hxx>

#include <IMark.hxx>
#include <DocumentSettingManager.hxx>
#include <hints.hxx>
#include <xmloff/odffields.hxx>

#include <editsh.hxx>

// for the dump "MSC-" compiler
static sal_Int32 GetSttOrEnd( bool bCondition, const SwContentNode& rNd )
{
    return bCondition ? 0 : rNd.Len();
}

SwPosition::SwPosition( const SwNodeIndex & rNodeIndex, const SwIndex & rContent )
    : nNode( rNodeIndex ), nContent( rContent )
{
}

SwPosition::SwPosition( const SwNodeIndex & rNodeIndex )
    : nNode( rNodeIndex ), nContent( nNode.GetNode().GetContentNode() )
{
}

SwPosition::SwPosition( const SwNode& rNode )
    : nNode( rNode ), nContent( nNode.GetNode().GetContentNode() )
{
}

SwPosition::SwPosition( SwContentNode & rNode, const sal_Int32 nOffset )
    : nNode( rNode ), nContent( &rNode, nOffset )
{
}

bool SwPosition::operator<(const SwPosition &rPos) const
{
    if( nNode < rPos.nNode )
        return true;
    if( nNode == rPos.nNode )
    {
        // note that positions with text node but no SwIndex registered are
        // created for text frames anchored at para (see SwXFrame::getAnchor())
        SwIndexReg const*const pThisReg(nContent.GetIdxReg());
        SwIndexReg const*const pOtherReg(rPos.nContent.GetIdxReg());
        if (pThisReg && pOtherReg)
        {
            return (nContent < rPos.nContent);
        }
        else // by convention position with no index is smaller
        {
            return pOtherReg != nullptr;
        }
    }
    return false;
}

bool SwPosition::operator>(const SwPosition &rPos) const
{
    if(nNode > rPos.nNode )
        return true;
    if( nNode == rPos.nNode )
    {
        // note that positions with text node but no SwIndex registered are
        // created for text frames anchored at para (see SwXFrame::getAnchor())
        SwIndexReg const*const pThisReg(nContent.GetIdxReg());
        SwIndexReg const*const pOtherReg(rPos.nContent.GetIdxReg());
        if (pThisReg && pOtherReg)
        {
            return (nContent > rPos.nContent);
        }
        else // by convention position with no index is smaller
        {
            return pThisReg != nullptr;
        }
    }
    return false;
}

bool SwPosition::operator<=(const SwPosition &rPos) const
{
    if(nNode < rPos.nNode )
        return true;
    if( nNode == rPos.nNode )
    {
        // note that positions with text node but no SwIndex registered are
        // created for text frames anchored at para (see SwXFrame::getAnchor())
        SwIndexReg const*const pThisReg(nContent.GetIdxReg());
        SwIndexReg const*const pOtherReg(rPos.nContent.GetIdxReg());
        if (pThisReg && pOtherReg)
        {
            return (nContent <= rPos.nContent);
        }
        else // by convention position with no index is smaller
        {
            return pThisReg == nullptr;
        }
    }
    return false;
}

bool SwPosition::operator>=(const SwPosition &rPos) const
{
    if(nNode > rPos.nNode )
        return true;
    if( nNode == rPos.nNode )
    {
        // note that positions with text node but no SwIndex registered are
        // created for text frames anchored at para (see SwXFrame::getAnchor())
        SwIndexReg const*const pThisReg(nContent.GetIdxReg());
        SwIndexReg const*const pOtherReg(rPos.nContent.GetIdxReg());
        if (pThisReg && pOtherReg)
        {
            return (nContent >= rPos.nContent);
        }
        else // by convention position with no index is smaller
        {
            return pOtherReg == nullptr;
        }
    }
    return false;
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

SwDoc * SwPosition::GetDoc() const
{
    return nNode.GetNode().GetDoc();
}

void SwPosition::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwPosition"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nNode"), BAD_CAST(OString::number(nNode.GetIndex()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nContent"), BAD_CAST(OString::number(nContent.GetIndex()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

std::ostream &operator <<(std::ostream& s, const SwPosition& position)
{
    return s << "SwPosition (node " << position.nNode.GetIndex() << ", offset " << position.nContent.GetIndex() << ")";
}

enum CHKSECTION { Chk_Both, Chk_One, Chk_None };

static CHKSECTION lcl_TstIdx( sal_uLong nSttIdx, sal_uLong nEndIdx, const SwNode& rEndNd )
{
    sal_uLong nStt = rEndNd.StartOfSectionIndex(), nEnd = rEndNd.GetIndex();
    CHKSECTION eSec = nStt < nSttIdx && nEnd >= nSttIdx ? Chk_One : Chk_None;
    if( nStt < nEndIdx && nEnd >= nEndIdx )
        return( eSec == Chk_One ? Chk_Both : Chk_One );
    return eSec;
}

static bool lcl_ChkOneRange( CHKSECTION eSec, bool bChkSections,
                    const SwNode& rBaseEnd, sal_uLong nStt, sal_uLong nEnd )
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

    while( ( pTmp = pNd->StartOfSectionNode())->EndOfSectionNode() !=
            &rBaseEnd )
        pNd = pTmp;

    sal_uLong nSttIdx = pNd->GetIndex(), nEndIdx = pNd->EndOfSectionIndex();
    return nSttIdx <= nStt && nStt <= nEndIdx &&
           nSttIdx <= nEnd && nEnd <= nEndIdx;
}

bool CheckNodesRange( const SwNodeIndex& rStt,
                      const SwNodeIndex& rEnd, bool bChkSection )
{
    const SwNodes& rNds = rStt.GetNodes();
    sal_uLong nStt = rStt.GetIndex(), nEnd = rEnd.GetIndex();
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

bool GoNext(SwNode* pNd, SwIndex * pIdx, sal_uInt16 nMode )
{
    if( pNd->IsContentNode() )
        return static_cast<SwContentNode*>(pNd)->GoNext( pIdx, nMode );
    return false;
}

bool GoPrevious( SwNode* pNd, SwIndex * pIdx, sal_uInt16 nMode )
{
    if( pNd->IsContentNode() )
        return static_cast<SwContentNode*>(pNd)->GoPrevious( pIdx, nMode );
    return false;
}

SwContentNode* GoNextNds( SwNodeIndex* pIdx, bool bChk )
{
    SwNodeIndex aIdx( *pIdx );
    SwContentNode* pNd = aIdx.GetNodes().GoNext( &aIdx );
    if( pNd )
    {
        if( bChk && 1 != aIdx.GetIndex() - pIdx->GetIndex() &&
            !CheckNodesRange( *pIdx, aIdx, true ) )
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
        if( bChk && 1 != pIdx->GetIndex() - aIdx.GetIndex() &&
            !CheckNodesRange( *pIdx, aIdx, true ) )
                pNd = nullptr;
        else
            *pIdx = aIdx;
    }
    return pNd;
}

SwPaM::SwPaM( const SwPosition& rPos, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rPos )
    , m_Bound2( rPos.nNode.GetNode().GetNodes() ) // default initialize
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
              long nMarkOffset, long nPointOffset, SwPaM* pRing )
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
    m_Bound1.nContent.Assign( m_Bound1.nNode.GetNode().GetContentNode(), 0 );
    m_Bound2.nContent.Assign( m_Bound2.nNode.GetNode().GetContentNode(), 0 );
}

SwPaM::SwPaM( const SwNode& rMark, const SwNode& rPoint,
              long nMarkOffset, long nPointOffset, SwPaM* pRing )
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
    m_Bound1.nContent.Assign( m_Bound1.nNode.GetNode().GetContentNode(), 0 );
    m_Bound2.nContent.Assign( m_Bound2.nNode.GetNode().GetContentNode(), 0 );
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
    m_pPoint->nContent.Assign( m_pPoint->nNode.GetNode().GetContentNode(),
        nPointContent);
    m_pMark ->nContent.Assign( m_pMark ->nNode.GetNode().GetContentNode(),
        nMarkContent );
}

SwPaM::SwPaM( const SwNode& rNode, sal_Int32 nContent, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rNode )
    , m_Bound2( m_Bound1.nNode.GetNode().GetNodes() ) // default initialize
    , m_pPoint( &m_Bound1 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( m_pPoint->nNode.GetNode().GetContentNode(),
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

#ifdef DBG_UTIL
void SwPaM::Exchange()
{
    if (m_pPoint != m_pMark)
    {
        SwPosition *pTmp = m_pPoint;
        m_pPoint = m_pMark;
        m_pMark = pTmp;
    }
}
#endif

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

    @return Newly created range, in Ring with parameter pOrigRg.
*/
std::unique_ptr<SwPaM> MakeRegion(SwMoveFnCollection const & fnMove,
        const SwPaM & rOrigRg)
{
    std::unique_ptr<SwPaM> pPam;
    {
        pPam.reset(new SwPaM(rOrigRg, const_cast<SwPaM*>(&rOrigRg))); // given search range
        // make sure that SPoint is on the "real" start position
        // FORWARD: SPoint always smaller than GetMark
        // BACKWARD: SPoint always bigger than GetMark
        if( (pPam->GetMark()->*fnMove.fnCmpOp)( *pPam->GetPoint() ) )
            pPam->Exchange();
    }
    return pPam;
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
    if( nullptr != ( pNd = pPos->nNode.GetNode().GetContentNode() ) &&
        nullptr != (pCFrame = pNd->getLayoutFrame(pNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), pPos, pLayPos ? &tmp : nullptr)) &&
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
bool SwPaM::HasReadonlySel( bool bFormView ) const
{
    bool bRet = false;

    const SwContentNode* pNd = GetPoint()->nNode.GetNode().GetContentNode();
    const SwContentFrame *pFrame = nullptr;
    if ( pNd != nullptr )
    {
        Point aTmpPt;
        std::pair<Point, bool> const tmp(aTmpPt, false);
        pFrame = pNd->getLayoutFrame(
            pNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
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
        pNd = GetMark()->nNode.GetNode().GetContentNode();
        pFrame = nullptr;
        if ( pNd != nullptr )
        {
            Point aTmpPt;
            std::pair<Point, bool> const tmp(aTmpPt, false);
            pFrame = pNd->getLayoutFrame(
                pNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
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
            sal_uLong nSttIdx = GetMark()->nNode.GetIndex(),
                    nEndIdx = GetPoint()->nNode.GetIndex();
            if( nEndIdx <= nSttIdx )
            {
                sal_uLong nTmp = nSttIdx;
                nSttIdx = nEndIdx;
                nEndIdx = nTmp;
            }

            // If a protected section should be between nodes, then the
            // selection needs to contain already x nodes.
            // (TextNd, SectNd, TextNd, EndNd, TextNd )
            if( nSttIdx + 3 < nEndIdx )
            {
                const SwSectionFormats& rFormats = GetDoc()->GetSections();
                for( SwSectionFormats::size_type n = rFormats.size(); n;  )
                {
                    const SwSectionFormat* pFormat = rFormats[ --n ];
                    if( pFormat->GetProtect().IsContentProtected() )
                    {
                        const SwFormatContent& rContent = pFormat->GetContent(false);
                        OSL_ENSURE( rContent.GetContentIdx(), "where is the SectionNode?" );
                        sal_uLong nIdx = rContent.GetContentIdx()->GetIndex();
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

    const SwDoc *pDoc = GetDoc();
    const IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess();
    sw::mark::IFieldmark* pA = GetPoint() ? pMarksAccess->getFieldmarkFor( *GetPoint( ) ) : nullptr;
    sw::mark::IFieldmark* pB = GetMark()  ? pMarksAccess->getFieldmarkFor( *GetMark( ) ) : pA;

    if (!bRet)
    {
        bool bUnhandledMark = pA && pA->GetFieldname( ) == ODF_UNHANDLED;
        // Unhandled fieldmarks case shouldn't be edited manually to avoid breaking anything
        if ( ( pA == pB ) && bUnhandledMark )
            bRet = true;
        else
        {
            bool bAtStartA = (pA != nullptr) && (pA->GetMarkStart() == *GetPoint());
            bool bAtStartB = (pB != nullptr) && (pB->GetMarkStart() == *GetMark());

            if ((pA == pB) && (bAtStartA != bAtStartB))
                bRet = true;
            else if (pA != pB)
            {
                // If both points are either outside or at marks edges (i.e. selection either
                // touches fields, or fully encloses it), then don't disable editing
                bRet = !( ( !pA || bAtStartA ) && ( !pB || bAtStartB ) );
            }
            if( !bRet && pDoc->GetDocumentSettingManager().get( DocumentSettingId::PROTECT_FORM ) && (pA || pB) )
            {
                // Form protection case
                bRet = ( pA == nullptr ) || ( pB == nullptr ) || bAtStartA || bAtStartB;
            }
        }
    }
    else
    {
        bRet = !( pA == pB && pA != nullptr );
    }

    if (!bRet)
    {
        // Paragraph Signatures and Classification fields are read-only.
        if (pDoc && pDoc->GetEditShell())
            bRet = pDoc->GetEditShell()->IsCursorInParagraphMetadataField();
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
        rPam.GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout());
    SwContentNode * pNd = nullptr;
    if( ((*rPam.GetPoint()).*fnMove.fnCmpOp)( *rPam.GetMark() ) ||
        ( *rPam.GetPoint() == *rPam.GetMark() && rbFirst ) )
    {
        if( rbFirst )
        {
            rbFirst = false;
            pNd = rPam.GetContentNode();
            if( pNd )
            {
                SwContentFrame const*const pFrame(pNd->getLayoutFrame(pLayout));
                if(
                    (
                        nullptr == pFrame ||
                        ( !bInReadOnly && pFrame->IsProtected() ) ||
                        (pFrame->IsTextFrame() && static_cast<SwTextFrame const*>(pFrame)->IsHiddenNow())
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
            SwNodes& rNodes = aPos.nNode.GetNodes();

            // go to next/previous ContentNode
            while( true )
            {
                if (i_pLayout && aPos.nNode.GetNode().IsTextNode())
                {
                    auto const fal(sw::GetFirstAndLastNode(*pLayout, aPos.nNode));
                    aPos.nNode = bSrchForward ? *fal.second : *fal.first;
                }

                pNd = bSrchForward
                        ? rNodes.GoNextSection( &aPos.nNode, true, !bInReadOnly )
                        : SwNodes::GoPrevSection( &aPos.nNode, true, !bInReadOnly );
                if( pNd )
                {
                    aPos.nContent.Assign( pNd, ::GetSttOrEnd( bSrchForward,*pNd ));
                    // is the position still in the area
                    if( (aPos.*fnMove.fnCmpOp)( *rPam.GetMark() ) )
                    {
                        // only in AutoTextSection can be nodes that are hidden
                        SwContentFrame const*const pFrame(pNd->getLayoutFrame(pLayout));
                        if (nullptr == pFrame ||
                            ( !bInReadOnly && pFrame->IsProtected() ) ||
                            ( pFrame->IsTextFrame() &&
                                static_cast<SwTextFrame const*>(pFrame)->IsHiddenNow()))
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
    SwNodes& rNodes = pPos->nNode.GetNodes();
    pPos->nNode = *rNodes.GetEndOfContent().StartOfSectionNode();
    // we always need to find a ContentNode!
    SwContentNode* pCNd = rNodes.GoNext( &pPos->nNode );
    if( pCNd )
        pCNd->MakeStartIndex( &pPos->nContent );
}

void GoEndDoc( SwPosition * pPos )
{
    SwNodes& rNodes = pPos->nNode.GetNodes();
    pPos->nNode = rNodes.GetEndOfContent();
    SwContentNode* pCNd = GoPreviousNds( &pPos->nNode, true );
    if( pCNd )
        pCNd->MakeEndIndex( &pPos->nContent );
}

void GoStartSection( SwPosition * pPos )
{
    // jump to section's beginning
    SwNodes& rNodes = pPos->nNode.GetNodes();
    sal_uInt16 nLevel = SwNodes::GetSectionLevel( pPos->nNode );
    if( pPos->nNode < rNodes.GetEndOfContent().StartOfSectionIndex() )
        nLevel--;
    do { SwNodes::GoStartOfSection( &pPos->nNode ); } while( nLevel-- );

    // already on a ContentNode
    pPos->nNode.GetNode().GetContentNode()->MakeStartIndex( &pPos->nContent );
}

/// go to the end of the current base section
void GoEndSection( SwPosition * pPos )
{
    // jump to section's beginning/end
    SwNodes& rNodes = pPos->nNode.GetNodes();
    sal_uInt16 nLevel = SwNodes::GetSectionLevel( pPos->nNode );
    if( pPos->nNode < rNodes.GetEndOfContent().StartOfSectionIndex() )
        nLevel--;
    do { SwNodes::GoEndOfSection( &pPos->nNode ); } while( nLevel-- );

    // now on a EndNode, thus to the previous ContentNode
    if( GoPreviousNds( &pPos->nNode, true ) )
        pPos->nNode.GetNode().GetContentNode()->MakeEndIndex( &pPos->nContent );
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
    SwContentNode *pNd = (*fnMove.fnNds)( &rPam.GetPoint()->nNode, true );
    if( pNd )
        rPam.GetPoint()->nContent.Assign( pNd,
                        ::GetSttOrEnd( &fnMove == &fnMoveForward, *pNd ) );
    return pNd;
}

bool GoInContent( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    if( (*fnMove.fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                        &rPam.GetPoint()->nContent, CRSR_SKIP_CHARS ))
        return true;
    return GoInNode( rPam, fnMove );
}

bool GoInContentCells( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    if( (*fnMove.fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                         &rPam.GetPoint()->nContent, CRSR_SKIP_CELLS ))
        return true;
    return GoInNode( rPam, fnMove );
}

bool GoInContentSkipHidden( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    if( (*fnMove.fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                        &rPam.GetPoint()->nContent, CRSR_SKIP_CHARS | CRSR_SKIP_HIDDEN ) )
        return true;
    return GoInNode( rPam, fnMove );
}

bool GoInContentCellsSkipHidden( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    if( (*fnMove.fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                         &rPam.GetPoint()->nContent, CRSR_SKIP_CELLS | CRSR_SKIP_HIDDEN ) )
        return true;
    return GoInNode( rPam, fnMove );
}

bool GoPrevPara( SwPaM & rPam, SwMoveFnCollection const & aPosPara )
{
    if( rPam.Move( fnMoveBackward, GoInNode ) )
    {
        // always on a ContentNode
        SwPosition& rPos = *rPam.GetPoint();
        SwContentNode * pNd = rPos.nNode.GetNode().GetContentNode();
        rPos.nContent.Assign( pNd,
                            ::GetSttOrEnd( &aPosPara == &fnMoveForward, *pNd ) );
        return true;
    }
    return false;
}

bool GoCurrPara( SwPaM & rPam, SwMoveFnCollection const & aPosPara )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwContentNode * pNd = rPos.nNode.GetNode().GetContentNode();
    if( pNd )
    {
        const sal_Int32 nOld = rPos.nContent.GetIndex();
        const sal_Int32 nNew = &aPosPara == &fnMoveForward ? 0 : pNd->Len();
        // if already at beginning/end then to the next/previous
        if( nOld != nNew )
        {
            rPos.nContent.Assign( pNd, nNew );
            return true;
        }
    }
    // move node to next/previous ContentNode
    if( ( &aPosPara==&fnParaStart && nullptr != ( pNd =
            GoPreviousNds( &rPos.nNode, true ))) ||
        ( &aPosPara==&fnParaEnd && nullptr != ( pNd =
            GoNextNds( &rPos.nNode, true ))) )
    {
        rPos.nContent.Assign( pNd,
                        ::GetSttOrEnd( &aPosPara == &fnMoveForward, *pNd ));
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
        SwContentNode * pNd = rPos.nNode.GetNode().GetContentNode();
        rPos.nContent.Assign( pNd,
                        ::GetSttOrEnd( &aPosPara == &fnMoveForward, *pNd ) );
        return true;
    }
    return false;
}

bool GoCurrSection( SwPaM & rPam, SwMoveFnCollection const & fnMove )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwPosition aSavePos( rPos ); // position for comparison
    (fnMove.fnSection)( &rPos.nNode );
    SwContentNode *pNd;
    if( nullptr == ( pNd = rPos.nNode.GetNode().GetContentNode()) &&
        nullptr == ( pNd = (*fnMove.fnNds)( &rPos.nNode, true )) )
    {
        rPos = aSavePos; // do not change cursor
        return false;
    }

    rPos.nContent.Assign( pNd,
                        ::GetSttOrEnd( &fnMove == &fnMoveForward, *pNd ) );
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
            const OUString& aTmpStr = pTextNode->GetText();

            if (bIsStartNode || bIsEndNode)
            {
                // Handle corner cases of start/end node(s)
                const sal_Int32 nStart = bIsStartNode
                    ? Start()->nContent.GetIndex()
                    : 0;
                const sal_Int32 nEnd = bIsEndNode
                    ? End()->nContent.GetIndex()
                    : aTmpStr.getLength();

                aResult.append(std::u16string_view(aTmpStr).substr(nStart, nEnd-nStart));
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
    const SwNode &_pNd = GetNode();
    const SwTextNode *_pTextNd = _pNd.GetTextNode();
    if (_pTextNd != nullptr)
    {
        // pretend that the PaM marks inserted text to recalc the portion...
        SwInsText aHint( Start()->nContent.GetIndex(),
                        End()->nContent.GetIndex() - Start()->nContent.GetIndex() + 1 );
        SwModify *_pModify=const_cast<SwModify*>(static_cast<SwModify const *>(_pTextNd));
        _pModify->ModifyNotification( nullptr, &aHint);
    }
}

void SwPaM::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwPaM"));

    xmlTextWriterStartElement(pWriter, BAD_CAST("point"));
    GetPoint()->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);

    if (HasMark())
    {
        xmlTextWriterStartElement(pWriter, BAD_CAST("mark"));
        GetMark()->dumpAsXml(pWriter);
        xmlTextWriterEndElement(pWriter);
    }

    xmlTextWriterEndElement(pWriter);
}

std::ostream &operator <<(std::ostream& s, const SwPaM& pam)
{
    if( pam.HasMark())
        return s << "SwPaM (point " << *pam.GetPoint() << ", mark " << *pam.GetMark() << ")";
    else
        return s << "SwPaM (point " << *pam.GetPoint() << ")";
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
