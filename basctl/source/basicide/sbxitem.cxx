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

namespace basctl
{

#ifdef ENABLE_ITEMS
namespace Item
{
Sbx::Sbx(const ScriptDocument* pDocument, const OUString& aLibName, const OUString& aName, const OUString& aMethodName, ItemType eType)
:   SbxStaticHelper(),
    m_aDocument(nullptr != pDocument ? *pDocument : ScriptDocument::getApplicationScriptDocument()),
    m_aLibName(aLibName),
    m_aName(aName),
    m_aMethodName(aMethodName),
    m_eType(eType)
{
}

Sbx::~Sbx()
{
    if(IsAdministrated())
    {
        GetStaticAdmin().HintExpired(this);
    }
}

std::shared_ptr<const Sbx> Sbx::Create(const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aName, ItemType eType)
{
    return std::static_pointer_cast<const Sbx>(GetStaticAdmin().Create(new Sbx(&rDocument, aLibName, aName, OUString(), eType)));
}

std::shared_ptr<const Sbx> Sbx::Create(const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aName, const OUString& aMethodName, ItemType eType)
{
    return std::static_pointer_cast<const Sbx>(GetStaticAdmin().Create(new Sbx(&rDocument, aLibName, aName, aMethodName, eType)));
}

bool Sbx::operator==(const IBase& rCandidate) const
{
    assert(IBase::operator==(rCandidate));
    const Sbx& rCand(static_cast<const Sbx&>(rCandidate));
    return (GetDocument() == rCand.GetDocument()
        && GetLibName() == rCand.GetLibName()
        && GetName() == rCand.GetName()
        && GetMethodName() == rCand.GetMethodName()
        && GetType() == rCand.GetType());
}
}
#endif

SfxPoolItem* SbxItem::CreateDefault() { SAL_WARN( "basctl.basicide", "No SbxItem factory available"); return nullptr; }
SbxItem::SbxItem (
    sal_uInt16 nWhichItem,
    ScriptDocument const& rDocument,
    OUString const& aLibName,
    OUString const& aName,
    ItemType eType
) :
    SfxPoolItem(nWhichItem),
    m_aDocument(rDocument),
    m_aLibName(aLibName),
    m_aName(aName),
    m_eType(eType)
{ }

SbxItem::SbxItem (
    sal_uInt16 nWhichItem,
    ScriptDocument const& rDocument,
    OUString const& aLibName,
    OUString const& aName,
    OUString const& aMethodName,
    ItemType eType
) :
    SfxPoolItem(nWhichItem),
    m_aDocument(rDocument),
    m_aLibName(aLibName),
    m_aName(aName),
    m_aMethodName(aMethodName),
    m_eType(eType)
{ }

SfxPoolItem *SbxItem::Clone(SfxItemPool*) const
{
    return new SbxItem(*this);
}

bool SbxItem::operator==(const SfxPoolItem& rCmp) const
{
    SbxItem const* pSbxItem = dynamic_cast<SbxItem const*>(&rCmp);
    assert(pSbxItem); //no SbxItem!
    return
        SfxPoolItem::operator==(rCmp) &&
        pSbxItem &&
        m_aDocument == pSbxItem->m_aDocument &&
        m_aLibName == pSbxItem->m_aLibName &&
        m_aName == pSbxItem->m_aName &&
        m_aMethodName == pSbxItem->m_aMethodName &&
        m_eType == pSbxItem->m_eType;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
