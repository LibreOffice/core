/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ITEM_SIMPLE_CNTOUSTRING_HXX
#define INCLUDED_ITEM_SIMPLE_CNTOUSTRING_HXX

//#include <item/base/ItemBaseStaticHelper.hxx>
#include <item/base/ItemBase.hxx>
#include <rtl/ustring.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for OUStringItem
    // It uses IAdministrator_unordered_set to provide an Administrator using
    // a hashed list for fast accesses. This requires ::operator== and
    // ::GetUniqueKey() to be implemented.
    class ITEM_DLLPUBLIC CntOUString : public ItemBase
    {
    public:
        static ItemControlBlock& GetStaticItemControlBlock();
        virtual ItemControlBlock& GetItemControlBlock() const override;

    private:
        // local variavbles
        rtl::OUString m_aValue;

    protected:
        // SharedPtr-constructor - protected BY DEFAULT - do NOT CHANGE (!)
        CntOUString(const rtl::OUString& rValue = rtl::OUString());

    public:
        virtual ~CntOUString();

        static std::shared_ptr<const CntOUString> Create(const rtl::OUString& rValue);
        virtual bool operator==(const ItemBase& rCandidate) const override;
        virtual size_t GetUniqueKey() const override;
        const rtl::OUString& GetValue() const;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_SIMPLE_CNTOUSTRING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
