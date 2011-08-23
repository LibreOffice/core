/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#define _TLBIGINT_INT64
#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#include "sbx.hxx"
#include "sbxconv.hxx"
#include <math.h>

// AB 29.10.99 Unicode
#ifndef _USE_NO_NAMESPACE
using namespace rtl;
#endif

namespace binfilter {

TYPEINIT1(SbxValue,SbxBase)

BigInt SbxINT64Converter::SbxINT64_2_BigInt( const SbxINT64 &r )
{
    BigInt a10000 = 0x10000;

    BigInt aReturn( r.nHigh );
    if( r.nHigh )
        aReturn *= a10000;
    aReturn += (USHORT)(r.nLow >> 16);
    aReturn *= a10000;
    aReturn += (USHORT)r.nLow;
    return aReturn;
}

BOOL SbxINT64Converter::BigInt_2_SbxINT64( const BigInt& b, SbxINT64 *p )
{
    if( !b.IsLong() ) {
        if( b.nLen > 4 || (b.nNum[3] & 0x8000) )
            return FALSE;

        p->nLow  = ((UINT32)b.nNum[1] << 16) | (UINT32)b.nNum[0];
        p->nHigh = ((UINT32)b.nNum[3] << 16) | (UINT32)b.nNum[2];
        if( b.bIsNeg )
            p->CHS();
    }
    else
        p->Set( (INT32)b.nVal );

    return TRUE;
}

BigInt SbxINT64Converter::SbxUINT64_2_BigInt( const SbxUINT64 &r )
{
    BigInt a10000 = 0x10000;

    BigInt aReturn(r.nHigh);
    if( r.nHigh )
        aReturn *= a10000;
    aReturn += (USHORT)(r.nLow >> 16);
    aReturn *= a10000;
    aReturn += (USHORT)r.nLow;
    return aReturn;
}

BOOL SbxINT64Converter::BigInt_2_SbxUINT64( const BigInt& b, SbxUINT64 *p )
{
    if( b.bIsBig ) {
        if( b.bIsNeg || b.nLen > 4 )
            return FALSE;

        p->nLow  = ((UINT32)b.nNum[1] << 16) | (UINT32)b.nNum[0];
        p->nHigh = ((UINT32)b.nNum[3] << 16) | (UINT32)b.nNum[2];
    }
    else {
        if( b.nVal < 0 )
            return FALSE;

        p->Set( (UINT32)b.nVal );
    }

    return TRUE;
}

/////////////////////////// SbxINT64 /////////////////////////////////////
SbxINT64 &SbxINT64::operator -= ( const SbxINT64 &r )
{
    BigInt b( SbxINT64Converter::SbxINT64_2_BigInt( *this ) );
    b -= SbxINT64Converter::SbxINT64_2_BigInt( r );
    SbxINT64Converter::BigInt_2_SbxINT64( b, this );
    return *this;
}
SbxINT64 &SbxINT64::operator += ( const SbxINT64 &r )
{
    BigInt b( SbxINT64Converter::SbxINT64_2_BigInt( *this ) );
    b += SbxINT64Converter::SbxINT64_2_BigInt( r );
    SbxINT64Converter::BigInt_2_SbxINT64( b, this );
    return *this;
}
SbxINT64 &SbxINT64::operator *= ( const SbxINT64 &r )
{
    BigInt b( SbxINT64Converter::SbxINT64_2_BigInt( *this ) );
    b *= SbxINT64Converter::SbxINT64_2_BigInt( r );
    SbxINT64Converter::BigInt_2_SbxINT64( b, this );
    return *this;
}
SbxINT64 &SbxINT64::operator %= ( const SbxINT64 &r )
{
    BigInt b( SbxINT64Converter::SbxINT64_2_BigInt( *this ) );
    b %= SbxINT64Converter::SbxINT64_2_BigInt( r );
    SbxINT64Converter::BigInt_2_SbxINT64( b, this );
    return *this;
}
SbxINT64 &SbxINT64::operator /= ( const SbxINT64 &r )
{
    BigInt b( SbxINT64Converter::SbxINT64_2_BigInt( *this ) );
    b /= SbxINT64Converter::SbxINT64_2_BigInt( r );
    SbxINT64Converter::BigInt_2_SbxINT64( b, this );
    return *this;
}
SbxINT64 &SbxINT64::operator &= ( const SbxINT64 &r )
{
    nHigh &= r.nHigh;
    nLow  &= r.nLow;
    return *this;
}
SbxINT64 &SbxINT64::operator |= ( const SbxINT64 &r )
{
    nHigh |= r.nHigh;
    nLow  |= r.nLow;
    return *this;
}
SbxINT64 &SbxINT64::operator ^= ( const SbxINT64 &r )
{
    nHigh ^= r.nHigh;
    nLow  ^= r.nLow;
    return *this;
}

SbxINT64 operator - ( const SbxINT64 &l, const SbxINT64 &r )
{
    SbxINT64 a(l);
    a -= r;
    return a;
}
SbxINT64 operator + ( const SbxINT64 &l, const SbxINT64 &r )
{
    SbxINT64 a(l);
    a += r;
    return a;
}
SbxINT64 operator / ( const SbxINT64 &l, const SbxINT64 &r )
{
    SbxINT64 a(l);
    a /= r;
    return a;
}
SbxINT64 operator % ( const SbxINT64 &l, const SbxINT64 &r )
{
    SbxINT64 a(l);
    a %= r;
    return a;
}
SbxINT64 operator * ( const SbxINT64 &l, const SbxINT64 &r )
{
    SbxINT64 a(l);
    a *= r;
    return a;
}
SbxINT64 operator & ( const SbxINT64 &l, const SbxINT64 &r )
{
    SbxINT64 a;
    a.nHigh = r.nHigh & l.nHigh;
    a.nLow  = r.nLow  & l.nLow;
    return a;
}
SbxINT64 operator | ( const SbxINT64 &l, const SbxINT64 &r )
{
    SbxINT64 a;
    a.nHigh = r.nHigh | l.nHigh;
    a.nLow  = r.nLow  | l.nLow;
    return a;
}
SbxINT64 operator ^ ( const SbxINT64 &r, const SbxINT64 &l )
{
    SbxINT64 a;
    a.nHigh = r.nHigh ^ l.nHigh;
    a.nLow  = r.nLow  ^ l.nLow;
    return a;
}

SbxINT64 operator - ( const SbxINT64 &r )
{
    SbxINT64 a( r );
    a.CHS();
    return a;
}
SbxINT64 operator ~ ( const SbxINT64 &r )
{
    SbxINT64 a;
    a.nHigh = ~r.nHigh;
    a.nLow  = ~r.nLow;
    return a;
}

SbxUINT64 &SbxUINT64::operator %= ( const SbxUINT64 &r )
{
    BigInt b( SbxINT64Converter::SbxUINT64_2_BigInt( *this ) );
    b %= SbxINT64Converter::SbxUINT64_2_BigInt( r );
    SbxINT64Converter::BigInt_2_SbxUINT64( b, this );
    return *this;
}
SbxUINT64 &SbxUINT64::operator /= ( const SbxUINT64 &r )
{
    BigInt b( SbxINT64Converter::SbxUINT64_2_BigInt( *this ) );
    b /= SbxINT64Converter::SbxUINT64_2_BigInt( r );
    SbxINT64Converter::BigInt_2_SbxUINT64( b, this );
    return *this;
}
/////////////////////////// Fehlerbehandlung /////////////////////////////

#ifdef _USED
// NOCH NACHZUBAUEN!

// Das Default-Handling setzt nur den Fehlercode.

#ifndef WNT
#if defined ( UNX )
int matherr( struct exception* p )
#else
int matherr( struct _exception* p )
#endif
{
    switch( p->type )
    {
#if defined ( UNX )
        case OVERFLOW: SbxBase::SetError( SbxERR_OVERFLOW ); break;
#else
        case _OVERFLOW: SbxBase::SetError( SbxERR_OVERFLOW ); break;
#endif
        default:		SbxBase::SetError( SbxERR_NOTIMP ); break;
    }
    return TRUE;
}
#endif

#endif // _USED


///////////////////////////// Konstruktoren //////////////////////////////

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
        case SbxINTEGER:	n |= SbxBYREF; aData.pInteger = (INT16*) p; break;
        case SbxULONG64:	n |= SbxBYREF; aData.pULong64 = (SbxUINT64*) p; break;
        case SbxLONG64:
        case SbxCURRENCY:	n |= SbxBYREF; aData.pLong64 = (SbxINT64*) p; break;
        case SbxLONG:		n |= SbxBYREF; aData.pLong = (INT32*) p; break;
        case SbxSINGLE:		n |= SbxBYREF; aData.pSingle = (float*) p; break;
        case SbxDATE:
        case SbxDOUBLE:		n |= SbxBYREF; aData.pDouble = (double*) p; break;
        case SbxSTRING:		n |= SbxBYREF; aData.pString = (XubString*) p; break;
        case SbxERROR:
        case SbxUSHORT:
        case SbxBOOL:		n |= SbxBYREF; aData.pUShort = (UINT16*) p; break;
        case SbxULONG:		n |= SbxBYREF; aData.pULong = (UINT32*) p; break;
        case SbxCHAR:		n |= SbxBYREF; aData.pChar = (xub_Unicode*) p; break;
        case SbxBYTE:		n |= SbxBYREF; aData.pByte = (BYTE*) p; break;
        case SbxINT:		n |= SbxBYREF; aData.pInt = (int*) p; break;
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
        // Pointer kopieren, Referenzen inkrementieren
        switch( aData.eType )
        {
            case SbxSTRING:
                if( aData.pString )
                    aData.pString = new XubString( *aData.pString );
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
            // Den Inhalt der Variablen auslesen
            SbxValues aNew;
            if( IsFixed() )
                // fest: dann muss der Typ stimmen
                aNew.eType = aData.eType;
            else if( r.IsFixed() )
                // Quelle fest: Typ uebernehmen
                aNew.eType = SbxDataType( r.aData.eType & 0x0FFF );
            else
                // beides Variant: dann isses egal
                aNew.eType = SbxVARIANT;
            if( r.Get( aNew ) )
                Put( aNew );
        }
    }
    return *this;
}

