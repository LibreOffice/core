/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"
#include <ManifestReader.hxx>
#include <ManifestImport.hxx>
#include <cppuhelper/factory.hxx>
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#include <vector>

using namespace ::rtl;
using namespace ::std;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::packages;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::packages::manifest;

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
