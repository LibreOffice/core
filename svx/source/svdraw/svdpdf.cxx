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

#include "svdpdf.hxx"

#include <config_features.h>

#if HAVE_FEATURE_PDFIUM
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <fpdf_save.h>
#include <fpdf_text.h>

#include <vcl/bitmapaccess.hxx>
#include <vcl/graph.hxx>

#include <editeng/editdata.hxx>
#include <o3tl/make_unique.hxx>
#include <math.h>
#include <svx/xpoly.hxx>
#include <vcl/svapp.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <vcl/metric.hxx>
#include <editeng/charscaleitem.hxx>
#include <svx/xflhtit.hxx>
#include <svx/svdattr.hxx>
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
#include <vcl/dibtools.hxx>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <sal/log.hxx>

namespace
{
/// Convert from DPI to pixels.
/// PDFs don't have resolution, rather,
/// dimensions are in inches, with 72 points / inch.
/// Here we effectively render at 96 DPI (to match
/// the image rendered in vcl::ImportPDF in pdfread.cxx).
double lcl_PointToPixel(double fPoint) { return fPoint * 96. / 72.; }
/// Convert from pixels to logic (twips).
long lcl_ToLogic(double value)
{
    // Convert to integral preserving two dp.
    const long in = static_cast<long>(value * 100.);
    const long out = OutputDevice::LogicToLogic(in, MapUnit::MapPixel, MapUnit::Map100thMM);
    return out / 100;
}

double sqrt2(double a, double b) { return sqrt(a * a + b * b); }
}

struct FPDFBitmapDeleter
{
    void operator()(FPDF_BITMAP bitmap) { FPDFBitmap_Destroy(bitmap); }
};

using namespace com::sun::star;

ImpSdrPdfImport::ImpSdrPdfImport(SdrModel& rModel, SdrLayerID nLay, const tools::Rectangle& rRect,
                                 const std::shared_ptr<uno::Sequence<sal_Int8>>& pPdfData)
    : maTmpList()
    , mpVD(VclPtr<VirtualDevice>::Create())
    , maScaleRect(rRect)
    , mpPdfData(pPdfData)
    , mnMapScalingOfs(0)
    , mpModel(&rModel)
    , mnLayer(nLay)
    , maOldLineColor()
    , mnLineWidth(0)
    , maDash(css::drawing::DashStyle_RECT, 0, 0, 0, 0, 0)
    , mbMov(false)
    , mbSize(false)
    , maOfs(0, 0)
    , mfScaleX(1.0)
    , mfScaleY(1.0)
    , maScaleX(1.0)
    , maScaleY(1.0)
    , mbFntDirty(true)
    , mbLastObjWasPolyWithoutLine(false)
    , mbNoLine(false)
    , mbNoFill(false)
    , maClip()
    , mpPdfDocument(nullptr)
    , mnPageCount(0)
    , mdPageWidthPts(0)
    , mdPageHeightPts(0)
{
    mpVD->EnableOutput(false);
    mpVD->SetLineColor();
    mpVD->SetFillColor();
    maOldLineColor.SetRed(mpVD->GetLineColor().GetRed() + 1);
    mpLineAttr = o3tl::make_unique<SfxItemSet>(rModel.GetItemPool(),
                                               svl::Items<XATTR_LINE_FIRST, XATTR_LINE_LAST>{});
    mpFillAttr = o3tl::make_unique<SfxItemSet>(rModel.GetItemPool(),
                                               svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});
    mpTextAttr = o3tl::make_unique<SfxItemSet>(rModel.GetItemPool(),
                                               svl::Items<EE_ITEMS_START, EE_ITEMS_END>{});
    checkClip();

    FPDF_LIBRARY_CONFIG aConfig;
    aConfig.version = 2;
    aConfig.m_pUserFontPaths = nullptr;
    aConfig.m_pIsolate = nullptr;
    aConfig.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&aConfig);

    // Load the buffer using pdfium.
    mpPdfDocument = FPDF_LoadMemDocument(mpPdfData->getConstArray(), mpPdfData->getLength(),
                                         /*password=*/nullptr);
    if (!mpPdfDocument)
    {
        //TODO: Handle failure to load.
        switch (FPDF_GetLastError())
        {
            case FPDF_ERR_SUCCESS:
                break;
            case FPDF_ERR_UNKNOWN:
                break;
            case FPDF_ERR_FILE:
                break;
            case FPDF_ERR_FORMAT:
                break;
            case FPDF_ERR_PASSWORD:
                break;
            case FPDF_ERR_SECURITY:
                break;
            case FPDF_ERR_PAGE:
                break;
            default:
                break;
        }

        return;
    }

    mnPageCount = FPDF_GetPageCount(mpPdfDocument);
}

ImpSdrPdfImport::~ImpSdrPdfImport()
{
    FPDF_CloseDocument(mpPdfDocument);
    FPDF_DestroyLibrary();
}

