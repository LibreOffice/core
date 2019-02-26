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


#include <comphelper/string.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <editeng/writingmodeitem.hxx>
#include <svx/sdtfchim.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outliner.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/textchain.hxx>
#include <svx/textchainflow.hxx>
#include <svl/itempool.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/flditem.hxx>
#include <svx/xftouit.hxx>
#include <tools/helpers.hxx>
#include <svx/xflgrit.hxx>
#include <svx/svdpool.hxx>
#include <svx/xflclit.hxx>
#include <svl/style.hxx>
#include <editeng/editeng.hxx>
#include <svl/itemiter.hxx>
#include <sdr/properties/textproperties.hxx>
#include <vcl/metaact.hxx>
#include <svx/sdr/contact/viewcontactoftextobj.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <vcl/virdev.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <sal/log.hxx>

using namespace com::sun::star;

// BaseProperties section
std::unique_ptr<sdr::properties::BaseProperties> SdrTextObj::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::TextProperties>(*this);
}

// DrawContact section
std::unique_ptr<sdr::contact::ViewContact> SdrTextObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfTextObj>(*this);
}

SdrTextObj::SdrTextObj(SdrModel& rSdrModel)
:   SdrAttrObj(rSdrModel),
    pEdtOutl(nullptr),
    eTextKind(OBJ_TEXT)
{
    bTextSizeDirty=false;
    bTextFrame=false;
    bNoShear=false;
    bDisableAutoWidthOnDragging=false;

    mbInEditMode = false;
    mbTextAnimationAllowed = true;
    maTextEditOffset = Point(0, 0);

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = true;
    mbInDownScale = false;
}

SdrTextObj::SdrTextObj(
    SdrModel& rSdrModel,
    const tools::Rectangle& rNewRect)
:   SdrAttrObj(rSdrModel),
    maRect(rNewRect),
    pEdtOutl(nullptr),
    eTextKind(OBJ_TEXT)
{
    bTextSizeDirty=false;
    bTextFrame=false;
    bNoShear=false;
    bDisableAutoWidthOnDragging=false;
    ImpJustifyRect(maRect);

    mbInEditMode = false;
    mbTextAnimationAllowed = true;
    mbInDownScale = false;
    maTextEditOffset = Point(0, 0);

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = true;
}

SdrTextObj::SdrTextObj(
    SdrModel& rSdrModel,
    SdrObjKind eNewTextKind)
:   SdrAttrObj(rSdrModel),
    pEdtOutl(nullptr),
    eTextKind(eNewTextKind)
{
    bTextSizeDirty=false;
    bTextFrame=true;
    bNoShear=true;
    bDisableAutoWidthOnDragging=false;

    mbInEditMode = false;
    mbTextAnimationAllowed = true;
    mbInDownScale = false;
    maTextEditOffset = Point(0, 0);

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = true;
}

SdrTextObj::SdrTextObj(
    SdrModel& rSdrModel,
    SdrObjKind eNewTextKind,
    const tools::Rectangle& rNewRect)
:   SdrAttrObj(rSdrModel),
    maRect(rNewRect),
    pEdtOutl(nullptr),
    eTextKind(eNewTextKind)
{
    bTextSizeDirty=false;
    bTextFrame=true;
    bNoShear=true;
    bDisableAutoWidthOnDragging=false;
    ImpJustifyRect(maRect);

    mbInEditMode = false;
    mbTextAnimationAllowed = true;
    mbInDownScale = false;
    maTextEditOffset = Point(0, 0);

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = true;
}

SdrTextObj::~SdrTextObj()
{
    SdrOutliner& rOutl(getSdrModelFromSdrObject().GetHitTestOutliner());
    if( rOutl.GetTextObj() == this )
        rOutl.SetTextObj( nullptr );
    mpText.reset();
    ImpDeregisterLink();
}

void SdrTextObj::FitFrameToTextSize()
{
    ImpJustifyRect(maRect);

    SdrText* pText = getActiveText();
    if(pText==nullptr || !pText->GetOutlinerParaObject())
        return;

    SdrOutliner& rOutliner=ImpGetDrawOutliner();
    rOutliner.SetPaperSize(Size(maRect.Right()-maRect.Left(),maRect.Bottom()-maRect.Top()));
    rOutliner.SetUpdateMode(true);
    rOutliner.SetText(*pText->GetOutlinerParaObject());
    Size aNewSize(rOutliner.CalcTextSize());
    rOutliner.Clear();
    aNewSize.AdjustWidth( 1 ); // because of possible rounding errors
    aNewSize.AdjustWidth(GetTextLeftDistance()+GetTextRightDistance() );
    aNewSize.AdjustHeight(GetTextUpperDistance()+GetTextLowerDistance() );
    tools::Rectangle aNewRect(maRect);
    aNewRect.SetSize(aNewSize);
    ImpJustifyRect(aNewRect);
    if (aNewRect!=maRect) {
        SetLogicRect(aNewRect);
    }
}

void SdrTextObj::NbcSetText(const OUString& rStr)
{
    SdrOutliner& rOutliner=ImpGetDrawOutliner();
    rOutliner.SetStyleSheet( 0, GetStyleSheet());
    rOutliner.SetUpdateMode(true);
    rOutliner.SetText(rStr,rOutliner.GetParagraph( 0 ));
    std::unique_ptr<OutlinerParaObject> pNewText=rOutliner.CreateParaObject();
    Size aSiz(rOutliner.CalcTextSize());
    rOutliner.Clear();
    NbcSetOutlinerParaObject(std::move(pNewText));
    aTextSize=aSiz;
    bTextSizeDirty=false;
}

void SdrTextObj::SetText(const OUString& rStr)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcSetText(rStr);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrTextObj::NbcSetText(SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat)
{
    SdrOutliner& rOutliner=ImpGetDrawOutliner();
    rOutliner.SetStyleSheet( 0, GetStyleSheet());
    rOutliner.Read(rInput,rBaseURL,eFormat);
    std::unique_ptr<OutlinerParaObject> pNewText=rOutliner.CreateParaObject();
    rOutliner.SetUpdateMode(true);
    Size aSiz(rOutliner.CalcTextSize());
    rOutliner.Clear();
    NbcSetOutlinerParaObject(std::move(pNewText));
    aTextSize=aSiz;
    bTextSizeDirty=false;
}

void SdrTextObj::SetText(SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcSetText(rInput,rBaseURL,eFormat);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

const Size& SdrTextObj::GetTextSize() const
{
    if (bTextSizeDirty)
    {
        Size aSiz;
        SdrText* pText = getActiveText();
        if( pText && pText->GetOutlinerParaObject ())
        {
            SdrOutliner& rOutliner=ImpGetDrawOutliner();
            rOutliner.SetText(*pText->GetOutlinerParaObject());
            rOutliner.SetUpdateMode(true);
            aSiz=rOutliner.CalcTextSize();
            rOutliner.Clear();
        }
        // casting to nonconst twice
        const_cast<SdrTextObj*>(this)->aTextSize=aSiz;
        const_cast<SdrTextObj*>(this)->bTextSizeDirty=false;
    }
    return aTextSize;
}

bool SdrTextObj::IsAutoGrowHeight() const
{
    if(!bTextFrame)
        return false; // AutoGrow only together with TextFrames

    const SfxItemSet& rSet = GetObjectItemSet();
    bool bRet = rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT).GetValue();

    if(bRet)
    {
        SdrTextAniKind eAniKind = rSet.Get(SDRATTR_TEXT_ANIKIND).GetValue();

        if(eAniKind == SdrTextAniKind::Scroll || eAniKind == SdrTextAniKind::Alternate || eAniKind == SdrTextAniKind::Slide)
        {
            SdrTextAniDirection eDirection = rSet.Get(SDRATTR_TEXT_ANIDIRECTION).GetValue();

            if(eDirection == SdrTextAniDirection::Up || eDirection == SdrTextAniDirection::Down)
            {
                bRet = false;
            }
        }
    }
    return bRet;
}

bool SdrTextObj::IsAutoGrowWidth() const
{
    if(!bTextFrame)
        return false; // AutoGrow only together with TextFrames

    const SfxItemSet& rSet = GetObjectItemSet();
    bool bRet = rSet.Get(SDRATTR_TEXT_AUTOGROWWIDTH).GetValue();

    bool bInEditMOde = IsInEditMode();

    if(!bInEditMOde && bRet)
    {
        SdrTextAniKind eAniKind = rSet.Get(SDRATTR_TEXT_ANIKIND).GetValue();

        if(eAniKind == SdrTextAniKind::Scroll || eAniKind == SdrTextAniKind::Alternate || eAniKind == SdrTextAniKind::Slide)
        {
            SdrTextAniDirection eDirection = rSet.Get(SDRATTR_TEXT_ANIDIRECTION).GetValue();

            if(eDirection == SdrTextAniDirection::Left || eDirection == SdrTextAniDirection::Right)
            {
                bRet = false;
            }
        }
    }
    return bRet;
}

SdrTextHorzAdjust SdrTextObj::GetTextHorizontalAdjust() const
{
    return GetTextHorizontalAdjust(GetObjectItemSet());
}

