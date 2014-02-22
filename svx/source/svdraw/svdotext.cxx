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


#include <comphelper/string.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>
#include "svx/svdglob.hxx"
#include "svx/svdstr.hrc"
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
#include <svx/sdr/properties/textproperties.hxx>
#include <vcl/metaact.hxx>
#include <svx/sdr/contact/viewcontactoftextobj.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <vcl/virdev.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "svdconv.hxx"



using namespace com::sun::star;




sdr::properties::BaseProperties* SdrTextObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::TextProperties(*this);
}




sdr::contact::ViewContact* SdrTextObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfTextObj(*this);
}



TYPEINIT1(SdrTextObj,SdrAttrObj);

SdrTextObj::SdrTextObj()
:   SdrAttrObj(),
    mpText(NULL),
    pEdtOutl(NULL),
    pFormTextBoundRect(NULL),
    eTextKind(OBJ_TEXT)
{
    bTextSizeDirty=sal_False;
    bTextFrame=sal_False;
    bNoShear=sal_False;
    bNoRotate=sal_False;
    bNoMirror=sal_False;
    bDisableAutoWidthOnDragging=sal_False;

    mbInEditMode = sal_False;
    mbTextHidden = sal_False;
    mbTextAnimationAllowed = sal_True;
    maTextEditOffset = Point(0, 0);

    
    mbSupportTextIndentingOnLineWidthChange = true;
    mbInDownScale = sal_False;
}

SdrTextObj::SdrTextObj(const Rectangle& rNewRect)
:   SdrAttrObj(),
    aRect(rNewRect),
    mpText(NULL),
    pEdtOutl(NULL),
    pFormTextBoundRect(NULL),
    eTextKind(OBJ_TEXT)
{
    bTextSizeDirty=sal_False;
    bTextFrame=sal_False;
    bNoShear=sal_False;
    bNoRotate=sal_False;
    bNoMirror=sal_False;
    bDisableAutoWidthOnDragging=sal_False;
    ImpJustifyRect(aRect);

    mbInEditMode = sal_False;
    mbTextHidden = sal_False;
    mbTextAnimationAllowed = sal_True;
    mbInDownScale = sal_False;
    maTextEditOffset = Point(0, 0);

    
    mbSupportTextIndentingOnLineWidthChange = true;
}

SdrTextObj::SdrTextObj(SdrObjKind eNewTextKind)
:   SdrAttrObj(),
    mpText(NULL),
    pEdtOutl(NULL),
    pFormTextBoundRect(NULL),
    eTextKind(eNewTextKind)
{
    bTextSizeDirty=sal_False;
    bTextFrame=sal_True;
    bNoShear=sal_True;
    bNoRotate=sal_False;
    bNoMirror=sal_True;
    bDisableAutoWidthOnDragging=sal_False;

    mbInEditMode = sal_False;
    mbTextHidden = sal_False;
    mbTextAnimationAllowed = sal_True;
    mbInDownScale = sal_False;
    maTextEditOffset = Point(0, 0);

    
    mbSupportTextIndentingOnLineWidthChange = true;
}

SdrTextObj::SdrTextObj(SdrObjKind eNewTextKind, const Rectangle& rNewRect)
:   SdrAttrObj(),
    aRect(rNewRect),
    mpText(NULL),
    pEdtOutl(NULL),
    pFormTextBoundRect(NULL),
    eTextKind(eNewTextKind)
{
    bTextSizeDirty=sal_False;
    bTextFrame=sal_True;
    bNoShear=sal_True;
    bNoRotate=sal_False;
    bNoMirror=sal_True;
    bDisableAutoWidthOnDragging=sal_False;
    ImpJustifyRect(aRect);

    mbInEditMode = sal_False;
    mbTextHidden = sal_False;
    mbTextAnimationAllowed = sal_True;
    mbInDownScale = sal_False;
    maTextEditOffset = Point(0, 0);

    
    mbSupportTextIndentingOnLineWidthChange = true;
}

SdrTextObj::~SdrTextObj()
{
    if( pModel )
    {
        SdrOutliner& rOutl = pModel->GetHitTestOutliner();
        if( rOutl.GetTextObj() == this )
            rOutl.SetTextObj( NULL );
    }

    delete mpText;

    delete pFormTextBoundRect;

    ImpLinkAbmeldung();
}

void SdrTextObj::FitFrameToTextSize()
{
    DBG_ASSERT(pModel!=NULL,"SdrTextObj::FitFrameToTextSize(): pModel=NULL!");
    ImpJustifyRect(aRect);

    SdrText* pText = getActiveText();
    if( pText!=NULL && pText->GetOutlinerParaObject() && pModel!=NULL)
    {
        SdrOutliner& rOutliner=ImpGetDrawOutliner();
        rOutliner.SetPaperSize(Size(aRect.Right()-aRect.Left(),aRect.Bottom()-aRect.Top()));
        rOutliner.SetUpdateMode(sal_True);
        rOutliner.SetText(*pText->GetOutlinerParaObject());
        Size aNewSize(rOutliner.CalcTextSize());
        rOutliner.Clear();
        aNewSize.Width()++; 
        aNewSize.Width()+=GetTextLeftDistance()+GetTextRightDistance();
        aNewSize.Height()+=GetTextUpperDistance()+GetTextLowerDistance();
        Rectangle aNewRect(aRect);
        aNewRect.SetSize(aNewSize);
        ImpJustifyRect(aNewRect);
        if (aNewRect!=aRect) {
            SetLogicRect(aNewRect);
        }
    }
}

void SdrTextObj::NbcSetText(const OUString& rStr)
{
    SdrOutliner& rOutliner=ImpGetDrawOutliner();
    rOutliner.SetStyleSheet( 0, GetStyleSheet());
    rOutliner.SetUpdateMode(sal_True);
    rOutliner.SetText(rStr,rOutliner.GetParagraph( 0 ));
    OutlinerParaObject* pNewText=rOutliner.CreateParaObject();
    Size aSiz(rOutliner.CalcTextSize());
    rOutliner.Clear();
    NbcSetOutlinerParaObject(pNewText);
    aTextSize=aSiz;
    bTextSizeDirty=sal_False;
}

void SdrTextObj::SetText(const OUString& rStr)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    NbcSetText(rStr);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrTextObj::NbcSetText(SvStream& rInput, const OUString& rBaseURL, sal_uInt16 eFormat)
{
    SdrOutliner& rOutliner=ImpGetDrawOutliner();
    rOutliner.SetStyleSheet( 0, GetStyleSheet());
    rOutliner.Read(rInput,rBaseURL,eFormat);
    OutlinerParaObject* pNewText=rOutliner.CreateParaObject();
    rOutliner.SetUpdateMode(sal_True);
    Size aSiz(rOutliner.CalcTextSize());
    rOutliner.Clear();
    NbcSetOutlinerParaObject(pNewText);
    aTextSize=aSiz;
    bTextSizeDirty=sal_False;
}

