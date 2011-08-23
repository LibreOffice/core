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

#include <tools/errcode.hxx>
#include "sbx.hxx"
#include "sbxconv.hxx"

namespace binfilter {

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
        case SbxSALINT64:
            nRes = (float) p->nInt64; break;
        case SbxSALUINT64:
            nRes = (float) ImpSalUInt64ToDouble( p->uInt64 ); break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            if( p->pDecimal )
                p->pDecimal->getSingle( nRes );
            else
                nRes = 0.0;
            break;
        case SbxDATE:
        case SbxDOUBLE:
        case SbxLONG64:
        case SbxULONG64:
        case SbxCURRENCY:
            {
            double dVal;
            if( p->eType ==	SbxCURRENCY )
                dVal = ImpCurrencyToDouble( p->nLong64 );
            else if( p->eType == SbxLONG64 )
                dVal = ImpINT64ToDouble( p->nLong64 );
            else if( p->eType == SbxULONG64 )
                dVal = ImpUINT64ToDouble( p->nULong64 );
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
            if( !p->pString )
                nRes = 0;
            else
            {
                double d;
                SbxDataType t;
                if( ImpScan( *p->pString, d, t, NULL ) != SbxERR_OK )
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
        // ab hier muss getestet werden
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            aTmp.nDouble = *p->pDouble; goto ref;
        case SbxBYREF | SbxULONG64:
            aTmp.nULong64 = *p->pULong64; goto ref;
        case SbxBYREF | SbxLONG64:
        case SbxBYREF | SbxSALINT64:
            nRes = (float) *p->pnInt64; break;
        case SbxBYREF | SbxSALUINT64:
            nRes = (float) ImpSalUInt64ToDouble( *p->puInt64 ); break;
        case SbxBYREF | SbxCURRENCY:
            aTmp.nLong64 = *p->pLong64; goto ref;
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
        case SbxULONG64:
            aTmp.pULong64 = &p->nULong64; goto direct;
        case SbxLONG64:
        case SbxCURRENCY:
            aTmp.pLong64 = &p->nLong64; goto direct;
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

        // keine Tests ab hier
        case SbxSINGLE:
            p->nSingle = n; break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
        {
            if( !p->pString )
                p->pString = new XubString;
            ImpCvtNum( (double) n, 6, *p->pString );
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
            *p->pChar = (xub_Unicode) n; break;
        case SbxBYREF | SbxBYTE:
            if( n > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXBYTE;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pByte = (BYTE) n; break;
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
            *p->pInteger = (INT16) n; break;
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
            *p->pUShort = (UINT16) n; break;
        case SbxBYREF | SbxLONG:
        {
            INT32 i;
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
                i = sal::static_int_cast< INT32 >(n);
            }
            *p->pLong = i; break;
        }
        case SbxBYREF | SbxULONG:
        {
            UINT32 i;
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
                i = sal::static_int_cast< UINT32 >(n);
            }
            *p->pULong = i; break;
        }
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = (double) n; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = ImpDoubleToSalInt64( (double) n ); break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = ImpDoubleToSalUInt64( (double) n ); break;
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
            *p->pLong64 = ImpDoubleToCurrency( n ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
