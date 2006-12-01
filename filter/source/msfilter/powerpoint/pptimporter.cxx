/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pptimporter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:27:45 $
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
#include "precompiled_filter.hxx"

#ifndef _PPTIMPORTER_HXX
#include "pptimporter.hxx"
#endif

#ifdef DBG_DUMP_PPT_IMPORT
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HDL_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#endif

#include <uno/mapping.hxx>

// -----------------
// - PptImporter -
// -----------------

NMSP_RTL::OUString PptImporter_getImplementationName()
    throw( NMSP_UNO::RuntimeException )
{
    return B2UCONST( "com.sun.star.presentation.PptImporter" );
}
#define SERVICE_NAME "com.sun.star.document.ImportFilter"
sal_Bool SAL_CALL PptImporter_supportsService( const NMSP_RTL::OUString& ServiceName )
    throw( NMSP_UNO::RuntimeException )
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SERVICE_NAME ) );
}

SEQ( NMSP_RTL::OUString ) SAL_CALL PptImporter_getSupportedServiceNames()
    throw( NMSP_UNO::RuntimeException )
{
    SEQ( NMSP_RTL::OUString ) aRet(1);
    NMSP_RTL::OUString* pArray = aRet.getArray();
    pArray[0] = B2UCONST( SERVICE_NAME );
    return aRet;
}
#undef SERVICE_NAME

// -----------------------------------------------------------------------------

PptImporter::PptImporter( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr ) :
    xFact( rxMgr )
{
    NMSP_RTL::OUString sService( B2UCONST( "com.sun.star.comp.Impress.XMLImporter" ) );
    try
    {
        xHdl      = REF( NMSP_SAX::XDocumentHandler ) ( xFact->createInstance( sService ), NMSP_UNO::UNO_QUERY );
        xImporter = REF( NMSP_DOCUMENT::XImporter )( xHdl, NMSP_UNO::UNO_QUERY );
    }
    catch( NMSP_UNO::Exception & )
    {
    }
}

// -----------------------------------------------------------------------------

PptImporter::~PptImporter()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL PptImporter::acquire() throw()
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL PptImporter::release() throw()
{
    OWeakObject::release();
}

// XFilter
sal_Bool SAL_CALL PptImporter::filter( const SEQ( NMSP_BEANS::PropertyValue )& aDescriptor )
    throw ( NMSP_UNO::RuntimeException )
{


#ifdef DBG_DUMP_PPT_IMPORT

    REF( NMSP_LANG::XMultiServiceFactory ) xServiceFactory( NMSP_COMPHELPER::getProcessServiceFactory() );
       REF( NMSP_UNO::XInterface ) xWriter( xServiceFactory->createInstance( B2UCONST( "com.sun.star.xml.sax.Writer" ) ) );
    SvFileStream aStrm( B2UCONST( "d:\\test2.xml" ), STREAM_READ | STREAM_WRITE | STREAM_TRUNC );
    REF( NMSP_IO::XOutputStream ) xOutStream( (::cppu::OWeakObject*) new NMSP_UTL::OOutputStreamWrapper( aStrm ), NMSP_UNO::UNO_QUERY );
    REF( NMSP_IO::XActiveDataSource ) xDataSource( xWriter , NMSP_UNO::UNO_QUERY );
    xDataSource->setOutputStream( xOutStream );
    aFilter.filter( aDescriptor, REF( NMSP_SAX::XDocumentHandler )( xWriter, NMSP_UNO::UNO_QUERY ) );

//  REF( NMSP_BEANS::XPropertySet ) xPropSet( xServiceFactory->createInstance( B2UCONST( "com.sun.star.beans.PropertySet" ) ) );
//  Any aAny;
//  aAny <<= (sal_Bool)sal_True;
//  xPropSet->setPropertyValue( B2UCONST( "UsePrettyPrinting" ), aAny );

#endif

    return aFilter.filter( aDescriptor, xHdl );
}
void SAL_CALL PptImporter::cancel()
    throw ( NMSP_UNO::RuntimeException )
{
    aFilter.cancel();
}

// XImporter
void SAL_CALL PptImporter::setTargetDocument( const REF( NMSP_LANG::XComponent )& xDoc )
    throw ( NMSP_LANG::IllegalArgumentException, NMSP_UNO::RuntimeException)
{
    xImporter->setTargetDocument( xDoc );
}

// XInitialization
void SAL_CALL PptImporter::initialize( const SEQ( NMSP_UNO::Any )& /* aArguments */ )
    throw ( NMSP_UNO::Exception, NMSP_UNO::RuntimeException )
{
}

// XServiceInfo
NMSP_RTL::OUString SAL_CALL PptImporter::getImplementationName()
    throw( NMSP_UNO::RuntimeException )
{
    return PptImporter_getImplementationName();
}
sal_Bool SAL_CALL PptImporter::supportsService( const NMSP_RTL::OUString& rServiceName )
    throw( NMSP_UNO::RuntimeException )
{
    return PptImporter_supportsService( rServiceName );
}
SEQ( NMSP_RTL::OUString ) SAL_CALL PptImporter::getSupportedServiceNames()
    throw ( NMSP_UNO::RuntimeException )
{
    return PptImporter_getSupportedServiceNames();
}