SdrTextHorzAdjust SdrTextObj::GetTextHorizontalAdjust(const SfxItemSet& rSet) const
{
    if(IsContourTextFrame())
        return SDRTEXTHORZADJUST_BLOCK;

    SdrTextHorzAdjust eRet = rSet.Get(SDRATTR_TEXT_HORZADJUST).GetValue();

    bool bInEditMode = IsInEditMode();

    if(!bInEditMode && eRet == SDRTEXTHORZADJUST_BLOCK)
    {
        SdrTextAniKind eAniKind = rSet.Get(SDRATTR_TEXT_ANIKIND).GetValue();

        if(eAniKind == SdrTextAniKind::Scroll || eAniKind == SdrTextAniKind::Alternate || eAniKind == SdrTextAniKind::Slide)
        {
            SdrTextAniDirection eDirection = rSet.Get(SDRATTR_TEXT_ANIDIRECTION).GetValue();

            if(eDirection == SdrTextAniDirection::Left || eDirection == SdrTextAniDirection::Right)
            {
                eRet = SDRTEXTHORZADJUST_LEFT;
            }
        }
    }

    return eRet;
} // defaults: BLOCK (justify) for text frame, CENTER for captions of drawing objects

SdrTextVertAdjust SdrTextObj::GetTextVerticalAdjust() const
{
    return GetTextVerticalAdjust(GetObjectItemSet());
}

SdrTextVertAdjust SdrTextObj::GetTextVerticalAdjust(const SfxItemSet& rSet) const
{
    if(IsContourTextFrame())
        return SDRTEXTVERTADJUST_TOP;

    // Take care for vertical text animation here
    SdrTextVertAdjust eRet = rSet.Get(SDRATTR_TEXT_VERTADJUST).GetValue();
    bool bInEditMode = IsInEditMode();

    // Take care for vertical text animation here
    if(!bInEditMode && eRet == SDRTEXTVERTADJUST_BLOCK)
    {
        SdrTextAniKind eAniKind = rSet.Get(SDRATTR_TEXT_ANIKIND).GetValue();

        if(eAniKind == SdrTextAniKind::Scroll || eAniKind == SdrTextAniKind::Alternate || eAniKind == SdrTextAniKind::Slide)
        {
            SdrTextAniDirection eDirection = rSet.Get(SDRATTR_TEXT_ANIDIRECTION).GetValue();

            if(eDirection == SdrTextAniDirection::Left || eDirection == SdrTextAniDirection::Right)
            {
                eRet = SDRTEXTVERTADJUST_TOP;
            }
        }
    }

    return eRet;
} // defaults: TOP for text frame, CENTER for captions of drawing objects

void SdrTextObj::ImpJustifyRect(tools::Rectangle& rRect)
{
    if (!rRect.IsEmpty()) {
        rRect.Justify();
        if (rRect.Left()==rRect.Right()) rRect.AdjustRight( 1 );
        if (rRect.Top()==rRect.Bottom()) rRect.AdjustBottom( 1 );
    }
}

void SdrTextObj::ImpCheckShear()
{
    if (bNoShear && aGeo.nShearAngle!=0) {
        aGeo.nShearAngle=0;
        aGeo.nTan=0;
    }
}

void SdrTextObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bNoTextFrame=!IsTextFrame();
    rInfo.bResizeFreeAllowed=bNoTextFrame || aGeo.nRotationAngle%9000==0;
    rInfo.bResizePropAllowed=true;
    rInfo.bRotateFreeAllowed=true;
    rInfo.bRotate90Allowed  =true;
    rInfo.bMirrorFreeAllowed=bNoTextFrame;
    rInfo.bMirror45Allowed  =bNoTextFrame;
    rInfo.bMirror90Allowed  =bNoTextFrame;

    // allow transparency
    rInfo.bTransparenceAllowed = true;

    rInfo.bShearAllowed     =bNoTextFrame;
    rInfo.bEdgeRadiusAllowed=true;
    bool bCanConv=ImpCanConvTextToCurve();
    rInfo.bCanConvToPath    =bCanConv;
    rInfo.bCanConvToPoly    =bCanConv;
    rInfo.bCanConvToPathLineToArea=bCanConv;
    rInfo.bCanConvToPolyLineToArea=bCanConv;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrTextObj::GetObjIdentifier() const
{
    return sal_uInt16(eTextKind);
}

bool SdrTextObj::HasTextImpl( SdrOutliner const * pOutliner )
{
    bool bRet=false;
    if(pOutliner)
    {
        Paragraph* p1stPara=pOutliner->GetParagraph( 0 );
        sal_Int32 nParaCount=pOutliner->GetParagraphCount();
        if(p1stPara==nullptr)
            nParaCount=0;

        if(nParaCount==1)
        {
            // if it is only one paragraph, check if that paragraph is empty
            if( pOutliner->GetText(p1stPara).isEmpty() )
                nParaCount = 0;
        }

        bRet= nParaCount!=0;
    }
    return bRet;
}

void SdrTextObj::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    const bool bRemove(pNewPage == nullptr && pOldPage != nullptr);
    const bool bInsert(pNewPage != nullptr && pOldPage == nullptr);
    const bool bLinked(IsLinkedText());

    if (bLinked && bRemove)
    {
        ImpDeregisterLink();
    }

    // call parent
    SdrAttrObj::handlePageChange(pOldPage, pNewPage);

    if (bLinked && bInsert)
    {
        ImpRegisterLink();
    }
}

void SdrTextObj::NbcSetEckenradius(long nRad)
{
    SetObjectItem(makeSdrEckenradiusItem(nRad));
}

// #115391# This implementation is based on the object size (aRect) and the
// states of IsAutoGrowWidth/Height to correctly set TextMinFrameWidth/Height
void SdrTextObj::AdaptTextMinSize()
{
    if (!bTextFrame)
        // Only do this for text frame.
        return;

    if (getSdrModelFromSdrObject().IsPasteResize())
        // Don't do this during paste resize.
        return;

    const bool bW = IsAutoGrowWidth();
    const bool bH = IsAutoGrowHeight();

    if (!bW && !bH)
        // No auto grow requested.  Bail out.
        return;

    SfxItemSet aSet(
        *GetObjectItemSet().GetPool(),
        svl::Items<SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
        SDRATTR_TEXT_MINFRAMEWIDTH, SDRATTR_TEXT_AUTOGROWWIDTH>{}); // contains SDRATTR_TEXT_MAXFRAMEWIDTH

    if(bW)
    {
        // Set minimum width.
        const long nDist = GetTextLeftDistance() + GetTextRightDistance();
        const long nW = std::max<long>(0, maRect.GetWidth() - 1 - nDist); // text width without margins

        aSet.Put(makeSdrTextMinFrameWidthItem(nW));

        if(!IsVerticalWriting() && bDisableAutoWidthOnDragging)
        {
            bDisableAutoWidthOnDragging = true;
            aSet.Put(makeSdrTextAutoGrowWidthItem(false));
        }
    }

    if(bH)
    {
        // Set Minimum height.
        const long nDist = GetTextUpperDistance() + GetTextLowerDistance();
        const long nH = std::max<long>(0, maRect.GetHeight() - 1 - nDist); // text height without margins

        aSet.Put(makeSdrTextMinFrameHeightItem(nH));

        if(IsVerticalWriting() && bDisableAutoWidthOnDragging)
        {
            bDisableAutoWidthOnDragging = false;
            aSet.Put(makeSdrTextAutoGrowHeightItem(false));
        }
    }

    SetObjectItemSet(aSet);
}

void SdrTextObj::ImpSetContourPolygon( SdrOutliner& rOutliner, tools::Rectangle const & rAnchorRect, bool bLineWidth ) const
{
    basegfx::B2DPolyPolygon aXorPolyPolygon(TakeXorPoly());
    std::unique_ptr<basegfx::B2DPolyPolygon> pContourPolyPolygon;
    basegfx::B2DHomMatrix aMatrix(basegfx::utils::createTranslateB2DHomMatrix(
        -rAnchorRect.Left(), -rAnchorRect.Top()));

    if(aGeo.nRotationAngle)
    {
        // Unrotate!
        aMatrix.rotate(-aGeo.nRotationAngle * F_PI18000);
    }

    aXorPolyPolygon.transform(aMatrix);

    if( bLineWidth )
    {
        // Take line width into account.
        // When doing the hit test, avoid this. (Performance!)
        pContourPolyPolygon.reset(new basegfx::B2DPolyPolygon());

        // test if shadow needs to be avoided for TakeContour()
        const SfxItemSet& rSet = GetObjectItemSet();
        bool bShadowOn = rSet.Get(SDRATTR_SHADOW).GetValue();

        // #i33696#
        // Remember TextObject currently set at the DrawOutliner, it WILL be
        // replaced during calculating the outline since it uses an own paint
        // and that one uses the DrawOutliner, too.
        const SdrTextObj* pLastTextObject = rOutliner.GetTextObj();

        if(bShadowOn)
        {
            // force shadow off
            SdrObject* pCopy(CloneSdrObject(getSdrModelFromSdrObject()));
            pCopy->SetMergedItem(makeSdrShadowItem(false));
            *pContourPolyPolygon = pCopy->TakeContour();
            SdrObject::Free( pCopy );
        }
        else
        {
            *pContourPolyPolygon = TakeContour();
        }

        // #i33696#
        // restore remembered text object
        if(pLastTextObject != rOutliner.GetTextObj())
        {
            rOutliner.SetTextObj(pLastTextObject);
        }

        pContourPolyPolygon->transform(aMatrix);
    }

    rOutliner.SetPolygon(aXorPolyPolygon, pContourPolyPolygon.get());
}

void SdrTextObj::TakeUnrotatedSnapRect(tools::Rectangle& rRect) const
{
    rRect=maRect;
}

