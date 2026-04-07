/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/BinaryDataContainer.hxx>
#include <memory>

namespace vcl::pdf
{
class PDFium;
class PDFiumDocument;
class PDFiumPage;
}

namespace drawinglayer::primitive2d
{
/** PdfPrimitive2D class

    This primitive renders PDF content using PDFium with view-dependent
    resolution. It decomposes into a BitmapPrimitive2D, rendering only
    the visible portion of the PDF page at the current discrete size.
    The cached result is invalidated when the view changes significantly
    (zoom or pan beyond the rendered area).
 */
class DRAWINGLAYER_DLLPUBLIC PdfPrimitive2D final : public BufferedDecompositionPrimitive2D
{
private:
    BinaryDataContainer maDataContainer;
    sal_Int32 mnPageIndex;
    basegfx::B2DHomMatrix maTransform;

    /// remembered discrete sizes from last decomposition for view-change detection
    double mfPreviousDiscreteSizeX;
    double mfPreviousDiscreteSizeY;
    basegfx::B2DRange maPreviousUnitVisiblePart;

    /// cached PDFium library instance
    mutable std::shared_ptr<vcl::pdf::PDFium> mpPdfium;

    /// cache the document, so we don't need to reopen on every use
    mutable std::unique_ptr<vcl::pdf::PDFiumDocument> mpPdfDocument;

    /// cache the current page, so we don't need to reopen on every use
    mutable std::unique_ptr<vcl::pdf::PDFiumPage> mpPdfPage;

    /// ensures PDFium document and page are opened, returns false on failure
    bool ensurePdfium() const;
    virtual Primitive2DReference
    create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

public:
    PdfPrimitive2D(BinaryDataContainer const& rDataContainer, sal_Int32 nPageIndex,
                   basegfx::B2DHomMatrix const& rTransform);

    const BinaryDataContainer& getDataContainer() const { return maDataContainer; }
    sal_Int32 getPageIndex() const { return mnPageIndex; }
    const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }

    /// access current cached PDFium document
    vcl::pdf::PDFiumDocument* getPdfDocument() const;

    /// access current cached PDFium page
    vcl::pdf::PDFiumPage* getPdfPage() const;

    virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

    virtual void
    get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                       const geometry::ViewInformation2D& rViewInformation) const override;

    virtual sal_uInt32 getPrimitive2DID() const override;
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