void SdrTextObj::SetText(SvStream& rInput, const OUString& rBaseURL, sal_uInt16 eFormat)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    NbcSetText(rInput,rBaseURL,eFormat);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
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
            rOutliner.SetUpdateMode(sal_True);
            aSiz=rOutliner.CalcTextSize();
            rOutliner.Clear();
        }
        
        ((SdrTextObj*)this)->aTextSize=aSiz;
        ((SdrTextObj*)this)->bTextSizeDirty=sal_False;
    }
    return aTextSize;
}

bool SdrTextObj::IsAutoGrowHeight() const
{
    if(!bTextFrame)
        return false; 

    const SfxItemSet& rSet = GetObjectItemSet();
    sal_Bool bRet = ((SdrTextAutoGrowHeightItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();

    if(bRet)
    {
        SdrTextAniKind eAniKind = ((SdrTextAniKindItem&)(rSet.Get(SDRATTR_TEXT_ANIKIND))).GetValue();

        if(eAniKind == SDRTEXTANI_SCROLL || eAniKind == SDRTEXTANI_ALTERNATE || eAniKind == SDRTEXTANI_SLIDE)
        {
            SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();

            if(eDirection == SDRTEXTANI_UP || eDirection == SDRTEXTANI_DOWN)
            {
                bRet = sal_False;
            }
        }
    }
    return bRet;
}

bool SdrTextObj::IsAutoGrowWidth() const
{
    if(!bTextFrame)
        return false; 

    const SfxItemSet& rSet = GetObjectItemSet();
    sal_Bool bRet = ((SdrTextAutoGrowHeightItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWWIDTH))).GetValue();

    sal_Bool bInEditMOde = IsInEditMode();

    if(!bInEditMOde && bRet)
    {
        SdrTextAniKind eAniKind = ((SdrTextAniKindItem&)(rSet.Get(SDRATTR_TEXT_ANIKIND))).GetValue();

        if(eAniKind == SDRTEXTANI_SCROLL || eAniKind == SDRTEXTANI_ALTERNATE || eAniKind == SDRTEXTANI_SLIDE)
        {
            SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();

            if(eDirection == SDRTEXTANI_LEFT || eDirection == SDRTEXTANI_RIGHT)
            {
                bRet = sal_False;
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

    SdrTextHorzAdjust eRet = ((SdrTextHorzAdjustItem&)(rSet.Get(SDRATTR_TEXT_HORZADJUST))).GetValue();

    sal_Bool bInEditMode = IsInEditMode();

    if(!bInEditMode && eRet == SDRTEXTHORZADJUST_BLOCK)
    {
        SdrTextAniKind eAniKind = ((SdrTextAniKindItem&)(rSet.Get(SDRATTR_TEXT_ANIKIND))).GetValue();

        if(eAniKind == SDRTEXTANI_SCROLL || eAniKind == SDRTEXTANI_ALTERNATE || eAniKind == SDRTEXTANI_SLIDE)
        {
            SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();

            if(eDirection == SDRTEXTANI_LEFT || eDirection == SDRTEXTANI_RIGHT)
            {
                eRet = SDRTEXTHORZADJUST_LEFT;
            }
        }
    }

    return eRet;
} 

SdrTextVertAdjust SdrTextObj::GetTextVerticalAdjust() const
{
    return GetTextVerticalAdjust(GetObjectItemSet());
}

SdrTextVertAdjust SdrTextObj::GetTextVerticalAdjust(const SfxItemSet& rSet) const
{
    if(IsContourTextFrame())
        return SDRTEXTVERTADJUST_TOP;

    
    SdrTextVertAdjust eRet = ((SdrTextVertAdjustItem&)(rSet.Get(SDRATTR_TEXT_VERTADJUST))).GetValue();
    sal_Bool bInEditMode = IsInEditMode();

    
    if(!bInEditMode && eRet == SDRTEXTVERTADJUST_BLOCK)
    {
        SdrTextAniKind eAniKind = ((SdrTextAniKindItem&)(rSet.Get(SDRATTR_TEXT_ANIKIND))).GetValue();

        if(eAniKind == SDRTEXTANI_SCROLL || eAniKind == SDRTEXTANI_ALTERNATE || eAniKind == SDRTEXTANI_SLIDE)
        {
            SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();

            if(eDirection == SDRTEXTANI_LEFT || eDirection == SDRTEXTANI_RIGHT)
            {
                eRet = SDRTEXTVERTADJUST_TOP;
            }
        }
    }

    return eRet;
} 

void SdrTextObj::ImpJustifyRect(Rectangle& rRect) const
{
    if (!rRect.IsEmpty()) {
        rRect.Justify();
        if (rRect.Left()==rRect.Right()) rRect.Right()++;
        if (rRect.Top()==rRect.Bottom()) rRect.Bottom()++;
    }
}

void SdrTextObj::ImpCheckShear()
{
    if (bNoShear && aGeo.nShearWink!=0) {
        aGeo.nShearWink=0;
        aGeo.nTan=0;
    }
}

void SdrTextObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bNoTextFrame=!IsTextFrame();
    rInfo.bResizeFreeAllowed=bNoTextFrame || aGeo.nDrehWink%9000==0;
    rInfo.bResizePropAllowed=true;
    rInfo.bRotateFreeAllowed=true;
    rInfo.bRotate90Allowed  =true;
    rInfo.bMirrorFreeAllowed=bNoTextFrame;
    rInfo.bMirror45Allowed  =bNoTextFrame;
    rInfo.bMirror90Allowed  =bNoTextFrame;

    
    rInfo.bTransparenceAllowed = true;

    
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetObjectItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.bGradientAllowed = (eFillStyle == XFILL_GRADIENT);
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

bool SdrTextObj::HasTextImpl( SdrOutliner* pOutliner )
{
    bool bRet=false;
    if(pOutliner)
    {
        Paragraph* p1stPara=pOutliner->GetParagraph( 0 );
        sal_Int32 nParaAnz=pOutliner->GetParagraphCount();
        if(p1stPara==NULL)
            nParaAnz=0;

        if(nParaAnz==1)
        {
            
            if( pOutliner->GetText(p1stPara).isEmpty() )
                nParaAnz = 0;
        }

        bRet= nParaAnz!=0;
    }
    return bRet;
}

bool SdrTextObj::HasEditText() const
{
    return HasTextImpl( pEdtOutl );
}

void SdrTextObj::SetPage(SdrPage* pNewPage)
{
    bool bRemove=pNewPage==NULL && pPage!=NULL;
    bool bInsert=pNewPage!=NULL && pPage==NULL;
    bool bLinked=IsLinkedText();

    if (bLinked && bRemove) {
        ImpLinkAbmeldung();
    }

    SdrAttrObj::SetPage(pNewPage);

    if (bLinked && bInsert) {
        ImpLinkAnmeldung();
    }
}

void SdrTextObj::SetModel(SdrModel* pNewModel)
{
    SdrModel* pOldModel=pModel;
    bool bLinked=IsLinkedText();
    bool bChg=pNewModel!=pModel;

    if (bLinked && bChg)
    {
        ImpLinkAbmeldung();
    }

    SdrAttrObj::SetModel(pNewModel);

    if( bChg )
    {
        if( pNewModel != 0 && pOldModel != 0 )
            SetTextSizeDirty();

        sal_Int32 nCount = getTextCount();
        for( sal_Int32 nText = 0; nText < nCount; nText++ )
        {
            SdrText* pText = getText( nText );
            if( pText )
                pText->SetModel( pNewModel );
        }
    }

    if (bLinked && bChg)
    {
        ImpLinkAnmeldung();
    }
}

bool SdrTextObj::NbcSetEckenradius(long nRad)
{
    SetObjectItem(SdrEckenradiusItem(nRad));
    return true;
}

bool SdrTextObj::NbcSetMinTextFrameHeight(long nHgt)
{
    if( bTextFrame && ( !pModel || !pModel->isLocked() ) )          
    {
        SetObjectItem(SdrTextMinFrameHeightItem(nHgt));

        
        
        if(IsVerticalWriting() && bDisableAutoWidthOnDragging)
        {
            bDisableAutoWidthOnDragging = sal_False;
            SetObjectItem(SdrTextAutoGrowHeightItem(sal_False));
        }

        return true;
    }
    return false;
}

bool SdrTextObj::NbcSetMinTextFrameWidth(long nWdt)
{
    if( bTextFrame && ( !pModel || !pModel->isLocked() ) )          
    {
        SetObjectItem(SdrTextMinFrameWidthItem(nWdt));

        
        
        if(!IsVerticalWriting() && bDisableAutoWidthOnDragging)
        {
            bDisableAutoWidthOnDragging = sal_False;
            SetObjectItem(SdrTextAutoGrowWidthItem(sal_False));
        }

        return true;
    }
    return false;
}

void SdrTextObj::ImpSetContourPolygon( SdrOutliner& rOutliner, Rectangle& rAnchorRect, sal_Bool bLineWidth ) const
{
    basegfx::B2DPolyPolygon aXorPolyPolygon(TakeXorPoly());
    basegfx::B2DPolyPolygon* pContourPolyPolygon = 0L;
    basegfx::B2DHomMatrix aMatrix(basegfx::tools::createTranslateB2DHomMatrix(
        -rAnchorRect.Left(), -rAnchorRect.Top()));

    if(aGeo.nDrehWink)
    {
        
        aMatrix.rotate(-aGeo.nDrehWink * nPi180);
    }

    aXorPolyPolygon.transform(aMatrix);

    if( bLineWidth )
    {
        
        
        pContourPolyPolygon = new basegfx::B2DPolyPolygon();

        
        const SfxItemSet& rSet = GetObjectItemSet();
        sal_Bool bShadowOn = ((SdrShadowItem&)(rSet.Get(SDRATTR_SHADOW))).GetValue();

        
        
        
        
        const SdrTextObj* pLastTextObject = rOutliner.GetTextObj();

        if(bShadowOn)
        {
            
            SdrObject* pCopy = Clone();
            pCopy->SetMergedItem(SdrShadowItem(sal_False));
            *pContourPolyPolygon = pCopy->TakeContour();
            SdrObject::Free( pCopy );
        }
        else
        {
            *pContourPolyPolygon = TakeContour();
        }

        
        
        if(pLastTextObject != rOutliner.GetTextObj())
        {
            rOutliner.SetTextObj(pLastTextObject);
        }

        pContourPolyPolygon->transform(aMatrix);
    }

    rOutliner.SetPolygon(aXorPolyPolygon, pContourPolyPolygon);
    delete pContourPolyPolygon;
}

void SdrTextObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
{
    rRect=aRect;
}

void SdrTextObj::TakeTextAnchorRect(Rectangle& rAnchorRect) const
{
    long nLeftDist=GetTextLeftDistance();
    long nRightDist=GetTextRightDistance();
    long nUpperDist=GetTextUpperDistance();
    long nLowerDist=GetTextLowerDistance();
    Rectangle aAnkRect(aRect); 
    bool bFrame=IsTextFrame();
    if (!bFrame) {
        TakeUnrotatedSnapRect(aAnkRect);
    }
    Point aRotateRef(aAnkRect.TopLeft());
    aAnkRect.Left()+=nLeftDist;
    aAnkRect.Top()+=nUpperDist;
    aAnkRect.Right()-=nRightDist;
    aAnkRect.Bottom()-=nLowerDist;

    
    
    ImpJustifyRect(aAnkRect);

    if (bFrame) {
        
        if (aAnkRect.GetWidth()<2) aAnkRect.Right()=aAnkRect.Left()+1; 
        if (aAnkRect.GetHeight()<2) aAnkRect.Bottom()=aAnkRect.Top()+1;
    }
    if (aGeo.nDrehWink!=0) {
        Point aTmpPt(aAnkRect.TopLeft());
        RotatePoint(aTmpPt,aRotateRef,aGeo.nSin,aGeo.nCos);
        aTmpPt-=aAnkRect.TopLeft();
        aAnkRect.Move(aTmpPt.X(),aTmpPt.Y());
    }
    rAnchorRect=aAnkRect;
}

void SdrTextObj::TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, bool bNoEditText,
                               Rectangle* pAnchorRect, bool bLineWidth ) const
{
    Rectangle aAnkRect; 
    TakeTextAnchorRect(aAnkRect);
    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
    SdrTextAniKind      eAniKind=GetTextAniKind();
    SdrTextAniDirection eAniDirection=GetTextAniDirection();

    bool bFitToSize(IsFitToSize());
    bool bContourFrame=IsContourTextFrame();

    bool bFrame=IsTextFrame();
    sal_uIntPtr nStat0=rOutliner.GetControlWord();
    Size aNullSize;
    if (!bContourFrame)
    {
        rOutliner.SetControlWord(nStat0|EE_CNTRL_AUTOPAGESIZE);
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

            sal_Bool bInEditMode = IsInEditMode();

            if (!bInEditMode && (eAniKind==SDRTEXTANI_SCROLL || eAniKind==SDRTEXTANI_ALTERNATE || eAniKind==SDRTEXTANI_SLIDE))
            {
                
                if (eAniDirection==SDRTEXTANI_LEFT || eAniDirection==SDRTEXTANI_RIGHT) nWdt=1000000;
                if (eAniDirection==SDRTEXTANI_UP || eAniDirection==SDRTEXTANI_DOWN) nHgt=1000000;
            }

            
            if(IsVerticalWriting())
            {
                nWdt = 1000000;
            }
            else
            {
                nHgt = 1000000;
            }

            rOutliner.SetMaxAutoPaperSize(Size(nWdt,nHgt));
        }

        
        
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

    
    SdrText* pText = getActiveText();
    OutlinerParaObject* pOutlinerParaObject = pText ? pText->GetOutlinerParaObject() : 0;
    OutlinerParaObject* pPara = (pEdtOutl && !bNoEditText) ? pEdtOutl->CreateParaObject() : pOutlinerParaObject;

    if (pPara)
    {
        bool bHitTest = false;
        if( pModel )
            bHitTest = &pModel->GetHitTestOutliner() == &rOutliner;

        const SdrTextObj* pTestObj = rOutliner.GetTextObj();
        if( !pTestObj || !bHitTest || pTestObj != this ||
            pTestObj->GetOutlinerParaObject() != pOutlinerParaObject )
        {
            if( bHitTest ) 
            {
                rOutliner.SetTextObj( this );
                rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
            }

            rOutliner.SetUpdateMode(sal_True);
            rOutliner.SetText(*pPara);
        }
    }
    else
    {
        rOutliner.SetTextObj( NULL );
    }

    if (pEdtOutl && !bNoEditText && pPara)
        delete pPara;

    rOutliner.SetUpdateMode(sal_True);
    rOutliner.SetControlWord(nStat0);

    if( pText )
        pText->CheckPortionInfo(rOutliner);

    Point aTextPos(aAnkRect.TopLeft());
    Size aTextSiz(rOutliner.GetPaperSize()); 

    
    
    
    if(!IsTextFrame())
    {
        if(aAnkRect.GetWidth() < aTextSiz.Width() && !IsVerticalWriting())
        {
            
            
            if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
            {
                eHAdj = SDRTEXTHORZADJUST_CENTER;
            }
        }

        if(aAnkRect.GetHeight() < aTextSiz.Height() && IsVerticalWriting())
        {
            
            
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
            aTextPos.X()+=nFreeWdt/2;
        if (eHAdj==SDRTEXTHORZADJUST_RIGHT)
            aTextPos.X()+=nFreeWdt;
    }
    if (eVAdj==SDRTEXTVERTADJUST_CENTER || eVAdj==SDRTEXTVERTADJUST_BOTTOM)
    {
        long nFreeHgt=aAnkRect.GetHeight()-aTextSiz.Height();
        if (eVAdj==SDRTEXTVERTADJUST_CENTER)
            aTextPos.Y()+=nFreeHgt/2;
        if (eVAdj==SDRTEXTVERTADJUST_BOTTOM)
            aTextPos.Y()+=nFreeHgt;
    }
    if (aGeo.nDrehWink!=0)
        RotatePoint(aTextPos,aAnkRect.TopLeft(),aGeo.nSin,aGeo.nCos);

    if (pAnchorRect)
        *pAnchorRect=aAnkRect;

    
    rTextRect=Rectangle(aTextPos,aTextSiz);
    if (bContourFrame)
        rTextRect=aAnkRect;
}

OutlinerParaObject* SdrTextObj::GetEditOutlinerParaObject() const
{
    OutlinerParaObject* pPara=NULL;
    if( HasTextImpl( pEdtOutl ) )
    {
        sal_Int32 nParaAnz = pEdtOutl->GetParagraphCount();
        pPara = pEdtOutl->CreateParaObject(0, nParaAnz);
    }
    return pPara;
}

void SdrTextObj::ImpSetCharStretching(SdrOutliner& rOutliner, const Size& rTextSize, const Size& rShapeSize, Fraction& rFitXKorreg) const
{
    OutputDevice* pOut = rOutliner.GetRefDevice();
    bool bNoStretching(false);

    if(pOut && pOut->GetOutDevType() == OUTDEV_PRINTER)
    {
        
        GDIMetaFile* pMtf = pOut->GetConnectMetaFile();
        OUString aTestString(static_cast<sal_Unicode>('J'));

        if(pMtf && (!pMtf->IsRecord() || pMtf->IsPause()))
            pMtf = NULL;

        if(pMtf)
            pMtf->Pause(true);

        Font aFontMerk(pOut->GetFont());
        Font aTmpFont( OutputDevice::GetDefaultFont( DEFAULTFONT_SERIF, LANGUAGE_SYSTEM, DEFAULTFONT_FLAGS_ONLYONE ) );

        aTmpFont.SetSize(Size(0,100));
        pOut->SetFont(aTmpFont);
        Size aSize1(pOut->GetTextWidth(aTestString), pOut->GetTextHeight());
        aTmpFont.SetSize(Size(800,100));
        pOut->SetFont(aTmpFont);
        Size aSize2(pOut->GetTextWidth(aTestString), pOut->GetTextHeight());
        pOut->SetFont(aFontMerk);

        if(pMtf)
            pMtf->Pause(false);

        bNoStretching = (aSize1 == aSize2);

#ifdef WNT
        
        
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

    long nXTolPl=nWantWdt/100; 
    long nXTolMi=nWantWdt/25;  
    long nXKorr =nWantWdt/20;  

    long nX=(nWantWdt*100) /nIsWdt; 
    long nY=(nWantHgt*100) /nIsHgt; 
    bool bChkX = true;
    if (bNoStretching) { 
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

        
        if(nIsWdt <= 1)
        {
            nX = nY;
            bNoMoreLoop = true;
        }

        
        if(nIsHgt <= 1)
        {
            nY = nX;
            bNoMoreLoop = true;
        }

        rOutliner.SetGlobalCharStretching((sal_uInt16)nX,(sal_uInt16)nY);
        nLoopCount++;
        Size aSiz(rOutliner.CalcTextSize());
        long nXDiff=aSiz.Width()-nWantWdt;
        rFitXKorreg=Fraction(nWantWdt,aSiz.Width());
        if (((nXDiff>=nXTolMi || !bChkX) && nXDiff<=nXTolPl) || nXDiff==nXDiff0) {
            bNoMoreLoop = true;
        } else {
            
            long nMul=nWantWdt;
            long nDiv=aSiz.Width();
            if (std::abs(nXDiff)<=2*nXKorr) {
                if (nMul>nDiv) nDiv+=(nMul-nDiv)/2; 
                else nMul+=(nDiv-nMul)/2;           
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
            aStr = ImpGetResStr(STR_ObjNameSingulOUTLINETEXT);
            break;
        }

        case OBJ_TITLETEXT  :
        {
            aStr = ImpGetResStr(STR_ObjNameSingulTITLETEXT);
            break;
        }

        default:
        {
            if(IsLinkedText())
                aStr = ImpGetResStr(STR_ObjNameSingulTEXTLNK);
            else
                aStr = ImpGetResStr(STR_ObjNameSingulTEXT);
            break;
        }
    }

    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
    if(pOutlinerParaObject && eTextKind != OBJ_OUTLINETEXT)
    {
        
        OUString aStr2(comphelper::string::stripStart(pOutlinerParaObject->GetTextObject().GetText(0), ' '));

        
        
        if(!aStr2.isEmpty() && aStr2.indexOf(sal_Unicode(255)) == -1)
        {
            
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
        case OBJ_OUTLINETEXT: sName=ImpGetResStr(STR_ObjNamePluralOUTLINETEXT); break;
        case OBJ_TITLETEXT  : sName=ImpGetResStr(STR_ObjNamePluralTITLETEXT);   break;
        default: {
            if (IsLinkedText()) {
                sName=ImpGetResStr(STR_ObjNamePluralTEXTLNK);
            } else {
                sName=ImpGetResStr(STR_ObjNamePluralTEXT);
            }
        } break;
    } 
    return sName;
}

SdrTextObj* SdrTextObj::Clone() const
{
    return CloneHelper< SdrTextObj >();
}

SdrTextObj& SdrTextObj::operator=(const SdrTextObj& rObj)
{
    if( this == &rObj )
        return *this;
    
    SdrObject::operator=(rObj);

    aRect     =rObj.aRect;
    aGeo      =rObj.aGeo;
    eTextKind =rObj.eTextKind;
    bTextFrame=rObj.bTextFrame;
    aTextSize=rObj.aTextSize;
    bTextSizeDirty=rObj.bTextSizeDirty;

    
    bNoShear = rObj.bNoShear;
    bNoRotate = rObj.bNoRotate;
    bNoMirror = rObj.bNoMirror;
    bDisableAutoWidthOnDragging = rObj.bDisableAutoWidthOnDragging;

    OutlinerParaObject* pNewOutlinerParaObject = 0;

    SdrText* pText = getActiveText();

    if( pText && rObj.HasText() )
    {
        const Outliner* pEO=rObj.pEdtOutl;
        if (pEO!=NULL)
        {
            pNewOutlinerParaObject = pEO->CreateParaObject();
        }
        else
        {
            pNewOutlinerParaObject = new OutlinerParaObject(*rObj.getActiveText()->GetOutlinerParaObject());
        }
    }

    mpText->SetOutlinerParaObject( pNewOutlinerParaObject );
    ImpSetTextStyleSheetListeners();
    return *this;
}

basegfx::B2DPolyPolygon SdrTextObj::TakeXorPoly() const
{
    Polygon aPol(aRect);
    if (aGeo.nShearWink!=0) ShearPoly(aPol,aRect.TopLeft(),aGeo.nTan);
    if (aGeo.nDrehWink!=0) RotatePoly(aPol,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);

    basegfx::B2DPolyPolygon aRetval;
    aRetval.append(aPol.getB2DPolygon());
    return aRetval;
}

basegfx::B2DPolyPolygon SdrTextObj::TakeContour() const
{
    basegfx::B2DPolyPolygon aRetval(SdrAttrObj::TakeContour());

    
    if ( pModel && GetOutlinerParaObject() && !IsFontwork() && !IsContourTextFrame() )
    {
        
        
        
        SdrOutliner& rOutliner=ImpGetDrawOutliner();

        Rectangle aAnchor2;
        Rectangle aR;
        TakeTextRect(rOutliner,aR,false,&aAnchor2);
        rOutliner.Clear();
        bool bFitToSize(IsFitToSize());
        if (bFitToSize) aR=aAnchor2;
        Polygon aPol(aR);
        if (aGeo.nDrehWink!=0) RotatePoly(aPol,aR.TopLeft(),aGeo.nSin,aGeo.nCos);

        aRetval.append(aPol.getB2DPolygon());
    }

    return aRetval;
}

void SdrTextObj::RecalcSnapRect()
{
    if (aGeo.nDrehWink!=0 || aGeo.nShearWink!=0) {
        Polygon aPol(aRect);
        if (aGeo.nShearWink!=0) ShearPoly(aPol,aRect.TopLeft(),aGeo.nTan);
        if (aGeo.nDrehWink!=0) RotatePoly(aPol,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
        maSnapRect=aPol.GetBoundRect();
    } else {
        maSnapRect=aRect;
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
        case 0: aP=aRect.TopLeft(); break;
        case 1: aP=aRect.TopRight(); break;
        case 2: aP=aRect.BottomLeft(); break;
        case 3: aP=aRect.BottomRight(); break;
        default: aP=aRect.Center(); break;
    }
    if (aGeo.nShearWink!=0) ShearPoint(aP,aRect.TopLeft(),aGeo.nTan);
    if (aGeo.nDrehWink!=0) RotatePoint(aP,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    return aP;
}

void SdrTextObj::ImpCheckMasterCachable()
{
    bNotMasterCachable=false;

    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();

    if(!bNotVisibleAsMaster && pOutlinerParaObject && pOutlinerParaObject->IsEditDoc() )
    {
        const EditTextObject& rText= pOutlinerParaObject->GetTextObject();
        bNotMasterCachable=rText.HasField(SvxPageField::StaticClassId());
        if( !bNotMasterCachable )
        {
            bNotMasterCachable=rText.HasField(SvxHeaderField::StaticClassId());
            if( !bNotMasterCachable )
            {
                bNotMasterCachable=rText.HasField(SvxFooterField::StaticClassId());
                if( !bNotMasterCachable )
                {
                    bNotMasterCachable=rText.HasField(SvxDateTimeField::StaticClassId());
                }
            }
        }
    }
}


void SdrTextObj::ImpInitDrawOutliner( SdrOutliner& rOutl ) const
{
    rOutl.SetUpdateMode(sal_False);
    sal_uInt16 nOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
    if ( !IsOutlText() )
        nOutlinerMode = OUTLINERMODE_TEXTOBJECT;
    rOutl.Init( nOutlinerMode );

    rOutl.SetGlobalCharStretching(100,100);
    sal_uIntPtr nStat=rOutl.GetControlWord();
    nStat&=~(EE_CNTRL_STRETCHING|EE_CNTRL_AUTOPAGESIZE);
    rOutl.SetControlWord(nStat);
    Size aNullSize;
    Size aMaxSize(100000,100000);
    rOutl.SetMinAutoPaperSize(aNullSize);
    rOutl.SetMaxAutoPaperSize(aMaxSize);
    rOutl.SetPaperSize(aMaxSize);
    rOutl.ClearPolygon();
}

SdrOutliner& SdrTextObj::ImpGetDrawOutliner() const
{
    SdrOutliner& rOutl=pModel->GetDrawOutliner(this);

    
    ImpInitDrawOutliner( rOutl );

    return rOutl;
}


void SdrTextObj::ImpSetupDrawOutlinerForPaint( bool             bContourFrame,
                                               SdrOutliner&     rOutliner,
                                               Rectangle&       rTextRect,
                                               Rectangle&       rAnchorRect,
                                               Rectangle&       rPaintRect,
                                               Fraction&        rFitXKorreg ) const
{
    if (!bContourFrame)
    {
        
        if (IsFitToSize() || IsAutoFit())
        {
            sal_uIntPtr nStat=rOutliner.GetControlWord();
            nStat|=EE_CNTRL_STRETCHING|EE_CNTRL_AUTOPAGESIZE;
            rOutliner.SetControlWord(nStat);
        }
    }
    rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
    TakeTextRect(rOutliner, rTextRect, false, &rAnchorRect);
    rPaintRect = rTextRect;

    if (!bContourFrame)
    {
        
        if (IsFitToSize())
        {
            ImpSetCharStretching(rOutliner,rTextRect.GetSize(),rAnchorRect.GetSize(),rFitXKorreg);
            rPaintRect=rAnchorRect;
        }
        else if (IsAutoFit())
        {
            ImpAutoFitText(rOutliner);
        }
    }
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
    
    

    
    sal_uInt16 nMinStretchX=0, nMinStretchY=0;
    sal_uInt16 aOldStretchXVals[]={0,0,0,0,0,0,0,0,0,0};
    const size_t aStretchArySize=SAL_N_ELEMENTS(aOldStretchXVals);
    for(unsigned int i=0; i<aStretchArySize; ++i)
    {
        const Size aCurrTextSize = rOutliner.CalcTextSizeNTP();
        double fFactor(1.0);
        if( bIsVerticalWriting )
            fFactor = double(rTextSize.Width())/aCurrTextSize.Width();
        else
            fFactor = double(rTextSize.Height())/aCurrTextSize.Height();
        
        
        
        
        
        fFactor = std::sqrt(fFactor);

        sal_uInt16 nCurrStretchX, nCurrStretchY;
        rOutliner.GetGlobalCharStretching(nCurrStretchX, nCurrStretchY);

        if (fFactor >= 1.0 )
        {
            
            
            nMinStretchX = std::max(nMinStretchX,nCurrStretchX);
            nMinStretchY = std::max(nMinStretchY,nCurrStretchY);
        }

        aOldStretchXVals[i] = nCurrStretchX;
        if( std::find(aOldStretchXVals, aOldStretchXVals+i, nCurrStretchX) != aOldStretchXVals+i )
            break; 

        if (fFactor < 1.0 || (fFactor >= 1.0 && nCurrStretchX != 100))
        {
            nCurrStretchX = sal::static_int_cast<sal_uInt16>(nCurrStretchX*fFactor);
            nCurrStretchY = sal::static_int_cast<sal_uInt16>(nCurrStretchY*fFactor);
            rOutliner.SetGlobalCharStretching(std::min(sal_uInt16(100),nCurrStretchX),
                                              std::min(sal_uInt16(100),nCurrStretchY));
            OSL_TRACE("SdrTextObj::onEditOutlinerStatusEvent(): zoom is %d", nCurrStretchX);
        }
    }

    OSL_TRACE("---- SdrTextObj::onEditOutlinerStatusEvent(): final zoom is %d ----", nMinStretchX);
    rOutliner.SetGlobalCharStretching(std::min(sal_uInt16(100),nMinStretchX),
                                      std::min(sal_uInt16(100),nMinStretchY));
}

void SdrTextObj::SetupOutlinerFormatting( SdrOutliner& rOutl, Rectangle& rPaintRect ) const
{
    ImpInitDrawOutliner( rOutl );
    UpdateOutlinerFormatting( rOutl, rPaintRect );
}

void SdrTextObj::UpdateOutlinerFormatting( SdrOutliner& rOutl, Rectangle& rPaintRect ) const
{
    Rectangle aTextRect;
    Rectangle aAnchorRect;
    Fraction aFitXKorreg(1,1);

    bool bContourFrame=IsContourTextFrame();

    if( GetModel() )
    {
        MapMode aMapMode(GetModel()->GetScaleUnit(), Point(0,0),
                         GetModel()->GetScaleFraction(),
                         GetModel()->GetScaleFraction());
        rOutl.SetRefMapMode(aMapMode);
    }

    ImpSetupDrawOutlinerForPaint( bContourFrame, rOutl, aTextRect, aAnchorRect, rPaintRect, aFitXKorreg );
}



OutlinerParaObject* SdrTextObj::GetOutlinerParaObject() const
{
    SdrText* pText = getActiveText();
    if( pText )
        return pText->GetOutlinerParaObject();
    else
        return 0;
}

void SdrTextObj::NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject)
{
    NbcSetOutlinerParaObjectForText( pTextObject, getActiveText() );
}

void SdrTextObj::NbcSetOutlinerParaObjectForText( OutlinerParaObject* pTextObject, SdrText* pText )
{
    if( pText )
        pText->SetOutlinerParaObject( pTextObject );

    if( pText->GetOutlinerParaObject() )
    {
        SvxWritingModeItem aWritingMode(pText->GetOutlinerParaObject()->IsVertical()
            ? com::sun::star::text::WritingMode_TB_RL
            : com::sun::star::text::WritingMode_LR_TB,
            SDRATTR_TEXTDIRECTION);
        GetProperties().SetObjectItemDirect(aWritingMode);
    }

    SetTextSizeDirty();
    if (IsTextFrame() && (IsAutoGrowHeight() || IsAutoGrowWidth()))
    { 
        NbcAdjustTextFrameWidthAndHeight();
    }
    if (!IsTextFrame())
    {
        
        SetRectsDirty(sal_True);
    }

    
    SetBoundRectDirty();
    ActionChanged();

    ImpSetTextStyleSheetListeners();
    ImpCheckMasterCachable();
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
            
            SetBoundRectDirty();
            SetRectsDirty(sal_True);
        }
        SetTextSizeDirty();
        ActionChanged();
        
        
        
        
        
        GetViewContact().flushViewObjectContacts(false);
    }
}

void SdrTextObj::ReformatText()
{
    if(GetOutlinerParaObject())
    {
        Rectangle aBoundRect0;
        if (pUserCall!=NULL)
            aBoundRect0=GetLastBoundRect();

        NbcReformatText();
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

SdrObjGeoData* SdrTextObj::NewGeoData() const
{
    return new SdrTextObjGeoData;
}

void SdrTextObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrAttrObj::SaveGeoData(rGeo);
    SdrTextObjGeoData& rTGeo=(SdrTextObjGeoData&)rGeo;
    rTGeo.aRect  =aRect;
    rTGeo.aGeo   =aGeo;
}

void SdrTextObj::RestGeoData(const SdrObjGeoData& rGeo)
{ 
    SdrAttrObj::RestGeoData(rGeo);
    SdrTextObjGeoData& rTGeo=(SdrTextObjGeoData&)rGeo;
    NbcSetLogicRect(rTGeo.aRect);
    aGeo   =rTGeo.aGeo;
    SetTextSizeDirty();
}

SdrFitToSizeType SdrTextObj::GetFitToSize() const
{
    SdrFitToSizeType eType = SDRTEXTFIT_NONE;

    if(!IsAutoGrowWidth())
        eType = ((SdrTextFitToSizeTypeItem&)(GetObjectItem(SDRATTR_TEXT_FITTOSIZE))).GetValue();

    return eType;
}

void SdrTextObj::ForceOutlinerParaObject()
{
    SdrText* pText = getActiveText();
    if( pText && (pText->GetOutlinerParaObject() == 0) )
    {
        sal_uInt16 nOutlMode = OUTLINERMODE_TEXTOBJECT;
        if( IsTextFrame() && eTextKind == OBJ_OUTLINETEXT )
            nOutlMode = OUTLINERMODE_OUTLINEOBJECT;

        pText->ForceOutlinerParaObject( nOutlMode );
    }
}

sal_Bool SdrTextObj::IsVerticalWriting() const
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

    return sal_False;
}

void SdrTextObj::SetVerticalWriting(sal_Bool bVertical)
{
    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
    if( !pOutlinerParaObject && bVertical )
    {
        
        
        ForceOutlinerParaObject();
        pOutlinerParaObject = GetOutlinerParaObject();
    }

    if( pOutlinerParaObject && (pOutlinerParaObject->IsVertical() != (bool)bVertical) )
    {
        
        const SfxItemSet& rSet = GetObjectItemSet();
        sal_Bool bAutoGrowWidth = ((SdrTextAutoGrowWidthItem&)rSet.Get(SDRATTR_TEXT_AUTOGROWWIDTH)).GetValue();
        sal_Bool bAutoGrowHeight = ((SdrTextAutoGrowHeightItem&)rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT)).GetValue();

        
        SdrTextHorzAdjust eHorz = ((SdrTextHorzAdjustItem&)(rSet.Get(SDRATTR_TEXT_HORZADJUST))).GetValue();
        SdrTextVertAdjust eVert = ((SdrTextVertAdjustItem&)(rSet.Get(SDRATTR_TEXT_VERTADJUST))).GetValue();

        
        Rectangle aObjectRect = GetSnapRect();

        
        SfxItemSet aNewSet(*rSet.GetPool(),
            SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
            
            SDRATTR_TEXT_VERTADJUST, SDRATTR_TEXT_VERTADJUST,
            SDRATTR_TEXT_AUTOGROWWIDTH, SDRATTR_TEXT_HORZADJUST,
            0, 0);

        aNewSet.Put(rSet);
        aNewSet.Put(SdrTextAutoGrowWidthItem(bAutoGrowHeight));
        aNewSet.Put(SdrTextAutoGrowHeightItem(bAutoGrowWidth));

        
        switch(eVert)
        {
            case SDRTEXTVERTADJUST_TOP: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT)); break;
            case SDRTEXTVERTADJUST_CENTER: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER)); break;
            case SDRTEXTVERTADJUST_BOTTOM: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT)); break;
            case SDRTEXTVERTADJUST_BLOCK: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK)); break;
        }
        switch(eHorz)
        {
            case SDRTEXTHORZADJUST_LEFT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BOTTOM)); break;
            case SDRTEXTHORZADJUST_CENTER: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER)); break;
            case SDRTEXTHORZADJUST_RIGHT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP)); break;
            case SDRTEXTHORZADJUST_BLOCK: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BLOCK)); break;
        }

        SetObjectItemSet(aNewSet);

        pOutlinerParaObject = GetOutlinerParaObject();
        if( pOutlinerParaObject )
        {
            
            pOutlinerParaObject->SetVertical(bVertical);
        }

        
        SetSnapRect(aObjectRect);
    }
}


