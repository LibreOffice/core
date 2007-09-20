/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layouter.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 11:49:37 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include "layouter.hxx"
#include "doc.hxx"
#include "sectfrm.hxx"
#include "ftnboss.hxx"
#include "cntfrm.hxx"
#include "pagefrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"

// --> OD 2004-06-23 #i28701#
#ifndef _MOVEDFWDFRMSBYOBJPOS_HXX
#include <movedfwdfrmsbyobjpos.hxx>
#endif
// <--
// --> OD 2004-10-22 #i35911#
#ifndef _OBJSTMPCONSIDERWRAPINFL_HXX
#include <objstmpconsiderwrapinfl.hxx>
#endif
// <--

#define LOOP_DETECT 250

class SwLooping
{
    USHORT nMinPage;
    USHORT nMaxPage;
    USHORT nCount;
    USHORT mnLoopControlStage;
public:
    SwLooping( SwPageFrm* pPage );
    void Control( SwPageFrm* pPage );
    void Drastic( SwFrm* pFrm );
    bool IsLoopingLouieLight() const { return nCount > LOOP_DETECT - 30; };
};

class SwEndnoter
{
    SwLayouter* pMaster;
    SwSectionFrm* pSect;
    SvPtrarr* pEndArr;
public:
    SwEndnoter( SwLayouter* pLay )
        : pMaster( pLay ), pSect( NULL ), pEndArr( NULL ) {}
    ~SwEndnoter() { delete pEndArr; }
    void CollectEndnotes( SwSectionFrm* pSct );
    void CollectEndnote( SwFtnFrm* pFtn );
    const SwSectionFrm* GetSect() { return pSect; }
    void InsertEndnotes();
    BOOL HasEndnotes() const { return pEndArr && pEndArr->Count(); }
};

void SwEndnoter::CollectEndnotes( SwSectionFrm* pSct )
{
    ASSERT( pSct, "CollectEndnotes: Which section?" );
    if( !pSect )
        pSect = pSct;
    else if( pSct != pSect )
        return;
    pSect->CollectEndnotes( pMaster );
}

void SwEndnoter::CollectEndnote( SwFtnFrm* pFtn )
{
    if( pEndArr && USHRT_MAX != pEndArr->GetPos( (VoidPtr)pFtn ) )
        return;

    if( pFtn->GetUpper() )
    {
        // pFtn is the master, he incorporates its follows
        SwFtnFrm *pNxt = pFtn->GetFollow();
        while ( pNxt )
        {
            SwFrm *pCnt = pNxt->ContainsAny();
            if ( pCnt )
            {
                do
                {   SwFrm *pNxtCnt = pCnt->GetNext();
                    pCnt->Cut();
                    pCnt->Paste( pFtn );
                    pCnt = pNxtCnt;
                } while ( pCnt );
            }
            else
            {   ASSERT( pNxt->Lower() && pNxt->Lower()->IsSctFrm(),
                        "Endnote without content?" );
                pNxt->Cut();
                delete pNxt;
            }
            pNxt = pFtn->GetFollow();
        }
        if( pFtn->GetMaster() )
            return;
        pFtn->Cut();
    }
    else if( pEndArr )
    {
        for ( USHORT i = 0; i < pEndArr->Count(); ++i )
        {
            SwFtnFrm *pEndFtn = (SwFtnFrm*)((*pEndArr)[i]);
            if( pEndFtn->GetAttr() == pFtn->GetAttr() )
            {
                delete pFtn;
                return;
            }
        }
    }
    if( !pEndArr )
        pEndArr = new SvPtrarr( 5, 5 );  // deleted from the SwLayouter
    pEndArr->Insert( (VoidPtr)pFtn, pEndArr->Count() );
}

void SwEndnoter::InsertEndnotes()
{
    if( !pSect )
        return;
    if( !pEndArr || !pEndArr->Count() )
    {
        pSect = NULL;
        return;
    }
    ASSERT( pSect->Lower() && pSect->Lower()->IsFtnBossFrm(),
            "InsertEndnotes: Where's my column?" );
    SwFrm* pRef = pSect->FindLastCntnt( FINDMODE_MYLAST );
    SwFtnBossFrm *pBoss = pRef ? pRef->FindFtnBossFrm()
                               : (SwFtnBossFrm*)pSect->Lower();
    pBoss->_MoveFtns( *pEndArr );
    delete pEndArr;
    pEndArr = NULL;
    pSect = NULL;
}

