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



#include "oox/token/propertynames.hxx"

namespace oox {

// ============================================================================

PropertyNameVector::PropertyNameVector()
{
    static const sal_Char* sppcPropertyNames[] =
    {
        // include auto-generated C array with property names as C strings
#include <token/propertynames.inc>
    };

    size_t nArraySize = (sizeof( sppcPropertyNames ) / sizeof( *sppcPropertyNames )) - 1;
    reserve( nArraySize );
    for( size_t nIndex = 0; nIndex < nArraySize; ++nIndex )
        push_back( ::rtl::OUString::createFromAscii( sppcPropertyNames[ nIndex ] ) );
}

// ============================================================================

} // namespace oox
