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

double ImpGetDouble( const SbxValues* p )
{
    double nRes;
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
            nRes = p->nLong; break;
        case SbxULONG:
            nRes = p->nULong; break;
        case SbxSINGLE:
            nRes = p->nSingle; break;
        case SbxDATE:
        case SbxDOUBLE:
            nRes = p->nDouble; break;
        case SbxCURRENCY:
            nRes = ImpCurrencyToDouble( p->nLong64 ); break;
        case SbxSALINT64:
            nRes = static_cast< double >(p->nInt64); break;
        case SbxSALUINT64:
            nRes = ImpSalUInt64ToDouble( p->uInt64 ); break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            if( p->pDecimal )
                p->pDecimal->getDouble( nRes );
            else
                nRes = 0.0;
            break;
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
                else
                    nRes = d;
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetDouble();
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
            nRes = *p->pLong; break;
        case SbxBYREF | SbxULONG:
            nRes = *p->pULong; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            nRes = *p->pUShort; break;
        case SbxBYREF | SbxSINGLE:
            nRes = *p->pSingle; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            nRes = *p->pDouble; break;
        case SbxBYREF | SbxCURRENCY:
            nRes = ImpCurrencyToDouble( *p->pLong64 ); break;
        case SbxBYREF | SbxSALINT64:
            nRes = static_cast< double >(*p->pnInt64); break;
        case SbxBYREF | SbxSALUINT64:
            nRes = ImpSalUInt64ToDouble( *p->puInt64 ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION ); nRes = 0;
    }
    return nRes;
}

void ImpPutDouble( SbxValues* p, double n, BOOL bCoreString )
{
    SbxValues aTmp;
start:
    switch( +p->eType )
    {
        // Hier sind Tests notwendig
        case SbxCHAR:
            aTmp.pChar = &p->nChar; goto direct;
        case SbxBYTE:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxINTEGER:
        case SbxBOOL:
            aTmp.pInteger = &p->nInteger; goto direct;
        case SbxLONG:
        case SbxCURRENCY:
            aTmp.pLong = &p->nLong; goto direct;
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        case SbxSINGLE:
            aTmp.pSingle = &p->nSingle; goto direct;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            SbxDecimal* pDec = ImpCreateDecimal( p );
            if( !pDec->setDouble( n ) )
                SbxBase::SetError( SbxERR_OVERFLOW ); 
            break;
            }
        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

            // ab hier nicht mehr
        case SbxSALINT64:
            p->nInt64 = ImpDoubleToSalInt64( n ); break;
        case SbxSALUINT64:
            p->uInt64 = ImpDoubleToSalUInt64( n ); break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pString )
                p->pString = new XubString;
            ImpCvtNum( (double) n, 14, *p->pString, bCoreString );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutDouble( n );
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
            if( n > SbxMAXLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXLNG;
            }
            else if( n < SbxMINLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINLNG;
            }
            *p->pLong = (INT32) n; break;
        case SbxBYREF | SbxULONG:
            if( n > SbxMAXULNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXULNG;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pULong = (UINT32) n; break;
        case SbxBYREF | SbxSINGLE:
            if( n > SbxMAXSNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXSNG;
            }
            else if( n < SbxMINSNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINSNG;
            }
            else if( n > 0 && n < SbxMAXSNG2 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXSNG2;
            }
            else if( n < 0 && n > SbxMINSNG2 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINSNG2;
            }
            *p->pSingle = (float) n; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = ImpDoubleToSalInt64( n ); break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = ImpDoubleToSalUInt64( n ); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = (double) n; break;
        case SbxBYREF | SbxCURRENCY:
            if( n > SbxMAXCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXCURR;
            }
            else if( n < SbxMINCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINCURR;
            }
            *p->pLong64 = ImpDoubleToCurrency( n ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

}