SwLooping::SwLooping( SwPageFrm* pPage )
{
    ASSERT( pPage, "Where's my page?" );
    nMinPage = pPage->GetPhyPageNum();
    nMaxPage = nMinPage;
    nCount = 0;
    mnLoopControlStage = 0;
}

void SwLooping::Drastic( SwFrm* pFrm )
{
    while( pFrm )
    {
        pFrm->ValidateThisAndAllLowers( mnLoopControlStage );
        pFrm = pFrm->GetNext();
    }
}

void SwLooping::Control( SwPageFrm* pPage )
{
    if( !pPage )
        return;
    USHORT nNew = pPage->GetPhyPageNum();
    if( nNew > nMaxPage )
        nMaxPage = nNew;
    if( nNew < nMinPage )
    {
        nMinPage = nNew;
        nMaxPage = nNew;
        nCount = 0;
        mnLoopControlStage = 0;
    }
    else if( nNew > nMinPage + 2 )
    {
        nMinPage = nNew - 2;
        nMaxPage = nNew;
        nCount = 0;
        mnLoopControlStage = 0;
    }
    else if( ++nCount > LOOP_DETECT )
    {
#ifndef PRODUCT
#if OSL_DEBUG_LEVEL > 1
        static BOOL bNoLouie = FALSE;
        if( bNoLouie )
            return;
#endif
#endif

        // FME 2007-08-30 #i81146# new loop control
#if OSL_DEBUG_LEVEL > 1
        ASSERT( 0 != mnLoopControlStage, "Looping Louie: Stage 1!" );
        ASSERT( 1 != mnLoopControlStage, "Looping Louie: Stage 2!!" );
        ASSERT( 2 >  mnLoopControlStage, "Looping Louie: Stage 3!!!" );
#endif

        Drastic( pPage->Lower() );
        if( nNew > nMinPage && pPage->GetPrev() )
            Drastic( ((SwPageFrm*)pPage->GetPrev())->Lower() );
        if( nNew < nMaxPage && pPage->GetNext() )
            Drastic( ((SwPageFrm*)pPage->GetNext())->Lower() );

        ++mnLoopControlStage;
        nCount = 0;
    }
}

/*************************************************************************
|*
|*  SwLayouter::SwLayouter()
|*
|*  Ersterstellung      AMA 02. Nov. 99
|*  Letzte Aenderung    AMA 02. Nov. 99
|*
|*************************************************************************/

SwLayouter::SwLayouter()
        : pEndnoter( NULL ),
          pLooping( NULL ),
          // --> OD 2004-06-23 #i28701#
          mpMovedFwdFrms( 0L ),
          // <--
          // --> OD 2004-10-22 #i35911#
          mpObjsTmpConsiderWrapInfl( 0L )
          // <--
{
}

SwLayouter::~SwLayouter()
{
    delete pEndnoter;
    delete pLooping;
    // --> OD 2004-06-23 #i28701#
    delete mpMovedFwdFrms;
    mpMovedFwdFrms = 0L;
    // <--
    // --> OD 2004-10-22 #i35911#
    delete mpObjsTmpConsiderWrapInfl;
    mpObjsTmpConsiderWrapInfl = 0L;
    // <--
}

void SwLayouter::_CollectEndnotes( SwSectionFrm* pSect )
{
    if( !pEndnoter )
        pEndnoter = new SwEndnoter( this );
    pEndnoter->CollectEndnotes( pSect );
}

BOOL SwLayouter::HasEndnotes() const
{
    return pEndnoter->HasEndnotes();
}

void SwLayouter::CollectEndnote( SwFtnFrm* pFtn )
{
    pEndnoter->CollectEndnote( pFtn );
}

void SwLayouter::InsertEndnotes( SwSectionFrm* pSect )
{
    if( !pEndnoter || pEndnoter->GetSect() != pSect )
        return;
    pEndnoter->InsertEndnotes();
}

void SwLayouter::LoopControl( SwPageFrm* pPage, BYTE nLoop )
{
    ASSERT( pLooping, "Looping: Lost control" );
    pLooping->Control( pPage );
}

