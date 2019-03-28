/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <item/base/ModelSpecificItemValues.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ModelSpecificItemValues::ModelSpecificItemValues()
    :   std::enable_shared_from_this<ModelSpecificItemValues>(),
        m_aAlternativeItems()
    {
    }

    ModelSpecificItemValues::~ModelSpecificItemValues()
    {
    }

    ModelSpecificItemValues::SharedPtr ModelSpecificItemValues::Create()
    {
        return ModelSpecificItemValues::SharedPtr(new ModelSpecificItemValues());
    }

    void ModelSpecificItemValues::SetAlternativeDefaultItem(const ItemBase::SharedPtr& rItem)
    {
        assert(rItem && "empty ItemBase::SharedPtr not allowed - and should be unable to be created (!)");
        const size_t hash_code(typeid(*rItem).hash_code());

        m_aAlternativeItems[hash_code] = rItem;
    }

    bool ModelSpecificItemValues::IsDefault(const ItemBase::SharedPtr& rItem) const
    {
        assert(rItem && "empty ItemBase::SharedPtr not allowed - and should be unable to be created (!)");

        if(!m_aAlternativeItems.empty())
        {
            const size_t hash_code(typeid(*rItem).hash_code());
            const auto aRetval(m_aAlternativeItems.find(hash_code));

            if(aRetval != m_aAlternativeItems.end())
            {
                return aRetval->second.get() == rItem.get()
                    || aRetval->second->operator==(*rItem);
            }
        }

        // use Item's own static global default
        return rItem->IsDefault();
    }

    const ItemBase::SharedPtr& ModelSpecificItemValues::GetDefault(const ItemBase::SharedPtr& rItem) const
    {
        assert(rItem && "empty ItemBase::SharedPtr not allowed - and should be unable to be created (!)");

        if(!m_aAlternativeItems.empty())
        {
            const size_t hash_code(typeid(*rItem).hash_code());
            const auto aRetval(m_aAlternativeItems.find(hash_code));

            if(aRetval != m_aAlternativeItems.end())
            {
                return aRetval->second;
            }
        }

        // loop through given instance
        return rItem;
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
