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

#include <vcl/toolkit/viewdataentry.hxx>

SvViewDataEntry::SvViewDataEntry() :
    nVisPos(0),
    mbSelected(false),
    mbHighlighted(false),
    mbExpanded(false),
    mbFocused(false),
    mbSelectable(true),
    mbDragTarget(false)
{
}

SvViewDataEntry::SvViewDataEntry( const SvViewDataEntry& rData ) :
    nVisPos(rData.nVisPos),
    mbSelected(false),
    mbHighlighted(false),
    mbExpanded(rData.mbExpanded),
    mbFocused(false),
    mbSelectable(rData.mbSelectable),
    mbDragTarget(false)
{
}

SvViewDataEntry::~SvViewDataEntry()
{
#ifdef DBG_UTIL
    nVisPos = 0x12345678;
#endif
}

void SvViewDataEntry::SetFocus( bool bFocus )
{
    mbFocused = bFocus;
}

void SvViewDataEntry::SetSelected( bool bSelected )
{
    mbSelected = bSelected;
    mbHighlighted = bSelected;
}

void SvViewDataEntry::SetExpanded( bool bExpanded )
{
    mbExpanded = bExpanded;
}

void SvViewDataEntry::SetSelectable( bool bSelectable )
{
    mbSelectable = bSelectable;
}

void SvViewDataEntry::Init(size_t nSize)
{
    maItems.resize(nSize);
}

const SvViewDataItem& SvViewDataEntry::GetItem(size_t nPos) const
{
    return maItems[nPos];
}

SvViewDataItem& SvViewDataEntry::GetItem(size_t nPos)
{
    return maItems[nPos];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