void SwLayouter::LoopingLouieLight( const SwDoc& rDoc, const SwTxtFrm& rFrm )
{
    if ( pLooping && pLooping->IsLoopingLouieLight() )
    {
#if OSL_DEBUG_LEVEL > 1
        ASSERT( false, "Looping Louie (Light): Fixating fractious frame" )
#endif
        SwLayouter::InsertMovedFwdFrm( rDoc, rFrm, rFrm.FindPageFrm()->GetPhyPageNum() );
    }
}

BOOL SwLayouter::StartLooping( SwPageFrm* pPage )
{
    if( pLooping )
        return FALSE;
    pLooping = new SwLooping( pPage );
    return TRUE;
}

void SwLayouter::EndLoopControl()
{
    delete pLooping;
    pLooping = NULL;
}

void SwLayouter::CollectEndnotes( SwDoc* pDoc, SwSectionFrm* pSect )
{
    ASSERT( pDoc, "No doc, no fun" );
    if( !pDoc->GetLayouter() )
        pDoc->SetLayouter( new SwLayouter() );
    pDoc->GetLayouter()->_CollectEndnotes( pSect );
}

BOOL SwLayouter::Collecting( SwDoc* pDoc, SwSectionFrm* pSect, SwFtnFrm* pFtn )
{
    if( !pDoc->GetLayouter() )
        return FALSE;
    SwLayouter *pLayouter = pDoc->GetLayouter();
    if( pLayouter->pEndnoter && pLayouter->pEndnoter->GetSect() && pSect &&
        ( pLayouter->pEndnoter->GetSect()->IsAnFollow( pSect ) ||
          pSect->IsAnFollow( pLayouter->pEndnoter->GetSect() ) ) )
    {
        if( pFtn )
            pLayouter->CollectEndnote( pFtn );
        return TRUE;
    }
    return FALSE;
}

BOOL SwLayouter::StartLoopControl( SwDoc* pDoc, SwPageFrm *pPage )
{
    ASSERT( pDoc, "No doc, no fun" );
    if( !pDoc->GetLayouter() )
        pDoc->SetLayouter( new SwLayouter() );
    return !pDoc->GetLayouter()->pLooping &&
            pDoc->GetLayouter()->StartLooping( pPage );
}

// --> OD 2004-06-23 #i28701#
// -----------------------------------------------------------------------------
// methods to manage text frames, which are moved forward by the positioning
// of its anchored objects
// -----------------------------------------------------------------------------
void SwLayouter::ClearMovedFwdFrms( const SwDoc& _rDoc )
{
    if ( _rDoc.GetLayouter() &&
         _rDoc.GetLayouter()->mpMovedFwdFrms )
    {
        _rDoc.GetLayouter()->mpMovedFwdFrms->Clear();
    }
}

void SwLayouter::InsertMovedFwdFrm( const SwDoc& _rDoc,
                                    const SwTxtFrm& _rMovedFwdFrmByObjPos,
                                    const sal_uInt32 _nToPageNum )
{
    if ( !_rDoc.GetLayouter() )
    {
        const_cast<SwDoc&>(_rDoc).SetLayouter( new SwLayouter() );
    }

    if ( !_rDoc.GetLayouter()->mpMovedFwdFrms )
    {
        const_cast<SwDoc&>(_rDoc).GetLayouter()->mpMovedFwdFrms =
                                                new SwMovedFwdFrmsByObjPos();
    }

    _rDoc.GetLayouter()->mpMovedFwdFrms->Insert( _rMovedFwdFrmByObjPos,
                                                 _nToPageNum );
}

// --> OD 2005-01-12 #i40155#
void SwLayouter::RemoveMovedFwdFrm( const SwDoc& _rDoc,
                                    const SwTxtFrm& _rTxtFrm )
{
    sal_uInt32 nDummy;
    if ( SwLayouter::FrmMovedFwdByObjPos( _rDoc, _rTxtFrm, nDummy ) )
    {
        _rDoc.GetLayouter()->mpMovedFwdFrms->Remove( _rTxtFrm );
    }
}
// <--

