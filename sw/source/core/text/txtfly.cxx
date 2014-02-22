/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "pam.hxx"
#include "swregion.hxx"
#include "dflyobj.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "porfly.hxx"
#include "porfld.hxx"
#include "txtfly.hxx"
#include "txtpaint.hxx"
#include "txtatr.hxx"
#include "notxtfrm.hxx"
#include "fmtcnct.hxx"
#include "inftxt.hxx"
#include <pormulti.hxx>
#include <svx/obj3d.hxx>
#include <editeng/txtrange.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>

#include <editeng/lspcitem.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <sortedobjs.hxx>
#include <layouter.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <svx/svdoedge.hxx>

#ifdef DBG_UTIL
#include "viewsh.hxx"
#include "viewopt.hxx"
#include "doc.hxx"
#endif


using namespace ::com::sun::star;

namespace
{
    
    struct AnchoredObjOrder
    {
        bool mbR2L;
        SwRectFn mfnRect;

        AnchoredObjOrder( const bool bR2L,
                           SwRectFn fnRect )
            : mbR2L( bR2L ),
              mfnRect( fnRect )
        {}

        bool operator()( const SwAnchoredObject* pListedAnchoredObj,
                         const SwAnchoredObject* pNewAnchoredObj )
        {
            const SwRect aBoundRectOfListedObj( pListedAnchoredObj->GetObjRectWithSpaces() );
            const SwRect aBoundRectOfNewObj( pNewAnchoredObj->GetObjRectWithSpaces() );
            if ( ( mbR2L &&
                   ( (aBoundRectOfListedObj.*mfnRect->fnGetRight)() ==
                     (aBoundRectOfNewObj.*mfnRect->fnGetRight)() ) ) ||
                 ( !mbR2L &&
                   ( (aBoundRectOfListedObj.*mfnRect->fnGetLeft)() ==
                     (aBoundRectOfNewObj.*mfnRect->fnGetLeft)() ) ) )
            {
                SwTwips nTopDiff =
                    (*mfnRect->fnYDiff)( (aBoundRectOfNewObj.*mfnRect->fnGetTop)(),
                                        (aBoundRectOfListedObj.*mfnRect->fnGetTop)() );
                if ( nTopDiff == 0 &&
                     ( ( mbR2L &&
                         ( (aBoundRectOfNewObj.*mfnRect->fnGetLeft)() >
                           (aBoundRectOfListedObj.*mfnRect->fnGetLeft)() ) ) ||
                       ( !mbR2L &&
                         ( (aBoundRectOfNewObj.*mfnRect->fnGetRight)() <
                           (aBoundRectOfListedObj.*mfnRect->fnGetRight)() ) ) ) )
                {
                    return true;
                }
                else if ( nTopDiff > 0 )
                {
                    return true;
                }
            }
            else if ( ( mbR2L &&
                        ( (aBoundRectOfListedObj.*mfnRect->fnGetRight)() >
                          (aBoundRectOfNewObj.*mfnRect->fnGetRight)() ) ) ||
                      ( !mbR2L &&
                        ( (aBoundRectOfListedObj.*mfnRect->fnGetLeft)() <
                          (aBoundRectOfNewObj.*mfnRect->fnGetLeft)() ) ) )
            {
                return true;
            }

            return false;
        }
    };
}

SwContourCache::SwContourCache() :
    nPntCnt( 0 ), nObjCnt( 0 )
{
    memset( (SdrObject**)pSdrObj, 0, sizeof(pSdrObj) );
    memset( pTextRanger, 0, sizeof(pTextRanger) );
}

SwContourCache::~SwContourCache()
{
    for( MSHORT i = 0; i < nObjCnt; delete pTextRanger[ i++ ] )
        ;
}

void SwContourCache::ClrObject( MSHORT nPos )
{
    OSL_ENSURE( pTextRanger[ nPos ], "ClrObject: Already cleared. Good Bye!" );
    nPntCnt -= pTextRanger[ nPos ]->GetPointCount();
    delete pTextRanger[ nPos ];
    --nObjCnt;
    memmove( (SdrObject**)pSdrObj + nPos, pSdrObj + nPos + 1,
             ( nObjCnt - nPos ) * sizeof( SdrObject* ) );
    memmove( pTextRanger + nPos, pTextRanger + nPos + 1,
             ( nObjCnt - nPos ) * sizeof( TextRanger* ) );
}

void ClrContourCache( const SdrObject *pObj )
{
    if( pContourCache && pObj )
        for( MSHORT i = 0; i < pContourCache->GetCount(); ++i )
            if( pObj == pContourCache->GetObject( i ) )
            {
                pContourCache->ClrObject( i );
                break;
            }
}

void ClrContourCache()
{
    if( pContourCache )
    {
        for( MSHORT i = 0; i < pContourCache->GetCount();
             delete pContourCache->pTextRanger[ i++ ] )
             ;
        pContourCache->nObjCnt = 0;
        pContourCache->nPntCnt = 0;
    }
}


const SwRect SwContourCache::CalcBoundRect( const SwAnchoredObject* pAnchoredObj,
                                            const SwRect &rLine,
                                            const SwTxtFrm* pFrm,
                                            const long nXPos,
                                            const bool bRight )
{
    SwRect aRet;
    const SwFrmFmt* pFmt = &(pAnchoredObj->GetFrmFmt());
    if( pFmt->GetSurround().IsContour() &&
        ( !pAnchoredObj->ISA(SwFlyFrm) ||
          ( static_cast<const SwFlyFrm*>(pAnchoredObj)->Lower() &&
            static_cast<const SwFlyFrm*>(pAnchoredObj)->Lower()->IsNoTxtFrm() ) ) )
    {
        aRet = pAnchoredObj->GetObjRectWithSpaces();
        if( aRet.IsOver( rLine ) )
        {
            if( !pContourCache )
                pContourCache = new SwContourCache;

            aRet = pContourCache->ContourRect(
                    pFmt, pAnchoredObj->GetDrawObj(), pFrm, rLine, nXPos, bRight );
        }
        else
            aRet.Width( 0 );
    }
    else
    {
        aRet = pAnchoredObj->GetObjRectWithSpaces();
    }

    return aRet;
}

