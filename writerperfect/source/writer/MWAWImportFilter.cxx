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

#include <cppuhelper/supportsservice.hxx>

#include <libmwaw/libmwaw.hxx>

#include "MWAWImportFilter.hxx"

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

bool MWAWImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                        OdtGenerator& rGenerator, utl::MediaDescriptor&)
{
    return MWAWDocument::MWAW_R_OK == MWAWDocument::parse(&rInput, &rGenerator);
}

bool MWAWImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    rTypeName.clear();

    MWAWDocument::Type docType = MWAWDocument::MWAW_T_UNKNOWN;
    MWAWDocument::Kind docKind = MWAWDocument::MWAW_K_UNKNOWN;
    const MWAWDocument::Confidence confidence
        = MWAWDocument::isFileFormatSupported(&rInput, docType, docKind);

    if (confidence == MWAWDocument::MWAW_C_EXCELLENT)
    {
        if (docKind == MWAWDocument::MWAW_K_TEXT)
        {
            switch (docType)
            {
                case MWAWDocument::MWAW_T_CLARISWORKS:
                    rTypeName = "writer_ClarisWorks";
                    break;
                case MWAWDocument::MWAW_T_MACWRITE:
                case MWAWDocument::MWAW_T_MACWRITEPRO:
                    rTypeName = "writer_MacWrite";
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
                case MWAWDocument::MWAW_T_WRITENOW:
                    rTypeName = "writer_WriteNow";
                    break;
                default:
                    rTypeName = "MWAW_Text_Document";
                    break;
            }
        }
    }

    return !rTypeName.isEmpty();
}

void MWAWImportFilter::doRegisterHandlers(OdtGenerator& rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-odg", &handleEmbeddedMWAWGraphicObject);
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-ods",
                                             &handleEmbeddedMWAWSpreadsheetObject);
}

// XServiceInfo
OUString SAL_CALL MWAWImportFilter::getImplementationName()
{
    return u"com.sun.star.comp.Writer.MWAWImportFilter"_ustr;
}

sal_Bool SAL_CALL MWAWImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL MWAWImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_MWAWImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new MWAWImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
