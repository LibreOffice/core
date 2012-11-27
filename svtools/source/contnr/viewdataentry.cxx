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

DBG_NAME(SvViewData);

SvViewData::SvViewData()
{
    DBG_CTOR(SvViewData,0);
    nFlags = 0;
    nVisPos = 0;
}

SvViewData::SvViewData( const SvViewData& rData )
{
    DBG_CTOR(SvViewData,0);
    nFlags  = rData.nFlags;
    nFlags &= ~( SVLISTENTRYFLAG_SELECTED | SVLISTENTRYFLAG_FOCUSED );
    nVisPos = rData.nVisPos;
}

SvViewData::~SvViewData()
{
    DBG_DTOR(SvViewData,0);
#ifdef DBG_UTIL
    nVisPos = 0x12345678;
    nFlags = 0x1234;
#endif
}

bool SvViewData::IsSelected() const
{
    return (nFlags & SVLISTENTRYFLAG_SELECTED) != 0;
}

bool SvViewData::IsExpanded() const
{
    return (nFlags & SVLISTENTRYFLAG_EXPANDED) != 0;
}

bool SvViewData::HasFocus() const
{
    return (nFlags & SVLISTENTRYFLAG_FOCUSED) != 0;
}

bool SvViewData::IsCursored() const
{
    return (nFlags & SVLISTENTRYFLAG_CURSORED) != 0;
}

bool SvViewData::IsSelectable() const
{
    return (nFlags & SVLISTENTRYFLAG_NOT_SELECTABLE) == 0;
}

void SvViewData::SetFocus( bool bFocus )
{
    if ( !bFocus )
        nFlags &= (~SVLISTENTRYFLAG_FOCUSED);
    else
        nFlags |= SVLISTENTRYFLAG_FOCUSED;
}

void SvViewData::SetCursored( bool bCursored )
{
    if ( !bCursored )
        nFlags &= (~SVLISTENTRYFLAG_CURSORED);
    else
        nFlags |= SVLISTENTRYFLAG_CURSORED;
}

sal_uInt16 SvViewData::GetFlags() const
{
    return nFlags;
}

void SvViewData::SetSelectable( bool bSelectable )
{
    if( bSelectable )
        nFlags &= (~SVLISTENTRYFLAG_NOT_SELECTABLE);
    else
        nFlags |= SVLISTENTRYFLAG_NOT_SELECTABLE;
}

DBG_NAME(SvViewDataEntry);

SvViewDataEntry::SvViewDataEntry()
    : SvViewData()
{
    DBG_CTOR(SvViewDataEntry,0);
    pItemData = 0;
}

SvViewDataEntry::~SvViewDataEntry()
{
    DBG_DTOR(SvViewDataEntry,0);
    delete [] pItemData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
