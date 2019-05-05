/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <svx/item/TransformAnchor.hxx>
#include <item/base/ItemAdministrator.hxx>
#include <item/base/ItemControlBlock.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    ItemControlBlock& TransformAnchor::GetStaticItemControlBlock()
    {
        static ItemControlBlock aItemControlBlock(
            [](){ return new TransformAnchor(); },
            "TransformAnchor");

        return aItemControlBlock;
    }

    TransformAnchor::TransformAnchor(RndStdIds nValue)
    :   CntInt16(
            TransformAnchor::GetStaticItemControlBlock(),
            static_cast<sal_Int16>(nValue))
    {
    }

    std::unique_ptr<ItemBase> TransformAnchor::clone() const
    {
        // use direct value(s) and std::make_unique
        return std::make_unique<TransformAnchor>(GetAnchorType());
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
