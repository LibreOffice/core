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
    CBlock*         pPrev;              // Previous block
    CBlock*         pNext;              // Next block
    sal_uInt16      nSize;              // block size
    sal_uInt16      nCount;             // number of pointers
    void**          pNodes;             // stores node pointers

#if defined DBG_UTIL
    static char const * DbgCheckCBlock(void const *);
#endif

public:
                    // used for list container
                    CBlock( sal_uInt16 nSize, CBlock* pPrev, CBlock* pNext );
                    // used for array container
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
|*    Description      Returns a node pointer given a block index
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
|*    Description       A pointer is often located in the first block,
|*                      thus check this position before calling GetObject
|*
*************************************************************************/

inline void* Container::ImpGetObject( sal_uIntPtr nIndex ) const
{
    if ( pFirstBlock && (nIndex < pFirstBlock->Count()) )
        // Return item within the found block
        return pFirstBlock->GetObject( (sal_uInt16)nIndex );
    else
        return GetObject( nIndex );
}

/*************************************************************************
|*
|*    Container::ImpGetOnlyNodes()
|*
|*    Description       If only one block exists, return its data array
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
