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

#include <vcl/errcode.hxx>
#include <basic/sbx.hxx>
#include <basic/sberrors.hxx>
#include "sbxconv.hxx"

#include <rtl/math.hxx>

sal_Int16 ImpGetInteger( const SbxValues* p )
{
    SbxValues aTmp;
    sal_Int16 nRes;
start:
    switch( +p->eType )
    {
        case SbxNULL:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
            [[fallthrough]];
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
            if( p->nUShort > sal_uInt16(SbxMAXINT) )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXINT;
            }
            else
                nRes = static_cast<sal_Int16>(p->nUShort);
            break;
        case SbxLONG:
            if( p->nLong > SbxMAXINT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXINT;
            }
            else if( p->nLong < SbxMININT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMININT;
            }
            else
                nRes = static_cast<sal_Int16>(p->nLong);
            break;
        case SbxULONG:
            if( p->nULong > SbxMAXINT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXINT;
            }
            else
                nRes = static_cast<sal_Int16>(p->nULong);
            break;
        case SbxSINGLE:
            if( p->nSingle > SbxMAXINT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXINT;
            }
            else if( p->nSingle < SbxMININT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMININT;
            }
            else
                nRes = static_cast<sal_Int16>(rtl::math::round( p->nSingle ));
            break;
        case SbxCURRENCY:
            {
                sal_Int64 tstVal = p->nInt64 / sal_Int64(CURRENCY_FACTOR);

                if( tstVal > SbxMAXINT )
                {
                    SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXINT;
                }
                else if( tstVal  < SbxMININT )
                {
                    SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMININT;
                }
                else
                    nRes = static_cast<sal_Int16>(tstVal);
                break;
            }
        case SbxSALINT64:
            if( p->nInt64 > SbxMAXINT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXINT;
            }
            else if( p->nInt64 < SbxMININT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMININT;
            }
            else
                nRes = static_cast<sal_Int16>(p->nInt64);
            break;
        case SbxSALUINT64:
            if( p->uInt64 > SbxMAXINT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXINT;
            }
            else
                nRes = static_cast<sal_Int16>(p->uInt64);
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
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXINT;
            }
            else if( dVal < SbxMININT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMININT;
            }
            else
                nRes = static_cast<sal_Int16>(rtl::math::round( dVal ));
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
                if( ImpScan( *p->pOUString, d, t, nullptr, false ) != ERRCODE_NONE )
                    nRes = 0;
                else if( d > SbxMAXINT )
                {
                    SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXINT;
                }
                else if( d < SbxMININT )
                {
                    SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMININT;
                }
                else
                    nRes = static_cast<sal_Int16>(rtl::math::round( d ));
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                nRes = pVal->GetInteger();
            else
            {
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT ); nRes = 0;
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
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION ); nRes = 0;
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
                p->pOUString = new OUString;
            ImpCvtNum( static_cast<double>(n), 0, *p->pOUString );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                pVal->PutInteger( n );
            else
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            if( n < SbxMINCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMINCHAR;
            }
            *p->pChar = static_cast<char>(n); break;
        case SbxBYREF | SbxBYTE:
            if( n > SbxMAXBYTE )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXBYTE;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
            }
            *p->pByte = static_cast<sal_uInt8>(n); break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            *p->pInteger = n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            if( n < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
            }
            *p->pUShort = static_cast<sal_uInt16>(n); break;
        case SbxBYREF | SbxLONG:
            *p->pLong = static_cast<sal_Int32>(n); break;
        case SbxBYREF | SbxULONG:
            if( n < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
            }
            *p->pULong = static_cast<sal_uInt32>(n); break;
        case SbxBYREF | SbxCURRENCY:
            *p->pnInt64 = n * CURRENCY_FACTOR; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            if( n < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW );
                *p->puInt64 = 0;
            }
            else
                *p->puInt64 = n;
            break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = static_cast<float>(n); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = static_cast<double>(n); break;

        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
    }
}


// sal_Int64 / hyper

sal_Int64 ImpDoubleToSalInt64( double d )
{
    sal_Int64 nRes;
    if( d > SAL_MAX_INT64 )
    {
        SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SAL_MAX_INT64;
    }
    else if( d < SAL_MIN_INT64 )
    {
        SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SAL_MIN_INT64;
    }
    else
        nRes = static_cast<sal_Int64>(rtl::math::round( d ));
    return nRes;
}

