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

#include <hintids.hxx>
#include <editeng/protitem.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <pamtyp.hxx>
#include <txtfrm.hxx>
#include <fmtcntnt.hxx>
#include <frmatr.hxx>
#include <swtable.hxx>
#include <crsskip.hxx>

// Formular view
#include <flyfrm.hxx>
#include <fmteiro.hxx>
#include <section.hxx>
#include <sectfrm.hxx>
#include <ndtxt.hxx>

#include <IMark.hxx>
#include <hints.hxx>

#include <xmloff/odffields.hxx>

// for the dump "MSC-" compiler
inline xub_StrLen GetSttOrEnd( sal_Bool bCondition, const SwCntntNode& rNd )
{
    return bCondition ? 0 : rNd.Len();
}

SwPosition::SwPosition( const SwNodeIndex & rNodeIndex, const SwIndex & rCntnt )
    : nNode( rNodeIndex ), nContent( rCntnt )
{
}

SwPosition::SwPosition( const SwNodeIndex & rNodeIndex )
    : nNode( rNodeIndex ), nContent( nNode.GetNode().GetCntntNode() )
{
}

SwPosition::SwPosition( const SwNode& rNode )
    : nNode( rNode ), nContent( nNode.GetNode().GetCntntNode() )
{
}

SwPosition::SwPosition( SwCntntNode & rNode, const xub_StrLen nOffset )
    : nNode( rNode ), nContent( &rNode, nOffset )
{
}


SwPosition::SwPosition( const SwPosition & rPos )
    : nNode( rPos.nNode ), nContent( rPos.nContent )
{
}

SwPosition &SwPosition::operator=(const SwPosition &rPos)
{
    nNode = rPos.nNode;
    nContent = rPos.nContent;
    return *this;
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
            return (pOtherReg) ? true : false;
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
            return (pThisReg) ? true : false;
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
            return (pThisReg) ? false : true;
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
            return (pOtherReg) ? false : true;
        }
    }
    return false;
}


bool SwPosition::operator==(const SwPosition &rPos) const
{
    return (nNode == rPos.nNode)
        // GetIndexReg may be null for FLY_AT_PARA frame anchor position
        && (nContent.GetIdxReg() == rPos.nContent.GetIdxReg())
        && (nContent == rPos.nContent);
}


bool SwPosition::operator!=(const SwPosition &rPos) const
{
    return (nNode != rPos.nNode)
        // GetIndexReg may be null for FLY_AT_PARA frame anchor position
        || (nContent.GetIdxReg() != rPos.nContent.GetIdxReg())
        || (nContent != rPos.nContent);
}

SwDoc * SwPosition::GetDoc() const
{
    return nNode.GetNode().GetDoc();
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


static sal_Bool lcl_ChkOneRange( CHKSECTION eSec, sal_Bool bChkSections,
                    const SwNode& rBaseEnd, sal_uLong nStt, sal_uLong nEnd )
{
    if( eSec != Chk_Both )
        return sal_False;

    if( !bChkSections )
        return sal_True;

    // search the surrounding section
    const SwNodes& rNds = rBaseEnd.GetNodes();
    const SwNode *pTmp, *pNd = rNds[ nStt ];
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();

    if( pNd == rNds[ nEnd ]->StartOfSectionNode() )
        return sal_True; // same StartNode, same section

    // already on a base node => error
    if( !pNd->StartOfSectionIndex() )
        return sal_False;

    while( ( pTmp = pNd->StartOfSectionNode())->EndOfSectionNode() !=
            &rBaseEnd )
        pNd = pTmp;

    sal_uLong nSttIdx = pNd->GetIndex(), nEndIdx = pNd->EndOfSectionIndex();
    return nSttIdx <= nStt && nStt <= nEndIdx &&
           nSttIdx <= nEnd && nEnd <= nEndIdx ? sal_True : sal_False;
}


sal_Bool CheckNodesRange( const SwNodeIndex& rStt,
                        const SwNodeIndex& rEnd, sal_Bool bChkSection )
{
    const SwNodes& rNds = rStt.GetNodes();
    sal_uLong nStt = rStt.GetIndex(), nEnd = rEnd.GetIndex();
    CHKSECTION eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfContent() );
    if( Chk_None != eSec ) return eSec == Chk_Both ? sal_True : sal_False;

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

    return sal_False; // somewhere in between => error
}


