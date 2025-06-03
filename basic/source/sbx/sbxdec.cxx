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
#ifdef _WIN32
#include <o3tl/char16_t2wchar_t.hxx>
#include <systools/win32/oleauto.hxx>
#endif

#include <basic/sberrors.hxx>
#include "sbxconv.hxx"

#include <com/sun/star/bridge/oleautomation/Decimal.hpp>

// Implementation SbxDecimal
SbxDecimal::SbxDecimal()
    : mnRefCount(0)
{
    setInt( 0 );
}

SbxDecimal::SbxDecimal( const SbxDecimal& rDec )
    : mnRefCount(0)
{
#ifdef _WIN32
    maDec = rDec.maDec;
#else
    (void)rDec;
#endif
}

SbxDecimal::SbxDecimal
    ( const css::bridge::oleautomation::Decimal& rAutomationDec )
    : mnRefCount(0)
{
#ifdef _WIN32
    maDec.scale = rAutomationDec.Scale;
    maDec.sign  = rAutomationDec.Sign;
    maDec.Lo32 = rAutomationDec.LowValue;
    maDec.Mid32 = rAutomationDec.MiddleValue;
    maDec.Hi32 = rAutomationDec.HighValue;
#else
    (void)rAutomationDec;
#endif
}

void SbxDecimal::fillAutomationDecimal
    ( css::bridge::oleautomation::Decimal& rAutomationDec )
{
#ifdef _WIN32
    rAutomationDec.Scale = maDec.scale;
    rAutomationDec.Sign = maDec.sign;
    rAutomationDec.LowValue = maDec.Lo32;
    rAutomationDec.MiddleValue = maDec.Mid32;
    rAutomationDec.HighValue = maDec.Hi32;
#else
    (void)rAutomationDec;
#endif
}

void releaseDecimalPtr( SbxDecimal*& rpDecimal )
{
    if( rpDecimal )
    {
        rpDecimal->mnRefCount--;
        if( rpDecimal->mnRefCount == 0 )
            delete rpDecimal;
        rpDecimal = nullptr;
    }
}

#ifdef _WIN32

bool SbxDecimal::operator -= ( const SbxDecimal &r )
{
    HRESULT hResult = VarDecSub( &maDec, const_cast<LPDECIMAL>(&r.maDec), &maDec );
    bool bRet = ( hResult == S_OK );
    return bRet;
}

bool SbxDecimal::operator += ( const SbxDecimal &r )
{
    HRESULT hResult = VarDecAdd( &maDec, const_cast<LPDECIMAL>(&r.maDec), &maDec );
    bool bRet = ( hResult == S_OK );
    return bRet;
}

bool SbxDecimal::operator /= ( const SbxDecimal &r )
{
    HRESULT hResult = VarDecDiv( &maDec, const_cast<LPDECIMAL>(&r.maDec), &maDec );
    bool bRet = ( hResult == S_OK );
    return bRet;
}

bool SbxDecimal::operator *= ( const SbxDecimal &r )
{
    HRESULT hResult = VarDecMul( &maDec, const_cast<LPDECIMAL>(&r.maDec), &maDec );
    bool bRet = ( hResult == S_OK );
    return bRet;
}

bool SbxDecimal::neg()
{
    HRESULT hResult = VarDecNeg( &maDec, &maDec );
    bool bRet = ( hResult == S_OK );
    return bRet;
}

bool SbxDecimal::isZero() const
{
    SbxDecimal aZeroDec;
    aZeroDec.setLong( 0 );
    bool bZero = CmpResult::EQ == compare( *this, aZeroDec );
    return bZero;
}

SbxDecimal::CmpResult compare( const SbxDecimal &rLeft, const SbxDecimal &rRight )
{
    HRESULT hResult = VarDecCmp( const_cast<LPDECIMAL>(&rLeft.maDec), const_cast<LPDECIMAL>(&rRight.maDec) );
    SbxDecimal::CmpResult eRes = static_cast<SbxDecimal::CmpResult>(hResult);
    return eRes;
}

