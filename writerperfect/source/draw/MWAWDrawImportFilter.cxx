/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MWAWDrawImportFilter: Sets up the filter, and calls DocumentCollector
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

#include "MWAWDrawImportFilter.hxx"

using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

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

bool MWAWDrawImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                            OdgGenerator& rGenerator, utl::MediaDescriptor&)
{
    return MWAWDocument::MWAW_R_OK == MWAWDocument::parse(&rInput, &rGenerator);
}

bool MWAWDrawImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    rTypeName.clear();

    MWAWDocument::Type docType = MWAWDocument::MWAW_T_UNKNOWN;
    MWAWDocument::Kind docKind = MWAWDocument::MWAW_K_UNKNOWN;
    const MWAWDocument::Confidence confidence
        = MWAWDocument::isFileFormatSupported(&rInput, docType, docKind);

    if (confidence == MWAWDocument::MWAW_C_EXCELLENT)
    {
        switch (docKind)
        {
            case MWAWDocument::MWAW_K_DRAW:
                switch (docType)
                {
                    case MWAWDocument::MWAW_T_CLARISWORKS:
                        rTypeName = "draw_ClarisWorks";
                        break;
                    default:
                        rTypeName = "MWAW_Drawing";
                        break;
                }
                break;
            case MWAWDocument::MWAW_K_PAINT:
                switch (docType)
                {
                    case MWAWDocument::MWAW_T_CLARISWORKS:
                        rTypeName = "draw_ClarisWorks";
                        break;
                    default:
                        rTypeName = "MWAW_Bitmap";
                        break;
                }
                break;
            default:
                break;
        }
    }

    return !rTypeName.isEmpty();
}

void MWAWDrawImportFilter::doRegisterHandlers(OdgGenerator& rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-odg", &handleEmbeddedMWAWGraphicObject);
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-ods",
                                             &handleEmbeddedMWAWSpreadsheetObject);
}

// XServiceInfo
OUString SAL_CALL MWAWDrawImportFilter::getImplementationName()
{
    return "com.sun.star.comp.Draw.MWAWDrawImportFilter";
}

sal_Bool SAL_CALL MWAWDrawImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SAL_CALL MWAWDrawImportFilter::getSupportedServiceNames()
{
    Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ImportFilter";
    pArray[1] = "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_MWAWDrawImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new MWAWDrawImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
