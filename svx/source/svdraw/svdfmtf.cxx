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

#include "svdfmtf.hxx"
#include <editeng/editdata.hxx>
#include <math.h>
#include <svx/xpoly.hxx>
#include <vcl/svapp.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/shdditem.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/akrnitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/colritem.hxx>
#include <vcl/metric.hxx>
#include <editeng/charscaleitem.hxx>
#include <svx/xflhtit.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include "svx/svditext.hxx"
#include <svx/svdotext.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdetc.hxx>
#include <svl/itemset.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <vcl/salbtype.hxx>     // FRound
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/xlndsit.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xfltrit.hxx>
#include <vcl/bmpacc.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>
#include <svx/svdpntv.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/svdtrans.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/svditer.hxx>
#include <svx/svdogrp.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

ImpSdrGDIMetaFileImport::ImpSdrGDIMetaFileImport(
    SdrModel& rModel,
    SdrLayerID nLay,
    const basegfx::B2DHomMatrix& rObjectTransform)
:   maTmpList(),
    maVD(),
    maObjectTransform(rObjectTransform),
    maMetaToUnit(),
    maCurrent(),
    mpLineAttr(0),
    mpFillAttr(0),
    mpTextAttr(0),
    mrModel(rModel),
    mnLayer(nLay),
    maOldLineColor(),
    mnLineWidth(0),
    maLineJoin(basegfx::B2DLINEJOIN_NONE),
    maLineCap(com::sun::star::drawing::LineCap_BUTT),
    maDash(XDASH_RECT, 0, 0, 0, 0, 0),
    maClip(),
    mbFntDirty(true),
    mbLastObjWasPolyWithoutLine(false),
    mbNoLine(false),
    mbNoFill(false),
    mbLastObjWasLine(false)
{
    maVD.EnableOutput(false);
    maVD.SetLineColor();
    maVD.SetFillColor();
    maOldLineColor.SetRed(maVD.GetLineColor().GetRed() + 1);
    mpLineAttr = new SfxItemSet(rModel.GetItemPool(), XATTR_LINE_FIRST, XATTR_LINE_LAST, 0, 0);
    mpFillAttr = new SfxItemSet(rModel.GetItemPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST, 0, 0);
    mpTextAttr = new SfxItemSet(rModel.GetItemPool(), EE_ITEMS_START, EE_ITEMS_END, 0, 0);
    checkClip();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ImpSdrGDIMetaFileImport::~ImpSdrGDIMetaFileImport()
{
    delete mpLineAttr;
    delete mpFillAttr;
    delete mpTextAttr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoLoopActions(GDIMetaFile& rMtf, SvdProgressInfo* pProgrInfo, sal_uInt32* pActionsToReport)
{
    const sal_uLong nCount(rMtf.GetActionCount());

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
            case META_PIXEL_ACTION          : DoAction((MetaPixelAction          &)*pAct); break;
            case META_POINT_ACTION          : DoAction((MetaPointAction          &)*pAct); break;
            case META_LINE_ACTION           : DoAction((MetaLineAction           &)*pAct); break;
            case META_RECT_ACTION           : DoAction((MetaRectAction           &)*pAct); break;
            case META_ROUNDRECT_ACTION      : DoAction((MetaRoundRectAction      &)*pAct); break;
            case META_ELLIPSE_ACTION        : DoAction((MetaEllipseAction        &)*pAct); break;
            case META_ARC_ACTION            : DoAction((MetaArcAction            &)*pAct); break;
            case META_PIE_ACTION            : DoAction((MetaPieAction            &)*pAct); break;
            case META_CHORD_ACTION          : DoAction((MetaChordAction          &)*pAct); break;
            case META_POLYLINE_ACTION       : DoAction((MetaPolyLineAction       &)*pAct); break;
            case META_POLYGON_ACTION        : DoAction((MetaPolygonAction        &)*pAct); break;
            case META_POLYPOLYGON_ACTION    : DoAction((MetaPolyPolygonAction    &)*pAct); break;
            case META_TEXT_ACTION           : DoAction((MetaTextAction           &)*pAct); break;
            case META_TEXTARRAY_ACTION      : DoAction((MetaTextArrayAction      &)*pAct); break;
            case META_STRETCHTEXT_ACTION    : DoAction((MetaStretchTextAction    &)*pAct); break;
            case META_BMP_ACTION            : DoAction((MetaBmpAction            &)*pAct); break;
            case META_BMPSCALE_ACTION       : DoAction((MetaBmpScaleAction       &)*pAct); break;
            case META_BMPEX_ACTION          : DoAction((MetaBmpExAction          &)*pAct); break;
            case META_BMPEXSCALE_ACTION     : DoAction((MetaBmpExScaleAction     &)*pAct); break;
            case META_LINECOLOR_ACTION      : DoAction((MetaLineColorAction      &)*pAct); break;
            case META_FILLCOLOR_ACTION      : DoAction((MetaFillColorAction      &)*pAct); break;
            case META_TEXTCOLOR_ACTION      : DoAction((MetaTextColorAction      &)*pAct); break;
            case META_TEXTFILLCOLOR_ACTION  : DoAction((MetaTextFillColorAction  &)*pAct); break;
            case META_FONT_ACTION           : DoAction((MetaFontAction           &)*pAct); break;
            case META_TEXTALIGN_ACTION      : DoAction((MetaTextAlignAction      &)*pAct); break;
            case META_MAPMODE_ACTION        : DoAction((MetaMapModeAction        &)*pAct); break;
            case META_CLIPREGION_ACTION     : DoAction((MetaClipRegionAction     &)*pAct); break;
            case META_MOVECLIPREGION_ACTION : DoAction((MetaMoveClipRegionAction &)*pAct); break;
            case META_ISECTRECTCLIPREGION_ACTION: DoAction((MetaISectRectClipRegionAction&)*pAct); break;
            case META_ISECTREGIONCLIPREGION_ACTION: DoAction((MetaISectRegionClipRegionAction&)*pAct); break;
            case META_RASTEROP_ACTION       : DoAction((MetaRasterOpAction       &)*pAct); break;
            case META_PUSH_ACTION           : DoAction((MetaPushAction           &)*pAct); break;
            case META_POP_ACTION            : DoAction((MetaPopAction            &)*pAct); break;
            case META_HATCH_ACTION          : DoAction((MetaHatchAction          &)*pAct); break;

            // #i125211# MetaCommentAction may change index, thus hand it over
            case META_COMMENT_ACTION        : DoAction((MetaCommentAction&)*pAct, rMtf, a);
                break;

            // missing actions added
            case META_TEXTRECT_ACTION       : DoAction((MetaTextRectAction&)*pAct); break;
            case META_BMPSCALEPART_ACTION   : DoAction((MetaBmpScalePartAction&)*pAct); break;
            case META_BMPEXSCALEPART_ACTION : DoAction((MetaBmpExScalePartAction&)*pAct); break;
            case META_MASK_ACTION           : DoAction((MetaMaskAction&)*pAct); break;
            case META_MASKSCALE_ACTION      : DoAction((MetaMaskScaleAction&)*pAct); break;
            case META_MASKSCALEPART_ACTION  : DoAction((MetaMaskScalePartAction&)*pAct); break;
            case META_GRADIENT_ACTION       : DoAction((MetaGradientAction&)*pAct); break;
            case META_WALLPAPER_ACTION      : DoAction((MetaWallpaperAction&)*pAct); break;
            case META_TRANSPARENT_ACTION    : DoAction((MetaTransparentAction&)*pAct); break;
            case META_EPS_ACTION            : DoAction((MetaEPSAction&)*pAct); break;
            case META_REFPOINT_ACTION       : DoAction((MetaRefPointAction&)*pAct); break;
            case META_TEXTLINECOLOR_ACTION  : DoAction((MetaTextLineColorAction&)*pAct); break;
            case META_TEXTLINE_ACTION       : DoAction((MetaTextLineAction&)*pAct); break;
            case META_FLOATTRANSPARENT_ACTION : DoAction((MetaFloatTransparentAction&)*pAct); break;
            case META_GRADIENTEX_ACTION     : DoAction((MetaGradientExAction&)*pAct); break;
            case META_LAYOUTMODE_ACTION     : DoAction((MetaLayoutModeAction&)*pAct); break;
            case META_TEXTLANGUAGE_ACTION   : DoAction((MetaTextLanguageAction&)*pAct); break;
            case META_OVERLINECOLOR_ACTION  : DoAction((MetaOverlineColorAction&)*pAct); break;
        }

        if(pProgrInfo && pActionsToReport)
        {
            (*pActionsToReport)++;

            if(*pActionsToReport >= 16) // Alle 16 Action updaten
            {
                if(!pProgrInfo->ReportActions(*pActionsToReport))
                    break;

                *pActionsToReport = 0;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_uInt32 ImpSdrGDIMetaFileImport::DoImport(
    const GDIMetaFile& rMtf,
    SdrObjList& rOL,
    sal_uInt32 nInsPos,
    SvdProgressInfo* pProgrInfo)
{
    const sal_uInt32 nActionCount(rMtf.GetActionCount());

    if(nActionCount)
    {
        // create mapping from MetaFile coordinates to unit coordinates
        maMetaToUnit.identity();
        maMetaToUnit.translate(-rMtf.GetPrefMapMode().GetOrigin().X(), -rMtf.GetPrefMapMode().GetOrigin().X());
        maMetaToUnit.scale(
            rMtf.GetPrefSize().Width() ? 1.0 / rMtf.GetPrefSize().Width() : 1.0,
            rMtf.GetPrefSize().Height() ? 1.0 / rMtf.GetPrefSize().Height() : 1.0);

        // create full mapping; first to unit coordinates, then apply object transformation
        maCurrent = maObjectTransform * maMetaToUnit;

        if(pProgrInfo)
        {
            pProgrInfo->SetActionCount(nActionCount);
        }

        sal_uInt32 nActionsToReport(0);

        // execute
        DoLoopActions(const_cast< GDIMetaFile& >(rMtf), pProgrInfo, &nActionsToReport);

        if(pProgrInfo)
        {
            pProgrInfo->ReportActions(nActionsToReport);
            nActionsToReport = 0;
        }

        // Beim berechnen der Fortschrittsanzeige wird GetActionCount()*3 benutzt.
        // Da in maTmpList allerdings weniger eintraege als GetActionCount()
        // existieren koennen, muessen hier die zuviel vermuteten Actionen wieder
        // hinzugefuegt werden.
        nActionsToReport = (rMtf.GetActionCount() - maTmpList.size()) * 2;

        // Alle noch nicht gemeldeten Rescales melden
        if(pProgrInfo)
        {
            pProgrInfo->ReportRescales(nActionsToReport);
            pProgrInfo->SetInsertCount(maTmpList.size());
        }

        nActionsToReport = 0;

        // alle in maTmpList zwischengespeicherten Objekte nun in rOL ab der Position nInsPos einfuegen
        if(nInsPos > rOL.GetObjCount())
        {
            nInsPos = rOL.GetObjCount();
        }

        for(sal_uInt32 i(0); i < maTmpList.size(); i++)
        {
            SdrObject* pObj = maTmpList[i];
            rOL.InsertObjectToSdrObjList(*pObj, nInsPos);
            nInsPos++;

            if(pProgrInfo)
            {
                nActionsToReport++;

                if(nActionsToReport >= 32) // Alle 32 Action updaten
                {
                    pProgrInfo->ReportInserts(nActionsToReport);
                    nActionsToReport = 0;
                }
            }
        }

        // ein letztesmal alle verbliebennen Inserts reporten
        if(pProgrInfo)
        {
            pProgrInfo->ReportInserts(nActionsToReport);
        }

    }
    return maTmpList.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

        maOldLineColor = maVD.GetLineColor();

        if(maVD.IsLineColor())
        {
            mpLineAttr->Put(XLineStyleItem(XLINE_SOLID));
            mpLineAttr->Put(XLineColorItem(String(), maVD.GetLineColor()));
        }
        else
        {
            mpLineAttr->Put(XLineStyleItem(XLINE_NONE));
        }

        switch(maLineJoin)
        {
            default : // basegfx::B2DLINEJOIN_NONE
                mpLineAttr->Put(XLineJointItem(com::sun::star::drawing::LineJoint_NONE));
                break;
            case basegfx::B2DLINEJOIN_MIDDLE:
                mpLineAttr->Put(XLineJointItem(com::sun::star::drawing::LineJoint_MIDDLE));
                break;
            case basegfx::B2DLINEJOIN_BEVEL:
                mpLineAttr->Put(XLineJointItem(com::sun::star::drawing::LineJoint_BEVEL));
                break;
            case basegfx::B2DLINEJOIN_MITER:
                mpLineAttr->Put(XLineJointItem(com::sun::star::drawing::LineJoint_MITER));
                break;
            case basegfx::B2DLINEJOIN_ROUND:
                mpLineAttr->Put(XLineJointItem(com::sun::star::drawing::LineJoint_ROUND));
                break;
        }

        // Add LineCap support
        mpLineAttr->Put(XLineCapItem(maLineCap));

        if(((maDash.GetDots() && maDash.GetDotLen()) || (maDash.GetDashes() && maDash.GetDashLen())) && maDash.GetDistance())
        {
            mpLineAttr->Put(XLineDashItem(String(), maDash));
        }
        else
        {
            mpLineAttr->Put(XLineDashItem(String(), XDash(XDASH_RECT)));
        }
    }
    else
    {
        mbNoLine = true;
    }

    if(bFill)
    {
        if(maVD.IsFillColor())
        {
            mpFillAttr->Put(XFillStyleItem(XFILL_SOLID));
            mpFillAttr->Put(XFillColorItem(String(), maVD.GetFillColor()));
        }
        else
        {
            mpFillAttr->Put(XFillStyleItem(XFILL_NONE));
        }
    }
    else
    {
        mbNoFill = true;
    }

    if(bText && mbFntDirty)
    {
        Font aFnt(maVD.GetFont());
        const double fUnitInReal((maCurrent * basegfx::B2DVector(0.0, 1.0)).getLength());
        const sal_uInt32 nHeight(FRound(aFnt.GetSize().Height() * fUnitInReal));

        mpTextAttr->Put( SvxFontItem( aFnt.GetFamily(), aFnt.GetName(), aFnt.GetStyleName(), aFnt.GetPitch(), aFnt.GetCharSet(), EE_CHAR_FONTINFO ) );
        mpTextAttr->Put( SvxFontItem( aFnt.GetFamily(), aFnt.GetName(), aFnt.GetStyleName(), aFnt.GetPitch(), aFnt.GetCharSet(), EE_CHAR_FONTINFO_CJK ) );
        mpTextAttr->Put( SvxFontItem( aFnt.GetFamily(), aFnt.GetName(), aFnt.GetStyleName(), aFnt.GetPitch(), aFnt.GetCharSet(), EE_CHAR_FONTINFO_CTL ) );
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
        mpTextAttr->Put(SvxColorItem(maVD.GetTextColor(), EE_CHAR_COLOR));
        //... svxfont textitem svditext
        mbFntDirty = false;
    }

    if(pObj)
    {
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
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::InsertObj(SdrObject* pObj)
{
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
                const basegfx::B2DRange aTextRange(pSdrTextObj->getObjectRange(0)); // TTTT: Check if this works
                const basegfx::B2DRange aClipRange(maClip.getB2DRange());

                // no overlap -> completely outside
                if(!aClipRange.overlaps(aTextRange))
                {
                    deleteSdrObjectSafeAndClearPointer(pObj);
                    break;
                }

                // when the clip is a rectangle fast check for inside is possible
                if(basegfx::tools::isRectangle(maClip) && aClipRange.isInside(aTextRange))
                {
                    // completely inside ClipRect
                    break;
                }

                // here text needs to be clipped; to do so, convert to SdrObjects with polygons
                // and add these recursively. Delete original object, do not add in this run
                SdrObject* pConverted = pSdrTextObj->ConvertToPolyObj(true, true);
                deleteSdrObjectSafeAndClearPointer(pObj);

                if(pConverted)
                {
                    // recursively add created conversion; per definition this shall not
                    // contain further SdrTextObjs. Visit only non-group objects
                    SdrObjListIter aIter(*pConverted, IM_DEEPNOGROUPS);

                    // work with clones; the created conversion may contain group objects
                    // and when working with the original objects the loop itself could
                    // break and the cleanup later would be pretty complicated (only delete group
                    // objects, are these empty, ...?)
                    while(aIter.IsMore())
                    {
                        SdrObject* pCandidate = aIter.Next();
                        OSL_ENSURE(pCandidate && 0 == dynamic_cast< SdrObjGroup* >(pCandidate), "SdrObjListIter with IM_DEEPNOGROUPS error (!)");
                        SdrObject* pNewClone = pCandidate->CloneSdrObject();

                        if(pNewClone)
                        {
                            InsertObj(pNewClone);
                        }
                        else
                        {
                            OSL_ENSURE(false, "SdrObject::Clone() failed (!)");
                        }
                    }

                    // cleanup temporary conversion objects
                    deleteSdrObjectSafeAndClearPointer(pConverted);
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

            deleteSdrObjectSafeAndClearPointer(pObj);

            if(!aOldRange.isEmpty())
            {
                // clip against ClipRegion
                const basegfx::B2DPolyPolygon aNewPoly(
                    basegfx::tools::clipPolyPolygonOnPolyPolygon(
                        aPoly,
                        maClip,
                        true,
                        aPoly.isClosed() ? false : true));
                const basegfx::B2DRange aNewRange(aNewPoly.getB2DRange());

                if(!aNewRange.isEmpty())
                {
                    pObj = new SdrPathObj(
                        mrModel,
                        aNewPoly);

                    pObj->SetLayer(aOldLayer);
                    pObj->SetMergedItemSet(aOldItemSet);

                    if(!!aBitmapEx)
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
                            basegfx::fround(ceil(std::min((double)aOrigSizePixel.Width(), aPixel.getWidth()))),
                            basegfx::fround(ceil(std::min((double)aOrigSizePixel.Height(), aPixel.getHeight()))));
                        const BitmapEx aClippedBitmap(
                            aBitmapEx,
                            aClipTopLeft,
                            aClipSize);

                        pObj->SetMergedItem(XFillStyleItem(XFILL_BITMAP));
                        pObj->SetMergedItem(XFillBitmapItem(String(), Graphic(aClippedBitmap)));
                        pObj->SetMergedItem(XFillBmpTileItem(false));
                        pObj->SetMergedItem(XFillBmpStretchItem(true));
                    }
                }
            }
        }
    }

    if(pObj)
    {
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
            deleteSdrObjectSafeAndClearPointer(pObj);
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
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaPixelAction& /*rAct*/)
{
    OSL_ENSURE(false, "Tried to construct SdrObject from MetaPixelAction: not supported (!)");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaPointAction& /*rAct*/)
{
    OSL_ENSURE(false, "Tried to construct SdrObject from MetaPointAction: not supported (!)");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaLineAction& rAct)
{
    const basegfx::B2DPoint aStart(rAct.GetStartPoint().X(), rAct.GetStartPoint().Y());
    const basegfx::B2DPoint aEnd(rAct.GetEndPoint().X(), rAct.GetEndPoint().Y());

    if(!aStart.equal(aEnd))
    {
        basegfx::B2DPolygon aLine;

        aLine.append(aStart);
        aLine.append(aEnd);
        aLine.transform(maCurrent);

        const LineInfo& rLineInfo = rAct.GetLineInfo();
        const sal_Int32 nNewLineWidth(rLineInfo.GetWidth());
        bool bCreateLineObject(true);

        if(mbLastObjWasLine && (nNewLineWidth == mnLineWidth) && CheckLastLineMerge(aLine))
        {
            bCreateLineObject = false;
        }

        if(bCreateLineObject)
        {
            SdrPathObj* pPath = new SdrPathObj(
                mrModel,
                basegfx::B2DPolyPolygon(aLine));

            mnLineWidth = nNewLineWidth;
            maLineJoin = rLineInfo.GetLineJoin();
            maLineCap = rLineInfo.GetLineCap();
            maDash = XDash(XDASH_RECT,
                rLineInfo.GetDotCount(), rLineInfo.GetDotLen(),
                rLineInfo.GetDashCount(), rLineInfo.GetDashLen(),
                rLineInfo.GetDistance());

            SetAttributes(pPath);

            mnLineWidth = 0;
            maLineJoin = basegfx::B2DLINEJOIN_NONE;
            maDash = XDash();

            InsertObj(pPath);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaRectAction& rAct)
{
    const Rectangle& rRect(rAct.GetRect());
    SdrRectObj* pRect = new SdrRectObj(
        mrModel,
        maCurrent * basegfx::tools::createScaleTranslateB2DHomMatrix(
            rRect.getWidth(), rRect.getHeight(),
            rRect.Left(), rRect.Top()));

    SetAttributes(pRect);
    InsertObj(pRect);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaRoundRectAction& rAct)
{
    const Rectangle& rRect(rAct.GetRect());
    SdrRectObj* pRect = new SdrRectObj(
        mrModel,
        maCurrent * basegfx::tools::createScaleTranslateB2DHomMatrix(
            rRect.getWidth(), rRect.getHeight(),
            rRect.Left(), rRect.Top()));

    SetAttributes(pRect);

    const sal_uInt32 nRad((rAct.GetHorzRound() + rAct.GetVertRound()) / 2);

    if(nRad)
    {
        pRect->SetMergedItem(SdrMetricItem(SDRATTR_ECKENRADIUS, nRad));
    }

    InsertObj(pRect);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaEllipseAction& rAct)
{
    const Rectangle& rRect(rAct.GetRect());
    SdrCircObj* pCirc = new SdrCircObj(
        mrModel,
        CircleType_Circle,
        maCurrent * basegfx::tools::createScaleTranslateB2DHomMatrix(
            rRect.getWidth(), rRect.getHeight(),
            rRect.Left(), rRect.Top()));

    SetAttributes(pCirc);
    InsertObj(pCirc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
    void impPrepareAngles(const Point& rCenter, const Point& rStart, const Point& rEnd, double& o_Start, double& o_End)
    {
        o_Start = atan2((double)(rStart.Y() - rCenter.Y()), (double)(rStart.X() - rCenter.X()));

        if(o_Start < 0.0)
        {
            o_Start += F_2PI;
        }

        o_End = atan2((double)(rEnd.Y() - rCenter.Y()), (double)(rEnd.X() - rCenter.X()));

        if(o_End < 0.0)
        {
            o_End += F_2PI;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaArcAction& rAct)
{
    double fStart(0.0);
    double fEnd(F_2PI);
    const Rectangle& rRect(rAct.GetRect());

    impPrepareAngles(rRect.Center(), rAct.GetStartPoint(), rAct.GetEndPoint(), fStart, fEnd);

    SdrCircObj* pCirc = new SdrCircObj(
        mrModel,
        CircleType_Circle,
        maCurrent * basegfx::tools::createScaleTranslateB2DHomMatrix(
            rRect.getWidth(), rRect.getHeight(),
            rRect.Left(), rRect.Top()),
        fStart,
        fEnd);

    SetAttributes(pCirc);
    InsertObj(pCirc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaPieAction& rAct)
{
    double fStart(0.0);
    double fEnd(F_2PI);
    const Rectangle& rRect(rAct.GetRect());

    impPrepareAngles(rRect.Center(), rAct.GetStartPoint(), rAct.GetEndPoint(), fStart, fEnd);

    SdrCircObj* pCirc = new SdrCircObj(
        mrModel,
        CircleType_Sector,
        maCurrent * basegfx::tools::createScaleTranslateB2DHomMatrix(
            rRect.getWidth(), rRect.getHeight(),
            rRect.Left(), rRect.Top()),
        fStart,
        fEnd);

    SetAttributes(pCirc);
    InsertObj(pCirc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaChordAction& rAct)
{
    double fStart(0.0);
    double fEnd(F_2PI);
    const Rectangle& rRect(rAct.GetRect());

    impPrepareAngles(rRect.Center(), rAct.GetStartPoint(), rAct.GetEndPoint(), fStart, fEnd);

    SdrCircObj* pCirc = new SdrCircObj(
        mrModel,
        CircleType_Segment,
        maCurrent * basegfx::tools::createScaleTranslateB2DHomMatrix(
            rRect.getWidth(), rRect.getHeight(),
            rRect.Left(), rRect.Top()),
        fStart,
        fEnd);

    SetAttributes(pCirc);
    InsertObj(pCirc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool ImpSdrGDIMetaFileImport::CheckLastLineMerge(const basegfx::B2DPolygon& rSrcPoly)
{
    // #i102706# Do not merge closed polygons
    if(rSrcPoly.isClosed())
    {
        return false;
    }

    if(mbLastObjWasLine && (maOldLineColor == maVD.GetLineColor()) && rSrcPoly.count())
    {
        SdrObject* pTmpObj = maTmpList.size() ? maTmpList[maTmpList.size() - 1] : 0;
        SdrPathObj* pLastPoly = dynamic_cast< SdrPathObj* >(pTmpObj);

        if(pLastPoly)
        {
            if(1 == pLastPoly->getB2DPolyPolygonInObjectCoordinates().count())
            {
                bool bOk(false);
                basegfx::B2DPolygon aDstPoly(pLastPoly->getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(0));

                // #i102706# Do not merge closed polygons
                if(aDstPoly.isClosed())
                {
                    return false;
                }

                if(aDstPoly.count())
                {
                    const sal_uInt32 nMaxDstPnt(aDstPoly.count() - 1L);
                    const sal_uInt32 nMaxSrcPnt(rSrcPoly.count() - 1L);

                    if(aDstPoly.getB2DPoint(nMaxDstPnt) == rSrcPoly.getB2DPoint(0L))
                    {
                        aDstPoly.append(rSrcPoly, 1L, rSrcPoly.count() - 1L);
                        bOk = true;
                    }
                    else if(aDstPoly.getB2DPoint(0L) == rSrcPoly.getB2DPoint(nMaxSrcPnt))
                    {
                        basegfx::B2DPolygon aNew(rSrcPoly);
                        aNew.append(aDstPoly, 1L, aDstPoly.count() - 1L);
                        aDstPoly = aNew;
                        bOk = true;
                    }
                    else if(aDstPoly.getB2DPoint(0L) == rSrcPoly.getB2DPoint(0L))
                    {
                        aDstPoly.flip();
                        aDstPoly.append(rSrcPoly, 1L, rSrcPoly.count() - 1L);
                        bOk = true;
                    }
                    else if(aDstPoly.getB2DPoint(nMaxDstPnt) == rSrcPoly.getB2DPoint(nMaxSrcPnt))
                    {
                        basegfx::B2DPolygon aNew(rSrcPoly);
                        aNew.flip();
                        aDstPoly.append(aNew, 1L, aNew.count() - 1L);
                        bOk = true;
                    }
                }

                if(bOk)
                {
                    pLastPoly->setB2DPolyPolygonInObjectCoordinates(basegfx::B2DPolyPolygon(aDstPoly));
                }

                return bOk;
            }
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool ImpSdrGDIMetaFileImport::CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon & rPolyPolygon)
{
    if(mbLastObjWasPolyWithoutLine)
    {
        SdrObject* pTmpObj = maTmpList.size() ? maTmpList[maTmpList.size() - 1] : 0;
        SdrPathObj* pLastPoly = dynamic_cast< SdrPathObj* >(pTmpObj);

        if(pLastPoly)
        {
            if(pLastPoly->getB2DPolyPolygonInObjectCoordinates() == rPolyPolygon)
            {
                SetAttributes(0);

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

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::checkClip()
{
    if(maVD.IsClipRegion())
    {
        maClip = maVD.GetClipRegion().GetAsB2DPolyPolygon();

        if(isClip())
        {
            maClip.transform(maCurrent);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool ImpSdrGDIMetaFileImport::isClip() const
{
    return !maClip.getB2DRange().isEmpty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction( MetaPolyLineAction& rAct )
{
    basegfx::B2DPolygon aSource(rAct.GetPolygon().getB2DPolygon());

    if(aSource.count())
    {
        aSource.transform(maCurrent);

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

        if(bCreateLineObject)
        {
            SdrPathObj* pPath = new SdrPathObj(
                mrModel,
                basegfx::B2DPolyPolygon(aSource));

            mnLineWidth = nNewLineWidth;
            maLineJoin = rLineInfo.GetLineJoin();
            maLineCap = rLineInfo.GetLineCap();
            maDash = XDash(XDASH_RECT,
                rLineInfo.GetDotCount(), rLineInfo.GetDotLen(),
                rLineInfo.GetDashCount(), rLineInfo.GetDashLen(),
                rLineInfo.GetDistance());

            SetAttributes(pPath);

            mnLineWidth = 0;
            maLineJoin = basegfx::B2DLINEJOIN_NONE;
            maDash = XDash();

            InsertObj(pPath);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaPolygonAction& rAct)
{
    basegfx::B2DPolygon aSource(rAct.GetPolygon().getB2DPolygon());

    if(aSource.count())
    {
        aSource.transform(maCurrent);

        if(!mbLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(basegfx::B2DPolyPolygon(aSource)))
        {
            // #i73407# make sure polygon is closed, it's a filled primitive
            aSource.setClosed(true);

            SdrPathObj* pPath = new SdrPathObj(
                mrModel,
                basegfx::B2DPolyPolygon(aSource));

            SetAttributes(pPath);
            InsertObj(pPath);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaPolyPolygonAction& rAct)
{
    basegfx::B2DPolyPolygon aSource(rAct.GetPolyPolygon().getB2DPolyPolygon());

    if(aSource.count())
    {
        aSource.transform(maCurrent);

        if(!mbLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(aSource))
        {
            // #i73407# make sure polygon is closed, it's a filled primitive
            aSource.setClosed(true);

            SdrPathObj* pPath = new SdrPathObj(
                mrModel,
                aSource);

            SetAttributes(pPath);
            InsertObj(pPath);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::ImportText( const Point& rPos, const XubString& rStr, const MetaAction& rAct )
{
    // calc text box size
    const FontMetric aFontMetric(maVD.GetFontMetric());
    const Font aFnt(maVD.GetFont());
    const FontAlign eAlg(aFnt.GetAlign());
    basegfx::B2DVector aTextScale(maVD.GetTextWidth(rStr), maVD.GetTextHeight());
    basegfx::B2DPoint aTextPos(rPos.X(), rPos.Y());
    basegfx::B2DHomMatrix aTextMatrix;

    if(ALIGN_BASELINE == eAlg)
    {
        aTextPos.setY(aTextPos.getY() - aFontMetric.GetAscent());
    }
    else if(ALIGN_BOTTOM == eAlg)
    {
        aTextPos.setY(aTextPos.getY() - aTextScale.getY());
    }

    aTextScale = maCurrent * aTextScale;
    aTextPos = maCurrent * aTextPos;

    if(aFnt.GetOrientation())
    {
        aTextMatrix.rotate(F_PI180 * (aFnt.GetOrientation() * 0.10));
    }

    aTextMatrix.scale(aTextScale);
    aTextMatrix.translate(aTextPos);

    SdrRectObj* pText = new SdrRectObj(
        mrModel,
        aTextMatrix,
        OBJ_TEXT,
        true);

    pText->SetMergedItem(SdrMetricItem(SDRATTR_TEXT_UPPERDIST, 0));
    pText->SetMergedItem(SdrMetricItem(SDRATTR_TEXT_LOWERDIST, 0));
    pText->SetMergedItem(SdrMetricItem(SDRATTR_TEXT_RIGHTDIST, 0));
    pText->SetMergedItem(SdrMetricItem(SDRATTR_TEXT_LEFTDIST, 0));

    if ( aFnt.GetWidth() || ( rAct.GetType() == META_STRETCHTEXT_ACTION ) )
    {
        pText->ClearMergedItem( SDRATTR_TEXT_AUTOGROWWIDTH );
        pText->SetMergedItem( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, false) );
        // don't let the margins eat the space needed for the text
        pText->SetMergedItem( SdrTextFitToSizeTypeItem( SDRTEXTFIT_ALLLINES ) );
    }
    else
    {
        pText->SetMergedItem( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, true ) );
    }

    pText->SetLayer(mnLayer);
    pText->SetText(rStr);
    SetAttributes(pText, true);

    if (!aFnt.IsTransparent())
    {
        SfxItemSet aAttr(*mpFillAttr->GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST, 0, 0);

        aAttr.Put(XFillStyleItem(XFILL_SOLID));
        aAttr.Put(XFillColorItem(String(), aFnt.GetFillColor()));
        pText->SetMergedItemSet(aAttr);
    }

    InsertObj(pText);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaTextAction& rAct)
{
    XubString aStr(rAct.GetText());
    aStr.Erase(0,rAct.GetIndex());
    aStr.Erase(rAct.GetLen());

    ImportText( rAct.GetPoint(), aStr, rAct );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaTextArrayAction& rAct)
{
    XubString aStr(rAct.GetText());
    aStr.Erase(0,rAct.GetIndex());
    aStr.Erase(rAct.GetLen());

    ImportText( rAct.GetPoint(), aStr, rAct );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaStretchTextAction& rAct)
{
    XubString aStr(rAct.GetText());
    aStr.Erase(0,rAct.GetIndex());
    aStr.Erase(rAct.GetLen());

    ImportText( rAct.GetPoint(), aStr, rAct );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpAction& rAct)
{
    const Rectangle aRect(rAct.GetPoint(), rAct.GetBitmap().GetSizePixel());
    const basegfx::B2DHomMatrix aGrafMatrix(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aRect.getWidth() + 1, aRect.getHeight() + 1,
            aRect.Left(), aRect.Top()));

    SdrGrafObj* pGraf = new SdrGrafObj(
        mrModel,
        Graphic(rAct.GetBitmap()),
        maCurrent * aGrafMatrix);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(XLINE_NONE));
    pGraf->SetMergedItem(XFillStyleItem(XFILL_NONE));

    InsertObj(pGraf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpScaleAction& rAct)
{
    const Rectangle aRect(rAct.GetPoint(), rAct.GetSize());
    const basegfx::B2DHomMatrix aGrafMatrix(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aRect.getWidth() + 1, aRect.getHeight() + 1,
            aRect.Left(), aRect.Top()));

    SdrGrafObj* pGraf = new SdrGrafObj(
        mrModel,
        Graphic(rAct.GetBitmap()),
        maCurrent * aGrafMatrix);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(XLINE_NONE));
    pGraf->SetMergedItem(XFillStyleItem(XFILL_NONE));

    InsertObj(pGraf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpExAction& rAct)
{
    const Rectangle aRect(rAct.GetPoint(), rAct.GetBitmapEx().GetSizePixel());
    const basegfx::B2DHomMatrix aGrafMatrix(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aRect.getWidth() + 1, aRect.getHeight() + 1,
            aRect.Left(), aRect.Top()));

    SdrGrafObj* pGraf = new SdrGrafObj(
        mrModel,
        rAct.GetBitmapEx(),
        maCurrent * aGrafMatrix);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(XLINE_NONE));
    pGraf->SetMergedItem(XFillStyleItem(XFILL_NONE));

    InsertObj(pGraf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpExScaleAction& rAct)
{
    const Rectangle aRect(rAct.GetPoint(), rAct.GetSize());
    const basegfx::B2DHomMatrix aGrafMatrix(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aRect.getWidth() + 1, aRect.getHeight() + 1,
            aRect.Left(), aRect.Top()));

    SdrGrafObj* pGraf = new SdrGrafObj(
        mrModel,
        rAct.GetBitmapEx(),
        maCurrent * aGrafMatrix);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(XLINE_NONE));
    pGraf->SetMergedItem(XFillStyleItem(XFILL_NONE));

    InsertObj(pGraf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction( MetaHatchAction& rAct )
{
    basegfx::B2DPolyPolygon aSource(rAct.GetPolyPolygon().getB2DPolyPolygon());

    if(aSource.count())
    {
        aSource.transform(maCurrent);

        if(!mbLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(aSource))
        {
            const Hatch& rHatch = rAct.GetHatch();
            SdrPathObj* pPath = new SdrPathObj(
                mrModel,
                aSource);
            // #i125211# Use the ranges from the SdrObject to create a new empty SfxItemSet
            SfxItemSet aHatchAttr(pPath->GetObjectItemPool(), pPath->GetMergedItemSet().GetRanges());
            XHatchStyle eStyle;

            switch(rHatch.GetStyle())
            {
                case(HATCH_TRIPLE) :
                {
                    eStyle = XHATCH_TRIPLE;
                    break;
                }

                case(HATCH_DOUBLE) :
                {
                    eStyle = XHATCH_DOUBLE;
                    break;
                }

                default:
                {
                    eStyle = XHATCH_SINGLE;
                    break;
                }
            }

            SetAttributes(pPath);
            aHatchAttr.Put(XFillStyleItem(XFILL_HATCH));
            aHatchAttr.Put(XFillHatchItem(&pPath->GetObjectItemPool(), XHatch(rHatch.GetColor(), eStyle, rHatch.GetDistance(), rHatch.GetAngle())));
            pPath->SetMergedItemSet(aHatchAttr);

            InsertObj(pPath);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaMapModeAction& rAct)
{
    rAct.Execute(&maVD);

    // create new transformation since this action may set a relative mapping action.
    // thus, first apply new mapping, then from metafuile to unit, and then object transformation
    maCurrent = maObjectTransform * maMetaToUnit * maVD.GetViewTransformation();

    mbLastObjWasPolyWithoutLine = false;
    mbLastObjWasLine = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction( MetaCommentAction& rAct, GDIMetaFile& rMtf, sal_uLong& a) // GDIMetaFile* pMtf )
{
    ByteString aSkipComment;

    if( a < rMtf.GetActionCount() && rAct.GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL )
    {
        // #i125211# Check if next action is a MetaGradientExAction
        MetaGradientExAction* pAct = dynamic_cast< MetaGradientExAction* >(rMtf.GetAction(a + 1));

        if( pAct && pAct->GetType() == META_GRADIENTEX_ACTION )
        {
            basegfx::B2DPolyPolygon aSource(pAct->GetPolyPolygon().getB2DPolyPolygon());

            if(aSource.count())
            {
                aSource.transform(maCurrent); // TTTT: needed? was missing before

                if(!mbLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(aSource))
                {
                    const Gradient& rGrad = pAct->GetGradient();
                    SdrPathObj* pPath = new SdrPathObj(
                        mrModel,
                        aSource);
                    // #i125211# Use the ranges from the SdrObject to create a new empty SfxItemSet
                    SfxItemSet aGradAttr(pPath->GetObjectItemPool(), pPath->GetMergedItemSet().GetRanges());
                    XGradient aXGradient;

                    aXGradient.SetGradientStyle((XGradientStyle)rGrad.GetStyle());
                    aXGradient.SetStartColor(rGrad.GetStartColor());
                    aXGradient.SetEndColor(rGrad.GetEndColor());
                    aXGradient.SetAngle((sal_uInt16)rGrad.GetAngle());
                    aXGradient.SetBorder(rGrad.GetBorder());
                    aXGradient.SetXOffset(rGrad.GetOfsX());
                    aXGradient.SetYOffset(rGrad.GetOfsY());
                    aXGradient.SetStartIntens(rGrad.GetStartIntensity());
                    aXGradient.SetEndIntens(rGrad.GetEndIntensity());
                    aXGradient.SetSteps(rGrad.GetSteps());

                    // no need to use SetAttributes(..) here since line and fill style
                    // need to be set individually
                    // SetAttributes(pPath);

                    // switch line off; when there was one there will be a
                    // META_POLYLINE_ACTION following creating another object
                    aGradAttr.Put(XLineStyleItem(XLINE_NONE));

                    // add detected gradient fillstyle
                    aGradAttr.Put(XFillStyleItem(XFILL_GRADIENT));
                    aGradAttr.Put(XFillGradientItem(&mrModel.GetItemPool(), aXGradient));

                    pPath->SetMergedItemSet(aGradAttr);

                    InsertObj(pPath);
                }
            }

            aSkipComment = "XGRAD_SEQ_END";
        }
    }

    if(aSkipComment.Len())
    {
        // #i125211# forward until closing MetaCommentAction
        MetaAction* pSkipAct = rMtf.GetAction(++a);

        while( pSkipAct
            && ((pSkipAct->GetType() != META_COMMENT_ACTION )
                || (((MetaCommentAction*)pSkipAct)->GetComment().CompareIgnoreCaseToAscii(aSkipComment.GetBuffer()) != COMPARE_EQUAL)))
        {
            pSkipAct = rMtf.GetAction(++a);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaTextRectAction& rAct)
{
    GDIMetaFile aTemp;

    // dismantle MetaTextRectActions to own metafile and execute
    maVD.AddTextRectActions(rAct.GetRect(), rAct.GetText(), rAct.GetStyle(), aTemp);
    DoLoopActions(aTemp, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpScalePartAction& rAct)
{
    const Rectangle aRect(rAct.GetDestPoint(), rAct.GetDestSize());
    const basegfx::B2DHomMatrix aGrafMatrix(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aRect.getWidth() + 1, aRect.getHeight() + 1,
            aRect.Left(), aRect.Top()));
    Bitmap aBitmap(rAct.GetBitmap());

    aBitmap.Crop(Rectangle(rAct.GetSrcPoint(), rAct.GetSrcSize()));
    SdrGrafObj* pGraf = new SdrGrafObj(
        mrModel,
        aBitmap,
        maCurrent * aGrafMatrix);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(XLINE_NONE));
    pGraf->SetMergedItem(XFillStyleItem(XFILL_NONE));
    InsertObj(pGraf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaBmpExScalePartAction& rAct)
{
    const Rectangle aRect(rAct.GetDestPoint(), rAct.GetDestSize());
    const basegfx::B2DHomMatrix aGrafMatrix(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aRect.getWidth() + 1, aRect.getHeight() + 1,
            aRect.Left(), aRect.Top()));
    BitmapEx aBitmapEx(rAct.GetBitmapEx());

    aBitmapEx.Crop(Rectangle(rAct.GetSrcPoint(), rAct.GetSrcSize()));
    SdrGrafObj* pGraf = new SdrGrafObj(
        mrModel,
        aBitmapEx,
        maCurrent * aGrafMatrix);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(XLINE_NONE));
    pGraf->SetMergedItem(XFillStyleItem(XFILL_NONE));
    InsertObj(pGraf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaMaskAction& rAct)
{
    const Rectangle aRect(rAct.GetPoint(), rAct.GetBitmap().GetSizePixel());
    const basegfx::B2DHomMatrix aGrafMatrix(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aRect.getWidth() + 1, aRect.getHeight() + 1,
            aRect.Left(), aRect.Top()));
    const BitmapEx aBitmapEx(rAct.GetBitmap(), rAct.GetColor());

    SdrGrafObj* pGraf = new SdrGrafObj(
        mrModel,
        aBitmapEx,
        maCurrent * aGrafMatrix);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(XLINE_NONE));
    pGraf->SetMergedItem(XFillStyleItem(XFILL_NONE));
    InsertObj(pGraf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaMaskScaleAction& rAct)
{
    const Rectangle aRect(rAct.GetPoint(), rAct.GetSize());
    const basegfx::B2DHomMatrix aGrafMatrix(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aRect.getWidth() + 1, aRect.getHeight() + 1,
            aRect.Left(), aRect.Top()));
    const BitmapEx aBitmapEx(rAct.GetBitmap(), rAct.GetColor());

    SdrGrafObj* pGraf = new SdrGrafObj(
        mrModel,
        aBitmapEx,
        maCurrent * aGrafMatrix);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(XLINE_NONE));
    pGraf->SetMergedItem(XFillStyleItem(XFILL_NONE));
    InsertObj(pGraf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaMaskScalePartAction& rAct)
{
    const Rectangle aRect(rAct.GetDestPoint(), rAct.GetDestSize());
    const basegfx::B2DHomMatrix aGrafMatrix(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aRect.getWidth() + 1, aRect.getHeight() + 1,
            aRect.Left(), aRect.Top()));
    BitmapEx aBitmapEx(rAct.GetBitmap(), rAct.GetColor());

    aBitmapEx.Crop(Rectangle(rAct.GetSrcPoint(), rAct.GetSrcSize()));
    SdrGrafObj* pGraf = new SdrGrafObj(
        mrModel,
        aBitmapEx,
        maCurrent * aGrafMatrix);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(XLINE_NONE));
    pGraf->SetMergedItem(XFillStyleItem(XFILL_NONE));
    InsertObj(pGraf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XGradientStyle getXGradientStyleFromGradientStyle(const GradientStyle& rGradientStyle)
{
    XGradientStyle aXGradientStyle(XGRAD_LINEAR);

    switch(rGradientStyle)
    {
        case GRADIENT_LINEAR: aXGradientStyle = XGRAD_LINEAR; break;
        case GRADIENT_AXIAL: aXGradientStyle = XGRAD_AXIAL; break;
        case GRADIENT_RADIAL: aXGradientStyle = XGRAD_RADIAL; break;
        case GRADIENT_ELLIPTICAL: aXGradientStyle = XGRAD_ELLIPTICAL; break;
        case GRADIENT_SQUARE: aXGradientStyle = XGRAD_SQUARE; break;
        case GRADIENT_RECT: aXGradientStyle = XGRAD_RECT; break;

        // Needed due to GRADIENT_FORCE_EQUAL_SIZE; this again is needed
        // to force the enum defines in VCL to a defined size for the compilers,
        // so despite it is never used it cannot be removed (would break the
        // API implementation probably).
        default: break;
    }

    return aXGradientStyle;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaGradientAction& rAct)
{
    basegfx::B2DRange aRange(
        rAct.GetRect().Left(),
        rAct.GetRect().Top(),
        rAct.GetRect().Right() + 1,
        rAct.GetRect().Bottom() + 1);

    if(!aRange.isEmpty())
    {
        basegfx::B2DHomMatrix aObjectTransform(
            basegfx::tools::createScaleTranslateB2DHomMatrix(
                aRange.getRange(),
                aRange.getMinimum()));
        SdrRectObj* pRect = new SdrRectObj(
            mrModel,
            maCurrent * aObjectTransform);

        // #i125211# Use the ranges from the SdrObject to create a new empty SfxItemSet
        SfxItemSet aGradientAttr(mrModel.GetItemPool(), pRect->GetMergedItemSet().GetRanges());
        const Gradient& rGradient = rAct.GetGradient();
        const XGradientStyle aXGradientStyle(getXGradientStyleFromGradientStyle(rGradient.GetStyle()));
        const XFillGradientItem aXFillGradientItem(
            &mrModel.GetItemPool(),
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
        aGradientAttr.Put(XFillStyleItem(XFILL_GRADIENT)); // #i125211#
        aGradientAttr.Put(aXFillGradientItem);
        pRect->SetMergedItemSet(aGradientAttr);

        InsertObj(pRect);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaWallpaperAction& /*rAct*/)
{
    OSL_ENSURE(false, "Tried to construct SdrObject from MetaWallpaperAction: not supported (!)");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaTransparentAction& rAct)
{
    basegfx::B2DPolyPolygon aSource(rAct.GetPolyPolygon().getB2DPolyPolygon());

    if(aSource.count())
    {
        aSource.transform(maCurrent);
        aSource.setClosed(true);

        SdrPathObj* pPath = new SdrPathObj(
            mrModel,
            aSource);

        SetAttributes(pPath);
        pPath->SetMergedItem(XFillTransparenceItem(rAct.GetTransparence()));
        InsertObj(pPath);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaEPSAction& /*rAct*/)
{
    OSL_ENSURE(false, "Tried to construct SdrObject from MetaEPSAction: not supported (!)");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaTextLineAction& /*rAct*/)
{
    OSL_ENSURE(false, "Tried to construct SdrObject from MetaTextLineAction: not supported (!)");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ImpSdrGDIMetaFileImport::DoAction(MetaGradientExAction& rAct)
{
    basegfx::B2DPolyPolygon aSource(rAct.GetPolyPolygon().getB2DPolyPolygon());

    if(aSource.count())
    {
        aSource.transform(maCurrent);

        if(!mbLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(aSource))
        {
            // #i125211# Use the ranges from the SdrObject to create a new empty SfxItemSet
            SdrPathObj* pPath = new SdrPathObj(
                mrModel,
                aSource);
            SfxItemSet aGradientAttr(mrModel.GetItemPool(), pPath->GetMergedItemSet().GetRanges());
            const Gradient& rGradient = rAct.GetGradient();
            const XGradientStyle aXGradientStyle(getXGradientStyleFromGradientStyle(rGradient.GetStyle()));
            const XFillGradientItem aXFillGradientItem(
                &mrModel.GetItemPool(),
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
            aGradientAttr.Put(XFillStyleItem(XFILL_GRADIENT)); // #i125211#
            aGradientAttr.Put(aXFillGradientItem);
            pPath->SetMergedItemSet(aGradientAttr);

            InsertObj(pPath);
        }
    }
}

void ImpSdrGDIMetaFileImport::DoAction(MetaFloatTransparentAction& rAct)
{
    const GDIMetaFile& rMtf = rAct.GetGDIMetaFile();

    if(rMtf.GetActionCount())
    {
        const Rectangle aRect(rAct.GetPoint(), rAct.GetSize());
        basegfx::B2DRange aRange(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());

        // go to object target coordinates to get a good relative size
        aRange.transform(maCurrent);

        // convert metafile sub-content to BitmapEx
        BitmapEx aBitmapEx(
            convertMetafileToBitmapEx(
                rMtf,
                aRange,
                125000));

        // handle colors
        const Gradient& rGradient = rAct.GetGradient();
        basegfx::BColor aStart(rGradient.GetStartColor().getBColor());
        basegfx::BColor aEnd(rGradient.GetEndColor().getBColor());

        if(100 != rGradient.GetStartIntensity())
        {
            aStart *= (double)rGradient.GetStartIntensity() / 100.0;
        }

        if(100 != rGradient.GetEndIntensity())
        {
            aEnd *= (double)rGradient.GetEndIntensity() / 100.0;
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
            VirtualDevice aVDev;

            aVDev.SetOutputSizePixel(aBitmapEx.GetBitmap().GetSizePixel());
            aVDev.DrawGradient(Rectangle(Point(0, 0), aVDev.GetOutputSizePixel()), rGradient);

            aNewMask = AlphaMask(aVDev.GetBitmap(Point(0, 0), aVDev.GetOutputSizePixel()));
            bHasNewMask = true;
        }

        if(bCreateObject)
        {
            if(bHasNewMask || bFixedTransparence)
            {
                if(!aBitmapEx.IsAlpha() && !aBitmapEx.IsTransparent())
                {
                    // no transparence yet, apply new one
                    if(bFixedTransparence)
                    {
                        sal_uInt8 aAlpha(static_cast< sal_uInt8 >(basegfx::fround(fTransparence * 255.0)));

                        aNewMask = AlphaMask(aBitmapEx.GetBitmap().GetSizePixel(), &aAlpha);
                    }

                    aBitmapEx = BitmapEx(aBitmapEx.GetBitmap(), aNewMask);
                }
                else
                {
                    // mix existing and new alpha mask
                    AlphaMask aOldMask;

                    if(aBitmapEx.IsAlpha())
                    {
                        aOldMask = aBitmapEx.GetAlpha();
                    }
                    else if(TRANSPARENT_BITMAP == aBitmapEx.GetTransparentType())
                    {
                        aOldMask = aBitmapEx.GetMask();
                    }
                    else if(TRANSPARENT_COLOR == aBitmapEx.GetTransparentType())
                    {
                        aOldMask = aBitmapEx.GetBitmap().CreateMask(aBitmapEx.GetTransparentColor());
                    }

                    BitmapWriteAccess* pOld = aOldMask.AcquireWriteAccess();

                    if(pOld)
                    {
                        const double fFactor(1.0 / 255.0);

                        if(bFixedTransparence)
                        {
                            const double fOpNew(1.0 - fTransparence);

                            for(sal_uInt32 y(0); y < static_cast< sal_uInt32 >(pOld->Height()); y++)
                            {
                                for(sal_uInt32 x(0); x < static_cast< sal_uInt32 >(pOld->Width()); x++)
                                {
                                    const double fOpOld(1.0 - (pOld->GetPixel(y, x).GetIndex() * fFactor));
                                    const sal_uInt8 aCol(static_cast< sal_uInt8 >(basegfx::fround((1.0 - (fOpOld * fOpNew)) * 255.0)));

                                    pOld->SetPixel(y, x, BitmapColor(aCol));
                                }
                            }
                        }
                        else
                        {
                            BitmapReadAccess* pNew = aNewMask.AcquireReadAccess();

                            if(pNew)
                            {
                                if(pOld->Width() == pNew->Width() && pOld->Height() == pNew->Height())
                                {
                                    for(sal_uInt32 y(0); y < static_cast< sal_uInt32 >(pOld->Height()); y++)
                                    {
                                        for(sal_uInt32 x(0); x < static_cast< sal_uInt32 >(pOld->Width()); x++)
                                        {
                                            const double fOpOld(1.0 - (pOld->GetPixel(y, x).GetIndex() * fFactor));
                                            const double fOpNew(1.0 - (pNew->GetPixel(y, x).GetIndex() * fFactor));
                                            const sal_uInt8 aCol(static_cast< sal_uInt8 >(basegfx::fround((1.0 - (fOpOld * fOpNew)) * 255.0)));

                                            pOld->SetPixel(y, x, BitmapColor(aCol));
                                        }
                                    }
                                }
                                else
                                {
                                    OSL_ENSURE(false, "Alpha masks have different sizes (!)");
                                }

                                aNewMask.ReleaseAccess(pNew);
                            }
                            else
                            {
                                OSL_ENSURE(false, "Got no access to new alpha mask (!)");
                            }
                        }

                        aOldMask.ReleaseAccess(pOld);
                    }
                    else
                    {
                        OSL_ENSURE(false, "Got no access to old alpha mask (!)");
                    }

                    // apply combined bitmap as mask
                    aBitmapEx = BitmapEx(aBitmapEx.GetBitmap(), aOldMask);
                }
            }

            // create and add object
            SdrGrafObj* pGraf = new SdrGrafObj(
                mrModel,
                aBitmapEx,
                basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aRange.getRange(),
                    aRange.getMinimum()));

            // for MetaFloatTransparentAction, do not use SetAttributes(...)
            // since these metafile content is not used to draw line/fill
            // dependent of these setting at the device content
            pGraf->SetMergedItem(XLineStyleItem(XLINE_NONE));
            pGraf->SetMergedItem(XFillStyleItem(XFILL_NONE));
            InsertObj(pGraf);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
