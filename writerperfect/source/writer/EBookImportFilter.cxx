/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* EBookImportFilter: Sets up the filter, and calls DocumentCollector
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

#include <libe-book/libe-book.h>

#include "EBookImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;

using libebook::EBOOKDocument;

bool EBookImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdtGenerator &rGenerator, utl::MediaDescriptor &rDescriptor)
{
    EBOOKDocument::Type type = EBOOKDocument::TYPE_UNKNOWN;

    rtl::OUString aFilterName;

    rDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] >>= aFilterName;
    assert(!aFilterName.isEmpty());

    if (aFilterName == "BroadBand eBook")
        type = EBOOKDocument::TYPE_BBEB;
    if (aFilterName == "FictionBook 2")
        type = EBOOKDocument::TYPE_FICTIONBOOK2;
    else if (aFilterName == "PalmDoc")
        type = EBOOKDocument::TYPE_PALMDOC;
    else if (aFilterName == "Plucker eBook")
        type = EBOOKDocument::TYPE_PLUCKER;
    else if (aFilterName == "eReader eBook")
        type = EBOOKDocument::TYPE_PEANUTPRESS;
    else if (aFilterName == "TealDoc")
        type = EBOOKDocument::TYPE_TEALDOC;
    else if (aFilterName == "zTXT")
        type = EBOOKDocument::TYPE_ZTXT;

    if (EBOOKDocument::TYPE_UNKNOWN != type)
        return EBOOKDocument::RESULT_OK == EBOOKDocument::parse(&rInput, &rGenerator, type);

    return false;
}

bool EBookImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
{
    rTypeName = "";

    EBOOKDocument::Type type = EBOOKDocument::TYPE_UNKNOWN;

    if (EBOOKDocument::CONFIDENCE_EXCELLENT == EBOOKDocument::isSupported(&rInput, &type))
    {
        switch (type)
        {
        case EBOOKDocument::TYPE_BBEB :
            rTypeName = "writer_BroadBand_eBook";
            break;
        case EBOOKDocument::TYPE_FICTIONBOOK2 :
            rTypeName = "writer_FictionBook_2";
            break;
        case EBOOKDocument::TYPE_PALMDOC :
            rTypeName = "writer_PalmDoc";
            break;
        case EBOOKDocument::TYPE_PLUCKER :
            rTypeName = "writer_Plucker_eBook";
            break;
        case EBOOKDocument::TYPE_PEANUTPRESS :
            rTypeName = "writer_eReader_eBook";
            break;
        case EBOOKDocument::TYPE_TEALDOC :
            rTypeName = "writer_TealDoc";
            break;
        case EBOOKDocument::TYPE_ZTXT :
            rTypeName = "writer_zTXT";
            break;
        default :
            SAL_WARN_IF(type != EBOOKDocument::TYPE_UNKNOWN, "writerperfect", "EBookImportFilter::doDetectFormat: document type " << type << " detected, but ignored");
        }
    }

    return !rTypeName.isEmpty();
}

OUString EBookImportFilter_getImplementationName()
throw (RuntimeException)
{
    return OUString("org.libreoffice.comp.Writer.EBookImportFilter");
}

Sequence< OUString > SAL_CALL EBookImportFilter_getSupportedServiceNames()
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

Reference< XInterface > SAL_CALL EBookImportFilter_createInstance(const Reference< XComponentContext > &rContext)
throw(Exception)
{
    return (cppu::OWeakObject *) new EBookImportFilter(rContext);
}

// XServiceInfo
OUString SAL_CALL EBookImportFilter::getImplementationName()
throw (RuntimeException, std::exception)
{
    return EBookImportFilter_getImplementationName();
}
sal_Bool SAL_CALL EBookImportFilter::supportsService(const OUString &rServiceName)
throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence< OUString > SAL_CALL EBookImportFilter::getSupportedServiceNames()
throw (RuntimeException, std::exception)
{
    return EBookImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
