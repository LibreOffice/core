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

#ifndef _IMPCONT_HXX
#define _IMPCONT_HXX

#include <tools/tools.h>
#include <tools/contnr.hxx>

typedef void* PVOID;

// ----------
// - CBlock -
// ----------

class CBlock
{
private:
    CBlock*         pPrev;              // Vorheriger Block
    CBlock*         pNext;              // Naechster Block
    sal_uInt16          nSize;              // Groesse des Blocks
    sal_uInt16          nCount;             // Anzahl Pointer
    void**          pNodes;             // Pointer auf die Daten

#if defined DBG_UTIL
    static char const * DbgCheckCBlock(void const *);
#endif

public:
                    // Fuer List-Container
                    CBlock( sal_uInt16 nSize, CBlock* pPrev, CBlock* pNext );
                    // Fuer Array-Container
                    CBlock( sal_uInt16 nSize, CBlock* pPrev );
                    // Copy-Ctor
                    CBlock( const CBlock& r, CBlock* pPrev );
                    ~CBlock();

    void            Insert( void* p, sal_uInt16 nIndex, sal_uInt16 nReSize );
    CBlock*         Split( void* p, sal_uInt16 nIndex, sal_uInt16 nReSize );
    void*           Remove( sal_uInt16 nIndex, sal_uInt16 nReSize );
    void*           Replace( void* pNew, sal_uInt16 nIndex );

    void**          GetNodes() const { return pNodes; }
    void**          GetObjectPtr( sal_uInt16 nIndex );
    void*           GetObject( sal_uInt16 nIndex ) const;

    void            SetSize( sal_uInt16 nNewSize );

    sal_uInt16          GetSize() const               { return nCount; }
    sal_uInt16          Count() const                 { return nCount; }
    void            SetPrevBlock( CBlock* p )     { pPrev = p;     }
    void            SetNextBlock( CBlock* p )     { pNext = p;     }
    CBlock*         GetPrevBlock() const          { return pPrev;  }
    CBlock*         GetNextBlock() const          { return pNext;  }
    void            Reset()                       { nCount = 0;    }

private:
                    CBlock( const CBlock& r );

    friend class Container;
};

/*************************************************************************
|*
|*    CBlock::GetObject()
|*
|*    Beschreibung      Gibt einen Pointer aus dem Block zurueck
|*
*************************************************************************/

inline void* CBlock::GetObject( sal_uInt16 nIndex ) const
{
    return pNodes[nIndex];
}

/*************************************************************************
|*
|*    Container::ImpGetObject()
|*
|*    Beschreibung      Wir gehen davon aus, das Pointer in der Regel
|*                      sich im ersten Block befindet und schalten
|*                      deshalb eine Inline-Methode davor
|*
*************************************************************************/

inline void* Container::ImpGetObject( sal_uIntPtr nIndex ) const
{
    if ( pFirstBlock && (nIndex < pFirstBlock->Count()) )
        // Item innerhalb des gefundenen Blocks zurueckgeben
        return pFirstBlock->GetObject( (sal_uInt16)nIndex );
    else
        return GetObject( nIndex );
}

/*************************************************************************
|*
|*    Container::ImpGetOnlyNodes()
|*
|*    Beschreibung      Wenn es nur einen Block gibt, wird davon
|*                      das Daten-Array zurueckgegeben
|*
*************************************************************************/

// #i70651#: Prevent warnings on Mac OS X
#ifdef MACOSX
#pragma GCC system_header
#endif

inline void** Container::ImpGetOnlyNodes() const
{
    if ( (pFirstBlock == pLastBlock) && pFirstBlock )
        return pFirstBlock->GetNodes();
    else
        return NULL;
}

#endif // _IMPCONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
