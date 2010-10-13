/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"
#include <ManifestWriter.hxx>
#include <ManifestExport.hxx>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>

#include <osl/diagnose.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::packages;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::packages::manifest;

ManifestWriter::ManifestWriter( const Reference < XMultiServiceFactory > & xNewFactory )
: xFactory ( xNewFactory )
{
}
ManifestWriter::~ManifestWriter()
{
}

// XManifestWriter methods
void SAL_CALL ManifestWriter::writeManifestSequence( const Reference< XOutputStream >& rStream, const Sequence< Sequence< PropertyValue > >& rSequence )
        throw (RuntimeException)
{
    OUString sSaxWriter ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.xml.sax.Writer" ) );
    Reference < XActiveDataSource > xSource ( xFactory->createInstance ( sSaxWriter ), UNO_QUERY );
    if (xSource.is())
    {
        xSource->setOutputStream ( rStream );
        Reference < XDocumentHandler > xHandler ( xSource, UNO_QUERY );
        if (xHandler.is())
            try {
                ManifestExport aExporter ( xHandler, rSequence);
            }
            catch( SAXException& )
            {
                throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
            }
    }
}

// Component methods
Reference < XInterface > SAL_CALL ManifestWriter_createInstance( Reference< XMultiServiceFactory > const & rServiceFactory )
{
    return *new ManifestWriter( rServiceFactory );
}

OUString ManifestWriter::static_getImplementationName()
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.comp.ManifestWriter" ) );
}

sal_Bool SAL_CALL ManifestWriter::static_supportsService(OUString const & rServiceName)
{
    return rServiceName == getSupportedServiceNames()[0];
}
Sequence < OUString > ManifestWriter::static_getSupportedServiceNames()
{
    Sequence < OUString > aNames(1);
    aNames[0] = OUString(RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.ManifestWriter" ) );
    return aNames;
}

OUString ManifestWriter::getImplementationName()
    throw (RuntimeException)
{
    return static_getImplementationName();
}

sal_Bool SAL_CALL ManifestWriter::supportsService(OUString const & rServiceName)
    throw (RuntimeException)
{
    return static_supportsService ( rServiceName );
}
Sequence < OUString > ManifestWriter::getSupportedServiceNames()
    throw (RuntimeException)
{
    return static_getSupportedServiceNames();
}
Reference < XSingleServiceFactory > ManifestWriter::createServiceFactory( Reference < XMultiServiceFactory > const & rServiceFactory )
{
    return cppu::createSingleFactory (rServiceFactory,
                                           static_getImplementationName(),
                                           ManifestWriter_createInstance,
                                           static_getSupportedServiceNames());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