void SdrTextObj::TakeTextAnchorRect(tools::Rectangle& rAnchorRect) const
{
    long nLeftDist=GetTextLeftDistance();
    long nRightDist=GetTextRightDistance();
    long nUpperDist=GetTextUpperDistance();
    long nLowerDist=GetTextLowerDistance();
    tools::Rectangle aAnkRect(maRect); // the rectangle in which we anchor
    bool bFrame=IsTextFrame();
    if (!bFrame) {
        TakeUnrotatedSnapRect(aAnkRect);
    }
    Point aRotateRef(aAnkRect.TopLeft());
    aAnkRect.AdjustLeft(nLeftDist );
    aAnkRect.AdjustTop(nUpperDist );
    aAnkRect.AdjustRight( -nRightDist );
    aAnkRect.AdjustBottom( -nLowerDist );

    // Since sizes may be bigger than the object bounds it is necessary to
    // justify the rect now.
    ImpJustifyRect(aAnkRect);

    if (bFrame) {
        // TODO: Optimize this.
        if (aAnkRect.GetWidth()<2) aAnkRect.SetRight(aAnkRect.Left()+1 ); // minimum size h and v: 2 px
        if (aAnkRect.GetHeight()<2) aAnkRect.SetBottom(aAnkRect.Top()+1 );
    }
    if (aGeo.nRotationAngle!=0) {
        Point aTmpPt(aAnkRect.TopLeft());
        RotatePoint(aTmpPt,aRotateRef,aGeo.nSin,aGeo.nCos);
        aTmpPt-=aAnkRect.TopLeft();
        aAnkRect.Move(aTmpPt.X(),aTmpPt.Y());
    }
    rAnchorRect=aAnkRect;
}

void SdrTextObj::TakeTextRect( SdrOutliner& rOutliner, tools::Rectangle& rTextRect, bool bNoEditText,
                               tools::Rectangle* pAnchorRect, bool bLineWidth ) const
{
    tools::Rectangle aAnkRect; // the rectangle in which we anchor
    TakeTextAnchorRect(aAnkRect);
    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
    SdrTextAniKind      eAniKind=GetTextAniKind();
    SdrTextAniDirection eAniDirection=GetTextAniDirection();

    bool bFitToSize(IsFitToSize());
    bool bContourFrame=IsContourTextFrame();

    bool bFrame=IsTextFrame();
    EEControlBits nStat0=rOutliner.GetControlWord();
    Size aNullSize;
    if (!bContourFrame)
    {
        rOutliner.SetControlWord(nStat0|EEControlBits::AUTOPAGESIZE);
        rOutliner.SetMinAutoPaperSize(aNullSize);
        rOutliner.SetMaxAutoPaperSize(Size(1000000,1000000));
    }

    if (!bFitToSize && !bContourFrame)
    {
        long nAnkWdt=aAnkRect.GetWidth();
        long nAnkHgt=aAnkRect.GetHeight();
        if (bFrame)
        {
            long nWdt=nAnkWdt;
            long nHgt=nAnkHgt;

            bool bInEditMode = IsInEditMode();

            if (!bInEditMode && (eAniKind==SdrTextAniKind::Scroll || eAniKind==SdrTextAniKind::Alternate || eAniKind==SdrTextAniKind::Slide))
            {
                // unlimited paper size for ticker text
                if (eAniDirection==SdrTextAniDirection::Left || eAniDirection==SdrTextAniDirection::Right) nWdt=1000000;
                if (eAniDirection==SdrTextAniDirection::Up || eAniDirection==SdrTextAniDirection::Down) nHgt=1000000;
            }

            bool bChainedFrame = IsChainable();
            // Might be required for overflow check working: do limit height to frame if box is chainable.
            if (!bChainedFrame) {
                // #i119885# Do not limit/force height to geometrical frame (vice versa for vertical writing)

                if(IsVerticalWriting())
                {
                    nWdt = 1000000;
                }
                else
                {
                    nHgt = 1000000;
                }
            }

            rOutliner.SetMaxAutoPaperSize(Size(nWdt,nHgt));
        }

        // New try with _BLOCK for hor and ver after completely
        // supporting full width for vertical text.
        if(SDRTEXTHORZADJUST_BLOCK == eHAdj && !IsVerticalWriting())
        {
            rOutliner.SetMinAutoPaperSize(Size(nAnkWdt, 0));
        }

        if(SDRTEXTVERTADJUST_BLOCK == eVAdj && IsVerticalWriting())
        {
            rOutliner.SetMinAutoPaperSize(Size(0, nAnkHgt));
        }
    }

    rOutliner.SetPaperSize(aNullSize);
    if (bContourFrame)
        ImpSetContourPolygon( rOutliner, aAnkRect, bLineWidth );

    // put text into the outliner, if available from the edit outliner
    SdrText* pText = getActiveText();
    OutlinerParaObject* pOutlinerParaObject = pText ? pText->GetOutlinerParaObject() : nullptr;
    OutlinerParaObject* pPara = (pEdtOutl && !bNoEditText) ? pEdtOutl->CreateParaObject().release() : pOutlinerParaObject;

    if (pPara)
    {
        const bool bHitTest(&getSdrModelFromSdrObject().GetHitTestOutliner() == &rOutliner);
        const SdrTextObj* pTestObj = rOutliner.GetTextObj();

        if( !pTestObj || !bHitTest || pTestObj != this ||
            pTestObj->GetOutlinerParaObject() != pOutlinerParaObject )
        {
            if( bHitTest ) // #i33696# take back fix #i27510#
            {
                rOutliner.SetTextObj( this );
                rOutliner.SetFixedCellHeight(GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT).GetValue());
            }

            rOutliner.SetUpdateMode(true);
            rOutliner.SetText(*pPara);
        }
    }
    else
    {
        rOutliner.SetTextObj( nullptr );
    }

    if (pEdtOutl && !bNoEditText && pPara)
        delete pPara;

    rOutliner.SetUpdateMode(true);
    rOutliner.SetControlWord(nStat0);

    if( pText )
        pText->CheckPortionInfo(rOutliner);

    Point aTextPos(aAnkRect.TopLeft());
    Size aTextSiz(rOutliner.GetPaperSize()); // GetPaperSize() adds a little tolerance, right?

    // For draw objects containing text correct hor/ver alignment if text is bigger
    // than the object itself. Without that correction, the text would always be
    // formatted to the left edge (or top edge when vertical) of the draw object.
    if(!IsTextFrame())
    {
        if(aAnkRect.GetWidth() < aTextSiz.Width() && !IsVerticalWriting())
        {
            // Horizontal case here. Correct only if eHAdj == SDRTEXTHORZADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
            {
                eHAdj = SDRTEXTHORZADJUST_CENTER;
            }
        }

        if(aAnkRect.GetHeight() < aTextSiz.Height() && IsVerticalWriting())
        {
            // Vertical case here. Correct only if eHAdj == SDRTEXTVERTADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTVERTADJUST_BLOCK == eVAdj)
            {
                eVAdj = SDRTEXTVERTADJUST_CENTER;
            }
        }
    }

    if (eHAdj==SDRTEXTHORZADJUST_CENTER || eHAdj==SDRTEXTHORZADJUST_RIGHT)
    {
        long nFreeWdt=aAnkRect.GetWidth()-aTextSiz.Width();
        if (eHAdj==SDRTEXTHORZADJUST_CENTER)
            aTextPos.AdjustX(nFreeWdt/2 );
        if (eHAdj==SDRTEXTHORZADJUST_RIGHT)
            aTextPos.AdjustX(nFreeWdt );
    }
    if (eVAdj==SDRTEXTVERTADJUST_CENTER || eVAdj==SDRTEXTVERTADJUST_BOTTOM)
    {
        long nFreeHgt=aAnkRect.GetHeight()-aTextSiz.Height();
        if (eVAdj==SDRTEXTVERTADJUST_CENTER)
            aTextPos.AdjustY(nFreeHgt/2 );
        if (eVAdj==SDRTEXTVERTADJUST_BOTTOM)
            aTextPos.AdjustY(nFreeHgt );
    }
    if (aGeo.nRotationAngle!=0)
        RotatePoint(aTextPos,aAnkRect.TopLeft(),aGeo.nSin,aGeo.nCos);

    if (pAnchorRect)
        *pAnchorRect=aAnkRect;

    // rTextRect might not be correct in some cases at ContourFrame
    rTextRect=tools::Rectangle(aTextPos,aTextSiz);
    if (bContourFrame)
        rTextRect=aAnkRect;
}

std::unique_ptr<OutlinerParaObject> SdrTextObj::GetEditOutlinerParaObject() const
{
    std::unique_ptr<OutlinerParaObject> pPara;
    if( HasTextImpl( pEdtOutl ) )
    {
        sal_Int32 nParaCount = pEdtOutl->GetParagraphCount();
        pPara = pEdtOutl->CreateParaObject(0, nParaCount);
    }
    return pPara;
}

