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
    sal_uInt16 nNextFree;
    sal_uInt16 nRange;
    sal_uInt16 nOffset;
public:
    sal_Bool Lock( const BitSet& rLockSet );
    sal_Bool IsLocked( sal_uInt16 nId ) const;
    IdPool( sal_uInt16 nMin = 1, sal_uInt16 nMax = USHRT_MAX );
    sal_uInt16 Get();
    sal_Bool Put( sal_uInt16 nId );
    sal_Bool Lock( const Range& rRange );
    sal_Bool Lock( sal_uInt16 nId );

};

//------------------------------------------------------------------------

// returns sal_True if the id is locked

inline sal_Bool IdPool::IsLocked( sal_uInt16 nId ) const
{
    return ( this->Contains(nId-nOffset) );
}


#endif

