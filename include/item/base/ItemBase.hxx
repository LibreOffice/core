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
#include <memory>
#include <vector>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    class ItemControlBlock;

    class ITEM_DLLPUBLIC ItemBase
    {
    public:
        // typedefs for PutValues/PutValue/CreateFromAny functionality
        // used from the SfxSlot mechanism (see CreateSlotItem)
        typedef std::pair<const css::uno::Any, sal_uInt8> AnyIDPair;
        typedef std::vector<AnyIDPair> AnyIDArgs;

        // PutValue/Any interface for automated instance creation from SfxType
        // mechanism (UNO API and sfx2 stuff)
        virtual void putValues(const AnyIDArgs& rArgs);

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
        virtual void putValue(const css::uno::Any& rVal, sal_uInt8 nMemberId);

    public:
        ItemBase(ItemControlBlock& rItemControlBlock);
        ItemBase(const ItemBase&);
        ItemBase& operator=(const ItemBase&);

        virtual bool operator==(const ItemBase&) const;
        bool operator!=(const ItemBase&) const;

        // default interface
        const ItemBase& getDefault() const;
        bool isDefault() const;

        // clone-op, secured by handing over a std::unique_ptr directly
        // to make explicit the ownership you get when calling this
        virtual std::unique_ptr<ItemBase> clone() const = 0;
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
