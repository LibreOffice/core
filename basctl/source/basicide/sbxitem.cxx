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

namespace basctl
{
// I2TM
namespace Item
{
    ::Item::ItemControlBlock& Sbx::GetStaticItemControlBlock()
    {
        static ::Item::ItemControlBlock aItemControlBlock(
            std::shared_ptr<::Item::ItemAdministrator>(new ::Item::IAdministrator_vector()),
            std::shared_ptr<const ::Item::ItemBase>(new Sbx()),
            [](){ return new Sbx(); });

        return aItemControlBlock;
    }

    ::Item::ItemControlBlock& Sbx::GetItemControlBlock() const
    {
        return Sbx::GetStaticItemControlBlock();
    }

    Sbx::Sbx(const ScriptDocument* pDocument, const OUString& aLibName, const OUString& aName, const OUString& aMethodName, ItemType eType)
    :   ::Item::ItemBase(),
        m_aDocument(nullptr != pDocument ? *pDocument : ScriptDocument::getApplicationScriptDocument()),
        m_aLibName(aLibName),
        m_aName(aName),
        m_aMethodName(aMethodName),
        m_eType(eType)
    {
    }

    Sbx::~Sbx()
    {
        implInstanceCleanup();
    }

    std::shared_ptr<const Sbx> Sbx::Create(
        const ScriptDocument& rDocument,
        const OUString& aLibName,
        const OUString& aName,
        const OUString& aMethodName,
        ItemType eType)
    {
        return std::static_pointer_cast<const Sbx>(
            Sbx::GetStaticItemControlBlock().GetItemAdministrator()->Create(
                new Sbx(&rDocument, aLibName, aName, aMethodName, eType)));
    }

    bool Sbx::operator==(const ItemBase& rCandidate) const
    {
        assert(ItemBase::operator==(rCandidate));
        const Sbx& rCand(static_cast<const Sbx&>(rCandidate));
        return (GetDocument() == rCand.GetDocument()
            && GetLibName() == rCand.GetLibName()
            && GetName() == rCand.GetName()
            && GetMethodName() == rCand.GetMethodName()
            && GetType() == rCand.GetType());
    }
}
// ~I2TM
} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
