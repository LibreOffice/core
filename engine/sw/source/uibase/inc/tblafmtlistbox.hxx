/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
#include <limits>
#include <vector>
#include <sal/types.h>
#include <vcl/weld.hxx>
#include <tblafmt.hxx>

/// The catalog index of the table style in each row of a dialog's style list box, for the
/// rows below the leading rows the dialog adds itself ("- none -"). A hidden catalog style has
/// no row.
class SwTableStyleListBoxIndexes
{
    std::vector<size_t> m_aTableIndexes;
    int m_nFirstStyleRow = 0;

public:
    /// The catalog index that stands for no style.
    static constexpr size_t nNoStyle = std::numeric_limits<size_t>::max();

    /// Append one row per style of rTable that is not hidden, in catalog order, below the
    /// nFirstStyleRow rows the list box already holds.
    void Fill(weld::TreeView& rListBox, const SwTableAutoFormatTable& rTable, int nFirstStyleRow)
    {
        m_aTableIndexes.clear();
        m_nFirstStyleRow = nFirstStyleRow;
        for (size_t i = 0; i < rTable.size(); ++i)
        {
            const SwTableAutoFormat& rFormat = rTable[i];
            if (rFormat.IsHidden())
                continue;
            m_aTableIndexes.push_back(i);
            rListBox.append_text(rFormat.GetUIName().toString());
        }
    }

    /// The row of the style with catalog index nTableIndex, or -1 when it has no row.
    int RowOf(size_t nTableIndex) const
    {
        auto it = std::find(m_aTableIndexes.begin(), m_aTableIndexes.end(), nTableIndex);
        if (it == m_aTableIndexes.end())
            return -1;
        return m_nFirstStyleRow + static_cast<int>(it - m_aTableIndexes.begin());
    }

    /// The catalog index of the style in row nRow, or nNoStyle for a row that holds no style.
    size_t TableIndexAt(int nRow) const
    {
        const int nPos = nRow - m_nFirstStyleRow;
        if (nPos < 0 || nPos >= static_cast<int>(m_aTableIndexes.size()))
            return nNoStyle;
        return m_aTableIndexes[nPos];
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
