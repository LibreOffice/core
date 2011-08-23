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

#include <string.h>


#include <osl/diagnose.h>

#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>
namespace binfilter {

using namespace rtl;
using namespace ::com::sun::star;

extern uno::Reference< uno::XInterface > SAL_CALL SdDrawingDocument_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString SdDrawingDocument_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdDrawingDocument_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL SdPresentationDocument_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString SdPresentationDocument_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdPresentationDocument_getSupportedServiceNames() throw( uno::RuntimeException );


#ifdef __cplusplus
extern "C"
{
#endif

void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void SAL_CALL writeInfo( registry::XRegistryKey * pRegistryKey, const OUString& rImplementationName, const uno::Sequence< OUString >& rServices )
{
    uno::Reference< registry::XRegistryKey > xNewKey(
        pRegistryKey->createKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);
}

sal_Bool SAL_CALL component_writeInfo( void * pServiceManager, void * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            registry::XRegistryKey *pKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey );

            writeInfo( pKey, SdDrawingDocument_getImplementationName(), SdDrawingDocument_getSupportedServiceNames() );
            writeInfo( pKey, SdPresentationDocument_getImplementationName(), SdPresentationDocument_getSupportedServiceNames() );
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_True;
}

void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference< lang::XSingleServiceFactory > xFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );
        if(0)//STRIP001 if( SdHtmlOptionsDialog_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
        }
        else if( SdDrawingDocument_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdDrawingDocument_getImplementationName(),
                SdDrawingDocument_createInstance,
                SdDrawingDocument_getSupportedServiceNames() );
        }
        else if( SdPresentationDocument_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdPresentationDocument_getImplementationName(),
                SdPresentationDocument_createInstance,
                SdPresentationDocument_getSupportedServiceNames() );
        }


        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
