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

#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>

#include <unotools/localfilehelper.hxx>
#include <vcl/errinf.hxx>
#include <vcl/weld.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/frame.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>

#include <strings.hrc>
#include <DrawViewShell.hxx>
#include <DrawDocShell.hxx>
#include <ClientView.hxx>
#include <FrameView.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/xoutbmp.hxx>

#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <sdresid.hxx>
#include <sdpdffilter.hxx>
#include <ViewShellBase.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <DrawController.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/drawing/GraphicFilterRequest.hpp>

#include <vcl/bitmap.hxx>
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
        assert(nPageNumber >= 0 && static_cast<size_t>(nPageNumber) < aGraphics.size());

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
