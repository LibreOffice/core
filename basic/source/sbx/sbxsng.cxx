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

float ImpGetSingle( const SbxValues* p )
{
    SbxValues aTmp;
    float nRes;
start:
    switch( +p->eType )
    {
        case SbxNULL:
            SbxBase::SetError( SbxERR_CONVERSION );
        case SbxEMPTY:
            nRes = 0; break;
        case SbxCHAR:
            nRes = p->nChar; break;
        case SbxBYTE:
            nRes = p->nByte; break;
        case SbxINTEGER:
        case SbxBOOL:
            nRes = p->nInteger; break;
        case SbxERROR:
        case SbxUSHORT:
            nRes = p->nUShort; break;
        case SbxLONG:
            nRes = (float) p->nLong; break;
        case SbxULONG:
            nRes = (float) p->nULong; break;
        case SbxSINGLE:
            nRes = p->nSingle; break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            if( p->pDecimal )
                p->pDecimal->getSingle( nRes );
            else
                nRes = 0.0;
            break;
        case SbxDATE:
        case SbxDOUBLE:
        case SbxCURRENCY:
        case SbxSALINT64:
        case SbxSALUINT64:
            {
            double dVal;
            if( p->eType == SbxCURRENCY )
                dVal = ImpCurrencyToDouble( p->nInt64 );
            else if( p->eType == SbxSALINT64 )
                dVal = (float) p->nInt64;
            else if( p->eType == SbxSALUINT64 )
                dVal = (float) p->uInt64;
            else
                dVal = p->nDouble;

            if( dVal > SbxMAXSNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW );
                nRes = static_cast< float >(SbxMAXSNG);
            }
            else if( dVal < SbxMINSNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW );
                nRes = static_cast< float >(SbxMINSNG);
            }
            // tests for underflow - storing value too small for precision of single
            else if( dVal > 0 && dVal < SbxMAXSNG2 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW );
                nRes = static_cast< float >(SbxMAXSNG2);
            }
            else if( dVal < 0 && dVal > SbxMINSNG2 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW );
                nRes = static_cast< float >(SbxMINSNG2);
            }
            else
                nRes = (float) dVal;
            break;
            }
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pOUString )
                nRes = 0;
            else
            {
                double d;
                SbxDataType t;
                if( ImpScan( *p->pOUString, d, t, NULL ) != SbxERR_OK )
                    nRes = 0;
                else if( d > SbxMAXSNG )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW );
                    nRes = static_cast< float >(SbxMAXSNG);
                }
                else if( d < SbxMINSNG )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW );
                    nRes = static_cast< float >(SbxMINSNG);
                }
                else
                    nRes = (float) d;
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetSingle();
            else
            {
                SbxBase::SetError( SbxERR_NO_OBJECT ); nRes = 0;
            }
            break;
        }

        case SbxBYREF | SbxCHAR:
            nRes = *p->pChar; break;
        case SbxBYREF | SbxBYTE:
            nRes = *p->pByte; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            nRes = *p->pInteger; break;
        case SbxBYREF | SbxLONG:
            nRes = (float) *p->pLong; break;
        case SbxBYREF | SbxULONG:
            nRes = (float) *p->pULong; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            nRes = *p->pUShort; break;
        case SbxBYREF | SbxSINGLE:
            nRes = *p->pSingle; break;
        // from here had to be tested
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            aTmp.nDouble = *p->pDouble; goto ref;
        case SbxBYREF | SbxSALINT64:
        case SbxBYREF | SbxCURRENCY:
            aTmp.nInt64 = *p->pnInt64; goto ref;
        case SbxBYREF | SbxSALUINT64:
            aTmp.uInt64 = *p->puInt64; goto ref;
        ref:
            aTmp.eType = SbxDataType( p->eType & 0x0FFF );
            p = &aTmp; goto start;

        default:
            SbxBase::SetError( SbxERR_CONVERSION ); nRes = 0;
    }
    return nRes;
}

void ImpPutSingle( SbxValues* p, float n )
{
    SbxValues aTmp;
start:
    switch( +p->eType )
    {
        case SbxCHAR:
            aTmp.pChar = &p->nChar; goto direct;
        case SbxBYTE:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxINTEGER:
        case SbxBOOL:
            aTmp.pInteger = &p->nInteger; goto direct;
        case SbxLONG:
            aTmp.pLong = &p->nLong; goto direct;
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        case SbxCURRENCY:
        case SbxSALINT64:
            aTmp.pnInt64 = &p->nInt64; goto direct;
        case SbxSALUINT64:
            aTmp.puInt64 = &p->uInt64; goto direct;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            SbxDecimal* pDec = ImpCreateDecimal( p );
            if( !pDec->setSingle( n ) )
                SbxBase::SetError( SbxERR_OVERFLOW );
            break;
            }
        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        // from here no tests
        case SbxSINGLE:
            p->nSingle = n; break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
        {
            if( !p->pOUString )
                p->pOUString = new ::rtl::OUString;
            ImpCvtNum( (double) n, 6, *p->pOUString );
            break;
        }
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutSingle( n );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            if( n > SbxMAXCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXCHAR;
            }
            else if( n < SbxMINCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINCHAR;
            }
            *p->pChar = (sal_Unicode) n; break;
        case SbxBYREF | SbxBYTE:
            if( n > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXBYTE;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pByte = (sal_uInt8) n; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            if( n > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXINT;
            }
            else if( n < SbxMININT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMININT;
            }
            *p->pInteger = (sal_Int16) n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            if( n > SbxMAXUINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXUINT;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pUShort = (sal_uInt16) n; break;
        case SbxBYREF | SbxLONG:
        {
            sal_Int32 i;
            if( n > SbxMAXLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); i = SbxMAXLNG;
            }
            else if( n < SbxMINLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); i = SbxMINLNG;
            }
            else
            {
                i = sal::static_int_cast< sal_Int32 >(n);
            }
            *p->pLong = i; break;
        }
        case SbxBYREF | SbxULONG:
        {
            sal_uInt32 i;
            if( n > SbxMAXULNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); i = SbxMAXULNG;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); i = 0;
            }
            else
            {
                i = sal::static_int_cast< sal_uInt32 >(n);
            }
            *p->pULong = i; break;
        }
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = (double) n; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = (sal_Int64)n; break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = (sal_uInt64)n; break;
        case SbxBYREF | SbxCURRENCY:
            double d;
            if( n > SbxMAXCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); d = SbxMAXCURR;
            }
            else if( n < SbxMINCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); d = SbxMINCURR;
            }
            else
            {
                d = n;
            }
            *p->pnInt64 = ImpDoubleToCurrency( d ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
