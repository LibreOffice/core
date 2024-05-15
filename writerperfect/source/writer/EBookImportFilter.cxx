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

#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

#include <libe-book/libe-book.h>

#include "EBookImportFilter.hxx"

using libebook::EBOOKDocument;

bool EBookImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                         OdtGenerator& rGenerator,
                                         utl::MediaDescriptor& rDescriptor)
{
    OUString aFilterName;

    rDescriptor[utl::MediaDescriptor::PROP_FILTERNAME] >>= aFilterName;
    assert(!aFilterName.isEmpty());

    if (aFilterName == "Palm_Text_Document")
    {
        return EBOOKDocument::RESULT_OK == EBOOKDocument::parse(&rInput, &rGenerator);
    }
    else
    {
        EBOOKDocument::Type type = EBOOKDocument::TYPE_UNKNOWN;

        if (aFilterName == "BroadBand eBook")
            type = EBOOKDocument::TYPE_BBEB;
        else if (aFilterName == "FictionBook 2")
            type = EBOOKDocument::TYPE_FICTIONBOOK2;
        else if (aFilterName == "PalmDoc")
            type = EBOOKDocument::TYPE_PALMDOC;
        else if (aFilterName == "Plucker eBook")
            type = EBOOKDocument::TYPE_PLUCKER;

        if (EBOOKDocument::TYPE_UNKNOWN != type)
            return EBOOKDocument::RESULT_OK == EBOOKDocument::parse(&rInput, &rGenerator, type);
    }

    return false;
}

bool EBookImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    rTypeName.clear();

    EBOOKDocument::Type type = EBOOKDocument::TYPE_UNKNOWN;

    if (EBOOKDocument::CONFIDENCE_EXCELLENT == EBOOKDocument::isSupported(&rInput, &type))
    {
        switch (type)
        {
            case EBOOKDocument::TYPE_BBEB:
                rTypeName = "writer_BroadBand_eBook";
                break;
            case EBOOKDocument::TYPE_FICTIONBOOK2:
                rTypeName = "writer_FictionBook_2";
                break;
            case EBOOKDocument::TYPE_PALMDOC:
                rTypeName = "writer_PalmDoc";
                break;
            case EBOOKDocument::TYPE_PLUCKER:
                rTypeName = "writer_Plucker_eBook";
                break;
            case EBOOKDocument::TYPE_PEANUTPRESS:
            case EBOOKDocument::TYPE_TEALDOC:
            case EBOOKDocument::TYPE_ZTXT:
                rTypeName = "Palm_Text_Document";
                break;
            default:
                SAL_WARN_IF(type != EBOOKDocument::TYPE_UNKNOWN, "writerperfect",
                            "EBookImportFilter::doDetectFormat: document type "
                                << type << " detected, but ignored");
        }
    }

    return !rTypeName.isEmpty();
}

// XServiceInfo
OUString SAL_CALL EBookImportFilter::getImplementationName()
{
    return u"org.libreoffice.comp.Writer.EBookImportFilter"_ustr;
}

sal_Bool SAL_CALL EBookImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL EBookImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_Writer_EBookImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new EBookImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