void SbxDecimal::setChar( sal_Unicode val )
{
    VarDecFromUI2( static_cast<sal_uInt16>(val), &maDec );
}

void SbxDecimal::setByte( sal_uInt8 val )
{
    VarDecFromUI1( val, &maDec );
}

void SbxDecimal::setShort( sal_Int16 val )
{
    VarDecFromI2( static_cast<short>(val), &maDec );
}

void SbxDecimal::setLong( sal_Int32 val )
{
    VarDecFromI4(static_cast<LONG>(val), &maDec);
}

bool SbxDecimal::setHyper( sal_Int64 val )
{
    return SUCCEEDED(VarDecFromI8(static_cast<LONG64>(val), &maDec));
}

void SbxDecimal::setUShort( sal_uInt16 val )
{
    VarDecFromUI2( val, &maDec );
}

void SbxDecimal::setULong( sal_uInt32 val )
{
    VarDecFromUI4( static_cast<ULONG>(val), &maDec );
}

bool SbxDecimal::setUHyper( sal_uInt64 val )
{
    return SUCCEEDED(VarDecFromUI8( static_cast<ULONG64>(val), &maDec ));
}

bool SbxDecimal::setSingle( float val )
{
    bool bRet = ( VarDecFromR4( val, &maDec ) == S_OK );
    return bRet;
}

bool SbxDecimal::setDouble( double val )
{
    bool bRet = ( VarDecFromR8( val, &maDec ) == S_OK );
    return bRet;
}

void SbxDecimal::setInt( int val )
{
    setLong( static_cast<sal_Int32>(val) );
}

void SbxDecimal::setUInt( unsigned int val )
{
    setULong( static_cast<sal_uInt32>(val) );
}

bool SbxDecimal::setString( OUString* pOUString )
{
    assert(pOUString);

    static LCID nLANGID = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );

    // Convert delimiter
    sal_Unicode cDecimalSep;
    sal_Unicode cThousandSep;
    sal_Unicode cDecimalSepAlt;
    ImpGetIntntlSep( cDecimalSep, cThousandSep, cDecimalSepAlt );

    bool bRet = false;
    HRESULT hResult;
    if( cDecimalSep != '.' || cThousandSep != ',' )
    {
        int nLen = pOUString->getLength();
        std::unique_ptr<sal_Unicode[]> pBuffer(new sal_Unicode[nLen +  1]);
        pBuffer[nLen] = 0;

        const sal_Unicode* pSrc = pOUString->getStr();
        for( int i = 0 ; i < nLen ; ++i )
        {
            sal_Unicode c = pSrc[i];
            if (c == cDecimalSep)
                c = '.';
            else if (c == cThousandSep)
                c = ',';

            pBuffer[i] = c;
        }
        hResult = VarDecFromStr( o3tl::toW(pBuffer.get()), nLANGID, 0, &maDec );
    }
    else
    {
        hResult = VarDecFromStr( o3tl::toW(pOUString->getStr()), nLANGID, 0, &maDec );
    }
    bRet = ( hResult == S_OK );
    return bRet;
}


bool SbxDecimal::getChar( sal_Unicode& rVal )
{
    USHORT n;
    bool bRet = ( VarUI2FromDec( &maDec, &n ) == S_OK );
    if (bRet) {
        rVal = n;
    }
    return bRet;
}

bool SbxDecimal::getByte( sal_uInt8& rVal )
{
    bool bRet = ( VarUI1FromDec( &maDec, &rVal ) == S_OK );
    return bRet;
}

bool SbxDecimal::getShort( sal_Int16& rVal )
{
    bool bRet = ( VarI2FromDec( &maDec, &rVal ) == S_OK );
    return bRet;
}

bool SbxDecimal::getLong( sal_Int32& rVal )
{
    bool bRet = ( VarI4FromDec( &maDec, &rVal ) == S_OK );
    return bRet;
}

bool SbxDecimal::getHyper( sal_Int64& rVal )
{
    bool bRet = ( VarI8FromDec( &maDec, &rVal ) == S_OK );
    return bRet;
}

