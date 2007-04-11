/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itemwrapper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:22:13 $
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

#ifndef SFX_ITEMWRAPPER_HXX
#define SFX_ITEMWRAPPER_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

// ============================================================================

namespace sfx {

// ============================================================================
// Helpers
// ============================================================================

class SFX2_DLLPUBLIC ItemWrapperHelper
{
public:
    /** Returns the WID of the passed SID in the item set. */
    static USHORT       GetWhichId( const SfxItemSet& rItemSet, USHORT nSlot );

    /** Returns true, if the passed item set supports the SID. */
    static bool         IsKnownItem( const SfxItemSet& rItemSet, USHORT nSlot );

    /** Returns an item from an item set, if it is not in "don't know" state.
        @return  Pointer to item, or 0 if it has "don't know" state. */
    static const SfxPoolItem* GetUniqueItem( const SfxItemSet& rItemSet, USHORT nSlot );

    /** Returns the default item from the pool of the passed item set. */
    static const SfxPoolItem& GetDefaultItem( const SfxItemSet& rItemSet, USHORT nSlot );

    /** Removes an item from rDestSet, if it is default in rOldSet. */
    static void         RemoveDefaultItem( SfxItemSet& rDestSet, const SfxItemSet& rOldSet, USHORT nSlot );
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

    inline explicit     SingleItemWrapper( USHORT nSlot ) : mnSlot( nSlot ) {}

    /** Returns the SID this wrapper works on. */
    inline USHORT       GetSlotId() const { return mnSlot; }

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
    USHORT              mnSlot;     /// The SID of this item wrapper.
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
    inline explicit     ValueItemWrapper( USHORT nSlot ) :
                            SingleItemWrapper< ItemT, ValueT >( nSlot ) {}

    virtual ValueT      GetItemValue( const ItemT& rItem ) const
                            { return static_cast< ValueT >( rItem.GetValue() ); }
    virtual void        SetItemValue( ItemT& rItem, ValueT aValue ) const
                            { rItem.SetValue( static_cast< InternalValueT >( aValue ) ); }
};

// ----------------------------------------------------------------------------

typedef ValueItemWrapper< SfxBoolItem,   BOOL >          BoolItemWrapper;
typedef ValueItemWrapper< SfxInt16Item,  INT16 >         Int16ItemWrapper;
typedef ValueItemWrapper< SfxUInt16Item, UINT16 >        UInt16ItemWrapper;
typedef ValueItemWrapper< SfxInt32Item,  INT32 >         Int32ItemWrapper;
typedef ValueItemWrapper< SfxUInt32Item, UINT32 >        UInt32ItemWrapper;
typedef ValueItemWrapper< SfxStringItem, const String& > StringItemWrapper;

// ============================================================================

/** An item wrapper that uses the item itself as value. */
template< typename ItemT >
class IdentItemWrapper : public SingleItemWrapper< ItemT, const ItemT& >
{
public:
    inline explicit     IdentItemWrapper( USHORT nSlot ) :
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

