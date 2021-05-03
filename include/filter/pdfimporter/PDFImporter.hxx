/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/filter/PDFiumLibrary.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <vcl/BinaryDataContainer.hxx>

class PDFImporter
{
private:
    std::shared_ptr<vcl::pdf::PDFium> mpPDFium;
    std::unique_ptr<vcl::pdf::PDFiumDocument> mpPdfDocument;
    std::unique_ptr<vcl::pdf::PDFiumPage> mpPdfPage;

    void importPdfObject(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                         std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage,
                         int nPageObjectIndex,
                         drawinglayer::primitive2d::Primitive2DContainer& rRootContainer);

    void importText(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                    std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage,
                    drawinglayer::primitive2d::Primitive2DContainer& rRootContainer);

    void importPath(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                    drawinglayer::primitive2d::Primitive2DContainer& rRootContainer);

    void importImage(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                     drawinglayer::primitive2d::Primitive2DContainer& rRootContainer);

public:
    PDFImporter(BinaryDataContainer& rDataContainer);

    bool importPage(int nPageIndex, drawinglayer::primitive2d::Primitive2DContainer& rContainer);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
