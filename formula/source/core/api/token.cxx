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


#include <cstddef>
#include <cstdio>
#include <algorithm>

#include <string.h>
#include <limits.h>
#include <osl/diagnose.h>

#include <com/sun/star/sheet/FormulaToken.hpp>
#include <formula/errorcodes.hxx>
#include <formula/token.hxx>
#include <formula/tokenarray.hxx>
#include <formula/FormulaCompiler.hxx>
#include <formula/compiler.hxx>
#include <svl/sharedstringpool.hxx>
#include <memory>

namespace formula
{
    using namespace com::sun::star;

// Align MemPools on 4k boundaries - 64 bytes (4k is a MUST for OS/2)

// Need a lot of FormulaDoubleToken
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( FormulaDoubleToken )
// Need quite some FormulaTypedDoubleToken
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( FormulaTypedDoubleToken )
// Need a lot of FormulaByteToken
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( FormulaByteToken )
// Need several FormulaStringToken
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( FormulaStringToken )


// --- helpers --------------------------------------------------------------

inline bool lcl_IsReference( OpCode eOp, StackVar eType )
{
    return
        (eOp == ocPush && (eType == svSingleRef || eType == svDoubleRef))
        || (eOp == ocColRowNameAuto && eType == svDoubleRef)
        || (eOp == ocColRowName && eType == svSingleRef)
        || (eOp == ocMatRef && eType == svSingleRef)
        ;
}

// --- class FormulaToken --------------------------------------------------------

FormulaToken::FormulaToken( StackVar eTypeP, OpCode e ) :
    eOp(e), eType( eTypeP ), mnRefCnt(0)
{
}

FormulaToken::FormulaToken( const FormulaToken& r ) :
    IFormulaToken(), eOp(r.eOp), eType( r.eType ), mnRefCnt(0)
{
}

FormulaToken::~FormulaToken()
{
}

bool FormulaToken::IsFunction() const
{
    return (eOp != ocPush && eOp != ocBad && eOp != ocColRowName &&
            eOp != ocColRowNameAuto && eOp != ocName && eOp != ocDBArea &&
            eOp != ocTableRef &&
           (GetByte() != 0                                                  // x parameters
        || (SC_OPCODE_START_NO_PAR <= eOp && eOp < SC_OPCODE_STOP_NO_PAR)   // no parameter
        || FormulaCompiler::IsOpCodeJumpCommand( eOp )                      // @ jump commands
        || (SC_OPCODE_START_1_PAR <= eOp && eOp < SC_OPCODE_STOP_1_PAR)     // one parameter
        || (SC_OPCODE_START_2_PAR <= eOp && eOp < SC_OPCODE_STOP_2_PAR)     // x parameters (cByte==0 in
                                                                            // FuncAutoPilot)
        || eOp == ocMacro || eOp == ocExternal                  // macros, AddIns
        || eOp == ocAnd || eOp == ocOr                          // former binary, now x parameters
        || (eOp >= ocInternalBegin && eOp <= ocInternalEnd)     // internal
        ));
}


sal_uInt8 FormulaToken::GetParamCount() const
{
    if ( eOp < SC_OPCODE_STOP_DIV && eOp != ocExternal && eOp != ocMacro &&
         !FormulaCompiler::IsOpCodeJumpCommand( eOp ) &&
         eOp != ocPercentSign )
        return 0;       // parameters and specials
                        // ocIf... jump commands not for FAP, have cByte then
//2do: bool parameter whether FAP or not?
    else if ( GetByte() )
        return GetByte();   // all functions, also ocExternal and ocMacro
    else if (SC_OPCODE_START_BIN_OP <= eOp && eOp < SC_OPCODE_STOP_BIN_OP)
        return 2;           // binary
    else if ((SC_OPCODE_START_UN_OP <= eOp && eOp < SC_OPCODE_STOP_UN_OP)
            || eOp == ocPercentSign)
        return 1;           // unary
    else if (SC_OPCODE_START_NO_PAR <= eOp && eOp < SC_OPCODE_STOP_NO_PAR)
        return 0;           // no parameter
    else if (SC_OPCODE_START_1_PAR <= eOp && eOp < SC_OPCODE_STOP_1_PAR)
        return 1;           // one parameter
    else if (FormulaCompiler::IsOpCodeJumpCommand( eOp ))
        return 1;           // only the condition counts as parameter
    else
        return 0;           // all the rest, no Parameter, or
                            // if so then it should be in cByte
}

bool FormulaToken::IsExternalRef() const
{
    bool bRet = false;
    switch (eType)
    {
        case svExternalSingleRef:
        case svExternalDoubleRef:
        case svExternalName:
            bRet = true;
            break;
        default:
            bRet = false;
            break;
    }
    return bRet;
}

bool FormulaToken::IsRef() const
{
    switch (eType)
    {
        case svSingleRef:
        case svDoubleRef:
        case svExternalSingleRef:
        case svExternalDoubleRef:
            return true;
        default:
            if (eOp == ocTableRef)
                return true;
    }

    return false;
}

bool FormulaToken::IsInForceArray() const
{
    ParamClass eParam = GetInForceArray();
    return eParam == ParamClass::ForceArray || eParam == ParamClass::ReferenceOrForceArray;
}

bool FormulaToken::operator==( const FormulaToken& rToken ) const
{
    // don't compare reference count!
    return  eType == rToken.eType && GetOpCode() == rToken.GetOpCode();
}


// --- virtual dummy methods -------------------------------------------------

sal_uInt8 FormulaToken::GetByte() const
{
    // ok to be called for any derived class
    return 0;
}

void FormulaToken::SetByte( sal_uInt8 )
{
    assert( !"virtual dummy called" );
}

ParamClass FormulaToken::GetInForceArray() const
{
    // ok to be called for any derived class
    return ParamClass::Unknown;
}

void FormulaToken::SetInForceArray( ParamClass )
{
    assert( !"virtual dummy called" );
}

double FormulaToken::GetDouble() const
{
    // This Get is worth an assert.
    assert( !"virtual dummy called" );
    return 0.0;
}

double & FormulaToken::GetDoubleAsReference()
{
    // This Get is worth an assert.
    assert( !"virtual dummy called" );
    static double fVal = 0.0;
    return fVal;
}

sal_Int16 FormulaToken::GetDoubleType() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetDoubleType: virtual dummy called" );
    return 0;
}

void FormulaToken::SetDoubleType( sal_Int16 )
{
    assert( !"virtual dummy called" );
}

svl::SharedString FormulaToken::GetString() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetString: virtual dummy called" );
    return svl::SharedString(); // invalid string
}

void FormulaToken::SetString( const svl::SharedString& )
{
    assert( !"virtual dummy called" );
}

sal_uInt16 FormulaToken::GetIndex() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetIndex: virtual dummy called" );
    return 0;
}

void FormulaToken::SetIndex( sal_uInt16 )
{
    assert( !"virtual dummy called" );
}

sal_Int16 FormulaToken::GetSheet() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetSheet: virtual dummy called" );
    return -1;
}

void FormulaToken::SetSheet( sal_Int16 )
{
    assert( !"virtual dummy called" );
}

