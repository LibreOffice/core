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

#include <string.h>
#include <limits.h>
#include <tools/debug.hxx>
#include <osl/diagnose.h>

#include "formula/token.hxx"
#include "formula/tokenarray.hxx"
#include "formula/FormulaCompiler.hxx"
#include <formula/compiler.hrc>
#include <svl/sharedstringpool.hxx>
#include <memory>

namespace formula
{
    using namespace com::sun::star;

// Align MemPools on 4k boundaries - 64 bytes (4k is a MUST for OS/2)

// Need a lot of FormulaDoubleToken
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( FormulaDoubleToken )
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
        || (ocIf == eOp || ocIfError == eOp || ocIfNA == eOp || ocChoose == eOp ) // @ jump commands
        || (SC_OPCODE_START_1_PAR <= eOp && eOp < SC_OPCODE_STOP_1_PAR)     // one parameter
        || (SC_OPCODE_START_2_PAR <= eOp && eOp < SC_OPCODE_STOP_2_PAR)     // x parameters (cByte==0 in
                                                                            // FuncAutoPilot)
        || eOp == ocMacro || eOp == ocExternal                  // macros, AddIns
        || eOp == ocAnd || eOp == ocOr                          // former binary, now x parameters
        || eOp == ocNot || eOp == ocNeg                         // unary but function
        || (eOp >= ocInternalBegin && eOp <= ocInternalEnd)     // internal
        ));
}


sal_uInt8 FormulaToken::GetParamCount() const
{
    if ( eOp < SC_OPCODE_STOP_DIV && eOp != ocExternal && eOp != ocMacro &&
         eOp != ocIf && eOp != ocIfError && eOp != ocIfNA && eOp != ocChoose &&
         eOp != ocPercentSign )
        return 0;       // parameters and specials
                        // ocIf, ocIfError, ocIfNA and ocChoose not for FAP, have cByte then
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
    else if ( eOp == ocIf || eOp == ocIfError || eOp == ocIfNA || eOp == ocChoose )
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
    SAL_WARN( "formula.core", "FormulaToken::SetByte: virtual dummy called" );
}

bool FormulaToken::IsInForceArray() const
{
    // ok to be called for any derived class
    return false;
}

void FormulaToken::SetInForceArray( bool )
{
    SAL_WARN( "formula.core", "FormulaToken::SetInForceArray: virtual dummy called" );
}

double FormulaToken::GetDouble() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetDouble: virtual dummy called" );
    return 0.0;
}

double & FormulaToken::GetDoubleAsReference()
{
    SAL_WARN( "formula.core", "FormulaToken::GetDouble: virtual dummy called" );
    static double fVal = 0.0;
    return fVal;
}

svl::SharedString FormulaToken::GetString() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetString: virtual dummy called" );
    return svl::SharedString(); // invalid string
}

sal_uInt16 FormulaToken::GetIndex() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetIndex: virtual dummy called" );
    return 0;
}

void FormulaToken::SetIndex( sal_uInt16 )
{
    SAL_WARN( "formula.core", "FormulaToken::SetIndex: virtual dummy called" );
}

bool FormulaToken::IsGlobal() const
{
    SAL_WARN( "formula.core", "FormulaToken::IsGlobal: virtual dummy called" );
    return true;
}