//




//



sal_Bool SdrTextObj::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& /*rPolyPolygon*/) const
{
    
    double fRotate = (aGeo.nDrehWink / 100.0) * F_PI180;
    double fShearX = (aGeo.nShearWink / 100.0) * F_PI180;

    
    Rectangle aRectangle(aRect);

    
    basegfx::B2DTuple aScale(aRectangle.GetWidth(), aRectangle.GetHeight());
    basegfx::B2DTuple aTranslate(aRectangle.Left(), aRectangle.Top());

    
    if( pModel && pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate -= basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    
    const SfxMapUnit eMapUnit(GetObjectMapUnit());
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                
                aTranslate.setX(ImplTwipsToMM(aTranslate.getX()));
                aTranslate.setY(ImplTwipsToMM(aTranslate.getY()));

                
                aScale.setX(ImplTwipsToMM(aScale.getX()));
                aScale.setY(ImplTwipsToMM(aScale.getY()));

                break;
            }
            default:
            {
                OSL_FAIL("TRGetBaseGeometry: Missing unit translation to 100th mm!");
            }
        }
    }

    
    rMatrix = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
        aScale,
        basegfx::fTools::equalZero(fShearX) ? 0.0 : tan(fShearX),
        basegfx::fTools::equalZero(fRotate) ? 0.0 : -fRotate,
        aTranslate);

    return sal_False;
}




