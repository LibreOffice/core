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



#include "oox/ole/vbaprojectfilter.hxx"

#include "oox/ole/vbaproject.hxx"

namespace oox {
namespace ole {

// ============================================================================

using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

VbaProjectFilterBase::VbaProjectFilterBase( const Reference< XComponentContext >& rxContext,
        const OUString& rAppName, const OUString& rStorageName ) throw( RuntimeException ) :
    BinaryFilterBase( rxContext ),
    maAppName( rAppName ),
    maStorageName( rStorageName )
{
}

bool VbaProjectFilterBase::importDocument() throw()
{
    StorageRef xVbaPrjStrg = openSubStorage( maStorageName, false );
    if( !xVbaPrjStrg || !xVbaPrjStrg->isStorage() )
        return false;

    getVbaProject().importVbaProject( *xVbaPrjStrg, getGraphicHelper() );
    return true;
}

bool VbaProjectFilterBase::exportDocument() throw()
{
    return false;
}

VbaProject* VbaProjectFilterBase::implCreateVbaProject() const
{
    return new VbaProject( getComponentContext(), getModel(), maAppName );
}

// ============================================================================

OUString SAL_CALL WordVbaProjectFilter_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.WordVbaProjectFilter" );
}

Sequence< OUString > SAL_CALL WordVbaProjectFilter_getSupportedServiceNames() throw()
{
    Sequence< OUString > aSeq( 1 );
    aSeq[ 0 ] = CREATE_OUSTRING( "com.sun.star.document.ImportFilter" );
    return aSeq;
}

Reference< XInterface > SAL_CALL WordVbaProjectFilter_createInstance(
        const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new WordVbaProjectFilter( rxContext ) );
}

// ----------------------------------------------------------------------------

WordVbaProjectFilter::WordVbaProjectFilter( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    VbaProjectFilterBase( rxContext, CREATE_OUSTRING( "Writer" ), CREATE_OUSTRING( "Macros" ) )
{
}

OUString WordVbaProjectFilter::implGetImplementationName() const
{
    return WordVbaProjectFilter_getImplementationName();
}

// ============================================================================

} // namespace ole
} // namespace oox
