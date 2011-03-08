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
#include <ManifestReader.hxx>
#include <ManifestImport.hxx>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <vector>

using namespace ::std;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::packages;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::packages::manifest;
using ::rtl::OUString;

ManifestReader::ManifestReader( const Reference < XMultiServiceFactory > & xNewFactory )
: xFactory ( xNewFactory )
{
}
ManifestReader::~ManifestReader()
{
}
Sequence< Sequence< PropertyValue > > SAL_CALL ManifestReader::readManifestSequence( const Reference< XInputStream >& rStream )
    throw (::com::sun::star::uno::RuntimeException)
{
    Sequence < Sequence < PropertyValue > > aManifestSequence;
    Reference < XParser > xParser (xFactory->createInstance ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.xml.sax.Parser" ) ) ), UNO_QUERY );
    if (xParser.is())
    {
        try
        {
            vector < Sequence < PropertyValue > > aManVector;
            Reference < XDocumentHandler > xFilter = new ManifestImport( aManVector );
            InputSource aParserInput;
            aParserInput.aInputStream = rStream;
            aParserInput.sSystemId = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "META-INF/manifest.xml" ) );
            xParser->setDocumentHandler ( xFilter );
            xParser->parseStream( aParserInput );
            aManifestSequence.realloc ( aManVector.size() );
            Sequence < PropertyValue > * pSequence = aManifestSequence.getArray();
            ::std::vector < Sequence < PropertyValue > >::const_iterator aIter = aManVector.begin();
            ::std::vector < Sequence < PropertyValue > >::const_iterator aEnd = aManVector.end();
            while( aIter != aEnd )
                *pSequence++ = (*aIter++);
        }
        catch (SAXParseException& )
        {
        }
        catch (SAXException& )
        {
        }
        catch (IOException& )
        {
        }
    }
    xParser->setDocumentHandler ( Reference < XDocumentHandler > () );
    return aManifestSequence;
}
// Component functions

Reference < XInterface > SAL_CALL ManifestReader_createInstance( Reference< XMultiServiceFactory > const & rServiceFactory )
{
    return *new ManifestReader( rServiceFactory );
}
OUString ManifestReader::static_getImplementationName()
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.comp.ManifestReader" ) );
}

sal_Bool SAL_CALL ManifestReader::static_supportsService(OUString const & rServiceName)
{
    return rServiceName == getSupportedServiceNames()[0];
}

Sequence < OUString > ManifestReader::static_getSupportedServiceNames()
{
    Sequence < OUString > aNames(1);
    aNames[0] = OUString(RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.ManifestReader" ) );
    return aNames;
}

OUString ManifestReader::getImplementationName()
    throw (RuntimeException)
{
    return static_getImplementationName();
}

sal_Bool SAL_CALL ManifestReader::supportsService(OUString const & rServiceName)
    throw (RuntimeException)
{
    return static_supportsService ( rServiceName );
}

Sequence < OUString > ManifestReader::getSupportedServiceNames()
    throw (RuntimeException)
{
    return static_getSupportedServiceNames();
}
Reference < XSingleServiceFactory > ManifestReader::createServiceFactory( Reference < XMultiServiceFactory > const & rServiceFactory )
{
    return cppu::createSingleFactory (rServiceFactory,
                                           static_getImplementationName(),
                                           ManifestReader_createInstance,
                                           static_getSupportedServiceNames());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