void SdrTextObj::TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& /*rPolyPolygon*/)
{
    
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate(0.0);
    double fShearX(0.0);
    rMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

    
    
    if(basegfx::fTools::less(aScale.getX(), 0.0) && basegfx::fTools::less(aScale.getY(), 0.0))
    {
        aScale.setX(fabs(aScale.getX()));
        aScale.setY(fabs(aScale.getY()));
        fRotate = fmod(fRotate + F_PI, F_2PI);
    }

    
    aGeo.nDrehWink = 0;
    aGeo.RecalcSinCos();
    aGeo.nShearWink = 0;
    aGeo.RecalcTan();

    
    const SfxMapUnit eMapUnit(GetObjectMapUnit());
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                
                aTranslate.setX(ImplMMToTwips(aTranslate.getX()));
                aTranslate.setY(ImplMMToTwips(aTranslate.getY()));

                
                aScale.setX(ImplMMToTwips(aScale.getX()));
                aScale.setY(ImplMMToTwips(aScale.getY()));

                break;
            }
            default:
            {
                OSL_FAIL("TRSetBaseGeometry: Missing unit translation to PoolMetric!");
            }
        }
    }

    
    if( pModel && pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate += basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    
    Point aPoint = Point();
    Size aSize(FRound(aScale.getX()), FRound(aScale.getY()));
    Rectangle aBaseRect(aPoint, aSize);
    SetSnapRect(aBaseRect);

    
    if(!basegfx::fTools::equalZero(fShearX))
    {
        GeoStat aGeoStat;
        aGeoStat.nShearWink = FRound((atan(fShearX) / F_PI180) * 100.0);
        aGeoStat.RecalcTan();
        Shear(Point(), aGeoStat.nShearWink, aGeoStat.nTan, false);
    }

    
    if(!basegfx::fTools::equalZero(fRotate))
    {
        GeoStat aGeoStat;

        
        
        
        aGeoStat.nDrehWink = NormAngle360(FRound(-fRotate / F_PI18000));
        aGeoStat.RecalcSinCos();
        Rotate(Point(), aGeoStat.nDrehWink, aGeoStat.nSin, aGeoStat.nCos);
    }

    
    if(!aTranslate.equalZero())
    {
        Move(Size(FRound(aTranslate.getX()), FRound(aTranslate.getY())));
    }
}

