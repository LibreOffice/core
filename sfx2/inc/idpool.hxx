/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: idpool.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:57:12 $
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
#ifndef _SFXIDPOOL_HXX
#define _SFXIDPOOL_HXX

#include <limits.h>     // USHRT_MAX
#include "bitset.hxx"

// class IdPool ----------------------------------------------------------

class IdPool: private BitSet
{
private:
    USHORT nNextFree;
    USHORT nRange;
    USHORT nOffset;
public:
    BOOL Lock( const BitSet& rLockSet );
    BOOL IsLocked( USHORT nId ) const;
    IdPool( USHORT nMin = 1, USHORT nMax = USHRT_MAX );
    USHORT Get();
    BOOL Put( USHORT nId );
    BOOL Lock( const Range& rRange );
    BOOL Lock( USHORT nId );

};

//------------------------------------------------------------------------

// returns TRUE if the id is locked

inline BOOL IdPool::IsLocked( USHORT nId ) const
{
    return ( this->Contains(nId-nOffset) );
}


#endif

