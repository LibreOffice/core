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

#include <item/base/IBaseStaticHelper.hxx>
#include <rtl/ustring.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for OUStringItem
    class CntOUString;
    typedef IBaseStaticHelper<CntOUString, IAdministrator_hash> CntOUStringStaticHelper;

    class ITEM_DLLPUBLIC CntOUString : public CntOUStringStaticHelper
    {
    private:
        rtl::OUString m_aValue;

    protected:
        friend CntOUStringStaticHelper;

        CntOUString(const rtl::OUString& rValue = rtl::OUString());

    public:
        virtual ~CntOUString();

        static std::shared_ptr<const CntOUString> Create(const rtl::OUString& rValue);
        virtual bool operator==(const IBase& rCandidate) const override;
        virtual size_t GetHash() const override;
        const rtl::OUString& GetValue() const;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_SIMPLE_CNTOUSTRING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
