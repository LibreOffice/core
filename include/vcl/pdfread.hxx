/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <vcl/graph.hxx>

namespace com::sun::star::uno
{
template <typename> class Sequence;
}
class Bitmap;

namespace vcl
{
/// Fills the rBitmaps vector with rendered pages.
VCL_DLLPUBLIC size_t RenderPDFBitmaps(const void* pBuffer, int nSize, std::vector<Bitmap>& rBitmaps,
                                      size_t nFirstPage = 0, int nPages = 1,
                                      double fResolutionDPI = 96.);

/// Imports a PDF stream into rGraphic as VectorGraphicData.
VCL_DLLPUBLIC bool ImportPDF(SvStream& rStream, Graphic& rGraphic);

struct PDFGraphicResult
{
    Graphic maGraphic;

    // Size in HMM
    Size maSize;

    PDFGraphicResult(Graphic const& rGraphic, Size const& rSize)
        : maGraphic(rGraphic)
        , maSize(rSize)
    {
    }
};

/// Import PDF as Graphic images (1 per page), but not loaded yet.
/// Returns the number of pages read.
VCL_DLLPUBLIC size_t ImportPDFUnloaded(const OUString& rURL,
                                       std::vector<PDFGraphicResult>& rGraphics);
}

#endif // INCLUDED_VCL_SOURCE_FILTER_IPDF_PDFREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
