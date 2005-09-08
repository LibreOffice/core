/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mbstream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:06:33 $
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

#include <precomp.h>
#include <cosv/mbstream.hxx>

// NOT FULLY DECLARED SERVICES
#include <string.h>


namespace csv
{



mbstream::mbstream( uintt i_nSize )
    :   dpOwnedMemorySpace( new char[i_nSize+1] ),
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
mbstream::do_read( void *          out_pDest,
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
mbstream::do_write( const void *    i_pSrc,
                    uintt           i_nNrofBytes )
{
    resize( max( 3 * (nSize+1) / 2, nCurPosition + i_nNrofBytes) );
    memcpy( dpOwnedMemorySpace+nCurPosition, i_pSrc, i_nNrofBytes );
    nCurPosition += i_nNrofBytes;
    return i_nNrofBytes;
}

uintt
mbstream::do_seek( intt     i_nDistance,
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

