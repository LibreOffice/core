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
#include <cppuhelper/logging.hxx>

#include <libwps/libwps.h>

#include "WPFTEncodingDialog.hxx"
#include "WPFTResMgr.hxx"
#include "MSWorksImportFilter.hxx"
#include "strings.hrc"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;

static bool handleEmbeddedWKSObject(const librevenge::RVNGBinaryData &data, OdfDocumentHandler *pHandler,  const OdfStreamType streamType)
{
    OdsGenerator exporter;
    exporter.addDocumentHandler(pHandler, streamType);
    return libwps::WPSDocument::parse(data.getDataStream(), &exporter)==libwps::WPS_OK;
}

bool MSWorksImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdtGenerator &rGenerator, utl::MediaDescriptor &)
{
    libwps::WPSKind kind = libwps::WPS_TEXT;
    libwps::WPSCreator creator;
    bool needEncoding = false;
    const libwps::WPSConfidence confidence = libwps::WPSDocument::isFileFormatSupported(&rInput, kind, creator, needEncoding);

    std::string fileEncoding;
    try
    {
        if ((kind == libwps::WPS_TEXT) && (confidence == libwps::WPS_CONFIDENCE_EXCELLENT) && needEncoding)
        {
            OUString title, encoding;

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

            const ScopedVclPtrInstance<writerperfect::WPFTEncodingDialog> pDlg(title, encoding);
            if (pDlg->Execute() == RET_OK)
            {
                if (!pDlg->GetEncoding().isEmpty())
                    fileEncoding=pDlg->GetEncoding().toUtf8().getStr();
            }
            // we can fail because we are in headless mode, the user has cancelled conversion, ...
            else if (pDlg->hasUserCalledCancel())
                return false;
        }
    }
    catch (css::uno::Exception &e)
    {
        SAL_WARN("writerperfect", "ignoring " << e);
    }
    return libwps::WPS_OK == libwps::WPSDocument::parse(&rInput, &rGenerator, "", fileEncoding.c_str());
}

bool MSWorksImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    libwps::WPSKind kind = libwps::WPS_TEXT;
    libwps::WPSCreator creator;
    bool needEncoding;
    const libwps::WPSConfidence confidence = libwps::WPSDocument::isFileFormatSupported(&rInput, kind, creator, needEncoding);

    if ((kind == libwps::WPS_TEXT) && (confidence == libwps::WPS_CONFIDENCE_EXCELLENT))
    {
        if (creator == libwps::WPS_MSWORKS)
        {
            rTypeName = "writer_MS_Works_Document";
        }
        else if (creator == libwps::WPS_RESERVED_0)
        {
            rTypeName = "writer_MS_Write";
        }
        else
        {
            rTypeName = "writer_DosWord";
        }

        return true;
    }

    return false;
}

void MSWorksImportFilter::doRegisterHandlers(OdtGenerator &rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/wks-ods", &handleEmbeddedWKSObject);
}

// XServiceInfo
OUString SAL_CALL MSWorksImportFilter::getImplementationName()
{
    return OUString("com.sun.star.comp.Writer.MSWorksImportFilter");
}

sal_Bool SAL_CALL MSWorksImportFilter::supportsService(const OUString &rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL MSWorksImportFilter::getSupportedServiceNames()
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

extern "C"
SAL_DLLPUBLIC_EXPORT css::uno::XInterface *SAL_CALL
com_sun_star_comp_Writer_MSWorksImportFilter_get_implementation(
    css::uno::XComponentContext *const context,
    const css::uno::Sequence<css::uno::Any> &)
{
    return cppu::acquire(new MSWorksImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