void ImpSdrPdfImport::DoObjects(SvdProgressInfo* pProgrInfo, sal_uInt32* pActionsToReport,
                                int nPageIndex)
{
    const int nPageCount = FPDF_GetPageCount(mpPdfDocument);
    if (nPageCount > 0 && nPageIndex >= 0 && nPageIndex < nPageCount)
    {
        // Render next page.
        FPDF_PAGE pPdfPage = FPDF_LoadPage(mpPdfDocument, nPageIndex);
        if (pPdfPage == nullptr)
            return;

        const double dPageWidth = FPDF_GetPageWidth(pPdfPage);
        const double dPageHeight = FPDF_GetPageHeight(pPdfPage);
        SetupPageScale(dPageWidth, dPageHeight);

        // Load the page text to extract it when we get text elements.
        FPDF_TEXTPAGE pTextPage = FPDFText_LoadPage(pPdfPage);

        const int nPageObjectCount = FPDFPage_CountObjects(pPdfPage);
        if (pProgrInfo)
            pProgrInfo->SetActionCount(nPageObjectCount);

        for (int nPageObjectIndex = 0; nPageObjectIndex < nPageObjectCount; ++nPageObjectIndex)
        {
            FPDF_PAGEOBJECT pPageObject = FPDFPage_GetObject(pPdfPage, nPageObjectIndex);
            ImportPdfObject(pPageObject, pTextPage, nPageObjectIndex);
            if (pProgrInfo && pActionsToReport)
            {
                (*pActionsToReport)++;

                if (*pActionsToReport >= 16)
                {
                    if (!pProgrInfo->ReportActions(*pActionsToReport))
                        break;

                    *pActionsToReport = 0;
                }
            }
        }

        FPDFText_ClosePage(pTextPage);
        FPDF_ClosePage(pPdfPage);
    }
}

void ImpSdrPdfImport::SetupPageScale(const double dPageWidth, const double dPageHeight)
{
    mfScaleX = mfScaleY = 1.0;

    // Store the page dimensions in Points.
    mdPageWidthPts = dPageWidth;
    mdPageHeightPts = dPageHeight;

    Size aPageSize(lcl_ToLogic(lcl_PointToPixel(dPageWidth)),
                   lcl_ToLogic(lcl_PointToPixel(dPageHeight)));

    if (aPageSize.Width() && aPageSize.Height() && (!maScaleRect.IsEmpty()))
    {
        maOfs = maScaleRect.TopLeft();

        if (aPageSize.Width() != (maScaleRect.GetWidth() - 1))
        {
            mfScaleX = static_cast<double>(maScaleRect.GetWidth() - 1)
                       / static_cast<double>(aPageSize.Width());
        }

        if (aPageSize.Height() != (maScaleRect.GetHeight() - 1))
        {
            mfScaleY = static_cast<double>(maScaleRect.GetHeight() - 1)
                       / static_cast<double>(aPageSize.Height());
        }
    }

    mbMov = maOfs.X() != 0 || maOfs.Y() != 0;
    mbSize = false;
    maScaleX = Fraction(1, 1);
    maScaleY = Fraction(1, 1);

    if (aPageSize.Width() != (maScaleRect.GetWidth() - 1))
    {
        maScaleX = Fraction(maScaleRect.GetWidth() - 1, aPageSize.Width());
        mbSize = true;
    }

    if (aPageSize.Height() != (maScaleRect.GetHeight() - 1))
    {
        maScaleY = Fraction(maScaleRect.GetHeight() - 1, aPageSize.Height());
        mbSize = true;
    }
}

size_t ImpSdrPdfImport::DoImport(SdrObjList& rOL, size_t nInsPos, int nPageNumber,
                                 SvdProgressInfo* pProgrInfo)
{
    sal_uInt32 nActionsToReport(0);

    // execute
    DoObjects(pProgrInfo, &nActionsToReport, nPageNumber);

    if (pProgrInfo)
    {
        pProgrInfo->ReportActions(nActionsToReport);
        nActionsToReport = 0;
    }

    // MapMode scaling
    MapScaling();

    // To calculate the progress meter, we use GetActionSize()*3.
    // However, maTmpList has a lower entry count limit than GetActionSize(),
    // so the actions that were assumed were too much have to be re-added.
    // nActionsToReport = (rMtf.GetActionSize() - maTmpList.size()) * 2;

    // announce all currently unannounced rescales
    if (pProgrInfo)
    {
        pProgrInfo->ReportRescales(nActionsToReport);
        pProgrInfo->SetInsertCount(maTmpList.size());
    }

    nActionsToReport = 0;

    // insert all objects cached in aTmpList now into rOL from nInsPos
    nInsPos = std::min(nInsPos, rOL.GetObjCount());

    for (SdrObject* pObj : maTmpList)
    {
        rOL.NbcInsertObject(pObj, nInsPos);
        nInsPos++;

        if (pProgrInfo)
        {
            nActionsToReport++;

            if (nActionsToReport >= 32) // update all 32 actions
            {
                pProgrInfo->ReportInserts(nActionsToReport);
                nActionsToReport = 0;
            }
        }
    }

    // report all remaining inserts for the last time
    if (pProgrInfo)
    {
        pProgrInfo->ReportInserts(nActionsToReport);
    }

    return maTmpList.size();
}

