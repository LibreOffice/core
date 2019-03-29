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

#include <item/base/ItemBaseStaticHelper.hxx>
#include <rtl/ustring.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for OUStringItem
    // It uses IAdministrator_unordered_set to provide an Administrator using
    // a hashed list for fast accesses. This requires ::operator== and
    // ::GetUniqueKey() to be implemented.
    class CntOUString;
    typedef ItemBaseStaticHelper<CntOUString, IAdministrator_unordered_set> CntOUStringStaticHelper;

    class ITEM_DLLPUBLIC CntOUString : public CntOUStringStaticHelper, public ItemBase
    {
    public:
        // SharedPtr typedef to be used handling instances of given type
        typedef std::shared_ptr<const CntOUString> SharedPtr;

    private:
        // need to offer internal access to ItemAdministrator
        // in each derivation - just calls GetStaticAdmin internally
        virtual ItemAdministrator* GetIAdministrator() const override;

    private:
        // local variavbles
        rtl::OUString m_aValue;

    protected:
        // allow local ItemAdministrator access to protected constructor
        friend CntOUStringStaticHelper;

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
