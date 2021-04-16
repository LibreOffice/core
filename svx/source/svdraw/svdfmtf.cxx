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

#include "svdfmtf.hxx"
#include <math.h>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/metric.hxx>
#include <editeng/charscaleitem.hxx>
#include <svx/xflhtit.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/sdtditm.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdetc.hxx>
#include <svl/itemset.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <tools/helpers.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/xlndsit.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>
#include <svx/svdpntv.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/svditer.hxx>
#include <svx/svdogrp.hxx>
#include <vcl/BitmapTools.hxx>
#include <osl/diagnose.h>

using namespace com::sun::star;

ImpSdrGDIMetaFileImport::ImpSdrGDIMetaFileImport(
    SdrModel& rModel,
    SdrLayerID nLay,
    const tools::Rectangle& rRect)
:   mpVD(VclPtr<VirtualDevice>::Create()),
    maScaleRect(rRect),
    mnMapScalingOfs(0),
    mpModel(&rModel),
    mnLayer(nLay),
    mnLineWidth(0),
    maLineJoin(basegfx::B2DLineJoin::NONE),
    maLineCap(css::drawing::LineCap_BUTT),
    maDash(css::drawing::DashStyle_RECT, 0, 0, 0, 0, 0),
    mbMov(false),
    mbSize(false),
    maOfs(0, 0),
    mfScaleX(1.0),
    mfScaleY(1.0),
    maScaleX(1.0),
    maScaleY(1.0),
    mbFntDirty(true),
    mbLastObjWasPolyWithoutLine(false),
    mbNoLine(false),
    mbNoFill(false),
    mbLastObjWasLine(false)
{
    mpVD->EnableOutput(false);
    mpVD->SetLineColor();
    mpVD->SetFillColor();
    maOldLineColor.SetRed( mpVD->GetLineColor().GetRed() + 1 );
    mpLineAttr = std::make_unique<SfxItemSetFixed<XATTR_LINE_FIRST, XATTR_LINE_LAST>>(rModel.GetItemPool());
    mpFillAttr = std::make_unique<SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST>>(rModel.GetItemPool());
    mpTextAttr = std::make_unique<SfxItemSetFixed<EE_ITEMS_START, EE_ITEMS_END>>(rModel.GetItemPool());
    checkClip();
}

void ImpSdrGDIMetaFileImport::DoLoopActions(GDIMetaFile const & rMtf, SvdProgressInfo* pProgrInfo, sal_uInt32* pActionsToReport)
{
    const sal_uLong nCount(rMtf.GetActionSize());

    for(sal_uLong a(0); a < nCount; a++)
    {
        MetaAction* pAct = rMtf.GetAction(a);

        if(!pAct)
        {
            OSL_ENSURE(false, "OOps, no action at valid position (!)");
            pAct = rMtf.GetAction(0);
        }

        switch (pAct->GetType())
        {
            case MetaActionType::PIXEL          : break;
            case MetaActionType::POINT          : break;
            case MetaActionType::LINE           : DoAction(static_cast<MetaLineAction           &>(*pAct)); break;
            case MetaActionType::RECT           : DoAction(static_cast<MetaRectAction           &>(*pAct)); break;
            case MetaActionType::ROUNDRECT      : DoAction(static_cast<MetaRoundRectAction      &>(*pAct)); break;
            case MetaActionType::ELLIPSE        : DoAction(static_cast<MetaEllipseAction        &>(*pAct)); break;
            case MetaActionType::ARC            : DoAction(static_cast<MetaArcAction            &>(*pAct)); break;
            case MetaActionType::PIE            : DoAction(static_cast<MetaPieAction            &>(*pAct)); break;
            case MetaActionType::CHORD          : DoAction(static_cast<MetaChordAction          &>(*pAct)); break;
            case MetaActionType::POLYLINE       : DoAction(static_cast<MetaPolyLineAction       &>(*pAct)); break;
            case MetaActionType::POLYGON        : DoAction(static_cast<MetaPolygonAction        &>(*pAct)); break;
            case MetaActionType::POLYPOLYGON    : DoAction(static_cast<MetaPolyPolygonAction    &>(*pAct)); break;
            case MetaActionType::TEXT           : DoAction(static_cast<MetaTextAction           &>(*pAct)); break;
            case MetaActionType::TEXTARRAY      : DoAction(static_cast<MetaTextArrayAction      &>(*pAct)); break;
            case MetaActionType::STRETCHTEXT    : DoAction(static_cast<MetaStretchTextAction    &>(*pAct)); break;
            case MetaActionType::BMP            : DoAction(static_cast<MetaBmpAction            &>(*pAct)); break;
            case MetaActionType::BMPSCALE       : DoAction(static_cast<MetaBmpScaleAction       &>(*pAct)); break;
            case MetaActionType::BMPEX          : DoAction(static_cast<MetaBmpExAction          &>(*pAct)); break;
            case MetaActionType::BMPEXSCALE     : DoAction(static_cast<MetaBmpExScaleAction     &>(*pAct)); break;
            case MetaActionType::LINECOLOR      : DoAction(static_cast<MetaLineColorAction      &>(*pAct)); break;
            case MetaActionType::FILLCOLOR      : DoAction(static_cast<MetaFillColorAction      &>(*pAct)); break;
            case MetaActionType::TEXTCOLOR      : DoAction(static_cast<MetaTextColorAction      &>(*pAct)); break;
            case MetaActionType::TEXTFILLCOLOR  : DoAction(static_cast<MetaTextFillColorAction  &>(*pAct)); break;
            case MetaActionType::FONT           : DoAction(static_cast<MetaFontAction           &>(*pAct)); break;
            case MetaActionType::TEXTALIGN      : DoAction(static_cast<MetaTextAlignAction      &>(*pAct)); break;
            case MetaActionType::MAPMODE        : DoAction(static_cast<MetaMapModeAction        &>(*pAct)); break;
            case MetaActionType::CLIPREGION     : DoAction(static_cast<MetaClipRegionAction     &>(*pAct)); break;
            case MetaActionType::MOVECLIPREGION : DoAction(static_cast<MetaMoveClipRegionAction &>(*pAct)); break;
            case MetaActionType::ISECTRECTCLIPREGION: DoAction(static_cast<MetaISectRectClipRegionAction&>(*pAct)); break;
            case MetaActionType::ISECTREGIONCLIPREGION: DoAction(static_cast<MetaISectRegionClipRegionAction&>(*pAct)); break;
            case MetaActionType::RASTEROP       : DoAction(static_cast<MetaRasterOpAction       &>(*pAct)); break;
            case MetaActionType::PUSH           : DoAction(static_cast<MetaPushAction           &>(*pAct)); break;
            case MetaActionType::POP            : DoAction(static_cast<MetaPopAction            &>(*pAct)); break;
            case MetaActionType::HATCH          : DoAction(static_cast<MetaHatchAction          &>(*pAct)); break;

            // #i125211# MetaCommentAction may change index, thus hand it over
            case MetaActionType::COMMENT        : DoAction(static_cast<MetaCommentAction&>(*pAct), rMtf, a);
                break;

            // missing actions added
            case MetaActionType::TEXTRECT       : DoAction(static_cast<MetaTextRectAction&>(*pAct)); break;
            case MetaActionType::BMPSCALEPART   : DoAction(static_cast<MetaBmpScalePartAction&>(*pAct)); break;
            case MetaActionType::BMPEXSCALEPART : DoAction(static_cast<MetaBmpExScalePartAction&>(*pAct)); break;
            case MetaActionType::MASK           : DoAction(static_cast<MetaMaskAction&>(*pAct)); break;
            case MetaActionType::MASKSCALE      : DoAction(static_cast<MetaMaskScaleAction&>(*pAct)); break;
            case MetaActionType::MASKSCALEPART  : DoAction(static_cast<MetaMaskScalePartAction&>(*pAct)); break;
            case MetaActionType::GRADIENT       : DoAction(static_cast<MetaGradientAction&>(*pAct)); break;
            case MetaActionType::WALLPAPER      : OSL_ENSURE(false, "Tried to construct SdrObject from MetaWallpaperAction: not supported (!)"); break;
            case MetaActionType::Transparent    : DoAction(static_cast<MetaTransparentAction&>(*pAct)); break;
            case MetaActionType::EPS            : OSL_ENSURE(false, "Tried to construct SdrObject from MetaEPSAction: not supported (!)"); break;
            case MetaActionType::REFPOINT       : DoAction(static_cast<MetaRefPointAction&>(*pAct)); break;
            case MetaActionType::TEXTLINECOLOR  : DoAction(static_cast<MetaTextLineColorAction&>(*pAct)); break;
            case MetaActionType::TEXTLINE       : OSL_ENSURE(false, "Tried to construct SdrObject from MetaTextLineAction: not supported (!)"); break;
            case MetaActionType::FLOATTRANSPARENT : DoAction(static_cast<MetaFloatTransparentAction&>(*pAct)); break;
            case MetaActionType::GRADIENTEX     : DoAction(static_cast<MetaGradientExAction&>(*pAct)); break;
            case MetaActionType::LAYOUTMODE     : DoAction(static_cast<MetaLayoutModeAction&>(*pAct)); break;
            case MetaActionType::TEXTLANGUAGE   : DoAction(static_cast<MetaTextLanguageAction&>(*pAct)); break;
            case MetaActionType::OVERLINECOLOR  : DoAction(static_cast<MetaOverlineColorAction&>(*pAct)); break;
            default: break;
        }

        if(pProgrInfo && pActionsToReport)
        {
            (*pActionsToReport)++;

            if(*pActionsToReport >= 16) // update all 16 actions
            {
                if(!pProgrInfo->ReportActions(*pActionsToReport))
                    break;

                *pActionsToReport = 0;
            }
        }
    }
}

