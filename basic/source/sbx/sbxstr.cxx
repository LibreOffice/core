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
#include "sbxres.hxx"
#include "runtime.hxx"
#include <rtl/ustrbuf.hxx>

// The conversion of an item onto String was handled via the Put-Methods
// of the several data types to avoid duplicated code.

OUString ImpGetString( const SbxValues* p )
{
    SbxValues aTmp;
    OUString aRes;
    aTmp.eType = SbxSTRING;
    aTmp.pOUString = &aRes;
    switch( +p->eType )
    {
        case SbxNULL:
            SbxBase::SetError( SbxERR_CONVERSION );
        case SbxEMPTY:
            break;
        case SbxCHAR:
            ImpPutChar( &aTmp, p->nChar ); break;
        case SbxBYTE:
            ImpPutByte( &aTmp, p->nByte ); break;
        case SbxINTEGER:
            ImpPutInteger( &aTmp, p->nInteger ); break;
        case SbxBOOL:
            ImpPutBool( &aTmp, p->nUShort ); break;
        case SbxUSHORT:
            ImpPutUShort( &aTmp, p->nUShort ); break;
        case SbxLONG:
            ImpPutLong( &aTmp, p->nLong ); break;
        case SbxULONG:
            ImpPutULong( &aTmp, p->nULong ); break;
        case SbxSINGLE:
            ImpPutSingle( &aTmp, p->nSingle ); break;
        case SbxDOUBLE:
            ImpPutDouble( &aTmp, p->nDouble ); break;
        case SbxCURRENCY:
            ImpPutCurrency( &aTmp, p->nInt64 ); break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            ImpPutDecimal( &aTmp, p->pDecimal ); break;
        case SbxSALINT64:
            ImpPutInt64( &aTmp, p->nInt64 ); break;
        case SbxSALUINT64:
            ImpPutUInt64( &aTmp, p->uInt64 ); break;
        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if ( p->pOUString )
            {
                *aTmp.pOUString = *p->pOUString;
            }
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
            {
                aRes = pVal->GetOUString();
            }
            else if( p->pObj && p->pObj->IsFixed()
                    && (p->pObj->GetType() == (SbxARRAY | SbxBYTE )) )
            {
                // convert byte array to string
                SbxArray* pArr = PTR_CAST(SbxArray, p->pObj);
                if( pArr )
                {
                    aRes = ByteArrayToString( pArr );
                }
            }
            else
            {
                SbxBase::SetError( SbxERR_NO_OBJECT );
            }
            break;
        }
        case SbxERROR:
            // Here the String "Error n" is generated
            aRes = SbxRes( STRING_ERRORMSG );
            aRes += OUString( p->nUShort ); break;
        case SbxDATE:
            ImpPutDate( &aTmp, p->nDouble ); break;

        case SbxBYREF | SbxCHAR:
            ImpPutChar( &aTmp, *p->pChar ); break;
        case SbxBYREF | SbxBYTE:
            ImpPutByte( &aTmp, *p->pByte ); break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            ImpPutInteger( &aTmp, *p->pInteger ); break;
        case SbxBYREF | SbxLONG:
            ImpPutLong( &aTmp, *p->pLong ); break;
        case SbxBYREF | SbxULONG:
            ImpPutULong( &aTmp, *p->pULong ); break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            ImpPutUShort( &aTmp, *p->pUShort ); break;
        case SbxBYREF | SbxSINGLE:
            ImpPutSingle( &aTmp, *p->pSingle ); break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            ImpPutDouble( &aTmp, *p->pDouble ); break;
        case SbxBYREF | SbxCURRENCY:
            ImpPutCurrency( &aTmp, *p->pnInt64 ); break;
        case SbxBYREF | SbxSALINT64:
            ImpPutInt64( &aTmp, *p->pnInt64 ); break;
        case SbxBYREF | SbxSALUINT64:
            ImpPutUInt64( &aTmp, *p->puInt64 ); break;
        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
    return aRes;
}

// From 1997-04-10, new function for SbxValue::GetCoreString()
OUString ImpGetCoreString( const SbxValues* p )
{
    // For now only for double
    if( ( p->eType & (~SbxBYREF) ) == SbxDOUBLE )
    {
        SbxValues aTmp;
        OUString aRes;
        aTmp.eType = SbxSTRING;
        aTmp.pOUString = &aRes;
        if( p->eType == SbxDOUBLE )
            ImpPutDouble( &aTmp, p->nDouble, true );    // true = bCoreString
        else
            ImpPutDouble( &aTmp, *p->pDouble, true );   // true = bCoreString
        return aRes;
    }
    else
        return ImpGetString( p );
}

