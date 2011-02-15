/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
friend class ScDocument;                // fuer FillInfo

public:
            ScMarkArray();
            ~ScMarkArray();
    void    Reset( sal_Bool bMarked = sal_False );
    sal_Bool    GetMark( SCROW nRow ) const;
    void    SetMarkArea( SCROW nStartRow, SCROW nEndRow, sal_Bool bMarked );
    sal_Bool    IsAllMarked( SCROW nStartRow, SCROW nEndRow ) const;
    sal_Bool    HasOneMark( SCROW& rStartRow, SCROW& rEndRow ) const;

    sal_Bool    HasMarks() const    { return ( nCount > 1 || ( nCount == 1 && pData[0].bMarked ) ); }

    void    CopyMarksTo( ScMarkArray& rDestMarkArray ) const;

    sal_Bool    Search( SCROW nRow, SCSIZE& nIndex ) const;
//UNUSED2009-05 void    DeleteArea(SCROW nStartRow, SCROW nEndRow);

    /// Including current row, may return -1 if bUp and not found
    SCsROW  GetNextMarked( SCsROW nRow, sal_Bool bUp ) const;
    SCROW   GetMarkEnd( SCROW nRow, sal_Bool bUp ) const;
};


class ScMarkArrayIter                   // selektierte Bereiche durchgehen
{
    const ScMarkArray*  pArray;
    SCSIZE              nPos;
public:
                ScMarkArrayIter( const ScMarkArray* pNewArray );
                ~ScMarkArrayIter();

    sal_Bool        Next( SCROW& rTop, SCROW& rBottom );
};



#endif