void SdrTextObj::ImpSetCharStretching(SdrOutliner& rOutliner, const Size& rTextSize, const Size& rShapeSize, Fraction& rFitXCorrection)
{
    OutputDevice* pOut = rOutliner.GetRefDevice();
    bool bNoStretching(false);

    if(pOut && pOut->GetOutDevType() == OUTDEV_PRINTER)
    {
        // check whether CharStretching is possible at all
        GDIMetaFile* pMtf = pOut->GetConnectMetaFile();
        OUString aTestString(u'J');

        if(pMtf && (!pMtf->IsRecord() || pMtf->IsPause()))
            pMtf = nullptr;

        if(pMtf)
            pMtf->Pause(true);

        vcl::Font aFontMerk(pOut->GetFont());
        vcl::Font aTmpFont( OutputDevice::GetDefaultFont( DefaultFontType::SERIF, LANGUAGE_SYSTEM, GetDefaultFontFlags::OnlyOne ) );

        aTmpFont.SetFontSize(Size(0,100));
        pOut->SetFont(aTmpFont);
        Size aSize1(pOut->GetTextWidth(aTestString), pOut->GetTextHeight());
        aTmpFont.SetFontSize(Size(800,100));
        pOut->SetFont(aTmpFont);
        Size aSize2(pOut->GetTextWidth(aTestString), pOut->GetTextHeight());
        pOut->SetFont(aFontMerk);

        if(pMtf)
            pMtf->Pause(false);

        bNoStretching = (aSize1 == aSize2);

#ifdef _WIN32
        // Windows zooms the font proportionally when using Size(100,500),
        // we don't like that.
        if(aSize2.Height() >= aSize1.Height() * 2)
        {
            bNoStretching = true;
        }
#endif
    }
    unsigned nLoopCount=0;
    bool bNoMoreLoop = false;
    long nXDiff0=0x7FFFFFFF;
    long nWantWdt=rShapeSize.Width();
    long nIsWdt=rTextSize.Width();
    if (nIsWdt==0) nIsWdt=1;

    long nWantHgt=rShapeSize.Height();
    long nIsHgt=rTextSize.Height();
    if (nIsHgt==0) nIsHgt=1;

    long nXTolPl=nWantWdt/100; // tolerance: +1%
    long nXTolMi=nWantWdt/25;  // tolerance: -4%
    long nXCorr =nWantWdt/20;  // correction scale: 5%

    long nX=(nWantWdt*100) /nIsWdt; // calculate X stretching
    long nY=(nWantHgt*100) /nIsHgt; // calculate Y stretching
    bool bChkX = true;
    if (bNoStretching) { // might only be possible proportionally
        if (nX>nY) { nX=nY; bChkX=false; }
        else { nY=nX; }
    }

    while (nLoopCount<5 && !bNoMoreLoop) {
        if (nX<0) nX=-nX;
        if (nX<1) { nX=1; bNoMoreLoop = true; }
        if (nX>65535) { nX=65535; bNoMoreLoop = true; }

        if (nY<0) nY=-nY;
        if (nY<1) { nY=1; bNoMoreLoop = true; }
        if (nY>65535) { nY=65535; bNoMoreLoop = true; }

        // exception, there is no text yet (horizontal case)
        if(nIsWdt <= 1)
        {
            nX = nY;
            bNoMoreLoop = true;
        }

        // exception, there is no text yet (vertical case)
        if(nIsHgt <= 1)
        {
            nY = nX;
            bNoMoreLoop = true;
        }

        rOutliner.SetGlobalCharStretching(static_cast<sal_uInt16>(nX),static_cast<sal_uInt16>(nY));
        nLoopCount++;
        Size aSiz(rOutliner.CalcTextSize());
        long nXDiff=aSiz.Width()-nWantWdt;
        rFitXCorrection=Fraction(nWantWdt,aSiz.Width());
        if (((nXDiff>=nXTolMi || !bChkX) && nXDiff<=nXTolPl) || nXDiff==nXDiff0) {
            bNoMoreLoop = true;
        } else {
            // correct stretching factors
            long nMul=nWantWdt;
            long nDiv=aSiz.Width();
            if (std::abs(nXDiff)<=2*nXCorr) {
                if (nMul>nDiv) nDiv+=(nMul-nDiv)/2; // but only add half of what we calculated,
                else nMul+=(nDiv-nMul)/2;           // because the EditEngine calculates wrongly later on
            }
            nX=nX*nMul/nDiv;
            if (bNoStretching) nY=nX;
        }
        nXDiff0=nXDiff;
    }
}

OUString SdrTextObj::TakeObjNameSingul() const
{
    OUString aStr;

    switch(eTextKind)
    {
        case OBJ_OUTLINETEXT:
        {
            aStr = SvxResId(STR_ObjNameSingulOUTLINETEXT);
            break;
        }

        case OBJ_TITLETEXT  :
        {
            aStr = SvxResId(STR_ObjNameSingulTITLETEXT);
            break;
        }

        default:
        {
            if(IsLinkedText())
                aStr = SvxResId(STR_ObjNameSingulTEXTLNK);
            else
                aStr = SvxResId(STR_ObjNameSingulTEXT);
            break;
        }
    }

    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
    if(pOutlinerParaObject && eTextKind != OBJ_OUTLINETEXT)
    {
        // shouldn't currently cause any problems at OUTLINETEXT
        OUString aStr2(comphelper::string::stripStart(pOutlinerParaObject->GetTextObject().GetText(0), ' '));

        // avoid non expanded text portions in object name
        // (second condition is new)
        if(!aStr2.isEmpty() && aStr2.indexOf(u'\x00FF') == -1)
        {
            // space between ResStr and content text
            aStr += " ";

            aStr += "\'";

            if(aStr2.getLength() > 10)
            {
                aStr2 = aStr2.copy(0, 8);
                aStr2 += "...";
            }

            aStr += aStr2;
            aStr += "\'";
        }
    }

    OUStringBuffer sName(aStr);

    OUString aName(GetName());
    if (!aName.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aName);
        sName.append('\'');
    }

    return sName.makeStringAndClear();
}

OUString SdrTextObj::TakeObjNamePlural() const
{
    OUString sName;
    switch (eTextKind) {
        case OBJ_OUTLINETEXT: sName=SvxResId(STR_ObjNamePluralOUTLINETEXT); break;
        case OBJ_TITLETEXT  : sName=SvxResId(STR_ObjNamePluralTITLETEXT);   break;
        default: {
            if (IsLinkedText()) {
                sName=SvxResId(STR_ObjNamePluralTEXTLNK);
            } else {
                sName=SvxResId(STR_ObjNamePluralTEXT);
            }
        } break;
    } // switch
    return sName;
}

SdrTextObj* SdrTextObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< SdrTextObj >(rTargetModel);
}

SdrTextObj& SdrTextObj::operator=(const SdrTextObj& rObj)
{
    if( this == &rObj )
        return *this;

    // call parent. tdf#116979: use the correct parent class
    SdrAttrObj::operator=(rObj);

    maRect = rObj.maRect;
    aGeo      =rObj.aGeo;
    eTextKind =rObj.eTextKind;
    bTextFrame=rObj.bTextFrame;
    aTextSize=rObj.aTextSize;
    bTextSizeDirty=rObj.bTextSizeDirty;

    // Not all of the necessary parameters were copied yet.
    bNoShear = rObj.bNoShear;
    bDisableAutoWidthOnDragging = rObj.bDisableAutoWidthOnDragging;
    SdrText* pText = getActiveText();

    if( pText && rObj.HasText() )
    {
        // before pNewOutlinerParaObject was created the same, but
        // set at mpText (outside this scope), but mpText might be
        // empty (this operator== seems not prepared for MultiText
        // objects). In the current form it makes only sense to
        // create locally and use locally on a known existing SdrText
        const Outliner* pEO=rObj.pEdtOutl;
        std::unique_ptr<OutlinerParaObject> pNewOutlinerParaObject;

        if (pEO!=nullptr)
        {
            pNewOutlinerParaObject = pEO->CreateParaObject();
        }
        else
        {
            pNewOutlinerParaObject.reset( new OutlinerParaObject(*rObj.getActiveText()->GetOutlinerParaObject()) );
        }

        pText->SetOutlinerParaObject( std::move(pNewOutlinerParaObject) );
    }

    ImpSetTextStyleSheetListeners();
    return *this;
}

basegfx::B2DPolyPolygon SdrTextObj::TakeXorPoly() const
{
    tools::Polygon aPol(maRect);
    if (aGeo.nShearAngle!=0) ShearPoly(aPol,maRect.TopLeft(),aGeo.nTan);
    if (aGeo.nRotationAngle!=0) RotatePoly(aPol,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);

    basegfx::B2DPolyPolygon aRetval;
    aRetval.append(aPol.getB2DPolygon());
    return aRetval;
}

basegfx::B2DPolyPolygon SdrTextObj::TakeContour() const
{
    basegfx::B2DPolyPolygon aRetval(SdrAttrObj::TakeContour());

    // and now add the BoundRect of the text, if necessary
    if ( GetOutlinerParaObject() && !IsFontwork() && !IsContourTextFrame() )
    {
        // using Clone()-Paint() strategy inside TakeContour() leaves a destroyed
        // SdrObject as pointer in DrawOutliner. Set *this again in fetching the outliner
        // in every case
        SdrOutliner& rOutliner=ImpGetDrawOutliner();

        tools::Rectangle aAnchor2;
        tools::Rectangle aR;
        TakeTextRect(rOutliner,aR,false,&aAnchor2);
        rOutliner.Clear();
        bool bFitToSize(IsFitToSize());
        if (bFitToSize) aR=aAnchor2;
        tools::Polygon aPol(aR);
        if (aGeo.nRotationAngle!=0) RotatePoly(aPol,aR.TopLeft(),aGeo.nSin,aGeo.nCos);

        aRetval.append(aPol.getB2DPolygon());
    }

    return aRetval;
}

