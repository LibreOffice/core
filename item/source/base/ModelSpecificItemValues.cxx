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
    ModelSpecificItemValues::ModelSpecificItemValues(
        MapUnit aMapUnit)
    :   std::enable_shared_from_this<ModelSpecificItemValues>(),
        m_aStaticItems(),
        m_aAlternativeItems(),
        m_aMapUnit(aMapUnit)
    {
    }

    ModelSpecificItemValues::~ModelSpecificItemValues()
    {
    }

    ModelSpecificItemValues::SharedPtr ModelSpecificItemValues::create(
        MapUnit aMapUnit)
    {
        return ModelSpecificItemValues::SharedPtr(
            new ModelSpecificItemValues(
                aMapUnit
            ));
    }

    void ModelSpecificItemValues::setStaticDefaultItem(const ItemBase& rItem)
    {
        const size_t hash_code(typeid(rItem).hash_code());
        const auto aRetval(m_aStaticItems.find(hash_code));

        if(aRetval == m_aStaticItems.end())
        {
            // new entry
            m_aStaticItems[hash_code] = rItem.clone().release();
        }
        else
        {
            // replace existing entry
            delete aRetval->second;
            aRetval->second = rItem.clone().release();
        }
    }

    void ModelSpecificItemValues::implclearStaticDefaultItem(size_t hash_code)
    {
        const auto aRetval(m_aStaticItems.find(hash_code));

        if(aRetval != m_aStaticItems.end())
        {
            delete aRetval->second;
            m_aStaticItems.erase(aRetval);
        }
    }

    void ModelSpecificItemValues::clearStaticDefaultItems()
    {
        for(const auto& candidate : m_aStaticItems)
        {
            delete candidate.second;
        }

        m_aStaticItems.clear();
    }

    void ModelSpecificItemValues::setAlternativeDefaultItem(const ItemBase& rItem)
    {
        const size_t hash_code(typeid(rItem).hash_code());
        const auto aRetval(m_aAlternativeItems.find(hash_code));

        if(aRetval == m_aAlternativeItems.end())
        {
            // new entry
            m_aAlternativeItems[hash_code] = rItem.clone().release();
        }
        else
        {
            // replace existing entry
            delete aRetval->second;
            aRetval->second = rItem.clone().release();
        }
    }

    void ModelSpecificItemValues::implclearAlternativeDefaultItem(size_t hash_code)
    {
        const auto aRetval(m_aAlternativeItems.find(hash_code));

        if(aRetval != m_aAlternativeItems.end())
        {
            delete aRetval->second;
            m_aAlternativeItems.erase(aRetval);
        }
    }

    void ModelSpecificItemValues::clearAlternativeDefaultItems()
    {
        for(const auto& candidate : m_aAlternativeItems)
        {
            delete candidate.second;
        }

        m_aAlternativeItems.clear();
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
        // check AlternativeItems 1st
        if(!m_aAlternativeItems.empty())
        {
            const size_t hash_code(typeid(rItem).hash_code());
            const auto aRetval(m_aAlternativeItems.find(hash_code));

            if(aRetval != m_aAlternativeItems.end())
            {
                return *aRetval->second;
            }
        }

        // check StaticItems 2nd
        if(!m_aStaticItems.empty())
        {
            const size_t hash_code(typeid(rItem).hash_code());
            const auto aRetval(m_aStaticItems.find(hash_code));

            if(aRetval != m_aStaticItems.end())
            {
                return *aRetval->second;
            }
        }

        // no limitations/replacements -> use given Item's default
        return rItem.getDefault();
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
