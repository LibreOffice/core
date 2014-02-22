/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <math.h>
#include <tools/stream.hxx>

#include <basic/sbx.hxx>
#include "sbxconv.hxx"
#include "runtime.hxx"

TYPEINIT1(SbxValue,SbxBase)



SbxValue::SbxValue() : SbxBase()
{
    aData.eType = SbxEMPTY;
}

SbxValue::SbxValue( SbxDataType t, void* p ) : SbxBase()
{
    int n = t & 0x0FFF;
    if( p )
        n |= SbxBYREF;
    if( n == SbxVARIANT )
        n = SbxEMPTY;
    else
        SetFlag( SBX_FIXED );
    if( p )
    {
        switch( t & 0x0FFF )
        {
            case SbxINTEGER:    n |= SbxBYREF; aData.pInteger = (sal_Int16*) p; break;
            case SbxSALUINT64:  n |= SbxBYREF; aData.puInt64 = (sal_uInt64*) p; break;
            case SbxSALINT64:
            case SbxCURRENCY:   n |= SbxBYREF; aData.pnInt64 = (sal_Int64*) p; break;
            case SbxLONG:       n |= SbxBYREF; aData.pLong = (sal_Int32*) p; break;
            case SbxSINGLE:     n |= SbxBYREF; aData.pSingle = (float*) p; break;
            case SbxDATE:
            case SbxDOUBLE:     n |= SbxBYREF; aData.pDouble = (double*) p; break;
            case SbxSTRING:     n |= SbxBYREF; aData.pOUString = (OUString*) p; break;
            case SbxERROR:
            case SbxUSHORT:
            case SbxBOOL:       n |= SbxBYREF; aData.pUShort = (sal_uInt16*) p; break;
            case SbxULONG:      n |= SbxBYREF; aData.pULong = (sal_uInt32*) p; break;
            case SbxCHAR:       n |= SbxBYREF; aData.pChar = (sal_Unicode*) p; break;
            case SbxBYTE:       n |= SbxBYREF; aData.pByte = (sal_uInt8*) p; break;
            case SbxINT:        n |= SbxBYREF; aData.pInt = (int*) p; break;
            case SbxOBJECT:
                aData.pObj = (SbxBase*) p;
                if( p )
                    aData.pObj->AddRef();
                break;
            case SbxDECIMAL:
                aData.pDecimal = (SbxDecimal*) p;
                if( p )
                    aData.pDecimal->addRef();
                break;
            default:
                DBG_ASSERT( !this, "Improper pointer argument" );
                n = SbxNULL;
        }
    }
    else
        memset( &aData, 0, sizeof( SbxValues ) );
    aData.eType = SbxDataType( n );
}

SbxValue::SbxValue( const SbxValue& r )
    : SvRefBase( r ), SbxBase( r )
{
    if( !r.CanRead() )
    {
        SetError( SbxERR_PROP_WRITEONLY );
        if( !IsFixed() )
            aData.eType = SbxNULL;
    }
    else
    {
        ((SbxValue*) &r)->Broadcast( SBX_HINT_DATAWANTED );
        aData = r.aData;
        
        switch( aData.eType )
        {
            case SbxSTRING:
                if( aData.pOUString )
                    aData.pOUString = new OUString( *aData.pOUString );
                break;
            case SbxOBJECT:
                if( aData.pObj )
                    aData.pObj->AddRef();
                break;
            case SbxDECIMAL:
                if( aData.pDecimal )
                    aData.pDecimal->addRef();
                break;
            default: break;
        }
    }
}

SbxValue& SbxValue::operator=( const SbxValue& r )
{
    if( &r != this )
    {
        if( !CanWrite() )
            SetError( SbxERR_PROP_READONLY );
        else
        {
            
            if( IsFixed() && (aData.eType == SbxOBJECT)
                && aData.pObj && ( aData.pObj->GetType() == (SbxARRAY | SbxBYTE) )
                && (r.aData.eType == SbxSTRING) )
            {
                OUString aStr = r.GetOUString();
                SbxArray* pArr = StringToByteArray(aStr);
                PutObject(pArr);
                return *this;
            }
            
            if( r.IsFixed() && (r.aData.eType == SbxOBJECT)
                && r.aData.pObj && ( r.aData.pObj->GetType() == (SbxARRAY | SbxBYTE) )
                && (aData.eType == SbxSTRING) )
            {
                SbxBase* pObj = r.GetObject();
                SbxArray* pArr = PTR_CAST(SbxArray, pObj);
                if( pArr )
                {
                    OUString aStr = ByteArrayToString( pArr );
                    PutString(aStr);
                    return *this;
                }
            }
            
            SbxValues aNew;
            if( IsFixed() )
                
                aNew.eType = aData.eType;
            else if( r.IsFixed() )
                
                aNew.eType = SbxDataType( r.aData.eType & 0x0FFF );
            else
                
                aNew.eType = SbxVARIANT;
            if( r.Get( aNew ) )
                Put( aNew );
        }
    }
    return *this;
}

SbxValue::~SbxValue()
{
    Broadcast( SBX_HINT_DYING );
    SetFlag( SBX_WRITE );
    SbxValue::Clear();
}

void SbxValue::Clear()
{
    switch( aData.eType )
    {
        case SbxNULL:
        case SbxEMPTY:
        case SbxVOID:
            break;
        case SbxSTRING:
            delete aData.pOUString; aData.pOUString = NULL;
            break;
        case SbxOBJECT:
            if( aData.pObj )
            {
                if( aData.pObj != this )
                {
                    SAL_WARN("basic.sbx", "Not at Parent-Prop - otherwise CyclicRef");
                    SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
                    bool bParentProp = pThisVar && 5345 ==
                    ( (sal_Int16) ( pThisVar->GetUserData() & 0xFFFF ) );
                    if ( !bParentProp )
                        aData.pObj->ReleaseRef();
                }
                aData.pObj = NULL;
            }
            break;
        case SbxDECIMAL:
            if( aData.eType == SbxDECIMAL )
                releaseDecimalPtr( aData.pDecimal );
            break;
        case SbxDATAOBJECT:
            aData.pData = NULL; break;
        default:
        {
            SbxValues aEmpty;
            memset( &aEmpty, 0, sizeof( SbxValues ) );
            aEmpty.eType = GetType();
            Put( aEmpty );
        }
    }
}



void SbxValue::Broadcast( sal_uIntPtr )
{}








SbxValue* SbxValue::TheRealValue() const
{
    return TheRealValue( sal_True );
}


bool handleToStringForCOMObjects( SbxObject* pObj, SbxValue* pVal );    

