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

#ifndef INCLUDED_SFX2_ITEMCONNECT_HXX
#define INCLUDED_SFX2_ITEMCONNECT_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>

#include <memory>

#include <sfx2/itemwrapper.hxx>
#include <sfx2/controlwrapper.hxx>

namespace sfx {

typedef int ItemConnFlags;

/** No special state for the connection. */
const ItemConnFlags ITEMCONN_NONE               = 0x0000;

/** Connection is inactive - virtual functions will not be called. */
const ItemConnFlags ITEMCONN_INACTIVE           = 0x0001;

/** Enable control(s), if the item is known. */
const ItemConnFlags ITEMCONN_ENABLE_KNOWN       = 0x0010;
/** Disable control(s), if the item is unknown. */
const ItemConnFlags ITEMCONN_DISABLE_UNKNOWN    = 0x0020;
/** Show control(s), if the item is known. */
const ItemConnFlags ITEMCONN_SHOW_KNOWN         = 0x0040;
/** Hide control(s), if the item is unknown. */
const ItemConnFlags ITEMCONN_HIDE_UNKNOWN       = 0x0080;

/** Default value for constructors. */
const ItemConnFlags ITEMCONN_DEFAULT            = ITEMCONN_NONE;


// Base connection classes


/** A helper for SfxTabPages to connect controls to items.

    This is the base class of all control connection classes. Their purpose is
    to connect one or more controls from an SfxTabPage with an item from an
    item set. The goal is to omit any additional code in the virtual functions
    Reset() and FillItemSet() in classes derived from SfxTabPage.

    Examples of connections:
    - A check box with an SfxBoolItem,
    - A metric (spin) field with an SfxInt32Item.
    - A group of radio buttons with an SfxEnumItem.

    Each SfxTabPage will contain a list of connection objects (derived from
    this class). The connection objects remember the item and control(s) they
    have to work on. The SfxTabPage will call the DoApplyFlags(), DoReset(),
    and DoFillItemSet() functions of all connection objects it knows. The code
    to initialize control(s) from the item value and fill the item from
    control(s) has to be written only once for each control type.

    Additional flags passed in the constructor allow to control the behaviour
    of the control(s) if the item is supported/unsupported in the currently
    used item set. For example, it is possible to specify that a control will
    be disabled or hidden if the item is not supported. This is done before
    each call of Reset().

    The special flag ITEMCONN_CLONE_ITEM controls how to create new items in
    the DoFillItemSet() function. The standard (and faster) method is to create
    a temporary item on the stack and put it into the item set. But this does
    not work if the item set expects a special item type derived from a common
    item class, i.e. a Boolean item derived from SfxBoolItem providing special
    item representation text. As this code does not know the item type, the
    item cannot be created on the stack. For this case the flag specifies to
    use the virtual Clone() method of the pool default item. This will create
    an item of the correct type but can still be used in conjunction with i.e.
    the standard BoolItemWrapper.

    How to use the item connection feature:

    A)  Single item <-> single control connection

        Example: An SfxBoolItem and a check box.

        A1) Create a new item wrapper class derived from the SingleItemWrapper
            template, or use the template directly, or use one of the
            predefined item wrappers. See documentation of the
            SingleItemWrapper template for details (itemwrapper.hxx).
        A2) Create a new control wrapper class derived from the
            SingleControlWrapper template and implement the abstract functions,
            or use one of the predefined control wrappers. See documentation of
            the SingleControlWrapper template for details (controlwrapper.hxx).
        A3) Create a new connection class derived from one of the following
            base classes, and implement the abstract functions, or use the
            ItemControlConnection template directly, or use one of the
            predefined connections.
        A4) Create connection objects in the constructor of the tab page, and
            insert them into the tab page with SfxTabPage::AddItemConnection().
        A5) Remove old code from the tab page's Reset() and FillItemSet()
            functions, if necessary.

    B)  Single item <-> multiple controls connections

        B1) See step A1. If the item contains multiple values (and not a
            structure that contains all the values for the different controls),
            the best way is to use the IdentItemWrapper template, that works
            with the item itself. This way it is possible to provide a 'data
            type' that contains the values for all controls.
        B2) Create a new control wrapper class derived from the
            MultiControlWrapper template. Add single control wrapper members
            for all controls to this class and register them in the
            constructor, using the RegisterControlWrapper() function. Implement
            the abstract functions GetControlValue() and SetControlValue().
            These functions should call the respective functions of the own
            single control wrappers and either fill a new data object (the item
            itself in most cases, see step B1) with all the values from the
            controls, or fill all the controls from the data object.
        B3) Create a new connection class derived from ItemControlConnection,
            or use the ItemControlConnection template directly. The multiple
            control wrapper from step B2 acts like a single control, therefore
            it is possible to use the ItemControlConnection.
        B4) See steps A4 and A5.

    C)  Multiple items <-> single control connections

        todo

    D)  Multiple items <-> multiple controls connections

        todo

    The current tree of base classes/templates and standard connections:

    ItemConnectionBase
     |
     +- DummyItemConnection   [1]
     |
     +- ItemControlConnection< ItemWrpT, ControlWrpT >
     |   |
     |   +- CheckBoxConnection   [1]
     |   |
     |   +- NumericConnection< ItemWrpT >   [1]
     |   |   |
     |   |   +- [ValueType]NumericConnection   [1] [2]
     |   |
     |   +- MetricConnection< ItemWrpT >   [1]
     |   |   |
     |   |   +- [ValueType]MetricConnection   [1] [2]
     |   |
     |   +- ListBoxConnection< ItemWrpT >   [1]
     |   |   |
     |   |   +- [ValueType]ListBoxConnection   [1] [2]
     |   |
     |   +- ValueSetConnection< ItemWrpT >   [1]
     |       |
     |       +- [ValueType]ValueSetConnection   [1] [2]
     |
     +- ItemConnectionArray   [1]

    Notes:
    [1] Standard connections ready to use.
    [2] [ValueType] is one of Int16, UInt16, Int32, UInt32.
 */
class SFX2_DLLPUBLIC ItemConnectionBase
{
public:
    virtual             ~ItemConnectionBase();

