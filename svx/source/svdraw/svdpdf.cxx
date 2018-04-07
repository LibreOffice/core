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
#endif

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

struct FPDFBitmapDeleter
{
    inline void operator()(FPDF_BITMAP bitmap) { FPDFBitmap_Destroy(bitmap); }
};

using namespace com::sun::star;

ImpSdrPdfImport::ImpSdrPdfImport(SdrModel& rModel, SdrLayerID nLay, const Rectangle& rRect,
                                 const std::shared_ptr<uno::Sequence<sal_Int8>>& pPdfData)
    : maTmpList()
    , mpVD(VclPtr<VirtualDevice>::Create())
    , maScaleRect(rRect)
    , mpPdfData(pPdfData)
    , mnMapScalingOfs(0)
    , mpLineAttr(nullptr)
    , mpFillAttr(nullptr)
    , mpTextAttr(nullptr)
    , mpModel(&rModel)
    , mnLayer(nLay)
    , maOldLineColor()
    , mnLineWidth(0)
    , maLineJoin(basegfx::B2DLineJoin::NONE)
    , maLineCap(css::drawing::LineCap_BUTT)
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
    , mbLastObjWasLine(false)
    , maClip()
{
    mpVD->EnableOutput(false);
    mpVD->SetLineColor();
    mpVD->SetFillColor();
    maOldLineColor.SetRed(mpVD->GetLineColor().GetRed() + 1);
    mpLineAttr = o3tl::make_unique<SfxItemSet>(rModel.GetItemPool(), XATTR_LINE_FIRST, XATTR_LINE_LAST, 0, 0);
    mpFillAttr = o3tl::make_unique<SfxItemSet>(rModel.GetItemPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST, 0, 0);
    mpTextAttr = o3tl::make_unique<SfxItemSet>(rModel.GetItemPool(), EE_ITEMS_START, EE_ITEMS_END, 0, 0);
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

void ImpSdrPdfImport::DoLoopActions(SvdProgressInfo* pProgrInfo, sal_uInt32* pActionsToReport)
{
    const int nPageCount = FPDF_GetPageCount(mpPdfDocument);
    SAL_WARN("sd.filter", "Pages: " << nPageCount);
    for (size_t nPageIndex = 0; nPageIndex <= 0; ++nPageIndex)
    {
        // Render next page.
        FPDF_PAGE pPdfPage = FPDF_LoadPage(mpPdfDocument, nPageIndex);
        if (pPdfPage == nullptr)
            break;

        const double dPageWidth = FPDF_GetPageWidth(pPdfPage);
        const double dPageHeight = FPDF_GetPageWidth(pPdfPage);
        SAL_WARN("sd.filter", "Loaded page: " << nPageIndex << ", width: " << dPageWidth
                                              << ", height: " << dPageHeight);

        const int nPageObjectCount = FPDFPage_CountObject(pPdfPage);
        for (int nPageObjectIndex = 0; nPageObjectIndex < nPageObjectCount; ++nPageObjectIndex)
        {
            FPDF_PAGEOBJECT pPageObject = FPDFPage_GetObject(pPdfPage, nPageObjectIndex);
            if (pPageObject == nullptr)
                continue;

            SAL_WARN("sd.filter", "Got page object number: " << nPageObjectIndex);

            // Process everything but text, which is done separately below.
            const int nPageObjectType = FPDFPageObj_GetType(pPageObject);
            switch (nPageObjectType)
            {
                case FPDF_PAGEOBJ_TEXT:
                    SAL_WARN("sd.filter", "Got page object TEXT");
                    break;
                case FPDF_PAGEOBJ_PATH:
                    SAL_WARN("sd.filter", "Got page object PATH");
                    break;
                case FPDF_PAGEOBJ_IMAGE:
                {
                    SAL_WARN("sd.filter", "Got page object IMAGE");
                    std::unique_ptr<void, FPDFBitmapDeleter> bitmap(
                        FPDFImageObj_GetBitmap(pPageObject));
                    if (!bitmap)
                    {
                        SAL_WARN("sd.filter", "Failed to get IMAGE");
                        continue;
                    }

                    const int format = FPDFBitmap_GetFormat(bitmap.get());
                    if (format == FPDFBitmap_Unknown)
                    {
                        SAL_WARN("sd.filter", "Failed to get IMAGE format");
                        continue;
                    }

                    const unsigned char* pBuf
                        = static_cast<const unsigned char*>(FPDFBitmap_GetBuffer(bitmap.get()));
                    const int nWidth = FPDFBitmap_GetWidth(bitmap.get());
                    const int nHeight = FPDFBitmap_GetHeight(bitmap.get());
                    const int nStride = FPDFBitmap_GetStride(bitmap.get());
                    Bitmap aBitmap(Size(nWidth, nHeight), 24);

                    switch (format)
                    {
                        case FPDFBitmap_Gray:
                            SAL_WARN("sd.filter", "Got IMAGE width: " << nWidth
                                                                      << ", height: " << nHeight
                                                                      << ", stride: " << nStride
                                                                      << ", format: Gray");
                            ReadRawDIB(aBitmap, pBuf, ScanlineFormat::N8BitTcMask, nHeight,
                                       nStride);
                            break;
                        case FPDFBitmap_BGR:
                            SAL_WARN("sd.filter", "Got IMAGE width: " << nWidth
                                                                      << ", height: " << nHeight
                                                                      << ", stride: " << nStride
                                                                      << ", format: BGR");
                            ReadRawDIB(aBitmap, pBuf, ScanlineFormat::N24BitTcBgr, nHeight,
                                       nStride);
                            break;
                        case FPDFBitmap_BGRx:
                            SAL_WARN("sd.filter", "Got IMAGE width: " << nWidth
                                                                      << ", height: " << nHeight
                                                                      << ", stride: " << nStride
                                                                      << ", format: BGRx");
                            ReadRawDIB(aBitmap, pBuf, ScanlineFormat::N32BitTcBgra, nHeight,
                                       nStride);
                            break;
                        case FPDFBitmap_BGRA:
                            SAL_WARN("sd.filter", "Got IMAGE width: " << nWidth
                                                                      << ", height: " << nHeight
                                                                      << ", stride: " << nStride
                                                                      << ", format: BGRA");
                            ReadRawDIB(aBitmap, pBuf, ScanlineFormat::N32BitTcBgra, nHeight,
                                       nStride);
                            break;
                        default:
                            SAL_WARN("sd.filter", "Got IMAGE width: " << nWidth
                                                                      << ", height: " << nHeight
                                                                      << ", stride: " << nStride
                                                                      << ", format: " << format);
                            break;
                    }

                    Rectangle aRect(Point(0, 0), Size(nWidth, nHeight));
                    // aRect.AdjustRight( 1 ); aRect.AdjustBottom( 1 );
                    SdrGrafObj* pGraf = new SdrGrafObj(Graphic(aBitmap), aRect);

                    // This action is not creating line and fill, set directly, do not use SetAttributes(..)
                    pGraf->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
                    pGraf->SetMergedItem(XFillStyleItem(drawing::FillStyle_NONE));
                    InsertObj(pGraf);
                }
                break;
                case FPDF_PAGEOBJ_SHADING:
                    SAL_WARN("sd.filter", "Got page object SHADING");
                    break;
                case FPDF_PAGEOBJ_FORM:
                    SAL_WARN("sd.filter", "Got page object FORM");
                    break;
                default:
                    SAL_WARN("sd.filter", "Unknown PDF page object type: " << nPageObjectType);
                    break;
            }
        }

        // Now do the text.
        FPDF_TEXTPAGE pTextPage = FPDFText_LoadPage(pPdfPage);
        if (pTextPage != nullptr)
        {
            const int nChars = FPDFText_CountChars(pTextPage);
            SAL_WARN("sd.filter", "Got page chars: " << nChars);

            const int nRects = FPDFText_CountRects(pTextPage, 0, nChars);
            SAL_WARN("sd.filter", "Got Rects: " << nRects);

            std::unique_ptr<sal_Unicode[]> pText(new sal_Unicode[nChars]);
            for (int nRectIndex = 0; nRectIndex < nRects; ++nRectIndex)
            {
                double left = 0;
                double top = 0;
                double right = 0;
                double bottom = 0;
                FPDFText_GetRect(pTextPage, nRectIndex, &left, &top, &right, &bottom);
                if (right < left)
                    std::swap(right, left);
                if (bottom < top)
                    std::swap(bottom, top);

                SAL_WARN("sd.filter", "Got Text Rect: " << left << ", " << top << ", " << right
                                                        << ", " << bottom);
                const int nBoundedChars = FPDFText_GetBoundedText(
                    pTextPage, left, top, right, bottom,
                    reinterpret_cast<unsigned short*>(pText.get()), nChars);
                OUString sText(pText.get(), nBoundedChars);
                SAL_WARN("sd.filter", "Got Text #" << nRectIndex + 1 << " (" << nBoundedChars
                                                   << "): [" << sText << "].");

                const double dHalfCharWidth = (right - left) / nBoundedChars / 2.0;
                SAL_WARN("sd.filter", "Half Char Width: " << dHalfCharWidth);
                const int nCharIndex = FPDFText_GetCharIndexAtPos(pTextPage, left + dHalfCharWidth,
                                                                  top + dHalfCharWidth,
                                                                  dHalfCharWidth, dHalfCharWidth);
                SAL_WARN("sd.filter", "Got Char Index: " << nCharIndex);

                // FPDF_FONT pFont = FPDFText_GetFont(pTextPage, nCharIndex);
                // const int nFontAscent = FPDFFont_GetAscent(pFont);
                // const int nFontDescent = FPDFFont_GetDescent(pFont);
                // FPDF_BYTESTRING sFontName = FPDFFont_GetName(pFont);
                // SAL_WARN("sd.filter", "Char #" << nCharIndex << ", Got Font [" << sFontName <<
                //                       "], Ascent: "<< nFontAscent << ", Descent: " << nFontDescent);

                // FontMetric aFontMetric = mpVD->GetFontMetric();
                // aFontMetric.SetAscent(nFontAscent);
                // aFontMetric.SetDescent(nFontDescent);

                const double dFontSize = FPDFText_GetFontSize(pTextPage, nCharIndex) * 140;
                SAL_WARN("sd.filter", "Got Font Size: " << dFontSize);
                vcl::Font aFnt = mpVD->GetFont();
                aFnt.SetFontSize(Size(dFontSize, dFontSize));
                mpVD->SetFont(aFnt);

                double x = 0;
                double y = 0;
                FPDFText_GetCharOrigin(pTextPage, nCharIndex, &x, &y);
                SAL_WARN("sd.filter", "Got Char Origin: " << x << ", " << y);

                // geometry::RealRectangle2D aRect;
                // aRect.X1 = left;
                // aRect.Y1 = top;
                // aRect.X2 = right;
                // aRect.Y2 = bottom;

                // geometry::Matrix2D aMatrix;
                // FPDFText_GetMatrix(pTextPage, nCharIndex, &aMatrix.m00, &aMatrix.m01, &aMatrix.m10, &aMatrix.m11);

                // basegfx::B2DHomMatrix fontMatrix(
                //     aMatrix.m00, aMatrix.m01, 0.0,
                //     aMatrix.m10, aMatrix.m11, 0.0);
                // fontMatrix.scale(dFontSize, dFontSize);

                // x = fontMatrix.get(0, 0) * x + fontMatrix.get(1, 0) * y + x;
                // y = fontMatrix.get(0, 1) * x + fontMatrix.get(1, 1) * y + y;
                // SAL_WARN("sd.filter", "Char Origin after xform: " << x << ", " << y);

                // basegfx::B2DHomMatrix totalTextMatrix1(fontMatrix);
                // basegfx::B2DHomMatrix totalTextMatrix2(fontMatrix);
                // totalTextMatrix1.translate(rRect.X1, rRect.Y1);
                // totalTextMatrix2.translate(rRect.X2, rRect.Y2);

                // basegfx::B2DHomMatrix corrMatrix;
                // corrMatrix.scale(1.0, -1.0);
                // // corrMatrix.translate(0.0, ascent);
                // totalTextMatrix1 = totalTextMatrix1 * corrMatrix;
                // totalTextMatrix2 = totalTextMatrix2 * corrMatrix;

                // totalTextMatrix1 *= getCurrentContext().Transformation;
                // totalTextMatrix2 *= getCurrentContext().Transformation;

                // basegfx::B2DHomMatrix invMatrix(totalTextMatrix1);
                // basegfx::B2DHomMatrix invPrevMatrix(prevTextMatrix);
                // invMatrix.invert();
                // invPrevMatrix.invert();
                // basegfx::B2DHomMatrix offsetMatrix1(totalTextMatrix1);
                // basegfx::B2DHomMatrix offsetMatrix2(totalTextMatrix2);
                // offsetMatrix1 *= invPrevMatrix;
                // offsetMatrix2 *= invMatrix;

                // double charWidth = offsetMatrix2.get(0, 2);
                // double prevSpaceWidth = offsetMatrix1.get(0, 2) - prevCharWidth;

                ImportText(Point(x, y), sText);
            }

            FPDFText_ClosePage(pTextPage);
        }

        FPDF_ClosePage(pPdfPage);
    }

    // const sal_uLong nCount(rMtf.GetActionSize());

    for (sal_uLong a(0); a < 0; a++)
    {
        if (pProgrInfo && pActionsToReport)
        {
            (*pActionsToReport)++;

            if (*pActionsToReport >= 16) // update all 16 actions
            {
                if (!pProgrInfo->ReportActions(*pActionsToReport))
                    break;

                *pActionsToReport = 0;
            }
        }
    }
}

size_t ImpSdrPdfImport::DoImport(SdrObjList& rOL, size_t nInsPos, size_t nPageNumber,
                                 SvdProgressInfo* pProgrInfo)
{
    // setup some global scale parameter
    // mfScaleX, mfScaleY, maScaleX, maScaleY, mbMov, mbSize
    mfScaleX = mfScaleY = 10;
    /*
    mfScaleX = mfScaleY = 1.0;
    const Size aMtfSize(rMtf.GetPrefSize());

    if (aMtfSize.Width() & aMtfSize.Height() && (!maScaleRect.IsEmpty()))
    {
        maOfs = maScaleRect.TopLeft();

        if (aMtfSize.Width() != (maScaleRect.GetWidth() - 1))
        {
            mfScaleX = static_cast<double>(maScaleRect.GetWidth() - 1)
                       / static_cast<double>(aMtfSize.Width());
        }

        if (aMtfSize.Height() != (maScaleRect.GetHeight() - 1))
        {
            mfScaleY = static_cast<double>(maScaleRect.GetHeight() - 1)
                       / static_cast<double>(aMtfSize.Height());
        }
    }

    mbMov = maOfs.X() != 0 || maOfs.Y() != 0;
    mbSize = false;
    maScaleX = Fraction(1, 1);
    maScaleY = Fraction(1, 1);

    if (aMtfSize.Width() != (maScaleRect.GetWidth() - 1))
    {
        maScaleX = Fraction(maScaleRect.GetWidth() - 1, aMtfSize.Width());
        mbSize = true;
    }

    if (aMtfSize.Height() != (maScaleRect.GetHeight() - 1))
    {
        maScaleY = Fraction(maScaleRect.GetHeight() - 1, aMtfSize.Height());
        mbSize = true;
    }
    */

    if (pProgrInfo)
    {
        // pProgrInfo->SetActionCount(rMtf.GetActionSize());
    }

    sal_uInt32 nActionsToReport(0);

    // execute
    DoLoopActions(pProgrInfo, &nActionsToReport);

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
            mpLineAttr->Put(XLineStyleItem(drawing::LineStyle_SOLID));
            mpLineAttr->Put(XLineColorItem(OUString(), mpVD->GetLineColor()));
        }
        else
        {
            mpLineAttr->Put(XLineStyleItem(drawing::LineStyle_NONE));
        }

        switch (maLineJoin)
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
        SAL_WARN("sd.filter", "Font Height: " << nHeight);

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
                if (basegfx::tools::isRectangle(maClip) && aClipRange.isInside(aTextRange))
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
                        SdrObject* pNewClone = pCandidate->Clone();

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
                const basegfx::B2DPolyPolygon aNewPoly(basegfx::tools::clipPolyPolygonOnPolyPolygon(
                    aPoly, maClip, true, !aPoly.isClosed()));
                const basegfx::B2DRange aNewRange(aNewPoly.getB2DRange());

                if (!aNewRange.isEmpty())
                {
                    pObj = new SdrPathObj(aNewPoly.isClosed() ? OBJ_POLY : OBJ_PLIN, aNewPoly);

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

bool ImpSdrPdfImport::CheckLastLineMerge(const basegfx::B2DPolygon& rSrcPoly)
{
    // #i102706# Do not merge closed polygons
    if (rSrcPoly.isClosed())
    {
        return false;
    }

    // #i73407# reformulation to use new B2DPolygon classes
    if (mbLastObjWasLine && (maOldLineColor == mpVD->GetLineColor()) && rSrcPoly.count())
    {
        SdrObject* pTmpObj = maTmpList.size() ? maTmpList[maTmpList.size() - 1] : nullptr;
        SdrPathObj* pLastPoly = dynamic_cast<SdrPathObj*>(pTmpObj);

        if (pLastPoly)
        {
            if (1 == pLastPoly->GetPathPoly().count())
            {
                bool bOk(false);
                basegfx::B2DPolygon aDstPoly(pLastPoly->GetPathPoly().getB2DPolygon(0));

                // #i102706# Do not merge closed polygons
                if (aDstPoly.isClosed())
                {
                    return false;
                }

                if (aDstPoly.count())
                {
                    const sal_uInt32 nMaxDstPnt(aDstPoly.count() - 1);
                    const sal_uInt32 nMaxSrcPnt(rSrcPoly.count() - 1);

                    if (aDstPoly.getB2DPoint(nMaxDstPnt) == rSrcPoly.getB2DPoint(0))
                    {
                        aDstPoly.append(rSrcPoly, 1, rSrcPoly.count() - 1);
                        bOk = true;
                    }
                    else if (aDstPoly.getB2DPoint(0) == rSrcPoly.getB2DPoint(nMaxSrcPnt))
                    {
                        basegfx::B2DPolygon aNew(rSrcPoly);
                        aNew.append(aDstPoly, 1, aDstPoly.count() - 1);
                        aDstPoly = aNew;
                        bOk = true;
                    }
                    else if (aDstPoly.getB2DPoint(0) == rSrcPoly.getB2DPoint(0))
                    {
                        aDstPoly.flip();
                        aDstPoly.append(rSrcPoly, 1, rSrcPoly.count() - 1);
                        bOk = true;
                    }
                    else if (aDstPoly.getB2DPoint(nMaxDstPnt) == rSrcPoly.getB2DPoint(nMaxSrcPnt))
                    {
                        basegfx::B2DPolygon aNew(rSrcPoly);
                        aNew.flip();
                        aDstPoly.append(aNew, 1, aNew.count() - 1);
                        bOk = true;
                    }
                }

                if (bOk)
                {
                    pLastPoly->NbcSetPathPoly(basegfx::B2DPolyPolygon(aDstPoly));
                }

                return bOk;
            }
        }
    }

    return false;
}

bool ImpSdrPdfImport::CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    // #i73407# reformulation to use new B2DPolygon classes
    if (mbLastObjWasPolyWithoutLine)
    {
        SdrObject* pTmpObj = maTmpList.size() ? maTmpList[maTmpList.size() - 1] : nullptr;
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
            const basegfx::B2DHomMatrix aTransform(basegfx::tools::createScaleTranslateB2DHomMatrix(
                mfScaleX, mfScaleY, maOfs.X(), maOfs.Y()));

            maClip.transform(aTransform);
        }
    }
}

