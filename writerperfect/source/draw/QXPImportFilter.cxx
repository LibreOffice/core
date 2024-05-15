/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* QXPImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libqxp/libqxp.h>

#include <cppuhelper/supportsservice.hxx>

#include "QXPImportFilter.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool QXPImportFilter::doImportDocument(weld::Window*, librevenge::RVNGInputStream& rInput,
                                       OdgGenerator& rGenerator, utl::MediaDescriptor&)
{
    return libqxp::QXPDocument::parse(&rInput, &rGenerator) == libqxp::QXPDocument::RESULT_OK;
}

bool QXPImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput, OUString& rTypeName)
{
    if (libqxp::QXPDocument::isSupported(&rInput))
    {
        rTypeName = "draw_QXP_Document";
        return true;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL QXPImportFilter::getImplementationName()
{
    return u"org.libreoffice.comp.Draw.QXPImportFilter"_ustr;
}

sal_Bool SAL_CALL QXPImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SAL_CALL QXPImportFilter::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ImportFilter"_ustr,
             u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_Draw_QXPImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new QXPImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
