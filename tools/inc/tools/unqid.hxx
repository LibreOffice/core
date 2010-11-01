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
#ifndef _UNQID_HXX
#define _UNQID_HXX

#include "tools/toolsdllapi.h"
#include <tools/unqidx.hxx>

// ---------------
// - ImpUniqueId -
// ---------------

struct ImpUniqueId
{
    ULONG    nId;
    USHORT   nRefCount;
    void     Release()
             {
                nRefCount--;
                if( 0 == nRefCount )
                    delete this;
             }
};

// ------------
// - UniqueId -
// ------------

class UniqueIdContainer;
class UniqueItemId
{
    friend class UniqueIdContainer;
    ImpUniqueId*    pId;

                    UniqueItemId( ImpUniqueId * pIdP )
                        { pId = pIdP; pId->nRefCount++; }
public:
                    UniqueItemId() { pId = NULL; }
                    UniqueItemId( const UniqueItemId & rId )
                        { pId = rId.pId; if( pId ) pId->nRefCount++; }
                    ~UniqueItemId()
                        { if( pId ) pId->Release(); }
    UniqueItemId&   operator = ( const UniqueItemId & rId )
                        {
                            if( rId.pId ) rId.pId->nRefCount++;
                            if( pId ) pId->Release();
                            pId = rId.pId;
                            return *this;
                        }
    ULONG           GetId() const { return pId ? pId->nId : 0; }
};

// ---------------------
// - UniqueIdContainer -
// ---------------------

class TOOLS_DLLPUBLIC UniqueIdContainer : private UniqueIndex
{
    USHORT              nCollectCount;

public: // Irgend etwas mit protected falsch
    void                Clear( BOOL bAll );
    UniqueItemId        CreateIdProt( ULONG nId );

public:
                        UniqueIdContainer( ULONG _nStartIndex,
                                           ULONG _nInitSize = 16,
                                           ULONG _nReSize = 16 )
                            : UniqueIndex( _nStartIndex, _nInitSize, _nReSize )
                            , nCollectCount( 0 )
                            {}
                        UniqueIdContainer( const UniqueIdContainer& );

                        ~UniqueIdContainer()
                            { Clear( TRUE ); }
    UniqueIdContainer&  operator = ( const UniqueIdContainer & );

    BOOL                IsIndexValid( ULONG nIndex ) const
                            { return UniqueIndex::IsIndexValid( nIndex ); }

    UniqueItemId        CreateId();
    static UniqueItemId CreateFreeId( ULONG nId ); // freies Id
};

#endif // _UNQID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