short* FormulaToken::GetJump() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetJump: virtual dummy called" );
    return nullptr;
}


const OUString& FormulaToken::GetExternal() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetExternal: virtual dummy called" );
    static  OUString              aDummyString;
    return aDummyString;
}

FormulaToken* FormulaToken::GetFAPOrigToken() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetFAPOrigToken: virtual dummy called" );
    return nullptr;
}

FormulaError FormulaToken::GetError() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetError: virtual dummy called" );
    return FormulaError::NONE;
}

void FormulaToken::SetError( FormulaError )
{
    assert( !"virtual dummy called" );
}

const ScSingleRefData* FormulaToken::GetSingleRef() const
{
    OSL_FAIL( "FormulaToken::GetSingleRef: virtual dummy called" );
    return nullptr;
}

ScSingleRefData* FormulaToken::GetSingleRef()
{
    OSL_FAIL( "FormulaToken::GetSingleRef: virtual dummy called" );
    return nullptr;
}

const ScComplexRefData* FormulaToken::GetDoubleRef() const
{
    OSL_FAIL( "FormulaToken::GetDoubleRef: virtual dummy called" );
    return nullptr;
}

ScComplexRefData* FormulaToken::GetDoubleRef()
{
    OSL_FAIL( "FormulaToken::GetDoubleRef: virtual dummy called" );
    return nullptr;
}

const ScSingleRefData* FormulaToken::GetSingleRef2() const
{
    OSL_FAIL( "FormulaToken::GetSingleRef2: virtual dummy called" );
    return nullptr;
}

ScSingleRefData* FormulaToken::GetSingleRef2()
{
    OSL_FAIL( "FormulaToken::GetSingleRef2: virtual dummy called" );
    return nullptr;
}

const ScMatrix* FormulaToken::GetMatrix() const
{
    OSL_FAIL( "FormulaToken::GetMatrix: virtual dummy called" );
    return nullptr;
}

ScMatrix* FormulaToken::GetMatrix()
{
    OSL_FAIL( "FormulaToken::GetMatrix: virtual dummy called" );
    return nullptr;
}

ScJumpMatrix* FormulaToken::GetJumpMatrix() const
{
    OSL_FAIL( "FormulaToken::GetJumpMatrix: virtual dummy called" );
    return nullptr;
}
const std::vector<ScComplexRefData>* FormulaToken::GetRefList() const
{
    OSL_FAIL( "FormulaToken::GetRefList: virtual dummy called" );
    return nullptr;
}

std::vector<ScComplexRefData>* FormulaToken::GetRefList()
{
    OSL_FAIL( "FormulaToken::GetRefList: virtual dummy called" );
    return nullptr;
}

bool FormulaToken::TextEqual( const FormulaToken& rToken ) const
{
    return *this == rToken;
}

// real implementations of virtual functions


sal_uInt8   FormulaByteToken::GetByte() const           { return nByte; }
void        FormulaByteToken::SetByte( sal_uInt8 n )    { nByte = n; }
ParamClass  FormulaByteToken::GetInForceArray() const    { return eInForceArray; }
void        FormulaByteToken::SetInForceArray( ParamClass c ) { eInForceArray = c; }
bool FormulaByteToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && nByte == r.GetByte() &&
        eInForceArray == r.GetInForceArray();
}


FormulaToken* FormulaFAPToken::GetFAPOrigToken() const  { return pOrigToken.get(); }
bool FormulaFAPToken::operator==( const FormulaToken& r ) const
{
    return FormulaByteToken::operator==( r ) && pOrigToken == r.GetFAPOrigToken();
}


short*      FormulaJumpToken::GetJump() const                   { return pJump.get(); }
ParamClass  FormulaJumpToken::GetInForceArray() const           { return eInForceArray; }
void        FormulaJumpToken::SetInForceArray( ParamClass c )   { eInForceArray = c; }
bool FormulaJumpToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && pJump[0] == r.GetJump()[0] &&
        memcmp( pJump.get()+1, r.GetJump()+1, pJump[0] * sizeof(short) ) == 0 &&
        eInForceArray == r.GetInForceArray();
}
FormulaJumpToken::~FormulaJumpToken()
{
}


bool FormulaTokenArray::AddFormulaToken(
    const sheet::FormulaToken& rToken, svl::SharedStringPool& rSPool, ExternalReferenceHelper* /*pExtRef*/)
{
    bool bError = false;
    const OpCode eOpCode = static_cast<OpCode>(rToken.OpCode);      //! assuming equal values for the moment

    const uno::TypeClass eClass = rToken.Data.getValueTypeClass();
    switch ( eClass )
    {
        case uno::TypeClass_VOID:
            // empty data -> use AddOpCode (does some special cases)
            AddOpCode( eOpCode );
            break;
        case uno::TypeClass_DOUBLE:
            // double is only used for "push"
            if ( eOpCode == ocPush )
                AddDouble( rToken.Data.get<double>() );
            else
                bError = true;
            break;
        case uno::TypeClass_LONG:
            {
                // long is svIndex, used for name / database area, or "byte" for spaces
                sal_Int32 nValue = rToken.Data.get<sal_Int32>();
                if ( eOpCode == ocDBArea )
                    Add( new formula::FormulaIndexToken( eOpCode, static_cast<sal_uInt16>(nValue) ) );
                else if ( eOpCode == ocTableRef )
                    bError = true;  /* TODO: implementation */
                else if ( eOpCode == ocSpaces )
                    Add( new formula::FormulaByteToken( ocSpaces, static_cast<sal_uInt8>(nValue) ) );
                else
                    bError = true;
            }
            break;
        case uno::TypeClass_STRING:
            {
                OUString aStrVal( rToken.Data.get<OUString>() );
                if ( eOpCode == ocPush )
                    AddString(rSPool.intern(aStrVal));
                else if ( eOpCode == ocBad )
                    AddBad( aStrVal );
                else if ( eOpCode == ocStringXML )
                    AddStringXML( aStrVal );
                else if ( eOpCode == ocExternal || eOpCode == ocMacro )
                    Add( new formula::FormulaExternalToken( eOpCode, aStrVal ) );
                else
                    bError = true;      // unexpected string: don't know what to do with it
            }
            break;
        default:
            bError = true;
    } // switch ( eClass )
    return bError;
}

bool FormulaTokenArray::Fill(
    const uno::Sequence<sheet::FormulaToken>& rSequence,
    svl::SharedStringPool& rSPool, ExternalReferenceHelper* pExtRef )
{
    bool bError = false;
    const sal_Int32 nCount = rSequence.getLength();
    for (sal_Int32 nPos=0; nPos<nCount; nPos++)
    {
        bool bOneError = AddFormulaToken(rSequence[nPos], rSPool, pExtRef);
        if (bOneError)
        {
            AddOpCode( ocErrName);  // add something that indicates an error
            bError = true;
        }
    }
    return bError;
}

void FormulaTokenArray::DelRPN()
{
    if( nRPN )
    {
        FormulaToken** p = pRPN;
        for( sal_uInt16 i = 0; i < nRPN; i++ )
        {
            (*p++)->DecRef();
        }
        delete [] pRPN;
    }
    pRPN = nullptr;
    nRPN = 0;
}

