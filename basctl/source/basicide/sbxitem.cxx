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

#include "sbxitem.hxx"

namespace basctl
{

SbxItem::SbxItem (
    sal_uInt16 nWhichItem,
    ScriptDocument const& rDocument,
    rtl::OUString const& aLibName,
    rtl::OUString const& aName,
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
    rtl::OUString const& aLibName,
    rtl::OUString const& aName,
    rtl::OUString const& aMethodName,
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

int SbxItem::operator==(const SfxPoolItem& rCmp) const
{
    SbxItem const* pSbxItem = dynamic_cast<SbxItem const*>(&rCmp);
    DBG_ASSERT(pSbxItem, "==: no SbxItem!" );
    return
        SfxPoolItem::operator==(rCmp) &&
        m_aDocument == pSbxItem->m_aDocument &&
        m_aLibName == pSbxItem->m_aLibName &&
        m_aName == pSbxItem->m_aName &&
        m_aMethodName == pSbxItem->m_aMethodName &&
        m_eType == pSbxItem->m_eType;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