sal_Bool GoNext(SwNode* pNd, SwIndex * pIdx, sal_uInt16 nMode )
{
    if( pNd->IsCntntNode() )
        return ((SwCntntNode*)pNd)->GoNext( pIdx, nMode );
    return sal_False;
}


sal_Bool GoPrevious( SwNode* pNd, SwIndex * pIdx, sal_uInt16 nMode )
{
    if( pNd->IsCntntNode() )
        return ((SwCntntNode*)pNd)->GoPrevious( pIdx, nMode );
    return sal_False;
}


SwCntntNode* GoNextNds( SwNodeIndex* pIdx, sal_Bool bChk )
{
    SwNodeIndex aIdx( *pIdx );
    SwCntntNode* pNd = aIdx.GetNodes().GoNext( &aIdx );
    if( pNd )
    {
        if( bChk && 1 != aIdx.GetIndex() - pIdx->GetIndex() &&
            !CheckNodesRange( *pIdx, aIdx, sal_True ) )
                pNd = 0;
        else
            *pIdx = aIdx;
    }
    return pNd;
}


SwCntntNode* GoPreviousNds( SwNodeIndex * pIdx, sal_Bool bChk )
{
    SwNodeIndex aIdx( *pIdx );
    SwCntntNode* pNd = aIdx.GetNodes().GoPrevious( &aIdx );
    if( pNd )
    {
        if( bChk && 1 != pIdx->GetIndex() - aIdx.GetIndex() &&
            !CheckNodesRange( *pIdx, aIdx, sal_True ) )
                pNd = 0;
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

    m_Bound1.nContent.Assign( m_Bound1.nNode.GetNode().GetCntntNode(), 0 );
    m_Bound2.nContent.Assign( m_Bound2.nNode.GetNode().GetCntntNode(), 0 );
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

    m_Bound1.nContent.Assign( m_Bound1.nNode.GetNode().GetCntntNode(), 0 );
    m_Bound2.nContent.Assign( m_Bound2.nNode.GetNode().GetCntntNode(), 0 );
}

SwPaM::SwPaM( const SwNodeIndex& rMark , xub_StrLen nMarkCntnt,
              const SwNodeIndex& rPoint, xub_StrLen nPointCntnt, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( rPoint.GetNode().GetCntntNode(), nPointCntnt);
    m_pMark ->nContent.Assign( rMark .GetNode().GetCntntNode(), nMarkCntnt );
}

SwPaM::SwPaM( const SwNode& rMark , xub_StrLen nMarkCntnt,
              const SwNode& rPoint, xub_StrLen nPointCntnt, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( m_pPoint->nNode.GetNode().GetCntntNode(),
        nPointCntnt);
    m_pMark ->nContent.Assign( m_pMark ->nNode.GetNode().GetCntntNode(),
        nMarkCntnt );
}

SwPaM::SwPaM( const SwNode& rNode, xub_StrLen nCntnt, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rNode )
    , m_Bound2( m_Bound1.nNode.GetNode().GetNodes() ) // default initialize
    , m_pPoint( &m_Bound1 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( m_pPoint->nNode.GetNode().GetCntntNode(),
        nCntnt );
}

SwPaM::SwPaM( const SwNodeIndex& rNodeIdx, xub_StrLen nCntnt, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rNodeIdx )
    , m_Bound2( rNodeIdx.GetNode().GetNodes() ) // default initialize
    , m_pPoint( &m_Bound1 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( rNodeIdx.GetNode().GetCntntNode(), nCntnt );
}

SwPaM::~SwPaM() {}

// @@@ semantic: no copy ctor.
SwPaM::SwPaM( SwPaM &rPam )
    : Ring( &rPam )
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
    (*m_pMark) = (*m_pPoint);
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
sal_Bool SwPaM::Move( SwMoveFn fnMove, SwGoInDoc fnGo )
{
    sal_Bool bRet = (*fnGo)( *this, fnMove );

    m_bIsInFrontOfLabel = false;

    return bRet;
}

/** make a new region

    Sets the first SwPaM onto the given SwPaM, or to the beginning or end of a
    document. SPoint stays at its position, GetMark will be changed respectively.

    @param fnMove  Contains information if beginning or end of document.
    @param pOrigRg The given region.

    @return Newly created area.
*/
SwPaM* SwPaM::MakeRegion( SwMoveFn fnMove, const SwPaM * pOrigRg )
{
    SwPaM* pPam;
    if( pOrigRg == 0 )
    {
        pPam = new SwPaM( *m_pPoint );
        pPam->SetMark(); // set beginning
        pPam->Move( fnMove, fnGoSection); // to beginning or end of a node

        // set SPoint onto its old position; set GetMark to the "end"
        pPam->Exchange();
    }
    else
    {
        pPam = new SwPaM( *(SwPaM*)pOrigRg ); // given search area
        // make sure that SPoint is on the "real" start position
        // FORWARD: SPoint always smaller than GetMark
        // BACKWARD: SPoint always bigger than GetMark
        if( (pPam->GetMark()->*fnMove->fnCmpOp)( *pPam->GetPoint() ) )
            pPam->Exchange();
    }
    return pPam;
}

SwPaM & SwPaM::Normalize(sal_Bool bPointFirst)
{
    if (HasMark())
        if ( ( bPointFirst && *m_pPoint > *m_pMark) ||
             (!bPointFirst && *m_pPoint < *m_pMark) )
        {
            Exchange();
        }

    return *this;
}

/// return page number at cursor (for reader and page bound frames)
sal_uInt16 SwPaM::GetPageNum( sal_Bool bAtPoint, const Point* pLayPos )
{
    const SwCntntFrm* pCFrm;
    const SwPageFrm *pPg;
    const SwCntntNode *pNd ;
    const SwPosition* pPos = bAtPoint ? m_pPoint : m_pMark;

    if( 0 != ( pNd = pPos->nNode.GetNode().GetCntntNode() ) &&
        0 != ( pCFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout(), pLayPos, pPos, sal_False )) &&
        0 != ( pPg = pCFrm->FindPageFrm() ))
        return pPg->GetPhyPageNum();
    return 0;
}

