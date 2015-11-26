/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MWAWImportFilter: Sets up the filter, and calls DocumentCollector
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

#include "MWAWImportFilter.hxx"

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

bool MWAWImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdtGenerator &rGenerator, utl::MediaDescriptor &)
{
    return MWAWDocument::MWAW_R_OK == MWAWDocument::parse(&rInput, &rGenerator);
}

bool MWAWImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    rTypeName.clear();

    MWAWDocument::Type docType = MWAWDocument::MWAW_T_UNKNOWN;
    MWAWDocument::Kind docKind = MWAWDocument::MWAW_K_UNKNOWN;
    const MWAWDocument::Confidence confidence = MWAWDocument::isFileFormatSupported(&rInput, docType, docKind);

    if (confidence == MWAWDocument::MWAW_C_EXCELLENT)
    {
        if (docKind == MWAWDocument::MWAW_K_TEXT)
        {
            switch (docType)
            {
            case MWAWDocument::MWAW_T_ACTA:
                rTypeName = "writer_Mac_Acta";
                break;
            case MWAWDocument::MWAW_T_BEAGLEWORKS:
                rTypeName = "writer_Beagle_Works";
                break;
            case MWAWDocument::MWAW_T_CLARISWORKS:
                rTypeName = "writer_ClarisWorks";
                break;
            case MWAWDocument::MWAW_T_DOCMAKER:
                rTypeName = "writer_DocMaker";
                break;
            case MWAWDocument::MWAW_T_EDOC:
                rTypeName = "writer_eDoc_Document";
                break;
            case MWAWDocument::MWAW_T_FULLWRITE:
                rTypeName = "writer_FullWrite_Professional";
                break;
            case MWAWDocument::MWAW_T_GREATWORKS:
                rTypeName = "writer_Great_Works";
                break;
            case MWAWDocument::MWAW_T_HANMACWORDJ:
                rTypeName = "writer_HanMac_Word_J";
                break;
            case MWAWDocument::MWAW_T_HANMACWORDK:
                rTypeName = "writer_HanMac_Word_K";
                break;
            case MWAWDocument::MWAW_T_LIGHTWAYTEXT:
                rTypeName = "writer_LightWayText";
                break;
            case MWAWDocument::MWAW_T_MACDOC:
                rTypeName = "writer_MacDoc";
                break;
            case MWAWDocument::MWAW_T_MACWRITE:
                rTypeName = "writer_MacWrite";
                break;
            case MWAWDocument::MWAW_T_MACWRITEPRO:
                rTypeName = "writer_MacWritePro";
                break;
            case MWAWDocument::MWAW_T_MARINERWRITE:
                rTypeName = "writer_Mariner_Write";
                break;
            case MWAWDocument::MWAW_T_MICROSOFTWORD:
                rTypeName = "writer_Mac_Word";
                break;
            case MWAWDocument::MWAW_T_MICROSOFTWORKS:
                rTypeName = "writer_Mac_Works";
                break;
            case MWAWDocument::MWAW_T_MINDWRITE:
                rTypeName = "writer_MindWrite";
                break;
            case MWAWDocument::MWAW_T_MORE:
                rTypeName = "writer_Mac_More";
                break;
            case MWAWDocument::MWAW_T_NISUSWRITER:
                rTypeName = "writer_Nisus_Writer";
                break;
            case MWAWDocument::MWAW_T_RAGTIME:
                rTypeName = "writer_Mac_RagTime";
                break;
            case MWAWDocument::MWAW_T_TEACHTEXT:
                rTypeName = "writer_TeachText";
                break;
            case MWAWDocument::MWAW_T_TEXEDIT:
                rTypeName = "writer_TexEdit";
                break;
            case MWAWDocument::MWAW_T_WRITENOW:
                rTypeName = "writer_WriteNow";
                break;
            case MWAWDocument::MWAW_T_WRITERPLUS:
                rTypeName = "writer_WriterPlus";
                break;
            case MWAWDocument::MWAW_T_ZWRITE:
                rTypeName = "writer_ZWrite";
                break;
            case MWAWDocument::MWAW_T_RESERVED3: // also MWAWDocument::MWAW_T_STYLE
                rTypeName = "writer_MWStyle";
                break;

            case MWAWDocument::MWAW_T_ADOBEILLUSTRATOR:
            case MWAWDocument::MWAW_T_CLARISRESOLVE:
            case MWAWDocument::MWAW_T_DBASE:
            case MWAWDocument::MWAW_T_FAMILYTREEMAKER:
            case MWAWDocument::MWAW_T_FILEMAKER:
            case MWAWDocument::MWAW_T_FOXBASE:
            case MWAWDocument::MWAW_T_FULLIMPACT:
            case MWAWDocument::MWAW_T_FULLPAINT:
            case MWAWDocument::MWAW_T_FRAMEMAKER:
            case MWAWDocument::MWAW_T_INFOGENIE:
            case MWAWDocument::MWAW_T_KALEIDAGRAPH:
            case MWAWDocument::MWAW_T_MACDRAFT:
            case MWAWDocument::MWAW_T_MACDRAW:
            case MWAWDocument::MWAW_T_MACDRAWPRO:
            case MWAWDocument::MWAW_T_MACPAINT:
            case MWAWDocument::MWAW_T_MICROSOFTFILE:
            case MWAWDocument::MWAW_T_MICROSOFTMULTIPLAN:
            case MWAWDocument::MWAW_T_OVERVUE:
            case MWAWDocument::MWAW_T_PAGEMAKER:
            case MWAWDocument::MWAW_T_PIXELPAINT:
            case MWAWDocument::MWAW_T_READYSETGO:
            case MWAWDocument::MWAW_T_SUPERPAINT:
            case MWAWDocument::MWAW_T_SYMPOSIUM:
            case MWAWDocument::MWAW_T_TRAPEZE:
            case MWAWDocument::MWAW_T_WINGZ:
            case MWAWDocument::MWAW_T_XPRESS:
            case MWAWDocument::MWAW_T_4DIMENSION:

            case MWAWDocument::MWAW_T_RESERVED1:
            case MWAWDocument::MWAW_T_RESERVED2:
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

void MWAWImportFilter::doRegisterHandlers(OdtGenerator &rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-odg", &handleEmbeddedMWAWGraphicObject);
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-ods", &handleEmbeddedMWAWSpreadsheetObject);
}

OUString MWAWImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("com.sun.star.comp.Writer.MWAWImportFilter");
}

Sequence< OUString > SAL_CALL MWAWImportFilter_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL MWAWImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return static_cast<cppu::OWeakObject *>(new MWAWImportFilter(rContext));
}

// XServiceInfo
OUString SAL_CALL MWAWImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return MWAWImportFilter_getImplementationName();
}
sal_Bool SAL_CALL MWAWImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL MWAWImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return MWAWImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
