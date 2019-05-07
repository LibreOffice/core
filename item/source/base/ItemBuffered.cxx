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
    void ItemBuffered::ItemData::putAnyValue(const css::uno::Any& /*rVal*/, sal_uInt8 /*nMemberId*/)
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
        assert(typeid(rRef) == typeid(*this) && "Unequal types in ItemBuffered::ItemData::operator== not allowed (!)");
        return (this == &rRef); // ptr-compare
    }

    bool ItemBuffered::ItemData::operator!=(const ItemData& rRef) const
    {
        assert(typeid(rRef) == typeid(*this) && "Unequal types in ItemBuffered::ItemData::operator!= not allowed (!)");
        return !(*this == rRef);
    }

    ItemBuffered::ItemData* ItemBuffered::ItemData::clone() const
    {
        return getItemAdministrator().clone(*this);
    }

} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    void ItemBuffered::acquire()
    {
        if(0 == m_pItemData->m_nRef)
        {
            // no owner yet - grab it. Ensure uniqueness of incarnation. find_or_set
            // will return an already existing equal incarnation if exists for this
            // type
            ItemData* pAlternative = m_pItemData->getItemAdministrator().find_or_set(m_pItemData);

            if(pAlternative != m_pItemData)
            {
                // if an incarnation already exists, delete suggested incarnation
                // and also use existing one
                delete m_pItemData;
                m_pItemData = pAlternative;
            }
        }

        // increse refCnt
        m_pItemData->m_nRef++;
    }

    void ItemBuffered::release()
    {
        // decrease RefCnt
        m_pItemData->m_nRef--;

        if(0 == m_pItemData->m_nRef)
        {
            // no owner anymore - deregister at ItemAdministrator and delete
            m_pItemData->getItemAdministrator().remove(m_pItemData);
            delete m_pItemData;
        }
    }

    void ItemBuffered::setItemData(ItemData* pItemData)
    {
        assert(nullptr != pItemData && "ItemBuffered::setItemData called without ItemData instance (!)");

        if(pItemData == m_pItemData || // ptr-compare
            (nullptr != pItemData && nullptr != m_pItemData // if both exist
                && *pItemData == *m_pItemData)) // content-conmpare
        {
            // no data change - no need to set
            delete pItemData;
        }
        else
        {
            // will be replaced
            if(nullptr != m_pItemData)
            {
                // release if used
                release();
            }

            // set instance locally
            m_pItemData = pItemData;

            // grep it
            acquire();
        }
    }

    void ItemBuffered::make_unique()
    {
        assert(nullptr != m_pItemData && "ItemBuffered::make_unique called but m_pItemData not set (!)");

        if(m_pItemData->m_nRef > 1)
        {
            // is not owned, need to create a copy
            release();

            // create copy and assign
            m_pItemData = m_pItemData->clone();

            // grep new instance
            acquire();
        }
    }

    void ItemBuffered::putAnyValue(const css::uno::Any& rVal, sal_uInt8 nMemberId)
    {
        assert(nullptr != m_pItemData && "ItemBuffered::putAnyValues called but m_pItemData not set (!)");

        // make sure we are unique - would be nice to first test for change,
        // but main usage of putAnyValues() is anyways uno api and thus will
        // be appied to new instances - so potentially no cloning needed
        make_unique();

        // forward call to ItemData incarnation
        m_pItemData->putAnyValue(rVal, nMemberId);
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
        // copy m_pItemData, increase refCnt
        m_pItemData->m_nRef++;
    }

    ItemBuffered::~ItemBuffered()
    {
        assert(nullptr != m_pItemData && "ItemBuffered::~ItemBuffered called but m_pItemData not set (!)");
        // decrease RefCnt
        release();
    }

    ItemBuffered& ItemBuffered::operator=(const ItemBuffered& rRef)
    {
        assert(nullptr != m_pItemData && "ItemBuffered::operator= called but m_pItemData not set (!)");

        // call baseclass (does nothing, just better style)
        ItemBase::operator=(rRef);

        // release (may delete if owner, copy from source, grab it)
        release();
        m_pItemData = rRef.m_pItemData;
        acquire();

        return *this;
    }

    bool ItemBuffered::operator==(const ItemBase& rRef) const
    {
        const ItemBuffered& rTmpRef(static_cast<const ItemBuffered&>(rRef));
        assert(nullptr != m_pItemData && nullptr != rTmpRef.m_pItemData && "ItemBuffered::operator== called but not all m_pItemData set (!)");
        return (
            ItemBase::operator==(rRef) || // ptr-compare
            m_pItemData == rTmpRef.m_pItemData || // ItemData ptr-compare
            *m_pItemData == *rTmpRef.m_pItemData); // ItemData content compare
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
