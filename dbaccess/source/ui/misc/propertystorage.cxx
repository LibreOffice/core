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

#include "propertystorage.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>

#include <memory>

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    /** === end UNO using === **/

    //====================================================================
    //= PropertyStorage
    //====================================================================
    //--------------------------------------------------------------------
    PropertyStorage::~PropertyStorage()
    {
    }

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        //----------------------------------------------------------------
        template < class ITEMTYPE, class UNOTYPE_Type >
        class ItemAdapter
        {
        public:
            static bool trySet( SfxItemSet& _rSet, ItemId _nItemId, const Any& _rValue )
            {
                const SfxPoolItem& rItem( _rSet.Get( _nItemId ) );
                const ITEMTYPE* pTypedItem = dynamic_cast< const ITEMTYPE* >( &rItem );
                if ( !pTypedItem )
                    return false;

                UNOTYPE_Type aValue( pTypedItem->GetValue() );
                OSL_VERIFY( _rValue >>= aValue );
                // TODO: one could throw an IllegalArgumentException here - finally, this method
                // is (to be) used from within an XPropertySet::setPropertyValue implementation,
                // where this would be the appropriate reaction on wrong value types
                ::std::auto_ptr< ITEMTYPE > pClone( dynamic_cast< ITEMTYPE* >( pTypedItem->Clone() ) );
                pClone->SetValue( aValue );
                _rSet.Put( *pClone );
                return true;
            }

            static bool tryGet( const SfxPoolItem& _rItem, Any& _out_rValue )
            {
                const ITEMTYPE* pTypedItem = dynamic_cast< const ITEMTYPE* >( &_rItem );
                if ( !pTypedItem )
                    return false;

                _out_rValue <<= UNOTYPE_Type( pTypedItem->GetValue() );
                return true;
            }
        };
    }

    //====================================================================
    //= SetItemPropertyStorage
    //====================================================================
    //--------------------------------------------------------------------
    void SetItemPropertyStorage::getPropertyValue( Any& _out_rValue ) const
    {
        const SfxPoolItem& rItem( m_rItemSet.Get( m_nItemID ) );

        // try some known item types
        if  (   ItemAdapter< SfxBoolItem, sal_Bool >::tryGet( rItem, _out_rValue )
            ||  ItemAdapter< SfxStringItem, ::rtl::OUString >::tryGet( rItem, _out_rValue )
            )
            return;

        OSL_ENSURE( false, "SetItemPropertyStorage::getPropertyValue: unsupported item type!" );
    }

    //--------------------------------------------------------------------
    void SetItemPropertyStorage::setPropertyValue( const Any& _rValue )
    {
        // try some known item types
        if  (   ItemAdapter< SfxBoolItem, sal_Bool >::trySet( m_rItemSet, m_nItemID, _rValue )
            ||  ItemAdapter< SfxStringItem, ::rtl::OUString >::trySet( m_rItemSet, m_nItemID, _rValue )
            )
            return;

        OSL_ENSURE( false, "SetItemPropertyStorage::setPropertyValue: unsupported item type!" );
    }

//........................................................................
} // namespace dbaui
//........................................................................
