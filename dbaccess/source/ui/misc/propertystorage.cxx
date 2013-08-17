/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "propertystorage.hxx"

#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>

#include <memory>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;

    // PropertyStorage
    PropertyStorage::~PropertyStorage()
    {
    }

    // helper
    namespace
    {
        #undef UNOTYPE
        template < class ITEMTYPE, class UNOTYPE >
        class ItemAdapter
        {
        public:
            static bool trySet( SfxItemSet& _rSet, ItemId _nItemId, const Any& _rValue )
            {
                const SfxPoolItem& rItem( _rSet.Get( _nItemId ) );
                const ITEMTYPE* pTypedItem = dynamic_cast< const ITEMTYPE* >( &rItem );
                if ( !pTypedItem )
                    return false;

                UNOTYPE aValue( pTypedItem->GetValue() );
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

                _out_rValue <<= UNOTYPE( pTypedItem->GetValue() );
                return true;
            }
        };
    }

    // SetItemPropertyStorage
    void SetItemPropertyStorage::getPropertyValue( Any& _out_rValue ) const
    {
        const SfxPoolItem& rItem( m_rItemSet.Get( m_nItemID ) );

        // try some known item types
        if  (   ItemAdapter< SfxBoolItem, sal_Bool >::tryGet( rItem, _out_rValue )
            ||  ItemAdapter< SfxStringItem, OUString >::tryGet( rItem, _out_rValue )
            )
            return;

        OSL_FAIL( "SetItemPropertyStorage::getPropertyValue: unsupported item type!" );
    }

    void SetItemPropertyStorage::setPropertyValue( const Any& _rValue )
    {
        // try some known item types
        if  (   ItemAdapter< SfxBoolItem, sal_Bool >::trySet( m_rItemSet, m_nItemID, _rValue )
            ||  ItemAdapter< SfxStringItem, OUString >::trySet( m_rItemSet, m_nItemID, _rValue )
            )
            return;

        OSL_FAIL( "SetItemPropertyStorage::setPropertyValue: unsupported item type!" );
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