size_t ImpSdrGDIMetaFileImport::DoImport(
    const GDIMetaFile& rMtf,
    SdrObjList& rOL,
    size_t nInsPos,
    SvdProgressInfo* pProgrInfo)
{
    // setup some global scale parameter
    // mfScaleX, mfScaleY, maScaleX, maScaleY, mbMov, mbSize
    mfScaleX = mfScaleY = 1.0;
    const Size aMtfSize(rMtf.GetPrefSize());

    if(aMtfSize.Width() & aMtfSize.Height() && (!maScaleRect.IsEmpty()))
    {
        maOfs = maScaleRect.TopLeft();

        if(aMtfSize.Width() != (maScaleRect.GetWidth() - 1))
        {
            mfScaleX = static_cast<double>( maScaleRect.GetWidth() - 1 ) / static_cast<double>(aMtfSize.Width());
        }

        if(aMtfSize.Height() != (maScaleRect.GetHeight() - 1))
        {
            mfScaleY = static_cast<double>( maScaleRect.GetHeight() - 1 ) / static_cast<double>(aMtfSize.Height());
        }
    }

    mbMov = maOfs.X()!=0 || maOfs.Y()!=0;
    mbSize = false;
    maScaleX = Fraction( 1, 1 );
    maScaleY = Fraction( 1, 1 );

    if(aMtfSize.Width() != (maScaleRect.GetWidth() - 1))
    {
        maScaleX = Fraction(maScaleRect.GetWidth() - 1, aMtfSize.Width());
        mbSize = true;
    }

    if(aMtfSize.Height() != (maScaleRect.GetHeight() - 1))
    {
        maScaleY = Fraction(maScaleRect.GetHeight() - 1, aMtfSize.Height());
        mbSize = true;
    }

    if(pProgrInfo)
    {
        pProgrInfo->SetActionCount(rMtf.GetActionSize());
    }

    sal_uInt32 nActionsToReport(0);

    // execute
    DoLoopActions(rMtf, pProgrInfo, &nActionsToReport);

    if(pProgrInfo)
    {
        pProgrInfo->ReportActions(nActionsToReport);
        nActionsToReport = 0;
    }

    // MapMode scaling
    MapScaling();

    // To calculate the progress meter, we use GetActionSize()*3.
    // However, maTmpList has a lower entry count limit than GetActionSize(),
    // so the actions that were assumed were too much have to be re-added.
    nActionsToReport = (rMtf.GetActionSize() - maTmpList.size()) * 2;

    // announce all currently unannounced rescales
    if(pProgrInfo)
    {
        pProgrInfo->ReportRescales(nActionsToReport);
        pProgrInfo->SetInsertCount(maTmpList.size());
    }

    nActionsToReport = 0;

    // insert all objects cached in aTmpList now into rOL from nInsPos
    nInsPos = std::min(nInsPos, rOL.GetObjCount());

    for(SdrObject* pObj : maTmpList)
    {
        rOL.NbcInsertObject(pObj, nInsPos);
        nInsPos++;

        if(pProgrInfo)
        {
            nActionsToReport++;

            if(nActionsToReport >= 32) // update all 32 actions
            {
                pProgrInfo->ReportInserts(nActionsToReport);
                nActionsToReport = 0;
            }
        }
    }

    // report all remaining inserts for the last time
    if(pProgrInfo)
    {
        pProgrInfo->ReportInserts(nActionsToReport);
    }

    return maTmpList.size();
}

