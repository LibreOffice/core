/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertystorage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:25:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "propertystorage.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <svtools/itemset.hxx>
#include <svtools/stritem.hxx>
#include <svtools/eitem.hxx>

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