SbxValue* SbxValue::TheRealValue( sal_Bool bObjInObjError ) const
{
    SbxValue* p = (SbxValue*) this;
    for( ;; )
    {
        SbxDataType t = SbxDataType( p->aData.eType & 0x0FFF );
        if( t == SbxOBJECT )
        {
            
            SbxObject* pObj = PTR_CAST(SbxObject,p->aData.pObj);
            if( pObj )
            {
                
                SbxVariable* pDflt = pObj->GetDfltProperty();

                
                
                
                
                
                
                if( bObjInObjError && !pDflt &&
                    ((SbxValue*) pObj)->aData.eType == SbxOBJECT &&
                    ((SbxValue*) pObj)->aData.pObj == pObj )
                {
#ifdef DISABLE_SCRIPTING 
                    const bool bSuccess = false;
#else
                    bool bSuccess = handleToStringForCOMObjects( pObj, p );
#endif
                    if( !bSuccess )
                    {
                        SetError( SbxERR_BAD_PROP_VALUE );
                        p = NULL;
                    }
                }
                else if( pDflt )
                    p = pDflt;
                break;
            }
            
            SbxArray* pArray = PTR_CAST(SbxArray,p->aData.pObj);
            if( pArray )
            {
                
                SbxArray* pPar = NULL;
                SbxVariable* pVar = PTR_CAST(SbxVariable,p);
                if( pVar )
                    pPar = pVar->GetParameters();
                if( pPar )
                {
                    
                    SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,p->aData.pObj);
                    if( pDimArray )
                        p = pDimArray->Get( pPar );
                    else
                        p = pArray->Get( pPar->Get( 1 )->GetInteger() );
                    break;
                }
            }
            
            SbxValue* pVal = PTR_CAST(SbxValue,p->aData.pObj);
            if( pVal )
                p = pVal;
            else
                break;
        }
        else
            break;
    }
    return p;
}

sal_Bool SbxValue::Get( SbxValues& rRes ) const
{
    sal_Bool bRes = sal_False;
    SbxError eOld = GetError();
    if( eOld != SbxERR_OK )
        ResetError();
    if( !CanRead() )
    {
        SetError( SbxERR_PROP_WRITEONLY );
        rRes.pObj = NULL;
    }
    else
    {
        
        SbxValue* p = const_cast<SbxValue*>(this);
        if( rRes.eType != SbxOBJECT && rRes.eType != SbxVARIANT )
            p = TheRealValue();
        if( p )
        {
            p->Broadcast( SBX_HINT_DATAWANTED );
            switch( rRes.eType )
            {
                case SbxEMPTY:
                case SbxVOID:
                case SbxNULL:    break;
                case SbxVARIANT: rRes = p->aData; break;
                case SbxINTEGER: rRes.nInteger = ImpGetInteger( &p->aData ); break;
                case SbxLONG:    rRes.nLong = ImpGetLong( &p->aData ); break;
                case SbxSALINT64:   rRes.nInt64 = ImpGetInt64( &p->aData ); break;
                case SbxSALUINT64:  rRes.uInt64 = ImpGetUInt64( &p->aData ); break;
                case SbxSINGLE:  rRes.nSingle = ImpGetSingle( &p->aData ); break;
                case SbxDOUBLE:  rRes.nDouble = ImpGetDouble( &p->aData ); break;
                case SbxCURRENCY:rRes.nInt64 = ImpGetCurrency( &p->aData ); break;
                case SbxDECIMAL: rRes.pDecimal = ImpGetDecimal( &p->aData ); break;
                case SbxDATE:    rRes.nDouble = ImpGetDate( &p->aData ); break;
                case SbxBOOL:
                    rRes.nUShort = sal::static_int_cast< sal_uInt16 >(
                        ImpGetBool( &p->aData ) );
                    break;
                case SbxCHAR:    rRes.nChar = ImpGetChar( &p->aData ); break;
                case SbxBYTE:    rRes.nByte = ImpGetByte( &p->aData ); break;
                case SbxUSHORT:  rRes.nUShort = ImpGetUShort( &p->aData ); break;
                case SbxULONG:   rRes.nULong = ImpGetULong( &p->aData ); break;
                case SbxLPSTR:
                case SbxSTRING:  p->aPic = ImpGetString( &p->aData );
                                 rRes.pOUString = &p->aPic; break;
                case SbxCoreSTRING: p->aPic = ImpGetCoreString( &p->aData );
                                    rRes.pOUString = &p->aPic; break;
                case SbxINT:
#if SAL_TYPES_SIZEOFINT == 2
                    rRes.nInt = (int) ImpGetInteger( &p->aData );
#else
                    rRes.nInt = (int) ImpGetLong( &p->aData );
#endif
                    break;
                case SbxUINT:
#if SAL_TYPES_SIZEOFINT == 2
                    rRes.nUInt = (int) ImpGetUShort( &p->aData );
#else
                    rRes.nUInt = (int) ImpGetULong( &p->aData );
#endif
                    break;
                case SbxOBJECT:
                    if( p->aData.eType == SbxOBJECT )
                        rRes.pObj = p->aData.pObj;
                    else
                    {
                        SetError( SbxERR_NO_OBJECT );
                        rRes.pObj = NULL;
                    }
                    break;
                default:
                    if( p->aData.eType == rRes.eType )
                        rRes = p->aData;
                    else
                    {
                        SetError( SbxERR_CONVERSION );
                        rRes.pObj = NULL;
                    }
            }
        }
        else
        {
            
            SbxDataType eTemp = rRes.eType;
            memset( &rRes, 0, sizeof( SbxValues ) );
            rRes.eType = eTemp;
        }
    }
    if( !IsError() )
    {
        bRes = sal_True;
        if( eOld != SbxERR_OK )
            SetError( eOld );
    }
    return bRes;
}

const OUString& SbxValue::GetCoreString() const
{
    SbxValues aRes;
    aRes.eType = SbxCoreSTRING;
    if( Get( aRes ) )
    {
        ((SbxValue*) this)->aToolString = *aRes.pOUString;
    }
    else
    {
        ((SbxValue*) this)->aToolString = "";
    }
    return aToolString;
}

OUString SbxValue::GetOUString() const
{
    OUString aResult;
    SbxValues aRes;
    aRes.eType = SbxSTRING;
    if( Get( aRes ) )
    {
        aResult = *aRes.pOUString;
    }
    return aResult;
}

sal_Bool SbxValue::GetBool() const
{
    SbxValues aRes;
    aRes.eType = SbxBOOL;
    Get( aRes );
    return sal_Bool( aRes.nUShort != 0 );
}

#define GET( g, e, t, m ) \
t SbxValue::g() const { SbxValues aRes(e); Get( aRes ); return aRes.m; }

