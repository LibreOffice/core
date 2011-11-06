/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_LOTRANGE_HXX
#define SC_LOTRANGE_HXX

#include <tools/solar.h>
#include <compiler.hxx>

// --------------------------------------------------------- class LotusRange -

class LotusRangeList;

typedef sal_uInt16  LR_ID;
#define ID_FAIL 0xFFFF

class LotusRange
{
    friend class LotusRangeList;
private:
    sal_uInt32              nHash;
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
    inline sal_Bool         operator ==( const LotusRange& ) const;
    inline sal_Bool         operator !=( const LotusRange& ) const;
    inline sal_Bool         IsSingle( void ) const;
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


inline sal_Bool LotusRange::operator ==( const LotusRange& rRef ) const
{
    return ( nHash == rRef.nHash && nColStart == rRef.nColStart &&
        nRowStart == rRef.nRowStart && nColEnd == rRef.nColEnd &&
        nRowEnd == rRef.nRowEnd );
}


inline sal_Bool LotusRange::operator !=( const LotusRange& rRef ) const
{
    return ( nHash != rRef.nHash || nColStart != rRef.nColStart ||
        nRowStart != rRef.nRowStart || nColEnd != rRef.nColEnd ||
        nRowEnd != rRef.nRowEnd );
}


inline sal_Bool LotusRange::IsSingle( void ) const
{
    return ( nColStart == nColEnd && nRowStart == nRowEnd );
}



// ----------------------------------------------------- class LotusRangeList -

class LotusRangeList : private List
{
private:
    LR_ID               nIdCnt;
    ScComplexRefData        aComplRef;
    static SCCOL        nEingCol;
    static SCROW        nEingRow;
public:
                        LotusRangeList( void );
                        ~LotusRangeList( void );
    inline sal_uInt16       GetIndex( SCCOL nCol, SCROW nRow );
    inline sal_uInt16       GetIndex( SCCOL nColS, SCROW nRowS, SCCOL nColE, SCROW nRowE );
    sal_uInt16              GetIndex( const LotusRange& );
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



