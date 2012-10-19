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

sal_uInt8 ImpGetByte( const SbxValues* p )
{
    SbxValues aTmp;
    sal_uInt8 nRes;
start:
    switch( +p->eType )
    {
        case SbxNULL:
            SbxBase::SetError( SbxERR_CONVERSION );
        case SbxEMPTY:
            nRes = 0; break;
        case SbxCHAR:
            if( p->nChar > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = (sal_uInt8) p->nChar;
            break;
        case SbxBYTE:
            nRes = (sal_uInt8) p->nByte;    break;
        case SbxINTEGER:
        case SbxBOOL:
            if( p->nInteger > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else if( p->nInteger < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = (sal_uInt8) p->nInteger;
            break;
        case SbxERROR:
        case SbxUSHORT:
            if( p->nUShort > (sal_uInt16) SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else
                nRes = (sal_uInt8) p->nUShort;
            break;
        case SbxLONG:
            if( p->nLong > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else if( p->nLong < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = (sal_uInt8) p->nLong;
            break;
        case SbxULONG:
            if( p->nULong > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else
                nRes = (sal_uInt8) p->nULong;
            break;
        case SbxCURRENCY:
        case SbxSALINT64:
        {
            sal_Int64 val = p->nInt64;
            if ( p->eType == SbxCURRENCY )
                val = val / CURRENCY_FACTOR;
            if( val > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else if( p->nInt64 < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = (sal_uInt8) val;
            break;
        }
        case SbxSALUINT64:
            if( p->uInt64 > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else
                nRes = (sal_uInt8) p->uInt64;
            break;
        case SbxSINGLE:
            if( p->nSingle > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else if( p->nSingle < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = (sal_uInt8) ImpRound( p->nSingle );
            break;
        case SbxDATE:
        case SbxDOUBLE:
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            double dVal;
            if( p->eType == SbxDECIMAL )
            {
                dVal = 0.0;
                if( p->pDecimal )
                    p->pDecimal->getDouble( dVal );
            }
            else
                dVal = p->nDouble;

            if( dVal > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else if( dVal < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = (sal_uInt8) ImpRound( dVal );
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
                else if( d > SbxMAXBYTE )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
                }
                else if( d < 0 )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
                }
                else
                    nRes = (sal_uInt8) ( d + 0.5 );
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetByte();
            else
            {
                SbxBase::SetError( SbxERR_NO_OBJECT ); nRes = 0;
            }
            break;
        }

        case SbxBYREF | SbxBYTE:
            nRes = p->nByte; break;

        // from here on will be tested
        case SbxBYREF | SbxCHAR:
            aTmp.nChar = *p->pChar; goto ref;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            aTmp.nInteger = *p->pInteger; goto ref;
        case SbxBYREF | SbxLONG:
            aTmp.nLong = *p->pLong; goto ref;
        case SbxBYREF | SbxULONG:
            aTmp.nULong = *p->pULong; goto ref;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            aTmp.nUShort = *p->pUShort; goto ref;
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

void ImpPutByte( SbxValues* p, sal_uInt8 n )
{
    switch( +p->eType )
    {
        case SbxBYTE:
            p->nByte = n; break;
        case SbxINTEGER:
        case SbxBOOL:
            p->nInteger = n; break;
        case SbxERROR:
        case SbxUSHORT:
            p->nUShort = n; break;
        case SbxLONG:
            p->nLong = n; break;
        case SbxULONG:
            p->nULong = n; break;
        case SbxSINGLE:
            p->nSingle = n; break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;
        case SbxCURRENCY:
            p->nInt64 = n * CURRENCY_FACTOR; break;
        case SbxSALINT64:
            p->nInt64 = n; break;
        case SbxSALUINT64:
            p->uInt64 = n; break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setByte( n );
            break;

        case SbxCHAR:
            p->nChar = (sal_Unicode) n; break;

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
                pVal->PutByte( n );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            *p->pChar = (sal_Unicode) n; break;
        case SbxBYREF | SbxBYTE:
            *p->pByte = n; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            *p->pInteger = n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            *p->pUShort = n; break;
        case SbxBYREF | SbxLONG:
            *p->pLong = n; break;
        case SbxBYREF | SbxULONG:
            *p->pULong = n; break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = n; break;
        case SbxBYREF | SbxCURRENCY:
            p->nInt64 = n * CURRENCY_FACTOR; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = n; break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
