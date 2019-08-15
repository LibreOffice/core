/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* VisioImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <libvisio/libvisio.h>
#include <libodfgen/libodfgen.hxx>

#include "VisioImportFilter.hxx"

bool VisioImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                         OdgGenerator& rGenerator, utl::MediaDescriptor&)
{
    return libvisio::VisioDocument::parse(&rInput, &rGenerator);
}

bool VisioImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    if (libvisio::VisioDocument::isSupported(&rInput))
    {
        rTypeName = "draw_Visio_Document";
        return true;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL VisioImportFilter::getImplementationName()
{
    return "com.sun.star.comp.Draw.VisioImportFilter";
}

sal_Bool SAL_CALL VisioImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL VisioImportFilter::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{ "com.sun.star.document.ImportFilter",
                                         "com.sun.star.document.ExtendedTypeDetection" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_VisioImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new VisioImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