void ImpSdrGDIMetaFileImport::SetAttributes(SdrObject* pObj, bool bForceTextAttr)
{
    mbNoLine = false;
    mbNoFill = false;
    bool bLine(!bForceTextAttr);
    bool bFill(!pObj || (pObj->IsClosedObj() && !bForceTextAttr));
    bool bText(bForceTextAttr || (pObj && pObj->GetOutlinerParaObject()));

    if(bLine)
    {
        if(mnLineWidth)
        {
            mpLineAttr->Put(XLineWidthItem(mnLineWidth));
        }
        else
        {
            mpLineAttr->Put(XLineWidthItem(0));
        }

        maOldLineColor = mpVD->GetLineColor();

        if(mpVD->IsLineColor())
        {
            mpLineAttr->Put(XLineStyleItem(drawing::LineStyle_SOLID));
            mpLineAttr->Put(XLineColorItem(OUString(), mpVD->GetLineColor()));
        }
        else
        {
            mpLineAttr->Put(XLineStyleItem(drawing::LineStyle_NONE));
        }

        switch(maLineJoin)
        {
            case basegfx::B2DLineJoin::NONE:
                mpLineAttr->Put(XLineJointItem(css::drawing::LineJoint_NONE));
                break;
            case basegfx::B2DLineJoin::Bevel:
                mpLineAttr->Put(XLineJointItem(css::drawing::LineJoint_BEVEL));
                break;
            case basegfx::B2DLineJoin::Miter:
                mpLineAttr->Put(XLineJointItem(css::drawing::LineJoint_MITER));
                break;
            case basegfx::B2DLineJoin::Round:
                mpLineAttr->Put(XLineJointItem(css::drawing::LineJoint_ROUND));
                break;
        }

        // Add LineCap support
        mpLineAttr->Put(XLineCapItem(maLineCap));

        if(((maDash.GetDots() && maDash.GetDotLen()) || (maDash.GetDashes() && maDash.GetDashLen())) && maDash.GetDistance())
        {
            mpLineAttr->Put(XLineDashItem(OUString(), maDash));
        }
        else
        {
            mpLineAttr->Put(XLineDashItem(OUString(), XDash(css::drawing::DashStyle_RECT)));
        }
    }
    else
    {
        mbNoLine = true;
    }

    if(bFill)
    {
        if(mpVD->IsFillColor())
        {
            mpFillAttr->Put(XFillStyleItem(drawing::FillStyle_SOLID));
            mpFillAttr->Put(XFillColorItem(OUString(), mpVD->GetFillColor()));
        }
        else
        {
            mpFillAttr->Put(XFillStyleItem(drawing::FillStyle_NONE));
        }
    }
    else
    {
        mbNoFill = true;
    }

    if(bText && mbFntDirty)
    {
        vcl::Font aFnt(mpVD->GetFont());
        const sal_uInt32 nHeight(FRound(aFnt.GetFontSize().Height() * mfScaleY));

        mpTextAttr->Put( SvxFontItem( aFnt.GetFamilyType(), aFnt.GetFamilyName(), aFnt.GetStyleName(), aFnt.GetPitch(), aFnt.GetCharSet(), EE_CHAR_FONTINFO ) );
        mpTextAttr->Put( SvxFontItem( aFnt.GetFamilyType(), aFnt.GetFamilyName(), aFnt.GetStyleName(), aFnt.GetPitch(), aFnt.GetCharSet(), EE_CHAR_FONTINFO_CJK ) );
        mpTextAttr->Put( SvxFontItem( aFnt.GetFamilyType(), aFnt.GetFamilyName(), aFnt.GetStyleName(), aFnt.GetPitch(), aFnt.GetCharSet(), EE_CHAR_FONTINFO_CTL ) );
        mpTextAttr->Put(SvxPostureItem(aFnt.GetItalic(), EE_CHAR_ITALIC));
        mpTextAttr->Put(SvxWeightItem(aFnt.GetWeight(), EE_CHAR_WEIGHT));
        mpTextAttr->Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT ) );
        mpTextAttr->Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
        mpTextAttr->Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );
        mpTextAttr->Put(SvxCharScaleWidthItem(100, EE_CHAR_FONTWIDTH));
        mpTextAttr->Put(SvxUnderlineItem(aFnt.GetUnderline(), EE_CHAR_UNDERLINE));
        mpTextAttr->Put(SvxOverlineItem(aFnt.GetOverline(), EE_CHAR_OVERLINE));
        mpTextAttr->Put(SvxCrossedOutItem(aFnt.GetStrikeout(), EE_CHAR_STRIKEOUT));
        mpTextAttr->Put(SvxShadowedItem(aFnt.IsShadow(), EE_CHAR_SHADOW));

        // #i118485# Setting this item leads to problems (written #i118498# for this)
        // mpTextAttr->Put(SvxAutoKernItem(aFnt.IsKerning(), EE_CHAR_KERNING));

        mpTextAttr->Put(SvxWordLineModeItem(aFnt.IsWordLineMode(), EE_CHAR_WLM));
        mpTextAttr->Put(SvxContourItem(aFnt.IsOutline(), EE_CHAR_OUTLINE));
        mpTextAttr->Put(SvxColorItem(mpVD->GetTextColor(), EE_CHAR_COLOR));
        //... svxfont textitem svditext
        mbFntDirty = false;
    }

    if(!pObj)
        return;

    pObj->SetLayer(mnLayer);

    if(bLine)
    {
        pObj->SetMergedItemSet(*mpLineAttr);
    }

    if(bFill)
    {
        pObj->SetMergedItemSet(*mpFillAttr);
    }

    if(bText)
    {
        pObj->SetMergedItemSet(*mpTextAttr);
        pObj->SetMergedItem(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT));
    }
}

void ImpSdrGDIMetaFileImport::InsertObj(SdrObject* pObj, bool bScale)
{
    if(bScale && !maScaleRect.IsEmpty())
    {
        if(mbSize)
        {
            pObj->NbcResize(Point(), maScaleX, maScaleY);
        }

        if(mbMov)
        {
            pObj->NbcMove(Size(maOfs.X(), maOfs.Y()));
        }
    }

    if(isClip())
    {
        const basegfx::B2DPolyPolygon aPoly(pObj->TakeXorPoly());
        const basegfx::B2DRange aOldRange(aPoly.getB2DRange());
        const SdrLayerID aOldLayer(pObj->GetLayer());
        const SfxItemSet aOldItemSet(pObj->GetMergedItemSet());
        const SdrGrafObj* pSdrGrafObj = dynamic_cast< SdrGrafObj* >(pObj);
        const SdrTextObj* pSdrTextObj = dynamic_cast< SdrTextObj* >(pObj);

        if(pSdrTextObj && pSdrTextObj->HasText())
        {
            // all text objects are created from ImportText and have no line or fill attributes, so
            // it is okay to concentrate on the text itself
            while(true)
            {
                const basegfx::B2DPolyPolygon aTextContour(pSdrTextObj->TakeContour());
                const basegfx::B2DRange aTextRange(aTextContour.getB2DRange());
                const basegfx::B2DRange aClipRange(maClip.getB2DRange());

                // no overlap -> completely outside
                if(!aClipRange.overlaps(aTextRange))
                {
                    SdrObject::Free(pObj);
                    break;
                }

                // when the clip is a rectangle fast check for inside is possible
                if(basegfx::utils::isRectangle(maClip) && aClipRange.isInside(aTextRange))
                {
                    // completely inside ClipRect
                    break;
                }

                // here text needs to be clipped; to do so, convert to SdrObjects with polygons
                // and add these recursively. Delete original object, do not add in this run
                SdrObject* pConverted = pSdrTextObj->ConvertToPolyObj(true, true).release();
                SdrObject::Free(pObj);

                if(pConverted)
                {
                    // recursively add created conversion; per definition this shall not
                    // contain further SdrTextObjs. Visit only non-group objects
                    SdrObjListIter aIter(*pConverted, SdrIterMode::DeepNoGroups);

                    // work with clones; the created conversion may contain group objects
                    // and when working with the original objects the loop itself could
                    // break and the cleanup later would be pretty complicated (only delete group
                    // objects, are these empty, ...?)
                    while(aIter.IsMore())
                    {
                        SdrObject* pCandidate = aIter.Next();
                        OSL_ENSURE(pCandidate && dynamic_cast< SdrObjGroup* >(pCandidate) ==  nullptr, "SdrObjListIter with SdrIterMode::DeepNoGroups error (!)");
                        SdrObject* pNewClone(pCandidate->CloneSdrObject(pCandidate->getSdrModelFromSdrObject()));

                        if(pNewClone)
                        {
                            InsertObj(pNewClone, false);
                        }
                        else
                        {
                            OSL_ENSURE(false, "SdrObject::Clone() failed (!)");
                        }
                    }

                    // cleanup temporary conversion objects
                    SdrObject::Free(pConverted);
                }

                break;
            }
        }
        else
        {
            BitmapEx aBitmapEx;

            if(pSdrGrafObj)
            {
                aBitmapEx = pSdrGrafObj->GetGraphic().GetBitmapEx();
            }

            SdrObject::Free(pObj);

            if(!aOldRange.isEmpty())
            {
                // clip against ClipRegion
                const basegfx::B2DPolyPolygon aNewPoly(
                    basegfx::utils::clipPolyPolygonOnPolyPolygon(
                        aPoly,
                        maClip,
                        true,
                        !aPoly.isClosed()));
                const basegfx::B2DRange aNewRange(aNewPoly.getB2DRange());

                if(!aNewRange.isEmpty())
                {
                    pObj = new SdrPathObj(
                        *mpModel,
                        aNewPoly.isClosed() ? OBJ_POLY : OBJ_PLIN,
                        aNewPoly);

                    pObj->SetLayer(aOldLayer);
                    pObj->SetMergedItemSet(aOldItemSet);

                    if(!aBitmapEx.IsEmpty())
                    {
                        // aNewRange is inside of aOldRange and defines which part of aBitmapEx is used
                        const double fScaleX(aBitmapEx.GetSizePixel().Width() / (aOldRange.getWidth() ? aOldRange.getWidth() : 1.0));
                        const double fScaleY(aBitmapEx.GetSizePixel().Height() / (aOldRange.getHeight() ? aOldRange.getHeight() : 1.0));
                        basegfx::B2DRange aPixel(aNewRange);
                        basegfx::B2DHomMatrix aTrans;

                        aTrans.translate(-aOldRange.getMinX(), -aOldRange.getMinY());
                        aTrans.scale(fScaleX, fScaleY);
                        aPixel.transform(aTrans);

                        const Size aOrigSizePixel(aBitmapEx.GetSizePixel());
                        const Point aClipTopLeft(
                            basegfx::fround(floor(std::max(0.0, aPixel.getMinX()))),
                            basegfx::fround(floor(std::max(0.0, aPixel.getMinY()))));
                        const Size aClipSize(
                            basegfx::fround(ceil(std::min(static_cast<double>(aOrigSizePixel.Width()), aPixel.getWidth()))),
                            basegfx::fround(ceil(std::min(static_cast<double>(aOrigSizePixel.Height()), aPixel.getHeight()))));
                        const BitmapEx aClippedBitmap(
                            aBitmapEx,
                            aClipTopLeft,
                            aClipSize);

                        pObj->SetMergedItem(XFillStyleItem(drawing::FillStyle_BITMAP));
                        pObj->SetMergedItem(XFillBitmapItem(OUString(), Graphic(aClippedBitmap)));
                        pObj->SetMergedItem(XFillBmpTileItem(false));
                        pObj->SetMergedItem(XFillBmpStretchItem(true));
                    }
                }
            }
        }
    }

    if(!pObj)
        return;

    // #i111954# check object for visibility
    // used are SdrPathObj, SdrRectObj, SdrCircObj, SdrGrafObj
    bool bVisible(false);

    if(pObj->HasLineStyle())
    {
        bVisible = true;
    }

    if(!bVisible && pObj->HasFillStyle())
    {
        bVisible = true;
    }

    if(!bVisible)
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >(pObj);

        if(pTextObj && pTextObj->HasText())
        {
            bVisible = true;
        }
    }

    if(!bVisible)
    {
        SdrGrafObj* pGrafObj = dynamic_cast< SdrGrafObj* >(pObj);

        if(pGrafObj)
        {
            // this may be refined to check if the graphic really is visible. It
            // is here to ensure that graphic objects without fill, line and text
            // get created
            bVisible = true;
        }
    }

    if(!bVisible)
    {
        SdrObject::Free(pObj);
    }
    else
    {
        maTmpList.push_back(pObj);

        if(dynamic_cast< SdrPathObj* >(pObj))
        {
            const bool bClosed(pObj->IsClosedObj());

            mbLastObjWasPolyWithoutLine = mbNoLine && bClosed;
            mbLastObjWasLine = !bClosed;
        }
        else
        {
            mbLastObjWasPolyWithoutLine = false;
            mbLastObjWasLine = false;
        }
    }
}

