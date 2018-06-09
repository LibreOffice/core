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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ucb;
using namespace com::sun::star::ui::dialogs;
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

    std::vector<std::pair<Graphic, Size>> aGraphics;
    if (vcl::ImportPDFUnloaded(aFileName, aGraphics, dResolutionDPI) == 0)
        return false;

    // Add as many pages as we need up-front.
    mrDocument.CreateFirstPages();
    for (int i = 0; i < aGraphics.size() - 1; ++i)
    {
        mrDocument.DuplicatePage(0);
    }

    for (std::pair<Graphic, Size>& aPair : aGraphics)
    {
        const Graphic& aGraphic = aPair.first;
        const Size& aSize = aPair.second;

        const sal_Int32 nPageNumber = aGraphic.getPageNumber();
        if (nPageNumber < 0 || nPageNumber >= aGraphics.size())
            continue; // Page is out of range

        // Create the page and insert the Graphic.
        SdPage* pPage = mrDocument.GetSdPage(nPageNumber, PageKind::Standard);
        Size aGrfSize(OutputDevice::LogicToLogic(aSize, aGraphic.GetPrefMapMode(),
                                                 MapMode(MapUnit::Map100thMM)));

        // Resize to original size based on 72 dpi to preserve page size.
        aGrfSize = Size(aGrfSize.Width() * 72. / dResolutionDPI,
                        aGrfSize.Height() * 72. / dResolutionDPI);

        // Make the page size match the rendered image.
        pPage->SetSize(aGrfSize);
        Point aPos(0, 0);

        pPage->InsertObject(new SdrGrafObj(aGraphic, tools::Rectangle(aPos, aGrfSize)));
    }

    return true;
}

bool SdPdfFilter::Export() { return false; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
