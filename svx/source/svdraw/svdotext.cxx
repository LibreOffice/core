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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdotext.hxx>
#include "svx/svditext.hxx"
#include <svx/svdpagv.hxx>  // fuer Abfrage im Paint, ob das
#include <svx/svdview.hxx>  // Objekt gerade editiert wird
#include <svx/svdpage.hxx>  // und fuer AnimationHandler (Laufschrift)
#include <svx/svdetc.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>  // OutlinerDefaults
#include "svx/svdglob.hxx"  // Stringcache
#include "svx/svdstr.hrc"   // Objektname
#include <editeng/writingmodeitem.hxx>
#include <svx/sdtfchim.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outliner.hxx>
#include <editeng/fhgtitem.hxx>
#include <svl/itempool.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/flditem.hxx>
#include <svx/xftouit.hxx>
#include <vcl/salbtype.hxx>     // FRound
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

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// #104018# replace macros above with type-safe methods
inline double ImplTwipsToMM(double fVal) { return (fVal * (127.0 / 72.0)); }
inline double ImplMMToTwips(double fVal) { return (fVal * (72.0 / 127.0)); }

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@@ @@@@@ @@   @@ @@@@@@  @@@@  @@@@@  @@@@@@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@     @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@@@    @@@     @@   @@  @@ @@@@@      @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@ @@  @@
//    @@   @@@@@ @@   @@   @@    @@@@  @@@@@   @@@@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrTextObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::TextProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrTextObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfTextObj(*this);
}

//////////////////////////////////////////////////////////////////////////////

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

    // #101684#
    mbInEditMode = sal_False;

    // #111096#
    mbTextHidden = sal_False;

    // #111096#
    mbTextAnimationAllowed = sal_True;

    // #108784#
    maTextEditOffset = Point(0, 0);

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_True;
    mbInDownScale = sal_False;
}

SdrTextObj::SdrTextObj(const Rectangle& rNewRect)
:   SdrAttrObj(),
    aRect(rNewRect),
    mpText(NULL),
    pEdtOutl(NULL),
    pFormTextBoundRect(NULL)
{
    bTextSizeDirty=sal_False;
    bTextFrame=sal_False;
    bNoShear=sal_False;
    bNoRotate=sal_False;
    bNoMirror=sal_False;
    bDisableAutoWidthOnDragging=sal_False;
    ImpJustifyRect(aRect);

    // #101684#
    mbInEditMode = sal_False;

    // #111096#
    mbTextHidden = sal_False;

    // #111096#
    mbTextAnimationAllowed = sal_True;
    mbInDownScale = sal_False;

    // #108784#
    maTextEditOffset = Point(0, 0);

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_True;
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

    // #101684#
    mbInEditMode = sal_False;

    // #111096#
    mbTextHidden = sal_False;

    // #111096#
    mbTextAnimationAllowed = sal_True;
    mbInDownScale = sal_False;

    // #108784#
    maTextEditOffset = Point(0, 0);

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_True;
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

    // #101684#
    mbInEditMode = sal_False;

    // #111096#
    mbTextHidden = sal_False;

    // #111096#
    mbTextAnimationAllowed = sal_True;
    mbInDownScale = sal_False;

    // #108784#
    maTextEditOffset = Point(0, 0);

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_True;
}

SdrTextObj::SdrTextObj(SdrObjKind eNewTextKind, const Rectangle& rNewRect, SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat)
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

    NbcSetText(rInput, rBaseURL, eFormat);

    // #101684#
    mbInEditMode = sal_False;

    // #111096#
    mbTextHidden = sal_False;

    // #111096#
    mbTextAnimationAllowed = sal_True;
    mbInDownScale = sal_False;

    // #108784#
    maTextEditOffset = Point(0, 0);

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_True;
}

SdrTextObj::~SdrTextObj()
{
    if( pModel )
    {
        SdrOutliner& rOutl = pModel->GetHitTestOutliner();
        if( rOutl.GetTextObj() == this )
            rOutl.SetTextObj( NULL );
    }

    if(mpText!=NULL)
        delete mpText;

    if (pFormTextBoundRect!=NULL)
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
        Rectangle aTextRect;
        Size aNewSize(rOutliner.CalcTextSize());
        rOutliner.Clear();
        aNewSize.Width()++; // wegen evtl. Rundungsfehler
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

void SdrTextObj::NbcSetText(const XubString& rStr)
{
    SdrOutliner& rOutliner=ImpGetDrawOutliner();
    rOutliner.SetStyleSheet( 0, GetStyleSheet());
    //OutputDevice* pRef1=rOutliner.GetRefDevice();
    rOutliner.SetUpdateMode(sal_True);
    rOutliner.SetText(rStr,rOutliner.GetParagraph( 0 ));
    OutlinerParaObject* pNewText=rOutliner.CreateParaObject();
    Size aSiz(rOutliner.CalcTextSize());
    //OutputDevice* pRef2=rOutliner.GetRefDevice();
    rOutliner.Clear();
    NbcSetOutlinerParaObject(pNewText);
    aTextSize=aSiz;
    bTextSizeDirty=sal_False;
}

void SdrTextObj::SetText(const XubString& rStr)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    // #110094#-14 SendRepaintBroadcast();
    NbcSetText(rStr);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    //if (GetBoundRect()!=aBoundRect0) {
    //  SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    //}
}

