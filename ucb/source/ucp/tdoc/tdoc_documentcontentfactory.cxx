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
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "cppuhelper/factory.hxx"

#include "tdoc_documentcontentfactory.hxx"

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
uno::Reference< ucb::XContent > SAL_CALL
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