GET( GetByte,     SbxBYTE,       sal_uInt8,     nByte )
GET( GetChar,     SbxCHAR,       sal_Unicode,   nChar )
GET( GetCurrency, SbxCURRENCY,   sal_Int64,     nInt64 )
GET( GetDate,     SbxDATE,       double,        nDouble )
GET( GetDouble,   SbxDOUBLE,     double,        nDouble )
GET( GetInteger,  SbxINTEGER,    sal_Int16,     nInteger )
GET( GetLong,     SbxLONG,       sal_Int32,     nLong )
GET( GetObject,   SbxOBJECT,     SbxBase*,      pObj )
GET( GetSingle,   SbxSINGLE,     float,         nSingle )
GET( GetULong,    SbxULONG,      sal_uInt32,    nULong )
GET( GetUShort,   SbxUSHORT,     sal_uInt16,    nUShort )
GET( GetInt64,    SbxSALINT64,   sal_Int64,     nInt64 )
GET( GetUInt64,   SbxSALUINT64,  sal_uInt64,    uInt64 )
GET( GetDecimal,  SbxDECIMAL,    SbxDecimal*,   pDecimal )




sal_Bool SbxValue::Put( const SbxValues& rVal )
{
    sal_Bool bRes = sal_False;
    SbxError eOld = GetError();
    if( eOld != SbxERR_OK )
        ResetError();
    if( !CanWrite() )
        SetError( SbxERR_PROP_READONLY );
    else if( rVal.eType & 0xF000 )
        SetError( SbxERR_NOTIMP );
    else
    {
        
        SbxValue* p = this;
        if( rVal.eType != SbxOBJECT )
            p = TheRealValue( sal_False );  
        if( p )
        {
            if( !p->CanWrite() )
                SetError( SbxERR_PROP_READONLY );
            else if( p->IsFixed() || p->SetType( (SbxDataType) ( rVal.eType & 0x0FFF ) ) )
              switch( rVal.eType & 0x0FFF )
            {
                case SbxEMPTY:
                case SbxVOID:
                case SbxNULL:       break;
                case SbxINTEGER:    ImpPutInteger( &p->aData, rVal.nInteger ); break;
                case SbxLONG:       ImpPutLong( &p->aData, rVal.nLong ); break;
                case SbxSALINT64:   ImpPutInt64( &p->aData, rVal.nInt64 ); break;
                case SbxSALUINT64:  ImpPutUInt64( &p->aData, rVal.uInt64 ); break;
                case SbxSINGLE:     ImpPutSingle( &p->aData, rVal.nSingle ); break;
                case SbxDOUBLE:     ImpPutDouble( &p->aData, rVal.nDouble ); break;
                case SbxCURRENCY:   ImpPutCurrency( &p->aData, rVal.nInt64 ); break;
                case SbxDECIMAL:    ImpPutDecimal( &p->aData, rVal.pDecimal ); break;
                case SbxDATE:       ImpPutDate( &p->aData, rVal.nDouble ); break;
                case SbxBOOL:       ImpPutBool( &p->aData, rVal.nInteger ); break;
                case SbxCHAR:       ImpPutChar( &p->aData, rVal.nChar ); break;
                case SbxBYTE:       ImpPutByte( &p->aData, rVal.nByte ); break;
                case SbxUSHORT:     ImpPutUShort( &p->aData, rVal.nUShort ); break;
                case SbxULONG:      ImpPutULong( &p->aData, rVal.nULong ); break;
                case SbxLPSTR:
                case SbxSTRING:     ImpPutString( &p->aData, rVal.pOUString ); break;
                case SbxINT:
#if SAL_TYPES_SIZEOFINT == 2
                    ImpPutInteger( &p->aData, (sal_Int16) rVal.nInt );
#else
                    ImpPutLong( &p->aData, (sal_Int32) rVal.nInt );
#endif
                    break;
                case SbxUINT:
#if SAL_TYPES_SIZEOFINT == 2
                    ImpPutUShort( &p->aData, (sal_uInt16) rVal.nUInt );
#else
                    ImpPutULong( &p->aData, (sal_uInt32) rVal.nUInt );
#endif
                    break;
                case SbxOBJECT:
                    if( !p->IsFixed() || p->aData.eType == SbxOBJECT )
                    {
                        
                        if( p->aData.eType == SbxOBJECT && p->aData.pObj == rVal.pObj )
                            break;

                        
                        p->SbxValue::Clear();

                        
                        p->aData.pObj = rVal.pObj;

                        
                        if( p->aData.pObj && p->aData.pObj != p )
                        {
                            if ( p != this )
                            {
                                OSL_FAIL( "TheRealValue" );
                            }
                            SAL_WARN("basic.sbx", "Not at Parent-Prop - otherwise CyclicRef");
                            SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
                            bool bParentProp = pThisVar && 5345 ==
                                    ( (sal_Int16) ( pThisVar->GetUserData() & 0xFFFF ) );
                            if ( !bParentProp )
                                p->aData.pObj->AddRef();
                        }
                    }
                    else
                        SetError( SbxERR_CONVERSION );
                    break;
                default:
                    if( p->aData.eType == rVal.eType )
                        p->aData = rVal;
                    else
                    {
                        SetError( SbxERR_CONVERSION );
                        if( !p->IsFixed() )
                            p->aData.eType = SbxNULL;
                    }
            }
            if( !IsError() )
            {
                p->SetModified( sal_True );
                p->Broadcast( SBX_HINT_DATACHANGED );
                if( eOld != SbxERR_OK )
                    SetError( eOld );
                bRes = sal_True;
            }
        }
    }
    return bRes;
}








sal_Bool SbxValue::PutStringExt( const OUString& r )
{
    
    OUString aStr( r );

    
    
    SbxDataType eTargetType = SbxDataType( aData.eType & 0x0FFF );

    
    SbxValues aRes;
    aRes.eType = SbxSTRING;

    
    
    sal_Bool bRet;
    if( ImpConvStringExt( aStr, eTargetType ) )
        aRes.pOUString = (OUString*)&aStr;
    else
        aRes.pOUString = (OUString*)&r;

    
    
    sal_uInt16 nFlags_ = GetFlags();
    if( ( eTargetType >= SbxINTEGER && eTargetType <= SbxCURRENCY ) ||
        ( eTargetType >= SbxCHAR && eTargetType <= SbxUINT ) ||
        eTargetType == SbxBOOL )
    {
        SbxValue aVal;
        aVal.Put( aRes );
        if( aVal.IsNumeric() )
            SetFlag( SBX_FIXED );
    }

    Put( aRes );
    bRet = sal_Bool( !IsError() );

    
    
    if( !bRet )
        ResetError();

    SetFlags( nFlags_ );
    return bRet;
}

sal_Bool SbxValue::PutBool( sal_Bool b )
{
    SbxValues aRes;
    aRes.eType = SbxBOOL;
    aRes.nUShort = sal::static_int_cast< sal_uInt16 >(b ? SbxTRUE : SbxFALSE);
    Put( aRes );
    return sal_Bool( !IsError() );
}

sal_Bool SbxValue::PutEmpty()
{
    sal_Bool bRet = SetType( SbxEMPTY );
        SetModified( sal_True );
    return bRet;
}

