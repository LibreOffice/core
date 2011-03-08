/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_formula.hxx"

#include <cstddef>
#include <cstdio>

#include <string.h>
#include <limits.h>
#include <tools/debug.hxx>

#include "formula/token.hxx"
#include "formula/tokenarray.hxx"
#include "formula/FormulaCompiler.hxx"
#include <formula/compiler.hrc>
#define MAXJUMPCOUNT 32     /* maximum number of jumps (ocChose) */

namespace formula
{
    using namespace com::sun::star;
// ImpTokenIterator wird je Interpreter angelegt, mehrfache auch durch
// SubCode via FormulaTokenIterator Push/Pop moeglich
IMPL_FIXEDMEMPOOL_NEWDEL( ImpTokenIterator, 32, 16 )

// Align MemPools on 4k boundaries - 64 bytes (4k is a MUST for OS/2)

// Need a lot of FormulaDoubleToken
const USHORT nMemPoolDoubleToken = (0x3000 - 64) / sizeof(FormulaDoubleToken);
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( FormulaDoubleToken, nMemPoolDoubleToken, nMemPoolDoubleToken )
// Need a lot of FormulaByteToken
const USHORT nMemPoolByteToken = (0x3000 - 64) / sizeof(FormulaByteToken);
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( FormulaByteToken, nMemPoolByteToken, nMemPoolByteToken )
// Need several FormulaStringToken
const USHORT nMemPoolStringToken = (0x1000 - 64) / sizeof(FormulaStringToken);
IMPL_FIXEDMEMPOOL_NEWDEL_DLL( FormulaStringToken, nMemPoolStringToken, nMemPoolStringToken )


// --- helpers --------------------------------------------------------------

inline BOOL lcl_IsReference( OpCode eOp, StackVar eType )
{
    return
        (eOp == ocPush && (eType == svSingleRef || eType == svDoubleRef))
        || (eOp == ocColRowNameAuto && eType == svDoubleRef)
        || (eOp == ocColRowName && eType == svSingleRef)
        || (eOp == ocMatRef && eType == svSingleRef)
        ;
}

// --- class FormulaToken --------------------------------------------------------
FormulaToken::~FormulaToken()
{
}

BOOL FormulaToken::Is3DRef() const
{
    return FALSE;
}

BOOL FormulaToken::IsFunction() const
{
//    OpCode eOp = GetOpCode();
    return (eOp != ocPush && eOp != ocBad && eOp != ocColRowName &&
            eOp != ocColRowNameAuto && eOp != ocName && eOp != ocDBArea &&
           (GetByte() != 0                                                  // x parameters
        || (SC_OPCODE_START_NO_PAR <= eOp && eOp < SC_OPCODE_STOP_NO_PAR)   // no parameter
        || (ocIf == eOp ||  ocChose ==  eOp     )                           // @ jump commands
        || (SC_OPCODE_START_1_PAR <= eOp && eOp < SC_OPCODE_STOP_1_PAR)     // one parameter
        || (SC_OPCODE_START_2_PAR <= eOp && eOp < SC_OPCODE_STOP_2_PAR)     // x parameters (cByte==0 in
                                                                            // FuncAutoPilot)
        || eOp == ocMacro || eOp == ocExternal                  // macros, AddIns
        || eOp == ocAnd || eOp == ocOr                          // former binary, now x parameters
        || eOp == ocNot || eOp == ocNeg                         // unary but function
        || (eOp >= ocInternalBegin && eOp <= ocInternalEnd)     // internal
        ));
}


BYTE FormulaToken::GetParamCount() const
{
    // OpCode eOp = GetOpCode();
    if ( eOp < SC_OPCODE_STOP_DIV && eOp != ocExternal && eOp != ocMacro &&
            eOp != ocIf && eOp != ocChose && eOp != ocPercentSign )
        return 0;       // parameters and specials
                        // ocIf and ocChose not for FAP, have cByte then
//2do: BOOL parameter whether FAP or not?
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
    else if ( eOp == ocIf || eOp == ocChose )
        return 1;           // only the condition counts as parameter
    else
        return 0;           // all the rest, no Parameter, or
                            // if so then it should be in cByte
}


BOOL FormulaToken::IsMatrixFunction() const
{
    return formula::FormulaCompiler::IsMatrixFunction(GetOpCode());
}

bool FormulaToken::IsExternalRef() const
{
    switch (eType)
    {
        case svExternalSingleRef:
        case svExternalDoubleRef:
        case svExternalName:
            return true;
    }
    return false;
}

BOOL FormulaToken::operator==( const FormulaToken& rToken ) const
{
    // don't compare reference count!
    return  eType == rToken.eType && GetOpCode() == rToken.GetOpCode();
}


// --- virtual dummy methods -------------------------------------------------

BYTE FormulaToken::GetByte() const
{
    // ok to be called for any derived class
    return 0;
}

void FormulaToken::SetByte( BYTE )
{
    DBG_ERRORFILE( "FormulaToken::SetByte: virtual dummy called" );
}

bool FormulaToken::HasForceArray() const
{
    // ok to be called for any derived class
    return false;
}

void FormulaToken::SetForceArray( bool )
{
    DBG_ERRORFILE( "FormulaToken::SetForceArray: virtual dummy called" );
}

double FormulaToken::GetDouble() const
{
    DBG_ERRORFILE( "FormulaToken::GetDouble: virtual dummy called" );
    return 0.0;
}

double & FormulaToken::GetDoubleAsReference()
{
    DBG_ERRORFILE( "FormulaToken::GetDouble: virtual dummy called" );
    static double fVal = 0.0;
    return fVal;
}

const String& FormulaToken::GetString() const
{
    DBG_ERRORFILE( "FormulaToken::GetString: virtual dummy called" );
    static  String              aDummyString;
    return aDummyString;
}

USHORT FormulaToken::GetIndex() const
{
    DBG_ERRORFILE( "FormulaToken::GetIndex: virtual dummy called" );
    return 0;
}

void FormulaToken::SetIndex( USHORT )
{
    DBG_ERRORFILE( "FormulaToken::SetIndex: virtual dummy called" );
}

short* FormulaToken::GetJump() const
{
    DBG_ERRORFILE( "FormulaToken::GetJump: virtual dummy called" );
    return NULL;
}


const String& FormulaToken::GetExternal() const
{
    DBG_ERRORFILE( "FormulaToken::GetExternal: virtual dummy called" );
    static  String              aDummyString;
    return aDummyString;
}

FormulaToken* FormulaToken::GetFAPOrigToken() const
{
    DBG_ERRORFILE( "FormulaToken::GetFAPOrigToken: virtual dummy called" );
    return NULL;
}

USHORT FormulaToken::GetError() const
{
    DBG_ERRORFILE( "FormulaToken::GetError: virtual dummy called" );
    return 0;
}

void FormulaToken::SetError( USHORT )
{
    DBG_ERRORFILE( "FormulaToken::SetError: virtual dummy called" );
}
BOOL FormulaToken::TextEqual( const FormulaToken& rToken ) const
{
    return *this == rToken;
}
// ==========================================================================
// real implementations of virtual functions
// --------------------------------------------------------------------------


BYTE FormulaByteToken::GetByte() const                       { return nByte; }
void FormulaByteToken::SetByte( BYTE n )                     { nByte = n; }
bool FormulaByteToken::HasForceArray() const                 { return bHasForceArray; }
void FormulaByteToken::SetForceArray( bool b )               { bHasForceArray = b; }
BOOL FormulaByteToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && nByte == r.GetByte() &&
        bHasForceArray == r.HasForceArray();
}


