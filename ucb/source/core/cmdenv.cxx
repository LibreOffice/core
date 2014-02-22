/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "cppuhelper/factory.hxx"
#include <cppuhelper/supportsservice.hxx>
#include "com/sun/star/lang/IllegalArgumentException.hpp"

#include "cmdenv.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
using namespace com::sun::star;
using namespace ucb_cmdenv;



//

//



UcbCommandEnvironment::UcbCommandEnvironment(
    const uno::Reference< lang::XMultiServiceFactory >& /*xSMgr*/ )

{
}



UcbCommandEnvironment::~UcbCommandEnvironment()
{
}


//

//



void SAL_CALL UcbCommandEnvironment::initialize(
        const uno::Sequence< uno::Any >& aArguments )
    throw( uno::Exception,
           uno::RuntimeException )
{
    if ( ( aArguments.getLength() < 2 ) ||
         !( aArguments[ 0 ] >>= m_xIH ) ||
         !( aArguments[ 1 ] >>= m_xPH ))
        throw lang::IllegalArgumentException();
}


//

//



OUString SAL_CALL UcbCommandEnvironment::getImplementationName()
    throw ( uno::RuntimeException )
{
    return getImplementationName_Static();
}



sal_Bool SAL_CALL
UcbCommandEnvironment::supportsService( const OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}



uno::Sequence< OUString > SAL_CALL
UcbCommandEnvironment::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}



OUString UcbCommandEnvironment::getImplementationName_Static()
{
    return OUString( "com.sun.star.comp.ucb.CommandEnvironment" );
}



uno::Sequence< OUString >
UcbCommandEnvironment::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = "com.sun.star.ucb.CommandEnvironment";
    return aSNS;
}


//

//



uno::Reference< task::XInteractionHandler > SAL_CALL
UcbCommandEnvironment::getInteractionHandler()
    throw ( uno::RuntimeException )
{
    return m_xIH;
}



uno::Reference< ucb::XProgressHandler > SAL_CALL
UcbCommandEnvironment::getProgressHandler()
    throw ( uno::RuntimeException )
{
    return m_xPH;
}


//

//


static uno::Reference< uno::XInterface > SAL_CALL
UcbCommandEnvironment_CreateInstance(
    const uno::Reference< lang::XMultiServiceFactory> & rSMgr )
    throw( uno::Exception )
{
    lang::XServiceInfo * pX = static_cast< lang::XServiceInfo * >(
        new UcbCommandEnvironment( rSMgr ) );
    return uno::Reference< uno::XInterface >::query( pX );
}



uno::Reference< lang::XSingleServiceFactory >
UcbCommandEnvironment::createServiceFactory(
    const uno::Reference< lang::XMultiServiceFactory >& rxServiceMgr )
{
    return uno::Reference< lang::XSingleServiceFactory >(
            cppu::createSingleFactory(
                rxServiceMgr,
                UcbCommandEnvironment::getImplementationName_Static(),
                UcbCommandEnvironment_CreateInstance,
                UcbCommandEnvironment::getSupportedServiceNames_Static() ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
