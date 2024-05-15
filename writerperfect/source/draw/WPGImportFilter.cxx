/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* WPGImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include <cppuhelper/supportsservice.hxx>

#include <libwpg/libwpg.h>

#include "WPGImportFilter.hxx"

bool WPGImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                       OdgGenerator& rGenerator, utl::MediaDescriptor&)
{
    return libwpg::WPGraphics::parse(&rInput, &rGenerator);
}

bool WPGImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    if (libwpg::WPGraphics::isSupported(&rInput))
    {
        rTypeName = "draw_WordPerfect_Graphics";
        return true;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL WPGImportFilter::getImplementationName()
{
    return u"com.sun.star.comp.Draw.WPGImportFilter"_ustr;
}

sal_Bool SAL_CALL WPGImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL WPGImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_WPGImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new WPGImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
