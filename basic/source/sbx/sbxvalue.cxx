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

#include <config_features.h>

#include <math.h>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <sal/log.hxx>

#include <basic/sbx.hxx>
#include <sbunoobj.hxx>
#include "sbxconv.hxx"
#include <runtime.hxx>


///////////////////////////// constructors

SbxValue::SbxValue() : SbxBase()
{
    aData.eType = SbxEMPTY;
}

SbxValue::SbxValue( SbxDataType t ) : SbxBase()
{
    int n = t & 0x0FFF;

    if( n == SbxVARIANT )
        n = SbxEMPTY;
    else
        SetFlag( SbxFlagBits::Fixed );
    aData.clear(SbxDataType( n ));
}

SbxValue::SbxValue( const SbxValue& r )
    : SvRefBase( r ), SbxBase( r )
{
    if( !r.CanRead() )
    {
        SetError( ERRCODE_BASIC_PROP_WRITEONLY );
        if( !IsFixed() )
            aData.eType = SbxNULL;
    }
    else
    {
        const_cast<SbxValue*>(&r)->Broadcast( SfxHintId::BasicDataWanted );
        aData = r.aData;
        // Copy pointer, increment references
        switch( aData.eType )
        {
            case SbxSTRING:
                if( aData.pOUString )
                    aData.pOUString = new OUString( *aData.pOUString );
                break;
            case SbxOBJECT:
                if( aData.pObj )
                    aData.pObj->AddFirstRef();
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
            SetError( ERRCODE_BASIC_PROP_READONLY );
        else
        {
            // string -> byte array
            if( IsFixed() && (aData.eType == SbxOBJECT)
                && aData.pObj && ( aData.pObj->GetType() == (SbxARRAY | SbxBYTE) )
                && (r.aData.eType == SbxSTRING) )
            {
                OUString aStr = r.GetOUString();
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
                SbxArray* pArr = dynamic_cast<SbxArray*>( pObj );
                if( pArr )
                {
                    OUString aStr = ByteArrayToString( pArr );
                    PutString(aStr);
                    return *this;
                }
            }
            // Readout the content of the variables
            SbxValues aNew;
            if( IsFixed() )
                // then the type has to match
                aNew.eType = aData.eType;
            else if( r.IsFixed() )
                // Source fixed: copy the type
                aNew.eType = SbxDataType( r.aData.eType & 0x0FFF );
            else
                // both variant: then don't care
                aNew.eType = SbxVARIANT;
            if( r.Get( aNew ) )
                Put( aNew );
        }
    }
    return *this;
}

SbxValue::~SbxValue()
{
    SetFlag( SbxFlagBits::Write );
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
            delete aData.pOUString; aData.pOUString = nullptr;
            break;
        case SbxOBJECT:
            if( aData.pObj )
            {
                if( aData.pObj != this )
                {
                    SAL_INFO("basic.sbx", "Not at Parent-Prop - otherwise CyclicRef");
                    SbxVariable *pThisVar = dynamic_cast<SbxVariable*>( this );
                    bool bParentProp = pThisVar && static_cast<sal_uInt16>(pThisVar->GetUserData()) ==
                        5345;
                    if ( !bParentProp )
                        aData.pObj->ReleaseRef();
                }
                aData.pObj = nullptr;
            }
            break;
        case SbxDECIMAL:
            releaseDecimalPtr( aData.pDecimal );
            break;
        case SbxDATAOBJECT:
            aData.pData = nullptr; break;
        default:
        {
            SbxValues aEmpty;
            aEmpty.clear(GetType());
            Put( aEmpty );
        }
    }
}

// Dummy

void SbxValue::Broadcast( SfxHintId )
{}

//////////////////////////// Readout data

// Detect the "right" variables. If it is an object, will be addressed either
// the object itself or its default property.
// If the variable contain a variable or an object, this will be
// addressed.