void ImpSdrGDIMetaFileImport::DoAction(MetaLineAction const & rAct)
{
    // #i73407# reformulation to use new B2DPolygon classes
    const basegfx::B2DPoint aStart(rAct.GetStartPoint().X(), rAct.GetStartPoint().Y());
    const basegfx::B2DPoint aEnd(rAct.GetEndPoint().X(), rAct.GetEndPoint().Y());

    if(aStart.equal(aEnd))
        return;

    basegfx::B2DPolygon aLine;
    const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));

    aLine.append(aStart);
    aLine.append(aEnd);
    aLine.transform(aTransform);

    const LineInfo& rLineInfo = rAct.GetLineInfo();
    const sal_Int32 nNewLineWidth(rLineInfo.GetWidth());
    bool bCreateLineObject(true);

    if(mbLastObjWasLine && (nNewLineWidth == mnLineWidth) && CheckLastLineMerge(aLine))
    {
        bCreateLineObject = false;
    }

    if(!bCreateLineObject)
        return;

    SdrPathObj* pPath = new SdrPathObj(
        *mpModel,
        OBJ_LINE,
        basegfx::B2DPolyPolygon(aLine));
    mnLineWidth = nNewLineWidth;
    maLineJoin = rLineInfo.GetLineJoin();
    maLineCap = rLineInfo.GetLineCap();
    maDash = XDash(css::drawing::DashStyle_RECT,
        rLineInfo.GetDotCount(), rLineInfo.GetDotLen(),
        rLineInfo.GetDashCount(), rLineInfo.GetDashLen(),
        rLineInfo.GetDistance());
    SetAttributes(pPath);
    mnLineWidth = 0;
    maLineJoin = basegfx::B2DLineJoin::NONE;
    maDash = XDash();
    InsertObj(pPath, false);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaRectAction const & rAct)
{
    SdrRectObj* pRect = new SdrRectObj(
        *mpModel,
        rAct.GetRect());
    SetAttributes(pRect);
    InsertObj(pRect);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaRoundRectAction const & rAct)
{
    SdrRectObj* pRect = new SdrRectObj(
        *mpModel,
        rAct.GetRect());
    SetAttributes(pRect);
    tools::Long nRad=(rAct.GetHorzRound()+rAct.GetVertRound())/2;
    if (nRad!=0) {
        SfxItemSetFixed<SDRATTR_CORNER_RADIUS, SDRATTR_CORNER_RADIUS> aSet(*mpLineAttr->GetPool());
        aSet.Put(SdrMetricItem(SDRATTR_CORNER_RADIUS, nRad));
        pRect->SetMergedItemSet(aSet);
    }
    InsertObj(pRect);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaEllipseAction const & rAct)
{
    SdrCircObj* pCirc=new SdrCircObj(
        *mpModel,
        SdrCircKind::Full,
        rAct.GetRect());
    SetAttributes(pCirc);
    InsertObj(pCirc);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaArcAction const & rAct)
{
    Point aCenter(rAct.GetRect().Center());
    Degree100 nStart=GetAngle(rAct.GetStartPoint()-aCenter);
    Degree100 nEnd=GetAngle(rAct.GetEndPoint()-aCenter);
    SdrCircObj* pCirc = new SdrCircObj(
        *mpModel,
        SdrCircKind::Arc,
        rAct.GetRect(),nStart,nEnd);
    SetAttributes(pCirc);
    InsertObj(pCirc);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaPieAction const & rAct)
{
    Point aCenter(rAct.GetRect().Center());
    Degree100 nStart=GetAngle(rAct.GetStartPoint()-aCenter);
    Degree100 nEnd=GetAngle(rAct.GetEndPoint()-aCenter);
    SdrCircObj* pCirc = new SdrCircObj(
        *mpModel,
        SdrCircKind::Section,
        rAct.GetRect(),
        nStart,
        nEnd);
    SetAttributes(pCirc);
    InsertObj(pCirc);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaChordAction const & rAct)
{
    Point aCenter(rAct.GetRect().Center());
    Degree100 nStart=GetAngle(rAct.GetStartPoint()-aCenter);
    Degree100 nEnd=GetAngle(rAct.GetEndPoint()-aCenter);
    SdrCircObj* pCirc = new SdrCircObj(
        *mpModel,
        SdrCircKind::Cut,
        rAct.GetRect(),
        nStart,
        nEnd);
    SetAttributes(pCirc);
    InsertObj(pCirc);
}