void ImpSdrPdfImport::SetAttributes(SdrObject* pObj, bool bForceTextAttr)
{
    mbNoLine = false;
    mbNoFill = false;
    bool bLine(!bForceTextAttr);
    bool bFill(!pObj || (pObj->IsClosedObj() && !bForceTextAttr));
    bool bText(bForceTextAttr || (pObj && pObj->GetOutlinerParaObject()));

    if (bLine)
    {
        if (mnLineWidth)
        {
            mpLineAttr->Put(XLineWidthItem(mnLineWidth));
        }
        else
        {
            mpLineAttr->Put(XLineWidthItem(0));
        }

        maOldLineColor = mpVD->GetLineColor();

        if (mpVD->IsLineColor())
        {
            mpLineAttr->Put(XLineStyleItem(drawing::LineStyle_SOLID)); //TODO support dashed lines.
            mpLineAttr->Put(XLineColorItem(OUString(), mpVD->GetLineColor()));
        }
        else
        {
            mpLineAttr->Put(XLineStyleItem(drawing::LineStyle_NONE));
        }

        mpLineAttr->Put(XLineJointItem(css::drawing::LineJoint_NONE));

        // Add LineCap support
        mpLineAttr->Put(XLineCapItem(gaLineCap));

        if (((maDash.GetDots() && maDash.GetDotLen())
             || (maDash.GetDashes() && maDash.GetDashLen()))
            && maDash.GetDistance())
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

    if (bFill)
    {
        if (mpVD->IsFillColor())
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

    if (bText && mbFntDirty)
    {
        vcl::Font aFnt(mpVD->GetFont());
        const sal_uInt32 nHeight(FRound(aFnt.GetFontSize().Height() * mfScaleY));

        mpTextAttr->Put(SvxFontItem(aFnt.GetFamilyType(), aFnt.GetFamilyName(), aFnt.GetStyleName(),
                                    aFnt.GetPitch(), aFnt.GetCharSet(), EE_CHAR_FONTINFO));
        mpTextAttr->Put(SvxFontItem(aFnt.GetFamilyType(), aFnt.GetFamilyName(), aFnt.GetStyleName(),
                                    aFnt.GetPitch(), aFnt.GetCharSet(), EE_CHAR_FONTINFO_CJK));
        mpTextAttr->Put(SvxFontItem(aFnt.GetFamilyType(), aFnt.GetFamilyName(), aFnt.GetStyleName(),
                                    aFnt.GetPitch(), aFnt.GetCharSet(), EE_CHAR_FONTINFO_CTL));
        mpTextAttr->Put(SvxPostureItem(aFnt.GetItalic(), EE_CHAR_ITALIC));
        mpTextAttr->Put(SvxWeightItem(aFnt.GetWeight(), EE_CHAR_WEIGHT));
        mpTextAttr->Put(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT));
        mpTextAttr->Put(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT_CJK));
        mpTextAttr->Put(SvxFontHeightItem(nHeight, 100, EE_CHAR_FONTHEIGHT_CTL));
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

    if (pObj)
    {
        pObj->SetLayer(mnLayer);

        if (bLine)
        {
            pObj->SetMergedItemSet(*mpLineAttr);
        }

        if (bFill)
        {
            pObj->SetMergedItemSet(*mpFillAttr);
        }

        if (bText)
        {
            pObj->SetMergedItemSet(*mpTextAttr);
            pObj->SetMergedItem(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT));
        }
    }
}