// Formular view - See also SwCrsrShell::IsCrsrReadonly()
static const SwFrm* lcl_FindEditInReadonlyFrm( const SwFrm& rFrm )
{
    const SwFrm* pRet = 0;

    const SwFlyFrm* pFly;
    const SwSectionFrm* pSectionFrm;

    if( rFrm.IsInFly() &&
       (pFly = rFrm.FindFlyFrm())->GetFmt()->GetEditInReadonly().GetValue() &&
        pFly->Lower() &&
       !pFly->Lower()->IsNoTxtFrm() )
    {
       pRet = pFly;
    }
    else if ( rFrm.IsInSct() &&
              0 != ( pSectionFrm = rFrm.FindSctFrm() )->GetSection() &&
              pSectionFrm->GetSection()->IsEditInReadonlyFlag() )
    {
        pRet = pSectionFrm;
    }

    return pRet;
}

/// is in protected section or selection surrounds something protected
sal_Bool SwPaM::HasReadonlySel( bool bFormView ) const
{
    sal_Bool bRet = sal_False;
    Point aTmpPt;
    const SwCntntNode *pNd;
    const SwCntntFrm *pFrm;

    if( 0 != ( pNd = GetPoint()->nNode.GetNode().GetCntntNode() ))
        pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout(), &aTmpPt, GetPoint(), sal_False );
    else
        pFrm = 0;

    // Will be set if point/mark are inside edit-in-readonly environment
    const SwFrm* pSttEIRFrm = 0;
    const SwFrm* pEndEIRFrm = 0;

    if( pFrm && ( pFrm->IsProtected() ||
                  ( bFormView && 0 == ( pSttEIRFrm = lcl_FindEditInReadonlyFrm( *pFrm ) ) ) ) )
        bRet = sal_True;
    else if( pNd )
    {
        const SwSectionNode* pSNd = pNd->GetSectionNode();
        if( pSNd && ( pSNd->GetSection().IsProtectFlag() ||
                      (bFormView && !pSNd->GetSection().IsEditInReadonlyFlag()) ) )
            bRet = sal_True;
    }

    if( !bRet && HasMark() && GetPoint()->nNode != GetMark()->nNode )
    {
        if( 0 != ( pNd = GetMark()->nNode.GetNode().GetCntntNode() ))
            pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout(), &aTmpPt, GetMark(), sal_False );
        else
            pFrm = 0;

        if( pFrm && ( pFrm->IsProtected() ||
                  ( bFormView && 0 == ( pEndEIRFrm = lcl_FindEditInReadonlyFrm( *pFrm ) ) ) ) )
            bRet = sal_True;
        else if( pNd )
        {
            const SwSectionNode* pSNd = pNd->GetSectionNode();
            if( pSNd && ( pSNd->GetSection().IsProtectFlag() ||
                          (bFormView && !pSNd->GetSection().IsEditInReadonlyFlag()) ) )
                bRet = sal_True;
        }

        if ( !bRet && bFormView )
        {
           // Check if start and end frame are inside the _same_
           // edit-in-readonly-environment. Otherwise we better return 'true'
           if ( pSttEIRFrm != pEndEIRFrm )
                bRet = sal_True;
        }

        // protected section in selection
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
            // (TxtNd, SectNd, TxtNd, EndNd, TxtNd )
            if( nSttIdx + 3 < nEndIdx )
            {
                const SwSectionFmts& rFmts = GetDoc()->GetSections();
                for( sal_uInt16 n = rFmts.size(); n;  )
                {
                    const SwSectionFmt* pFmt = rFmts[ --n ];
                    if( pFmt->GetProtect().IsCntntProtected() )
                    {
                        const SwFmtCntnt& rCntnt = pFmt->GetCntnt(sal_False);
                        OSL_ENSURE( rCntnt.GetCntntIdx(), "wo ist der SectionNode?" );
                        sal_uLong nIdx = rCntnt.GetCntntIdx()->GetIndex();
                        if( nSttIdx <= nIdx && nEndIdx >= nIdx &&
                            rCntnt.GetCntntIdx()->GetNode().GetNodes().IsDocNodes() )
                        {
                            bRet = sal_True;
                            break;
                        }
                    }
                }
            }
        }
    }
    //FIXME FieldBk
    // TODO: Form Protection when Enhanced Fields are enabled
    const SwDoc *pDoc = GetDoc();
    sw::mark::IMark* pA = NULL;
    sw::mark::IMark* pB = NULL;
    bool bUnhandledMark = false;
    if ( pDoc )
    {
        const IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess( );
        pA = GetPoint() ? pMarksAccess->getFieldmarkFor( *GetPoint( ) ) : NULL;
        pB = GetMark( ) ? pMarksAccess->getFieldmarkFor( *GetMark( ) ) : pA;

        sw::mark::IFieldmark* pFieldmark = pMarksAccess->getFieldmarkFor( *GetPoint() );
        if ( pFieldmark )
            bUnhandledMark = pFieldmark->GetFieldname( ) == ODF_UNHANDLED;
    }

    if (!bRet)
    {
        // Unhandled fieldmarks case shouldn't be edited manually to avoid breaking anything
        if ( ( pA == pB ) && bUnhandledMark )
            bRet = sal_True;
        else
        {
            // Form protection case
            bool bAtStartA = pA != NULL && pA->GetMarkStart() == *GetPoint();
            bool bAtStartB = pB != NULL && pB->GetMarkStart() == *GetMark();
            bRet = ( pA != pB ) || bAtStartA || bAtStartB;
            bool bProtectForm = pDoc->get( IDocumentSettingAccess::PROTECT_FORM );
            if ( bProtectForm )
                bRet |= ( pA == NULL || pB == NULL );
        }
    }
    else
    {
        bRet = !( pA == pB && pA != NULL );
    }
    return bRet;
}

