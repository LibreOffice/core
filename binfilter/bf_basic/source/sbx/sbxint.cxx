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

double ImpRound( double d )
{
    return d + ( d < 0 ? -0.5 : 0.5 );
}

INT16 ImpGetInteger( const SbxValues* p )
{
    SbxValues aTmp;
    INT16 nRes;
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
            if( p->nUShort > (USHORT) SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
            }
            else
                nRes = (INT16) p->nUShort;
            break;
        case SbxLONG:
            if( p->nLong > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
            }
            else if( p->nLong < SbxMININT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMININT;
            }
            else
                nRes = (INT16) p->nLong;
            break;
        case SbxULONG:
            if( p->nULong > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
            }
            else
                nRes = (INT16) p->nULong;
            break;
        case SbxSINGLE:
            if( p->nSingle > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
            }
            else if( p->nSingle < SbxMININT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMININT;
            }
            else
                nRes = (INT16) ImpRound( p->nSingle );
            break;
        case SbxSALINT64:
            if( p->nInt64 > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
            }
            else if( p->nInt64 < SbxMININT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMININT;
            }
            else
                nRes = (INT16) p->nInt64;
            break;
        case SbxSALUINT64:
            if( p->uInt64 > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
            }
            else
                nRes = (INT16) p->uInt64;
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

            if( dVal > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
            }
            else if( dVal < SbxMININT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMININT;
            }
            else
                nRes = (INT16) ImpRound( dVal );
            break;
            }
        case SbxLPSTR:
        case SbxSTRING:
        case SbxBYREF | SbxSTRING:
            if( !p->pString )
                nRes = 0;
            else
            {
                double d;
                SbxDataType t;
                if( ImpScan( *p->pString, d, t, NULL ) != SbxERR_OK )
                    nRes = 0;
                else if( d > SbxMAXINT )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
                }
                else if( d < SbxMININT )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMININT;
                }
                else
                    nRes = (INT16) ImpRound( d );
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetInteger();
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

        // ab hier muss getestet werden
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

void ImpPutInteger( SbxValues* p, INT16 n )
{
    SbxValues aTmp;
start:
    switch( +p->eType )
    {
        // hier muss getestet werden
        case SbxCHAR:
            aTmp.pChar = &p->nChar; goto direct;
        case SbxBYTE:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        case SbxSALUINT64:
            aTmp.puInt64 = &p->uInt64; goto direct;
        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        // ab hier nicht mehr
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
        case SbxSALINT64:
            p->nInt64 = n; break;
        case SbxULONG64:
            p->nULong64 = ImpDoubleToUINT64( (double)n ); break;
        case SbxLONG64:
            p->nLong64 = ImpDoubleToINT64( (double)n ); break;
        case SbxCURRENCY:
            p->nLong64 = ImpDoubleToCurrency( (double)n ); break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setInt( n );
            break;

        case SbxLPSTR:
        case SbxSTRING:
        case SbxBYREF | SbxSTRING:
            if( !p->pString )
                p->pString = new XubString;
            ImpCvtNum( (double) n, 0, *p->pString );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutInteger( n );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            if( n < SbxMINCHAR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINCHAR;
            }
            *p->pChar = (char) n; break;
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
            *p->pInteger = n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pUShort = (UINT16) n; break;
        case SbxBYREF | SbxLONG:
            *p->pLong = (INT32) n; break;
        case SbxBYREF | SbxULONG:
            if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pULong = (UINT32) n; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); *p->puInt64 = 0;
            }
            else
                *p->puInt64 = n; 
            break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = (float) n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = (double) n; break;
        case SbxBYREF | SbxULONG64:
            *p->pULong64 = ImpDoubleToUINT64( (double)n ); break;
        case SbxBYREF | SbxLONG64:
            *p->pLong64 = ImpDoubleToINT64( (double)n ); break;
        case SbxBYREF | SbxCURRENCY:
            *p->pLong64 = ImpDoubleToCurrency( (double)n ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}


// sal_Int64 / hyper

sal_Int64 ImpDoubleToSalInt64( double d )
{
    sal_Int64 nRes;
    if( d > SbxMAXSALINT64 )
    {
        SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXSALINT64;
    }
    else if( d < SbxMINSALINT64 )
    {
        SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINSALINT64;
    }
    else
        nRes = (sal_Int64) ImpRound( d );
    return nRes;
}

