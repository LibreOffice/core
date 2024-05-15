/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* CMXImportFilter: Sets up the filter, and calls OdgExporter
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

#include "CMXImportFilter.hxx"

bool CMXImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                       OdgGenerator& rGenerator, utl::MediaDescriptor&)
{
    return libcdr::CMXDocument::parse(&rInput, &rGenerator);
}

bool CMXImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    if (libcdr::CMXDocument::isSupported(&rInput))
    {
        rTypeName = "draw_Corel_Presentation_Exchange";
        return true;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL CMXImportFilter::getImplementationName()
{
    return u"com.sun.star.comp.Draw.CMXImportFilter"_ustr;
}

sal_Bool SAL_CALL CMXImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL CMXImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_CMXImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new CMXImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