/// This function returns the next node in direction of search. If there is no
/// left or the next is out of the area, then a null-pointer is returned.
/// @param rbFirst If <true> than first time request. If so than the position of
///        the PaM must not be changed!
SwCntntNode* GetNode( SwPaM & rPam, sal_Bool& rbFirst, SwMoveFn fnMove,
                        sal_Bool bInReadOnly )
{
    SwCntntNode * pNd = 0;
    SwCntntFrm* pFrm;
    if( ((*rPam.GetPoint()).*fnMove->fnCmpOp)( *rPam.GetMark() ) ||
        ( *rPam.GetPoint() == *rPam.GetMark() && rbFirst ) )
    {
        if( rbFirst )
        {
            rbFirst = sal_False;
            pNd = rPam.GetCntntNode();
            if( pNd )
            {
                if(
                    (
                        0 == ( pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout() ) ) ||
                        ( !bInReadOnly && pFrm->IsProtected() ) ||
                        (pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsHiddenNow())
                    ) ||
                    ( !bInReadOnly && pNd->FindSectionNode() &&
                        pNd->FindSectionNode()->GetSection().IsProtect()
                    )
                  )
                    {
                        pNd = 0;
                    }
            }
        }

        if( !pNd ) // is the cursor not on a CntntNode?
        {
            SwPosition aPos( *rPam.GetPoint() );
            sal_Bool bSrchForward = fnMove == fnMoveForward;
            SwNodes& rNodes = aPos.nNode.GetNodes();

            // go to next/previous CntntNode
            while( sal_True )
            {
                pNd = bSrchForward
                        ? rNodes.GoNextSection( &aPos.nNode, sal_True, !bInReadOnly )
                        : rNodes.GoPrevSection( &aPos.nNode, sal_True, !bInReadOnly );
                if( pNd )
                {
                    aPos.nContent.Assign( pNd, ::GetSttOrEnd( bSrchForward,*pNd ));
                    // is the position still in the area
                    if( (aPos.*fnMove->fnCmpOp)( *rPam.GetMark() ) )
                    {
                        // only in AutoTextSection can be nodes that are hidden
                        if( 0 == ( pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout() ) ) ||
                            ( !bInReadOnly && pFrm->IsProtected() ) ||
                            ( pFrm->IsTxtFrm() &&
                                ((SwTxtFrm*)pFrm)->IsHiddenNow() ) )
                        {
                            pNd = 0;
                            continue;
                        }
                        *(SwPosition*)rPam.GetPoint() = aPos;
                    }
                    else
                        pNd = 0; // no valid node
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
    SwCntntNode* pCNd = rNodes.GoNext( &pPos->nNode );
    if( pCNd )
        pCNd->MakeStartIndex( &pPos->nContent );
}


void GoEndDoc( SwPosition * pPos )
{
    SwNodes& rNodes = pPos->nNode.GetNodes();
    pPos->nNode = rNodes.GetEndOfContent();
    SwCntntNode* pCNd = GoPreviousNds( &pPos->nNode, sal_True );
    if( pCNd )
        pCNd->MakeEndIndex( &pPos->nContent );
}


void GoStartSection( SwPosition * pPos )
{
    // jump to section's beginning
    SwNodes& rNodes = pPos->nNode.GetNodes();
    sal_uInt16 nLevel = rNodes.GetSectionLevel( pPos->nNode );
    if( pPos->nNode < rNodes.GetEndOfContent().StartOfSectionIndex() )
        nLevel--;
    do { rNodes.GoStartOfSection( &pPos->nNode ); } while( nLevel-- );

    // already on a CntntNode
    pPos->nNode.GetNode().GetCntntNode()->MakeStartIndex( &pPos->nContent );
}

/// go to the end of the current base section
void GoEndSection( SwPosition * pPos )
{
    // jump to section's beginning/end
    SwNodes& rNodes = pPos->nNode.GetNodes();
    sal_uInt16 nLevel = rNodes.GetSectionLevel( pPos->nNode );
    if( pPos->nNode < rNodes.GetEndOfContent().StartOfSectionIndex() )
        nLevel--;
    do { rNodes.GoEndOfSection( &pPos->nNode ); } while( nLevel-- );

    // now on a EndNode, thus to the previous CntntNode
    if( GoPreviousNds( &pPos->nNode, sal_True ) )
        pPos->nNode.GetNode().GetCntntNode()->MakeEndIndex( &pPos->nContent );
}



sal_Bool GoInDoc( SwPaM & rPam, SwMoveFn fnMove )
{
    (*fnMove->fnDoc)( rPam.GetPoint() );
    return sal_True;
}


sal_Bool GoInSection( SwPaM & rPam, SwMoveFn fnMove )
{
    (*fnMove->fnSections)( (SwPosition*)rPam.GetPoint() );
    return sal_True;
}


sal_Bool GoInNode( SwPaM & rPam, SwMoveFn fnMove )
{
    SwCntntNode *pNd = (*fnMove->fnNds)( &rPam.GetPoint()->nNode, sal_True );
    if( pNd )
        rPam.GetPoint()->nContent.Assign( pNd,
                        ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
    return 0 != pNd;
}


sal_Bool GoInCntnt( SwPaM & rPam, SwMoveFn fnMove )
{
    if( (*fnMove->fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                        &rPam.GetPoint()->nContent, CRSR_SKIP_CHARS ))
        return sal_True;
    return GoInNode( rPam, fnMove );
}

sal_Bool GoInCntntCells( SwPaM & rPam, SwMoveFn fnMove )
{
    if( (*fnMove->fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                         &rPam.GetPoint()->nContent, CRSR_SKIP_CELLS ))
        return sal_True;
    return GoInNode( rPam, fnMove );
}

sal_Bool GoInCntntSkipHidden( SwPaM & rPam, SwMoveFn fnMove )
{
    if( (*fnMove->fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                        &rPam.GetPoint()->nContent, CRSR_SKIP_CHARS | CRSR_SKIP_HIDDEN ) )
        return sal_True;
    return GoInNode( rPam, fnMove );
}

sal_Bool GoInCntntCellsSkipHidden( SwPaM & rPam, SwMoveFn fnMove )
{
    if( (*fnMove->fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                         &rPam.GetPoint()->nContent, CRSR_SKIP_CELLS | CRSR_SKIP_HIDDEN ) )
        return sal_True;
    return GoInNode( rPam, fnMove );
}





sal_Bool GoPrevPara( SwPaM & rPam, SwPosPara aPosPara )
{
    if( rPam.Move( fnMoveBackward, fnGoNode ) )
    {
        // always on a CntntNode
        SwPosition& rPos = *rPam.GetPoint();
        SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();
        rPos.nContent.Assign( pNd,
                            ::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ) );
        return sal_True;
    }
    return sal_False;
}


sal_Bool GoCurrPara( SwPaM & rPam, SwPosPara aPosPara )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();
    if( pNd )
    {
        xub_StrLen nOld = rPos.nContent.GetIndex(),
                   nNew = aPosPara == fnMoveForward ? 0 : pNd->Len();
        // if already at beginning/end then to the next/previous
        if( nOld != nNew )
        {
            rPos.nContent.Assign( pNd, nNew );
            return sal_True;
        }
    }
    // move node to next/previous CntntNode
    if( ( aPosPara==fnParaStart && 0 != ( pNd =
            GoPreviousNds( &rPos.nNode, sal_True ))) ||
        ( aPosPara==fnParaEnd && 0 != ( pNd =
            GoNextNds( &rPos.nNode, sal_True ))) )
    {
        rPos.nContent.Assign( pNd,
                        ::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ));
        return sal_True;
    }
    return sal_False;
}


