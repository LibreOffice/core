/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contnr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:09:14 $
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
#ifndef _CONTNR_HXX
#define _CONTNR_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class CBlock;

// -------------
// - Container -
// -------------

// Maximale Blockgroesse
#define CONTAINER_MAXBLOCKSIZE      ((USHORT)0x3FF0)

#define CONTAINER_APPEND            ((ULONG)0xFFFFFFFF)
#define CONTAINER_ENTRY_NOTFOUND    ((ULONG)0xFFFFFFFF)

class TOOLS_DLLPUBLIC Container
{
private:
    CBlock*     pFirstBlock;
    CBlock*     pCurBlock;
    CBlock*     pLastBlock;
    USHORT      nCurIndex;
    USHORT      nBlockSize;
    USHORT      nInitSize;
    USHORT      nReSize;
    ULONG       nCount;

    TOOLS_DLLPRIVATE void ImpCopyContainer(Container const *);
#if defined DBG_UTIL
    TOOLS_DLLPRIVATE static char const * DbgCheckContainer(void const *);
#endif

protected:
#ifdef _IMPCONT_HXX
    void        ImpInsert( void* p, CBlock* pBlock, USHORT nIndex );
    void*       ImpRemove( CBlock* pBlock, USHORT nIndex );
    void*       ImpGetObject( ULONG nIndex ) const;
    void**      ImpGetOnlyNodes() const;
#endif
    void**      GetObjectPtr( ULONG nIndex );

public:
                Container( USHORT nBlockSize,
                           USHORT nInitSize,
                           USHORT nReSize );
                Container( ULONG nSize );
                Container( const Container& rContainer );
                ~Container();

    void        Insert( void* p );
    void        Insert( void* p, ULONG nIndex );
    void        Insert( void* pNew, void* pOld );

    void*       Remove();
    void*       Remove( ULONG nIndex );
    void*       Remove( void* p )
                    { return Remove( GetPos( p ) ); }

    void*       Replace( void* p );
    void*       Replace( void* p, ULONG nIndex );
    void*       Replace( void* pNew, void* pOld )
                    { return Replace( pNew, GetPos( pOld ) ); }

    void        SetSize( ULONG nNewSize );
    ULONG       GetSize() const { return nCount; }

    ULONG       Count() const { return nCount; }
    void        Clear();

    void*       GetCurObject() const;
    ULONG       GetCurPos() const;
    void*       GetObject( ULONG nIndex ) const;
    ULONG       GetPos( const void* p ) const;
    ULONG       GetPos( const void* p, ULONG nStartIndex,
                        BOOL bForward = TRUE ) const;

    void*       Seek( ULONG nIndex );
    void*       Seek( void* p ) { return Seek( GetPos( p ) ); }

    void*       First();
    void*       Last();
    void*       Next();
    void*       Prev();

    Container&  operator =( const Container& rContainer );

    BOOL        operator ==( const Container& rContainer ) const;
    BOOL        operator !=( const Container& rContainer ) const
                    { return !(Container::operator==( rContainer )); }
};

#endif // _CONTNR_HXX
