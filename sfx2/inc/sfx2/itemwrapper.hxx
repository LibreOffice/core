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

#ifndef SFX_ITEMWRAPPER_HXX
#define SFX_ITEMWRAPPER_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>

// ============================================================================

namespace sfx {

// ============================================================================
// Helpers
// ============================================================================

class SFX2_DLLPUBLIC ItemWrapperHelper
{
public:
    /** Returns the WID of the passed SID in the item set. */
    static sal_uInt16       GetWhichId( const SfxItemSet& rItemSet, sal_uInt16 nSlot );

    /** Returns true, if the passed item set supports the SID. */
    static bool         IsKnownItem( const SfxItemSet& rItemSet, sal_uInt16 nSlot );

    /** Returns an item from an item set, if it is not in "don't know" state.
        @return  Pointer to item, or 0 if it has "don't know" state. */
    static const SfxPoolItem* GetUniqueItem( const SfxItemSet& rItemSet, sal_uInt16 nSlot );

    /** Returns the default item from the pool of the passed item set. */
    static const SfxPoolItem& GetDefaultItem( const SfxItemSet& rItemSet, sal_uInt16 nSlot );

    /** Removes an item from rDestSet, if it is default in rOldSet. */
    static void         RemoveDefaultItem( SfxItemSet& rDestSet, const SfxItemSet& rOldSet, sal_uInt16 nSlot );
};

// ============================================================================
// Item wrappers
// ============================================================================

/** Base class wrapping a single item.

    Objects of this class store the SID of an item. Exchanging data with the
    item is done with the virtual functions GetItemValue() and SetItemValue().
    Derived classes implement these functions according to the item type they
    work on.

    The current tree of base classes/templates and standard item wrappers:

    SingleItemWrapper< ItemT, ValueT >
     |
     +- ValueItemWrapper< ItemT, ValueT >   [1]
     |   |
     |   +- BoolItemWrapper   [1]
     |   +- Int16ItemWrapper   [1]
     |   +- UInt16ItemWrapper   [1]
     |   +- Int32ItemWrapper   [1]
     |   +- UInt32ItemWrapper   [1]
     |   +- StringItemWrapper   [1]
     |
     +- IdentItemWrapper< ItemT >   [1]

    Notes:
    [1] Standard wrappers ready to use.

    See documentation of class ItemConnectionBase for more details.
 */
template< typename ItemT, typename ValueT >
class SingleItemWrapper
{
public:
    typedef ItemT                               ItemType;
    typedef ValueT                              ItemValueType;
    typedef SingleItemWrapper< ItemT, ValueT >  SingleItemWrapperType;

    inline explicit     SingleItemWrapper( sal_uInt16 nSlot ) : mnSlot( nSlot ) {}

    /** Returns the SID this wrapper works on. */
    inline sal_uInt16       GetSlotId() const { return mnSlot; }

    /** Returns the item from an item set, if it is not in "don't know" state.
        @descr  Similar to ItemWrapperHelper::GetUniqueItem(), but works always
        with the own SID and returns the correct item type.
        @return Pointer to item, or 0 if it has "don't know" state. */
    const ItemT*        GetUniqueItem( const SfxItemSet& rItemSet ) const;
    /** Returns the default item from the pool of the passed item set.
        @descr  Similar to ItemWrapperHelper::GetDefaultItem(), but works
        always with the own SID and returns the correct item type. */
    const ItemT&        GetDefaultItem( const SfxItemSet& rItemSet ) const;

    /** Derived classes return the value of the passed item. */
    virtual ValueT      GetItemValue( const ItemT& rItem ) const = 0;
    /** Derived classes set the value at the passed item. */
    virtual void        SetItemValue( ItemT& rItem, ValueT aValue ) const = 0;

private:
    sal_uInt16              mnSlot;     /// The SID of this item wrapper.
};

// ============================================================================

/** An item wrapper usable for most types of items.

    The item type must support the following functions:
    - ValueT ItemT::GetValue() const
    - void ItemT::SetValue( ValueT )

    The template parameter InternalValueT can be used to specify the internal
    value type of the item, if it differs from ValueT. This parameter has to be
    used to prevent compiler warnings.
 */
template< typename ItemT, typename ValueT, typename InternalValueT = ValueT >
class ValueItemWrapper : public SingleItemWrapper< ItemT, ValueT >
{
public:
    inline explicit     ValueItemWrapper( sal_uInt16 nSlot ) :
                            SingleItemWrapper< ItemT, ValueT >( nSlot ) {}

    virtual ValueT      GetItemValue( const ItemT& rItem ) const
                            { return static_cast< ValueT >( rItem.GetValue() ); }
    virtual void        SetItemValue( ItemT& rItem, ValueT aValue ) const
                            { rItem.SetValue( static_cast< InternalValueT >( aValue ) ); }
};

// ----------------------------------------------------------------------------

typedef ValueItemWrapper< SfxBoolItem,   sal_Bool >          BoolItemWrapper;
typedef ValueItemWrapper< SfxInt16Item,  sal_Int16 >         Int16ItemWrapper;
typedef ValueItemWrapper< SfxUInt16Item, sal_uInt16 >        UInt16ItemWrapper;
typedef ValueItemWrapper< SfxInt32Item,  sal_Int32 >         Int32ItemWrapper;
typedef ValueItemWrapper< SfxUInt32Item, sal_uInt32 >        UInt32ItemWrapper;
typedef ValueItemWrapper< SfxStringItem, const String& > StringItemWrapper;

// ============================================================================

/** An item wrapper that uses the item itself as value. */
template< typename ItemT >
class IdentItemWrapper : public SingleItemWrapper< ItemT, const ItemT& >
{
public:
    inline explicit     IdentItemWrapper( sal_uInt16 nSlot ) :
                            SingleItemWrapper< ItemT, const ItemT& >( nSlot ) {}

    virtual const ItemT& GetItemValue( const ItemT& rItem ) const
                            { return rItem; }
    virtual void        SetItemValue( ItemT& rItem, const ItemT& rValue ) const
                            { rItem = rValue; }
};

// ============================================================================


// ============================================================================
//               ***  Implementation of template functions  ***
// ============================================================================

// ============================================================================
// Item wrappers
// ============================================================================

template< typename ItemT, typename ValueT >
const ItemT* SingleItemWrapper< ItemT, ValueT >::GetUniqueItem( const SfxItemSet& rItemSet ) const
{
    return static_cast< const ItemT* >( ItemWrapperHelper::GetUniqueItem( rItemSet, mnSlot ) );
}

template< typename ItemT, typename ValueT >
const ItemT& SingleItemWrapper< ItemT, ValueT >::GetDefaultItem( const SfxItemSet& rItemSet ) const
{
    return static_cast< const ItemT& >( ItemWrapperHelper::GetDefaultItem( rItemSet, mnSlot ) );
}

// ============================================================================

} // namespace sfx

#endif