bool SbxDecimal::getUShort( sal_uInt16& rVal )
{
    bool bRet = ( VarUI2FromDec( &maDec, &rVal ) == S_OK );
    return bRet;
}

bool SbxDecimal::getULong( sal_uInt32& rVal )
{
    bool bRet = ( VarUI4FromDec( &maDec, &rVal ) == S_OK );
    return bRet;
}

bool SbxDecimal::getUHyper( sal_uInt64& rVal )
{
    bool bRet = ( VarUI8FromDec( &maDec, &rVal ) == S_OK );
    return bRet;
}

bool SbxDecimal::getSingle( float& rVal )
{
    bool bRet = ( VarR4FromDec( &maDec, &rVal ) == S_OK );
    return bRet;
}

bool SbxDecimal::getDouble( double& rVal )
{
    bool bRet = ( VarR8FromDec( &maDec, &rVal ) == S_OK );
    return bRet;
}

#else
// !_WIN32

bool SbxDecimal::operator -= ( const SbxDecimal & )
{
    return false;
}

bool SbxDecimal::operator += ( const SbxDecimal & )
{
    return false;
}

bool SbxDecimal::operator /= ( const SbxDecimal & )
{
    return false;
}

bool SbxDecimal::operator *= ( const SbxDecimal & )
{
    return false;
}

bool SbxDecimal::neg()
{
    return false;
}

bool SbxDecimal::isZero() const
{
    return false;
}

SbxDecimal::CmpResult compare( SAL_UNUSED_PARAMETER const SbxDecimal &, SAL_UNUSED_PARAMETER const SbxDecimal & )
{
    return SbxDecimal::CmpResult::LT;
}

void SbxDecimal::setChar( SAL_UNUSED_PARAMETER sal_Unicode ) {}
void SbxDecimal::setByte( SAL_UNUSED_PARAMETER sal_uInt8 ) {}
void SbxDecimal::setShort( SAL_UNUSED_PARAMETER sal_Int16 ) {}
void SbxDecimal::setLong( SAL_UNUSED_PARAMETER sal_Int32 ) {}
bool SbxDecimal::setHyper( SAL_UNUSED_PARAMETER sal_Int64 ) { return false; }
void SbxDecimal::setUShort( SAL_UNUSED_PARAMETER sal_uInt16 ) {}
void SbxDecimal::setULong( SAL_UNUSED_PARAMETER sal_uInt32 ) {}
bool SbxDecimal::setUHyper( SAL_UNUSED_PARAMETER sal_uInt64 ) { return false; }
bool SbxDecimal::setSingle( SAL_UNUSED_PARAMETER float ) { return false; }
bool SbxDecimal::setDouble( SAL_UNUSED_PARAMETER double ) { return false; }
void SbxDecimal::setInt( SAL_UNUSED_PARAMETER int ) {}
void SbxDecimal::setUInt( SAL_UNUSED_PARAMETER unsigned int ) {}
bool SbxDecimal::setString( SAL_UNUSED_PARAMETER OUString* ) { return false; }

bool SbxDecimal::getChar( SAL_UNUSED_PARAMETER sal_Unicode& ) { return false; }
bool SbxDecimal::getByte( SAL_UNUSED_PARAMETER sal_uInt8& ) { return false; }
bool SbxDecimal::getShort( SAL_UNUSED_PARAMETER sal_Int16& ) { return false; }
bool SbxDecimal::getLong( SAL_UNUSED_PARAMETER sal_Int32& ) { return false; }
bool SbxDecimal::getHyper( SAL_UNUSED_PARAMETER sal_Int64& ) { return false; }
bool SbxDecimal::getUShort( SAL_UNUSED_PARAMETER sal_uInt16& ) { return false; }
bool SbxDecimal::getULong( SAL_UNUSED_PARAMETER sal_uInt32& ) { return false; }
bool SbxDecimal::getUHyper( SAL_UNUSED_PARAMETER sal_uInt64& ) { return false; }
bool SbxDecimal::getSingle( SAL_UNUSED_PARAMETER float& ) { return false; }
bool SbxDecimal::getDouble( SAL_UNUSED_PARAMETER double& ) { return false; }