    /** Returns true if this connection is active. */
    bool                IsActive() const;

    /** Calls the virtual ApplyFlags() function, if connection is active. */
    void                DoApplyFlags( const SfxItemSet& rItemSet );
    /** Calls the virtual Reset() function, if connection is active. */
    void                DoReset( const SfxItemSet& rItemSet );
    /** Calls the virtual FillItemSet() function, if connection is active. */
    bool                DoFillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet );

protected:
    explicit            ItemConnectionBase( ItemConnFlags nFlags = ITEMCONN_DEFAULT );

    /** Derived classes implement actions according to current flags here. */
    virtual void        ApplyFlags( const SfxItemSet& rItemSet ) = 0;
    /** Derived classes implement initializing controls from item sets here. */
    virtual void        Reset( const SfxItemSet& rItemSet ) = 0;
    /** Derived classes implement filling item sets from controls here. */
    virtual bool        FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet ) = 0;

    /** Returns whether to enable a control, according to current flags. */
    TriState            GetEnableState( bool bKnown ) const;
    /** Returns whether to show a control, according to current flags. */
    TriState            GetShowState( bool bKnown ) const;

private:
                        ItemConnectionBase( const ItemConnectionBase& ) = delete;
    ItemConnectionBase& operator=( const ItemConnectionBase& ) = delete;

    ItemConnFlags       mnFlags;    /// Flags for additional options.
};



/** Base class template for single item <-> single control connection objects.

    This template uses functions provided by the SingleItemWrapper and the
    SingleControlWrapper template classes. The virtual functions ApplyFlags(),
    Reset(), and FillItemSet() are implemented here in a generic way using the
    virtual functions of the wrapper classes. Derived classes only have to
    create or otherwise provide appropriate wrappers.
 */
template< typename ItemWrpT, typename ControlWrpT >
class ItemControlConnection : public ItemConnectionBase
{
public:
    typedef ItemWrpT                                        ItemWrapperType;
    typedef ControlWrpT                                     ControlWrapperType;
    typedef ItemControlConnection< ItemWrpT, ControlWrpT >  ItemControlConnectionType;
    typedef typename ItemWrpT::ItemType                     ItemType;
    typedef typename ItemWrpT::ItemValueType                ItemValueType;
    typedef typename ControlWrpT::ControlType               ControlType;
    typedef typename ControlWrpT::ControlValueType          ControlValueType;

