/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MWAWPresentationImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/awt/XWindow.hpp>

#include <libmwaw/libmwaw.hxx>
#include <libodfgen/libodfgen.hxx>

#include "MWAWPresentationImportFilter.hxx"

static bool handleEmbeddedMWAWGraphicObject(const librevenge::RVNGBinaryData& data,
                                            OdfDocumentHandler* pHandler,
                                            const OdfStreamType streamType)
{
    OdgGenerator exporter;
    exporter.addDocumentHandler(pHandler, streamType);
    return MWAWDocument::decodeGraphic(data, &exporter);
}

static bool handleEmbeddedMWAWSpreadsheetObject(const librevenge::RVNGBinaryData& data,
                                                OdfDocumentHandler* pHandler,
                                                const OdfStreamType streamType)
{
    OdsGenerator exporter;
    exporter.registerEmbeddedObjectHandler("image/mwaw-odg", &handleEmbeddedMWAWGraphicObject);
    exporter.addDocumentHandler(pHandler, streamType);
    return MWAWDocument::decodeSpreadsheet(data, &exporter);
}

bool MWAWPresentationImportFilter::doImportDocument(weld::Window*,
                                                    librevenge::RVNGInputStream& rInput,
                                                    OdpGenerator& rGenerator, utl::MediaDescriptor&)
{
    return MWAWDocument::MWAW_R_OK == MWAWDocument::parse(&rInput, &rGenerator);
}

bool MWAWPresentationImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput,
                                                  OUString& rTypeName)
{
    rTypeName.clear();

    MWAWDocument::Type docType = MWAWDocument::MWAW_T_UNKNOWN;
    MWAWDocument::Kind docKind = MWAWDocument::MWAW_K_UNKNOWN;
    const MWAWDocument::Confidence confidence
        = MWAWDocument::isFileFormatSupported(&rInput, docType, docKind);

    if (confidence == MWAWDocument::MWAW_C_EXCELLENT)
    {
        if (docKind == MWAWDocument::MWAW_K_PRESENTATION)
        {
            switch (docType)
            {
                case MWAWDocument::MWAW_T_CLARISWORKS:
                    rTypeName = "impress_ClarisWorks";
                    break;
                case MWAWDocument::MWAW_T_RESERVED8:
                    rTypeName = "impress_PowerPoint3";
                    break;
                default:
                    rTypeName = "MWAW_Presentation";
                    break;
            }
        }
    }

    return !rTypeName.isEmpty();
}

void MWAWPresentationImportFilter::doRegisterHandlers(OdpGenerator& rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-odg", &handleEmbeddedMWAWGraphicObject);
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-ods",
                                             &handleEmbeddedMWAWSpreadsheetObject);
}

// XServiceInfo
OUString SAL_CALL MWAWPresentationImportFilter::getImplementationName()
{
    return "com.sun.star.comp.Impress.MWAWPresentationImportFilter";
}
sal_Bool SAL_CALL MWAWPresentationImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}
css::uno::Sequence<OUString> SAL_CALL MWAWPresentationImportFilter::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{ "com.sun.star.document.ImportFilter",
                                         "com.sun.star.document.ExtendedTypeDetection" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Impress_MWAWPresentationImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new MWAWPresentationImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
