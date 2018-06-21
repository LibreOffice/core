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

#include <tools/stream.hxx>
#include <vcl/graph.hxx>

namespace vcl
{

/// Fills the rBitmaps vector with rendered pages.
VCL_DLLPUBLIC size_t RenderPDFBitmaps(const void* pBuffer, int nSize, std::vector<Bitmap>& rBitmaps,
                                      const size_t nFirstPage = 0, int nPages = 1,
                                      const double fResolutionDPI = 96.);

/// Imports a PDF stream into rGraphic as a GDIMetaFile.
VCL_DLLPUBLIC bool ImportPDF(SvStream& rStream, Bitmap &rBitmap,
                             size_t nPageIndex,
                             css::uno::Sequence<sal_Int8> &rPdfFata,
                             sal_uInt64 nPos = STREAM_SEEK_TO_BEGIN,
                             sal_uInt64 nSize = STREAM_SEEK_TO_END,
                             const double fResolutionDPI = 96.);

VCL_DLLPUBLIC bool ImportPDF(SvStream& rStream, Graphic& rGraphic,
                             const double fResolutionDPI = 96.);

VCL_DLLPUBLIC size_t ImportPDF(const OUString& rURL,
                               std::vector<Bitmap>& rBitmaps,
                               css::uno::Sequence<sal_Int8>& rPdfData,
                               const double fResolutionDPI = 96.);
}

#endif // INCLUDED_VCL_SOURCE_FILTER_IPDF_PDFREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
