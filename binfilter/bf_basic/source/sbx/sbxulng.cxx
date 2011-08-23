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

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#include "sbx.hxx"
#include "sbxconv.hxx"

namespace binfilter {

UINT32 ImpGetULong( const SbxValues* p )
{
    SbxValues aTmp;
    UINT32 nRes;
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
                nRes = (UINT32) ( p->nSingle + 0.5 );
            break;
        case SbxDATE:
        case SbxDOUBLE:
        case SbxLONG64:
        case SbxULONG64:
        case SbxSALINT64:
        case SbxSALUINT64:
        case SbxCURRENCY:
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            double dVal;
            if( p->eType ==	SbxCURRENCY )
                dVal = ImpCurrencyToDouble( p->nLong64 );
            else if( p->eType == SbxLONG64 )
                dVal = ImpINT64ToDouble( p->nLong64 );
            else if( p->eType == SbxULONG64 )
                dVal = ImpUINT64ToDouble( p->nULong64 );
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
                nRes = (UINT32) ( dVal + 0.5 );
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
                else if( d > SbxMAXULNG )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXULNG;
                }
                else if( d < 0 )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
                }
                else
                    nRes = (UINT32) ( d + 0.5 );
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

        // Tests ab hier
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
        case SbxBYREF | SbxSALINT64:
            aTmp.nInt64 = *p->pnInt64; goto ref;
        case SbxBYREF | SbxSALUINT64:
            aTmp.uInt64 = *p->puInt64; goto ref;
        case SbxBYREF | SbxULONG64:
            aTmp.nULong64 = *p->pULong64; goto ref;
        case SbxBYREF | SbxLONG64:
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

void ImpPutULong( SbxValues* p, UINT32 n )
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
        case SbxSALINT64:
            p->nInt64 = n; break;
        case SbxSALUINT64:
            p->uInt64 = n; break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setULong( n );
            break;

        // Tests ab hier
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
        case SbxULONG64:
            aTmp.pULong64 = &p->nULong64; goto direct;
        case SbxLONG64:
        case SbxCURRENCY:
            aTmp.pLong64 = &p->nLong64; goto direct;
        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pString )
                p->pString = new XubString;
            ImpCvtNum( (double) n, 0, *p->pString );
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
            *p->pChar = (xub_Unicode) n; break;
        case SbxBYREF | SbxBYTE:
            if( n > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXBYTE;
            }
            *p->pByte = (BYTE) n; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            if( n > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXINT;
            }
            *p->pInteger = (INT16) n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            if( n > SbxMAXUINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXUINT;
            }
            *p->pUShort = (UINT16) n; break;
        case SbxBYREF | SbxLONG:
            if( n > SbxMAXLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXLNG;
            }
            *p->pLong = (INT32) n; break;
        case SbxBYREF | SbxULONG:
            *p->pULong = n; break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = (float) n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = n; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = n; break;
        case SbxBYREF | SbxCURRENCY:
            double d;
            if( n > SbxMAXCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); d = SbxMAXCURR;
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