SbxValue* SbxValue::TheRealValue( bool bObjInObjError ) const
{
    SbxValue* p = const_cast<SbxValue*>(this);
    for( ;; )
    {
        SbxDataType t = SbxDataType( p->aData.eType & 0x0FFF );
        if( t == SbxOBJECT )
        {
            // The block contains an object or a variable
            SbxObject* pObj = dynamic_cast<SbxObject*>( p->aData.pObj );
            if( pObj )
            {
                // Has the object a default property?
                SbxVariable* pDflt = pObj->GetDfltProperty();

                // If this is an object and contains itself,
                // we cannot access on it
                // The old condition to set an error is not correct,
                // because e.g. a regular variant variable with an object
                // could be affected if another value should be assigned.
                // Therefore with flag.
                if( bObjInObjError && !pDflt &&
                    static_cast<SbxValue*>(pObj)->aData.eType == SbxOBJECT &&
                    static_cast<SbxValue*>(pObj)->aData.pObj == pObj )
                {
#if !HAVE_FEATURE_SCRIPTING
                    const bool bSuccess = false;
#else
                    bool bSuccess = handleToStringForCOMObjects( pObj, p );
#endif
                    if( !bSuccess )
                    {
                        SetError( ERRCODE_BASIC_BAD_PROP_VALUE );
                        p = nullptr;
                    }
                }
                else if( pDflt )
                    p = pDflt;
                break;
            }
            // Did we have an array?
            SbxArray* pArray = dynamic_cast<SbxArray*>( p->aData.pObj );
            if( pArray )
            {
                // When indicated get the parameter
                SbxArray* pPar = nullptr;
                SbxVariable* pVar = dynamic_cast<SbxVariable*>( p );
                if( pVar )
                    pPar = pVar->GetParameters();
                if( pPar )
                {
                    // Did we have a dimensioned array?
                    SbxDimArray* pDimArray = dynamic_cast<SbxDimArray*>( p->aData.pObj );
                    if( pDimArray )
                        p = pDimArray->Get( pPar );
                    else
                        p = pArray->Get( pPar->Get( 1 )->GetInteger() );
                    break;
                }
            }
            // Otherwise guess a SbxValue
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->aData.pObj );
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

bool SbxValue::Get( SbxValues& rRes ) const
{
    bool bRes = false;
    ErrCode eOld = GetError();
    if( eOld != ERRCODE_NONE )
        ResetError();
    if( !CanRead() )
    {
        SetError( ERRCODE_BASIC_PROP_WRITEONLY );
        rRes.pObj = nullptr;
    }
    else
    {
        // If an object or a VARIANT is requested, don't search the real values
        SbxValue* p = const_cast<SbxValue*>(this);
        if( rRes.eType != SbxOBJECT && rRes.eType != SbxVARIANT )
            p = TheRealValue( true );
        if( p )
        {
            p->Broadcast( SfxHintId::BasicDataWanted );
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
                    rRes.nInt = static_cast<int>(ImpGetLong( &p->aData ));
                    break;
                case SbxUINT:
                    rRes.nUInt = static_cast<int>(ImpGetULong( &p->aData ));
                    break;
                case SbxOBJECT:
                    if( p->aData.eType == SbxOBJECT )
                        rRes.pObj = p->aData.pObj;
                    else
                    {
                        SetError( ERRCODE_BASIC_NO_OBJECT );
                        rRes.pObj = nullptr;
                    }
                    break;
                default:
                    if( p->aData.eType == rRes.eType )
                        rRes = p->aData;
                    else
                    {
                        SetError( ERRCODE_BASIC_CONVERSION );
                        rRes.pObj = nullptr;
                    }
            }
        }
        else
        {
            // Object contained itself
            SbxDataType eTemp = rRes.eType;
            rRes.clear(eTemp);
        }
    }
    if( !IsError() )
    {
        bRes = true;
        if( eOld != ERRCODE_NONE )
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
        const_cast<SbxValue*>(this)->aToolString = *aRes.pOUString;
    }
    else
    {
        const_cast<SbxValue*>(this)->aToolString.clear();
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

bool SbxValue::GetBool() const
{
    SbxValues aRes;
    aRes.eType = SbxBOOL;
    Get( aRes );
    return aRes.nUShort != 0;
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


//////////////////////////// Write data

bool SbxValue::Put( const SbxValues& rVal )
{
    bool bRes = false;
    ErrCode eOld = GetError();
    if( eOld != ERRCODE_NONE )
        ResetError();
    if( !CanWrite() )
        SetError( ERRCODE_BASIC_PROP_READONLY );
    else if( rVal.eType & 0xF000 )
        SetError( ERRCODE_BASIC_BAD_ARGUMENT );
    else
    {
        // If an object is requested, don't search the real values
        SbxValue* p = this;
        if( rVal.eType != SbxOBJECT )
            p = TheRealValue( false );  // Don't allow an error here
        if( p )
        {
            if( !p->CanWrite() )
                SetError( ERRCODE_BASIC_PROP_READONLY );
            else if( p->IsFixed() || p->SetType( static_cast<SbxDataType>( rVal.eType & 0x0FFF ) ) )
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
                    ImpPutLong( &p->aData, static_cast<sal_Int32>(rVal.nInt) );
                    break;
                case SbxUINT:
                    ImpPutULong( &p->aData, static_cast<sal_uInt32>(rVal.nUInt) );
                    break;
                case SbxOBJECT:
                    if( !p->IsFixed() || p->aData.eType == SbxOBJECT )
                    {
                        // is already inside
                        if( p->aData.eType == SbxOBJECT && p->aData.pObj == rVal.pObj )
                            break;

                        // Delete only the value part!
                        p->SbxValue::Clear();

                        // real assignment
                        p->aData.pObj = rVal.pObj;

                        // if necessary increment Ref-Count
                        if( p->aData.pObj && p->aData.pObj != p )
                        {
                            if ( p != this )
                            {
                                OSL_FAIL( "TheRealValue" );
                            }
                            SAL_INFO("basic.sbx", "Not at Parent-Prop - otherwise CyclicRef");
                            SbxVariable *pThisVar = dynamic_cast<SbxVariable*>( this );
                            bool bParentProp = pThisVar && static_cast<sal_uInt16>(pThisVar->GetUserData()) ==
                                5345;
                            if ( !bParentProp )
                                p->aData.pObj->AddFirstRef();
                        }
                    }
                    else
                        SetError( ERRCODE_BASIC_CONVERSION );
                    break;
                default:
                    if( p->aData.eType == rVal.eType )
                        p->aData = rVal;
                    else
                    {
                        SetError( ERRCODE_BASIC_CONVERSION );
                        if( !p->IsFixed() )
                            p->aData.eType = SbxNULL;
                    }
            }
            if( !IsError() )
            {
                p->SetModified( true );
                p->Broadcast( SfxHintId::BasicDataChanged );
                if( eOld != ERRCODE_NONE )
                    SetError( eOld );
                bRes = true;
            }
        }
    }
    return bRes;
}

// From 1996-03-28:
// Method to execute a pretreatment of the strings at special types.
// In particular necessary for BASIC-IDE, so that
// the output in the Watch-Window can be written back with PutStringExt,
// if Float were declared with ',' as the decimal separator or BOOl
// explicit with "TRUE" or "FALSE".
// Implementation in ImpConvStringExt (SBXSCAN.CXX)
void SbxValue::PutStringExt( const OUString& r )
{
    // Copy; if it is Unicode convert it immediately
    OUString aStr( r );

    // Identify the own type (not as in Put() with TheRealValue(),
    // Objects are not handled anyway)
    SbxDataType eTargetType = SbxDataType( aData.eType & 0x0FFF );

    // tinker a Source-Value
    SbxValues aRes;
    aRes.eType = SbxSTRING;

    // Only if really something was converted, take the copy,
    // otherwise take the original (Unicode remains)
    bool bRet;
    if( ImpConvStringExt( aStr, eTargetType ) )
        aRes.pOUString = &aStr;
    else
        aRes.pOUString = const_cast<OUString*>(&r);

    // #34939: For Strings which contain a number, and if this has a Num-Type,
    // set a Fixed flag so that the type will not be changed
    SbxFlagBits nFlags_ = GetFlags();
    if( ( eTargetType >= SbxINTEGER && eTargetType <= SbxCURRENCY ) ||
        ( eTargetType >= SbxCHAR && eTargetType <= SbxUINT ) ||
        eTargetType == SbxBOOL )
    {
        SbxValue aVal;
        aVal.Put( aRes );
        if( aVal.IsNumeric() )
            SetFlag( SbxFlagBits::Fixed );
    }

    Put( aRes );
    bRet = bool( !IsError() );

    // If FIXED resulted in an error, set it back
    // (UI-Action should not result in an error, but simply fail)
    if( !bRet )
        ResetError();

    SetFlags( nFlags_ );
}

bool SbxValue::PutBool( bool b )
{
    SbxValues aRes;
    aRes.eType = SbxBOOL;
    aRes.nUShort = sal::static_int_cast< sal_uInt16 >(b ? SbxTRUE : SbxFALSE);
    Put( aRes );
    return !IsError();
}

bool SbxValue::PutEmpty()
{
    bool bRet = SetType( SbxEMPTY );
    SetModified( true );
    return bRet;
}

void SbxValue::PutNull()
{
    bool bRet = SetType( SbxNULL );
    if( bRet )
        SetModified( true );
}


// Special decimal methods
void SbxValue::PutDecimal( css::bridge::oleautomation::Decimal const & rAutomationDec )
{
    SbxValue::Clear();
    aData.pDecimal = new SbxDecimal( rAutomationDec );
    aData.pDecimal->addRef();
    aData.eType = SbxDECIMAL;
}

void SbxValue::fillAutomationDecimal
    ( css::bridge::oleautomation::Decimal& rAutomationDec ) const
{
    SbxDecimal* pDecimal = GetDecimal();
    if( pDecimal != nullptr )
    {
        pDecimal->fillAutomationDecimal( rAutomationDec );
    }
}


bool SbxValue::PutString( const OUString& r )
{
    SbxValues aRes;
    aRes.eType = SbxSTRING;
    aRes.pOUString = const_cast<OUString*>(&r);
    Put( aRes );
    return !IsError();
}


#define PUT( p, e, t, m ) \
bool SbxValue::p( t n ) \
{ SbxValues aRes(e); aRes.m = n; Put( aRes ); return !IsError(); }

void SbxValue::PutDate( double n )
{ SbxValues aRes(SbxDATE); aRes.nDouble = n; Put( aRes ); }
void SbxValue::PutErr( sal_uInt16 n )
{ SbxValues aRes(SbxERROR); aRes.nUShort = n; Put( aRes ); }

PUT( PutByte,     SbxBYTE,       sal_uInt8,        nByte )
PUT( PutChar,     SbxCHAR,       sal_Unicode,      nChar )
PUT( PutCurrency, SbxCURRENCY,   sal_Int64,        nInt64 )
PUT( PutDouble,   SbxDOUBLE,     double,           nDouble )
PUT( PutInteger,  SbxINTEGER,    sal_Int16,        nInteger )
PUT( PutLong,     SbxLONG,       sal_Int32,        nLong )
PUT( PutObject,   SbxOBJECT,     SbxBase*,         pObj )
PUT( PutSingle,   SbxSINGLE,     float,            nSingle )
PUT( PutULong,    SbxULONG,      sal_uInt32,       nULong )
PUT( PutUShort,   SbxUSHORT,     sal_uInt16,       nUShort )
PUT( PutInt64,    SbxSALINT64,   sal_Int64,        nInt64 )
PUT( PutUInt64,   SbxSALUINT64,  sal_uInt64,       uInt64 )
PUT( PutDecimal,  SbxDECIMAL,    SbxDecimal*,      pDecimal )

////////////////////////// Setting of the data type

bool SbxValue::IsFixed() const
{
    return (GetFlags() & SbxFlagBits::Fixed) || ((aData.eType & SbxBYREF) != 0);
}

// A variable is numeric, if it is EMPTY or really numeric
// or if it contains a complete convertible String

// #41692, implement it for RTL and Basic-Core separately
bool SbxValue::IsNumeric() const
{
    return ImpIsNumeric( /*bOnlyIntntl*/false );
}

bool SbxValue::IsNumericRTL() const
{
    return ImpIsNumeric( /*bOnlyIntntl*/true );
}

bool SbxValue::ImpIsNumeric( bool bOnlyIntntl ) const
{

    if( !CanRead() )
    {
        SetError( ERRCODE_BASIC_PROP_WRITEONLY );
        return false;
    }
    // Test downcast!!!
    if( dynamic_cast<const SbxVariable*>( this) != nullptr )
        const_cast<SbxVariable*>(static_cast<const SbxVariable*>(this))->Broadcast( SfxHintId::BasicDataWanted );
    SbxDataType t = GetType();
    if( t == SbxSTRING )
    {
        if( aData.pOUString )
        {
            OUString s( *aData.pOUString );
            double n;
            SbxDataType t2;
            sal_uInt16 nLen = 0;
            if( ImpScan( s, n, t2, &nLen, bOnlyIntntl ) == ERRCODE_NONE )
                return nLen == s.getLength();
        }
        return false;
    }
    else
        return t == SbxEMPTY
            || ( t >= SbxINTEGER && t <= SbxCURRENCY )
            || ( t >= SbxCHAR && t <= SbxUINT );
}

SbxClassType SbxValue::GetClass() const
{
    return SbxClassType::Value;
}

SbxDataType SbxValue::GetType() const
{
    return SbxDataType( aData.eType & 0x0FFF );
}


bool SbxValue::SetType( SbxDataType t )
{
    DBG_ASSERT( !( t & 0xF000 ), "SetType of BYREF|ARRAY is forbidden!" );
    if( ( t == SbxEMPTY && aData.eType == SbxVOID )
     || ( aData.eType == SbxEMPTY && t == SbxVOID ) )
        return true;
    if( ( t & 0x0FFF ) == SbxVARIANT )
    {
        // Try to set the data type to Variant
        ResetFlag( SbxFlagBits::Fixed );
        if( IsFixed() )
        {
            SetError( ERRCODE_BASIC_CONVERSION );
            return false;
        }
        t = SbxEMPTY;
    }
    if( ( t & 0x0FFF ) != ( aData.eType & 0x0FFF ) )
    {
        if( !CanWrite() || IsFixed() )
        {
            SetError( ERRCODE_BASIC_CONVERSION );
            return false;
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
                        SAL_WARN("basic.sbx", "Not at Parent-Prop - otherwise CyclicRef");
                        SbxVariable *pThisVar = dynamic_cast<SbxVariable*>( this );
                        sal_uInt16 nSlotId = pThisVar
                                    ? static_cast<sal_uInt16>(pThisVar->GetUserData())
                                    : 0;
                        DBG_ASSERT( nSlotId != 5345 || pThisVar->GetName() == "Parent",
                                    "SID_PARENTOBJECT is not named 'Parent'" );
                        bool bParentProp = nSlotId == 5345;
                        if ( !bParentProp )
                            aData.pObj->ReleaseRef();
                    }
                    break;
                default: break;
            }
            aData.clear(t);
        }
    }
    return true;
}

bool SbxValue::Convert( SbxDataType eTo )
{
    eTo = SbxDataType( eTo & 0x0FFF );
    if( ( aData.eType & 0x0FFF ) == eTo )
        return true;
    if( !CanWrite() )
        return false;
    if( eTo == SbxVARIANT )
    {
        // Trial to set the data type to Variant
        ResetFlag( SbxFlagBits::Fixed );
        if( IsFixed() )
        {
            SetError( ERRCODE_BASIC_CONVERSION );
            return false;
        }
        else
            return true;
    }
    // Converting from null doesn't work. Once null, always null!
    if( aData.eType == SbxNULL )
    {
        SetError( ERRCODE_BASIC_CONVERSION );
        return false;
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
            SetModified( true );
        }
        return true;
    }
    else
        return false;
}
////////////////////////////////// Calculating

