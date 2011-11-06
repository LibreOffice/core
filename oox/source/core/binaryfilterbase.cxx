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



#include "oox/core/binaryfilterbase.hxx"

#include "oox/ole/olestorage.hxx"

namespace oox {
namespace core {

// ============================================================================

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

BinaryFilterBase::BinaryFilterBase( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    FilterBase( rxContext )
{
}

BinaryFilterBase::~BinaryFilterBase()
{
}

// private --------------------------------------------------------------------

StorageRef BinaryFilterBase::implCreateStorage( const Reference< XInputStream >& rxInStream ) const
{
    return StorageRef( new ::oox::ole::OleStorage( getComponentContext(), rxInStream, true ) );
}

StorageRef BinaryFilterBase::implCreateStorage( const Reference< XStream >& rxOutStream ) const
{
    return StorageRef( new ::oox::ole::OleStorage( getComponentContext(), rxOutStream, true ) );
}

// ============================================================================

} // namespace core
} // namespace oox
