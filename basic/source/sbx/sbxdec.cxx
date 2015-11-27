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

#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#include <memory>

// Implementation SbxDecimal
SbxDecimal::SbxDecimal()
{
    setInt( 0 );
    mnRefCount = 0;
}

SbxDecimal::SbxDecimal( const SbxDecimal& rDec )
{
#ifdef WIN32
    maDec = rDec.maDec;
#else
    (void)rDec;
#endif
    mnRefCount = 0;
}

SbxDecimal::SbxDecimal
    ( const css::bridge::oleautomation::Decimal& rAutomationDec )
{
#ifdef WIN32
    maDec.scale = rAutomationDec.Scale;
    maDec.sign  = rAutomationDec.Sign;
    maDec.Lo32 = rAutomationDec.LowValue;
    maDec.Mid32 = rAutomationDec.MiddleValue;
    maDec.Hi32 = rAutomationDec.HighValue;
#else
    (void)rAutomationDec;
#endif
    mnRefCount = 0;
}

void SbxDecimal::fillAutomationDecimal
    ( css::bridge::oleautomation::Decimal& rAutomationDec )
{
#ifdef WIN32
    rAutomationDec.Scale = maDec.scale;
    rAutomationDec.Sign = maDec.sign;
    rAutomationDec.LowValue = maDec.Lo32;
    rAutomationDec.MiddleValue = maDec.Mid32;
    rAutomationDec.HighValue = maDec.Hi32;
#else
    (void)rAutomationDec;
#endif
}

SbxDecimal::~SbxDecimal()
{
}

void releaseDecimalPtr( SbxDecimal*& rpDecimal )
{
    if( rpDecimal )
    {
        rpDecimal->mnRefCount--;
        if( rpDecimal->mnRefCount == 0 )
        {
            delete rpDecimal;
            rpDecimal = nullptr;
        }
    }
}

#ifdef WIN32

bool SbxDecimal::operator -= ( const SbxDecimal &r )
{
    HRESULT hResult = VarDecSub( &maDec, (LPDECIMAL)&r.maDec, &maDec );
    bool bRet = ( hResult == S_OK );
    return bRet;
}

bool SbxDecimal::operator += ( const SbxDecimal &r )
{
    HRESULT hResult = VarDecAdd( &maDec, (LPDECIMAL)&r.maDec, &maDec );
    bool bRet = ( hResult == S_OK );
    return bRet;
}

bool SbxDecimal::operator /= ( const SbxDecimal &r )
{
    HRESULT hResult = VarDecDiv( &maDec, (LPDECIMAL)&r.maDec, &maDec );
    bool bRet = ( hResult == S_OK );
    return bRet;
}

bool SbxDecimal::operator *= ( const SbxDecimal &r )
{
    HRESULT hResult = VarDecMul( &maDec, (LPDECIMAL)&r.maDec, &maDec );
    bool bRet = ( hResult == S_OK );
    return bRet;
}

bool SbxDecimal::neg()
{
    HRESULT hResult = VarDecNeg( &maDec, &maDec );
    bool bRet = ( hResult == S_OK );
    return bRet;
}

bool SbxDecimal::isZero()
{
    SbxDecimal aZeroDec;
    aZeroDec.setLong( 0 );
    bool bZero = ( EQ == compare( *this, aZeroDec ) );
    return bZero;
}

SbxDecimal::CmpResult compare( const SbxDecimal &rLeft, const SbxDecimal &rRight )
{
    HRESULT hResult = VarDecCmp( (LPDECIMAL)&rLeft.maDec, (LPDECIMAL)&rRight.maDec );
    SbxDecimal::CmpResult eRes = (SbxDecimal::CmpResult)hResult;
    return eRes;
}

void SbxDecimal::setChar( sal_Unicode val )
{
    VarDecFromUI2( (sal_uInt16)val, &maDec );
}

void SbxDecimal::setByte( sal_uInt8 val )
{
    VarDecFromUI1( (sal_uInt8)val, &maDec );
}

