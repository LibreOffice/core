/*************************************************************************
 *
 *  $RCSfile: token.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dr $ $Date: 2001-02-14 11:10:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#if STLPORT_VERSION<321
#include <stddef.h>
#else
#include <cstddef>
#endif
#include <string.h>

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#include "compiler.hxx"
#include "rechead.hxx"

struct ImpTokenIterator
{
    ImpTokenIterator* pNext;
    ScTokenArray* pArr;
    short nPC;

    DECL_FIXEDMEMPOOL_NEWDEL( ImpTokenIterator );
};

// ImpTokenIterator wird je Interpreter angelegt, mehrfache auch durch
// SubCode via ScTokenIterator Push/Pop moeglich
IMPL_FIXEDMEMPOOL_NEWDEL( ImpTokenIterator, 32, 16 );

// Raw-ScToken Groesse ist FixMembers + MAXSTRLEN ~= 264
IMPL_FIXEDMEMPOOL_NEWDEL( ScToken, 8, 4 );
// ScDoubleToken werden evtl. massig gebraucht, FixMembers + sizeof(double) ~= 16
const USHORT nMemPoolDoubleToken = (0x4000 - 64) / sizeof(ScDoubleToken);
IMPL_FIXEDMEMPOOL_NEWDEL( ScDoubleToken, nMemPoolDoubleToken, nMemPoolDoubleToken );

//-----------------------Funktionen der Klasse ScToken------------------------

inline BOOL lcl_IsReference( OpCode eOp, StackVar eType )
{
    return
        (eOp == ocPush && (eType == svSingleRef || eType == svDoubleRef))
        || (eOp == ocColRowNameAuto && eType == svDoubleRef)
        || (eOp == ocMatRef && eType == svSingleRef)
        ;
}


xub_StrLen ScToken::GetStrLen( const sal_Unicode* pStr )
{
    if ( !pStr )
        return 0;
    register const sal_Unicode* p = pStr;
    while ( *p )
        p++;
    return p - pStr;
}


BOOL ScToken::IsFunction() const
{
    return (eOp != ocPush && eOp != ocBad && eOp != ocColRowName &&
            eOp != ocColRowNameAuto && eOp != ocName && eOp != ocDBArea &&
           (cByte != 0                              // x Parameter
        || (ocEndUnOp < eOp && eOp <= ocEndNoPar)   // kein Parameter
        || (ocIf == eOp ||  ocChose ==  eOp     )   //@ Sprung Kommandos
        || (ocEndNoPar < eOp && eOp <= ocEnd1Par)   // ein Parameter
        || (ocEnd1Par < eOp && eOp <= ocEnd2Par)    // x Parameter (cByte==0 in
                                                    // FuncAutoPilot)
        || eOp == ocMacro || eOp == ocExternal      // Makros, AddIns
        || eOp == ocAnd || eOp == ocOr              // ehemals binaer, jetzt x Params
        || eOp == ocNot || eOp == ocNeg             // unaer aber Function
        || (eOp >= ocInternalBegin && eOp <= ocInternalEnd)     // Internal
        ));
}


BYTE ScToken::GetParamCount() const
{
    if ( eOp <= ocEndDiv && eOp != ocExternal && eOp != ocMacro &&
            eOp != ocIf && eOp != ocChose )
        return 0;       // Parameter und Specials
                        // ocIf und ocChose fuer FAP nicht, haben dann cByte
//2do: BOOL-Parameter ob FAP oder nicht?
    else if ( cByte )
        return cByte;   // alle Funktionen, gilt auch fuer ocExternal und ocMacro
    else if ( ocEndDiv < eOp && eOp <= ocEndBinOp )
        return 2;       // binaer
    else if ( ocEndBinOp < eOp && eOp <= ocEndUnOp )
        return 1;       // unaer
    else if ( ocEndUnOp < eOp && eOp <= ocEndNoPar )
        return 0;       // kein Parameter
    else if ( ocEndNoPar < eOp && eOp <= ocEnd1Par )
        return 1;       // ein Parameter
    else
        return 0;       // der Rest, kein Parameter, oder
                        // wenn dann sollte er in cByte sein
}


BOOL ScToken::IsMatrixFunction() const
{
    switch ( eOp )
    {
        case ocDde :
        case ocGrowth :
        case ocTrend :
        case ocRKP :
        case ocRGP :
        case ocFrequency :
        case ocMatTrans :
        case ocMatMult :
        case ocMatInv :
        case ocMatrixUnit :
            return TRUE;
        break;
    }
    return FALSE;
}


void ScToken::SetOpCode( OpCode e )
{
    eOp   = e;
    if( eOp == ocIf )
    {
        eType = svJump; nJump[ 0 ] = 3; // If, Else, Behind
    }
    else if( eOp == ocChose )
    {
        eType = svJump; nJump[ 0 ] = MAXJUMPCOUNT+1;
    }
    else if( eOp == ocMissing )
        eType = svMissing;
    else
        eType = svByte, cByte = 0;
    nRefCnt = 0;
}

void ScToken::SetString( const sal_Unicode* pStr )
{
    eOp   = ocPush;
    eType = svString;
    if ( pStr )
    {
        xub_StrLen nLen = GetStrLen( pStr ) + 1;
        if( nLen > MAXSTRLEN )
            nLen = MAXSTRLEN;
        memcpy( cStr, pStr, GetStrLenBytes( nLen ) );
        cStr[ nLen-1 ] = 0;
    }
    else
        cStr[0] = 0;
    nRefCnt = 0;
}

void ScToken::SetSingleReference( const SingleRefData& rRef )
{
    eOp       = ocPush;
    eType     = svSingleRef;
    aRef.Ref1 =
    aRef.Ref2 = rRef;
    nRefCnt   = 0;
}

void ScToken::SetDoubleReference( const ComplRefData& rRef )
{
    eOp   = ocPush;
    eType = svDoubleRef;
    aRef  = rRef;
    nRefCnt = 0;
}

void ScToken::SetReference( ComplRefData& rRef )
{
    DBG_ASSERT( lcl_IsReference( eOp, GetType() ), "SetReference: no Ref" );
    aRef = rRef;
    if( GetType() == svSingleRef )
        aRef.Ref2 = aRef.Ref1;
}

void ScToken::SetByte( BYTE c )
{
    eOp   = ocPush;
    eType = svByte;
    cByte = c;
    nRefCnt = 0;
}

void ScToken::SetDouble(double rVal)
{
    eOp   = ocPush;
    eType = svDouble;
    nValue = rVal;
    nRefCnt = 0;
}

void ScToken::SetInt(int rVal)
{
    eOp   = ocPush;
    eType = svDouble;
    nValue = (double)rVal;
    nRefCnt = 0;
}

void ScToken::SetName( USHORT n )
{
    eOp    = ocName;
    eType  = svIndex;
    nIndex = n;
    nRefCnt = 0;
}

ComplRefData& ScToken::GetReference()
{
    DBG_ASSERT( lcl_IsReference( eOp, GetType() ), "GetReference: no Ref" );
    return aRef;
}

void ScToken::SetExternal( const sal_Unicode* pStr )
{
    eOp   = ocExternal;
    eType = svExternal;
    xub_StrLen nLen = GetStrLen( pStr ) + 1;
    if( nLen >= MAXSTRLEN )
        nLen = MAXSTRLEN-1;
    // Platz fuer Byte-Parameter lassen!
    memcpy( cStr+1, pStr, GetStrLenBytes( nLen ) );
    cStr[ nLen+1 ] = 0;
    nRefCnt = 0;
}

void ScToken::SetMatrix( ScMatrix* p )
{
    eOp   = ocPush;
    eType = svMatrix;
    pMat  = p;
    nRefCnt = 0;
}

ScToken* ScToken::Clone() const
{
    ScToken* p;
    if ( eType == svDouble )
    {
        p = (ScToken*) new ScDoubleToken;
        p->eOp = eOp;
        p->eType = eType;
        p->nValue = nValue;
    }
    else
    {
        USHORT n = offsetof( ScToken, cByte );
        switch( eType )
        {
            case svByte:        n++; break;
            case svDouble:      n += sizeof(double); break;
            case svString:      n += GetStrLenBytes( cStr ) + GetStrLenBytes( 1 ); break;
            case svSingleRef:
            case svDoubleRef:   n += sizeof(aRef); break;
            case svMatrix:      n += sizeof(ScMatrix*); break;
            case svIndex:       n += sizeof(USHORT); break;
            case svJump:        n += nJump[ 0 ] * 2 + 2; break;
            case svExternal:    n += GetStrLenBytes( cStr+1 ) + GetStrLenBytes( 2 ); break;
            default:            n += (BYTE) cStr[ 0 ];  // unbekannt eingelesen!
        }
        p = (ScToken*) new BYTE[ n ];
        memcpy( p, this, n * sizeof(BYTE) );
    }
    p->nRefCnt = 0;
    p->bRaw = FALSE;
    return p;
}

BOOL ScToken::operator== (const ScToken& rToken) const
{
    //  Ref-Count und bRaw darf hier nicht mit verglichen werden!!!

    USHORT n = 0;
    switch( eType )
    {
        case svByte:        n++; break;
        case svDouble:      n += sizeof(double); break;
        case svString:      n += GetStrLenBytes( cStr ) + GetStrLenBytes( 1 ); break;
        case svSingleRef:
        case svDoubleRef:   n += sizeof(aRef); break;
        case svMatrix:      n += sizeof(ScMatrix*); break;
        case svIndex:       n += sizeof(USHORT); break;
        case svJump:        n += nJump[ 0 ] * 2 + 2; break;
        case svExternal:    n += GetStrLenBytes( cStr+1 ) + GetStrLenBytes( 2 ); break;
        default:            n += (BYTE) cStr[ 0 ];  // unbekannt eingelesen!
    }
    return eOp == rToken.eOp && eType == rToken.eType &&
            ( n == 0 || memcmp( &cByte, &rToken.cByte, n ) == 0 );
}

void ScToken::Delete()
{
    if ( bRaw )
        delete this;                            // FixedMemPool ScToken
    else
    {   // per Clone erzeugt
        switch ( eType )
        {
            case svDouble :
                delete (ScDoubleToken*) this;   // FixedMemPool ScDoubleToken
            break;
            default:
                delete [] (BYTE*) this;
        }
    }
}

//  TextEqual: gleiche Formel eingegeben (fuer Optimierung beim Sortieren)

BOOL ScToken::TextEqual(const ScToken& rToken) const
{
    if ( eType == svSingleRef || eType == svDoubleRef )
    {
        //  bei relativen Refs auch nur den relativen Teil vergleichen

        if ( eOp != rToken.eOp || eType != rToken.eType )
            return FALSE;

        ComplRefData aTemp1 = aRef;
        ComplRefData aTemp2 = rToken.aRef;

        ScAddress aPos;
        aTemp1.SmartRelAbs(aPos);
        aTemp2.SmartRelAbs(aPos);

        //  memcmp geht schief wegen des Alignment-Bytes hinter bFlags
        //  nach SmartRelAbs muessen nur die absoluten Teile verglichen werden
        return aTemp1.Ref1.nCol   == aTemp2.Ref1.nCol   &&
               aTemp1.Ref1.nRow   == aTemp2.Ref1.nRow   &&
               aTemp1.Ref1.nTab   == aTemp2.Ref1.nTab   &&
               aTemp1.Ref1.bFlags == aTemp2.Ref1.bFlags &&
               aTemp1.Ref2.nCol   == aTemp2.Ref2.nCol   &&
               aTemp1.Ref2.nRow   == aTemp2.Ref2.nRow   &&
               aTemp1.Ref2.nTab   == aTemp2.Ref2.nTab   &&
               aTemp1.Ref2.bFlags == aTemp2.Ref2.bFlags;
    }
    else
        return *this == rToken;     // sonst normaler operator==
}

//////////////////////////////////////////////////////////////////////////

ScToken* ScTokenArray::GetNextReference()
{
    while( nIndex < nLen )
    {
        ScToken* t = pCode[ nIndex++ ];
        switch( t->GetType() )
        {
            case svSingleRef:
            case svDoubleRef:
                return t;
        }
    }
    return NULL;
}

ScToken* ScTokenArray::GetNextColRowName()
{
    while( nIndex < nLen )
    {
        ScToken* t = pCode[ nIndex++ ];
        if ( t->GetOpCode() == ocColRowName )
            return t;
    }
    return NULL;
}

ScToken* ScTokenArray::GetNextReferenceRPN()
{
    while( nIndex < nRPN )
    {
        ScToken* t = pRPN[ nIndex++ ];
        switch( t->GetType() )
        {
            case svSingleRef:
            case svDoubleRef:
                return t;
        }
    }
    return NULL;
}

ScToken* ScTokenArray::GetNextReferenceOrName()
{
    for( ScToken* t = Next(); t; t = Next() )
    {
        switch( t->GetType() )
        {
            case svSingleRef:
            case svDoubleRef:
            case svIndex:
                return t;
        }
    }
    return NULL;
}

ScToken* ScTokenArray::GetNextName()
{
    for( ScToken* t = Next(); t; t = Next() )
    {
        if( t->GetType() == svIndex )
            return t;
    }
    return NULL;
}

ScToken* ScTokenArray::GetNextDBArea()
{
    for( ScToken* t = Next(); t; t = Next() )
    {
        if ( t->GetOpCode() == ocDBArea )
            return t;
    }
    return NULL;
}

ScToken* ScTokenArray::GetNextOpCodeRPN( OpCode eOp )
{
    while( nIndex < nRPN )
    {
        ScToken* t = pRPN[ nIndex++ ];
        if ( t->GetOpCode() == eOp )
            return t;
    }
    return NULL;
}

ScToken* ScTokenArray::Next()
{
    if( pCode && nIndex < nLen )
        return pCode[ nIndex++ ];
    else
        return NULL;
}

ScToken* ScTokenArray::NextRPN()
{
    if( pRPN && nIndex < nRPN )
        return pRPN[ nIndex++ ];
    else
        return NULL;
}

ScToken* ScTokenArray::PrevRPN()
{
    if( pRPN && nIndex )
        return pRPN[ --nIndex ];
    else
        return NULL;
}

void ScTokenArray::DelRPN()
{
    if( nRPN )
    {
        ScToken** p = pRPN;
        for( USHORT i = 0; i < nRPN; i++ )
        {
            (*p++)->DecRef();
        }
        delete [] pRPN;
    }
    pRPN = NULL;
    nRPN = nIndex = 0;
}

ScToken* ScTokenArray::PeekNext()
{
    if( pCode && nIndex < nLen )
        return pCode[ nIndex ];
    else
        return NULL;
}

ScToken* ScTokenArray::PeekNextNoSpaces()
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

ScToken* ScTokenArray::PeekPrevNoSpaces()
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

BOOL ScTokenArray::HasOpCodeRPN( OpCode eOp ) const
{
    for ( USHORT j=0; j < nRPN; j++ )
    {
        if ( pRPN[j]->GetOpCode() == eOp )
            return TRUE;
    }
    return FALSE;
}

BOOL ScTokenArray::HasName() const
{
    for ( USHORT j=0; j < nLen; j++ )
    {
        if( pCode[j]->GetType() == svIndex )
            return TRUE;
    }
    return FALSE;
}

BOOL ScTokenArray::IsReference( ScRange& rRange ) const
{
    BOOL bIs = FALSE;
    if ( pCode && nLen == 1 )
    {
        ScToken* pToken = pCode[0];
        if ( pToken )
        {
            if ( pToken->GetType() == svSingleRef )
            {
                SingleRefData& rRef = pToken->GetReference().Ref1;
                rRange.aStart = rRange.aEnd = ScAddress( rRef.nCol, rRef.nRow, rRef.nTab );
                bIs = TRUE;
            }
            else if ( pToken->GetType() == svDoubleRef )
            {
                ComplRefData& rCompl = pToken->GetReference();
                SingleRefData& rRef1 = rCompl.Ref1;
                SingleRefData& rRef2 = rCompl.Ref2;
                rRange.aStart = ScAddress( rRef1.nCol, rRef1.nRow, rRef1.nTab );
                rRange.aEnd   = ScAddress( rRef2.nCol, rRef2.nRow, rRef2.nTab );
                bIs = TRUE;
            }
        }
    }
    return bIs;
}

inline void lcl_GetAddress( ScAddress& rAddress, const ScToken& rToken )    // rToken must be a svSingleRef
{
    rAddress.Set( rToken.aRef.Ref1.nCol, rToken.aRef.Ref1.nRow, rToken.aRef.Ref1.nTab );
}

BOOL ScTokenArray::GetTableOpRefs(
        ScAddress& rFormula,
        ScAddress& rColFirstPos, ScAddress& rColRelPos,
        ScAddress& rRowFirstPos, ScAddress& rRowRelPos,
        BOOL& rbIsMode2 ) const
{
    ScToken* pToken;
    BOOL bRet = FALSE;
    rbIsMode2 = FALSE;
    if( pCode && nLen )
    {
        enum
        {
            stBegin, stTableOp, stOpen, stFormula, stFormulaSep,
            stColFirst, stColFirstSep, stColRel, stColRelSep,
            stRowFirst, stRowFirstSep, stRowRel, stClose, stError
        } eState = stBegin;     // last read token

        USHORT nIndex = 0;
        while( (eState != stError) && (nIndex < nLen) )
        {
            pToken = pCode[ nIndex ];
            if( pToken )
            {
                OpCode eOpCode = pToken->GetOpCode();
                BOOL bIsSingleRef = (eOpCode == ocPush) && (pToken->GetType() == svSingleRef);
                BOOL bIsSep = (eOpCode == ocSep);

                if( eOpCode != ocSpaces )
                {
                    switch( eState )
                    {
                        case stBegin:
                            eState = (eOpCode == ocTableOp) ? stTableOp : stError;
                        break;
                        case stTableOp:
                            eState = (eOpCode == ocOpen) ? stOpen : stError;
                        break;
                        case stOpen:
                            eState = bIsSingleRef ? stFormula : stError;
                            if( bIsSingleRef )
                                lcl_GetAddress( rFormula, *pToken );
                        break;
                        case stFormula:
                            eState = bIsSep ? stFormulaSep : stError;
                        break;
                        case stFormulaSep:
                            eState = bIsSingleRef ? stColFirst : stError;
                            if( bIsSingleRef )
                                lcl_GetAddress( rColFirstPos, *pToken );
                        break;
                        case stColFirst:
                            eState = bIsSep ? stColFirstSep : stError;
                        break;
                        case stColFirstSep:
                            eState = bIsSingleRef ? stColRel : stError;
                            if( bIsSingleRef )
                                lcl_GetAddress( rColRelPos, *pToken );
                        break;
                        case stColRel:
                            eState = bIsSep ? stColRelSep : ((eOpCode == ocClose) ? stClose : stError);
                        break;
                        case stColRelSep:
                            eState = bIsSingleRef ? stRowFirst : stError;
                            if( bIsSingleRef )
                            {
                                lcl_GetAddress( rRowFirstPos, *pToken );
                                rbIsMode2 = TRUE;
                            }
                        break;
                        case stRowFirst:
                            eState = bIsSep ? stRowFirstSep : stError;
                        break;
                        case stRowFirstSep:
                            eState = bIsSingleRef ? stRowRel : stError;
                            if( bIsSingleRef )
                                lcl_GetAddress( rRowRelPos, *pToken );
                        break;
                        case stRowRel:
                            eState = (eOpCode == ocClose) ? stClose : stError;
                        break;
                        default:
                            eState = stError;
                    }
                }
            }
            else
                eState = stError;

            nIndex++;
        }
        bRet = (eState == stClose);
    }
    return bRet;
}

void ScTokenArray::Load30( SvStream& rStream, const ScAddress& rPos )
{
    Clear();
    ScToken* pToks[ MAXCODE ];
    ScToken t;
    for( nLen = 0; nLen < MAXCODE; nLen++ )
    {
        t.Load30( rStream );
        if( t.GetOpCode() == ocStop )
            break;
        else if( t.GetOpCode() == ocPush
          && ( t.GetType() == svSingleRef || t.GetType() == svDoubleRef ) )
        {
            nRefs++;
            t.aRef.CalcRelFromAbs( rPos );
        }
        ScToken* p = pToks[ nLen ] = t.Clone();
        p->IncRef();
    }
    pCode = new ScToken*[ nLen ];
    memcpy( pCode, pToks, nLen * sizeof( ScToken* ) );
}

void ScTokenArray::Load( SvStream& rStream, USHORT nVer, const ScAddress& rPos )
{
    Clear();
    // 0x10 - nRefs
    // 0x20 - nError
    // 0x40 - TokenArray
    // 0x80 - CodeArray
    BYTE cData;
    rStream >> cData;
    if( cData & 0x0F )
        rStream.SeekRel( cData & 0x0F );
    if ( nVer < SC_RECALC_MODE_BITS )
    {
        BYTE cMode;
        rStream >> cMode;
        ImportRecalcMode40( (ScRecalcMode40) cMode );
    }
    else
        rStream >> nMode;
    if( cData & 0x10 )
        rStream >> nRefs;
    if( cData & 0x20 )
        rStream >> nError;
    ScToken* pToks[ MAXCODE ];
    ScToken** pp = pToks;
    ScToken t;
    if( cData & 0x40 )
    {
        rStream >> nLen;
        for( USHORT i = 0; i < nLen; i++ )
        {
            t.Load( rStream, nVer );
            if ( t.GetType() == svSingleRef || t.GetType() == svDoubleRef )
                t.aRef.CalcRelFromAbs( rPos );
                // gespeichert wurde und wird immer absolut
            *pp = t.Clone();
            (*pp++)->IncRef();
        }
        pCode = new ScToken*[ nLen ];
        memcpy( pCode, pToks, nLen * sizeof( ScToken* ) );
    }
    pp = pToks;
    if( cData & 0x80 )
    {
        rStream >> nRPN;
        for( USHORT i = 0; i < nRPN; i++, pp++ )
        {
            BYTE b1, b2 = 0;
            UINT16 nIdx;
            rStream >> b1;
            // 0xFF      - Token folgt
            // 0x40-0x7F - untere 6 Bits, 1 Byte mit 8 weiteren Bits
            // 0x00-0x3F - Index
            if( b1 == 0xFF )
            {
                t.Load( rStream, nVer );
                if ( t.GetType() == svSingleRef || t.GetType() == svDoubleRef )
                    t.aRef.CalcRelFromAbs( rPos );
                    // gespeichert wurde und wird immer absolut
                *pp = t.Clone();
            }
            else
            {
                if( b1 & 0x40 )
                {
                    rStream >> b2;
                    nIdx = ( b1 & 0x3F ) | ( b2 << 6 );
                }
                else
                    nIdx = b1;
                *pp = pCode[ nIdx ];
            }
            (*pp)->IncRef();
            // #73616# CONVERT function recalculated on each load
            if ( nVer < SC_CONVERT_RECALC_ON_LOAD && (*pp)->GetOpCode() == ocConvert )
                AddRecalcMode( RECALCMODE_ONLOAD );
        }
        pRPN = new ScToken*[ nRPN ];
        memcpy( pRPN, pToks, nRPN * sizeof( ScToken* ) );
        // Aeltere Versionen: kein UPN-Array laden
        if( nVer < SC_NEWIF )
            DelRPN();
    }
}

void ScTokenArray::Store( SvStream& rStream, const ScAddress& rPos ) const
{
    // 0x10 - nRefs
    // 0x20 - nError
    // 0x40 - TokenArray
    // 0x80 - CodeArray
    BYTE cFlags = 0;
    if( nRefs )
        cFlags |= 0x10;
    if( nError )
        cFlags |= 0x20;
    if( nLen )
        cFlags |= 0x40;
    if( nRPN )
        cFlags |= 0x80;
    rStream << cFlags;
    // Hier ggf. Zusatzdaten!
    if ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 )
        rStream << (BYTE) ExportRecalcMode40();
    else
        rStream << (BYTE) nMode;
    if( cFlags & 0x10 )
        rStream << (INT16) nRefs;
    if( cFlags & 0x20 )
        rStream << (UINT16) nError;
    if( cFlags & 0x40 )
    {
        rStream << nLen;
        ScToken** p = pCode;
        for( USHORT i = 0; i < nLen; i++, p++ )
        {
            // gespeichert wurde und wird immer absolut
            switch ( (*p)->GetType() )
            {
                case svSingleRef :
                        (*p)->aRef.Ref1.CalcAbsIfRel( rPos );
                    break;
                case svDoubleRef :
                        (*p)->aRef.CalcAbsIfRel( rPos );
                    break;
            }
            (*p)->Store( rStream );
        }
    }
    if( cFlags & 0x80 )
    {
        rStream << nRPN;
        ScToken** p = pRPN;
        for( USHORT i = 0; i < nRPN; i++, p++ )
        {
            ScToken* t = *p;
            USHORT nIdx = 0xFFFF;
            if( t->GetRef() > 1 )
            {
                ScToken** p2 = pCode;
                for( USHORT j = 0; j < nLen; j++, p2++ )
                {
                    if( *p2 == t )
                    {
                        nIdx = j; break;
                    }
                }
            }
            // 0xFF      - Token folgt
            // 0x40-0x7F - untere 6 Bits, 1 Byte mit 8 weiteren Bits
            // 0x00-0x3F - Index
            if( nIdx == 0xFFFF )
            {
                // gespeichert wurde und wird immer absolut
                switch ( t->GetType() )
                {
                    case svSingleRef :
                            t->aRef.Ref1.CalcAbsIfRel( rPos );
                        break;
                    case svDoubleRef :
                            t->aRef.CalcAbsIfRel( rPos );
                        break;
                }
                rStream << (BYTE) 0xFF;
                t->Store( rStream );
            }
            else
            {
                if( nIdx < 0x40 )
                    rStream << (BYTE) nIdx;
                else
                    rStream << (BYTE) ( ( nIdx & 0x3F ) | 0x40 )
                            << (BYTE) ( nIdx >> 6 );
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////

ScTokenArray::ScTokenArray()
{
    pCode = NULL; pRPN = NULL;
    nError = nLen = nIndex = nRPN = nRefs = 0;
    ClearRecalcMode();
}

ScTokenArray::ScTokenArray( const ScTokenArray& rArr )
{
    Assign( rArr );
}

ScTokenArray::~ScTokenArray()
{
    Clear();
}

void ScTokenArray::Assign( const ScTokenArray& r )
{
    nLen   = r.nLen;
    nRPN   = r.nRPN;
    nIndex = r.nIndex;
    nError = r.nError;
    nRefs  = r.nRefs;
    nMode  = r.nMode;
    pCode  = NULL;
    pRPN   = NULL;
    ScToken** pp;
    if( nLen )
    {
        pp = pCode = new ScToken*[ nLen ];
        memcpy( pp, r.pCode, nLen * sizeof( ScToken* ) );
        for( USHORT i = 0; i < nLen; i++ )
            (*pp++)->IncRef();
    }
    if( nRPN )
    {
        pp = pRPN = new ScToken*[ nRPN ];
        memcpy( pp, r.pRPN, nRPN * sizeof( ScToken* ) );
        for( USHORT i = 0; i < nRPN; i++ )
            (*pp++)->IncRef();
    }
}

ScTokenArray& ScTokenArray::operator=( const ScTokenArray& rArr )
{
    Clear();
    Assign( rArr );
    return *this;
}

ScTokenArray* ScTokenArray::Clone() const
{
    ScTokenArray* p = new ScTokenArray;
    p->nLen = nLen;
    p->nRPN = nRPN;
    p->nRefs = nRefs;
    p->nMode = nMode;
    p->nError = nError;
    ScToken** pp;
    if( nLen )
    {
        pp = p->pCode = new ScToken*[ nLen ];
        memcpy( pp, pCode, nLen * sizeof( ScToken* ) );
        for( USHORT i = 0; i < nLen; i++, pp++ )
        {
            *pp = (*pp)->Clone();
            (*pp)->IncRef();
        }
    }
    if( nRPN )
    {
        pp = p->pRPN = new ScToken*[ nRPN ];
        memcpy( pp, pRPN, nRPN * sizeof( ScToken* ) );
        for( USHORT i = 0; i < nRPN; i++, pp++ )
        {
            ScToken* t = *pp;
            if( t->GetRef() > 1 )
            {
                ScToken** p2 = pCode;
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

void ScTokenArray::Clear()
{
    if( nRPN ) DelRPN();
    if( pCode )
    {
        ScToken** p = pCode;
        for( USHORT i = 0; i < nLen; i++ )
        {
            (*p++)->DecRef();
        }
        delete [] pCode;
    }
    pCode = NULL; pRPN = NULL;
    nError = nLen = nIndex = nRPN = nRefs = 0;
    ClearRecalcMode();
}

ScToken* ScTokenArray::AddToken( const ScToken& r )
{
    return Add( r.Clone() );
}

// Wird auch vom Compiler genutzt. Das Token ist per new angelegt!

ScToken* ScTokenArray::Add( ScToken* t )
{
    if( !pCode )
        pCode = new ScToken*[ MAXCODE ];
    if( nLen < MAXCODE )
    {
        pCode[ nLen++ ] = t;
        if( t->GetOpCode() == ocPush
            && ( t->GetType() == svSingleRef || t->GetType() == svDoubleRef ) )
            nRefs++;
        t->IncRef();
        return t;
    }
    else
    {
        t->Delete();
        return NULL;
    }
}

ScToken* ScTokenArray::AddOpCode( OpCode e )
{
    ScToken t;
    t.SetOpCode( e );
    return AddToken( t );
}

ScToken* ScTokenArray::AddString( const sal_Unicode* pStr )
{
    ScToken t;
    t.SetString( pStr );
    return AddToken( t );
}

ScToken* ScTokenArray::AddDouble(double rVal )
{
    ScToken t;
    t.SetDouble( rVal );
    return AddToken( t );
}

ScToken* ScTokenArray::AddSingleReference( const SingleRefData& rRef )
{
    ScToken t;
    t.SetSingleReference( rRef );
    return AddToken( t );
}

ScToken* ScTokenArray::AddDoubleReference( const ComplRefData& rRef )
{
    ScToken t;
    t.SetDoubleReference( rRef );
    return AddToken( t );
}

ScToken* ScTokenArray::AddName( USHORT n )
{
    ScToken t;
    t.SetName( n );
    return AddToken( t );
}

ScToken* ScTokenArray::AddExternal( const sal_Unicode* pStr )
{
    ScToken t;
    t.SetExternal( pStr );
    return AddToken( t );
}

ScToken* ScTokenArray::AddMatrix( ScMatrix* p )
{
    ScToken t;
    t.SetMatrix( p );
    return AddToken( t );
}

ScToken* ScTokenArray::AddColRowName( const SingleRefData& rRef )
{
    ScToken t;
    t.SetSingleReference( rRef );
    t.eOp = ocColRowName;
    return AddToken( t );
}

ScToken* ScTokenArray::AddBad( const sal_Unicode* pStr )
{
    ScToken t;
    t.SetString( pStr );
    t.eOp = ocBad;
    return AddToken( t );
}


BOOL ScTokenArray::GetAdjacentExtendOfOuterFuncRefs( USHORT& nExtend,
        const ScAddress& rPos, ScDirection eDir )
{
    USHORT nCol, nRow;
    switch ( eDir )
    {
        case DIR_BOTTOM :
            if ( rPos.Row() < MAXROW )
                nRow = (nExtend = rPos.Row()) + 1;
            else
                return FALSE;
        break;
        case DIR_RIGHT :
            if ( rPos.Col() < MAXCOL )
                nCol = (nExtend = rPos.Col()) + 1;
            else
                return FALSE;
        break;
        case DIR_TOP :
            if ( rPos.Row() > 0 )
                nRow = (nExtend = rPos.Row()) - 1;
            else
                return FALSE;
        break;
        case DIR_LEFT :
            if ( rPos.Col() > 0 )
                nCol = (nExtend = rPos.Col()) - 1;
            else
                return FALSE;
        break;
        default:
            DBG_ERRORFILE( "unknown Direction" );
            return FALSE;
    }
    if ( pRPN && nRPN )
    {
        ScToken* t = pRPN[nRPN-1];
        if ( t->GetType() == svByte )
        {
            BYTE nParamCount = t->cByte;
            if ( nParamCount && nRPN > nParamCount )
            {
                BOOL bRet = FALSE;
                USHORT nParam = nRPN - nParamCount - 1;
                for ( ; nParam < nRPN-1; nParam++ )
                {
                    ScToken* p = pRPN[nParam];
                    switch ( p->GetType() )
                    {
                        case svSingleRef :
                        case svDoubleRef :
                        {
                            ComplRefData& rRef = p->GetReference();
                            rRef.CalcAbsIfRel( rPos );
                            switch ( eDir )
                            {
                                case DIR_BOTTOM :
                                    if ( rRef.Ref1.nRow == nRow
                                            && rRef.Ref2.nRow > nExtend )
                                    {
                                        nExtend = rRef.Ref2.nRow;
                                        bRet = TRUE;
                                    }
                                break;
                                case DIR_RIGHT :
                                    if ( rRef.Ref1.nCol == nCol
                                            && rRef.Ref2.nCol > nExtend )
                                    {
                                        nExtend = rRef.Ref2.nCol;
                                        bRet = TRUE;
                                    }
                                break;
                                case DIR_TOP :
                                    if ( rRef.Ref2.nRow == nRow
                                            && rRef.Ref1.nRow < nExtend )
                                    {
                                        nExtend = rRef.Ref1.nRow;
                                        bRet = TRUE;
                                    }
                                break;
                                case DIR_LEFT :
                                    if ( rRef.Ref2.nCol == nCol
                                            && rRef.Ref1.nCol < nExtend )
                                    {
                                        nExtend = rRef.Ref1.nCol;
                                        bRet = TRUE;
                                    }
                                break;
                            }
                        }
                    } // switch
                } // for
                return bRet;
            }
        }
    }
    return FALSE;
}


void ScTokenArray::ImportRecalcMode40( ScRecalcMode40 eMode )
{
    switch ( eMode )
    {
        case RC_NORMAL :
            nMode = RECALCMODE_NORMAL;
        break;
        case RC_ALWAYS :
            nMode = RECALCMODE_ALWAYS;
        break;
        case RC_ONLOAD :
            nMode = RECALCMODE_ONLOAD;
        break;
        case RC_ONLOAD_ONCE :
            nMode = RECALCMODE_ONLOAD_ONCE;
        break;
        case RC_FORCED :
            nMode = RECALCMODE_NORMAL | RECALCMODE_FORCED;
        break;
        case RC_ONREFMOVE :
            nMode = RECALCMODE_NORMAL | RECALCMODE_ONREFMOVE;
        break;
        default:
            DBG_ERRORFILE( "ScTokenArray::ImportRecalcMode40: unknown ScRecalcMode40" );
            nMode = RECALCMODE_NORMAL;
    }
}


ScRecalcMode40 ScTokenArray::ExportRecalcMode40() const
{
    //! Reihenfolge ist wichtig
    if ( nMode & RECALCMODE_ALWAYS )
        return RC_ALWAYS;
    if ( nMode & RECALCMODE_ONLOAD )
        return RC_ONLOAD;
    if ( nMode & RECALCMODE_FORCED )
        return RC_FORCED;
    if ( nMode & RECALCMODE_ONREFMOVE )
        return RC_ONREFMOVE;
    // kommt eigentlich nicht vor weil in Calc bereits umgesetzt,
    // und woanders gibt es keinen 4.0-Export, deswegen als letztes
    if ( nMode & RECALCMODE_ONLOAD_ONCE )
        return RC_ONLOAD_ONCE;
    return RC_NORMAL;
}


void ScTokenArray::AddRecalcMode( ScRecalcMode nBits )
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


BOOL ScTokenArray::HasMatrixDoubleRefOps()
{
    if ( pRPN && nRPN )
    {
        // RPN-Interpreter Simulation
        // als Ergebnis jeder Funktion wird einfach ein Double angenommen
        ScToken** pStack = new ScToken* [nRPN];
        ScToken* pResult = new ScToken;
        pResult->SetDouble( 0.0 );
        short sp = 0;
        for ( USHORT j = 0; j < nRPN; j++ )
        {
            ScToken* t = pRPN[j];
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
                sp -= nParams;
                if ( sp < 0 )
                {
                    DBG_ERROR( "ScTokenArray::HasMatrixDoubleRefOps: sp < 0" );
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


///////////////////////////////////////////////////////////////////////////

void ScToken::Load30( SvStream& rStream )
{
    UINT16 nOp;
    BYTE n;
    nRefCnt = 0;
    rStream >> nOp;
    eOp = (OpCode) nOp;
    switch( eOp )
    {
        case ocIf:
            eType = svJump; nJump[ 0 ] = 3; break;  // then, else, behind
        case ocChose:
            eType = svJump; nJump[ 0 ] = MAXJUMPCOUNT+1; break;
        case ocPush:
            rStream >> n;
            eType = (StackVar) n;
            switch( eType )
            {
                case svByte:
                    rStream >> cByte;
                    break;
                case svDouble:
                    rStream >> nValue;
                    break;
                case svString:
                {
                    sal_Char c[ MAXSTRLEN+1 ];
                    rStream >> nOp;
                    if( nOp > MAXSTRLEN-1 )
                    {
                        DBG_ERROR("Dokument huehnerich");
                        USHORT nDiff = nOp - (MAXSTRLEN-1);
                        nOp = MAXSTRLEN-1;
                        rStream.Read( c, nOp );
                        rStream.SeekRel( nDiff );
                    }
                    else
                        rStream.Read( c, nOp );
                    CharSet eSrc = rStream.GetStreamCharSet();
                    for ( BYTE j=0; j<nOp; j++ )
                        cStr[j] = ByteString::ConvertToUnicode( c[j], eSrc );
                    cStr[ nOp ] = 0;
                    break;
                }
                case svSingleRef:
                {
                    OldSingleRefBools aBools;
                    rStream >> aRef.Ref1.nCol
                            >> aRef.Ref1.nRow
                            >> aRef.Ref1.nTab
                            >> aBools.bRelCol
                            >> aBools.bRelRow
                            >> aBools.bRelTab
                            >> aBools.bOldFlag3D;
                    aRef.Ref1.OldBoolsToNewFlags( aBools );
                    aRef.Ref2 = aRef.Ref1;
                    break;
                }
                case svDoubleRef:
                {
                    OldSingleRefBools aBools1;
                    OldSingleRefBools aBools2;
                    rStream >> aRef.Ref1.nCol
                            >> aRef.Ref1.nRow
                            >> aRef.Ref1.nTab
                            >> aRef.Ref2.nCol
                            >> aRef.Ref2.nRow
                            >> aRef.Ref2.nTab
                            >> aBools1.bRelCol
                            >> aBools1.bRelRow
                            >> aBools1.bRelTab
                            >> aBools2.bRelCol
                            >> aBools2.bRelRow
                            >> aBools2.bRelTab
                            >> aBools1.bOldFlag3D
                            >> aBools2.bOldFlag3D;
                    aRef.Ref1.OldBoolsToNewFlags( aBools1 );
                    aRef.Ref2.OldBoolsToNewFlags( aBools2 );
                    break;
                }
                default: DBG_ERROR("Unknown Stack Variable");
                break;
            }
            break;
        case ocName:
            eType = svIndex;
            rStream >> nIndex;
            break;
        case ocExternal:
        {
            sal_Char c[ MAXSTRLEN+1 ];
            eType = svExternal;
            rStream >> nOp;
            // lieber ein rottes Dokument als stack overwrite
            if( nOp > MAXSTRLEN-2 )
            {
                DBG_ERROR("Dokument huehnerich");
                USHORT nDiff = nOp - (MAXSTRLEN-2);
                nOp = MAXSTRLEN-2;
                rStream.Read( c, nOp );
                rStream.SeekRel( nDiff );
            }
            else
                rStream.Read( c, nOp );
            CharSet eSrc = rStream.GetStreamCharSet();
            for ( BYTE j=1; j<nOp; j++ )
                cStr[j] = ByteString::ConvertToUnicode( c[j-1], eSrc );
            cStr[ 0 ] = 0;      //! parameter count is what?!?
            cStr[ nOp ] = 0;
            break;
        }
        default:
            eType = svByte;
            cByte = 0;
    }
}

// Bei unbekannten Tokens steht in cStr (k)ein Pascal-String (cStr[0] = Laenge),
// der nur gepuffert wird. cStr[0] = GESAMT-Laenge inkl. [0] !!!

void ScToken::Load( SvStream& rStream, USHORT nVer )
{
    BYTE n;
    UINT16 nOp;
    USHORT i;
    rStream >> nOp >> n;
    eOp = (OpCode) nOp;
    eType = (StackVar) n;
    switch( eType )
    {
        case svByte:
            rStream >> cByte;
            break;
        case svDouble:
            rStream >> nValue;
            break;
        case svExternal:
        {
            sal_Char c[ MAXSTRLEN+1 ];
            rStream >> cByte >> n;
            if( n > MAXSTRLEN-2 )
            {
                DBG_ERRORFILE( "bad string array boundary" );
                USHORT nDiff = n - (MAXSTRLEN-2);
                n = MAXSTRLEN-2;
                rStream.Read( c+1, n );
                rStream.SeekRel( nDiff );
            }
            else
                rStream.Read( c+1, n );
            CharSet eSrc = rStream.GetStreamCharSet();
            for ( BYTE j=1; j<n+1; j++ )
                cStr[j] = ByteString::ConvertToUnicode( c[j], eSrc );
            cStr[ n+1 ] = 0;
            break;
        }
        case svString:
        {
            sal_Char c[ MAXSTRLEN+1 ];
            rStream >> n;
            if( n > MAXSTRLEN-1 )
            {
                DBG_ERRORFILE( "bad string array boundary" );
                USHORT nDiff = n - (MAXSTRLEN-1);
                n = MAXSTRLEN-1;
                rStream.Read( c, n );
                rStream.SeekRel( nDiff );
            }
            else
                rStream.Read( c, n );
            cStr[ n ] = 0;
            CharSet eSrc = rStream.GetStreamCharSet();
            for ( BYTE j=0; j<n; j++ )
                cStr[j] = ByteString::ConvertToUnicode( c[j], eSrc );
            cStr[ n ] = 0;
            break;
        }
        case svSingleRef:
        case svDoubleRef:
        {
            SingleRefData& r = aRef.Ref1;
            rStream >> r.nCol
                    >> r.nRow
                    >> r.nTab
                    >> n;
            if ( nVer < SC_RELATIVE_REFS )
            {
                OldSingleRefBools aBools;
                aBools.bRelCol = ( n & 0x03 );
                aBools.bRelRow = ( ( n >> 2 ) & 0x03 );
                aBools.bRelTab = ( ( n >> 4 ) & 0x03 );
                aBools.bOldFlag3D = ( ( n >> 6 ) & 0x03 );
                r.OldBoolsToNewFlags( aBools );
            }
            else
                r.CreateFlagsFromLoadByte( n );
            if( eType == svSingleRef )
                aRef.Ref2 = r;
            else
            {
                SingleRefData& r = aRef.Ref2;
                rStream >> r.nCol
                        >> r.nRow
                        >> r.nTab
                        >> n;
                if ( nVer < SC_RELATIVE_REFS )
                {
                    OldSingleRefBools aBools;
                    aBools.bRelCol = ( n & 0x03 );
                    aBools.bRelRow = ( ( n >> 2 ) & 0x03 );
                    aBools.bRelTab = ( ( n >> 4 ) & 0x03 );
                    aBools.bOldFlag3D = ( ( n >> 6 ) & 0x03 );
                    r.OldBoolsToNewFlags( aBools );
                }
                else
                    r.CreateFlagsFromLoadByte( n );
            }
            break;
        }
        case svIndex:
            rStream >> nIndex;
            break;
        case svJump:
            rStream >> n;
            nJump[ 0 ] = n;
            for( i = 1; i <= n; i++ )
                rStream >> nJump[ i ];
            break;
        case svMissing:
        case svErr:
            break;
        default:
        {
            rStream >> n;
            cStr[ 0 ] = n;
            // eigentlich kann es nicht 0 sein, aber bei rottem Dokument
            // nicht wild in den Stack lesen
            if ( n > 1 )
                rStream.Read( cStr+1, n-1 );
        }
    }
}

void ScToken::Store( SvStream& rStream ) const
{
    short i;
    rStream << (UINT16) eOp << (BYTE) eType;
    switch( eType )
    {
        case svByte:
            rStream << cByte;
            break;
        case svDouble:
            rStream << nValue;
            break;
        case svExternal:
        {
            ByteString aTmp( cStr+1, rStream.GetStreamCharSet() );
            rStream << cByte
                    << (UINT8) aTmp.Len();
            rStream.Write( aTmp.GetBuffer(), (UINT8) aTmp.Len() );
        }
            break;
        case svString:
        {
            ByteString aTmp( cStr, rStream.GetStreamCharSet() );
            rStream << (UINT8) aTmp.Len();
            rStream.Write( aTmp.GetBuffer(), (UINT8) aTmp.Len() );
        }
            break;
        case svSingleRef:
        case svDoubleRef:
        {
            const SingleRefData& r = aRef.Ref1;
            BYTE n = r.CreateStoreByteFromFlags();
            rStream << (INT16) r.nCol
                    << (INT16) r.nRow
                    << (INT16) r.nTab
                    << (BYTE) n;
            if( eType == svDoubleRef )
            {
                const SingleRefData& r = aRef.Ref2;
                BYTE n = r.CreateStoreByteFromFlags();
                rStream << (INT16) r.nCol
                        << (INT16) r.nRow
                        << (INT16) r.nTab
                        << (BYTE) n;
            }
            break;
        }
        case svIndex:
            rStream << (UINT16) nIndex;
            break;
        case svJump:
            rStream << (BYTE) nJump[ 0 ];
            for( i = 1; i <= nJump[ 0 ]; i++ )
                rStream << (UINT16) nJump[ i ];
            break;
        case svMissing:
        case svErr:
            break;
        default:
            rStream.Write( cStr, cStr[ 0 ] );
    }
}

/*----------------------------------------------------------------------*/

