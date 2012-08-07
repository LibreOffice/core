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

SbxItem::SbxItem(sal_uInt16 nWhich_, const ScriptDocument& rDocument, const ::rtl::OUString& aLibName, const ::rtl::OUString& aName, BasicIDEType nType)
    :SfxPoolItem( nWhich_ )
    ,m_aDocument(rDocument)
    ,m_aLibName(aLibName)
    ,m_aName(aName)
    ,m_nType(nType)
{
}

SbxItem::SbxItem(sal_uInt16 nWhich_, const ScriptDocument& rDocument, const ::rtl::OUString& aLibName, const ::rtl::OUString& aName, const ::rtl::OUString& aMethodName, BasicIDEType nType)
    :SfxPoolItem( nWhich_ )
    ,m_aDocument(rDocument)
    ,m_aLibName(aLibName)
    ,m_aName(aName)
    ,m_aMethodName(aMethodName)
    ,m_nType(nType)
{
}

SbxItem::SbxItem(const SbxItem& rCopy)
    :SfxPoolItem(rCopy)
    ,m_aDocument(rCopy.m_aDocument)
    ,m_aLibName(rCopy.m_aLibName)
    ,m_aName(rCopy.m_aName)
    ,m_aMethodName(rCopy.m_aMethodName)
    ,m_nType(rCopy.m_nType)
{
}

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
        m_nType == pSbxItem->m_nType;
}

const ScriptDocument& SbxItem::GetDocument() const
{
    return m_aDocument;
}

const ::rtl::OUString& SbxItem::GetLibName() const
{
    return m_aLibName;
}

const ::rtl::OUString& SbxItem::GetName() const
{
    return m_aName;
}

const ::rtl::OUString& SbxItem::GetMethodName() const
{
    return m_aMethodName;
}

BasicIDEType SbxItem::GetType() const
{
    return m_nType;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
