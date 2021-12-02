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
#include "sbxres.hxx"
#include <rtlproto.hxx>

enum SbxBOOL ImpGetBool( const SbxValues* p )
{
    enum SbxBOOL nRes;
    switch( +p->eType )
    {
        case SbxNULL:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
            [[fallthrough]];
        case SbxEMPTY:
            nRes = SbxFALSE; break;
        case SbxCHAR:
            nRes = p->nChar ? SbxTRUE : SbxFALSE; break;
        case SbxBYTE:
            nRes = p->nByte ? SbxTRUE : SbxFALSE; break;
        case SbxINTEGER:
        case SbxBOOL:
            nRes = p->nInteger ? SbxTRUE : SbxFALSE; break;
        case SbxERROR:
        case SbxUSHORT:
            nRes = p->nUShort ? SbxTRUE : SbxFALSE; break;
        case SbxLONG:
            nRes = p->nLong ? SbxTRUE : SbxFALSE; break;
        case SbxULONG:
            nRes = p->nULong ? SbxTRUE : SbxFALSE; break;
        case SbxSINGLE:
            nRes = p->nSingle ? SbxTRUE : SbxFALSE; break;
        case SbxDATE:
        case SbxDOUBLE:
            nRes = p->nDouble ? SbxTRUE : SbxFALSE; break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            {
            double dVal = 0.0;
            if( p->pDecimal )
                p->pDecimal->getDouble( dVal );
            nRes = dVal ? SbxTRUE : SbxFALSE;
            }
            break;
        case SbxSALINT64:
        case SbxCURRENCY:
            nRes = p->nInt64 ? SbxTRUE : SbxFALSE; break;
        case SbxSALUINT64:
            nRes = p->uInt64 ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            nRes = SbxFALSE;
            if ( p->pOUString )
            {
                if( p->pOUString->equalsIgnoreAsciiCase( GetSbxRes( StringId::True ) ) )
                    nRes = SbxTRUE;
                else if( !p->pOUString->equalsIgnoreAsciiCase( GetSbxRes( StringId::False ) ) )
                {
                    // it can be convertible to a number
                    bool bError = true;
                    double n;
                    SbxDataType t;
                    sal_uInt16 nLen = 0;
                    if( ImpScan( *p->pOUString, n, t, &nLen, !LibreOffice6FloatingPointMode() ) == ERRCODE_NONE )
                    {
                        if( nLen == p->pOUString->getLength() )
                        {
                            bError = false;
                            if( n != 0.0 )
                                nRes = SbxTRUE;
                        }
                    }
                    if( bError )
                        SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
                }
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                nRes = pVal->GetBool() ? SbxTRUE : SbxFALSE;
            else
            {
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT ); nRes = SbxFALSE;
            }
            break;
        }

        case SbxBYREF | SbxCHAR:
            nRes = *p->pChar ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxBYTE:
            nRes = *p->pByte ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            nRes = *p->pInteger ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxLONG:
            nRes = *p->pLong ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxULONG:
            nRes = *p->pULong ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            nRes = *p->pUShort ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxSINGLE:
            nRes = ( *p->pSingle != 0 ) ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            nRes = ( *p->pDouble != 0 ) ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxCURRENCY:
        case SbxBYREF | SbxSALINT64:
            nRes = ( *p->pnInt64 ) ? SbxTRUE : SbxFALSE; break;
        case SbxBYREF | SbxSALUINT64:
            nRes = ( *p->puInt64 ) ? SbxTRUE : SbxFALSE; break;
        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION ); nRes = SbxFALSE;
    }
    return nRes;
}

void ImpPutBool( SbxValues* p, sal_Int16 n )
{
    if( n )
        n = SbxTRUE;
    switch( +p->eType )
    {
        case SbxCHAR:
            p->nChar = static_cast<sal_Unicode>(n); break;
        case SbxUINT:
            p->nByte = static_cast<sal_uInt8>(n); break;
        case SbxINTEGER:
        case SbxBOOL:
            p->nInteger = n; break;
        case SbxLONG:
            p->nLong = n; break;
        case SbxULONG:
            p->nULong = static_cast<sal_uInt32>(n); break;
        case SbxERROR:
        case SbxUSHORT:
            p->nUShort = static_cast<sal_uInt16>(n); break;
        case SbxSINGLE:
            p->nSingle = n; break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;
        case SbxCURRENCY:
        case SbxSALINT64:
            p->nInt64 = static_cast<sal_Int64>(n); break;
        case SbxSALUINT64:
            p->uInt64 = static_cast<sal_uInt64>(n); break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpCreateDecimal( p )->setInt( n );
            break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if ( !p->pOUString )
                p->pOUString = new OUString( GetSbxRes( n ? StringId::True : StringId::False ) );
            else
                *p->pOUString = GetSbxRes( n ? StringId::True : StringId::False );
            break;

        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                pVal->PutBool( n != 0 );
            else
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            *p->pChar = static_cast<sal_Unicode>(n); break;
        case SbxBYREF | SbxBYTE:
            *p->pByte = static_cast<sal_uInt8>(n); break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            *p->pInteger = n; break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            *p->pUShort = static_cast<sal_uInt16>(n); break;
        case SbxBYREF | SbxLONG:
            *p->pLong = n; break;
        case SbxBYREF | SbxULONG:
            *p->pULong = static_cast<sal_uInt32>(n); break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = n; break;
        case SbxBYREF | SbxCURRENCY:
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = static_cast<sal_Int64>(n); break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = static_cast<sal_uInt64>(n); break;
        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
