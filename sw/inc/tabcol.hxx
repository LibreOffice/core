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
#include "swrect.hxx"

struct SwTabColsEntry
{
    SwTwips nPos;
    SwTwips nMin;
    SwTwips nMax;
    bool bHidden; // For each entry a flag, hidden or not.
                  // If the flag bHidden is true column separator
                  // is not in current line. It must maintained
                  // but it may not be displayed.
};

typedef std::vector< SwTabColsEntry > SwTabColsEntries;

class SW_DLLPUBLIC SwTabCols
{
    SwTwips m_nLeftMin;  // Leftmost border (reference point) for
                         // document coordinates.
                         // All other values are relative to this point!
    SwTwips m_nLeft;     // Left border of table.
    SwTwips m_nRight;    // Right border of table.
    SwTwips m_nRightMax; // Maximum right border of table.

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
    SwTwips& operator[]( size_t nPos ) { return m_aData[nPos].nPos; }
    SwTwips operator[]( size_t nPos ) const { return m_aData[nPos].nPos; }
    size_t Count() const { return m_aData.size(); }

    bool IsHidden( size_t nPos ) const         { return m_aData[nPos].bHidden; }
    void SetHidden( size_t nPos, bool bValue ) { m_aData[nPos].bHidden = bValue; }

    void Insert( SwTwips nValue, bool bValue, size_t nPos );
    void Insert( SwTwips nValue, SwTwips nMin, SwTwips nMax, bool bValue, size_t nPos );
    void Remove( size_t nPos, size_t nCount = 1 );

    const SwTabColsEntry& GetEntry( size_t nPos ) const { return m_aData[nPos]; }
          SwTabColsEntry& GetEntry( size_t nPos )  { return m_aData[nPos]; }

    SwTwips GetLeftMin() const { return m_nLeftMin; }
    SwTwips GetLeft()  const { return m_nLeft;    }
    SwTwips GetRight() const { return m_nRight;   }
    SwTwips GetRightMax()const { return m_nRightMax;}

    void SetLeftMin ( SwTwips nNew )   { m_nLeftMin = nNew; }
    void SetLeft    ( SwTwips nNew )   { m_nLeft = nNew;    }
    void SetRight   ( SwTwips nNew )   { m_nRight = nNew;   }
    void SetRightMax( SwTwips nNew )   { m_nRightMax = nNew;}

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
