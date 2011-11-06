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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <osl/mutex.hxx>
#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "StyleMap.hxx"

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


StyleMap::StyleMap()
{
}



StyleMap::~StyleMap()
{
}


// XUnoTunnel & co
const Sequence< sal_Int8 > & StyleMap::getUnoTunnelId() throw()
{
    static Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( reinterpret_cast<sal_uInt8*>( aSeq.getArray() ),
                            0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

StyleMap* StyleMap::getImplementation( Reference< XInterface > xInt ) throw()
{
    Reference< XUnoTunnel > xUT( xInt, UNO_QUERY );
    if( xUT.is() )
        return reinterpret_cast<StyleMap *>(
                xUT->getSomething( StyleMap::getUnoTunnelId() ) );
    else
        return 0;
}

// XUnoTunnel
sal_Int64 SAL_CALL StyleMap::getSomething(
        const Sequence< sal_Int8 >& rId )
    throw( RuntimeException )
{
    if( rId.getLength() == 16 &&
        0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                             rId.getConstArray(), 16 ) )
    {
        return reinterpret_cast<sal_Int64>( this );
    }
    return 0;
}


