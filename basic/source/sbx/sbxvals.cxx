/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#define _TLBIGINT_INT64
#include <tools/bigint.hxx>
#include <basic/sbx.hxx>

///////////////////////////// BigInt/Currency

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

sal_Bool BigInt::INT64( SbxINT64 *p ) const
{
    if( bIsBig ) {
        if( nLen > 4 || (nNum[3] & 0x8000) )
            return sal_False;

        p->nLow  = ((sal_uInt32)nNum[1] << 16) | (sal_uInt32)nNum[0];
        p->nHigh = ((sal_uInt32)nNum[3] << 16) | (sal_uInt32)nNum[2];
        if( bIsNeg )
            p->CHS();
    }
    else
        p->Set( (sal_Int32)nVal );

    return sal_True;
}

BigInt::BigInt( const SbxINT64 &r )
{
    BigInt a10000 = 0x10000;

    *this = r.nHigh;
    if( r.nHigh )
        *this *= a10000;
    *this += (sal_uInt16)(r.nLow >> 16);
    *this *= a10000;
    *this += (sal_uInt16)r.nLow;
}

sal_Bool BigInt::UINT64( SbxUINT64 *p ) const
{
    if( bIsBig ) {
        if( bIsNeg || nLen > 4 )
            return sal_False;

        p->nLow  = ((sal_uInt32)nNum[1] << 16) | (sal_uInt32)nNum[0];
        p->nHigh = ((sal_uInt32)nNum[3] << 16) | (sal_uInt32)nNum[2];
    }
    else {
        if( nVal < 0 )
            return sal_False;

        p->Set( (sal_uInt32)nVal );
    }

    return sal_True;
}

BigInt::BigInt( const SbxUINT64 &r )
{
    BigInt a10000 = 0x10000;

    *this = BigInt(r.nHigh);
    if( r.nHigh )
        *this *= a10000;
    *this += (sal_uInt16)(r.nLow >> 16);
    *this *= a10000;
    *this += (sal_uInt16)r.nLow;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
