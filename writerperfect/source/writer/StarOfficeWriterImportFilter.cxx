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

#include <sfx2/passwd.hxx>

#include "StarOfficeWriterImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

static bool handleEmbeddedSTOFFWriterGraphicObject(const librevenge::RVNGBinaryData& data,
                                                   OdfDocumentHandler* pHandler,
                                                   const OdfStreamType streamType)
{
    OdgGenerator exporter;
    exporter.addDocumentHandler(pHandler, streamType);
    return STOFFDocument::decodeGraphic(data, &exporter);
}

static bool handleEmbeddedSTOFFWriterSpreadsheetObject(const librevenge::RVNGBinaryData& data,
                                                       OdfDocumentHandler* pHandler,
                                                       const OdfStreamType streamType)
{
    OdsGenerator exporter;
    exporter.registerEmbeddedObjectHandler("image/stoff-odg",
                                           &handleEmbeddedSTOFFWriterGraphicObject);
    exporter.addDocumentHandler(pHandler, streamType);
    return STOFFDocument::decodeSpreadsheet(data, &exporter);
}

bool StarOfficeWriterImportFilter::doImportDocument(weld::Window* pParent,
                                                    librevenge::RVNGInputStream& rInput,
                                                    OdtGenerator& rGenerator, utl::MediaDescriptor&)
{
    STOFFDocument::Kind docKind = STOFFDocument::STOFF_K_UNKNOWN;
    const STOFFDocument::Confidence confidence
        = STOFFDocument::isFileFormatSupported(&rInput, docKind);
    OString aUtf8Passwd;
    if (confidence == STOFFDocument::STOFF_C_SUPPORTED_ENCRYPTION)
    {
        // try to ask for a password
        try
        {
            SfxPasswordDialog aPasswdDlg(pParent);
            aPasswdDlg.SetMinLen(0);
            if (!aPasswdDlg.run())
                return false;
            OUString aPasswd = aPasswdDlg.GetPassword();
            aUtf8Passwd = OUStringToOString(aPasswd, RTL_TEXTENCODING_UTF8);
        }
        catch (...)
        {
            // ok, we will probably guess it
        }
    }
    return STOFFDocument::STOFF_R_OK
           == STOFFDocument::parse(&rInput, &rGenerator,
                                   !aUtf8Passwd.isEmpty() ? aUtf8Passwd.getStr() : nullptr);
}

bool StarOfficeWriterImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput,
                                                  OUString& rTypeName)
{
    rTypeName.clear();

    STOFFDocument::Kind docKind = STOFFDocument::STOFF_K_UNKNOWN;
    const STOFFDocument::Confidence confidence
        = STOFFDocument::isFileFormatSupported(&rInput, docKind);

    if (confidence == STOFFDocument::STOFF_C_EXCELLENT
        || confidence == STOFFDocument::STOFF_C_SUPPORTED_ENCRYPTION)
    {
        switch (docKind)
        {
            case STOFFDocument::STOFF_K_TEXT:
                rTypeName = "StarOffice_Writer";
                break;
            default:
                break;
        }
    }

    return !rTypeName.isEmpty();
}

void StarOfficeWriterImportFilter::doRegisterHandlers(OdtGenerator& rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/stoff-odg",
                                             &handleEmbeddedSTOFFWriterGraphicObject);
    rGenerator.registerEmbeddedObjectHandler("image/stoff-ods",
                                             &handleEmbeddedSTOFFWriterSpreadsheetObject);
}

// XServiceInfo
OUString SAL_CALL StarOfficeWriterImportFilter::getImplementationName()
{
    return u"org.libreoffice.comp.Writer.StarOfficeWriterImportFilter"_ustr;
}

sal_Bool SAL_CALL StarOfficeWriterImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SAL_CALL StarOfficeWriterImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_Writer_StarOfficeWriterImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new StarOfficeWriterImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