const SwRect SwContourCache::ContourRect( const SwFmt* pFmt,
    const SdrObject* pObj, const SwTxtFrm* pFrm, const SwRect &rLine,
    const long nXPos, const bool bRight )
{
    SwRect aRet;
    MSHORT nPos = 0; 
    while( nPos < GetCount() && pObj != pSdrObj[ nPos ] )
        ++nPos;
    if( GetCount() == nPos ) 
    {
        if( nObjCnt == POLY_CNT )
        {
            nPntCnt -= pTextRanger[ --nObjCnt ]->GetPointCount();
            delete pTextRanger[ nObjCnt ];
        }
        ::basegfx::B2DPolyPolygon aPolyPolygon;
        ::basegfx::B2DPolyPolygon* pPolyPolygon = 0L;

        if ( pObj->ISA(SwVirtFlyDrawObj) )
        {
            
            
            PolyPolygon aPoly;
            if( !((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetContour( aPoly ) )
                aPoly = PolyPolygon( ((SwVirtFlyDrawObj*)pObj)->
                                     GetFlyFrm()->Frm().SVRect() );
            aPolyPolygon.clear();
            aPolyPolygon.append(aPoly.getB2DPolyPolygon());
        }
        else
        {
            if( !pObj->ISA( E3dObject ) )
            {
                aPolyPolygon = pObj->TakeXorPoly();
            }

            ::basegfx::B2DPolyPolygon aContourPoly(pObj->TakeContour());
            pPolyPolygon = new ::basegfx::B2DPolyPolygon(aContourPoly);
        }
        const SvxLRSpaceItem &rLRSpace = pFmt->GetLRSpace();
        const SvxULSpaceItem &rULSpace = pFmt->GetULSpace();
        memmove( pTextRanger + 1, pTextRanger, nObjCnt * sizeof( TextRanger* ) );
        memmove( (SdrObject**)pSdrObj + 1, pSdrObj, nObjCnt++ * sizeof( SdrObject* ) );
        pSdrObj[ 0 ] = pObj; 
                             
        pTextRanger[ 0 ] = new TextRanger( aPolyPolygon, pPolyPolygon, 20,
            (sal_uInt16)rLRSpace.GetLeft(), (sal_uInt16)rLRSpace.GetRight(),
            pFmt->GetSurround().IsOutside(), false, pFrm->IsVertical() );
        pTextRanger[ 0 ]->SetUpper( rULSpace.GetUpper() );
        pTextRanger[ 0 ]->SetLower( rULSpace.GetLower() );

        delete pPolyPolygon;

        nPntCnt += pTextRanger[ 0 ]->GetPointCount();
        while( nPntCnt > POLY_MAX && nObjCnt > POLY_MIN )
        {
            nPntCnt -= pTextRanger[ --nObjCnt ]->GetPointCount();
            delete pTextRanger[ nObjCnt ];
        }
    }
    else if( nPos )
    {
        const SdrObject* pTmpObj = pSdrObj[ nPos ];
        TextRanger* pTmpRanger = pTextRanger[ nPos ];
        memmove( (SdrObject**)pSdrObj + 1, pSdrObj, nPos * sizeof( SdrObject* ) );
        memmove( pTextRanger + 1, pTextRanger, nPos * sizeof( TextRanger* ) );
        pSdrObj[ 0 ] = pTmpObj;
        pTextRanger[ 0 ] = pTmpRanger;
    }
    SWRECTFN( pFrm )
    long nTmpTop = (rLine.*fnRect->fnGetTop)();
    
    long nTmpBottom = (rLine.*fnRect->fnGetBottom)();

    Range aRange( std::min( nTmpTop, nTmpBottom ), std::max( nTmpTop, nTmpBottom ) );

    LongDqPtr pTmp = pTextRanger[ 0 ]->GetTextRanges( aRange );

    MSHORT nCount;
    if( 0 != ( nCount = pTmp->size() ) )
    {
        MSHORT nIdx = 0;
        while( nIdx < nCount && (*pTmp)[ nIdx ] < nXPos )
            ++nIdx;
        bool bOdd = (nIdx % 2);
        bool bSet = true;
        if( bOdd )
            --nIdx; 
        else if( ! bRight && ( nIdx >= nCount || (*pTmp)[ nIdx ] != nXPos ) )
        {
            if( nIdx )
                nIdx -= 2; 
            else
                bSet = false; 
        }

        if( bSet && nIdx < nCount )
        {
            (aRet.*fnRect->fnSetTopAndHeight)( (rLine.*fnRect->fnGetTop)(),
                                               (rLine.*fnRect->fnGetHeight)() );
            (aRet.*fnRect->fnSetLeft)( (*pTmp)[ nIdx ] );
            (aRet.*fnRect->fnSetRight)( (*pTmp)[ nIdx + 1 ] + 1 );
        }
    }
    return aRet;
}


SwTxtFly::SwTxtFly() :
    pPage(0),
    mpCurrAnchoredObj(0),
    pCurrFrm(0),
    pMaster(0),
    mpAnchoredObjList(0),
    nMinBottom(0),
    nNextTop(0),
    nIndex(0)
{
}

SwTxtFly::SwTxtFly( const SwTxtFrm *pFrm )
{
    CtorInitTxtFly( pFrm );
}

SwTxtFly::SwTxtFly( const SwTxtFly& rTxtFly )
{
    pPage = rTxtFly.pPage;
    mpCurrAnchoredObj = rTxtFly.mpCurrAnchoredObj;
    pCurrFrm = rTxtFly.pCurrFrm;
    pMaster = rTxtFly.pMaster;
    if( rTxtFly.mpAnchoredObjList )
    {
        mpAnchoredObjList = new SwAnchoredObjList( *(rTxtFly.mpAnchoredObjList) );
    }
    else
    {
        mpAnchoredObjList = NULL;
    }

    bOn = rTxtFly.bOn;
    bTopRule = rTxtFly.bTopRule;
    nMinBottom = rTxtFly.nMinBottom;
    nNextTop = rTxtFly.nNextTop;
    nIndex = rTxtFly.nIndex;
    mbIgnoreCurrentFrame = rTxtFly.mbIgnoreCurrentFrame;
    mbIgnoreContour = rTxtFly.mbIgnoreContour;
    mbIgnoreObjsInHeaderFooter = rTxtFly.mbIgnoreObjsInHeaderFooter;
}

SwTxtFly::~SwTxtFly()
{
    delete mpAnchoredObjList;
}

void SwTxtFly::CtorInitTxtFly( const SwTxtFrm *pFrm )
{
    mbIgnoreCurrentFrame = false;
    mbIgnoreContour = false;
    
    mbIgnoreObjsInHeaderFooter = false;
    pPage = pFrm->FindPageFrm();
    const SwFlyFrm* pTmp = pFrm->FindFlyFrm();
    
    mpCurrAnchoredObj = pTmp;
    pCurrFrm = pFrm;
    pMaster = pCurrFrm->IsFollow() ? NULL : pCurrFrm;
    
    mpAnchoredObjList = NULL;
    
    
    
    
    
    bOn = pPage->GetSortedObjs() != 0;
    bTopRule = true;
    nMinBottom = 0;
    nIndex = ULONG_MAX;
}


SwRect SwTxtFly::_GetFrm( const SwRect &rRect, bool bTop ) const
{
    SwRect aRet;
    if( ForEach( rRect, &aRet, true ) )
    {
        SWRECTFN( pCurrFrm )
        if( bTop )
            (aRet.*fnRect->fnSetTop)( (rRect.*fnRect->fnGetTop)() );

        
        const SwTwips nRetBottom = (aRet.*fnRect->fnGetBottom)();
        const SwTwips nRectBottom = (rRect.*fnRect->fnGetBottom)();
        if ( (*fnRect->fnYDiff)( nRetBottom, nRectBottom ) > 0 ||
             (aRet.*fnRect->fnGetHeight)() < 0 )
            (aRet.*fnRect->fnSetBottom)( nRectBottom );
    }
    return aRet;
}


bool SwTxtFly::IsAnyFrm() const
{
    SWAP_IF_SWAPPED( pCurrFrm )

    OSL_ENSURE( bOn, "IsAnyFrm: Why?" );
    SwRect aRect( pCurrFrm->Frm().Pos() + pCurrFrm->Prt().Pos(),
        pCurrFrm->Prt().SSize() );

    const bool bRet = ForEach( aRect, NULL, false );
    UNDO_SWAP( pCurrFrm )
    return bRet;
}

bool SwTxtFly::IsAnyObj( const SwRect &rRect ) const
{
   OSL_ENSURE( bOn, "SwTxtFly::IsAnyObj: Who's knocking?" );

    SwRect aRect( rRect );
    if ( aRect.IsEmpty() )
        aRect = SwRect( pCurrFrm->Frm().Pos() + pCurrFrm->Prt().Pos(),
                        pCurrFrm->Prt().SSize() );

    const SwSortedObjs *pSorted = pPage->GetSortedObjs();
    if( pSorted ) 
                  
    {
        for ( MSHORT i = 0; i < pSorted->Count(); ++i )
        {
            const SwAnchoredObject* pObj = (*pSorted)[i];

            const SwRect aBound( pObj->GetObjRectWithSpaces() );

            
            if( pObj->GetObjRect().Left() > aRect.Right() )
                continue;

            
            if( mpCurrAnchoredObj != pObj && aBound.IsOver( aRect ) )
                return true;
        }
    }
    return false;
}

const SwCntntFrm* SwTxtFly::_GetMaster()
{
    pMaster = pCurrFrm;
    while( pMaster && pMaster->IsFollow() )
        pMaster = (SwCntntFrm*)pMaster->FindMaster();
    return pMaster;
}


bool SwTxtFly::DrawTextOpaque( SwDrawTextInfo &rInf )
{
    SwSaveClip aClipSave( rInf.GetpOut() );
    SwRect aRect( rInf.GetPos(), rInf.GetSize() );
    if( rInf.GetSpace() )
    {
        sal_Int32 nTmpLen = COMPLETE_STRING == rInf.GetLen() ? rInf.GetText().getLength() :
                                                      rInf.GetLen();
        if( rInf.GetSpace() > 0 )
        {
            sal_Int32 nSpaceCnt = 0;
            const sal_Int32 nEndPos = rInf.GetIdx() + nTmpLen;
            for( sal_Int32 nPos = rInf.GetIdx(); nPos < nEndPos; ++nPos )
            {
                if( CH_BLANK == rInf.GetText()[ nPos ] )
                    ++nSpaceCnt;
            }
            if( nSpaceCnt )
                aRect.Width( aRect.Width() + nSpaceCnt * rInf.GetSpace() );
        }
        else
            aRect.Width( aRect.Width() - nTmpLen * rInf.GetSpace() );
    }

    if( aClipSave.IsOn() && rInf.GetOut().IsClipRegion() )
    {
        SwRect aClipRect( rInf.GetOut().GetClipRegion().GetBoundRect() );
        aRect.Intersection( aClipRect );
    }

    SwRegionRects aRegion( aRect );

    bool bOpaque = false;
    
    const sal_uInt32 nCurrOrd = mpCurrAnchoredObj
                            ? mpCurrAnchoredObj->GetDrawObj()->GetOrdNum()
                            : SAL_MAX_UINT32;
    OSL_ENSURE( !bTopRule, "DrawTextOpaque: Wrong TopRule" );

    
    SwAnchoredObjList::size_type nCount( bOn ? GetAnchoredObjList()->size() : 0 );
    if ( bOn && nCount > 0 )
    {
        MSHORT nHellId = pPage->getRootFrm()->GetCurrShell()->getIDocumentDrawModelAccess()->GetHellId();
        for( MSHORT i = 0; i < nCount; ++i )
        {
            
            const SwAnchoredObject* pTmpAnchoredObj = (*mpAnchoredObjList)[i];
            if( dynamic_cast<const SwFlyFrm*>(pTmpAnchoredObj) &&
                mpCurrAnchoredObj != pTmpAnchoredObj )
            {
                
                const SwFlyFrm* pFly = dynamic_cast<const SwFlyFrm*>(pTmpAnchoredObj);
                if( aRegion.GetOrigin().IsOver( pFly->Frm() ) )
                {
                    const SwFrmFmt *pFmt = pFly->GetFmt();
                    const SwFmtSurround &rSur = pFmt->GetSurround();
                    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                    
                    if( !( pFly->IsBackgroundTransparent()
                           || pFly->IsShadowTransparent() ) &&
                        SURROUND_THROUGHT == rSur.GetSurround() &&
                        ( !rSur.IsAnchorOnly() ||
                          
                          GetMaster() == pFly->GetAnchorFrm() ||
                          ((FLY_AT_PARA != rAnchor.GetAnchorId()) &&
                           (FLY_AT_CHAR != rAnchor.GetAnchorId())
                          )
                        ) &&
                        
                        pTmpAnchoredObj->GetDrawObj()->GetLayer() != nHellId &&
                        nCurrOrd < pTmpAnchoredObj->GetDrawObj()->GetOrdNum()
                      )
                    {
                        
                        const SwNoTxtFrm *pNoTxt =
                                pFly->Lower() && pFly->Lower()->IsNoTxtFrm()
                                                   ? (SwNoTxtFrm*)pFly->Lower()
                                                   : 0;
                        if ( !pNoTxt ||
                             (!pNoTxt->IsTransparent() && !rSur.IsContour()) )
                        {
                            bOpaque = true;
                            aRegion -= pFly->Frm();
                        }
                    }
                }
            }
        }
    }

    Point aPos( rInf.GetPos().X(), rInf.GetPos().Y() + rInf.GetAscent() );
    const Point aOldPos(rInf.GetPos());
    rInf.SetPos( aPos );

    if( !bOpaque )
    {
        if( rInf.GetKern() )
            rInf.GetFont()->_DrawStretchText( rInf );
        else
            rInf.GetFont()->_DrawText( rInf );
        rInf.SetPos(aOldPos);
        return false;
    }
    else if( !aRegion.empty() )
    {
        
        SwSaveClip aClipVout( rInf.GetpOut() );
        for( MSHORT i = 0; i < aRegion.size(); ++i )
        {
            SwRect &rRect = aRegion[i];
            if( rRect != aRegion.GetOrigin() )
                aClipVout.ChgClip( rRect );
            if( rInf.GetKern() )
                rInf.GetFont()->_DrawStretchText( rInf );
            else
                rInf.GetFont()->_DrawText( rInf );
        }
    }
    rInf.SetPos(aOldPos);
    return true;
}


void SwTxtFly::DrawFlyRect( OutputDevice* pOut, const SwRect &rRect,
        const SwTxtPaintInfo &rInf, bool bNoGraphic )
{
    SwRegionRects aRegion( rRect );
    OSL_ENSURE( !bTopRule, "DrawFlyRect: Wrong TopRule" );
    
    SwAnchoredObjList::size_type nCount( bOn ? GetAnchoredObjList()->size() : 0 );
    if ( bOn && nCount > 0 )
    {
        MSHORT nHellId = pPage->getRootFrm()->GetCurrShell()->getIDocumentDrawModelAccess()->GetHellId();
        for( MSHORT i = 0; i < nCount; ++i )
        {
            
            const SwAnchoredObject* pAnchoredObjTmp = (*mpAnchoredObjList)[i];
            if( mpCurrAnchoredObj != pAnchoredObjTmp &&
                dynamic_cast<const SwFlyFrm*>(pAnchoredObjTmp) )
            {
                
                const SwFmtSurround& rSur = pAnchoredObjTmp->GetFrmFmt().GetSurround();

                
                
                
                
                const SwFlyFrm* pFly = dynamic_cast<const SwFlyFrm*>(pAnchoredObjTmp);
                
                
                bool bClipFlyArea =
                        ( ( SURROUND_THROUGHT == rSur.GetSurround() )
                          
                          ? (pAnchoredObjTmp->GetDrawObj()->GetLayer() != nHellId)
                          : !rSur.IsContour() ) &&
                        !pFly->IsBackgroundTransparent() &&
                        !pFly->IsShadowTransparent() &&
                        ( !pFly->Lower() ||
                          !pFly->Lower()->IsNoTxtFrm() ||
                          !static_cast<const SwNoTxtFrm*>(pFly->Lower())->IsTransparent() );
                if ( bClipFlyArea )
                {
                    
                    SwRect aFly( pAnchoredObjTmp->GetObjRect() );
                    
                    ::SwAlignRect( aFly, pPage->getRootFrm()->GetCurrShell() );
                    if( aFly.Width() > 0 && aFly.Height() > 0 )
                        aRegion -= aFly;
                }
            }
        }
    }

    for( MSHORT i = 0; i < aRegion.size(); ++i )
    {
        if ( bNoGraphic )
            pOut->DrawRect( aRegion[i].SVRect() );
        else
        {
            OSL_ENSURE( ((SvxBrushItem*)-1) != rInf.GetBrushItem(),
                    "DrawRect: Uninitialized BrushItem!" );
            ::DrawGraphic( rInf.GetBrushItem(), 0, 0, pOut, rInf.GetBrushRect(),
                       aRegion[i] );
        }
    }
}



bool SwTxtFly::GetTop( const SwAnchoredObject* _pAnchoredObj,
                       const bool bInFtn,
                       const bool bInFooterOrHeader )
{
    
    
    if( _pAnchoredObj != mpCurrAnchoredObj )
    {
        
        const SdrObject* pNew = _pAnchoredObj->GetDrawObj();
        
        if(pNew && pNew->ISA(SdrEdgeObj))
        {
            if(((SdrEdgeObj*)pNew)->GetConnectedNode(true)
                || ((SdrEdgeObj*)pNew)->GetConnectedNode(false))
            {
                return false;
            }
        }

        if( ( bInFtn || bInFooterOrHeader ) && bTopRule )
        {
            
            const SwFrmFmt& rFrmFmt = _pAnchoredObj->GetFrmFmt();
            const SwFmtAnchor& rNewA = rFrmFmt.GetAnchor();
            if (FLY_AT_PAGE == rNewA.GetAnchorId())
            {
                if ( bInFtn )
                    return false;

                if ( bInFooterOrHeader )
                {
                    SwFmtVertOrient aVert( rFrmFmt.GetVertOrient() );
                    bool bVertPrt = aVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA ||
                            aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA;
                    if( bVertPrt )
                        return false;
                }
            }
        }

        
        
        
        bool bEvade = !mpCurrAnchoredObj ||
                          Is_Lower_Of( dynamic_cast<const SwFlyFrm*>(mpCurrAnchoredObj), pNew);

        if ( !bEvade )
        {
            
            
            

            
            
            if ( !bTopRule )
                bEvade = true;
            else
            {
                
                
                const SwFmtChain &rChain = mpCurrAnchoredObj->GetFrmFmt().GetChain();
                if ( !rChain.GetPrev() && !rChain.GetNext() )
                {
                    
                    const SwFmtAnchor& rNewA = _pAnchoredObj->GetFrmFmt().GetAnchor();
                    
                    const SwFmtAnchor& rCurrA = mpCurrAnchoredObj->GetFrmFmt().GetAnchor();

                    
                    
                    if (FLY_AS_CHAR == rCurrA.GetAnchorId())
                        return false;

                    
                    
                    
                    
                    if (FLY_AT_PAGE == rNewA.GetAnchorId())
                    {
                        if (FLY_AT_PAGE == rCurrA.GetAnchorId())
                        {
                            bEvade = true;
                        }
                        else
                            return false;
                    }
                    else if (FLY_AT_PAGE == rCurrA.GetAnchorId())
                        return false; 
                    else if (FLY_AT_FLY == rNewA.GetAnchorId())
                        bEvade = true; 
                    else if( FLY_AT_FLY == rCurrA.GetAnchorId() )
                        return false; 
                    
                    
                    
                    
                    
                    
                    else
                        return false;
                }
            }

            
            
            bEvade &= ( mpCurrAnchoredObj->GetDrawObj()->GetOrdNum() < pNew->GetOrdNum() );
            if( bEvade )
            {
                
                SwRect aTmp( _pAnchoredObj->GetObjRectWithSpaces() );
                if ( !aTmp.IsOver( mpCurrAnchoredObj->GetObjRectWithSpaces() ) )
                    bEvade = false;
            }
        }

        if ( bEvade )
        {
            
            const SwFmtAnchor& rNewA = _pAnchoredObj->GetFrmFmt().GetAnchor();
            OSL_ENSURE( FLY_AS_CHAR != rNewA.GetAnchorId(),
                    "Don't call GetTop with a FlyInCntFrm" );
            if (FLY_AT_PAGE == rNewA.GetAnchorId())
                return true;  

            
            
            
            
            
            
            const SwFrm* pTmp = _pAnchoredObj->GetAnchorFrm();
            if( pTmp == pCurrFrm )
                return true;
            if( pTmp->IsTxtFrm() && ( pTmp->IsInFly() || pTmp->IsInFtn() ) )
            {
                
                Point aPos = _pAnchoredObj->GetObjRect().Pos();
                pTmp = GetVirtualUpper( pTmp, aPos );
            }
            
            
            
            
            else if ( pTmp->IsTxtFrm() && pTmp->IsInTab() )
            {
                pTmp = const_cast<SwAnchoredObject*>(_pAnchoredObj)
                                ->GetAnchorFrmContainingAnchPos()->GetUpper();
            }
            
            
            
            
            
            
            
            
            
            
            
            const IDocumentSettingAccess* pIDSA = pCurrFrm->GetTxtNode()->getIDocumentSettingAccess();
            if ( (  pIDSA->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) ||
                   !pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) ) &&
                 ::FindKontext( pTmp, 0 ) == ::FindKontext( pCurrFrm, 0 ) )
            {
                return true;
            }

            const SwFrm* pHeader = 0;
            if ( pCurrFrm->GetNext() != pTmp &&
                 ( IsFrmInSameKontext( pTmp, pCurrFrm ) ||
                   
                   ( !pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) &&
                     0 != ( pHeader = pTmp->FindFooterOrHeader() ) &&
                     !pHeader->IsFooterFrm() &&
                     pCurrFrm->IsInDocBody() ) ) )
            {
                if( pHeader || FLY_AT_FLY == rNewA.GetAnchorId() )
                    return true;

                
                
                sal_uLong nTmpIndex = rNewA.GetCntntAnchor()->nNode.GetIndex();
                
                
                
                
                
                if( ULONG_MAX == nIndex )
                    nIndex = pCurrFrm->GetNode()->GetIndex();

                if( nIndex >= nTmpIndex )
                    return true;
            }
        }
    }
    return false;
}