sal_Bool SbxValue::PutNull()
{
    sal_Bool bRet = SetType( SbxNULL );
    if( bRet )
        SetModified( sal_True );
    return bRet;
}



sal_Bool SbxValue::PutDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec )
{
    SbxValue::Clear();
    aData.pDecimal = new SbxDecimal( rAutomationDec );
    aData.pDecimal->addRef();
    aData.eType = SbxDECIMAL;
    return sal_True;
}

sal_Bool SbxValue::fillAutomationDecimal
    ( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec ) const
{
    SbxDecimal* pDecimal = GetDecimal();
    if( pDecimal != NULL )
    {
        pDecimal->fillAutomationDecimal( rAutomationDec );
        return sal_True;
    }
    return sal_False;
}


sal_Bool SbxValue::PutString( const OUString& r )
{
    SbxValues aRes;
    aRes.eType = SbxSTRING;
    aRes.pOUString = (OUString*) &r;
    Put( aRes );
    return sal_Bool( !IsError() );
}


#define PUT( p, e, t, m ) \
sal_Bool SbxValue::p( t n ) \
{ SbxValues aRes(e); aRes.m = n; Put( aRes ); return sal_Bool( !IsError() ); }

PUT( PutByte,     SbxBYTE,       sal_uInt8,        nByte )
PUT( PutChar,     SbxCHAR,       sal_Unicode,      nChar )
PUT( PutCurrency, SbxCURRENCY,   const sal_Int64&, nInt64 )
PUT( PutDate,     SbxDATE,       double,           nDouble )
PUT( PutDouble,   SbxDOUBLE,     double,           nDouble )
PUT( PutErr,      SbxERROR,      sal_uInt16,       nUShort )
PUT( PutInteger,  SbxINTEGER,    sal_Int16,        nInteger )
PUT( PutLong,     SbxLONG,       sal_Int32,        nLong )
PUT( PutObject,   SbxOBJECT,     SbxBase*,         pObj )
PUT( PutSingle,   SbxSINGLE,     float,            nSingle )
PUT( PutULong,    SbxULONG,      sal_uInt32,       nULong )
PUT( PutUShort,   SbxUSHORT,     sal_uInt16,       nUShort )
PUT( PutInt64,    SbxSALINT64,   sal_Int64,        nInt64 )
PUT( PutUInt64,   SbxSALUINT64,  sal_uInt64,       uInt64 )
PUT( PutDecimal,  SbxDECIMAL,    SbxDecimal*,      pDecimal )



sal_Bool SbxValue::IsFixed() const
{
    return ( (GetFlags() & SBX_FIXED) | (aData.eType & SbxBYREF) ) != 0;
}





sal_Bool SbxValue::IsNumeric() const
{
    return ImpIsNumeric( /*bOnlyIntntl*/false );
}

sal_Bool SbxValue::IsNumericRTL() const
{
    return ImpIsNumeric( /*bOnlyIntntl*/true );
}

sal_Bool SbxValue::ImpIsNumeric( bool bOnlyIntntl ) const
{

    if( !CanRead() )
    {
        SetError( SbxERR_PROP_WRITEONLY ); return sal_False;
    }
    
    if( this->ISA(SbxVariable) )
        ((SbxVariable*)this)->Broadcast( SBX_HINT_DATAWANTED );
    SbxDataType t = GetType();
    if( t == SbxSTRING )
    {
        if( aData.pOUString )
        {
            OUString s( *aData.pOUString );
            double n;
            SbxDataType t2;
            sal_uInt16 nLen = 0;
            if( ImpScan( s, n, t2, &nLen, /*bAllowIntntl*/false, bOnlyIntntl ) == SbxERR_OK )
                return sal_Bool( nLen == s.getLength() );
        }
        return sal_False;
    }
    else
        return sal_Bool( t == SbxEMPTY
            || ( t >= SbxINTEGER && t <= SbxCURRENCY )
            || ( t >= SbxCHAR && t <= SbxUINT ) );
}

SbxClassType SbxValue::GetClass() const
{
    return SbxCLASS_VALUE;
}

SbxDataType SbxValue::GetType() const
{
    return SbxDataType( aData.eType & 0x0FFF );
}

SbxDataType SbxValue::GetFullType() const
{
    return aData.eType;
}

sal_Bool SbxValue::SetType( SbxDataType t )
{
    DBG_ASSERT( !( t & 0xF000 ), "SetType of BYREF|ARRAY is forbidden!" );
    if( ( t == SbxEMPTY && aData.eType == SbxVOID )
     || ( aData.eType == SbxEMPTY && t == SbxVOID ) )
        return sal_True;
    if( ( t & 0x0FFF ) == SbxVARIANT )
    {
        
        ResetFlag( SBX_FIXED );
        if( IsFixed() )
        {
            SetError( SbxERR_CONVERSION ); return sal_False;
        }
        t = SbxEMPTY;
    }
    if( ( t & 0x0FFF ) != ( aData.eType & 0x0FFF ) )
    {
        if( !CanWrite() || IsFixed() )
        {
            SetError( SbxERR_CONVERSION ); return sal_False;
        }
        else
        {
            
            switch( aData.eType )
            {
                case SbxSTRING:
                    delete aData.pOUString;
                    break;
                case SbxOBJECT:
                    if( aData.pObj && aData.pObj != this )
                    {
                        SAL_WARN("basic.sbx", "Not at Parent-Prop - otherwise CyclicRef");
                        SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
                        sal_uInt16 nSlotId = pThisVar
                                    ? ( (sal_Int16) ( pThisVar->GetUserData() & 0xFFFF ) )
                                    : 0;
                        DBG_ASSERT( nSlotId != 5345 || pThisVar->GetName().equalsAscii("Parent"),
                                    "SID_PARENTOBJECT is not named 'Parent'" );
                        bool bParentProp = 5345 == nSlotId;
                        if ( !bParentProp )
                            aData.pObj->ReleaseRef();
                    }
                    break;
                default: break;
            }
            
            memset( &aData, 0, sizeof( SbxValues ) );
            aData.eType = t;
        }
    }
    return sal_True;
}

sal_Bool SbxValue::Convert( SbxDataType eTo )
{
    eTo = SbxDataType( eTo & 0x0FFF );
    if( ( aData.eType & 0x0FFF ) == eTo )
        return sal_True;
    if( !CanWrite() )
        return sal_False;
    if( eTo == SbxVARIANT )
    {
        
        ResetFlag( SBX_FIXED );
        if( IsFixed() )
        {
            SetError( SbxERR_CONVERSION ); return sal_False;
        }
        else
            return sal_True;
    }
    
    if( aData.eType == SbxNULL )
    {
        SetError( SbxERR_CONVERSION ); return sal_False;
    }

    
    SbxValues aNew;
    aNew.eType = eTo;
    if( Get( aNew ) )
    {
        
        
        if( !IsFixed() )
        {
            SetType( eTo );
            Put( aNew );
            SetModified( sal_True );
        }
        Broadcast( SBX_HINT_CONVERTED );
        return sal_True;
    }
    else
        return sal_False;
}


