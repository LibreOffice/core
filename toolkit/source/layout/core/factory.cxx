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



#include "factory.hxx"

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <cppuhelper/factory.hxx>

#include "root.hxx"

using namespace ::com::sun::star;
using namespace layoutimpl;

void * SAL_CALL comp_Layout_component_getFactory( const char * pImplName, void * pServiceManager, void * /*registryKey*/ )
    {
        void * pRet = 0;

        ::rtl::OUString aImplName( ::rtl::OUString::createFromAscii( pImplName ) );
        uno::Reference< lang::XSingleServiceFactory > xFactory;

        if ( pServiceManager && aImplName.equals( LayoutFactory::impl_staticGetImplementationName() ) )
            xFactory = ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>( pServiceManager ),
                                                         LayoutFactory::impl_staticGetImplementationName(),
                                                         LayoutFactory::impl_staticCreateSelfInstance,
                                                         LayoutFactory::impl_staticGetSupportedServiceNames() );
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }

        return pRet;
    }

// Component registration
::rtl::OUString SAL_CALL LayoutFactory::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.awt.Layout" );
}

uno::Sequence< ::rtl::OUString > SAL_CALL LayoutFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.awt.Layout");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.awt.Layout");
    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL LayoutFactory::impl_staticCreateSelfInstance(
    const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new LayoutFactory( xServiceManager ) );
}

// XServiceInfo
::rtl::OUString SAL_CALL LayoutFactory::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

uno::Sequence< ::rtl::OUString > SAL_CALL LayoutFactory::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

sal_Bool SAL_CALL LayoutFactory::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();
    for ( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
        if ( ServiceName.compareTo( aSeq[i] ) == 0 )
            return sal_True;

    return sal_False;
}

// XSingleServiceFactory
uno::Reference< uno::XInterface > SAL_CALL LayoutFactory::createInstance()
    throw ( uno::Exception,
            uno::RuntimeException )
{
    return uno::Reference< uno::XInterface >(
        static_cast< OWeakObject* >( new LayoutRoot( m_xFactory ) ),
        uno::UNO_QUERY );
}

uno::Reference< uno::XInterface > SAL_CALL LayoutFactory::createInstanceWithArguments(
    const uno::Sequence< uno::Any >& aArguments )
    throw ( uno::Exception,
            uno::RuntimeException )
{
    uno::Reference< uno::XInterface > layout = createInstance();
    uno::Reference< lang::XInitialization > xInit( layout, uno::UNO_QUERY );
    xInit->initialize( aArguments );
    return layout;
}