SwAnchoredObjList* SwTxtFly::InitAnchoredObjList()
{
    OSL_ENSURE( pCurrFrm, "InitFlyList: No Frame, no FlyList" );
    
    OSL_ENSURE( !mpAnchoredObjList, "InitFlyList: FlyList already initialized" );

    SWAP_IF_SWAPPED( pCurrFrm )

    const SwSortedObjs *pSorted = pPage->GetSortedObjs();
    const sal_uInt32 nCount = pSorted ? pSorted->Count() : 0;
    
    
    const bool bFooterHeader = 0 != pCurrFrm->FindFooterOrHeader();
    const IDocumentSettingAccess* pIDSA = pCurrFrm->GetTxtNode()->getIDocumentSettingAccess();
    
    const bool bWrapAllowed = ( pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) ||
                                    ( !pCurrFrm->IsInFtn() && !bFooterHeader ) );

    bOn = false;

    if( nCount && bWrapAllowed )
    {
        
        mpAnchoredObjList = new SwAnchoredObjList();

        
        
        SwRect aRect;
        if ( pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) )
        {
            aRect = pCurrFrm->Prt();
            aRect += pCurrFrm->Frm().Pos();
        }
        else
        {
            aRect = pCurrFrm->Frm();
        }
        
        
        SWRECTFN( pCurrFrm )
        const long nRight = (aRect.*fnRect->fnGetRight)() - 1;
        const long nLeft = (aRect.*fnRect->fnGetLeft)() + 1;
        const bool bR2L = pCurrFrm->IsRightToLeft();

        const IDocumentDrawModelAccess* pIDDMA = pCurrFrm->GetTxtNode()->getIDocumentDrawModelAccess();

        for( sal_uInt32 i = 0; i < nCount; i++ )
        {
            
            
            
            
            
            
            
            
            SwAnchoredObject* pAnchoredObj = (*pSorted)[ i ];
            assert(pAnchoredObj);
            if ( !pAnchoredObj ||
                 !pIDDMA->IsVisibleLayerId( pAnchoredObj->GetDrawObj()->GetLayer() ) ||
                 !pAnchoredObj->ConsiderForTextWrap() ||
                 ( mbIgnoreObjsInHeaderFooter && !bFooterHeader &&
                   pAnchoredObj->GetAnchorFrm()->FindFooterOrHeader() ) )
            {
                continue;
            }

            const SwRect aBound( pAnchoredObj->GetObjRectWithSpaces() );
            if ( nRight < (aBound.*fnRect->fnGetLeft)() ||
                 (*fnRect->fnYDiff)( (aRect.*fnRect->fnGetTop)(),
                                     (aBound.*fnRect->fnGetBottom)() ) > 0 ||
                 nLeft > (aBound.*fnRect->fnGetRight)() ||
                 (aBound.*fnRect->fnGetHeight)() >
                                    2 * (pPage->Frm().*fnRect->fnGetHeight)() )
            {
                continue;
            }

            
            
            
            if ( GetTop( pAnchoredObj, pCurrFrm->IsInFtn(), bFooterHeader ) )
            {
                
                
                
                
                
                
                {
                    SwAnchoredObjList::iterator aInsPosIter =
                            std::lower_bound( mpAnchoredObjList->begin(),
                                              mpAnchoredObjList->end(),
                                              pAnchoredObj,
                                              AnchoredObjOrder( bR2L, fnRect ) );

                    mpAnchoredObjList->insert( aInsPosIter, pAnchoredObj );
                }

                const SwFmtSurround &rFlyFmt = pAnchoredObj->GetFrmFmt().GetSurround();
                
                if ( rFlyFmt.IsAnchorOnly() &&
                     pAnchoredObj->GetAnchorFrm() == GetMaster() )
                {
                    const SwFmtVertOrient &rTmpFmt =
                                    pAnchoredObj->GetFrmFmt().GetVertOrient();
                    if( text::VertOrientation::BOTTOM != rTmpFmt.GetVertOrient() )
                        nMinBottom = ( bVert && nMinBottom ) ?
                                     std::min( nMinBottom, aBound.Left() ) :
                                     std::max( nMinBottom, (aBound.*fnRect->fnGetBottom)() );
                }

                bOn = true;
            }
        }
        if( nMinBottom )
        {
            SwTwips nMax = (pCurrFrm->GetUpper()->*fnRect->fnGetPrtBottom)();
            if( (*fnRect->fnYDiff)( nMinBottom, nMax ) > 0 )
                nMinBottom = nMax;
        }
    }
    else
    {
        
        mpAnchoredObjList = new SwAnchoredObjList();
    }

    UNDO_SWAP( pCurrFrm )

    
    return mpAnchoredObjList;
}

