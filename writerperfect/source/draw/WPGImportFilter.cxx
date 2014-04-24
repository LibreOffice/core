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

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <libodfgen/libodfgen.hxx>

#include "WPGImportFilter.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;

bool WPGImportFilter::doImportDocument( WPXInputStream &rInput, libwpg::WPGPaintInterface &rGenerator )
{
    SAL_INFO("writerperfect", "WPGImportFilter::doImportDocument");

    return libwpg::WPGraphics::parse(&rInput, &rGenerator);
}

bool WPGImportFilter::doDetectFormat( WPXInputStream &rInput, OUString &rTypeName )
{
    SAL_INFO("writerperfect", "WPGImportFilter::doDetectFormat");

    if (libwpg::WPGraphics::isSupported(&rInput))
    {
        rTypeName = "draw_WordPerfect_Graphics";
        return true;
    }

    return false;
}

OUString WPGImportFilter_getImplementationName ()
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "WPGImportFilter_getImplementationName");
    return OUString (  "com.sun.star.comp.Draw.WPGImportFilter"  );
}

Sequence< OUString > SAL_CALL WPGImportFilter_getSupportedServiceNames(  )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "WPGImportFilter_getSupportedServiceNames");
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL WPGImportFilter_createInstance( const Reference< XComponentContext > & rContext)
throw( Exception )
{
    SAL_INFO("writerperfect", "WPGImportFilter_createInstance");
    return (cppu::OWeakObject *) new WPGImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL WPGImportFilter::getImplementationName(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "WPGImportFilter::getImplementationName");
    return WPGImportFilter_getImplementationName();
}
sal_Bool SAL_CALL WPGImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "WPGImportFilter::supportsService");
    return cppu::supportsService( this, rServiceName );
}
Sequence< OUString > SAL_CALL WPGImportFilter::getSupportedServiceNames(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "WPGImportFilter::getSupportedServiceNames");
    return WPGImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
