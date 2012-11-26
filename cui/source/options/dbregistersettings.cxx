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
#include "precompiled_cui.hxx"

#include "dbregistersettings.hxx"

#include <rtl/ustring.hxx>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= DatabaseMapItem
    //====================================================================
    DatabaseMapItem::DatabaseMapItem( sal_uInt16 _nId, const DatabaseRegistrations& _rRegistrations )
        :SfxPoolItem( _nId )
        ,m_aRegistrations( _rRegistrations )
    {
    }

    //--------------------------------------------------------------------
    int DatabaseMapItem::operator==( const SfxPoolItem& _rCompare ) const
    {
        const DatabaseMapItem* pItem = dynamic_cast< const DatabaseMapItem* >( &_rCompare);
        if ( !pItem )
            return sal_False;

        if ( m_aRegistrations.size() != pItem->m_aRegistrations.size() )
            return sal_False;

        return m_aRegistrations == pItem->m_aRegistrations;
    }

    //--------------------------------------------------------------------
    SfxPoolItem* DatabaseMapItem::Clone( SfxItemPool* ) const
    {
        return new DatabaseMapItem( Which(), m_aRegistrations );
    }

    //--------------------------------------------------------------------

//........................................................................
}   // namespace svx
//........................................................................


