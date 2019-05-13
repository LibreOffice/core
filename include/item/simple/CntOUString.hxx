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

#include <item/base/ItemBase.hxx>
#include <rtl/ustring.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for CntUnencodedStringItem
    // this is a helper base class, so it has *no* method
    //     static ItemControlBlock& GetStaticItemControlBlock();
    // and also no public constructor (!), but implements all the
    // tooling methods for Items using a sal_Int16 internally
    class ITEM_DLLPUBLIC CntOUString : public ItemBase
    {
    private:
        rtl::OUString m_aValue;

    protected:
        // constructor for derived classes that *have* to hand
        // in the to-be-used ItemControlBlock
        CntOUString(ItemControlBlock& rItemControlBlock, const rtl::OUString& rValue = rtl::OUString());

    public:
        CntOUString() = delete;
        virtual bool operator==(const ItemBase&) const;

        const rtl::OUString& getValue() const { return m_aValue; }
        void putValue(const rtl::OUString& rValue) { m_aValue = rValue; }

        virtual bool getPresentation(
            SfxItemPresentation,
            MapUnit,
            MapUnit,
            rtl::OUString&,
            const IntlWrapper&) const override;
        virtual bool queryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
        virtual bool putAnyValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_SIMPLE_CNTOUSTRING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