bool SbxValue::Compute( SbxOperator eOp, const SbxValue& rOp )
{
#if !HAVE_FEATURE_SCRIPTING
    const bool bVBAInterop = false;
#else
    bool bVBAInterop =  SbiRuntime::isVBAEnabled();
#endif
    SbxDataType eThisType = GetType();
    SbxDataType eOpType = rOp.GetType();
    ErrCode eOld = GetError();
    if( eOld != ERRCODE_NONE )
        ResetError();
    if( !CanWrite() )
        SetError( ERRCODE_BASIC_PROP_READONLY );
    else if( !rOp.CanRead() )
        SetError( ERRCODE_BASIC_PROP_WRITEONLY );
    // Special rule 1: If one operand is null, the result is null
    else if( eThisType == SbxNULL || eOpType == SbxNULL )
        SetType( SbxNULL );
    // Special rule 2: If the operand is Empty, the result is the 2. operand
    else if( eThisType == SbxEMPTY
    && !bVBAInterop
    )
        *this = rOp;
    // 1996-2-13: Don't test for SbxEMPTY before Get
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
                if( aL.pOUString != nullptr && aR.pOUString != nullptr )
                {
                    // tdf#108039: catch possible bad_alloc
                    try {
                        *aL.pOUString += *aR.pOUString;
                    }
                    catch (const std::bad_alloc&) {
                        SetError(ERRCODE_BASIC_MATH_OVERFLOW);
                    }
                }
                // Not even Left OK?
                else if( aL.pOUString == nullptr )
                {
                    aL.pOUString = new OUString();
                }
            }
            else
                SetError( ERRCODE_BASIC_CONVERSION );
        }
        else if( eOpType == SbxSTRING && rOp.IsFixed() )
        {   // Numeric: there is no String allowed on the right side
            SetError( ERRCODE_BASIC_CONVERSION );
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
                    if ( !bVBAInterop || (  eOp != SbxNOT ) )
                        goto Lbl_OpIsEmpty;
                }
                if( Get( aL ) ) switch( eOp )
                {
                    case SbxIDIV:
                        if( aL.eType == SbxCURRENCY )
                            if( !aR.nInt64 ) SetError( ERRCODE_BASIC_ZERODIV );
                            else {
                                aL.nInt64 /= aR.nInt64;
                                aL.nInt64 *= CURRENCY_FACTOR;
                        }
                        else if( aL.eType == SbxSALUINT64 )
                            if( !aR.uInt64 ) SetError( ERRCODE_BASIC_ZERODIV );
                            else aL.uInt64 /= aR.uInt64;
                        else if( aL.eType == SbxSALINT64 )
                            if( !aR.nInt64 ) SetError( ERRCODE_BASIC_ZERODIV );
                            else aL.nInt64 /= aR.nInt64;
                        else if( aL.eType == SbxLONG )
                            if( !aR.nLong ) SetError( ERRCODE_BASIC_ZERODIV );
                            else aL.nLong /= aR.nLong;
                        else
                            if( !aR.nULong ) SetError( ERRCODE_BASIC_ZERODIV );
                            else aL.nULong /= aR.nULong;
                        break;
                    case SbxMOD:
                        if( aL.eType == SbxCURRENCY || aL.eType == SbxSALINT64 )
                            if( !aR.nInt64 ) SetError( ERRCODE_BASIC_ZERODIV );
                            else aL.nInt64 %= aR.nInt64;
                        else if( aL.eType == SbxSALUINT64 )
                            if( !aR.uInt64 ) SetError( ERRCODE_BASIC_ZERODIV );
                            else aL.uInt64 %= aR.uInt64;
                        else if( aL.eType == SbxLONG )
                            if( !aR.nLong ) SetError( ERRCODE_BASIC_ZERODIV );
                            else aL.nLong %= aR.nLong;
                        else
                            if( !aR.nULong ) SetError( ERRCODE_BASIC_ZERODIV );
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
                                    SetError( ERRCODE_BASIC_ZERODIV );
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
                                SetError( ERRCODE_BASIC_BAD_ARGUMENT );
                        }
                        if( !bOk )
                            SetError( ERRCODE_BASIC_MATH_OVERFLOW );
                    }
                    else
                    {
                        SetError( ERRCODE_BASIC_CONVERSION );
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
                    case SbxMUL:
                        {
                            // first overflow check: see if product will fit - test real value of product (hence 2 curr factors)
                            double dTest = static_cast<double>(aL.nInt64) * static_cast<double>(aR.nInt64) / double(CURRENCY_FACTOR_SQUARE);
                            if( dTest < SbxMINCURR || SbxMAXCURR < dTest)
                            {
                                aL.nInt64 = SAL_MAX_INT64;
                                if( dTest < SbxMINCURR ) aL.nInt64 = SAL_MIN_INT64;
                                SetError( ERRCODE_BASIC_MATH_OVERFLOW );
                                break;
                            }
                            // second overflow check: see if unscaled product overflows - if so use doubles
                            dTest = static_cast<double>(aL.nInt64) * static_cast<double>(aR.nInt64);
                            if( dTest < SAL_MIN_INT64 || SAL_MAX_INT64 < dTest)
                            {
                                aL.nInt64 = static_cast<sal_Int64>( dTest / double(CURRENCY_FACTOR) );
                                break;
                            }
                            // precise calc: multiply then scale back (move decimal pt)
                            aL.nInt64 *= aR.nInt64;
                            aL.nInt64 /= CURRENCY_FACTOR;
                            break;
                        }

                    case SbxDIV:
                        {
                            if( !aR.nInt64 )
                            {
                                SetError( ERRCODE_BASIC_ZERODIV );
                                break;
                            }
                            // first overflow check: see if quotient will fit - calc real value of quotient (curr factors cancel)
                            double dTest = static_cast<double>(aL.nInt64) / static_cast<double>(aR.nInt64);
                            if( dTest < SbxMINCURR || SbxMAXCURR < dTest)
                            {
                                SetError( ERRCODE_BASIC_MATH_OVERFLOW );
                                break;
                            }
                            // second overflow check: see if scaled dividend overflows - if so use doubles
                            dTest = static_cast<double>(aL.nInt64) * double(CURRENCY_FACTOR);
                            if( dTest < SAL_MIN_INT64 || SAL_MAX_INT64 < dTest)
                            {
                                aL.nInt64 = static_cast<sal_Int64>(dTest / static_cast<double>(aR.nInt64));
                                break;
                            }
                            // precise calc: scale (move decimal pt) then divide
                            aL.nInt64 *= CURRENCY_FACTOR;
                            aL.nInt64 /= aR.nInt64;
                            break;
                        }

                    case SbxPLUS:
                        {
                            double dTest = ( static_cast<double>(aL.nInt64) + static_cast<double>(aR.nInt64) ) / double(CURRENCY_FACTOR);
                            if( dTest < SbxMINCURR || SbxMAXCURR < dTest)
                            {
                                SetError( ERRCODE_BASIC_MATH_OVERFLOW );
                                break;
                            }
                            aL.nInt64 += aR.nInt64;
                            break;
                        }

                    case SbxMINUS:
                        {
                            double dTest = ( static_cast<double>(aL.nInt64) - static_cast<double>(aR.nInt64) ) / double(CURRENCY_FACTOR);
                            if( dTest < SbxMINCURR || SbxMAXCURR < dTest)
                            {
                                SetError( ERRCODE_BASIC_MATH_OVERFLOW );
                                break;
                            }
                            aL.nInt64 -= aR.nInt64;
                            break;
                        }
                    case SbxNEG:
                        aL.nInt64 = -aL.nInt64;
                        break;
                    default:
                        SetError( ERRCODE_BASIC_BAD_ARGUMENT );
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
                    if ( !bVBAInterop || ( eOp != SbxNEG ) )
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
                            if( !aR.nDouble ) SetError( ERRCODE_BASIC_ZERODIV );
                            else aL.nDouble /= aR.nDouble;
                            break;
                        case SbxPLUS:
                            aL.nDouble += aR.nDouble; break;
                        case SbxMINUS:
                            aL.nDouble -= aR.nDouble; break;
                        case SbxNEG:
                            aL.nDouble = -aL.nDouble; break;
                        default:
                            SetError( ERRCODE_BASIC_BAD_ARGUMENT );
                    }
                    // Date with "+" or "-" needs special handling that
                    // forces the Date type. If the operation is '+' the
                    // result is always a Date, if '-' the result is only
                    // a Date if one of lhs or rhs ( but not both ) is already
                    // a Date
                    if( GetType() == SbxDATE || rOp.GetType() == SbxDATE )
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

    bool bRes = !IsError();
    if( bRes && eOld != ERRCODE_NONE )
        SetError( eOld );
    return bRes;
}

