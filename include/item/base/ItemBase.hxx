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

#ifndef INCLUDED_ITEM_BASE_ITEMBASE_HXX
#define INCLUDED_ITEM_BASE_ITEMBASE_HXX

#include <sal/types.h>
#include <item/itemdllapi.h>
#include <com/sun/star/uno/Any.hxx>
#include <tools/mapunit.hxx>
#include <memory>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// predefines

class IntlWrapper;
typedef struct _xmlTextWriter* xmlTextWriterPtr;

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    /*
    * The values of this enum describe the degree of textual
    * representation of an item after calling the virtual
    * method <SfxPoolItem::GetPresentation()const>.
    */
    enum class SfxItemPresentation
    {
        Nameless,
        Complete
    };

    class ItemControlBlock;

    // Baseclass for implementation of (refactored) Items. It is
    // pure virtual (do not construct ItemBase itself). There
    // are two kinds of Items now:
    // (1) SimpleItems derived from ItemBase directly
    // (2) BufferedItems derived from ItemBuffered
    // Items in the old form were buffered at the ItemPool
    // and needed as basic content the value, RefCnt, which
    // and kind - thus the size of an incarnation will be not
    // smaller than the naked item itself, the contrary. So
    // where is the reason for buffering and keeping only one
    // ref-conted instance of every possible incarnation?
    // The new Item is similar, the buffered version (2) has
    // the ItemControlBlock&, the item's value(s) itself and a
    // refCounting of some kind. Thus the consequence is:
    // As long as an Item including the ItemControlBlock& and
    // it's data is smaller than the refCnted version, do not
    // refCnt it, but profit from small footprint and simple
    // copyability - derive from (1).
    // Thus all new Items simple and small enough (ca. 3-4 'machine' words,
    // depending on 32/64bit) should use ItemBase as base (see CntInt16
    // as example)
    // They should use ItemBuffered as soon as some relevant size
    // is reached. That again will - due to the Item-incarnation
    // referencing the ItemData - also have small footprint and simple
    // copyability, but be a bit harder to implement.
    // Note: All Items can be implemented either way, the decision should
    // be made upon size of one incarnation and copyability (some are better
    // refCnted). (2) has all the tooling to do that in a unified way.
    // For examples how to implement your Item, check
    // item\test\ItemTest.cxx where for test purposes a simple two-
    // value Item is implemented both ways, as example derived adding
    // a third value, and derived to get a new ItemType for typeSafe
    // future handling.
    class ITEM_DLLPUBLIC ItemBase
    {
    public:
        // typedefs for PutValues/PutValue/CreateFromAny functionality
        // used from the SfxSlot mechanism (see CreateSlotItem)
        typedef std::pair<const css::uno::Any, sal_uInt8> AnyIDPair;
        typedef std::vector<AnyIDPair> AnyIDArgs;

        // PutValue/Any interface for automated instance creation from SfxType
        // mechanism (UNO API and sfx2 stuff)
        bool putAnyValues(const AnyIDArgs& rArgs);

        // fast identifyer for invalidate/disable state extra-Items,
        // only to be used internally for ItemSet stuff. This could be
        // done using dynamic_cast, but is faster this way. This could
        // be private or protected, but would require ItemSet to be
        // declared as friend then. May be changed, but for now, use as
        // simple public virtual bool methods
        virtual bool isInvalidateItem() const;
        virtual bool isDisableItem() const;

    private:
        // local reference to instance of ItemControlBlock for this
        // incarnation - same as type::GetStaticItemControlBlock(), but
        // this way accessible for all incarnations - at the cost of
        // one local reference
        friend bool isDefault(const ItemBase& rCandidate);
        ItemControlBlock& m_rItemControlBlock;

    protected:
        // PutValue/Any interface for automated instance creation from SfxType
        // mechanism (UNO API and sfx2 stuff)
        virtual bool putAnyValue(const css::uno::Any& rVal, sal_uInt8 nMemberId);

        // constructor for derived classes, thus protected. Derived
        // classes hand in their specific ItemControlBlock to be used
        ItemBase(ItemControlBlock& rItemControlBlock);

    public:
        virtual ~ItemBase();

        // copy constructor and assigment operator (both allowed)
        ItemBase(const ItemBase&);
        ItemBase& operator=(const ItemBase&);

        // compare, != uses it internally and thus is not virtual
        virtual bool operator==(const ItemBase&) const;
        bool operator!=(const ItemBase&) const;

        // default-value interface
        const ItemBase& getDefault() const;
        bool isDefault() const;

        // clone-op, secured by returning a std::unique_ptr to make
        // explicit the ownership you get when calling this
        std::unique_ptr<ItemBase> clone() const;

        // ca. 220 impls
        // /**  @return true if it has a valid string representation */
        virtual bool getPresentation(
            SfxItemPresentation ePresentation,
            MapUnit eCoreMetric,
            MapUnit ePresentationMetric,
            OUString &rText,
            const IntlWrapper& rIntlWrapper) const;

        // ::ScaleMetrics 18 implementations
        // used by sdr::properties::ScaleItemSet / itemSetTools only
        // used by DefaultProperties::DefaultProperties only -> probably copying SdrObjects between Writer/Draw/Impress
        virtual void scaleMetrics(long lMult, long lDiv);

        // around 250 impls for each
        virtual bool queryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const;
        // virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId );
        // -> 'virtual bool putAnyValue(const css::uno::Any& rVal, sal_uInt8 nMemberId);'

        // ca. 150 impls
        virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;
    };

    // static versions of default interface
    bool isDefault(const ItemBase& rCandidate);
    template<class T> const T& getDefault()
    {
        return static_cast<const T&>(T::GetStaticItemControlBlock().getDefault());
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////
#endif // INCLUDED_ITEM_BASE_ITEMBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
