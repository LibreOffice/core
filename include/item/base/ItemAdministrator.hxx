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

#ifndef INCLUDED_ITEM_BASE_ITEMADMINISTRATOR_HXX
#define INCLUDED_ITEM_BASE_ITEMADMINISTRATOR_HXX

#include <item/base/ItemBuffered.hxx>
#include <functional>
#include <vector>
#include <set>
#include <unordered_set>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    class ItemAdministrator
    {
    private:
        std::function<ItemBuffered::ItemData*()>    m_aConstructItem;

    protected:
    public:
        ItemAdministrator(std::function<ItemBuffered::ItemData*()> aConstructItem);
        virtual ~ItemAdministrator();

        // noncopyable
        ItemAdministrator(const ItemAdministrator&) = delete;
        ItemAdministrator& operator=(const ItemAdministrator&) = delete;

        // buffer accesses
        virtual ItemBuffered::ItemData* find_or_set(ItemBuffered::ItemData*);
        virtual void remove(ItemBuffered::ItemData*);

        // instance supplier
        ItemBuffered::ItemData* createNewDataInstance() const;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ///////////////////////////////////////////////////////////////////////////////
    // ItemAdministrator-implementation using std::set for implementation
    // which guarantees good runtime for accesses (sorted list).
    // To make the requirements clear, the operator< (or less()) has to be
    // handed over at construction as lambda or function pointer
    class ITEM_DLLPUBLIC ItemAdministrator_set : public ItemAdministrator
    {
    private:
        std::set<
            ItemBuffered::ItemData*,
            std::function<bool(ItemBuffered::ItemData*, ItemBuffered::ItemData*)>>     m_aEntries;

    protected:
    public:
        ItemAdministrator_set(
            std::function<ItemBuffered::ItemData*()> aConstructItem,
            std::function<bool(ItemBuffered::ItemData*, ItemBuffered::ItemData*)> aLess);

        // buffer accesses
        virtual ItemBuffered::ItemData* find_or_set(ItemBuffered::ItemData*);
        virtual void remove(ItemBuffered::ItemData*);
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // ItemAdministrator-implementation using std::unordered_set for
    // implementation which guarantees good runtime for accesses (hashed).
    // To make the requirements clear, the needed operators have to be
    // handed over at construction as lambdas or function pointers
    class ITEM_DLLPUBLIC ItemAdministrator_unordered_set : public ItemAdministrator
    {
    private:
        std::unordered_set<
            ItemBuffered::ItemData*,
            std::function<size_t(ItemBuffered::ItemData*)>,
            std::function<bool(ItemBuffered::ItemData*, ItemBuffered::ItemData*)>> m_aEntries;

    protected:
    public:
        ItemAdministrator_unordered_set(
            std::function<ItemBuffered::ItemData*()> aConstructItem,
            std::function<size_t(ItemBuffered::ItemData*)> aHash,
            std::function<bool(ItemBuffered::ItemData*, ItemBuffered::ItemData*)> aCompare);

        // buffer accesses
        virtual ItemBuffered::ItemData* find_or_set(ItemBuffered::ItemData*);
        virtual void remove(ItemBuffered::ItemData*);
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // ItemAdministrator-implementation using std::vector for implementation.
    // this is slow, but at least uses a 2nd vector to administer the free
    // slots in the 1st list. This is BTW what SfxItemPool does right now,
    // so this is slowest, but guarantees same speed as current implementation.
    // To make the requirements clear, the operator== (or same()) has to be
    // handed over at construction as lambda or function pointer
    class ITEM_DLLPUBLIC ItemAdministrator_vector : public ItemAdministrator
    {
    private:
        // this unsorted list is used when only operator== is available. Thus
        // this is the slowest of the available IAdministrators, ony use when
        // not avoidable or in a phase of change (SfxItem's have operator== already).
        // It uses a std::vector to hold all instances, plus a list of free slots
        // (just indices) for reuse to avoid re-organizing the vector
        std::vector<ItemBuffered::ItemData*> m_aEntries;
        std::vector<size_t> m_aFreeSlots;
        std::function<bool(ItemBuffered::ItemData*, ItemBuffered::ItemData*)> m_aSame;

        std::vector<ItemBuffered::ItemData*>::iterator find(ItemBuffered::ItemData* pIBase);
        void insert(ItemBuffered::ItemData* pIBase);
        void erase(std::vector<ItemBuffered::ItemData*>::iterator& rIter);

    public:
        ItemAdministrator_vector(
            std::function<ItemBuffered::ItemData*()> aConstructItem,
            std::function<bool(ItemBuffered::ItemData*, ItemBuffered::ItemData*)> aSame);

        // buffer accesses
        virtual ItemBuffered::ItemData* find_or_set(ItemBuffered::ItemData*);
        virtual void remove(ItemBuffered::ItemData*);
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_ITEMADMINISTRATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