sal_uInt64 ImpDoubleToSalUInt64( double d )
{
    sal_uInt64 nRes;
    if( d > SAL_MAX_UINT64 )
    {
        SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SAL_MAX_UINT64;
    }
    else if( d < 0.0 )
    {
        SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = 0;
    }
    else
        nRes = static_cast<sal_uInt64>(rtl::math::round( d ));
    return nRes;
}


double ImpSalUInt64ToDouble( sal_uInt64 n )
{
    double d = 0.0;
    if( n > SAL_MAX_INT64 )
        SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
    else
        d = static_cast<double>(static_cast<sal_Int64>(n));
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
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
            [[fallthrough]];
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
            nRes = static_cast<sal_Int64>(p->nUShort); break;
        case SbxLONG:
            nRes = static_cast<sal_Int64>(p->nLong); break;
        case SbxULONG:
            nRes = static_cast<sal_Int64>(p->nULong); break;
        case SbxSINGLE:
            nRes = static_cast<sal_Int64>(p->nSingle);
            break;
        case SbxDATE:
        case SbxDOUBLE:
            nRes = static_cast<sal_Int64>(p->nDouble);
            break;
        case SbxCURRENCY:
            nRes = p->nInt64 / CURRENCY_FACTOR; break;
        case SbxSALINT64:
            nRes = p->nInt64; break;
        case SbxSALUINT64:
            if( p->uInt64 > SAL_MAX_INT64 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SAL_MAX_INT64;
            }
            else
                nRes = static_cast<sal_Int64>(p->uInt64);
            break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pOUString )
                nRes = 0;
            else
            {
                ::OString aOStr = OUStringToOString( *p->pOUString, RTL_TEXTENCODING_ASCII_US );
                nRes = aOStr.toInt64();
                if( nRes == 0 )
                {
                    // Check if really 0 or invalid conversion
                    double d;
                    SbxDataType t;
                    if( ImpScan( *p->pOUString, d, t, nullptr, false ) != ERRCODE_NONE )
                        nRes = 0;
                    else
                        nRes = static_cast<sal_Int64>(d);
                }
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                nRes = pVal->GetInt64();
            else
            {
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT ); nRes = 0;
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
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION ); nRes = 0;
    }
    return nRes;
}

