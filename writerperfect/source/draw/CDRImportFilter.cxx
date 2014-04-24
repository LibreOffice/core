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

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <libcdr/libcdr.h>
#include <libodfgen/libodfgen.hxx>

#include "CDRImportFilter.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool CDRImportFilter::doImportDocument( WPXInputStream &rInput, libwpg::WPGPaintInterface &rGenerator )
{
    SAL_INFO("writerperfect", "CDRImportFilter::doImportDocument");

    return libcdr::CDRDocument::parse(&rInput, &rGenerator);
}

bool CDRImportFilter::doDetectFormat( WPXInputStream &rInput, OUString &rTypeName )
{
    SAL_INFO("writerperfect", "CDRImportFilter::doDetectFormat");

    if (libcdr::CDRDocument::isSupported(&rInput))
    {
        rTypeName = "draw_CorelDraw_Document";
        return true;
    }

    return false;
}

OUString CDRImportFilter_getImplementationName ()
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "CDRImportFilter_getImplementationName");
    return OUString ( "com.sun.star.comp.Draw.CDRImportFilter" );
}

Sequence< OUString > SAL_CALL CDRImportFilter_getSupportedServiceNames(  )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "CDRImportFilter_getSupportedServiceNames");
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL CDRImportFilter_createInstance( const Reference< XComponentContext > & rContext)
throw( Exception )
{
    SAL_INFO("writerperfect", "CDRImportFilter_createInstance");
    return (cppu::OWeakObject *) new CDRImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL CDRImportFilter::getImplementationName(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "CDRImportFilter::getImplementationName");
    return CDRImportFilter_getImplementationName();
}
sal_Bool SAL_CALL CDRImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "CDRImportFilter::supportsService");
    return cppu::supportsService( this, rServiceName );
}
Sequence< OUString > SAL_CALL CDRImportFilter::getSupportedServiceNames(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "CDRImportFilter::getSupportedServiceNames");
    return CDRImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
