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

#ifndef INCLUDED_SC_INC_MARKARR_HXX
#define INCLUDED_SC_INC_MARKARR_HXX

#include "address.hxx"
#include <vector>

struct ScMarkEntry
{
    SCROW           nRow;
    bool            bMarked;
    bool operator ==( const ScMarkEntry &rOther ) const { return bMarked == rOther.bMarked && nRow == rOther.nRow; };
};

class ScMarkArray
{
    std::vector <ScMarkEntry> m_data;

friend class ScMarkArrayIter;
friend class ScDocument;                // for FillInfo

public:
            ScMarkArray();
            ScMarkArray( ScMarkArray&& rArray );
            ~ScMarkArray();
    bool    operator ==( const ScMarkArray &rOther ) const { return m_data == rOther.m_data; };
    ScMarkArray& operator =( const ScMarkArray& rOther ) { m_data = rOther.m_data; return *this; };

    void    clear();
    bool    GetMark( SCROW nRow ) const;
    void    SetMarkArea( SCROW nStartRow, SCROW nEndRow, bool bMarked );
    bool    IsAllMarked( SCROW nStartRow, SCROW nEndRow ) const;
    bool    HasOneMark( SCROW& rStartRow, SCROW& rEndRow ) const;

    bool    HasMarks() const    { return m_data.size() > 1 || ( m_data.size() == 1 && m_data[0].bMarked ); }

    /// Including current row, may return -1 if bUp and not found
    SCsROW  GetNextMarked( SCsROW nRow, bool bUp ) const;
    SCROW   GetMarkEnd( SCROW nRow, bool bUp ) const;

private:
    bool    Search( SCROW nRow, SCSIZE& nIndex ) const;

};

class ScMarkArrayIter                   // iterate over selected range
{
    const ScMarkArray*  pArray;
    SCSIZE              nPos;
public:
                ScMarkArrayIter( const ScMarkArray* pNewArray );
                ~ScMarkArrayIter();

    bool        Next( SCROW& rTop, SCROW& rBottom );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