void ImpPutString( SbxValues* p, const OUString* n )
{
    SbxValues aTmp;
    aTmp.eType = SbxSTRING;
    OUString* pTmp = NULL;
    // as a precaution, if a NULL-Ptr appears
    if( !n )
        n = pTmp = new OUString;
    aTmp.pOUString = (OUString*)n;
    switch( +p->eType )
    {
        case SbxCHAR:
            p->nChar = ImpGetChar( &aTmp ); break;
        case SbxBYTE:
            p->nByte = ImpGetByte( &aTmp ); break;
        case SbxINTEGER:
        case SbxBOOL:
            p->nInteger = ImpGetInteger( &aTmp ); break;
        case SbxLONG:
            p->nLong = ImpGetLong( &aTmp ); break;
        case SbxULONG:
            p->nULong = ImpGetULong( &aTmp ); break;
        case SbxERROR:
        case SbxUSHORT:
            p->nUShort = ImpGetUShort( &aTmp ); break;
        case SbxSINGLE:
            p->nSingle = ImpGetSingle( &aTmp ); break;
        case SbxDATE:
            p->nDouble = ImpGetDate( &aTmp ); break;
        case SbxDOUBLE:
            p->nDouble = ImpGetDouble( &aTmp ); break;
        case SbxCURRENCY:
            p->nInt64 = ImpGetCurrency( &aTmp ); break;
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            releaseDecimalPtr( p->pDecimal );
            p->pDecimal = ImpGetDecimal( &aTmp ); break;
        case SbxSALINT64:
            p->nInt64 = ImpGetInt64( &aTmp ); break;
        case SbxSALUINT64:
            p->uInt64 = ImpGetUInt64( &aTmp ); break;

        case SbxBYREF | SbxSTRING:
        case SbxSTRING:
        case SbxLPSTR:
            if( !n->isEmpty() )
            {
                if( !p->pOUString )
                    p->pOUString = new OUString( *n );
                else
                    *p->pOUString = *n;
            }
            else
                delete p->pOUString, p->pOUString = NULL;
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
            if( pVal )
                pVal->PutString( *n );
            else
                SbxBase::SetError( SbxERR_NO_OBJECT );
            break;
        }
        case SbxBYREF | SbxCHAR:
            *p->pChar = ImpGetChar( p ); break;
        case SbxBYREF | SbxBYTE:
            *p->pByte = ImpGetByte( p ); break;
        case SbxBYREF | SbxINTEGER:
            *p->pInteger = ImpGetInteger( p ); break;
        case SbxBYREF | SbxBOOL:
            *p->pUShort = sal::static_int_cast< sal_uInt16 >( ImpGetBool( p ) );
            break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            *p->pUShort = ImpGetUShort( p ); break;
        case SbxBYREF | SbxLONG:
            *p->pLong = ImpGetLong( p ); break;
        case SbxBYREF | SbxULONG:
            *p->pULong = ImpGetULong( p ); break;
        case SbxBYREF | SbxSINGLE:
            *p->pSingle = ImpGetSingle( p ); break;
        case SbxBYREF | SbxDATE:
            *p->pDouble = ImpGetDate( p ); break;
        case SbxBYREF | SbxDOUBLE:
            *p->pDouble = ImpGetDouble( p ); break;
        case SbxBYREF | SbxCURRENCY:
            *p->pnInt64 = ImpGetCurrency( p ); break;
        case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = ImpGetInt64( p ); break;
        case SbxBYREF | SbxSALUINT64:
            *p->puInt64 = ImpGetUInt64( p ); break;
        default:
            SbxBase::SetError( SbxERR_CONVERSION );
    }
    delete pTmp;
}


// Convert string to an array of bytes, preserving unicode (2bytes per character)
SbxArray* StringToByteArray(const OUString& rStr)
{
    sal_Int32 nArraySize = rStr.getLength() * 2;
    const sal_Unicode* pSrc = rStr.getStr();
    SbxDimArray* pArray = new SbxDimArray(SbxBYTE);
#ifdef DISABLE_SCRIPTING
    bool bIncIndex = false;
#else
    bool bIncIndex = ( IsBaseIndexOne() && SbiRuntime::isVBAEnabled() );
#endif
    if( nArraySize )
    {
        if( bIncIndex )
            pArray->AddDim32( 1, nArraySize );
        else
            pArray->AddDim32( 0, nArraySize-1 );
    }
    else
    {
        pArray->unoAddDim( 0, -1 );
    }

    for( sal_uInt16 i=0; i< nArraySize; i++)
    {
        SbxVariable* pNew = new SbxVariable( SbxBYTE );
        sal_uInt8 aByte = static_cast< sal_uInt8 >( (i%2) ? ((*pSrc) >> 8) & 0xff : (*pSrc) & 0xff );
        pNew->PutByte( aByte );
        pNew->SetFlag( SBX_WRITE );
        pArray->Put( pNew, i );
        if( i%2 )
            pSrc++;
    }
    return pArray;
}

// Convert an array of bytes to string (2bytes per character)
OUString ByteArrayToString(SbxArray* pArr)
{
    sal_uInt16 nCount = pArr->Count();
    OUStringBuffer aStrBuf;
    sal_Unicode aChar = 0;
    for( sal_uInt16 i = 0 ; i < nCount ; i++ )
    {
        sal_Unicode aTempChar = pArr->Get(i)->GetByte();
        if( i%2 )
        {
            aChar = (aTempChar << 8 ) | aChar;
            aStrBuf.append(aChar);
            aChar = 0;
        }
        else
        {
            aChar = aTempChar;
        }
    }

    if( nCount%2 )
    {
        aStrBuf.append(aChar);
    }

    return aStrBuf.makeStringAndClear();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
