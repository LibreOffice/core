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



#ifndef _BPARR_HXX
#define _BPARR_HXX

#include <tools/solar.h>
#include <tools/debug.hxx>
#include <swdllapi.h>

struct BlockInfo;
class BigPtrArray;

class BigPtrEntry
{
    friend class BigPtrArray;
    BlockInfo* pBlock;
    sal_uInt16 nOffset;
public:
    virtual ~BigPtrEntry() {}
protected:
    BigPtrEntry() : pBlock(0), nOffset(0) {}

    inline sal_uLong GetPos() const;
    inline BigPtrArray& GetArray() const;
};
typedef BigPtrEntry* ElementPtr;


typedef sal_Bool (*FnForEach)( const ElementPtr&, void* pArgs );

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
    sal_uLong nStart, nEnd;             // Start- und EndIndex
    sal_uInt16 nElem;                   // Anzahl Elemente
};

class SW_DLLPUBLIC BigPtrArray
{
    BlockInfo** ppInf;              // Block-Infos
    sal_uLong       nSize;              // Anzahl Elemente
    sal_uInt16      nMaxBlock;          // akt. max Anzahl Bloecke
    sal_uInt16      nBlock;             // Anzahl Bloecke
    sal_uInt16      nCur;               // letzter Block

    sal_uInt16      Index2Block( sal_uLong ) const; // Blocksuche
    BlockInfo*  InsBlock( sal_uInt16 );         // Block einfuegen
    void        BlockDel( sal_uInt16 );         // es wurden Bloecke geloescht
    void        UpdIndex( sal_uInt16 );         // Indexe neu berechnen

protected:
    // fuelle alle Bloecke auf.
    // Der short gibt in Prozent an, wie voll die Bloecke werden sollen.
    // Der ReturnWert besagt, das irgendetwas "getan" wurde
    sal_uInt16 Compress( short = COMPRESSLVL );

public:
    BigPtrArray();
    ~BigPtrArray();

    sal_uLong Count() const { return nSize; }

    void Insert( const ElementPtr& r, sal_uLong pos );
    void Remove( sal_uLong pos, sal_uLong n = 1 );
    void Move( sal_uLong from, sal_uLong to );
    void Replace( sal_uLong pos, const ElementPtr& r);

    ElementPtr operator[]( sal_uLong ) const;
    void ForEach( FnForEach fn, void* pArgs = NULL )
    {
        ForEach( 0, nSize, fn, pArgs );
    }
    void ForEach( sal_uLong nStart, sal_uLong nEnd, FnForEach fn, void* pArgs = NULL );
};



inline sal_uLong BigPtrEntry::GetPos() const
{
    DBG_ASSERT( this == pBlock->pData[ nOffset ], "Element nicht im Block" );
    return pBlock->nStart + nOffset;
}

inline BigPtrArray& BigPtrEntry::GetArray() const
{
    return *pBlock->pBigArr;
}


#endif
