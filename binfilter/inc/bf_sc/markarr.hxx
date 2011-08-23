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

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
namespace binfilter {

#define SC_MARKARRAY_DELTA    4

struct ScMarkEntry
{
    USHORT			nRow;
    BOOL			bMarked;
};

class ScMarkArray
{
    USHORT			nCount;
    USHORT          nLimit;
    ScMarkEntry*	pData;

friend class ScMarkArrayIter;
friend class ScDocument;				// fuer FillInfo

public:
            ScMarkArray();
            ~ScMarkArray();
    void	Reset( BOOL bMarked = FALSE );
    BOOL	GetMark( USHORT nRow ) const;
    void	SetMarkArea( USHORT nStartRow, USHORT nEndRow, BOOL bMarked );
    BOOL	IsAllMarked( USHORT nStartRow, USHORT nEndRow ) const;
    BOOL	HasOneMark( USHORT& rStartRow, USHORT& rEndRow ) const;
    BOOL	HasMarks() const;
    void	CopyMarksTo( ScMarkArray& rDestMarkArray ) const;

    BOOL 	Search( USHORT nRow, short& nIndex ) const;

    short	GetNextMarked( short nRow, BOOL bUp ) const;		// inkl. aktuelle
    USHORT	GetMarkEnd( USHORT nRow, BOOL bUp ) const;
};


class ScMarkArrayIter					// selektierte Bereiche durchgehen
{
    const ScMarkArray*	pArray;
    USHORT				nPos;
public:
                ScMarkArrayIter( const ScMarkArray* pNewArray );
                ~ScMarkArrayIter();

    BOOL		Next( USHORT& rTop, USHORT& rBottom );
};



} //namespace binfilter
#endif

