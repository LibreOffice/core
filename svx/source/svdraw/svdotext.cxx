/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdotext.hxx>
#include <editeng/editdata.hxx>
#include <svx/svditext.hxx>
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
#include <svx/svdlegacy.hxx>
#include <svx/svdtrans.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdrtexthelpers.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

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

SdrTextObj::SdrTextObj(
    SdrModel& rSdrModel,
    const basegfx::B2DHomMatrix& rTransform,
    SdrObjKind eNewTextKind,
    bool bIsTextFrame)
:   SdrAttrObj(rSdrModel, rTransform),
    mpText(0),
    pEdtOutl(0),
    eTextKind(eNewTextKind),
    maTextEditOffset()
{
    bTextSizeDirty = false;
    bTextFrame = bIsTextFrame;
    bNoShear = true;
    bNoRotate = false;
    bNoMirror = true;
    bDisableAutoWidthOnDragging = false;

    // #101684#
    mbInEditMode = false;

    // #111096#
    mbTextAnimationAllowed = sal_True;
    mbAdaptingTextMinSize = false;
    mbAdjustingTextFrameWidthAndHeight = false;
}

SdrTextObj::~SdrTextObj()
{
    if(mpText)
    {
        delete mpText;
    }

    ImpLinkAbmeldung();
}

void SdrTextObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrTextObj* pSource = dynamic_cast< const SdrTextObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrAttrObj::copyDataFromSdrObject(rSource);

            // copy local data
            eTextKind = pSource->eTextKind;
            bTextFrame = pSource->bTextFrame;
            aTextSize = pSource->aTextSize;
            bTextSizeDirty = pSource->bTextSizeDirty;
            bNoShear = pSource->bNoShear;
            bNoRotate = pSource->bNoRotate;
            bNoMirror = pSource->bNoMirror;
            bDisableAutoWidthOnDragging = pSource->bDisableAutoWidthOnDragging;
            OutlinerParaObject* pNewOutlinerParaObject = 0;
            SdrText* pText = getActiveText();

            if(pText && pSource->HasText())
            {
                const Outliner* pEO = pSource->pEdtOutl;

                if(pEO)
                {
                    pNewOutlinerParaObject = pEO->CreateParaObject();
                }
                else
                {
                    pNewOutlinerParaObject = new OutlinerParaObject(*pSource->getActiveText()->GetOutlinerParaObject());
                }
            }

            mpText->SetOutlinerParaObject(pNewOutlinerParaObject);

            if(pNewOutlinerParaObject && &rSource.getSdrModelFromSdrObject() != &getSdrModelFromSdrObject())
            {
                // It is a clone to another model
                mpText->ImpModelChange(rSource.getSdrModelFromSdrObject(), getSdrModelFromSdrObject());
            }

            ImpSetTextStyleSheetListeners();
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SdrTextObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrTextObj* pClone = new SdrTextObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

bool SdrTextObj::DoesSupportTextIndentingOnLineWidthChange() const
{
    return true;
}

void SdrTextObj::FitFrameToTextSize()
{
    SdrText* pText = getActiveText();

    if( pText && pText->GetOutlinerParaObject())
    {
        SdrOutliner& rOutliner=ImpGetDrawOutliner();
        const Rectangle aLogicRect(sdr::legacy::GetLogicRect(*this));

        rOutliner.SetPaperSize(Size(aLogicRect.Right()-aLogicRect.Left(),aLogicRect.Bottom()-aLogicRect.Top()));
        rOutliner.SetUpdateMode(true);
        rOutliner.SetText(*pText->GetOutlinerParaObject());

        Size aNewSize(rOutliner.CalcTextSize());

        rOutliner.Clear();
        aNewSize.Width()++; // wegen evtl. Rundungsfehler
        aNewSize.Width()+=GetTextLeftDistance()+GetTextRightDistance();
        aNewSize.Height()+=GetTextUpperDistance()+GetTextLowerDistance();
        Rectangle aNewRect(aLogicRect);
        aNewRect.SetSize(aNewSize);

        if (aNewRect!=aLogicRect)
        {
            sdr::legacy::SetLogicRect(*this, aNewRect);
        }
    }
}

void SdrTextObj::SetText(const XubString& rStr)
{
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
    SdrOutliner& rOutliner=ImpGetDrawOutliner();

    rOutliner.SetStyleSheet( 0, GetStyleSheet());
    rOutliner.SetUpdateMode(true);
    rOutliner.SetText(rStr,rOutliner.GetParagraph( 0 ));

    OutlinerParaObject* pNewText=rOutliner.CreateParaObject();
    Size aSiz(rOutliner.CalcTextSize());

    rOutliner.Clear();
    SetOutlinerParaObject(pNewText);
    aTextSize=aSiz;
    bTextSizeDirty=false;
    SetChanged();
}

