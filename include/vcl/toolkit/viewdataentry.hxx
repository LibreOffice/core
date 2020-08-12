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

#pragma once

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <tools/solar.h>

#include <vector>

class SvTreeListBox;

struct SvViewDataItem
{
    int mnWidth = 0;
    int mnHeight = 0;
};

/**
 * View-dependent data for a tree list entry created in the virtual function
 * SvTreeListBox::CreateViewData(). The item array contains the same number
 * of items as that of the items in its corresponding tree list entry.
 *
 * When an entry is selected, it is logically selected and visually
 * highlighted. When an entry is only highlighted, it looks visually
 * highlighted, but not logically selected.
 */
class SvViewDataEntry
{
    friend class SvTreeList;

    std::vector<SvViewDataItem> maItems;
    sal_uLong nVisPos;
    bool mbSelected:1;
    bool mbHighlighted:1;
    bool mbExpanded:1;
    bool mbFocused:1;
    bool mbSelectable:1;
    bool mbDragTarget:1;

public:
    SvViewDataEntry();
    SvViewDataEntry( const SvViewDataEntry& );
    ~SvViewDataEntry();

    bool IsSelected() const { return mbSelected;}
    bool IsHighlighted() const { return mbHighlighted;}
    bool IsExpanded() const { return mbExpanded;}
    bool HasFocus() const { return mbFocused;}
    bool IsSelectable() const { return mbSelectable;}
    bool IsDragTarget() const { return mbDragTarget;}
    void SetFocus( bool bFocus );
    void SetSelected( bool bSelected );
    void SetExpanded( bool bExpanded );
    void SetSelectable( bool bSelectable );
    void SetDragTarget( bool bDragTarget )
    {
        mbDragTarget = bDragTarget;
    }

    void Init(size_t nSize);

    const SvViewDataItem& GetItem(size_t nPos) const;
    SvViewDataItem& GetItem(size_t nPos);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
