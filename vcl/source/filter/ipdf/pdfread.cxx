/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pdfread.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <unotools/streamwrap.hxx>
#include <vcl/wmf.hxx>

using namespace com::sun::star;

namespace
{

/// Imports a PDF stream into Draw.
uno::Reference<lang::XComponent> importIntoDraw(SvStream& rStream)
{
    // Create an empty Draw component.
    uno::Reference<frame::XDesktop2> xDesktop = css::frame::Desktop::create(comphelper::getProcessComponentContext());
    uno::Reference<frame::XComponentLoader> xComponentLoader(xDesktop, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aLoadArguments =
    {
        comphelper::makePropertyValue("Hidden", true)
    };
    uno::Reference<lang::XComponent> xComponent = xComponentLoader->loadComponentFromURL("private:factory/sdraw", "_default", 0, aLoadArguments);

    // Import the PDF into it.
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    // Need to go via FilterFactory, otherwise XmlFilterAdaptor::initialize() is not called.
    uno::Reference<lang::XMultiServiceFactory> xFilterFactory(xMultiServiceFactory->createInstance("com.sun.star.document.FilterFactory"), uno::UNO_QUERY);
    uno::Reference<document::XFilter> xFilter(xFilterFactory->createInstanceWithArguments("draw_pdf_import", uno::Sequence<uno::Any>()), uno::UNO_QUERY);
    uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY);
    xImporter->setTargetDocument(xComponent);

    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(rStream));
    uno::Sequence<beans::PropertyValue> aImportArguments =
    {
        // XmlFilterAdaptor::importImpl() mandates URL, even if it's empty.
        comphelper::makePropertyValue("URL", OUString()),
        comphelper::makePropertyValue("InputStream", xStream),
    };

    if (xFilter->filter(aImportArguments))
        return xComponent;
    else
    {
        xComponent->dispose();
        return uno::Reference<lang::XComponent>();
    }
}

}

namespace vcl
{

bool ImportPDF(SvStream& rStream, Graphic& rGraphic)
{
    uno::Reference<lang::XComponent> xComponent = importIntoDraw(rStream);
    if (!xComponent.is())
        return false;
    comphelper::ScopeGuard aGuard([&xComponent]()
    {
        xComponent->dispose();
    });

    // Get the preview of the first page.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    if (xDrawPages->getCount() <= 0)
        return false;

    uno::Reference<beans::XPropertySet> xFirstPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    uno::Sequence<sal_Int8> aSequence;
    if (!(xFirstPage->getPropertyValue("PreviewMetafile") >>= aSequence))
        return false;

    if (!aSequence.hasElements())
        return false;

    // Convert it into a GDIMetaFile.
    SvMemoryStream aPreviewStream(aSequence.getLength());
    aPreviewStream.WriteBytes(aSequence.getArray(), aSequence.getLength());
    aPreviewStream.Seek(0);
    GDIMetaFile aMtf;
    aMtf.Read(aPreviewStream);

    rGraphic = aMtf;

    // Save the original PDF stream for later use.
    rStream.Seek(STREAM_SEEK_TO_END);
    uno::Sequence<sal_Int8> aPdfData(rStream.Tell());
    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    rStream.ReadBytes(aPdfData.getArray(), aPdfData.getLength());
    rGraphic.setPdfData(aPdfData);

    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
