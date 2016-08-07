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
#include "precompiled_dbui.hxx"

#include "optionalboolitem.hxx"

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= OptionalBoolItem
    //====================================================================
    TYPEINIT1( OptionalBoolItem, SfxPoolItem );
    //--------------------------------------------------------------------
    OptionalBoolItem::OptionalBoolItem( sal_Int16 _nWhich )
        :SfxPoolItem( _nWhich )
        ,m_aValue()
    {
    }

    //--------------------------------------------------------------------
    OptionalBoolItem::OptionalBoolItem( const OptionalBoolItem& _rSource )
        :SfxPoolItem( _rSource )
        ,m_aValue( _rSource.m_aValue )
    {
    }

    //--------------------------------------------------------------------
    int OptionalBoolItem::operator==( const SfxPoolItem& _rItem ) const
    {
        const OptionalBoolItem* pCompare = PTR_CAST( OptionalBoolItem, &_rItem );
        if ( !pCompare )
            return 0;

        if ( m_aValue == pCompare->m_aValue )
            return 1;

        return 0;
    }

    //--------------------------------------------------------------------
    SfxPoolItem* OptionalBoolItem::Clone( SfxItemPool* /*_pPool*/ ) const
    {
        return new OptionalBoolItem( *this );
    }

//........................................................................
} // namespace dbaui
//........................................................................
