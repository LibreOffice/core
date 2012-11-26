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
#include "precompiled_svx.hxx"
#include "fmitems.hxx"

#include <svx/svxids.hrc>
#include <tools/stream.hxx>

//========================================================================
// class FmInterfaceItem
//========================================================================

//------------------------------------------------------------------------------
int FmInterfaceItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
    return( xInterface == ((FmInterfaceItem&)rAttr).GetInterface() );
}

//------------------------------------------------------------------------------
SfxPoolItem* FmInterfaceItem::Clone( SfxItemPool* ) const
{
    return new FmInterfaceItem( *this );
}

//------------------------------------------------------------------------------
SvStream& FmInterfaceItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    DBG_ERROR( "FmInterfaceItem::Store: not implemented!" );
    return rStrm;
}

//------------------------------------------------------------------------------
SfxPoolItem* FmInterfaceItem::Create( SvStream& /*rStrm*/, sal_uInt16 ) const
{
    DBG_ERROR( "FmInterfaceItem::Create: not implemented!" );
    return new FmInterfaceItem( *this );
}




