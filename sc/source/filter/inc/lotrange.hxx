/*************************************************************************
 *
 *  $RCSfile: lotrange.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:55:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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



