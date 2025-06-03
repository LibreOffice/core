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

#include <comphelper/errcode.hxx>
#include <basic/sberrors.hxx>
#include "sbxconv.hxx"
#include "sbxres.hxx"

enum SbxBOOL ImpGetBool( const SbxValues* p )
{
    auto SbxBoolFrom = [](auto val) { return val != 0 ? SbxTRUE : SbxFALSE; };
    switch( +p->eType )
    {
        case SbxEMPTY:
            return SbxFALSE;
        case SbxCHAR:
            return SbxBoolFrom(p->nChar);
        case SbxBYREF | SbxCHAR:
            return SbxBoolFrom(*p->pChar);
        case SbxBYTE:
            return SbxBoolFrom(p->nByte);
        case SbxBYREF | SbxBYTE:
            return SbxBoolFrom(*p->pByte);
        case SbxINTEGER:
        case SbxBOOL:
            return SbxBoolFrom(p->nInteger);
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            return SbxBoolFrom(*p->pInteger);
        case SbxERROR:
        case SbxUSHORT:
            return SbxBoolFrom(p->nUShort);
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            return SbxBoolFrom(*p->pUShort);
        case SbxLONG:
            return SbxBoolFrom(p->nLong);
        case SbxBYREF | SbxLONG:
            return SbxBoolFrom(*p->pLong);
        case SbxULONG:
            return SbxBoolFrom(p->nULong);
        case SbxBYREF | SbxULONG:
            return SbxBoolFrom(*p->pULong);
        case SbxSINGLE:
            return SbxBoolFrom(p->nSingle);
        case SbxBYREF | SbxSINGLE:
            return SbxBoolFrom(*p->pSingle);
        case SbxDATE:
        case SbxDOUBLE:
            return SbxBoolFrom(p->nDouble);
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            return SbxBoolFrom(*p->pDouble);
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            if( p->pDecimal )
                return SbxBoolFrom(p->pDecimal->getWithOverflow<double>());
            return SbxFALSE;
        case SbxSALINT64:
        case SbxCURRENCY:
            return SbxBoolFrom(p->nInt64);
        case SbxBYREF | SbxSALINT64:
        case SbxBYREF | SbxCURRENCY:
            return SbxBoolFrom(*p->pnInt64);
        case SbxSALUINT64:
            return SbxBoolFrom(p->uInt64);
        case SbxBYREF | SbxSALUINT64:
            return SbxBoolFrom(*p->puInt64);

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if ( p->pOUString )
            {
                if( p->pOUString->equalsIgnoreAsciiCase( GetSbxRes( StringId::True ) ) )
                    return SbxTRUE;
                if( p->pOUString->equalsIgnoreAsciiCase( GetSbxRes( StringId::False ) ) )
                    return SbxFALSE;

                // it can be convertible to a number
                double n;
                SbxDataType t;
                sal_Int32 nLen = 0;
                if( ImpScan( *p->pOUString, n, t, &nLen ) == ERRCODE_NONE )
                {
                    if( nLen == p->pOUString->getLength() )
                    {
                        return SbxBoolFrom(n);
                    }
                }
                SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
            }
            return SbxFALSE;

        case SbxOBJECT:
            if (SbxValue* pVal = dynamic_cast<SbxValue*>(p->pObj))
                return SbxBoolFrom(pVal->GetBool());
            SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
            return SbxFALSE;

        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
            return SbxFALSE;
    }
}

void ImpPutBool( SbxValues* p, sal_Int16 n )
{
    if( n )
        n = SbxTRUE;
    switch( +p->eType )
    {
        case SbxCHAR:
            p->nChar = static_cast<sal_Unicode>(n); break;
        case SbxBYREF | SbxCHAR:
            *p->pChar = static_cast<sal_Unicode>(n); break;
        case SbxBYTE:
            p->nByte = static_cast<sal_uInt8>(n); break;
        case SbxBYREF | SbxBYTE:
            *p->pByte = static_cast<sal_uInt8>(n); break;
        case SbxINTEGER:
        case SbxBOOL:
            p->nInteger = n; break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            *p->pInteger = n; break;
        case SbxERROR:
        case SbxUSHORT:
            p->nUShort = static_cast<sal_uInt16>(n); break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            *p->pUShort = static_cast<sal_uInt16>(n); break;
        case SbxLONG:
            p->nLong = n; break;
        case SbxBYREF | SbxLONG:
            *p->pLong = n; break;
        case SbxULONG:
            p->nULong = static_cast<sal_uInt32>(n); break;
        case SbxBYREF | SbxULONG:
            *p->pULong = static_cast<sal_uInt32>(n); break;
        case SbxSINGLE:
            p->nSingle = n; break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = n; break;
        case SbxDATE:
        case SbxDOUBLE:
            p->nDouble = n; break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = n; break;
        case SbxCURRENCY:
        case SbxSALINT64:
            p->nInt64 = n; break;
        case SbxBYREF | SbxCURRENCY:
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = n; break;
        case SbxSALUINT64:
            p->uInt64 = static_cast<sal_uInt64>(n); break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = static_cast<sal_uInt64>(n); break;

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
            if (SbxValue* pVal = dynamic_cast<SbxValue*>(p->pObj))
                pVal->PutBool( n != 0 );
            else
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
            break;

        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