void SdrTextObj::NbcSetText(SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat)
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

void SdrTextObj::SetText(SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    // #110094#-14 SendRepaintBroadcast();
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
        // 2x casting auf nonconst
        ((SdrTextObj*)this)->aTextSize=aSiz;
        ((SdrTextObj*)this)->bTextSizeDirty=sal_False;
    }
    return aTextSize;
}

bool SdrTextObj::IsAutoGrowHeight() const
{
    if(!bTextFrame)
        return sal_False; // AutoGrow nur bei TextFrames

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
        return sal_False; // AutoGrow nur bei TextFrames

    const SfxItemSet& rSet = GetObjectItemSet();
    sal_Bool bRet = ((SdrTextAutoGrowHeightItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWWIDTH))).GetValue();

    // #101684#
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

    // #101684#
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
} // defaults: BLOCK fuer Textrahmen, CENTER fuer beschriftete Grafikobjekte

SdrTextVertAdjust SdrTextObj::GetTextVerticalAdjust() const
{
    return GetTextVerticalAdjust(GetObjectItemSet());
}

SdrTextVertAdjust SdrTextObj::GetTextVerticalAdjust(const SfxItemSet& rSet) const
{
    if(IsContourTextFrame())
        return SDRTEXTVERTADJUST_TOP;

    // #103516# Take care for vertical text animation here
    SdrTextVertAdjust eRet = ((SdrTextVertAdjustItem&)(rSet.Get(SDRATTR_TEXT_VERTADJUST))).GetValue();
    sal_Bool bInEditMode = IsInEditMode();

    // #103516# Take care for vertical text animation here
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
} // defaults: TOP fuer Textrahmen, CENTER fuer beschriftete Grafikobjekte

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
    rInfo.bResizePropAllowed=sal_True;
    rInfo.bRotateFreeAllowed=sal_True;
    rInfo.bRotate90Allowed  =sal_True;
    rInfo.bMirrorFreeAllowed=bNoTextFrame;
    rInfo.bMirror45Allowed  =bNoTextFrame;
    rInfo.bMirror90Allowed  =bNoTextFrame;

    // allow transparence
    rInfo.bTransparenceAllowed = sal_True;

    // gradient depends on fillstyle
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetObjectItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.bGradientAllowed = (eFillStyle == XFILL_GRADIENT);
    rInfo.bShearAllowed     =bNoTextFrame;
    rInfo.bEdgeRadiusAllowed=sal_True;
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
        sal_uIntPtr nParaAnz=pOutliner->GetParagraphCount();
        if(p1stPara==NULL)
            nParaAnz=0;

        if(nParaAnz==1)
        {
            // if it is only one paragraph, check if that paragraph is empty
            XubString aStr(pOutliner->GetText(p1stPara));

            if(!aStr.Len())
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

bool SdrTextObj::NbcSetAutoGrowHeight(bool bAuto)
{
    if(bTextFrame)
    {
        SetObjectItem(SdrTextAutoGrowHeightItem(bAuto));
        return true;
    }
    return false;
}

bool SdrTextObj::NbcSetMinTextFrameHeight(long nHgt)
{
    if( bTextFrame && ( !pModel || !pModel->isLocked() ) )          // SJ: #i44922#
    {
        SetObjectItem(SdrTextMinFrameHeightItem(nHgt));

        // #84974# use bDisableAutoWidthOnDragging as
        // bDisableAutoHeightOnDragging if vertical.
        if(IsVerticalWriting() && bDisableAutoWidthOnDragging)
        {
            bDisableAutoWidthOnDragging = sal_False;
            SetObjectItem(SdrTextAutoGrowHeightItem(sal_False));
        }

        return true;
    }
    return false;
}

bool SdrTextObj::NbcSetMaxTextFrameHeight(long nHgt)
{
    if(bTextFrame)
    {
        SetObjectItem(SdrTextMaxFrameHeightItem(nHgt));
        return true;
    }
    return false;
}

bool SdrTextObj::NbcSetAutoGrowWidth(bool bAuto)
{
    if(bTextFrame)
    {
        SetObjectItem(SdrTextAutoGrowWidthItem(bAuto));
        return true;
    }
    return false;
}

bool SdrTextObj::NbcSetMinTextFrameWidth(long nWdt)
{
    if( bTextFrame && ( !pModel || !pModel->isLocked() ) )          // SJ: #i44922#
    {
        SetObjectItem(SdrTextMinFrameWidthItem(nWdt));

        // #84974# use bDisableAutoWidthOnDragging only
        // when not vertical.
        if(!IsVerticalWriting() && bDisableAutoWidthOnDragging)
        {
            bDisableAutoWidthOnDragging = sal_False;
            SetObjectItem(SdrTextAutoGrowWidthItem(sal_False));
        }

        return true;
    }
    return false;
}

bool SdrTextObj::NbcSetMaxTextFrameWidth(long nWdt)
{
    if(bTextFrame)
    {
        SetObjectItem(SdrTextMaxFrameWidthItem(nWdt));
        return true;
    }
    return false;
}

bool SdrTextObj::NbcSetFitToSize(SdrFitToSizeType eFit)
{
    if(bTextFrame)
    {
        SetObjectItem(SdrTextFitToSizeTypeItem(eFit));
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
        // Unrotate!
        aMatrix.rotate(-aGeo.nDrehWink * nPi180);
    }

    aXorPolyPolygon.transform(aMatrix);

    if( bLineWidth )
    {
        // Strichstaerke beruecksichtigen
        // Beim Hittest muss das unterbleiben (Performance!)
        pContourPolyPolygon = new basegfx::B2DPolyPolygon();

        // #86258# test if shadow needs to be avoided for TakeContour()
        const SfxItemSet& rSet = GetObjectItemSet();
        sal_Bool bShadowOn = ((SdrShadowItem&)(rSet.Get(SDRATTR_SHADOW))).GetValue();

        // #i33696#
        // Remember TextObject currently set at the DrawOutliner, it WILL be
        // replaced during calculating the outline since it uses an own paint
        // and that one uses the DrawOutliner, too.
        const SdrTextObj* pLastTextObject = rOutliner.GetTextObj();

        if(bShadowOn)
        {
            // #86258# force shadow off
            SdrObject* pCopy = Clone();
            pCopy->SetMergedItem(SdrShadowItem(sal_False));
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

    rOutliner.SetPolygon(aXorPolyPolygon, pContourPolyPolygon);
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
    Rectangle aAnkRect(aRect); // Rect innerhalb dem geankert wird
    bool bFrame=IsTextFrame();
    if (!bFrame) {
        TakeUnrotatedSnapRect(aAnkRect);
    }
    Point aRotateRef(aAnkRect.TopLeft());
    aAnkRect.Left()+=nLeftDist;
    aAnkRect.Top()+=nUpperDist;
    aAnkRect.Right()-=nRightDist;
    aAnkRect.Bottom()-=nLowerDist;

    // #108816#
    // Since sizes may be bigger than the object bounds it is necessary to
    // justify the rect now.
    ImpJustifyRect(aAnkRect);

    if (bFrame) {
        // !!! hier noch etwas verfeinern !!!
        if (aAnkRect.GetWidth()<2) aAnkRect.Right()=aAnkRect.Left()+1;   // Mindestgroesse 2
        if (aAnkRect.GetHeight()<2) aAnkRect.Bottom()=aAnkRect.Top()+1;  // Mindestgroesse 2
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
    Rectangle aAnkRect; // Rect innerhalb dem geankert wird
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

            // #101684#
            sal_Bool bInEditMode = IsInEditMode();

            if (!bInEditMode && (eAniKind==SDRTEXTANI_SCROLL || eAniKind==SDRTEXTANI_ALTERNATE || eAniKind==SDRTEXTANI_SLIDE))
            {
                // Grenzenlose Papiergroesse fuer Laufschrift
                if (eAniDirection==SDRTEXTANI_LEFT || eAniDirection==SDRTEXTANI_RIGHT) nWdt=1000000;
                if (eAniDirection==SDRTEXTANI_UP || eAniDirection==SDRTEXTANI_DOWN) nHgt=1000000;
            }
            rOutliner.SetMaxAutoPaperSize(Size(nWdt,nHgt));
        }

        // #103516# New try with _BLOCK for hor and ver after completely
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
    OutlinerParaObject* pOutlinerParaObject = pText ? pText->GetOutlinerParaObject() : 0;
    OutlinerParaObject* pPara = (pEdtOutl && !bNoEditText) ? pEdtOutl->CreateParaObject() : pOutlinerParaObject;

    if (pPara)
    {
        sal_Bool bHitTest = sal_False;
        if( pModel )
            bHitTest = &pModel->GetHitTestOutliner() == &rOutliner;

        const SdrTextObj* pTestObj = rOutliner.GetTextObj();
        if( !pTestObj || !bHitTest || pTestObj != this ||
            pTestObj->GetOutlinerParaObject() != pOutlinerParaObject )
        {
            if( bHitTest ) // #i33696# take back fix #i27510#
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
    Size aTextSiz(rOutliner.GetPaperSize()); // GetPaperSize() hat etwas Toleranz drauf, oder?

    // #106653#
    // For draw objects containing text correct hor/ver alignment if text is bigger
    // than the object itself. Without that correction, the text would always be
    // formatted to the left edge (or top edge when vertical) of the draw object.
    if(!IsTextFrame())
    {
        if(aAnkRect.GetWidth() < aTextSiz.Width() && !IsVerticalWriting())
        {
            // #110129#
            // Horizontal case here. Correct only if eHAdj == SDRTEXTHORZADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
            {
                eHAdj = SDRTEXTHORZADJUST_CENTER;
            }
        }

        if(aAnkRect.GetHeight() < aTextSiz.Height() && IsVerticalWriting())
        {
            // #110129#
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

    // rTextRect ist bei ContourFrame in einigen Faellen nicht korrekt
    rTextRect=Rectangle(aTextPos,aTextSiz);
    if (bContourFrame)
        rTextRect=aAnkRect;
}

OutlinerParaObject* SdrTextObj::GetEditOutlinerParaObject() const
{
    OutlinerParaObject* pPara=NULL;
    if( HasTextImpl( pEdtOutl ) )
    {
        sal_uInt16 nParaAnz = static_cast< sal_uInt16 >( pEdtOutl->GetParagraphCount() );
        pPara = pEdtOutl->CreateParaObject(0, nParaAnz);
    }
    return pPara;
}

void SdrTextObj::ImpSetCharStretching(SdrOutliner& rOutliner, const Size& rTextSize, const Size& rShapeSize, Fraction& rFitXKorreg) const
{
    OutputDevice* pOut = rOutliner.GetRefDevice();
    sal_Bool bNoStretching(sal_False);

    if(pOut && pOut->GetOutDevType() == OUTDEV_PRINTER)
    {
        // #35762#: Checken ob CharStretching ueberhaupt moeglich
        GDIMetaFile* pMtf = pOut->GetConnectMetaFile();
        UniString aTestString(sal_Unicode('J'));

        if(pMtf && (!pMtf->IsRecord() || pMtf->IsPause()))
            pMtf = NULL;

        if(pMtf)
            pMtf->Pause(sal_True);

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
            pMtf->Pause(sal_False);

        bNoStretching = (aSize1 == aSize2);

#ifdef WNT
        // #35762# Windows vergroessert bei Size(100,500) den Font proportional
        // Und das finden wir nicht so schoen.
        if(aSize2.Height() >= aSize1.Height() * 2)
        {
            bNoStretching = sal_True;
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

    long nXTolPl=nWantWdt/100; // Toleranz +1%
    long nXTolMi=nWantWdt/25;  // Toleranz -4%
    long nXKorr =nWantWdt/20;  // Korrekturmasstab 5%

    long nX=(nWantWdt*100) /nIsWdt; // X-Stretching berechnen
    long nY=(nWantHgt*100) /nIsHgt; // Y-Stretching berechnen
    bool bChkX = true;
    if (bNoStretching) { // #35762# evtl. nur proportional moeglich
        if (nX>nY) { nX=nY; bChkX=sal_False; }
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

        // #87877# exception, there is no text yet (vertical case)
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
        if (((nXDiff>=nXTolMi || !bChkX) && nXDiff<=nXTolPl) || nXDiff==nXDiff0/*&& Abs(nYDiff)<=nYTol*/) {
            bNoMoreLoop = true;
        } else {
            // Stretchingfaktoren korregieren
            long nMul=nWantWdt;
            long nDiv=aSiz.Width();
            if (Abs(nXDiff)<=2*nXKorr) {
                if (nMul>nDiv) nDiv+=(nMul-nDiv)/2; // und zwar nur um die haelfte des berechneten
                else nMul+=(nDiv-nMul)/2;           // weil die EE ja eh wieder falsch rechnet
            }
            nX=nX*nMul/nDiv;
            if (bNoStretching) nY=nX;
        }
        nXDiff0=nXDiff;
    }
}

void SdrTextObj::StartTextAnimation(OutputDevice* /*pOutDev*/, const Point& /*rOffset*/, long /*nExtraData*/)
{
    // #111096#
    // use new text animation
    SetTextAnimationAllowed(sal_True);
}

void SdrTextObj::StopTextAnimation(OutputDevice* /*pOutDev*/, long /*nExtraData*/)
{
    // #111096#
    // use new text animation
    SetTextAnimationAllowed(sal_False);
}

void SdrTextObj::TakeObjNameSingul(XubString& rName) const
{
    XubString aStr;

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
        // Macht bei OUTLINETEXT wohl derzeit noch etwas Probleme
        XubString aStr2(pOutlinerParaObject->GetTextObject().GetText(0));
        aStr2.EraseLeadingChars();

        // #69446# avoid non expanded text portions in object name
        // (second condition is new)
        if(aStr2.Len() && aStr2.Search(sal_Unicode(255)) == STRING_NOTFOUND)
        {
            // #76681# space between ResStr and content text
            aStr += sal_Unicode(' ');

            aStr += sal_Unicode('\'');

            if(aStr2.Len() > 10)
            {
                aStr2.Erase(8);
                aStr2.AppendAscii("...", 3);
            }

            aStr += aStr2;
            aStr += sal_Unicode('\'');
        }
    }

    rName = aStr;

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }

}

void SdrTextObj::TakeObjNamePlural(XubString& rName) const
{
    switch (eTextKind) {
        case OBJ_OUTLINETEXT: rName=ImpGetResStr(STR_ObjNamePluralOUTLINETEXT); break;
        case OBJ_TITLETEXT  : rName=ImpGetResStr(STR_ObjNamePluralTITLETEXT);   break;
        default: {
            if (IsLinkedText()) {
                rName=ImpGetResStr(STR_ObjNamePluralTEXTLNK);
            } else {
                rName=ImpGetResStr(STR_ObjNamePluralTEXT);
            }
        } break;
    } // switch
}

void SdrTextObj::operator=(const SdrObject& rObj)
{
    // call parent
    SdrObject::operator=(rObj);

    const SdrTextObj* pTextObj = dynamic_cast< const SdrTextObj* >( &rObj );
    if (pTextObj!=NULL)
    {
        aRect     =pTextObj->aRect;
        aGeo      =pTextObj->aGeo;
        eTextKind =pTextObj->eTextKind;
        bTextFrame=pTextObj->bTextFrame;
        aTextSize=pTextObj->aTextSize;
        bTextSizeDirty=pTextObj->bTextSizeDirty;

        // #101776# Not all of the necessary parameters were copied yet.
        bNoShear = pTextObj->bNoShear;
        bNoRotate = pTextObj->bNoRotate;
        bNoMirror = pTextObj->bNoMirror;
        bDisableAutoWidthOnDragging = pTextObj->bDisableAutoWidthOnDragging;

        OutlinerParaObject* pNewOutlinerParaObject = 0;

        SdrText* pText = getActiveText();

        if( pText && pTextObj->HasText() )
        {
            const Outliner* pEO=pTextObj->pEdtOutl;
            if (pEO!=NULL)
            {
                pNewOutlinerParaObject = pEO->CreateParaObject();
            }
            else
            {
                pNewOutlinerParaObject = new OutlinerParaObject(*pTextObj->getActiveText()->GetOutlinerParaObject());
            }
        }

        mpText->SetOutlinerParaObject( pNewOutlinerParaObject );
        ImpSetTextStyleSheetListeners();
    }
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

    // und nun noch ggf. das BoundRect des Textes dazu
    if ( pModel && GetOutlinerParaObject() && !IsFontwork() && !IsContourTextFrame() )
    {
        // #80328# using Clone()-Paint() strategy inside TakeContour() leaves a destroyed
        // SdrObject as pointer in DrawOutliner. Set *this again in fetching the outliner
        // in every case
        SdrOutliner& rOutliner=ImpGetDrawOutliner();

        Rectangle aAnchor2;
        Rectangle aR;
        TakeTextRect(rOutliner,aR,sal_False,&aAnchor2);
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
    bNotMasterCachable=sal_False;

    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();

    if(!bNotVisibleAsMaster && pOutlinerParaObject && pOutlinerParaObject->IsEditDoc() )
    {
        const EditTextObject& rText= pOutlinerParaObject->GetTextObject();
        bNotMasterCachable=rText.HasField(SvxPageField::StaticType());
        if( !bNotMasterCachable )
        {
            bNotMasterCachable=rText.HasField(SvxHeaderField::StaticType());
            if( !bNotMasterCachable )
            {
                bNotMasterCachable=rText.HasField(SvxFooterField::StaticType());
                if( !bNotMasterCachable )
                {
                    bNotMasterCachable=rText.HasField(SvxDateTimeField::StaticType());
                }
            }
        }
    }
}

// #101029#: Extracted from ImpGetDrawOutliner()
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

    // #101029#: Code extracted to ImpInitDrawOutliner()
    ImpInitDrawOutliner( rOutl );

    return rOutl;
}

boost::shared_ptr< SdrOutliner > SdrTextObj::CreateDrawOutliner()
{
    boost::shared_ptr< SdrOutliner > xDrawOutliner( pModel->CreateDrawOutliner(this) );
    ImpInitDrawOutliner( *(xDrawOutliner.get()) );
    return xDrawOutliner;
}

// #101029#: Extracted from Paint()
void SdrTextObj::ImpSetupDrawOutlinerForPaint( bool             bContourFrame,
                                               SdrOutliner&     rOutliner,
                                               Rectangle&       rTextRect,
                                               Rectangle&       rAnchorRect,
                                               Rectangle&       rPaintRect,
                                               Fraction&        rFitXKorreg ) const
{
    if (!bContourFrame)
    {
        // FitToSize erstmal nicht mit ContourFrame
        if (IsFitToSize() || IsAutoFit())
        {
            sal_uIntPtr nStat=rOutliner.GetControlWord();
            nStat|=EE_CNTRL_STRETCHING|EE_CNTRL_AUTOPAGESIZE;
            rOutliner.SetControlWord(nStat);
        }
    }
    rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
    TakeTextRect(rOutliner, rTextRect, sal_False, &rAnchorRect);
    rPaintRect = rTextRect;

    if (!bContourFrame)
    {
        // FitToSize erstmal nicht mit ContourFrame
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
    // EditEngine formatting is unstable enough for
    // line-breaking text that we need some more samples

    // loop early-exits if we detect an already attained value
    sal_uInt16 nMinStretchX=0, nMinStretchY=0;
    sal_uInt16 aOldStretchXVals[]={0,0,0,0,0,0,0,0,0,0};
    const size_t aStretchArySize=sizeof(aOldStretchXVals)/sizeof(*aOldStretchXVals);
    for(unsigned int i=0; i<aStretchArySize; ++i)
    {
        const Size aCurrTextSize = rOutliner.CalcTextSize();
        double fFactor(1.0);
        if( bIsVerticalWriting )
            fFactor = double(rTextSize.Width())/aCurrTextSize.Width();
        else
            fFactor = double(rTextSize.Height())/aCurrTextSize.Height();

        sal_uInt16 nCurrStretchX, nCurrStretchY;
        rOutliner.GetGlobalCharStretching(nCurrStretchX, nCurrStretchY);

        if (fFactor >= 1.0 )
        {
            // resulting text area fits into available shape rect -
            // err on the larger streching, to optimally fill area
            nMinStretchX = std::max(nMinStretchX,nCurrStretchX);
            nMinStretchY = std::max(nMinStretchY,nCurrStretchY);
        }

        aOldStretchXVals[i] = nCurrStretchX;
        if( std::find(aOldStretchXVals, aOldStretchXVals+i, nCurrStretchX) != aOldStretchXVals+i )
            break; // same value already attained once; algo is looping, exit

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

////////////////////////////////////////////////////////////////////////////////////////////////////

OutlinerParaObject* SdrTextObj::GetOutlinerParaObject() const
{
    SdrText* pText = getActiveText();
    if( pText )
        return pText->GetOutlinerParaObject();
    else
        return 0;
}

bool SdrTextObj::HasOutlinerParaObject() const
{
    SdrText* pText = getActiveText();
    if( pText && pText->GetOutlinerParaObject() )
        return true;
    return false;
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
    { // Textrahmen anpassen!
        NbcAdjustTextFrameWidthAndHeight();
    }
    if (!IsTextFrame())
    {
        // Das SnapRect behaelt seine Groesse bei
        SetRectsDirty(sal_True);
    }

    // always invalidate BoundRect on change
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
            // Das SnapRect behaelt seine Groesse bei
            SetBoundRectDirty();
            SetRectsDirty(sal_True);
        }
        SetTextSizeDirty();
        ActionChanged();
        // FME, AW: i22396
        // Necessary here since we have no compare operator at the outliner
        // para object which may detect changes regarding the combination
        // of outliner para data and configuration (e.g., change of
        // formatting of text numerals)
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

        // #110094#-14 SendRepaintBroadcast();
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
{ // RectsDirty wird von SdrObject gerufen
    SdrAttrObj::RestGeoData(rGeo);
    SdrTextObjGeoData& rTGeo=(SdrTextObjGeoData&)rGeo;
    aRect  =rTGeo.aRect;
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
    // #89459#
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
        // we only need to force a outliner para object if the default of
        // horizontal text is changed
        ForceOutlinerParaObject();
        pOutlinerParaObject = GetOutlinerParaObject();
    }

    if( pOutlinerParaObject && (pOutlinerParaObject->IsVertical() != (bool)bVertical) )
    {
        // get item settings
        const SfxItemSet& rSet = GetObjectItemSet();
        sal_Bool bAutoGrowWidth = ((SdrTextAutoGrowWidthItem&)rSet.Get(SDRATTR_TEXT_AUTOGROWWIDTH)).GetValue();
        sal_Bool bAutoGrowHeight = ((SdrTextAutoGrowHeightItem&)rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT)).GetValue();

        // #103516# Also exchange hor/ver adjust items
        SdrTextHorzAdjust eHorz = ((SdrTextHorzAdjustItem&)(rSet.Get(SDRATTR_TEXT_HORZADJUST))).GetValue();
        SdrTextVertAdjust eVert = ((SdrTextVertAdjustItem&)(rSet.Get(SDRATTR_TEXT_VERTADJUST))).GetValue();

        // rescue object size
        Rectangle aObjectRect = GetSnapRect();

        // prepare ItemSet to set exchanged width and height items
        SfxItemSet aNewSet(*rSet.GetPool(),
            SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
            // #103516# Expanded item ranges to also support hor and ver adjust.
            SDRATTR_TEXT_VERTADJUST, SDRATTR_TEXT_VERTADJUST,
            SDRATTR_TEXT_AUTOGROWWIDTH, SDRATTR_TEXT_HORZADJUST,
            0, 0);

        aNewSet.Put(rSet);
        aNewSet.Put(SdrTextAutoGrowWidthItem(bAutoGrowHeight));
        aNewSet.Put(SdrTextAutoGrowHeightItem(bAutoGrowWidth));

        // #103516# Exchange horz and vert adjusts
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
            // set ParaObject orientation accordingly
            pOutlinerParaObject->SetVertical(bVertical);
        }

        // restore object size
        SetSnapRect(aObjectRect);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// transformation interface for StarOfficeAPI. This implements support for
// homogen 3x3 matrices containing the transformation of the SdrObject. At the
// moment it contains a shearX, rotation and translation, but for setting all linear
// transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
// gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
// with the base geometry and returns TRUE. Otherwise it returns FALSE.
sal_Bool SdrTextObj::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& /*rPolyPolygon*/) const
{
    // get turn and shear
    double fRotate = (aGeo.nDrehWink / 100.0) * F_PI180;
    double fShearX = (aGeo.nShearWink / 100.0) * F_PI180;

    // get aRect, this is the unrotated snaprect
    Rectangle aRectangle(aRect);

    // fill other values
    basegfx::B2DTuple aScale(aRectangle.GetWidth(), aRectangle.GetHeight());
    basegfx::B2DTuple aTranslate(aRectangle.Left(), aRectangle.Top());

    // position maybe relative to anchorpos, convert
    if( pModel && pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate -= basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // force MapUnit to 100th mm
    SfxMapUnit eMapUnit = GetObjectItemSet().GetPool()->GetMetric(0);
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // postion
                aTranslate.setX(ImplTwipsToMM(aTranslate.getX()));
                aTranslate.setY(ImplTwipsToMM(aTranslate.getY()));

                // size
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

    // build matrix
    rMatrix = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
        aScale,
        basegfx::fTools::equalZero(fShearX) ? 0.0 : tan(fShearX),
        basegfx::fTools::equalZero(fRotate) ? 0.0 : -fRotate,
        aTranslate);

    return sal_False;
}

// sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
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

    // #i75086# Old DrawingLayer (GeoStat and geometry) does not support holding negative scalings
    // in X and Y which equal a 180 degree rotation. Recognize it and react accordingly
    if(basegfx::fTools::less(aScale.getX(), 0.0) && basegfx::fTools::less(aScale.getY(), 0.0))
    {
        aScale.setX(fabs(aScale.getX()));
        aScale.setY(fabs(aScale.getY()));
        fRotate = fmod(fRotate + F_PI, F_2PI);
    }

    // reset object shear and rotations
    aGeo.nDrehWink = 0;
    aGeo.RecalcSinCos();
    aGeo.nShearWink = 0;
    aGeo.RecalcTan();

    // force metric to pool metric
    SfxMapUnit eMapUnit = GetObjectItemSet().GetPool()->GetMetric(0);
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                aTranslate.setX(ImplMMToTwips(aTranslate.getX()));
                aTranslate.setY(ImplMMToTwips(aTranslate.getY()));

                // size
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

    // if anchor is used, make position relative to it
    if( pModel && pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate += basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // build and set BaseRect (use scale)
    Point aPoint = Point();
    Size aSize(FRound(aScale.getX()), FRound(aScale.getY()));
    Rectangle aBaseRect(aPoint, aSize);
    SetSnapRect(aBaseRect);

    // shear?
    if(!basegfx::fTools::equalZero(fShearX))
    {
        GeoStat aGeoStat;
        aGeoStat.nShearWink = FRound((atan(fShearX) / F_PI180) * 100.0);
        aGeoStat.RecalcTan();
        Shear(Point(), aGeoStat.nShearWink, aGeoStat.nTan, sal_False);
    }

    // rotation?
    if(!basegfx::fTools::equalZero(fRotate))
    {
        GeoStat aGeoStat;

        // #i78696#
        // fRotate is matematically correct, but aGeoStat.nDrehWink is
        // mirrored -> mirror value here
        aGeoStat.nDrehWink = NormAngle360(FRound(-fRotate / F_PI18000));
        aGeoStat.RecalcSinCos();
        Rotate(Point(), aGeoStat.nDrehWink, aGeoStat.nSin, aGeoStat.nCos);
    }

    // translate?
    if(!aTranslate.equalZero())
    {
        Move(Size(FRound(aTranslate.getX()), FRound(aTranslate.getY())));
    }
}

bool SdrTextObj::IsRealyEdited() const
{
    return pEdtOutl && pEdtOutl->IsModified();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// moved inlines here form hxx

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
    return (bTextFrame) ? false // Default ist FALSE
        : ((XFormTextStyleItem&)(GetObjectItemSet().Get(XATTR_FORMTXTSTYLE))).GetValue()!=XFT_NONE;
}

bool SdrTextObj::IsHideContour() const
{
    return (bTextFrame) ? false // Default ist: Nein, kein HideContour; HideContour nicht bei TextFrames
        : ((XFormTextHideFormItem&)(GetObjectItemSet().Get(XATTR_FORMTXTHIDEFORM))).GetValue();
}

bool SdrTextObj::IsContourTextFrame() const
{
    return (bTextFrame) ? false // ContourFrame nicht bei normalen TextFrames
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

// #111096#
// Access to thext hidden flag
sal_Bool SdrTextObj::GetTextHidden() const
{
    return mbTextHidden;
}

void SdrTextObj::NbcSetTextHidden(sal_Bool bNew)
{
    if(bNew != mbTextHidden)
    {
        mbTextHidden = bNew;
    }
}

// #111096#
// Get necessary data for text scroll animation. ATM base it on a Text-Metafile and a
// painting rectangle. Rotation is excluded from the returned values.
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

    // get outliner set up. To avoid getting a somehow rotated MetaFile,
    // temporarily disable object rotation.
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

    // create the MetaFile
    pRetval = new GDIMetaFile;
    VirtualDevice aBlackHole;
    aBlackHole.EnableOutput(sal_False);
    pRetval->Record(&aBlackHole);
    Point aPaintPos = aPaintRect.TopLeft();

    rOutliner.Draw(&aBlackHole, aPaintPos);

    pRetval->Stop();
    pRetval->WindStart();

    // return PaintRectanglePixel and pRetval;
    rScrollRectangle = aScrollFrameRect;
    rPaintRectangle = aPaintRect;

    return pRetval;
}

// #111096#
// Access to TextAnimationAllowed flag
bool SdrTextObj::IsTextAnimationAllowed() const
{
    return mbTextAnimationAllowed;
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

            // sucks that we cannot disable paints via
            // pEdtOutl->SetUpdateMode(FALSE) - but EditEngine skips
            // formatting as well, then.
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

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Konzept des TextObjekts:
// ~~~~~~~~~~~~~~~~~~~~~~~~
// Attribute/Varianten:
// - sal_Bool Textrahmen / beschriftetes Zeichenobjekt
// - sal_Bool FontWork                 (wenn nicht Textrahmen und nicht ContourTextFrame)
// - sal_Bool ContourTextFrame         (wenn nicht Textrahmen und nicht Fontwork)
// - long Drehwinkel               (wenn nicht FontWork)
// - long Textrahmenabstaende      (wenn nicht FontWork)
// - sal_Bool FitToSize                (wenn nicht FontWork)
// - sal_Bool AutoGrowingWidth/Height  (wenn nicht FitToSize und nicht FontWork)
// - long Min/MaxFrameWidth/Height (wenn AutoGrowingWidth/Height)
// - enum Horizontale Textverankerung Links,Mitte,Rechts,Block,Stretch(ni)
// - enum Vertikale Textverankerung Oben,Mitte,Unten,Block,Stretch(ni)
// - enum Laufschrift              (wenn nicht FontWork)
//
// Jedes abgeleitete Objekt ist entweder ein Textrahmen (bTextFrame=sal_True)
// oder ein beschriftetes Zeichenobjekt (bTextFrame=sal_False).
//
// Defaultverankerung von Textrahmen:
//   SDRTEXTHORZADJUST_BLOCK, SDRTEXTVERTADJUST_TOP
//   = statische Pooldefaults
// Defaultverankerung von beschrifteten Zeichenobjekten:
//   SDRTEXTHORZADJUST_CENTER, SDRTEXTVERTADJUST_CENTER
//   durch harte Attributierung von SdrAttrObj
//
// Jedes vom SdrTextObj abgeleitete Objekt muss ein "UnrotatedSnapRect"
// (->TakeUnrotatedSnapRect()) liefern (Drehreferenz ist TopLeft dieses
// Rechtecks (aGeo.nDrehWink)), welches die Grundlage der Textverankerung
// bildet. Von diesem werden dann ringsum die Textrahmenabstaende abgezogen;
// das Ergebnis ist der Ankerbereich (->TakeTextAnchorRect()). Innerhalb
// dieses Bereichs wird dann in Abhaengigkeit von der horizontalen und
// vertikalen Ausrichtung (SdrTextVertAdjust,SdrTextHorzAdjust) der Ankerpunkt
// sowie der Ausgabebereich bestimmt. Bei beschrifteten Grafikobjekten kann
// der Ausgabebereich durchaus groesser als der Ankerbereich werden, bei
// Textrahmen ist er stets kleiner oder gleich (ausser bei negativen Textrahmen-
// abstaenden).
//
// FitToSize hat Prioritaet vor Textverankerung und AutoGrowHeight/Width. Der
// Ausgabebereich ist bei FitToSize immer genau der Ankerbereich. Weiterhin
// gibt es bei FitToSize keinen automatischen Zeilenumbruch.
//
// ContourTextFrame:
// - long Drehwinkel
// - long Textrahmenabstaende         spaeter vielleicht
// - sal_Bool FitToSize                   spaeter vielleicht
// - sal_Bool AutoGrowingWidth/Height     viel spaeter vielleicht
// - long Min/MaxFrameWidth/Height    viel spaeter vielleicht
// - enum Horizontale Textverankerung spaeter vielleicht, erstmal Links, Absatz zentr.
// - enum Vertikale Textverankerung   spaeter vielleicht, erstmal oben
// - enum Laufschrift                 spaeter vielleicht (evtl. sogar mit korrektem Clipping)
//
// Bei Aenderungen zu beachten:
// - Paint
// - HitTest
// - ConvertToPoly
// - Edit
// - Drucken,Speichern, Paint in Nachbarview waerend Edit
// - ModelChanged (z.B. durch NachbarView oder Lineale) waerend Edit
// - FillColorChanged waerend Edit
// - uvm...
//
/////////////////////////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
