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

#ifndef INCLUDED_ITEM_BASE_ITEMCONTROLBLOCK_HXX
#define INCLUDED_ITEM_BASE_ITEMCONTROLBLOCK_HXX

#include <functional>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // predefine ItemAdministrator and ItemBase - no need to include
    class ItemAdministrator;
    class ItemBase;

    class ITEM_DLLPUBLIC ItemControlBlock
    {
    private:
        std::shared_ptr<ItemAdministrator>  m_aItemAdministrator;
        std::shared_ptr<const ItemBase>     m_aDefaultItem;
        std::function<ItemBase*()>          m_aConstructDefaultItem;
        std::function<ItemBase*()>          m_aConstructItem;
        OUString                            m_aName;

    public:
        ItemControlBlock(
            const std::shared_ptr<ItemAdministrator>& rItemAdministrator,
            std::function<ItemBase*()>constructDefaultItem,
            std::function<ItemBase*()>constructItem,
            const OUString& rName);
        ItemControlBlock();

        const std::shared_ptr<ItemAdministrator>& GetItemAdministrator()
        {
            return m_aItemAdministrator;
        }

        const std::shared_ptr<const ItemBase>& GetDefaultItem() const;

        ItemBase* ConstructItem() const
        {
            return m_aConstructItem();
        }

        std::shared_ptr<const ItemBase> CreateFromAny(const ItemBase::AnyIDArgs& rArgs);

        bool IsDefault(const ItemBase& rItem) const;

        const OUString& GetName() const
        {
            return m_aName;
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_ITEMCONTROLBLOCK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