SbxValue::~SbxValue()
{
#ifndef C50
    Broadcast( SBX_HINT_DYING );
    SetFlag( SBX_WRITE );
    SbxValue::Clear();
#else
    // Provisorischer Fix fuer Solaris 5.0 Compiler Bug
    // bei Nutzung virtueller Vererbung. Virtuelle Calls
    // im Destruktor vermeiden. Statt Clear() zu rufen
    // moegliche Objekt-Referenzen direkt freigeben.
    if( aData.eType == SbxOBJECT )
    {
        if( aData.pObj && aData.pObj != this )
        {
            HACK(nicht bei Parent-Prop - sonst CyclicRef)
            SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
            BOOL bParentProp = pThisVar && 5345 ==
            ( (INT16) ( pThisVar->GetUserData() & 0xFFFF ) );
            if ( !bParentProp )
                aData.pObj->ReleaseRef();
        }
    }
    else if( aData.eType == SbxDECIMAL )
    {
        releaseDecimalPtr( aData.pDecimal );
    }
#endif
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
            delete aData.pString; aData.pString = NULL;
            break;
        case SbxOBJECT:
            if( aData.pObj )
            {
                if( aData.pObj != this )
                {
                    HACK(nicht bei Parent-Prop - sonst CyclicRef)
                    SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
                    BOOL bParentProp = pThisVar && 5345 ==
                    ( (INT16) ( pThisVar->GetUserData() & 0xFFFF ) );
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

void SbxValue::Broadcast( ULONG )
{}

//////////////////////////// Daten auslesen //////////////////////////////

// Ermitteln der "richtigen" Variablen. Falls es ein Objekt ist, wird
// entweder das Objekt selbst oder dessen Default-Property angesprochen.
// Falls die Variable eine Variable oder ein Objekt enthaelt, wird
// dieses angesprochen.

SbxValue* SbxValue::TheRealValue() const
{
    return TheRealValue( TRUE );
}

// #55226 Zusaetzliche Info transportieren
SbxValue* SbxValue::TheRealValue( BOOL bObjInObjError ) const
{
    SbxValue* p = (SbxValue*) this;
    for( ;; )
    {
        SbxDataType t = SbxDataType( p->aData.eType & 0x0FFF );
        if( t == SbxOBJECT )
        {
            // Der Block enthaelt ein Objekt oder eine Variable
            SbxObject* pObj = PTR_CAST(SbxObject,p->aData.pObj);
            if( pObj )
            {
                // Hat das Objekt eine Default-Property?
                SbxVariable* pDflt = pObj->GetDfltProperty();

                // Falls dies ein Objekt ist und sich selbst enthaelt,
                // koennen wir nicht darauf zugreifen
                // #55226# Die alte Bedingung, um einen Fehler zu setzen,
                // ist nicht richtig, da z.B. eine ganz normale Variant-
                // Variable mit Objekt davon betroffen sein kann, wenn ein
                // anderer Wert zugewiesen werden soll. Daher mit Flag.
                if( bObjInObjError && !pDflt &&
                    ((SbxValue*) pObj)->aData.eType == SbxOBJECT &&
                    ((SbxValue*) pObj)->aData.pObj == pObj )
                {
                    SetError( SbxERR_BAD_PROP_VALUE );
                    p = NULL;
                }
                else if( pDflt )
                    p = pDflt;
                /* ALT:
                else
                    p = pDflt ? pDflt : (SbxVariable*) pObj;
                */
                break;
            }
            // Haben wir ein Array?
            SbxArray* pArray = PTR_CAST(SbxArray,p->aData.pObj);
            if( pArray )
            {
                // Ggf. Parameter holen
                SbxArray* pPar = NULL;
                SbxVariable* pVar = PTR_CAST(SbxVariable,p);
                if( pVar )
                    pPar = pVar->GetParameters();
                if( pPar )
                {
                    // Haben wir ein dimensioniertes Array?
                    SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,p->aData.pObj);
                    if( pDimArray )
                        p = pDimArray->Get( pPar );
                    else
                        p = pArray->Get( pPar->Get( 1 )->GetInteger() );
                    break;
                }
            }
            // Sonst einen SbxValue annehmen
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

BOOL SbxValue::Get( SbxValues& rRes ) const
{
    BOOL bRes = FALSE;
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
        // Falls nach einem Objekt oder einem VARIANT gefragt wird, nicht
        // die wahren Werte suchen
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
                case SbxNULL:	 break;
                case SbxVARIANT: rRes = p->aData; break;
                case SbxINTEGER: rRes.nInteger = ImpGetInteger( &p->aData ); break;
                case SbxLONG:	 rRes.nLong = ImpGetLong( &p->aData ); break;
                case SbxSALINT64:	rRes.nInt64 = ImpGetInt64( &p->aData ); break;
                case SbxSALUINT64:	rRes.uInt64 = ImpGetUInt64( &p->aData ); break;
                case SbxSINGLE:	 rRes.nSingle = ImpGetSingle( &p->aData ); break;
                case SbxDOUBLE:	 rRes.nDouble = ImpGetDouble( &p->aData ); break;
                case SbxCURRENCY:rRes.nLong64 = ImpGetCurrency( &p->aData ); break;
                case SbxDECIMAL: rRes.pDecimal = ImpGetDecimal( &p->aData ); break;
                case SbxDATE:	 rRes.nDouble = ImpGetDate( &p->aData ); break;
                case SbxBOOL:
                    rRes.nUShort = sal::static_int_cast< UINT16 >(
                        ImpGetBool( &p->aData ) );
                    break;
                case SbxCHAR:	 rRes.nChar = ImpGetChar( &p->aData ); break;
                case SbxBYTE:	 rRes.nByte = ImpGetByte( &p->aData ); break;
                case SbxUSHORT:	 rRes.nUShort = ImpGetUShort( &p->aData ); break;
                case SbxULONG:	 rRes.nULong = ImpGetULong( &p->aData ); break;
                case SbxLPSTR:
                case SbxSTRING:	 p->aPic = ImpGetString( &p->aData );
                                 rRes.pString = &p->aPic; break;
                case SbxCoreSTRING:	p->aPic = ImpGetCoreString( &p->aData );
                                    rRes.pString = &p->aPic; break;
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
            // Objekt enthielt sich selbst
            SbxDataType eTemp = rRes.eType;
            memset( &rRes, 0, sizeof( SbxValues ) );
            rRes.eType = eTemp;
        }
    }
    if( !IsError() )
    {
        bRes = TRUE;
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
        // Geht in Ordnung, da Ptr eine Kopie ist
        return *aRes.pString;
    else
    {
        ((SbxValue*) this)->aPic.Erase();
        return aPic;
    }
}

const XubString& SbxValue::GetCoreString() const
{
    SbxValues aRes;
    aRes.eType = SbxCoreSTRING;
    if( Get( aRes ) )
        // Geht in Ordnung, da Ptr eine Kopie ist
        return *aRes.pString;
    else
    {
        ((SbxValue*) this)->aPic.Erase();
        return aPic;
    }
}

BOOL SbxValue::GetBool() const
{
    SbxValues aRes;
    aRes.eType = SbxBOOL;
    Get( aRes );
    return BOOL( aRes.nUShort != 0 );
}

#define GET( g, e, t, m ) \
t SbxValue::g() const { SbxValues aRes(e); Get( aRes ); return aRes.m; }

GET( GetByte,     SbxBYTE,       BYTE,             nByte )
GET( GetChar,     SbxCHAR,       xub_Unicode,           nChar )
GET( GetCurrency, SbxCURRENCY,   SbxINT64,         nLong64 )
GET( GetDate,     SbxDATE,       double,           nDouble )
GET( GetDouble,   SbxDOUBLE,     double,           nDouble )
GET( GetInteger,  SbxINTEGER,    INT16,            nInteger )
GET( GetLong,     SbxLONG,       INT32,            nLong )
GET( GetObject,   SbxOBJECT,     SbxBase*,         pObj )
GET( GetSingle,   SbxSINGLE,     float,            nSingle )
GET( GetULong,    SbxULONG,      UINT32,           nULong )
GET( GetUShort,   SbxUSHORT,     UINT16,           nUShort )
GET( GetInt64,    SbxSALINT64,   sal_Int64,        nInt64 )
GET( GetUInt64,   SbxSALUINT64,  sal_uInt64,       uInt64 )
#ifdef WNT
GET( GetDecimal,  SbxDECIMAL,    SbxDecimal*,      pDecimal )
#endif

//////////////////////////// Daten schreiben /////////////////////////////

BOOL SbxValue::Put( const SbxValues& rVal )
{
    BOOL bRes = FALSE;
    SbxError eOld = GetError();
    if( eOld != SbxERR_OK )
        ResetError();
    if( !CanWrite() )
        SetError( SbxERR_PROP_READONLY );
    else if( rVal.eType & 0xF000 )
        SetError( SbxERR_NOTIMP );
    else
    {
        // Falls nach einem Objekt gefragt wird, nicht
        // die wahren Werte suchen
        SbxValue* p = this;
        if( rVal.eType != SbxOBJECT )
            p = TheRealValue( FALSE );	// #55226 Hier keinen Fehler erlauben
        if( p )
        {
            if( !p->CanWrite() )
                SetError( SbxERR_PROP_READONLY );
            else if( p->IsFixed() || p->SetType( (SbxDataType) ( rVal.eType & 0x0FFF ) ) )
              switch( rVal.eType & 0x0FFF )
            {
                case SbxEMPTY:
                case SbxVOID:
                case SbxNULL:		break;
                case SbxINTEGER:	ImpPutInteger( &p->aData, rVal.nInteger ); break;
                case SbxLONG:		ImpPutLong( &p->aData, rVal.nLong ); break;
                case SbxSALINT64:	ImpPutInt64( &p->aData, rVal.nInt64 ); break;
                case SbxSALUINT64:	ImpPutUInt64( &p->aData, rVal.uInt64 ); break;
                case SbxSINGLE:		ImpPutSingle( &p->aData, rVal.nSingle ); break;
                case SbxDOUBLE:		ImpPutDouble( &p->aData, rVal.nDouble ); break;
                case SbxCURRENCY:	ImpPutCurrency( &p->aData, rVal.nLong64 ); break;
                case SbxDECIMAL:	ImpPutDecimal( &p->aData, rVal.pDecimal ); break;
                case SbxDATE:		ImpPutDate( &p->aData, rVal.nDouble ); break;
                case SbxBOOL:		ImpPutBool( &p->aData, rVal.nInteger ); break;
                case SbxCHAR:		ImpPutChar( &p->aData, rVal.nChar ); break;
                case SbxBYTE:		ImpPutByte( &p->aData, rVal.nByte ); break;
                case SbxUSHORT:		ImpPutUShort( &p->aData, rVal.nUShort ); break;
                case SbxULONG:		ImpPutULong( &p->aData, rVal.nULong ); break;
                case SbxLPSTR:
                case SbxSTRING:		ImpPutString( &p->aData, rVal.pString ); break;
                case SbxINT:
#if SAL_TYPES_SIZEOFINT == 2
                    ImpPutInteger( &p->aData, (INT16) rVal.nInt );
#else
                    ImpPutLong( &p->aData, (INT32) rVal.nInt );
#endif
                    break;
                case SbxUINT:
#if SAL_TYPES_SIZEOFINT == 2
                    ImpPutUShort( &p->aData, (UINT16) rVal.nUInt );
#else
                    ImpPutULong( &p->aData, (UINT32) rVal.nUInt );
#endif
                    break;
                case SbxOBJECT:
                    if( !p->IsFixed() || p->aData.eType == SbxOBJECT )
                    {
                        // ist schon drin
                        if( p->aData.eType == SbxOBJECT && p->aData.pObj == rVal.pObj )
                            break;

                        // Nur den Werteteil loeschen!
                        p->SbxValue::Clear();

                        // eingentliche Zuweisung
                        p->aData.pObj = rVal.pObj;

                        // ggf. Ref-Count mitzaehlen
                        if( p->aData.pObj && p->aData.pObj != p )
                        {
                            if ( p != this )
                            {
                                DBG_ERROR( "TheRealValue" );
                            }
                            HACK(nicht bei Parent-Prop - sonst CyclicRef)
                            SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
                            BOOL bParentProp = pThisVar && 5345 ==
                                    ( (INT16) ( pThisVar->GetUserData() & 0xFFFF ) );
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
                p->SetModified( TRUE );
                p->Broadcast( SBX_HINT_DATACHANGED );
                if( eOld != SbxERR_OK )
                    SetError( eOld );
                bRes = TRUE;
            }
        }
    }
    return bRes;
}

BOOL SbxValue::PutBool( BOOL b )
{
    SbxValues aRes;
    aRes.eType = SbxBOOL;
    aRes.nUShort = sal::static_int_cast< UINT16 >(b ? SbxTRUE : SbxFALSE);
    Put( aRes );
    return BOOL( !IsError() );
}

BOOL SbxValue::PutEmpty()
{
    BOOL bRet = SetType( SbxEMPTY );
        SetModified( TRUE );
    return bRet;
}

BOOL SbxValue::PutString( const XubString& r )
{
    SbxValues aRes;
    aRes.eType = SbxSTRING;
    aRes.pString = (XubString*) &r;
    Put( aRes );
    return BOOL( !IsError() );
}

#define PUT( p, e, t, m ) \
BOOL SbxValue::p( t n ) \
{ SbxValues aRes(e); aRes.m = n; Put( aRes ); return BOOL( !IsError() ); }

PUT( PutByte,     SbxBYTE,       BYTE,             nByte )
PUT( PutChar,     SbxCHAR,       xub_Unicode,      nChar )
PUT( PutCurrency, SbxCURRENCY,   const SbxINT64&,  nLong64 )
PUT( PutDate,     SbxDATE,       double,           nDouble )
PUT( PutDouble,   SbxDOUBLE,     double,           nDouble )
PUT( PutInteger,  SbxINTEGER,    INT16,            nInteger )
PUT( PutLong,     SbxLONG,       INT32,            nLong )
PUT( PutObject,   SbxOBJECT,     SbxBase*,         pObj )
PUT( PutSingle,   SbxSINGLE,     float,            nSingle )
PUT( PutULong,    SbxULONG,      UINT32,           nULong )
PUT( PutUShort,   SbxUSHORT,     UINT16,           nUShort )
PUT( PutInt64,    SbxSALINT64,   sal_Int64,        nInt64 )
PUT( PutUInt64,   SbxSALUINT64,  sal_uInt64,       uInt64 )
#ifdef WNT
PUT( PutDecimal,  SbxDECIMAL,    SbxDecimal*,      pDecimal )
#endif

////////////////////////// Setzen des Datentyps ///////////////////////////

BOOL SbxValue::IsFixed() const
{
    return ( (GetFlags() & SBX_FIXED) | (aData.eType & SbxBYREF) ) != 0;
}

// Eine Variable ist numerisch, wenn sie EMPTY oder wirklich numerisch ist
// oder einen vollstaendig konvertierbaren String enthaelt

// #41692, fuer RTL und Basic-Core getrennt implementieren
BOOL SbxValue::IsNumeric() const
{
    return ImpIsNumeric( /*bOnlyIntntl*/FALSE );
}

BOOL SbxValue::ImpIsNumeric( BOOL bOnlyIntntl ) const
{

    if( !CanRead() )
    {
        SetError( SbxERR_PROP_WRITEONLY ); return FALSE;
    }
    // Downcast pruefen!!!
    if( this->ISA(SbxVariable) )
        ((SbxVariable*)this)->Broadcast( SBX_HINT_DATAWANTED );
    SbxDataType t = GetType();
    if( t == SbxSTRING )
    {
        if( aData.pString )
        {
            XubString s( *aData.pString );
            double n;
            SbxDataType t2;
            USHORT nLen = 0;
            if( ImpScan( s, n, t2, &nLen, /*bAllowIntntl*/FALSE, bOnlyIntntl ) == SbxERR_OK )
                return BOOL( nLen == s.Len() );
        }
        return FALSE;
    }
    else
        return BOOL( t == SbxEMPTY
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

BOOL SbxValue::SetType( SbxDataType t )
{
    DBG_ASSERT( !( t & 0xF000 ), "Setzen von BYREF|ARRAY verboten!" );
    if( ( t == SbxEMPTY && aData.eType == SbxVOID )
     || ( aData.eType == SbxEMPTY && t == SbxVOID ) )
        return TRUE;
    if( ( t & 0x0FFF ) == SbxVARIANT )
    {
        // Versuch, den Datentyp auf Variant zu setzen
        ResetFlag( SBX_FIXED );
        if( IsFixed() )
        {
            SetError( SbxERR_CONVERSION ); return FALSE;
        }
        t = SbxEMPTY;
    }
    if( ( t & 0x0FFF ) != ( aData.eType & 0x0FFF ) )
    {
        if( !CanWrite() || IsFixed() )
        {
            SetError( SbxERR_CONVERSION ); return FALSE;
        }
        else
        {
            // Eventuelle Objekte freigeben
            switch( aData.eType )
            {
                case SbxSTRING:
                    delete aData.pString;
                    break;
                case SbxOBJECT:
                    if( aData.pObj && aData.pObj != this )
                    {
                        HACK(nicht bei Parent-Prop - sonst CyclicRef)
                        SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
                        UINT16 nSlotId = pThisVar
                                    ? ( (INT16) ( pThisVar->GetUserData() & 0xFFFF ) )
                                    : 0;
                        DBG_ASSERT( nSlotId != 5345 || pThisVar->GetName() == UniString::CreateFromAscii( "Parent" ),
                                    "SID_PARENTOBJECT heisst nicht 'Parent'" );
                        BOOL bParentProp = 5345 == nSlotId;
                        if ( !bParentProp )
                            aData.pObj->ReleaseRef();
                    }
                    break;
                default: break;
            }
            // Das klappt immer, da auch die Float-Repraesentationen 0 sind.
            memset( &aData, 0, sizeof( SbxValues ) );
            aData.eType = t;
        }
    }
    return TRUE;
}

BOOL SbxValue::Convert( SbxDataType eTo )
{
    eTo = SbxDataType( eTo & 0x0FFF );
    if( ( aData.eType & 0x0FFF ) == eTo )
        return TRUE;
    if( !CanWrite() )
        return FALSE;
    if( eTo == SbxVARIANT )
    {
        // Versuch, den Datentyp auf Variant zu setzen
        ResetFlag( SBX_FIXED );
        if( IsFixed() )
        {
            SetError( SbxERR_CONVERSION ); return FALSE;
        }
        else
            return TRUE;
    }
    // Convert from Null geht niemals. Einmal Null, immer Null!
    if( aData.eType == SbxNULL )
    {
        SetError( SbxERR_CONVERSION ); return FALSE;
    }

    // Konversion der Daten:
    SbxValues aNew;
    aNew.eType = eTo;
    if( Get( aNew ) )
    {
        // Der Datentyp konnte konvertiert werden. Bei Fixed-Elementen
        // ist hier Ende, da die Daten nicht uebernommen zu werden brauchen
        if( !IsFixed() )
        {
            SetType( eTo );
            Put( aNew );
            SetModified( TRUE );
        }
        Broadcast( SBX_HINT_CONVERTED );
        return TRUE;
    }
    else
        return FALSE;
}
////////////////////////////////// Rechnen /////////////////////////////////

BOOL SbxValue::Compute( SbxOperator eOp, const SbxValue& rOp )
{
    SbxDataType eThisType = GetType();
    SbxDataType eOpType = rOp.GetType();
    SbxError eOld = GetError();
    if( eOld != SbxERR_OK )
        ResetError();
    if( !CanWrite() )
        SetError( SbxERR_PROP_READONLY );
    else if( !rOp.CanRead() )
        SetError( SbxERR_PROP_WRITEONLY );
    // Sonderregel 1: Ist ein Operand Null, ist das Ergebnis Null
    else if( eThisType == SbxNULL || eOpType == SbxNULL )
        SetType( SbxNULL );
    // Sonderregel 2: Ist ein Operand Empty, ist das Ergebnis der 2. Operand
    else if( eThisType == SbxEMPTY )
        *this = rOp;
    // 13.2.96: Nicht schon vor Get auf SbxEMPTY pruefen
    else
    {
        SbxValues aL, aR;
        bool bDecimal = false;
        if( eThisType == SbxSTRING || eOp == SbxCAT )
        {
            if( eOp == SbxCAT || eOp == SbxPLUS )
            {
                // AB 5.11.1999, OUString beruecksichtigen
                aL.eType = aR.eType = SbxSTRING;
                rOp.Get( aR );
                // AB 8.12.1999, #70399: Hier wieder GetType() rufen, Get() kann Typ aendern!
                if( rOp.GetType() == SbxEMPTY )
                    goto Lbl_OpIsEmpty;
                Get( aL );

                // #30576: Erstmal testen, ob Wandlung geklappt hat
                if( aL.pString != NULL && aR.pString != NULL )
                {
                    *aL.pString += *aR.pString;
                }
                // Nicht einmal Left OK?
                else if( aL.pString == NULL )
                {
                    aL.pString = new String();
                }
                Put( aL );
            }
            else
                SetError( SbxERR_CONVERSION );
        }
        else if( eOpType == SbxSTRING && rOp.IsFixed() )
        {	// Numerisch: rechts darf kein String stehen
            SetError( SbxERR_CONVERSION );
        }
        else if( ( eOp >= SbxIDIV && eOp <= SbxNOT ) || eOp == SbxMOD )
        {
            if( GetType() == eOpType )
            {
                if( GetType() == SbxULONG64
                    || GetType() == SbxLONG64
                    || GetType() == SbxCURRENCY
                    || GetType() == SbxULONG )
                    aL.eType = aR.eType = GetType();
//				else if( GetType() == SbxDouble || GetType() == SbxSingle )
//					aL.eType = aR.eType = SbxLONG64;
                else
                    aL.eType = aR.eType = SbxLONG;
            }
            else if( GetType() == SbxCURRENCY || eOpType == SbxCURRENCY
                     || GetType() == SbxULONG64 || eOpType == SbxULONG64
                     || GetType() == SbxLONG64 || eOpType == SbxLONG64 )
                aL.eType = aR.eType = SbxLONG64;
//			else if( GetType() == SbxDouble || rOP.GetType() == SbxDouble
//			         || GetType() == SbxSingle || rOP.GetType() == SbxSingle )
//				aL.eType = aR.eType = SbxLONG64;
            else
                aL.eType = aR.eType = SbxLONG;

            if( rOp.Get( aR ) )
            {
                if( rOp.GetType() == SbxEMPTY )
                    goto Lbl_OpIsEmpty;
                if( Get( aL ) ) switch( eOp )
                {
                    case SbxIDIV:
                        if( aL.eType == SbxCURRENCY )
                            aL.eType = SbxLONG64;
                        if( aL.eType == SbxLONG64 )
                            if( !aR.nLong64 ) SetError( SbxERR_ZERODIV );
                            else aL.nLong64 /= aR.nLong64;
                        else if( aL.eType == SbxULONG64 )
                            if( !aR.nULong64 ) SetError( SbxERR_ZERODIV );
                            else aL.nULong64 /= aR.nULong64;
                        else if( aL.eType == SbxLONG )
                            if( !aR.nLong ) SetError( SbxERR_ZERODIV );
                            else aL.nLong /= aR.nLong;
                        else
                            if( !aR.nULong ) SetError( SbxERR_ZERODIV );
                            else aL.nULong /= aR.nULong;
                        break;
                    case SbxMOD:
                        if( aL.eType == SbxCURRENCY )
                            aL.eType = SbxLONG64;
                        if( aL.eType == SbxLONG64 )
                            if( !aR.nLong64 ) SetError( SbxERR_ZERODIV );
                            else aL.nLong64 %= aR.nLong64;
                        else if( aL.eType == SbxULONG64 )
                            if( !aR.nULong64 ) SetError( SbxERR_ZERODIV );
                            else aL.nULong64 %= aR.nULong64;
                        else if( aL.eType == SbxLONG )
                            if( !aR.nLong ) SetError( SbxERR_ZERODIV );
                            else aL.nLong %= aR.nLong;
                        else
                            if( !aR.nULong ) SetError( SbxERR_ZERODIV );
                            else aL.nULong %= aR.nULong;
                        break;
                    case SbxAND:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                            aL.nLong64 &= aR.nLong64;
                        else
                            aL.nLong &= aR.nLong;
                        break;
                    case SbxOR:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                            aL.nLong64 |= aR.nLong64;
                        else
                            aL.nLong |= aR.nLong;
                        break;
                    case SbxXOR:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                            aL.nLong64 ^= aR.nLong64;
                        else
                            aL.nLong ^= aR.nLong;
                        break;
                    case SbxEQV:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                            aL.nLong64 = (aL.nLong64 & aR.nLong64) | (~aL.nLong64 & ~aR.nLong64);
                        else
                            aL.nLong = (aL.nLong & aR.nLong) | (~aL.nLong & ~aR.nLong);
                        break;
                    case SbxIMP:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                            aL.nLong64 = ~aL.nLong64 | aR.nLong64;
                        else
                            aL.nLong = ~aL.nLong | aR.nLong;
                        break;
                    case SbxNOT:
                        if( aL.eType != SbxLONG && aL.eType != SbxULONG )
                            aL.nLong64 = ~aL.nLong64;
                        else
                            aL.nLong = ~aL.nLong;
                        break;
                    default: break;
                }
            }
        }
        else if( ( GetType() == SbxDECIMAL || rOp.GetType() == SbxDECIMAL ) &&
                 ( eOp == SbxMUL || eOp == SbxDIV || eOp == SbxPLUS || eOp == SbxMINUS || eOp == SbxNEG ) )
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
                static BigInt n10K( 10000 );

                if( rOp.GetType() == SbxEMPTY )
                    goto Lbl_OpIsEmpty;

                if( Get( aL ) ) switch( eOp )
                {
                    case SbxMUL:
                    {
                        // #i20704 Implement directly
                        BigInt b1( SbxINT64Converter::SbxINT64_2_BigInt( aL.nLong64 ) );
                        BigInt b2( SbxINT64Converter::SbxINT64_2_BigInt( aR.nLong64 ) );
                        b1 *= b2;
                        b1 /= n10K;
                        double d = double( b1 ) / 10000.0;
                        if( d > SbxMAXCURR || d < SbxMINCURR )
                            SetError( SbxERR_OVERFLOW );
                        else
                            SbxINT64Converter::BigInt_2_SbxINT64( b1, &aL.nLong64 );
                        break;
                    }
                    case SbxDIV:
                        if( !aR.nLong64 )
                        {
                            SetError( SbxERR_ZERODIV );
                        }
                        else
                        {
                            // #i20704 Implement directly
                            BigInt b1( SbxINT64Converter::SbxINT64_2_BigInt( aL.nLong64 ) );
                            BigInt b2( SbxINT64Converter::SbxINT64_2_BigInt( aR.nLong64 ) );
                            b1 *= n10K;
                            b1 /= b2;
                            double d = double( b1 ) / 10000.0;
                            if( d > SbxMAXCURR || d < SbxMINCURR )
                                SetError( SbxERR_OVERFLOW );
                            else
                                SbxINT64Converter::BigInt_2_SbxINT64( b1, &aL.nLong64 );
                        }
                        break;
                    case SbxPLUS:
                        aL.nLong64 += aR.nLong64; break;
                    case SbxMINUS:
                        aL.nLong64 -= aR.nLong64; break;
                    case SbxNEG:
                        aL.nLong64 = -aL.nLong64; break;
                    default:
                        SetError( SbxERR_NOTIMP );
                }
            }
        }
        else
        {	// Andere Operatoren
            aL.eType = aR.eType = SbxDOUBLE;
            if( rOp.Get( aR ) )
            {
                if( rOp.GetType() == SbxEMPTY )
                    goto Lbl_OpIsEmpty;
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

                    // #45465 Date braucht bei + eine Spezial-Behandlung
                    if( eOp == SbxPLUS && (GetType() == SbxDATE || rOp.GetType() == SbxDATE ) )
                        aL.eType = SbxDATE;
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

    BOOL bRes = BOOL( !IsError() );
    if( bRes && eOld != SbxERR_OK )
        SetError( eOld );
    return bRes;
}

// Die Vergleichs-Routine liefert TRUE oder FALSE.

BOOL SbxValue::Compare( SbxOperator eOp, const SbxValue& rOp ) const
{
    BOOL bRes = FALSE;
    SbxError eOld = GetError();
    if( eOld != SbxERR_OK )
        ResetError();
    if( !CanRead() || !rOp.CanRead() )
        SetError( SbxERR_PROP_WRITEONLY );
    else if( GetType() == SbxNULL && rOp.GetType() == SbxNULL )
        bRes = TRUE;
    else if( GetType() == SbxEMPTY && rOp.GetType() == SbxEMPTY )
        bRes = TRUE;
    // Sonderregel 1: Ist ein Operand Null, ist das Ergebnis FALSE
    else if( GetType() == SbxNULL || rOp.GetType() == SbxNULL )
        bRes = FALSE;
    // Sonderregel 2: Wenn beide Variant sind und einer ist numerisch,
    // und der andere ein String, ist num < str
    else if( !IsFixed() && !rOp.IsFixed()
     && ( rOp.GetType() == SbxSTRING && GetType() != SbxSTRING && IsNumeric() ) )
        bRes = BOOL( eOp == SbxLT || eOp == SbxLE || eOp == SbxNE );
    else if( !IsFixed() && !rOp.IsFixed()
     && ( GetType() == SbxSTRING && rOp.GetType() != SbxSTRING && rOp.IsNumeric() ) )
        bRes = BOOL( eOp == SbxGT || eOp == SbxGE || eOp == SbxNE );
    else
    {
        SbxValues aL, aR;
        // Wenn einer der Operanden ein String ist,
        // findet ein Stringvergleich statt
        if( GetType() == SbxSTRING || rOp.GetType() == SbxSTRING )
        {
            aL.eType = aR.eType = SbxSTRING;
            if( Get( aL ) && rOp.Get( aR ) ) switch( eOp )
            {
                case SbxEQ:
                    bRes = BOOL( *aL.pString == *aR.pString ); break;
                case SbxNE:
                    bRes = BOOL( *aL.pString != *aR.pString ); break;
                case SbxLT:
                    bRes = BOOL( *aL.pString <  *aR.pString ); break;
                case SbxGT:
                    bRes = BOOL( *aL.pString >  *aR.pString ); break;
                case SbxLE:
                    bRes = BOOL( *aL.pString <= *aR.pString ); break;
                case SbxGE:
                    bRes = BOOL( *aL.pString >= *aR.pString ); break;
                default:
                    SetError( SbxERR_NOTIMP );
            }
        }
        // AB 19.12.95: Wenn SbxSINGLE beteiligt, auf SINGLE konvertieren,
        //				sonst gibt es numerische Fehler
        else if( GetType() == SbxSINGLE || rOp.GetType() == SbxSINGLE )
        {
            aL.eType = aR.eType = SbxSINGLE;
            if( Get( aL ) && rOp.Get( aR ) )
              switch( eOp )
            {
                case SbxEQ:
                    bRes = BOOL( aL.nSingle == aR.nSingle ); break;
                case SbxNE:
                    bRes = BOOL( aL.nSingle != aR.nSingle ); break;
                case SbxLT:
                    bRes = BOOL( aL.nSingle <  aR.nSingle ); break;
                case SbxGT:
                    bRes = BOOL( aL.nSingle >  aR.nSingle ); break;
                case SbxLE:
                    bRes = BOOL( aL.nSingle <= aR.nSingle ); break;
                case SbxGE:
                    bRes = BOOL( aL.nSingle >= aR.nSingle ); break;
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
                        bRes = BOOL( eRes == SbxDecimal::EQ ); break;
                    case SbxNE:
                        bRes = BOOL( eRes != SbxDecimal::EQ ); break;
                    case SbxLT:
                        bRes = BOOL( eRes == SbxDecimal::LT ); break;
                    case SbxGT:
                        bRes = BOOL( eRes == SbxDecimal::GT ); break;
                    case SbxLE:
                        bRes = BOOL( eRes != SbxDecimal::GT ); break;
                    case SbxGE:
                        bRes = BOOL( eRes != SbxDecimal::LT ); break;
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
        // Alles andere auf SbxDOUBLE-Basis vergleichen
        else
        {
            aL.eType = aR.eType = SbxDOUBLE;
            if( Get( aL ) && rOp.Get( aR ) )
              switch( eOp )
            {
                case SbxEQ:
                    bRes = BOOL( aL.nDouble == aR.nDouble ); break;
                case SbxNE:
                    bRes = BOOL( aL.nDouble != aR.nDouble ); break;
                case SbxLT:
                    bRes = BOOL( aL.nDouble <  aR.nDouble ); break;
                case SbxGT:
                    bRes = BOOL( aL.nDouble >  aR.nDouble ); break;
                case SbxLE:
                    bRes = BOOL( aL.nDouble <= aR.nDouble ); break;
                case SbxGE:
                    bRes = BOOL( aL.nDouble >= aR.nDouble ); break;
                default:
                    SetError( SbxERR_NOTIMP );
            }
        }
    }
    if( eOld != SbxERR_OK )
        SetError( eOld );
    return bRes;
}

///////////////////////////// Lesen/Schreiben ////////////////////////////

BOOL SbxValue::LoadData( SvStream& r, USHORT )
{
    SbxValue::Clear();
    UINT16 nType;
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
            // Floats als ASCII
            XubString aVal;
            r.ReadByteString( aVal, RTL_TEXTENCODING_ASCII_US );
            double d;
            SbxDataType t;
            if( ImpScan( aVal, d, t, NULL ) != SbxERR_OK || t == SbxDOUBLE )
            {
                aData.nSingle = 0.0F;
                return FALSE;
            }
            aData.nSingle = (float) d;
            break;
        }
        case SbxDATE:
        case SbxDOUBLE:
        {
            // Floats als ASCII
            XubString aVal;
            r.ReadByteString( aVal, RTL_TEXTENCODING_ASCII_US );
            SbxDataType t;
            if( ImpScan( aVal, aData.nDouble, t, NULL ) != SbxERR_OK )
            {
                aData.nDouble = 0.0;
                return FALSE;
            }
            break;
        }
        case SbxULONG64:
        {
            r >> aData.nULong64.nHigh >> aData.nULong64.nLow;
            break;
        }
        case SbxLONG64:
        case SbxCURRENCY:
        {
            r >> aData.nLong64.nHigh >> aData.nLong64.nLow;
            break;
        }
        case SbxSTRING:
        {
            XubString aVal;
            r.ReadByteString( aVal, RTL_TEXTENCODING_ASCII_US );
            if( aVal.Len() )
                aData.pString = new XubString( aVal );
            else
                aData.pString = NULL; // JSM 22.09.1995
            break;
        }
        case SbxERROR:
        case SbxUSHORT:
            r >> aData.nUShort; break;
        case SbxOBJECT:
        {
            BYTE nMode;
            r >> nMode;
            switch( nMode )
            {
                case 0:
                    aData.pObj = NULL;
                    break;
                case 1:
                    aData.pObj = SbxBase::Load( r );
                    return BOOL( aData.pObj != NULL );
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
            BYTE n;
            r >> n;
            // Passt der Int auf diesem System?
            if( n > SAL_TYPES_SIZEOFINT )
                r >> aData.nLong, aData.eType = SbxLONG;
            else
                r >> aData.nInt;
            break;
        }
        case SbxUINT:
        {
            BYTE n;
            r >> n;
            // Passt der UInt auf diesem System?
            if( n > SAL_TYPES_SIZEOFINT )
                r >> aData.nULong, aData.eType = SbxULONG;
            else
                r >> (sal_uInt32&)aData.nUInt;
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
            return FALSE;
    }
    return TRUE;
}

BOOL SbxValue::StoreData( SvStream& r ) const
{
    UINT16 nType = sal::static_int_cast< UINT16 >(aData.eType);
    r << nType;
    switch( nType & 0x0FFF )
    {
        case SbxBOOL:
        case SbxINTEGER:
            r << aData.nInteger; break;
        case SbxLONG:
            r << aData.nLong; break;
        case SbxDATE:
            // #49935: Als double speichern, sonst Fehler beim Einlesen
            ((SbxValue*)this)->aData.eType = (SbxDataType)( ( nType & 0xF000 ) | SbxDOUBLE );
            r.WriteByteString( GetCoreString(), RTL_TEXTENCODING_ASCII_US );
            ((SbxValue*)this)->aData.eType = (SbxDataType)nType;
            break;
        case SbxSINGLE:
        case SbxDOUBLE:
            r.WriteByteString( GetCoreString(), RTL_TEXTENCODING_ASCII_US );
            break;
        case SbxULONG64:
        {
            r << aData.nULong64.nHigh << aData.nULong64.nLow;
            break;
        }
        case SbxLONG64:
        case SbxCURRENCY:
        {
            r << aData.nLong64.nHigh << aData.nLong64.nLow;
            break;
        }
        case SbxSTRING:
            if( aData.pString )
            {
                r.WriteByteString( *aData.pString, RTL_TEXTENCODING_ASCII_US );
            }
            else
            {
                String aEmpty;
                r.WriteByteString( aEmpty, RTL_TEXTENCODING_ASCII_US );
            }
            break;
        case SbxERROR:
        case SbxUSHORT:
            r << aData.nUShort; break;
        case SbxOBJECT:
            // sich selbst als Objektptr speichern geht nicht!
            if( aData.pObj )
            {
                if( PTR_CAST(SbxValue,aData.pObj) != this )
                {
                    r << (BYTE) 1;
                    return aData.pObj->Store( r );
                }
                else
                    r << (BYTE) 2;
            }
            else
                r << (BYTE) 0;
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
            BYTE n = SAL_TYPES_SIZEOFINT;
            r << n << (sal_Int32)aData.nInt;
            break;
        }
        case SbxUINT:
        {
            BYTE n = SAL_TYPES_SIZEOFINT;
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
            return FALSE;
    }
    return TRUE;
}

}