void FormulaToken::SetGlobal( bool )
{
    SAL_WARN( "formula.core", "FormulaToken::SetGlobal: virtual dummy called" );
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

sal_uInt16 FormulaToken::GetError() const
{
    SAL_WARN( "formula.core", "FormulaToken::GetError: virtual dummy called" );
    return 0;
}

void FormulaToken::SetError( sal_uInt16 )
{
    SAL_WARN( "formula.core", "FormulaToken::SetError: virtual dummy called" );
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



sal_uInt8 FormulaByteToken::GetByte() const                       { return nByte; }
void FormulaByteToken::SetByte( sal_uInt8 n )                     { nByte = n; }
bool FormulaByteToken::IsInForceArray() const                { return bIsInForceArray; }
void FormulaByteToken::SetInForceArray( bool b )             { bIsInForceArray = b; }
bool FormulaByteToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && nByte == r.GetByte() &&
        bIsInForceArray == r.IsInForceArray();
}


FormulaToken* FormulaFAPToken::GetFAPOrigToken() const { return pOrigToken.get(); }
bool FormulaFAPToken::operator==( const FormulaToken& r ) const
{
    return FormulaByteToken::operator==( r ) && pOrigToken == r.GetFAPOrigToken();
}


short* FormulaJumpToken::GetJump() const                     { return pJump; }
bool FormulaJumpToken::IsInForceArray() const                { return bIsInForceArray; }
void FormulaJumpToken::SetInForceArray( bool b )             { bIsInForceArray = b; }
bool FormulaJumpToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && pJump[0] == r.GetJump()[0] &&
        memcmp( pJump+1, r.GetJump()+1, pJump[0] * sizeof(short) ) == 0 &&
        bIsInForceArray == r.IsInForceArray();
}
FormulaJumpToken::~FormulaJumpToken()
{
    delete [] pJump;
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
                    AddToken( formula::FormulaIndexToken( eOpCode, static_cast<sal_uInt16>(nValue) ) );
                else if ( eOpCode == ocTableRef )
                    bError = true;  /* TODO: implementation */
                else if ( eOpCode == ocSpaces )
                    AddToken( formula::FormulaByteToken( ocSpaces, static_cast<sal_uInt8>(nValue) ) );
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
                    AddToken( formula::FormulaExternalToken( eOpCode, aStrVal ) );
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
FormulaToken* FormulaTokenArray::GetNextReference()
{
    while( nIndex < nLen )
    {
        FormulaToken* t = pCode[ nIndex++ ];
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

FormulaToken* FormulaTokenArray::GetNextColRowName()
{
    while( nIndex < nLen )
    {
        FormulaToken* t = pCode[ nIndex++ ];
        if ( t->GetOpCode() == ocColRowName )
            return t;
    }
    return nullptr;
}

FormulaToken* FormulaTokenArray::GetNextReferenceRPN()
{
    while( nIndex < nRPN )
    {
        FormulaToken* t = pRPN[ nIndex++ ];
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

FormulaToken* FormulaTokenArray::GetNextReferenceOrName()
{
    if( pCode )
    {
        while ( nIndex < nLen )
        {
            FormulaToken* t = pCode[ nIndex++ ];
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

FormulaToken* FormulaTokenArray::GetNextName()
{
    if( pCode )
    {
        while ( nIndex < nLen )
        {
            FormulaToken* t = pCode[ nIndex++ ];
            if( t->GetType() == svIndex )
                return t;
        }
    } // if( pCode )
    return nullptr;
}

FormulaToken* FormulaTokenArray::Next()
{
    if( pCode && nIndex < nLen )
        return pCode[ nIndex++ ];
    else
        return nullptr;
}

FormulaToken* FormulaTokenArray::NextNoSpaces()
{
    if( pCode )
    {
        while( (nIndex < nLen) && (pCode[ nIndex ]->GetOpCode() == ocSpaces) )
            ++nIndex;
        if( nIndex < nLen )
            return pCode[ nIndex++ ];
    }
    return nullptr;
}

FormulaToken* FormulaTokenArray::NextRPN()
{
    if( pRPN && nIndex < nRPN )
        return pRPN[ nIndex++ ];
    else
        return nullptr;
}

FormulaToken* FormulaTokenArray::PrevRPN()
{
    if( pRPN && nIndex )
        return pRPN[ --nIndex ];
    else
        return nullptr;
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
    nRPN = nIndex = 0;
}

FormulaToken* FormulaTokenArray::PeekPrev( sal_uInt16 & nIdx )
{
    if (0 < nIdx && nIdx <= nLen)
        return pCode[--nIdx];
    return nullptr;
}

FormulaToken* FormulaTokenArray::PeekNext()
{
    if( pCode && nIndex < nLen )
        return pCode[ nIndex ];
    else
        return nullptr;
}

FormulaToken* FormulaTokenArray::PeekNextNoSpaces()
{
    if( pCode && nIndex < nLen )
    {
        sal_uInt16 j = nIndex;
        while ( pCode[j]->GetOpCode() == ocSpaces && j < nLen )
            j++;
        if ( j < nLen )
            return pCode[ j ];
        else
            return nullptr;
    }
    else
        return nullptr;
}

FormulaToken* FormulaTokenArray::PeekPrevNoSpaces()
{
    if( pCode && nIndex > 1 )
    {
        sal_uInt16 j = nIndex - 2;
        while ( pCode[j]->GetOpCode() == ocSpaces && j > 0 )
            j--;
        if ( j > 0 || pCode[j]->GetOpCode() != ocSpaces )
            return pCode[ j ];
        else
            return nullptr;
    }
    else
        return nullptr;
}

bool FormulaTokenArray::HasReferences() const
{
    for (sal_uInt16 i = 0; i < nLen; ++i)
    {
        if (pCode[i]->IsRef())
            return true;
    }

    for (sal_uInt16 i = 0; i < nRPN; ++i)
    {
        if (pRPN[i]->IsRef())
            return true;
    }

    return false;
}

bool FormulaTokenArray::HasExternalRef() const
{
    for ( sal_uInt16 j=0; j < nLen; j++ )
    {
        if (pCode[j]->IsExternalRef())
            return true;
    }
    return false;
}

bool FormulaTokenArray::HasOpCode( OpCode eOp ) const
{
    for ( sal_uInt16 j=0; j < nLen; j++ )
    {
        if ( pCode[j]->GetOpCode() == eOp )
            return true;
    }
    return false;
}

bool FormulaTokenArray::HasOpCodeRPN( OpCode eOp ) const
{
    for ( sal_uInt16 j=0; j < nRPN; j++ )
    {
        if ( pRPN[j]->GetOpCode() == eOp )
            return true;
    }
    return false;
}

bool FormulaTokenArray::HasNameOrColRowName() const
{
    for ( sal_uInt16 j=0; j < nLen; j++ )
    {
        if( pCode[j]->GetType() == svIndex || pCode[j]->GetOpCode() == ocColRowName )
            return true;
    }
    return false;
}

bool FormulaTokenArray::HasOpCodes(const unordered_opcode_set& rOpCodes) const
{
    FormulaToken** p = pCode;
    FormulaToken** pEnd = p + static_cast<size_t>(nLen);
    for (; p != pEnd; ++p)
    {
        OpCode eOp = (*p)->GetOpCode();
        if (rOpCodes.count(eOp) > 0)
            return true;
    }

    return false;
}

FormulaTokenArray::FormulaTokenArray() :
    pCode(nullptr),
    pRPN(nullptr),
    nLen(0),
    nRPN(0),
    nIndex(0),
    nError(0),
    nMode(ScRecalcMode::NORMAL),
    bHyperLink(false),
    mbFromRangeName(false)
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
    nIndex = r.nIndex;
    nError = r.nError;
    nMode  = r.nMode;
    bHyperLink = r.bHyperLink;
    mbFromRangeName = r.mbFromRangeName;
    pCode  = nullptr;
    pRPN   = nullptr;
    FormulaToken** pp;
    if( nLen )
    {
        pp = pCode = new FormulaToken*[ nLen ];
        memcpy( pp, r.pCode, nLen * sizeof( FormulaToken* ) );
        for( sal_uInt16 i = 0; i < nLen; i++ )
            (*pp++)->IncRef();
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
    pCode = new FormulaToken*[ nLen ];

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

FormulaTokenArray* FormulaTokenArray::Clone() const
{
    FormulaTokenArray* p = new FormulaTokenArray;
    p->nLen = nLen;
    p->nRPN = nRPN;
    p->nMode = nMode;
    p->nError = nError;
    p->bHyperLink = bHyperLink;
    p->mbFromRangeName = mbFromRangeName;
    FormulaToken** pp;
    if( nLen )
    {
        pp = p->pCode = new FormulaToken*[ nLen ];
        memcpy( pp, pCode, nLen * sizeof( FormulaToken* ) );
        for( sal_uInt16 i = 0; i < nLen; i++, pp++ )
        {
            *pp = (*pp)->Clone();
            (*pp)->IncRef();
        }
    }
    if( nRPN )
    {
        pp = p->pRPN = new FormulaToken*[ nRPN ];
        memcpy( pp, pRPN, nRPN * sizeof( FormulaToken* ) );
        for( sal_uInt16 i = 0; i < nRPN; i++, pp++ )
        {
            FormulaToken* t = *pp;
            if( t->GetRef() > 1 )
            {
                FormulaToken** p2 = pCode;
                sal_uInt16 nIdx = 0xFFFF;
                for( sal_uInt16 j = 0; j < nLen; j++, p2++ )
                {
                    if( *p2 == t )
                    {
                        nIdx = j; break;
                    }
                }
                if( nIdx == 0xFFFF )
                    *pp = t->Clone();
                else
                    *pp = p->pCode[ nIdx ];
            }
            else
                *pp = t->Clone();
            (*pp)->IncRef();
        }
    }
    return p;
}

void FormulaTokenArray::Clear()
{
    if( nRPN ) DelRPN();
    if( pCode )
    {
        FormulaToken** p = pCode;
        for( sal_uInt16 i = 0; i < nLen; i++ )
        {
            (*p++)->DecRef();
        }
        delete [] pCode;
    }
    pCode = nullptr; pRPN = nullptr;
    nError = nLen = nIndex = nRPN = 0;
    bHyperLink = false;
    mbFromRangeName = false;
    ClearRecalcMode();
}

void FormulaTokenArray::CheckToken( const FormulaToken& /*r*/ )
{
    // Do nothing.
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
    if (eMode == BACKWARD_CODE_ONLY)
        nOffset = nLen - nOffset - 1;

    if (nOffset < nLen)
    {
        CheckToken(*t);
        t->IncRef();
        FormulaToken* p = pCode[nOffset];
        pCode[nOffset] = t;
        if (eMode == FORWARD_CODE_AND_RPN && p->GetRef() > 1)
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
        t->Delete();
        return nullptr;
    }
}

FormulaToken* FormulaTokenArray::Add( FormulaToken* t )
{
    if( !pCode )
        pCode = new FormulaToken*[ FORMULA_MAXTOKENS ];
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
        t->Delete();
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

FormulaToken* FormulaTokenArray::AddExternal( const sal_Unicode* pStr )
{
    return AddExternal( OUString( pStr ) );
}

FormulaToken* FormulaTokenArray::AddExternal( const OUString& rStr,
        OpCode eOp /* = ocExternal */ )
{
    return Add( new FormulaExternalToken( eOp, rStr ) );
}

FormulaToken* FormulaTokenArray::AddBad( const OUString& rStr )
{
    return Add( new FormulaStringOpToken( ocBad, rStr ) );
}

FormulaToken* FormulaTokenArray::AddStringXML( const OUString& rStr )
{
    return Add( new FormulaStringOpToken( ocStringXML, rStr ) );
}



void FormulaTokenArray::AddRecalcMode( ScRecalcMode nBits )
{
    //! Order is important.
    if ( nBits & ScRecalcMode::ALWAYS )
        SetExclusiveRecalcModeAlways();
    else if ( !IsRecalcModeAlways() )
    {
        if ( nBits & ScRecalcMode::ONLOAD )
            SetExclusiveRecalcModeOnLoad();
        else if ( nBits & ScRecalcMode::ONLOAD_ONCE && !IsRecalcModeOnLoad() )
            SetExclusiveRecalcModeOnLoadOnce();
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
        for ( sal_uInt16 j = 0; j < nRPN; j++ )
        {
            FormulaToken* t = pRPN[j];
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
            else if ( eOp == ocIf || eOp == ocIfError || eOp == ocIfNA || eOp == ocChoose )
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
        case ocLogNormDist:
        case ocNormDist:
            return true;
        case ocMissing:
        case ocLog:
            return isPODF();    // rewrite only for PODF
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
        case ocLogNormDist:
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
                        // FIXME: rather tough.
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
    for ( FormulaToken *pCur = First(); pCur; pCur = Next() )
    {
        if ( rConv.isRewriteNeeded( pCur->GetOpCode()))
            return true;
    }
    return false;
}

bool FormulaTokenArray::NeedsOoxmlRewrite()
{
    for ( FormulaToken *pCur = First(); pCur; pCur = Next() )
    {
        if ( MissingConventionOOXML::isRewriteNeeded( pCur->GetOpCode()))
            return true;
    }
    return false;
}


FormulaTokenArray * FormulaTokenArray::RewriteMissing( const MissingConvention & rConv )
{
    const size_t nAlloc = 256;
    FormulaMissingContext aCtx[ nAlloc ];
    int aOpCodeAddressStack[ nAlloc ];  // use of ADDRESS() function
    const int nOmitAddressArg = 3;      // ADDRESS() 4th parameter A1/R1C1
    sal_uInt16 nTokens = GetLen() + 1;
    FormulaMissingContext* pCtx = (nAlloc < nTokens ? new FormulaMissingContext[nTokens] : &aCtx[0]);
    int* pOcas = (nAlloc < nTokens ? new int[nTokens] : &aOpCodeAddressStack[0]);
    // Never go below 0, never use 0, mpFunc always NULL.
    pCtx[0].Clear();
    int nFn = 0;
    int nOcas = 0;

    FormulaTokenArray *pNewArr = new FormulaTokenArray;
    // At least ScRecalcMode::ALWAYS needs to be set.
    pNewArr->AddRecalcMode( GetRecalcMode());

    for ( FormulaToken *pCur = First(); pCur; pCur = Next() )
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
                // Omit erverything except a trailing separator, the leading
                // separator is omitted below. The other way around would leave
                // an extraneous separator if no parameter followed.
                if (!(pOcas[ i ] == nFn && pCur->GetOpCode() == ocSep))
                    bAdd = false;
            }
        }
        switch ( pCur->GetOpCode() )
        {
            case ocOpen:
                ++nFn;      // all following operations on _that_ function
                pCtx[ nFn ].mpFunc = PeekPrevNoSpaces();
                pCtx[ nFn ].mnCurArg = 0;
                if (rConv.isPODF() && pCtx[ nFn ].mpFunc && pCtx[ nFn ].mpFunc->GetOpCode() == ocAddress)
                    pOcas[ nOcas++ ] = nFn;     // entering ADDRESS() if PODF
                break;
            case ocClose:
                pCtx[ nFn ].AddMoreArgs( pNewArr, rConv );
                DBG_ASSERT( nFn > 0, "FormulaTokenArray::RewriteMissing: underflow");
                if (nOcas > 0 && pOcas[ nOcas-1 ] == nFn)
                    --nOcas;                    // leaving ADDRESS()
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
            if ( ( pCur->GetOpCode() == ocCeil || pCur->GetOpCode() == ocFloor ) &&
                 rConv.getConvention() == MissingConvention::FORMULA_MISSING_CONVENTION_OOXML )
            {
                FormulaToken *pToken = new FormulaToken( svByte,
                        ( pCur->GetOpCode() == ocCeil ? ocCeil_Math : ocFloor_Math ) );
                pNewArr->AddToken( *pToken );
            }
            else
                pNewArr->AddToken( *pCur );
        }
    }

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
            pRet = new FormulaByteToken( eOp, 0, false );
            break;
    }
    return AddToken( *pRet );
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
        FormulaTokenIterator::Item pHere = maStack.back();
        maStack.pop_back();
        t = PeekNextOperator();
        maStack.push_back(pHere);
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

    if (nIdx < cur.pArr->nRPN && nIdx < cur.nStop)
    {
        const FormulaToken* t = cur.pArr->pRPN[ nIdx ];
        // such an OpCode ends an IF() or CHOOSE() path
        return (t->GetOpCode() == ocSep || t->GetOpCode() == ocClose) ? nullptr : t;
    }
    return nullptr;
}

bool FormulaTokenIterator::IsEndOfPath() const
{
    return GetNonEndOfPathToken( maStack.back().nPC + 1) == nullptr;
}



// real implementations of virtual functions


double      FormulaDoubleToken::GetDouble() const            { return fDouble; }
double &    FormulaDoubleToken::GetDoubleAsReference()       { return fDouble; }
bool FormulaDoubleToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && fDouble == r.GetDouble();
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

bool FormulaStringToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && maString == r.GetString();
}

FormulaStringOpToken::FormulaStringOpToken( OpCode e, const svl::SharedString& r ) :
    FormulaByteToken( e, 0, svString, false ), maString( r ) {}

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

bool FormulaStringOpToken::operator==( const FormulaToken& r ) const
{
    return FormulaByteToken::operator==( r ) && maString == r.GetString();
}

sal_uInt16  FormulaIndexToken::GetIndex() const             { return nIndex; }
void        FormulaIndexToken::SetIndex( sal_uInt16 n )     { nIndex = n; }
bool        FormulaIndexToken::IsGlobal() const             { return mbGlobal; }
void        FormulaIndexToken::SetGlobal( bool b )          { mbGlobal = b; }
bool FormulaIndexToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && nIndex == r.GetIndex() &&
        mbGlobal == r.IsGlobal();
}
const OUString&   FormulaExternalToken::GetExternal() const    { return aExternal; }
sal_uInt8            FormulaExternalToken::GetByte() const        { return nByte; }
void            FormulaExternalToken::SetByte( sal_uInt8 n )      { nByte = n; }
bool FormulaExternalToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && nByte == r.GetByte() &&
        aExternal == r.GetExternal();
}


sal_uInt16          FormulaErrorToken::GetError() const          { return nError; }
void            FormulaErrorToken::SetError( sal_uInt16 nErr )   { nError = nErr; }
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


FormulaSubroutineToken::FormulaSubroutineToken( const FormulaSubroutineToken& r ) :
    FormulaToken( r ),
    mpArray( r.mpArray->Clone())
{
}
FormulaSubroutineToken::~FormulaSubroutineToken()
{
    delete mpArray;
}
bool FormulaSubroutineToken::operator==( const FormulaToken& r ) const
{
    // Arrays don't equal..
    return FormulaToken::operator==( r ) &&
        (mpArray == static_cast<const FormulaSubroutineToken&>(r).mpArray);
}


bool FormulaUnknownToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r );
}


} // formula


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
