/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* PageMakerImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libpagemaker/libpagemaker.h>

#include <cppuhelper/supportsservice.hxx>

#include "PageMakerImportFilter.hxx"

bool PageMakerImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                             OdgGenerator& rGenerator, utl::MediaDescriptor&)
{
    return libpagemaker::PMDocument::parse(&rInput, &rGenerator);
}

bool PageMakerImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    if (libpagemaker::PMDocument::isSupported(&rInput))
    {
        rTypeName = "draw_PageMaker_Document";
        return true;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL PageMakerImportFilter::getImplementationName()
{
    return u"org.libreoffice.comp.Draw.PageMakerImportFilter"_ustr;
}

sal_Bool SAL_CALL PageMakerImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL PageMakerImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_Draw_PageMakerImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new PageMakerImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