FormulaToken* FormulaTokenArray::FirstToken() const
{
    if (!pCode || nLen == 0)
        return nullptr;
    return pCode[0];
}

FormulaToken* FormulaTokenArray::PeekPrev( sal_uInt16 & nIdx ) const
{
    if (0 < nIdx && nIdx <= nLen)
        return pCode[--nIdx];
    return nullptr;
}

FormulaToken* FormulaTokenArray::FirstRPNToken() const
{
    if (!pRPN || nRPN == 0)
        return nullptr;
    return pRPN[0];
}

bool FormulaTokenArray::HasReferences() const
{
    for (auto i: Tokens())
    {
        if (i->IsRef())
            return true;
    }

    for (auto i: RPNTokens())
    {
        if (i->IsRef())
            return true;
    }

    return false;
}

bool FormulaTokenArray::HasExternalRef() const
{
    for (auto i: Tokens())
    {
        if (i->IsExternalRef())
            return true;
    }
    return false;
}

bool FormulaTokenArray::HasOpCode( OpCode eOp ) const
{
    for (auto i: Tokens())
    {
        if (i->GetOpCode() == eOp)
            return true;
    }
    return false;
}

bool FormulaTokenArray::HasOpCodeRPN( OpCode eOp ) const
{
    for (auto i: RPNTokens())
    {
        if (i->GetOpCode() == eOp)
            return true;
    }
    return false;
}

bool FormulaTokenArray::HasNameOrColRowName() const
{
    for (auto i: Tokens())
    {
        if (i->GetType() == svIndex || i->GetOpCode() == ocColRowName )
            return true;
    }
    return false;
}

bool FormulaTokenArray::HasOpCodes(const unordered_opcode_set& rOpCodes) const
{
    for (auto i: Tokens())
    {
        if (rOpCodes.count(i->GetOpCode()) > 0)
            return true;
    }

    return false;
}

FormulaTokenArray::FormulaTokenArray() :
    pCode(nullptr),
    pRPN(nullptr),
    nLen(0),
    nRPN(0),
    nError(FormulaError::NONE),
    nMode(ScRecalcMode::NORMAL),
    bHyperLink(false),
    mbFromRangeName(false),
    mbShareable(true),
    mbFinalized(false)
{
}

FormulaTokenArray::FormulaTokenArray( const FormulaTokenArray& rArr )
{
    Assign( rArr );
}

FormulaTokenArray::~FormulaTokenArray()
{
    Clear();
}

void FormulaTokenArray::Assign( const FormulaTokenArray& r )
{
    nLen   = r.nLen;
    nRPN   = r.nRPN;
    nError = r.nError;
    nMode  = r.nMode;
    bHyperLink = r.bHyperLink;
    mbFromRangeName = r.mbFromRangeName;
    mbShareable = r.mbShareable;
    mbFinalized = r.mbFinalized;
    pCode  = nullptr;
    pRPN   = nullptr;
    FormulaToken** pp;
    if( nLen )
    {
        pCode.reset(new FormulaToken*[ nLen ]);
        pp = pCode.get();
        memcpy( pp, r.pCode.get(), nLen * sizeof( FormulaToken* ) );
        for( sal_uInt16 i = 0; i < nLen; i++ )
            (*pp++)->IncRef();
        mbFinalized = true;
    }
    if( nRPN )
    {
        pp = pRPN = new FormulaToken*[ nRPN ];
        memcpy( pp, r.pRPN, nRPN * sizeof( FormulaToken* ) );
        for( sal_uInt16 i = 0; i < nRPN; i++ )
            (*pp++)->IncRef();
    }
}

/// Optimisation for efficiently creating StringXML placeholders
void FormulaTokenArray::Assign( sal_uInt16 nCode, FormulaToken **pTokens )
{
    assert( nLen == 0 );
    assert( pCode == nullptr );

    nLen = nCode;
    pCode.reset(new FormulaToken*[ nLen ]);
    mbFinalized = true;

    for( sal_uInt16 i = 0; i < nLen; i++ )
    {
        FormulaToken *t = pTokens[ i ];
        assert( t->GetOpCode() == ocStringXML );
        pCode[ i ] = t;
        t->IncRef();
    }
}

FormulaTokenArray& FormulaTokenArray::operator=( const FormulaTokenArray& rArr )
{
    Clear();
    Assign( rArr );
    return *this;
}

void FormulaTokenArray::Clear()
{
    if( nRPN ) DelRPN();
    if( pCode )
    {
        FormulaToken** p = pCode.get();
        for( sal_uInt16 i = 0; i < nLen; i++ )
        {
            (*p++)->DecRef();
        }
        pCode.reset();
    }
    pRPN = nullptr;
    nError = FormulaError::NONE;
    nLen = nRPN = 0;
    bHyperLink = false;
    mbFromRangeName = false;
    mbShareable = true;
    mbFinalized = false;
    ClearRecalcMode();
}

void FormulaTokenArray::CheckToken( const FormulaToken& /*r*/ )
{
    // Do nothing.
}

void FormulaTokenArray::CheckAllRPNTokens()
{
    if( nRPN )
    {
        FormulaToken** p = pRPN;
        for( sal_uInt16 i = 0; i < nRPN; i++ )
        {
            CheckToken( *p[ i ] );
        }
    }
}

FormulaToken* FormulaTokenArray::AddToken( const FormulaToken& r )
{
    return Add( r.Clone() );
}

FormulaToken* FormulaTokenArray::MergeArray( )
{
    return nullptr;
}

FormulaToken* FormulaTokenArray::ReplaceToken( sal_uInt16 nOffset, FormulaToken* t,
        FormulaTokenArray::ReplaceMode eMode )
{
    if (nOffset < nLen)
    {
        CheckToken(*t);
        t->IncRef();
        FormulaToken* p = pCode[nOffset];
        pCode[nOffset] = t;
        if (eMode == CODE_AND_RPN && p->GetRef() > 1)
        {
            for (sal_uInt16 i=0; i < nRPN; ++i)
            {
                if (pRPN[i] == p)
                {
                    t->IncRef();
                    pRPN[i] = t;
                    p->DecRef();
                    if (p->GetRef() == 1)
                        break;  // for
                }
            }
        }
        p->DecRef();    // may be dead now
        return t;
    }
    else
    {
        t->DeleteIfZeroRef();
        return nullptr;
    }
}

