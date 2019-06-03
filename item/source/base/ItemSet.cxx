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
    // single global static instance for helper class ImplInvalidateItem
    static const ItemBase& getInvalidateItem()
    {
        // helper class for an ImplInvalidateItem - placeholder for InvaidateState
        // SfxItemState::DONTCARE -> IsInvalidItem -> pItem == INVALID_POOL_ITEM -> reinterpret_cast<SfxPoolItem*>(-1)
        // the instance of this class is *never* returned in any way or helper data struture,
        // but is *strictly* local
        class ImplInvalidateItem : public ItemBase
        {
        public:
            static ItemControlBlock& GetStaticItemControlBlock()
            {
                static ItemControlBlock aItemControlBlock(
                    [](){ return new ImplInvalidateItem(); },
                    [](const ItemBase& rRef){ return new ImplInvalidateItem(static_cast<const ImplInvalidateItem&>(rRef)); },
                    "ImplInvalidateItem");

                return aItemControlBlock;
            }

            ImplInvalidateItem()
            :   ItemBase(ImplInvalidateItem::GetStaticItemControlBlock())
            {
            }
        };

        static const ImplInvalidateItem aImplInvalidateItem;
        return aImplInvalidateItem;
    }

    // single global static instance for helper class ImplDisableItem
    static const ItemBase& getDisableItem()
    {
        // helper class for a ImplDisableItem - placeholder for InvaidateState
        // SfxItemState::DISABLED -> IsVoidItem() -> instance of SfxVoidItem, virtual bool IsVoidItem()
        // the instance of this class is *never* returned in any way or helper data struture,
        // but is *strictly* local
        class ImplDisableItem : public ItemBase
        {
        public:
            static ItemControlBlock& GetStaticItemControlBlock()
            {
                static ItemControlBlock aItemControlBlock(
                    [](){ return new ImplDisableItem(); },
                    [](const ItemBase& rRef){ return new ImplDisableItem(static_cast<const ImplDisableItem&>(rRef)); },
                    "ImplDisableItem");

                return aItemControlBlock;
            }

            ImplDisableItem()
            :   ItemBase(ImplDisableItem::GetStaticItemControlBlock())
            {
            }
        };

        static const ImplDisableItem aImplDisableItem;
        return aImplDisableItem;
    }

    const ItemBase* ItemSet::implGetStateAndItem(const ItemControlBlock& rICB, IState& rIState, bool bSearchParent) const
    {
        const auto aEntry(m_aItems.find(&rICB));

        if(aEntry != m_aItems.end()) // && aEntry->second)
        {
            assert(nullptr != aEntry->second && "empty const ItemBase* in ItemSet (!)");

            if(aEntry->second == &getInvalidateItem())
            {
                // SfxItemState::DONTCARE
                rIState = IState::DONTCARE;
                return &implGetDefault(aEntry->first->getDefault());
            }

            if(aEntry->second == &getDisableItem())
            {
                // SfxItemState::DISABLED
                rIState = IState::DISABLED;
                return &implGetDefault(aEntry->first->getDefault());
            }

            // SfxItemState::SET
            rIState = IState::SET;
            return aEntry->second;
        }

        // not set locally
        const ItemBase* pRetval(nullptr);

        if(bSearchParent && m_aParent)
        {
            // continue searching in parent
            pRetval = m_aParent->implGetStateAndItem(rICB, rIState, bSearchParent);
        }

        if(nullptr == pRetval)
        {
            // SfxItemState::DEFAULT
            pRetval = &implGetDefault(rICB.getDefault());
        }

        return pRetval;
    }

    void ItemSet::implInvalidateItem(const ItemControlBlock& rICB)
    {
        const auto aRetval(m_aItems.find(&rICB));

        if(aRetval == m_aItems.end())
        {
            m_aItems[&rICB] = &getInvalidateItem();
        }
        else
        {
            if(&getInvalidateItem() != aRetval->second && &getDisableItem() != aRetval->second)
            {
                delete aRetval->second;
            }

            aRetval->second = &getInvalidateItem();
        }
    }

    void ItemSet::implDisableItem(const ItemControlBlock& rICB)
    {
        const auto aRetval(m_aItems.find(&rICB));

        if(aRetval == m_aItems.end())
        {
            m_aItems[&rICB] = &getDisableItem();
        }
        else
        {
            if(&getInvalidateItem() != aRetval->second && &getDisableItem() != aRetval->second)
            {
                delete aRetval->second;
            }

            aRetval->second = &getDisableItem();
        }
    }

    bool ItemSet::implClearItem(const ItemControlBlock& rICB)
    {
        const auto aRetval(m_aItems.find(&rICB));

        if(aRetval != m_aItems.end())
        {
            if(&getInvalidateItem() != aRetval->second && &getDisableItem() != aRetval->second)
            {
                delete aRetval->second;
            }

            m_aItems.erase(aRetval);
            return true;
        }

        return false;
    }

    const ItemBase& ItemSet::implGetDefault(const ItemBase& rCurrent) const
    {
        if(m_aModelSpecificIValues)
        {
            // may use model-specific default, get from helper
            // helper *will* fallback to ItemBase default
            return m_aModelSpecificIValues->getDefault(rCurrent);
        }

        return rCurrent;
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

        const ItemControlBlock& rICB(rItem.getItemControlBlock());

        if(bDefault)
        {
            // SfxItemState::DEFAULT is represented by not being set
            m_aItems.erase(&rICB);
        }
        else
        {
            // SfxItemState::SET
            const auto aEntry(m_aItems.find(&rICB));

            if(aEntry == m_aItems.end())
            {
                m_aItems[&rICB] = rItem.clone().release();
            }
            else
            {
                if(&getInvalidateItem() != aEntry->second && &getDisableItem() != aEntry->second)
                {
                    delete aEntry->second;
                }

                aEntry->second = rItem.clone().release();
            }
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
                implClearItem(*candidate.first);
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
                    if(&getInvalidateItem() != aRetval->second && &getDisableItem() != aRetval->second)
                    {
                        delete aRetval->second;
                    }

                    aRetval->second = pNew;
                }
            }
        }
    }

    std::vector<std::pair<const ItemBase*, ItemSet::IState>> ItemSet::getAllItemsAndStates() const
    {
        std::vector<std::pair<const ItemBase*, ItemSet::IState>> aRetval;

        if(!m_aItems.empty())
        {
            aRetval.reserve(m_aItems.size());

            for(const auto& candidate : m_aItems)
            {
                if(candidate.second == &getInvalidateItem())
                {
                    aRetval.push_back(std::pair<const ItemBase*, ItemSet::IState>(
                        &implGetDefault(candidate.first->getDefault()),
                        IState::DONTCARE));
                }
                else if(candidate.second == &getDisableItem())
                {
                    aRetval.push_back(std::pair<const ItemBase*, ItemSet::IState>(
                        &implGetDefault(candidate.first->getDefault()),
                        IState::DISABLED));
                }
                else
                {
                    aRetval.push_back(std::pair<const ItemBase*, ItemSet::IState>(
                        candidate.second,
                        IState::SET));
                }
            }
        }

        return aRetval;
    }

    std::vector<const ItemBase*> ItemSet::getItemsOfState(IState eIState) const
    {
        assert((IState::SET == eIState || IState::DISABLED == eIState || IState::DONTCARE == eIState)
            && "only IStates SET/DISBALE/DONTCARE allowed (!)");
        std::vector<const ItemBase*> aRetval;

        if(!m_aItems.empty())
        {
            aRetval.reserve(m_aItems.size());

            for(const auto& candidate : m_aItems)
            {
                if(IState::DONTCARE == eIState && candidate.second == &getInvalidateItem())
                {
                    aRetval.push_back(&implGetDefault(candidate.first->getDefault()));
                }
                else if(IState::DISABLED == eIState && candidate.second == &getDisableItem())
                {
                    aRetval.push_back(&implGetDefault(candidate.first->getDefault()));
                }
                else if(IState::SET == eIState)
                {
                    aRetval.push_back(candidate.second);
                }
            }
        }

        return aRetval;
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
