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
#include <item/base/ItemControlBlock.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ItemControlBlock& getEmptyStaticItemControlBlock()
    {
        static ItemControlBlock aItemControlBlock;
        return aItemControlBlock;
    }

    // single global static instance for helper class ImplInvalidateItem
    static const ItemBase& getInvalidateItem()
    {
        // helper class for an ImplInvalidateItem - placeholder for InvaidateState
        // SfxItemState::DONTCARE -> IsInvalidItem -> pItem == INVALID_POOL_ITEM -> reinterpret_cast<SfxPoolItem*>(-1)
        class ImplInvalidateItem : public ItemBase
        {
        public:
            ImplInvalidateItem() : ItemBase(getEmptyStaticItemControlBlock()) {}
            virtual std::unique_ptr<ItemBase> clone() const { return std::make_unique<ImplInvalidateItem>(); }
        };

        static const ImplInvalidateItem aImplInvalidateItem;
        return aImplInvalidateItem;
    }

    // single global static instance for helper class ImplDisableItem
    static const ItemBase& getDisableItem()
    {
        // helper class for a ImplDisableItem - placeholder for InvaidateState
        // SfxItemState::DISABLED -> IsVoidItem() -> instance of SfxVoidItem, virtual bool IsVoidItem()
        class ImplDisableItem : public ItemBase
        {
        public:
            ImplDisableItem() : ItemBase(getEmptyStaticItemControlBlock()) {}
            virtual std::unique_ptr<ItemBase> clone() const { return std::make_unique<ImplDisableItem>(); }
        };

        static const ImplDisableItem aImplDisableItem;
        return aImplDisableItem;
    }

    const ItemBase* ItemSet::implGetStateAndItem(size_t hash_code, IState& rIState, bool bSearchParent) const
    {
        const auto aRetval(m_aItems.find(hash_code));

        if(aRetval != m_aItems.end()) // && aRetval->second)
        {
            assert(nullptr != aRetval->second && "empty const ItemBase* in ItemSet (!)");

            if(aRetval->second == &getInvalidateItem())
            {
                // SfxItemState::DONTCARE
                rIState = IState::DONTCARE;
                return nullptr;
            }

            if(aRetval->second == &getDisableItem())
            {
                // SfxItemState::DISABLED
                rIState = IState::DISABLED;
                return nullptr;
            }

            // SfxItemState::SET
            rIState = IState::SET;
            return aRetval->second;
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
        const auto aRetval(m_aItems.find(hash_code));

        if(aRetval == m_aItems.end())
        {
            m_aItems[hash_code] = &getInvalidateItem();
        }
        else
        {
            delete aRetval->second;
            aRetval->second = &getInvalidateItem();
        }
    }

    void ItemSet::implDisableItem(size_t hash_code)
    {
        const auto aRetval(m_aItems.find(hash_code));

        if(aRetval == m_aItems.end())
        {
            m_aItems[hash_code] = &getDisableItem();
        }
        else
        {
            delete aRetval->second;
            aRetval->second = &getDisableItem();
        }
    }

    bool ItemSet::implClearItem(size_t hash_code)
    {
        const auto aRetval(m_aItems.find(hash_code));

        if(aRetval != m_aItems.end())
        {
            delete aRetval->second;
            m_aItems.erase(aRetval);
            return true;
        }

        return false;
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

    void ItemSet::setParent(const ItemSet::SharedPtr& rNewParent)
    {
        m_aParent = rNewParent;
    }

    const ItemSet::SharedPtr& ItemSet::getParent() const
    {
        return m_aParent;
    }

    ItemSet::SharedPtr ItemSet::create(const ModelSpecificItemValues::SharedPtr& rModelSpecificIValues)
    {
        return ItemSet::SharedPtr(new ItemSet(rModelSpecificIValues));
    }

    const ModelSpecificItemValues::SharedPtr& ItemSet::getModelSpecificIValues() const
    {
        return m_aModelSpecificIValues;
    }

    void ItemSet::setItem(const ItemBase& rItem)
    {
        bool bDefault(false);

        // detect if rItem is default item, include evtl. model-specific
        // defaults, not only the Item-implementation single global one
        if(m_aModelSpecificIValues)
        {
            // may use model-specific default, get from helper
            // helper *will* fallback to ItemBase default
            bDefault = m_aModelSpecificIValues->isDefault(rItem);
        }
        else
        {
            // use Item's own static global default
            bDefault = rItem.isDefault();
        }

        const size_t hash_code(typeid(rItem).hash_code());

        if(bDefault)
        {
            // SfxItemState::DEFAULT is represented by not being set
            m_aItems.erase(hash_code);
        }
        else
        {
            // SfxItemState::SET
            m_aItems[hash_code] = rItem.clone().release();
        }
    }

    void ItemSet::setItems(const ItemSet& rSource, bool bDontCareToDefault)
    {
        for(const auto& candidate : rSource.m_aItems)
        {
            assert(nullptr != candidate.second && "empty const ItemBase* not allowed (!)");
            const ItemBase* pNew(nullptr);

            if(candidate.second == &getInvalidateItem())
            {
                if(bDontCareToDefault)
                {
                    // prepare setting to default -> no new value
                    // keep nullptr
                }
                else
                {
                    // prepare SfxItemState::DONTCARE
                    pNew = &getInvalidateItem();
                }
            }
            else if(candidate.second == &getDisableItem())
            {
                // prepare SfxItemState::DISABLED
                pNew = &getDisableItem();
            }
            else
            {
                // prepare SfxItemState::SET
                pNew = candidate.second->clone().release();
            }

            if(nullptr == pNew)
            {
                m_aItems.erase(candidate.first);
            }
            else
            {
                const auto aRetval(m_aItems.find(candidate.first));

                if(aRetval == m_aItems.end())
                {
                    m_aItems[candidate.first] = pNew;
                }
                else
                {
                    delete aRetval->second;
                    aRetval->second = pNew;
                }
            }
        }
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