SwTwips SwTxtFly::CalcMinBottom() const
{
    SwTwips nRet = 0;
    const SwCntntFrm *pLclMaster = GetMaster();
    OSL_ENSURE(pLclMaster, "SwTxtFly without master");
    const SwSortedObjs *pDrawObj = pLclMaster ? pLclMaster->GetDrawObjs() : NULL;
    const sal_uInt32 nCount = pDrawObj ? pDrawObj->Count() : 0;
    if( nCount )
    {
        SwTwips nEndOfFrm = pCurrFrm->Frm().Bottom();
        for( sal_uInt32 i = 0; i < nCount; i++ )
        {
            SwAnchoredObject* pAnchoredObj = (*pDrawObj)[ i ];
            const SwFmtSurround &rFlyFmt = pAnchoredObj->GetFrmFmt().GetSurround();
            if( rFlyFmt.IsAnchorOnly() )
            {
                const SwFmtVertOrient &rTmpFmt =
                                    pAnchoredObj->GetFrmFmt().GetVertOrient();
                if( text::VertOrientation::BOTTOM != rTmpFmt.GetVertOrient() )
                {
                    const SwRect aBound( pAnchoredObj->GetObjRectWithSpaces() );
                    if( aBound.Top() < nEndOfFrm )
                        nRet = std::max( nRet, aBound.Bottom() );
                }
            }
        }
        SwTwips nMax = pCurrFrm->GetUpper()->Frm().Top() +
                       pCurrFrm->GetUpper()->Prt().Bottom();
        if( nRet > nMax )
            nRet = nMax;
    }
    return nRet;
}


