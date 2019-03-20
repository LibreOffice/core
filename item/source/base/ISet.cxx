/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <item/base/ISet.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ISet::ISet(const ModelSpecificIValues::SharedPtr& rModelSpecificIValues)
    :   std::enable_shared_from_this<ISet>(),
        m_aParent(),
        m_aModelSpecificIValues(rModelSpecificIValues),
        m_aItems()
    {
    }

    ISet::~ISet()
    {
    }

    void ISet::SetParent(const ISet::SharedPtr& rNewParent)
    {
        m_aParent = rNewParent;
    }

    const ISet::SharedPtr& ISet::GetParent() const
    {
        return m_aParent;
    }

    ISet::SharedPtr ISet::Create(const ModelSpecificIValues::SharedPtr& rModelSpecificIValues)
    {
        return ISet::SharedPtr(new ISet(rModelSpecificIValues));
    }

    const ModelSpecificIValues::SharedPtr& ISet::GetModelSpecificIValues() const
    {
        return m_aModelSpecificIValues;
    }

    void ISet::SetItem(const IBase::SharedPtr& rItem)
    {
        assert(rItem && "empty IBase::SharedPtr not allowed - and should be unable to be created (!)");
        bool bDefault(false);

        // detect if rItem is default item, include evtl. model-specific
        // defaults, not only the Item-implementation single global one
        if(m_aModelSpecificIValues)
        {
            // may use model-specific default, get from helper
            // helper *will* fallback to IBase default
            bDefault = m_aModelSpecificIValues->IsDefault(rItem);
        }
        else
        {
            // use Item's own static global default
            bDefault = rItem->IsDefault();
        }

        const size_t hash_code(typeid(*rItem).hash_code());

        if(bDefault)
        {
            // SfxItemState::DEFAULT is represented by not being set
            m_aItems.erase(hash_code);
        }
        else
        {
            // SfxItemState::SET
            m_aItems[hash_code] = rItem;
        }
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