    typedef std::unique_ptr< ItemWrpT >                     ItemWrapperRef;
    typedef std::unique_ptr< ControlWrpT >                  ControlWrapperRef;

    /** Receives pointer to a newly created control wrapper.
        @descr  Takes ownership of the control wrapper. */
    explicit            ItemControlConnection( sal_uInt16 nSlot, ControlWrpT* pNewCtrlWrp,
                            ItemConnFlags nFlags = ITEMCONN_DEFAULT );

    /** Convenience constructor. Receives reference to a control directly.
        @descr  May only be used, if ControlWrpT::ControlWrpT( ControlType& )
        constructor exists. */
    explicit            ItemControlConnection( sal_uInt16 nSlot, ControlType& rControl,
                            ItemConnFlags nFlags = ITEMCONN_DEFAULT );

    virtual             ~ItemControlConnection();

protected:
    /** Actions according to current flags for the control. */
    virtual void        ApplyFlags( const SfxItemSet& rItemSet ) SAL_OVERRIDE;
    /** Resets the control according to the item contents. */
    virtual void        Reset( const SfxItemSet& rItemSet ) SAL_OVERRIDE;
    /** Fills the item set according to the control's state. */
    virtual bool        FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet ) SAL_OVERRIDE;

    ItemWrapperType     maItemWrp;
    ControlWrapperRef   mxCtrlWrp;
};


// Standard connections


/** This is a helper class to enable/disable/show/hide a control only.

    This class does nothing special in the Reset() and FillItemSet() functions.
    It can be used to control the visibility of i.e. fixed lines or fixed texts
    related to the availability of an item by passing the appropriate flags to
    the constructor of this connection.
 */
class SFX2_DLLPUBLIC DummyItemConnection:
    public ItemConnectionBase, public DummyWindowWrapper
{
public:
    explicit            DummyItemConnection( sal_uInt16 nSlot, vcl::Window& rWindow,
                            ItemConnFlags nFlags = ITEMCONN_DEFAULT );

protected:
    virtual void        ApplyFlags( const SfxItemSet& rItemSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rItemSet ) SAL_OVERRIDE;
    virtual bool        FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet ) SAL_OVERRIDE;

private:
    sal_uInt16              mnSlot;
};



/** Connection between an SfxBoolItem and a VCL CheckBox. */
typedef ItemControlConnection< BoolItemWrapper, CheckBoxWrapper > CheckBoxConnection;



/** Connection between an item and the VCL NumericField. */
template< typename ItemWrpT >
class NumericConnection : public ItemControlConnection< ItemWrpT,
        NumericFieldWrapper< typename ItemWrpT::ItemValueType > >
{
    typedef ItemControlConnection< ItemWrpT,
        NumericFieldWrapper< typename ItemWrpT::ItemValueType > >
    ItemControlConnectionType;

public:
    typedef typename ItemControlConnectionType::ControlWrapperType NumericFieldWrapperType;

    explicit            NumericConnection( sal_uInt16 nSlot, NumericField& rField,
                            ItemConnFlags nFlags = ITEMCONN_DEFAULT );
};



typedef NumericConnection< UInt16ItemWrapper > UInt16NumericConnection;
typedef NumericConnection< UInt32ItemWrapper > UInt32NumericConnection;



/** Connection between an item and the VCL MetricField.

    Adds support of different field units during control value <-> item value
    conversion. The field unit passed to the constructor applies for the item
    values, while the field unit used in the control has to be set at the
    control itself.
 */
template< typename ItemWrpT >
class MetricConnection : public ItemControlConnection< ItemWrpT,
        MetricFieldWrapper< typename ItemWrpT::ItemValueType > >
{
    typedef ItemControlConnection< ItemWrpT,
        MetricFieldWrapper< typename ItemWrpT::ItemValueType > >
    ItemControlConnectionType;

public:
    typedef typename ItemControlConnectionType::ControlWrapperType MetricFieldWrapperType;

    explicit            MetricConnection( sal_uInt16 nSlot, MetricField& rField,
                            FieldUnit eItemUnit = FUNIT_NONE, ItemConnFlags nFlags = ITEMCONN_DEFAULT );
};