bool SwLayouter::FrmMovedFwdByObjPos( const SwDoc& _rDoc,
                                      const SwTxtFrm& _rTxtFrm,
                                      sal_uInt32& _ornToPageNum )
{
    if ( !_rDoc.GetLayouter() )
    {
        _ornToPageNum = 0;
        return false;
    }
    else if ( !_rDoc.GetLayouter()->mpMovedFwdFrms )
    {
        _ornToPageNum = 0;
        return false;
    }
    else
    {
        return _rDoc.GetLayouter()->mpMovedFwdFrms->
                                FrmMovedFwdByObjPos( _rTxtFrm, _ornToPageNum );
    }
}
// <--
// --> OD 2004-10-05 #i26945#
bool SwLayouter::DoesRowContainMovedFwdFrm( const SwDoc& _rDoc,
                                            const SwRowFrm& _rRowFrm )
{
    if ( !_rDoc.GetLayouter() )
    {
        return false;
    }
    else if ( !_rDoc.GetLayouter()->mpMovedFwdFrms )
    {
        return false;
    }
    else
    {
        return _rDoc.GetLayouter()->
                        mpMovedFwdFrms->DoesRowContainMovedFwdFrm( _rRowFrm );
    }
}
// <--

// --> OD 2004-10-22 #i35911#
void SwLayouter::ClearObjsTmpConsiderWrapInfluence( const SwDoc& _rDoc )
{
    if ( _rDoc.GetLayouter() &&
         _rDoc.GetLayouter()->mpObjsTmpConsiderWrapInfl )
    {
        _rDoc.GetLayouter()->mpObjsTmpConsiderWrapInfl->Clear();
    }
}
void SwLayouter::InsertObjForTmpConsiderWrapInfluence(
                                            const SwDoc& _rDoc,
                                            SwAnchoredObject& _rAnchoredObj )
{
    if ( !_rDoc.GetLayouter() )
    {
        const_cast<SwDoc&>(_rDoc).SetLayouter( new SwLayouter() );
    }

    if ( !_rDoc.GetLayouter()->mpObjsTmpConsiderWrapInfl )
    {
        const_cast<SwDoc&>(_rDoc).GetLayouter()->mpObjsTmpConsiderWrapInfl =
                                new SwObjsMarkedAsTmpConsiderWrapInfluence();
    }

    _rDoc.GetLayouter()->mpObjsTmpConsiderWrapInfl->Insert( _rAnchoredObj );
}
// <--
// --> OD 2005-01-12 #i40155#
void SwLayouter::ClearFrmsNotToWrap( const SwDoc& _rDoc )
{
    if ( _rDoc.GetLayouter() )
    {
        const_cast<SwDoc&>(_rDoc).GetLayouter()->maFrmsNotToWrap.clear();
    }
}

void SwLayouter::InsertFrmNotToWrap( const SwDoc& _rDoc,
                                             const SwFrm& _rFrm )
{
    if ( !_rDoc.GetLayouter() )
    {
        const_cast<SwDoc&>(_rDoc).SetLayouter( new SwLayouter() );
    }

    if ( !SwLayouter::FrmNotToWrap( _rDoc, _rFrm ) )
    {
        const_cast<SwDoc&>(_rDoc).GetLayouter()->maFrmsNotToWrap.push_back( &_rFrm );
    }
}

bool SwLayouter::FrmNotToWrap( const IDocumentLayoutAccess& _rDLA,
                               const SwFrm& _rFrm )
{
    const SwLayouter* pLayouter = _rDLA.GetLayouter();
    if ( !pLayouter )
    {
        return false;
    }
    else
    {
        bool bFrmNotToWrap( false );
        std::vector< const SwFrm* >::const_iterator aIter =
                            pLayouter->maFrmsNotToWrap.begin();
        for ( ; aIter != pLayouter->maFrmsNotToWrap.end(); ++aIter )
        {
            const SwFrm* pFrm = *(aIter);
            if ( pFrm == &_rFrm )
            {
                bFrmNotToWrap = true;
                break;
            }
        }
        return bFrmNotToWrap;
    }
}
// <--

void LOOPING_LOUIE_LIGHT( bool bCondition, const SwTxtFrm& rTxtFrm )
{
    if ( bCondition )
    {
        const SwDoc& rDoc = *rTxtFrm.GetAttrSet()->GetDoc();
        if ( rDoc.GetLayouter() )
        {
            const_cast<SwDoc&>(rDoc).GetLayouter()->LoopingLouieLight( rDoc, rTxtFrm );
        }
    }
}