void ImpSdrPdfImport::InsertObj(SdrObject* pObj, bool bScale)
{
    if (bScale && !maScaleRect.IsEmpty())
    {
        if (mbSize)
        {
            pObj->NbcResize(Point(), maScaleX, maScaleY);
        }

        if (mbMov)
        {
            pObj->NbcMove(Size(maOfs.X(), maOfs.Y()));
        }
    }

    if (isClip())
    {
        const basegfx::B2DPolyPolygon aPoly(pObj->TakeXorPoly());
        const basegfx::B2DRange aOldRange(aPoly.getB2DRange());
        const SdrLayerID aOldLayer(pObj->GetLayer());
        const SfxItemSet aOldItemSet(pObj->GetMergedItemSet());
        const SdrGrafObj* pSdrGrafObj = dynamic_cast<SdrGrafObj*>(pObj);
        const SdrTextObj* pSdrTextObj = dynamic_cast<SdrTextObj*>(pObj);

        if (pSdrTextObj && pSdrTextObj->HasText())
        {
            // all text objects are created from ImportText and have no line or fill attributes, so
            // it is okay to concentrate on the text itself
            while (true)
            {
                const basegfx::B2DPolyPolygon aTextContour(pSdrTextObj->TakeContour());
                const basegfx::B2DRange aTextRange(aTextContour.getB2DRange());
                const basegfx::B2DRange aClipRange(maClip.getB2DRange());

                // no overlap -> completely outside
                if (!aClipRange.overlaps(aTextRange))
                {
                    SdrObject::Free(pObj);
                    break;
                }

                // when the clip is a rectangle fast check for inside is possible
                if (basegfx::utils::isRectangle(maClip) && aClipRange.isInside(aTextRange))
                {
                    // completely inside ClipRect
                    break;
                }

                // here text needs to be clipped; to do so, convert to SdrObjects with polygons
                // and add these recursively. Delete original object, do not add in this run
                SdrObject* pConverted = pSdrTextObj->ConvertToPolyObj(true, true);
                SdrObject::Free(pObj);

                if (pConverted)
                {
                    // recursively add created conversion; per definition this shall not
                    // contain further SdrTextObjs. Visit only non-group objects
                    SdrObjListIter aIter(*pConverted, SdrIterMode::DeepNoGroups);

                    // work with clones; the created conversion may contain group objects
                    // and when working with the original objects the loop itself could
                    // break and the cleanup later would be pretty complicated (only delete group
                    // objects, are these empty, ...?)
                    while (aIter.IsMore())
                    {
                        SdrObject* pCandidate = aIter.Next();
                        OSL_ENSURE(pCandidate && dynamic_cast<SdrObjGroup*>(pCandidate) == nullptr,
                                   "SdrObjListIter with SdrIterMode::DeepNoGroups error (!)");
                        SdrObject* pNewClone(
                            pCandidate->CloneSdrObject(pCandidate->getSdrModelFromSdrObject()));

                        if (pNewClone)
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

            if (pSdrGrafObj)
            {
                aBitmapEx = pSdrGrafObj->GetGraphic().GetBitmapEx();
            }

            SdrObject::Free(pObj);

            if (!aOldRange.isEmpty())
            {
                // clip against ClipRegion
                const basegfx::B2DPolyPolygon aNewPoly(basegfx::utils::clipPolyPolygonOnPolyPolygon(
                    aPoly, maClip, true, !aPoly.isClosed()));
                const basegfx::B2DRange aNewRange(aNewPoly.getB2DRange());

                if (!aNewRange.isEmpty())
                {
                    pObj = new SdrPathObj(*mpModel, aNewPoly.isClosed() ? OBJ_POLY : OBJ_PLIN,
                                          aNewPoly);

                    pObj->SetLayer(aOldLayer);
                    pObj->SetMergedItemSet(aOldItemSet);

                    if (!!aBitmapEx)
                    {
                        // aNewRange is inside of aOldRange and defines which part of aBitmapEx is used
                        const double fScaleX(aBitmapEx.GetSizePixel().Width()
                                             / (aOldRange.getWidth() ? aOldRange.getWidth() : 1.0));
                        const double fScaleY(
                            aBitmapEx.GetSizePixel().Height()
                            / (aOldRange.getHeight() ? aOldRange.getHeight() : 1.0));
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
                            basegfx::fround(ceil(std::min(
                                static_cast<double>(aOrigSizePixel.Width()), aPixel.getWidth()))),
                            basegfx::fround(
                                ceil(std::min(static_cast<double>(aOrigSizePixel.Height()),
                                              aPixel.getHeight()))));
                        const BitmapEx aClippedBitmap(aBitmapEx, aClipTopLeft, aClipSize);

                        pObj->SetMergedItem(XFillStyleItem(drawing::FillStyle_BITMAP));
                        pObj->SetMergedItem(XFillBitmapItem(OUString(), Graphic(aClippedBitmap)));
                        pObj->SetMergedItem(XFillBmpTileItem(false));
                        pObj->SetMergedItem(XFillBmpStretchItem(true));
                    }
                }
            }
        }
    }

    if (pObj)
    {
        // #i111954# check object for visibility
        // used are SdrPathObj, SdrRectObj, SdrCircObj, SdrGrafObj
        bool bVisible(false);

        if (pObj->HasLineStyle())
        {
            bVisible = true;
        }

        if (!bVisible && pObj->HasFillStyle())
        {
            bVisible = true;
        }

        if (!bVisible)
        {
            SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>(pObj);

            if (pTextObj && pTextObj->HasText())
            {
                bVisible = true;
            }
        }

        if (!bVisible)
        {
            SdrGrafObj* pGrafObj = dynamic_cast<SdrGrafObj*>(pObj);

            if (pGrafObj)
            {
                // this may be refined to check if the graphic really is visible. It
                // is here to ensure that graphic objects without fill, line and text
                // get created
                bVisible = true;
            }
        }

        if (!bVisible)
        {
            SdrObject::Free(pObj);
        }
        else
        {
            maTmpList.push_back(pObj);

            if (dynamic_cast<SdrPathObj*>(pObj))
            {
                const bool bClosed(pObj->IsClosedObj());

                mbLastObjWasPolyWithoutLine = mbNoLine && bClosed;
            }
            else
            {
                mbLastObjWasPolyWithoutLine = false;
            }
        }
    }
}

bool ImpSdrPdfImport::CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    // #i73407# reformulation to use new B2DPolygon classes
    if (mbLastObjWasPolyWithoutLine)
    {
        SdrObject* pTmpObj = !maTmpList.empty() ? maTmpList[maTmpList.size() - 1] : nullptr;
        SdrPathObj* pLastPoly = dynamic_cast<SdrPathObj*>(pTmpObj);

        if (pLastPoly)
        {
            if (pLastPoly->GetPathPoly() == rPolyPolygon)
            {
                SetAttributes(nullptr);

                if (!mbNoLine && mbNoFill)
                {
                    pLastPoly->SetMergedItemSet(*mpLineAttr);

                    return true;
                }
            }
        }
    }

    return false;
}

void ImpSdrPdfImport::checkClip()
{
    if (mpVD->IsClipRegion())
    {
        maClip = mpVD->GetClipRegion().GetAsB2DPolyPolygon();

        if (isClip())
        {
            const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(
                mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));

            maClip.transform(aTransform);
        }
    }
}