#endif

void SbxDecimal::getString( OUString& rString )
{
#ifdef _WIN32
    static LCID nLANGID = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );

    sal::systools::BStr pBStr;
    // VarBstrFromDec allocates new BSTR that needs to be released with SysFreeString
    HRESULT hResult = VarBstrFromDec( &maDec, nLANGID, 0, &pBStr );
    if( hResult == S_OK )
    {
        // Convert delimiter
        sal_Unicode cDecimalSep;
        sal_Unicode cThousandSep;
        sal_Unicode cDecimalSepAlt;
        ImpGetIntntlSep( cDecimalSep, cThousandSep, cDecimalSepAlt );

        if( cDecimalSep != '.' || cThousandSep != ',' )
        {
            sal_Unicode c;
            int i = 0;
            while( (c = pBStr[i]) != 0 )
            {
                if( c == '.' )
                    pBStr[i] = cDecimalSep;
                else if( c == ',' )
                    pBStr[i] = cThousandSep;
                i++;
            }
        }
        rString = pBStr;
    }
#else
    (void)rString;
#endif
}

void SbxDecimal::HandleFailure(bool isSuccess)
{
    if (!isSuccess)
        SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
}

SbxDecimal* ImpCreateDecimal( SbxValues* p )
{
    if( !p )
        return nullptr;

    SbxDecimal*& rpDecimal = p->pDecimal;
    if( rpDecimal == nullptr )
    {
        rpDecimal = new SbxDecimal();
        rpDecimal->addRef();
    }
    return rpDecimal;
}

SbxDecimal* ImpGetDecimal( const SbxValues* p )
{
    SbxDataType eType = p->eType;
    if( eType == SbxDECIMAL && p->pDecimal )
    {
        SbxDecimal* pnDecRes = new SbxDecimal( *p->pDecimal );
        pnDecRes->addRef();
        return pnDecRes;
    }
    SbxDecimal* pnDecRes = new SbxDecimal();
    pnDecRes->addRef();

    switch( +eType )
    {
        case SbxEMPTY:
            pnDecRes->setShort( 0 ); break;
        case SbxCHAR:
            pnDecRes->setChar( p->nChar ); break;
        case SbxBYTE:
            pnDecRes->setByte( p->nByte ); break;
        case SbxINTEGER:
        case SbxBOOL:
            pnDecRes->setInt( p->nInteger ); break;
        case SbxERROR:
        case SbxUSHORT:
            pnDecRes->setUShort( p->nUShort ); break;
        case SbxLONG:
            pnDecRes->setLong( p->nLong ); break;
        case SbxULONG:
            pnDecRes->setULong( p->nULong ); break;
        case SbxSINGLE:
            pnDecRes->setWithOverflow(p->nSingle);
            break;
        case SbxBYREF | SbxSINGLE:
            pnDecRes->setWithOverflow(*p->pSingle);
            break;
        case SbxCURRENCY:
            pnDecRes->setWithOverflow(CurTo<double>(p->nInt64));
            break;
        case SbxBYREF | SbxCURRENCY:
            pnDecRes->setWithOverflow(CurTo<double>(*p->pnInt64));
            break;
        case SbxSALINT64:
            pnDecRes->setWithOverflow(p->nInt64);
            break;
        case SbxBYREF | SbxSALINT64:
            pnDecRes->setWithOverflow(*p->pnInt64);
            break;
        case SbxSALUINT64:
            pnDecRes->setWithOverflow(p->uInt64);
            break;
        case SbxBYREF | SbxSALUINT64:
            pnDecRes->setWithOverflow(*p->puInt64);
            break;
        case SbxDATE:
        case SbxDOUBLE:
            pnDecRes->setWithOverflow(p->nDouble);
            break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            pnDecRes->setWithOverflow(*p->pDouble);
            break;
        case SbxLPSTR:
        case SbxSTRING:
        case SbxBYREF | SbxSTRING:
            if ( p->pOUString )
                pnDecRes->setString( p->pOUString );
            break;
        case SbxOBJECT:
            if (SbxValue* pVal = dynamic_cast<SbxValue*>(p->pObj))
                pnDecRes->setDecimal( pVal->GetDecimal() );
            else
            {
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
                pnDecRes->setShort( 0 );
            }
            break;

        case SbxBYREF | SbxCHAR:
            pnDecRes->setChar( *p->pChar ); break;
        case SbxBYREF | SbxBYTE:
            pnDecRes->setByte( *p->pByte ); break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            pnDecRes->setInt( *p->pInteger ); break;
        case SbxBYREF | SbxLONG:
            pnDecRes->setLong( *p->pLong ); break;
        case SbxBYREF | SbxULONG:
            pnDecRes->setULong( *p->pULong ); break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            pnDecRes->setUShort( *p->pUShort ); break;

        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION ); pnDecRes->setShort( 0 );
    }
    return pnDecRes;
}