sal_Bool GoNextPara( SwPaM & rPam, SwPosPara aPosPara )
{
    if( rPam.Move( fnMoveForward, fnGoNode ) )
    {
        // always on a CntntNode
        SwPosition& rPos = *rPam.GetPoint();
        SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();
        rPos.nContent.Assign( pNd,
                        ::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ) );
        return sal_True;
    }
    return sal_False;
}



sal_Bool GoCurrSection( SwPaM & rPam, SwMoveFn fnMove )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwPosition aSavePos( rPos ); // position for comparison
    SwNodes& rNds = aSavePos.nNode.GetNodes();
    (rNds.*fnMove->fnSection)( &rPos.nNode );
    SwCntntNode *pNd;
    if( 0 == ( pNd = rPos.nNode.GetNode().GetCntntNode()) &&
        0 == ( pNd = (*fnMove->fnNds)( &rPos.nNode, sal_True )) )
    {
        rPos = aSavePos; // do not change cursor
        return sal_False;
    }

    rPos.nContent.Assign( pNd,
                        ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
    return aSavePos != rPos;
}


sal_Bool GoNextSection( SwPaM & rPam, SwMoveFn fnMove )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwPosition aSavePos( rPos ); // position for comparison
    SwNodes& rNds = aSavePos.nNode.GetNodes();
    rNds.GoEndOfSection( &rPos.nNode );

    // no other CntntNode existent?
    if( !GoInCntnt( rPam, fnMoveForward ) )
    {
        rPos = aSavePos; // do not change cursor
        return sal_False;
    }
    (rNds.*fnMove->fnSection)( &rPos.nNode );
    SwCntntNode *pNd = rPos.nNode.GetNode().GetCntntNode();
    rPos.nContent.Assign( pNd,
                        ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
    return sal_True;
}


