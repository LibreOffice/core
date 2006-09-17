/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ManifestWriter.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 17:23:29 $
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

#ifndef _MANIFEST_WRITER_HXX
#include <ManifestWriter.hxx>
#endif
#ifndef _MANIFEST_EXPORT_HXX
#include <ManifestExport.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXEXCEPTION_HPP
#include <com/sun/star/xml/sax/SAXException.hpp>
#endif

using namespace ::rtl;
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
                throw RuntimeException();
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