void SdrTextObj::RecalcSnapRect()
{
    if (aGeo.nRotationAngle!=0 || aGeo.nShearAngle!=0)
    {
        tools::Polygon aPol(maRect);
        if (aGeo.nShearAngle!=0) ShearPoly(aPol,maRect.TopLeft(),aGeo.nTan);
        if (aGeo.nRotationAngle!=0) RotatePoly(aPol,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
        maSnapRect=aPol.GetBoundRect();
    } else {
        maSnapRect = maRect;
    }
}

sal_uInt32 SdrTextObj::GetSnapPointCount() const
{
    return 4L;
}

Point SdrTextObj::GetSnapPoint(sal_uInt32 i) const
{
    Point aP;
    switch (i) {
        case 0: aP=maRect.TopLeft(); break;
        case 1: aP=maRect.TopRight(); break;
        case 2: aP=maRect.BottomLeft(); break;
        case 3: aP=maRect.BottomRight(); break;
        default: aP=maRect.Center(); break;
    }
    if (aGeo.nShearAngle!=0) ShearPoint(aP,maRect.TopLeft(),aGeo.nTan);
    if (aGeo.nRotationAngle!=0) RotatePoint(aP,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    return aP;
}

// Extracted from ImpGetDrawOutliner()
void SdrTextObj::ImpInitDrawOutliner( SdrOutliner& rOutl ) const
{
    rOutl.SetUpdateMode(false);
    OutlinerMode nOutlinerMode = OutlinerMode::OutlineObject;
    if ( !IsOutlText() )
        nOutlinerMode = OutlinerMode::TextObject;
    rOutl.Init( nOutlinerMode );

    rOutl.SetGlobalCharStretching();
    EEControlBits nStat=rOutl.GetControlWord();
    nStat &= ~EEControlBits(EEControlBits::STRETCHING|EEControlBits::AUTOPAGESIZE);
    rOutl.SetControlWord(nStat);
    Size aMaxSize(100000,100000);
    rOutl.SetMinAutoPaperSize(Size());
    rOutl.SetMaxAutoPaperSize(aMaxSize);
    rOutl.SetPaperSize(aMaxSize);
    rOutl.ClearPolygon();
}

SdrOutliner& SdrTextObj::ImpGetDrawOutliner() const
{
    SdrOutliner& rOutl(getSdrModelFromSdrObject().GetDrawOutliner(this));

    // Code extracted to ImpInitDrawOutliner()
    ImpInitDrawOutliner( rOutl );

    return rOutl;
}

// Extracted from Paint()
void SdrTextObj::ImpSetupDrawOutlinerForPaint( bool             bContourFrame,
                                               SdrOutliner&     rOutliner,
                                               tools::Rectangle&       rTextRect,
                                               tools::Rectangle&       rAnchorRect,
                                               tools::Rectangle&       rPaintRect,
                                               Fraction&        rFitXCorrection ) const
{
    if (!bContourFrame)
    {
        // FitToSize can't be used together with ContourFrame for now
        if (IsFitToSize() || IsAutoFit())
        {
            EEControlBits nStat=rOutliner.GetControlWord();
            nStat|=EEControlBits::STRETCHING|EEControlBits::AUTOPAGESIZE;
            rOutliner.SetControlWord(nStat);
        }
    }
    rOutliner.SetFixedCellHeight(GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT).GetValue());
    TakeTextRect(rOutliner, rTextRect, false, &rAnchorRect);
    rPaintRect = rTextRect;

    if (!bContourFrame)
    {
        // FitToSize can't be used together with ContourFrame for now
        if (IsFitToSize())
        {
            ImpSetCharStretching(rOutliner,rTextRect.GetSize(),rAnchorRect.GetSize(),rFitXCorrection);
            rPaintRect=rAnchorRect;
        }
        else if (IsAutoFit())
        {
            ImpAutoFitText(rOutliner);
        }
    }
}

double SdrTextObj::GetFontScaleY() const
{
    SdrText* pText = getActiveText();
    if (pText == nullptr || !pText->GetOutlinerParaObject())
        return 1.0;

    SdrOutliner& rOutliner = ImpGetDrawOutliner();
    const Size aShapeSize = GetSnapRect().GetSize();
    const Size aSize = Size(aShapeSize.Width() - GetTextLeftDistance() - GetTextRightDistance(),
        aShapeSize.Height() - GetTextUpperDistance() - GetTextLowerDistance());

    rOutliner.SetPaperSize(aSize);
    rOutliner.SetUpdateMode(true);
    rOutliner.SetText(*pText->GetOutlinerParaObject());
    bool bIsVerticalWriting = IsVerticalWriting();

    // Algorithm from SdrTextObj::ImpAutoFitText

    sal_uInt16 nMinStretchX = 0, nMinStretchY = 0;
    sal_uInt16 nCurrStretchX = 100, nCurrStretchY = 100;
    sal_uInt16 aOldStretchXVals[] = { 0,0,0 };
    const size_t aStretchArySize = SAL_N_ELEMENTS(aOldStretchXVals);
    for (unsigned int i = 0; i<aStretchArySize; ++i)
    {
        const Size aCurrTextSize = rOutliner.CalcTextSizeNTP();
        double fFactor(1.0);
        if (bIsVerticalWriting)
        {
            if (aCurrTextSize.Width() != 0)
            {
                fFactor = double(aSize.Width()) / aCurrTextSize.Width();
            }
        }
        else if (aCurrTextSize.Height() != 0)
        {
            fFactor = double(aSize.Height()) / aCurrTextSize.Height();
        }
        fFactor = std::sqrt(fFactor);

        rOutliner.GetGlobalCharStretching(nCurrStretchX, nCurrStretchY);

        if (fFactor >= 1.0)
        {
            nMinStretchX = std::max(nMinStretchX, nCurrStretchX);
            nMinStretchY = std::max(nMinStretchY, nCurrStretchY);
        }

        aOldStretchXVals[i] = nCurrStretchX;
        if (std::find(aOldStretchXVals, aOldStretchXVals + i, nCurrStretchX) != aOldStretchXVals + i)
            break; // same value already attained once; algo is looping, exit

        if (fFactor < 1.0 || nCurrStretchX != 100)
        {
            nCurrStretchX = sal::static_int_cast<sal_uInt16>(nCurrStretchX*fFactor);
            nCurrStretchY = sal::static_int_cast<sal_uInt16>(nCurrStretchY*fFactor);
            rOutliner.SetGlobalCharStretching(std::min(sal_uInt16(100), nCurrStretchX),
                std::min(sal_uInt16(100), nCurrStretchY));
        }
    }

    return std::min(sal_uInt16(100), nCurrStretchY) / 100.0;
}

void SdrTextObj::ImpAutoFitText( SdrOutliner& rOutliner ) const
{
    const Size aShapeSize=GetSnapRect().GetSize();
    ImpAutoFitText( rOutliner,
                    Size(aShapeSize.Width()-GetTextLeftDistance()-GetTextRightDistance(),
                         aShapeSize.Height()-GetTextUpperDistance()-GetTextLowerDistance()),
                    IsVerticalWriting() );
}

void SdrTextObj::ImpAutoFitText( SdrOutliner& rOutliner, const Size& rTextSize, bool bIsVerticalWriting )
{
    // EditEngine formatting is unstable enough for
    // line-breaking text that we need some more samples

    // loop early-exits if we detect an already attained value
    sal_uInt16 nMinStretchX=0, nMinStretchY=0;
    sal_uInt16 aOldStretchXVals[]={0,0,0,0,0,0,0,0,0,0};
    const size_t aStretchArySize=SAL_N_ELEMENTS(aOldStretchXVals);
    for(unsigned int i=0; i<aStretchArySize; ++i)
    {
        const Size aCurrTextSize = rOutliner.CalcTextSizeNTP();
        double fFactor(1.0);
        if( bIsVerticalWriting )
        {
            if (aCurrTextSize.Width() != 0)
            {
                fFactor = double(rTextSize.Width())/aCurrTextSize.Width();
            }
        }
        else if (aCurrTextSize.Height() != 0)
        {
            fFactor = double(rTextSize.Height())/aCurrTextSize.Height();
        }
        // fFactor scales in both x and y directions
        // - this is fine for bulleted words
        // - but it scales too much for a long paragraph
        // - taking sqrt scales long paragraphs the best
        // - bulleted words will have to go through more iterations
        fFactor = std::sqrt(fFactor);

        sal_uInt16 nCurrStretchX, nCurrStretchY;
        rOutliner.GetGlobalCharStretching(nCurrStretchX, nCurrStretchY);

        if (fFactor >= 1.0 )
        {
            // resulting text area fits into available shape rect -
            // err on the larger stretching, to optimally fill area
            nMinStretchX = std::max(nMinStretchX,nCurrStretchX);
            nMinStretchY = std::max(nMinStretchY,nCurrStretchY);
        }

        aOldStretchXVals[i] = nCurrStretchX;
        if( std::find(aOldStretchXVals, aOldStretchXVals+i, nCurrStretchX) != aOldStretchXVals+i )
            break; // same value already attained once; algo is looping, exit

        if (fFactor < 1.0 || nCurrStretchX != 100)
        {
            nCurrStretchX = sal::static_int_cast<sal_uInt16>(nCurrStretchX*fFactor);
            nCurrStretchY = sal::static_int_cast<sal_uInt16>(nCurrStretchY*fFactor);
            rOutliner.SetGlobalCharStretching(std::min(sal_uInt16(100),nCurrStretchX),
                                              std::min(sal_uInt16(100),nCurrStretchY));
            SAL_INFO("svx", "zoom is " << nCurrStretchX);
        }
    }

    SAL_INFO("svx", "final zoom is " << nMinStretchX);
    rOutliner.SetGlobalCharStretching(std::min(sal_uInt16(100),nMinStretchX),
                                      std::min(sal_uInt16(100),nMinStretchY));
}

void SdrTextObj::SetupOutlinerFormatting( SdrOutliner& rOutl, tools::Rectangle& rPaintRect ) const
{
    ImpInitDrawOutliner( rOutl );
    UpdateOutlinerFormatting( rOutl, rPaintRect );
}

void SdrTextObj::UpdateOutlinerFormatting( SdrOutliner& rOutl, tools::Rectangle& rPaintRect ) const
{
    tools::Rectangle aTextRect;
    tools::Rectangle aAnchorRect;
    Fraction aFitXCorrection(1,1);

    const bool bContourFrame(IsContourTextFrame());
    const MapMode aMapMode(
        getSdrModelFromSdrObject().GetScaleUnit(),
        Point(0,0),
        getSdrModelFromSdrObject().GetScaleFraction(),
        getSdrModelFromSdrObject().GetScaleFraction());

    rOutl.SetRefMapMode(aMapMode);
    ImpSetupDrawOutlinerForPaint(
        bContourFrame,
        rOutl,
        aTextRect,
        aAnchorRect,
        rPaintRect,
        aFitXCorrection);
}


OutlinerParaObject* SdrTextObj::GetOutlinerParaObject() const
{
    SdrText* pText = getActiveText();
    if( pText )
        return pText->GetOutlinerParaObject();
    else
        return nullptr;
}

void SdrTextObj::NbcSetOutlinerParaObject(std::unique_ptr<OutlinerParaObject> pTextObject)
{
    NbcSetOutlinerParaObjectForText( std::move(pTextObject), getActiveText() );
}

void SdrTextObj::NbcSetOutlinerParaObjectForText( std::unique_ptr<OutlinerParaObject> pTextObject, SdrText* pText )
{
    if( pText )
        pText->SetOutlinerParaObject( std::move(pTextObject) );

    if (pText && pText->GetOutlinerParaObject())
    {
        SvxWritingModeItem aWritingMode(pText->GetOutlinerParaObject()->IsVertical() && pText->GetOutlinerParaObject()->IsTopToBottom()
            ? css::text::WritingMode_TB_RL
            : css::text::WritingMode_LR_TB,
            SDRATTR_TEXTDIRECTION);
        GetProperties().SetObjectItemDirect(aWritingMode);
    }

    SetTextSizeDirty();
    if (IsTextFrame() && (IsAutoGrowHeight() || IsAutoGrowWidth()))
    { // adapt text frame!
        NbcAdjustTextFrameWidthAndHeight();
    }
    if (!IsTextFrame())
    {
        // the SnapRect keeps its size
        SetRectsDirty(true);
    }

    // always invalidate BoundRect on change
    SetBoundRectDirty();
    ActionChanged();

    ImpSetTextStyleSheetListeners();
}

void SdrTextObj::NbcReformatText()
{
    SdrText* pText = getActiveText();
    if( pText && pText->GetOutlinerParaObject() )
    {
        pText->ReformatText();
        if (bTextFrame)
        {
            NbcAdjustTextFrameWidthAndHeight();
        }
        else
        {
            // the SnapRect keeps its size
            SetBoundRectDirty();
            SetRectsDirty(true);
        }
        SetTextSizeDirty();
        ActionChanged();
        // i22396
        // Necessary here since we have no compare operator at the outliner
        // para object which may detect changes regarding the combination
        // of outliner para data and configuration (e.g., change of
        // formatting of text numerals)
        GetViewContact().flushViewObjectContacts(false);
    }
}

SdrObjGeoData* SdrTextObj::NewGeoData() const
{
    return new SdrTextObjGeoData;
}

void SdrTextObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrAttrObj::SaveGeoData(rGeo);
    SdrTextObjGeoData& rTGeo=static_cast<SdrTextObjGeoData&>(rGeo);
    rTGeo.aRect = maRect;
    rTGeo.aGeo   =aGeo;
}

