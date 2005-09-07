/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxvals.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:54:06 $
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

#define _TLBIGINT_INT64

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif
#include "sbx.hxx"

///////////////////////////// BigInt/Currency //////////////////////////////

SbxValues::SbxValues( const BigInt &rBig ) : eType(SbxCURRENCY)
{
    rBig.INT64( &nLong64 );
}

BOOL BigInt::INT64( SbxINT64 *p ) const
{
    if( bIsBig ) {
        if( nLen > 4 || (nNum[3] & 0x8000) )
            return FALSE;

        p->nLow  = ((UINT32)nNum[1] << 16) | (UINT32)nNum[0];
        p->nHigh = ((UINT32)nNum[3] << 16) | (UINT32)nNum[2];
        if( bIsNeg )
            p->CHS();
    }
    else
        p->Set( nVal );

    return TRUE;
}

BigInt::BigInt( const SbxINT64 &r )
{
    BigInt a10000 = 0x10000;

    *this = r.nHigh;
    if( r.nHigh )
        *this *= a10000;
    *this += (USHORT)(r.nLow >> 16);
    *this *= a10000;
    *this += (USHORT)r.nLow;
}

BOOL BigInt::UINT64( SbxUINT64 *p ) const
{
    if( bIsBig ) {
        if( bIsNeg || nLen > 4 )
            return FALSE;

        p->nLow  = ((UINT32)nNum[1] << 16) | (UINT32)nNum[0];
        p->nHigh = ((UINT32)nNum[3] << 16) | (UINT32)nNum[2];
    }
    else {
        if( nVal < 0 )
            return FALSE;

        p->Set( (UINT32)nVal );
    }

    return TRUE;
}

BigInt::BigInt( const SbxUINT64 &r )
{
    BigInt a10000 = 0x10000;

    *this = BigInt(r.nHigh);
    if( r.nHigh )
        *this *= a10000;
    *this += (USHORT)(r.nLow >> 16);
    *this *= a10000;
    *this += (USHORT)r.nLow;
}