sal_Bool GoPrevSection( SwPaM & rPam, SwMoveFn fnMove )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwPosition aSavePos( rPos ); // position for comparison
    SwNodes& rNds = aSavePos.nNode.GetNodes();
    rNds.GoStartOfSection( &rPos.nNode );

    // no further CntntNode existent?
    if( !GoInCntnt( rPam, fnMoveBackward ))
    {
        rPos = aSavePos; // do not change cursor
        return sal_False;
    }
    (rNds.*fnMove->fnSection)( &rPos.nNode );
    SwCntntNode *pNd = rPos.nNode.GetNode().GetCntntNode();
    rPos.nContent.Assign( pNd,
                            ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ));
    return sal_True;
}

String SwPaM::GetTxt() const
{
    String aResult;

    SwNodeIndex aNodeIndex = Start()->nNode;

    // The first node can be already the end node.
    // A first end node must be handled, too. Therefore do-while and no
    // incrementing of aNodeIndex in the first pass.
    bool bFirst = true;
    do
    {
        if (! bFirst)
        {
            aNodeIndex++;
        }

        bFirst = false;

        SwTxtNode * pTxtNode = aNodeIndex.GetNode().GetTxtNode();

        if (pTxtNode != NULL)
        {
            const String & aTmpStr = pTxtNode->GetTxt();

            if (aNodeIndex == Start()->nNode)
            {
                xub_StrLen nEnd;
                if (End()->nNode == aNodeIndex)
                    nEnd = End()->nContent.GetIndex();
                else
                    nEnd = aTmpStr.Len();

                aResult += aTmpStr.Copy(Start()->nContent.GetIndex(),
                                        nEnd - Start()->nContent.GetIndex()) ;
            }
            else if (aNodeIndex == End()->nNode)
                aResult += aTmpStr.Copy(0, End()->nContent.GetIndex());
            else
                aResult += aTmpStr;
        }
    }
    while (aNodeIndex != End()->nNode);

    return aResult;
}

void SwPaM::InvalidatePaM()
{
    const SwNode *_pNd=this->GetNode();
    const SwTxtNode *_pTxtNd=(_pNd!=NULL?_pNd->GetTxtNode():NULL);
    if (_pTxtNd!=NULL)
    {
        // pretent that the PaM marks inserted text to recalc the portion...
        SwInsTxt aHint( Start()->nContent.GetIndex(),
                        End()->nContent.GetIndex() - Start()->nContent.GetIndex() + 1 );
        SwModify *_pModify=(SwModify*)_pTxtNd;
        _pModify->ModifyNotification( 0, &aHint);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
