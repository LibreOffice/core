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

#include "dcontact.hxx"
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
    // #i68520#
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
    memset( pSdrObj, 0, sizeof(pSdrObj) );
    memset( pTextRanger, 0, sizeof(pTextRanger) );
}

SwContourCache::~SwContourCache()
{
    for( sal_uInt16 i = 0; i < nObjCnt; delete pTextRanger[ i++ ] )
        ;
}

void SwContourCache::ClrObject( sal_uInt16 nPos )
{
    assert(pTextRanger[nPos] && "ClrObject: Already cleared. Good Bye!");
    nPntCnt -= pTextRanger[ nPos ]->GetPointCount();
    delete pTextRanger[ nPos ];
    --nObjCnt;
    memmove( const_cast<SdrObject**>(pSdrObj) + nPos, pSdrObj + nPos + 1,
             ( nObjCnt - nPos ) * sizeof( SdrObject* ) );
    memmove( pTextRanger + nPos, pTextRanger + nPos + 1,
             ( nObjCnt - nPos ) * sizeof( TextRanger* ) );
}

void ClrContourCache( const SdrObject *pObj )
{
    if( pContourCache && pObj )
        for( sal_uInt16 i = 0; i < pContourCache->GetCount(); ++i )
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
        for( sal_uInt16 i = 0; i < pContourCache->GetCount();
             delete pContourCache->pTextRanger[ i++ ] )
             ;
        pContourCache->nObjCnt = 0;
        pContourCache->nPntCnt = 0;
    }
}

