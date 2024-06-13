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

#include <sal/config.h>

#include <sfx2/docfile.hxx>
#include <svx/svdograf.hxx>
#include <o3tl/safeint.hxx>

#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <sdpdffilter.hxx>

#include <vcl/graph.hxx>
#include <vcl/pdfread.hxx>

#include <Annotation.hxx>

#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/text/XText.hpp>

#include <basegfx/polygon/b2dpolygontools.hxx>

using namespace css;

SdPdfFilter::SdPdfFilter(SfxMedium& rMedium, sd::DrawDocShell& rDocShell)
    : SdFilter(rMedium, rDocShell)
{
}

SdPdfFilter::~SdPdfFilter() {}

bool SdPdfFilter::Import()
{
    const OUString aFileName(
        mrMedium.GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::NONE));

    std::vector<vcl::PDFGraphicResult> aGraphics;
    if (vcl::ImportPDFUnloaded(aFileName, aGraphics) == 0)
        return false;

    bool bWasLocked = mrDocument.isLocked();
    mrDocument.setLock(true);
    const bool bSavedUndoEnabled = mrDocument.IsUndoEnabled();
    mrDocument.EnableUndo(false);

    // Add as many pages as we need up-front.
    mrDocument.CreateFirstPages();
    for (size_t i = 0; i < aGraphics.size() - 1; ++i)
    {
        mrDocument.DuplicatePage(0);
    }

    for (vcl::PDFGraphicResult const& rPDFGraphicResult : aGraphics)
    {
        const Graphic& rGraphic = rPDFGraphicResult.GetGraphic();
        const Size& aSizeHMM = rPDFGraphicResult.GetSize();

        const sal_Int32 nPageNumber = rGraphic.getPageNumber();
        assert(nPageNumber >= 0 && o3tl::make_unsigned(nPageNumber) < aGraphics.size());

        // Create the page and insert the Graphic.
        SdPage* pPage = mrDocument.GetSdPage(nPageNumber, PageKind::Standard);
        if (!pPage) // failed to duplicate page, out of memory?
            return false;

        // Make the page size match the rendered image.
        pPage->SetSize(aSizeHMM);

        SdrModel& rModel = pPage->getSdrModelFromSdrPage();

        rtl::Reference<SdrGrafObj> pSdrGrafObj
            = new SdrGrafObj(rModel, rGraphic, tools::Rectangle(Point(), aSizeHMM));

        pSdrGrafObj->SetResizeProtect(true);
        pSdrGrafObj->SetMoveProtect(true);

        pPage->InsertObject(pSdrGrafObj.get());

        for (auto const& rPDFAnnotation : rPDFGraphicResult.GetAnnotations())
        {
            rtl::Reference<sdr::annotation::Annotation> xAnnotation = pPage->createAnnotation();

            xAnnotation->setAuthor(rPDFAnnotation.maAuthor);

            uno::Reference<text::XText> xText(xAnnotation->getTextRange());
            xText->setString(rPDFAnnotation.maText);
            // position is in mm not 100thmm
            geometry::RealPoint2D aUnoPosition(rPDFAnnotation.maRectangle.getMinX() / 100.0,
                                               rPDFAnnotation.maRectangle.getMinY() / 100.0);
            geometry::RealSize2D aUnoSize(rPDFAnnotation.maRectangle.getWidth() / 100.0,
                                          rPDFAnnotation.maRectangle.getHeight() / 100.0);
            xAnnotation->setPosition(aUnoPosition);
            xAnnotation->setSize(aUnoSize);
            xAnnotation->setDateTime(rPDFAnnotation.maDateTime);

            if (rPDFAnnotation.mpMarker)
            {
                sdr::annotation::CreationInfo aInfo;
                aInfo.maRectangle = rPDFAnnotation.maRectangle;

                if (rPDFAnnotation.meSubType == vcl::pdf::PDFAnnotationSubType::Polygon)
                {
                    aInfo.meType = sdr::annotation::AnnotationType::Polygon;

                    auto* pMarker = static_cast<vcl::pdf::PDFAnnotationMarkerPolygon*>(
                        rPDFAnnotation.mpMarker.get());

                    aInfo.maPolygons.push_back(pMarker->maPolygon);
                    aInfo.mnWidth = pMarker->mnWidth;
                    aInfo.maColor = rPDFAnnotation.maColor;
                    aInfo.mbColor = true;
                    aInfo.maFillColor = pMarker->maFillColor;
                    aInfo.mbFillColor = true;
                }
                else if (rPDFAnnotation.meSubType == vcl::pdf::PDFAnnotationSubType::Square)
                {
                    aInfo.meType = sdr::annotation::AnnotationType::Square;

                    auto* pMarker = static_cast<vcl::pdf::PDFAnnotationMarkerSquare*>(
                        rPDFAnnotation.mpMarker.get());

                    aInfo.mnWidth = pMarker->mnWidth;
                    aInfo.maColor = rPDFAnnotation.maColor;
                    aInfo.mbColor = true;
                    aInfo.maFillColor = pMarker->maFillColor;
                    aInfo.mbFillColor = true;
                }
                else if (rPDFAnnotation.meSubType == vcl::pdf::PDFAnnotationSubType::Circle)
                {
                    aInfo.meType = sdr::annotation::AnnotationType::Circle;

                    auto* pMarker = static_cast<vcl::pdf::PDFAnnotationMarkerCircle*>(
                        rPDFAnnotation.mpMarker.get());

                    aInfo.mnWidth = pMarker->mnWidth;
                    aInfo.maColor = rPDFAnnotation.maColor;
                    aInfo.mbColor = true;
                    aInfo.maFillColor = pMarker->maFillColor;
                    aInfo.mbFillColor = true;
                }
                else if (rPDFAnnotation.meSubType == vcl::pdf::PDFAnnotationSubType::Ink)
                {
                    aInfo.meType = sdr::annotation::AnnotationType::Ink;

                    auto* pMarker = static_cast<vcl::pdf::PDFAnnotationMarkerInk*>(
                        rPDFAnnotation.mpMarker.get());

                    aInfo.maPolygons = pMarker->maStrokes;

                    aInfo.mnWidth = pMarker->mnWidth;
                    aInfo.maColor = rPDFAnnotation.maColor;
                    aInfo.mbColor = true;
                    aInfo.maFillColor = pMarker->maFillColor;
                    aInfo.mbFillColor = true;
                }
                else if (rPDFAnnotation.meSubType == vcl::pdf::PDFAnnotationSubType::Highlight)
                {
                    aInfo.meType = sdr::annotation::AnnotationType::Highlight;

                    auto* pMarker = static_cast<vcl::pdf::PDFAnnotationMarkerHighlight*>(
                        rPDFAnnotation.mpMarker.get());

                    aInfo.maColor = rPDFAnnotation.maColor;
                    aInfo.maColor.SetAlpha(0xFF - 0x90);
                    aInfo.mbColor = true;

                    aInfo.mnWidth = 1;

                    aInfo.maFillColor = rPDFAnnotation.maColor;
                    aInfo.maFillColor.SetAlpha(0xFF - 0x90);
                    aInfo.mbFillColor = true;

                    aInfo.maPolygons = pMarker->maQuads;
                }
                else if (rPDFAnnotation.meSubType == vcl::pdf::PDFAnnotationSubType::Line)
                {
                    auto* pMarker = static_cast<vcl::pdf::PDFAnnotationMarkerLine*>(
                        rPDFAnnotation.mpMarker.get());

                    aInfo.meType = sdr::annotation::AnnotationType::Line;

                    basegfx::B2DPolygon aPolygon;
                    aPolygon.append(pMarker->maLineStart);
                    aPolygon.append(pMarker->maLineEnd);
                    aInfo.maPolygons.push_back(aPolygon);

                    aInfo.mnWidth = pMarker->mnWidth;
                    aInfo.maColor = rPDFAnnotation.maColor;
                    aInfo.mbColor = true;
                    aInfo.mbFillColor = false;
                }
                else if (rPDFAnnotation.meSubType == vcl::pdf::PDFAnnotationSubType::FreeText)
                {
                    aInfo.meType = sdr::annotation::AnnotationType::FreeText;
                }
                else if (rPDFAnnotation.meSubType == vcl::pdf::PDFAnnotationSubType::Stamp)
                {
                    auto* pMarker = static_cast<vcl::pdf::PDFAnnotationMarkerStamp*>(
                        rPDFAnnotation.mpMarker.get());

                    aInfo.meType = sdr::annotation::AnnotationType::Stamp;
                    aInfo.maBitmapEx = pMarker->maBitmapEx;
                }

                xAnnotation->setCreationInfo(aInfo);
            }

            pPage->addAnnotation(xAnnotation, -1);
        }
    }
    mrDocument.setLock(bWasLocked);
    mrDocument.EnableUndo(bSavedUndoEnabled);
    return true;
}

bool SdPdfFilter::Export() { return false; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