sal_Bool SbxValue::Compute( SbxOperator eOp, const SbxValue& rOp )
{
#ifdef DISABLE_SCRIPTING
    bool bVBAInterop = false;
#else
    bool bVBAInterop =  SbiRuntime::isVBAEnabled();
#endif
    SbxDataType eThisType = GetType();
    SbxDataType eOpType = rOp.GetType();
    SbxError eOld = GetError();
    if( eOld != SbxERR_OK )
        ResetError();
    if( !CanWrite() )
        SetError( SbxERR_PROP_READONLY );
    else if( !rOp.CanRead() )
        SetError( SbxERR_PROP_WRITEONLY );
    
    else if( eThisType == SbxNULL || eOpType == SbxNULL )
        SetType( SbxNULL );
    
    else if( eThisType == SbxEMPTY
    && !bVBAInterop
    )
        *this = rOp;
    
    else
    {
        SbxValues aL, aR;
        bool bDecimal = false;
        if( bVBAInterop && ( ( eThisType == SbxSTRING && eOpType != SbxSTRING && eOpType != SbxEMPTY ) ||
             ( eThisType != SbxSTRING && eThisType != SbxEMPTY && eOpType == SbxSTRING ) ) &&
             ( eOp == SbxMUL || eOp == SbxDIV || eOp == SbxPLUS || eOp == SbxMINUS ) )
        {
            goto Lbl_OpIsDouble;
        }
        else if( eThisType == SbxSTRING || eOp == SbxCAT || ( bVBAInterop && ( eOpType == SbxSTRING ) && (  eOp == SbxPLUS ) ) )
        {
            if( eOp == SbxCAT || eOp == SbxPLUS )
            {
                
                aL.eType = aR.eType = SbxSTRING;
                rOp.Get( aR );
                
                if( rOp.GetType() == SbxEMPTY )
                    goto Lbl_OpIsEmpty;
                Get( aL );

                
                if( aL.pOUString != NULL && aR.pOUString != NULL )
                {
                    *aL.pOUString += *aR.pOUString;
                }
                
                else if( aL.pOUString == NULL )
                {
                    aL.pOUString = new OUString();
                }
                Put( aL );
            }
            else
                SetError( SbxERR_CONVERSION );
        }
        else if( eOpType == SbxSTRING && rOp.IsFixed() )
        {   
            SetError( SbxERR_CONVERSION );
            
        }
        else if( ( eOp >= SbxIDIV && eOp <= SbxNOT ) || eOp == SbxMOD )
        {
            if( GetType() == eOpType )
            {
                if( GetType() == SbxSALUINT64 || GetType() == SbxSALINT64
                 || GetType() == SbxCURRENCY  || GetType() == SbxULONG )
                    aL.eType = aR.eType = GetType();
                else if ( bVBAInterop && eOpType == SbxBOOL )
                    aL.eType = aR.eType = SbxBOOL;
                else
                    aL.eType = aR.eType = SbxLONG;
            }
            else
                aL.eType = aR.eType = SbxLONG;

            if( rOp.Get( aR ) )     
            {
                if( rOp.GetType() == SbxEMPTY )
                {
                    if ( !bVBAInterop || ( bVBAInterop && ( eOp != SbxNOT  ) ) )
                        goto Lbl_OpIsEmpty;
                }
                if( Get( aL ) ) switch( eOp )
                {
                    case SbxIDIV:
                        if( aL.eType == SbxCURRENCY )
                            if( !aR.nInt64 ) SetError( SbxERR_ZERODIV );
                            else {
                                aL.nInt64 /= aR.nInt64;
                                aL.nInt64 *= CURRENCY_FACTOR;
                        }
                        else if( aL.eType == SbxSALUINT64 )
                            if( !aR.uInt64 ) SetError( SbxERR_ZERODIV );
                            else aL.uInt64 /= aR.uInt64;
                        else if( aL.eType == SbxSALINT64 )
                            if( !aR.nInt64 ) SetError( SbxERR_ZERODIV );
                            else aL.nInt64 /= aR.nInt64;
                        else if( aL.eType == SbxLONG )
                            if( !aR.nLong ) SetError( SbxERR_ZERODIV );
                            else aL.nLong /= aR.nLong;
                        else
                            if( !aR.nULong ) SetError( SbxERR_ZERODIV );
                            else aL.nULong /= aR.nULong;
                        break;
                    case SbxMOD:
                        if( aL.eType == SbxCURRENCY || aL.eType == SbxSALINT64 )
                            if( !aR.nInt64 ) SetError( SbxERR_ZERODIV );
                            else aL.nInt64 %= aR.nInt64;
                        else if( aL.eType == SbxSALUINT64 )
                            if( !aR.uInt64 ) SetError( SbxERR_ZERODIV );
                            else aL.uInt64 %= aR.uInt64;
                        else if( aL.eType == SbxLONG )
                            if( !aR.nLong ) SetError( SbxERR_ZERODIV );
                            else aL.nLong %= aR.nLong;
                        else
                            if( !aR.nULong ) SetError( SbxERR_ZERODIV );
                            else aL.nULong %= aR.nULong;
                        break;
                    case SbxAND:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                            aL.nInt64 &= aR.nInt64;
                        else
                            aL.nLong &= aR.nLong;
                        break;
                    case SbxOR:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                            aL.nInt64 |= aR.nInt64;
                        else
                            aL.nLong |= aR.nLong;
                        break;
                    case SbxXOR:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                            aL.nInt64 ^= aR.nInt64;
                        else
                            aL.nLong ^= aR.nLong;
                        break;
                    case SbxEQV:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                            aL.nInt64 = (aL.nInt64 & aR.nInt64) | (~aL.nInt64 & ~aR.nInt64);
                        else
                            aL.nLong = (aL.nLong & aR.nLong) | (~aL.nLong & ~aR.nLong);
                        break;
                    case SbxIMP:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                            aL.nInt64 = ~aL.nInt64 | aR.nInt64;
                        else
                            aL.nLong = ~aL.nLong | aR.nLong;
                        break;
                    case SbxNOT:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                        {
                            if ( aL.eType != SbxBOOL )
                                aL.nInt64 = ~aL.nInt64;
                            else
                                aL.nLong = ~aL.nLong;
                        }
                        else
                            aL.nLong = ~aL.nLong;
                        break;
                    default: break;
                }
            }
        }
        else if( ( GetType() == SbxDECIMAL || rOp.GetType() == SbxDECIMAL )
              && ( eOp == SbxMUL || eOp == SbxDIV || eOp == SbxPLUS || eOp == SbxMINUS || eOp == SbxNEG ) )
        {
            aL.eType = aR.eType = SbxDECIMAL;
            bDecimal = true;
            if( rOp.Get( aR ) )
            {
                if( rOp.GetType() == SbxEMPTY )
                {
                    releaseDecimalPtr( aL.pDecimal );
                    goto Lbl_OpIsEmpty;
                }
                if( Get( aL ) )
                {
                    if( aL.pDecimal && aR.pDecimal )
                    {
                        bool bOk = true;
                        switch( eOp )
                        {
                            case SbxMUL:
                                bOk = ( *(aL.pDecimal) *= *(aR.pDecimal) );
                                break;
                            case SbxDIV:
                                if( aR.pDecimal->isZero() )
                                    SetError( SbxERR_ZERODIV );
                                else
                                    bOk = ( *(aL.pDecimal) /= *(aR.pDecimal) );
                                break;
                            case SbxPLUS:
                                bOk = ( *(aL.pDecimal) += *(aR.pDecimal) );
                                break;
                            case SbxMINUS:
                                bOk = ( *(aL.pDecimal) -= *(aR.pDecimal) );
                                break;
                            case SbxNEG:
                                bOk = ( aL.pDecimal->neg() );
                                break;
                            default:
                                SetError( SbxERR_NOTIMP );
                        }
                        if( !bOk )
                            SetError( SbxERR_OVERFLOW );
                    }
                    else
                    {
                        SetError( SbxERR_CONVERSION );
                    }
                }
            }
        }
        else if( GetType() == SbxCURRENCY || rOp.GetType() == SbxCURRENCY )
        {
            aL.eType = SbxCURRENCY;
            aR.eType = SbxCURRENCY;

            if( rOp.Get( aR ) )
            {
                if( rOp.GetType() == SbxEMPTY )
                    goto Lbl_OpIsEmpty;

                if( Get( aL ) ) switch( eOp )
                {
                    double dTest;
                    case SbxMUL:
                        
                        dTest = (double)aL.nInt64 * (double)aR.nInt64 / (double)CURRENCY_FACTOR_SQUARE;
                        if( dTest < SbxMINCURR || SbxMAXCURR < dTest)
                        {
                            aL.nInt64 = SAL_MAX_INT64;
                            if( dTest < SbxMINCURR ) aL.nInt64 = SAL_MIN_INT64;
                            SetError( SbxERR_OVERFLOW );
                            break;
                        }
                        
                        dTest = (double)aL.nInt64 * (double)aR.nInt64;
                        if( dTest < SAL_MIN_INT64 || SAL_MAX_INT64 < dTest)
                        {
                            aL.nInt64 = (sal_Int64)( dTest / (double)CURRENCY_FACTOR );
                            break;
                        }
                        
                        aL.nInt64 *= aR.nInt64;
                        aL.nInt64 /= CURRENCY_FACTOR;
                        break;

                    case SbxDIV:
                        if( !aR.nInt64 )
                        {
                            SetError( SbxERR_ZERODIV );
                            break;
                        }
                        
                        dTest = (double)aL.nInt64 / (double)aR.nInt64;
                        if( dTest < SbxMINCURR || SbxMAXCURR < dTest)
                        {
                            SetError( SbxERR_OVERFLOW );
                            break;
                        }
                        
                        dTest = (double)aL.nInt64 * (double)CURRENCY_FACTOR;
                        if( dTest < SAL_MIN_INT64 || SAL_MAX_INT64 < dTest)
                        {
                            aL.nInt64 = (sal_Int64)(dTest / (double)aR.nInt64);
                            break;
                        }
                        
                        aL.nInt64 *= CURRENCY_FACTOR;
                        aL.nInt64 /= aR.nInt64;
                        break;

                    case SbxPLUS:
                        dTest = ( (double)aL.nInt64 + (double)aR.nInt64 ) / (double)CURRENCY_FACTOR;
                        if( dTest < SbxMINCURR || SbxMAXCURR < dTest)
                        {
                            SetError( SbxERR_OVERFLOW );
                            break;
                        }
                        aL.nInt64 += aR.nInt64;
                        break;

                    case SbxMINUS:
                        dTest = ( (double)aL.nInt64 - (double)aR.nInt64 ) / (double)CURRENCY_FACTOR;
                        if( dTest < SbxMINCURR || SbxMAXCURR < dTest)
                        {
                            SetError( SbxERR_OVERFLOW );
                            break;
                        }
                        aL.nInt64 -= aR.nInt64;
                        break;
                    case SbxNEG:
                        aL.nInt64 = -aL.nInt64;
                        break;
                    default:
                        SetError( SbxERR_NOTIMP );
                }
            }
        }
        else
Lbl_OpIsDouble:
        {   
            aL.eType = aR.eType = SbxDOUBLE;
            if( rOp.Get( aR ) )
            {
                if( rOp.GetType() == SbxEMPTY )
                {
                    if ( !bVBAInterop || ( bVBAInterop && ( eOp != SbxNEG ) ) )
                        goto Lbl_OpIsEmpty;
                }
                if( Get( aL ) )
                {
                    switch( eOp )
                    {
                        case SbxEXP:
                            aL.nDouble = pow( aL.nDouble, aR.nDouble );
                            break;
                        case SbxMUL:
                            aL.nDouble *= aR.nDouble; break;
                        case SbxDIV:
                            if( !aR.nDouble ) SetError( SbxERR_ZERODIV );
                            else aL.nDouble /= aR.nDouble; break;
                        case SbxPLUS:
                            aL.nDouble += aR.nDouble; break;
                        case SbxMINUS:
                            aL.nDouble -= aR.nDouble; break;
                        case SbxNEG:
                            aL.nDouble = -aL.nDouble; break;
                        default:
                            SetError( SbxERR_NOTIMP );
                    }
                    
                    
                    
                    
                    
                    if( ( GetType() == SbxDATE || rOp.GetType() == SbxDATE ) )
                    {
                        if( eOp == SbxPLUS  || ( ( eOp == SbxMINUS ) &&  ( GetType() != rOp.GetType() ) ) )
                            aL.eType = SbxDATE;
                    }

                }
            }

        }
        if( !IsError() )
            Put( aL );
        if( bDecimal )
        {
            releaseDecimalPtr( aL.pDecimal );
            releaseDecimalPtr( aR.pDecimal );
        }
    }
Lbl_OpIsEmpty:

    sal_Bool bRes = sal_Bool( !IsError() );
    if( bRes && eOld != SbxERR_OK )
        SetError( eOld );
    return bRes;
}



