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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <libodfgen/libodfgen.hxx>

#include <libwpg/libwpg.h>

#include "WPGImportFilter.hxx"

using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

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
    return "com.sun.star.comp.Draw.WPGImportFilter";
}

sal_Bool SAL_CALL WPGImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SAL_CALL WPGImportFilter::getSupportedServiceNames()
{
    Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ImportFilter";
    pArray[1] = "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_WPGImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new WPGImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