bool ImpSdrPdfImport::isClip() const { return !maClip.getB2DRange().isEmpty(); }
void ImpSdrPdfImport::ImportPdfObject(FPDF_PAGEOBJECT pPageObject, FPDF_TEXTPAGE pTextPage,
                                      int nPageObjectIndex)
{
    if (pPageObject == nullptr)
        return;

    const int nPageObjectType = FPDFPageObj_GetType(pPageObject);
    switch (nPageObjectType)
    {
        case FPDF_PAGEOBJ_TEXT:
            ImportText(pPageObject, pTextPage, nPageObjectIndex);
            break;
        case FPDF_PAGEOBJ_PATH:
            ImportPath(pPageObject, nPageObjectIndex);
            break;
        case FPDF_PAGEOBJ_IMAGE:
            ImportImage(pPageObject, nPageObjectIndex);
            break;
        case FPDF_PAGEOBJ_SHADING:
            SAL_WARN("sd.filter", "Got page object SHADING: " << nPageObjectIndex);
            break;
        case FPDF_PAGEOBJ_FORM:
            ImportForm(pPageObject, pTextPage, nPageObjectIndex);
            break;
        default:
            SAL_WARN("sd.filter", "Unknown PDF page object #" << nPageObjectIndex
                                                              << " of type: " << nPageObjectType);
            break;
    }
}

void ImpSdrPdfImport::ImportForm(FPDF_PAGEOBJECT pPageObject, FPDF_TEXTPAGE pTextPage,
                                 int /*nPageObjectIndex*/)
{
    // Get the form matrix to perform correct translation/scaling of the form sub-objects.
    const Matrix aOldMatrix = mCurMatrix;

    double a, b, c, d, e, f;
    FPDFFormObj_GetMatrix(pPageObject, &a, &b, &c, &d, &e, &f);
    mCurMatrix = Matrix(a, b, c, d, e, f);

    const int nCount = FPDFFormObj_CountObjects(pPageObject);
    for (int nIndex = 0; nIndex < nCount; ++nIndex)
    {
        FPDF_PAGEOBJECT pFormObject = FPDFFormObj_GetObject(pPageObject, nIndex);
        ImportPdfObject(pFormObject, pTextPage, -1);
    }

    // Restore the old one.
    mCurMatrix = aOldMatrix;
}

