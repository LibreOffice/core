/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <item/base/ItemControlBlock.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ItemControlBlock::ItemControlBlock(
        const std::shared_ptr<ItemAdministrator>& rItemAdministrator,
        std::function<ItemBase*()>constructDefaultItem,
        std::function<ItemBase*()>constructItem,
        const OUString& rName)
    :   m_aItemAdministrator(rItemAdministrator),
        m_aDefaultItem(),
        m_aConstructDefaultItem(constructDefaultItem),
        m_aConstructItem(constructItem),
        m_aName(rName)
    {
        assert(rItemAdministrator && "nullptr not allowed, an ItemAdministrator *is* required (!)");
    }

    ItemControlBlock::ItemControlBlock()
    :   m_aItemAdministrator(),
        m_aDefaultItem(),
        m_aConstructDefaultItem(),
        m_aConstructItem()
    {
    }

    const std::shared_ptr<const ItemBase>& ItemControlBlock::GetDefaultItem() const
    {
        if(!m_aDefaultItem)
        {
            const_cast<ItemControlBlock*>(this)->m_aDefaultItem.reset(m_aConstructDefaultItem());
        }

        return m_aDefaultItem;
    }

    std::shared_ptr<const ItemBase> ItemControlBlock::CreateFromAny(const ItemBase::AnyIDArgs& rArgs)
    {
        ItemBase* pNewInstance(ConstructItem());
        pNewInstance->PutValues(rArgs);
        return std::shared_ptr<const ItemBase>(pNewInstance);
    }

    bool ItemControlBlock::IsDefault(const ItemBase& rItem) const
    {
        assert(typeid(rItem) == typeid(*GetDefaultItem()) && "different types compared - not allowed (!)");
        return &rItem == GetDefaultItem().get() || rItem.operator==(*GetDefaultItem().get());
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
