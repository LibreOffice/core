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

#include "address.hxx"
#include <tools/long.hxx>
#include <vector>

class ScRangeList;
struct ScSheetLimits;

struct ScMarkEntry
{
    SCROW           nRow : 30; // 30 because 31 causes compiler problems with VisualStudio
    bool            bMarked : 1;

    bool operator==(const ScMarkEntry& rOther) const
    { return nRow == rOther.nRow && bMarked == rOther.bMarked; }
};

/**
  This is a rather odd datastructure. We store alternating marked/not-marked entries,
  and for each entry the range is defined as :
      [previousEntry.nRow+1, currentEntry.nRow]
*/
class ScMarkArray
{
    const ScSheetLimits &       mrSheetLimits;
    std::vector<ScMarkEntry>    mvData;

friend class ScMarkArrayIter;
friend class ScDocument;                // for FillInfo

public:
    SC_DLLPUBLIC ScMarkArray( const ScSheetLimits& rLimits );
            ScMarkArray( ScMarkArray&& rArray ) noexcept;
            ScMarkArray( const ScMarkArray& rArray );
    void    Reset( bool bMarked = false, SCSIZE nNeeded = 1 );
    bool    GetMark( SCROW nRow ) const;
    SC_DLLPUBLIC void SetMarkArea( SCROW nStartRow, SCROW nEndRow, bool bMarked );
    void    Set( std::vector<ScMarkEntry> && );
    bool    IsAllMarked( SCROW nStartRow, SCROW nEndRow ) const;
    bool    HasOneMark( SCROW& rStartRow, SCROW& rEndRow ) const;

    bool    HasMarks() const    { return mvData.size() > 1 || ( mvData.size() == 1 && mvData[0].bMarked ); }

    ScMarkArray& operator=( ScMarkArray const & rSource );
    ScMarkArray& operator=(ScMarkArray&& rSource) noexcept;
    bool operator==(ScMarkArray const & rOther ) const;

    SC_DLLPUBLIC bool Search( SCROW nRow, SCSIZE& nIndex ) const;

    /// Including current row, may return -1 if bUp and not found
    SCROW   GetNextMarked( SCROW nRow, bool bUp ) const;
    SCROW   GetMarkEnd( SCROW nRow, bool bUp ) const;

    void    Shift( SCROW nStartRow, tools::Long nOffset );
    void    Intersect( const ScMarkArray& rOther );
};

class SC_DLLPUBLIC ScMarkArrayIter // iterate over selected range
{
    const ScMarkArray*  pArray;
    SCSIZE              nPos;
public:
                ScMarkArrayIter( const ScMarkArray* pNewArray );

    bool        Next( SCROW& rTop, SCROW& rBottom );
    void        reset( const ScMarkArray* pNewArray );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
