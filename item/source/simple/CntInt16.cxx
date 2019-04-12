/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <item/simple/CntInt16.hxx>
#include <item/base/ItemAdministrator.hxx>
#include <item/base/ItemControlBlock.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ItemControlBlock& CntInt16::GetStaticItemControlBlock()
    {
        static ItemControlBlock aItemControlBlock(
            std::shared_ptr<ItemAdministrator>(new IAdministrator_set()),
            nullptr,
            [](){ return new CntInt16(CntInt16::GetStaticItemControlBlock()); },
            "CntInt16");

        return aItemControlBlock;
    }

    CntInt16::CntInt16(
        ItemControlBlock& rItemControlBlock,
        sal_Int16 nValue)
    :   ItemBase(rItemControlBlock),
        m_nValue(nValue)
    {
    }

    CntInt16::~CntInt16()
    {
        // needs to be called from here to have the fully derived implementation type
        // in the helper method - do NOT move to a imaginable general
        // implementation in ItemBaseStaticHelper (!)
        implInstanceCleanup();
    }

    std::shared_ptr<const CntInt16> CntInt16::Create(sal_Int16 nValue)
    {
        // use ::Create(...) method with local incarnation, it will handle
        // - detection of being default (will delete local incarnation)
        // - detection of reuse (will delete local incarnation)
        // - detectiomn of new use - will create shared_ptr for local incarnation and buffer
        return std::static_pointer_cast<const CntInt16>(
            CntInt16::GetStaticItemControlBlock().GetItemAdministrator()->Create(
                new CntInt16(
                    CntInt16::GetStaticItemControlBlock(),
                    nValue)));
    }

    bool CntInt16::operator==(const ItemBase& rCandidate) const
    {
        if(ItemBase::operator==(rCandidate)) // compares ptrs
        {
            return true;
        }

        return (GetValue() == static_cast<const CntInt16&>(rCandidate).GetValue());
    }

    bool CntInt16::operator<(const ItemBase& rCandidate) const
    {
        return static_cast<const CntInt16*>(this)->GetValue() < static_cast<const CntInt16&>(rCandidate).GetValue();
    }

    sal_Int16 CntInt16::GetValue() const
    {
        return m_nValue;
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
