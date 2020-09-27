/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MSWorksImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>

#include <libwps/libwps.h>

#include <WPFTEncodingDialog.hxx>
#include <WPFTResMgr.hxx>
#include "MSWorksImportFilter.hxx"
#include <strings.hrc>

static bool handleEmbeddedWKSObject(const librevenge::RVNGBinaryData& data,
                                    OdfDocumentHandler* pHandler, const OdfStreamType streamType)
{
    OdsGenerator exporter;
    exporter.addDocumentHandler(pHandler, streamType);
    return libwps::WPSDocument::parse(data.getDataStream(), &exporter) == libwps::WPS_OK;
}

bool MSWorksImportFilter::doImportDocument(weld::Window* pParent,
                                           librevenge::RVNGInputStream& rInput,
                                           OdtGenerator& rGenerator,
                                           utl::MediaDescriptor& mediaDescriptor)
{
    libwps::WPSKind kind = libwps::WPS_TEXT;
    libwps::WPSCreator creator;
    bool needEncoding = false;
    const libwps::WPSConfidence confidence
        = libwps::WPSDocument::isFileFormatSupported(&rInput, kind, creator, needEncoding);

    std::string fileEncoding;
    if ((kind == libwps::WPS_TEXT) && (confidence == libwps::WPS_CONFIDENCE_EXCELLENT)
        && needEncoding)
    {
        OUString encoding;
        // first check if we can find the encoding in the filter options (headless mode)
        mediaDescriptor[utl::MediaDescriptor::PROP_FILTEROPTIONS()] >>= encoding;
        if (!encoding.isEmpty()) // TODO: check if the encoding string is valid
            fileEncoding = encoding.toUtf8().getStr();
        else
        {
            OUString title;

            switch (creator)
            {
                case libwps::WPS_MSWORKS:
                    title = WpResId(STR_ENCODING_DIALOG_TITLE_MSWORKS);
                    encoding = "CP850";
                    break;
                case libwps::WPS_RESERVED_0: // MS Write
                    title = WpResId(STR_ENCODING_DIALOG_TITLE_MSWRITE);
                    encoding = "CP1252";
                    break;
                case libwps::WPS_RESERVED_1: // DosWord
                    title = WpResId(STR_ENCODING_DIALOG_TITLE_DOSWORD);
                    encoding = "CP850";
                    break;
                default:
                    title = WpResId(STR_ENCODING_DIALOG_TITLE);
                    encoding = "CP850";
                    break;
            }

            fileEncoding = encoding.toUtf8().getStr(); // set default to the proposed encoding
            try
            {
                writerperfect::WPFTEncodingDialog aDlg(pParent, title, encoding);
                if (aDlg.run() == RET_OK)
                {
                    if (!aDlg.GetEncoding().isEmpty())
                        fileEncoding = aDlg.GetEncoding().toUtf8().getStr();
                }
                // we can fail because we are in headless mode, the user has cancelled conversion, ...
                else if (aDlg.hasUserCalledCancel())
                    return false;
            }
            catch (css::uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("writerperfect", "ignoring");
            }
        }
    }
    return libwps::WPS_OK
           == libwps::WPSDocument::parse(&rInput, &rGenerator, "", fileEncoding.c_str());
}

bool MSWorksImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    libwps::WPSKind kind = libwps::WPS_TEXT;
    libwps::WPSCreator creator;
    bool needEncoding;
    const libwps::WPSConfidence confidence
        = libwps::WPSDocument::isFileFormatSupported(&rInput, kind, creator, needEncoding);

    if ((kind == libwps::WPS_TEXT) && (confidence == libwps::WPS_CONFIDENCE_EXCELLENT))
    {
        switch (creator)
        {
            case libwps::WPS_MSWORKS:
                rTypeName = "writer_MS_Works_Document";
                break;
            case libwps::WPS_RESERVED_0:
                rTypeName = "writer_MS_Write";
                break;
            case libwps::WPS_RESERVED_1:
                rTypeName = "writer_DosWord";
                break;
            case libwps::WPS_RESERVED_4:
                rTypeName = "writer_PocketWord_File";
                break;
            default:
                break;
        }
    }

    return !rTypeName.isEmpty();
}

void MSWorksImportFilter::doRegisterHandlers(OdtGenerator& rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/wks-ods", &handleEmbeddedWKSObject);
}

// XServiceInfo
OUString SAL_CALL MSWorksImportFilter::getImplementationName()
{
    return "com.sun.star.comp.Writer.MSWorksImportFilter";
}

sal_Bool SAL_CALL MSWorksImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL MSWorksImportFilter::getSupportedServiceNames()
{
    return { "com.sun.star.document.ImportFilter", "com.sun.star.document.ExtendedTypeDetection" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_MSWorksImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new MSWorksImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
