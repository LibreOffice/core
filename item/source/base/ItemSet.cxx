/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <item/base/ItemSet.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ItemSet::ItemSet(const ModelSpecificItemValues::SharedPtr& rModelSpecificIValues)
    :   std::enable_shared_from_this<ItemSet>(),
        m_aParent(),
        m_aModelSpecificIValues(rModelSpecificIValues),
        m_aItems()
    {
    }

    ItemSet::~ItemSet()
    {
    }

    void ItemSet::SetParent(const ItemSet::SharedPtr& rNewParent)
    {
        m_aParent = rNewParent;
    }

    const ItemSet::SharedPtr& ItemSet::GetParent() const
    {
        return m_aParent;
    }

    ItemSet::SharedPtr ItemSet::Create(const ModelSpecificItemValues::SharedPtr& rModelSpecificIValues)
    {
        return ItemSet::SharedPtr(new ItemSet(rModelSpecificIValues));
    }

    const ModelSpecificItemValues::SharedPtr& ItemSet::GetModelSpecificIValues() const
    {
        return m_aModelSpecificIValues;
    }

    void ItemSet::SetItem(const std::shared_ptr<const ItemBase>& rItem)
    {
        assert(rItem && "empty std::shared_ptr<const ItemBase> not allowed - and should be unable to be created (!)");
        bool bDefault(false);

        // detect if rItem is default item, include evtl. model-specific
        // defaults, not only the Item-implementation single global one
        if(m_aModelSpecificIValues)
        {
            // may use model-specific default, get from helper
            // helper *will* fallback to ItemBase default
            bDefault = m_aModelSpecificIValues->IsDefault(rItem);
        }
        else
        {
            // use Item's own static global default
            bDefault = ItemBase::IsDefault(rItem);
        }

        const size_t hash_code(typeid(*rItem).hash_code());

        if(bDefault)
        {
            // SfxItemState::DEFAULT is represented by not being set
            m_aItems.erase(hash_code);
        }
        else
        {
            // SfxItemState::SET
            m_aItems[hash_code] = rItem;
        }
    }

    void ItemSet::SetItems(const ItemSet& rSource, bool bDontCareToDefault)
    {
        for(const auto& candidate : rSource.m_aItems)
        {
            assert(candidate.second && "empty std::shared_ptr<const ItemBase> not allowed - and should be unable to be created (!)");

            if(bDontCareToDefault && candidate.second.get() == getInvalidateItem().get())
            {
                // SfxItemState::DONTCARE
                m_aItems.erase(candidate.first);
            }
            else
            {
                // SfxItemState::SET || SfxItemState::DISABLED
                m_aItems.insert(candidate);
            }
        }
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
