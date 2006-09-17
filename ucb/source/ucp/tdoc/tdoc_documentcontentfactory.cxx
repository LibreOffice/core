/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdoc_documentcontentfactory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:01:55 $
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
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "cppuhelper/factory.hxx"

#include "tdoc_documentcontentfactory.hxx"

using namespace com::sun;
using namespace com::sun::star;

using namespace tdoc_ucp;

//=========================================================================
//=========================================================================
//
// DocumentContentFactory Implementation.
//
//=========================================================================
//=========================================================================

DocumentContentFactory::DocumentContentFactory(
            const uno::Reference< lang::XMultiServiceFactory >& xSMgr )
: m_xSMgr( xSMgr )
{
}

//=========================================================================
// virtual
DocumentContentFactory::~DocumentContentFactory()
{
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
::rtl::OUString SAL_CALL DocumentContentFactory::getImplementationName()
    throw ( uno::RuntimeException )
{
    return getImplementationName_Static();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL
DocumentContentFactory::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSNL = getSupportedServiceNames();
    const rtl::OUString * pArray = aSNL.getConstArray();
    for ( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
    {
        if ( pArray[ i ] == ServiceName )
            return sal_True;
    }
    return sal_False;
}

//=========================================================================
// virtual
uno::Sequence< ::rtl::OUString > SAL_CALL
DocumentContentFactory::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

//=========================================================================
// static
rtl::OUString DocumentContentFactory::getImplementationName_Static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.ucb.TransientDocumentsDocumentContentFactory" ) );
}

//=========================================================================
// static
uno::Sequence< rtl::OUString >
DocumentContentFactory::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ]
        = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.frame.TransientDocumentsDocumentContentFactory" ) );
    return aSNS;
}

//=========================================================================
//
// XTransientDocumentsDocumentContentFactory methods.
//
//=========================================================================

// virtual
uno::Reference< star::ucb::XContent > SAL_CALL
DocumentContentFactory::createDocumentContent(
        const uno::Reference< frame::XModel >& Model )
    throw ( lang::IllegalArgumentException, uno::RuntimeException )
{
    uno::Reference< frame::XTransientDocumentsDocumentContentFactory > xDocFac;
    try
    {
        xDocFac
            = uno::Reference< frame::XTransientDocumentsDocumentContentFactory >(
                m_xSMgr->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.ucb.TransientDocumentsContentProvider" ) )
                    ),
                uno::UNO_QUERY );
    }
    catch ( uno::Exception const & )
    {
        // handled below.
    }

    if ( xDocFac.is() )
        return xDocFac->createDocumentContent( Model );

    throw uno::RuntimeException(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
            "Unable to obtain document content factory!" ) ),
        static_cast< cppu::OWeakObject * >( this ) );
}

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

static uno::Reference< uno::XInterface > SAL_CALL
DocumentContentFactory_CreateInstance(
    const uno::Reference< lang::XMultiServiceFactory> & rSMgr )
    throw( uno::Exception )
{
    lang::XServiceInfo * pX = static_cast< lang::XServiceInfo * >(
        new DocumentContentFactory( rSMgr ) );
    return uno::Reference< uno::XInterface >::query( pX );
}

//=========================================================================
// static
uno::Reference< lang::XSingleServiceFactory >
DocumentContentFactory::createServiceFactory(
    const uno::Reference< lang::XMultiServiceFactory >& rxServiceMgr )
{
    return uno::Reference< lang::XSingleServiceFactory >(
            cppu::createOneInstanceFactory(
                rxServiceMgr,
                DocumentContentFactory::getImplementationName_Static(),
                DocumentContentFactory_CreateInstance,
                DocumentContentFactory::getSupportedServiceNames_Static() ) );
}