typedef MetricConnection< UInt16ItemWrapper > UInt16MetricConnection;
typedef MetricConnection< UInt32ItemWrapper > UInt32MetricConnection;



/** Connection between an item and a VCL ListBox.

    Optionally a map can be passed that maps list box positions to item values.
    This map MUST be terminated with an entry containing
    WRAPPER_LISTBOX_ENTRY_NOTFOUND as list box position. The item value
    contained in this last entry is used as default item value in case of an
    error.
 */
template< typename ItemWrpT >
class ListBoxConnection : public ItemControlConnection< ItemWrpT,
        ListBoxWrapper< typename ItemWrpT::ItemValueType > >
{
    typedef ItemControlConnection< ItemWrpT,
        ListBoxWrapper< typename ItemWrpT::ItemValueType > >
    ItemControlConnectionType;

public:
    typedef typename ItemControlConnectionType::ControlWrapperType  ListBoxWrapperType;
    typedef typename ListBoxWrapperType::MapEntryType               MapEntryType;

    explicit            ListBoxConnection( sal_uInt16 nSlot, ListBox& rListBox,
                            const MapEntryType* pMap = 0, ItemConnFlags nFlags = ITEMCONN_DEFAULT );
};



typedef ListBoxConnection< UInt16ItemWrapper > UInt16ListBoxConnection;
typedef ListBoxConnection< UInt32ItemWrapper > UInt32ListBoxConnection;



/** Connection between an item and an SVTOOLS ValueSet.

    Optionally a map can be passed that maps value set identifiers to item
    values. This map MUST be terminated with an entry containing
    WRAPPER_VALUESET_ITEM_NOTFOUND as value set identifier. The item value
    contained in this last entry is used as default item value in case of an
    error.
 */
template< typename ItemWrpT >
class ValueSetConnection : public ItemControlConnection< ItemWrpT,
        ValueSetWrapper< typename ItemWrpT::ItemValueType > >
{
    typedef ItemControlConnection< ItemWrpT,
        ValueSetWrapper< typename ItemWrpT::ItemValueType > >
    ItemControlConnectionType;

public:
    typedef typename ItemControlConnectionType::ControlWrapperType  ValueSetWrapperType;
    typedef typename ValueSetWrapperType::MapEntryType              MapEntryType;

    explicit            ValueSetConnection( sal_uInt16 nSlot, ValueSet& rValueSet,
                            const MapEntryType* pMap = 0, ItemConnFlags nFlags = ITEMCONN_DEFAULT );
};



typedef ValueSetConnection< UInt16ItemWrapper > UInt16ValueSetConnection;
typedef ValueSetConnection< UInt32ItemWrapper > UInt32ValueSetConnection;


// Array of connections


class ItemConnectionArrayImpl;

/** A container of connection objects.

    This is a connection with the only purpose to contain other connection
    objects. This way it is possible to create a tree structure of connections
    for a convenient connection management. This class is used by the class
    SfxTabPage to store all connections.
 */
class ItemConnectionArray : public ItemConnectionBase
{
public:
    explicit            ItemConnectionArray();
    virtual             ~ItemConnectionArray();

    /** Adds a new connection to the list.
        @descr  Takes ownership of the connection! */
    void                AddConnection( ItemConnectionBase* pConnection );

protected:
    virtual void        ApplyFlags( const SfxItemSet& rItemSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rItemSet ) SAL_OVERRIDE;
    virtual bool        FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet ) SAL_OVERRIDE;

private:
    std::unique_ptr< ItemConnectionArrayImpl > mxImpl;
};




//               ***  Implementation of template functions  ***



// Base connection classes


template< typename ItemWrpT, typename ControlWrpT >
ItemControlConnection< ItemWrpT, ControlWrpT >::ItemControlConnection(
        sal_uInt16 nSlot, ControlWrpT* pNewCtrlWrp, ItemConnFlags nFlags ) :
    ItemConnectionBase( nFlags ),
    maItemWrp( nSlot ),
    mxCtrlWrp( pNewCtrlWrp )
{
}