// The comparison routine deliver TRUE or FALSE.

bool SbxValue::Compare( SbxOperator eOp, const SbxValue& rOp ) const
{
#if !HAVE_FEATURE_SCRIPTING
    const bool bVBAInterop = false;
#else
    bool bVBAInterop =  SbiRuntime::isVBAEnabled();
#endif

    bool bRes = false;
    ErrCode eOld = GetError();
    if( eOld != ERRCODE_NONE )
        ResetError();
    if( !CanRead() || !rOp.CanRead() )
        SetError( ERRCODE_BASIC_PROP_WRITEONLY );
    else if( GetType() == SbxNULL && rOp.GetType() == SbxNULL && !bVBAInterop )
    {
        bRes = true;
    }
    else if( GetType() == SbxEMPTY && rOp.GetType() == SbxEMPTY )
        bRes = !bVBAInterop || ( eOp == SbxEQ );
    // Special rule 1: If an operand is null, the result is FALSE
    else if( GetType() == SbxNULL || rOp.GetType() == SbxNULL )
        bRes = false;
    // Special rule 2: If both are variant and one is numeric
    // and the other is a String, num is < str
    else if( !IsFixed() && !rOp.IsFixed()
     && ( rOp.GetType() == SbxSTRING && GetType() != SbxSTRING && IsNumeric() ) && !bVBAInterop
    )
        bRes = eOp == SbxLT || eOp == SbxLE || eOp == SbxNE;
    else if( !IsFixed() && !rOp.IsFixed()
     && ( GetType() == SbxSTRING && rOp.GetType() != SbxSTRING && rOp.IsNumeric() )
&& !bVBAInterop
    )
        bRes = eOp == SbxGT || eOp == SbxGE || eOp == SbxNE;
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
                    bRes = ( *aL.pOUString == *aR.pOUString ); break;
                case SbxNE:
                    bRes = ( *aL.pOUString != *aR.pOUString ); break;
                case SbxLT:
                    bRes = ( *aL.pOUString <  *aR.pOUString ); break;
                case SbxGT:
                    bRes = ( *aL.pOUString >  *aR.pOUString ); break;
                case SbxLE:
                    bRes = ( *aL.pOUString <= *aR.pOUString ); break;
                case SbxGE:
                    bRes = ( *aL.pOUString >= *aR.pOUString ); break;
                default:
                    SetError( ERRCODE_BASIC_BAD_ARGUMENT );
            }
        }
        // From 1995-12-19: If SbxSINGLE participate, then convert to SINGLE,
        //              otherwise it shows a numeric error
        else if( GetType() == SbxSINGLE || rOp.GetType() == SbxSINGLE )
        {
            aL.eType = aR.eType = SbxSINGLE;
            if( Get( aL ) && rOp.Get( aR ) )
              switch( eOp )
            {
                case SbxEQ:
                    bRes = ( aL.nSingle == aR.nSingle ); break;
                case SbxNE:
                    bRes = ( aL.nSingle != aR.nSingle ); break;
                case SbxLT:
                    bRes = ( aL.nSingle <  aR.nSingle ); break;
                case SbxGT:
                    bRes = ( aL.nSingle >  aR.nSingle ); break;
                case SbxLE:
                    bRes = ( aL.nSingle <= aR.nSingle ); break;
                case SbxGE:
                    bRes = ( aL.nSingle >= aR.nSingle ); break;
                default:
                    SetError( ERRCODE_BASIC_BAD_ARGUMENT );
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
                        bRes = ( eRes == SbxDecimal::CmpResult::EQ ); break;
                    case SbxNE:
                        bRes = ( eRes != SbxDecimal::CmpResult::EQ ); break;
                    case SbxLT:
                        bRes = ( eRes == SbxDecimal::CmpResult::LT ); break;
                    case SbxGT:
                        bRes = ( eRes == SbxDecimal::CmpResult::GT ); break;
                    case SbxLE:
                        bRes = ( eRes != SbxDecimal::CmpResult::GT ); break;
                    case SbxGE:
                        bRes = ( eRes != SbxDecimal::CmpResult::LT ); break;
                    default:
                        SetError( ERRCODE_BASIC_BAD_ARGUMENT );
                }
            }
            else
            {
                SetError( ERRCODE_BASIC_CONVERSION );
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
                    bRes = ( aL.nDouble == aR.nDouble ); break;
                case SbxNE:
                    bRes = ( aL.nDouble != aR.nDouble ); break;
                case SbxLT:
                    bRes = ( aL.nDouble <  aR.nDouble ); break;
                case SbxGT:
                    bRes = ( aL.nDouble >  aR.nDouble ); break;
                case SbxLE:
                    bRes = ( aL.nDouble <= aR.nDouble ); break;
                case SbxGE:
                    bRes = ( aL.nDouble >= aR.nDouble ); break;
                default:
                    SetError( ERRCODE_BASIC_BAD_ARGUMENT );
            }
            // at least one value was got
            // if this is VBA then a conversion error for one
            // side will yield a false result of an equality test
            else if ( bGetR || bGetL )
            {
                if ( bVBAInterop && eOp == SbxEQ && GetError() == ERRCODE_BASIC_CONVERSION )
                {
#ifndef IOS
                    ResetError();
                    bRes = false;
#endif
                }
            }
        }
    }
    if( eOld != ERRCODE_NONE )
        SetError( eOld );
    return bRes;
}

