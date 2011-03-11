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
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.awt.Layout"));
}

uno::Sequence< ::rtl::OUString > SAL_CALL LayoutFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.Layout"));
    aRet[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.awt.Layout"));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
