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
#include <o3tl/typed_flags_set.hxx>
#include <sfx2/dllapi.h>
#include <sfx2/itemwrapper.hxx>
#include <sfx2/controlwrapper.hxx>
#include <svl/itemset.hxx>
#include <memory>


enum class ItemConnFlags
{
/** No special state for the connection. */
    NONE               = 0x0000,
/** Hide control(s), if the item is unknown. */
    HideUnknown        = 0x0001,
};
namespace o3tl
{
    template<> struct typed_flags<ItemConnFlags> : is_typed_flags<ItemConnFlags, 0x0001> {};
}

namespace sfx {

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
     |   +- NumericConnection< ItemWrpT >   [1]
     |   |   |
     |   |   +- [ValueType]NumericConnection   [1] [2]
     |   |
     |   +- ListBoxConnection< ItemWrpT >   [1]
     |       |
     |       +- [ValueType]ListBoxConnection   [1] [2]
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

    /** Calls the virtual ApplyFlags() function, if connection is active. */
    void                DoApplyFlags( const SfxItemSet* pItemSet );
    /** Calls the virtual Reset() function, if connection is active. */
    void                DoReset( const SfxItemSet* pItemSet );
    /** Calls the virtual FillItemSet() function, if connection is active. */
    bool                DoFillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet );

protected:
    explicit            ItemConnectionBase( ItemConnFlags nFlags );

    /** Derived classes implement actions according to current flags here. */
    virtual void        ApplyFlags( const SfxItemSet* pItemSet ) = 0;
    /** Derived classes implement initializing controls from item sets here. */
    virtual void        Reset( const SfxItemSet* pItemSet ) = 0;
    /** Derived classes implement filling item sets from controls here. */
    virtual bool        FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet ) = 0;

    /** Returns whether to show a control, according to current flags. */
    TriState            GetShowState( bool bKnown ) const;

private:
                        ItemConnectionBase( const ItemConnectionBase& ) = delete;
    ItemConnectionBase& operator=( const ItemConnectionBase& ) = delete;

    ItemConnFlags       mnFlags;    /// Flags for additional options.
};

} // namespace sfx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