sal_uInt16 FormulaTokenArray::RemoveToken( sal_uInt16 nOffset, sal_uInt16 nCount )
{
    if (nOffset < nLen)
    {
        SAL_WARN_IF( nOffset + nCount > nLen, "formula.core",
                "FormulaTokenArray::RemoveToken - nOffset " << nOffset << " + nCount " << nCount << " > nLen " << nLen);
        const sal_uInt16 nStop = ::std::min( static_cast<sal_uInt16>(nOffset + nCount), nLen);
        nCount = nStop - nOffset;
        for (sal_uInt16 j = nOffset; j < nStop; ++j)
        {
            FormulaToken* p = pCode[j];
            if (p->GetRef() > 1)
            {
                for (sal_uInt16 i=0; i < nRPN; ++i)
                {
                    if (pRPN[i] == p)
                    {
                        // Shift remaining tokens in pRPN down.
                        for (sal_uInt16 x=i+1; x < nRPN; ++x)
                        {
                            pRPN[x-1] = pRPN[x];
                        }
                        --nRPN;

                        p->DecRef();
                        if (p->GetRef() == 1)
                            break;  // for
                    }
                }
            }
            p->DecRef();    // may be dead now
        }

        // Shift remaining tokens in pCode down.
        for (sal_uInt16 x = nStop; x < nLen; ++x)
        {
            pCode[x-nCount] = pCode[x];
        }
        nLen -= nCount;

        return nCount;
    }
    else
    {
        SAL_WARN("formula.core","FormulaTokenArray::RemoveToken - nOffset " << nOffset << " >= nLen " << nLen);
        return 0;
    }
}

FormulaToken* FormulaTokenArray::Add( FormulaToken* t )
{
    assert(!mbFinalized);
    if (mbFinalized)
    {
        t->DeleteIfZeroRef();
        return nullptr;
    }

    if( !pCode )
        pCode.reset(new FormulaToken*[ FORMULA_MAXTOKENS ]);
    if( nLen < FORMULA_MAXTOKENS - 1 )
    {
        CheckToken(*t);
        pCode[ nLen++ ] = t;
        t->IncRef();
        if( t->GetOpCode() == ocArrayClose )
            return MergeArray();
        return t;
    }
    else
    {
        t->DeleteIfZeroRef();
        if ( nLen == FORMULA_MAXTOKENS - 1 )
        {
            t = new FormulaByteToken( ocStop );
            pCode[ nLen++ ] = t;
            t->IncRef();
        }
        return nullptr;
    }
}

FormulaToken* FormulaTokenArray::AddString( const svl::SharedString& rStr )
{
    return Add( new FormulaStringToken( rStr ) );
}

FormulaToken* FormulaTokenArray::AddDouble( double fVal )
{
    return Add( new FormulaDoubleToken( fVal ) );
}

void FormulaTokenArray::AddExternal( const sal_Unicode* pStr )
{
    AddExternal( OUString( pStr ) );
}

FormulaToken* FormulaTokenArray::AddExternal( const OUString& rStr,
        OpCode eOp /* = ocExternal */ )
{
    return Add( new FormulaExternalToken( eOp, rStr ) );
}

FormulaToken* FormulaTokenArray::AddBad( const OUString& rStr )
{
    return Add( new FormulaStringOpToken( ocBad, svl::SharedString( rStr ) ) ); // string not interned
}

FormulaToken* FormulaTokenArray::AddStringXML( const OUString& rStr )
{
    return Add( new FormulaStringOpToken( ocStringXML, svl::SharedString( rStr ) ) );   // string not interned
}


void FormulaTokenArray::AddRecalcMode( ScRecalcMode nBits )
{
    const unsigned nExclusive = static_cast<sal_uInt8>(nBits & ScRecalcMode::EMask);
    if (nExclusive)
    {
        unsigned nExBit;
        if (nExclusive & (nExclusive - 1))
        {
            // More than one bit set, use highest priority.
            for (nExBit = 1; (nExBit & static_cast<sal_uInt8>(ScRecalcMode::EMask)) != 0; nExBit <<= 1)
            {
                if (nExclusive & nExBit)
                    break;
            }
        }
        else
        {
            // Only one bit is set.
            nExBit = nExclusive;
        }
        // Set exclusive bit if priority is higher than existing.
        if (nExBit < static_cast<sal_uInt8>(nMode & ScRecalcMode::EMask))
            SetMaskedRecalcMode( static_cast<ScRecalcMode>(nExBit));
    }
    SetCombinedBitsRecalcMode( nBits );
}


bool FormulaTokenArray::HasMatrixDoubleRefOps()
{
    if ( pRPN && nRPN )
    {
        // RPN-Interpreter simulation.
        // Simply assumes a double as return value of each function.
        std::unique_ptr<FormulaToken*[]> pStack(new FormulaToken* [nRPN]);
        FormulaToken* pResult = new FormulaDoubleToken( 0.0 );
        short sp = 0;
        for ( auto t: RPNTokens() )
        {
            OpCode eOp = t->GetOpCode();
            sal_uInt8 nParams = t->GetParamCount();
            switch ( eOp )
            {
                case ocAdd :
                case ocSub :
                case ocMul :
                case ocDiv :
                case ocPow :
                case ocPower :
                case ocAmpersand :
                case ocEqual :
                case ocNotEqual :
                case ocLess :
                case ocGreater :
                case ocLessEqual :
                case ocGreaterEqual :
                {
                    for ( sal_uInt8 k = nParams; k; k-- )
                    {
                        if ( sp >= k && pStack[sp-k]->GetType() == svDoubleRef )
                        {
                            pResult->Delete();
                            return true;
                        }
                    }
                }
                break;
                default:
                {
                    // added to avoid warnings
                }
            }
            if ( eOp == ocPush || lcl_IsReference( eOp, t->GetType() )  )
                pStack[sp++] = t;
            else if (FormulaCompiler::IsOpCodeJumpCommand( eOp ))
            {   // ignore Jumps, pop previous Result (Condition)
                if ( sp )
                    --sp;
            }
            else
            {   // pop parameters, push result
                sp = sal::static_int_cast<short>( sp - nParams );
                if ( sp < 0 )
                {
                    SAL_WARN("formula.core", "FormulaTokenArray::HasMatrixDoubleRefOps: sp < 0" );
                    sp = 0;
                }
                pStack[sp++] = pResult;
            }
        }
        pResult->Delete();
    }

    return false;
}

// --- Formula rewrite of a token array

inline bool MissingConventionODF::isRewriteNeeded( OpCode eOp ) const
{
    switch (eOp)
    {
        case ocGammaDist:
        case ocPoissonDist:
        case ocAddress:
        case ocLogInv:
        case ocLogNormDist:
        case ocNormDist:
            return true;
        case ocMissing:
        case ocLog:
            return isPODF();    // rewrite only for PODF
        case ocDBCount:
        case ocDBCount2:
            return isODFF();    // rewrite only for ODFF
        default:
            return false;
    }
}

/*
 fdo 81596
To be implemented yet:
  ocExternal:    ?
  ocMacro:       ?
  ocIndex:       INDEX() ?
*/
inline bool MissingConventionOOXML::isRewriteNeeded( OpCode eOp )
{
    switch (eOp)
    {
        case ocIf:

        case ocExternal:
        case ocEuroConvert:
        case ocMacro:

        case ocRound:
        case ocRoundUp:
        case ocRoundDown:

        case ocIndex:

        case ocCeil:
        case ocFloor:

        case ocGammaDist:
        case ocFDist_LT:
        case ocPoissonDist:
        case ocNormDist:
        case ocLogInv:
        case ocLogNormDist:
        case ocHypGeomDist:

        case ocDBCount:
        case ocDBCount2:
            return true;

        default:
            return false;
    }
}

class FormulaMissingContext
{
    public:
            const FormulaToken* mpFunc;
            int                 mnCurArg;

