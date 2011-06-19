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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"
#include <tools/errcode.hxx>
#include <basic/sbx.hxx>
#include "sbxconv.hxx"

xub_Unicode ImpGetChar( const SbxValues* p )
{
    SbxValues aTmp;
    xub_Unicode nRes = 0;
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
            nRes = (xub_Unicode) p->nByte;
            break;
        case SbxINTEGER:
        case SbxBOOL:
            if( p->nInteger < SbxMINCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
            }
            else
                nRes = (xub_Unicode) p->nInteger;
            break;
        case SbxERROR:
        case SbxUSHORT:
            nRes = (xub_Unicode) p->nUShort;
            break;
        case SbxLONG:
            if( p->nLong > SbxMAXCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else if( p->nLong < SbxMINCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
            }
            else
                nRes = (xub_Unicode) p->nLong;
            break;
        case SbxULONG:
            if( p->nULong > SbxMAXCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else
                nRes = (xub_Unicode) p->nULong;
            break;
        case SbxCURRENCY:
        case SbxSALINT64:
        {
            sal_Int64 val = p->nInt64;

            if ( p->eType == SbxCURRENCY )
                val = val / CURRENCY_FACTOR;

            if( val > SbxMAXCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else if( p->nInt64 < SbxMINCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
            }
            else
                nRes = (xub_Unicode) val;
            break;
        }
        case SbxSALUINT64:
            if( p->uInt64 > SbxMAXCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else
                nRes = (xub_Unicode) p->uInt64;
            break;
        case SbxSINGLE:
            if( p->nSingle > SbxMAXCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else if( p->nSingle < SbxMINCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
            }
            else
                nRes = (xub_Unicode) ImpRound( p->nSingle );
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

            if( dVal > SbxMAXCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else if( dVal < SbxMINCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
            }
            else
                nRes = (sal_uInt8) ImpRound( dVal );
            break;
            }
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if ( p->pOUString )
            {
                double d;
                SbxDataType t;
                if( ImpScan( *p->pOUString, d, t, NULL ) != SbxERR_OK )
                    nRes = 0;
                else if( d > SbxMAXCHAR )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
                }
                else if( d < SbxMINCHAR )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
                }
                else
                    nRes = (xub_Unicode) ImpRound( d );
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetChar();
            else
            {
                SbxBase::SetError( SbxERR_NO_OBJECT ); nRes = 0;
            }
            break;
        }

        case SbxBYREF | SbxCHAR:
            nRes = *p->pChar; break;
        // from here on will be tested
        case SbxBYREF | SbxBYTE:
            aTmp.nByte = *p->pByte; goto ref;
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

void ImpPutChar( SbxValues* p, xub_Unicode n )
{
    SbxValues aTmp;
start:
    switch( +p->eType )
    {
        case SbxCHAR:
            p->nChar = n; break;
        case SbxINTEGER:
        case SbxBOOL:
            p->nInteger = n; break;
        case SbxLONG:
            p->nLong = n; break;
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
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setChar( n );
            break;

        // from here on will be tested
        case SbxBYTE:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if ( !p->pOUString )
                p->pOUString = new ::rtl::OUString( n );
            else
                *p->pOUString = ::rtl::OUString( n );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutChar( n );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            *p->pChar = n; break;
        case SbxBYREF | SbxBYTE:
            *p->pByte = (sal_uInt8) n; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            *p->pInteger = n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            *p->pUShort = (sal_uInt16) n; break;
        case SbxBYREF | SbxLONG:
            *p->pLong = (sal_Int32) n; break;
        case SbxBYREF | SbxULONG:
            *p->pULong = (sal_uInt32) n; break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = (float) n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = (double) n; break;
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
