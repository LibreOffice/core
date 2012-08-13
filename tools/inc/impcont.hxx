/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
