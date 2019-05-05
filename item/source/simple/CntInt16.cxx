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
    ItemControlBlock& CntInt16::GetStaticItemControlBlock()
    {
        static ItemControlBlock aItemControlBlock(
            [](){ return new CntInt16(); },
            "CntInt16");

        return aItemControlBlock;
    }

    CntInt16::CntInt16(ItemControlBlock& rItemControlBlock, sal_Int16 nVal)
    :   ItemBase(rItemControlBlock),
        m_nValue(nVal)
    {
    }

    CntInt16::CntInt16(sal_Int16 nVal)
    :   ItemBase(CntInt16::GetStaticItemControlBlock()),
        m_nValue(nVal)
    {
    }

    bool CntInt16::operator==(const ItemBase& rRef) const
    {
        return ItemBase::operator==(rRef) || // ptr-compare
            getValue() == static_cast<const CntInt16&>(rRef).getValue();
    }

    std::unique_ptr<ItemBase> CntInt16::clone() const
    {
        // use direct value(s) and std::make_unique
        return std::make_unique<CntInt16>(getValue());
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