bool ImpSdrGDIMetaFileImport::CheckLastLineMerge(const basegfx::B2DPolygon& rSrcPoly)
{
    // #i102706# Do not merge closed polygons
    if(rSrcPoly.isClosed())
    {
        return false;
    }

    // #i73407# reformulation to use new B2DPolygon classes
    if(mbLastObjWasLine && (maOldLineColor == mpVD->GetLineColor()) && rSrcPoly.count())
    {
        SdrObject* pTmpObj = !maTmpList.empty() ? maTmpList[maTmpList.size() - 1] : nullptr;
        SdrPathObj* pLastPoly = dynamic_cast< SdrPathObj* >(pTmpObj);

        if(pLastPoly)
        {
            if(1 == pLastPoly->GetPathPoly().count())
            {
                bool bOk(false);
                basegfx::B2DPolygon aDstPoly(pLastPoly->GetPathPoly().getB2DPolygon(0));

                // #i102706# Do not merge closed polygons
                if(aDstPoly.isClosed())
                {
                    return false;
                }

                if(aDstPoly.count())
                {
                    const sal_uInt32 nMaxDstPnt(aDstPoly.count() - 1);
                    const sal_uInt32 nMaxSrcPnt(rSrcPoly.count() - 1);

                    if(aDstPoly.getB2DPoint(nMaxDstPnt) == rSrcPoly.getB2DPoint(0))
                    {
                        aDstPoly.append(rSrcPoly, 1, rSrcPoly.count() - 1);
                        bOk = true;
                    }
                    else if(aDstPoly.getB2DPoint(0) == rSrcPoly.getB2DPoint(nMaxSrcPnt))
                    {
                        basegfx::B2DPolygon aNew(rSrcPoly);
                        aNew.append(aDstPoly, 1, aDstPoly.count() - 1);
                        aDstPoly = aNew;
                        bOk = true;
                    }
                    else if(aDstPoly.getB2DPoint(0) == rSrcPoly.getB2DPoint(0))
                    {
                        aDstPoly.flip();
                        aDstPoly.append(rSrcPoly, 1, rSrcPoly.count() - 1);
                        bOk = true;
                    }
                    else if(aDstPoly.getB2DPoint(nMaxDstPnt) == rSrcPoly.getB2DPoint(nMaxSrcPnt))
                    {
                        basegfx::B2DPolygon aNew(rSrcPoly);
                        aNew.flip();
                        aDstPoly.append(aNew, 1, aNew.count() - 1);
                        bOk = true;
                    }
                }

                if(bOk)
                {
                    pLastPoly->NbcSetPathPoly(basegfx::B2DPolyPolygon(aDstPoly));
                }

                return bOk;
            }
        }
    }

    return false;
}

bool ImpSdrGDIMetaFileImport::CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon & rPolyPolygon)
{
    // #i73407# reformulation to use new B2DPolygon classes
    if(mbLastObjWasPolyWithoutLine)
    {
        SdrObject* pTmpObj = !maTmpList.empty() ? maTmpList[maTmpList.size() - 1] : nullptr;
        SdrPathObj* pLastPoly = dynamic_cast< SdrPathObj* >(pTmpObj);

        if(pLastPoly)
        {
            if(pLastPoly->GetPathPoly() == rPolyPolygon)
            {
                SetAttributes(nullptr);

                if(!mbNoLine && mbNoFill)
                {
                    pLastPoly->SetMergedItemSet(*mpLineAttr);

                    return true;
                }
            }
        }
    }

    return false;
}

void ImpSdrGDIMetaFileImport::checkClip()
{
    if(!mpVD->IsClipRegion())
        return;

    maClip = mpVD->GetClipRegion().GetAsB2DPolyPolygon();

    if(isClip())
    {
        const basegfx::B2DHomMatrix aTransform(
            basegfx::utils::createScaleTranslateB2DHomMatrix(
                mfScaleX,
                mfScaleY,
                maOfs.X(),
                maOfs.Y()));

        maClip.transform(aTransform);
    }
}

bool ImpSdrGDIMetaFileImport::isClip() const
{
    return !maClip.getB2DRange().isEmpty();
}

void ImpSdrGDIMetaFileImport::DoAction( MetaPolyLineAction const & rAct )
{
    // #i73407# reformulation to use new B2DPolygon classes
    basegfx::B2DPolygon aSource(rAct.GetPolygon().getB2DPolygon());

    if(aSource.count())
    {
        const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));
        aSource.transform(aTransform);
    }

    const LineInfo& rLineInfo = rAct.GetLineInfo();
    const sal_Int32 nNewLineWidth(rLineInfo.GetWidth());
    bool bCreateLineObject(true);

    if(mbLastObjWasLine && (nNewLineWidth == mnLineWidth) && CheckLastLineMerge(aSource))
    {
        bCreateLineObject = false;
    }
    else if(mbLastObjWasPolyWithoutLine && CheckLastPolyLineAndFillMerge(basegfx::B2DPolyPolygon(aSource)))
    {
        bCreateLineObject = false;
    }

    if(!bCreateLineObject)
        return;

    SdrPathObj* pPath = new SdrPathObj(
        *mpModel,
        aSource.isClosed() ? OBJ_POLY : OBJ_PLIN,
        basegfx::B2DPolyPolygon(aSource));
    mnLineWidth = nNewLineWidth;
    maLineJoin = rLineInfo.GetLineJoin();
    maLineCap = rLineInfo.GetLineCap();
    maDash = XDash(css::drawing::DashStyle_RECT,
        rLineInfo.GetDotCount(), rLineInfo.GetDotLen(),
        rLineInfo.GetDashCount(), rLineInfo.GetDashLen(),
        rLineInfo.GetDistance());
    SetAttributes(pPath);
    mnLineWidth = 0;
    maLineJoin = basegfx::B2DLineJoin::NONE;
    maDash = XDash();
    InsertObj(pPath, false);
}

void ImpSdrGDIMetaFileImport::DoAction( MetaPolygonAction const & rAct )
{
    // #i73407# reformulation to use new B2DPolygon classes
    basegfx::B2DPolygon aSource(rAct.GetPolygon().getB2DPolygon());

    if(!aSource.count())
        return;

    const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));
    aSource.transform(aTransform);

    if(!mbLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(basegfx::B2DPolyPolygon(aSource)))
    {
        // #i73407# make sure polygon is closed, it's a filled primitive
        aSource.setClosed(true);
        SdrPathObj* pPath = new SdrPathObj(
            *mpModel,
            OBJ_POLY,
            basegfx::B2DPolyPolygon(aSource));
        SetAttributes(pPath);
        InsertObj(pPath, false);
    }
}

void ImpSdrGDIMetaFileImport::DoAction(MetaPolyPolygonAction const & rAct)
{
    // #i73407# reformulation to use new B2DPolygon classes
    basegfx::B2DPolyPolygon aSource(rAct.GetPolyPolygon().getB2DPolyPolygon());

    if(!aSource.count())
        return;

    const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));
    aSource.transform(aTransform);

    if(!mbLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(aSource))
    {
        // #i73407# make sure polygon is closed, it's a filled primitive
        aSource.setClosed(true);
        SdrPathObj* pPath = new SdrPathObj(
            *mpModel,
            OBJ_POLY,
            aSource);
        SetAttributes(pPath);
        InsertObj(pPath, false);
    }
}

