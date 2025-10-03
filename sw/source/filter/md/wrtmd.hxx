/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <map>
#include <stack>

#include <rtl/ustring.hxx>

#include <shellio.hxx>
#include <swdllapi.h>

/// Stores information about an SwNode that is at the start or end of a table cell.
struct SwMDCellInfo
{
    bool bCellStart = false;
    bool bCellEnd = false;
    bool bRowStart = false;
    bool bRowEnd = false;
    bool bFirstRowEnd = false;

    // These are only set in the bFirstRowEnd == true case.
    size_t nFirstRowBoxCount = 0;
    std::vector<SvxAdjust> aFirstRowBoxAdjustments;
};

/// Tracks information about one SwTableNode, the instance is alive while the write of the table is
/// in progress.
struct SwMDTableInfo
{
    std::map<SwNodeOffset, SwMDCellInfo> aCellInfos;
    const SwEndNode* pEndNode = nullptr;
};

/// Tracks an anchored object to be exported at a specific document model position.
struct SwMDFly
{
    SwNodeOffset m_nAnchorNodeOffset = {};
    sal_Int32 m_nAnchorContentOffset = {};
    const SwFrameFormat* m_pFrameFormat = nullptr;

    bool operator<(const SwMDFly&) const;
};

class SwMDWriter : public Writer
{
public:
    SW_DLLPUBLIC explicit SwMDWriter(const OUString& rBaseURL);

    bool isInTable() const { return m_bOutTable; }
    SwNodeOffset StartNodeIndex() const { return m_nStartNodeIndex; }
    void SetListLevelPrefixSize(int nListLevel, int nPrefixSize);
    const std::map<int, int>& GetListLevelPrefixSizes() const { return m_aListLevelPrefixSizes; }
    std::stack<SwMDTableInfo>& GetTableInfos() { return m_aTableInfos; }

    void SetTaskListItems(int nTaskListItems) { m_nTaskListItems = nTaskListItems; }
    int GetTaskListItems() const { return m_nTaskListItems; }
    o3tl::sorted_vector<SwMDFly>& GetFlys() { return m_aFlys; }

protected:
    ErrCode WriteStream() override;

private:
    void Out_SwDoc(SwPaM* pPam);
    void CollectFlys();

    bool m_bOutTable = false;
    SwNodeOffset m_nStartNodeIndex{ 0 };
    /// List level -> prefix size map, e.g. "1. " size is 3.
    std::map<int, int> m_aListLevelPrefixSizes;
    std::stack<SwMDTableInfo> m_aTableInfos;
    /// Number of currently open task list items.
    int m_nTaskListItems = 0;
    /// Anchored fly frames, e.g. images.
    o3tl::sorted_vector<SwMDFly> m_aFlys;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
