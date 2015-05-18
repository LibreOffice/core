/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MSWorksCalcImportFilter: Sets up the filter, and calls DocumentCollector
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

#include "writerperfect/WPFTEncodingDialog.hxx"
#include "MSWorksCalcImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;

bool MSWorksCalcImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdsGenerator &rGenerator, utl::MediaDescriptor &)
{
    libwps::WPSKind kind = libwps::WPS_TEXT;
    libwps::WPSCreator creator;
    bool needEncoding;
    const libwps::WPSConfidence confidence = libwps::WPSDocument::isFileFormatSupported(&rInput, kind, creator, needEncoding);

    std::string fileEncoding("");
    if ((kind == libwps::WPS_SPREADSHEET || kind == libwps::WPS_DATABASE) && (confidence == libwps::WPS_CONFIDENCE_EXCELLENT) && needEncoding)
    {
        OUString title, encoding;
        if (creator == libwps::WPS_MSWORKS)
        {
            title="Import MsWorks files(libwps)";
            encoding="CP850";
        }
        else if (creator == libwps::WPS_LOTUS)
        {
            title="Import Lotus files(libwps)";
            encoding="CP437";
        }
        else if (creator == libwps::WPS_SYMPHONY)
        {
            title="Import Symphony files(libwps)";
            encoding="CP437";
        }
        else
        {
            title="Import Quattro Pro files(libwps)";
            encoding="CP437";
        }
        try
        {
            const ScopedVclPtrInstance<writerperfect::WPFTEncodingDialog> pDlg(nullptr, &title, &encoding);
            if (pDlg->Execute() == RET_OK)
            {
                if (!pDlg->GetEncoding().isEmpty())
                    fileEncoding=pDlg->GetEncoding().toUtf8().getStr();
            }
            // we can fail because we are in headless mode, the user has cancelled conversion, ...
            else if (pDlg->hasUserCalledCancel())
                return false;
        }
        catch (...)
        {
        }
    }
    return libwps::WPS_OK == libwps::WPSDocument::parse(&rInput, &rGenerator, "", fileEncoding.c_str());
}

bool MSWorksCalcImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    libwps::WPSKind kind = libwps::WPS_TEXT;
    libwps::WPSCreator creator;
    bool needEncoding;
    const libwps::WPSConfidence confidence = libwps::WPSDocument::isFileFormatSupported(&rInput, kind, creator, needEncoding);

    if ((kind == libwps::WPS_SPREADSHEET || kind == libwps::WPS_DATABASE) && confidence == libwps::WPS_CONFIDENCE_EXCELLENT)
    {
        if (creator == libwps::WPS_MSWORKS)
        {
            rTypeName = "calc_MS_Works_Document";
            return true;
        }
        if (creator == libwps::WPS_LOTUS || creator == libwps::WPS_SYMPHONY)
        {
            rTypeName = "calc_WPS_Lotus_Document";
            return true;
        }
        if (creator == libwps::WPS_QUATTRO_PRO)
        {
            rTypeName = "calc_WPS_QPro_Document";
            return true;
        }
    }

    return false;
}

void MSWorksCalcImportFilter::doRegisterHandlers(OdsGenerator &)
{
}

OUString MSWorksCalcImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("com.sun.star.comp.Calc.MSWorksCalcImportFilter");
}

Sequence< OUString > SAL_CALL MSWorksCalcImportFilter_getSupportedServiceNames()
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

Reference< XInterface > SAL_CALL MSWorksCalcImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return (cppu::OWeakObject *) new MSWorksCalcImportFilter(rContext);
}

// XServiceInfo
OUString SAL_CALL MSWorksCalcImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return MSWorksCalcImportFilter_getImplementationName();
}
sal_Bool SAL_CALL MSWorksCalcImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL MSWorksCalcImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return MSWorksCalcImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
