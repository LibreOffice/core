/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

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

        OSL_FAIL( "SetItemPropertyStorage::getPropertyValue: unsupported item type!" );
    }

    //--------------------------------------------------------------------
    void SetItemPropertyStorage::setPropertyValue( const Any& _rValue )
    {
        // try some known item types
        if  (   ItemAdapter< SfxBoolItem, sal_Bool >::trySet( m_rItemSet, m_nItemID, _rValue )
            ||  ItemAdapter< SfxStringItem, ::rtl::OUString >::trySet( m_rItemSet, m_nItemID, _rValue )
            )
            return;

        OSL_FAIL( "SetItemPropertyStorage::setPropertyValue: unsupported item type!" );
    }

//........................................................................
} // namespace dbaui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
