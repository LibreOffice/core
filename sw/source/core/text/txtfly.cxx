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


#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "pam.hxx"          // SwPosition
#include "swregion.hxx"     // SwRegionRects
#include "dflyobj.hxx"      // SdrObject
#include "flyfrm.hxx"       // SwFlyFrm
#include "frmtool.hxx"      // ::DrawGraphic
#include "porfly.hxx"       // NewFlyCntPortion
#include "porfld.hxx"       // SwGrfNumPortion
#include "txtfly.hxx"       // SwTxtFly
#include "txtpaint.hxx"     // SwSaveClip
#include "txtatr.hxx"       // SwTxtFlyCnt
#include "notxtfrm.hxx"
#include "fmtcnct.hxx"      // SwFmtChain
#include "inftxt.hxx"
#include <pormulti.hxx>     // SwMultiPortion
#include <svx/obj3d.hxx>
#include <editeng/txtrange.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
// #i28701#
#include <editeng/lspcitem.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>
#include <sortedobjs.hxx>
#include <layouter.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <svx/svdoedge.hxx>

#ifdef DBG_UTIL
#include "viewsh.hxx"
#include "viewopt.hxx"  // SwViewOptions, only for testing (Test2)
#include "doc.hxx"
#endif


using namespace ::com::sun::star;

namespace
{
    // #i68520#
    struct AnchoredObjOrder
    {
        sal_Bool mbR2L;
        SwRectFn mfnRect;

