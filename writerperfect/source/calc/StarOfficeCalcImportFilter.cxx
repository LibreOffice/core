/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/supportsservice.hxx>

#include <libstaroffice/libstaroffice.hxx>
#include <libodfgen/libodfgen.hxx>

#include "StarOfficeCalcImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;

static bool handleEmbeddedSTOFFGraphicObject(const librevenge::RVNGBinaryData &data, OdfDocumentHandler *pHandler,  const OdfStreamType streamType)
{
    OdgGenerator exporter;
    exporter.addDocumentHandler(pHandler, streamType);
    return STOFFDocument::decodeGraphic(data, &exporter);
}

static bool handleEmbeddedSTOFFSpreadsheetObject(const librevenge::RVNGBinaryData &data, OdfDocumentHandler *pHandler,  const OdfStreamType streamType)
{
    OdsGenerator exporter;
    exporter.addDocumentHandler(pHandler, streamType);
    return STOFFDocument::decodeSpreadsheet(data, &exporter);
}

bool StarOfficeCalcImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdsGenerator &rGenerator, utl::MediaDescriptor &)
{
    return STOFFDocument::STOFF_R_OK == STOFFDocument::parse(&rInput, &rGenerator);
}

bool StarOfficeCalcImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    rTypeName.clear();
    STOFFDocument::Kind docKind = STOFFDocument::STOFF_K_UNKNOWN;
    const STOFFDocument::Confidence confidence = STOFFDocument::isFileFormatSupported(&rInput, docKind);
    if (confidence == STOFFDocument::STOFF_C_EXCELLENT || confidence == STOFFDocument::STOFF_C_SUPPORTED_ENCRYPTION)
    {
        switch (docKind)
        {
        case STOFFDocument::STOFF_K_DATABASE:
        case STOFFDocument::STOFF_K_SPREADSHEET:
            rTypeName = "StarOffice_Spreadsheet";
            break;
        default:
            break;
        }
    }

    return !rTypeName.isEmpty();
}

void StarOfficeCalcImportFilter::doRegisterHandlers(OdsGenerator &rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/stoff-odg", &handleEmbeddedSTOFFGraphicObject);
    rGenerator.registerEmbeddedObjectHandler("image/stoff-ods", &handleEmbeddedSTOFFSpreadsheetObject);
}

// XServiceInfo
OUString SAL_CALL StarOfficeCalcImportFilter::getImplementationName()
{
    return OUString("org.libreoffice.comp.Calc.StarOfficeCalcImportFilter");
}

sal_Bool SAL_CALL StarOfficeCalcImportFilter::supportsService(const OUString &rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL StarOfficeCalcImportFilter::getSupportedServiceNames()
{
    return Sequence< OUString > {"com.sun.star.document.ImportFilter", "com.sun.star.document.ExtendedTypeDetection"};
}

extern "C"
SAL_DLLPUBLIC_EXPORT css::uno::XInterface *SAL_CALL
org_libreoffice_comp_Calc_StarOfficeCalcImportFilter_get_implementation(
    css::uno::XComponentContext *const context,
    const css::uno::Sequence<css::uno::Any> &)
{
    return cppu::acquire(new StarOfficeCalcImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
