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

#include <precomp.h>
#include <cosv/mbstream.hxx>

// NOT FULLY DECLARED SERVICES
#include <string.h>


namespace csv
{



mbstream::mbstream( uintt i_nSize )
    :	dpOwnedMemorySpace( new char[i_nSize+1] ),
        nSize( i_nSize ),
        nCurPosition( 0 )
{
    dpOwnedMemorySpace[i_nSize] = '\0';
}

mbstream::~mbstream()
{
    delete [] dpOwnedMemorySpace;
}

void
mbstream::resize( uintt  i_nSize )
{
    DYN char * pNew = new char[i_nSize];
    memcpy( pNew, dpOwnedMemorySpace, min(i_nSize,nSize) );
    delete [] dpOwnedMemorySpace;
    dpOwnedMemorySpace = pNew;
    nSize = i_nSize;
}

uintt
mbstream::do_read( void *	       out_pDest,
                   uintt           i_nNrofBytes )
{
    uintt  ret = min( i_nNrofBytes, nSize - nCurPosition );
    memcpy( out_pDest, dpOwnedMemorySpace, ret );
    nCurPosition += ret;
    return ret;
}

bool
mbstream::inq_eod() const
{
     return nCurPosition == nSize;
}

uintt
mbstream::do_write( const void *   	i_pSrc,
                    uintt           i_nNrofBytes )
{
    resize( max( 3 * (nSize+1) / 2, nCurPosition + i_nNrofBytes) );
    memcpy( dpOwnedMemorySpace+nCurPosition, i_pSrc, i_nNrofBytes );
    nCurPosition += i_nNrofBytes;
    return i_nNrofBytes;
}

uintt
mbstream::do_seek( intt 	i_nDistance,
                   seek_dir i_eStartPoint )
{
    switch ( i_eStartPoint )
    {
         case beg:       if ( uintt(i_nDistance) < nSize )
                            nCurPosition = uintt(i_nDistance);
                        break;
         case cur:       if ( i_nDistance < 0
                                ?   uintt(-i_nDistance) <= nCurPosition
                                :   uintt(i_nDistance) + nCurPosition < nSize )
                            nCurPosition = uintt( intt(nCurPosition) + i_nDistance );
                        break;
         case end:       if ( i_nDistance < 0
                             AND uintt(-i_nDistance) < nSize - 1 )
                            nCurPosition = uintt( intt(nSize) - 1 + i_nDistance );
                        break;
    }
    return position();
}

uintt
mbstream::inq_position() const
{
    return nCurPosition;
}


}   // namespace csv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
