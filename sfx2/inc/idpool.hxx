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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
