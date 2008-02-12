/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: markarr.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 14:23:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_MARKARR_HXX
#define SC_MARKARR_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#define SC_MARKARRAY_DELTA    4

struct ScMarkEntry
{
    SCROW           nRow;
    BOOL            bMarked;
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
    void    Reset( BOOL bMarked = FALSE );
    BOOL    GetMark( SCROW nRow ) const;
    void    SetMark( SCROW nRow, BOOL bMarked );
    void    SetMarkArea( SCROW nStartRow, SCROW nEndRow, BOOL bMarked );
    BOOL    IsAllMarked( SCROW nStartRow, SCROW nEndRow ) const;
    BOOL    HasOneMark( SCROW& rStartRow, SCROW& rEndRow ) const;

    BOOL    HasMarks() const    { return ( nCount > 1 || ( nCount == 1 && pData[0].bMarked ) ); }

    void    CopyMarksTo( ScMarkArray& rDestMarkArray ) const;

    BOOL    Search( SCROW nRow, SCSIZE& nIndex ) const;
    void    DeleteArea(SCROW nStartRow, SCROW nEndRow);
    void    SwapCol(ScMarkArray& rMarkArray);
    void    MoveTo(SCROW nStartRow, SCROW nEndRow, ScMarkArray& rMarkArray);

    /// Including current row, may return -1 if bUp and not found
    SCsROW  GetNextMarked( SCsROW nRow, BOOL bUp ) const;
    SCROW   GetMarkEnd( SCROW nRow, BOOL bUp ) const;
};


class ScMarkArrayIter                   // selektierte Bereiche durchgehen
{
    const ScMarkArray*  pArray;
    SCSIZE              nPos;
public:
                ScMarkArrayIter( const ScMarkArray* pNewArray );
                ~ScMarkArrayIter();

    BOOL        Next( SCROW& rTop, SCROW& rBottom );
};



#endif

