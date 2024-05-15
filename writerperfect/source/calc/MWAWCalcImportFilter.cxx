/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MWAWCalcImportFilter: Sets up the filter, and calls DocumentCollector
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

#include "MWAWCalcImportFilter.hxx"

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
    exporter.addDocumentHandler(pHandler, streamType);
    return MWAWDocument::decodeSpreadsheet(data, &exporter);
}

bool MWAWCalcImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                            OdsGenerator& rGenerator, utl::MediaDescriptor&)
{
    return MWAWDocument::MWAW_R_OK == MWAWDocument::parse(&rInput, &rGenerator);
}

bool MWAWCalcImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
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
            case MWAWDocument::MWAW_K_DATABASE:
                switch (docType)
                {
                    case MWAWDocument::MWAW_T_CLARISWORKS:
                        rTypeName = "calc_ClarisWorks";
                        break;
                    case MWAWDocument::MWAW_T_MICROSOFTWORKS:
                        rTypeName = "calc_Mac_Works";
                        break;
                    default:
                        rTypeName = "MWAW_Database";
                        break;
                }
                break;
            case MWAWDocument::MWAW_K_SPREADSHEET:
                switch (docType)
                {
                    case MWAWDocument::MWAW_T_CLARISRESOLVE:
                        rTypeName = "calc_Claris_Resolve";
                        break;
                    case MWAWDocument::MWAW_T_CLARISWORKS:
                        rTypeName = "calc_ClarisWorks";
                        break;
                    case MWAWDocument::MWAW_T_MICROSOFTWORKS:
                        rTypeName = "calc_Mac_Works";
                        break;
                    default:
                        rTypeName = "MWAW_Spreadsheet";
                        break;
                }
                break;
            default:
                break;
        }
    }

    return !rTypeName.isEmpty();
}

void MWAWCalcImportFilter::doRegisterHandlers(OdsGenerator& rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-odg", &handleEmbeddedMWAWGraphicObject);
    rGenerator.registerEmbeddedObjectHandler("image/mwaw-ods",
                                             &handleEmbeddedMWAWSpreadsheetObject);
}

// XServiceInfo
OUString SAL_CALL MWAWCalcImportFilter::getImplementationName()
{
    return u"com.sun.star.comp.Calc.MWAWCalcImportFilter"_ustr;
}

sal_Bool SAL_CALL MWAWCalcImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL MWAWCalcImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Calc_MWAWCalcImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new MWAWCalcImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
