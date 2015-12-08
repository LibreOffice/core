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

#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/supportsservice.hxx>

#include <libmwaw/libmwaw.hxx>
#include <libodfgen/libodfgen.hxx>

#include "MWAWPresentationImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;

static bool handleEmbeddedMWAWGraphicObject(const librevenge::RVNGBinaryData &data, OdfDocumentHandler *pHandler,  const OdfStreamType streamType)
{
    OdgGenerator exporter;
    exporter.addDocumentHandler(pHandler, streamType);
    return MWAWDocument::decodeGraphic(data, &exporter);
}

static bool handleEmbeddedMWAWSpreadsheetObject(const librevenge::RVNGBinaryData &data, OdfDocumentHandler *pHandler,  const OdfStreamType streamType)
{
    OdsGenerator exporter;
    exporter.registerEmbeddedObjectHandler("image/mwaw-odg", &handleEmbeddedMWAWGraphicObject);
    exporter.addDocumentHandler(pHandler, streamType);
    return MWAWDocument::decodeSpreadsheet(data, &exporter);
}

bool MWAWPresentationImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdpGenerator &rGenerator, utl::MediaDescriptor &)
{
    return MWAWDocument::MWAW_R_OK == MWAWDocument::parse(&rInput, &rGenerator);
}

bool MWAWPresentationImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    rTypeName.clear();

    MWAWDocument::Type docType = MWAWDocument::MWAW_T_UNKNOWN;
    MWAWDocument::Kind docKind = MWAWDocument::MWAW_K_UNKNOWN;
    const MWAWDocument::Confidence confidence = MWAWDocument::isFileFormatSupported(&rInput, docType, docKind);

    if (confidence == MWAWDocument::MWAW_C_EXCELLENT)
    {
        if (docKind == MWAWDocument::MWAW_K_PRESENTATION)
        {
            switch (docType)
            {
            case MWAWDocument::MWAW_T_CLARISWORKS:
                rTypeName = "impress_ClarisWorks";
                break;
            default:
                rTypeName = "MWAW_Presentation";
                break;
            }
        }
    }

    return !rTypeName.isEmpty();
}

void MWAWPresentationImportFilter::doRegisterHandlers(OdpGenerator &rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-odg", &handleEmbeddedMWAWGraphicObject);
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-ods", &handleEmbeddedMWAWSpreadsheetObject);
}

OUString MWAWPresentationImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("com.sun.star.comp.Impress.MWAWPresentationImportFilter");
}

Sequence< OUString > SAL_CALL MWAWPresentationImportFilter_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL MWAWPresentationImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return static_cast<cppu::OWeakObject *>(new MWAWPresentationImportFilter(rContext));
}

// XServiceInfo
OUString SAL_CALL MWAWPresentationImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return MWAWPresentationImportFilter_getImplementationName();
}
sal_Bool SAL_CALL MWAWPresentationImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL MWAWPresentationImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return MWAWPresentationImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
