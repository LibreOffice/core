/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "MyJob.h"

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
    Reference < XEventBroadcaster > xBrd( mxMSF->createInstance(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.GlobalEventBroadcaster" ) ) ), UNO_QUERY );
    Reference < com::sun::star::document::XEventListener > xLstner( mxMSF->createInstance(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Office.MyListener" ) ) ), UNO_QUERY );
    if ( xBrd.is() )
        xBrd->addEventListener( xLstner );
    return Any();
}

OUString MyJob_getImplementationName ()
    throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Office.MyJob" ) );
}

#define SERVICE_NAME "com.sun.star.task.Job"

sal_Bool SAL_CALL MyJob_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SERVICE_NAME ) );
}

Sequence< OUString > SAL_CALL MyJob_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
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