void ImpSdrPdfImport::ImportText(FPDF_PAGEOBJECT pPageObject, FPDF_TEXTPAGE pTextPage,
                                 int /*nPageObjectIndex*/)
{
    float left;
    float bottom;
    float right;
    float top;
    if (!FPDFPageObj_GetBounds(pPageObject, &left, &bottom, &right, &top))
    {
        SAL_WARN("sd.filter", "FAILED to get TEXT bounds");
    }

    if (left == right || top == bottom)
        return;

    double a, b, c, d, e, f;
    FPDFText_GetMatrix(pPageObject, &a, &b, &c, &d, &e, &f);
    Matrix aTextMatrix(mCurMatrix);

    aTextMatrix.Transform(left, right, top, bottom);
    const tools::Rectangle aRect = PointsToLogic(left, right, top, bottom);

    const int nChars = FPDFTextObj_GetText(pPageObject, pTextPage, nullptr, 0);
    std::unique_ptr<sal_Unicode[]> pText(new sal_Unicode[nChars]);

    const int nActualChars = FPDFTextObj_GetText(pPageObject, pTextPage, pText.get(), nChars);
    if (nActualChars <= 0)
    {
        return;
    }

    OUString sText(pText.get(), nActualChars);

    const double dFontSize = FPDFTextObj_GetFontSize(pPageObject);
    double dFontSizeH = fabs(sqrt2(a, c) * dFontSize);
    double dFontSizeV = fabs(sqrt2(b, d) * dFontSize);
    dFontSizeH = lcl_PointToPixel(dFontSizeH);
    dFontSizeV = lcl_PointToPixel(dFontSizeV);
    dFontSizeH = lcl_ToLogic(dFontSizeH);
    dFontSizeV = lcl_ToLogic(dFontSizeV);

    const Size aFontSize(dFontSizeH, dFontSizeV);
    vcl::Font aFnt = mpVD->GetFont();
    if (aFontSize != aFnt.GetFontSize())
    {
        aFnt.SetFontSize(aFontSize);
        mpVD->SetFont(aFnt);
        mbFntDirty = true;
    }

    const int nFontName = 80 + 1;
    std::unique_ptr<char[]> pFontName(new char[nFontName]); // + terminating null
    char* pCharFontName = reinterpret_cast<char*>(pFontName.get());
    int nFontNameChars = FPDFTextObj_GetFontName(pPageObject, pCharFontName, nFontName);
    if (nFontName >= nFontNameChars)
    {
        OUString sFontName = OUString::createFromAscii(pFontName.get());
        if (sFontName != aFnt.GetFamilyName())
        {
            aFnt.SetFamilyName(sFontName);
            mpVD->SetFont(aFnt);
            mbFntDirty = true;
        }
    }

    Color aTextColor(COL_TRANSPARENT);
    bool bFill = false;
    bool bUse = true;
    switch (FPDFText_GetTextRenderMode(pPageObject))
    {
        case FPDF_TEXTRENDERMODE_FILL:
        case FPDF_TEXTRENDERMODE_FILL_CLIP:
        case FPDF_TEXTRENDERMODE_FILL_STROKE:
        case FPDF_TEXTRENDERMODE_FILL_STROKE_CLIP:
            bFill = true;
            break;
        case FPDF_TEXTRENDERMODE_STROKE:
        case FPDF_TEXTRENDERMODE_STROKE_CLIP:
            break;
        case FPDF_TEXTRENDERMODE_INVISIBLE:
        case FPDF_TEXTRENDERMODE_CLIP:
            bUse = false;
            break;
    }
    if (bUse)
    {
        unsigned int nR, nG, nB, nA;
        bool bRet = bFill ? FPDFPageObj_GetFillColor(pPageObject, &nR, &nG, &nB, &nA)
                          : FPDFPageObj_GetStrokeColor(pPageObject, &nR, &nG, &nB, &nA);
        if (bRet)
            aTextColor = Color(nR, nG, nB);
    }

    if (aTextColor != mpVD->GetTextColor())
    {
        mpVD->SetTextColor(aTextColor);
        mbFntDirty = true;
    }

    ImportText(aRect.TopLeft(), aRect.GetSize(), sText);
}

void ImpSdrPdfImport::ImportText(const Point& rPos, const Size& rSize, const OUString& rStr)
{
    // calc text box size, add 5% to make it fit safely

    FontMetric aFontMetric(mpVD->GetFontMetric());
    vcl::Font aFnt(mpVD->GetFont());
    FontAlign eAlg(aFnt.GetAlignment());

    // sal_Int32 nTextWidth = static_cast<sal_Int32>(mpVD->GetTextWidth(rStr) * mfScaleX);
    sal_Int32 nTextHeight = static_cast<sal_Int32>(mpVD->GetTextHeight() * mfScaleY);

    Point aPos(FRound(rPos.X() * mfScaleX + maOfs.X()), FRound(rPos.Y() * mfScaleY + maOfs.Y()));
    Size aSize(FRound(rSize.Width() * mfScaleX), FRound(rSize.Height() * mfScaleY));

    if (eAlg == ALIGN_BASELINE)
        aPos.AdjustY(-FRound(aFontMetric.GetAscent() * mfScaleY));
    else if (eAlg == ALIGN_BOTTOM)
        aPos.AdjustY(-nTextHeight);

    tools::Rectangle aTextRect(aPos, aSize);
    SdrRectObj* pText = new SdrRectObj(*mpModel, OBJ_TEXT, aTextRect);

    pText->SetMergedItem(makeSdrTextUpperDistItem(0));
    pText->SetMergedItem(makeSdrTextLowerDistItem(0));
    pText->SetMergedItem(makeSdrTextRightDistItem(0));
    pText->SetMergedItem(makeSdrTextLeftDistItem(0));

    if (aFnt.GetAverageFontWidth())
    {
        pText->ClearMergedItem(SDRATTR_TEXT_AUTOGROWWIDTH);
        pText->SetMergedItem(makeSdrTextAutoGrowHeightItem(false));
        // don't let the margins eat the space needed for the text
        pText->SetMergedItem(SdrTextFitToSizeTypeItem(drawing::TextFitToSizeType_ALLLINES));
    }
    else
    {
        pText->SetMergedItem(makeSdrTextAutoGrowWidthItem(true));
    }

    pText->SetLayer(mnLayer);
    pText->NbcSetText(rStr);
    SetAttributes(pText, true);
    pText->SetSnapRect(aTextRect);

    if (!aFnt.IsTransparent())
    {
        SfxItemSet aAttr(*mpFillAttr->GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});
        aAttr.Put(XFillStyleItem(drawing::FillStyle_SOLID));
        aAttr.Put(XFillColorItem(OUString(), aFnt.GetFillColor()));
        pText->SetMergedItemSet(aAttr);
    }
    sal_uInt32 nAngle = aFnt.GetOrientation();
    if (nAngle)
    {
        nAngle *= 10;
        double a = nAngle * nPi180;
        double nSin = sin(a);
        double nCos = cos(a);
        pText->NbcRotate(aPos, nAngle, nSin, nCos);
    }
    InsertObj(pText, false);
}

