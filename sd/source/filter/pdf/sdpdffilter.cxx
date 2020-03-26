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

#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <sdpdffilter.hxx>

#include <vcl/graph.hxx>
#include <vcl/pdfread.hxx>

SdPdfFilter::SdPdfFilter(SfxMedium& rMedium, sd::DrawDocShell& rDocShell)
    : SdFilter(rMedium, rDocShell)
{
}

SdPdfFilter::~SdPdfFilter() {}

bool SdPdfFilter::Import()
{
    const OUString aFileName(
        mrMedium.GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::NONE));

    // Rendering resolution.
    const double dResolutionDPI = 96.0;

    std::vector<std::pair<Graphic, Size>> aGraphics;
    if (vcl::ImportPDFUnloaded(aFileName, aGraphics, dResolutionDPI) == 0)
        return false;

    // Add as many pages as we need up-front.
    mrDocument.CreateFirstPages();
    for (size_t i = 0; i < aGraphics.size() - 1; ++i)
    {
        mrDocument.DuplicatePage(0);
    }

    for (const std::pair<Graphic, Size>& aPair : aGraphics)
    {
        const Graphic& rGraphic = aPair.first;
        const Size& aSize = aPair.second;

        const sal_Int32 nPageNumber = rGraphic.getPageNumber();
        assert(nPageNumber >= 0 && o3tl::make_unsigned(nPageNumber) < aGraphics.size());

        // Create the page and insert the Graphic.
        SdPage* pPage = mrDocument.GetSdPage(nPageNumber, PageKind::Standard);
        Size aGraphicSize(OutputDevice::LogicToLogic(aSize, rGraphic.GetPrefMapMode(),
                                                     MapMode(MapUnit::Map100thMM)));

        // Resize to original size based on 72 dpi to preserve page size.
        aGraphicSize = Size(aGraphicSize.Width() * 72.0 / dResolutionDPI,
                            aGraphicSize.Height() * 72.0 / dResolutionDPI);

        // Make the page size match the rendered image.
        pPage->SetSize(aGraphicSize);
        Point aPosition(0, 0);

        SdrGrafObj* pSdrGrafObj = new SdrGrafObj(pPage->getSdrModelFromSdrPage(), rGraphic,
                                                 tools::Rectangle(aPosition, aGraphicSize));
        pPage->InsertObject(pSdrGrafObj);
    }

    return true;
}

bool SdPdfFilter::Export() { return false; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
