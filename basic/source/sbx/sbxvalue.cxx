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


#include <math.h>
#include <tools/stream.hxx>

#include <basic/sbx.hxx>
#include "sbxconv.hxx"
#include "runtime.hxx"

TYPEINIT1(SbxValue,SbxBase)

///////////////////////////// constructors //////////////////////////////

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
        case SbxSTRING:     n |= SbxBYREF; aData.pOUString = (::rtl::OUString*) p; break;
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
            DBG_ASSERT( !this, "Angabe eines Pointers unzulaessig" );
            n = SbxNULL;
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
        // Copy pointer, increment references
        switch( aData.eType )
        {
            case SbxSTRING:
                if( aData.pOUString )
                    aData.pOUString = new ::rtl::OUString( *aData.pOUString );
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
            // string -> byte array
            if( IsFixed() && (aData.eType == SbxOBJECT)
                && aData.pObj && ( aData.pObj->GetType() == (SbxARRAY | SbxBYTE) )
                && (r.aData.eType == SbxSTRING) )
            {
                ::rtl::OUString aStr = r.GetString();
                SbxArray* pArr = StringToByteArray(aStr);
                PutObject(pArr);
                return *this;
            }
            // byte array -> string
            if( r.IsFixed() && (r.aData.eType == SbxOBJECT)
                && r.aData.pObj && ( r.aData.pObj->GetType() == (SbxARRAY | SbxBYTE) )
                && (aData.eType == SbxSTRING) )
            {
                SbxBase* pObj = r.GetObject();
                SbxArray* pArr = PTR_CAST(SbxArray, pObj);
                if( pArr )
                {
                    ::rtl::OUString aStr = ByteArrayToString( pArr );
                    PutString(aStr);
                    return *this;
                }
            }
            // Readout the content of the variables
            SbxValues aNew;
            if( IsFixed() )
                // firm: then the type had to match
                aNew.eType = aData.eType;
            else if( r.IsFixed() )
                // Source firm: take over the type
                aNew.eType = SbxDataType( r.aData.eType & 0x0FFF );
            else
                // both variant: then it is equal
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
                    HACK(nicht bei Parent-Prop - sonst CyclicRef)
                    SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
                    sal_Bool bParentProp = pThisVar && 5345 ==
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

// Dummy

void SbxValue::Broadcast( sal_uIntPtr )
{}

//////////////////////////// Readout data //////////////////////////////

// Detect the "right" variables. If it is an object, will be addressed either
// the object itself or its default property.
// If the variable contain a variable or an object, this will be
// addressed.

SbxValue* SbxValue::TheRealValue() const
{
    return TheRealValue( sal_True );
}

// #55226 ship additional information
bool handleToStringForCOMObjects( SbxObject* pObj, SbxValue* pVal );    // sbunoobj.cxx

SbxValue* SbxValue::TheRealValue( sal_Bool bObjInObjError ) const
{
    SbxValue* p = (SbxValue*) this;
    for( ;; )
    {
        SbxDataType t = SbxDataType( p->aData.eType & 0x0FFF );
        if( t == SbxOBJECT )
        {
            // The block contains an object or a variable
            SbxObject* pObj = PTR_CAST(SbxObject,p->aData.pObj);
            if( pObj )
            {
                // Has the object a default property?
                SbxVariable* pDflt = pObj->GetDfltProperty();

                // If this is an object and contains itself,
                // we cannot access on it
                // The old condition to set an error
                // is not correct, because e.g. a regular variant variable with an object
                // could be affected thereof, if another value should be assigned.
                // Therefore with flag.
                if( bObjInObjError && !pDflt &&
                    ((SbxValue*) pObj)->aData.eType == SbxOBJECT &&
                    ((SbxValue*) pObj)->aData.pObj == pObj )
                {
#ifdef DISABLE_SCRIPTING // No sbunoobj
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
            // Did we have an array?
            SbxArray* pArray = PTR_CAST(SbxArray,p->aData.pObj);
            if( pArray )
            {
                // When indicated get the parameter
                SbxArray* pPar = NULL;
                SbxVariable* pVar = PTR_CAST(SbxVariable,p);
                if( pVar )
                    pPar = pVar->GetParameters();
                if( pPar )
                {
                    // Did we have a dimensioned array?
                    SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,p->aData.pObj);
                    if( pDimArray )
                        p = pDimArray->Get( pPar );
                    else
                        p = pArray->Get( pPar->Get( 1 )->GetInteger() );
                    break;
                }
            }
            // Elsewise guess a SbxValue
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
        // If there was asked for an object or a VARIANT, don't search
        // the real values
        SbxValue* p = (SbxValue*) this;
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
            // Object contained itself
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

const XubString& SbxValue::GetString() const
{
    SbxValues aRes;
    aRes.eType = SbxSTRING;
    if( Get( aRes ) )
        ((SbxValue*) this)->aToolString = *aRes.pOUString;
    else
        ((SbxValue*) this)->aToolString.Erase();

    return aToolString;
}

const XubString& SbxValue::GetCoreString() const
{
    SbxValues aRes;
    aRes.eType = SbxCoreSTRING;
    if( Get( aRes ) )
        ((SbxValue*) this)->aToolString = *aRes.pOUString;
    else
        ((SbxValue*) this)->aToolString.Erase();

    return aToolString;
}

::rtl::OUString SbxValue::GetOUString() const
{
    ::rtl::OUString aResult;
    SbxValues aRes;
    aRes.eType = SbxSTRING;
    if( Get( aRes ) )
        aResult = *aRes.pOUString;

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
GET( GetChar,     SbxCHAR,       xub_Unicode,   nChar )
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


//////////////////////////// Write data /////////////////////////////

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
        // If there was asked for an object, don't search
        // the real values
        SbxValue* p = this;
        if( rVal.eType != SbxOBJECT )
            p = TheRealValue( sal_False );  // Don't allow an error here
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
                        // is already inside
                        if( p->aData.eType == SbxOBJECT && p->aData.pObj == rVal.pObj )
                            break;

                        // Delete only the value part!
                        p->SbxValue::Clear();

                        // real allocation
                        p->aData.pObj = rVal.pObj;

                        // if necessary cont in Ref-Count
                        if( p->aData.pObj && p->aData.pObj != p )
                        {
                            if ( p != this )
                            {
                                OSL_FAIL( "TheRealValue" );
                            }
                            HACK(nicht bei Parent-Prop - sonst CyclicRef)
                            SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
                            sal_Bool bParentProp = pThisVar && 5345 ==
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

// From 1996-03-28:
// Method to execute a pretreatment of the strings at special types.
// In particular necessary for BASIC-IDE, so that
// the output in the Watch-Window can be writen back with PutStringExt,
// if Float were declared with ',' as the decimal seperator or BOOl
// explicit with "TRUE" or "FALSE".
// Implementation in ImpConvStringExt (SBXSCAN.CXX)
sal_Bool SbxValue::PutStringExt( const ::rtl::OUString& r )
{
    // Copy; if it is Unicode convert it immediately
    ::rtl::OUString aStr( r );

    // Identify the own type (not as in Put() with TheRealValue(),
    // Objects are not handled anyway)
    SbxDataType eTargetType = SbxDataType( aData.eType & 0x0FFF );

    // tinker a Source-Value
    SbxValues aRes;
    aRes.eType = SbxSTRING;

    // Only if really something was converted, take the copy,
    // elsewise take the original (Unicode remain)
    sal_Bool bRet;
    if( ImpConvStringExt( aStr, eTargetType ) )
        aRes.pOUString = (::rtl::OUString*)&aStr;
    else
        aRes.pOUString = (::rtl::OUString*)&r;

    // #34939: Set a Fixed-Flag at Strings. which contain a number, and
    // if this has a Num-Type, so that the type will not be changed
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

    // If it throwed an error with FIXED, set it back
    // (UI-Action should not cast an error, but only fail)
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


// Special decimal methods
sal_Bool SbxValue::PutDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec )
{
    SbxValue::Clear();
    aData.pDecimal = new SbxDecimal( rAutomationDec );
    aData.pDecimal->addRef();
    aData.eType = SbxDECIMAL;
    return sal_True;
}

sal_Bool SbxValue::fillAutomationDecimal
    ( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec )
{
    SbxDecimal* pDecimal = GetDecimal();
    if( pDecimal != NULL )
    {
        pDecimal->fillAutomationDecimal( rAutomationDec );
        return sal_True;
    }
    return sal_False;
}


sal_Bool SbxValue::PutString( const ::rtl::OUString& r )
{
    SbxValues aRes;
    aRes.eType = SbxSTRING;
    aRes.pOUString = (::rtl::OUString*) &r;
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

////////////////////////// Setting of the data type ///////////////////////////

sal_Bool SbxValue::IsFixed() const
{
    return ( (GetFlags() & SBX_FIXED) | (aData.eType & SbxBYREF) ) != 0;
}

// A variable is numeric, if it is EMPTY or realy numeric
// or if it contains a complete convertible String

// #41692, implement it for RTL and Basic-Core seperably
sal_Bool SbxValue::IsNumeric() const
{
    return ImpIsNumeric( /*bOnlyIntntl*/sal_False );
}

sal_Bool SbxValue::IsNumericRTL() const
{
    return ImpIsNumeric( /*bOnlyIntntl*/sal_True );
}

sal_Bool SbxValue::ImpIsNumeric( sal_Bool bOnlyIntntl ) const
{

    if( !CanRead() )
    {
        SetError( SbxERR_PROP_WRITEONLY ); return sal_False;
    }
    // Test downcast!!!
    if( this->ISA(SbxVariable) )
        ((SbxVariable*)this)->Broadcast( SBX_HINT_DATAWANTED );
    SbxDataType t = GetType();
    if( t == SbxSTRING )
    {
        if( aData.pOUString )
        {
            ::rtl::OUString s( *aData.pOUString );
            double n;
            SbxDataType t2;
            sal_uInt16 nLen = 0;
            if( ImpScan( s, n, t2, &nLen, /*bAllowIntntl*/sal_False, bOnlyIntntl ) == SbxERR_OK )
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
    DBG_ASSERT( !( t & 0xF000 ), "Setzen von BYREF|ARRAY verboten!" );
    if( ( t == SbxEMPTY && aData.eType == SbxVOID )
     || ( aData.eType == SbxEMPTY && t == SbxVOID ) )
        return sal_True;
    if( ( t & 0x0FFF ) == SbxVARIANT )
    {
        // Trial to set the data type to Variant
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
            // De-allocate potential objects
            switch( aData.eType )
            {
                case SbxSTRING:
                    delete aData.pOUString;
                    break;
                case SbxOBJECT:
                    if( aData.pObj && aData.pObj != this )
                    {
                        HACK(nicht bei Parent-Prop - sonst CyclicRef)
                        SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
                        sal_uInt16 nSlotId = pThisVar
                                    ? ( (sal_Int16) ( pThisVar->GetUserData() & 0xFFFF ) )
                                    : 0;
                        DBG_ASSERT( nSlotId != 5345 || pThisVar->GetName().EqualsAscii("Parent"),
                                    "SID_PARENTOBJECT heisst nicht 'Parent'" );
                        sal_Bool bParentProp = 5345 == nSlotId;
                        if ( !bParentProp )
                            aData.pObj->ReleaseRef();
                    }
                    break;
                default: break;
            }
            // This works always, because the Float representations are 0 as well.
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
        // Trial to set the data type to Variant
        ResetFlag( SBX_FIXED );
        if( IsFixed() )
        {
            SetError( SbxERR_CONVERSION ); return sal_False;
        }
        else
            return sal_True;
    }
    // Converting from zero doesn't work. Once zero, always zero!
    if( aData.eType == SbxNULL )
    {
        SetError( SbxERR_CONVERSION ); return sal_False;
    }

    // Conversion of the data:
    SbxValues aNew;
    aNew.eType = eTo;
    if( Get( aNew ) )
    {
        // The data type could be converted. It ends here with fixed elements,
        // because the data had not to be taken over
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
////////////////////////////////// Calculating /////////////////////////////////

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
    // Special rule 1: If one operand is zero, the result is zero
    else if( eThisType == SbxNULL || eOpType == SbxNULL )
        SetType( SbxNULL );
    // Special rule 2: If the operand is Empty, the result is the 2. operand
    else if( eThisType == SbxEMPTY
    && !bVBAInterop
    )
        *this = rOp;
    // 1996-2-13: Don't test already before Get upon SbxEMPTY
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
                // From 1999-11-5, keep OUString in mind
                aL.eType = aR.eType = SbxSTRING;
                rOp.Get( aR );
                // From 1999-12-8, #70399: Here call GetType() again, Get() can change the type!
                if( rOp.GetType() == SbxEMPTY )
                    goto Lbl_OpIsEmpty;
                Get( aL );

                // #30576: To begin with test, if the conversion worked
                if( aL.pOUString != NULL && aR.pOUString != NULL )
                {
                    *aL.pOUString += *aR.pOUString;
                }
                // Not even Left OK?
                else if( aL.pOUString == NULL )
                {
                    aL.pOUString = new ::rtl::OUString();
                }
                Put( aL );
            }
            else
                SetError( SbxERR_CONVERSION );
        }
        else if( eOpType == SbxSTRING && rOp.IsFixed() )
        {   // Numeric: there is no String allowed on the right side
            SetError( SbxERR_CONVERSION );
            // falls all the way out
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

            if( rOp.Get( aR ) )     // re-do Get after type assigns above
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
                        // first overflow check: see if product will fit - test real value of product (hence 2 curr factors)
                        dTest = (double)aL.nInt64 * (double)aR.nInt64 / (double)CURRENCY_FACTOR_SQUARE;
                        if( dTest < SbxMINCURR || SbxMAXCURR < dTest)
                        {
                            aL.nInt64 = SAL_MAX_INT64;
                            if( dTest < SbxMINCURR ) aL.nInt64 = SAL_MIN_INT64;
                            SetError( SbxERR_OVERFLOW );
                            break;
                        }
                        // second overflow check: see if unscaled product overflows - if so use doubles
                        dTest = (double)aL.nInt64 * (double)aR.nInt64;
                        if( dTest < SAL_MIN_INT64 || SAL_MAX_INT64 < dTest)
                        {
                            aL.nInt64 = (sal_Int64)( dTest / (double)CURRENCY_FACTOR );
                            break;
                        }
                        // precise calc: multiply then scale back (move decimal pt)
                        aL.nInt64 *= aR.nInt64;
                        aL.nInt64 /= CURRENCY_FACTOR;
                        break;

                    case SbxDIV:
                        if( !aR.nInt64 )
                        {
                            SetError( SbxERR_ZERODIV );
                            break;
                        }
                        // first overflow check: see if quotient will fit - calc real value of quotient (curr factors cancel)
                        dTest = (double)aL.nInt64 / (double)aR.nInt64;
                        if( dTest < SbxMINCURR || SbxMAXCURR < dTest)
                        {
                            SetError( SbxERR_OVERFLOW );
                            break;
                        }
                        // second overflow check: see if scaled dividend overflows - if so use doubles
                        dTest = (double)aL.nInt64 * (double)CURRENCY_FACTOR;
                        if( dTest < SAL_MIN_INT64 || SAL_MAX_INT64 < dTest)
                        {
                            aL.nInt64 = (sal_Int64)(dTest / (double)aR.nInt64);
                            break;
                        }
                        // precise calc: scale (move decimal pt) then divide
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
        {   // other types and operators including Date, Double and Single
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
                    // Date with "+" or "-" needs special handling that
                    // forces the Date type. If the operation is '+' the
                    // result is always a Date, if '-' the result is only
                    // a Date if one of lhs or rhs ( but not both ) is already
                    // a Date
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

// The comparison routine deliver TRUE or FALSE.

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
    // Special rule 1: If an operand is zero, the result is FALSE
    else if( GetType() == SbxNULL || rOp.GetType() == SbxNULL )
        bRes = sal_False;
    // Special rule 2: If both are variant and one is numeric
    // and the other is a String, num is < str
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
        // If one of the operands is a String,
        // a String comparing take place
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
        // From 1995-12-19: If SbxSINGLE participate, then convert to SINGLE,
        //              elsewise it shows a numeric error
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
        // Everything else comparing on a SbxDOUBLE-Basis
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
            // at least one value was got
            // if this is VBA then a conversion error for one
            // side will yield a false result of an equality test
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

///////////////////////////// Reading/Writing ////////////////////////////

sal_Bool SbxValue::LoadData( SvStream& r, sal_uInt16 )
{
    // #TODO see if these types are really dumped to any stream
    // more than likely this is functionality used in the binfilter alone
    SbxValue::Clear();
    sal_uInt16 nType;
    r >> nType;
    aData.eType = SbxDataType( nType );
    switch( nType )
    {
        case SbxBOOL:
        case SbxINTEGER:
            r >> aData.nInteger; break;
        case SbxLONG:
            r >> aData.nLong; break;
        case SbxSINGLE:
        {
            // Floats as ASCII
            XubString aVal = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(r,
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
            // Floats as ASCII
            XubString aVal = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(r,
                RTL_TEXTENCODING_ASCII_US);
            SbxDataType t;
            if( ImpScan( aVal, aData.nDouble, t, NULL ) != SbxERR_OK )
            {
                aData.nDouble = 0.0;
                return sal_False;
            }
            break;
        }
        //#fdo39428 SvStream no longer supports operator>>(long&)
        //SvStream now has operator>>(sal_Int64&)
        case SbxSALINT64:
            r >> aData.nInt64;
            break;
        case SbxSALUINT64:
            r >> aData.uInt64;
            break;
        case SbxCURRENCY:
        {
            sal_uInt32 tmpHi = 0;
            sal_uInt32 tmpLo = 0;
            r >> tmpHi >> tmpLo;
            aData.nInt64 = ((sal_Int64)tmpHi << 32);
            aData.nInt64 |= (sal_Int64)tmpLo;
            break;
        }
        case SbxSTRING:
        {
            rtl::OUString aVal = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(r,
                RTL_TEXTENCODING_ASCII_US);
            if( aVal.getLength() )
                aData.pOUString = new ::rtl::OUString( aVal );
            else
                aData.pOUString = NULL; // JSM 1995-09-22
            break;
        }
        case SbxERROR:
        case SbxUSHORT:
            r >> aData.nUShort; break;
        case SbxOBJECT:
        {
            sal_uInt8 nMode;
            r >> nMode;
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
            r >> c;
            aData.nChar = c;
            break;
        }
        case SbxBYTE:
            r >> aData.nByte; break;
        case SbxULONG:
            r >> aData.nULong; break;
        case SbxINT:
        {
            sal_uInt8 n;
            r >> n;
            // Match the Int on this system?
            if( n > SAL_TYPES_SIZEOFINT )
                r >> aData.nLong, aData.eType = SbxLONG;
            else {
                sal_Int32 nInt;
                r >> nInt;
                aData.nInt = nInt;
            }
            break;
        }
        case SbxUINT:
        {
            sal_uInt8 n;
            r >> n;
            // Match the UInt on this system?
            if( n > SAL_TYPES_SIZEOFINT )
                r >> aData.nULong, aData.eType = SbxULONG;
            else {
                sal_uInt32 nUInt;
                r >> nUInt;
                aData.nUInt = nUInt;
            }
            break;
        }
        case SbxEMPTY:
        case SbxNULL:
        case SbxVOID:
            break;
        case SbxDATAOBJECT:
            r >> aData.nLong;
            break;
        // #78919 For backwards compatibility
        case SbxWSTRING:
        case SbxWCHAR:
            break;
        default:
            memset (&aData,0,sizeof(aData));
            ResetFlag(SBX_FIXED);
            aData.eType = SbxNULL;
            DBG_ASSERT( !this, "Nicht unterstuetzer Datentyp geladen" );
            return sal_False;
    }
    return sal_True;
}

sal_Bool SbxValue::StoreData( SvStream& r ) const
{
    sal_uInt16 nType = sal::static_int_cast< sal_uInt16 >(aData.eType);
    r << nType;
    switch( nType & 0x0FFF )
    {
        case SbxBOOL:
        case SbxINTEGER:
            r << aData.nInteger; break;
        case SbxLONG:
            r << aData.nLong; break;
        case SbxDATE:
            // #49935: Save as double, elsewise an error during the read in
            ((SbxValue*)this)->aData.eType = (SbxDataType)( ( nType & 0xF000 ) | SbxDOUBLE );
            write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(r, GetCoreString(), RTL_TEXTENCODING_ASCII_US);
            ((SbxValue*)this)->aData.eType = (SbxDataType)nType;
            break;
        case SbxSINGLE:
        case SbxDOUBLE:
            write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(r, GetCoreString(), RTL_TEXTENCODING_ASCII_US);
            break;
        case SbxSALUINT64:
        case SbxSALINT64:
            // see comment in SbxValue::StoreData
            r << aData.uInt64;
            break;
        case SbxCURRENCY:
        {
            sal_Int32 tmpHi = ( (aData.nInt64 >> 32) &  0xFFFFFFFF );
            sal_Int32 tmpLo = ( sal_Int32 )aData.nInt64;
            r << tmpHi << tmpLo;
            break;
        }
        case SbxSTRING:
            if( aData.pOUString )
            {
                write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(r, *aData.pOUString, RTL_TEXTENCODING_ASCII_US);
            }
            else
            {
                write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(r, rtl::OUString(), RTL_TEXTENCODING_ASCII_US);
            }
            break;
        case SbxERROR:
        case SbxUSHORT:
            r << aData.nUShort; break;
        case SbxOBJECT:
            // to save itself as Objectptr does not work!
            if( aData.pObj )
            {
                if( PTR_CAST(SbxValue,aData.pObj) != this )
                {
                    r << (sal_uInt8) 1;
                    return aData.pObj->Store( r );
                }
                else
                    r << (sal_uInt8) 2;
            }
            else
                r << (sal_uInt8) 0;
            break;
        case SbxCHAR:
        {
            char c = sal::static_int_cast< char >(aData.nChar);
            r << c;
            break;
        }
        case SbxBYTE:
            r << aData.nByte; break;
        case SbxULONG:
            r << aData.nULong; break;
        case SbxINT:
        {
            sal_uInt8 n = SAL_TYPES_SIZEOFINT;
            r << n << (sal_Int32)aData.nInt;
            break;
        }
        case SbxUINT:
        {
            sal_uInt8 n = SAL_TYPES_SIZEOFINT;
            r << n << (sal_uInt32)aData.nUInt;
            break;
        }
        case SbxEMPTY:
        case SbxNULL:
        case SbxVOID:
            break;
        case SbxDATAOBJECT:
            r << aData.nLong;
            break;
        // #78919 For backwards compatibility
        case SbxWSTRING:
        case SbxWCHAR:
            break;
        default:
            DBG_ASSERT( !this, "Speichern eines nicht unterstuetzten Datentyps" );
            return sal_False;
    }
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
