/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <item/simple/CntEnum.hxx>
#include <item/base/ItemControlBlock.hxx>
#include <comphelper/extract.hxx>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    CntEnumBase::CntEnumBase(ItemControlBlock& rItemControlBlock)
    :   ItemBase(rItemControlBlock)
    {
    }

    bool CntEnumBase::operator==(const ItemBase& rRef) const
    {
        return ItemBase::operator==(rRef) || // ptr-compare
            getEnumValueAsSalUInt16() == static_cast<const CntEnumBase&>(rRef).getEnumValueAsSalUInt16();
    }

    bool CntEnumBase::getPresentation(
        SfxItemPresentation,
        MapUnit,
        MapUnit,
        rtl::OUString& rText,
        const IntlWrapper&) const
    {
        rText = rtl::OUString::number(getEnumValueAsSalUInt16());
        return true;
    }

    bool CntEnumBase::queryValue(css::uno::Any& rVal, sal_uInt8) const
    {
        rVal <<= sal_Int32(getEnumValueAsSalUInt16());
        return true;
    }

    bool CntEnumBase::putAnyValue(const css::uno::Any& rVal, sal_uInt8)
    {
        sal_Int32 nTheValue(0);

        if(::cppu::enum2int(nTheValue, rVal))
        {
            setEnumValueAsSalUInt16(static_cast<sal_uInt16>(nTheValue));
            return true;
        }

        assert(false && "CntEnumBase::putAnyValue - Wrong type!");
        return false;
    }

    bool CntEnumBase::hasBoolValue() const
    {
        return false;
    }

    bool CntEnumBase::getBoolValue() const
    {
        return false;
    }

    void CntEnumBase::setBoolValue(bool /*bValue*/)
    {
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
