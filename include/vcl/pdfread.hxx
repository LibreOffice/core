/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <vcl/pdf/PDFAnnotationSubType.hxx>
#include <vcl/pdf/PDFAnnotationMarker.hxx>

namespace vcl
{
/// Fills the rBitmaps vector with rendered pages.
VCL_DLLPUBLIC size_t RenderPDFBitmaps(const void* pBuffer, int nSize,
                                      std::vector<BitmapEx>& rBitmaps, size_t nFirstPage = 0,
                                      int nPages = 1, const basegfx::B2DTuple* pSizeHint = nullptr);

/// Imports a PDF stream as a VectorGraphicData.
VCL_DLLPUBLIC bool
importPdfVectorGraphicData(SvStream& rStream,
                           std::shared_ptr<VectorGraphicData>& rVectorGraphicData);

/// Imports a PDF stream into rGraphic.
VCL_DLLPUBLIC bool ImportPDF(SvStream& rStream, Graphic& rGraphic);

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
};

class PDFGraphicResult
{
    Graphic maGraphic;
    // Size in HMM
    Size maSize;

    std::vector<PDFGraphicAnnotation> maAnnotations;

public:
    PDFGraphicResult(Graphic aGraphic, Size const& rSize,
                     std::vector<PDFGraphicAnnotation> aAnnotations)
        : maGraphic(std::move(aGraphic))
        , maSize(rSize)
        , maAnnotations(std::move(aAnnotations))
    {
    }

    const Graphic& GetGraphic() const { return maGraphic; }
    const Size& GetSize() const { return maSize; }
    const std::vector<PDFGraphicAnnotation>& GetAnnotations() const { return maAnnotations; }
};

/// Import PDF as Graphic images (1 per page), but not loaded yet.
/// Returns the number of pages read.
VCL_DLLPUBLIC size_t ImportPDFUnloaded(const OUString& rURL,
                                       std::vector<PDFGraphicResult>& rGraphics);
}

#endif // INCLUDED_VCL_SOURCE_FILTER_IPDF_PDFREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
