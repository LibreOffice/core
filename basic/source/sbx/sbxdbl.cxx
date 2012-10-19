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
#include "runtime.hxx"

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
            nRes = ImpCurrencyToDouble( p->nInt64 ); break;
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
            if( !p->pOUString )
            {
                nRes = 0;
#ifndef DISABLE_SCRIPTING
                if ( SbiRuntime::isVBAEnabled() )// VBA only behaviour
                    SbxBase::SetError( SbxERR_CONVERSION );
#endif
            }
            else
            {
                double d;
                SbxDataType t;
                if( ImpScan( *p->pOUString, d, t, NULL ) != SbxERR_OK )
                {
                    nRes = 0;
#ifndef DISABLE_SCRIPTING
                    if ( SbiRuntime::isVBAEnabled() )// VBA only behaviour
                        SbxBase::SetError( SbxERR_CONVERSION );
#endif
                }
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
            nRes = ImpCurrencyToDouble( *p->pnInt64 ); break;
        case SbxBYREF | SbxSALINT64:
            nRes = static_cast< double >(*p->pnInt64); break;
        case SbxBYREF | SbxSALUINT64:
            nRes = ImpSalUInt64ToDouble( *p->puInt64 ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION ); nRes = 0;
    }
    return nRes;
}

void ImpPutDouble( SbxValues* p, double n, bool bCoreString )
{
    SbxValues aTmp;
start:
    switch( +p->eType )
    {
        // Here are tests necessary
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

        case SbxCURRENCY:
            if( n > SbxMAXCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMAXCURR;
            }
            else if( n < SbxMINCURR )
            {
                SbxBase::SetError( SbxERR_OVERFLOW ); n = SbxMINCURR;
            }
            p->nInt64 = ImpDoubleToCurrency( n );
            break;

            // from here on no longer
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
            if( !p->pOUString )
                p->pOUString = new ::rtl::OUString;
            ImpCvtNum( (double) n, 14, *p->pOUString, bCoreString );
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
            *p->pnInt64 = ImpDoubleToCurrency( n ); break;

        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