void ImpSdrGDIMetaFileImport::ImportText( const Point& rPos, const OUString& rStr, const MetaAction& rAct )
{
    // calc text box size, add 5% to make it fit safely

    FontMetric aFontMetric( mpVD->GetFontMetric() );
    vcl::Font aFnt( mpVD->GetFont() );
    TextAlign eAlg( aFnt.GetAlignment() );

    sal_Int32 nTextWidth = static_cast<sal_Int32>( mpVD->GetTextWidth( rStr ) * mfScaleX );
    sal_Int32 nTextHeight = static_cast<sal_Int32>( mpVD->GetTextHeight() * mfScaleY );

    Point aPos( FRound(rPos.X() * mfScaleX + maOfs.X()), FRound(rPos.Y() * mfScaleY + maOfs.Y()) );
    Size aSize( nTextWidth, nTextHeight );

    if ( eAlg == ALIGN_BASELINE )
        aPos.AdjustY( -(FRound(aFontMetric.GetAscent() * mfScaleY)) );
    else if ( eAlg == ALIGN_BOTTOM )
        aPos.AdjustY( -nTextHeight );

    tools::Rectangle aTextRect( aPos, aSize );
    SdrRectObj* pText = new SdrRectObj(
        *mpModel,
        OBJ_TEXT,
        aTextRect);

    pText->SetMergedItem ( makeSdrTextUpperDistItem (0));
    pText->SetMergedItem ( makeSdrTextLowerDistItem (0));
    pText->SetMergedItem ( makeSdrTextRightDistItem (0));
    pText->SetMergedItem ( makeSdrTextLeftDistItem (0));

    if ( aFnt.GetAverageFontWidth() || ( rAct.GetType() == MetaActionType::STRETCHTEXT ) )
    {
        pText->ClearMergedItem( SDRATTR_TEXT_AUTOGROWWIDTH );
        pText->SetMergedItem( makeSdrTextAutoGrowHeightItem( false ) );
        // don't let the margins eat the space needed for the text
        pText->SetMergedItem( SdrTextFitToSizeTypeItem(drawing::TextFitToSizeType_ALLLINES) );
    }
    else
    {
        pText->SetMergedItem( makeSdrTextAutoGrowWidthItem( true ) );
    }

    pText->SetLayer(mnLayer);
    pText->NbcSetText( rStr );
    SetAttributes( pText, true );
    pText->SetSnapRect( aTextRect );

    if (!aFnt.IsTransparent())
    {
        SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST> aAttr(*mpFillAttr->GetPool());
        aAttr.Put(XFillStyleItem(drawing::FillStyle_SOLID));
        aAttr.Put(XFillColorItem(OUString(), aFnt.GetFillColor()));
        pText->SetMergedItemSet(aAttr);
    }
    Degree100 nAngle = to<Degree100>(aFnt.GetOrientation());
    if ( nAngle )
        pText->SdrAttrObj::NbcRotate(aPos,nAngle);
    InsertObj( pText, false );
}

void ImpSdrGDIMetaFileImport::DoAction(MetaTextAction const & rAct)
{
    OUString aStr(rAct.GetText());
    aStr = aStr.copy(rAct.GetIndex(), rAct.GetLen());
    ImportText( rAct.GetPoint(), aStr, rAct );
}

void ImpSdrGDIMetaFileImport::DoAction(MetaTextArrayAction const & rAct)
{
    OUString aStr(rAct.GetText());
    aStr = aStr.copy(rAct.GetIndex(), rAct.GetLen());
    ImportText( rAct.GetPoint(), aStr, rAct );
}