void ImpPutInt64( SbxValues* p, sal_Int64 n )
{
    SbxValues aTmp;

start:
    switch( +p->eType )
    {
        // Check necessary
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
            p->nSingle = static_cast<float>(n); break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = static_cast<double>(n); break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
        {
            if( !p->pOUString )
                p->pOUString = new OUString;

            ::OString  aOStr  = OString::number( n );
            (*p->pOUString) = ::OStringToOUString( aOStr, RTL_TEXTENCODING_ASCII_US );
            break;
        }
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                pVal->PutInt64( n );
            else
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            if( n > SbxMAXCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXCHAR;
            }
            else if( n < SbxMINCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMINCHAR;
            }
            *p->pChar = static_cast<sal_Unicode>(n); break;
        case SbxBYREF | SbxBYTE:
            if( n > SbxMAXBYTE )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXBYTE;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
            }
            *p->pByte = static_cast<sal_uInt8>(n); break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            if( n > SbxMAXINT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXINT;
            }
            else if( n < SbxMININT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMININT;
            }
            *p->pInteger = static_cast<sal_Int16>(n); break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            if( n > SbxMAXUINT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXUINT;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
            }
            *p->pUShort = static_cast<sal_uInt16>(n); break;
        case SbxBYREF | SbxLONG:
            if( n > SbxMAXLNG )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXLNG;
            }
            else if( n < SbxMINLNG )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMINLNG;
            }
            *p->pLong = static_cast<sal_Int32>(n); break;
        case SbxBYREF | SbxULONG:
            if( n > SbxMAXULNG )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXULNG;
            }
            else if( n < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
            }
            *p->pULong = static_cast<sal_uInt32>(n); break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = static_cast<float>(n); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = static_cast<double>(n); break;
        case SbxBYREF | SbxCURRENCY:
            *p->pnInt64 = n * CURRENCY_FACTOR; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            if( n < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
            }
            *p->puInt64 = n; break;

        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
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
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
            [[fallthrough]];
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
            nRes = static_cast<sal_uInt64>(p->nULong); break;
        case SbxSINGLE:
            nRes = static_cast<sal_uInt64>(p->nSingle); break;
        case SbxDATE:
        case SbxDOUBLE:
            {
//TODO overflow check
            nRes = static_cast<sal_uInt64>(p->nDouble);
            break;
            }
        case SbxCURRENCY:
            nRes = p->nInt64 * CURRENCY_FACTOR; break;
        case SbxSALINT64:
            if( p->nInt64 < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = 0;
            }
            else
                nRes = static_cast<sal_uInt64>(p->nInt64);
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
                ::OString aOStr = OUStringToOString
                    ( *p->pOUString, RTL_TEXTENCODING_ASCII_US );
                sal_Int64 n64 = aOStr.toInt64();
                if( n64 == 0 )
                {
                    // Check if really 0 or invalid conversion
                    double d;
                    SbxDataType t;
                    if( ImpScan( *p->pOUString, d, t, nullptr, false ) != ERRCODE_NONE )
                        nRes = 0;
                    else if( d > SAL_MAX_UINT64 )
                    {
                        SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SAL_MAX_UINT64;
                    }
                    else if( d < 0.0 )
                    {
                        SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = 0;
                    }
                    else
                        nRes = static_cast<sal_uInt64>(rtl::math::round( d ));
                }
                else if( n64 < 0 )
                {
                    SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = 0;
                }
                else
                {
                    nRes = n64;
                }
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                nRes = pVal->GetUInt64();
            else
            {
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT ); nRes = 0;
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
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION ); nRes = 0;
    }
    return nRes;
}

void ImpPutUInt64( SbxValues* p, sal_uInt64 n )
{
    SbxValues aTmp;

start:
    switch( +p->eType )
    {
        // Check necessary
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

        // Check not necessary
        case SbxSALUINT64:
            p->uInt64 = n; break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pOUString )
                p->pOUString = new OUString;
            if( n > SAL_MAX_INT64 )
                SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
            else
            {
                ::OString  aOStr  = OString::number( n );
                (*p->pOUString) = ::OStringToOUString( aOStr, RTL_TEXTENCODING_ASCII_US );
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                pVal->PutUInt64( n );
            else
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            if( n > SbxMAXCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXCHAR;
            }
            *p->pChar = static_cast<sal_Unicode>(n); break;
        case SbxBYREF | SbxBYTE:
            if( n > SbxMAXBYTE )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXBYTE;
            }
            *p->pByte = static_cast<sal_uInt8>(n); break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            if( n > SbxMAXINT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXINT;
            }
            *p->pInteger = static_cast<sal_Int16>(n); break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            if( n > SbxMAXUINT )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXUINT;
            }
            *p->pUShort = static_cast<sal_uInt16>(n); break;
        case SbxBYREF | SbxLONG:
            if( n > SbxMAXLNG )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXLNG;
            }
            *p->pLong = static_cast<sal_Int32>(n); break;
        case SbxBYREF | SbxULONG:
            if( n > SbxMAXULNG )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = SbxMAXULNG;
            }
            *p->pULong = static_cast<sal_uInt32>(n); break;
        case SbxBYREF | SbxSINGLE:
            *p->pDouble = static_cast<float>(ImpSalUInt64ToDouble( n )); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:

            *p->pDouble = ImpSalUInt64ToDouble( n ); break;
        case SbxBYREF | SbxCURRENCY:
            if ( n > ( SAL_MAX_INT64 / CURRENCY_FACTOR ) )
            {
                 SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW );
                 n = SAL_MAX_INT64;
            }
            *p->pnInt64 = static_cast<sal_Int64>( n * CURRENCY_FACTOR ); break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = n; break;
        case SbxBYREF | SbxSALINT64:
            if( n > SAL_MAX_INT64 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
            }
            *p->pnInt64 = static_cast<sal_Int64>(n); break;

        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
