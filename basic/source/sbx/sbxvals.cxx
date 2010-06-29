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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#define _TLBIGINT_INT64
#include <tools/bigint.hxx>
#include <basic/sbx.hxx>

///////////////////////////// BigInt/Currency //////////////////////////////

SbxValues::SbxValues( const BigInt &rBig ) : eType(SbxCURRENCY)
{
    rBig.INT64( &nLong64 );
}

//TODO:  BigInt is TOOLS_DLLPUBLIC, and its four member functions only declared
// and defined within basic (#define _TLBIGINT_INT64) are a bad hack that causes
// "warning C4273: 'function' : inconsistent dll linkage" on MSC; this whole
// mess should be cleaned up properly (e.g., by completely removing Sbx[U]INT64
// and using sal_[u]Int64 instead):
#if defined _MSC_VER
#pragma warning(disable: 4273)
#endif

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
        p->Set( (INT32)nVal );

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