bool SwTxtFly::ForEach( const SwRect &rRect, SwRect* pRect, bool bAvoid ) const
{
    SWAP_IF_SWAPPED( pCurrFrm )

    bool bRet = false;
    
    SwAnchoredObjList::size_type nCount( bOn ? GetAnchoredObjList()->size() : 0 );
    if ( bOn && nCount > 0 )
    {
        for( SwAnchoredObjList::size_type i = 0; i < nCount; ++i )
        {
            
            const SwAnchoredObject* pAnchoredObj = (*mpAnchoredObjList)[i];

            SwRect aRect( pAnchoredObj->GetObjRectWithSpaces() );

            
            SWRECTFN( pCurrFrm )
            if( (aRect.*fnRect->fnGetLeft)() > (rRect.*fnRect->fnGetRight)() )
                break;
            
            if ( mpCurrAnchoredObj != pAnchoredObj && aRect.IsOver( rRect ) )
            {
                
                const SwFmt* pFmt( &(pAnchoredObj->GetFrmFmt()) );
                const SwFmtSurround &rSur = pFmt->GetSurround();
                if( bAvoid )
                {
                    
                    
                    
                    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                    if( ( SURROUND_THROUGHT == rSur.GetSurround() &&
                          ( !rSur.IsAnchorOnly() ||
                            
                            GetMaster() == pAnchoredObj->GetAnchorFrm() ||
                            ((FLY_AT_PARA != rAnchor.GetAnchorId()) &&
                             (FLY_AT_CHAR != rAnchor.GetAnchorId())) ) )
                        || aRect.Top() == FAR_AWAY )
                        continue;
                }

                
                
                
                
                
                
                
                
                if ( mbIgnoreCurrentFrame &&
                     GetMaster() == pAnchoredObj->GetAnchorFrm() )
                    continue;

                if( pRect )
                {
                    
                    SwRect aFly = AnchoredObjToRect( pAnchoredObj, rRect );
                    if( aFly.IsEmpty() || !aFly.IsOver( rRect ) )
                        continue;
                    if( !bRet || (
                        ( !pCurrFrm->IsRightToLeft() &&
                          ( (aFly.*fnRect->fnGetLeft)() <
                            (pRect->*fnRect->fnGetLeft)() ) ) ||
                        ( pCurrFrm->IsRightToLeft() &&
                          ( (aFly.*fnRect->fnGetRight)() >
                            (pRect->*fnRect->fnGetRight)() ) ) ) )
                        *pRect = aFly;
                    if( rSur.IsContour() )
                    {
                        bRet = true;
                        continue;
                    }
                }
                bRet = true;
                break;
            }
        }
    }

    UNDO_SWAP( pCurrFrm )

    return bRet;
}