bool ImpSdrPdfImport::isClip() const { return !maClip.getB2DRange().isEmpty(); }

void ImpSdrPdfImport::ImportText(const Point& rPos, const OUString& rStr)
{
    // calc text box size, add 5% to make it fit safely

    FontMetric aFontMetric(mpVD->GetFontMetric());
    vcl::Font aFnt(mpVD->GetFont());
    FontAlign eAlg(aFnt.GetAlignment());

    sal_Int32 nTextWidth = static_cast<sal_Int32>(mpVD->GetTextWidth(rStr) * mfScaleX);
    sal_Int32 nTextHeight = static_cast<sal_Int32>(mpVD->GetTextHeight() * mfScaleY);
    SAL_WARN("sd.filter", "TextWidth: " << nTextWidth << ", TextHeight: " << nTextHeight);

    Point aPos(FRound(rPos.X() * mfScaleX + maOfs.X()), FRound(rPos.Y() * mfScaleY + maOfs.Y()));
    Size aSize(nTextWidth, nTextHeight);

    if (eAlg == ALIGN_BASELINE)
        aPos.Y() -= FRound(aFontMetric.GetAscent() * mfScaleY);
    else if (eAlg == ALIGN_BOTTOM)
        aPos.Y() -= nTextHeight;

    Rectangle aTextRect(aPos, aSize);
    SdrRectObj* pText = new SdrRectObj(OBJ_TEXT, aTextRect);

    pText->SetMergedItem(makeSdrTextUpperDistItem(0));
    pText->SetMergedItem(makeSdrTextLowerDistItem(0));
    pText->SetMergedItem(makeSdrTextRightDistItem(0));
    pText->SetMergedItem(makeSdrTextLeftDistItem(0));

    if (aFnt.GetAverageFontWidth())
    {
        pText->ClearMergedItem(SDRATTR_TEXT_AUTOGROWWIDTH);
        pText->SetMergedItem(makeSdrTextAutoGrowHeightItem(false));
        // don't let the margins eat the space needed for the text
        pText->SetMergedItem(SdrTextFitToSizeTypeItem(SdrFitToSizeType::AllLines));
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
        SfxItemSet aAttr(*mpFillAttr->GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST, 0, 0);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