void ImpSdrGDIMetaFileImport::DoAction(MetaStretchTextAction const & rAct)
{
    OUString aStr(rAct.GetText());
    aStr = aStr.copy(rAct.GetIndex(), rAct.GetLen());
    ImportText( rAct.GetPoint(), aStr, rAct );
}

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpAction const & rAct)
{
    tools::Rectangle aRect(rAct.GetPoint(),rAct.GetBitmap().GetSizePixel());
    aRect.AdjustRight( 1 ); aRect.AdjustBottom( 1 );
    SdrGrafObj* pGraf = new SdrGrafObj(
        *mpModel,
        Graphic(BitmapEx(rAct.GetBitmap())),
        aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpScaleAction const & rAct)
{
    tools::Rectangle aRect(rAct.GetPoint(),rAct.GetSize());
    aRect.AdjustRight( 1 ); aRect.AdjustBottom( 1 );
    SdrGrafObj* pGraf = new SdrGrafObj(
        *mpModel,
        Graphic(BitmapEx(rAct.GetBitmap())),
        aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpExAction const & rAct)
{
    tools::Rectangle aRect(rAct.GetPoint(),rAct.GetBitmapEx().GetSizePixel());
    aRect.AdjustRight( 1 ); aRect.AdjustBottom( 1 );
    SdrGrafObj* pGraf = new SdrGrafObj(
        *mpModel,
        rAct.GetBitmapEx(),
        aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpExScaleAction const & rAct)
{
    tools::Rectangle aRect(rAct.GetPoint(),rAct.GetSize());
    aRect.AdjustRight( 1 ); aRect.AdjustBottom( 1 );
    SdrGrafObj* pGraf = new SdrGrafObj(
        *mpModel,
        rAct.GetBitmapEx(),
        aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}


void ImpSdrGDIMetaFileImport::DoAction( MetaHatchAction const & rAct )
{
    // #i73407# reformulation to use new B2DPolygon classes
    basegfx::B2DPolyPolygon aSource(rAct.GetPolyPolygon().getB2DPolyPolygon());

    if(!aSource.count())
        return;

    const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));
    aSource.transform(aTransform);

    if(mbLastObjWasPolyWithoutLine && CheckLastPolyLineAndFillMerge(aSource))
        return;

    const Hatch& rHatch = rAct.GetHatch();
    SdrPathObj* pPath = new SdrPathObj(
        *mpModel,
        OBJ_POLY,
        aSource);
    // #i125211# Use the ranges from the SdrObject to create a new empty SfxItemSet
    SfxItemSet aHatchAttr(mpModel->GetItemPool(), pPath->GetMergedItemSet().GetRanges());
    css::drawing::HatchStyle eStyle;

    switch(rHatch.GetStyle())
    {
        case HatchStyle::Triple :
        {
            eStyle = css::drawing::HatchStyle_TRIPLE;
            break;
        }

        case HatchStyle::Double :
        {
            eStyle = css::drawing::HatchStyle_DOUBLE;
            break;
        }

        default:
        {
            eStyle = css::drawing::HatchStyle_SINGLE;
            break;
        }
    }

    SetAttributes(pPath);
    aHatchAttr.Put(XFillStyleItem(drawing::FillStyle_HATCH));
    aHatchAttr.Put(XFillHatchItem(XHatch(rHatch.GetColor(), eStyle, rHatch.GetDistance(), rHatch.GetAngle())));
    pPath->SetMergedItemSet(aHatchAttr);

    InsertObj(pPath, false);
}


void ImpSdrGDIMetaFileImport::DoAction(MetaLineColorAction& rAct)
{
    rAct.Execute(mpVD);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaMapModeAction& rAct)
{
    MapScaling();
    rAct.Execute(mpVD);
    mbLastObjWasPolyWithoutLine = false;
    mbLastObjWasLine = false;
}

void ImpSdrGDIMetaFileImport::MapScaling()
{
    const size_t nCount(maTmpList.size());
    const MapMode& rMap = mpVD->GetMapMode();
    Point aMapOrg( rMap.GetOrigin() );
    bool bMov2(aMapOrg.X() != 0 || aMapOrg.Y() != 0);

    if(bMov2)
    {
        for(size_t i = mnMapScalingOfs; i < nCount; i++)
        {
            SdrObject* pObj = maTmpList[i];

            pObj->NbcMove(Size(aMapOrg.X(), aMapOrg.Y()));
        }
    }

    mnMapScalingOfs = nCount;
}


void ImpSdrGDIMetaFileImport::DoAction( MetaCommentAction const & rAct, GDIMetaFile const & rMtf, sal_uLong& a) // GDIMetaFile* pMtf )
{
    bool aSkipComment = false;

    if (a < rMtf.GetActionSize() && rAct.GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_BEGIN"))
    {
        // #i125211# Check if next action is a MetaGradientExAction
        MetaGradientExAction* pAct = dynamic_cast< MetaGradientExAction* >(rMtf.GetAction(a + 1));

        if( pAct && pAct->GetType() == MetaActionType::GRADIENTEX )
        {
            // #i73407# reformulation to use new B2DPolygon classes
            basegfx::B2DPolyPolygon aSource(pAct->GetPolyPolygon().getB2DPolyPolygon());

            if(aSource.count())
            {
                if(!mbLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(aSource))
                {
                    const Gradient& rGrad = pAct->GetGradient();
                    SdrPathObj* pPath = new SdrPathObj(
                        *mpModel,
                        OBJ_POLY,
                        aSource);
                    // #i125211# Use the ranges from the SdrObject to create a new empty SfxItemSet
                    SfxItemSet aGradAttr(mpModel->GetItemPool(), pPath->GetMergedItemSet().GetRanges());
                    XGradient aXGradient;

                    aXGradient.SetGradientStyle(static_cast<css::awt::GradientStyle>(rGrad.GetStyle()));
                    aXGradient.SetStartColor(rGrad.GetStartColor());
                    aXGradient.SetEndColor(rGrad.GetEndColor());
                    aXGradient.SetAngle(rGrad.GetAngle());
                    aXGradient.SetBorder(rGrad.GetBorder());
                    aXGradient.SetXOffset(rGrad.GetOfsX());
                    aXGradient.SetYOffset(rGrad.GetOfsY());
                    aXGradient.SetStartIntens(rGrad.GetStartIntensity());
                    aXGradient.SetEndIntens(rGrad.GetEndIntensity());
                    aXGradient.SetSteps(rGrad.GetSteps());

                    // no need to use SetAttributes(..) here since line and fill style
                    // need to be set individually
                    // SetAttributes(pPath);

                    // switch line off; if there was one there will be a
                    // MetaActionType::POLYLINE following creating another object
                    aGradAttr.Put(XLineStyleItem(drawing::LineStyle_NONE));

                    // add detected gradient fillstyle
                    aGradAttr.Put(XFillStyleItem(drawing::FillStyle_GRADIENT));
                    aGradAttr.Put(XFillGradientItem(aXGradient));

                    pPath->SetMergedItemSet(aGradAttr);

                    InsertObj(pPath);
                }
            }

            aSkipComment = true;
        }
    }

    if(aSkipComment)
    {
        // #i125211# forward until closing MetaCommentAction
        MetaAction* pSkipAct = rMtf.GetAction(++a);

        while( pSkipAct
            && ((pSkipAct->GetType() != MetaActionType::COMMENT )
                || !(static_cast<MetaCommentAction*>(pSkipAct)->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_END"))))
        {
            pSkipAct = rMtf.GetAction(++a);
        }
    }
}

void ImpSdrGDIMetaFileImport::DoAction(MetaTextRectAction const & rAct)
{
    GDIMetaFile aTemp;

    mpVD->AddTextRectActions(rAct.GetRect(), rAct.GetText(), rAct.GetStyle(), aTemp);
    DoLoopActions(aTemp, nullptr, nullptr);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpScalePartAction const & rAct)
{
    tools::Rectangle aRect(rAct.GetDestPoint(), rAct.GetDestSize());
    BitmapEx aBitmapEx(rAct.GetBitmap());

    aRect.AdjustRight( 1 );
    aRect.AdjustBottom( 1 );
    aBitmapEx.Crop(tools::Rectangle(rAct.GetSrcPoint(), rAct.GetSrcSize()));
    SdrGrafObj* pGraf = new SdrGrafObj(
        *mpModel,
        aBitmapEx,
        aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpExScalePartAction const & rAct)
{
    tools::Rectangle aRect(rAct.GetDestPoint(),rAct.GetDestSize());
    BitmapEx aBitmapEx(rAct.GetBitmapEx());

    aRect.AdjustRight( 1 );
    aRect.AdjustBottom( 1 );
    aBitmapEx.Crop(tools::Rectangle(rAct.GetSrcPoint(), rAct.GetSrcSize()));
    SdrGrafObj* pGraf = new SdrGrafObj(
        *mpModel,
        aBitmapEx,
        aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaMaskAction const & rAct)
{
    tools::Rectangle aRect(rAct.GetPoint(), rAct.GetBitmap().GetSizePixel());
    BitmapEx aBitmapEx(rAct.GetBitmap(), rAct.GetColor());

    aRect.AdjustRight( 1 ); aRect.AdjustBottom( 1 );
    SdrGrafObj* pGraf = new SdrGrafObj(
        *mpModel,
        aBitmapEx,
        aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaMaskScaleAction const & rAct)
{
    tools::Rectangle aRect(rAct.GetPoint(), rAct.GetSize());
    BitmapEx aBitmapEx(rAct.GetBitmap(), rAct.GetColor());

    aRect.AdjustRight( 1 ); aRect.AdjustBottom( 1 );
    SdrGrafObj* pGraf = new SdrGrafObj(
        *mpModel,
        aBitmapEx,
        aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaMaskScalePartAction const & rAct)
{
    tools::Rectangle aRect(rAct.GetDestPoint(), rAct.GetDestSize());
    BitmapEx aBitmapEx(rAct.GetBitmap(), rAct.GetColor());

    aRect.AdjustRight( 1 ); aRect.AdjustBottom( 1 );
    aBitmapEx.Crop(tools::Rectangle(rAct.GetSrcPoint(), rAct.GetSrcSize()));
    SdrGrafObj* pGraf = new SdrGrafObj(
        *mpModel,
        aBitmapEx,
        aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}

namespace
{
    css::awt::GradientStyle getXGradientStyleFromGradientStyle(const GradientStyle& rGradientStyle)
    {
        css::awt::GradientStyle aXGradientStyle(css::awt::GradientStyle_LINEAR);

        switch(rGradientStyle)
        {
            case GradientStyle::Linear: aXGradientStyle = css::awt::GradientStyle_LINEAR; break;
            case GradientStyle::Axial: aXGradientStyle = css::awt::GradientStyle_AXIAL; break;
            case GradientStyle::Radial: aXGradientStyle = css::awt::GradientStyle_RADIAL; break;
            case GradientStyle::Elliptical: aXGradientStyle = css::awt::GradientStyle_ELLIPTICAL; break;
            case GradientStyle::Square: aXGradientStyle = css::awt::GradientStyle_SQUARE; break;
            case GradientStyle::Rect: aXGradientStyle = css::awt::GradientStyle_RECT; break;

            // Needed due to GradientStyle::FORCE_EQUAL_SIZE; this again is needed
            // to force the enum defines in VCL to a defined size for the compilers,
            // so despite it is never used it cannot be removed (would break the
            // API implementation probably).
            case GradientStyle::FORCE_EQUAL_SIZE: break;
            default:
                break;
        }

        return aXGradientStyle;
    }
}

void ImpSdrGDIMetaFileImport::DoAction(MetaGradientAction const & rAct)
{
    basegfx::B2DRange aRange = vcl::unotools::b2DRectangleFromRectangle(rAct.GetRect());

    if(aRange.isEmpty())
        return;

    const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));
    aRange.transform(aTransform);
    const Gradient& rGradient = rAct.GetGradient();
    SdrRectObj* pRect = new SdrRectObj(
        *mpModel,
        tools::Rectangle(
            floor(aRange.getMinX()),
            floor(aRange.getMinY()),
            ceil(aRange.getMaxX()),
            ceil(aRange.getMaxY())));
    // #i125211# Use the ranges from the SdrObject to create a new empty SfxItemSet
    SfxItemSet aGradientAttr(mpModel->GetItemPool(), pRect->GetMergedItemSet().GetRanges());
    const css::awt::GradientStyle aXGradientStyle(getXGradientStyleFromGradientStyle(rGradient.GetStyle()));
    const XFillGradientItem aXFillGradientItem(
        XGradient(
            rGradient.GetStartColor(),
            rGradient.GetEndColor(),
            aXGradientStyle,
            rGradient.GetAngle(),
            rGradient.GetOfsX(),
            rGradient.GetOfsY(),
            rGradient.GetBorder(),
            rGradient.GetStartIntensity(),
            rGradient.GetEndIntensity(),
            rGradient.GetSteps()));

    SetAttributes(pRect);
    aGradientAttr.Put(XFillStyleItem(drawing::FillStyle_GRADIENT)); // #i125211#
    aGradientAttr.Put(aXFillGradientItem);
    pRect->SetMergedItemSet(aGradientAttr);

    InsertObj(pRect, false);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaTransparentAction const & rAct)
{
    basegfx::B2DPolyPolygon aSource(rAct.GetPolyPolygon().getB2DPolyPolygon());

    if(!aSource.count())
        return;

    const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));
    aSource.transform(aTransform);
    aSource.setClosed(true);

    SdrPathObj* pPath = new SdrPathObj(
        *mpModel,
        OBJ_POLY,
        aSource);
    SetAttributes(pPath);
    pPath->SetMergedItem(XFillTransparenceItem(rAct.GetTransparence()));
    InsertObj(pPath, false);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaGradientExAction const & rAct)
{
    basegfx::B2DPolyPolygon aSource(rAct.GetPolyPolygon().getB2DPolyPolygon());

    if(!aSource.count())
        return;

    const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));
    aSource.transform(aTransform);

    if(mbLastObjWasPolyWithoutLine && CheckLastPolyLineAndFillMerge(aSource))
        return;

    const Gradient& rGradient = rAct.GetGradient();
    SdrPathObj* pPath = new SdrPathObj(
        *mpModel,
        OBJ_POLY,
        aSource);
    // #i125211# Use the ranges from the SdrObject to create a new empty SfxItemSet
    SfxItemSet aGradientAttr(mpModel->GetItemPool(), pPath->GetMergedItemSet().GetRanges());
    const css::awt::GradientStyle aXGradientStyle(getXGradientStyleFromGradientStyle(rGradient.GetStyle()));
    const XFillGradientItem aXFillGradientItem(
        XGradient(
            rGradient.GetStartColor(),
            rGradient.GetEndColor(),
            aXGradientStyle,
            rGradient.GetAngle(),
            rGradient.GetOfsX(),
            rGradient.GetOfsY(),
            rGradient.GetBorder(),
            rGradient.GetStartIntensity(),
            rGradient.GetEndIntensity(),
            rGradient.GetSteps()));

    SetAttributes(pPath);
    aGradientAttr.Put(XFillStyleItem(drawing::FillStyle_GRADIENT)); // #i125211#
    aGradientAttr.Put(aXFillGradientItem);
    pPath->SetMergedItemSet(aGradientAttr);

    InsertObj(pPath, false);
}

void ImpSdrGDIMetaFileImport::DoAction(MetaFloatTransparentAction const & rAct)
{
    const GDIMetaFile& rMtf = rAct.GetGDIMetaFile();

    if(!rMtf.GetActionSize())
        return;

    const tools::Rectangle aRect(rAct.GetPoint(),rAct.GetSize());

    // convert metafile sub-content to BitmapEx
    BitmapEx aBitmapEx(
        convertMetafileToBitmapEx(
            rMtf,
            vcl::unotools::b2DRectangleFromRectangle(aRect),
            125000));

    // handle colors
    const Gradient& rGradient = rAct.GetGradient();
    basegfx::BColor aStart(rGradient.GetStartColor().getBColor());
    basegfx::BColor aEnd(rGradient.GetEndColor().getBColor());

    if(100 != rGradient.GetStartIntensity())
    {
        aStart *= static_cast<double>(rGradient.GetStartIntensity()) / 100.0;
    }

    if(100 != rGradient.GetEndIntensity())
    {
        aEnd *= static_cast<double>(rGradient.GetEndIntensity()) / 100.0;
    }

    const bool bEqualColors(aStart == aEnd);
    const bool bNoSteps(1 == rGradient.GetSteps());
    bool bCreateObject(true);
    bool bHasNewMask(false);
    AlphaMask aNewMask;
    double fTransparence(0.0);
    bool bFixedTransparence(false);

    if(bEqualColors || bNoSteps)
    {
        // single transparence
        const basegfx::BColor aMedium(basegfx::average(aStart, aEnd));
        fTransparence = aMedium.luminance();

        if(basegfx::fTools::lessOrEqual(fTransparence, 0.0))
        {
            // no transparence needed, all done
        }
        else if(basegfx::fTools::moreOrEqual(fTransparence, 1.0))
        {
            // all transparent, no object
            bCreateObject = false;
        }
        else
        {
            // 0.0 < transparence < 1.0, apply fixed transparence
            bFixedTransparence = true;
        }
    }
    else
    {
        // gradient transparence
        ScopedVclPtrInstance< VirtualDevice > pVDev;

        pVDev->SetOutputSizePixel(aBitmapEx.GetBitmap().GetSizePixel());
        pVDev->DrawGradient(tools::Rectangle(Point(0, 0), pVDev->GetOutputSizePixel()), rGradient);

        aNewMask = AlphaMask(pVDev->GetBitmap(Point(0, 0), pVDev->GetOutputSizePixel()));
        bHasNewMask = true;
    }

    if(!bCreateObject)
        return;

    if(bHasNewMask || bFixedTransparence)
    {
        if(!aBitmapEx.IsAlpha())
        {
            // no transparence yet, apply new one
            if(bFixedTransparence)
            {
                sal_uInt8 aAlpha(basegfx::fround((1.0 - fTransparence) * 255.0));

                aNewMask = AlphaMask(aBitmapEx.GetBitmap().GetSizePixel(), &aAlpha);
            }

            aBitmapEx = BitmapEx(aBitmapEx.GetBitmap(), aNewMask);
        }
        else
        {
            vcl::bitmap::DrawAlphaBitmapAndAlphaGradient(aBitmapEx, bFixedTransparence, fTransparence, aNewMask);
        }
    }

    // create and add object
    SdrGrafObj* pGraf = new SdrGrafObj(
        *mpModel,
        aBitmapEx,
        aRect);

    // for MetaFloatTransparentAction, do not use SetAttributes(...)
    // since these metafile content is not used to draw line/fill
    // dependent of these setting at the device content
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
