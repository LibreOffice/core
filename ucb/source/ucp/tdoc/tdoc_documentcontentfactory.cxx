/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
    return rtl::OUString(
        "com.sun.star.comp.ucb.TransientDocumentsDocumentContentFactory" );
}

//=========================================================================
// static
uno::Sequence< rtl::OUString >
DocumentContentFactory::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ]
        = rtl::OUString(
            "com.sun.star.frame.TransientDocumentsDocumentContentFactory" );
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
                    rtl::OUString(
                        "com.sun.star.ucb.TransientDocumentsContentProvider" )
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
        rtl::OUString(
            "Unable to obtain document content factory!" ),
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
