/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sbxitem.hxx>
#include <sal/log.hxx>
#include <item/base/ItemAdministrator.hxx>
#include <item/base/ItemControlBlock.hxx>

namespace basctl
{
// I2TM
namespace Item
{
    ::Item::ItemControlBlock& Sbx::GetStaticItemControlBlock()
    {
        static ::Item::ItemControlBlock aItemControlBlock(
            [](){ return new Sbx(ScriptDocument::getApplicationScriptDocument()); },
            "Sbx");

        return aItemControlBlock;
    }

    ::Item::ItemAdministrator& Sbx::SbxData::getItemAdministrator() const
    {
        static ::Item::ItemAdministrator_vector aItemAdministrator_vector(
            // hand over localized lambda call to construct a new instance of Item
            [](){ return new Sbx::SbxData(ScriptDocument::getApplicationScriptDocument()); },
            // hand over localized lambda operator==
            [](ItemData* A, ItemData* B)
            {
                return static_cast<Sbx::SbxData*>(A)->operator==(*static_cast<Sbx::SbxData*>(B));
            });

        return aItemAdministrator_vector;
    }

    Sbx::SbxData::SbxData(
        const ScriptDocument& rDocument,
        const OUString& aLibName,
        const OUString& aName,
        const OUString& aMethodName,
        ItemType eType)
    :   ::Item::ItemBuffered::ItemData(),
        m_aDocument(rDocument),
        m_aLibName(aLibName),
        m_aName(aName),
        m_aMethodName(aMethodName),
        m_eType(eType)
    {
    }

    bool Sbx::SbxData::operator==(const ItemData& rRef) const
    {
        return ItemData::operator==(rRef) || // ptr-compare
            (GetDocument() == static_cast<const SbxData&>(rRef).GetDocument() &&
             GetLibName() == static_cast<const SbxData&>(rRef).GetLibName() &&
             GetName() == static_cast<const SbxData&>(rRef).GetName() &&
             GetMethodName() == static_cast<const SbxData&>(rRef).GetMethodName() &&
             GetType() == static_cast<const SbxData&>(rRef).GetType());
    }

    Sbx::Sbx(
        const ScriptDocument& rDocument,
        const OUString& aLibName,
        const OUString& aName,
        const OUString& aMethodName,
        ItemType eType)
    :   ::Item::ItemBuffered(Sbx::GetStaticItemControlBlock())
    {
        // This call initializes the values and is *required*.
        setItemData(
            new Sbx::SbxData(
                rDocument,
                aLibName,
                aName,
                aMethodName,
                eType));
    }
}
// ~I2TM
} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
