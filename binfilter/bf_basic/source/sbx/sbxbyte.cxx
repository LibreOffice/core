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

BYTE ImpGetByte( const SbxValues* p )
{
    SbxValues aTmp;
    BYTE nRes;
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
                nRes = (BYTE) p->nChar;
            break;
        case SbxBYTE:
            nRes = (BYTE) p->nByte;	break;
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
                nRes = (BYTE) p->nInteger;
            break;
        case SbxERROR:
        case SbxUSHORT:
            if( p->nUShort > (USHORT) SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else
                nRes = (BYTE) p->nUShort;
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
                nRes = (BYTE) p->nLong;
            break;
        case SbxULONG:
            if( p->nULong > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else
                nRes = (BYTE) p->nULong;
            break;
        case SbxSALINT64:
            if( p->nInt64 > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else if( p->nInt64 < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = (BYTE) p->nInt64;
            break;
        case SbxSALUINT64:
            if( p->uInt64 > SbxMAXBYTE )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
            }
            else
                nRes = (BYTE) p->uInt64;
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
                nRes = (BYTE) ImpRound( p->nSingle );
            break;
        case SbxDATE:
        case SbxDOUBLE:
        case SbxLONG64:
        case SbxULONG64:
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
            else if( p->eType == SbxDECIMAL )
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
                nRes = (BYTE) ImpRound( dVal );
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
                else if( d > SbxMAXBYTE )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXBYTE;
                }
                else if( d < 0 )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
                }
                else
                    nRes = (BYTE) ( d + 0.5 );
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

        // ab hier wird getestet
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
        case SbxBYREF | SbxULONG64:
            aTmp.nULong64 = *p->pULong64; goto ref;
        case SbxBYREF | SbxLONG64:
        case SbxBYREF | SbxCURRENCY:
            aTmp.nLong64 = *p->pLong64; goto ref;
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

void ImpPutByte( SbxValues* p, BYTE n )
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
        case SbxSALINT64:
            p->nInt64 = n; break;
        case SbxSALUINT64:
            p->uInt64 = n; break;
        case SbxULONG64:
            p->nULong64 = ImpDoubleToUINT64( (double)n ); break;
        case SbxLONG64:
            p->nLong64 = ImpDoubleToINT64( (double)n ); break;
        case SbxCURRENCY:
            p->nLong64 = ImpDoubleToCurrency( (double)n ); break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setByte( n );
            break;

        case SbxCHAR:
            p->nChar = (xub_Unicode) n; break;

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
                pVal->PutByte( n );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            *p->pChar = (xub_Unicode) n; break;
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
        case SbxBYREF | SbxULONG64:
            *p->pULong64 = ImpDoubleToUINT64( (double)n ); break;
        case SbxBYREF | SbxLONG64:
            *p->pLong64 = ImpDoubleToINT64( (double)n ); break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = n; break;
        case SbxBYREF | SbxCURRENCY:
            *p->pLong64 = ImpDoubleToCurrency( (double)n ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

}
