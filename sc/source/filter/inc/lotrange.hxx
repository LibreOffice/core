/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lotrange.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:22:20 $
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


#ifndef _LOTRANGE_HXX
#define _LOTRANGE_HXX

#include <tools/solar.h>
#include <compiler.hxx>

// --------------------------------------------------------- class LotusRange -

class LotusRangeList;

typedef UINT16  LR_ID;
#define ID_FAIL 0xFFFF

class LotusRange
{
    friend class LotusRangeList;
private:
    UINT32              nHash;
    SCCOL               nColStart;
    SCROW               nRowStart;
    SCCOL               nColEnd;
    SCROW               nRowEnd;
    LR_ID               nId;
    void                MakeHash( void );
    inline void         Copy( const LotusRange& );
    inline void         SetId( LR_ID nId );
public:
                        LotusRange( SCCOL nCol, SCROW nRow );
                        LotusRange( SCCOL nColS, SCROW nRowS, SCCOL nColE, SCROW nRowE );
                        LotusRange( const LotusRange& );
    inline LotusRange   &operator =( const LotusRange& );
    inline BOOL         operator ==( const LotusRange& ) const;
    inline BOOL         operator !=( const LotusRange& ) const;
    inline BOOL         IsSingle( void ) const;
};


inline void LotusRange::Copy( const LotusRange& rCpy )
{
    nColStart = rCpy.nColStart;
    nRowStart = rCpy.nRowStart;
    nColEnd = rCpy.nColEnd;
    nRowEnd = rCpy.nRowEnd;
}


inline void LotusRange::SetId( LR_ID nNewId )
{
    nId = nNewId;
}


inline LotusRange &LotusRange::operator =( const LotusRange& rCpy )
{
    Copy( rCpy );
    return *this;
}


inline BOOL LotusRange::operator ==( const LotusRange& rRef ) const
{
    return ( nHash == rRef.nHash && nColStart == rRef.nColStart &&
        nRowStart == rRef.nRowStart && nColEnd == rRef.nColEnd &&
        nRowEnd == rRef.nRowEnd );
}


inline BOOL LotusRange::operator !=( const LotusRange& rRef ) const
{
    return ( nHash != rRef.nHash || nColStart != rRef.nColStart ||
        nRowStart != rRef.nRowStart || nColEnd != rRef.nColEnd ||
        nRowEnd != rRef.nRowEnd );
}


inline BOOL LotusRange::IsSingle( void ) const
{
    return ( nColStart == nColEnd && nRowStart == nRowEnd );
}



// ----------------------------------------------------- class LotusRangeList -

class LotusRangeList : private List
{
private:
    LR_ID               nIdCnt;
    ComplRefData        aComplRef;
    static SCCOL        nEingCol;
    static SCROW        nEingRow;
public:
                        LotusRangeList( void );
                        ~LotusRangeList( void );
    inline UINT16       GetIndex( SCCOL nCol, SCROW nRow );
    inline UINT16       GetIndex( SCCOL nColS, SCROW nRowS, SCCOL nColE, SCROW nRowE );
    UINT16              GetIndex( const LotusRange& );
    inline void         Append( SCCOL nCol, SCROW nRow, const String& );
    inline void         Append( SCCOL nColS, SCROW nRowS, SCCOL nColE, SCROW nRowE, const String& );
    void                Append( LotusRange* pLR, const String& rName );
    inline static void  SetEing( const SCCOL nCol, const SCROW nRow );
};


inline LR_ID LotusRangeList::GetIndex( SCCOL nCol, SCROW nRow )
{
    LotusRange aRef( nCol, nRow );
    return GetIndex( aRef );
}


inline LR_ID LotusRangeList::GetIndex( SCCOL nColS, SCROW nRowS, SCCOL nColE, SCROW nRowE )
{
    LotusRange aRef( nColS, nRowS, nColE, nRowE );
    return GetIndex( aRef );
}


inline void LotusRangeList::Append( SCCOL nCol, SCROW nRow, const String& rName )
{
    Append( new LotusRange( nCol, nRow ), rName );
}


inline void LotusRangeList::Append( SCCOL nColS, SCROW nRowS, SCCOL nColE, SCROW nRowE, const String& r )
{
    Append( new LotusRange( nColS, nRowS, nColE, nRowE ), r );
}


inline void LotusRangeList::SetEing( const SCCOL nCol, const SCROW nRow )
{
    nEingCol = nCol;
    nEingRow = nRow;
}

#endif



