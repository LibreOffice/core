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
#include "precompiled_svtools.hxx"

#include "hatchwindowfactory.hxx"
#include "hatchwindow.hxx"
#include "cppuhelper/factory.hxx"
#include <vcl/svapp.hxx>

#include "documentcloser.hxx"

using namespace ::com::sun::star;

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OHatchWindowFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.HatchWindowFactory" ));
    aRet[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.embed.HatchWindowFactory" ));
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OHatchWindowFactory::impl_staticGetImplementationName()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.embed.HatchWindowFactory" ));
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OHatchWindowFactory::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new OHatchWindowFactory( xServiceManager ) );
}


//-------------------------------------------------------------------------
uno::Reference< embed::XHatchWindow > SAL_CALL OHatchWindowFactory::createHatchWindowInstance(
                const uno::Reference< awt::XWindowPeer >& xParent,
                const awt::Rectangle& aBounds,
                const awt::Size& aHandlerSize )
    throw (uno::RuntimeException)
{
    if ( !xParent.is() )
        throw lang::IllegalArgumentException(); // TODO

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    VCLXHatchWindow* pResult = new VCLXHatchWindow();
    pResult->initializeWindow( xParent, aBounds, aHandlerSize );
    return uno::Reference< embed::XHatchWindow >( static_cast< embed::XHatchWindow* >( pResult ) );
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OHatchWindowFactory::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OHatchWindowFactory::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OHatchWindowFactory::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

//-------------------------------------------------------------------------

extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager, void * /* pRegistryKey */)
{
    void * pResult = 0;
    if (pServiceManager)
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory;
        if (OHatchWindowFactory::impl_staticGetImplementationName().compareToAscii (pImplementationName ) == 0)
        {
            xFactory = cppu::createOneInstanceFactory(
                reinterpret_cast< lang::XMultiServiceFactory* >(pServiceManager),
                OHatchWindowFactory::impl_staticGetImplementationName(),
                OHatchWindowFactory::impl_staticCreateSelfInstance,
                OHatchWindowFactory::impl_staticGetSupportedServiceNames());
        }
        else if (ODocumentCloser::impl_staticGetImplementationName().compareToAscii (pImplementationName ) == 0)
        {
            xFactory = cppu::createSingleFactory(
                reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                ODocumentCloser::impl_staticGetImplementationName(),
                ODocumentCloser::impl_staticCreateSelfInstance,
                ODocumentCloser::impl_staticGetSupportedServiceNames() );
        }

        if (xFactory.is())
        {
            xFactory->acquire();
            pResult = xFactory.get();
        }
    }
    return pResult;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
