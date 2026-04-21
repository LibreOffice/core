/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_SOURCE_FILTER_IPDF_PDFREAD_HXX
#define INCLUDED_VCL_SOURCE_FILTER_IPDF_PDFREAD_HXX

#include <vector>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <tools/color.hxx>
#include <vcl/graph.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include <vcl/pdf/PDFAnnotationSubType.hxx>
#include <vcl/pdf/PDFAnnotationMarker.hxx>

namespace vcl
{
VCL_DLLPUBLIC bool
ImportPDF(SvStream& rStream, Graphic& rGraphic, sal_Int32 nPageIndex,
          const css::uno::Reference<css::task::XInteractionHandler>& xInteractionHandler,
          bool& bEncrypted);

inline bool ImportPDF(SvStream& rStream, Graphic& rGraphic)
{
    bool bEncrypted;

    return ImportPDF(rStream, rGraphic, -1, nullptr, bEncrypted);
}

    // When inserting a PDF file as an image or pasting PDF data from the clipboard, at least on a
    // Retina iMac, the resulting rendered image does not look sharp without this surprisingly large
    // extra scaling factor. Exact reasons unknown. And it isn't enough to have it be just 2 (which is
    // the actual Retina factor on my iMac). Possibly the fuzziness is related to what Pdfium uses to
    // render text.

    // Also, look at CountDPIScaleFactor() in vcl/source/window/window.cxx. The GetDPIScaleFactor() API
    // lies on macOS even more than it does on other platforms, it claims that the DPI scale factor is
    // always 1. But in fact most Macs nowadays have a HiDPI ("Retina") display. But we can't just "fix"
    // things by making GetDPIScaleFactor() always return 2 on macOS, even if that wouldn't be any more
    // wrong, because that then causes other regressions that I have no time to look into now.

#ifdef MACOSX
constexpr int PDF_INSERT_MAGIC_SCALE_FACTOR = 8;
#else
constexpr int PDF_INSERT_MAGIC_SCALE_FACTOR = 1;
#endif

struct PDFGraphicAnnotation
{
    OUString maAuthor;
    OUString maText;

    basegfx::B2DRectangle maRectangle; // In HMM
    css::util::DateTime maDateTime;

    Color maColor;

    pdf::PDFAnnotationSubType meSubType;
    std::shared_ptr<pdf::PDFAnnotationMarker> mpMarker;

    /// Raw /F value (annotation flags bitfield). 0 if absent.
    int mnFlags = 0;
    /// Pdfium index of this annotation on its page. Indices are page-scoped; PDF spec requires
    /// /IRT parents to live on the same page as the referring annotation, so a per-page map is
    /// sufficient to resolve them.
    int mnPdfiumIndex = -1;
    /// Pdfium index of the parent annotation (/IRT) on the same page; -1 if root (no /IRT) or
    /// if the PDF is malformed and /IRT pointed to a different page.
    int mnParentPdfiumIndex = -1;
    /// /RT (reply type): "R" = reply (default), "Group" = grouped with parent but not a reply.
    OUString maReplyType = u"R"_ustr;
    /// Raw /State value (e.g. "Marked", "Unmarked", "Accepted", "Completed").
    OUString maState;
    /// Raw /StateModel value ("Marked" or "Review").
    OUString maStateModel;
};

class PDFGraphicResult
{
    Graphic maGraphic;
    // Size in HMM
    Size maSize;

    std::vector<PDFGraphicAnnotation> maAnnotations;
    std::vector<std::pair<basegfx::B2DRectangle, OUString>> maLinksInfo;

public:
    PDFGraphicResult(Graphic aGraphic, Size const& rSize,
                     std::vector<PDFGraphicAnnotation> aAnnotations,
                     std::vector<std::pair<basegfx::B2DRectangle, OUString>> aLinks)
        : maGraphic(std::move(aGraphic))
        , maSize(rSize)
        , maAnnotations(std::move(aAnnotations))
        , maLinksInfo(std::move(aLinks))
    {
    }

    const Graphic& GetGraphic() const { return maGraphic; }
    const Size& GetSize() const { return maSize; }
    const std::vector<PDFGraphicAnnotation>& GetAnnotations() const { return maAnnotations; }
    const std::vector<std::pair<basegfx::B2DRectangle, OUString>>& GetLinksInfo() const
    {
        return maLinksInfo;
    }
};

/// Import PDF as Graphic images (1 per page), but not loaded yet.
/// Returns the number of pages read.
VCL_DLLPUBLIC size_t ImportPDFUnloaded(const OUString& rURL,
                                       std::vector<PDFGraphicResult>& rGraphics);
VCL_DLLPUBLIC size_t ImportPDFUnloaded(SvStream& rStream, std::vector<PDFGraphicResult>& rGraphics);
}

#endif // INCLUDED_VCL_SOURCE_FILTER_IPDF_PDFREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