                    void    Clear() { mpFunc = nullptr; mnCurArg = 0; }
            inline  bool    AddDefaultArg( FormulaTokenArray* pNewArr, int nArg, double f ) const;
                    bool    AddMissingExternal( FormulaTokenArray* pNewArr ) const;
                    bool    AddMissing( FormulaTokenArray *pNewArr, const MissingConvention & rConv  ) const;
                    void    AddMoreArgs( FormulaTokenArray *pNewArr, const MissingConvention & rConv  ) const;
};

void FormulaMissingContext::AddMoreArgs( FormulaTokenArray *pNewArr, const MissingConvention & rConv  ) const
{
    if ( !mpFunc )
        return;

    switch (rConv.getConvention())
    {
        case MissingConvention::FORMULA_MISSING_CONVENTION_ODFF:
        case MissingConvention::FORMULA_MISSING_CONVENTION_PODF:
            {
                switch (mpFunc->GetOpCode())
                {
                    case ocGammaDist:
                        if (mnCurArg == 2)
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 1.0 );      // 4th, Cumulative=true()
                        }
                        break;
                    case ocPoissonDist:
                        if (mnCurArg == 1)
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 1.0 );      // 3rd, Cumulative=true()
                        }
                        break;
                    case ocNormDist:
                        if ( mnCurArg == 2 )
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 1.0 );      // 4th, Cumulative=true()
                        }
                        break;
                    case ocLogInv:
                    case ocLogNormDist:
                        if ( mnCurArg == 0 )
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 0.0 );      // 2nd, mean = 0.0
                        }
                        if ( mnCurArg <= 1 )
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 1.0 );      // 3rd, standard deviation = 1.0
                        }
                        break;
                    case ocLog:
                        if ( rConv.isPODF() && mnCurArg == 0 )
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 10.0 );     // 2nd, basis 10
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        case MissingConvention::FORMULA_MISSING_CONVENTION_OOXML:
            {
                switch (mpFunc->GetOpCode())
                {
                    case ocIf:
                        if( mnCurArg == 0 )
                        {
                            // Excel needs at least two parameters in IF function
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddOpCode( ocTrue );   // 2nd, true() as function
                            pNewArr->AddOpCode( ocOpen );   // so the result is of logical type
                            pNewArr->AddOpCode( ocClose );  // and survives roundtrip
                        }
                        break;

                    case ocEuroConvert:
                        if ( mnCurArg == 2 )
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 0.0 );      // 4th, FullPrecision = false()
                        }
                        break;

                    case ocPoissonDist:
                        if (mnCurArg == 1)
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 1.0 );      // 3rd, Cumulative=true()
                        }
                        break;

                    case ocGammaDist:
                    case ocFDist_LT:
                    case ocNormDist:
                        if (mnCurArg == 2)
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 1.0 );      // 4th, Cumulative=true()
                        }
                        break;

                    case ocLogInv:
                    case ocLogNormDist:
                        if ( mnCurArg == 0 )
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 0.0 );      // 2nd, mean = 0.0
                        }
                        if ( mnCurArg <= 1 )
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 1.0 );      // 3rd, standard deviation = 1.0
                        }
                        break;

                    case ocHypGeomDist:
                        if ( mnCurArg == 3 )
                        {
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 0.0 );      // 5th, Cumulative = false()
                        }
                        break;

                    case ocRound:
                    case ocRoundUp:
                    case ocRoundDown:
                        if( mnCurArg == 0 )
                        {
                            // ROUND, ROUNDUP, ROUNDDOWN functions are fixed to 2 parameters in Excel
                            pNewArr->AddOpCode( ocSep );
                            pNewArr->AddDouble( 0.0 );      // 2nd, 0.0
                        }
                        break;

                    default:
                        break;
                }
            }
            break;
    }

}

inline bool FormulaMissingContext::AddDefaultArg( FormulaTokenArray* pNewArr, int nArg, double f ) const
{
    if (mnCurArg == nArg)
    {
        pNewArr->AddDouble( f );
        return true;
    }
    return false;
}

bool FormulaMissingContext::AddMissingExternal( FormulaTokenArray *pNewArr ) const
{
    // Only called for PODF, not ODFF. No need to distinguish.

    const OUString &rName = mpFunc->GetExternal();

    // initial (fast) check:
    sal_Unicode nLastChar = rName[ rName.getLength() - 1];
    if ( nLastChar != 't' && nLastChar != 'm' )
        return false;

    if (rName.equalsIgnoreAsciiCase(
                "com.sun.star.sheet.addin.Analysis.getAccrint" ))
    {
        return AddDefaultArg( pNewArr, 4, 1000.0 );
    }
    if (rName.equalsIgnoreAsciiCase(
                "com.sun.star.sheet.addin.Analysis.getAccrintm" ))
    {
        return AddDefaultArg( pNewArr, 3, 1000.0 );
    }
    return false;
}

bool FormulaMissingContext::AddMissing( FormulaTokenArray *pNewArr, const MissingConvention & rConv  ) const
{
    if ( !mpFunc )
        return false;

    bool bRet = false;
    const OpCode eOp = mpFunc->GetOpCode();

    switch (rConv.getConvention())
    {
        case MissingConvention::FORMULA_MISSING_CONVENTION_ODFF:
            {
                // Add for ODFF
                switch (eOp)
                {
                    case ocAddress:
                        return AddDefaultArg( pNewArr, 2, 1.0 );    // abs
                    default:
                        break;
                }
            }
            break;
        case MissingConvention::FORMULA_MISSING_CONVENTION_PODF:
            {
                // Add for PODF
                switch (eOp)
                {
                    case ocAddress:
                        return AddDefaultArg( pNewArr, 2, 1.0 );    // abs
                    case ocFixed:
                        return AddDefaultArg( pNewArr, 1, 2.0 );
                    case ocBetaDist:
                    case ocBetaInv:
                    case ocPMT:
                        return AddDefaultArg( pNewArr, 3, 0.0 );
                    case ocIpmt:
                    case ocPpmt:
                        return AddDefaultArg( pNewArr, 4, 0.0 );
                    case ocPV:
                    case ocFV:
                        bRet |= AddDefaultArg( pNewArr, 2, 0.0 );   // pmt
                        bRet |= AddDefaultArg( pNewArr, 3, 0.0 );   // [fp]v
                        break;
                    case ocRate:
                        bRet |= AddDefaultArg( pNewArr, 1, 0.0 );   // pmt
                        bRet |= AddDefaultArg( pNewArr, 3, 0.0 );   // fv
                        bRet |= AddDefaultArg( pNewArr, 4, 0.0 );   // type
                        break;
                    case ocExternal:
                        return AddMissingExternal( pNewArr );

                        // --- more complex cases ---

                    case ocOffset:
                        // FIXME: rather tough
                        // if arg 3 (height) omitted, export arg1 (rows)
                        break;
                    default:
                        break;
                }
            }
            break;
        case MissingConvention::FORMULA_MISSING_CONVENTION_OOXML:
            {
                switch (eOp)
                {
                    case ocExternal:
                        return AddMissingExternal( pNewArr );
                    default:
                        break;
                }
            }
            break;
    }

    return bRet;
}