void ImpPutDecimal( SbxValues* p, SbxDecimal* pDec )
{
    if( !pDec )
        return;

    switch( +p->eType )
    {
        case SbxCHAR:
            assignWithOverflowTo(p->nChar, *pDec);
            break;
        case SbxBYREF | SbxCHAR:
            assignWithOverflowTo(*p->pChar, *pDec);
            break;
        case SbxBYTE:
            assignWithOverflowTo(p->nByte, *pDec);
            break;
        case SbxBYREF | SbxBYTE:
            assignWithOverflowTo(*p->pByte, *pDec);
            break;
        case SbxINTEGER:
        case SbxBOOL:
            assignWithOverflowTo(p->nInteger, *pDec);
            break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            assignWithOverflowTo(*p->pInteger, *pDec);
            break;
        case SbxERROR:
        case SbxUSHORT:
            assignWithOverflowTo(p->nUShort, *pDec);
            break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            assignWithOverflowTo(*p->pUShort, *pDec);
            break;
        case SbxLONG:
            assignWithOverflowTo(p->nLong, *pDec);
            break;
        case SbxBYREF | SbxLONG:
            assignWithOverflowTo(*p->pLong, *pDec);
            break;
        case SbxULONG:
            assignWithOverflowTo(p->nULong, *pDec);
            break;
        case SbxBYREF | SbxULONG:
            assignWithOverflowTo(*p->pULong, *pDec);
            break;
        case SbxCURRENCY:
            p->nInt64 = CurFrom(pDec->getWithOverflow<double>());
            break;
        case SbxBYREF | SbxCURRENCY:
            *p->pnInt64 = CurFrom(pDec->getWithOverflow<double>());
            break;
        case SbxSALINT64:
            assignWithOverflowTo(p->nInt64, *pDec);
            break;
        case SbxBYREF | SbxSALINT64:
            assignWithOverflowTo(*p->pnInt64, *pDec);
            break;
        case SbxSALUINT64:
            assignWithOverflowTo(p->uInt64, *pDec);
            break;
        case SbxBYREF | SbxSALUINT64:
            assignWithOverflowTo(*p->puInt64, *pDec);
            break;

        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
            if( pDec != p->pDecimal )
            {
                releaseDecimalPtr( p->pDecimal );
                p->pDecimal = pDec;
                if( pDec )
                    pDec->addRef();
            }
            break;
        case SbxSINGLE:
            assignWithOverflowTo(p->nSingle, *pDec);
            break;
        case SbxBYREF | SbxSINGLE:
            assignWithOverflowTo(*p->pSingle, *pDec);
            break;
        case SbxDATE:
        case SbxDOUBLE:
            assignWithOverflowTo(p->nDouble, *pDec);
            break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            assignWithOverflowTo(*p->pDouble, *pDec);
            break;

        case SbxLPSTR:
        case SbxSTRING:
        case SbxBYREF | SbxSTRING:
            if( !p->pOUString )
                p->pOUString = new OUString;
            pDec->getString( *p->pOUString );
            break;
        case SbxOBJECT:
            if (SbxValue* pVal = dynamic_cast<SbxValue*>(p->pObj))
                pVal->PutDecimal( pDec );
            else
                SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
            break;

        default:
            SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
