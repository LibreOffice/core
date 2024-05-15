/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MSPUBImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/supportsservice.hxx>

#include <libmspub/libmspub.h>

#include "MSPUBImportFilter.hxx"

bool MSPUBImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                         OdgGenerator& rGenerator, utl::MediaDescriptor&)
{
    return libmspub::MSPUBDocument::parse(&rInput, &rGenerator);
}

bool MSPUBImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    if (libmspub::MSPUBDocument::isSupported(&rInput))
    {
        rTypeName = "draw_Publisher_Document";
        return true;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL MSPUBImportFilter::getImplementationName()
{
    return u"com.sun.star.comp.Draw.MSPUBImportFilter"_ustr;
}

sal_Bool SAL_CALL MSPUBImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL MSPUBImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_MSPUBImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new MSPUBImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
