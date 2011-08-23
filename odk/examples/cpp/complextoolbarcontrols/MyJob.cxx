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


#include "MyJob.h"
#include <osl/diagnose.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>

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
    Reference < XEventBroadcaster > xBrd( mxMSF->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.GlobalEventBroadcaster") ), UNO_QUERY );
    Reference < com::sun::star::document::XEventListener > xLstner( mxMSF->createInstance(
        ::rtl::OUString::createFromAscii("com.sun.star.comp.Office.MyListener" ) ), UNO_QUERY );
    if ( xBrd.is() )
        xBrd->addEventListener( xLstner );
    return Any();
}

OUString MyJob_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Office.MyJob" ) );
}

#define SERVICE_NAME "com.sun.star.task.Job"

sal_Bool SAL_CALL MyJob_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
}

Sequence< OUString > SAL_CALL MyJob_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
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
    return MyJob_supportsService( rServiceName );
}

Sequence< OUString > SAL_CALL MyJob::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return MyJob_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
