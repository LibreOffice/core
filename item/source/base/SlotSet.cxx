/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <item/base/SlotSet.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    SlotSet::SlotSet()
    :   std::enable_shared_from_this<SlotSet>(),
        m_aSlots()
    {
    }

    SlotSet::~SlotSet()
    {
    }

    SlotSet::SharedPtr SlotSet::Create()
    {
        return SlotSet::SharedPtr(new SlotSet());
    }

    void SlotSet::SetSlot(SlotID aSlotID, const IBase::SharedPtr& rItem)
    {
        assert(rItem && "empty IBase::SharedPtr not allowed - and should be unable to be created (!)");
        m_aSlots[aSlotID] = rItem;
    }

    void SlotSet::SetSlots(const SlotSet& rSlotSet)
    {
        m_aSlots.insert(rSlotSet.m_aSlots.begin(), rSlotSet.m_aSlots.end());
    }

    bool SlotSet::ClearSlot(SlotID aSlotID)
    {
        return (0 != m_aSlots.erase(aSlotID));
    }

    void SlotSet::ClearAllSlots()
    {
        m_aSlots.clear();
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
