/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ITEM_SIMPLE_CNTINT16_HXX
#define INCLUDED_ITEM_SIMPLE_CNTINT16_HXX

#include <item/base/ItemBase.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for SfxInt16Item
    // this is a helper base class, so it has *no* method
    //     static ItemControlBlock& GetStaticItemControlBlock();
    // and also no public constructor (!), but implements all the
    // tooling methods for Items using a sal_Int16 internally
    class ITEM_DLLPUBLIC CntInt16 : public ItemBase
    {
    private:
        sal_Int16 m_nValue;

    protected:
        // constructor for derived classes that *have* to hand
        // in the to-be-used ItemControlBlock
        CntInt16(ItemControlBlock& rItemControlBlock, sal_Int16 nVal = 0);

    public:
        CntInt16() = delete;
        virtual bool operator==(const ItemBase&) const;

        sal_Int16 getValue() const { return m_nValue; }
        void setValue(sal_Int16 nNew) { m_nValue = nNew; }

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

#endif // INCLUDED_ITEM_SIMPLE_CNTINT16_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
