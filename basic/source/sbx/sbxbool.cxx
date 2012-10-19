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

#include <tools/errcode.hxx>
#include <basic/sbx.hxx>
#include "sbxconv.hxx"
#include "sbxres.hxx"

enum SbxBOOL ImpGetBool( const SbxValues* p )
{
    enum SbxBOOL nRes;
    switch( +p->eType )
    {
        case SbxNULL:
            SbxBase::SetError( SbxERR_CONVERSION );
        case SbxEMPTY:
            nRes = SbxFALSE; break;
        case SbxCHAR:
            nRes = p->nChar ? SbxTRUE : SbxFALSE; break;
        case SbxBYTE:
            nRes = p->nByte ? SbxTRUE : SbxFALSE; break;
        case SbxINTEGER:
        case SbxBOOL:
            nRes = p->nInteger ? SbxTRUE : SbxFALSE; break;
        case SbxERROR:
        case SbxUSHORT:
            nRes = p->nUShort ? SbxTRUE : SbxFALSE; break;
        case SbxLONG:
            nRes = p->nLong ? SbxTRUE : SbxFALSE; break;
        case SbxULONG:
            nRes = p->nULong ? SbxTRUE : SbxFALSE; break;
        case SbxSINGLE:
            nRes = p->nSingle ? SbxTRUE : SbxFALSE; break;
        case SbxDATE:
        case SbxDOUBLE:
            nRes = p->nDouble ? SbxTRUE : SbxFALSE; break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            double dVal = 0.0;
            if( p->pDecimal )
                p->pDecimal->getDouble( dVal );
            nRes = dVal ? SbxTRUE : SbxFALSE;
            }
            break;
        case SbxSALINT64:
        case SbxCURRENCY:
            nRes = p->nInt64 ? SbxTRUE : SbxFALSE; break;
        case SbxSALUINT64:
            nRes = p->uInt64 ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            nRes = SbxFALSE;
            if ( p->pOUString )
            {
                if( p->pOUString->equalsIgnoreAsciiCase( SbxRes( STRING_TRUE ) ) )
                    nRes = SbxTRUE;
                else if( !p->pOUString->equalsIgnoreAsciiCase( SbxRes( STRING_FALSE ) ) )
                {
                    // it can be convertable to a number
                    bool bError = true;
                    double n;
                    SbxDataType t;
                    sal_uInt16 nLen = 0;
                    if( ImpScan( *p->pOUString, n, t, &nLen ) == SbxERR_OK )
                    {
                        if( nLen == p->pOUString->getLength() )
                        {
                            bError = false;
                            if( n != 0.0 )
                                nRes = SbxTRUE;
                        }
                    }
                    if( bError )
                        SbxBase::SetError( SbxERR_CONVERSION );
                }
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetBool() ? SbxTRUE : SbxFALSE;
            else
            {
                SbxBase::SetError( SbxERR_NO_OBJECT ); nRes = SbxFALSE;
            }
            break;
        }

        case SbxBYREF | SbxCHAR:
            nRes = *p->pChar ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxBYTE:
            nRes = *p->pByte ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            nRes = *p->pInteger ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxLONG:
            nRes = *p->pLong ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxULONG:
            nRes = *p->pULong ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            nRes = *p->pUShort ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxSINGLE:
            nRes = ( *p->pSingle != 0 ) ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            nRes = ( *p->pDouble != 0 ) ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxCURRENCY:
        case SbxBYREF | SbxSALINT64:
            nRes = ( *p->pnInt64 ) ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxSALUINT64:
            nRes = ( *p->puInt64 ) ? SbxTRUE : SbxFALSE; break;
        default:
            SbxBase::SetError( SbxERR_CONVERSION ); nRes = SbxFALSE;
    }
    return nRes;
}

void ImpPutBool( SbxValues* p, sal_Int16 n )
{
    if( n )
        n = SbxTRUE;
    switch( +p->eType )
    {
        case SbxCHAR:
            p->nChar = (sal_Unicode) n; break;
        case SbxUINT:
            p->nByte = (sal_uInt8) n; break;
        case SbxINTEGER:
        case SbxBOOL:
            p->nInteger = n; break;
        case SbxLONG:
            p->nLong = n; break;
        case SbxULONG:
            p->nULong = (sal_uInt32) n; break;
        case SbxERROR:
        case SbxUSHORT:
            p->nUShort = (sal_uInt16) n; break;
        case SbxSINGLE:
            p->nSingle = n; break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;
        case SbxCURRENCY:
        case SbxSALINT64:
            p->nInt64 = (sal_Int64) n; break;
        case SbxSALUINT64:
            p->uInt64 = (sal_uInt64) n; break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setInt( (sal_Int16)n );
            break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if ( !p->pOUString )
                p->pOUString = new ::rtl::OUString( SbxRes( n ? STRING_TRUE : STRING_FALSE ) );
            else
                *p->pOUString = SbxRes( n ? STRING_TRUE : STRING_FALSE );
            break;

        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutBool( sal_Bool( n != 0 ) );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            *p->pChar = (sal_Unicode) n; break;
        case SbxBYREF | SbxBYTE:
            *p->pByte = (sal_uInt8) n; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            *p->pInteger = (sal_Int16) n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            *p->pUShort = (sal_uInt16) n; break;
        case SbxBYREF | SbxLONG:
            *p->pLong = n; break;
        case SbxBYREF | SbxULONG:
            *p->pULong = (sal_uInt32) n; break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = n; break;
        case SbxBYREF | SbxCURRENCY:
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = (sal_Int64) n; break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = (sal_uInt64) n; break;
        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
