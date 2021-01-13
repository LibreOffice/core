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

#include <svdpdf.hxx>

#include <config_features.h>

#if HAVE_FEATURE_PDFIUM
#include <fpdfview.h>
#include <fpdf_edit.h>

#include <tools/UnitConversion.hxx>
#include <vcl/graph.hxx>
#include <vcl/vectorgraphicdata.hxx>

#include <math.h>
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
#include <editeng/fontitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <vcl/metric.hxx>
#include <editeng/charscaleitem.hxx>
#include <svx/sdtditm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdorect.hxx>
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
#include <svx/xfillit0.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xlineit0.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/svditer.hxx>
#include <svx/svdogrp.hxx>
#include <vcl/dibtools.hxx>
#include <sal/log.hxx>

namespace
{
double sqrt2(double a, double b) { return sqrt(a * a + b * b); }
}

using namespace com::sun::star;

ImpSdrPdfImport::ImpSdrPdfImport(SdrModel& rModel, SdrLayerID nLay, const tools::Rectangle& rRect,
                                 Graphic const& rGraphic)
    : maTmpList()
    , mpVD(VclPtr<VirtualDevice>::Create())
    , maScaleRect(rRect)
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
    , mnPageCount(0)
    , mdPageHeightPts(0)
    , mpPDFium(vcl::pdf::PDFiumLibrary::get())
{
    mpVD->EnableOutput(false);
    mpVD->SetLineColor();
    mpVD->SetFillColor();
    maOldLineColor.SetRed(mpVD->GetLineColor().GetRed() + 1);
    mpLineAttr = std::make_unique<SfxItemSet>(rModel.GetItemPool(),
                                              svl::Items<XATTR_LINE_FIRST, XATTR_LINE_LAST>{});
    mpFillAttr = std::make_unique<SfxItemSet>(rModel.GetItemPool(),
                                              svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});
    mpTextAttr = std::make_unique<SfxItemSet>(rModel.GetItemPool(),
                                              svl::Items<EE_ITEMS_START, EE_ITEMS_END>{});
    checkClip();

    // Load the buffer using pdfium.
    auto const& rVectorGraphicData = rGraphic.getVectorGraphicData();
    auto* pData = rVectorGraphicData->getBinaryDataContainer().getData();
    sal_Int32 nSize = rVectorGraphicData->getBinaryDataContainer().getSize();
    mpPdfDocument = mpPDFium->openDocument(pData, nSize);
    if (!mpPdfDocument)
        return;

    mnPageCount = mpPdfDocument->getPageCount();
}

ImpSdrPdfImport::~ImpSdrPdfImport() = default;

