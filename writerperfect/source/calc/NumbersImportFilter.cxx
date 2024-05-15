/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* NumbersImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libetonyek/libetonyek.h>

#include <cppuhelper/supportsservice.hxx>

#include "NumbersImportFilter.hxx"

using libetonyek::EtonyekDocument;

bool NumbersImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                           OdsGenerator& rGenerator, utl::MediaDescriptor&)
{
    return EtonyekDocument::parse(&rInput, &rGenerator);
}

bool NumbersImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    EtonyekDocument::Type type = EtonyekDocument::TYPE_UNKNOWN;
    const EtonyekDocument::Confidence confidence = EtonyekDocument::isSupported(&rInput, &type);
    if ((confidence == EtonyekDocument::CONFIDENCE_EXCELLENT)
        && (type == EtonyekDocument::TYPE_NUMBERS))
    {
        rTypeName = "calc_AppleNumbers";
        return true;
    }

    return false;
}

void NumbersImportFilter::doRegisterHandlers(OdsGenerator&) {}

// XServiceInfo
OUString SAL_CALL NumbersImportFilter::getImplementationName()
{
    return u"org.libreoffice.comp.Calc.NumbersImportFilter"_ustr;
}

sal_Bool SAL_CALL NumbersImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL NumbersImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_Calc_NumbersImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new NumbersImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
