/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_ITEM_BASE_SLOTSET_HXX
#define INCLUDED_ITEM_BASE_SLOTSET_HXX

#include <cassert>
#include <item/base/ModelSpecificIValues.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    class ITEM_DLLPUBLIC SlotSet : public std::enable_shared_from_this<SlotSet>
    {
    public:
        // SharedPtr typedef to be used handling instances of this type
        typedef std::shared_ptr<SlotSet> SharedPtr;
        typedef size_t SlotID;

    private:
        // the slots as content
        std::unordered_map<SlotID, IBase::SharedPtr> m_aSlots;

    protected:
        // constructor - protected BY DEFAULT - do NOT CHANGE (!)
        // Use ::Create(...) method instead
        SlotSet();

    public:
        virtual ~SlotSet();

        bool empty() const
        {
            return m_aSlots.empty();
        }

        // noncopyable
        SlotSet(const SlotSet&) = delete;
        SlotSet& operator=(const SlotSet&) = delete;

        static SharedPtr Create();

        void SetSlot(SlotID aSlotID, const IBase::SharedPtr& rItem);
        void SetSlots(const SlotSet& rSlotSet);

        template< typename TargetType > std::shared_ptr<const TargetType> GetSlot(SlotID aSlotID) const
        {
            const auto aRetval(m_aSlots.find(aSlotID));

            if(aRetval != m_aSlots.end()) // && aRetval->second)
            {
                assert(aRetval->second && "empty IBase::SharedPtr set in SlotSet (!)");
                assert(typeid(*aRetval->second) == typeid(TargetType) && "wrong IBase::SharedPtr type in SlotSet (!)");

                return std::static_pointer_cast<TargetType>(aRetval->second);
            }

            return TargetType::GetDefault();
        }

        bool ClearSlot(SlotID aSlotID);
        void ClearAllSlots();
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_SLOTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
