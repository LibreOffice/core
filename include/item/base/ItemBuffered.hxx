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

#ifndef INCLUDED_ITEM_BASE_ITEMBUFFERD_HXX
#define INCLUDED_ITEM_BASE_ITEMBUFFERD_HXX

#include <item/base/ItemBase.hxx>
#include <sal/types.h>
#include <item/itemdllapi.h>
#include <com/sun/star/uno/Any.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    class ItemAdministrator;

    class ITEM_DLLPUBLIC ItemBuffered : public ItemBase
    {
    public:
        class ITEM_DLLPUBLIC ItemData
        {
        private:
            friend class ItemBuffered;
            friend class ItemAdministrator;
            sal_Int32   m_nRef;

        protected:
            virtual ItemAdministrator& getItemAdministrator() const = 0;

            // PutValue/Any interface for automated instance creation from SfxType
            // mechanism (UNO API and sfx2 stuff)
            virtual void putAnyValue(const css::uno::Any& rVal, sal_uInt8 nMemberId);

        public:
            ItemData();
            virtual ~ItemData();

            // noncopyable
            ItemData(const ItemData&) = delete;
            ItemData& operator=(const ItemData&) = delete;

            virtual bool operator==(const ItemData&) const;
            bool operator!=(const ItemData&) const;
        };

    private:
        ItemData*     m_pItemData;

        void acquire();
        void release();

    public:
        // PutValue/Any interface for automated instance creation from SfxType
        // mechanism (UNO API and sfx2 stuff)
        virtual void putAnyValues(const AnyIDArgs& rArgs);

    protected:
        // Method to internally (thus protected) set a new ItemData
        // instance anytime. Can be used e.g. to change data. The
        // currently used ItemData is correctly handled if in use.
        // This *Has* to be used in constructors to setup m_pItemData.
        // This is ownership change and required to completely setup
        // the object e.g. in derived constructors (!) No nullptr
        // hand-over allowed (!)
        void setItemData(ItemData* pItemData);

        // access to ItemData instance - only const (!) and
        // only internal for derived classes to implement
        // data read access methods (Get-Methods)
        ItemData const& getItemData() const { return *m_pItemData; }

    public:
        ItemBuffered(ItemControlBlock& rItemControlBlock);
        ItemBuffered(const ItemBuffered&);
        virtual ~ItemBuffered();
        ItemBuffered& operator=(const ItemBuffered&);

        virtual bool operator==(const ItemBase&) const;
        virtual std::unique_ptr<ItemBase> clone() const;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////
#endif // INCLUDED_ITEM_BASE_ITEMBUFFERD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
