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

namespace oox {
    namespace core {    DECLARE_FUNCTIONS( FastTokenHandler );          }
    namespace core {    DECLARE_FUNCTIONS( FilterDetect );              }
    namespace docprop { DECLARE_FUNCTIONS( DocumentPropertiesImport );  }
    namespace ole {     DECLARE_FUNCTIONS( WordVbaProjectFilter );      }
    namespace ppt {     DECLARE_FUNCTIONS( PowerPointImport );          }
    namespace shape {   DECLARE_FUNCTIONS( ShapeContextHandler );       }
    namespace xls {     DECLARE_FUNCTIONS( BiffDetector );              }
    namespace xls {     DECLARE_FUNCTIONS( ExcelFilter );               }
    namespace xls {     DECLARE_FUNCTIONS( ExcelBiffFilter );           }
    namespace xls {     DECLARE_FUNCTIONS( ExcelVbaProjectFilter );     }
    namespace xls {     DECLARE_FUNCTIONS( OOXMLFormulaParser );        }
}

#undef DECLARE_FUNCTIONS

// ============================================================================

namespace {

#define IMPLEMENTATION_ENTRY( className ) \
    { &className##_createInstance, &className##_getImplementationName, &className##_getSupportedServiceNames, ::cppu::createSingleComponentFactory, 0, 0 }

static ::cppu::ImplementationEntry const spServices[] =
{
    IMPLEMENTATION_ENTRY( ::oox::core::FastTokenHandler ),
    IMPLEMENTATION_ENTRY( ::oox::core::FilterDetect ),
    IMPLEMENTATION_ENTRY( ::oox::docprop::DocumentPropertiesImport ),
    IMPLEMENTATION_ENTRY( ::oox::ole::WordVbaProjectFilter ),
    IMPLEMENTATION_ENTRY( ::oox::ppt::PowerPointImport ),
    IMPLEMENTATION_ENTRY( ::oox::shape::ShapeContextHandler ),
    IMPLEMENTATION_ENTRY( ::oox::xls::BiffDetector ),
    IMPLEMENTATION_ENTRY( ::oox::xls::ExcelFilter ),
    IMPLEMENTATION_ENTRY( ::oox::xls::ExcelBiffFilter ),
    IMPLEMENTATION_ENTRY( ::oox::xls::ExcelVbaProjectFilter ),
    IMPLEMENTATION_ENTRY( ::oox::xls::OOXMLFormulaParser ),
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

// ============================================================================
