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
    std::unordered_map<size_t, ItemControlBlock*>& getRegisteredItemControlBlocks()
    {
        // all globally registered ItemControlBlocks
        static std::unordered_map<size_t, ItemControlBlock*> aItemControlBlocks;
        return aItemControlBlocks;
    }

    ItemControlBlock::ItemControlBlock()
    :   m_aDefaultItem(),
        m_aConstructDefaultItem(),
        m_aCloneItem(),
        m_aHashCode(0),
        m_aName()
    {
        // EmptyItemControlBlock: *only* for internal use, fallback for
        // extra-Items like ImplInvalidateItem/ImplDisableItem
        // Do *not* register this instance at aItemControlBlocks (!)
    }

    ItemControlBlock::ItemControlBlock(
        std::function<ItemBase*()>aConstructDefaultItem,
        std::function<ItemBase*(const ItemBase&)>aCloneItem,
        size_t aHashCode,
        const OUString& rName)
    :   m_aDefaultItem(),
        m_aConstructDefaultItem(aConstructDefaultItem),
        m_aCloneItem(aCloneItem),
        m_aHashCode(aHashCode),
        m_aName(rName)
    {
        assert(nullptr != m_aConstructDefaultItem && "nullptr not allowed, a Item-Constructor *is* required (!)");
        assert(nullptr != aCloneItem && "nullptr not allowed, a Item-Clone lambda *is* required (!)");
        assert(size_t(0) != m_aHashCode && "NULL hash_code not allowed, a Item-identifier (usually typeid(T).hash_code()) *is* required (!)");
        assert(getRegisteredItemControlBlocks().find(m_aHashCode) == getRegisteredItemControlBlocks().end()
            && "Constructed ItemControlBlock already globally registered - this hints to an error (!)");
        // globally register new ItemControlBlock
        getRegisteredItemControlBlocks()[m_aHashCode] = this;
    }

    ItemControlBlock::~ItemControlBlock()
    {
        assert((0 == m_aHashCode || // is the EmptyItemControlBlock
            getRegisteredItemControlBlocks().find(m_aHashCode) != getRegisteredItemControlBlocks().end()) // or has to exist
                && "Destructed ItemControlBlock was not globally registered - this hints to an error (!)");
        // since ItemControlBlocks themselves are static this can only happen when
        // handling libs, e.g. lib shutdown and of course app shutdown. Nonetheless
        // do this to avoid errors
        if(0 != m_aHashCode) // do not forget default constructor -> EmptyItemControlBlock
        {
            getRegisteredItemControlBlocks().erase(getRegisteredItemControlBlocks().find(m_aHashCode));
        }
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

    ItemControlBlock* ItemControlBlock::getItemControlBlock(size_t hash_code)
    {
        if(size_t(0) != hash_code)
        {
            std::unordered_map<size_t, ItemControlBlock*>& rBlocks(getRegisteredItemControlBlocks());
            const auto aCandidate(rBlocks.find(hash_code));

            if(aCandidate != rBlocks.end())
            {
                return aCandidate->second;
            }
        }

        static ItemControlBlock aEmptyItemControlBlock;
        return &aEmptyItemControlBlock;
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
