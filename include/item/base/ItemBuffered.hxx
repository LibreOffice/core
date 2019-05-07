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

    // Baseclass for implementing buffered Items. The class is
    // pure virtual and can/should not be incarnated directly,
    // only derivatiohns from it.
    // Please see include\item\base\ItemBase.hxx for more basic
    // discussion about that.
    class ITEM_DLLPUBLIC ItemBuffered : public ItemBase
    {
    public:
        // This is the pattern for the ItemData block. It is by purpose
        // included to class ItemBuffered to make clear that it's usage
        // only makes sense inside that class
        class ITEM_DLLPUBLIC ItemData
        {
        private:
            friend class ItemBuffered;
            friend class ItemAdministrator;
            sal_Int32   m_nRef;

        protected:
            // access to the ItemAAdministrator (see include\item\base\ItemAdministrator.hxx)
            virtual ItemAdministrator& getItemAdministrator() const = 0;

            // PutValue/Any interface for automated instance creation from SfxType
            // mechanism (UNO API and sfx2 stuff). Default does nothing, but asserts
            // for missing implementation
            virtual void putAnyValue(const css::uno::Any& rVal, sal_uInt8 nMemberId);

        public:
            ItemData();
            virtual ~ItemData();

            // noncopyable
            ItemData(const ItemData&) = delete;
            ItemData& operator=(const ItemData&) = delete;

            // compare Op - operator!= is using operator==
            // so only operator== needs to be implemented
            virtual bool operator==(const ItemData&) const;
            bool operator!=(const ItemData&) const;

            // clone-op, needs to create new instance and copy content.
            // Call is forwarded to ItemAdministrator which needs a copy
            // operator at construction time
            ItemData* clone() const;
        };

    private:
        // reference to the data block, will never be nullptr
        ItemData*     m_pItemData;

        // internal acquire/release of DataBlock
        void acquire();
        void release();

    protected:
        // PutValue/Any interface for automated instance creation from SfxType
        // mechanism (UNO API and sfx2 stuff)
        virtual void putAnyValue(const css::uno::Any& rVal, sal_uInt8 nMemberId);

    protected:
        // Method to internally (thus protected) set a new ItemData
        // instance anytime. Can be used e.g. to change data. The
        // currently used ItemData is correctly handled if in use.
        // This *Has* to be used in constructors to setup m_pItemData.
        // This is ownership change and required to completely setup
        // the object e.g. in derived constructors (!) No nullptr
        // hand-over allowed (!)
        void setItemData(ItemData* pItemData);

        // guarantees local instance refCnt == 1, so only owner. Will
        // execute needed actions when required.
        // difference to setItemData is that it potentially has to
        // clone a copy itself, thus setItemData may be more effective
        void make_unique();

        // access to ItemData instance. CAUTION! If you want to use this
        // to change data at your derivation of ItemData, you *have* to
        // use setItemData or make_unique to be safe (!)
        ItemData& getItemData() const { return *m_pItemData; }

        // constructor for derived classes, thus protected. Derived
        // classes hand in their specific ItemControlBlock to be used
        ItemBuffered(ItemControlBlock& rItemControlBlock);

    public:
        virtual ~ItemBuffered();

        // copy constructor and assigment operator (both allowed).
        // Both use the refCnted incarnation of ItemData and are
        // complete
        ItemBuffered(const ItemBuffered&);
        ItemBuffered& operator=(const ItemBuffered&);

        // compare, default implementation uses ItemData and
        // is complete
        virtual bool operator==(const ItemBase&) const;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////
#endif // INCLUDED_ITEM_BASE_ITEMBUFFERD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
