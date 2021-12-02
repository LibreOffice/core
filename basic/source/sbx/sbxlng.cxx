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

#include <sal/config.h>

#include <o3tl/float_int_conversion.hxx>
#include <vcl/errcode.hxx>
#include <basic/sberrors.hxx>
#include "sbxconv.hxx"
#include <rtlproto.hxx>

#include <rtl/math.hxx>

sal_Int32 ImpGetLong( const SbxValues* p )
{
    SbxValues aTmp;
    sal_Int32 nRes;
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
            if( p->nULong > SbxMAXLNG )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXLNG;
            }
            else
                nRes = static_cast<sal_Int32>(p->nULong);
            break;
        case SbxSINGLE:
            nRes = ImpDoubleToLong(p->nSingle);
            break;
        case SbxSALINT64:
            nRes = p->nInt64;
            break;
        case SbxSALUINT64:
            nRes = p->uInt64;
            break;
        case SbxCURRENCY:
        {
            sal_Int64  tstVal = p->nInt64 / CURRENCY_FACTOR;
            nRes = static_cast<sal_Int32>(tstVal);
            if( tstVal < SbxMINLNG || SbxMAXLNG < tstVal )  SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW );
            if( SbxMAXLNG < tstVal ) nRes = SbxMAXLNG;
            if( tstVal < SbxMINLNG ) nRes = SbxMINLNG;
            break;
        }
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

            nRes = ImpDoubleToLong(dVal);
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
                if( ImpScan( *p->pOUString, d, t, nullptr, !LibreOffice6FloatingPointMode() ) != ERRCODE_NONE )
                    nRes = 0;
                else
                    nRes = ImpDoubleToLong(d);
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                nRes = pVal->GetLong();
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

        // from here had to be tested
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

void ImpPutLong( SbxValues* p, sal_Int32 n )
{
    SbxValues aTmp;

start:
    switch( +p->eType )
    {
        // From here had to be tested
        case SbxCHAR:
            aTmp.pChar = &p->nChar; goto direct;
        case SbxBYTE:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxINTEGER:
        case SbxBOOL:
            aTmp.pInteger = &p->nInteger; goto direct;
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxSALUINT64:
            aTmp.puInt64 = &p->uInt64; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort;
        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        // from here no longer
        case SbxLONG:
            p->nLong = n; break;
        case SbxSINGLE:
            p->nSingle = static_cast<float>(n); break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;
        case SbxCURRENCY:
            p->nInt64 = n * CURRENCY_FACTOR; break;
        case SbxSALINT64:
            p->nInt64 = n; break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setLong( n );
            break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !p->pOUString )
                p->pOUString = new OUString;
            ImpCvtNum( static_cast<double>(n), 0, *p->pOUString );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                pVal->PutLong( n );
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
            *p->pLong = n; break;
        case SbxBYREF | SbxULONG:
            if( n < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); n = 0;
            }
            *p->pULong = static_cast<sal_uInt32>(n); break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            if( n < 0 )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); *p->puInt64 = 0;
            }
            else
                *p->puInt64 = n;
            break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = static_cast<float>(n); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = static_cast<double>(n); break;
        case SbxBYREF | SbxCURRENCY:
            *p->pnInt64 = static_cast<sal_Int64>(n) * sal_Int64(CURRENCY_FACTOR); break;
        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
