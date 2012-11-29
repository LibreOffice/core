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

#ifndef __SVTOOLS_VIEWDATAENTRY_HXX__
#define __SVTOOLS_VIEWDATAENTRY_HXX__

#include "svtdllapi.h"
#include "tools/solar.h"
#include "tools/gen.hxx"

#include <vector>

// Entryflags that are attached to the View
#define SVLISTENTRYFLAG_SELECTED        0x0001
#define SVLISTENTRYFLAG_EXPANDED        0x0002
#define SVLISTENTRYFLAG_FOCUSED         0x0004
#define SVLISTENTRYFLAG_CURSORED        0x0008
#define SVLISTENTRYFLAG_NOT_SELECTABLE  0x0010

struct SvViewDataItem
{
    Size maSize;
};

/**
 * View-dependent data for an Entry is created in the virtual function
 * SvTreeListBox::CreateViewData. The View creation of Items should not be
 * changed.
 */
class SVT_DLLPUBLIC SvViewDataEntry
{
    friend class SvTreeList;
    friend class SvListView;

    std::vector<SvViewDataItem> maItems;
    sal_uLong nVisPos;
    sal_uInt16 nFlags;

public:
    SvViewDataEntry();
    SvViewDataEntry( const SvViewDataEntry& );
    ~SvViewDataEntry();

    bool IsSelected() const;
    bool IsExpanded() const;
    bool HasFocus() const;
    bool IsCursored() const;
    bool IsSelectable() const;
    void SetFocus( bool bFocus );
    void SetCursored( bool bCursored );
    void SetSelected( bool bSelected );
    void SetExpanded( bool bExpanded );
    sal_uInt16 GetFlags() const;
    void SetSelectable( bool bSelectable );

    void Init(size_t nSize);

    const SvViewDataItem* GetItem(size_t nPos) const;
    SvViewDataItem* GetItem(size_t nPos);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
