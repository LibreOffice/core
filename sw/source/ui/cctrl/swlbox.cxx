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

using namespace nsSwComboBoxStyle;


//     Description: ListboxElement
SwBoxEntry::SwBoxEntry() :
    bModified(sal_False),
    bNew(sal_False),
    nId(LISTBOX_APPEND)
{
}

SwBoxEntry::SwBoxEntry(const String& aNam, sal_uInt16 nIdx) :
    bModified(sal_False),
    bNew(sal_False),
    aName(aNam),
    nId(nIdx)
{
}

SwBoxEntry::SwBoxEntry(const SwBoxEntry& rOld) :
    bModified(rOld.bModified),
    bNew(rOld.bNew),
    aName(rOld.aName),
    nId(rOld.nId)
{
}

SwComboBox::SwComboBox(Window* pParent, sal_uInt16 nStyleBits) :
    ComboBox(pParent),
    nStyle(nStyleBits)
{
    Init();
}

SwComboBox::SwComboBox(Window* pParent, const ResId& rId, sal_uInt16 nStyleBits) :
    ComboBox(pParent, rId),
    nStyle(nStyleBits)
{
    Init();
}

void SwComboBox::Init()
{
    // create administration for the resource's Stringlist
    sal_uInt16 nSize = GetEntryCount();
    for( sal_uInt16 i=0; i < nSize; ++i )
    {
        SwBoxEntry* pTmp = new SwBoxEntry(ComboBox::GetEntry(i), i);
        aEntryLst.push_back(pTmp);
    }
}

// Basic class Dtor
SwComboBox::~SwComboBox()
{
}

void SwComboBox::InsertEntry(const SwBoxEntry& rEntry)
{
    InsertSorted(new SwBoxEntry(rEntry));
}

void SwComboBox::RemoveEntry(sal_uInt16 nPos)
{
    if(nPos >= aEntryLst.size())
        return;

    // Remove old element
    SwBoxEntry* pEntry = &aEntryLst[nPos];
    ComboBox::RemoveEntry(nPos);

    // Don't add new entries to the list
    if(pEntry->bNew)
    {
        aEntryLst.erase(aEntryLst.begin() + nPos);
    }
    else
    {
        // add to DelEntryLst
        aDelEntryLst.transfer(aDelEntryLst.end(),
                aEntryLst.begin() + nPos, aEntryLst);
    }
}

sal_uInt16 SwComboBox::GetEntryPos(const SwBoxEntry& rEntry) const
{
    return ComboBox::GetEntryPos(rEntry.aName);
}

const SwBoxEntry& SwComboBox::GetEntry(sal_uInt16 nPos) const
{
    if(nPos < aEntryLst.size())
        return aEntryLst[nPos];

    return aDefault;
}

sal_uInt16 SwComboBox::GetRemovedCount() const
{
    return aDelEntryLst.size();
}

const SwBoxEntry& SwComboBox::GetRemovedEntry(sal_uInt16 nPos) const
{
    if(nPos < aDelEntryLst.size())
        return aDelEntryLst[nPos];

    return aDefault;
}

void SwComboBox::InsertSorted(SwBoxEntry* pEntry)
{
    ComboBox::InsertEntry(pEntry->aName);
    sal_uInt16 nPos = ComboBox::GetEntryPos(pEntry->aName);
    aEntryLst.insert( aEntryLst.begin() + nPos, pEntry );
}

void SwComboBox::KeyInput( const KeyEvent& rKEvt )
{
    sal_uInt16 nChar = rKEvt.GetCharCode();

    if(nStyle & CBS_FILENAME)
    {
#if defined UNX
        if(nChar == '/' || nChar == ' ' )
            return;
#else
        if(nChar == ':' || nChar == '\\' || nChar == '.' || nChar == ' ')
            return;
#endif
    }
    ComboBox::KeyInput(rKEvt);
}

// Convert text according to option
OUString SwComboBox::GetText() const
{
    OUString aTxt( ComboBox::GetText() );

    if(nStyle & CBS_LOWER)
        aTxt = GetAppCharClass().lowercase( aTxt );
    else if( nStyle & CBS_UPPER )
        aTxt = GetAppCharClass().uppercase( aTxt );

    return aTxt;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
