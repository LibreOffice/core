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
    void SetAdministratedFromItemAdministrator(ItemBase& rIBase)
    {
        rIBase.m_bAdministrated = true;
    }

    ItemAdministrator::ItemAdministrator(const ItemBase* pDefault)
    :   m_aDefault(ItemBase::SharedPtr(pDefault))
    {
        assert(pDefault != nullptr && "nullptr not allowed, default *is* required (!)");
    }

    ItemAdministrator::~ItemAdministrator()
    {
    }

    void ItemAdministrator::HintExpired(const ItemBase* /*pIBase*/)
    {
        // Default does nothing and will be triggered from m_aDefault being destructed
        // using the HintExpired calls in the administrated classes. This happens due to
        // m_aDefault being destroyed from ~ItemAdministrator() above as last thing in
        // ItemAdministrator cleanup, the derived classes are no longer available at that time.
    }

    const ItemBase::SharedPtr& ItemAdministrator::GetDefault() const
    {
        return m_aDefault;
    }

    bool ItemAdministrator::IsDefault(const ItemBase* pIBase) const
    {
        assert(pIBase != nullptr && "nullptr not allowed (!)");
        return pIBase == m_aDefault.get() || pIBase->operator==(*m_aDefault.get());
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    IAdministrator_set::IAdministrator_set(const ItemBase* pDefault)
    :   ItemAdministrator(pDefault),
        m_aEntries()
    {
    }

    ItemBase::SharedPtr IAdministrator_set::Create(const ItemBase* pIBase)
    {
        assert(pIBase != nullptr && "nullptr not allowed (!)");

        if(IsDefault(pIBase))
        {
            // if the Item to-be-created equals default, delete it and
            // use the existing single global default
            delete pIBase;
            return GetDefault();
        }

        // check for existance
        auto ExistingEntry(m_aEntries.find(pIBase));

        if(m_aEntries.end() != ExistingEntry)
        {
            // if the Item to-be-created exists already, delete it and
            // use the existing instance
            delete pIBase;
            return (*ExistingEntry)->shared_from_this();
        }
        else
        {
            // start using offered instance and administrate it from now
            SetAdministratedFromItemAdministrator(*const_cast<ItemBase*>(pIBase));
            m_aEntries.insert(pIBase);
            return ItemBase::SharedPtr(pIBase);
        }
    }

    void IAdministrator_set::HintExpired(const ItemBase* pIBase)
    {
        // called from ::~Item. This happens in two cases:
        // (1) a temporary Item instance gets deleted
        // (2) last shared_ptr was deleted
        // The caller should have already checked the
        // Administrated-flag, so only administrated instances
        // of ItemBase should be handled here which is case (2)
        assert(pIBase != nullptr && "nullptr not allowed (!)");
        assert(pIBase->IsAdministrated() && "call only for administrated instances of ItemBase (!)");
        m_aEntries.erase(pIBase);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    IAdministrator_unordered_set::IAdministrator_unordered_set(const ItemBase* pDefault)
    :   ItemAdministrator(pDefault),
        m_aEntries()
    {
    }

    ItemBase::SharedPtr IAdministrator_unordered_set::Create(const ItemBase* pIBase)
    {
        assert(pIBase != nullptr && "nullptr not allowed (!)");

        if(IsDefault(pIBase))
        {
            // if the Item to-be-created equals default, delete it and
            // use the existing single global default
            delete pIBase;
            return GetDefault();
        }

        // check for existance
        auto ExistingEntry(m_aEntries.find(pIBase));

        if(m_aEntries.end() != ExistingEntry)
        {
            // if the Item to-be-created exists already, delete it and
            // use the existing instance
            delete pIBase;
            return (*ExistingEntry)->shared_from_this();
        }
        else
        {
            // start using offered instance and administrate it from now
            SetAdministratedFromItemAdministrator(*const_cast<ItemBase*>(pIBase));
            m_aEntries.insert(pIBase);
            return ItemBase::SharedPtr(pIBase);
        }
    }

    void IAdministrator_unordered_set::HintExpired(const ItemBase* pIBase)
    {
        // called from ::~Item. This happens in two cases:
        // (1) a temporary Item instance gets deleted
        // (2) last shared_ptr was deleted
        // The caller should have already checked the
        // Administrated-flag, so only administrated instances
        // of ItemBase should be handled here which is case (2)
        assert(pIBase != nullptr && "nullptr not allowed (!)");
        assert(pIBase->IsAdministrated() && "call only for administrated instances of ItemBase (!)");
        m_aEntries.erase(pIBase);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    std::vector<const ItemBase*>::iterator IAdministrator_vector::find(const ItemBase* pIBase)
    {
        // find has to linearly traverse through instances and use operator==
        assert(pIBase != nullptr && "nullptr not allowed (!)");
        for(std::vector<const ItemBase*>::iterator candidate(m_aEntries.begin()); candidate != m_aEntries.end(); candidate++)
        {
            if(*candidate != nullptr)
            {
                if(*candidate == pIBase || (*candidate)->operator==(*pIBase))
                {
                    return candidate;
                }
            }
        }

        return m_aEntries.end();
    }

    void IAdministrator_vector::insert(const ItemBase* pIBase)
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

    void IAdministrator_vector::erase(std::vector<const ItemBase*>::iterator& rIter)
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
                std::vector<const ItemBase*> aNewEntries;
                aNewEntries.reserve(m_aEntries.size() - m_aFreeSlots.size());

                for(std::vector<const ItemBase*>::iterator candidate(m_aEntries.begin()); candidate != m_aEntries.end(); candidate++)
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

    IAdministrator_vector::IAdministrator_vector(const ItemBase* pDefault)
    :   ItemAdministrator(pDefault),
        m_aEntries(),
        m_aFreeSlots()
    {
    }

    ItemBase::SharedPtr IAdministrator_vector::Create(const ItemBase* pIBase)
    {
        assert(pIBase != nullptr && "nullptr not allowed (!)");

        if(IsDefault(pIBase))
        {
            // if the Item to-be-created equals default, delete it and
            // use the existing single global default
            delete pIBase;
            return GetDefault();
        }

        // check for existance
        auto ExistingEntry(find(pIBase));

        if(m_aEntries.end() != ExistingEntry)
        {
            // if the Item to-be-created exists already, delete it and
            // use the existing instance
            delete pIBase;
            return (*ExistingEntry)->shared_from_this();
        }
        else
        {
            // start using offered instance and administrate it from now
            SetAdministratedFromItemAdministrator(*const_cast<ItemBase*>(pIBase));
            insert(pIBase);
            return ItemBase::SharedPtr(pIBase);
        }
    }

    void IAdministrator_vector::HintExpired(const ItemBase* pIBase)
    {
        // called from ::~Item. This happens in two cases:
        // (1) a temporary Item instance gets deleted
        // (2) last shared_ptr was deleted
        // The caller should have already checked the
        // Administrated-flag, so only administrated instances
        // of ItemBase should be handled here which is case (2)
        assert(pIBase != nullptr && "nullptr not allowed (!)");
        assert(pIBase->IsAdministrated() && "call only for administrated instances of ItemBase (!)");
        auto iter(find(pIBase));
        erase(iter);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
