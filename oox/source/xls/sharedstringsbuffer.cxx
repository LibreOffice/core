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



#include "oox/xls/sharedstringsbuffer.hxx"

#include "oox/xls/biffinputstream.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;

// ============================================================================

SharedStringsBuffer::SharedStringsBuffer( const WorkbookHelper& rHelper ) :
     WorkbookHelper( rHelper )
{
}

RichStringRef SharedStringsBuffer::createRichString()
{
    RichStringRef xString( new RichString( *this ) );
    maStrings.push_back( xString );
    return xString;
}

void SharedStringsBuffer::importSst( BiffInputStream& rStrm )
{
    rStrm.skip( 4 );
    sal_Int32 nStringCount = rStrm.readInt32();
    if( nStringCount > 0 )
    {
        maStrings.clear();
        maStrings.reserve( static_cast< size_t >( nStringCount ) );
        for( ; !rStrm.isEof() && (nStringCount > 0); --nStringCount )
        {
            RichStringRef xString( new RichString( *this ) );
            maStrings.push_back( xString );
            xString->importUniString( rStrm );
        }
    }
}

void SharedStringsBuffer::finalizeImport()
{
    maStrings.forEachMem( &RichString::finalizeImport );
}

RichStringRef SharedStringsBuffer::getString( sal_Int32 nStringId ) const
{
    return maStrings.get( nStringId );
}

// ============================================================================

} // namespace xls
} // namespace oox
