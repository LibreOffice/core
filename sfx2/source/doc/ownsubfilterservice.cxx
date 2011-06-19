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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <ownsubfilterservice.hxx>
#include <sfx2/objsh.hxx>

using namespace ::com::sun::star;

namespace sfx2 {

//-------------------------------------------------------------------------
OwnSubFilterService::OwnSubFilterService( const uno::Reference < lang::XMultiServiceFactory >& xFactory )
: m_xFactory( xFactory )
, m_pObjectShell( NULL )
{
}

//-------------------------------------------------------------------------
OwnSubFilterService::~OwnSubFilterService()
{
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OwnSubFilterService::impl_getStaticSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.OwnSubFilter"));
    aRet[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.document.OwnSubFilter"));
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OwnSubFilterService::impl_getStaticImplementationName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.document.OwnSubFilter"));
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OwnSubFilterService::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new OwnSubFilterService( xServiceManager ) );
}

//-------------------------------------------------------------------------
uno::Reference< lang::XSingleServiceFactory > SAL_CALL OwnSubFilterService::impl_createFactory(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return ::cppu::createSingleFactory( xServiceManager,
                                OwnSubFilterService::impl_getStaticImplementationName(),
                                OwnSubFilterService::impl_staticCreateSelfInstance,
                                OwnSubFilterService::impl_getStaticSupportedServiceNames() );
}


// XFilter

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OwnSubFilterService::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
    throw (uno::RuntimeException)
{
    if ( !m_pObjectShell )
        throw uno::RuntimeException();

    return m_pObjectShell->ImportFromGeneratedStream_Impl( m_xStream, aDescriptor );
}

//-------------------------------------------------------------------------
void SAL_CALL OwnSubFilterService::cancel()
    throw (uno::RuntimeException)
{
    // not implemented
}


// XInitialization

//-------------------------------------------------------------------------
void SAL_CALL OwnSubFilterService::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw (uno::Exception, uno::RuntimeException)
{
    if ( !m_xFactory.is() )
        throw uno::RuntimeException();

    if ( aArguments.getLength() != 2 )
        throw lang::IllegalArgumentException();

    if ( m_pObjectShell )
        throw frame::DoubleInitializationException();

    if ( ( aArguments[1] >>= m_xStream ) && m_xStream.is()
      && ( aArguments[0] >>= m_xModel ) && m_xModel.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::lang::XUnoTunnel > xObj( m_xModel, uno::UNO_QUERY_THROW );
        ::com::sun::star::uno::Sequence < sal_Int8 > aSeq( SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence() );
        sal_Int64 nHandle = xObj->getSomething( aSeq );
        if ( nHandle )
            m_pObjectShell = reinterpret_cast< SfxObjectShell* >( sal::static_int_cast< sal_IntPtr >( nHandle ));
    }

    if ( !m_pObjectShell )
        throw lang::IllegalArgumentException();
}


// XServiceInfo

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OwnSubFilterService::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_getStaticImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OwnSubFilterService::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_getStaticSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OwnSubFilterService::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
