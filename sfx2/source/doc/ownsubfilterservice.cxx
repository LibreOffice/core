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
uno::Sequence< OUString > SAL_CALL OwnSubFilterService::impl_getStaticSupportedServiceNames()
{
    uno::Sequence< OUString > aRet(2);
    aRet[0] = OUString("com.sun.star.document.OwnSubFilter");
    aRet[1] = OUString("com.sun.star.comp.document.OwnSubFilter");
    return aRet;
}

//-------------------------------------------------------------------------
OUString SAL_CALL OwnSubFilterService::impl_getStaticImplementationName()
{
    return OUString("com.sun.star.comp.document.OwnSubFilter");
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
OUString SAL_CALL OwnSubFilterService::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_getStaticImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OwnSubFilterService::supportsService( const OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< OUString > aSeq = impl_getStaticSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName == aSeq[nInd] )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< OUString > SAL_CALL OwnSubFilterService::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
