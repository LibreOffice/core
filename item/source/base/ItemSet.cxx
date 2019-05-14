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

    // helper class for an ImplInvalidateItem - placeholder for InvaidateState
    // SfxItemState::DONTCARE -> IsInvalidItem -> pItem == INVALID_POOL_ITEM -> reinterpret_cast<SfxPoolItem*>(-1)
    // instances of this class are *never* returned in any way or helper data struture,
    // but are *strictly* local. Thus also need no 'static ItemControlBlock& GetStaticItemControlBlock()' (for now)
    class ImplInvalidateItem : public ItemBase
    {
    private:
        const ItemBase& m_rItemDefault;

    public:
        ImplInvalidateItem(const ItemBase& rItemDefault)
        :   ItemBase(getEmptyStaticItemControlBlock()),
            m_rItemDefault(rItemDefault)
        {
        }

        virtual bool isInvalidateItem() const override
        {
            return true;
        }

        const ItemBase& getItemDefault() const { return m_rItemDefault; }
    };

    // helper class for a ImplDisableItem - placeholder for InvaidateState
    // SfxItemState::DISABLED -> IsVoidItem() -> instance of SfxVoidItem, virtual bool IsVoidItem()
    // instances of this class are *never* returned in any way or helper data struture,
    // but are *strictly* local. Thus also need no 'static ItemControlBlock& GetStaticItemControlBlock()' (for now)
    class ImplDisableItem : public ItemBase
    {
    private:
        const ItemBase& m_rItemDefault;

    public:
        ImplDisableItem(const ItemBase& rItemDefault)
        :   ItemBase(getEmptyStaticItemControlBlock()),
            m_rItemDefault(rItemDefault)
        {
        }

        virtual bool isDisableItem() const override
        {
            return true;
        }

        const ItemBase& getItemDefault() const { return m_rItemDefault; }
    };

    const ItemBase* ItemSet::implGetStateAndItem(size_t hash_code, IState& rIState, bool bSearchParent) const
    {
        const auto aRetval(m_aItems.find(hash_code));

        if(aRetval != m_aItems.end()) // && aRetval->second)
        {
            assert(nullptr != aRetval->second && "empty const ItemBase* in ItemSet (!)");

            if(aRetval->second->isInvalidateItem())
            {
                // SfxItemState::DONTCARE
                rIState = IState::DONTCARE;
                return &static_cast<const ImplInvalidateItem*>(aRetval->second)->getItemDefault();
            }

            if(aRetval->second->isDisableItem())
            {
                // SfxItemState::DISABLED
                rIState = IState::DISABLED;
                return &static_cast<const ImplDisableItem*>(aRetval->second)->getItemDefault();
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

    void ItemSet::implInvalidateItem(size_t hash_code, const ItemBase& rItemDefault)
    {
        const auto aRetval(m_aItems.find(hash_code));

        if(aRetval == m_aItems.end())
        {
            m_aItems[hash_code] = new ImplInvalidateItem(rItemDefault);
        }
        else
        {
            delete aRetval->second;
            aRetval->second = new ImplInvalidateItem(rItemDefault);
        }
    }

    void ItemSet::implDisableItem(size_t hash_code, const ItemBase& rItemDefault)
    {
        const auto aRetval(m_aItems.find(hash_code));

        if(aRetval == m_aItems.end())
        {
            m_aItems[hash_code] = new ImplDisableItem(rItemDefault);
        }
        else
        {
            delete aRetval->second;
            aRetval->second = new ImplDisableItem(rItemDefault);
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

            if(candidate.second->isInvalidateItem())
            {
                if(bDontCareToDefault)
                {
                    // prepare setting to default -> no new value
                    // keep nullptr
                }
                else
                {
                    // prepare SfxItemState::DONTCARE
                    pNew = new ImplInvalidateItem(static_cast<const ImplInvalidateItem*>(candidate.second)->getItemDefault());
                }
            }
            else if(candidate.second->isDisableItem())
            {
                // prepare SfxItemState::DISABLED
                pNew = new ImplDisableItem(static_cast<const ImplDisableItem*>(candidate.second)->getItemDefault());
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

    std::vector<std::pair<const ItemBase*, ItemSet::IState>> ItemSet::getAllItemsAndStates() const
    {
        std::vector<std::pair<const ItemBase*, ItemSet::IState>> aRetval;

        if(!m_aItems.empty())
        {
            aRetval.reserve(m_aItems.size());

            for(const auto& candidate : m_aItems)
            {
                if(candidate.second->isInvalidateItem())
                {
                    aRetval.push_back(std::pair<const ItemBase*, ItemSet::IState>(
                        &static_cast<const ImplInvalidateItem*>(candidate.second)->getItemDefault(),
                        IState::DONTCARE));
                }
                else if(candidate.second->isDisableItem())
                {
                    aRetval.push_back(std::pair<const ItemBase*, ItemSet::IState>(
                        &static_cast<const ImplDisableItem*>(candidate.second)->getItemDefault(),
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
            && "only IStates SET/DISBALE/DONTCALE allowed (!)");
        std::vector<const ItemBase*> aRetval;

        if(!m_aItems.empty())
        {
            aRetval.reserve(m_aItems.size());

            for(const auto& candidate : m_aItems)
            {
                if(IState::DONTCARE == eIState && candidate.second->isInvalidateItem())
                {
                    aRetval.push_back(&static_cast<const ImplInvalidateItem*>(candidate.second)->getItemDefault());
                }
                else if(IState::DISABLED == eIState && candidate.second->isDisableItem())
                {
                    aRetval.push_back(&static_cast<const ImplDisableItem*>(candidate.second)->getItemDefault());
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
