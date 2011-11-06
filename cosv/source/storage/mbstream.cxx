/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

