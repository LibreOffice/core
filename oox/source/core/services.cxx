/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
