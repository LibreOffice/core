/*************************************************************************
 *
 *  $RCSfile: ManifestReader.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mtg $ $Date: 2001-04-19 14:09:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/

#ifndef _MANIFEST_READER_HXX
#include "ManifestReader.hxx"
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP
#include <com/sun/star/xml/sax/SaxParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif
#ifndef _MANIFEST_IMPORT_HXX
#include <ManifestImport.hxx>
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
Any SAL_CALL ManifestReader::queryInterface( const Type& rType )
    throw(RuntimeException)
{
    return ::cppu::queryInterface ( rType                                       ,
                                        // OWeakObject interfaces
                                        reinterpret_cast< XInterface*       > ( this )  ,
                                        static_cast< XWeak*         > ( this )  ,
                                        // My own interfaces
                                        static_cast< XManifestReader*       > ( this ) );

}

void SAL_CALL ManifestReader::acquire(  )
    throw()
{
    OWeakObject::acquire();
}
void SAL_CALL ManifestReader::release(  )
    throw()
{
    OWeakObject::release();
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
    return aManifestSequence;
}
// Component functions

Reference < XInterface > SAL_CALL ManifestReader_createInstance( Reference< XMultiServiceFactory > const & rServiceFactory )
{
    return *new ManifestReader( rServiceFactory );
}

OUString ManifestReader::getImplementationName()
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.comp.ManifestReader" ) );
}

sal_Bool SAL_CALL ManifestReader::supportsService(OUString const & rServiceName)
    throw (RuntimeException)
{
    return rServiceName == getSupportedServiceNames()[0];
}

Sequence < OUString > ManifestReader::getSupportedServiceNames()
{
    Sequence < OUString > aNames(1);
    aNames[0] = OUString(RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.ManifestReader" ) );
    return aNames;
}
Reference < XSingleServiceFactory > ManifestReader::createServiceFactory( Reference < XMultiServiceFactory > const & rServiceFactory )
{
    return cppu::createSingleFactory (rServiceFactory,
                                           getImplementationName(),
                                           ManifestReader_createInstance,
                                           getSupportedServiceNames());
}
