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

#include <cppuhelper/implementationentry.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star::uno;

// Declare static functions providing service information =====================

#define DECLARE_FUNCTIONS( className )                                                  \
extern OUString SAL_CALL className##_getImplementationName() throw();                   \
extern Sequence< OUString > SAL_CALL className##_getSupportedServiceNames() throw();    \
extern Reference< XInterface > SAL_CALL className##_createInstance(                     \
    const Reference< XComponentContext >& rxContext ) throw (Exception)

namespace ooo { namespace vba { DECLARE_FUNCTIONS( VBAMacroResolver ); } }

#undef DECLARE_FUNCTIONS

// ============================================================================

namespace {

#define IMPLEMENTATION_ENTRY( className ) \
    { &className##_createInstance, &className##_getImplementationName, &className##_getSupportedServiceNames, ::cppu::createSingleComponentFactory, 0, 0 }

static ::cppu::ImplementationEntry const spServices[] =
{
    IMPLEMENTATION_ENTRY( ::ooo::vba::VBAMacroResolver ),
    { 0, 0, 0, 0, 0, 0 }
};

#undef IMPLEMENTATION_ENTRY

} // namespace

// ----------------------------------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
        const sal_Char** ppEnvironmentTypeName, uno_Environment** /*ppEnvironment*/ )
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    return ::cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey, spServices );
}

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    return ::cppu::component_writeInfoHelper( pServiceManager, pRegistryKey, spServices );
}

// ============================================================================
