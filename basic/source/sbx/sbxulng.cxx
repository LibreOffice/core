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

sal_uInt32 ImpGetULong( const SbxValues* p )
{
    SbxValues aTmp;
    sal_uInt32 nRes;
start:
    switch( +p->eType )
    {
        case SbxNULL:
            SbxBase::SetError( SbxERR_CONVERSION );
        case SbxEMPTY:
            nRes = 0; break;
        case SbxCHAR:
            nRes = p->nChar;
            break;
        case SbxBYTE:
            nRes = p->nByte; break;
        case SbxINTEGER:
        case SbxBOOL:
            if( p->nInteger < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = p->nInteger;
            break;
        case SbxERROR:
        case SbxUSHORT:
            nRes = p->nUShort;
            break;
        case SbxLONG:
            if( p->nLong < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = p->nLong;
            break;
        case SbxULONG:
            nRes = p->nULong; break;
        case SbxSINGLE:
            if( p->nSingle > SbxMAXULNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXULNG;
            }
            else if( p->nSingle < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = (sal_uInt32) ( p->nSingle + 0.5 );
            break;
        case SbxDATE:
        case SbxDOUBLE:
        case SbxSALINT64:
        case SbxSALUINT64:
        case SbxCURRENCY:
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            double dVal;
            if( p->eType == SbxCURRENCY )
                dVal = ImpCurrencyToDouble( p->nInt64 );
            else if( p->eType == SbxSALINT64 )
                dVal = static_cast< double >(p->nInt64);
            else if( p->eType == SbxSALUINT64 )
                dVal = ImpSalUInt64ToDouble( p->uInt64 );
            else if( p->eType == SbxDECIMAL )
            {
                dVal = 0.0;
                if( p->pDecimal )
                    p->pDecimal->getDouble( dVal );
            }
            else
                dVal = p->nDouble;

            if( dVal > SbxMAXULNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXULNG;
            }
            else if( dVal < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = (sal_uInt32) ( dVal + 0.5 );
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
                else if( d > SbxMAXULNG )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXULNG;
                }
                else if( d < 0 )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
                }
                else
                    nRes = (sal_uInt32) ( d + 0.5 );
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetULong();
            else
            {
                SbxBase::SetError( SbxERR_NO_OBJECT ); nRes = 0;
            }
            break;
        }

        case SbxBYREF | SbxBYTE:
            nRes = *p->pByte; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            nRes = *p->pUShort; break;
        case SbxBYREF | SbxULONG:
            nRes = *p->pULong; break;

        // from here on tests
        case SbxBYREF | SbxCHAR:
            aTmp.nChar = *p->pChar; goto ref;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            aTmp.nInteger = *p->pInteger; goto ref;
        case SbxBYREF | SbxLONG:
            aTmp.nLong = *p->pLong; goto ref;
        case SbxBYREF | SbxSINGLE:
            aTmp.nSingle = *p->pSingle; goto ref;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            aTmp.nDouble = *p->pDouble; goto ref;
        case SbxBYREF | SbxCURRENCY:
        case SbxBYREF | SbxSALINT64:
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

void ImpPutULong( SbxValues* p, sal_uInt32 n )
{
    SbxValues aTmp;
start:
    switch( +p->eType )
    {
        case SbxULONG:
            p->nULong = n; break;
        case SbxSINGLE:
            p->nSingle = (float) n; break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;
        case SbxCURRENCY:
        case SbxSALINT64:
            aTmp.pnInt64 = &p->nInt64; goto direct;
        case SbxSALUINT64:
            p->uInt64 = n; break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setULong( n );
            break;

        // from here on tests
        case SbxCHAR:
            aTmp.pChar = &p->nChar; goto direct;
        case SbxUINT:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxINTEGER:
        case SbxBOOL:
            aTmp.pInteger = &p->nInteger; goto direct;
        case SbxLONG:
            aTmp.pLong = &p->nLong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pOUString )
                p->pOUString = new ::rtl::OUString;
            ImpCvtNum( (double) n, 0, *p->pOUString );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutULong( n );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            if( n > SbxMAXCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXCHAR;
            }
            *p->pChar = (sal_Unicode) n; break;
        case SbxBYREF | SbxBYTE:
            if( n > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXBYTE;
            }
            *p->pByte = (sal_uInt8) n; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            if( n > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXINT;
            }
            *p->pInteger = (sal_Int16) n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            if( n > SbxMAXUINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXUINT;
            }
            *p->pUShort = (sal_uInt16) n; break;
        case SbxBYREF | SbxLONG:
            if( n > SbxMAXLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXLNG;
            }
            *p->pLong = (sal_Int32) n; break;
        case SbxBYREF | SbxULONG:
            *p->pULong = n; break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = (float) n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = n; break;
        case SbxBYREF | SbxCURRENCY:
            *p->pnInt64 = n * CURRENCY_FACTOR; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = n; break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