sal_Bool SbxValue::Compare( SbxOperator eOp, const SbxValue& rOp ) const
{
#ifdef DISABLE_SCRIPTING
    bool bVBAInterop = false;
#else
    bool bVBAInterop =  SbiRuntime::isVBAEnabled();
#endif

    sal_Bool bRes = sal_False;
    SbxError eOld = GetError();
    if( eOld != SbxERR_OK )
        ResetError();
    if( !CanRead() || !rOp.CanRead() )
        SetError( SbxERR_PROP_WRITEONLY );
    else if( GetType() == SbxNULL && rOp.GetType() == SbxNULL && !bVBAInterop )
    {
        bRes = sal_True;
    }
    else if( GetType() == SbxEMPTY && rOp.GetType() == SbxEMPTY )
        bRes = !bVBAInterop ? sal_True : ( eOp == SbxEQ ? sal_True : sal_False );
    
    else if( GetType() == SbxNULL || rOp.GetType() == SbxNULL )
        bRes = sal_False;
    
    
    else if( !IsFixed() && !rOp.IsFixed()
     && ( rOp.GetType() == SbxSTRING && GetType() != SbxSTRING && IsNumeric() ) && !bVBAInterop
    )
        bRes = sal_Bool( eOp == SbxLT || eOp == SbxLE || eOp == SbxNE );
    else if( !IsFixed() && !rOp.IsFixed()
     && ( GetType() == SbxSTRING && rOp.GetType() != SbxSTRING && rOp.IsNumeric() )
&& !bVBAInterop
    )
        bRes = sal_Bool( eOp == SbxGT || eOp == SbxGE || eOp == SbxNE );
    else
    {
        SbxValues aL, aR;
        
        
        if( GetType() == SbxSTRING || rOp.GetType() == SbxSTRING )
        {
            aL.eType = aR.eType = SbxSTRING;
            if( Get( aL ) && rOp.Get( aR ) ) switch( eOp )
            {
                case SbxEQ:
                    bRes = sal_Bool( *aL.pOUString == *aR.pOUString ); break;
                case SbxNE:
                    bRes = sal_Bool( *aL.pOUString != *aR.pOUString ); break;
                case SbxLT:
                    bRes = sal_Bool( *aL.pOUString <  *aR.pOUString ); break;
                case SbxGT:
                    bRes = sal_Bool( *aL.pOUString >  *aR.pOUString ); break;
                case SbxLE:
                    bRes = sal_Bool( *aL.pOUString <= *aR.pOUString ); break;
                case SbxGE:
                    bRes = sal_Bool( *aL.pOUString >= *aR.pOUString ); break;
                default:
                    SetError( SbxERR_NOTIMP );
            }
        }
        
        
        else if( GetType() == SbxSINGLE || rOp.GetType() == SbxSINGLE )
        {
            aL.eType = aR.eType = SbxSINGLE;
            if( Get( aL ) && rOp.Get( aR ) )
              switch( eOp )
            {
                case SbxEQ:
                    bRes = sal_Bool( aL.nSingle == aR.nSingle ); break;
                case SbxNE:
                    bRes = sal_Bool( aL.nSingle != aR.nSingle ); break;
                case SbxLT:
                    bRes = sal_Bool( aL.nSingle <  aR.nSingle ); break;
                case SbxGT:
                    bRes = sal_Bool( aL.nSingle >  aR.nSingle ); break;
                case SbxLE:
                    bRes = sal_Bool( aL.nSingle <= aR.nSingle ); break;
                case SbxGE:
                    bRes = sal_Bool( aL.nSingle >= aR.nSingle ); break;
                default:
                    SetError( SbxERR_NOTIMP );
            }
        }
        else if( GetType() == SbxDECIMAL && rOp.GetType() == SbxDECIMAL )
        {
            aL.eType = aR.eType = SbxDECIMAL;
            Get( aL );
            rOp.Get( aR );
            if( aL.pDecimal && aR.pDecimal )
            {
                SbxDecimal::CmpResult eRes = compare( *aL.pDecimal, *aR.pDecimal );
                switch( eOp )
                {
                    case SbxEQ:
                        bRes = sal_Bool( eRes == SbxDecimal::EQ ); break;
                    case SbxNE:
                        bRes = sal_Bool( eRes != SbxDecimal::EQ ); break;
                    case SbxLT:
                        bRes = sal_Bool( eRes == SbxDecimal::LT ); break;
                    case SbxGT:
                        bRes = sal_Bool( eRes == SbxDecimal::GT ); break;
                    case SbxLE:
                        bRes = sal_Bool( eRes != SbxDecimal::GT ); break;
                    case SbxGE:
                        bRes = sal_Bool( eRes != SbxDecimal::LT ); break;
                    default:
                        SetError( SbxERR_NOTIMP );
                }
            }
            else
            {
                SetError( SbxERR_CONVERSION );
            }
            releaseDecimalPtr( aL.pDecimal );
            releaseDecimalPtr( aR.pDecimal );
        }
        
        else
        {
            aL.eType = aR.eType = SbxDOUBLE;
            bool bGetL = Get( aL );
            bool bGetR = rOp.Get( aR );
            if( bGetL && bGetR )
              switch( eOp )
            {
                case SbxEQ:
                    bRes = sal_Bool( aL.nDouble == aR.nDouble ); break;
                case SbxNE:
                    bRes = sal_Bool( aL.nDouble != aR.nDouble ); break;
                case SbxLT:
                    bRes = sal_Bool( aL.nDouble <  aR.nDouble ); break;
                case SbxGT:
                    bRes = sal_Bool( aL.nDouble >  aR.nDouble ); break;
                case SbxLE:
                    bRes = sal_Bool( aL.nDouble <= aR.nDouble ); break;
                case SbxGE:
                    bRes = sal_Bool( aL.nDouble >= aR.nDouble ); break;
                default:
                    SetError( SbxERR_NOTIMP );
            }
            
            
            
            else if ( bGetR || bGetL )
            {
                if ( bVBAInterop && eOp == SbxEQ && GetError() == SbxERR_CONVERSION )
                {
                    ResetError();
                    bRes = sal_False;
                }
            }
        }
    }
    if( eOld != SbxERR_OK )
        SetError( eOld );
    return bRes;
}



