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


#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include "cmdenv.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
using namespace com::sun::star;
using namespace ucb_cmdenv;


// UcbCommandEnvironment Implementation.


UcbCommandEnvironment::UcbCommandEnvironment()
{
}


// virtual
UcbCommandEnvironment::~UcbCommandEnvironment()
{
}


// XInitialization methods.


// virtual
void SAL_CALL UcbCommandEnvironment::initialize(
        const uno::Sequence< uno::Any >& aArguments )
{
    if ( ( aArguments.getLength() < 2 ) ||
         !( aArguments[ 0 ] >>= m_xIH ) ||
         !( aArguments[ 1 ] >>= m_xPH ))
        throw lang::IllegalArgumentException();
}


// XServiceInfo methods.


// virtual
OUString SAL_CALL UcbCommandEnvironment::getImplementationName()
{
    return getImplementationName_Static();
}


// virtual
sal_Bool SAL_CALL
UcbCommandEnvironment::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


// virtual
uno::Sequence< OUString > SAL_CALL
UcbCommandEnvironment::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}


// static
OUString UcbCommandEnvironment::getImplementationName_Static()
{
    return OUString( "com.sun.star.comp.ucb.CommandEnvironment" );
}


// static
uno::Sequence< OUString >
UcbCommandEnvironment::getSupportedServiceNames_Static()
{
    uno::Sequence<OUString> aSNS { "com.sun.star.ucb.CommandEnvironment" };
    return aSNS;
}


// XCommandInfo methods.


// virtual
uno::Reference< task::XInteractionHandler > SAL_CALL
UcbCommandEnvironment::getInteractionHandler()
{
    return m_xIH;
}


// virtual
uno::Reference< ucb::XProgressHandler > SAL_CALL
UcbCommandEnvironment::getProgressHandler()
{
    return m_xPH;
}


// Service factory implementation.

/// @throws uno::Exception
static uno::Reference< uno::XInterface >
UcbCommandEnvironment_CreateInstance(
    const uno::Reference< lang::XMultiServiceFactory> & /*rSMgr*/ )
{
    lang::XServiceInfo* pX = new UcbCommandEnvironment;
    return uno::Reference< uno::XInterface >::query( pX );
}


// static
uno::Reference< lang::XSingleServiceFactory >
UcbCommandEnvironment::createServiceFactory(
    const uno::Reference< lang::XMultiServiceFactory >& rxServiceMgr )
{
    return cppu::createSingleFactory(
                rxServiceMgr,
                UcbCommandEnvironment::getImplementationName_Static(),
                UcbCommandEnvironment_CreateInstance,
                UcbCommandEnvironment::getSupportedServiceNames_Static() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
