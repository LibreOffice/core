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
#ifndef INCLUDED_SW_INC_TABCOL_HXX
#define INCLUDED_SW_INC_TABCOL_HXX

#include <vector>
#include <memory>

#include "swdllapi.h"
#include <tools/long.hxx>

struct SwTabColsEntry
{
    tools::Long nPos;
    tools::Long nMin;
    tools::Long nMax;
    bool bHidden; // For each entry a flag, hidden or not.
                  // If the flag bHidden is true column separator
                  // is not in current line. It must maintained
                  // but it may not be displayed.
};

typedef std::vector< SwTabColsEntry > SwTabColsEntries;

class SW_DLLPUBLIC SwTabCols
{
    tools::Long m_nLeftMin;  // Leftmost border (reference point) for
                      // document coordinates.
                      // All other values are relative to this point!
    tools::Long m_nLeft;     // Left border of table.
    tools::Long m_nRight;    // Right border of table.
    tools::Long m_nRightMax; // Maximum right border of table.

    bool m_bLastRowAllowedToChange; // If the last row of the table frame
                                    // is split across pages, it may not
                                    // change its size.

    SwTabColsEntries m_aData;

    //For the CopyCTor.
    const SwTabColsEntries& GetData() const { return m_aData; }

public:
    SwTabCols( sal_uInt16 nSize = 0 );
    SwTabCols( const SwTabCols& );
    SwTabCols &operator=( const SwTabCols& );
    tools::Long& operator[]( size_t nPos ) { return m_aData[nPos].nPos; }
    tools::Long operator[]( size_t nPos ) const { return m_aData[nPos].nPos; }
    size_t Count() const { return m_aData.size(); }

    bool IsHidden( size_t nPos ) const         { return m_aData[nPos].bHidden; }
    void SetHidden( size_t nPos, bool bValue ) { m_aData[nPos].bHidden = bValue; }

    void Insert( tools::Long nValue, bool bValue, size_t nPos );
    void Insert( tools::Long nValue, tools::Long nMin, tools::Long nMax, bool bValue, size_t nPos );
    void Remove( size_t nPos, size_t nCount = 1 );

    const SwTabColsEntry& GetEntry( size_t nPos ) const { return m_aData[nPos]; }
          SwTabColsEntry& GetEntry( size_t nPos )  { return m_aData[nPos]; }

    tools::Long GetLeftMin() const { return m_nLeftMin; }
    tools::Long GetLeft()  const { return m_nLeft;    }
    tools::Long GetRight() const { return m_nRight;   }
    tools::Long GetRightMax()const { return m_nRightMax;}

    void SetLeftMin ( tools::Long nNew )   { m_nLeftMin = nNew; }
    void SetLeft    ( tools::Long nNew )   { m_nLeft = nNew;    }
    void SetRight   ( tools::Long nNew )   { m_nRight = nNew;   }
    void SetRightMax( tools::Long nNew )   { m_nRightMax = nNew;}

    bool IsLastRowAllowedToChange() const { return m_bLastRowAllowedToChange; }
    void SetLastRowAllowedToChange( bool bNew ) { m_bLastRowAllowedToChange = bNew; }
};

class SwTable;
class SwTabFrame;
class SwFrame;

struct SwColCache {
    std::unique_ptr<SwTabCols> pLastCols;
    SwTable const* pLastTable  = nullptr;
    SwTabFrame const* pLastTabFrame = nullptr;
    SwFrame const* pLastCellFrame = nullptr;
};

#endif // INCLUDED_SW_INC_TABCOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