bool FormulaTokenArray::NeedsPodfRewrite( const MissingConventionODF & rConv )
{
    for ( auto i: Tokens()  )
    {
        if ( rConv.isRewriteNeeded( i->GetOpCode()))
            return true;
    }
    return false;
}

bool FormulaTokenArray::NeedsOoxmlRewrite()
{
    for ( auto i: Tokens() )
    {
        if ( MissingConventionOOXML::isRewriteNeeded( i->GetOpCode()))
            return true;
    }
    return false;
}


FormulaTokenArray * FormulaTokenArray::RewriteMissing( const MissingConvention & rConv )
{
    const size_t nAlloc = 256;
    FormulaMissingContext aCtx[ nAlloc ];

    /* TODO: with some effort we might be able to merge the two almost
     * identical function stacks into one and generalize things, otherwise
     * adding yet another "omit argument" would be copypasta. */

    int aOpCodeAddressStack[ nAlloc ];  // use of ADDRESS() function
    const int nOmitAddressArg = 3;      // ADDRESS() 4th parameter A1/R1C1

    int aOpCodeDcountStack[ nAlloc ];   // use of DCOUNT()/DCOUNTA() function
    const int nOmitDcountArg = 1;       // DCOUNT() and DCOUNTA() 2nd parameter DatabaseField if 0

    sal_uInt16 nTokens = GetLen() + 1;
    FormulaMissingContext* pCtx = (nAlloc < nTokens ? new FormulaMissingContext[nTokens] : &aCtx[0]);
    int* pOcas = (nAlloc < nTokens ? new int[nTokens] : &aOpCodeAddressStack[0]);
    int* pOcds = (nAlloc < nTokens ? new int[nTokens] : &aOpCodeDcountStack[0]);
    // Never go below 0, never use 0, mpFunc always NULL.
    pCtx[0].Clear();
    int nFn = 0;
    int nOcas = 0;
    int nOcds = 0;

    FormulaTokenArray *pNewArr = new FormulaTokenArray;
    // At least ScRecalcMode::ALWAYS needs to be set.
    pNewArr->AddRecalcMode( GetRecalcMode());

    FormulaTokenArrayPlainIterator aIter(*this);
    for ( FormulaToken *pCur = aIter.First(); pCur; pCur = aIter.Next() )
    {
        bool bAdd = true;
        // Don't write the expression of the new inserted ADDRESS() parameter.
        // Do NOT omit the new second parameter of INDIRECT() though. If that
        // was done for both, INDIRECT() actually could calculate different and
        // valid (but wrong) results with the then changed return value of
        // ADDRESS(). Better let it generate an error instead.
        for (int i = nOcas; i-- > 0 && bAdd; )
        {
            if (pCtx[ pOcas[ i ] ].mnCurArg == nOmitAddressArg)
            {
                // Omit everything except a trailing separator, the leading
                // separator is omitted below. The other way around would leave
                // an extraneous separator if no parameter followed.
                if (!(pOcas[ i ] == nFn && pCur->GetOpCode() == ocSep))
                    bAdd = false;
            }
        }
        // Strip the 2nd argument (leaving empty) of DCOUNT() and DCOUNTA() if
        // it is 0.
        for (int i = nOcds; i-- > 0 && bAdd; )
        {
            if (pCtx[ pOcds[ i ] ].mnCurArg == nOmitDcountArg)
            {
                // Omit only a literal 0 value, nothing else.
                if (pOcds[ i ] == nFn && pCur->GetOpCode() == ocPush && pCur->GetType() == svDouble &&
                        pCur->GetDouble() == 0.0)
                {
                    // No other expression, between separators.
                    FormulaToken* p = aIter.PeekPrevNoSpaces();
                    if (p && p->GetOpCode() == ocSep)
                    {
                        p = aIter.PeekNextNoSpaces();
                        if (p && p->GetOpCode() == ocSep)
                            bAdd = false;
                    }
                }
            }
        }
        switch ( pCur->GetOpCode() )
        {
            case ocOpen:
                {
                    ++nFn;      // all following operations on _that_ function
                    pCtx[ nFn ].mpFunc = aIter.PeekPrevNoSpaces();
                    pCtx[ nFn ].mnCurArg = 0;
                    OpCode eOp;
                    if (rConv.isPODF() && pCtx[ nFn ].mpFunc && pCtx[ nFn ].mpFunc->GetOpCode() == ocAddress)
                        pOcas[ nOcas++ ] = nFn;     // entering ADDRESS() if PODF
                    else if ((rConv.isODFF() || rConv.isOOXML()) && pCtx[ nFn ].mpFunc &&
                            ((eOp = pCtx[ nFn ].mpFunc->GetOpCode()) == ocDBCount || eOp == ocDBCount2))
                        pOcds[ nOcds++ ] = nFn;     // entering DCOUNT() or DCOUNTA() if ODFF or OOXML
                }
            break;
            case ocClose:
                pCtx[ nFn ].AddMoreArgs( pNewArr, rConv );
                SAL_WARN_IF(nFn <= 0, "formula.core", "FormulaTokenArray::RewriteMissing: underflow");
                if (nOcas > 0 && pOcas[ nOcas-1 ] == nFn)
                    --nOcas;                    // leaving ADDRESS()
                else if (nOcds > 0 && pOcds[ nOcds-1 ] == nFn)
                    --nOcds;                    // leaving DCOUNT() or DCOUNTA()
                if (nFn > 0)
                    --nFn;
                break;
            case ocSep:
                pCtx[ nFn ].mnCurArg++;
                // Omit leading separator of ADDRESS() parameter.
                if (nOcas && pOcas[ nOcas-1 ] == nFn && pCtx[ nFn ].mnCurArg == nOmitAddressArg)
                {
                    bAdd = false;
                }
                break;
            case ocMissing:
                if ( bAdd )
                    bAdd = !pCtx[ nFn ].AddMissing( pNewArr, rConv );
                break;
            default:
                break;
        }
        if (bAdd)
        {
            OpCode eOp = pCur->GetOpCode();
            if ( ( eOp == ocCeil || eOp == ocFloor ||
                   ( eOp == ocLogNormDist && pCur->GetByte() == 4 ) ) &&
                 rConv.getConvention() == MissingConvention::FORMULA_MISSING_CONVENTION_OOXML )
            {
                switch ( eOp )
                {
                    case ocCeil :
                        eOp = ocCeil_Math;
                        break;
                    case ocFloor :
                        eOp = ocFloor_Math;
                        break;
                    case ocLogNormDist :
                        eOp = ocLogNormDist_MS;
                        break;
                    default :
                        eOp = ocNone;
                        break;
                }
                FormulaToken *pToken = new FormulaToken( svByte, eOp );
                pNewArr->Add( pToken );
            }
            else if ( eOp == ocHypGeomDist &&
                      rConv.getConvention() == MissingConvention::FORMULA_MISSING_CONVENTION_OOXML )
            {
                FormulaToken *pToken = new FormulaToken( svByte, ocHypGeomDist_MS );
                pNewArr->Add( pToken );
            }
            else
                pNewArr->AddToken( *pCur );
        }
    }

    if (pOcds != &aOpCodeDcountStack[0])
        delete [] pOcds;
    if (pOcas != &aOpCodeAddressStack[0])
        delete [] pOcas;
    if (pCtx != &aCtx[0])
        delete [] pCtx;

    return pNewArr;
}

