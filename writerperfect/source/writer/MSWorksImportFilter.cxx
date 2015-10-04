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

#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/supportsservice.hxx>

#include <libwps/libwps.h>

#include "WPFTEncodingDialog.hxx"
#include "MSWorksImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
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

    std::string fileEncoding("");
    try
    {
        if ((kind == libwps::WPS_TEXT) && (creator == libwps::WPS_MSWORKS) && (confidence == libwps::WPS_CONFIDENCE_EXCELLENT) && needEncoding)
        {
            const ScopedVclPtrInstance<writerperfect::WPFTEncodingDialog> pDlg(
                "Import MsWorks files(libwps)", "CP850");
            if (pDlg->Execute() == RET_OK)
            {
                if (!pDlg->GetEncoding().isEmpty())
                    fileEncoding=pDlg->GetEncoding().toUtf8().getStr();
            }
            // we can fail because we are in headless mode, the user has cancelled conversion, ...
            else if (pDlg->hasUserCalledCancel())
                return false;
        }
        else if ((kind == libwps::WPS_TEXT) && (creator == libwps::WPS_MSWRITE) && (confidence == libwps::WPS_CONFIDENCE_EXCELLENT) && needEncoding)
        {
            const ScopedVclPtrInstance<writerperfect::WPFTEncodingDialog> pDlg(
                "Import MsWrite files(libwps)", "CP1252");
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
        SAL_WARN("writerperfect", "ignoring Exception " << e.Message);
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
        else
        {
            rTypeName = "writer_MS_Write";
        }
        return true;
    }

    return false;
}

void MSWorksImportFilter::doRegisterHandlers(OdtGenerator &rGenerator)
{
    rGenerator.registerEmbeddedObjectHandler("image/wks-ods", &handleEmbeddedWKSObject);
}

OUString MSWorksImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("com.sun.star.comp.Writer.MSWorksImportFilter");
}

Sequence< OUString > SAL_CALL MSWorksImportFilter_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}
#undef SERVICE_NAME2
#undef SERVICE_NAME1

Reference< XInterface > SAL_CALL MSWorksImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return static_cast<cppu::OWeakObject *>(new MSWorksImportFilter(rContext));
}

// XServiceInfo
OUString SAL_CALL MSWorksImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return MSWorksImportFilter_getImplementationName();
}
sal_Bool SAL_CALL MSWorksImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL MSWorksImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return MSWorksImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
