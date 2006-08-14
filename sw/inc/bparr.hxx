/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bparr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:17:05 $
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

#ifndef _BPARR_HXX
#define _BPARR_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

struct BlockInfo;
class BigPtrArray;

class BigPtrEntry
{
    friend class BigPtrArray;
    BlockInfo* pBlock;
    USHORT nOffset;
public:
    virtual ~BigPtrEntry() {}
protected:
    BigPtrEntry() : pBlock(0), nOffset(0) {}

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

#define COMPRESSLVL 80

struct BlockInfo {                  // Block-Info:
    BigPtrArray* pBigArr;           // in diesem Array steht der Block
    ElementPtr* pData;              // Datenblock
    ULONG nStart, nEnd;             // Start- und EndIndex
    USHORT nElem;                   // Anzahl Elemente
};

class BigPtrArray
{
    BlockInfo** ppInf;              // Block-Infos
    ULONG       nSize;              // Anzahl Elemente
    USHORT      nMaxBlock;          // akt. max Anzahl Bloecke
    USHORT      nBlock;             // Anzahl Bloecke
    USHORT      nCur;               // letzter Block

    USHORT      Index2Block( ULONG ) const; // Blocksuche
    BlockInfo*  InsBlock( USHORT );         // Block einfuegen
    void        BlockDel( USHORT );         // es wurden Bloecke geloescht
    void        UpdIndex( USHORT );         // Indexe neu berechnen

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
    void Remove( ULONG pos, ULONG n = 1 );
    void Move( ULONG from, ULONG to );
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


#endif