sal_uInt64 ImpDoubleToSalUInt64( double d )
{
    sal_uInt64 nRes;
    if( d > SbxMAXSALUINT64 )
    {
        SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXSALUINT64;
    }
    else if( d < 0.0 )
    {
        SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
    }
    else
        nRes = (sal_uInt64) ImpRound( d );
    return nRes;
}

double ImpSalUInt64ToDouble( sal_uInt64 n )
{
    double d = 0.0;
    if( n > SbxMAXSALINT64 )
        SbxBase::SetError( SbxERR_CONVERSION );
    else
        d = (double)(sal_Int64) n;
    return d;
}


sal_Int64 ImpGetInt64( const SbxValues* p )
{
    SbxValues aTmp;
    sal_Int64 nRes;
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
            nRes = p->nLong; break;
        case SbxULONG:
            nRes = (sal_Int64) p->nULong; break;
        case SbxSINGLE:
            nRes = ImpDoubleToSalInt64( (double)p->nSingle );
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

            nRes = ImpDoubleToSalInt64( dVal );
            break;
            }
        case SbxSALINT64:
            nRes = p->nInt64; break;
        case SbxSALUINT64:
            if( p->uInt64 > SbxMAXSALINT64 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXSALINT64;
            }
            else
                nRes = (sal_Int64) p->uInt64;
            break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pString )
                nRes = 0;
            else
            {
                ::rtl::OUString aOUStr( *p->pString );
                   ::rtl::OString aOStr = ::rtl::OUStringToOString
                    ( aOUStr, RTL_TEXTENCODING_ASCII_US );
                nRes = aOStr.toInt64();
                if( nRes == 0 )
                {
                    // Check if really 0 or invalid conversion
                    double d;
                    SbxDataType t;
                    if( ImpScan( *p->pString, d, t, NULL ) != SbxERR_OK )
                        nRes = 0;
                    else
                        nRes = ImpDoubleToSalInt64( d );
                }
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetInt64();
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
        case SbxBYREF | SbxSALINT64:
            nRes = *p->pnInt64; break;

        // from here the values has to be checked
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

void ImpPutInt64( SbxValues* p, sal_Int64 n )
{
    SbxValues aTmp;

start:
    switch( +p->eType )
    {
        // Check neccessary
        case SbxCHAR:
            aTmp.pChar = &p->nChar; goto direct;
        case SbxBYTE:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxINTEGER:
        case SbxBOOL:
            aTmp.pInteger = &p->nInteger; goto direct;
        case SbxULONG64:
            aTmp.pULong64 = &p->nULong64; goto direct;
        case SbxLONG64:
        case SbxCURRENCY:
            aTmp.pLong64 = &p->nLong64; goto direct;
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        case SbxLONG:
            aTmp.pnInt64 = &p->nInt64; goto direct;
        case SbxSALUINT64:
            aTmp.puInt64 = &p->uInt64; goto direct;

        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        // Check not neccessary
        case SbxSALINT64:
            p->nInt64 = n; break;
        case SbxSINGLE:
            p->nSingle = (float) n; break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = (double) n; break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
        {
            if( !p->pString )
                p->pString = new XubString;

            ::rtl::OString  aOStr  = ::rtl::OString::valueOf( n );
               ::rtl::OUString aOUStr = ::rtl::OStringToOUString
                ( aOStr, RTL_TEXTENCODING_ASCII_US );
            (*p->pString) = aOUStr;
            break;
        }
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutInt64( n );
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
            *p->pSingle = (float) n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = (double) n; break;
        case SbxBYREF | SbxCURRENCY:
            if( n > SbxMAXCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = (sal_Int64) SbxMAXCURR;
            }
            else if( n < SbxMINCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = (sal_Int64) SbxMINCURR;
            }
            *p->pLong64 = ImpDoubleToCurrency( (double)n ); break;

        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->puInt64 = (sal_Int64) n; break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

sal_uInt64 ImpGetUInt64( const SbxValues* p )
{
    SbxValues aTmp;
    sal_uInt64 nRes;
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
            nRes = p->nLong; break;
        case SbxULONG:
            nRes = (sal_uInt64) p->nULong; break;
        case SbxSINGLE:
            nRes = ImpDoubleToSalUInt64( (double)p->nSingle );
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

            nRes = ImpDoubleToSalUInt64( dVal );
            break;
            }
        case SbxSALINT64:
            if( p->nInt64 < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = (sal_uInt64) p->nInt64;
        case SbxSALUINT64:
            nRes = p->uInt64; break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pString )
                nRes = 0;
            else
            {
                ::rtl::OUString aOUStr( *p->pString );
                   ::rtl::OString aOStr = ::rtl::OUStringToOString
                    ( aOUStr, RTL_TEXTENCODING_ASCII_US );
                sal_Int64 n64 = aOStr.toInt64();
                if( n64 == 0 )
                {
                    // Check if really 0 or invalid conversion
                    double d;
                    SbxDataType t;
                    if( ImpScan( *p->pString, d, t, NULL ) != SbxERR_OK )
                        nRes = 0;
                    else if( d > SbxMAXSALUINT64 )
                    {
                        SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXSALUINT64;
                    }
                    else if( d < 0.0 )
                    {
                        SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
                    }
                    else
                        nRes = (sal_uInt64) ImpRound( d );
                }
                else if( n64 < 0 )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
                }
                else
                {
                    nRes = n64;
                }
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                nRes = pVal->GetUInt64();
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
        case SbxBYREF | SbxSALUINT64:
            nRes = *p->puInt64; break;

        // from here the values has to be checked
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
        ref:
            aTmp.eType = SbxDataType( p->eType & 0x0FFF );
            p = &aTmp; goto start;

        default:
            SbxBase::SetError( SbxERR_CONVERSION ); nRes = 0;
    }
    return nRes;
}

void ImpPutUInt64( SbxValues* p, sal_uInt64 n )
{
    SbxValues aTmp;

start:
    switch( +p->eType )
    {
        // Check neccessary
        case SbxCHAR:
            aTmp.pChar = &p->nChar; goto direct;
        case SbxBYTE:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxINTEGER:
        case SbxBOOL:
            aTmp.pInteger = &p->nInteger; goto direct;
        case SbxULONG64:
            aTmp.pULong64 = &p->nULong64; goto direct;
        case SbxLONG64:
        case SbxCURRENCY:
            aTmp.pLong64 = &p->nLong64; goto direct;
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        case SbxLONG:
            aTmp.pnInt64 = &p->nInt64; goto direct;
        case SbxSALINT64:
            aTmp.pnInt64 = &p->nInt64; goto direct;
        case SbxSINGLE:
            aTmp.pSingle = &p->nSingle; goto direct;
        case SbxDATE:
        case SbxDOUBLE:
            aTmp.pDouble = &p->nDouble; goto direct;

        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        // Check not neccessary
        case SbxSALUINT64:
            p->uInt64 = n; break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pString )
                p->pString = new XubString;
            if( n > SbxMAXSALINT64 )
                SbxBase::SetError( SbxERR_CONVERSION );
            else
            {
                ::rtl::OString  aOStr  = ::rtl::OString::valueOf( (sal_Int64)n );
                   ::rtl::OUString aOUStr = ::rtl::OStringToOUString
                    ( aOStr, RTL_TEXTENCODING_ASCII_US );
                (*p->pString) = aOUStr;
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutUInt64( n );
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
            if( n > SbxMAXULNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXULNG;
            }
            *p->pULong = (UINT32) n; break;
        case SbxBYREF | SbxSINGLE:
            *p->pDouble = (float)ImpSalUInt64ToDouble( n ); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = ImpSalUInt64ToDouble( n ); break;
        case SbxBYREF | SbxCURRENCY:
            if( n > SbxMAXSALINT64 || (sal_Int64)n > SbxMAXCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = (sal_Int64) SbxMAXCURR;
            }
            *p->pLong64 = ImpDoubleToCurrency( (double)(sal_Int64) n ); break;

        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = n; break;
        case SbxBYREF | SbxSALINT64:
            if( n > SbxMAXSALINT64 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pnInt64 = (sal_Int64) n; break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