void SdrTextObj::SetText(SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat)
{
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
    SdrOutliner& rOutliner=ImpGetDrawOutliner();
    rOutliner.SetStyleSheet( 0, GetStyleSheet());
    rOutliner.Read(rInput,rBaseURL,eFormat);
    OutlinerParaObject* pNewText=rOutliner.CreateParaObject();
    rOutliner.SetUpdateMode(true);
    Size aSiz(rOutliner.CalcTextSize());
    rOutliner.Clear();
    SetOutlinerParaObject(pNewText);
    aTextSize=aSiz;
    bTextSizeDirty=false;
    SetChanged();
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
        // 2x casting auf nonconst
        ((SdrTextObj*)this)->aTextSize=aSiz;
        ((SdrTextObj*)this)->bTextSizeDirty=false;
    }

    return aTextSize;
}

bool SdrTextObj::IsAutoGrowHeight() const
{
    if(!bTextFrame)
        return false; // AutoGrow nur bei TextFrames

    const SfxItemSet& rSet = GetObjectItemSet();
    bool bRet = ((SdrOnOffItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();

    if(bRet)
    {
        SdrTextAniKind eAniKind = ((SdrTextAniKindItem&)(rSet.Get(SDRATTR_TEXT_ANIKIND))).GetValue();

        if(eAniKind == SDRTEXTANI_SCROLL || eAniKind == SDRTEXTANI_ALTERNATE || eAniKind == SDRTEXTANI_SLIDE)
        {
            SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();

            if(eDirection == SDRTEXTANI_UP || eDirection == SDRTEXTANI_DOWN)
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
        return false; // AutoGrow nur bei TextFrames

    const SfxItemSet& rSet = GetObjectItemSet();
    bool bRet = ((SdrOnOffItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWWIDTH))).GetValue();

    // #101684#
    bool bInEditMOde = IsInEditMode();

    if(!bInEditMOde && bRet)
    {
        SdrTextAniKind eAniKind = ((SdrTextAniKindItem&)(rSet.Get(SDRATTR_TEXT_ANIKIND))).GetValue();

        if(eAniKind == SDRTEXTANI_SCROLL || eAniKind == SDRTEXTANI_ALTERNATE || eAniKind == SDRTEXTANI_SLIDE)
        {
            SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();

            if(eDirection == SDRTEXTANI_LEFT || eDirection == SDRTEXTANI_RIGHT)
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

    SdrTextHorzAdjust eRet = ((SdrTextHorzAdjustItem&)(rSet.Get(SDRATTR_TEXT_HORZADJUST))).GetValue();

    // #101684#
    bool bInEditMode = IsInEditMode();

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
    bool bInEditMode = IsInEditMode();

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

void SdrTextObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bNoTextFrame = !IsTextFrame();
    rInfo.mbResizeFreeAllowed = bNoTextFrame;

    if(!rInfo.mbResizeFreeAllowed)
    {
        sal_Int32 nAngle(sdr::legacy::GetRotateAngle(*this));
        rInfo.mbResizeFreeAllowed = (0 == nAngle % 9000);
    }

    rInfo.mbResizePropAllowed = true;
    rInfo.mbRotateFreeAllowed = true;
    rInfo.mbRotate90Allowed = true;
    rInfo.mbMirrorFreeAllowed = bNoTextFrame;
    rInfo.mbMirror45Allowed = bNoTextFrame;
    rInfo.mbMirror90Allowed = bNoTextFrame;

    // allow transparence
    rInfo.mbTransparenceAllowed = true;

    // gradient depends on fillstyle
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetObjectItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.mbGradientAllowed = (XFILL_GRADIENT == eFillStyle);
    rInfo.mbShearAllowed = bNoTextFrame;
    rInfo.mbEdgeRadiusAllowed = true;
    bool bCanConv = ImpCanConvTextToCurve();
    rInfo.mbCanConvToPath = bCanConv;
    rInfo.mbCanConvToPoly = bCanConv;
    rInfo.mbCanConvToPathLineToArea = bCanConv;
    rInfo.mbCanConvToPolyLineToArea = bCanConv;
    rInfo.mbCanConvToContour = (rInfo.mbCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrTextObj::GetObjIdentifier() const
{
    return sal_uInt16(eTextKind);
}

bool SdrTextObj::HasTextImpl( const SdrOutliner* pOutliner )
{
    bool bRet=false;
    if(pOutliner)
    {
        Paragraph* p1stPara=pOutliner->GetParagraph( 0 );
        sal_uInt32 nParaAnz = pOutliner->GetParagraphCount();
        if(p1stPara==0)
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

void SdrTextObj::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    if(pOldPage != pNewPage)
    {
        const bool bRemove(!pNewPage && pOldPage);
        const bool bInsert(pNewPage && !pOldPage);
        const bool bLinked(IsLinkedText());

        if(bLinked && bRemove)
        {
            ImpLinkAbmeldung();
        }

        // call parent
        SdrAttrObj::handlePageChange(pOldPage, pNewPage);

        if(bLinked && bInsert)
        {
            ImpLinkAnmeldung();
        }
    }
}

void SdrTextObj::SetEdgeRadius(sal_Int32 nRad)
{
    if(nRad != GetEdgeRadius())
    {
        SetObjectItem(SdrMetricItem(SDRATTR_ECKENRADIUS, nRad));
    }
}

// #115391# This implementation is based on the absolute object size (getSdrObjectScale()) and the
// states of IsAutoGrowWidth/Height to correctly set TextMinFrameWidth/Height
void SdrTextObj::AdaptTextMinSize()
{
    if(mbAdaptingTextMinSize)
    {
        return;
    }

    mbAdaptingTextMinSize = true;

    if(bTextFrame && !IsPasteResize())
    {
        const bool bW(IsAutoGrowWidth());
        const bool bH(IsAutoGrowHeight());

        if(bW || bH)
        {
            SfxItemSet aSet(
                *GetObjectItemSet().GetPool(),
                SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
                SDRATTR_TEXT_MINFRAMEWIDTH, SDRATTR_TEXT_AUTOGROWWIDTH, // contains SDRATTR_TEXT_MAXFRAMEWIDTH
                0, 0);

            if(bW)
            {
                const sal_Int32 nDist(GetTextLeftDistance() + GetTextRightDistance());
                const double fW(std::max(0.0, fabs(getSdrObjectScale().getX()) - double(nDist)));

                aSet.Put(SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH, basegfx::fround(fW)));

                if(!IsVerticalWriting() && bDisableAutoWidthOnDragging)
                {
                    bDisableAutoWidthOnDragging = true;
                    aSet.Put(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, false));
                }
            }

            if(bH)
            {
                const sal_Int32 nDist(GetTextUpperDistance() + GetTextLowerDistance());
                const double fH(std::max(0.0, fabs(getSdrObjectScale().getY()) - double(nDist)));

                aSet.Put(SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT, basegfx::fround(fH)));

                if(IsVerticalWriting() && bDisableAutoWidthOnDragging)
                {
                    bDisableAutoWidthOnDragging = false;
                    SetObjectItem(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, false));
                }
            }

            SetObjectItemSet(aSet);
            AdjustTextFrameWidthAndHeight();
        }
    }

    mbAdaptingTextMinSize = false;
}

basegfx::B2DRange SdrTextObj::getUnifiedTextRange() const
{
    // per default the text range is the whole object range
    basegfx::B2DRange aRetval(basegfx::B2DRange::getUnitB2DRange());

    const sal_Int32 nLeftDist(GetTextLeftDistance());
    const sal_Int32 nRightDist(GetTextRightDistance());
    const sal_Int32 nUpperDist(GetTextUpperDistance());
    const sal_Int32 nLowerDist(GetTextLowerDistance());

    if(nLeftDist || nRightDist || nUpperDist || nLowerDist)
    {
        const double fAbsInvScaleX(basegfx::fTools::equalZero(getSdrObjectScale().getX()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getX()));
        const double fAbsInvScaleY(basegfx::fTools::equalZero(getSdrObjectScale().getY()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getY()));

        // add/remove the text distances
        return basegfx::B2DRange(
            aRetval.getMinX() + (nLeftDist * fAbsInvScaleX),
            aRetval.getMinY() + (nUpperDist * fAbsInvScaleY),
            aRetval.getMaxX() - (nRightDist * fAbsInvScaleX),
            aRetval.getMaxY() - (nLowerDist * fAbsInvScaleY));
    }

    return aRetval;
}

void SdrTextObj::TakeTextRange(SdrOutliner& rOutliner, basegfx::B2DRange& rTextRange, basegfx::B2DRange& rAnchorRange) const
{
    // get TextRange without shear, rotate and mirror, just scaled
    // and centered in logic coordinates
    rAnchorRange = getScaledCenteredTextRange(*this);

    // Rect innerhalb dem geankert wird
    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
    SdrTextAniKind      eAniKind=GetTextAniKind();
    SdrTextAniDirection eAniDirection=GetTextAniDirection();
    SdrFitToSizeType eFit=GetFitToSize();
    bool bFitToSize = (eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
    bool bContourFrame = IsContourTextFrame();
    bool bFrame = IsTextFrame();
    sal_uInt32 nStat0 = rOutliner.GetControlWord();
    Size aNullSize;

    if (!bContourFrame)
    {
        rOutliner.SetControlWord(nStat0|EE_CNTRL_AUTOPAGESIZE);
        rOutliner.SetMinAutoPaperSize(aNullSize);
        rOutliner.SetMaxAutoPaperSize(Size(1000000,1000000));

        if(!bFitToSize)
        {
            const sal_Int32 nAnkWdt(basegfx::fround(rAnchorRange.getWidth()));
            const sal_Int32 nAnkHgt(basegfx::fround(rAnchorRange.getHeight()));

            if (bFrame)
            {
                sal_Int32 nWdt(nAnkWdt);
                sal_Int32 nHgt(nAnkHgt);

                // #101684#
                bool bInEditMode = IsInEditMode();

                if (!bInEditMode && (eAniKind==SDRTEXTANI_SCROLL || eAniKind==SDRTEXTANI_ALTERNATE || eAniKind==SDRTEXTANI_SLIDE))
                {
                    // Grenzenlose Papiergroesse fuer Laufschrift
                    if (eAniDirection==SDRTEXTANI_LEFT || eAniDirection==SDRTEXTANI_RIGHT) nWdt=1000000;
                    if (eAniDirection==SDRTEXTANI_UP || eAniDirection==SDRTEXTANI_DOWN) nHgt=1000000;
                }

                // #119885# Do not limit/force height to geometrical frame (vice versa for vertical writing)
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
    }

    rOutliner.SetPaperSize(aNullSize);

    // put text into the outliner, if available from the edit outliner
    SdrText* pText = getActiveText();
    OutlinerParaObject* pOutlinerParaObject = pText ? pText->GetOutlinerParaObject() : 0;
    OutlinerParaObject* pPara = pEdtOutl ? pEdtOutl->CreateParaObject() : pOutlinerParaObject;

    if (pPara)
    {
        const SdrTextObj* pTestObj = rOutliner.GetTextObj();

        if( !pTestObj || pTestObj != this || pTestObj->GetOutlinerParaObject() != pOutlinerParaObject )
        {
            rOutliner.SetTextObj( this );
            rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
            rOutliner.SetUpdateMode(true);
            rOutliner.SetText(*pPara);
        }
    }
    else
    {
        rOutliner.SetTextObj( 0 );
    }

    if (pEdtOutl && pPara)
    {
        delete pPara;
    }

    rOutliner.SetUpdateMode(true);
    rOutliner.SetControlWord(nStat0);

    if( pText )
    {
        pText->CheckPortionInfo(rOutliner);
    }

    basegfx::B2DPoint aTextPos(rAnchorRange.getMinimum());
    const basegfx::B2DVector aTextSiz(rOutliner.GetPaperSize().getWidth(), rOutliner.GetPaperSize().getHeight());

    // #106653#
    // For draw objects containing text correct hor/ver alignment if text is bigger
    // than the object itself. Without that correction, the text would always be
    // formatted to the left edge (or top edge when vertical) of the draw object.
    if(!IsTextFrame())
    {
        if(rAnchorRange.getWidth() < aTextSiz.getX() && !IsVerticalWriting())
        {
            // #110129#
            // Horizontal case here. Correct only if eHAdj == SDRTEXTHORZADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
            {
                eHAdj = SDRTEXTHORZADJUST_CENTER;
            }
        }

        if(rAnchorRange.getHeight() < aTextSiz.getY() && IsVerticalWriting())
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

    if(SDRTEXTHORZADJUST_CENTER == eHAdj || SDRTEXTHORZADJUST_RIGHT == eHAdj)
    {
        const double fFreeWdt(rAnchorRange.getWidth() - aTextSiz.getX());

        if(SDRTEXTHORZADJUST_CENTER == eHAdj)
        {
            aTextPos.setX(aTextPos.getX() + (fFreeWdt * 0.5));
        }
        else if(SDRTEXTHORZADJUST_RIGHT == eHAdj)
        {
            aTextPos.setX(aTextPos.getX() + fFreeWdt);
        }
    }

    if(SDRTEXTVERTADJUST_CENTER == eVAdj || SDRTEXTVERTADJUST_BOTTOM == eVAdj)
    {
        const double fFreeHgt(rAnchorRange.getHeight() - aTextSiz.getY());

        if(SDRTEXTVERTADJUST_CENTER == eVAdj)
        {
            aTextPos.setY(aTextPos.getY() + (fFreeHgt * 0.5));
        }
        else if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
        {
            aTextPos.setY(aTextPos.getY() + fFreeHgt);
        }
    }

    if(bContourFrame)
    {
        basegfx::B2DPolyPolygon aContourOutline(getAlignedTextContourPolyPolygon(*this));
        rTextRange = rAnchorRange = aContourOutline.getB2DRange();
        aContourOutline.transform(basegfx::tools::createTranslateB2DHomMatrix(-aContourOutline.getB2DRange().getMinimum()));
        rOutliner.SetPolygon(aContourOutline);
    }
    else
    {
        rOutliner.ClearPolygon();
        rTextRange = basegfx::B2DRange(aTextPos, aTextPos + aTextSiz);
    }
}

OutlinerParaObject* SdrTextObj::GetEditOutlinerParaObject() const
{
    OutlinerParaObject* pPara=0;
    if( HasTextImpl( pEdtOutl ) )
    {
        sal_uInt16 nParaAnz = static_cast< sal_uInt16 >( pEdtOutl->GetParagraphCount() );
        pPara = pEdtOutl->CreateParaObject(0, nParaAnz);
    }
    return pPara;
}

void SdrTextObj::ImpSetCharStretching(SdrOutliner& rOutliner, const basegfx::B2DRange& rTextRange, const basegfx::B2DRange& rAnchorRange) const
{
    const basegfx::B2DVector aOutlinerScale(
        basegfx::fTools::equalZero(rTextRange.getWidth()) ? 1.0 : rTextRange.getWidth(),
        basegfx::fTools::equalZero(rTextRange.getHeight()) ? 1.0 : rTextRange.getHeight());

    // calculate global char stretching scale parameters. Use non-mirrored sizes
    // to layout without mirroring
    const double fScaleX(fabs(rAnchorRange.getWidth()) / aOutlinerScale.getX());
    const double fScaleY(fabs(rAnchorRange.getHeight()) / aOutlinerScale.getY());

    rOutliner.SetGlobalCharStretching((sal_Int16)basegfx::fround(fScaleX * 100.0), (sal_Int16)basegfx::fround(fScaleY * 100.0));
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

basegfx::B2DPolyPolygon SdrTextObj::TakeXorPoly() const
{
    basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());

    aPolygon.transform(getSdrObjectTransformation());

    return basegfx::B2DPolyPolygon(aPolygon);
}

sal_uInt32 SdrTextObj::GetSnapPointCount() const
{
    return 4L;
}

basegfx::B2DPoint SdrTextObj::GetSnapPoint(sal_uInt32 i) const
{
    basegfx::B2DPoint aPoint;

    switch (i)
    {
        case 0: aPoint = basegfx::B2DPoint(0.0, 0.0); break;
        case 1: aPoint = basegfx::B2DPoint(1.0, 0.0); break;
        case 2: aPoint = basegfx::B2DPoint(0.0, 1.0); break;
        case 3: aPoint = basegfx::B2DPoint(1.0, 1.0); break;
        default: aPoint = basegfx::B2DPoint(0.5, 0.5); break;
    }

    return getSdrObjectTransformation() * aPoint;
}

// #101029#: Extracted from ImpGetDrawOutliner()
void SdrTextObj::ImpInitDrawOutliner( SdrOutliner& rOutl ) const
{
    rOutl.SetUpdateMode(false);
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
    SdrOutliner& rOutl = getSdrModelFromSdrObject().GetDrawOutliner(this);

    // #101029#: Code extracted to ImpInitDrawOutliner()
    ImpInitDrawOutliner( rOutl );

    return rOutl;
}

boost::shared_ptr< SdrOutliner > SdrTextObj::CreateDrawOutliner()
{
    boost::shared_ptr< SdrOutliner > xDrawOutliner( getSdrModelFromSdrObject().CreateDrawOutliner(this) );
    ImpInitDrawOutliner( *(xDrawOutliner.get()) );
    return xDrawOutliner;
}

// #101029#: Extracted from Paint()
void SdrTextObj::ImpSetupDrawOutlinerForPaint(
    bool bContourFrame,
                                               SdrOutliner&     rOutliner,
    basegfx::B2DRange& rTextRange,
    basegfx::B2DRange& rAnchorRange,
    basegfx::B2DRange& rPaintRange) const
{
    if (!bContourFrame)
    {
        // FitToSize erstmal nicht mit ContourFrame
        SdrFitToSizeType eFit=GetFitToSize();
        if (eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES)
        {
            sal_uIntPtr nStat=rOutliner.GetControlWord();
            nStat|=EE_CNTRL_STRETCHING|EE_CNTRL_AUTOPAGESIZE;
            rOutliner.SetControlWord(nStat);
        }
    }

    rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
    TakeTextRange(rOutliner, rTextRange, rAnchorRange);
    rPaintRange = rTextRange;

    if (!bContourFrame)
    {
        // FitToSize erstmal nicht mit ContourFrame
        SdrFitToSizeType eFit=GetFitToSize();
        if (eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES)
        {
            ImpSetCharStretching(rOutliner, rTextRange, rAnchorRange);
            rPaintRange = rAnchorRange;
        }
    }
}

void SdrTextObj::SetupOutlinerFormatting( SdrOutliner& rOutl, basegfx::B2DRange& rPaintRange ) const
{
    ImpInitDrawOutliner( rOutl );
    UpdateOutlinerFormatting( rOutl, rPaintRange );
}

void SdrTextObj::UpdateOutlinerFormatting( SdrOutliner& rOutl, basegfx::B2DRange& rPaintRange ) const
{
    basegfx::B2DRange aTextRange;
    basegfx::B2DRange aAnchorRange;
    const bool bContourFrame(IsContourTextFrame());
    const MapMode aMapMode(
        getSdrModelFromSdrObject().GetExchangeObjectUnit(), Point(0,0),
        getSdrModelFromSdrObject().GetExchangeObjectScale(),
        getSdrModelFromSdrObject().GetExchangeObjectScale());

        rOutl.SetRefMapMode(aMapMode);
    ImpSetupDrawOutlinerForPaint( bContourFrame, rOutl, aTextRange, aAnchorRange, rPaintRange );
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

void SdrTextObj::SetOutlinerParaObject(OutlinerParaObject* pTextObject)
{
    bool bChange(false);
    OutlinerParaObject* pCurrent = GetOutlinerParaObject();

    // use OutlinerParaObject compare operator
    if(pCurrent != pTextObject)
    {
        bChange = true;
    }

    // if both exist, check if the difference is in redlining
    if(!bChange && pTextObject && pCurrent)
    {
        bChange = !pTextObject->isWrongListEqual(*pCurrent);
    }

    if(bChange)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

        SetOutlinerParaObjectForText( pTextObject, getActiveText() );
        SetChanged();
    }
}

void SdrTextObj::SetOutlinerParaObjectForText( OutlinerParaObject* pTextObject, SdrText* pText )
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
        // Textrahmen anpassen!
        AdjustTextFrameWidthAndHeight();
    }

    // always invalidate BoundRect on change
    ActionChanged();

    ImpSetTextStyleSheetListeners();
}

void SdrTextObj::ReformatText()
{
    SdrText* pText = getActiveText();

    if( pText && pText->GetOutlinerParaObject() )
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

        pText->ReformatText();

        if (bTextFrame)
        {
            AdjustTextFrameWidthAndHeight();
        }

        SetTextSizeDirty();

        // FME, AW: i22396
        // Necessary here since we have no compare operator at the outliner
        // para object which may detect changes regarding the combination
        // of outliner para data and configuration (e.g., change of
        // formatting of text numerals)
        GetViewContact().flushViewObjectContacts(false);
        SetChanged();
    }
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

bool SdrTextObj::IsVerticalWriting() const
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

    if( pOutlinerParaObject && (pOutlinerParaObject->IsVertical() != (bool)bVertical) )
    {
        // get item settings
        const SfxItemSet& rSet = GetObjectItemSet();
        bool bAutoGrowWidth = ((SdrOnOffItem&)rSet.Get(SDRATTR_TEXT_AUTOGROWWIDTH)).GetValue();
        bool bAutoGrowHeight = ((SdrOnOffItem&)rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT)).GetValue();

        // #103516# Also exchange hor/ver adjust items
        SdrTextHorzAdjust eHorz = ((SdrTextHorzAdjustItem&)(rSet.Get(SDRATTR_TEXT_HORZADJUST))).GetValue();
        SdrTextVertAdjust eVert = ((SdrTextVertAdjustItem&)(rSet.Get(SDRATTR_TEXT_VERTADJUST))).GetValue();

        // rescue object size
        Rectangle aObjectRect(sdr::legacy::GetSnapRect(*this));

        // prepare ItemSet to set exchanged width and height items
        SfxItemSet aNewSet(*rSet.GetPool(),
            SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
            // #103516# Expanded item ranges to also support hor and ver adjust.
            SDRATTR_TEXT_VERTADJUST, SDRATTR_TEXT_VERTADJUST,
            SDRATTR_TEXT_AUTOGROWWIDTH, SDRATTR_TEXT_HORZADJUST,
            0, 0);

        aNewSet.Put(rSet);
        aNewSet.Put(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, bAutoGrowHeight));
        aNewSet.Put(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, bAutoGrowWidth));

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
        sdr::legacy::SetSnapRect(*this, aObjectRect);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrTextObj::IsRealyEdited() const // TTTT: Is this used?
{
    return pEdtOutl && pEdtOutl->IsModified();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// moved inlines here form hxx

sal_Int32 SdrTextObj::GetEdgeRadius() const
{
    return ((SdrMetricItem&)(GetObjectItemSet().Get(SDRATTR_ECKENRADIUS))).GetValue();
}

sal_Int32 SdrTextObj::GetMinTextFrameHeight() const
{
    return ((SdrMetricItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_MINFRAMEHEIGHT))).GetValue();
}

sal_Int32 SdrTextObj::GetMaxTextFrameHeight() const
{
    return ((SdrMetricItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_MAXFRAMEHEIGHT))).GetValue();
}

sal_Int32 SdrTextObj::GetMinTextFrameWidth() const
{
    return ((SdrMetricItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_MINFRAMEWIDTH))).GetValue();
}

sal_Int32 SdrTextObj::GetMaxTextFrameWidth() const
{
    return ((SdrMetricItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_MAXFRAMEWIDTH))).GetValue();
}

bool SdrTextObj::IsFontwork() const
{
    return (bTextFrame)
        ? false // Default ist false
        : ((XFormTextStyleItem&)(GetObjectItemSet().Get(XATTR_FORMTXTSTYLE))).GetValue()!=XFT_NONE;
}

bool SdrTextObj::IsHideContour() const
{
    return (bTextFrame)
        ? false // Default ist: Nein, kein HideContour; HideContour nicht bei TextFrames
        : ((XFormTextHideFormItem&)(GetObjectItemSet().Get(XATTR_FORMTXTHIDEFORM))).GetValue();
}

bool SdrTextObj::IsContourTextFrame() const
{
    return (bTextFrame)
        ? false // ContourFrame nicht bei normalen TextFrames
        : ((SdrOnOffItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_CONTOURFRAME))).GetValue();
}