bool SdrTextObj::IsRealyEdited() const
{
    return pEdtOutl && pEdtOutl->IsModified();
}




long SdrTextObj::GetEckenradius() const
{
    return ((SdrEckenradiusItem&)(GetObjectItemSet().Get(SDRATTR_ECKENRADIUS))).GetValue();
}

long SdrTextObj::GetMinTextFrameHeight() const
{
    return ((SdrTextMinFrameHeightItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_MINFRAMEHEIGHT))).GetValue();
}

long SdrTextObj::GetMaxTextFrameHeight() const
{
    return ((SdrTextMaxFrameHeightItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_MAXFRAMEHEIGHT))).GetValue();
}

long SdrTextObj::GetMinTextFrameWidth() const
{
    return ((SdrTextMinFrameWidthItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_MINFRAMEWIDTH))).GetValue();
}

long SdrTextObj::GetMaxTextFrameWidth() const
{
    return ((SdrTextMaxFrameWidthItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_MAXFRAMEWIDTH))).GetValue();
}

bool SdrTextObj::IsFontwork() const
{
    return (bTextFrame) ? false 
        : ((XFormTextStyleItem&)(GetObjectItemSet().Get(XATTR_FORMTXTSTYLE))).GetValue()!=XFT_NONE;
}

