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
    ItemControlBlock& CntOUString::GetStaticItemControlBlock()
    {
        static ItemControlBlock aItemControlBlock(
            [](){ return new CntOUString(); },
            [](const ItemBase& rRef){ return new CntOUString(static_cast<const CntOUString&>(rRef)); },
            "CntOUString");

        return aItemControlBlock;
    }

    CntOUString::CntOUString(const rtl::OUString& rValue)
    :   ItemBase(CntOUString::GetStaticItemControlBlock()),
        m_aValue(rValue)
    {
    }

    bool CntOUString::operator==(const ItemBase& rRef) const
    {
        return ItemBase::operator==(rRef) || // ptr-compare
            getValue() == static_cast<const CntOUString&>(rRef).getValue();
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
