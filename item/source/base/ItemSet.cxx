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
    // single global static instance for helper class ImplInvalidateItem
    static const std::shared_ptr<const ItemBase>& getInvalidateItem()
    {
        // helper class for an ImplInvalidateItem - placeholder for InvaidateState
        // SfxItemState::DONTCARE -> IsInvalidItem -> pItem == INVALID_POOL_ITEM -> reinterpret_cast<SfxPoolItem*>(-1)
        class ImplInvalidateItem : public ItemBase
        {
        private:
            ItemControlBlock m_aItemControlBlock;
        public:
            ImplInvalidateItem() : ItemBase(m_aItemControlBlock), m_aItemControlBlock() {}
        };

        static std::shared_ptr<const ItemBase> aImplInvalidateItem(new ImplInvalidateItem());

        return aImplInvalidateItem;
    }

    // single global static instance for helper class ImplDisableItem
    static const std::shared_ptr<const ItemBase>& getDisableItem()
    {
        // helper class for a ImplDisableItem - placeholder for InvaidateState
        // SfxItemState::DISABLED -> IsVoidItem() -> instance of SfxVoidItem, virtual bool IsVoidItem()
        class ImplDisableItem : public ItemBase
        {
        private:
            ItemControlBlock m_aItemControlBlock;
        public:
            ImplDisableItem() : ItemBase(m_aItemControlBlock), m_aItemControlBlock() {}
        };

        static std::shared_ptr<const ItemBase> aImplDisableItem(new ImplDisableItem());

        return aImplDisableItem;
    }

    const ItemBase* ItemSet::implGetStateAndItem(size_t hash_code, IState& rIState, bool bSearchParent) const
    {
        const auto aRetval(m_aItems.find(hash_code));

        if(aRetval != m_aItems.end()) // && aRetval->second)
        {
            assert(aRetval->second && "empty std::shared_ptr<const ItemBase> set in ItemSet (!)");

            if(aRetval->second.get() == getInvalidateItem().get())
            {
                // SfxItemState::DONTCARE
                rIState = IState::DONTCARE;
                return nullptr;
            }

            if(aRetval->second.get() == getDisableItem().get())
            {
                // SfxItemState::DISABLED
                rIState = IState::DISABLED;
                return nullptr;
            }

            // SfxItemState::SET
            rIState = IState::SET;
            return aRetval->second.get();
        }

        // not set
        if(bSearchParent && m_aParent)
        {
            // continue searching in parent
            return m_aParent->implGetStateAndItem(hash_code, rIState, bSearchParent);
        }

        // SfxItemState::DEFAULT
        // already handed in as default - no need to set explicitely // rIState = IState::DEFAULT;
        return nullptr;
    }

    void ItemSet::implInvalidateItem(size_t hash_code)
    {
        m_aItems[hash_code] = getInvalidateItem();
    }

    void ItemSet::implDisableItem(size_t hash_code)
    {
        m_aItems[hash_code] = getDisableItem();
    }

    void ItemSet::implGetDefault(std::shared_ptr<const ItemBase>& rRetval) const
    {
        if(m_aModelSpecificIValues)
        {
            // may use model-specific default, get from helper
            // helper *will* fallback to ItemBase default
            rRetval = m_aModelSpecificIValues->GetDefault(rRetval);
        }
    }

    bool ItemSet::implClearItem(size_t hash_code)
    {
        return (0 != m_aItems.erase(hash_code));
    }

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