void SdrTextObj::RestGeoData(const SdrObjGeoData& rGeo)
{ // RectsDirty is called by SdrObject
    SdrAttrObj::RestGeoData(rGeo);
    const SdrTextObjGeoData& rTGeo=static_cast<const SdrTextObjGeoData&>(rGeo);
    NbcSetLogicRect(rTGeo.aRect);
    aGeo   =rTGeo.aGeo;
    SetTextSizeDirty();
}

drawing::TextFitToSizeType SdrTextObj::GetFitToSize() const
{
    drawing::TextFitToSizeType eType = drawing::TextFitToSizeType_NONE;

    if(!IsAutoGrowWidth())
        eType = GetObjectItem(SDRATTR_TEXT_FITTOSIZE).GetValue();

    return eType;
}

const tools::Rectangle& SdrTextObj::GetGeoRect() const
{
    return maRect;
}

void SdrTextObj::ForceOutlinerParaObject()
{
    SdrText* pText = getActiveText();
    if( pText && (pText->GetOutlinerParaObject() == nullptr) )
    {
        OutlinerMode nOutlMode = OutlinerMode::TextObject;
        if( IsTextFrame() && eTextKind == OBJ_OUTLINETEXT )
            nOutlMode = OutlinerMode::OutlineObject;

        pText->ForceOutlinerParaObject( nOutlMode );
    }
}

TextChain *SdrTextObj::GetTextChain() const
{
    //if (!IsChainable())
    //    return NULL;

    return getSdrModelFromSdrObject().GetTextChain();
}

bool SdrTextObj::IsVerticalWriting() const
{
    if(pEdtOutl)
    {
        return pEdtOutl->IsVertical();
    }

    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
    if(pOutlinerParaObject)
    {
        return pOutlinerParaObject->IsVertical();
    }

    return false;
}

void SdrTextObj::SetVerticalWriting(bool bVertical)
{
    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();

    if( !pOutlinerParaObject && bVertical )
    {
        // we only need to force a outliner para object if the default of
        // horizontal text is changed
        ForceOutlinerParaObject();
        pOutlinerParaObject = GetOutlinerParaObject();
    }

    if (!pOutlinerParaObject ||
        (pOutlinerParaObject->IsVertical() == bVertical))
        return;

    // get item settings
    const SfxItemSet& rSet = GetObjectItemSet();
    bool bAutoGrowWidth = rSet.Get(SDRATTR_TEXT_AUTOGROWWIDTH).GetValue();
    bool bAutoGrowHeight = rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT).GetValue();

    // Also exchange hor/ver adjust items
    SdrTextHorzAdjust eHorz = rSet.Get(SDRATTR_TEXT_HORZADJUST).GetValue();
    SdrTextVertAdjust eVert = rSet.Get(SDRATTR_TEXT_VERTADJUST).GetValue();

    // rescue object size
    tools::Rectangle aObjectRect = GetSnapRect();

    // prepare ItemSet to set exchanged width and height items
    SfxItemSet aNewSet(*rSet.GetPool(),
        svl::Items<SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
        // Expanded item ranges to also support hor and ver adjust.
        SDRATTR_TEXT_VERTADJUST, SDRATTR_TEXT_VERTADJUST,
        SDRATTR_TEXT_AUTOGROWWIDTH, SDRATTR_TEXT_HORZADJUST>{});

    aNewSet.Put(rSet);
    aNewSet.Put(makeSdrTextAutoGrowWidthItem(bAutoGrowHeight));
    aNewSet.Put(makeSdrTextAutoGrowHeightItem(bAutoGrowWidth));

    // Exchange horz and vert adjusts
    switch (eVert)
    {
        case SDRTEXTVERTADJUST_TOP: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT)); break;
        case SDRTEXTVERTADJUST_CENTER: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER)); break;
        case SDRTEXTVERTADJUST_BOTTOM: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT)); break;
        case SDRTEXTVERTADJUST_BLOCK: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK)); break;
    }
    switch (eHorz)
    {
        case SDRTEXTHORZADJUST_LEFT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BOTTOM)); break;
        case SDRTEXTHORZADJUST_CENTER: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER)); break;
        case SDRTEXTHORZADJUST_RIGHT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP)); break;
        case SDRTEXTHORZADJUST_BLOCK: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BLOCK)); break;
    }

    SetObjectItemSet(aNewSet);

    pOutlinerParaObject = GetOutlinerParaObject();
    if (pOutlinerParaObject)
    {
        // set ParaObject orientation accordingly
        pOutlinerParaObject->SetVertical(bVertical);
    }

    // restore object size
    SetSnapRect(aObjectRect);
}

// transformation interface for StarOfficeAPI. This implements support for
// homogeneous 3x3 matrices containing the transformation of the SdrObject. At the
// moment it contains a shearX, rotation and translation, but for setting all linear
// transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.


// gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
// with the base geometry and returns TRUE. Otherwise it returns FALSE.
bool SdrTextObj::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& /*rPolyPolygon*/) const
{
    // get turn and shear
    double fRotate = basegfx::deg2rad(aGeo.nRotationAngle / 100.0);
    double fShearX = basegfx::deg2rad(aGeo.nShearAngle / 100.0);

    // get aRect, this is the unrotated snaprect
    tools::Rectangle aRectangle(maRect);

    // fill other values
    basegfx::B2DTuple aScale(aRectangle.GetWidth(), aRectangle.GetHeight());
    basegfx::B2DTuple aTranslate(aRectangle.Left(), aRectangle.Top());

    // position maybe relative to anchorpos, convert
    if( getSdrModelFromSdrObject().IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate -= basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // build matrix
    rMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
        aScale,
        basegfx::fTools::equalZero(fShearX) ? 0.0 : tan(fShearX),
        basegfx::fTools::equalZero(fRotate) ? 0.0 : -fRotate,
        aTranslate);

    return false;
}

