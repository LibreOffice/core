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
        const std::shared_ptr<const ItemBase>& rDefaultItem,
        std::function<ItemBase*()>constructItem)
    :   m_aItemAdministrator(rItemAdministrator),
        m_aDefaultItem(rDefaultItem),
        m_aConstructItem(constructItem)
    {
        assert(rItemAdministrator && "nullptr not allowed, an ItemAdministrator *is* required (!)");
        assert(rDefaultItem && "nullptr not allowed, a default value *is* required (!)");
    }

    ItemControlBlock::ItemControlBlock()
    :   m_aItemAdministrator(),
        m_aDefaultItem(),
        m_aConstructItem()
    {
    }

    std::shared_ptr<const ItemBase> ItemControlBlock::CreateFromAny(const ItemBase::AnyIDArgs& rArgs)
    {
        ItemBase* pNewInstance(ConstructItem());
        pNewInstance->PutValues(rArgs);
        return std::shared_ptr<const ItemBase>(pNewInstance);
    }

    bool ItemControlBlock::IsDefaultDDD(const ItemBase& rItem) const
    {
        assert(nullptr != m_aDefaultItem.get() && "empty DefaultItem detected - not allowed (!)");
        assert(typeid(rItem) == typeid(*m_aDefaultItem) && "different types compared - not allowed (!)");
        return &rItem == m_aDefaultItem.get() || rItem.operator==(*m_aDefaultItem.get());
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
