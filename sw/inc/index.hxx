/*************************************************************************
 *
 *  $RCSfile: index.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:02:58 $
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
#ifndef _INDEX_HXX
#define _INDEX_HXX

#include <limits.h>

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>               // for RTTI of SwIndexReg
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>             // for xub_StrLen
#endif

#define INVALID_INDEX STRING_NOTFOUND

// Maximale Anzahl von Indizies im IndexArray (zum Abtesten auf Ueberlaeufe)
class SwIndex;
class SwIndexReg;

#ifdef PRODUCT
#define INLINE inline
#else
#define INLINE
#endif

class SwIndex
{
    friend class SwIndexReg;

#ifndef PRODUCT
    static int nSerial;
    int MySerial;
#endif

    xub_StrLen  nIndex;
    SwIndexReg* pArray;
    SwIndex *pNext, *pPrev;

    SwIndex& ChgValue( const SwIndex& rIdx, xub_StrLen nNewValue );
    void Remove();                  // Ausketten

public:
    SwIndex( SwIndexReg * pReg, xub_StrLen nIdx = 0 );
    SwIndex( const SwIndex & );
    SwIndex( const SwIndex &, short nDiff );
    ~SwIndex() { Remove(); }

    INLINE xub_StrLen operator++();
    INLINE xub_StrLen operator--();
#ifndef CFRONT
    INLINE xub_StrLen operator++(int);
    INLINE xub_StrLen operator--(int);
#endif

    INLINE xub_StrLen operator+=( xub_StrLen );
    INLINE xub_StrLen operator-=( xub_StrLen );
    INLINE xub_StrLen operator+=( const SwIndex& );
    INLINE xub_StrLen operator-=( const SwIndex& );

    INLINE BOOL operator<( const SwIndex& ) const;
    INLINE BOOL operator<=( const SwIndex& ) const;
    INLINE BOOL operator>( const SwIndex& ) const;
    INLINE BOOL operator>=( const SwIndex& ) const;
    BOOL operator==( const SwIndex& rSwIndex ) const
    { return (nIndex == rSwIndex.nIndex) &&  (pArray == rSwIndex.pArray); }

    BOOL operator!=( const SwIndex& rSwIndex ) const
    { return (nIndex != rSwIndex.nIndex) ||  (pArray != rSwIndex.pArray); }

    BOOL operator<( xub_StrLen nWert ) const    { return nIndex <  nWert; }
    BOOL operator<=( xub_StrLen nWert ) const   { return nIndex <= nWert; }
    BOOL operator>( xub_StrLen nWert ) const    { return nIndex >  nWert; }
    BOOL operator>=( xub_StrLen nWert ) const   { return nIndex >= nWert; }
    BOOL operator==( xub_StrLen nWert ) const   { return nIndex == nWert; }
    BOOL operator!=( xub_StrLen nWert ) const   { return nIndex != nWert; }

    INLINE SwIndex& operator=( xub_StrLen );
    SwIndex& operator=( const SwIndex & );

    // gebe den Wert vom Index als xub_StrLen zurueck
    xub_StrLen GetIndex() const { return nIndex; }

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
                         BOOL bNegativ = FALSE, BOOL bDelete = FALSE );

    void ChkArr();

    BOOL HasAnyIndex() const { return 0 != pFirst; }

public:
    SwIndexReg();
#ifndef PRODUCT
    ~SwIndexReg();
#endif

    // rtti, abgeleitete moegens gleichtun oder nicht. Wenn sie es gleichtun
    // kann ueber das SwIndexReg typsicher gecastet werden.
    TYPEINFO();

    void SetDefMiddle( const SwIndex* pIdx ) { pMiddle = pIdx; }
    void MoveTo( SwIndexReg& rArr );
};


#ifdef PRODUCT

inline xub_StrLen SwIndex::operator++()
{
    return ChgValue( *this, nIndex+1 ).nIndex;
}
inline xub_StrLen SwIndex::operator--()
{
    return ChgValue( *this, nIndex-1 ).nIndex;
}
#ifndef CFRONT
inline xub_StrLen SwIndex::operator++(int)
{
    xub_StrLen nOldIndex = nIndex;
    ChgValue( *this, nIndex+1 );
    return nOldIndex;
}
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
inline xub_StrLen SwIndex::operator-=( xub_StrLen nWert )
{
    return ChgValue( *this, nIndex - nWert ).nIndex;
}
inline xub_StrLen SwIndex::operator+=( const  SwIndex& rIndex )
{
    return ChgValue( *this, nIndex + rIndex.nIndex ).nIndex;
}
inline xub_StrLen SwIndex::operator-=( const SwIndex& rIndex )
{
    return ChgValue( *this, nIndex - rIndex.nIndex ).nIndex;
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


#endif