        AnchoredObjOrder( const sal_Bool bR2L,
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
    OSL_ENSURE( pTextRanger[ nPos ], "ClrObject: Allready cleared. Good Bye!" );
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

// #i68520#
const SwRect SwContourCache::CalcBoundRect( const SwAnchoredObject* pAnchoredObj,
                                            const SwRect &rLine,
                                            const SwTxtFrm* pFrm,
                                            const long nXPos,
                                            const sal_Bool bRight )
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
    const long nXPos, const sal_Bool bRight )
{
    SwRect aRet;
    MSHORT nPos = 0; // Suche im Cache ...
    while( nPos < GetCount() && pObj != pSdrObj[ nPos ] )
        ++nPos;
    if( GetCount() == nPos ) // nicht gefunden
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
            // GetContour() causes the graphic to be loaded, which may cause
            // the graphic to change its size, call ClrObject()
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
        pSdrObj[ 0 ] = pObj; // due to #37347 the Object must be entered only
                             // after GetContour()
        pTextRanger[ 0 ] = new TextRanger( aPolyPolygon, pPolyPolygon, 20,
            (sal_uInt16)rLRSpace.GetLeft(), (sal_uInt16)rLRSpace.GetRight(),
            pFmt->GetSurround().IsOutside(), sal_False, pFrm->IsVertical() );
        pTextRanger[ 0 ]->SetUpper( rULSpace.GetUpper() );
        pTextRanger[ 0 ]->SetLower( rULSpace.GetLower() );

        delete pPolyPolygon;
        // UPPER_LOWER_TEST
#ifdef DBG_UTIL
        const ViewShell* pTmpViewShell = pFmt->GetDoc()->GetCurrentViewShell();
        if( pTmpViewShell )
        {
            sal_Bool bT2 = pTmpViewShell->GetViewOptions()->IsTest2();
            sal_Bool bT6 = pTmpViewShell->GetViewOptions()->IsTest6();
            if( bT2 || bT6 )
            {
                if( bT2 )
                    pTextRanger[ 0 ]->SetFlag7( sal_True );
                else
                    pTextRanger[ 0 ]->SetFlag6( sal_True );
            }
        }
#endif
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
    // fnGetBottom is top + height
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
            --nIdx; // within interval
        else if( ! bRight && ( nIdx >= nCount || (*pTmp)[ nIdx ] != nXPos ) )
        {
            if( nIdx )
                nIdx -= 2; // an interval to the left
            else
                bSet = false; // before the first interval
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
    bLeftSide = rTxtFly.bLeftSide;
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
    mbIgnoreCurrentFrame = sal_False;
    mbIgnoreContour = sal_False;
    // #118809#
    mbIgnoreObjsInHeaderFooter = sal_False;
    pPage = pFrm->FindPageFrm();
    const SwFlyFrm* pTmp = pFrm->FindFlyFrm();
    // #i68520#
    mpCurrAnchoredObj = pTmp;
    pCurrFrm = pFrm;
    pMaster = pCurrFrm->IsFollow() ? NULL : pCurrFrm;
    // #i68520#
    mpAnchoredObjList = NULL;
    // If we're not overlapped by a frame or if a FlyCollection does not exist
    // at all, we switch off forever.
    // It could be, however, that a line is added while formatting, that
    // extends into a frame.
    // That's why we do not optimize for: bOn = pSortedFlys && IsAnyFrm();
    bOn = pPage->GetSortedObjs() != 0;
    bTopRule = sal_True;
    bLeftSide = sal_False;
    nMinBottom = 0;
    nIndex = ULONG_MAX;
}


SwRect SwTxtFly::_GetFrm( const SwRect &rRect, sal_Bool bTop ) const
{
    SwRect aRet;
    if( ForEach( rRect, &aRet, sal_True ) )
    {
        SWRECTFN( pCurrFrm )
        if( bTop )
            (aRet.*fnRect->fnSetTop)( (rRect.*fnRect->fnGetTop)() );

        // 8110: Do not always adapt the bottom
        const SwTwips nRetBottom = (aRet.*fnRect->fnGetBottom)();
        const SwTwips nRectBottom = (rRect.*fnRect->fnGetBottom)();
        if ( (*fnRect->fnYDiff)( nRetBottom, nRectBottom ) > 0 ||
             (aRet.*fnRect->fnGetHeight)() < 0 )
            (aRet.*fnRect->fnSetBottom)( nRectBottom );
    }
    return aRet;
}


sal_Bool SwTxtFly::IsAnyFrm() const
{
    SWAP_IF_SWAPPED( pCurrFrm )

    OSL_ENSURE( bOn, "IsAnyFrm: Why?" );
    SwRect aRect( pCurrFrm->Frm().Pos() + pCurrFrm->Prt().Pos(),
        pCurrFrm->Prt().SSize() );

    const sal_Bool bRet = ForEach( aRect, NULL, sal_False );
    UNDO_SWAP( pCurrFrm )
    return bRet;
}

sal_Bool SwTxtFly::IsAnyObj( const SwRect &rRect ) const
{
   OSL_ENSURE( bOn, "SwTxtFly::IsAnyObj: Who's knocking?" );

    SwRect aRect( rRect );
    if ( aRect.IsEmpty() )
        aRect = SwRect( pCurrFrm->Frm().Pos() + pCurrFrm->Prt().Pos(),
                        pCurrFrm->Prt().SSize() );

    const SwSortedObjs *pSorted = pPage->GetSortedObjs();
    if( pSorted ) // bOn actually makes sure that we have objects on the side,
                  // but who knows who deleted somehting in the meantime?
    {
        for ( MSHORT i = 0; i < pSorted->Count(); ++i )
        {
            const SwAnchoredObject* pObj = (*pSorted)[i];

            const SwRect aBound( pObj->GetObjRectWithSpaces() );

            // Optimization
            if( pObj->GetObjRect().Left() > aRect.Right() )
                continue;

            // #i68520#
            if( mpCurrAnchoredObj != pObj && aBound.IsOver( aRect ) )
                return sal_True;
        }
    }
    return sal_False;
}

const SwCntntFrm* SwTxtFly::_GetMaster()
{
    pMaster = pCurrFrm;
    while( pMaster && pMaster->IsFollow() )
        pMaster = (SwCntntFrm*)pMaster->FindMaster();
    return pMaster;
}


sal_Bool SwTxtFly::DrawTextOpaque( SwDrawTextInfo &rInf )
{
    SwSaveClip aClipSave( rInf.GetpOut() );
    SwRect aRect( rInf.GetPos(), rInf.GetSize() );
    if( rInf.GetSpace() )
    {
        xub_StrLen nTmpLen = STRING_LEN == rInf.GetLen() ? rInf.GetText().getLength() :
                                                      rInf.GetLen();
        if( rInf.GetSpace() > 0 )
        {
            xub_StrLen nSpaceCnt = 0;
            const xub_StrLen nEndPos = rInf.GetIdx() + nTmpLen;
            for( xub_StrLen nPos = rInf.GetIdx(); nPos < nEndPos; ++nPos )
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
    // #i68520#
    const sal_uInt32 nCurrOrd = mpCurrAnchoredObj
                            ? mpCurrAnchoredObj->GetDrawObj()->GetOrdNum()
                            : SAL_MAX_UINT32;
    OSL_ENSURE( !bTopRule, "DrawTextOpaque: Wrong TopRule" );

    // #i68520#
    SwAnchoredObjList::size_type nCount( bOn ? GetAnchoredObjList()->size() : 0 );
    if ( bOn && nCount > 0 )
    {
        MSHORT nHellId = pPage->getRootFrm()->GetCurrShell()->getIDocumentDrawModelAccess()->GetHellId();
        for( MSHORT i = 0; i < nCount; ++i )
        {
            // #i68520#
            const SwAnchoredObject* pTmpAnchoredObj = (*mpAnchoredObjList)[i];
            if( dynamic_cast<const SwFlyFrm*>(pTmpAnchoredObj) &&
                mpCurrAnchoredObj != pTmpAnchoredObj )
            {
                // #i68520#
                const SwFlyFrm* pFly = dynamic_cast<const SwFlyFrm*>(pTmpAnchoredObj);
                if( aRegion.GetOrigin().IsOver( pFly->Frm() ) )
                {
                    const SwFrmFmt *pFmt = pFly->GetFmt();
                    const SwFmtSurround &rSur = pFmt->GetSurround();
                    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                    // Only the ones who are opaque and more to the top
                    if( !( pFly->IsBackgroundTransparent()
                           || pFly->IsShadowTransparent() ) &&
                        SURROUND_THROUGHT == rSur.GetSurround() &&
                        ( !rSur.IsAnchorOnly() ||
                          // #i68520#
                          GetMaster() == pFly->GetAnchorFrm() ||
                          ((FLY_AT_PARA != rAnchor.GetAnchorId()) &&
                           (FLY_AT_CHAR != rAnchor.GetAnchorId())
                          )
                        ) &&
                        // #i68520#
                        pTmpAnchoredObj->GetDrawObj()->GetLayer() != nHellId &&
                        nCurrOrd < pTmpAnchoredObj->GetDrawObj()->GetOrdNum()
                      )
                    {
                        // Except for the content is transparent
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
    const Point &rOld = rInf.GetPos();
    rInf.SetPos( aPos );

    if( !bOpaque )
    {
        if( rInf.GetKern() )
            rInf.GetFont()->_DrawStretchText( rInf );
        else
            rInf.GetFont()->_DrawText( rInf );
        rInf.SetPos( rOld );
        return sal_False;
    }
    else if( !aRegion.empty() )
    {
        // What a huge effort ...
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
    rInf.SetPos( rOld );
    return sal_True;
}


void SwTxtFly::DrawFlyRect( OutputDevice* pOut, const SwRect &rRect,
        const SwTxtPaintInfo &rInf, sal_Bool bNoGraphic )
{
    SwRegionRects aRegion( rRect );
    OSL_ENSURE( !bTopRule, "DrawFlyRect: Wrong TopRule" );
    // #i68520#
    SwAnchoredObjList::size_type nCount( bOn ? GetAnchoredObjList()->size() : 0 );
    if ( bOn && nCount > 0 )
    {
        MSHORT nHellId = pPage->getRootFrm()->GetCurrShell()->getIDocumentDrawModelAccess()->GetHellId();
        for( MSHORT i = 0; i < nCount; ++i )
        {
            // #i68520#
            const SwAnchoredObject* pAnchoredObjTmp = (*mpAnchoredObjList)[i];
            if( mpCurrAnchoredObj != pAnchoredObjTmp &&
                dynamic_cast<const SwFlyFrm*>(pAnchoredObjTmp) )
            {
                // #i68520#
                const SwFmtSurround& rSur = pAnchoredObjTmp->GetFrmFmt().GetSurround();

                // OD 24.01.2003 #106593# - correct clipping of fly frame area.
                // Consider that fly frame background/shadow can be transparent
                // and <SwAlignRect(..)> fly frame area
                // #i68520#
                const SwFlyFrm* pFly = dynamic_cast<const SwFlyFrm*>(pAnchoredObjTmp);
                // #i47804# - consider transparent graphics
                // and OLE objects.
                bool bClipFlyArea =
                        ( ( SURROUND_THROUGHT == rSur.GetSurround() )
                          // #i68520#
                          ? (pAnchoredObjTmp->GetDrawObj()->GetLayer() != nHellId)
                          : !rSur.IsContour() ) &&
                        !pFly->IsBackgroundTransparent() &&
                        !pFly->IsShadowTransparent() &&
                        ( !pFly->Lower() ||
                          !pFly->Lower()->IsNoTxtFrm() ||
                          !static_cast<const SwNoTxtFrm*>(pFly->Lower())->IsTransparent() );
                if ( bClipFlyArea )
                {
                    // #i68520#
                    SwRect aFly( pAnchoredObjTmp->GetObjRect() );
                    // OD 24.01.2003 #106593#
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

// #i26945# - change first parameter
// Now it's the <SwAnchoredObject> instance of the floating screen object
sal_Bool SwTxtFly::GetTop( const SwAnchoredObject* _pAnchoredObj,
                           const sal_Bool bInFtn,
                           const sal_Bool bInFooterOrHeader )
{
    // #i68520#
    // <mpCurrAnchoredObj> is set, if <pCurrFrm> is inside a fly frame
    if( _pAnchoredObj != mpCurrAnchoredObj )
    {
        // #i26945#
        const SdrObject* pNew = _pAnchoredObj->GetDrawObj();
        // #102344# Ignore connectors which have one or more connections
        if(pNew && pNew->ISA(SdrEdgeObj))
        {
            if(((SdrEdgeObj*)pNew)->GetConnectedNode(sal_True)
                || ((SdrEdgeObj*)pNew)->GetConnectedNode(sal_False))
            {
                return sal_False;
            }
        }

        if( ( bInFtn || bInFooterOrHeader ) && bTopRule )
        {
            // #i26945#
            const SwFrmFmt& rFrmFmt = _pAnchoredObj->GetFrmFmt();
            const SwFmtAnchor& rNewA = rFrmFmt.GetAnchor();
            if (FLY_AT_PAGE == rNewA.GetAnchorId())
            {
                if ( bInFtn )
                    return sal_False;

                if ( bInFooterOrHeader )
                {
                    SwFmtVertOrient aVert( rFrmFmt.GetVertOrient() );
                    bool bVertPrt = aVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA ||
                            aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA;
                    if( bVertPrt )
                        return sal_False;
                }
            }
        }

        // #i68520#
        // bEvade: consider pNew, if we are not inside a fly
        //         consider pNew, if pNew is lower of <mpCurrAnchoredObj>
        sal_Bool bEvade = !mpCurrAnchoredObj ||
                          Is_Lower_Of( dynamic_cast<const SwFlyFrm*>(mpCurrAnchoredObj), pNew);

        if ( !bEvade )
        {
            // We are currently inside a fly frame and pNew is not
            // inside this fly frame. We can do some more checks if
            // we have to consider pNew.

            // If bTopRule is not set, we ignore the frame types.
            // We directly check the z-order
            if ( !bTopRule )
                bEvade = sal_True;
            else
            {
                // Within chained Flys we only avoid Lower
                // #i68520#
                const SwFmtChain &rChain = mpCurrAnchoredObj->GetFrmFmt().GetChain();
                if ( !rChain.GetPrev() && !rChain.GetNext() )
                {
                    // #i26945#
                    const SwFmtAnchor& rNewA = _pAnchoredObj->GetFrmFmt().GetAnchor();
                    // #i68520#
                    const SwFmtAnchor& rCurrA = mpCurrAnchoredObj->GetFrmFmt().GetAnchor();

                    // If <mpCurrAnchoredObj> is anchored as character, its content
                    // does not wrap around pNew
                    if (FLY_AS_CHAR == rCurrA.GetAnchorId())
                        return sal_False;

                    // If pNew is anchored to page and <mpCurrAnchoredObj is not anchored
                    // to page, the content of <mpCurrAnchoredObj> does not wrap around pNew
                    // If both pNew and <mpCurrAnchoredObj> are anchored to page, we can do
                    // some more checks
                    if (FLY_AT_PAGE == rNewA.GetAnchorId())
                    {
                        if (FLY_AT_PAGE == rCurrA.GetAnchorId())
                        {
                            bEvade = sal_True;
                        }
                        else
                            return sal_False;
                    }
                    else if (FLY_AT_PAGE == rCurrA.GetAnchorId())
                        return sal_False; // Page anchored ones only avoid page anchored ones
                    else if (FLY_AT_FLY == rNewA.GetAnchorId())
                        bEvade = sal_True; // Non-page anchored ones avoid frame anchored ones
                    else if( FLY_AT_FLY == rCurrA.GetAnchorId() )
                        return sal_False; // Frame anchored ones do not avoid paragraph anchored ones
                    // #i57062#
                    // In order to avoid loop situation, it's decided to adjust
                    // the wrapping behaviour of content of at-paragraph/at-character
                    // anchored objects to one in the page header/footer and
                    // the document body --> content of at-paragraph/at-character
                    // anchored objects doesn't wrap around each other.
                    else
                        return sal_False;
                }
            }

            // But: we never avoid a subordinate one and additionally we only avoid when overlapping.
            // #i68520#
            bEvade &= ( mpCurrAnchoredObj->GetDrawObj()->GetOrdNum() < pNew->GetOrdNum() );
            if( bEvade )
            {
                // #i68520#
                SwRect aTmp( _pAnchoredObj->GetObjRectWithSpaces() );
                if ( !aTmp.IsOver( mpCurrAnchoredObj->GetObjRectWithSpaces() ) )
                    bEvade = sal_False;
            }
        }

        if ( bEvade )
        {
            // #i26945#
            const SwFmtAnchor& rNewA = _pAnchoredObj->GetFrmFmt().GetAnchor();
            OSL_ENSURE( FLY_AS_CHAR != rNewA.GetAnchorId(),
                    "Don't call GetTop with a FlyInCntFrm" );
            if (FLY_AT_PAGE == rNewA.GetAnchorId())
                return sal_True;  // We always avoid page anchored ones

            // If Flys anchored at paragraph are caught in a FlyCnt, then
            // their influence ends at the borders of the FlyCnt!
            // If we are currently formatting the text of the FlyCnt, then
            // it has to get out of the way of the Frm anchored at paragraph!
            // pCurrFrm ist the anchor of pNew?
            // #i26945#
            const SwFrm* pTmp = _pAnchoredObj->GetAnchorFrm();
            if( pTmp == pCurrFrm )
                return sal_True;
            if( pTmp->IsTxtFrm() && ( pTmp->IsInFly() || pTmp->IsInFtn() ) )
            {
                // #i26945#
                Point aPos = _pAnchoredObj->GetObjRect().Pos();
                pTmp = GetVirtualUpper( pTmp, aPos );
            }
            // #i26945#
            // #115759#
            // If <pTmp> is a text frame inside a table, take the upper
            // of the anchor frame, which contains the anchor position.
            else if ( pTmp->IsTxtFrm() && pTmp->IsInTab() )
            {
                pTmp = const_cast<SwAnchoredObject*>(_pAnchoredObj)
                                ->GetAnchorFrmContainingAnchPos()->GetUpper();
            }
            // #i28701# - consider all objects in same context,
            // if wrapping style is considered on object positioning.
            // Thus, text will wrap around negative positioned objects.
            // #i3317# - remove condition on checking,
            // if wrappings style is considered on object postioning.
            // Thus, text is wrapping around negative positioned objects.
            // #i35640# - no consideration of negative
            // positioned objects, if wrapping style isn't considered on
            // object position and former text wrapping is applied.
            // This condition is typically for documents imported from the
            // OpenOffice.org file format.
            const IDocumentSettingAccess* pIDSA = pCurrFrm->GetTxtNode()->getIDocumentSettingAccess();
            if ( (  pIDSA->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) ||
                   !pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) ) &&
                 ::FindKontext( pTmp, 0 ) == ::FindKontext( pCurrFrm, 0 ) )
            {
                return sal_True;
            }

            const SwFrm* pHeader = 0;
            if ( pCurrFrm->GetNext() != pTmp &&
                 ( IsFrmInSameKontext( pTmp, pCurrFrm ) ||
                   // #i13832#, #i24135# wrap around objects in page header
                   ( !pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) &&
                     0 != ( pHeader = pTmp->FindFooterOrHeader() ) &&
                     !pHeader->IsFooterFrm() &&
                     pCurrFrm->IsInDocBody() ) ) )
            {
                if( pHeader || FLY_AT_FLY == rNewA.GetAnchorId() )
                    return sal_True;

                // Compare indices:
                // The Index of the other is retrieved from the anchor attr.
                sal_uLong nTmpIndex = rNewA.GetCntntAnchor()->nNode.GetIndex();
                // Now check whether the current paragraph is before the anchor
                // of the displaced object in the text, then we don't have to
                // get out of its way.
                // If possible determine Index via SwFmtAnchor because
                // otherwise it's quite expensive.
                if( ULONG_MAX == nIndex )
                    nIndex = pCurrFrm->GetNode()->GetIndex();

                if( nIndex >= nTmpIndex )
                    return sal_True;
            }
        }
    }
    return sal_False;
}

// #i68520#
SwAnchoredObjList* SwTxtFly::InitAnchoredObjList()
{
    OSL_ENSURE( pCurrFrm, "InitFlyList: No Frame, no FlyList" );
    // #i68520#
    OSL_ENSURE( !mpAnchoredObjList, "InitFlyList: FlyList already initialized" );

    SWAP_IF_SWAPPED( pCurrFrm )

    const SwSortedObjs *pSorted = pPage->GetSortedObjs();
    const sal_uInt32 nCount = pSorted ? pSorted->Count() : 0;
    // --> #108724# Page header/footer content doesn't have to wrap around
    //              floating screen objects
    const bool bFooterHeader = 0 != pCurrFrm->FindFooterOrHeader();
    const IDocumentSettingAccess* pIDSA = pCurrFrm->GetTxtNode()->getIDocumentSettingAccess();
    // #i40155# - check, if frame is marked not to wrap
    const bool bWrapAllowed = ( pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) ||
                                    ( !pCurrFrm->IsInFtn() && !bFooterHeader ) );

    bOn = sal_False;

    if( nCount && bWrapAllowed )
    {
        // #i68520#
        mpAnchoredObjList = new SwAnchoredObjList();

        // #i28701# - consider complete frame area for new
        // text wrapping
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
        // Make ourselves a little smaller than we are,
        // so that 1-Twip-overlappings are ignored (#49532)
        SWRECTFN( pCurrFrm )
        const long nRight = (aRect.*fnRect->fnGetRight)() - 1;
        const long nLeft = (aRect.*fnRect->fnGetLeft)() + 1;
        const sal_Bool bR2L = pCurrFrm->IsRightToLeft();

        const IDocumentDrawModelAccess* pIDDMA = pCurrFrm->GetTxtNode()->getIDocumentDrawModelAccess();

        for( sal_uInt32 i = 0; i < nCount; i++ )
        {
            // #i68520#
            // do not consider hidden objects
            // check, if object has to be considered for text wrap
            // #118809# - If requested, do not consider
            // objects in page header|footer for text frames not in page
            // header|footer. This is requested for the calculation of
            // the base offset for objects <SwTxtFrm::CalcBaseOfstForFly()>
            // #i20505# Do not consider oversized objects
            SwAnchoredObject* pAnchoredObj = (*pSorted)[ i ];
            if ( !pIDDMA->IsVisibleLayerId( pAnchoredObj->GetDrawObj()->GetLayer() ) ||
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

            // #i26945# - pass <pAnchoredObj> to method
            // <GetTop(..)> instead of only the <SdrObject> instance of the
            // anchored object
            if ( GetTop( pAnchoredObj, pCurrFrm->IsInFtn(), bFooterHeader ) )
            {
                // OD 11.03.2003 #107862# - adjust insert position:
                // overlapping objects should be sorted from left to right and
                // inside left to right sorting from top to bottom.
                // If objects on the same position are found, they are sorted
                // on its width.
                // #i68520#
                {
                    SwAnchoredObjList::iterator aInsPosIter =
                            std::lower_bound( mpAnchoredObjList->begin(),
                                              mpAnchoredObjList->end(),
                                              pAnchoredObj,
                                              AnchoredObjOrder( bR2L, fnRect ) );

                    mpAnchoredObjList->insert( aInsPosIter, pAnchoredObj );
                }

                const SwFmtSurround &rFlyFmt = pAnchoredObj->GetFrmFmt().GetSurround();
                // #i68520#
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

                bOn = sal_True;
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
        // #i68520#
        mpAnchoredObjList = new SwAnchoredObjList();
    }

    UNDO_SWAP( pCurrFrm )

    // #i68520#
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


sal_Bool SwTxtFly::ForEach( const SwRect &rRect, SwRect* pRect, sal_Bool bAvoid ) const
{
    SWAP_IF_SWAPPED( pCurrFrm )

    sal_Bool bRet = sal_False;
    // #i68520#
    SwAnchoredObjList::size_type nCount( bOn ? GetAnchoredObjList()->size() : 0 );
    if ( bOn && nCount > 0 )
    {
        for( SwAnchoredObjList::size_type i = 0; i < nCount; ++i )
        {
            // #i68520#
            const SwAnchoredObject* pAnchoredObj = (*mpAnchoredObjList)[i];

            SwRect aRect( pAnchoredObj->GetObjRectWithSpaces() );

            // Optimierung
            SWRECTFN( pCurrFrm )
            if( (aRect.*fnRect->fnGetLeft)() > (rRect.*fnRect->fnGetRight)() )
                break;
            // #i68520#
            if ( mpCurrAnchoredObj != pAnchoredObj && aRect.IsOver( rRect ) )
            {
                // #i68520#
                const SwFmt* pFmt( &(pAnchoredObj->GetFrmFmt()) );
                const SwFmtSurround &rSur = pFmt->GetSurround();
                if( bAvoid )
                {
                    // If the text flows below, it has no influence on
                    // formatting. In LineIter::DrawText() it is "just"
                    // necessary to clevely set the ClippingRegions
                    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                    if( ( SURROUND_THROUGHT == rSur.GetSurround() &&
                          ( !rSur.IsAnchorOnly() ||
                            // #i68520#
                            GetMaster() == pAnchoredObj->GetAnchorFrm() ||
                            ((FLY_AT_PARA != rAnchor.GetAnchorId()) &&
                             (FLY_AT_CHAR != rAnchor.GetAnchorId())) ) )
                        || aRect.Top() == FAR_AWAY )
                        continue;
                }

                // #i58642#
                // Compare <GetMaster()> instead of <pCurrFrm> with the anchor
                // frame of the anchored object, because a follow frame have
                // to ignore the anchored objects of its master frame.
                // Note: Anchored objects are always registered at the master
                //       frame, exception are as-character anchored objects,
                //       but these aren't handled here.
                // #i68520#
                if ( mbIgnoreCurrentFrame &&
                     GetMaster() == pAnchoredObj->GetAnchorFrm() )
                    continue;

                if( pRect )
                {
                    // #i68520#
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
                        bRet = sal_True;
                        continue;
                    }
                }
                bRet = sal_True;
                break;
            }
        }
    }

    UNDO_SWAP( pCurrFrm )

    return bRet;
}


// #i68520#
SwAnchoredObjList::size_type SwTxtFly::GetPos( const SwAnchoredObject* pAnchoredObj ) const
{
    SwAnchoredObjList::size_type nCount = GetAnchoredObjList()->size();
    SwAnchoredObjList::size_type nRet = 0;
    while ( nRet < nCount && pAnchoredObj != (*mpAnchoredObjList)[ nRet ] )
        ++nRet;
    return nRet;
}

// #i68520#
void SwTxtFly::CalcRightMargin( SwRect &rFly,
                                SwAnchoredObjList::size_type nFlyPos,
                                const SwRect &rLine ) const
{
    // Usually the right margin is the right margin of the Printarea
    OSL_ENSURE( ! pCurrFrm->IsVertical() || ! pCurrFrm->IsSwapped(),
            "SwTxtFly::CalcRightMargin with swapped frame" );
    SWRECTFN( pCurrFrm )
    // #118796# - correct determination of right of printing area
    SwTwips nRight = (pCurrFrm->*fnRect->fnGetPrtRight)();
    SwTwips nFlyRight = (rFly.*fnRect->fnGetRight)();
    SwRect aLine( rLine );
    (aLine.*fnRect->fnSetRight)( nRight );
    (aLine.*fnRect->fnSetLeft)( (rFly.*fnRect->fnGetLeft)() );

    // It is possible that there is another object that is _above_ us
    // and protrudes into the same line.
    // Flys with run-through are invisible for those below, i.e., they
    // are ignored for computing the margins of other Flys.
    // 3301: pNext->Frm().IsOver( rLine ) is necessary
    // #i68520#
    SwSurround eSurroundForTextWrap;

    bool bStop = false;
    // #i68520#
    SwAnchoredObjList::size_type nPos = 0;

    // #i68520#
    while( nPos < mpAnchoredObjList->size() && !bStop )
    {
        if( nPos == nFlyPos )
        {
            ++nPos;
            continue;
        }
        // #i68520#
        const SwAnchoredObject* pNext = (*mpAnchoredObjList)[ nPos++ ];
        if ( pNext == mpCurrAnchoredObj )
            continue;
        eSurroundForTextWrap = _GetSurroundForTextWrap( pNext );
        if( SURROUND_THROUGHT == eSurroundForTextWrap )
            continue;

        const SwRect aTmp( SwContourCache::CalcBoundRect
                ( pNext, aLine, pCurrFrm, nFlyRight, sal_True ) );
        SwTwips nTmpRight = (aTmp.*fnRect->fnGetRight)();

        // optimization:
        // Record in nNextTop at which Y-position frame related changes are
        // likely.  This is so that, despite only looking at frames in the
        // current line height, for frames without wrap the line height is
        // incremented so that with a single line the lower border of the frame
        // (or possibly the upper border of another frame) is reached.
        // Especially in HTML documents there are often (dummy) paragraphs in
        // 2 pt font, and they used to only evade big frames after huge numbers
        // of empty lines.
        const long nTmpTop = (aTmp.*fnRect->fnGetTop)();
        if( (*fnRect->fnYDiff)( nTmpTop, (aLine.*fnRect->fnGetTop)() ) > 0 )
        {
            if( (*fnRect->fnYDiff)( nNextTop, nTmpTop ) > 0 )
                SetNextTop( nTmpTop ); // upper border of next frame
        }
        else if (!(aTmp.*fnRect->fnGetWidth)()) // typical for Objects with contour wrap
        {   // For Objects with contour wrap that start before the current
            // line, and end below it, but do not actually overlap it, the
            // optimization has to be disabled, because the circumstances
            // can change in the next line.
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
                // overrule the FlyFrm
                if( nRight > nFlyRight )
                    nRight = nFlyRight;
                bStop = true;
            }
        }
    }
    (rFly.*fnRect->fnSetRight)( nRight );
}

// #i68520#
void SwTxtFly::CalcLeftMargin( SwRect &rFly,
                               SwAnchoredObjList::size_type nFlyPos,
                               const SwRect &rLine ) const
{
    OSL_ENSURE( ! pCurrFrm->IsVertical() || ! pCurrFrm->IsSwapped(),
            "SwTxtFly::CalcLeftMargin with swapped frame" );
    SWRECTFN( pCurrFrm )
    // #118796# - correct determination of left of printing area
    SwTwips nLeft = (pCurrFrm->*fnRect->fnGetPrtLeft)();
    const SwTwips nFlyLeft = (rFly.*fnRect->fnGetLeft)();

    if( nLeft > nFlyLeft )
        nLeft = rFly.Left();

    SwRect aLine( rLine );
    (aLine.*fnRect->fnSetLeft)( nLeft );

    // It is possible that there is another object that is _above_ us
    // and protrudes into the same line.
    // Flys with run-through are invisible for those below, i.e., they
    // are ignored for computing the margins of other Flys.
    // 3301: pNext->Frm().IsOver( rLine ) is necessary

    // #i68520#
    SwAnchoredObjList::size_type nMyPos = nFlyPos;
    while( ++nFlyPos < mpAnchoredObjList->size() )
    {
        // #i68520#
        const SwAnchoredObject* pNext = (*mpAnchoredObjList)[ nFlyPos ];
        const SwRect aTmp( pNext->GetObjRectWithSpaces() );
        if( (aTmp.*fnRect->fnGetLeft)() >= nFlyLeft )
            break;
    }

    while( nFlyPos )
    {
        if( --nFlyPos == nMyPos )
            continue;
        // #i68520#
        const SwAnchoredObject* pNext = (*mpAnchoredObjList)[ nFlyPos ];
        if( pNext == mpCurrAnchoredObj )
            continue;
        SwSurround eSurroundForTextWrap = _GetSurroundForTextWrap( pNext );
        if( SURROUND_THROUGHT == eSurroundForTextWrap )
            continue;

        const SwRect aTmp( SwContourCache::CalcBoundRect
                ( pNext, aLine, pCurrFrm, nFlyLeft, sal_False ) );

        if( (aTmp.*fnRect->fnGetLeft)() < nFlyLeft && aTmp.IsOver( aLine ) )
        {
            // #118796# - no '+1', because <..fnGetRight>
            // returns the correct value.
            SwTwips nTmpRight = (aTmp.*fnRect->fnGetRight)();
            if ( nLeft <= nTmpRight )
                nLeft = nTmpRight;

            break;
        }
    }
    (rFly.*fnRect->fnSetLeft)( nLeft );
}

// #i68520#
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

    // so the line may grow up to the lower edge of the frame
    SetNextTop( (aFly.*fnRect->fnGetBottom)() );
    SwAnchoredObjList::size_type nFlyPos = GetPos( pAnchoredObj );

    // LEFT and RIGHT, we grow the rectangle.
    // We have some problems, when several frames are to be seen.
    // At the moment, only the easier case is assumed:
    //  + LEFT means that the text must flow on the left of the frame,
    //    that is the frame expands to the right edge of the print area
    //    or to the next frame.
    //  + RIGHT is the opposite.
    // Otherwise the set distance between text and frame is always
    // added up.
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

// #i68520#

// Wrap only on sides with at least 2cm space for the text
#define TEXT_MIN 1134

// MS Word wraps on sides with even less space (value guessed).
#define TEXT_MIN_SMALL 300

// Wrap on both sides up to a frame width of 1.5cm
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

    // in cause of run-through and nowrap ignore smartly
    if( SURROUND_THROUGHT == eSurroundForTextWrap ||
        SURROUND_NONE == eSurroundForTextWrap )
        return eSurroundForTextWrap;

    // left is left and right is right
    if ( pCurrFrm->IsRightToLeft() )
    {
        if ( SURROUND_LEFT == eSurroundForTextWrap )
            eSurroundForTextWrap = SURROUND_RIGHT;
        else if ( SURROUND_RIGHT == eSurroundForTextWrap )
            eSurroundForTextWrap = SURROUND_LEFT;
    }

    // "ideal page wrap":
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

sal_Bool SwTxtFly::IsAnyFrm( const SwRect &rLine ) const
{

    SWAP_IF_SWAPPED( pCurrFrm )

    OSL_ENSURE( bOn, "IsAnyFrm: Why?" );

    const sal_Bool bRet = ForEach( rLine, NULL, sal_False );
    UNDO_SWAP( pCurrFrm )
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