// sets the base geometry of the object using infos contained in the homogeneous 3x3 matrix.
// If it's an SdrPathObj it will use the provided geometry information. The Polygon has
// to use (0,0) as upper left and will be scaled to the given size in the matrix.
void SdrTextObj::TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& /*rPolyPolygon*/)
{
    // break up matrix
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate(0.0);
    double fShearX(0.0);
    rMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

    // flip?
    bool bFlipX = aScale.getX() < 0.0,
         bFlipY = aScale.getY() < 0.0;
    if (bFlipX)
    {
        aScale.setX(fabs(aScale.getX()));
    }
    if (bFlipY)
    {
        aScale.setY(fabs(aScale.getY()));
    }

    // reset object shear and rotations
    aGeo.nRotationAngle = 0;
    aGeo.RecalcSinCos();
    aGeo.nShearAngle = 0;
    aGeo.RecalcTan();

    // if anchor is used, make position relative to it
    if( getSdrModelFromSdrObject().IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate += basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // build and set BaseRect (use scale)
    Size aSize(FRound(aScale.getX()), FRound(aScale.getY()));
    tools::Rectangle aBaseRect(Point(), aSize);
    SetSnapRect(aBaseRect);

    // flip?
    if (bFlipX)
    {
        Mirror(Point(), Point(0, 1));
    }
    if (bFlipY)
    {
        Mirror(Point(), Point(1, 0));
    }

    // shear?
    if(!basegfx::fTools::equalZero(fShearX))
    {
        GeoStat aGeoStat;
        aGeoStat.nShearAngle = FRound(basegfx::rad2deg(atan(fShearX)) * 100.0);
        aGeoStat.RecalcTan();
        Shear(Point(), aGeoStat.nShearAngle, aGeoStat.nTan, false);
    }

    // rotation?
    if(!basegfx::fTools::equalZero(fRotate))
    {
        GeoStat aGeoStat;

        // #i78696#
        // fRotate is matematically correct, but aGeoStat.nRotationAngle is
        // mirrored -> mirror value here
        aGeoStat.nRotationAngle = NormAngle36000(FRound(-fRotate / F_PI18000));
        aGeoStat.RecalcSinCos();
        Rotate(Point(), aGeoStat.nRotationAngle, aGeoStat.nSin, aGeoStat.nCos);
    }

    // translate?
    if(!aTranslate.equalZero())
    {
        Move(Size(FRound(aTranslate.getX()), FRound(aTranslate.getY())));
    }
}

bool SdrTextObj::IsReallyEdited() const
{
    return pEdtOutl && pEdtOutl->IsModified();
}

// moved inlines here form hxx

long SdrTextObj::GetEckenradius() const
{
    return GetObjectItemSet().Get(SDRATTR_ECKENRADIUS).GetValue();
}

long SdrTextObj::GetMinTextFrameHeight() const
{
    return GetObjectItemSet().Get(SDRATTR_TEXT_MINFRAMEHEIGHT).GetValue();
}

long SdrTextObj::GetMaxTextFrameHeight() const
{
    return GetObjectItemSet().Get(SDRATTR_TEXT_MAXFRAMEHEIGHT).GetValue();
}

long SdrTextObj::GetMinTextFrameWidth() const
{
    return GetObjectItemSet().Get(SDRATTR_TEXT_MINFRAMEWIDTH).GetValue();
}

long SdrTextObj::GetMaxTextFrameWidth() const
{
    return GetObjectItemSet().Get(SDRATTR_TEXT_MAXFRAMEWIDTH).GetValue();
}

bool SdrTextObj::IsFontwork() const
{
    return !bTextFrame // Default is FALSE
        && GetObjectItemSet().Get(XATTR_FORMTXTSTYLE).GetValue() != XFormTextStyle::NONE;
}

bool SdrTextObj::IsHideContour() const
{
    return !bTextFrame // Default is: no, don't HideContour; HideContour not together with TextFrames
        && GetObjectItemSet().Get(XATTR_FORMTXTHIDEFORM).GetValue();
}

bool SdrTextObj::IsContourTextFrame() const
{
    return !bTextFrame // ContourFrame not together with normal TextFrames
        && GetObjectItemSet().Get(SDRATTR_TEXT_CONTOURFRAME).GetValue();
}

long SdrTextObj::GetTextLeftDistance() const
{
    return GetObjectItemSet().Get(SDRATTR_TEXT_LEFTDIST).GetValue();
}

long SdrTextObj::GetTextRightDistance() const
{
    return GetObjectItemSet().Get(SDRATTR_TEXT_RIGHTDIST).GetValue();
}

long SdrTextObj::GetTextUpperDistance() const
{
    return GetObjectItemSet().Get(SDRATTR_TEXT_UPPERDIST).GetValue();
}

long SdrTextObj::GetTextLowerDistance() const
{
    return GetObjectItemSet().Get(SDRATTR_TEXT_LOWERDIST).GetValue();
}

SdrTextAniKind SdrTextObj::GetTextAniKind() const
{
    return GetObjectItemSet().Get(SDRATTR_TEXT_ANIKIND).GetValue();
}

SdrTextAniDirection SdrTextObj::GetTextAniDirection() const
{
    return GetObjectItemSet().Get(SDRATTR_TEXT_ANIDIRECTION).GetValue();
}

// Get necessary data for text scroll animation. ATM base it on a Text-Metafile and a
// painting rectangle. Rotation is excluded from the returned values.
GDIMetaFile* SdrTextObj::GetTextScrollMetaFileAndRectangle(
    tools::Rectangle& rScrollRectangle, tools::Rectangle& rPaintRectangle)
{
    GDIMetaFile* pRetval = nullptr;
    SdrOutliner& rOutliner = ImpGetDrawOutliner();
    tools::Rectangle aTextRect;
    tools::Rectangle aAnchorRect;
    tools::Rectangle aPaintRect;
    Fraction aFitXCorrection(1,1);
    bool bContourFrame(IsContourTextFrame());

    // get outliner set up. To avoid getting a somehow rotated MetaFile,
    // temporarily disable object rotation.
    sal_Int32 nAngle(aGeo.nRotationAngle);
    aGeo.nRotationAngle = 0;
    ImpSetupDrawOutlinerForPaint( bContourFrame, rOutliner, aTextRect, aAnchorRect, aPaintRect, aFitXCorrection );
    aGeo.nRotationAngle = nAngle;

    tools::Rectangle aScrollFrameRect(aPaintRect);
    const SfxItemSet& rSet = GetObjectItemSet();
    SdrTextAniDirection eDirection = rSet.Get(SDRATTR_TEXT_ANIDIRECTION).GetValue();

    if(SdrTextAniDirection::Left == eDirection || SdrTextAniDirection::Right == eDirection)
    {
        aScrollFrameRect.SetLeft( aAnchorRect.Left() );
        aScrollFrameRect.SetRight( aAnchorRect.Right() );
    }

    if(SdrTextAniDirection::Up == eDirection || SdrTextAniDirection::Down == eDirection)
    {
        aScrollFrameRect.SetTop( aAnchorRect.Top() );
        aScrollFrameRect.SetBottom( aAnchorRect.Bottom() );
    }

    // create the MetaFile
    pRetval = new GDIMetaFile;
    ScopedVclPtrInstance< VirtualDevice > pBlackHole;
    pBlackHole->EnableOutput(false);
    pRetval->Record(pBlackHole);
    Point aPaintPos = aPaintRect.TopLeft();

    rOutliner.Draw(pBlackHole, aPaintPos);

    pRetval->Stop();
    pRetval->WindStart();

    // return PaintRectanglePixel and pRetval;
    rScrollRectangle = aScrollFrameRect;
    rPaintRectangle = aPaintRect;

    return pRetval;
}

// Access to TextAnimationAllowed flag
bool SdrTextObj::IsAutoFit() const
{
    return GetFitToSize() == drawing::TextFitToSizeType_AUTOFIT;
}

bool SdrTextObj::IsFitToSize() const
{
    const drawing::TextFitToSizeType eFit = GetFitToSize();
    return (eFit == drawing::TextFitToSizeType_PROPORTIONAL
         || eFit == drawing::TextFitToSizeType_ALLLINES);
}

void SdrTextObj::SetTextAnimationAllowed(bool bNew)
{
    if(mbTextAnimationAllowed != bNew)
    {
        mbTextAnimationAllowed = bNew;
        ActionChanged();
    }
}

/** called from the SdrObjEditView during text edit when the status of the edit outliner changes */
void SdrTextObj::onEditOutlinerStatusEvent( EditStatus* pEditStatus )
{
    const EditStatusFlags nStat = pEditStatus->GetStatusWord();
    const bool bGrowX = bool(nStat & EditStatusFlags::TEXTWIDTHCHANGED);
    const bool bGrowY = bool(nStat & EditStatusFlags::TextHeightChanged);
    if(bTextFrame && (bGrowX || bGrowY))
    {
        if ((bGrowX && IsAutoGrowWidth()) || (bGrowY && IsAutoGrowHeight()))
        {
            AdjustTextFrameWidthAndHeight();
        }
        else if ( (IsAutoFit() || IsFitToSize()) && !mbInDownScale)
        {
            assert(pEdtOutl);
            mbInDownScale = true;

            // sucks that we cannot disable paints via
            // pEdtOutl->SetUpdateMode(FALSE) - but EditEngine skips
            // formatting as well, then.
            ImpAutoFitText(*pEdtOutl);
            mbInDownScale = false;
        }
    }
}

/* Begin chaining code */

// XXX: Make it a method somewhere?
static SdrObject *ImpGetObjByName(SdrObjList const *pObjList, OUString const& aObjName)
{
    // scan the whole list
    size_t nObjCount = pObjList->GetObjCount();
    for (size_t i = 0; i < nObjCount; i++) {
        SdrObject *pCurObj = pObjList->GetObj(i);

        if (pCurObj->GetName() == aObjName) {
            return pCurObj;
        }
    }
    // not found
    return nullptr;
}

// XXX: Make it a (private) method of SdrTextObj
static void ImpUpdateChainLinks(SdrTextObj *pTextObj, OUString const& aNextLinkName)
{
    // XXX: Current implementation constraints text boxes to be on the same page

    // No next link
    if (aNextLinkName.isEmpty()) {
        pTextObj->SetNextLinkInChain(nullptr);
        return;
    }

    SdrPage *pPage(pTextObj->getSdrPageFromSdrObject());
    assert(pPage);
    SdrTextObj *pNextTextObj = dynamic_cast< SdrTextObj * >
                                (ImpGetObjByName(pPage, aNextLinkName));
    if (!pNextTextObj) {
        SAL_INFO("svx.chaining", "[CHAINING] Can't find object as next link.");
        return;
    }

    pTextObj->SetNextLinkInChain(pNextTextObj);
}

bool SdrTextObj::IsChainable() const
{
    // Read it as item
    const SfxItemSet& rSet = GetObjectItemSet();
    OUString aNextLinkName = rSet.Get(SDRATTR_TEXT_CHAINNEXTNAME).GetValue();

    // Update links if any inconsistency is found
    bool bNextLinkUnsetYet = !aNextLinkName.isEmpty() && !mpNextInChain;
    bool bInconsistentNextLink = mpNextInChain && mpNextInChain->GetName() != aNextLinkName;
    // if the link is not set despite there should be one OR if it has changed
    if (bNextLinkUnsetYet || bInconsistentNextLink) {
        ImpUpdateChainLinks(const_cast<SdrTextObj *>(this), aNextLinkName);
    }

    return !aNextLinkName.isEmpty(); // XXX: Should we also check for GetNilChainingEvent? (see old code below)

/*
    // Check that no overflow is going on
    if (!GetTextChain() || GetTextChain()->GetNilChainingEvent(this))
        return false;
*/
}

void SdrTextObj::onChainingEvent()
{
    if (!pEdtOutl)
        return;

    // Outliner for text transfer
    SdrOutliner &aDrawOutliner = ImpGetDrawOutliner();

    EditingTextChainFlow aTxtChainFlow(this);
    aTxtChainFlow.CheckForFlowEvents(pEdtOutl);

    if (aTxtChainFlow.IsOverflow()) {
        SAL_INFO("svx.chaining", "[CHAINING] Overflow going on");
        // One outliner is for non-overflowing text, the other for overflowing text
        // We remove text directly from the editing outliner
        aTxtChainFlow.ExecuteOverflow(pEdtOutl, &aDrawOutliner);
    } else if (aTxtChainFlow.IsUnderflow()) {
        SAL_INFO("svx.chaining", "[CHAINING] Underflow going on");
        // underflow-induced overflow
        aTxtChainFlow.ExecuteUnderflow(&aDrawOutliner);
        bool bIsOverflowFromUnderflow = aTxtChainFlow.IsOverflow();
        // handle overflow
        if (bIsOverflowFromUnderflow) {
            SAL_INFO("svx.chaining", "[CHAINING] Overflow going on (underflow induced)");
            // prevents infinite loops when setting text for editing outliner
            aTxtChainFlow.ExecuteOverflow(&aDrawOutliner, &aDrawOutliner);
        }
    }
}

SdrTextObj* SdrTextObj::GetNextLinkInChain() const
{
    /*
    if (GetTextChain())
        return GetTextChain()->GetNextLink(this);

    return NULL;
    */

    return mpNextInChain;
}

void SdrTextObj::SetNextLinkInChain(SdrTextObj *pNextObj)
{
    // Basically a doubly linked list implementation

    SdrTextObj *pOldNextObj = mpNextInChain;

    // Replace next link
    mpNextInChain = pNextObj;
    // Deal with old next link's prev link
    if (pOldNextObj) {
        pOldNextObj->mpPrevInChain = nullptr;
    }

    // Deal with new next link's prev link
    if (mpNextInChain) {
        // If there is a prev already at all and this is not already the current object
        if (mpNextInChain->mpPrevInChain &&
            mpNextInChain->mpPrevInChain != this)
            mpNextInChain->mpPrevInChain->mpNextInChain = nullptr;
        mpNextInChain->mpPrevInChain = this;
    }

    // TODO: Introduce check for circular chains

}

SdrTextObj* SdrTextObj::GetPrevLinkInChain() const
{
    /*
    if (GetTextChain())
        return GetTextChain()->GetPrevLink(this);

    return NULL;
    */

    return mpPrevInChain;
}

bool SdrTextObj::GetPreventChainable() const
{
    // Prevent chaining it 1) during dragging && 2) when we are editing next link
    return mbIsUnchainableClone || (GetNextLinkInChain() && GetNextLinkInChain()->IsInEditMode());
}

 SdrObject* SdrTextObj::getFullDragClone() const
 {
    SdrObject *pClone = SdrAttrObj::getFullDragClone();
    SdrTextObj *pTextObjClone = dynamic_cast<SdrTextObj *>(pClone);
    if (pTextObjClone != nullptr) {
        // Avoid transferring of text for chainable object during dragging
        pTextObjClone->mbIsUnchainableClone = true;
    }

    return pClone;
 }

/* End chaining code */

/** returns the currently active text. */
SdrText* SdrTextObj::getActiveText() const
{
    if( !mpText )
        return getText( 0 );
    else
        return mpText.get();
}

/** returns the nth available text. */
SdrText* SdrTextObj::getText( sal_Int32 nIndex ) const
{
    if( nIndex == 0 )
    {
        if( !mpText )
            const_cast< SdrTextObj* >(this)->mpText.reset( new SdrText( *const_cast< SdrTextObj* >(this) ) );
        return mpText.get();
    }
    else
    {
        return nullptr;
    }
}

/** returns the number of texts available for this object. */
sal_Int32 SdrTextObj::getTextCount() const
{
    return 1;
}

/** changes the current active text */
void SdrTextObj::setActiveText( sal_Int32 /*nIndex*/ )
{
}

/** returns the index of the text that contains the given point or -1 */
sal_Int32 SdrTextObj::CheckTextHit(const Point& /*rPnt*/) const
{
    return 0;
}

void SdrTextObj::SetObjectItemNoBroadcast(const SfxPoolItem& rItem)
{
    static_cast< sdr::properties::TextProperties& >(GetProperties()).SetObjectItemNoBroadcast(rItem);
}


// The concept of the text object:
// ~~~~~~~~~~~~~~~~~~~~~~~~
// Attributes/Variations:
// - bool text frame / graphics object with caption
// - bool FontWork                 (if it is not a text frame and not a ContourTextFrame)
// - bool ContourTextFrame         (if it is not a text frame and not Fontwork)
// - long rotation angle               (if it is not FontWork)
// - long text frame margins           (if it is not FontWork)
// - bool FitToSize                (if it is not FontWork)
// - bool AutoGrowingWidth/Height  (if it is not FitToSize and not FontWork)
// - long Min/MaxFrameWidth/Height     (if AutoGrowingWidth/Height)
// - enum horizontal text anchoring left,center,right,justify/block,Stretch(ni)
// - enum vertical text anchoring top, middle, bottom, block, stretch(ni)
// - enum ticker text                  (if it is not FontWork)

// Every derived object is either a text frame (bTextFrame=true)
// or a drawing object with a caption (bTextFrame=false).

// Default anchoring for text frames:
//   SDRTEXTHORZADJUST_BLOCK, SDRTEXTVERTADJUST_TOP
//   = static Pool defaults
// Default anchoring for drawing objects with a caption:
//   SDRTEXTHORZADJUST_CENTER, SDRTEXTVERTADJUST_CENTER
//   via "hard" attribution of SdrAttrObj

// Every object derived from SdrTextObj must return an "UnrotatedSnapRect"
// (->TakeUnrotatedSnapRect()) (the reference point for the rotation is the top
// left of the rectangle (aGeo.nRotationAngle)) which is the basis for anchoring
// text. We then subtract the text frame margins from this rectangle, as a re-
// sult we get the anchoring area (->TakeTextAnchorRect()). Within this area, we
// calculate the anchoring point and the painting area, depending on the hori-
// zontal and vertical adjustment of the text (SdrTextVertAdjust,
// SdrTextHorzAdjust).
// In the case of drawing objects with a caption the painting area might well
// be larger than the anchoring area, for text frames on the other hand, it is
// always of the same or a smaller size (except when there are negative text
// frame margins).

// FitToSize takes priority over text anchoring and AutoGrowHeight/Width. When
// FitToSize is turned on, the painting area is always equal to the anchoring
// area. Additionally, FitToSize doesn't allow automatic line breaks.

// ContourTextFrame:
// - long rotation angle
// - long text frame margins (maybe later)
// - bool FitToSize (maybe later)
// - bool AutoGrowingWidth/Height (maybe much later)
// - long Min/MaxFrameWidth/Height (maybe much later)
// - enum horizontal text anchoring (maybe later, for now: left, centered)
// - enum vertical text anchoring (maybe later, for now: top)
// - enum ticker text (maybe later, maybe even with correct clipping)

// When making changes, check these:
// - Paint
// - HitTest
// - ConvertToPoly
// - Edit
// - Printing, Saving, Painting in neighboring View while editing
// - ModelChanged (e. g. through a neighboring View or rulers) while editing
// - FillColorChanged while editing
// - and many more...


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