SwAnchoredObjList::size_type SwTxtFly::GetPos( const SwAnchoredObject* pAnchoredObj ) const
{
    SwAnchoredObjList::size_type nCount = GetAnchoredObjList()->size();
    SwAnchoredObjList::size_type nRet = 0;
    while ( nRet < nCount && pAnchoredObj != (*mpAnchoredObjList)[ nRet ] )
        ++nRet;
    return nRet;
}


void SwTxtFly::CalcRightMargin( SwRect &rFly,
                                SwAnchoredObjList::size_type nFlyPos,
                                const SwRect &rLine ) const
{
    
    OSL_ENSURE( ! pCurrFrm->IsVertical() || ! pCurrFrm->IsSwapped(),
            "SwTxtFly::CalcRightMargin with swapped frame" );
    SWRECTFN( pCurrFrm )
    
    SwTwips nRight = (pCurrFrm->*fnRect->fnGetPrtRight)();
    SwTwips nFlyRight = (rFly.*fnRect->fnGetRight)();
    SwRect aLine( rLine );
    (aLine.*fnRect->fnSetRight)( nRight );
    (aLine.*fnRect->fnSetLeft)( (rFly.*fnRect->fnGetLeft)() );

    
    
    
    
    
    
    SwSurround eSurroundForTextWrap;

    bool bStop = false;
    
    SwAnchoredObjList::size_type nPos = 0;

    
    while( nPos < mpAnchoredObjList->size() && !bStop )
    {
        if( nPos == nFlyPos )
        {
            ++nPos;
            continue;
        }
        
        const SwAnchoredObject* pNext = (*mpAnchoredObjList)[ nPos++ ];
        if ( pNext == mpCurrAnchoredObj )
            continue;
        eSurroundForTextWrap = _GetSurroundForTextWrap( pNext );
        if( SURROUND_THROUGHT == eSurroundForTextWrap )
            continue;

        const SwRect aTmp( SwContourCache::CalcBoundRect
                ( pNext, aLine, pCurrFrm, nFlyRight, true ) );
        SwTwips nTmpRight = (aTmp.*fnRect->fnGetRight)();

        
        
        
        
        
        
        
        
        
        const long nTmpTop = (aTmp.*fnRect->fnGetTop)();
        if( (*fnRect->fnYDiff)( nTmpTop, (aLine.*fnRect->fnGetTop)() ) > 0 )
        {
            if( (*fnRect->fnYDiff)( nNextTop, nTmpTop ) > 0 )
                SetNextTop( nTmpTop ); 
        }
        else if (!(aTmp.*fnRect->fnGetWidth)()) 
        {   
            
            
            
            if( ! (aTmp.*fnRect->fnGetHeight)() ||
                (*fnRect->fnYDiff)( (aTmp.*fnRect->fnGetBottom)(),
                                    (aLine.*fnRect->fnGetTop)() ) > 0 )
                SetNextTop( 0 );
        }
        if( aTmp.IsOver( aLine ) && nTmpRight > nFlyRight )
        {
            nFlyRight = nTmpRight;
            if( SURROUND_RIGHT == eSurroundForTextWrap ||
                SURROUND_PARALLEL == eSurroundForTextWrap )
            {
                
                if( nRight > nFlyRight )
                    nRight = nFlyRight;
                bStop = true;
            }
        }
    }
    (rFly.*fnRect->fnSetRight)( nRight );
}