sal_Int32 SdrTextObj::GetTextLeftDistance() const
{
    return ((SdrMetricItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_LEFTDIST))).GetValue();
}

sal_Int32 SdrTextObj::GetTextRightDistance() const
{
    return ((SdrMetricItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_RIGHTDIST))).GetValue();
}

sal_Int32 SdrTextObj::GetTextUpperDistance() const
{
    return ((SdrMetricItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_UPPERDIST))).GetValue();
}

sal_Int32 SdrTextObj::GetTextLowerDistance() const
{
    return ((SdrMetricItem&)(GetObjectItemSet().Get(SDRATTR_TEXT_LOWERDIST))).GetValue();
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
// Get necessary data for text scroll animation. ATM base it on a Text-Metafile and a
// painting rectangle. Rotation is excluded from the returned values.
GDIMetaFile* SdrTextObj::GetTextScrollMetaFileAndRange(basegfx::B2DRange& rScrollRange, basegfx::B2DRange& rPaintRange) const
{
    GDIMetaFile* pRetval = 0;
    SdrOutliner& rOutliner = ImpGetDrawOutliner();
    basegfx::B2DRange aTextRange;
    basegfx::B2DRange aAnchorRange;
    basegfx::B2DRange aPaintRange;
    bool bContourFrame(IsContourTextFrame());

    // get outliner set up. To avoid getting a somehow rotated MetaFile,
    // temporarily disable object rotation.
    basegfx::B2DVector aScale;
    basegfx::B2DPoint aTranslate;
    double fRotate, fShearX;
    const basegfx::B2DHomMatrix aOriginalTransformation(getSdrObjectTransformation());
    aOriginalTransformation.decompose(aScale, aTranslate, fRotate, fShearX);

    if(!basegfx::fTools::equalZero(fRotate))
    {
        // allow myself to cast, this method will not be necessary for very long anymore
        const_cast< SdrTextObj* >(this)->maSdrObjectTransformation.setB2DHomMatrix(
            basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                aScale, fShearX, 0.0, aTranslate));
    }

    ImpSetupDrawOutlinerForPaint(bContourFrame, rOutliner, aTextRange, aAnchorRange, aPaintRange );

    if(!basegfx::fTools::equalZero(fRotate))
    {
        const_cast< SdrTextObj* >(this)->maSdrObjectTransformation.setB2DHomMatrix(aOriginalTransformation);
    }

    basegfx::B2DRange aScrollFrameRange(aPaintRange);
    const SfxItemSet& rSet = GetObjectItemSet();
    SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();

    if(SDRTEXTANI_LEFT == eDirection || SDRTEXTANI_RIGHT == eDirection)
    {
        aScrollFrameRange = basegfx::B2DRange(
            aAnchorRange.getMinX(), aScrollFrameRange.getMinY(),
            aAnchorRange.getMaxX(), aScrollFrameRange.getMaxY());
    }

    if(SDRTEXTANI_UP == eDirection || SDRTEXTANI_DOWN == eDirection)
    {
        aScrollFrameRange = basegfx::B2DRange(
            aScrollFrameRange.getMinX(), aAnchorRange.getMinY(),
            aScrollFrameRange.getMaxX(), aAnchorRange.getMaxY());
    }

    // create the MetaFile
    pRetval = new GDIMetaFile;
    VirtualDevice aBlackHole;
    aBlackHole.EnableOutput(sal_False);
    pRetval->Record(&aBlackHole);
    const Point aPaintPos(basegfx::fround(aPaintRange.getMinX()), basegfx::fround(aPaintRange.getMinY()));

    rOutliner.Draw(&aBlackHole, aPaintPos);

    pRetval->Stop();
    pRetval->WindStart();

    // return PaintRectanglePixel and pRetval;
    rScrollRange = aScrollFrameRange;
    rPaintRange = aPaintRange;

    return pRetval;
}

