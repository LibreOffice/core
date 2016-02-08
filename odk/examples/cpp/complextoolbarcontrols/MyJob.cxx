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


#include "MyJob.h"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <cppuhelper/supportsservice.hxx>

using rtl::OUString;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::beans::NamedValue;
using com::sun::star::document::XEventBroadcaster;

Any SAL_CALL MyJob::execute( const Sequence< NamedValue >& aArguments )
    throw ( IllegalArgumentException, Exception, RuntimeException )
{
    Reference < XEventBroadcaster > xBrd( mxMSF->createInstance(
        "com.sun.star.frame.GlobalEventBroadcaster" ), UNO_QUERY );
    Reference < com::sun::star::document::XEventListener > xLstner( mxMSF->createInstance(
        "com.sun.star.comp.Office.MyListener" ), UNO_QUERY );
    if ( xBrd.is() )
        xBrd->addEventListener( xLstner );
    return Any();
}

OUString MyJob_getImplementationName ()
    throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.Office.MyJob" );
}

#define SERVICE_NAME "com.sun.star.task.Job"

Sequence< OUString > SAL_CALL MyJob_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( SERVICE_NAME );
    return aRet;
}

#undef SERVICE_NAME

Reference< XInterface > SAL_CALL MyJob_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new MyJob( rSMgr );
}

// XServiceInfo
OUString SAL_CALL MyJob::getImplementationName(  )
    throw (RuntimeException)
{
    return MyJob_getImplementationName();
}

sal_Bool SAL_CALL MyJob::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL MyJob::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return MyJob_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