// #i68520#
const SwRect SwContourCache::CalcBoundRect( const SwAnchoredObject* pAnchoredObj,
                                            const SwRect &rLine,
                                            const SwTextFrame* pFrame,
                                            const long nXPos,
                                            const bool bRight )
{
    SwRect aRet;
    const SwFrameFormat* pFormat = &(pAnchoredObj->GetFrameFormat());
    if( pFormat->GetSurround().IsContour() &&
        ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) ==  nullptr ||
          ( static_cast<const SwFlyFrame*>(pAnchoredObj)->Lower() &&
            static_cast<const SwFlyFrame*>(pAnchoredObj)->Lower()->IsNoTextFrame() ) ) )
    {
        aRet = pAnchoredObj->GetObjRectWithSpaces();
        if( aRet.IsOver( rLine ) )
        {
            if( !pContourCache )
                pContourCache = new SwContourCache;

            aRet = pContourCache->ContourRect(
                    pFormat, pAnchoredObj->GetDrawObj(), pFrame, rLine, nXPos, bRight );
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

const SwRect SwContourCache::ContourRect( const SwFormat* pFormat,
    const SdrObject* pObj, const SwTextFrame* pFrame, const SwRect &rLine,
    const long nXPos, const bool bRight )
{
    SwRect aRet;
    sal_uInt16 nPos = 0; // Search in the Cache
    while( nPos < GetCount() && pObj != pSdrObj[ nPos ] )
        ++nPos;
    if( GetCount() == nPos ) // Not found
    {
        if( nObjCnt == POLY_CNT )
        {
            nPntCnt -= pTextRanger[ --nObjCnt ]->GetPointCount();
            delete pTextRanger[ nObjCnt ];
        }
        ::basegfx::B2DPolyPolygon aPolyPolygon;
        ::basegfx::B2DPolyPolygon* pPolyPolygon = nullptr;

        if ( dynamic_cast< const SwVirtFlyDrawObj *>( pObj ) !=  nullptr )
        {
            // GetContour() causes the graphic to be loaded, which may cause
            // the graphic to change its size, call ClrObject()
            tools::PolyPolygon aPoly;
            if( !static_cast<const SwVirtFlyDrawObj*>(pObj)->GetFlyFrame()->GetContour( aPoly ) )
                aPoly = tools::PolyPolygon( static_cast<const SwVirtFlyDrawObj*>(pObj)->
                                     GetFlyFrame()->Frame().SVRect() );
            aPolyPolygon.clear();
            aPolyPolygon.append(aPoly.getB2DPolyPolygon());
        }
        else
        {
            if( dynamic_cast< const E3dObject *>( pObj ) ==  nullptr )
            {
                aPolyPolygon = pObj->TakeXorPoly();
            }

            ::basegfx::B2DPolyPolygon aContourPoly(pObj->TakeContour());
            pPolyPolygon = new ::basegfx::B2DPolyPolygon(aContourPoly);
        }
        const SvxLRSpaceItem &rLRSpace = pFormat->GetLRSpace();
        const SvxULSpaceItem &rULSpace = pFormat->GetULSpace();
        memmove( pTextRanger + 1, pTextRanger, nObjCnt * sizeof( TextRanger* ) );
        memmove( const_cast<SdrObject**>(pSdrObj) + 1, pSdrObj, nObjCnt++ * sizeof( SdrObject* ) );
        pSdrObj[ 0 ] = pObj; // due to #37347 the Object must be entered only
                             // after GetContour()
        pTextRanger[ 0 ] = new TextRanger( aPolyPolygon, pPolyPolygon, 20,
            (sal_uInt16)rLRSpace.GetLeft(), (sal_uInt16)rLRSpace.GetRight(),
            pFormat->GetSurround().IsOutside(), false, pFrame->IsVertical() );
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
        memmove( const_cast<SdrObject**>(pSdrObj) + 1, pSdrObj, nPos * sizeof( SdrObject* ) );
        memmove( pTextRanger + 1, pTextRanger, nPos * sizeof( TextRanger* ) );
        pSdrObj[ 0 ] = pTmpObj;
        pTextRanger[ 0 ] = pTmpRanger;
    }
    SWRECTFN( pFrame )
    long nTmpTop = (rLine.*fnRect->fnGetTop)();
    // fnGetBottom is top + height
    long nTmpBottom = (rLine.*fnRect->fnGetBottom)();

    Range aRange( std::min( nTmpTop, nTmpBottom ), std::max( nTmpTop, nTmpBottom ) );

    LongDqPtr pTmp = pTextRanger[ 0 ]->GetTextRanges( aRange );

    const size_t nCount = pTmp->size();
    if( 0 != nCount )
    {
        size_t nIdx = 0;
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

SwTextFly::SwTextFly()
    : pPage(nullptr)
    , mpCurrAnchoredObj(nullptr)
    , pCurrFrame(nullptr)
    , pMaster(nullptr)
    , mpAnchoredObjList(nullptr)
    , nMinBottom(0)
    , nNextTop(0)
    , nIndex(0)
    , bOn(false)
    , bTopRule(false)
    , mbIgnoreCurrentFrame(false)
    , mbIgnoreContour(false)
    , mbIgnoreObjsInHeaderFooter(false)

{
}

SwTextFly::SwTextFly( const SwTextFrame *pFrame )
{
    CtorInitTextFly( pFrame );
}

SwTextFly::SwTextFly( const SwTextFly& rTextFly )
{
    pPage = rTextFly.pPage;
    mpCurrAnchoredObj = rTextFly.mpCurrAnchoredObj;
    pCurrFrame = rTextFly.pCurrFrame;
    pMaster = rTextFly.pMaster;
    if( rTextFly.mpAnchoredObjList )
    {
        mpAnchoredObjList = new SwAnchoredObjList( *(rTextFly.mpAnchoredObjList) );
    }
    else
    {
        mpAnchoredObjList = nullptr;
    }

    bOn = rTextFly.bOn;
    bTopRule = rTextFly.bTopRule;
    nMinBottom = rTextFly.nMinBottom;
    nNextTop = rTextFly.nNextTop;
    nIndex = rTextFly.nIndex;
    mbIgnoreCurrentFrame = rTextFly.mbIgnoreCurrentFrame;
    mbIgnoreContour = rTextFly.mbIgnoreContour;
    mbIgnoreObjsInHeaderFooter = rTextFly.mbIgnoreObjsInHeaderFooter;
}

SwTextFly::~SwTextFly()
{
    delete mpAnchoredObjList;
}

void SwTextFly::CtorInitTextFly( const SwTextFrame *pFrame )
{
    mbIgnoreCurrentFrame = false;
    mbIgnoreContour = false;
    mbIgnoreObjsInHeaderFooter = false;
    pPage = pFrame->FindPageFrame();
    const SwFlyFrame* pTmp = pFrame->FindFlyFrame();
    // #i68520#
    mpCurrAnchoredObj = pTmp;
    pCurrFrame = pFrame;
    pMaster = pCurrFrame->IsFollow() ? nullptr : pCurrFrame;
    // #i68520#
    mpAnchoredObjList = nullptr;
    // If we're not overlapped by a frame or if a FlyCollection does not exist
    // at all, we switch off forever.
    // It could be, however, that a line is added while formatting, that
    // extends into a frame.
    // That's why we do not optimize for: bOn = pSortedFlys && IsAnyFrame();
    bOn = pPage->GetSortedObjs() != nullptr;
    bTopRule = true;
    nMinBottom = 0;
    nNextTop = 0;
    nIndex = ULONG_MAX;
}

SwRect SwTextFly::_GetFrame( const SwRect &rRect, bool bTop ) const
{
    SwRect aRet;
    if( ForEach( rRect, &aRet, true ) )
    {
        SWRECTFN( pCurrFrame )
        if( bTop )
            (aRet.*fnRect->fnSetTop)( (rRect.*fnRect->fnGetTop)() );

        // Do not always adapt the bottom
        const SwTwips nRetBottom = (aRet.*fnRect->fnGetBottom)();
        const SwTwips nRectBottom = (rRect.*fnRect->fnGetBottom)();
        if ( (*fnRect->fnYDiff)( nRetBottom, nRectBottom ) > 0 ||
             (aRet.*fnRect->fnGetHeight)() < 0 )
            (aRet.*fnRect->fnSetBottom)( nRectBottom );
    }
    return aRet;
}

bool SwTextFly::IsAnyFrame() const
{
    SwSwapIfSwapped swap(const_cast<SwTextFrame *>(pCurrFrame));

    OSL_ENSURE( bOn, "IsAnyFrame: Why?" );
    SwRect aRect( pCurrFrame->Frame().Pos() + pCurrFrame->Prt().Pos(),
        pCurrFrame->Prt().SSize() );

    return ForEach( aRect, nullptr, false );
}

bool SwTextFly::IsAnyObj( const SwRect &rRect ) const
{
   OSL_ENSURE( bOn, "SwTextFly::IsAnyObj: Who's knocking?" );

    SwRect aRect( rRect );
    if ( aRect.IsEmpty() )
        aRect = SwRect( pCurrFrame->Frame().Pos() + pCurrFrame->Prt().Pos(),
                        pCurrFrame->Prt().SSize() );

    const SwSortedObjs *pSorted = pPage->GetSortedObjs();
    if( pSorted ) // bOn actually makes sure that we have objects on the side,
                  // but who knows who deleted somehting in the meantime?
    {
        for ( size_t i = 0; i < pSorted->size(); ++i )
        {
            const SwAnchoredObject* pObj = (*pSorted)[i];

            const SwRect aBound( pObj->GetObjRectWithSpaces() );

            // Optimization
            if( pObj->GetObjRect().Left() > aRect.Right() )
                continue;

            // #i68520#
            if( mpCurrAnchoredObj != pObj && aBound.IsOver( aRect ) )
                return true;
        }
    }
    return false;
}

const SwContentFrame* SwTextFly::_GetMaster()
{
    pMaster = pCurrFrame;
    while( pMaster && pMaster->IsFollow() )
        pMaster = static_cast<SwContentFrame*>(pMaster->FindMaster());
    return pMaster;
}

bool SwTextFly::DrawTextOpaque( SwDrawTextInfo &rInf )
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
    // #i68520#
    const sal_uInt32 nCurrOrd = mpCurrAnchoredObj
                            ? mpCurrAnchoredObj->GetDrawObj()->GetOrdNum()
                            : SAL_MAX_UINT32;
    OSL_ENSURE( !bTopRule, "DrawTextOpaque: Wrong TopRule" );

    // #i68520#
    SwAnchoredObjList::size_type nCount( bOn ? GetAnchoredObjList()->size() : 0 );
    if ( bOn && nCount > 0 )
    {
        const sal_uInt16 nHellId = pPage->getRootFrame()->GetCurrShell()->getIDocumentDrawModelAccess().GetHellId();
        for( SwAnchoredObjList::size_type i = 0; i < nCount; ++i )
        {
            // #i68520#
            const SwAnchoredObject* pTmpAnchoredObj = (*mpAnchoredObjList)[i];
            if( dynamic_cast<const SwFlyFrame*>(pTmpAnchoredObj) &&
                mpCurrAnchoredObj != pTmpAnchoredObj )
            {
                // #i68520#
                const SwFlyFrame& rFly = dynamic_cast<const SwFlyFrame&>(*pTmpAnchoredObj);
                if( aRegion.GetOrigin().IsOver( rFly.Frame() ) )
                {
                    const SwFrameFormat *pFormat = rFly.GetFormat();
                    const SwFormatSurround &rSur = pFormat->GetSurround();
                    const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
                    // Only the ones who are opaque and more to the top
                    if( ! rFly.IsBackgroundTransparent() &&
                        SURROUND_THROUGHT == rSur.GetSurround() &&
                        ( !rSur.IsAnchorOnly() ||
                          // #i68520#
                          GetMaster() == rFly.GetAnchorFrame() ||
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
                        const SwNoTextFrame *pNoText =
                                rFly.Lower() && rFly.Lower()->IsNoTextFrame()
                                                   ? static_cast<const SwNoTextFrame*>(rFly.Lower())
                                                   : nullptr;
                        if ( !pNoText ||
                             (!pNoText->IsTransparent() && !rSur.IsContour()) )
                        {
                            bOpaque = true;
                            aRegion -= rFly.Frame();
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
        // What a huge effort ...
        SwSaveClip aClipVout( rInf.GetpOut() );
        for( size_t i = 0; i < aRegion.size(); ++i )
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

void SwTextFly::DrawFlyRect( OutputDevice* pOut, const SwRect &rRect,
        const SwTextPaintInfo &rInf, bool bNoGraphic )
{
    SwRegionRects aRegion( rRect );
    OSL_ENSURE( !bTopRule, "DrawFlyRect: Wrong TopRule" );
    // #i68520#
    SwAnchoredObjList::size_type nCount( bOn ? GetAnchoredObjList()->size() : 0 );
    if ( bOn && nCount > 0 )
    {
        const sal_uInt16 nHellId = pPage->getRootFrame()->GetCurrShell()->getIDocumentDrawModelAccess().GetHellId();
        for( SwAnchoredObjList::size_type i = 0; i < nCount; ++i )
        {
            // #i68520#
            const SwAnchoredObject* pAnchoredObjTmp = (*mpAnchoredObjList)[i];
            if (mpCurrAnchoredObj == pAnchoredObjTmp)
                continue;

            // #i68520#
            const SwFlyFrame* pFly = dynamic_cast<const SwFlyFrame*>(pAnchoredObjTmp);
            if (pFly)
            {
                // #i68520#
                const SwFormatSurround& rSur = pAnchoredObjTmp->GetFrameFormat().GetSurround();

                // OD 24.01.2003 #106593# - correct clipping of fly frame area.
                // Consider that fly frame background/shadow can be transparent
                // and <SwAlignRect(..)> fly frame area
                // #i47804# - consider transparent graphics
                // and OLE objects.
                bool bClipFlyArea =
                        ( ( SURROUND_THROUGHT == rSur.GetSurround() )
                          // #i68520#
                          ? (pAnchoredObjTmp->GetDrawObj()->GetLayer() != nHellId)
                          : !rSur.IsContour() ) &&
                        !pFly->IsBackgroundTransparent() &&
                        ( !pFly->Lower() ||
                          !pFly->Lower()->IsNoTextFrame() ||
                          !static_cast<const SwNoTextFrame*>(pFly->Lower())->IsTransparent() );
                if ( bClipFlyArea )
                {
                    // #i68520#
                    SwRect aFly( pAnchoredObjTmp->GetObjRect() );
                    // OD 24.01.2003 #106593#
                    ::SwAlignRect( aFly, pPage->getRootFrame()->GetCurrShell(), pOut );
                    if( aFly.Width() > 0 && aFly.Height() > 0 )
                        aRegion -= aFly;
                }
            }
        }
    }

    for( size_t i = 0; i < aRegion.size(); ++i )
    {
        if ( bNoGraphic )
        {
            pOut->DrawRect( aRegion[i].SVRect() );
        }
        else
        {
            if(reinterpret_cast<SvxBrushItem*>(-1) != rInf.GetBrushItem())
            {
                ::DrawGraphic(rInf.GetBrushItem(), pOut, rInf.GetBrushRect(), aRegion[i] );
            }
            else
            {
                OSL_ENSURE(false, "DrawRect: Uninitialized BrushItem!" );
            }
        }
    }
}

/**
 * #i26945# - change first parameter
 * Now it's the <SwAnchoredObject> instance of the floating screen object
 */
bool SwTextFly::GetTop( const SwAnchoredObject* _pAnchoredObj,
                       const bool bInFootnote,
                       const bool bInFooterOrHeader )
{
    // #i68520#
    // <mpCurrAnchoredObj> is set, if <pCurrFrame> is inside a fly frame
    if( _pAnchoredObj != mpCurrAnchoredObj )
    {
        // #i26945#
        const SdrObject* pNew = _pAnchoredObj->GetDrawObj();
        // #102344# Ignore connectors which have one or more connections
        if (const SdrEdgeObj* pEdgeObj = dynamic_cast<const SdrEdgeObj*>(pNew))
        {
            if (pEdgeObj->GetConnectedNode(true) || pEdgeObj->GetConnectedNode(false))
            {
                return false;
            }
        }

        if( ( bInFootnote || bInFooterOrHeader ) && bTopRule )
        {
            // #i26945#
            const SwFrameFormat& rFrameFormat = _pAnchoredObj->GetFrameFormat();
            const SwFormatAnchor& rNewA = rFrameFormat.GetAnchor();
            if (FLY_AT_PAGE == rNewA.GetAnchorId())
            {
                if ( bInFootnote )
                    return false;

                if ( bInFooterOrHeader )
                {
                    SwFormatVertOrient aVert( rFrameFormat.GetVertOrient() );
                    bool bVertPrt = aVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA ||
                            aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA;
                    if( bVertPrt )
                        return false;
                }
            }
        }

        // #i68520#
        // bEvade: consider pNew, if we are not inside a fly
        //         consider pNew, if pNew is lower of <mpCurrAnchoredObj>
        bool bEvade = !mpCurrAnchoredObj ||
                          Is_Lower_Of( dynamic_cast<const SwFlyFrame*>(mpCurrAnchoredObj), pNew);

        if ( !bEvade )
        {
            // We are currently inside a fly frame and pNew is not
            // inside this fly frame. We can do some more checks if
            // we have to consider pNew.

            // If bTopRule is not set, we ignore the frame types.
            // We directly check the z-order
            if ( !bTopRule )
                bEvade = true;
            else
            {
                // Within chained Flys we only avoid Lower
                // #i68520#
                const SwFormatChain &rChain = mpCurrAnchoredObj->GetFrameFormat().GetChain();
                if ( !rChain.GetPrev() && !rChain.GetNext() )
                {
                    // #i26945#
                    const SwFormatAnchor& rNewA = _pAnchoredObj->GetFrameFormat().GetAnchor();
                    // #i68520#
                    const SwFormatAnchor& rCurrA = mpCurrAnchoredObj->GetFrameFormat().GetAnchor();

                    // If <mpCurrAnchoredObj> is anchored as character, its content
                    // does not wrap around pNew
                    if (FLY_AS_CHAR == rCurrA.GetAnchorId())
                        return false;

                    // If pNew is anchored to page and <mpCurrAnchoredObj is not anchored
                    // to page, the content of <mpCurrAnchoredObj> does not wrap around pNew
                    // If both pNew and <mpCurrAnchoredObj> are anchored to page, we can do
                    // some more checks
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
                        return false; // Page anchored ones only avoid page anchored ones
                    else if (FLY_AT_FLY == rNewA.GetAnchorId())
                        bEvade = true; // Non-page anchored ones avoid frame anchored ones
                    else if( FLY_AT_FLY == rCurrA.GetAnchorId() )
                        return false; // Frame anchored ones do not avoid paragraph anchored ones
                    // #i57062#
                    // In order to avoid loop situation, it's decided to adjust
                    // the wrapping behaviour of content of at-paragraph/at-character
                    // anchored objects to one in the page header/footer and
                    // the document body --> content of at-paragraph/at-character
                    // anchored objects doesn't wrap around each other.
                    else
                        return false;
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
                    bEvade = false;
            }
        }

        if ( bEvade )
        {
            // #i26945#
            const SwFormatAnchor& rNewA = _pAnchoredObj->GetFrameFormat().GetAnchor();
            OSL_ENSURE( FLY_AS_CHAR != rNewA.GetAnchorId(),
                    "Don't call GetTop with a FlyInContentFrame" );
            if (FLY_AT_PAGE == rNewA.GetAnchorId())
                return true;  // We always avoid page anchored ones

            // If Flys anchored at paragraph are caught in a FlyCnt, then
            // their influence ends at the borders of the FlyCnt!
            // If we are currently formatting the text of the FlyCnt, then
            // it has to get out of the way of the Frame anchored at paragraph!
            // pCurrFrame is the anchor of pNew?
            // #i26945#
            const SwFrame* pTmp = _pAnchoredObj->GetAnchorFrame();
            if( pTmp == pCurrFrame )
                return true;
            if( pTmp->IsTextFrame() && ( pTmp->IsInFly() || pTmp->IsInFootnote() ) )
            {
                // #i26945#
                Point aPos = _pAnchoredObj->GetObjRect().Pos();
                pTmp = GetVirtualUpper( pTmp, aPos );
            }
            // #i26945#
            // If <pTmp> is a text frame inside a table, take the upper
            // of the anchor frame, which contains the anchor position.
            else if ( pTmp->IsTextFrame() && pTmp->IsInTab() )
            {
                pTmp = const_cast<SwAnchoredObject*>(_pAnchoredObj)
                                ->GetAnchorFrameContainingAnchPos()->GetUpper();
            }
            // #i28701# - consider all objects in same context,
            // if wrapping style is considered on object positioning.
            // Thus, text will wrap around negative positioned objects.
            // #i3317# - remove condition on checking,
            // if wrappings style is considered on object positioning.
            // Thus, text is wrapping around negative positioned objects.
            // #i35640# - no consideration of negative
            // positioned objects, if wrapping style isn't considered on
            // object position and former text wrapping is applied.
            // This condition is typically for documents imported from the
            // OpenOffice.org file format.
            const IDocumentSettingAccess* pIDSA = pCurrFrame->GetTextNode()->getIDocumentSettingAccess();
            if ( (  pIDSA->get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) ||
                   !pIDSA->get(DocumentSettingId::USE_FORMER_TEXT_WRAPPING) ) &&
                 ::FindKontext( pTmp, 0 ) == ::FindKontext( pCurrFrame, 0 ) )
            {
                return true;
            }

            const SwFrame* pHeader = nullptr;
            if ( pCurrFrame->GetNext() != pTmp &&
                 ( IsFrameInSameKontext( pTmp, pCurrFrame ) ||
                   // #i13832#, #i24135# wrap around objects in page header
                   ( !pIDSA->get(DocumentSettingId::USE_FORMER_TEXT_WRAPPING) &&
                     nullptr != ( pHeader = pTmp->FindFooterOrHeader() ) &&
                     !pHeader->IsFooterFrame() &&
                     pCurrFrame->IsInDocBody() ) ) )
            {
                if( pHeader || FLY_AT_FLY == rNewA.GetAnchorId() )
                    return true;

                // Compare indices:
                // The Index of the other is retrieved from the anchor attr.
                sal_uLong nTmpIndex = rNewA.GetContentAnchor()->nNode.GetIndex();
                // Now check whether the current paragraph is before the anchor
                // of the displaced object in the text, then we don't have to
                // get out of its way.
                // If possible determine Index via SwFormatAnchor because
                // otherwise it's quite expensive.
                if( ULONG_MAX == nIndex )
                    nIndex = pCurrFrame->GetNode()->GetIndex();

                if( nIndex >= nTmpIndex )
                    return true;
            }
        }
    }
    return false;
}

// #i68520#
SwAnchoredObjList* SwTextFly::InitAnchoredObjList()
{
    OSL_ENSURE( pCurrFrame, "InitFlyList: No Frame, no FlyList" );
    // #i68520#
    OSL_ENSURE( !mpAnchoredObjList, "InitFlyList: FlyList already initialized" );

    SwSwapIfSwapped swap(const_cast<SwTextFrame *>(pCurrFrame));

    const SwSortedObjs *pSorted = pPage->GetSortedObjs();
    const size_t nCount = pSorted ? pSorted->size() : 0;
    // --> #108724# Page header/footer content doesn't have to wrap around
    //              floating screen objects
    const bool bFooterHeader = nullptr != pCurrFrame->FindFooterOrHeader();
    const IDocumentSettingAccess* pIDSA = pCurrFrame->GetTextNode()->getIDocumentSettingAccess();
    // #i40155# - check, if frame is marked not to wrap
    const bool bWrapAllowed = ( pIDSA->get(DocumentSettingId::USE_FORMER_TEXT_WRAPPING) ||
                                    ( !pCurrFrame->IsInFootnote() && !bFooterHeader ) );

    bOn = false;

    if( nCount && bWrapAllowed )
    {
        // #i68520#
        mpAnchoredObjList = new SwAnchoredObjList();

        // #i28701# - consider complete frame area for new
        // text wrapping
        SwRect aRect;
        if ( pIDSA->get(DocumentSettingId::USE_FORMER_TEXT_WRAPPING) )
        {
            aRect = pCurrFrame->Prt();
            aRect += pCurrFrame->Frame().Pos();
        }
        else
        {
            aRect = pCurrFrame->Frame();
        }
        // Make ourselves a little smaller than we are,
        // so that 1-Twip-overlappings are ignored (#49532)
        SWRECTFN( pCurrFrame )
        const long nRight = (aRect.*fnRect->fnGetRight)() - 1;
        const long nLeft = (aRect.*fnRect->fnGetLeft)() + 1;
        const bool bR2L = pCurrFrame->IsRightToLeft();

        const IDocumentDrawModelAccess& rIDDMA = pCurrFrame->GetTextNode()->getIDocumentDrawModelAccess();

        for( size_t i = 0; i < nCount; ++i )
        {
            // #i68520#
            // do not consider hidden objects
            // check, if object has to be considered for text wrap
            // #118809# - If requested, do not consider
            // objects in page header|footer for text frames not in page
            // header|footer. This is requested for the calculation of
            // the base offset for objects <SwTextFrame::CalcBaseOfstForFly()>
            // #i20505# Do not consider oversized objects
            SwAnchoredObject* pAnchoredObj = (*pSorted)[ i ];
            assert(pAnchoredObj);
            if ( !pAnchoredObj ||
                 !rIDDMA.IsVisibleLayerId( pAnchoredObj->GetDrawObj()->GetLayer() ) ||
                 !pAnchoredObj->ConsiderForTextWrap() ||
                 ( mbIgnoreObjsInHeaderFooter && !bFooterHeader &&
                   pAnchoredObj->GetAnchorFrame()->FindFooterOrHeader() ) )
            {
                continue;
            }

            const SwRect aBound( pAnchoredObj->GetObjRectWithSpaces() );
            if ( nRight < (aBound.*fnRect->fnGetLeft)() ||
                 (*fnRect->fnYDiff)( (aRect.*fnRect->fnGetTop)(),
                                     (aBound.*fnRect->fnGetBottom)() ) > 0 ||
                 nLeft > (aBound.*fnRect->fnGetRight)() ||
                 (aBound.*fnRect->fnGetHeight)() >
                                    2 * (pPage->Frame().*fnRect->fnGetHeight)() )
            {
                continue;
            }

            // #i26945# - pass <pAnchoredObj> to method
            // <GetTop(..)> instead of only the <SdrObject> instance of the
            // anchored object
            if ( GetTop( pAnchoredObj, pCurrFrame->IsInFootnote(), bFooterHeader ) )
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

                const SwFormatSurround &rFlyFormat = pAnchoredObj->GetFrameFormat().GetSurround();
                // #i68520#
                if ( rFlyFormat.IsAnchorOnly() &&
                     pAnchoredObj->GetAnchorFrame() == GetMaster() )
                {
                    const SwFormatVertOrient &rTmpFormat =
                                    pAnchoredObj->GetFrameFormat().GetVertOrient();
                    if( text::VertOrientation::BOTTOM != rTmpFormat.GetVertOrient() )
                        nMinBottom = ( bVert && nMinBottom ) ?
                                     std::min( nMinBottom, aBound.Left() ) :
                                     std::max( nMinBottom, (aBound.*fnRect->fnGetBottom)() );
                }

                bOn = true;
            }
        }
        if( nMinBottom )
        {
            SwTwips nMax = (pCurrFrame->GetUpper()->*fnRect->fnGetPrtBottom)();
            if( (*fnRect->fnYDiff)( nMinBottom, nMax ) > 0 )
                nMinBottom = nMax;
        }
    }
    else
    {
        // #i68520#
        mpAnchoredObjList = new SwAnchoredObjList();
    }

    // #i68520#
    return mpAnchoredObjList;
}

SwTwips SwTextFly::CalcMinBottom() const
{
    SwTwips nRet = 0;
    const SwContentFrame *pLclMaster = GetMaster();
    OSL_ENSURE(pLclMaster, "SwTextFly without master");
    const SwSortedObjs *pDrawObj = pLclMaster ? pLclMaster->GetDrawObjs() : nullptr;
    const size_t nCount = pDrawObj ? pDrawObj->size() : 0;
    if( nCount )
    {
        SwTwips nEndOfFrame = pCurrFrame->Frame().Bottom();
        for( size_t i = 0; i < nCount; ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pDrawObj)[ i ];
            const SwFormatSurround &rFlyFormat = pAnchoredObj->GetFrameFormat().GetSurround();
            if( rFlyFormat.IsAnchorOnly() )
            {
                const SwFormatVertOrient &rTmpFormat =
                                    pAnchoredObj->GetFrameFormat().GetVertOrient();
                if( text::VertOrientation::BOTTOM != rTmpFormat.GetVertOrient() )
                {
                    const SwRect aBound( pAnchoredObj->GetObjRectWithSpaces() );
                    if( aBound.Top() < nEndOfFrame )
                        nRet = std::max( nRet, aBound.Bottom() );
                }
            }
        }
        SwTwips nMax = pCurrFrame->GetUpper()->Frame().Top() +
                       pCurrFrame->GetUpper()->Prt().Bottom();
        if( nRet > nMax )
            nRet = nMax;
    }
    return nRet;
}

bool SwTextFly::ForEach( const SwRect &rRect, SwRect* pRect, bool bAvoid ) const
{
    SwSwapIfSwapped swap(const_cast<SwTextFrame *>(pCurrFrame));

    bool bRet = false;
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
            SWRECTFN( pCurrFrame )
            if( (aRect.*fnRect->fnGetLeft)() > (rRect.*fnRect->fnGetRight)() )
                break;
            // #i68520#
            if ( mpCurrAnchoredObj != pAnchoredObj && aRect.IsOver( rRect ) )
            {
                // #i68520#
                const SwFormat* pFormat( &(pAnchoredObj->GetFrameFormat()) );
                const SwFormatSurround &rSur = pFormat->GetSurround();
                if( bAvoid )
                {
                    // If the text flows below, it has no influence on
                    // formatting. In LineIter::DrawText() it is "just"
                    // necessary to cleverly set the ClippingRegions
                    const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
                    if( ( SURROUND_THROUGHT == rSur.GetSurround() &&
                          ( !rSur.IsAnchorOnly() ||
                            // #i68520#
                            GetMaster() == pAnchoredObj->GetAnchorFrame() ||
                            ((FLY_AT_PARA != rAnchor.GetAnchorId()) &&
                             (FLY_AT_CHAR != rAnchor.GetAnchorId())) ) )
                        || aRect.Top() == FAR_AWAY )
                        continue;
                }

                // #i58642#
                // Compare <GetMaster()> instead of <pCurrFrame> with the anchor
                // frame of the anchored object, because a follow frame have
                // to ignore the anchored objects of its master frame.
                // Note: Anchored objects are always registered at the master
                //       frame, exception are as-character anchored objects,
                //       but these aren't handled here.
                // #i68520#
                if ( mbIgnoreCurrentFrame &&
                     GetMaster() == pAnchoredObj->GetAnchorFrame() )
                    continue;

                if( pRect )
                {
                    // #i68520#
                    SwRect aFly = AnchoredObjToRect( pAnchoredObj, rRect );
                    if( aFly.IsEmpty() || !aFly.IsOver( rRect ) )
                        continue;
                    if( !bRet || (
                        ( !pCurrFrame->IsRightToLeft() &&
                          ( (aFly.*fnRect->fnGetLeft)() <
                            (pRect->*fnRect->fnGetLeft)() ) ) ||
                        ( pCurrFrame->IsRightToLeft() &&
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

    return bRet;
}

// #i68520#
SwAnchoredObjList::size_type SwTextFly::GetPos( const SwAnchoredObject* pAnchoredObj ) const
{
    SwAnchoredObjList::size_type nCount = GetAnchoredObjList()->size();
    SwAnchoredObjList::size_type nRet = 0;
    while ( nRet < nCount && pAnchoredObj != (*mpAnchoredObjList)[ nRet ] )
        ++nRet;
    return nRet;
}

// #i68520#
void SwTextFly::CalcRightMargin( SwRect &rFly,
                                SwAnchoredObjList::size_type nFlyPos,
                                const SwRect &rLine ) const
{
    // Usually the right margin is the right margin of the Printarea
    OSL_ENSURE( ! pCurrFrame->IsVertical() || ! pCurrFrame->IsSwapped(),
            "SwTextFly::CalcRightMargin with swapped frame" );
    SWRECTFN( pCurrFrame )
    // #118796# - correct determination of right of printing area
    SwTwips nRight = (pCurrFrame->*fnRect->fnGetPrtRight)();
    SwTwips nFlyRight = (rFly.*fnRect->fnGetRight)();
    SwRect aLine( rLine );
    (aLine.*fnRect->fnSetRight)( nRight );
    (aLine.*fnRect->fnSetLeft)( (rFly.*fnRect->fnGetLeft)() );

    // It is possible that there is another object that is _above_ us
    // and protrudes into the same line.
    // Flys with run-through are invisible for those below, i.e., they
    // are ignored for computing the margins of other Flys.
    // 3301: pNext->Frame().IsOver( rLine ) is necessary
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
                ( pNext, aLine, pCurrFrame, nFlyRight, true ) );
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
                // overrule the FlyFrame
                if( nRight > nFlyRight )
                    nRight = nFlyRight;
                bStop = true;
            }
        }
    }
    (rFly.*fnRect->fnSetRight)( nRight );
}

// #i68520#
void SwTextFly::CalcLeftMargin( SwRect &rFly,
                               SwAnchoredObjList::size_type nFlyPos,
                               const SwRect &rLine ) const
{
    OSL_ENSURE( ! pCurrFrame->IsVertical() || ! pCurrFrame->IsSwapped(),
            "SwTextFly::CalcLeftMargin with swapped frame" );
    SWRECTFN( pCurrFrame )
    // #118796# - correct determination of left of printing area
    SwTwips nLeft = (pCurrFrame->*fnRect->fnGetPrtLeft)();
    const SwTwips nFlyLeft = (rFly.*fnRect->fnGetLeft)();

    if( nLeft > nFlyLeft )
        nLeft = rFly.Left();

    SwRect aLine( rLine );
    (aLine.*fnRect->fnSetLeft)( nLeft );

    // It is possible that there is another object that is _above_ us
    // and protrudes into the same line.
    // Flys with run-through are invisible for those below, i.e., they
    // are ignored for computing the margins of other Flys.
    // 3301: pNext->Frame().IsOver( rLine ) is necessary

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
                ( pNext, aLine, pCurrFrame, nFlyLeft, false ) );

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
SwRect SwTextFly::AnchoredObjToRect( const SwAnchoredObject* pAnchoredObj,
                            const SwRect &rLine ) const
{
    SWRECTFN( pCurrFrame )

    const long nXPos = pCurrFrame->IsRightToLeft() ?
                       rLine.Right() :
                       (rLine.*fnRect->fnGetLeft)();

    SwRect aFly = mbIgnoreContour ?
                  pAnchoredObj->GetObjRectWithSpaces() :
                  SwContourCache::CalcBoundRect( pAnchoredObj, rLine, pCurrFrame,
                                                 nXPos, ! pCurrFrame->IsRightToLeft() );

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

SwSurround SwTextFly::_GetSurroundForTextWrap( const SwAnchoredObject* pAnchoredObj ) const
{
    const SwFrameFormat* pFormat = &(pAnchoredObj->GetFrameFormat());
    const SwFormatSurround &rFlyFormat = pFormat->GetSurround();
    SwSurround eSurroundForTextWrap = rFlyFormat.GetSurround();

    if( rFlyFormat.IsAnchorOnly() && pAnchoredObj->GetAnchorFrame() != GetMaster() )
    {
        const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
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
    if ( pCurrFrame->IsRightToLeft() )
    {
        if ( SURROUND_LEFT == eSurroundForTextWrap )
            eSurroundForTextWrap = SURROUND_RIGHT;
        else if ( SURROUND_RIGHT == eSurroundForTextWrap )
            eSurroundForTextWrap = SURROUND_LEFT;
    }

    // "ideal page wrap":
    if ( SURROUND_IDEAL == eSurroundForTextWrap )
    {
        SWRECTFN( pCurrFrame )
        const long nCurrLeft = (pCurrFrame->*fnRect->fnGetPrtLeft)();
        const long nCurrRight = (pCurrFrame->*fnRect->fnGetPrtRight)();
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
                ->getIDocumentSettingAccess()->get(DocumentSettingId::SURROUND_TEXT_WRAP_SMALL )
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

bool SwTextFly::IsAnyFrame( const SwRect &rLine ) const
{

    SwSwapIfSwapped swap(const_cast<SwTextFrame *>(pCurrFrame));

    OSL_ENSURE( bOn, "IsAnyFrame: Why?" );

    return ForEach( rLine, nullptr, false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