FormulaToken* FormulaFAPToken::GetFAPOrigToken() const { return pOrigToken.get(); }
BOOL FormulaFAPToken::operator==( const FormulaToken& r ) const
{
    return FormulaByteToken::operator==( r ) && pOrigToken == r.GetFAPOrigToken();
}
short* FormulaJumpToken::GetJump() const                     { return pJump; }
BOOL FormulaJumpToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && pJump[0] == r.GetJump()[0] &&
        memcmp( pJump+1, r.GetJump()+1, pJump[0] * sizeof(short) ) == 0;
}
FormulaJumpToken::~FormulaJumpToken()
{
    delete [] pJump;
}


bool FormulaTokenArray::AddFormulaToken(const sheet::FormulaToken& _aToken,ExternalReferenceHelper* /*_pRef*/)
{
    bool bError = false;
    const OpCode eOpCode = static_cast<OpCode>(_aToken.OpCode);      //! assuming equal values for the moment

    const uno::TypeClass eClass = _aToken.Data.getValueTypeClass();
    switch ( eClass )
    {
        case uno::TypeClass_VOID:
            // empty data -> use AddOpCode (does some special cases)
            AddOpCode( eOpCode );
            break;
        case uno::TypeClass_DOUBLE:
            // double is only used for "push"
            if ( eOpCode == ocPush )
                AddDouble( _aToken.Data.get<double>() );
            else
                bError = true;
            break;
        case uno::TypeClass_LONG:
            {
                // long is svIndex, used for name / database area, or "byte" for spaces
                sal_Int32 nValue = _aToken.Data.get<sal_Int32>();
                if ( eOpCode == ocName || eOpCode == ocDBArea )
                    AddToken( formula::FormulaIndexToken( eOpCode, static_cast<USHORT>(nValue) ) );
                else if ( eOpCode == ocSpaces )
                    AddToken( formula::FormulaByteToken( ocSpaces, static_cast<BYTE>(nValue) ) );
                else
                    bError = true;
            }
            break;
        case uno::TypeClass_STRING:
            {
                String aStrVal( _aToken.Data.get<rtl::OUString>() );
                if ( eOpCode == ocPush )
                    AddString( aStrVal );
                else if ( eOpCode == ocBad )
                    AddBad( aStrVal );
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
bool FormulaTokenArray::Fill(const uno::Sequence< sheet::FormulaToken >& _aSequence,ExternalReferenceHelper* _pRef)
{
    bool bError = false;
    const sal_Int32 nCount = _aSequence.getLength();
    for (sal_Int32 nPos=0; nPos<nCount; nPos++)
    {
        bError |= AddFormulaToken( _aSequence[nPos] ,_pRef);
    }
    return bError;
}
//////////////////////////////////////////////////////////////////////////
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
    return NULL;
}

FormulaToken* FormulaTokenArray::GetNextColRowName()
{
    while( nIndex < nLen )
    {
        FormulaToken* t = pCode[ nIndex++ ];
        if ( t->GetOpCode() == ocColRowName )
            return t;
    }
    return NULL;
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
    return NULL;
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
    return NULL;
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
    return NULL;
}

FormulaToken* FormulaTokenArray::GetNextDBArea()
{
    if( pCode )
    {
        while ( nIndex < nLen )
        {
            FormulaToken* t = pCode[ nIndex++ ];
            if( t->GetOpCode() == ocDBArea )
                return t;
        } // while ( nIndex < nLen )+
    }
    return NULL;
}

FormulaToken* FormulaTokenArray::GetNextOpCodeRPN( OpCode eOp )
{
    while( nIndex < nRPN )
    {
        FormulaToken* t = pRPN[ nIndex++ ];
        if ( t->GetOpCode() == eOp )
            return t;
    }
    return NULL;
}

FormulaToken* FormulaTokenArray::Next()
{
    if( pCode && nIndex < nLen )
        return pCode[ nIndex++ ];
    else
        return NULL;
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
    return NULL;
}

FormulaToken* FormulaTokenArray::NextRPN()
{
    if( pRPN && nIndex < nRPN )
        return pRPN[ nIndex++ ];
    else
        return NULL;
}

FormulaToken* FormulaTokenArray::PrevRPN()
{
    if( pRPN && nIndex )
        return pRPN[ --nIndex ];
    else
        return NULL;
}

void FormulaTokenArray::DelRPN()
{
    if( nRPN )
    {
        FormulaToken** p = pRPN;
        for( USHORT i = 0; i < nRPN; i++ )
        {
            (*p++)->DecRef();
        }
        delete [] pRPN;
    }
    pRPN = NULL;
    nRPN = nIndex = 0;
}

FormulaToken* FormulaTokenArray::PeekPrev( USHORT & nIdx )
{
    if (0 < nIdx && nIdx <= nLen)
        return pCode[--nIdx];
    return NULL;
}

FormulaToken* FormulaTokenArray::PeekNext()
{
    if( pCode && nIndex < nLen )
        return pCode[ nIndex ];
    else
        return NULL;
}

FormulaToken* FormulaTokenArray::PeekNextNoSpaces()
{
    if( pCode && nIndex < nLen )
    {
        USHORT j = nIndex;
        while ( pCode[j]->GetOpCode() == ocSpaces && j < nLen )
            j++;
        if ( j < nLen )
            return pCode[ j ];
        else
            return NULL;
    }
    else
        return NULL;
}

FormulaToken* FormulaTokenArray::PeekPrevNoSpaces()
{
    if( pCode && nIndex > 1 )
    {
        USHORT j = nIndex - 2;
        while ( pCode[j]->GetOpCode() == ocSpaces && j > 0 )
            j--;
        if ( j > 0 || pCode[j]->GetOpCode() != ocSpaces )
            return pCode[ j ];
        else
            return NULL;
    }
    else
        return NULL;
}

bool FormulaTokenArray::HasExternalRef() const
{
    for ( USHORT j=0; j < nLen; j++ )
    {
        if (pCode[j]->IsExternalRef())
            return true;
    }
    return false;
}

BOOL FormulaTokenArray::HasOpCode( OpCode eOp ) const
{
    for ( USHORT j=0; j < nLen; j++ )
    {
        if ( pCode[j]->GetOpCode() == eOp )
            return TRUE;
    }
    return FALSE;
}

BOOL FormulaTokenArray::HasOpCodeRPN( OpCode eOp ) const
{
    for ( USHORT j=0; j < nRPN; j++ )
    {
        if ( pRPN[j]->GetOpCode() == eOp )
            return TRUE;
    }
    return FALSE;
}

BOOL FormulaTokenArray::HasNameOrColRowName() const
{
    for ( USHORT j=0; j < nLen; j++ )
    {
        if( pCode[j]->GetType() == svIndex || pCode[j]->GetOpCode() == ocColRowName )
            return TRUE;
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////

FormulaTokenArray::FormulaTokenArray()
{
    pCode = NULL; pRPN = NULL;
    nError = nLen = nIndex = nRPN = nRefs = 0;
    bHyperLink = FALSE;
    ClearRecalcMode();
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
    nRefs  = r.nRefs;
    nMode  = r.nMode;
    bHyperLink = r.bHyperLink;
    pCode  = NULL;
    pRPN   = NULL;
    FormulaToken** pp;
    if( nLen )
    {
        pp = pCode = new FormulaToken*[ nLen ];
        memcpy( pp, r.pCode, nLen * sizeof( FormulaToken* ) );
        for( USHORT i = 0; i < nLen; i++ )
            (*pp++)->IncRef();
    }
    if( nRPN )
    {
        pp = pRPN = new FormulaToken*[ nRPN ];
        memcpy( pp, r.pRPN, nRPN * sizeof( FormulaToken* ) );
        for( USHORT i = 0; i < nRPN; i++ )
            (*pp++)->IncRef();
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
    p->nRefs = nRefs;
    p->nMode = nMode;
    p->nError = nError;
    p->bHyperLink = bHyperLink;
    FormulaToken** pp;
    if( nLen )
    {
        pp = p->pCode = new FormulaToken*[ nLen ];
        memcpy( pp, pCode, nLen * sizeof( FormulaToken* ) );
        for( USHORT i = 0; i < nLen; i++, pp++ )
        {
            *pp = (*pp)->Clone();
            (*pp)->IncRef();
        }
    }
    if( nRPN )
    {
        pp = p->pRPN = new FormulaToken*[ nRPN ];
        memcpy( pp, pRPN, nRPN * sizeof( FormulaToken* ) );
        for( USHORT i = 0; i < nRPN; i++, pp++ )
        {
            FormulaToken* t = *pp;
            if( t->GetRef() > 1 )
            {
                FormulaToken** p2 = pCode;
                USHORT nIdx = 0xFFFF;
                for( USHORT j = 0; j < nLen; j++, p2++ )
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
        for( USHORT i = 0; i < nLen; i++ )
        {
            (*p++)->DecRef();
        }
        delete [] pCode;
    }
    pCode = NULL; pRPN = NULL;
    nError = nLen = nIndex = nRPN = nRefs = 0;
    bHyperLink = FALSE;
    ClearRecalcMode();
}

FormulaToken* FormulaTokenArray::AddToken( const FormulaToken& r )
{
    return Add( r.Clone() );
}

FormulaToken* FormulaTokenArray::MergeArray( )
{
    return NULL;
}

FormulaToken* FormulaTokenArray::Add( FormulaToken* t )
{
    if( !pCode )
        pCode = new FormulaToken*[ MAXCODE ];
    if( nLen < MAXCODE-1 )
    {
        // fprintf (stderr, "Add : %d\n", t->GetOpCode());
        pCode[ nLen++ ] = t;
        if( t->GetOpCode() == ocPush
            && ( t->GetType() == svSingleRef || t->GetType() == svDoubleRef ) )
            nRefs++;
        t->IncRef();
        if( t->GetOpCode() == ocArrayClose )
            return MergeArray();
        return t;
    }
    else
    {
        t->Delete();
        if ( nLen == MAXCODE-1 )
        {
            t = new FormulaByteToken( ocStop );
            pCode[ nLen++ ] = t;
            t->IncRef();
        }
        return NULL;
    }
}

FormulaToken* FormulaTokenArray::AddString( const sal_Unicode* pStr )
{
    return AddString( String( pStr ) );
}

FormulaToken* FormulaTokenArray::AddString( const String& rStr )
{
    return Add( new FormulaStringToken( rStr ) );
}

FormulaToken* FormulaTokenArray::AddDouble( double fVal )
{
    return Add( new FormulaDoubleToken( fVal ) );
}

FormulaToken* FormulaTokenArray::AddName( USHORT n )
{
    return Add( new FormulaIndexToken( ocName, n ) );
}

FormulaToken* FormulaTokenArray::AddExternal( const sal_Unicode* pStr )
{
    return AddExternal( String( pStr ) );
}

FormulaToken* FormulaTokenArray::AddExternal( const String& rStr,
        OpCode eOp /* = ocExternal */ )
{
    return Add( new FormulaExternalToken( eOp, rStr ) );
}

FormulaToken* FormulaTokenArray::AddBad( const sal_Unicode* pStr )
{
    return AddBad( String( pStr ) );
}

FormulaToken* FormulaTokenArray::AddBad( const String& rStr )
{
    return Add( new FormulaStringOpToken( ocBad, rStr ) );
}



void FormulaTokenArray::AddRecalcMode( ScRecalcMode nBits )
{
    //! Reihenfolge ist wichtig
    if ( nBits & RECALCMODE_ALWAYS )
        SetRecalcModeAlways();
    else if ( !IsRecalcModeAlways() )
    {
        if ( nBits & RECALCMODE_ONLOAD )
            SetRecalcModeOnLoad();
        else if ( nBits & RECALCMODE_ONLOAD_ONCE && !IsRecalcModeOnLoad() )
            SetRecalcModeOnLoadOnce();
    }
    SetCombinedBitsRecalcMode( nBits );
}


BOOL FormulaTokenArray::HasMatrixDoubleRefOps()
{
    if ( pRPN && nRPN )
    {
        // RPN-Interpreter Simulation
        // als Ergebnis jeder Funktion wird einfach ein Double angenommen
        FormulaToken** pStack = new FormulaToken* [nRPN];
        FormulaToken* pResult = new FormulaDoubleToken( 0.0 );
        short sp = 0;
        for ( USHORT j = 0; j < nRPN; j++ )
        {
            FormulaToken* t = pRPN[j];
            OpCode eOp = t->GetOpCode();
            BYTE nParams = t->GetParamCount();
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
                    for ( BYTE k = nParams; k; k-- )
                    {
                        if ( sp >= k && pStack[sp-k]->GetType() == svDoubleRef )
                        {
                            pResult->Delete();
                            delete [] pStack;
                            return TRUE;
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
            else if ( eOp == ocIf || eOp == ocChose )
            {   // Jumps ignorieren, vorheriges Result (Condition) poppen
                if ( sp )
                    --sp;
            }
            else
            {   // pop parameters, push result
                sp = sal::static_int_cast<short>( sp - nParams );
                if ( sp < 0 )
                {
                    OSL_FAIL( "FormulaTokenArray::HasMatrixDoubleRefOps: sp < 0" );
                    sp = 0;
                }
                pStack[sp++] = pResult;
            }
        }
        pResult->Delete();
        delete [] pStack;
    }

    return FALSE;
}



// --- POF (plain old formula) rewrite of a token array ---------------------

inline bool MissingConvention::isRewriteNeeded( OpCode eOp ) const
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
            return !isODFF();   // rewrite only for PODF
        default:
            return false;
    }
}

class FormulaMissingContext
{
    public:
            const FormulaToken* mpFunc;
            int                 mnCurArg;

                    void    Clear() { mpFunc = NULL; mnCurArg = 0; }
            inline  bool    AddDefaultArg( FormulaTokenArray* pNewArr, int nArg, double f ) const;
                    bool    AddMissingExternal( FormulaTokenArray* pNewArr ) const;
                    bool    AddMissing( FormulaTokenArray *pNewArr, const MissingConvention & rConv  ) const;
                    void    AddMoreArgs( FormulaTokenArray *pNewArr, const MissingConvention & rConv  ) const;
};

void FormulaMissingContext::AddMoreArgs( FormulaTokenArray *pNewArr, const MissingConvention & rConv  ) const
{
    if ( !mpFunc )
        return;

    switch (mpFunc->GetOpCode())
    {
        case ocGammaDist:
            if (mnCurArg == 2)
            {
                pNewArr->AddOpCode( ocSep );
                pNewArr->AddDouble( 1.0 );      // 4th, Cumulative=TRUE()
            }
            break;
        case ocPoissonDist:
            if (mnCurArg == 1)
            {
                pNewArr->AddOpCode( ocSep );
                pNewArr->AddDouble( 1.0 );      // 3rd, Cumulative=TRUE()
            }
            break;
        case ocNormDist:
            if ( mnCurArg == 2 )
            {
                pNewArr->AddOpCode( ocSep );
                pNewArr->AddDouble( 1.0 );      // 4th, Cumulative=TRUE()
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
            if ( !rConv.isODFF() && mnCurArg == 0 )
            {
                pNewArr->AddOpCode( ocSep );
                pNewArr->AddDouble( 10.0 );     // 2nd, basis 10
            }
            break;
        default:
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

    const String &rName = mpFunc->GetExternal();

    // initial (fast) check:
    sal_Unicode nLastChar = rName.GetChar( rName.Len() - 1);
    if ( nLastChar != 't' && nLastChar != 'm' )
        return false;

    if (rName.EqualsIgnoreCaseAscii(
                "com.sun.star.sheet.addin.Analysis.getAccrint" ))
    {
        return AddDefaultArg( pNewArr, 4, 1000.0 );
    }
    if (rName.EqualsIgnoreCaseAscii(
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

    // Add for both, PODF and ODFF
    switch (eOp)
    {
        case ocAddress:
            return AddDefaultArg( pNewArr, 2, 1.0 );    // abs
        default:
            break;
    }

    if (rConv.isODFF())
    {
        // Add for ODFF
    }
    else
    {
        // Add for PODF
        switch (eOp)
        {
            case ocFixed:
                return AddDefaultArg( pNewArr, 1, 2.0 );
            case ocBetaDist:
            case ocBetaInv:
            case ocRMZ:     // PMT
                return AddDefaultArg( pNewArr, 3, 0.0 );
            case ocZinsZ:   // IPMT
            case ocKapz:    // PPMT
                return AddDefaultArg( pNewArr, 4, 0.0 );
            case ocBW:      // PV
            case ocZW:      // FV
                bRet |= AddDefaultArg( pNewArr, 2, 0.0 );   // pmt
                bRet |= AddDefaultArg( pNewArr, 3, 0.0 );   // [fp]v
                break;
            case ocZins:    // RATE
                bRet |= AddDefaultArg( pNewArr, 1, 0.0 );   // pmt
                bRet |= AddDefaultArg( pNewArr, 3, 0.0 );   // fv
                bRet |= AddDefaultArg( pNewArr, 4, 0.0 );   // type
                break;
            case ocExternal:
                return AddMissingExternal( pNewArr );

                // --- more complex cases ---

            case ocOffset:
                // FIXME: rather tough.
                // if arg 3 (height) ommitted, export arg1 (rows)
                break;
            default:
                break;
        }
    }

    return bRet;
}

bool FormulaTokenArray::NeedsPofRewrite( const MissingConvention & rConv )
{
    for ( FormulaToken *pCur = First(); pCur; pCur = Next() )
    {
        if ( rConv.isRewriteNeeded( pCur->GetOpCode()))
            return true;
    }
    return false;
}


FormulaTokenArray * FormulaTokenArray::RewriteMissingToPof( const MissingConvention & rConv )
{
    const size_t nAlloc = 256;
    FormulaMissingContext aCtx[ nAlloc ];
    int aOpCodeAddressStack[ nAlloc ];  // use of ADDRESS() function
    const int nOmitAddressArg = 3;      // ADDRESS() 4th parameter A1/R1C1
    USHORT nTokens = GetLen() + 1;
    FormulaMissingContext* pCtx = (nAlloc < nTokens ? new FormulaMissingContext[nTokens] : &aCtx[0]);
    int* pOcas = (nAlloc < nTokens ? new int[nTokens] : &aOpCodeAddressStack[0]);
    // Never go below 0, never use 0, mpFunc always NULL.
    pCtx[0].Clear();
    int nFn = 0;
    int nOcas = 0;

    FormulaTokenArray *pNewArr = new FormulaTokenArray;
    // At least RECALCMODE_ALWAYS needs to be set.
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
            //fprintf( stderr, "ocAddress %d arg %d%s\n", (int)i, (int)pCtx[ pOcas[ i ] ].mnCurArg, (bAdd ? "" : " omitted"));
        }
        switch ( pCur->GetOpCode() )
        {
            case ocOpen:
                ++nFn;      // all following operations on _that_ function
                pCtx[ nFn ].mpFunc = PeekPrevNoSpaces();
                pCtx[ nFn ].mnCurArg = 0;
                if (pCtx[ nFn ].mpFunc && pCtx[ nFn ].mpFunc->GetOpCode() == ocAddress && !rConv.isODFF())
                    pOcas[ nOcas++ ] = nFn;     // entering ADDRESS() if PODF
                break;
            case ocClose:
                pCtx[ nFn ].AddMoreArgs( pNewArr, rConv );
                DBG_ASSERT( nFn > 0, "FormulaTokenArray::RewriteMissingToPof: underflow");
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
                    //fprintf( stderr, "ocAddress %d sep %d omitted\n", (int)nOcas-1, nOmitAddressArg);
                }
                break;
            case ocMissing:
                if (bAdd)
                    bAdd = !pCtx[ nFn ].AddMissing( pNewArr, rConv );
                break;
            default:
                break;
        }
        if (bAdd)
            pNewArr->AddToken( *pCur );
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
        USHORT i = nLen - 1;
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
    FormulaToken* pRet = NULL;
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
        case ocChose:
            {
                short nJump[MAXJUMPCOUNT + 1];
                nJump[ 0 ] = ocIf == eOp ? 3 : MAXJUMPCOUNT+1;
                pRet = new FormulaJumpToken( eOp, (short*)nJump );
            }
            break;
        default:
            pRet = new FormulaByteToken( eOp, 0, FALSE );
            break;
    }
    return AddToken( *pRet );
}


/*----------------------------------------------------------------------*/

FormulaTokenIterator::FormulaTokenIterator( const FormulaTokenArray& rArr )
{
    pCur = NULL;
    Push( &rArr );
}

FormulaTokenIterator::~FormulaTokenIterator()
{
    while( pCur )
        Pop();
}

void FormulaTokenIterator::Push( const FormulaTokenArray* pArr )
{
    ImpTokenIterator* p = new ImpTokenIterator;
    p->pArr  = pArr;
    p->nPC   = -1;
    p->nStop = SHRT_MAX;
    p->pNext = pCur;
    pCur     = p;
}

void FormulaTokenIterator::Pop()
{
    ImpTokenIterator* p = pCur;
    if( p )
    {
        pCur = p->pNext;
        delete p;
    }
}

void FormulaTokenIterator::Reset()
{
    while( pCur->pNext )
        Pop();
    pCur->nPC = -1;
}

const FormulaToken* FormulaTokenIterator::First()
{
    Reset();
    return Next();
}

const FormulaToken* FormulaTokenIterator::Next()
{
    const FormulaToken* t = NULL;
    ++pCur->nPC;
    if( pCur->nPC < pCur->pArr->nRPN && pCur->nPC < pCur->nStop )
    {
        t = pCur->pArr->pRPN[ pCur->nPC ];
        // such an OpCode ends an IF() or CHOOSE() path
        if( t->GetOpCode() == ocSep || t->GetOpCode() == ocClose )
            t = NULL;
    }
    if( !t && pCur->pNext )
    {
        Pop();
        t = Next();
    }
    return t;
}

//! The nPC counts after a Push() are -1

void FormulaTokenIterator::Jump( short nStart, short nNext, short nStop )
{
    pCur->nPC = nNext;
    if( nStart != nNext )
    {
        Push( pCur->pArr );
        pCur->nPC = nStart;
        pCur->nStop = nStop;
    }
}

bool FormulaTokenIterator::IsEndOfPath() const
{
    USHORT nTest = pCur->nPC + 1;
    if( nTest < pCur->pArr->nRPN  && nTest < pCur->nStop )
    {
        const FormulaToken* t = pCur->pArr->pRPN[ nTest ];
        // such an OpCode ends an IF() or CHOOSE() path
        return t->GetOpCode() == ocSep || t->GetOpCode() == ocClose;
    }
    return true;
}
// -----------------------------------------------------------------------------
// ==========================================================================
// real implementations of virtual functions
// --------------------------------------------------------------------------

double      FormulaDoubleToken::GetDouble() const            { return fDouble; }
double &    FormulaDoubleToken::GetDoubleAsReference()       { return fDouble; }
BOOL FormulaDoubleToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && fDouble == r.GetDouble();
}


const String& FormulaStringToken::GetString() const          { return aString; }
BOOL FormulaStringToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && aString == r.GetString();
}


const String& FormulaStringOpToken::GetString() const             { return aString; }
BOOL FormulaStringOpToken::operator==( const FormulaToken& r ) const
{
    return FormulaByteToken::operator==( r ) && aString == r.GetString();
}

USHORT  FormulaIndexToken::GetIndex() const                  { return nIndex; }
void    FormulaIndexToken::SetIndex( USHORT n )              { nIndex = n; }
BOOL FormulaIndexToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && nIndex == r.GetIndex();
}
const String&   FormulaExternalToken::GetExternal() const    { return aExternal; }
BYTE            FormulaExternalToken::GetByte() const        { return nByte; }
void            FormulaExternalToken::SetByte( BYTE n )      { nByte = n; }
BOOL FormulaExternalToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && nByte == r.GetByte() &&
        aExternal == r.GetExternal();
}


USHORT          FormulaErrorToken::GetError() const          { return nError; }
void            FormulaErrorToken::SetError( USHORT nErr )   { nError = nErr; }
BOOL FormulaErrorToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) &&
        nError == static_cast< const FormulaErrorToken & >(r).GetError();
}
double          FormulaMissingToken::GetDouble() const       { return 0.0; }
const String&   FormulaMissingToken::GetString() const
{
    static  String              aDummyString;
    return aDummyString;
}
BOOL FormulaMissingToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r );
}


BOOL FormulaUnknownToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r );
}

// -----------------------------------------------------------------------------
} // formula
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