void SbxDecimal::setShort( sal_Int16 val )
{
    VarDecFromI2( (short)val, &maDec );
}

void SbxDecimal::setLong( sal_Int32 val )
{
    VarDecFromI4( (long)val, &maDec );
}

void SbxDecimal::setUShort( sal_uInt16 val )
{
    VarDecFromUI2( (sal_uInt16)val, &maDec );
}

void SbxDecimal::setULong( sal_uInt32 val )
{
    VarDecFromUI4( static_cast<ULONG>(val), &maDec );
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
    setLong( (sal_Int32)val );
}

void SbxDecimal::setUInt( unsigned int val )
{
    setULong( (sal_uInt32)val );
}

bool SbxDecimal::setString( OUString* pOUString )
{
    assert(pOUString);

    static LCID nLANGID = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );

    // Convert delimiter
    sal_Unicode cDecimalSep;
    sal_Unicode cThousandSep;
    ImpGetIntntlSep( cDecimalSep, cThousandSep );

    bool bRet = false;
    HRESULT hResult;
    if( cDecimalSep != '.' || cThousandSep != ',' )
    {
        int nLen = pOUString->getLength();
        std::unique_ptr<sal_Unicode[]> pBuffer(new sal_Unicode[nLen +  1]);
        pBuffer[nLen] = 0;

        const sal_Unicode* pSrc = pOUString->getStr();
        int i;
        for( i = 0 ; i < nLen ; ++i )
            pBuffer[i] = pSrc[i];

        sal_Unicode c;
        i = 0;
        while( (c = pBuffer[i]) != 0 )
        {
            if( c == cDecimalSep )
                pBuffer[i] = '.';
            else if( c == cThousandSep )
                pBuffer[i] = ',';
            i++;
        }
        hResult = VarDecFromStr( (OLECHAR*)pBuffer.get(), nLANGID, 0, &maDec );
    }
    else
    {
        hResult = VarDecFromStr( (OLECHAR*)pOUString->getStr(), nLANGID, 0, &maDec );
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
// !WIN32

bool SbxDecimal::operator -= ( const SbxDecimal &r )
{
    (void)r;
    return false;
}

bool SbxDecimal::operator += ( const SbxDecimal &r )
{
    (void)r;
    return false;
}

bool SbxDecimal::operator /= ( const SbxDecimal &r )
{
    (void)r;
    return false;
}

bool SbxDecimal::operator *= ( const SbxDecimal &r )
{
    (void)r;
    return false;
}

bool SbxDecimal::neg()
{
    return false;
}

bool SbxDecimal::isZero()
{
    return false;
}

SbxDecimal::CmpResult compare( const SbxDecimal &rLeft, const SbxDecimal &rRight )
{
    (void)rLeft;
    (void)rRight;
    return (SbxDecimal::CmpResult)0;
}

void SbxDecimal::setChar( sal_Unicode val )     { (void)val; }
void SbxDecimal::setByte( sal_uInt8 val )           { (void)val; }
void SbxDecimal::setShort( sal_Int16 val )          { (void)val; }
void SbxDecimal::setLong( sal_Int32 val )           { (void)val; }
void SbxDecimal::setUShort( sal_uInt16 val )        { (void)val; }
void SbxDecimal::setULong( sal_uInt32 val )         { (void)val; }
bool SbxDecimal::setSingle( float val )         { (void)val; return false; }
bool SbxDecimal::setDouble( double val )        { (void)val; return false; }
void SbxDecimal::setInt( int val )              { (void)val; }
void SbxDecimal::setUInt( unsigned int val )    { (void)val; }
bool SbxDecimal::setString( OUString* pOUString )    { (void)pOUString;  return false; }

bool SbxDecimal::getChar( sal_Unicode& rVal )   { (void)rVal; return false; }
bool SbxDecimal::getShort( sal_Int16& rVal )        { (void)rVal; return false; }
bool SbxDecimal::getLong( sal_Int32& rVal )         { (void)rVal; return false; }
bool SbxDecimal::getUShort( sal_uInt16& rVal )      { (void)rVal; return false; }
bool SbxDecimal::getULong( sal_uInt32& rVal )       { (void)rVal; return false; }
bool SbxDecimal::getSingle( float& rVal )       { (void)rVal; return false; }
bool SbxDecimal::getDouble( double& rVal )      { (void)rVal; return false; }

#endif

bool SbxDecimal::getString( OUString& rString )
{
#ifdef WIN32
    static LCID nLANGID = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );

    bool bRet = false;

    OLECHAR sz[100];
    BSTR aBStr = SysAllocString( sz );
    if( aBStr != NULL )
    {
        HRESULT hResult = VarBstrFromDec( &maDec, nLANGID, 0, &aBStr );
        bRet = ( hResult == S_OK );
        if( bRet )
        {
            // Convert delimiter
            sal_Unicode cDecimalSep;
            sal_Unicode cThousandSep;
            ImpGetIntntlSep( cDecimalSep, cThousandSep );

            if( cDecimalSep != '.' || cThousandSep != ',' )
            {
                sal_Unicode c;
                int i = 0;
                while( (c = aBStr[i]) != 0 )
                {
                    if( c == '.' )
                        aBStr[i] = cDecimalSep;
                    else if( c == ',' )
                        aBStr[i] = cThousandSep;
                    i++;
                }
            }
            rString = reinterpret_cast<const sal_Unicode*>(aBStr);
        }

        SysFreeString( aBStr );
    }
    return bRet;
#else
    (void)rString;
    return false;
#endif
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
    SbxValues aTmp;
    SbxDecimal* pnDecRes;

    SbxDataType eType = p->eType;
    if( eType == SbxDECIMAL && p->pDecimal )
    {
        pnDecRes = new SbxDecimal( *p->pDecimal );
        pnDecRes->addRef();
        return pnDecRes;
    }
    pnDecRes = new SbxDecimal();
    pnDecRes->addRef();

start:
    switch( +eType )
    {
        case SbxNULL:
            SbxBase::SetError( ERRCODE_SBX_CONVERSION );
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
            if( !pnDecRes->setSingle( p->nSingle ) )
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
            break;
        case SbxCURRENCY:
            {
                if( !pnDecRes->setDouble( ImpCurrencyToDouble( p->nInt64 ) ) )
                    SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
                break;
            }
        case SbxSALINT64:
            {
                if( !pnDecRes->setDouble( (double)p->nInt64 ) )
                    SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
                break;
            }
        case SbxSALUINT64:
            {
                if( !pnDecRes->setDouble( (double)p->uInt64 ) )
                    SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
                break;
            }
        case SbxDATE:
        case SbxDOUBLE:
        {
            double dVal = p->nDouble;
            if( !pnDecRes->setDouble( dVal ) )
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
            break;
        }
        case SbxLPSTR:
        case SbxSTRING:
        case SbxBYREF | SbxSTRING:
            if ( p->pOUString )
                pnDecRes->setString( p->pOUString );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                pnDecRes->setDecimal( pVal->GetDecimal() );
            else
            {
                SbxBase::SetError( ERRCODE_SBX_NO_OBJECT );
                pnDecRes->setShort( 0 );
            }
            break;
        }

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

        // from here on had to be tested
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
            SbxBase::SetError( ERRCODE_SBX_CONVERSION ); pnDecRes->setShort( 0 );
    }
    return pnDecRes;
}

