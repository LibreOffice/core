/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ITEM_SIMPLE_CNTENUM_HXX
#define INCLUDED_ITEM_SIMPLE_CNTENUM_HXX

#include <item/base/ItemBase.hxx>
#include <item/base/ItemControlBlock.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    class ITEM_DLLPUBLIC CntEnumBase : public ItemBase
    {
    protected:
        // constructor for derived classes that *have* to hand
        // in the to-be-used ItemControlBlock
        CntEnumBase(ItemControlBlock& rItemControlBlock);

    public:
        CntEnumBase() = delete;
        virtual bool operator==(const ItemBase&) const override;

        virtual bool getPresentation(
            SfxItemPresentation,
            MapUnit,
            MapUnit,
            rtl::OUString&,
            const IntlWrapper&) const override;
        virtual bool queryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
        virtual bool putAnyValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    protected:
        // sal_uInt16 value interface
        virtual sal_uInt16 getEnumValueAsSalUInt16() const = 0;
        virtual void setEnumValueAsSalUInt16(sal_uInt16 nValue) = 0;

    public:
        // bool value interface - optionally supported, for UI stuff
        virtual bool hasBoolValue() const;
        virtual bool getBoolValue() const;
        virtual void setBoolValue(bool bValue);
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    template<typename EnumT> class SAL_DLLPUBLIC_RTTI CntEnum : public CntEnumBase
    {
    private:
        EnumT m_nValue;

    protected:
        // constructor for derived classes that *have* to hand
        // in the to-be-used ItemControlBlock
        CntEnum(ItemControlBlock& rItemControlBlock, EnumT nVal)
        :   CntEnumBase(rItemControlBlock),
            m_nValue(nVal)
        {
        }

    public:
        CntEnum() = delete;

        // value interface
        EnumT getValue() const
        {
            return m_nValue;
        }

        void setValue(EnumT nNew)
        {
            m_nValue = nNew;
        }

    protected:
        // sal_uInt16 value interface
        virtual sal_uInt16 getEnumValueAsSalUInt16() const override
        {
            return static_cast<sal_uInt16>(getValue());
        }

        virtual void setEnumValueAsSalUInt16(sal_uInt16 nValue) override
        {
            setValue(static_cast<EnumT>(nValue));
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_SIMPLE_CNTENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
