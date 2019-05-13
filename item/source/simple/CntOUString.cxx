/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <item/simple/CntOUString.hxx>
#include <item/base/ItemControlBlock.hxx>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    CntOUString::CntOUString(ItemControlBlock& rItemControlBlock, const rtl::OUString& rValue)
    :   ItemBase(rItemControlBlock),
        m_aValue(rValue)
    {
    }

    bool CntOUString::operator==(const ItemBase& rRef) const
    {
        return ItemBase::operator==(rRef) || // ptr-compare
            getValue() == static_cast<const CntOUString&>(rRef).getValue();
    }

    bool CntOUString::getPresentation(
        SfxItemPresentation,
        MapUnit,
        MapUnit,
        rtl::OUString& rText,
        const IntlWrapper&) const
    {
        rText = m_aValue;
        return true;
    }

    bool CntOUString::queryValue(css::uno::Any& rVal, sal_uInt8) const
    {
        rVal <<= m_aValue;
        return true;
    }

    bool CntOUString::putAnyValue(const css::uno::Any& rVal, sal_uInt8)
    {
        rtl::OUString aTheValue;

        if(rVal >>= aTheValue)
        {
            m_aValue = aTheValue;
            return true;
        }

        assert(false && "CntOUString::putAnyValue - Wrong type!");
        return false;
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
