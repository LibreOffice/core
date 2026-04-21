/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <osl/process.h>
#include <sfx2/docfile.hxx>
#include <svx/svdograf.hxx>
#include <o3tl/safeint.hxx>

#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <sdpdffilter.hxx>

#include <vcl/graph.hxx>
#include <vcl/pdfread.hxx>

#include <Annotation.hxx>
#include <DrawDocShell.hxx>
#include <unomodel.hxx>

#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <tools/datetime.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace css;

SdPdfFilter::SdPdfFilter(SfxMedium& rMedium, sd::DrawDocShell& rDocShell)
    : SdFilter(rMedium, rDocShell)
{
}

SdPdfFilter::~SdPdfFilter() {}

static bool ImportPDF(SvStream& rStream, SdDrawDocument& rDocument)
{
    std::vector<vcl::PDFGraphicResult> aGraphics;
    if (vcl::ImportPDFUnloaded(rStream, aGraphics) == 0)
        return false;

    bool bWasLocked = rDocument.isLocked();
    rDocument.setLock(true);
    const bool bSavedUndoEnabled = rDocument.IsUndoEnabled();
    rDocument.EnableUndo(false);
    rDocument.setPDFDocument(true);

    SdrModel& rModel = rDocument;

    // Add as many pages as we need up-front.
    rDocument.CreateFirstPages();
    sal_uInt16 nPageToDuplicate = 0;
    for (size_t i = 0; i < aGraphics.size() - 1; ++i)
    {
        // duplicating the last page is cheaper than repeatedly duplicating the first one
        nPageToDuplicate = rDocument.DuplicatePage(nPageToDuplicate);
    }

    bool bAllPagesSameSize(false);
    Size aMasterSizeHMM;

    for (vcl::PDFGraphicResult const& rPDFGraphicResult : aGraphics)
    {
        const Graphic& rGraphic = rPDFGraphicResult.GetGraphic();
        const Size& aSizeHMM = rPDFGraphicResult.GetSize();

        const sal_Int32 nPageNumber = rGraphic.getPageNumber();
        assert(nPageNumber >= 0 && o3tl::make_unsigned(nPageNumber) < aGraphics.size());

        // Create the page and insert the Graphic.
        SdPage* pPage = rDocument.GetSdPage(nPageNumber, PageKind::Standard);
        if (!pPage) // failed to duplicate page, out of memory?
            return false;

        // Make the page size match the rendered image.
        pPage->SetSize(aSizeHMM);
        if (nPageNumber == 0)
        {
            aMasterSizeHMM = aSizeHMM;
            bAllPagesSameSize = true;
        }
        else
            bAllPagesSameSize &= aMasterSizeHMM == aSizeHMM;

        rtl::Reference<SdrGrafObj> pSdrGrafObj
            = new SdrGrafObj(rModel, rGraphic, tools::Rectangle(Point(), aSizeHMM));

        pSdrGrafObj->SetResizeProtect(true);
        pSdrGrafObj->SetMoveProtect(true);

        pPage->InsertObject(pSdrGrafObj.get());

        // Per-page bookkeeping for threaded-comment resolution. Pdfium annotation indices and
        // /IRT parent links are page-scoped, so these structures live inside the page loop; a
        // cross-page /IRT has already been dropped to -1 by findAnnotations.
        std::map<int, rtl::Reference<sdr::annotation::Annotation>> aPdfiumIndexToAnnot;
        std::vector<std::pair<rtl::Reference<sdr::annotation::Annotation>, int>> aAnnotToParent;
        // Per PDF spec, the "state" of a comment lives in a separate annotation that carries
        // /IRT + /State + /StateModel and points at the target. Acrobat models this as independent
        // per-user state: each reviewer maintains their own state (identified by the state-change
        // annotation's /T author). A target comment is shown with a resolved checkmark as long as
        // *any* reviewer's current state is "Completed"; its tooltip then lists every reviewer
        // who has marked it.
        //
        // We currently collapse all of that into a single per-comment boolean
        // (sd::Annotation::IsResolved). The lossy mapping we pick is:
        //   - each reviewer's state = the latest state-change they authored for that target
        //     (by /M);
        //   - the target is considered resolved if any reviewer's latest state is "Completed".
        // On export we will only be able to encode a single reviewer's state (the current editing
        // user), so other reviewers' per-user states that we observed on import are not
        // round-tripped.
        struct StateChange
        {
            int mnTargetPdfiumIndex;
            OUString maAuthor;
            OUString maState;
            OUString maStateModel;
            DateTime maDateTime{ DateTime::EMPTY };
        };
        std::vector<StateChange> aStateChanges;

        // We only honour the "Review" state (the "Marked" model is defined by the spec, but it's
        // unclear how to treat it correctly: Acrobat doesn't show anything for "Marked/Marked").
        // "Review/Completed" is the value that represents "resolved"; other states are not
        // currently modelled. Acrobat stamps state-change annotations with /F = Hidden | NoZoom |
        // NoRotate = 2|8|16 = 26, so they don't appear as visible replies. /State on annotations
        // that lack those flag bits isn't recognised by Acrobat as a state-change, so we match
        // its rule: don't collapse unless all three flag bits are present. (Print, value 4, is
        // often also set alongside but is not required.)
        constexpr int kStateChangeFlagMask = 2 | 8 | 16;

        for (auto const& rPDFAnnotation : rPDFGraphicResult.GetAnnotations())
        {
            const bool bHasParent = rPDFAnnotation.mnParentPdfiumIndex != -1;
            const bool bHasState
                = !rPDFAnnotation.maState.isEmpty() && !rPDFAnnotation.maStateModel.isEmpty();
            // /RT = "Group" means /IRT is a grouping relationship, not a reply. State-changes
            // and reply-parent links only apply when /RT = "R" (the spec's implicit default).
            const bool bIsReplyRelation = rPDFAnnotation.maReplyType == u"R";

            // /IRT + /State + /StateModel on a reply-relation is structurally a state-change
            // annotation. If the flag bits also match Acrobat's gate, collapse it into the
            // target's resolved state. If the flags don't match, Acrobat treats the combination
            // as malformed and skips the annotation entirely — we do the same.
            if (bHasParent && bHasState && bIsReplyRelation)
            {
                // Only /StateModel = "Review" contributes to our resolved boolean; state-changes
                // in the "Marked" model are valid per spec but not representable in our model
                // and are discarded.
                if ((rPDFAnnotation.mnFlags & kStateChangeFlagMask) == kStateChangeFlagMask
                    && rPDFAnnotation.maStateModel == u"Review")
                    aStateChanges.push_back({ rPDFAnnotation.mnParentPdfiumIndex,
                                              rPDFAnnotation.maAuthor, rPDFAnnotation.maState,
                                              rPDFAnnotation.maStateModel,
                                              DateTime(rPDFAnnotation.maDateTime) });
                continue;
            }

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

            // Every imported PDF comment supports reply / resolve.
            xAnnotation->SetThreaded(true);

            // /State on an annotation that isn't a recognised state-change (missing /IRT, or
            // missing the Hidden/NoZoom/NoRotate flag bits) is silently ignored here; Acrobat
            // ignores it the same way, so matching that behaviour keeps round-trips consistent.

            if (bHasParent && bIsReplyRelation)
                aAnnotToParent.emplace_back(xAnnotation, rPDFAnnotation.mnParentPdfiumIndex);

            aPdfiumIndexToAnnot[rPDFAnnotation.mnPdfiumIndex] = xAnnotation;

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
                    aInfo.maBitmap = pMarker->maBitmap;
                }

                xAnnotation->setCreationInfo(aInfo);
            }

            pPage->addAnnotation(xAnnotation, -1);
        }

        // Resolve reply parent links now that every (non-state-change) annotation on this page
        // has been created and its GetId() is known.
        for (auto const & [ xChild, nParentPdfiumIndex ] : aAnnotToParent)
        {
            auto it = aPdfiumIndexToAnnot.find(nParentPdfiumIndex);
            if (it != aPdfiumIndexToAnnot.end())
                xChild->SetParentId(it->second->GetId());
        }

        // Collapse per-user states to one boolean per target: target's latest state per reviewer
        // (by /M) -> resolved if any of those latest states is "Completed". /StateModel was
        // already constrained to "Review" at insertion, so here we only compare /State values.
        struct LatestPerUser
        {
            DateTime maDateTime{ DateTime::EMPTY };
            OUString maState;
        };
        std::map<int, std::map<OUString, LatestPerUser>> aLatestPerUser;
        for (auto const& rStateChange : aStateChanges)
        {
            auto& rSlot = aLatestPerUser[rStateChange.mnTargetPdfiumIndex][rStateChange.maAuthor];
            if (rSlot.maState.isEmpty() || rSlot.maDateTime < rStateChange.maDateTime)
                rSlot = { rStateChange.maDateTime, rStateChange.maState };
        }
        for (auto const & [ nTarget, rPerUser ] : aLatestPerUser)
        {
            auto it = aPdfiumIndexToAnnot.find(nTarget);
            if (it == aPdfiumIndexToAnnot.end())
                continue;
            const bool bAnyCompleted
                = std::any_of(rPerUser.begin(), rPerUser.end(), [](auto const& rEntry) {
                      return rEntry.second.maState == u"Completed";
                  });
            if (bAnyCompleted)
                it->second->SetResolved(true);
        }

        pPage->setLinkAnnotations(rPDFGraphicResult.GetLinksInfo());
    }

    if (SdPage* pPage = bAllPagesSameSize ? rDocument.GetSdPage(0, PageKind::Standard) : nullptr)
    {
        SdPage& rMasterPage = static_cast<SdPage&>(pPage->TRG_GetMasterPage());
        rMasterPage.SetSize(aMasterSizeHMM);
    }

    rDocument.setLock(bWasLocked);
    rDocument.EnableUndo(bSavedUndoEnabled);
    return true;
}

