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
#include <basic/sberrors.hxx>
#include "sbxconv.hxx"
#include <rtlproto.hxx>

#include <rtl/math.hxx>

sal_Unicode ImpGetChar( const SbxValues* p )
{
    SbxValues aTmp;
    sal_Unicode nRes = 0;
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
            nRes = static_cast<sal_Unicode>(p->nByte);
            break;
        case SbxINTEGER:
        case SbxBOOL:
            if( p->nInteger < SbxMINCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMINCHAR;
            }
            else
                nRes = static_cast<sal_Unicode>(p->nInteger);
            break;
        case SbxERROR:
        case SbxUSHORT:
            nRes = static_cast<sal_Unicode>(p->nUShort);
            break;
        case SbxLONG:
            if( p->nLong > SbxMAXCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else if( p->nLong < SbxMINCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMINCHAR;
            }
            else
                nRes = static_cast<sal_Unicode>(p->nLong);
            break;
        case SbxULONG:
            if( p->nULong > SbxMAXCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else
                nRes = static_cast<sal_Unicode>(p->nULong);
            break;
        case SbxCURRENCY:
        case SbxSALINT64:
        {
            sal_Int64 val = p->nInt64;

            if ( p->eType == SbxCURRENCY )
                val = val / CURRENCY_FACTOR;

            if( val > SbxMAXCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else if( p->nInt64 < SbxMINCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMINCHAR;
            }
            else
                nRes = static_cast<sal_Unicode>(val);
            break;
        }
        case SbxSALUINT64:
            if( p->uInt64 > SbxMAXCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else
                nRes = static_cast<sal_Unicode>(p->uInt64);
            break;
        case SbxSINGLE:
            if( p->nSingle > SbxMAXCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else if( p->nSingle < SbxMINCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMINCHAR;
            }
            else
                nRes = static_cast<sal_Unicode>(rtl::math::round( p->nSingle ));
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
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXCHAR;
            }
            else if( dVal < SbxMINCHAR )
            {
                SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMINCHAR;
            }
            else
                nRes = static_cast<sal_uInt8>(rtl::math::round( dVal ));
            break;
            }
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if ( p->pOUString )
            {
                double d;
                SbxDataType t;
                if( ImpScan( *p->pOUString, d, t, nullptr, !LibreOffice6FloatingPointMode() ) != ERRCODE_NONE )
                    nRes = 0;
                else if( d > SbxMAXCHAR )
                {
                    SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMAXCHAR;
                }
                else if( d < SbxMINCHAR )
                {
                    SbxBase::SetError( ERRCODE_BASIC_MATH_OVERFLOW ); nRes = SbxMINCHAR;
                }
                else
                    nRes = static_cast<sal_Unicode>(rtl::math::round( d ));
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                nRes = pVal->GetChar();
            else
            {
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT ); nRes = 0;
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
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION ); nRes = 0;
    }
    return nRes;
}

void ImpPutChar( SbxValues* p, sal_Unicode n )
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
                p->pOUString = new OUString( n );
            else
                *p->pOUString = OUString( n );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                pVal->PutChar( n );
            else
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            *p->pChar = n; break;
        case SbxBYREF | SbxBYTE:
            *p->pByte = static_cast<sal_uInt8>(n); break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            *p->pInteger = n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            *p->pUShort = static_cast<sal_uInt16>(n); break;
        case SbxBYREF | SbxLONG:
            *p->pLong = static_cast<sal_Int32>(n); break;
        case SbxBYREF | SbxULONG:
            *p->pULong = static_cast<sal_uInt32>(n); break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = static_cast<float>(n); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = static_cast<double>(n); break;
        case SbxBYREF | SbxCURRENCY:
            p->nInt64 = n * CURRENCY_FACTOR; break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = n; break;

        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
