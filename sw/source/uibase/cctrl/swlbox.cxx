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

#include <osl/diagnose.h>
#include <unotools/charclass.hxx>
#include <swtypes.hxx>
#include <swlbox.hxx>

//     Description: ListboxElement
SwBoxEntry::SwBoxEntry() :
    bNew(false)
{
}

SwBoxEntry::SwBoxEntry(const OUString& aNam) :
    bNew(false),
    aName(aNam)
{
}

SwBoxEntry::SwBoxEntry(const SwBoxEntry& rOld) :
    bNew(rOld.bNew),
    aName(rOld.aName)
{
}

SwComboBox::SwComboBox(vcl::Window* pParent, WinBits nStyle)
    : ComboBox(pParent, nStyle)
{
    Init();
}

void SwComboBox::Init()
{
    // create administration for the resource's Stringlist
    sal_Int32 nSize = GetEntryCount();
    for( sal_Int32 i=0; i < nSize; ++i )
    {
        m_EntryList.push_back(SwBoxEntry(ComboBox::GetEntry(i)));
    }
}

// Basic class Dtor
SwComboBox::~SwComboBox()
{
}

void SwComboBox::InsertSwEntry(const SwBoxEntry& rEntry)
{
    InsertSorted(rEntry);
}

sal_Int32 SwComboBox::InsertEntry(const OUString& rStr, sal_Int32)
{
    InsertSwEntry(SwBoxEntry(rStr));
    return 0;
}

void SwComboBox::RemoveEntryAt(sal_Int32 const nPos)
{
    if (nPos < 0 || static_cast<size_t>(nPos) >= m_EntryList.size())
        return;

    // Remove old element
    SwBoxEntry const& rEntry = m_EntryList[nPos];
    ComboBox::RemoveEntryAt(nPos);

    // Don't add new entries to the list
    if (rEntry.bNew)
    {
        m_EntryList.erase(m_EntryList.begin() + nPos);
    }
    else
    {
        // add to DelEntryLst
        m_DelEntryList.push_back(m_EntryList[nPos]);
        m_EntryList.erase(m_EntryList.begin() + nPos);
    }
}

sal_Int32 SwComboBox::GetSwEntryPos(const SwBoxEntry& rEntry) const
{
    return ComboBox::GetEntryPos(rEntry.aName);
}

const SwBoxEntry& SwComboBox::GetSwEntry(sal_Int32 const nPos) const
{
    if (0 <= nPos && static_cast<size_t>(nPos) < m_EntryList.size())
        return m_EntryList[nPos];

    return aDefault;
}

sal_Int32 SwComboBox::GetRemovedCount() const
{
    return static_cast<sal_Int32>(m_DelEntryList.size());
}

const SwBoxEntry& SwComboBox::GetRemovedEntry(sal_Int32 nPos) const
{
    if (0 <= nPos && static_cast<size_t>(nPos) < m_DelEntryList.size())
        return m_DelEntryList[nPos];

    return aDefault;
}

void SwComboBox::InsertSorted(SwBoxEntry const& rEntry)
{
    ComboBox::InsertEntry(rEntry.aName);
    sal_Int32 nPos = ComboBox::GetEntryPos(rEntry.aName);
    m_EntryList.insert(m_EntryList.begin() + nPos, rEntry);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