bool FormulaTokenArray::MayReferenceFollow()
{
    if ( pCode && nLen > 0 )
    {
        // ignore trailing spaces
        sal_uInt16 i = nLen - 1;
        while ( i > 0 && pCode[i]->GetOpCode() == SC_OPCODE_SPACES )
        {
            --i;
        }
        if ( i > 0 || pCode[i]->GetOpCode() != SC_OPCODE_SPACES )
        {
            OpCode eOp = pCode[i]->GetOpCode();
            if ( (SC_OPCODE_START_BIN_OP <= eOp && eOp < SC_OPCODE_STOP_BIN_OP ) ||
                 (SC_OPCODE_START_UN_OP <= eOp && eOp < SC_OPCODE_STOP_UN_OP ) ||
                 eOp == SC_OPCODE_OPEN || eOp == SC_OPCODE_SEP )
            {
                return true;
            }
        }
    }
    return false;
}
FormulaToken* FormulaTokenArray::AddOpCode( OpCode eOp )
{
    FormulaToken* pRet = nullptr;
    switch ( eOp )
    {
        case ocOpen:
        case ocClose:
        case ocSep:
        case ocArrayOpen:
        case ocArrayClose:
        case ocArrayRowSep:
        case ocArrayColSep:
            pRet = new FormulaToken( svSep,eOp );
            break;
        case ocIf:
        case ocIfError:
        case ocIfNA:
        case ocChoose:
            {
                short nJump[FORMULA_MAXJUMPCOUNT + 1];
                if ( eOp == ocIf )
                    nJump[ 0 ] = 3;
                else if ( eOp == ocChoose )
                    nJump[ 0 ] = FORMULA_MAXJUMPCOUNT + 1;
                else
                    nJump[ 0 ] = 2;
                pRet = new FormulaJumpToken( eOp, nJump );
            }
            break;
        default:
            pRet = new FormulaByteToken( eOp, 0, ParamClass::Unknown );
            break;
    }
    return Add( pRet );
}

void FormulaTokenArray::ReinternStrings( svl::SharedStringPool& rPool )
{
    for (auto i: Tokens())
    {
        switch (i->GetType())
        {
            case svString:
                i->SetString( rPool.intern( i->GetString().getString()));
                break;
            default:
                ;   // nothing
        }
    }
}


/*----------------------------------------------------------------------*/

FormulaTokenIterator::Item::Item(const FormulaTokenArray* pArray, short pc, short stop) :
    pArr(pArray), nPC(pc), nStop(stop)
{
}

FormulaTokenIterator::FormulaTokenIterator( const FormulaTokenArray& rArr )
{
    Push( &rArr );
}

FormulaTokenIterator::~FormulaTokenIterator()
{
}

void FormulaTokenIterator::Push( const FormulaTokenArray* pArr )
{
    FormulaTokenIterator::Item item(pArr, -1, SHRT_MAX);

    maStack.push_back(item);
}

void FormulaTokenIterator::Pop()
{
    maStack.pop_back();
}

void FormulaTokenIterator::Reset()
{
    while( maStack.size() > 1 )
        maStack.pop_back();

    maStack.back().nPC = -1;
}

FormulaToken* FormulaTokenArrayPlainIterator::GetNextName()
{
    if( mpFTA->GetArray() )
    {
        while ( mnIndex < mpFTA->GetLen() )
        {
            FormulaToken* t = mpFTA->GetArray()[ mnIndex++ ];
            if( t->GetType() == svIndex )
                return t;
        }
    }
    return nullptr;
}

const FormulaToken* FormulaTokenIterator::Next()
{
    const FormulaToken* t = GetNonEndOfPathToken( ++maStack.back().nPC );
    if( !t && maStack.size() > 1 )
    {
        Pop();
        t = Next();
    }
    return t;
}

const FormulaToken* FormulaTokenIterator::PeekNextOperator()
{
    const FormulaToken* t = nullptr;
    short nIdx = maStack.back().nPC;
    while (!t && ((t = GetNonEndOfPathToken( ++nIdx)) != nullptr))
    {
        if (t->GetOpCode() == ocPush)
            t = nullptr;   // ignore operands
    }
    if (!t && maStack.size() > 1)
    {
        FormulaTokenIterator::Item aHere = maStack.back();
        maStack.pop_back();
        t = PeekNextOperator();
        maStack.push_back(aHere);
    }
    return t;
}

//! The nPC counts after a Push() are -1

void FormulaTokenIterator::Jump( short nStart, short nNext, short nStop )
{
    maStack.back().nPC = nNext;
    if( nStart != nNext )
    {
        Push( maStack.back().pArr );
        maStack.back().nPC = nStart;
        maStack.back().nStop = nStop;
    }
}

const FormulaToken* FormulaTokenIterator::GetNonEndOfPathToken( short nIdx ) const
{
    FormulaTokenIterator::Item cur = maStack.back();

    if (nIdx < cur.pArr->GetCodeLen() && nIdx < cur.nStop)
    {
        const FormulaToken* t = cur.pArr->GetCode()[ nIdx ];
        // such an OpCode ends an IF() or CHOOSE() path
        return (t->GetOpCode() == ocSep || t->GetOpCode() == ocClose) ? nullptr : t;
    }
    return nullptr;
}

bool FormulaTokenIterator::IsEndOfPath() const
{
    return GetNonEndOfPathToken( maStack.back().nPC + 1) == nullptr;
}

FormulaToken* FormulaTokenArrayPlainIterator::GetNextReference()
{
    while( mnIndex < mpFTA->GetLen() )
    {
        FormulaToken* t = mpFTA->GetArray()[ mnIndex++ ];
        switch( t->GetType() )
        {
            case svSingleRef:
            case svDoubleRef:
            case svExternalSingleRef:
            case svExternalDoubleRef:
                return t;
            default:
            {
                // added to avoid warnings
            }
        }
    }
    return nullptr;
}

FormulaToken* FormulaTokenArrayPlainIterator::GetNextColRowName()
{
    while( mnIndex < mpFTA->GetLen() )
    {
        FormulaToken* t = mpFTA->GetArray()[ mnIndex++ ];
        if ( t->GetOpCode() == ocColRowName )
            return t;
    }
    return nullptr;
}

FormulaToken* FormulaTokenArrayPlainIterator::GetNextReferenceRPN()
{
    while( mnIndex < mpFTA->GetCodeLen() )
    {
        FormulaToken* t = mpFTA->GetCode()[ mnIndex++ ];
        switch( t->GetType() )
        {
            case svSingleRef:
            case svDoubleRef:
            case svExternalSingleRef:
            case svExternalDoubleRef:
                return t;
            default:
            {
                // added to avoid warnings
            }
        }
    }
    return nullptr;
}

