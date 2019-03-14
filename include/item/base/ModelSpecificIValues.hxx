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

#ifndef INCLUDED_ITEM_BASE_MODELSPECIFICIVALUES_HXX
#define INCLUDED_ITEM_BASE_MODELSPECIFICIVALUES_HXX

#include <memory>
#include <item/itemdllapi.h>
#include <unordered_map>
#include <item/base/IBase.hxx>
#include <sal/types.h>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // Model-specific ItemValues, e.g. overridden defaults.
    // - override defaults per Set (already in place)
    // - maybe: old SfxItemInfo replacement which is a mapping
    //   between SlotID and WhichID
    // - maybe: ItemNames - currently in ::TakeItemName and others
    // - maybe: hold MapMode info to allow correct GetPresentation
    //   executions
    class ITEM_DLLPUBLIC ModelSpecificIValues : public std::enable_shared_from_this<ModelSpecificIValues>
    {
    public:
        // SharedPtr typedef to be used handling instances of this type
        typedef std::shared_ptr<ModelSpecificIValues> SharedPtr;

    private:
        // the alternative default items
        std::unordered_map<size_t, IBase::SharedPtr> m_aAlternativeItems;

    protected:
        // constructor - protected BY DEFAULT - do NOT CHANGE (!)
        // Use ::Create(...) method instead
        ModelSpecificIValues();

    public:
        virtual ~ModelSpecificIValues();

        // noncopyable
        ModelSpecificIValues(const ModelSpecificIValues&) = delete;
        ModelSpecificIValues& operator=(const ModelSpecificIValues&) = delete;

        // create instance
        static SharedPtr Create();

        // AlternativeDefault interface
        void SetAlternativeDefaultItem(const IBase::SharedPtr& rItem);
        bool IsDefault(const IBase::SharedPtr& rItem) const;
        const IBase::SharedPtr& GetDefault(const IBase::SharedPtr& rItem) const;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_MODELSPECIFICIVALUES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
