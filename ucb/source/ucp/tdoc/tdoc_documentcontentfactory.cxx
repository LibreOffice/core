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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>

#include "tdoc_documentcontentfactory.hxx"

using namespace com::sun::star;
using namespace tdoc_ucp;


// DocumentContentFactory Implementation.


DocumentContentFactory::DocumentContentFactory(
            const uno::Reference< uno::XComponentContext >& rxContext )
: m_xContext( rxContext )
{
}


// virtual
DocumentContentFactory::~DocumentContentFactory()
{
}


// XServiceInfo methods.


// virtual
OUString SAL_CALL DocumentContentFactory::getImplementationName()
{
    return "com.sun.star.comp.ucb.TransientDocumentsDocumentContentFactory";
}

// virtual
sal_Bool SAL_CALL
DocumentContentFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

// virtual
uno::Sequence< OUString > SAL_CALL
DocumentContentFactory::getSupportedServiceNames()
{
    return { "com.sun.star.frame.TransientDocumentsDocumentContentFactory" };
}


// XTransientDocumentsDocumentContentFactory methods.


// virtual
uno::Reference< ucb::XContent > SAL_CALL
DocumentContentFactory::createDocumentContent(
        const uno::Reference< frame::XModel >& Model )
{
    uno::Reference< frame::XTransientDocumentsDocumentContentFactory > xDocFac;
    try
    {
        xDocFac.set( m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.ucb.TransientDocumentsContentProvider", m_xContext),
                     uno::UNO_QUERY );
    }
    catch ( uno::Exception const & )
    {
        // handled below.
    }

    if ( xDocFac.is() )
        return xDocFac->createDocumentContent( Model );

    throw uno::RuntimeException(
        "Unable to obtain document content factory!",
        static_cast< cppu::OWeakObject * >( this ) );
}


// Service factory implementation.

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_tdoc_DocumentContentFactory_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new DocumentContentFactory(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