void ImpSdrPdfImport::MapScaling()
{
    const size_t nCount(maTmpList.size());
    const MapMode& rMap = mpVD->GetMapMode();
    Point aMapOrg(rMap.GetOrigin());
    bool bMov2(aMapOrg.X() != 0 || aMapOrg.Y() != 0);

    if (bMov2)
    {
        for (size_t i = mnMapScalingOfs; i < nCount; i++)
        {
            SdrObject* pObj = maTmpList[i];

            pObj->NbcMove(Size(aMapOrg.X(), aMapOrg.Y()));
        }
    }

    mnMapScalingOfs = nCount;
}

void ImpSdrPdfImport::ImportImage(FPDF_PAGEOBJECT pPageObject, int /*nPageObjectIndex*/)
{
    std::unique_ptr<std::remove_pointer<FPDF_BITMAP>::type, FPDFBitmapDeleter> bitmap(
        FPDFImageObj_GetBitmap(pPageObject));
    if (!bitmap)
    {
        SAL_WARN("sd.filter", "Failed to get IMAGE");
        return;
    }

    const int format = FPDFBitmap_GetFormat(bitmap.get());
    if (format == FPDFBitmap_Unknown)
    {
        SAL_WARN("sd.filter", "Failed to get IMAGE format");
        return;
    }

    const unsigned char* pBuf
        = static_cast<const unsigned char*>(FPDFBitmap_GetBuffer(bitmap.get()));
    const int nWidth = FPDFBitmap_GetWidth(bitmap.get());
    const int nHeight = FPDFBitmap_GetHeight(bitmap.get());
    const int nStride = FPDFBitmap_GetStride(bitmap.get());
    Bitmap aBitmap(Size(nWidth, nHeight), 24);

    switch (format)
    {
        case FPDFBitmap_BGR:
            ReadRawDIB(aBitmap, pBuf, ScanlineFormat::N24BitTcBgr, nHeight, nStride);
            break;
        case FPDFBitmap_BGRx:
            ReadRawDIB(aBitmap, pBuf, ScanlineFormat::N32BitTcRgba, nHeight, nStride);
            break;
        case FPDFBitmap_BGRA:
            ReadRawDIB(aBitmap, pBuf, ScanlineFormat::N32BitTcBgra, nHeight, nStride);
            break;
        default:
            SAL_WARN("sd.filter", "Got IMAGE width: " << nWidth << ", height: " << nHeight
                                                      << ", stride: " << nStride
                                                      << ", format: " << format);
            break;
    }

    float left;
    float bottom;
    float right;
    float top;
    if (!FPDFPageObj_GetBounds(pPageObject, &left, &bottom, &right, &top))
    {
        SAL_WARN("sd.filter", "FAILED to get image bounds");
    }

    tools::Rectangle aRect = PointsToLogic(left, right, top, bottom);
    aRect.AdjustRight(1);
    aRect.AdjustBottom(1);

    SdrGrafObj* pGraf = new SdrGrafObj(*mpModel, Graphic(aBitmap), aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}

void ImpSdrPdfImport::ImportPath(FPDF_PAGEOBJECT pPageObject, int /*nPageObjectIndex*/)
{
    double a, b, c, d, e, f;
    FPDFPath_GetMatrix(pPageObject, &a, &b, &c, &d, &e, &f);
    Matrix aPathMatrix(a, b, c, d, e, f);
    aPathMatrix.Concatinate(mCurMatrix);

    basegfx::B2DPolyPolygon aPolyPoly;
    basegfx::B2DPolygon aPoly;
    std::vector<basegfx::B2DPoint> aBezier;

    const int nSegments = FPDFPath_CountSegments(pPageObject);
    for (int nSegmentIndex = 0; nSegmentIndex < nSegments; ++nSegmentIndex)
    {
        FPDF_PATHSEGMENT pPathSegment = FPDFPath_GetPathSegment(pPageObject, nSegmentIndex);
        if (pPathSegment != nullptr)
        {
            float fx, fy;
            if (!FPDFPathSegment_GetPoint(pPathSegment, &fx, &fy))
            {
                SAL_WARN("sd.filter", "Failed to get PDF path segment point");
                continue;
            }

            double x = fx;
            double y = fy;
            aPathMatrix.Transform(x, y);
            const bool bClose = FPDFPathSegment_GetClose(pPathSegment);
            if (bClose)
                aPoly.setClosed(bClose); // TODO: Review

            Point aPoint = PointsToLogic(x, y);
            x = aPoint.X();
            y = aPoint.Y();

            const int nSegmentType = FPDFPathSegment_GetType(pPathSegment);
            switch (nSegmentType)
            {
                case FPDF_SEGMENT_LINETO:
                    aPoly.append(basegfx::B2DPoint(x, y));
                    break;

                case FPDF_SEGMENT_BEZIERTO:
                    aBezier.emplace_back(x, y);
                    if (aBezier.size() == 3)
                    {
                        aPoly.appendBezierSegment(aBezier[0], aBezier[1], aBezier[2]);
                        aBezier.clear();
                    }
                    break;

                case FPDF_SEGMENT_MOVETO:
                    // New Poly.
                    if (aPoly.count() > 0)
                    {
                        aPolyPoly.append(aPoly, 1);
                        aPoly.clear();
                    }

                    aPoly.append(basegfx::B2DPoint(x, y));
                    break;

                case FPDF_SEGMENT_UNKNOWN:
                default:
                    SAL_WARN("sd.filter", "Unknown path segment type in PDF: " << nSegmentType);
                    break;
            }
        }
    }

    if (aBezier.size() == 3)
    {
        aPoly.appendBezierSegment(aBezier[0], aBezier[1], aBezier[2]);
        aBezier.clear();
    }

    if (aPoly.count() > 0)
    {
        aPolyPoly.append(aPoly, 1);
        aPoly.clear();
    }

    const basegfx::B2DHomMatrix aTransform(
        basegfx::utils::createScaleTranslateB2DHomMatrix(mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));
    aPolyPoly.transform(aTransform);

    float fWidth = 1;
    FPDFPageObj_GetStrokeWidth(pPageObject, &fWidth);
    const double dWidth = 0.5 * fabs(sqrt2(aPathMatrix.a(), aPathMatrix.c()) * fWidth);
    mnLineWidth = lcl_ToLogic(lcl_PointToPixel(dWidth));

    int nFillMode = FPDF_FILLMODE_ALTERNATE;
    FPDF_BOOL bStroke = 1; // Assume we have to draw, unless told otherwise.
    if (FPDFPath_GetDrawMode(pPageObject, &nFillMode, &bStroke))
    {
        if (nFillMode == FPDF_FILLMODE_ALTERNATE)
            mpVD->SetDrawMode(DrawModeFlags::Default);
        else if (nFillMode == FPDF_FILLMODE_WINDING)
            mpVD->SetDrawMode(DrawModeFlags::Default);
        else
            mpVD->SetDrawMode(DrawModeFlags::NoFill);
    }

    unsigned int nR;
    unsigned int nG;
    unsigned int nB;
    unsigned int nA;
    FPDFPath_GetFillColor(pPageObject, &nR, &nG, &nB, &nA);
    mpVD->SetFillColor(Color(nR, nG, nB));

    if (bStroke)
    {
        FPDFPath_GetStrokeColor(pPageObject, &nR, &nG, &nB, &nA);
        mpVD->SetLineColor(Color(nR, nG, nB));
    }
    else
        mpVD->SetLineColor(COL_TRANSPARENT);

    if (!mbLastObjWasPolyWithoutLine || !CheckLastPolyLineAndFillMerge(aPolyPoly))
    {
        SdrPathObj* pPath = new SdrPathObj(*mpModel, OBJ_POLY, aPolyPoly);
        SetAttributes(pPath);
        InsertObj(pPath, false);
    }
}

Point ImpSdrPdfImport::PointsToLogic(double x, double y) const
{
    y = correctVertOrigin(y);
    x = lcl_PointToPixel(x);
    y = lcl_PointToPixel(y);

    Point aPos(lcl_ToLogic(x), lcl_ToLogic(y));
    return aPos;
}

tools::Rectangle ImpSdrPdfImport::PointsToLogic(double left, double right, double top,
                                                double bottom) const
{
    top = correctVertOrigin(top);
    bottom = correctVertOrigin(bottom);

    left = lcl_PointToPixel(left);
    right = lcl_PointToPixel(right);
    top = lcl_PointToPixel(top);
    bottom = lcl_PointToPixel(bottom);

    Point aPos(lcl_ToLogic(left), lcl_ToLogic(top));
    Size aSize(lcl_ToLogic(right - left), lcl_ToLogic(bottom - top));
    tools::Rectangle aRect(aPos, aSize);
    return aRect;
}

#endif // HAVE_FEATURE_PDFIUM

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
