/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* FreehandImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <libfreehand/libfreehand.h>
#include <libodfgen/libodfgen.hxx>

#include "FreehandImportFilter.hxx"

bool FreehandImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                            OdgGenerator& rGenerator, utl::MediaDescriptor&)
{
    return libfreehand::FreeHandDocument::parse(&rInput, &rGenerator);
}

bool FreehandImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    if (libfreehand::FreeHandDocument::isSupported(&rInput))
    {
        rTypeName = "draw_Freehand_Document";
        return true;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL FreehandImportFilter::getImplementationName()
{
    return "com.sun.star.comp.Draw.FreehandImportFilter";
}

sal_Bool SAL_CALL FreehandImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL FreehandImportFilter::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{ "com.sun.star.document.ImportFilter",
                                         "com.sun.star.document.ExtendedTypeDetection" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_FreehandImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new FreehandImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
