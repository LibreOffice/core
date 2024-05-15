/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* CDRImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include <cppuhelper/supportsservice.hxx>

#include <libcdr/libcdr.h>

#include "CDRImportFilter.hxx"

bool CDRImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                       OdgGenerator& rGenerator, utl::MediaDescriptor&)
{
    return libcdr::CDRDocument::parse(&rInput, &rGenerator);
}

bool CDRImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    if (libcdr::CDRDocument::isSupported(&rInput))
    {
        rTypeName = "draw_CorelDraw_Document";
        return true;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL CDRImportFilter::getImplementationName()
{
    return u"com.sun.star.comp.Draw.CDRImportFilter"_ustr;
}

sal_Bool SAL_CALL CDRImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL CDRImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_CDRImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new CDRImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
