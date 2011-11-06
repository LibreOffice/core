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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "com/sun/star/beans/XPropertySet.hpp"


using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace osl;

namespace comphelper
{

/*
    This function preserves only that the xProcessFactory variable will not be create when
    the library is loaded.
*/
Reference< XMultiServiceFactory > localProcessFactory( const Reference< XMultiServiceFactory >& xSMgr, sal_Bool bSet )
{
    Guard< Mutex > aGuard( Mutex::getGlobalMutex() );

    static Reference< XMultiServiceFactory > xProcessFactory;
    if ( bSet )
    {
        xProcessFactory = xSMgr;
    }

    return xProcessFactory;
}


void setProcessServiceFactory(const Reference< XMultiServiceFactory >& xSMgr)
{
    localProcessFactory( xSMgr, sal_True );
}

Reference< XMultiServiceFactory > getProcessServiceFactory()
{
    Reference< XMultiServiceFactory> xReturn;
    xReturn = localProcessFactory( xReturn, sal_False );
    return xReturn;
}

Reference< XInterface > createProcessComponent( const ::rtl::OUString& _rServiceSpecifier ) SAL_THROW( ( RuntimeException ) )
{
    Reference< XInterface > xComponent;

    Reference< XMultiServiceFactory > xFactory( getProcessServiceFactory() );
    if ( xFactory.is() )
        xComponent = xFactory->createInstance( _rServiceSpecifier );

    return xComponent;
}

Reference< XInterface > createProcessComponentWithArguments( const ::rtl::OUString& _rServiceSpecifier,
        const Sequence< Any >& _rArgs ) SAL_THROW( ( RuntimeException ) )
{
    Reference< XInterface > xComponent;

    Reference< XMultiServiceFactory > xFactory( getProcessServiceFactory() );
    if ( xFactory.is() )
        xComponent = xFactory->createInstanceWithArguments( _rServiceSpecifier, _rArgs );

    return xComponent;
}

Reference< XComponentContext > getProcessComponentContext()
{
    Reference< XComponentContext > xRet;
    uno::Reference<beans::XPropertySet> const xProps(
        comphelper::getProcessServiceFactory(), uno::UNO_QUERY );
    if (xProps.is()) {
        try {
            xRet.set( xProps->getPropertyValue( rtl::OUString(
                              RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ),
                      uno::UNO_QUERY );
        }
        catch (beans::UnknownPropertyException const&) {
        }
    }
    return xRet;
}

} // namespace comphelper

extern "C" {
uno::XComponentContext * comphelper_getProcessComponentContext()
{
    uno::Reference<uno::XComponentContext> xRet;
    xRet = ::comphelper::getProcessComponentContext();
    if (xRet.is())
        xRet->acquire();
    return xRet.get();
}
} // extern "C"

