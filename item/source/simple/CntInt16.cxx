/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <item/simple/CntInt16.hxx>
#include <item/base/ItemControlBlock.hxx>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    CntInt16::CntInt16(ItemControlBlock& rItemControlBlock, sal_Int16 nVal)
    :   ItemBase(rItemControlBlock),
        m_nValue(nVal)
    {
    }

    bool CntInt16::operator==(const ItemBase& rRef) const
    {
        return ItemBase::operator==(rRef) || // ptr-compare
            getValue() == static_cast<const CntInt16&>(rRef).getValue();
    }

    bool CntInt16::getPresentation(
        SfxItemPresentation,
        MapUnit,
        MapUnit,
        rtl::OUString& rText,
        const IntlWrapper&) const
    {
        rText = rtl::OUString::number(m_nValue);
        return true;
    }

    bool CntInt16::queryValue(css::uno::Any& rVal, sal_uInt8) const
    {
        sal_Int16 nValue = m_nValue;
        rVal <<= nValue;
        return true;
    }

    bool CntInt16::putAnyValue(const css::uno::Any& rVal, sal_uInt8)
    {
        sal_Int16 nValue(0);

        if(rVal >>= nValue)
        {
            m_nValue = nValue;
            return true;
        }

        assert(false && "CntInt16::putAnyValue - Wrong type!");
        return false;
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
