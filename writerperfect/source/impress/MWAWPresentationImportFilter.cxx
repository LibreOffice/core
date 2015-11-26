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

            case MWAWDocument::MWAW_T_ACTA:
            case MWAWDocument::MWAW_T_BEAGLEWORKS:
            case MWAWDocument::MWAW_T_ADOBEILLUSTRATOR:
            case MWAWDocument::MWAW_T_CLARISRESOLVE:
            case MWAWDocument::MWAW_T_DBASE:
            case MWAWDocument::MWAW_T_DOCMAKER:
            case MWAWDocument::MWAW_T_EDOC:
            case MWAWDocument::MWAW_T_FAMILYTREEMAKER:
            case MWAWDocument::MWAW_T_FILEMAKER:
            case MWAWDocument::MWAW_T_FOXBASE:
            case MWAWDocument::MWAW_T_FRAMEMAKER:
            case MWAWDocument::MWAW_T_FULLIMPACT:
            case MWAWDocument::MWAW_T_FULLPAINT:
            case MWAWDocument::MWAW_T_FULLWRITE:
            case MWAWDocument::MWAW_T_GREATWORKS:
            case MWAWDocument::MWAW_T_INFOGENIE:
            case MWAWDocument::MWAW_T_KALEIDAGRAPH:
            case MWAWDocument::MWAW_T_HANMACWORDJ:
            case MWAWDocument::MWAW_T_HANMACWORDK:
            case MWAWDocument::MWAW_T_LIGHTWAYTEXT:
            case MWAWDocument::MWAW_T_MACDOC:
            case MWAWDocument::MWAW_T_MACDRAFT:
            case MWAWDocument::MWAW_T_MACDRAW:
            case MWAWDocument::MWAW_T_MACDRAWPRO:
            case MWAWDocument::MWAW_T_MACPAINT:
            case MWAWDocument::MWAW_T_MACWRITE:
            case MWAWDocument::MWAW_T_MACWRITEPRO:
            case MWAWDocument::MWAW_T_MARINERWRITE:
            case MWAWDocument::MWAW_T_MINDWRITE:
            case MWAWDocument::MWAW_T_MICROSOFTFILE:
            case MWAWDocument::MWAW_T_MICROSOFTMULTIPLAN:
            case MWAWDocument::MWAW_T_MICROSOFTWORD:
            case MWAWDocument::MWAW_T_MICROSOFTWORKS:
            case MWAWDocument::MWAW_T_MORE:
            case MWAWDocument::MWAW_T_NISUSWRITER:
            case MWAWDocument::MWAW_T_OVERVUE:
            case MWAWDocument::MWAW_T_PAGEMAKER:
            case MWAWDocument::MWAW_T_PIXELPAINT:
            case MWAWDocument::MWAW_T_RAGTIME:
            case MWAWDocument::MWAW_T_READYSETGO:
            case MWAWDocument::MWAW_T_SUPERPAINT:
            case MWAWDocument::MWAW_T_SYMPOSIUM:
            case MWAWDocument::MWAW_T_TEACHTEXT:
            case MWAWDocument::MWAW_T_TEXEDIT:
            case MWAWDocument::MWAW_T_TRAPEZE:
            case MWAWDocument::MWAW_T_WINGZ:
            case MWAWDocument::MWAW_T_WRITENOW:
            case MWAWDocument::MWAW_T_WRITERPLUS:
            case MWAWDocument::MWAW_T_XPRESS:
            case MWAWDocument::MWAW_T_ZWRITE:
            case MWAWDocument::MWAW_T_4DIMENSION:

            case MWAWDocument::MWAW_T_RESERVED1:
            case MWAWDocument::MWAW_T_RESERVED2:
            case MWAWDocument::MWAW_T_RESERVED3:
            case MWAWDocument::MWAW_T_RESERVED4:
            case MWAWDocument::MWAW_T_RESERVED5:
            case MWAWDocument::MWAW_T_RESERVED6:
            case MWAWDocument::MWAW_T_RESERVED7:
            case MWAWDocument::MWAW_T_RESERVED8:
            case MWAWDocument::MWAW_T_RESERVED9:
            case MWAWDocument::MWAW_T_UNKNOWN:
            default:
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
