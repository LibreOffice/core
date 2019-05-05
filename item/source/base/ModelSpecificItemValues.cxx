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

    ModelSpecificItemValues::SharedPtr ModelSpecificItemValues::create()
    {
        return ModelSpecificItemValues::SharedPtr(new ModelSpecificItemValues());
    }

    void ModelSpecificItemValues::setAlternativeDefaultItem(const ItemBase& rItem)
    {
        const size_t hash_code(typeid(rItem).hash_code());
        const auto aRetval(m_aAlternativeItems.find(hash_code));

        if(aRetval == m_aAlternativeItems.end())
        {
            m_aAlternativeItems[hash_code] = rItem.clone().release();
        }
        else
        {
            delete aRetval->second;
            aRetval->second = rItem.clone().release();
        }
    }

    bool ModelSpecificItemValues::isDefault(const ItemBase& rItem) const
    {
        if(!m_aAlternativeItems.empty())
        {
            const size_t hash_code(typeid(rItem).hash_code());
            const auto aRetval(m_aAlternativeItems.find(hash_code));

            if(aRetval != m_aAlternativeItems.end())
            {
                return *aRetval->second == rItem;
            }
        }

        // use Item's own static global default
        return rItem.isDefault();
    }

    const ItemBase& ModelSpecificItemValues::getDefault(const ItemBase& rItem) const
    {
        if(!m_aAlternativeItems.empty())
        {
            const size_t hash_code(typeid(rItem).hash_code());
            const auto aRetval(m_aAlternativeItems.find(hash_code));

            if(aRetval != m_aAlternativeItems.end())
            {
                return *aRetval->second;
            }
        }

        return rItem;
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
