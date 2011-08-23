/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _INDEX_HXX
#define _INDEX_HXX

#include <limits.h>

#include <tools/solar.h>

#include <tools/rtti.hxx>				// for RTTI of SwIndexReg
/*N*/ #include <tools/debug.hxx> //for stripping
namespace binfilter {
#define INVALID_INDEX STRING_NOTFOUND

// Maximale Anzahl von Indizies im IndexArray (zum Abtesten auf Ueberlaeufe)
class SwIndex;
class SwIndexReg;

#ifndef DBG_UTIL
#define INLINE inline
#else
#define INLINE
#endif

class SwIndex
{
    friend class SwIndexReg;

#ifdef DBG_UTIL
    static int nSerial;
    int MySerial;
#endif

    xub_StrLen	nIndex;
    SwIndexReg*	pArray;
    SwIndex *pNext, *pPrev;

    SwIndex& ChgValue( const SwIndex& rIdx, xub_StrLen nNewValue );
    void Remove();					// Ausketten

public:
    SwIndex( SwIndexReg * pReg, xub_StrLen nIdx = 0 );
    SwIndex( const SwIndex & );
    ~SwIndex() { Remove(); }

    INLINE xub_StrLen operator++();
#ifndef CFRONT
    INLINE xub_StrLen operator++(int);
    INLINE xub_StrLen operator--(int);
#endif

    INLINE xub_StrLen operator+=( xub_StrLen );
    INLINE xub_StrLen operator+=( const SwIndex& );

    INLINE BOOL operator<( const SwIndex& ) const;
    INLINE BOOL operator<=( const SwIndex& ) const;
    INLINE BOOL operator>( const SwIndex& ) const;
    INLINE BOOL operator>=( const SwIndex& ) const;
    BOOL operator==( const SwIndex& rSwIndex ) const
    { return (nIndex == rSwIndex.nIndex) &&  (pArray == rSwIndex.pArray); }

    BOOL operator!=( const SwIndex& rSwIndex ) const
    { return (nIndex != rSwIndex.nIndex) ||  (pArray != rSwIndex.pArray); }

    BOOL operator<( xub_StrLen nWert ) const 	{ return nIndex <  nWert; }
    BOOL operator<=( xub_StrLen nWert ) const   { return nIndex <= nWert; }
    BOOL operator>( xub_StrLen nWert ) const    { return nIndex >  nWert; }
    BOOL operator>=( xub_StrLen nWert ) const   { return nIndex >= nWert; }
    BOOL operator==( xub_StrLen nWert ) const   { return nIndex == nWert; }
    BOOL operator!=( xub_StrLen nWert ) const   { return nIndex != nWert; }

    INLINE SwIndex& operator=( xub_StrLen );
    SwIndex& operator=( const SwIndex & );

    // gebe den Wert vom Index als xub_StrLen zurueck
    xub_StrLen GetIndex() const	{ return nIndex; }

    // ermoeglicht Zuweisungen ohne Erzeugen eines temporaeren
    // Objektes
    SwIndex &Assign(SwIndexReg *,xub_StrLen);

        // Herausgabe des Pointers auf das IndexArray,
        // (fuers RTTI am SwIndexReg)
    const SwIndexReg* GetIdxReg() const { return pArray; }
};

#undef INLINE

class SwIndexReg
{
    friend class SwIndex;
    const SwIndex *pFirst, *pLast, *pMiddle;

    // ein globales Array, in das Indizies verschoben werden, die mal
    // temporaer "ausgelagert" werden muessen; oder die zum Zeitpunkt des
    // anlegens kein gueltiges Array kennen (SwPaM/SwPosition!)
    friend void _InitCore();
    friend void _FinitCore();
    static SwIndexReg* pEmptyIndexArray;

protected:
    virtual void Update( const SwIndex & aPos, xub_StrLen nLen,
                        BOOL bNegativ = FALSE );


    BOOL HasAnyIndex() const { return 0 != pFirst; }

    // fuer das Move am RegArray:

public:
    SwIndexReg();
#ifdef DBG_UTIL
    ~SwIndexReg();
#endif

    // rtti, abgeleitete moegens gleichtun oder nicht. Wenn sie es gleichtun
    // kann ueber das SwIndexReg typsicher gecastet werden.
    TYPEINFO();

    void SetDefMiddle( const SwIndex* pIdx ) { pMiddle = pIdx; }
    void MoveTo( SwIndexReg& rArr );
};


#ifndef DBG_UTIL

inline xub_StrLen SwIndex::operator++() {DBG_BF_ASSERT(0, "STRIP"); return 0;} //STRIP001 INLINE xub_StrLen operator++(int);
#ifndef CFRONT
inline xub_StrLen SwIndex::operator++(int) {DBG_BF_ASSERT(0, "STRIP"); return 0;} //STRIP001 INLINE xub_StrLen operator++(int);
 inline xub_StrLen SwIndex::operator--(int)
 {
     xub_StrLen nOldIndex = nIndex;
     ChgValue( *this, nIndex-1 );
     return nOldIndex;
 }
#endif

inline xub_StrLen SwIndex::operator+=( xub_StrLen nWert )
{
    return ChgValue( *this, nIndex + nWert ).nIndex;
}
inline xub_StrLen SwIndex::operator+=( const  SwIndex& rIndex )
{
    return ChgValue( *this, nIndex + rIndex.nIndex ).nIndex;
}

  inline BOOL SwIndex::operator<( const SwIndex& rIndex ) const
  {
      return nIndex < rIndex.nIndex;
  }
  inline BOOL SwIndex::operator<=( const SwIndex& rIndex ) const
  {
      return nIndex <= rIndex.nIndex;
  }
  inline BOOL SwIndex::operator>( const SwIndex& rIndex ) const
  {
      return nIndex > rIndex.nIndex;
  }
  inline BOOL SwIndex::operator>=( const SwIndex& rIndex ) const
  {
      return nIndex >= rIndex.nIndex;
  }
 inline SwIndex& SwIndex::operator=( xub_StrLen nWert )
 {
     if( nIndex != nWert )
         ChgValue( *this, nWert );
     return *this;
 }

// PRODUCT
#endif


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
