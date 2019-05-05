/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <item/base/ItemAdministrator.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ItemAdministrator::ItemAdministrator(std::function<ItemBuffered::ItemData*()> aConstructItem)
    :   m_aConstructItem(aConstructItem)
    {
        assert(nullptr != m_aConstructItem && "ItemAdministrator::ItemAdministrator called without valid constructItem lambda (!)");
    }

    ItemAdministrator::~ItemAdministrator()
    {
    }

    ItemBuffered::ItemData* ItemAdministrator::find_or_set(ItemBuffered::ItemData* pItemData)
    {
        return pItemData;
    }

    void ItemAdministrator::remove(ItemBuffered::ItemData* /*pItemData*/)
    {
    }

    ItemBuffered::ItemData* ItemAdministrator::createNewDataInstance() const
    {
        return m_aConstructItem();
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ItemAdministrator_set::ItemAdministrator_set(
        std::function<ItemBuffered::ItemData*()> aConstructItem,
        std::function<bool(ItemBuffered::ItemData*, ItemBuffered::ItemData*)> aLess)
    :   ItemAdministrator(aConstructItem),
        m_aEntries(aLess)
    {
    }

    ItemBuffered::ItemData* ItemAdministrator_set::find_or_set(ItemBuffered::ItemData* pItemData)
    {
        auto ExistingEntry(m_aEntries.find(pItemData));

        if(m_aEntries.end() != ExistingEntry)
        {
            return *ExistingEntry;
        }
        else
        {
            m_aEntries.insert(pItemData);
            return pItemData;
        }
    }

    void ItemAdministrator_set::remove(ItemBuffered::ItemData* pItemData)
    {
        m_aEntries.erase(pItemData);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ItemAdministrator_unordered_set::ItemAdministrator_unordered_set(
        std::function<ItemBuffered::ItemData*()> aConstructItem,
        std::function<size_t(ItemBuffered::ItemData*)> aHash,
        std::function<bool(ItemBuffered::ItemData*, ItemBuffered::ItemData*)> aCompare)
    :   ItemAdministrator(aConstructItem),
        m_aEntries(
            0, // bucket_count -> not sure, want a system default. Will zero give me that?
            aHash,
            aCompare)
    {
    }

    ItemBuffered::ItemData* ItemAdministrator_unordered_set::find_or_set(ItemBuffered::ItemData* pItemData)
    {
        auto ExistingEntry(m_aEntries.find(pItemData));

        if(m_aEntries.end() != ExistingEntry)
        {
            return *ExistingEntry;
        }
        else
        {
            m_aEntries.insert(pItemData);
            return pItemData;
        }
    }

    void ItemAdministrator_unordered_set::remove(ItemBuffered::ItemData* pItemData)
    {
        m_aEntries.erase(pItemData);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    std::vector<ItemBuffered::ItemData*>::iterator ItemAdministrator_vector::find(ItemBuffered::ItemData* pIBase)
    {
        // find has to linearly traverse through instances and use operator==
        assert(pIBase != nullptr && "nullptr not allowed (!)");
        for(std::vector<ItemBuffered::ItemData*>::iterator candidate(m_aEntries.begin()); candidate != m_aEntries.end(); candidate++)
        {
            if(nullptr != *candidate)
            {
                if(*candidate == pIBase || m_aSame(*candidate, pIBase))
                {
                    return candidate;
                }
            }
        }

        return m_aEntries.end();
    }

    void ItemAdministrator_vector::insert(ItemBuffered::ItemData* pIBase)
    {
        // insert reuses free slots if there are some, else
        // appends to existing vector
        assert(pIBase != nullptr && "nullptr not allowed (!)");
        if(m_aFreeSlots.empty())
        {
            m_aEntries.push_back(pIBase);
            return;
        }

        const size_t freeIndex(m_aFreeSlots.back());
        m_aFreeSlots.pop_back();
        m_aEntries[freeIndex] = pIBase;
        return;
    }

    void ItemAdministrator_vector::erase(std::vector<ItemBuffered::ItemData*>::iterator& rIter)
    {
        if(rIter != m_aEntries.end())
        {
            // erase does not modify the vector's size, just resets the
            // pointer and remembers a new free slot.
            m_aFreeSlots.push_back(std::distance(m_aEntries.begin(), rIter));
            *rIter = nullptr;

            // at some useful criteria, re-organize the vector to safe space and
            // runtime for ::find
            if(m_aEntries.size() > 8
                && m_aEntries.size() > m_aFreeSlots.size()
                && m_aFreeSlots.size() * 2 > m_aEntries.size())
            {
                std::vector<ItemBuffered::ItemData*> aNewEntries;
                aNewEntries.reserve(m_aEntries.size() - m_aFreeSlots.size());

                for(std::vector<ItemBuffered::ItemData*>::iterator candidate(m_aEntries.begin()); candidate != m_aEntries.end(); candidate++)
                {
                    if(*candidate != nullptr)
                    {
                        aNewEntries.push_back(*candidate);
                    }
                }

                m_aEntries = aNewEntries;
                m_aFreeSlots.clear();
            }
            else if(m_aFreeSlots.size() == m_aEntries.size())
            {
                m_aEntries.clear();
                m_aFreeSlots.clear();
            }
        }
    }

    ItemAdministrator_vector::ItemAdministrator_vector(
        std::function<ItemBuffered::ItemData*()> aConstructItem,
        std::function<bool(ItemBuffered::ItemData*, ItemBuffered::ItemData*)> aSame)
    :   ItemAdministrator(aConstructItem),
        m_aEntries(),
        m_aFreeSlots(),
        m_aSame(aSame)
    {
    }

    ItemBuffered::ItemData* ItemAdministrator_vector::find_or_set(ItemBuffered::ItemData* pItemData)
    {
        auto ExistingEntry(find(pItemData));

        if(m_aEntries.end() != ExistingEntry)
        {
            return *ExistingEntry;
        }
        else
        {
            insert(pItemData);
            return pItemData;
        }
    }

    void ItemAdministrator_vector::remove(ItemBuffered::ItemData* pItemData)
    {
        auto iter(find(pItemData));
        erase(iter);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
