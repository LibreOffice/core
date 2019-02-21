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

#include <sfx2/docfile.hxx>
#include <svx/svdograf.hxx>

#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <sdpdffilter.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/graph.hxx>
#include <vcl/pdfread.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ucb;
using namespace ::sfx2;

SdPdfFilter::SdPdfFilter(SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell)
    : SdFilter(rMedium, rDocShell)
{
}

SdPdfFilter::~SdPdfFilter() {}

bool SdPdfFilter::Import()
{
    //FIXME: Replace with parsing the PDF elements to allow editing.
    //FIXME: For now we import as images for simplicity.

    const OUString aFileName(
        mrMedium.GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::NONE));

    // Rendering resolution.
    const double dResolutionDPI = 96.;

    uno::Sequence<sal_Int8> aPdfData;
    std::vector<Bitmap> aBitmaps;
    if (vcl::ImportPDF(aFileName, aBitmaps, aPdfData, dResolutionDPI) == 0)
        return false;

    // Prepare the link with the PDF stream.
    const size_t nGraphicContentSize = aPdfData.getLength();
    std::unique_ptr<sal_uInt8[]> pGraphicContent(new sal_uInt8[nGraphicContentSize]);
    memcpy(pGraphicContent.get(), aPdfData.get(), nGraphicContentSize);
    std::shared_ptr<GfxLink> pGfxLink(std::make_shared<GfxLink>(
        std::move(pGraphicContent), nGraphicContentSize, GfxLinkType::NativePdf));
    auto pPdfData = std::make_shared<uno::Sequence<sal_Int8>>(aPdfData);

    mrDocument.CreateFirstPages();
    for (size_t i = 0; i < aBitmaps.size() - 1; ++i)
    {
        mrDocument.DuplicatePage(0);
    }

    size_t nPageNumber = 0;
    for (Bitmap& aBitmap : aBitmaps)
    {
        // Create the Graphic and link the original PDF stream.
        Graphic aGraphic(aBitmap);
        aGraphic.setPdfData(pPdfData);
        aGraphic.setPageNumber(nPageNumber);
        aGraphic.SetGfxLink(pGfxLink);

        // Create the page and insert the Graphic.
        SdPage* pPage = mrDocument.GetSdPage(nPageNumber++, PageKind::Standard);
        Size aGrfSize(OutputDevice::LogicToLogic(aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode(),
                                                 MapMode(MapUnit::Map100thMM)));

        // Resize to original size based on 72 dpi to preserve page size.
        aGrfSize = Size(aGrfSize.Width() * 72. / dResolutionDPI,
                        aGrfSize.Height() * 72. / dResolutionDPI);

        // Make the page size match the rendered image.
        pPage->SetSize(aGrfSize);
        Point aPos(0, 0);

        pPage->InsertObject(new SdrGrafObj(pPage->getSdrModelFromSdrPage(), aGraphic,
                                           tools::Rectangle(aPos, aGrfSize)));
    }

    return true;
}

bool SdPdfFilter::Export() { return false; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
