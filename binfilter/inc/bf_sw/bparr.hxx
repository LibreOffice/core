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

#ifndef _BPARR_HXX
#define _BPARR_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
namespace binfilter {

struct BlockInfo;
class BigPtrArray;

class BigPtrEntry
{
    friend class BigPtrArray;
    BlockInfo* pBlock;
    USHORT nOffset;
protected:
    BigPtrEntry() : pBlock(0), nOffset(0) {}
    virtual ~BigPtrEntry() {}

    inline ULONG GetPos() const;
    inline BigPtrArray& GetArray() const;
};
typedef BigPtrEntry* ElementPtr;


typedef BOOL (*FnForEach)( const ElementPtr&, void* pArgs );

// 1000 Eintr„ge pro Block = etwas weniger als 4K
#define MAXENTRY 1000


// Anzahl Eintraege, die bei der Kompression frei bleiben duerfen
// dieser Wert ist fuer den Worst Case, da wir MAXBLOCK mit ca 25%
// Overhead definiert haben, reichen 80% = 800 Eintraege vollkommen aus
// Will mann voellige Kompression haben, muss eben 100 angegeben werden.

#define	COMPRESSLVL 80

struct BlockInfo {					// Block-Info:
    BigPtrArray* pBigArr;			// in diesem Array steht der Block
    ElementPtr*	pData;				// Datenblock
    ULONG nStart, nEnd;				// Start- und EndIndex
    USHORT nElem;					// Anzahl Elemente
};

class BigPtrArray
{
    BlockInfo** ppInf;				// Block-Infos
    ULONG		nSize;				// Anzahl Elemente
    USHORT		nMaxBlock;			// akt. max Anzahl Bloecke
    USHORT		nBlock;				// Anzahl Bloecke
    USHORT		nCur;				// letzter Block

    USHORT		Index2Block( ULONG ) const;	// Blocksuche
    BlockInfo*	InsBlock( USHORT );			// Block einfuegen
    void		BlockDel( USHORT );			// es wurden Bloecke geloescht
    void		UpdIndex( USHORT );			// Indexe neu berechnen

protected:
    // fuelle alle Bloecke auf.
    // Der short gibt in Prozent an, wie voll die Bloecke werden sollen.
    // Der ReturnWert besagt, das irgendetwas "getan" wurde
    USHORT Compress( short = COMPRESSLVL );

public:
    BigPtrArray();
    ~BigPtrArray();

    ULONG Count() const { return nSize; }

    void Insert( const ElementPtr& r, ULONG pos );
//	void Insert( const ElementPtr* p, ULONG n, ULONG pos );
    void Remove( ULONG pos, ULONG n = 1 );
    void Replace( ULONG pos, const ElementPtr& r);

    ElementPtr operator[]( ULONG ) const;
    void ForEach( FnForEach fn, void* pArgs = NULL )
    {
        ForEach( 0, nSize, fn, pArgs );
    }
    void ForEach( ULONG nStart, ULONG nEnd, FnForEach fn, void* pArgs = NULL );
};



inline ULONG BigPtrEntry::GetPos() const
{
    DBG_ASSERT( this == pBlock->pData[ nOffset ], "Element nicht im Block" );
    return pBlock->nStart + nOffset;
}

inline BigPtrArray& BigPtrEntry::GetArray() const
{
    return *pBlock->pBigArr;
}


} //namespace binfilter
#endif