ScTokenIterator::ScTokenIterator( const ScTokenArray& rArr )
{
    pCur = NULL;
    Push( (ScTokenArray*) &rArr );
}

ScTokenIterator::~ScTokenIterator()
{
    while( pCur )
        Pop();
}

void ScTokenIterator::Push( ScTokenArray* pArr )
{
    ImpTokenIterator* p = new ImpTokenIterator;
    p->pArr  = pArr;
    p->nPC   = -1;
    p->pNext = pCur;
    pCur     = p;
}

void ScTokenIterator::Pop()
{
    ImpTokenIterator* p = pCur;
    if( p )
    {
        pCur = p->pNext;
        delete p;
    }
}

void ScTokenIterator::Reset()
{
    while( pCur->pNext )
        Pop();
    pCur->nPC = -1;
}

const ScToken* ScTokenIterator::First()
{
    Reset();
    return Next();
}

const ScToken* ScTokenIterator::Next()
{
    const ScToken* t = NULL;
    if( ++pCur->nPC < pCur->pArr->nRPN )
    {
        t = pCur->pArr->pRPN[ pCur->nPC ];
        // ein derartiger Opcode endet einen WENN- oder WAHL-Bereich
        if( t->GetOpCode() == ocSep || t->GetOpCode() == ocClose )
            t = NULL;
    }
    if( !t && pCur->pNext )
    {
        Pop(); t = Next();
    }
    return t;
}

// Die PC-Werte sind -1!

void ScTokenIterator::Jump( short nStart, short nNext )
{
    pCur->nPC = nNext;
    if( nStart != nNext )
    {
        Push( pCur->pArr );
        pCur->nPC = nStart;
    }
}


