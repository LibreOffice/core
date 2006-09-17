/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ManifestReader.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 17:23:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"

#ifndef _MANIFEST_READER_HXX
#include <ManifestReader.hxx>
#endif
#ifndef _MANIFEST_IMPORT_HXX
#include <ManifestImport.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
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
