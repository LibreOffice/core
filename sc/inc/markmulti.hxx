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

#ifndef INCLUDED_SC_INC_MARKMULTI_HXX
#define INCLUDED_SC_INC_MARKMULTI_HXX

#include "address.hxx"
#include "segmenttree.hxx"
#include "markarr.hxx"

#include <map>

class ScMultiSel
{

private:
    typedef std::map<SCCOL, ScMarkArray> MapType;
    MapType aMultiSelContainer;
    ScMarkArray aRowSel;

friend class ScMultiSelIter;

public:
    ScMultiSel();
    ScMultiSel( const ScMultiSel& rMultiSel );
    ~ScMultiSel();

    ScMultiSel& operator=(const ScMultiSel& rMultiSel);
    ScMultiSel& operator=(const ScMultiSel&& rMultiSel) = delete;

    SCCOL size() const
    {
        return static_cast<SCCOL>( aMultiSelContainer.size() );
    }

    bool HasMarks( SCCOL nCol ) const;
    bool HasOneMark( SCCOL nCol, SCROW& rStartRow, SCROW& rEndRow ) const;
    bool GetMark( SCCOL nCol, SCROW nRow ) const;
    bool IsAllMarked( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const;
    bool HasEqualRowsMarked( SCCOL nCol1, SCCOL nCol2 ) const;
    SCROW GetNextMarked( SCCOL nCol, SCROW nRow, bool bUp ) const;
    void SetMarkArea( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow, bool bMark );
    bool IsRowMarked( SCROW nRow ) const;
    bool IsRowRangeMarked( SCROW nStartRow, SCROW nEndRow ) const;
    bool IsEmpty() const { return ( !aMultiSelContainer.size() && !aRowSel.HasMarks() ); }
    ScMarkArray GetMarkArray( SCCOL nCol ) const;
    void Clear();
    void MarkAllCols( SCROW nStartRow, SCROW nEndRow );
    bool HasAnyMarks() const;
    void ShiftCols(SCCOL nStartCol, long nColOffset);
    void ShiftRows(SCROW nStartRow, long nRowOffset);

    // For faster access from within ScMarkData, instead of creating
    // ScMultiSelIter with ScFlatBoolRowSegments bottleneck.
    const ScMarkArray& GetRowSelArray() const;
    const ScMarkArray* GetMultiSelArray( SCCOL nCol ) const;
};

class ScMultiSelIter
{

private:
    std::unique_ptr<ScFlatBoolRowSegments>  pRowSegs;
    ScMarkArrayIter                         aMarkArrayIter;
    SCROW nNextSegmentStart;
public:
    ScMultiSelIter( const ScMultiSel& rMultiSel, SCCOL nCol );
    ~ScMultiSelIter();

    bool Next( SCROW& rTop, SCROW& rBottom );
    /** Only to be used by ScMultiSel::IsAllMarked() or otherwise sure that a
        segment tree is actually used. */
    bool GetRangeData( SCROW nRow, ScFlatBoolRowSegments::RangeData& rRowRange ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
