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

#include "svtools/viewdataentry.hxx"

#include "tools/debug.hxx"

// Entryflags that are attached to the View
#define SVLISTENTRYFLAG_SELECTED        0x0001
#define SVLISTENTRYFLAG_EXPANDED        0x0002
#define SVLISTENTRYFLAG_FOCUSED         0x0004
#define SVLISTENTRYFLAG_CURSORED        0x0008
#define SVLISTENTRYFLAG_NOT_SELECTABLE  0x0010

DBG_NAME(SvViewDataEntry);

SvViewDataEntry::SvViewDataEntry() :
    nVisPos(0), nFlags(0)
{
    DBG_CTOR(SvViewDataEntry,0);
}

SvViewDataEntry::SvViewDataEntry( const SvViewDataEntry& rData ) :
    nVisPos(rData.nVisPos), nFlags(rData.nFlags)
{
    DBG_CTOR(SvViewDataEntry,0);
    nFlags &= ~( SVLISTENTRYFLAG_SELECTED | SVLISTENTRYFLAG_FOCUSED );
}

SvViewDataEntry::~SvViewDataEntry()
{
    DBG_DTOR(SvViewDataEntry,0);
#ifdef DBG_UTIL
    nVisPos = 0x12345678;
    nFlags = 0x1234;
#endif
}

bool SvViewDataEntry::IsSelected() const
{
    return (nFlags & SVLISTENTRYFLAG_SELECTED) != 0;
}

bool SvViewDataEntry::IsExpanded() const
{
    return (nFlags & SVLISTENTRYFLAG_EXPANDED) != 0;
}

bool SvViewDataEntry::HasFocus() const
{
    return (nFlags & SVLISTENTRYFLAG_FOCUSED) != 0;
}

bool SvViewDataEntry::IsCursored() const
{
    return (nFlags & SVLISTENTRYFLAG_CURSORED) != 0;
}

bool SvViewDataEntry::IsSelectable() const
{
    return (nFlags & SVLISTENTRYFLAG_NOT_SELECTABLE) == 0;
}

void SvViewDataEntry::SetFocus( bool bFocus )
{
    if ( !bFocus )
        nFlags &= (~SVLISTENTRYFLAG_FOCUSED);
    else
        nFlags |= SVLISTENTRYFLAG_FOCUSED;
}

void SvViewDataEntry::SetCursored( bool bCursored )
{
    if ( !bCursored )
        nFlags &= (~SVLISTENTRYFLAG_CURSORED);
    else
        nFlags |= SVLISTENTRYFLAG_CURSORED;
}

void SvViewDataEntry::SetSelected( bool bSelected )
{
    if ( !bSelected )
        nFlags &= (~SVLISTENTRYFLAG_SELECTED);
    else
        nFlags |= SVLISTENTRYFLAG_SELECTED;
}

void SvViewDataEntry::SetExpanded( bool bExpanded )
{
    if ( !bExpanded )
        nFlags &= (~SVLISTENTRYFLAG_EXPANDED);
    else
        nFlags |= SVLISTENTRYFLAG_EXPANDED;
}

void SvViewDataEntry::SetSelectable( bool bSelectable )
{
    if( bSelectable )
        nFlags &= (~SVLISTENTRYFLAG_NOT_SELECTABLE);
    else
        nFlags |= SVLISTENTRYFLAG_NOT_SELECTABLE;
}

void SvViewDataEntry::Init(size_t nSize)
{
    maItems.resize(nSize);
}

const SvViewDataItem* SvViewDataEntry::GetItem(size_t nPos) const
{
    return &maItems[nPos];
}

SvViewDataItem* SvViewDataEntry::GetItem(size_t nPos)
{
    return &maItems[nPos];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
