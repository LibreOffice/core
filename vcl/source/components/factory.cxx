/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: factory.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_vcl.hxx"
#include <tools/debug.hxx>
#ifndef _OSL_MUTEX_HXX
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX
#include <rtl/ustrbuf.hxx>
#endif
#include <uno/dispatcher.h> // declaration of generic uno interface
#include <uno/mapping.hxx> // mapping stuff
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/dllapi.h>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


// service implementation

extern Sequence< OUString > SAL_CALL vcl_session_getSupportedServiceNames();
extern OUString SAL_CALL vcl_session_getImplementationName();
extern Reference< XInterface > SAL_CALL vcl_session_createInstance( const Reference< XMultiServiceFactory > & );

namespace vcl
{
extern Sequence< OUString > SAL_CALL DisplayAccess_getSupportedServiceNames();
extern OUString SAL_CALL DisplayAccess_getImplementationName();
extern Reference< XInterface > SAL_CALL DisplayAccess_createInstance( const Reference< XMultiServiceFactory > &  );

extern Sequence< OUString > SAL_CALL FontIdentificator_getSupportedServiceNames();
extern OUString SAL_CALL FontIdentificator_getImplementationName();
extern Reference< XInterface > SAL_CALL FontIdentificator_createInstance( const Reference< XMultiServiceFactory > &  );
}

extern "C" {

    VCL_DLLPUBLIC void SAL_CALL component_getImplementationEnvironment(
        const sal_Char** ppEnvTypeName,
        uno_Environment** /*ppEnv*/ )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    VCL_DLLPUBLIC sal_Bool SAL_CALL component_writeInfo( void* /*pServiceManager*/, void* pXUnoKey )
    {
        if( pXUnoKey )
        {
            try
            {
                Reference< ::com::sun::star::registry::XRegistryKey > xKey( reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >( pXUnoKey ) );

                OUStringBuffer aImplName(64);
                aImplName.appendAscii( "/" );
                aImplName.append( vcl_session_getImplementationName() );
                aImplName.appendAscii( "/UNO/SERVICES/" );
                aImplName.append( vcl_session_getSupportedServiceNames()[0] );
                xKey->createKey( aImplName.makeStringAndClear() );

                aImplName.appendAscii( "/" );
                aImplName.append( vcl::DisplayAccess_getImplementationName() );
                aImplName.appendAscii( "/UNO/SERVICES/" );
                aImplName.append( vcl::DisplayAccess_getSupportedServiceNames()[0] );
                xKey->createKey( aImplName.makeStringAndClear() );

                aImplName.appendAscii( "/" );
                aImplName.append( vcl::FontIdentificator_getImplementationName() );
                aImplName.appendAscii( "/UNO/SERVICES/" );
                aImplName.append( vcl::FontIdentificator_getSupportedServiceNames()[0] );
                xKey->createKey( aImplName.makeStringAndClear() );

                return sal_True;
            }
            catch( ::com::sun::star::registry::InvalidRegistryException& )
            {
            }
        }
        return sal_False;
    }

    VCL_DLLPUBLIC void* SAL_CALL component_getFactory(
        const sal_Char* pImplementationName,
        void* pXUnoSMgr,
        void* /*pXUnoKey*/
        )
    {
        void* pRet = 0;

        if( pXUnoSMgr )
        {
            Reference< ::com::sun::star::lang::XMultiServiceFactory > xMgr(
                reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pXUnoSMgr )
                );
            Reference< ::com::sun::star::lang::XSingleServiceFactory > xFactory;
            if( vcl_session_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, vcl_session_getImplementationName(), vcl_session_createInstance,
                    vcl_session_getSupportedServiceNames() );
            }
            else if( vcl::DisplayAccess_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, vcl::DisplayAccess_getImplementationName(), vcl::DisplayAccess_createInstance,
                    vcl::DisplayAccess_getSupportedServiceNames() );
            }
            else if( vcl::FontIdentificator_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, vcl::FontIdentificator_getImplementationName(), vcl::FontIdentificator_createInstance,
                    vcl::FontIdentificator_getSupportedServiceNames() );
            }
            if( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        return pRet;
    }

} /* extern "C" */