void ImpPutDecimal( SbxValues* p, SbxDecimal* pDec )
{
    if( !pDec )
        return;

    SbxValues aTmp;
start:
    switch( +p->eType )
    {
        // here had to be tested
        case SbxCHAR:
            aTmp.pChar = &p->nChar; goto direct;
        case SbxBYTE:
            aTmp.pByte = &p->nByte; goto direct;
        case SbxULONG:
            aTmp.pULong = &p->nULong; goto direct;
        case SbxERROR:
        case SbxUSHORT:
            aTmp.pUShort = &p->nUShort; goto direct;
        case SbxINTEGER:
        case SbxBOOL:
            aTmp.pInteger = &p->nInteger; goto direct;
        case SbxLONG:
            aTmp.pLong = &p->nLong; goto direct;
        case SbxCURRENCY:
        case SbxSALINT64:
            aTmp.pnInt64 = &p->nInt64; goto direct;
        case SbxSALUINT64:
            aTmp.puInt64 = &p->uInt64; goto direct;

        direct:
            aTmp.eType = SbxDataType( p->eType | SbxBYREF );
            p = &aTmp; goto start;

        // from here on no longer
        case SbxDECIMAL:
        case SbxBYREF | SbxDECIMAL:
        {
            if( pDec != p->pDecimal )
            {
                releaseDecimalPtr( p->pDecimal );
                p->pDecimal = pDec;
                if( pDec )
                    pDec->addRef();
            }
            break;
        }
        case SbxSINGLE:
        {
            float f(0.0);
            pDec->getSingle( f );
            p->nSingle = f;
            break;
        }
        case SbxDATE:
        case SbxDOUBLE:
        {
            double d(0.0);
            pDec->getDouble( d );
            p->nDouble = d;
            break;
        }

        case SbxLPSTR:
        case SbxSTRING:
        case SbxBYREF | SbxSTRING:
            if( !p->pOUString )
                p->pOUString = new OUString;
            pDec->getString( *p->pOUString );
            break;
        case SbxOBJECT:
        {
            SbxValue* pVal = dynamic_cast<SbxValue*>( p->pObj );
            if( pVal )
                pVal->PutDecimal( pDec );
            else
                SbxBase::SetError( ERRCODE_SBX_NO_OBJECT );
            break;
        }

        case SbxBYREF | SbxCHAR:
            if( !pDec->getChar( *p->pChar ) )
            {
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
                *p->pChar = 0;
            }
            break;
        case SbxBYREF | SbxBYTE:
            if( !pDec->getChar( *p->pChar ) )
            {
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
                *p->pByte = 0;
            }
            break;
        case SbxBYREF | SbxINTEGER:
        case SbxBYREF | SbxBOOL:
            if( !pDec->getShort( *p->pInteger ) )
            {
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
                *p->pInteger = 0;
            }
            break;
        case SbxBYREF | SbxERROR:
        case SbxBYREF | SbxUSHORT:
            if( !pDec->getUShort( *p->pUShort ) )
            {
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
                *p->pUShort = 0;
            }
            break;
        case SbxBYREF | SbxLONG:
            if( !pDec->getLong( *p->pLong ) )
            {
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
                *p->pLong = 0;
            }
            break;
        case SbxBYREF | SbxULONG:
            if( !pDec->getULong( *p->pULong ) )
            {
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
                *p->pULong = 0;
            }
            break;
        case SbxBYREF | SbxCURRENCY:
            {
            double d(0.0);
            if( !pDec->getDouble( d ) )
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
            *p->pnInt64 = ImpDoubleToCurrency( d );
            }
            break;
        case SbxBYREF | SbxSALINT64:
            {
            double d(0.0);
            if( !pDec->getDouble( d ) )
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
            else
                *p->pnInt64 = ImpDoubleToSalInt64( d );
            }
            break;
        case SbxBYREF | SbxSALUINT64:
            {
            double d(0.0);
            if( !pDec->getDouble( d ) )
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
            else
                *p->puInt64 = ImpDoubleToSalUInt64( d );
            }
            break;
        case SbxBYREF | SbxSINGLE:
            if( !pDec->getSingle( *p->pSingle ) )
            {
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
                *p->pSingle = 0;
            }
            break;
        case SbxBYREF | SbxDATE:
        case SbxBYREF | SbxDOUBLE:
            if( !pDec->getDouble( *p->pDouble ) )
            {
                SbxBase::SetError( ERRCODE_SBX_OVERFLOW );
                *p->pDouble = 0;
            }
            break;
        default:
            SbxBase::SetError( ERRCODE_SBX_CONVERSION );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