template< typename ItemWrpT, typename ControlWrpT >
ItemControlConnection< ItemWrpT, ControlWrpT >::ItemControlConnection(
        sal_uInt16 nSlot, ControlType& rControl, ItemConnFlags nFlags ) :
    ItemConnectionBase( nFlags ),
    maItemWrp( nSlot ),
    mxCtrlWrp( new ControlWrpT( rControl ) )
{
}

template< typename ItemWrpT, typename ControlWrpT >
ItemControlConnection< ItemWrpT, ControlWrpT >::~ItemControlConnection()
{
}

template< typename ItemWrpT, typename ControlWrpT >
void ItemControlConnection< ItemWrpT, ControlWrpT >::ApplyFlags( const SfxItemSet& rItemSet )
{
    bool bKnown = ItemWrapperHelper::IsKnownItem( rItemSet, maItemWrp.GetSlotId() );
    mxCtrlWrp->ModifyControl( GetEnableState( bKnown ), GetShowState( bKnown ) );
}

template< typename ItemWrpT, typename ControlWrpT >
void ItemControlConnection< ItemWrpT, ControlWrpT >::Reset( const SfxItemSet& rItemSet )
{
    const ItemType* pItem = maItemWrp.GetUniqueItem( rItemSet );
    mxCtrlWrp->SetControlDontKnow( pItem == 0 );
    if( pItem )
        mxCtrlWrp->SetControlValue( maItemWrp.GetItemValue( *pItem ) );
}

template< typename ItemWrpT, typename ControlWrpT >
bool ItemControlConnection< ItemWrpT, ControlWrpT >::FillItemSet(
        SfxItemSet& rDestSet, const SfxItemSet& rOldSet )
{
    const ItemType* pOldItem = maItemWrp.GetUniqueItem( rOldSet );
    bool bChanged = false;
    if( !mxCtrlWrp->IsControlDontKnow() )
    {
        // first store the control value in a local variable
        ControlValueType aCtrlValue( mxCtrlWrp->GetControlValue() );
        // convert to item value type -> possible to convert i.e. from 'T' to 'const T&'
        ItemValueType aNewValue( aCtrlValue );
        // do not rely on existence of ItemValueType::operator!=
        if( !pOldItem || !(maItemWrp.GetItemValue( *pOldItem ) == aNewValue) )
        {
            sal_uInt16 nWhich = ItemWrapperHelper::GetWhichId( rDestSet, maItemWrp.GetSlotId() );
            std::unique_ptr< ItemType > xItem(
                static_cast< ItemType* >( maItemWrp.GetDefaultItem( rDestSet ).Clone() ) );
            xItem->SetWhich( nWhich );
            maItemWrp.SetItemValue( *xItem, aNewValue );
            rDestSet.Put( *xItem );
            bChanged = true;
        }
    }
    if( !bChanged )
        ItemWrapperHelper::RemoveDefaultItem( rDestSet, rOldSet, maItemWrp.GetSlotId() );
    return bChanged;
}


// Standard connections


template< typename ItemWrpT >
NumericConnection< ItemWrpT >::NumericConnection(
        sal_uInt16 nSlot, NumericField& rField, ItemConnFlags nFlags ) :
    ItemControlConnectionType( nSlot, rField, nFlags )
{
}



template< typename ItemWrpT >
MetricConnection< ItemWrpT >::MetricConnection(
        sal_uInt16 nSlot, MetricField& rField, FieldUnit eItemUnit, ItemConnFlags nFlags ) :
    ItemControlConnectionType( nSlot, new MetricFieldWrapperType( rField, eItemUnit ), nFlags )
{
}



template< typename ItemWrpT >
ListBoxConnection< ItemWrpT >::ListBoxConnection(
        sal_uInt16 nSlot, ListBox& rListBox, const MapEntryType* pMap, ItemConnFlags nFlags ) :
    ItemControlConnectionType( nSlot, new ListBoxWrapperType( rListBox, pMap ), nFlags )
{
}



template< typename ItemWrpT >
ValueSetConnection< ItemWrpT >::ValueSetConnection(
        sal_uInt16 nSlot, ValueSet& rValueSet, const MapEntryType* pMap, ItemConnFlags nFlags ) :
    ItemControlConnectionType( nSlot, new ValueSetWrapperType( rValueSet, pMap ), nFlags )
{
}



} // namespace sfx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