// #111096#
// Access to TextAnimationAllowed flag
bool SdrTextObj::IsTextAnimationAllowed() const
{
    return mbTextAnimationAllowed;
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
    const sal_uInt32 nStat = pEditStatus->GetStatusWord();
    const bool bGrowX=(nStat & EE_STAT_TEXTWIDTHCHANGED) !=0;
    const bool bGrowY=(nStat & EE_STAT_TEXTHEIGHTCHANGED) !=0;
    if(bTextFrame && (bGrowX || bGrowY))
    {
        const bool bAutoGrowHgt= bTextFrame && IsAutoGrowHeight();
        const bool bAutoGrowWdt= bTextFrame && IsAutoGrowWidth();

        if ((bGrowX && bAutoGrowWdt) || (bGrowY && bAutoGrowHgt))
        {
            AdjustTextFrameWidthAndHeight();
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
sal_Int32 SdrTextObj::CheckTextHit(const basegfx::B2DPoint& /*rPnt*/) const
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
// - bool Textrahmen / beschriftetes Zeichenobjekt
// - bool FontWork                 (wenn nicht Textrahmen und nicht ContourTextFrame)
// - bool ContourTextFrame         (wenn nicht Textrahmen und nicht Fontwork)
// - sal_Int32 Drehwinkel               (wenn nicht FontWork)
// - sal_Int32 Textrahmenabstaende      (wenn nicht FontWork)
// - bool FitToSize                (wenn nicht FontWork)
// - bool AutoGrowingWidth/Height  (wenn nicht FitToSize und nicht FontWork)
// - sal_Int32 Min/MaxFrameWidth/Height (wenn AutoGrowingWidth/Height)
// - enum Horizontale Textverankerung Links,Mitte,Rechts,Block,Stretch(ni)
// - enum Vertikale Textverankerung Oben,Mitte,Unten,Block,Stretch(ni)
// - enum Laufschrift              (wenn nicht FontWork)
//
// Jedes abgeleitete Objekt ist entweder ein Textrahmen (bTextFrame=true)
// oder ein beschriftetes Zeichenobjekt (bTextFrame=false).
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
// TTTT: TakeUnrotatedSnapRect() removed, needed...?
//
// FitToSize hat Prioritaet vor Textverankerung und AutoGrowHeight/Width. Der
// Ausgabebereich ist bei FitToSize immer genau der Ankerbereich. Weiterhin
// gibt es bei FitToSize keinen automatischen Zeilenumbruch.
//
// ContourTextFrame:
// - sal_Int32 Drehwinkel
// - sal_Int32 Textrahmenabstaende         spaeter vielleicht
// - bool FitToSize                   spaeter vielleicht
// - bool AutoGrowingWidth/Height     viel spaeter vielleicht
// - sal_Int32 Min/MaxFrameWidth/Height    viel spaeter vielleicht
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
// eof