///////////////////////////// Reading/Writing

bool SbxValue::LoadData( SvStream& r, sal_uInt16 )
{
    // #TODO see if these types are really dumped to any stream
    // more than likely this is functionality used in the binfilter alone
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
            // Floats as ASCII
            OUString aVal = read_uInt16_lenPrefixed_uInt8s_ToOUString(r,
                RTL_TEXTENCODING_ASCII_US);
            double d;
            SbxDataType t;
            if( ImpScan( aVal, d, t, nullptr, false ) != ERRCODE_NONE || t == SbxDOUBLE )
            {
                aData.nSingle = 0.0F;
                return false;
            }
            aData.nSingle = static_cast<float>(d);
            break;
        }
        case SbxDATE:
        case SbxDOUBLE:
        {
            // Floats as ASCII
            OUString aVal = read_uInt16_lenPrefixed_uInt8s_ToOUString(r,
                RTL_TEXTENCODING_ASCII_US);
            SbxDataType t;
            if( ImpScan( aVal, aData.nDouble, t, nullptr, false ) != ERRCODE_NONE )
            {
                aData.nDouble = 0.0;
                return false;
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
            aData.nInt64 = (static_cast<sal_Int64>(tmpHi) << 32);
            aData.nInt64 |= static_cast<sal_Int64>(tmpLo);
            break;
        }
        case SbxSTRING:
        {
            OUString aVal = read_uInt16_lenPrefixed_uInt8s_ToOUString(r,
                RTL_TEXTENCODING_ASCII_US);
            if( !aVal.isEmpty() )
                aData.pOUString = new OUString( aVal );
            else
                aData.pOUString = nullptr; // JSM 1995-09-22
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
                    aData.pObj = nullptr;
                    break;
                case 1:
                    aData.pObj = SbxBase::Load( r );
                    return ( aData.pObj != nullptr );
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
            // Match the Int on this system?
            if( n > SAL_TYPES_SIZEOFINT )
            {
                r.ReadInt32( aData.nLong );
                aData.eType = SbxLONG;
            }
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
            // Match the UInt on this system?
            if( n > SAL_TYPES_SIZEOFINT )
            {
                r.ReadUInt32( aData.nULong );
                aData.eType = SbxULONG;
            }
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
        // #78919 For backwards compatibility
        case SbxWSTRING:
        case SbxWCHAR:
            break;
        default:
            aData.clear(SbxNULL);
            ResetFlag(SbxFlagBits::Fixed);
            SAL_WARN( "basic.sbx", "Loaded a non-supported data type" );

            return false;
    }
    return true;
}

    bool SbxValue::StoreData( SvStream& r ) const
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
                // #49935: Save as double, otherwise an error during the read in
                const_cast<SbxValue*>(this)->aData.eType = static_cast<SbxDataType>( ( nType & 0xF000 ) | SbxDOUBLE );
                write_uInt16_lenPrefixed_uInt8s_FromOUString(r, GetCoreString(), RTL_TEXTENCODING_ASCII_US);
                const_cast<SbxValue*>(this)->aData.eType = static_cast<SbxDataType>(nType);
                break;
            case SbxSINGLE:
            case SbxDOUBLE:
                write_uInt16_lenPrefixed_uInt8s_FromOUString(r, GetCoreString(), RTL_TEXTENCODING_ASCII_US);
                break;
            case SbxSALUINT64:
            case SbxSALINT64:
                // see comment in SbxValue::StoreData
                r.WriteUInt64( aData.uInt64 );
                break;
            case SbxCURRENCY:
            {
                sal_Int32 tmpHi = ( (aData.nInt64 >> 32) &  0xFFFFFFFF );
                sal_Int32 tmpLo = static_cast<sal_Int32>(aData.nInt64);
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
                // to save itself as Objectptr does not work!
                if( aData.pObj )
                {
                    if( dynamic_cast<SbxValue*>( aData.pObj) != this  )
                    {
                        r.WriteUChar( 1 );
                        return aData.pObj->Store( r );
                    }
                    else
                        r.WriteUChar( 2 );
                }
                else
                    r.WriteUChar( 0 );
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
                r.WriteUChar( SAL_TYPES_SIZEOFINT ).WriteInt32( aData.nInt );
                break;
            }
            case SbxUINT:
            {
                r.WriteUChar( SAL_TYPES_SIZEOFINT ).WriteUInt32( aData.nUInt );
                break;
            }
            case SbxEMPTY:
            case SbxNULL:
            case SbxVOID:
                break;
            case SbxDATAOBJECT:
                r.WriteInt32( aData.nLong );
                break;
            // #78919 For backwards compatibility
            case SbxWSTRING:
            case SbxWCHAR:
                break;
            default:
                SAL_WARN( "basic.sbx", "Saving a non-supported data type" );
                return false;
        }
        return true;
    }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
