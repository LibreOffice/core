/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ZMFImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libzmf/libzmf.h>

#include <cppuhelper/supportsservice.hxx>

#include "ZMFImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool ZMFImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                       OdgGenerator& rGenerator, utl::MediaDescriptor&)
{
    return libzmf::ZMFDocument::parse(&rInput, &rGenerator);
}

bool ZMFImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    if (libzmf::ZMFDocument::isSupported(&rInput))
    {
        rTypeName = "draw_ZMF_Document";
        return true;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL ZMFImportFilter::getImplementationName()
{
    return u"org.libreoffice.comp.Draw.ZMFImportFilter"_ustr;
}

sal_Bool SAL_CALL ZMFImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SAL_CALL ZMFImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_Draw_ZMFImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new ZMFImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
