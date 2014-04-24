/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* FreehandImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <libfreehand/libfreehand.h>
#include <libodfgen/libodfgen.hxx>

#include "FreehandImportFilter.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool FreehandImportFilter::doImportDocument( WPXInputStream &rInput, libwpg::WPGPaintInterface &rGenerator )
{
    SAL_INFO("writerperfect", "FreehandImportFilter::doImportDocument");

    return libfreehand::FreeHandDocument::parse(&rInput, &rGenerator);
}

bool FreehandImportFilter::doDetectFormat( WPXInputStream &rInput, OUString &rTypeName )
{
    SAL_INFO("writerperfect", "FreehandImportFilter::doDetectFormat");

    if (libfreehand::FreeHandDocument::isSupported(&rInput))
    {
        rTypeName = "draw_Freehand_Document";
        return true;
    }

    return false;
}

OUString FreehandImportFilter_getImplementationName ()
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "FreehandImportFilter_getImplementationName");
    return OUString ( "com.sun.star.comp.Draw.FreehandImportFilter" );
}

Sequence< OUString > SAL_CALL FreehandImportFilter_getSupportedServiceNames(  )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "FreehandImportFilter_getSupportedServiceNames");
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL FreehandImportFilter_createInstance( const Reference< XComponentContext > & rContext)
throw( Exception )
{
    SAL_INFO("writerperfect", "FreehandImportFilter_createInstance");
    return (cppu::OWeakObject *) new FreehandImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL FreehandImportFilter::getImplementationName(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "FreehandImportFilter::getImplementationName");
    return FreehandImportFilter_getImplementationName();
}
sal_Bool SAL_CALL FreehandImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "FreehandImportFilter::supportsService");
    return cppu::supportsService( this, rServiceName );
}
Sequence< OUString > SAL_CALL FreehandImportFilter::getSupportedServiceNames(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "FreehandImportFilter::getSupportedServiceNames");
    return FreehandImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
