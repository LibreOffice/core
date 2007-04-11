/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unqid.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:20:22 $
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
#ifndef _UNQID_HXX
#define _UNQID_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _UNQIDX_HXX
#include <tools/unqidx.hxx>
#endif

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