// --> OD 2006-05-10 #i65250#
bool SwLayouter::MoveBwdSuppressed( const SwDoc& p_rDoc,
                                    const SwFlowFrm& p_rFlowFrm,
                                    const SwLayoutFrm& p_rNewUpperFrm )
{
    bool bMoveBwdSuppressed( false );

    if ( !p_rDoc.GetLayouter() )
    {
        const_cast<SwDoc&>(p_rDoc).SetLayouter( new SwLayouter() );
    }

    // create hash map key
    tMoveBwdLayoutInfoKey aMoveBwdLayoutInfo;
    aMoveBwdLayoutInfo.mnFrmId = p_rFlowFrm.GetFrm()->GetFrmId();
    aMoveBwdLayoutInfo.mnNewUpperPosX = p_rNewUpperFrm.Frm().Pos().X();
    aMoveBwdLayoutInfo.mnNewUpperPosY = p_rNewUpperFrm.Frm().Pos().Y();
    aMoveBwdLayoutInfo.mnNewUpperWidth = p_rNewUpperFrm.Frm().Width();
    aMoveBwdLayoutInfo.mnNewUpperHeight =  p_rNewUpperFrm.Frm().Height();
    SWRECTFN( (&p_rNewUpperFrm) )
    const SwFrm* pLastLower( p_rNewUpperFrm.Lower() );
    while ( pLastLower && pLastLower->GetNext() )
    {
        pLastLower = pLastLower->GetNext();
    }
    aMoveBwdLayoutInfo.mnFreeSpaceInNewUpper =
            pLastLower
            ? (pLastLower->Frm().*fnRect->fnBottomDist)( (p_rNewUpperFrm.*fnRect->fnGetPrtBottom)() )
            : (p_rNewUpperFrm.Frm().*fnRect->fnGetHeight)();

    // check for moving backward suppress threshold
    const sal_uInt16 cMoveBwdCountSuppressThreshold = 20;
    if ( ++const_cast<SwDoc&>(p_rDoc).GetLayouter()->maMoveBwdLayoutInfo[ aMoveBwdLayoutInfo ] >
                                                cMoveBwdCountSuppressThreshold )
    {
        bMoveBwdSuppressed = true;
    }

    return bMoveBwdSuppressed;
}

void SwLayouter::ClearMoveBwdLayoutInfo( const SwDoc& _rDoc )
{
    if ( _rDoc.GetLayouter() )
    {
        // Checking content of <maMoveBwdLayoutInfo
        {
            sal_uInt64 nMaxSize( const_cast<SwDoc&>(_rDoc).GetLayouter()->maMoveBwdLayoutInfo.max_size() );
            sal_uInt64 nSize( const_cast<SwDoc&>(_rDoc).GetLayouter()->maMoveBwdLayoutInfo.size() );
            sal_uInt64 nBucketCount( const_cast<SwDoc&>(_rDoc).GetLayouter()->maMoveBwdLayoutInfo.bucket_count() );

            std::hash_map< const tMoveBwdLayoutInfoKey, sal_uInt16, fMoveBwdLayoutInfoKeyHash, fMoveBwdLayoutInfoKeyEq >::const_iterator
                aIter = const_cast<SwDoc&>(_rDoc).GetLayouter()->maMoveBwdLayoutInfo.begin();
            std::hash_map< const tMoveBwdLayoutInfoKey, sal_uInt16, fMoveBwdLayoutInfoKeyHash, fMoveBwdLayoutInfoKeyEq >::const_iterator
                aIterEnd = const_cast<SwDoc&>(_rDoc).GetLayouter()->maMoveBwdLayoutInfo.end();
            sal_uInt64 nCounter( 0 );
            for ( ; aIter != aIterEnd; ++aIter )
            {
                tMoveBwdLayoutInfoKey aMoveBwdLayoutInfo = (*aIter).first;
                sal_uInt16 nCount = (*aIter).second;
                ++nCounter;
            }
        }
        const_cast<SwDoc&>(_rDoc).GetLayouter()->maMoveBwdLayoutInfo.clear();
    }
}
// <--