void ImpSdrPdfImport::DoObjects(SvdProgressInfo* pProgrInfo, sal_uInt32* pActionsToReport,
                                int nPageIndex)
{
    const int nPageCount = mpPdfDocument->getPageCount();
    if (!(nPageCount > 0 && nPageIndex >= 0 && nPageIndex < nPageCount))
        return;

    // Render next page.
    auto pPdfPage = mpPdfDocument->openPage(nPageIndex);
    if (!pPdfPage)
        return;

    basegfx::B2DSize dPageSize = mpPdfDocument->getPageSize(nPageIndex);

    const double dPageWidth = dPageSize.getX();
    const double dPageHeight = dPageSize.getY();

    SetupPageScale(dPageWidth, dPageHeight);

    // Load the page text to extract it when we get text elements.
    auto pTextPage = pPdfPage->getTextPage();

    const int nPageObjectCount = pPdfPage->getObjectCount();
    if (pProgrInfo)
        pProgrInfo->SetActionCount(nPageObjectCount);

    for (int nPageObjectIndex = 0; nPageObjectIndex < nPageObjectCount; ++nPageObjectIndex)
    {
        auto pPageObject = pPdfPage->getObject(nPageObjectIndex);
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
}

void ImpSdrPdfImport::SetupPageScale(const double dPageWidth, const double dPageHeight)
{
    mfScaleX = mfScaleY = 1.0;

    // Store the page dimensions in Points.
    mdPageHeightPts = dPageHeight;

    Size aPageSize(convertPointToMm100(dPageWidth), convertPointToMm100(dPageHeight));

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

    if (!pObj)
        return;

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
                SdrObjectUniquePtr pConverted = pSdrTextObj->ConvertToPolyObj(true, true);
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

    if (!pObj)
        return;

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
void ImpSdrPdfImport::ImportPdfObject(
    std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
    std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage, int nPageObjectIndex)
{
    if (!pPageObject)
        return;

    const vcl::pdf::PDFPageObjectType ePageObjectType = pPageObject->getType();
    switch (ePageObjectType)
    {
        case vcl::pdf::PDFPageObjectType::Text:
            ImportText(pPageObject, pTextPage, nPageObjectIndex);
            break;
        case vcl::pdf::PDFPageObjectType::Path:
            ImportPath(pPageObject, nPageObjectIndex);
            break;
        case vcl::pdf::PDFPageObjectType::Image:
            ImportImage(pPageObject, nPageObjectIndex);
            break;
        case vcl::pdf::PDFPageObjectType::Shading:
            SAL_WARN("sd.filter", "Got page object SHADING: " << nPageObjectIndex);
            break;
        case vcl::pdf::PDFPageObjectType::Form:
            ImportForm(pPageObject, pTextPage, nPageObjectIndex);
            break;
        default:
            SAL_WARN("sd.filter", "Unknown PDF page object #" << nPageObjectIndex << " of type: "
                                                              << static_cast<int>(ePageObjectType));
            break;
    }
}

void ImpSdrPdfImport::ImportForm(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                                 std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage,
                                 int /*nPageObjectIndex*/)
{
    // Get the form matrix to perform correct translation/scaling of the form sub-objects.
    const basegfx::B2DHomMatrix aOldMatrix = maCurrentMatrix;

    maCurrentMatrix = pPageObject->getMatrix();

    const int nCount = pPageObject->getFormObjectCount();
    for (int nIndex = 0; nIndex < nCount; ++nIndex)
    {
        auto pFormObject = pPageObject->getFormObject(nIndex);

        ImportPdfObject(pFormObject, pTextPage, -1);
    }

    // Restore the old one.
    maCurrentMatrix = aOldMatrix;
}

void ImpSdrPdfImport::ImportText(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                                 std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage,
                                 int /*nPageObjectIndex*/)
{
    basegfx::B2DRectangle aTextRect = pPageObject->getBounds();
    basegfx::B2DHomMatrix aMatrix = pPageObject->getMatrix();

    basegfx::B2DHomMatrix aTextMatrix(maCurrentMatrix);

    aTextRect *= aTextMatrix;
    const tools::Rectangle aRect = PointsToLogic(aTextRect.getMinX(), aTextRect.getMaxX(),
                                                 aTextRect.getMinY(), aTextRect.getMaxY());

    OUString sText = pPageObject->getText(pTextPage);

    const double dFontSize = pPageObject->getFontSize();
    double dFontSizeH = fabs(sqrt2(aMatrix.a(), aMatrix.c()) * dFontSize);
    double dFontSizeV = fabs(sqrt2(aMatrix.b(), aMatrix.d()) * dFontSize);

    dFontSizeH = convertPointToMm100(dFontSizeH);
    dFontSizeV = convertPointToMm100(dFontSizeV);

    const Size aFontSize(dFontSizeH, dFontSizeV);
    vcl::Font aFnt = mpVD->GetFont();
    if (aFontSize != aFnt.GetFontSize())
    {
        aFnt.SetFontSize(aFontSize);
        mpVD->SetFont(aFnt);
        mbFntDirty = true;
    }

    OUString sFontName = pPageObject->getFontName();
    if (!sFontName.isEmpty() && sFontName != aFnt.GetFamilyName())
    {
        aFnt.SetFamilyName(sFontName);
        mpVD->SetFont(aFnt);
        mbFntDirty = true;
    }

    Color aTextColor(COL_TRANSPARENT);
    bool bFill = false;
    bool bUse = true;
    switch (pPageObject->getTextRenderMode())
    {
        case vcl::pdf::PDFTextRenderMode::Fill:
        case vcl::pdf::PDFTextRenderMode::FillClip:
        case vcl::pdf::PDFTextRenderMode::FillStroke:
        case vcl::pdf::PDFTextRenderMode::FillStrokeClip:
            bFill = true;
            break;
        case vcl::pdf::PDFTextRenderMode::Stroke:
        case vcl::pdf::PDFTextRenderMode::StrokeClip:
        case vcl::pdf::PDFTextRenderMode::Unknown:
            break;
        case vcl::pdf::PDFTextRenderMode::Invisible:
        case vcl::pdf::PDFTextRenderMode::Clip:
            bUse = false;
            break;
    }
    if (bUse)
    {
        Color aColor = bFill ? pPageObject->getFillColor() : pPageObject->getStrokeColor();
        if (aColor != COL_TRANSPARENT)
            aTextColor = aColor.GetRGBColor();
    }

    if (aTextColor != mpVD->GetTextColor())
    {
        mpVD->SetTextColor(aTextColor);
        mbFntDirty = true;
    }

    InsertTextObject(aRect.TopLeft(), aRect.GetSize(), sText);
}

void ImpSdrPdfImport::InsertTextObject(const Point& rPos, const Size& rSize, const OUString& rStr)
{
    // calc text box size, add 5% to make it fit safely

    FontMetric aFontMetric(mpVD->GetFontMetric());
    vcl::Font aFont(mpVD->GetFont());
    FontAlign eAlignment(aFont.GetAlignment());

    // sal_Int32 nTextWidth = static_cast<sal_Int32>(mpVD->GetTextWidth(rStr) * mfScaleX);
    sal_Int32 nTextHeight = static_cast<sal_Int32>(mpVD->GetTextHeight() * mfScaleY);

    Point aPosition(FRound(rPos.X() * mfScaleX + maOfs.X()),
                    FRound(rPos.Y() * mfScaleY + maOfs.Y()));
    Size aSize(FRound(rSize.Width() * mfScaleX), FRound(rSize.Height() * mfScaleY));

    if (eAlignment == ALIGN_BASELINE)
        aPosition.AdjustY(-FRound(aFontMetric.GetAscent() * mfScaleY));
    else if (eAlignment == ALIGN_BOTTOM)
        aPosition.AdjustY(-nTextHeight);

    tools::Rectangle aTextRect(aPosition, aSize);
    SdrRectObj* pText = new SdrRectObj(*mpModel, OBJ_TEXT, aTextRect);

    pText->SetMergedItem(makeSdrTextUpperDistItem(0));
    pText->SetMergedItem(makeSdrTextLowerDistItem(0));
    pText->SetMergedItem(makeSdrTextRightDistItem(0));
    pText->SetMergedItem(makeSdrTextLeftDistItem(0));

    if (aFont.GetAverageFontWidth())
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

    if (!aFont.IsTransparent())
    {
        SfxItemSet aAttr(*mpFillAttr->GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});
        aAttr.Put(XFillStyleItem(drawing::FillStyle_SOLID));
        aAttr.Put(XFillColorItem(OUString(), aFont.GetFillColor()));
        pText->SetMergedItemSet(aAttr);
    }
    Degree100 nAngle = toDegree100(aFont.GetOrientation());
    if (nAngle)
        pText->SdrAttrObj::NbcRotate(aPosition, nAngle);
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

void ImpSdrPdfImport::ImportImage(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                                  int /*nPageObjectIndex*/)
{
    std::unique_ptr<vcl::pdf::PDFiumBitmap> bitmap = pPageObject->getImageBitmap();
    if (!bitmap)
    {
        SAL_WARN("sd.filter", "Failed to get IMAGE");
        return;
    }

    const vcl::pdf::PDFBitmapType format = bitmap->getFormat();
    if (format == vcl::pdf::PDFBitmapType::Unknown)
    {
        SAL_WARN("sd.filter", "Failed to get IMAGE format");
        return;
    }

    const unsigned char* pBuf = bitmap->getBuffer();
    const int nWidth = bitmap->getWidth();
    const int nHeight = bitmap->getHeight();
    const int nStride = bitmap->getStride();
    BitmapEx aBitmap(Size(nWidth, nHeight), 24);

    switch (format)
    {
        case vcl::pdf::PDFBitmapType::BGR:
            ReadRawDIB(aBitmap, pBuf, ScanlineFormat::N24BitTcBgr, nHeight, nStride);
            break;
        case vcl::pdf::PDFBitmapType::BGRx:
            ReadRawDIB(aBitmap, pBuf, ScanlineFormat::N32BitTcRgba, nHeight, nStride);
            break;
        case vcl::pdf::PDFBitmapType::BGRA:
            ReadRawDIB(aBitmap, pBuf, ScanlineFormat::N32BitTcBgra, nHeight, nStride);
            break;
        default:
            SAL_WARN("sd.filter", "Got IMAGE width: " << nWidth << ", height: " << nHeight
                                                      << ", stride: " << nStride
                                                      << ", format: " << static_cast<int>(format));
            break;
    }

    basegfx::B2DRectangle aBounds = pPageObject->getBounds();
    float left = aBounds.getMinX();
    // Upside down.
    float bottom = aBounds.getMinY();
    float right = aBounds.getMaxX();
    // Upside down.
    float top = aBounds.getMaxY();
    tools::Rectangle aRect = PointsToLogic(left, right, top, bottom);
    aRect.AdjustRight(1);
    aRect.AdjustBottom(1);

    SdrGrafObj* pGraf = new SdrGrafObj(*mpModel, Graphic(aBitmap), aRect);

    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
    InsertObj(pGraf);
}

void ImpSdrPdfImport::ImportPath(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                                 int /*nPageObjectIndex*/)
{
    auto aPathMatrix = pPageObject->getMatrix();

    aPathMatrix *= maCurrentMatrix;

    basegfx::B2DPolyPolygon aPolyPoly;
    basegfx::B2DPolygon aPoly;
    std::vector<basegfx::B2DPoint> aBezier;

    const int nSegments = pPageObject->getPathSegmentCount();
    for (int nSegmentIndex = 0; nSegmentIndex < nSegments; ++nSegmentIndex)
    {
        auto pPathSegment = pPageObject->getPathSegment(nSegmentIndex);
        if (pPathSegment != nullptr)
        {
            basegfx::B2DPoint aB2DPoint = pPathSegment->getPoint();
            aB2DPoint *= aPathMatrix;

            const bool bClose = pPathSegment->isClosed();
            if (bClose)
                aPoly.setClosed(bClose); // TODO: Review

            Point aPoint = PointsToLogic(aB2DPoint.getX(), aB2DPoint.getY());
            aB2DPoint.setX(aPoint.X());
            aB2DPoint.setY(aPoint.Y());

            const vcl::pdf::PDFSegmentType eSegmentType = pPathSegment->getType();
            switch (eSegmentType)
            {
                case vcl::pdf::PDFSegmentType::Lineto:
                    aPoly.append(aB2DPoint);
                    break;

                case vcl::pdf::PDFSegmentType::Bezierto:
                    aBezier.emplace_back(aB2DPoint.getX(), aB2DPoint.getY());
                    if (aBezier.size() == 3)
                    {
                        aPoly.appendBezierSegment(aBezier[0], aBezier[1], aBezier[2]);
                        aBezier.clear();
                    }
                    break;

                case vcl::pdf::PDFSegmentType::Moveto:
                    // New Poly.
                    if (aPoly.count() > 0)
                    {
                        aPolyPoly.append(aPoly, 1);
                        aPoly.clear();
                    }

                    aPoly.append(aB2DPoint);
                    break;

                case vcl::pdf::PDFSegmentType::Unknown:
                default:
                    SAL_WARN("sd.filter", "Unknown path segment type in PDF: "
                                              << static_cast<int>(eSegmentType));
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

    float fWidth = pPageObject->getStrokeWidth();
    const double dWidth = 0.5 * fabs(sqrt2(aPathMatrix.a(), aPathMatrix.c()) * fWidth);
    mnLineWidth = convertPointToMm100(dWidth);

    int nFillMode = FPDF_FILLMODE_ALTERNATE;
    FPDF_BOOL bStroke = 1; // Assume we have to draw, unless told otherwise.
    if (FPDFPath_GetDrawMode(pPageObject->getPointer(), &nFillMode, &bStroke))
    {
        if (nFillMode == FPDF_FILLMODE_ALTERNATE)
            mpVD->SetDrawMode(DrawModeFlags::Default);
        else if (nFillMode == FPDF_FILLMODE_WINDING)
            mpVD->SetDrawMode(DrawModeFlags::Default);
        else
            mpVD->SetDrawMode(DrawModeFlags::NoFill);
    }

    mpVD->SetFillColor(pPageObject->getFillColor());

    if (bStroke)
    {
        mpVD->SetLineColor(pPageObject->getStrokeColor());
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

    Point aPos(convertPointToMm100(x), convertPointToMm100(y));
    return aPos;
}

tools::Rectangle ImpSdrPdfImport::PointsToLogic(double left, double right, double top,
                                                double bottom) const
{
    top = correctVertOrigin(top);
    bottom = correctVertOrigin(bottom);

    Point aPos(convertPointToMm100(left), convertPointToMm100(top));
    Size aSize(convertPointToMm100(right - left), convertPointToMm100(bottom - top));

    return tools::Rectangle(aPos, aSize);
}

#endif // HAVE_FEATURE_PDFIUM

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
