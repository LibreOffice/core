/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: facreg.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:56:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _UNO_LBNAMES_H_
#include <uno/lbnames.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef INCLUDED_STRING_H
#include <string.h>
#define INCLUDED_STRING_H
#endif

using namespace rtl;
using namespace com::sun::star;

extern uno::Reference< uno::XInterface > SAL_CALL SdDrawingDocument_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString SdDrawingDocument_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdDrawingDocument_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL SdPresentationDocument_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString SdPresentationDocument_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdPresentationDocument_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL SdHtmlOptionsDialog_CreateInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString SdHtmlOptionsDialog_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdHtmlOptionsDialog_getSupportedServiceNames() throw( uno::RuntimeException );

extern uno::Reference< uno::XInterface > SAL_CALL SdUnoModule_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString SdUnoModule_getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL SdUnoModule_getSupportedServiceNames() throw( uno::RuntimeException );

namespace sd
{
extern uno::Reference< uno::XInterface > SAL_CALL RandomNode_createInstance( const uno::Reference< lang::XMultiServiceFactory > & _rxFactory );
extern OUString RandomNode__getImplementationName() throw( uno::RuntimeException );
extern uno::Sequence< OUString > SAL_CALL RandomNode_getSupportedServiceNames() throw( uno::RuntimeException );
}

#ifdef __cplusplus
extern "C"
{
#endif

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char  ** ppEnvTypeName,
    uno_Environment **  )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

static void SAL_CALL writeInfo(
    registry::XRegistryKey          * pRegistryKey,
    const OUString                  & rImplementationName,
    const uno::Sequence< OUString > & rServices )
{
    uno::Reference< registry::XRegistryKey > xNewKey(
        pRegistryKey->createKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(
    void * ,
    void * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            registry::XRegistryKey *pKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey );

            writeInfo( pKey, SdHtmlOptionsDialog_getImplementationName(), SdHtmlOptionsDialog_getSupportedServiceNames() );
            writeInfo( pKey, SdDrawingDocument_getImplementationName(), SdDrawingDocument_getSupportedServiceNames() );
            writeInfo( pKey, SdPresentationDocument_getImplementationName(), SdPresentationDocument_getSupportedServiceNames() );
            writeInfo( pKey, SdUnoModule_getImplementationName(), SdUnoModule_getSupportedServiceNames() );
            writeInfo( pKey, sd::RandomNode__getImplementationName(), sd::RandomNode_getSupportedServiceNames() );
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_True;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName,
    void           * pServiceManager,
    void           *  )
{
    void * pRet = 0;

    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference< lang::XSingleServiceFactory > xFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );
        if( SdHtmlOptionsDialog_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdHtmlOptionsDialog_getImplementationName(),
                SdHtmlOptionsDialog_CreateInstance,
                SdHtmlOptionsDialog_getSupportedServiceNames() );
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
        else if( SdUnoModule_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdUnoModule_getImplementationName(),
                SdUnoModule_createInstance,
                SdUnoModule_getSupportedServiceNames() );
        }
        else if( sd::RandomNode__getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                sd::RandomNode__getImplementationName(),
                sd::RandomNode_createInstance,
                sd::RandomNode_getSupportedServiceNames() );
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