FormulaToken* FormulaTokenArrayPlainIterator::GetNextReferenceOrName()
{
    if( mpFTA->GetArray() )
    {
        while ( mnIndex < mpFTA->GetLen() )
        {
            FormulaToken* t = mpFTA->GetArray()[ mnIndex++ ];
            switch( t->GetType() )
            {
                case svSingleRef:
                case svDoubleRef:
                case svIndex:
                case svExternalSingleRef:
                case svExternalDoubleRef:
                case svExternalName:
                    return t;
                default:
                {
                    // added to avoid warnings
                }
             }
         }
     }
    return nullptr;
}

FormulaToken* FormulaTokenArrayPlainIterator::Next()
{
    if( mpFTA->GetArray() && mnIndex < mpFTA->GetLen() )
        return mpFTA->GetArray()[ mnIndex++ ];
    else
        return nullptr;
}

FormulaToken* FormulaTokenArrayPlainIterator::NextNoSpaces()
{
    if( mpFTA->GetArray() )
    {
        while( (mnIndex < mpFTA->GetLen()) && (mpFTA->GetArray()[ mnIndex ]->GetOpCode() == ocSpaces) )
            ++mnIndex;
        if( mnIndex < mpFTA->GetLen() )
            return mpFTA->GetArray()[ mnIndex++ ];
    }
    return nullptr;
}

FormulaToken* FormulaTokenArrayPlainIterator::NextRPN()
{
    if( mpFTA->GetCode() && mnIndex < mpFTA->GetCodeLen() )
        return mpFTA->GetCode()[ mnIndex++ ];
    else
        return nullptr;
}

FormulaToken* FormulaTokenArrayPlainIterator::PrevRPN()
{
    if( mpFTA->GetCode() && mnIndex )
        return mpFTA->GetCode()[ --mnIndex ];
    else
        return nullptr;
}

FormulaToken* FormulaTokenArrayPlainIterator::PeekNext()
{
    if( mpFTA->GetArray() && mnIndex < mpFTA->GetLen() )
        return mpFTA->GetArray()[ mnIndex ];
    else
        return nullptr;
}

FormulaToken* FormulaTokenArrayPlainIterator::PeekNextNoSpaces() const
{
    if( mpFTA->GetArray() && mnIndex < mpFTA->GetLen() )
    {
        sal_uInt16 j = mnIndex;
        while ( j < mpFTA->GetLen() && mpFTA->GetArray()[j]->GetOpCode() == ocSpaces )
            j++;
        if ( j < mpFTA->GetLen() )
            return mpFTA->GetArray()[ j ];
        else
            return nullptr;
    }
    else
        return nullptr;
}

FormulaToken* FormulaTokenArrayPlainIterator::PeekPrevNoSpaces() const
{
    if( mpFTA->GetArray() && mnIndex > 1 )
    {
        sal_uInt16 j = mnIndex - 2;
        while ( mpFTA->GetArray()[j]->GetOpCode() == ocSpaces && j > 0 )
            j--;
        if ( j > 0 || mpFTA->GetArray()[j]->GetOpCode() != ocSpaces )
            return mpFTA->GetArray()[ j ];
        else
            return nullptr;
    }
    else
        return nullptr;
}

void FormulaTokenArrayPlainIterator::AfterRemoveToken( sal_uInt16 nOffset, sal_uInt16 nCount )
{
    const sal_uInt16 nStop = std::min( static_cast<sal_uInt16>(nOffset + nCount), mpFTA->GetLen());

    if (mnIndex >= nOffset)
    {
        if (mnIndex < nStop)
            mnIndex = nOffset + 1;
        else
            mnIndex -= nStop - nOffset;
    }
}

// real implementations of virtual functions


double      FormulaDoubleToken::GetDouble() const            { return fDouble; }
double &    FormulaDoubleToken::GetDoubleAsReference()       { return fDouble; }

sal_Int16 FormulaDoubleToken::GetDoubleType() const
{
    // This is a plain double value without type information, don't emit a
    // warning via FormulaToken::GetDoubleType().
    return 0;
}

bool FormulaDoubleToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && fDouble == r.GetDouble();
}

sal_Int16 FormulaTypedDoubleToken::GetDoubleType() const
{
    return mnType;
}

void FormulaTypedDoubleToken::SetDoubleType( sal_Int16 nType )
{
    mnType = nType;
}

bool FormulaTypedDoubleToken::operator==( const FormulaToken& r ) const
{
    return FormulaDoubleToken::operator==( r ) && mnType == r.GetDoubleType();
}

FormulaStringToken::FormulaStringToken( const svl::SharedString& r ) :
    FormulaToken( svString ), maString( r )
{
}

FormulaStringToken::FormulaStringToken( const FormulaStringToken& r ) :
    FormulaToken( r ), maString( r.maString ) {}

FormulaToken* FormulaStringToken::Clone() const
{
    return new FormulaStringToken(*this);
}

svl::SharedString FormulaStringToken::GetString() const
{
    return maString;
}

void FormulaStringToken::SetString( const svl::SharedString& rStr )
{
    maString = rStr;
}

bool FormulaStringToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && maString == r.GetString();
}

FormulaStringOpToken::FormulaStringOpToken( OpCode e, const svl::SharedString& r ) :
    FormulaByteToken( e, 0, svString, ParamClass::Unknown ), maString( r ) {}

FormulaStringOpToken::FormulaStringOpToken( const FormulaStringOpToken& r ) :
    FormulaByteToken( r ), maString( r.maString ) {}

FormulaToken* FormulaStringOpToken::Clone() const
{
    return new FormulaStringOpToken(*this);
}

svl::SharedString FormulaStringOpToken::GetString() const
{
    return maString;
}

void FormulaStringOpToken::SetString( const svl::SharedString& rStr )
{
    maString = rStr;
}

bool FormulaStringOpToken::operator==( const FormulaToken& r ) const
{
    return FormulaByteToken::operator==( r ) && maString == r.GetString();
}

sal_uInt16  FormulaIndexToken::GetIndex() const             { return nIndex; }
void        FormulaIndexToken::SetIndex( sal_uInt16 n )     { nIndex = n; }
sal_Int16   FormulaIndexToken::GetSheet() const             { return mnSheet; }
void        FormulaIndexToken::SetSheet( sal_Int16 n )      { mnSheet = n; }
bool FormulaIndexToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && nIndex == r.GetIndex() &&
        mnSheet == r.GetSheet();
}
const OUString& FormulaExternalToken::GetExternal() const       { return aExternal; }
sal_uInt8       FormulaExternalToken::GetByte() const           { return nByte; }
void            FormulaExternalToken::SetByte( sal_uInt8 n )    { nByte = n; }
bool FormulaExternalToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && nByte == r.GetByte() &&
        aExternal == r.GetExternal();
}


FormulaError      FormulaErrorToken::GetError() const             { return nError; }
void            FormulaErrorToken::SetError( FormulaError nErr )  { nError = nErr; }
bool FormulaErrorToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) &&
        nError == static_cast< const FormulaErrorToken & >(r).GetError();
}
double          FormulaMissingToken::GetDouble() const       { return 0.0; }

svl::SharedString FormulaMissingToken::GetString() const
{
    return svl::SharedString::getEmptyString();
}

bool FormulaMissingToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r );
}


bool FormulaUnknownToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r );
}


} // formula


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