sal_Bool SbxValue::LoadData( SvStream& r, sal_uInt16 )
{
    
    
    SbxValue::Clear();
    sal_uInt16 nType;
    r.ReadUInt16( nType );
    aData.eType = SbxDataType( nType );
    switch( nType )
    {
        case SbxBOOL:
        case SbxINTEGER:
            r.ReadInt16( aData.nInteger ); break;
        case SbxLONG:
            r.ReadInt32( aData.nLong ); break;
        case SbxSINGLE:
        {
            
            OUString aVal = read_uInt16_lenPrefixed_uInt8s_ToOUString(r,
                RTL_TEXTENCODING_ASCII_US);
            double d;
            SbxDataType t;
            if( ImpScan( aVal, d, t, NULL ) != SbxERR_OK || t == SbxDOUBLE )
            {
                aData.nSingle = 0.0F;
                return sal_False;
            }
            aData.nSingle = (float) d;
            break;
        }
        case SbxDATE:
        case SbxDOUBLE:
        {
            
            OUString aVal = read_uInt16_lenPrefixed_uInt8s_ToOUString(r,
                RTL_TEXTENCODING_ASCII_US);
            SbxDataType t;
            if( ImpScan( aVal, aData.nDouble, t, NULL ) != SbxERR_OK )
            {
                aData.nDouble = 0.0;
                return sal_False;
            }
            break;
        }
        
        case SbxSALINT64:
            r.ReadInt64(aData.nInt64);
            break;
        case SbxSALUINT64:
            r.ReadUInt64( aData.uInt64 );
            break;
        case SbxCURRENCY:
        {
            sal_uInt32 tmpHi = 0;
            sal_uInt32 tmpLo = 0;
            r.ReadUInt32( tmpHi ).ReadUInt32( tmpLo );
            aData.nInt64 = ((sal_Int64)tmpHi << 32);
            aData.nInt64 |= (sal_Int64)tmpLo;
            break;
        }
        case SbxSTRING:
        {
            OUString aVal = read_uInt16_lenPrefixed_uInt8s_ToOUString(r,
                RTL_TEXTENCODING_ASCII_US);
            if( !aVal.isEmpty() )
                    aData.pOUString = new OUString( aVal );
                else
                    aData.pOUString = NULL; 
                break;
            }
            case SbxERROR:
            case SbxUSHORT:
                r.ReadUInt16( aData.nUShort ); break;
            case SbxOBJECT:
            {
                sal_uInt8 nMode;
                r.ReadUChar( nMode );
                switch( nMode )
                {
                    case 0:
                        aData.pObj = NULL;
                        break;
                    case 1:
                        aData.pObj = SbxBase::Load( r );
                        return sal_Bool( aData.pObj != NULL );
                    case 2:
                        aData.pObj = this;
                        break;
                }
                break;
            }
            case SbxCHAR:
            {
                char c;
                r.ReadChar( c );
                aData.nChar = c;
                break;
            }
            case SbxBYTE:
                r.ReadUChar( aData.nByte ); break;
            case SbxULONG:
                r.ReadUInt32( aData.nULong ); break;
            case SbxINT:
            {
                sal_uInt8 n;
                r.ReadUChar( n );
                
                if( n > SAL_TYPES_SIZEOFINT )
                    r.ReadInt32( aData.nLong ), aData.eType = SbxLONG;
                else {
                    sal_Int32 nInt;
                    r.ReadInt32( nInt );
                    aData.nInt = nInt;
                }
                break;
            }
            case SbxUINT:
            {
                sal_uInt8 n;
                r.ReadUChar( n );
                
                if( n > SAL_TYPES_SIZEOFINT )
                    r.ReadUInt32( aData.nULong ), aData.eType = SbxULONG;
                else {
                    sal_uInt32 nUInt;
                    r.ReadUInt32( nUInt );
                    aData.nUInt = nUInt;
                }
                break;
            }
            case SbxEMPTY:
            case SbxNULL:
            case SbxVOID:
                break;
            case SbxDATAOBJECT:
                r.ReadInt32( aData.nLong );
                break;
            
            case SbxWSTRING:
            case SbxWCHAR:
                break;
            default:
                memset (&aData,0,sizeof(aData));
                ResetFlag(SBX_FIXED);
                aData.eType = SbxNULL;
                DBG_ASSERT( !this, "Loaded a non-supported data type" );

                return sal_False;
        }
        return sal_True;
    }

    sal_Bool SbxValue::StoreData( SvStream& r ) const
    {
        sal_uInt16 nType = sal::static_int_cast< sal_uInt16 >(aData.eType);
        r.WriteUInt16( nType );
        switch( nType & 0x0FFF )
        {
            case SbxBOOL:
            case SbxINTEGER:
                r.WriteInt16( aData.nInteger ); break;
            case SbxLONG:
                r.WriteInt32( aData.nLong ); break;
            case SbxDATE:
                
                ((SbxValue*)this)->aData.eType = (SbxDataType)( ( nType & 0xF000 ) | SbxDOUBLE );
                write_uInt16_lenPrefixed_uInt8s_FromOUString(r, GetCoreString(), RTL_TEXTENCODING_ASCII_US);
                ((SbxValue*)this)->aData.eType = (SbxDataType)nType;
                break;
            case SbxSINGLE:
            case SbxDOUBLE:
                write_uInt16_lenPrefixed_uInt8s_FromOUString(r, GetCoreString(), RTL_TEXTENCODING_ASCII_US);
                break;
            case SbxSALUINT64:
            case SbxSALINT64:
                
                r.WriteUInt64( aData.uInt64 );
                break;
            case SbxCURRENCY:
            {
                sal_Int32 tmpHi = ( (aData.nInt64 >> 32) &  0xFFFFFFFF );
                sal_Int32 tmpLo = ( sal_Int32 )aData.nInt64;
                r.WriteInt32( tmpHi ).WriteInt32( tmpLo );
                break;
            }
            case SbxSTRING:
                if( aData.pOUString )
                {
                    write_uInt16_lenPrefixed_uInt8s_FromOUString(r, *aData.pOUString, RTL_TEXTENCODING_ASCII_US);
                }
                else
                {
                    write_uInt16_lenPrefixed_uInt8s_FromOUString(r, OUString(), RTL_TEXTENCODING_ASCII_US);
            }
            break;
        case SbxERROR:
        case SbxUSHORT:
            r.WriteUInt16( aData.nUShort ); break;
        case SbxOBJECT:
            
            if( aData.pObj )
            {
                if( PTR_CAST(SbxValue,aData.pObj) != this )
                {
                    r.WriteUChar( (sal_uInt8) 1 );
                    return aData.pObj->Store( r );
                }
                else
                    r.WriteUChar( (sal_uInt8) 2 );
            }
            else
                r.WriteUChar( (sal_uInt8) 0 );
            break;
        case SbxCHAR:
        {
            char c = sal::static_int_cast< char >(aData.nChar);
            r.WriteChar( c );
            break;
        }
        case SbxBYTE:
            r.WriteUChar( aData.nByte ); break;
        case SbxULONG:
            r.WriteUInt32( aData.nULong ); break;
        case SbxINT:
        {
            sal_uInt8 n = SAL_TYPES_SIZEOFINT;
            r.WriteUChar( n ).WriteInt32( (sal_Int32)aData.nInt );
            break;
        }
        case SbxUINT:
        {
            sal_uInt8 n = SAL_TYPES_SIZEOFINT;
            r.WriteUChar( n ).WriteUInt32( (sal_uInt32)aData.nUInt );
            break;
        }
        case SbxEMPTY:
        case SbxNULL:
        case SbxVOID:
            break;
        case SbxDATAOBJECT:
            r.WriteInt32( aData.nLong );
            break;
        
        case SbxWSTRING:
        case SbxWCHAR:
            break;
        default:
            DBG_ASSERT( !this, "Saving a non-supported data type" );
            return sal_False;
    }
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
