/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <item/base/ItemBuffered.hxx>
#include <item/base/ItemAdministrator.hxx>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    void ItemBuffered::ItemData::putValue(const css::uno::Any& /*rVal*/, sal_uInt8 /*nMemberId*/)
    {
        // not intended to be used, error
        assert(false && "Error: Some instance tries to set an Any at an ::ItemData instance - implement ItemData::putValue method there (!)");
    }

    ItemBuffered::ItemData::ItemData()
    :   m_nRef(0)
    {
    }

    ItemBuffered::ItemData::~ItemData()
    {
        assert(0 == m_nRef && "Still used ItemData deleted (!)");
    }

    bool ItemBuffered::ItemData::operator==(const ItemData& rRef) const
    {
        return (this == &rRef); // ptr-compare
    }

    bool ItemBuffered::ItemData::operator!=(const ItemData& rRef) const
    {
        return !(*this == rRef);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    void ItemBuffered::acquire()
    {
        if(0 == m_pItemData->m_nRef)
        {
            {
                ItemData* pAlternative = m_pItemData->getItemAdministrator().find_or_set(m_pItemData);

                if(pAlternative != m_pItemData)
                {
                    delete m_pItemData;
                    m_pItemData = pAlternative;
                }
            }
        }

        m_pItemData->m_nRef++;
    }

    void ItemBuffered::release()
    {
        m_pItemData->m_nRef--;

        if(0 == m_pItemData->m_nRef)
        {
            m_pItemData->getItemAdministrator().remove(m_pItemData);
            delete m_pItemData;
        }
    }

    void ItemBuffered::setItemData(ItemData* pItemData)
    {
        assert(nullptr != pItemData && "ItemBuffered::setItemData called without ItemData instance (!)");

        if(pItemData == m_pItemData || (nullptr != pItemData && nullptr != m_pItemData && *pItemData == *m_pItemData))
        {
            // no data change - no need to set
            delete pItemData;
        }
        else
        {
            if(nullptr != m_pItemData)
            {
                release();
            }

            m_pItemData = pItemData;
            acquire();
        }
    }

    void ItemBuffered::putValues(const AnyIDArgs& rArgs)
    {
        assert(nullptr != m_pItemData && "ItemBuffered::putValues called but m_pItemData not set (!)");

        if(!rArgs.empty())
        {
            ItemData* pNewItemData(m_pItemData->getItemAdministrator().createNewDataInstance());
            assert(nullptr != pNewItemData && "ItemBuffered::putValues could not create new ItemData instance (!)");

            for(const auto& arg : rArgs)
            {
                pNewItemData->putValue(arg.first, arg.second);
            }

            setItemData(pNewItemData);
        }
    }

    ItemBuffered::ItemBuffered(ItemControlBlock& rItemControlBlock)
    :   ItemBase(rItemControlBlock),
        m_pItemData(nullptr)
    {
        // to set m_pItemData, derived classes *have* to call setItemData
    }

    ItemBuffered::ItemBuffered(const ItemBuffered& rRef)
    :   ItemBase(rRef),
        m_pItemData(rRef.m_pItemData)
    {
        assert(nullptr != m_pItemData && "ItemBuffered::ItemBuffered copy-conatructor called but m_pItemData not set in source (!)");
        m_pItemData->m_nRef++;
    }

    ItemBuffered::~ItemBuffered()
    {
        assert(nullptr != m_pItemData && "ItemBuffered::~ItemBuffered called but m_pItemData not set (!)");
        release();
    }

    ItemBuffered& ItemBuffered::operator=(const ItemBuffered& rRef)
    {
        ItemBase::operator=(rRef);
        assert(nullptr != m_pItemData && "ItemBuffered::operator= called but m_pItemData not set (!)");
        release();
        m_pItemData = rRef.m_pItemData;
        acquire();
        return *this;
    }

    bool ItemBuffered::operator==(const ItemBuffered& rRef) const
    {
        assert(nullptr != m_pItemData && nullptr != rRef.m_pItemData && "ItemBuffered::operator== called but not all m_pItemData set (!)");
        return (
            ItemBase::operator==(rRef) || // ptr-compare
            m_pItemData == rRef.m_pItemData || // ItemData ptr-compare
            *m_pItemData == *rRef.m_pItemData); // ItemData content compare
    }

    std::unique_ptr<ItemBase> ItemBuffered::clone() const
    {
        // use copy constructor
        return std::unique_ptr<ItemBuffered>(new ItemBuffered(*this));
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
