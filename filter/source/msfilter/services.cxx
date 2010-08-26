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

#include <cppuhelper/factory.hxx>
#include <string.h>

using ::rtl::OUString;
using namespace ::com::sun::star;

// ============================================================================

/*  Steps to add an implementation 'MyClass' as a service:

    Step 1: Implement the static functions  MyClass_getSupportedServiceNames(),
    MyClass_getImplementationName(), and MyClass_createInstance() in the cxx
    file of MyClass (see other examples in this module).

    Step 2: Add the line 'DECL_STATIC_FUNCS_???( MyClass )' below in this file.
    If the service will be instanciated without component context, use
    DECL_STATIC_FUNCS_FACTORY. If the service will be instanciated with
    component context, use DECL_STATIC_FUNCS_COMPCONTEXT.

    Step 3: Add 'WRITE_SERVICE_INFO( MyClass )' in function
    component_writeInfo() below in this file.

    Step 4: Add 'IMPLEMENT_SINGLEFACTORY( MyClass )' (instanciation without
    component context) or 'IMPLEMENT_SINGLECOMPFACTORY( MyClass )'
    (instanciation with component context) in function component_getFactory()
    below in this file.
 */

// ============================================================================

// Declare static functions providing service information ---------------------

#define DECL_STATIC_FUNCS_FACTORY( className )                                              \
extern OUString SAL_CALL className##_getImplementationName() throw();                       \
extern uno::Sequence< OUString > SAL_CALL className##_getSupportedServiceNames() throw();   \
extern uno::Reference< uno::XInterface > SAL_CALL className##_createInstance(               \
    const uno::Reference< lang::XMultiServiceFactory >& rxFactory ) throw (uno::Exception)

#define DECL_STATIC_FUNCS_COMPCONTEXT( className )                                          \
extern OUString SAL_CALL className##_getImplementationName() throw();                       \
extern uno::Sequence< OUString > SAL_CALL className##_getSupportedServiceNames() throw();   \
extern uno::Reference< uno::XInterface > SAL_CALL className##_createInstance(               \
    const uno::Reference< uno::XComponentContext >& rxContext ) throw (uno::Exception)

// step 2: add new classes in this list
namespace ooo { namespace vba { DECL_STATIC_FUNCS_FACTORY( VBAMacroResolver ); } }

#undef DECL_STATIC_FUNCS_FACTORY
#undef DECL_STATIC_FUNCS_COMPCONTEXT

// ----------------------------------------------------------------------------

extern "C" {

// ----------------------------------------------------------------------------

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment( const sal_Char **ppEnvironmentTypeName, uno_Environment ** /*ppEnvironment*/ )
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// Write service information to registry --------------------------------------

void SAL_CALL writeInfo( registry::XRegistryKey* pRegistryKey, const OUString& rImplementationName, const uno::Sequence< OUString >& rServices )
{
    uno::Reference< registry::XRegistryKey > xNewKey( pRegistryKey->createKey(
        OUString( sal_Unicode( '/' ) ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES" ) ) ) );
    for( sal_Int32 i = 0; i < rServices.getLength(); ++i )
        xNewKey->createKey( rServices[i] );
}

#define WRITE_SERVICE_INFO( className ) \
    writeInfo( pKey, className##_getImplementationName(), className##_getSupportedServiceNames() )

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo( void* /*pServiceManager*/, void* pRegistryKey )
{
    if( pRegistryKey ) try
    {
        registry::XRegistryKey* pKey = reinterpret_cast< registry::XRegistryKey* >( pRegistryKey );

        // step 3: add new classes in this list
        WRITE_SERVICE_INFO( ::ooo::vba::VBAMacroResolver );
    }
    catch( registry::InvalidRegistryException& )
    {
        OSL_ENSURE( sal_False, "so_vba - component_writeInfo - InvalidRegistryException" );
    }
    return sal_True;
}

#undef WRITE_SERVICE_INFO

// Create a factory for the service instances ---------------------------------

#define IMPLEMENT_SINGLEFACTORY( className )                                                    \
if( !bFound && className##_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )    \
{                                                                                               \
    xFactory = ::cppu::createSingleFactory( xMSF, className##_getImplementationName(),          \
        className##_createInstance, className##_getSupportedServiceNames() );                   \
    bFound = true;                                                                              \
}

#define IMPLEMENT_SINGLECOMPFACTORY( className )                                                \
if( !bFound && className##_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )    \
{                                                                                               \
    xCompFactory = ::cppu::createSingleComponentFactory( className##_createInstance,            \
        className##_getImplementationName(), className##_getSupportedServiceNames() );          \
    bFound = true;                                                                              \
}

SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /*pRegistryKey*/ )
{
    void* pReturn = 0;
    if( pImplName && pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ) );

        // define variables which are used in the macros
        uno::Reference< lang::XSingleServiceFactory > xFactory;
        uno::Reference< lang::XSingleComponentFactory > xCompFactory;
        const sal_Int32 nImplNameLen = strlen( pImplName );
        bool bFound = false;

        // Step 4: add new class in this list
        IMPLEMENT_SINGLEFACTORY( ::ooo::vba::VBAMacroResolver )

        // factory is valid -> service was found
        if( xFactory.is())
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
        else if( xCompFactory.is() )
        {
            xCompFactory->acquire();
            pReturn = xCompFactory.get();
        }
    }
    return pReturn;
}

#undef IMPLEMENT_SINGLEFACTORY
#undef IMPLEMENT_SINGLECOMPFACTORY

// ----------------------------------------------------------------------------

} // extern "C"

// ============================================================================
