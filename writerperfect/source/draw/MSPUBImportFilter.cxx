/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MSPUBImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <libmspub/libmspub.h>
#include <libodfgen/libodfgen.hxx>

#include "MSPUBImportFilter.hxx"

using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool MSPUBImportFilter::doImportDocument(librevenge::RVNGInputStream &rInput, OdgGenerator &rGenerator, utl::MediaDescriptor &)
{
    return libmspub::MSPUBDocument::parse(&rInput, &rGenerator);
}

bool MSPUBImportFilter::doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName)
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
    return OUString("com.sun.star.comp.Draw.MSPUBImportFilter");
}

sal_Bool SAL_CALL MSPUBImportFilter::supportsService(const OUString &rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL MSPUBImportFilter::getSupportedServiceNames()
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

extern "C"
SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_Draw_MSPUBImportFilter_get_implementation(
    css::uno::XComponentContext *const context,
    const css::uno::Sequence<css::uno::Any> &)
{
    return cppu::acquire(new MSPUBImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
