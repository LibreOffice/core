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
#include <item/base/ItemBase.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ItemControlBlock::ItemControlBlock(
        std::function<ItemBase*()>aConstructDefaultItem,
        std::function<ItemBase*(const ItemBase&)>aCloneItem,
        const OUString& rName)
    :   m_aDefaultItem(),
        m_aConstructDefaultItem(aConstructDefaultItem),
        m_aCloneItem(aCloneItem),
        m_aName(rName)
    {
        assert(nullptr != m_aConstructDefaultItem && "nullptr not allowed, a Item-Constructor *is* required (!)");
        assert(nullptr != aCloneItem && "nullptr not allowed, a Item-Clone lambda *is* required (!)");
    }

    const ItemBase& ItemControlBlock::getDefault() const
    {
        if(!m_aDefaultItem)
        {
            const_cast<ItemControlBlock*>(this)->m_aDefaultItem.reset(m_aConstructDefaultItem());
            assert(m_aDefaultItem && "could *not* construct default item (!)");
        }

        return *m_aDefaultItem;
    }

    bool ItemControlBlock::isDefault(const ItemBase& rItem) const
    {
        assert(typeid(rItem) == typeid(getDefault()) && "different types compared - not allowed (!)");
        return &rItem == &getDefault() ||   // ptr-compare
            rItem.operator==(getDefault()); // content-compare
    }

    std::unique_ptr<ItemBase> ItemControlBlock::clone(const ItemBase& rRef) const
    {
        return std::unique_ptr<ItemBase>(m_aCloneItem(rRef));
    }

    std::unique_ptr<const ItemBase> ItemControlBlock::createFromAny(const ItemBase::AnyIDArgs& rArgs)
    {
        ItemBase* pNewInstance(m_aConstructDefaultItem());
        pNewInstance->putAnyValues(rArgs);
        return std::unique_ptr<const ItemBase>(pNewInstance);
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