bool SdPdfFilter::Import()
{
    const OUString aFileName(
        mrMedium.GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::NONE));

    std::unique_ptr<SvStream> xStream(::utl::UcbStreamHelper::CreateStream(
        aFileName, StreamMode::READ | StreamMode::SHARE_DENYNONE));

    return ImportPDF(*xStream, mrDocument);
}

bool SdPdfFilter::Export() { return false; }

extern "C" SAL_DLLPUBLIC_EXPORT bool TestFODGExportPDF(SvStream& rStream)
{
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        osl_setEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData, OUString("1").pData);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            osl_clearEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData);
    });

    const uno::Reference<uno::XComponentContext>& xContext(
        comphelper::getProcessComponentContext());
    uno::Reference<css::frame::XModel2> xModel(
        xContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.drawing.DrawingDocument"_ustr, xContext),
        uno::UNO_QUERY_THROW);

    uno::Reference<css::frame::XLoadable> xModelLoad(xModel, uno::UNO_QUERY_THROW);
    xModelLoad->initNew();

    SdXImpressDocument& rDrawDoc = dynamic_cast<SdXImpressDocument&>(*xModel);

    bool ret = ImportPDF(rStream, *rDrawDoc.GetDoc());

    if (ret)
    {
        utl::TempFileFast aTempFile;

        uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(
            comphelper::getProcessServiceFactory());
        uno::Reference<uno::XInterface> xInterface(
            xMultiServiceFactory->createInstance(u"com.sun.star.comp.Writer.XmlFilterAdaptor"_ustr),
            uno::UNO_QUERY);

        css::uno::Sequence<OUString> aUserData{ u"com.sun.star.comp.filter.OdfFlatXml"_ustr,
                                                u""_ustr,
                                                u"com.sun.star.comp.Draw.XMLOasisImporter"_ustr,
                                                u"com.sun.star.comp.Draw.XMLOasisExporter"_ustr,
                                                u""_ustr,
                                                u""_ustr,
                                                u"true"_ustr };
        uno::Sequence<beans::PropertyValue> aAdaptorArgs(comphelper::InitPropertySequence({
            { "UserData", uno::Any(aUserData) },
        }));
        css::uno::Sequence<uno::Any> aOuterArgs{ uno::Any(aAdaptorArgs) };

        uno::Reference<lang::XInitialization> xInit(xInterface, uno::UNO_QUERY_THROW);
        xInit->initialize(aOuterArgs);

        uno::Reference<document::XFilter> xFODGFilter(xInterface, uno::UNO_QUERY);
        uno::Reference<document::XExporter> xExporter(xFODGFilter, uno::UNO_QUERY);
        xExporter->setSourceDocument(xModel);

        uno::Reference<io::XOutputStream> xOutputStream(
            new utl::OStreamWrapper(*aTempFile.GetStream(StreamMode::READWRITE)));

        uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
            { { "FilterName", uno::Any(u"OpenDocument Drawing Flat XML"_ustr) },
              { "OutputStream", uno::Any(xOutputStream) },
              { "FilterOptions",
                uno::Any(
                    u"{\"DecomposePDF\":{\"type\":\"boolean\",\"value\":\"true\"}}"_ustr) } }));
        xFODGFilter->filter(aDescriptor);
    }

    css::uno::Reference<css::util::XCloseable> xClose(xModel, css::uno::UNO_QUERY);
    xClose->close(false);

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
