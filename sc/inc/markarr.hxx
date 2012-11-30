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

#ifndef SC_MARKARR_HXX
#define SC_MARKARR_HXX

#include "address.hxx"
#include <tools/solar.h>

#define SC_MARKARRAY_DELTA    4

struct ScMarkEntry
{
    SCROW           nRow;
    sal_Bool            bMarked;
};

class ScMarkArray
{
    SCSIZE          nCount;
    SCSIZE          nLimit;
    ScMarkEntry*    pData;

friend class ScMarkArrayIter;
friend class ScDocument;                // for FillInfo

public:
            ScMarkArray();
            ~ScMarkArray();
    void    Reset( sal_Bool bMarked = false );
    sal_Bool    GetMark( SCROW nRow ) const;
    void    SetMarkArea( SCROW nStartRow, SCROW nEndRow, sal_Bool bMarked );
    sal_Bool    IsAllMarked( SCROW nStartRow, SCROW nEndRow ) const;
    sal_Bool    HasOneMark( SCROW& rStartRow, SCROW& rEndRow ) const;

    sal_Bool    HasMarks() const    { return ( nCount > 1 || ( nCount == 1 && pData[0].bMarked ) ); }

    void    CopyMarksTo( ScMarkArray& rDestMarkArray ) const;

    sal_Bool    Search( SCROW nRow, SCSIZE& nIndex ) const;

    /// Including current row, may return -1 if bUp and not found
    SCsROW  GetNextMarked( SCsROW nRow, sal_Bool bUp ) const;
    SCROW   GetMarkEnd( SCROW nRow, sal_Bool bUp ) const;
};


class ScMarkArrayIter                   // iterate over selected range
{
    const ScMarkArray*  pArray;
    SCSIZE              nPos;
public:
                ScMarkArrayIter( const ScMarkArray* pNewArray );
                ~ScMarkArrayIter();

    sal_Bool        Next( SCROW& rTop, SCROW& rBottom );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