void SwTxtFly::CalcLeftMargin( SwRect &rFly,
                               SwAnchoredObjList::size_type nFlyPos,
                               const SwRect &rLine ) const
{
    OSL_ENSURE( ! pCurrFrm->IsVertical() || ! pCurrFrm->IsSwapped(),
            "SwTxtFly::CalcLeftMargin with swapped frame" );
    SWRECTFN( pCurrFrm )
    
    SwTwips nLeft = (pCurrFrm->*fnRect->fnGetPrtLeft)();
    const SwTwips nFlyLeft = (rFly.*fnRect->fnGetLeft)();

    if( nLeft > nFlyLeft )
        nLeft = rFly.Left();

    SwRect aLine( rLine );
    (aLine.*fnRect->fnSetLeft)( nLeft );

    
    
    
    
    

    
    SwAnchoredObjList::size_type nMyPos = nFlyPos;
    while( ++nFlyPos < mpAnchoredObjList->size() )
    {
        
        const SwAnchoredObject* pNext = (*mpAnchoredObjList)[ nFlyPos ];
        const SwRect aTmp( pNext->GetObjRectWithSpaces() );
        if( (aTmp.*fnRect->fnGetLeft)() >= nFlyLeft )
            break;
    }

    while( nFlyPos )
    {
        if( --nFlyPos == nMyPos )
            continue;
        
        const SwAnchoredObject* pNext = (*mpAnchoredObjList)[ nFlyPos ];
        if( pNext == mpCurrAnchoredObj )
            continue;
        SwSurround eSurroundForTextWrap = _GetSurroundForTextWrap( pNext );
        if( SURROUND_THROUGHT == eSurroundForTextWrap )
            continue;

        const SwRect aTmp( SwContourCache::CalcBoundRect
                ( pNext, aLine, pCurrFrm, nFlyLeft, false ) );

        if( (aTmp.*fnRect->fnGetLeft)() < nFlyLeft && aTmp.IsOver( aLine ) )
        {
            
            
            SwTwips nTmpRight = (aTmp.*fnRect->fnGetRight)();
            if ( nLeft <= nTmpRight )
                nLeft = nTmpRight;

            break;
        }
    }
    (rFly.*fnRect->fnSetLeft)( nLeft );
}