bool SdrTextObj::IsHideContour() const
{
    return (bTextFrame) ? false 
        : ((XFormTextHideFormItem&)(GetObjectItemSet().Get(XATTR_FORMTXTHIDEFORM))).GetValue();
}

bool SdrTextObj::IsContourTextFrame() const
{
    return (bTextFrame) ? false 
        : ((SdrTextContourFrameItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_CONTOURFRAME))).GetValue();
}

long SdrTextObj::GetTextLeftDistance() const
{
    return ((SdrTextLeftDistItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_LEFTDIST))).GetValue();
}

long SdrTextObj::GetTextRightDistance() const
{
    return ((SdrTextRightDistItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_RIGHTDIST))).GetValue();
}

long SdrTextObj::GetTextUpperDistance() const
{
    return ((SdrTextUpperDistItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_UPPERDIST))).GetValue();
}

long SdrTextObj::GetTextLowerDistance() const
{
    return ((SdrTextLowerDistItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_LOWERDIST))).GetValue();
}

SdrTextAniKind SdrTextObj::GetTextAniKind() const
{
    return ((SdrTextAniKindItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_ANIKIND))).GetValue();
}

SdrTextAniDirection SdrTextObj::GetTextAniDirection() const
{
    return ((SdrTextAniDirectionItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();
}



GDIMetaFile* SdrTextObj::GetTextScrollMetaFileAndRectangle(
    Rectangle& rScrollRectangle, Rectangle& rPaintRectangle)
{
    GDIMetaFile* pRetval = 0L;
    SdrOutliner& rOutliner = ImpGetDrawOutliner();
    Rectangle aTextRect;
    Rectangle aAnchorRect;
    Rectangle aPaintRect;
    Fraction aFitXKorreg(1,1);
    bool bContourFrame(IsContourTextFrame());

    
    
    sal_Int32 nAngle(aGeo.nDrehWink);
    aGeo.nDrehWink = 0L;
    ImpSetupDrawOutlinerForPaint( bContourFrame, rOutliner, aTextRect, aAnchorRect, aPaintRect, aFitXKorreg );
    aGeo.nDrehWink = nAngle;

    Rectangle aScrollFrameRect(aPaintRect);
    const SfxItemSet& rSet = GetObjectItemSet();
    SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();

    if(SDRTEXTANI_LEFT == eDirection || SDRTEXTANI_RIGHT == eDirection)
    {
        aScrollFrameRect.Left() = aAnchorRect.Left();
        aScrollFrameRect.Right() = aAnchorRect.Right();
    }

    if(SDRTEXTANI_UP == eDirection || SDRTEXTANI_DOWN == eDirection)
    {
        aScrollFrameRect.Top() = aAnchorRect.Top();
        aScrollFrameRect.Bottom() = aAnchorRect.Bottom();
    }

    
    pRetval = new GDIMetaFile;
    VirtualDevice aBlackHole;
    aBlackHole.EnableOutput(false);
    pRetval->Record(&aBlackHole);
    Point aPaintPos = aPaintRect.TopLeft();

    rOutliner.Draw(&aBlackHole, aPaintPos);

    pRetval->Stop();
    pRetval->WindStart();

    
    rScrollRectangle = aScrollFrameRect;
    rPaintRectangle = aPaintRect;

    return pRetval;
}


bool SdrTextObj::IsAutoFit() const
{
    return GetFitToSize()==SDRTEXTFIT_AUTOFIT;
}

bool SdrTextObj::IsFitToSize() const
{
    const SdrFitToSizeType eFit=GetFitToSize();
    return (eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
}

void SdrTextObj::SetTextAnimationAllowed(sal_Bool bNew)
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
    const sal_uInt32 nStat = pEditStatus->GetStatusWord();
    const bool bGrowX=(nStat & EE_STAT_TEXTWIDTHCHANGED) !=0;
    const bool bGrowY=(nStat & EE_STAT_TEXTHEIGHTCHANGED) !=0;
    if(bTextFrame && (bGrowX || bGrowY))
    {
        if ((bGrowX && IsAutoGrowWidth()) || (bGrowY && IsAutoGrowHeight()))
        {
            AdjustTextFrameWidthAndHeight();
        }
        else if (IsAutoFit() && !mbInDownScale)
        {
            OSL_ASSERT(pEdtOutl);
            mbInDownScale = sal_True;

            
            
            
            ImpAutoFitText(*pEdtOutl);
            mbInDownScale = sal_False;
        }
    }
}

/** returns the currently active text. */
SdrText* SdrTextObj::getActiveText() const
{
    if( !mpText )
        return getText( 0 );
    else
        return mpText;
}

/** returns the nth available text. */
SdrText* SdrTextObj::getText( sal_Int32 nIndex ) const
{
    if( nIndex == 0 )
    {
        if( mpText == 0 )
            const_cast< SdrTextObj* >(this)->mpText = new SdrText( *(const_cast< SdrTextObj* >(this)) );
        return mpText;
    }
    else
    {
        return 0;
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


//














//


//






//












//



//









//









//


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
