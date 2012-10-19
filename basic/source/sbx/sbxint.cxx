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

double ImpRound( double d )
{
    return d + ( d < 0 ? -0.5 : 0.5 );
}

sal_Int16 ImpGetInteger( const SbxValues* p )
{
    SbxValues aTmp;
    sal_Int16 nRes;
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
            if( p->nUShort > (sal_uInt16) SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
            }
            else
                nRes = (sal_Int16) p->nUShort;
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
                nRes = (sal_Int16) p->nLong;
            break;
        case SbxULONG:
            if( p->nULong > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
            }
            else
                nRes = (sal_Int16) p->nULong;
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
                nRes = (sal_Int16) ImpRound( p->nSingle );
            break;
        case SbxCURRENCY:
            {
                sal_Int64 tstVal = (sal_Int64) p->nInt64 / (sal_Int64) CURRENCY_FACTOR;

                if( tstVal > SbxMAXINT )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
                }
                else if( tstVal  < SbxMININT )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMININT;
                }
                else
                    nRes = (sal_Int16) (tstVal);
                break;
            }
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
                nRes = (sal_Int16) p->nInt64;
            break;
        case SbxSALUINT64:
            if( p->uInt64 > SbxMAXINT )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXINT;
            }
            else
                nRes = (sal_Int16) p->uInt64;
            break;
        case SbxDATE:
        case SbxDOUBLE:
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
        {
            double dVal = 0.0;
            if( p->eType == SbxDECIMAL )
            {
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
                nRes = (sal_Int16) ImpRound( dVal );
            break;
            }
        case SbxLPSTR:
        case SbxSTRING:
        case SbxBYREF | SbxSTRING:
            if( !p->pOUString )
                nRes = 0;
            else
            {
                double d;
                SbxDataType t;
                if( ImpScan( *p->pOUString, d, t, NULL ) != SbxERR_OK )
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
                    nRes = (sal_Int16) ImpRound( d );
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

        // from here had to be tested
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

void ImpPutInteger( SbxValues* p, sal_Int16 n )
{
    SbxValues aTmp;
start:
    switch( +p->eType )
    {
        // here had to be tested
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

        // from here no tests needed
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
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setInt( n );
            break;

        case SbxLPSTR:
        case SbxSTRING:
        case SbxBYREF | SbxSTRING:
            if( !p->pOUString )
                p->pOUString = new ::rtl::OUString;
            ImpCvtNum( (double) n, 0, *p->pOUString );
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
            *p->pByte = (sal_uInt8) n; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            *p->pInteger = n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pUShort = (sal_uInt16) n; break;
        case SbxBYREF | SbxLONG:
            *p->pLong = (sal_Int32) n; break;
        case SbxBYREF | SbxULONG:
            if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pULong = (sal_uInt32) n; break;
        case SbxBYREF | SbxCURRENCY:
            *p->pnInt64 = n * CURRENCY_FACTOR; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW );
                *p->puInt64 = 0;
            }
            else
                *p->puInt64 = n;
            break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = (float) n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = (double) n; break;

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
            nRes = (sal_Int64) p->nUShort; break;
        case SbxLONG:
            nRes = (sal_Int64) p->nLong; break;
        case SbxULONG:
            nRes = (sal_Int64) p->nULong; break;
        case SbxSINGLE:
            nRes = (sal_Int64) p->nSingle;
            break;
        case SbxDATE:
        case SbxDOUBLE:
            nRes = (sal_Int64) p->nDouble;
            break;
        case SbxCURRENCY:
            nRes = p->nInt64 / CURRENCY_FACTOR; break;
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
            if( !p->pOUString )
                nRes = 0;
            else
            {
                   ::rtl::OString aOStr = ::rtl::OUStringToOString
                    ( *p->pOUString, RTL_TEXTENCODING_ASCII_US );
                nRes = aOStr.toInt64();
                if( nRes == 0 )
                {
                    // Check if really 0 or invalid conversion
                    double d;
                    SbxDataType t;
                    if( ImpScan( *p->pOUString, d, t, NULL ) != SbxERR_OK )
                        nRes = 0;
                    else
                        nRes = (sal_Int64) d;
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
        case SbxBYREF | SbxCURRENCY:
            nRes = p->nInt64 / CURRENCY_FACTOR; break;
        case SbxBYREF | SbxSALINT64:
            nRes = *p->pnInt64; break;

        // from here the values has to be checked
        case SbxBYREF | SbxSALUINT64:
            aTmp.uInt64 = *p->puInt64; goto ref;

        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            aTmp.nUShort = *p->pUShort; goto ref;
        case SbxBYREF | SbxSINGLE:
            aTmp.nSingle = *p->pSingle; goto ref;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            aTmp.nDouble = *p->pDouble; goto ref;
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
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        case SbxLONG:
            aTmp.pnInt64 = &p->nInt64; goto direct;
        case SbxCURRENCY:
        case SbxSALINT64:
            aTmp.pnInt64 = &p->nInt64; goto direct;
        case SbxSALUINT64:
            aTmp.puInt64 = &p->uInt64; goto direct;

        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        case SbxSINGLE:
            p->nSingle = (float) n; break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = (double) n; break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
        {
            if( !p->pOUString )
                p->pOUString = new ::rtl::OUString;

            ::rtl::OString  aOStr  = ::rtl::OString::valueOf( n );
               (*p->pOUString) = ::rtl::OStringToOUString
                ( aOStr, RTL_TEXTENCODING_ASCII_US );
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
            if( n > SbxMAXLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXLNG;
            }
            else if( n < SbxMINLNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINLNG;
            }
            *p->pLong = (sal_Int32) n; break;
        case SbxBYREF | SbxULONG:
            if( n > SbxMAXULNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXULNG;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = 0;
            }
            *p->pULong = (sal_uInt32) n; break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = (float) n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = (double) n; break;
        case SbxBYREF | SbxCURRENCY:
            *p->pnInt64 = n * CURRENCY_FACTOR; break;
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
            nRes = (sal_uInt64) p->nSingle; break;
        case SbxDATE:
        case SbxDOUBLE:
            {
//TODO overflow check
            nRes = (sal_uInt64) p->nDouble;
            break;
            }
        case SbxCURRENCY:
            nRes = p->nInt64 * CURRENCY_FACTOR; break;
        case SbxSALINT64:
            if( p->nInt64 < 0 )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); nRes = 0;
            }
            else
                nRes = (sal_uInt64) p->nInt64;
            break;
        case SbxSALUINT64:
            nRes = p->uInt64; break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pOUString )
                nRes = 0;
            else
            {
                   ::rtl::OString aOStr = ::rtl::OUStringToOString
                    ( *p->pOUString, RTL_TEXTENCODING_ASCII_US );
                sal_Int64 n64 = aOStr.toInt64();
                if( n64 == 0 )
                {
                    // Check if really 0 or invalid conversion
                    double d;
                    SbxDataType t;
                    if( ImpScan( *p->pOUString, d, t, NULL ) != SbxERR_OK )
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

        // from here on the value has to be checked
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
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        case SbxLONG:
            aTmp.pnInt64 = &p->nInt64; goto direct;
        case SbxCURRENCY:
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
            if( !p->pOUString )
                p->pOUString = new ::rtl::OUString;
            if( n > SbxMAXSALINT64 )
                SbxBase::SetError( SbxERR_CONVERSION );
            else
            {
                ::rtl::OString  aOStr  = ::rtl::OString::valueOf( (sal_Int64)n );
                   (*p->pOUString) = ::rtl::OStringToOUString
                    ( aOStr, RTL_TEXTENCODING_ASCII_US );
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
            if( n > SbxMAXULNG )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXULNG;
            }
            *p->pULong = (sal_uInt32) n; break;
        case SbxBYREF | SbxSINGLE:
            *p->pDouble = (float)ImpSalUInt64ToDouble( n ); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:

            *p->pDouble = ImpSalUInt64ToDouble( n ); break;
        case SbxBYREF | SbxCURRENCY:
            if ( n > ( SbxMAXSALINT64 / CURRENCY_FACTOR ) )
                 SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXSALINT64;
            *p->pnInt64 = ( sal_Int64) ( n * CURRENCY_FACTOR ); break;
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