SwRect SwTxtFly::AnchoredObjToRect( const SwAnchoredObject* pAnchoredObj,
                            const SwRect &rLine ) const
{
    SWRECTFN( pCurrFrm )

    const long nXPos = pCurrFrm->IsRightToLeft() ?
                       rLine.Right() :
                       (rLine.*fnRect->fnGetLeft)();

    SwRect aFly = mbIgnoreContour ?
                  pAnchoredObj->GetObjRectWithSpaces() :
                  SwContourCache::CalcBoundRect( pAnchoredObj, rLine, pCurrFrm,
                                                 nXPos, ! pCurrFrm->IsRightToLeft() );

    if( !aFly.Width() )
        return aFly;

    
    SetNextTop( (aFly.*fnRect->fnGetBottom)() );
    SwAnchoredObjList::size_type nFlyPos = GetPos( pAnchoredObj );

    
    
    
    
    
    
    
    
    
    switch( _GetSurroundForTextWrap( pAnchoredObj ) )
    {
        case SURROUND_LEFT :
        {
            CalcRightMargin( aFly, nFlyPos, rLine );
            break;
        }
        case SURROUND_RIGHT :
        {
            CalcLeftMargin( aFly, nFlyPos, rLine );
            break;
        }
        case SURROUND_NONE :
        {
            CalcRightMargin( aFly, nFlyPos, rLine );
            CalcLeftMargin( aFly, nFlyPos, rLine );
            break;
        }
        default:
            break;
    }
    return aFly;
}




#define TEXT_MIN 1134


#define TEXT_MIN_SMALL 300


#define FRAME_MAX 850

SwSurround SwTxtFly::_GetSurroundForTextWrap( const SwAnchoredObject* pAnchoredObj ) const
{
    const SwFrmFmt* pFmt = &(pAnchoredObj->GetFrmFmt());
    const SwFmtSurround &rFlyFmt = pFmt->GetSurround();
    SwSurround eSurroundForTextWrap = rFlyFmt.GetSurround();

    if( rFlyFmt.IsAnchorOnly() && pAnchoredObj->GetAnchorFrm() != GetMaster() )
    {
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        if ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
            (FLY_AT_CHAR == rAnchor.GetAnchorId()))
        {
            return SURROUND_NONE;
        }
    }

    
    if( SURROUND_THROUGHT == eSurroundForTextWrap ||
        SURROUND_NONE == eSurroundForTextWrap )
        return eSurroundForTextWrap;

    
    if ( pCurrFrm->IsRightToLeft() )
    {
        if ( SURROUND_LEFT == eSurroundForTextWrap )
            eSurroundForTextWrap = SURROUND_RIGHT;
        else if ( SURROUND_RIGHT == eSurroundForTextWrap )
            eSurroundForTextWrap = SURROUND_LEFT;
    }

    
    if ( SURROUND_IDEAL == eSurroundForTextWrap )
    {
        SWRECTFN( pCurrFrm )
        const long nCurrLeft = (pCurrFrm->*fnRect->fnGetPrtLeft)();
        const long nCurrRight = (pCurrFrm->*fnRect->fnGetPrtRight)();
        const SwRect aRect( pAnchoredObj->GetObjRectWithSpaces() );
        long nFlyLeft = (aRect.*fnRect->fnGetLeft)();
        long nFlyRight = (aRect.*fnRect->fnGetRight)();

        if ( nFlyRight < nCurrLeft || nFlyLeft > nCurrRight )
            eSurroundForTextWrap = SURROUND_PARALLEL;
        else
        {
            long nLeft = nFlyLeft - nCurrLeft;
            long nRight = nCurrRight - nFlyRight;
            if( nFlyRight - nFlyLeft > FRAME_MAX )
            {
                if( nLeft < nRight )
                    nLeft = 0;
                else
                    nRight = 0;
            }
            const int textMin = GetMaster()->GetNode()
                ->getIDocumentSettingAccess()->get(IDocumentSettingAccess::SURROUND_TEXT_WRAP_SMALL )
                ? TEXT_MIN_SMALL : TEXT_MIN;
            if( nLeft < textMin )
                nLeft = 0;
            if( nRight < textMin )
                nRight = 0;
            if( nLeft )
                eSurroundForTextWrap = nRight ? SURROUND_PARALLEL : SURROUND_LEFT;
            else
                eSurroundForTextWrap = nRight ? SURROUND_RIGHT: SURROUND_NONE;
        }
    }

    return eSurroundForTextWrap;
}

bool SwTxtFly::IsAnyFrm( const SwRect &rLine ) const
{

    SWAP_IF_SWAPPED( pCurrFrm )

    OSL_ENSURE( bOn, "IsAnyFrm: Why?" );

    const bool bRet = ForEach( rLine, NULL, false );
    UNDO_SWAP( pCurrFrm )
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
