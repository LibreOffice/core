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
#include <tools/mempool.hxx>
#include <osl/diagnose.h>
#include <sfx2/docfile.hxx>

#include "token.hxx"
#include "tokenarray.hxx"
#include "reftokenhelper.hxx"
#include "clipparam.hxx"
#include "compiler.hxx"
#include <formula/compiler.hrc>
#include "rechead.hxx"
#include "parclass.hxx"
#include "jumpmatrix.hxx"
#include "rangeseq.hxx"
#include "externalrefmgr.hxx"
#include "document.hxx"
#include "refupdatecontext.hxx"

using ::std::vector;

#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/ExternalReference.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/sheet/NameToken.hpp>

using namespace formula;
using namespace com::sun::star;

namespace
{
    void lcl_SingleRefToCalc( ScSingleRefData& rRef, const sheet::SingleReference& rAPI )
    {
        rRef.InitFlags();

        rRef.SetColRel(     ( rAPI.Flags & sheet::ReferenceFlags::COLUMN_RELATIVE ) != 0 );
        rRef.SetRowRel(     ( rAPI.Flags & sheet::ReferenceFlags::ROW_RELATIVE    ) != 0 );
        rRef.SetTabRel(     ( rAPI.Flags & sheet::ReferenceFlags::SHEET_RELATIVE  ) != 0 );
        rRef.SetColDeleted( ( rAPI.Flags & sheet::ReferenceFlags::COLUMN_DELETED  ) != 0 );
        rRef.SetRowDeleted( ( rAPI.Flags & sheet::ReferenceFlags::ROW_DELETED     ) != 0 );
        rRef.SetTabDeleted( ( rAPI.Flags & sheet::ReferenceFlags::SHEET_DELETED   ) != 0 );
        rRef.SetFlag3D(     ( rAPI.Flags & sheet::ReferenceFlags::SHEET_3D        ) != 0 );
        rRef.SetRelName(    ( rAPI.Flags & sheet::ReferenceFlags::RELATIVE_NAME   ) != 0 );

        if (rRef.IsColRel())
            rRef.SetRelCol(static_cast<SCCOL>(rAPI.RelativeColumn));
        else
            rRef.SetAbsCol(static_cast<SCCOL>(rAPI.Column));

        if (rRef.IsRowRel())
            rRef.SetRelRow(static_cast<SCROW>(rAPI.RelativeRow));
        else
            rRef.SetAbsRow(static_cast<SCROW>(rAPI.Row));

        if (rRef.IsTabRel())
            rRef.SetRelTab(static_cast<SCsTAB>(rAPI.RelativeSheet));
        else
            rRef.SetAbsTab(static_cast<SCsTAB>(rAPI.Sheet));
    }

    void lcl_ExternalRefToCalc( ScSingleRefData& rRef, const sheet::SingleReference& rAPI )
    {
        rRef.InitFlags();

        rRef.SetColRel(     ( rAPI.Flags & sheet::ReferenceFlags::COLUMN_RELATIVE ) != 0 );
        rRef.SetRowRel(     ( rAPI.Flags & sheet::ReferenceFlags::ROW_RELATIVE    ) != 0 );
        rRef.SetColDeleted( ( rAPI.Flags & sheet::ReferenceFlags::COLUMN_DELETED  ) != 0 );
        rRef.SetRowDeleted( ( rAPI.Flags & sheet::ReferenceFlags::ROW_DELETED     ) != 0 );
        rRef.SetTabDeleted( false );    // sheet must not be deleted for external refs
        rRef.SetFlag3D(     ( rAPI.Flags & sheet::ReferenceFlags::SHEET_3D        ) != 0 );
        rRef.SetRelName(    false );

        if (rRef.IsColRel())
            rRef.SetRelCol(static_cast<SCCOL>(rAPI.RelativeColumn));
        else
            rRef.SetAbsCol(static_cast<SCCOL>(rAPI.Column));

        if (rRef.IsRowRel())
            rRef.SetRelRow(static_cast<SCROW>(rAPI.RelativeRow));
        else
            rRef.SetAbsRow(static_cast<SCROW>(rAPI.Row));

        // sheet index must be absolute for external refs
        rRef.SetAbsTab(0);
    }
//
} // namespace

// Align MemPools on 4k boundaries - 64 bytes (4k is a MUST for OS/2)

// Since RawTokens are temporary for the compiler, don't align on 4k and waste memory.
// ScRawToken size is FixMembers + MAXSTRLEN + ~4 ~= 1036
IMPL_FIXEDMEMPOOL_NEWDEL( ScRawToken )
// Some ScDoubleRawToken, FixMembers + sizeof(double) ~= 16
IMPL_FIXEDMEMPOOL_NEWDEL( ScDoubleRawToken )

// Need a whole bunch of ScSingleRefToken
IMPL_FIXEDMEMPOOL_NEWDEL( ScSingleRefToken )
// Need quite a lot of ScDoubleRefToken
IMPL_FIXEDMEMPOOL_NEWDEL( ScDoubleRefToken )

// --- class ScRawToken -----------------------------------------------------

xub_StrLen ScRawToken::GetStrLen( const sal_Unicode* pStr )
{
    if ( !pStr )
        return 0;
    const sal_Unicode* p = pStr;
    while ( *p )
        p++;
    return sal::static_int_cast<xub_StrLen>( p - pStr );
}


void ScRawToken::SetOpCode( OpCode e )
{
    eOp   = e;
    switch (eOp)
    {
        case ocIf:
            eType = svJump;
            nJump[ 0 ] = 3; // If, Else, Behind
            break;
        case ocIfError:
        case ocIfNA:
            eType = svJump;
            nJump[ 0 ] = 2; // If, Behind
            break;
        case ocChose:
            eType = svJump;
            nJump[ 0 ] = FORMULA_MAXJUMPCOUNT + 1;
            break;
        case ocMissing:
            eType = svMissing;
            break;
        case ocSep:
        case ocOpen:
        case ocClose:
        case ocArrayRowSep:
        case ocArrayColSep:
        case ocArrayOpen:
        case ocArrayClose:
            eType = svSep;
            break;
        default:
            eType = svByte;
            sbyte.cByte = 0;
            sbyte.bHasForceArray = ScParameterClassification::HasForceArray( eOp);
    }
    nRefCnt = 0;
}

void ScRawToken::SetString( const sal_Unicode* pStr )
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

void ScRawToken::SetSingleReference( const ScSingleRefData& rRef )
{
    eOp       = ocPush;
    eType     = svSingleRef;
    aRef.Ref1 =
    aRef.Ref2 = rRef;
    nRefCnt   = 0;
}

void ScRawToken::SetDoubleReference( const ScComplexRefData& rRef )
{
    eOp   = ocPush;
    eType = svDoubleRef;
    aRef  = rRef;
    nRefCnt = 0;
}

void ScRawToken::SetDouble(double rVal)
{
    eOp   = ocPush;
    eType = svDouble;
    nValue = rVal;
    nRefCnt = 0;
}

void ScRawToken::SetErrorConstant( sal_uInt16 nErr )
{
    eOp   = ocPush;
    eType = svError;
    nError = nErr;
    nRefCnt = 0;
}

void ScRawToken::SetName(bool bGlobal, sal_uInt16 nIndex)
{
    eOp = ocName;
    eType = svIndex;
    nRefCnt = 0;

    name.bGlobal = bGlobal;
    name.nIndex = nIndex;
}

void ScRawToken::SetExternalSingleRef( sal_uInt16 nFileId, const String& rTabName, const ScSingleRefData& rRef )
{
    eOp = ocPush;
    eType = svExternalSingleRef;
    nRefCnt = 0;

    extref.nFileId = nFileId;
    extref.aRef.Ref1 =
    extref.aRef.Ref2 = rRef;

    xub_StrLen n = rTabName.Len();
    memcpy(extref.cTabName, rTabName.GetBuffer(), n*sizeof(sal_Unicode));
    extref.cTabName[n] = 0;
}

void ScRawToken::SetExternalDoubleRef( sal_uInt16 nFileId, const String& rTabName, const ScComplexRefData& rRef )
{
    eOp = ocPush;
    eType = svExternalDoubleRef;
    nRefCnt = 0;

    extref.nFileId = nFileId;
    extref.aRef = rRef;

    xub_StrLen n = rTabName.Len();
    memcpy(extref.cTabName, rTabName.GetBuffer(), n*sizeof(sal_Unicode));
    extref.cTabName[n] = 0;
}

void ScRawToken::SetExternalName( sal_uInt16 nFileId, const String& rName )
{
    eOp = ocPush;
    eType = svExternalName;
    nRefCnt = 0;

    extname.nFileId = nFileId;

    xub_StrLen n = rName.Len();
    memcpy(extname.cName, rName.GetBuffer(), n*sizeof(sal_Unicode));
    extname.cName[n] = 0;
}



void ScRawToken::SetExternal( const sal_Unicode* pStr )
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


bool ScRawToken::IsValidReference() const
{
    switch (eType)
    {
        case svSingleRef:
            return aRef.Ref1.Valid();
        case svDoubleRef:
            return aRef.Valid();
        case svExternalSingleRef:
        case svExternalDoubleRef:
            return true;
        default:
            ;   // nothing
    }
    return false;
}


sal_uInt16 ScRawToken::sbyteOffset()
{
    // offset of sbyte in ScRawToken
    // offsetof(ScRawToken, sbyte) gives a warning with gcc, because ScRawToken is no POD

    ScRawToken aToken;
    return static_cast<sal_uInt16>( reinterpret_cast<char*>(&aToken.sbyte) - reinterpret_cast<char*>(&aToken) );
}

ScRawToken* ScRawToken::Clone() const
{
    ScRawToken* p;
    if ( eType == svDouble )
    {
        p = (ScRawToken*) new ScDoubleRawToken;
        p->eOp = eOp;
        p->eType = eType;
        p->nValue = nValue;
    }
    else
    {
        static sal_uInt16 nOffset = sbyteOffset();     // offset of sbyte
        sal_uInt16 n = nOffset;

        switch( eType )
        {
            case svSep:         break;
            case svByte:        n += sizeof(ScRawToken::sbyte); break;
            case svDouble:      n += sizeof(double); break;
            case svError:       n += sizeof(nError); break;
            case svString:      n = sal::static_int_cast<sal_uInt16>( n + GetStrLenBytes( cStr ) + GetStrLenBytes( 1 ) ); break;
            case svSingleRef:
            case svDoubleRef:   n += sizeof(aRef); break;
            case svMatrix:      n += sizeof(ScMatrix*); break;
            case svIndex:       n += sizeof(name); break;
            case svJump:        n += nJump[ 0 ] * 2 + 2; break;
            case svExternal:    n = sal::static_int_cast<sal_uInt16>( n + GetStrLenBytes( cStr+1 ) + GetStrLenBytes( 2 ) ); break;

            // external references
            case svExternalSingleRef:
            case svExternalDoubleRef: n += sizeof(extref); break;
            case svExternalName:      n += sizeof(extname); break;
            default:
            {
                OSL_TRACE( "unknown ScRawToken::Clone() type %d", int(eType));
            }
        }
        p = (ScRawToken*) new sal_uInt8[ n ];
        memcpy( p, this, n * sizeof(sal_uInt8) );
    }
    p->nRefCnt = 0;
    p->bRaw = false;
    return p;
}


FormulaToken* ScRawToken::CreateToken() const
{
#if OSL_DEBUG_LEVEL > 1
#define IF_NOT_OPCODE_ERROR(o,c) if (eOp!=o) OSL_TRACE( #c "::ctor: OpCode %d lost, converted to " #o "; maybe inherit from FormulaToken instead!", int(eOp))
#else
#define IF_NOT_OPCODE_ERROR(o,c)
#endif
    switch ( GetType() )
    {
        case svByte :
            return new FormulaByteToken( eOp, sbyte.cByte, sbyte.bHasForceArray );
        case svDouble :
            IF_NOT_OPCODE_ERROR( ocPush, FormulaDoubleToken);
            return new FormulaDoubleToken( nValue );
        case svString :
            if (eOp == ocPush)
                return new FormulaStringToken( OUString( cStr ) );
            else
                return new FormulaStringOpToken( eOp, OUString( cStr ) );
        case svSingleRef :
            if (eOp == ocPush)
                return new ScSingleRefToken( aRef.Ref1 );
            else
                return new ScSingleRefToken( aRef.Ref1, eOp );
        case svDoubleRef :
            if (eOp == ocPush)
                return new ScDoubleRefToken( aRef );
            else
                return new ScDoubleRefToken( aRef, eOp );
        case svMatrix :
            IF_NOT_OPCODE_ERROR( ocPush, ScMatrixToken);
            return new ScMatrixToken( pMat );
        case svIndex :
            return new FormulaIndexToken( eOp, name.nIndex, name.bGlobal);
        case svExternalSingleRef:
            {
                OUString aTabName(extref.cTabName);
                return new ScExternalSingleRefToken(extref.nFileId, aTabName, extref.aRef.Ref1);
            }
        case svExternalDoubleRef:
            {
                OUString aTabName(extref.cTabName);
                return new ScExternalDoubleRefToken(extref.nFileId, aTabName, extref.aRef);
            }
        case svExternalName:
            {
                OUString aName(extname.cName);
                return new ScExternalNameToken( extname.nFileId, aName );
            }
        case svJump :
            return new FormulaJumpToken( eOp, (short*) nJump );
        case svExternal :
            return new FormulaExternalToken( eOp, sbyte.cByte, OUString( cStr+1 ) );
        case svFAP :
            return new FormulaFAPToken( eOp, sbyte.cByte, NULL );
        case svMissing :
            IF_NOT_OPCODE_ERROR( ocMissing, FormulaMissingToken);
            return new FormulaMissingToken;
        case svSep :
            return new FormulaToken( svSep,eOp );
        case svError :
            return new FormulaErrorToken( nError );
        case svUnknown :
            return new FormulaUnknownToken( eOp );
        default:
            {
                OSL_TRACE( "unknown ScRawToken::CreateToken() type %d", int(GetType()));
                return new FormulaUnknownToken( ocBad );
            }
    }
#undef IF_NOT_OPCODE_ERROR
}


void ScRawToken::Delete()
{
    if ( bRaw )
        delete this;                            // FixedMemPool ScRawToken
    else
    {   // created per Clone
        switch ( eType )
        {
            case svDouble :
                delete (ScDoubleRawToken*) this;    // FixedMemPool ScDoubleRawToken
            break;
            default:
                delete [] (sal_uInt8*) this;
        }
    }
}


// --- class ScToken --------------------------------------------------------

static ScSingleRefData lcl_ScToken_InitSingleRef()
{
    ScSingleRefData aRef;
    aRef.InitAddress( ScAddress() );
    return aRef;
}

static ScComplexRefData lcl_ScToken_InitDoubleRef()
{
    ScComplexRefData aRef;
    aRef.Ref1 = lcl_ScToken_InitSingleRef();
    aRef.Ref2 = aRef.Ref1;
    return aRef;
}

ScToken::~ScToken()
{
}

//  TextEqual: if same formula entered (for optimization in sort)
bool ScToken::TextEqual( const FormulaToken& _rToken ) const
{
    if ( eType == svSingleRef || eType == svDoubleRef )
    {
        //  in relative Refs only compare relative parts

        if ( eType != _rToken.GetType() || GetOpCode() != _rToken.GetOpCode() )
            return false;

        const ScToken& rToken = static_cast<const ScToken&>(_rToken);
        ScComplexRefData aTemp1;
        if ( eType == svSingleRef )
        {
            aTemp1.Ref1 = GetSingleRef();
            aTemp1.Ref2 = aTemp1.Ref1;
        }
        else
            aTemp1 = GetDoubleRef();

        ScComplexRefData aTemp2;
        if ( rToken.eType == svSingleRef )
        {
            aTemp2.Ref1 = rToken.GetSingleRef();
            aTemp2.Ref2 = aTemp2.Ref1;
        }
        else
            aTemp2 = rToken.GetDoubleRef();

        ScAddress aPos;
        ScRange aRange1 = aTemp1.toAbs(aPos), aRange2 = aTemp2.toAbs(aPos);

        //  memcmp doesn't work because of the alignment byte after bFlags.
        //  After SmartRelAbs only absolute parts have to be compared.
        return aRange1 == aRange2 && aTemp1.Ref1.mnFlagValue == aTemp2.Ref1.mnFlagValue && aTemp1.Ref2.mnFlagValue == aTemp2.Ref2.mnFlagValue;
    }
    else
        return *this == _rToken;     // else normal operator==
}


bool ScToken::Is3DRef() const
{
    switch ( eType )
    {
        case svDoubleRef :
            if ( GetSingleRef2().IsFlag3D() )
                return true;
        //! fallthru
        case svSingleRef :
            if ( GetSingleRef().IsFlag3D() )
                return true;
            break;
        default:
        {
            // added to avoid warnings
        }
    }
    return false;
}

#if DEBUG_FORMULA_COMPILER
void ScToken::Dump() const
{
    cout << "-- ScToken (base class)" << endl;
}
#endif

FormulaTokenRef ScToken::ExtendRangeReference( FormulaToken & rTok1, FormulaToken & rTok2,
        const ScAddress & rPos, bool bReuseDoubleRef )
{

    StackVar sv1, sv2;
    // Doing a RangeOp with RefList is probably utter nonsense, but Xcl
    // supports it, so do we.
    if (((sv1 = rTok1.GetType()) != svSingleRef && sv1 != svDoubleRef && sv1 != svRefList &&
            sv1 != svExternalSingleRef && sv1 != svExternalDoubleRef ) ||
        ((sv2 = rTok2.GetType()) != svSingleRef && sv2 != svDoubleRef && sv2 != svRefList))
        return NULL;

    ScToken *p1 = static_cast<ScToken*>(&rTok1);
    ScToken *p2 = static_cast<ScToken*>(&rTok2);

    ScTokenRef xRes;
    bool bExternal = (sv1 == svExternalSingleRef);
    if ((sv1 == svSingleRef || bExternal) && sv2 == svSingleRef)
    {
        // Range references like Sheet1.A1:A2 are generalized and built by
        // first creating a DoubleRef from the first SingleRef, effectively
        // generating Sheet1.A1:A1, and then extending that with A2 as if
        // Sheet1.A1:A1:A2 was encountered, so the mechanisms to adjust the
        // references apply as well.

        /* Given the current structure of external references an external
         * reference can only be extended if the second reference does not
         * point to a different sheet. 'file'#Sheet1.A1:A2 is ok,
         * 'file'#Sheet1.A1:Sheet2.A2 is not. Since we can't determine from a
         * svSingleRef whether the sheet would be different from the one given
         * in the external reference, we have to bail out if there is any sheet
         * specified. NOTE: Xcl does handle external 3D references as in
         * '[file]Sheet1:Sheet2'!A1:A2
         *
         * FIXME: For OOo syntax be smart and remember an external singleref
         * encountered and if followed by ocRange and singleref, create an
         * external singleref for the second singleref. Both could then be
         * merged here. For Xcl syntax already parse an external range
         * reference entirely, cumbersome. */

        const ScSingleRefData& rRef2 = p2->GetSingleRef();
        if (bExternal && rRef2.IsFlag3D())
            return NULL;

        ScComplexRefData aRef;
        aRef.Ref1 = aRef.Ref2 = p1->GetSingleRef();
        aRef.Ref2.SetFlag3D( false);
        aRef.Extend( rRef2, rPos);
        if (bExternal)
            xRes = new ScExternalDoubleRefToken( p1->GetIndex(), p1->GetString(), aRef);
        else
            xRes = new ScDoubleRefToken( aRef);
    }
    else
    {
        bExternal |= (sv1 == svExternalDoubleRef);
        const ScRefList* pRefList = NULL;
        if (sv1 == svDoubleRef)
        {
            xRes = (bReuseDoubleRef && p1->GetRef() == 1 ? p1 : static_cast<ScToken*>(p1->Clone()));
            sv1 = svUnknown;    // mark as handled
        }
        else if (sv2 == svDoubleRef)
        {
            xRes = (bReuseDoubleRef && p2->GetRef() == 1 ? p2 : static_cast<ScToken*>(p2->Clone()));
            sv2 = svUnknown;    // mark as handled
        }
        else if (sv1 == svRefList)
            pRefList = p1->GetRefList();
        else if (sv2 == svRefList)
            pRefList = p2->GetRefList();
        if (pRefList)
        {
            if (!pRefList->size())
                return NULL;
            if (bExternal)
                return NULL;    // external reference list not possible
            xRes = new ScDoubleRefToken( (*pRefList)[0] );
        }
        if (!xRes)
            return NULL;    // shouldn't happen..
        StackVar sv[2] = { sv1, sv2 };
        ScToken* pt[2] = { p1, p2 };
        ScComplexRefData& rRef = xRes->GetDoubleRef();
        for (size_t i=0; i<2; ++i)
        {
            switch (sv[i])
            {
                case svSingleRef:
                    rRef.Extend( pt[i]->GetSingleRef(), rPos);
                    break;
                case svDoubleRef:
                    rRef.Extend( pt[i]->GetDoubleRef(), rPos);
                    break;
                case svRefList:
                    {
                        const ScRefList* p = pt[i]->GetRefList();
                        if (!p->size())
                            return NULL;
                        ScRefList::const_iterator it( p->begin());
                        ScRefList::const_iterator end( p->end());
                        for ( ; it != end; ++it)
                        {
                            rRef.Extend( *it, rPos);
                        }
                    }
                    break;
                case svExternalSingleRef:
                    if (rRef.Ref1.IsFlag3D() || rRef.Ref2.IsFlag3D())
                        return NULL;    // no other sheets with external refs
                    else
                        rRef.Extend( pt[i]->GetSingleRef(), rPos);
                    break;
                case svExternalDoubleRef:
                    if (rRef.Ref1.IsFlag3D() || rRef.Ref2.IsFlag3D())
                        return NULL;    // no other sheets with external refs
                    else
                        rRef.Extend( pt[i]->GetDoubleRef(), rPos);
                    break;
                default:
                    ;   // nothing, prevent compiler warning
            }
        }
    }
    return FormulaTokenRef(xRes.get());
}

const ScSingleRefData& ScToken::GetSingleRef() const
{
    OSL_FAIL( "ScToken::GetSingleRef: virtual dummy called" );
    static ScSingleRefData aDummySingleRef = lcl_ScToken_InitSingleRef();
    return aDummySingleRef;
}

ScSingleRefData& ScToken::GetSingleRef()
{
    OSL_FAIL( "ScToken::GetSingleRef: virtual dummy called" );
    static ScSingleRefData aDummySingleRef = lcl_ScToken_InitSingleRef();
    return aDummySingleRef;
}

const ScComplexRefData& ScToken::GetDoubleRef() const
{
    OSL_FAIL( "ScToken::GetDoubleRef: virtual dummy called" );
    static ScComplexRefData aDummyDoubleRef = lcl_ScToken_InitDoubleRef();
    return aDummyDoubleRef;
}

ScComplexRefData& ScToken::GetDoubleRef()
{
    OSL_FAIL( "ScToken::GetDoubleRef: virtual dummy called" );
    static ScComplexRefData aDummyDoubleRef = lcl_ScToken_InitDoubleRef();
    return aDummyDoubleRef;
}

const ScSingleRefData& ScToken::GetSingleRef2() const
{
    OSL_FAIL( "ScToken::GetSingleRef2: virtual dummy called" );
    static ScSingleRefData aDummySingleRef = lcl_ScToken_InitSingleRef();
    return aDummySingleRef;
}

ScSingleRefData& ScToken::GetSingleRef2()
{
    OSL_FAIL( "ScToken::GetSingleRef2: virtual dummy called" );
    static ScSingleRefData aDummySingleRef = lcl_ScToken_InitSingleRef();
    return aDummySingleRef;
}

const ScMatrix* ScToken::GetMatrix() const
{
    OSL_FAIL( "ScToken::GetMatrix: virtual dummy called" );
    return NULL;
}

ScMatrix* ScToken::GetMatrix()
{
    OSL_FAIL( "ScToken::GetMatrix: virtual dummy called" );
    return NULL;
}


ScJumpMatrix* ScToken::GetJumpMatrix() const
{
    OSL_FAIL( "ScToken::GetJumpMatrix: virtual dummy called" );
    return NULL;
}
const ScRefList* ScToken::GetRefList() const
{
    OSL_FAIL( "ScToken::GetRefList: virtual dummy called" );
    return NULL;
}

ScRefList* ScToken::GetRefList()
{
    OSL_FAIL( "ScToken::GetRefList: virtual dummy called" );
    return NULL;
}
// ==========================================================================
// real implementations of virtual functions
// --------------------------------------------------------------------------




const ScSingleRefData&    ScSingleRefToken::GetSingleRef() const  { return aSingleRef; }
ScSingleRefData&          ScSingleRefToken::GetSingleRef()        { return aSingleRef; }
bool ScSingleRefToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && aSingleRef == static_cast<const ScToken&>(r).GetSingleRef();
}

#if DEBUG_FORMULA_COMPILER
void ScSingleRefToken::Dump() const
{
    cout << "-- ScSingleRefToken" << endl;
    aSingleRef.Dump(1);
}
#endif

const ScSingleRefData&    ScDoubleRefToken::GetSingleRef() const  { return aDoubleRef.Ref1; }
ScSingleRefData&          ScDoubleRefToken::GetSingleRef()        { return aDoubleRef.Ref1; }
const ScComplexRefData&     ScDoubleRefToken::GetDoubleRef() const  { return aDoubleRef; }
ScComplexRefData&           ScDoubleRefToken::GetDoubleRef()        { return aDoubleRef; }
const ScSingleRefData&    ScDoubleRefToken::GetSingleRef2() const { return aDoubleRef.Ref2; }
ScSingleRefData&          ScDoubleRefToken::GetSingleRef2()       { return aDoubleRef.Ref2; }
bool ScDoubleRefToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && aDoubleRef == static_cast<const ScToken&>(r).GetDoubleRef();
}

#if DEBUG_FORMULA_COMPILER
void ScDoubleRefToken::Dump() const
{
    cout << "-- ScDoubleRefToken" << endl;
    aDoubleRef.Dump(1);
}
#endif

const ScRefList*        ScRefListToken::GetRefList() const  { return &aRefList; }
      ScRefList*        ScRefListToken::GetRefList()        { return &aRefList; }
bool ScRefListToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && &aRefList == static_cast<const ScToken&>(r).GetRefList();
}

ScMatrixToken::ScMatrixToken( const ScMatrixRef& p ) :
    ScToken(formula::svMatrix), pMatrix(p) {}

ScMatrixToken::ScMatrixToken( const ScMatrixToken& r ) :
    ScToken(r), pMatrix(r.pMatrix) {}

const ScMatrix* ScMatrixToken::GetMatrix() const        { return pMatrix.get(); }
ScMatrix*       ScMatrixToken::GetMatrix()              { return pMatrix.get(); }
bool ScMatrixToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && pMatrix == static_cast<const ScToken&>(r).GetMatrix();
}

// ============================================================================

ScExternalSingleRefToken::ScExternalSingleRefToken( sal_uInt16 nFileId, const String& rTabName, const ScSingleRefData& r ) :
    ScToken( svExternalSingleRef, ocPush),
    mnFileId(nFileId),
    maTabName(rTabName),
    maSingleRef(r)
{
}

ScExternalSingleRefToken::ScExternalSingleRefToken( const ScExternalSingleRefToken& r ) :
    ScToken(r),
    mnFileId(r.mnFileId),
    maTabName(r.maTabName),
    maSingleRef(r.maSingleRef)
{
}

ScExternalSingleRefToken::~ScExternalSingleRefToken()
{
}

sal_uInt16 ScExternalSingleRefToken::GetIndex() const
{
    return mnFileId;
}

const String& ScExternalSingleRefToken::GetString() const
{
    return maTabName;
}

const ScSingleRefData& ScExternalSingleRefToken::GetSingleRef() const
{
    return maSingleRef;
}

ScSingleRefData& ScExternalSingleRefToken::GetSingleRef()
{
    return maSingleRef;
}

bool ScExternalSingleRefToken::operator ==( const FormulaToken& r ) const
{
    if (!FormulaToken::operator==(r))
        return false;

    if (mnFileId != r.GetIndex())
        return false;

    if (maTabName != r.GetString())
        return false;

    return maSingleRef == static_cast<const ScToken&>(r).GetSingleRef();
}

// ============================================================================

ScExternalDoubleRefToken::ScExternalDoubleRefToken( sal_uInt16 nFileId, const String& rTabName, const ScComplexRefData& r ) :
    ScToken( svExternalDoubleRef, ocPush),
    mnFileId(nFileId),
    maTabName(rTabName),
    maDoubleRef(r)
{
}

ScExternalDoubleRefToken::ScExternalDoubleRefToken( const ScExternalDoubleRefToken& r ) :
    ScToken(r),
    mnFileId(r.mnFileId),
    maTabName(r.maTabName),
    maDoubleRef(r.maDoubleRef)
{
}

ScExternalDoubleRefToken::~ScExternalDoubleRefToken()
{
}

sal_uInt16 ScExternalDoubleRefToken::GetIndex() const
{
    return mnFileId;
}

const String& ScExternalDoubleRefToken::GetString() const
{
    return maTabName;
}

const ScSingleRefData& ScExternalDoubleRefToken::GetSingleRef() const
{
    return maDoubleRef.Ref1;
}

ScSingleRefData& ScExternalDoubleRefToken::GetSingleRef()
{
    return maDoubleRef.Ref1;
}

const ScSingleRefData& ScExternalDoubleRefToken::GetSingleRef2() const
{
    return maDoubleRef.Ref2;
}

ScSingleRefData& ScExternalDoubleRefToken::GetSingleRef2()
{
    return maDoubleRef.Ref2;
}

const ScComplexRefData& ScExternalDoubleRefToken::GetDoubleRef() const
{
    return maDoubleRef;
}

ScComplexRefData& ScExternalDoubleRefToken::GetDoubleRef()
{
    return maDoubleRef;
}

bool ScExternalDoubleRefToken::operator ==( const FormulaToken& r ) const
{
    if (!ScToken::operator==(r))
        return false;

    if (mnFileId != r.GetIndex())
        return false;

    if (maTabName != r.GetString())
        return false;

    return maDoubleRef == static_cast<const ScToken&>(r).GetDoubleRef();
}

// ============================================================================

ScExternalNameToken::ScExternalNameToken( sal_uInt16 nFileId, const String& rName ) :
    ScToken( svExternalName, ocPush),
    mnFileId(nFileId),
    maName(rName)
{
}

ScExternalNameToken::ScExternalNameToken( const ScExternalNameToken& r ) :
    ScToken(r),
    mnFileId(r.mnFileId),
    maName(r.maName)
{
}

ScExternalNameToken::~ScExternalNameToken() {}

sal_uInt16 ScExternalNameToken::GetIndex() const
{
    return mnFileId;
}

const String& ScExternalNameToken::GetString() const
{
    return maName;
}

bool ScExternalNameToken::operator==( const FormulaToken& r ) const
{
    if ( !FormulaToken::operator==(r) )
        return false;

    if (mnFileId != r.GetIndex())
        return false;

    xub_StrLen nLen = maName.Len();
    const String& rName = r.GetString();
    if (nLen != rName.Len())
        return false;

    const sal_Unicode* p1 = maName.GetBuffer();
    const sal_Unicode* p2 = rName.GetBuffer();
    for (xub_StrLen j = 0; j < nLen; ++j)
    {
        if (p1[j] != p2[j])
            return false;
    }
    return true;
}

// ============================================================================

ScJumpMatrix* ScJumpMatrixToken::GetJumpMatrix() const  { return pJumpMatrix; }
bool ScJumpMatrixToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) && pJumpMatrix == static_cast<const ScToken&>(r).GetJumpMatrix();
}
ScJumpMatrixToken::~ScJumpMatrixToken()
{
    delete pJumpMatrix;
}

double          ScEmptyCellToken::GetDouble() const     { return 0.0; }
const String &  ScEmptyCellToken::GetString() const
{
    static  String              aDummyString;
    return aDummyString;
}
bool ScEmptyCellToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) &&
        bInherited == static_cast< const ScEmptyCellToken & >(r).IsInherited() &&
        bDisplayedAsString == static_cast< const ScEmptyCellToken & >(r).IsDisplayedAsString();
}

ScMatrixCellResultToken::ScMatrixCellResultToken( const ScConstMatrixRef& pMat, formula::FormulaToken* pUL ) :
    ScToken(formula::svMatrixCell), xMatrix(pMat), xUpperLeft(pUL) {}

ScMatrixCellResultToken::ScMatrixCellResultToken( const ScMatrixCellResultToken& r ) :
    ScToken(r), xMatrix(r.xMatrix), xUpperLeft(r.xUpperLeft) {}

double          ScMatrixCellResultToken::GetDouble() const  { return xUpperLeft->GetDouble(); }
const String &  ScMatrixCellResultToken::GetString() const  { return xUpperLeft->GetString(); }
const ScMatrix* ScMatrixCellResultToken::GetMatrix() const  { return xMatrix.get(); }
// Non-const GetMatrix() is private and unused but must be implemented to
// satisfy vtable linkage.
ScMatrix* ScMatrixCellResultToken::GetMatrix()
{
    return const_cast<ScMatrix*>(xMatrix.get());
}

bool ScMatrixCellResultToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) &&
        xUpperLeft == static_cast<const ScMatrixCellResultToken &>(r).xUpperLeft &&
        xMatrix == static_cast<const ScMatrixCellResultToken &>(r).xMatrix;
}

FormulaToken* ScMatrixCellResultToken::Clone() const
{
    return new ScMatrixCellResultToken(*this);
}

void ScMatrixCellResultToken::Assign( const ScMatrixCellResultToken & r )
{
    xMatrix = r.xMatrix;
    xUpperLeft = r.xUpperLeft;
}

ScMatrixFormulaCellToken::ScMatrixFormulaCellToken(
    SCCOL nC, SCROW nR, const ScConstMatrixRef& pMat, formula::FormulaToken* pUL ) :
    ScMatrixCellResultToken(pMat, pUL), nRows(nR), nCols(nC) {}

ScMatrixFormulaCellToken::ScMatrixFormulaCellToken( SCCOL nC, SCROW nR ) :
    ScMatrixCellResultToken(NULL, NULL), nRows(nR), nCols(nC) {}

ScMatrixFormulaCellToken::ScMatrixFormulaCellToken( const ScMatrixFormulaCellToken& r ) :
    ScMatrixCellResultToken(r), nRows(r.nRows), nCols(r.nCols)
{
    // xUpperLeft is modifiable through
    // SetUpperLeftDouble(), so clone it.
    if (xUpperLeft)
        xUpperLeft = xUpperLeft->Clone();
}

bool ScMatrixFormulaCellToken::operator==( const FormulaToken& r ) const
{
    const ScMatrixFormulaCellToken* p = dynamic_cast<const ScMatrixFormulaCellToken*>(&r);
    return p && ScMatrixCellResultToken::operator==( r ) &&
        nCols == p->nCols && nRows == p->nRows;
}

void ScMatrixFormulaCellToken::Assign( const ScMatrixCellResultToken & r )
{
    ScMatrixCellResultToken::Assign( r);
}

void ScMatrixFormulaCellToken::Assign( const formula::FormulaToken& r )
{
    if (this == &r)
        return;
    const ScMatrixCellResultToken* p = dynamic_cast<const ScMatrixCellResultToken*>(&r);
    if (p)
        ScMatrixCellResultToken::Assign( *p);
    else
    {
        OSL_ENSURE( r.GetType() != svMatrix, "ScMatrixFormulaCellToken::operator=: assigning ScMatrixToken to ScMatrixFormulaCellToken is not proper, use ScMatrixCellResultToken instead");
        if (r.GetType() == svMatrix)
        {
            xUpperLeft = NULL;
            xMatrix = static_cast<const ScToken&>(r).GetMatrix();
        }
        else
        {
            xUpperLeft = &r;
            xMatrix = NULL;
        }
    }
}

void ScMatrixFormulaCellToken::SetUpperLeftDouble( double f )
{
    switch (GetUpperLeftType())
    {
        case svDouble:
            const_cast<FormulaToken*>(xUpperLeft.get())->GetDoubleAsReference() = f;
            break;
        case svUnknown:
            if (!xUpperLeft)
            {
                xUpperLeft = new FormulaDoubleToken( f);
                break;
            }
            // fall thru
        default:
            {
                OSL_FAIL("ScMatrixFormulaCellToken::SetUpperLeftDouble: not modifying unhandled token type");
            }
    }
}

void ScMatrixFormulaCellToken::ResetResult()
{
    xMatrix = NULL;
    xUpperLeft = NULL;
}


double ScHybridCellToken::GetDouble() const { return mfDouble; }
const String& ScHybridCellToken::GetString() const { return maString; }
bool ScHybridCellToken::operator==( const FormulaToken& r ) const
{
    return FormulaToken::operator==( r ) &&
        mfDouble == r.GetDouble() && maString == r.GetString() &&
        maFormula == static_cast<const ScHybridCellToken &>(r).GetFormula();
}




//////////////////////////////////////////////////////////////////////////

bool ScTokenArray::AddFormulaToken(const com::sun::star::sheet::FormulaToken& _aToken,formula::ExternalReferenceHelper* _pRef)
{
    bool bError = FormulaTokenArray::AddFormulaToken(_aToken,_pRef);
    if ( bError )
    {
        bError = false;
        const OpCode eOpCode = static_cast<OpCode>(_aToken.OpCode);      //! assuming equal values for the moment

        const uno::TypeClass eClass = _aToken.Data.getValueTypeClass();
        switch ( eClass )
        {
            case uno::TypeClass_STRUCT:
                {
                    uno::Type aType = _aToken.Data.getValueType();
                    if ( aType.equals( cppu::UnoType<sheet::SingleReference>::get() ) )
                    {
                        ScSingleRefData aSingleRef;
                        sheet::SingleReference aApiRef;
                        _aToken.Data >>= aApiRef;
                        lcl_SingleRefToCalc( aSingleRef, aApiRef );
                        if ( eOpCode == ocPush )
                            AddSingleReference( aSingleRef );
                        else if ( eOpCode == ocColRowName )
                            AddColRowName( aSingleRef );
                        else
                            bError = true;
                    }
                    else if ( aType.equals( cppu::UnoType<sheet::ComplexReference>::get() ) )
                    {
                        ScComplexRefData aComplRef;
                        sheet::ComplexReference aApiRef;
                        _aToken.Data >>= aApiRef;
                        lcl_SingleRefToCalc( aComplRef.Ref1, aApiRef.Reference1 );
                        lcl_SingleRefToCalc( aComplRef.Ref2, aApiRef.Reference2 );

                        if ( eOpCode == ocPush )
                            AddDoubleReference( aComplRef );
                        else
                            bError = true;
                    }
                    else if ( aType.equals( cppu::UnoType<sheet::NameToken>::get() ) )
                    {
                        sheet::NameToken aTokenData;
                        _aToken.Data >>= aTokenData;
                        if ( eOpCode == ocName )
                            AddRangeName(aTokenData.Index, aTokenData.Global);
                        else if (eOpCode == ocDBArea)
                            AddDBRange(aTokenData.Index);
                        else
                            bError = true;
                    }
                    else if ( aType.equals( cppu::UnoType<sheet::ExternalReference>::get() ) )
                    {
                        sheet::ExternalReference aApiExtRef;
                        if( (eOpCode == ocPush) && (_aToken.Data >>= aApiExtRef) && (0 <= aApiExtRef.Index) && (aApiExtRef.Index <= SAL_MAX_UINT16) )
                        {
                            sal_uInt16 nFileId = static_cast< sal_uInt16 >( aApiExtRef.Index );
                            sheet::SingleReference aApiSRef;
                            sheet::ComplexReference aApiCRef;
                            OUString aName;
                            if( aApiExtRef.Reference >>= aApiSRef )
                            {
                                // try to resolve cache index to sheet name
                                size_t nCacheId = static_cast< size_t >( aApiSRef.Sheet );
                                String aTabName = _pRef->getCacheTableName( nFileId, nCacheId );
                                if( aTabName.Len() > 0 )
                                {
                                    ScSingleRefData aSingleRef;
                                    // convert column/row settings, set sheet index to absolute
                                    lcl_ExternalRefToCalc( aSingleRef, aApiSRef );
                                    AddExternalSingleReference( nFileId, aTabName, aSingleRef );
                                }
                                else
                                    bError = true;
                            }
                            else if( aApiExtRef.Reference >>= aApiCRef )
                            {
                                // try to resolve cache index to sheet name.
                                size_t nCacheId = static_cast< size_t >( aApiCRef.Reference1.Sheet );
                                String aTabName = _pRef->getCacheTableName( nFileId, nCacheId );
                                if( aTabName.Len() > 0 )
                                {
                                    ScComplexRefData aComplRef;
                                    // convert column/row settings, set sheet index to absolute
                                    lcl_ExternalRefToCalc( aComplRef.Ref1, aApiCRef.Reference1 );
                                    lcl_ExternalRefToCalc( aComplRef.Ref2, aApiCRef.Reference2 );
                                    // NOTE: This assumes that cached sheets are in consecutive order!
                                    aComplRef.Ref2.nTab = aComplRef.Ref1.nTab + static_cast<SCsTAB>(aApiCRef.Reference2.Sheet - aApiCRef.Reference1.Sheet);
                                    AddExternalDoubleReference( nFileId, aTabName, aComplRef );
                                }
                                else
                                    bError = true;
                            }
                            else if( aApiExtRef.Reference >>= aName )
                            {
                                if( !aName.isEmpty() )
                                    AddExternalName( nFileId, aName );
                                else
                                    bError = true;
                            }
                            else
                                bError = true;
                        }
                        else
                            bError = true;
                    }
                    else
                        bError = true;      // unknown struct
                }
                break;
            case uno::TypeClass_SEQUENCE:
                {
                    if ( eOpCode != ocPush )
                        bError = true;      // not an inline array
                    else if (!_aToken.Data.getValueType().equals( getCppuType(
                                    (uno::Sequence< uno::Sequence< uno::Any > > *)0)))
                        bError = true;      // unexpected sequence type
                    else
                    {
                        ScMatrixRef xMat = ScSequenceToMatrix::CreateMixedMatrix( _aToken.Data);
                        if (xMat)
                            AddMatrix( xMat);
                        else
                            bError = true;
                    }
                }
                break;
            default:
                bError = true;
        }
    }
    return bError;
}

void ScTokenArray::CheckToken( const FormulaToken& r )
{
    if (meVectorState == FormulaVectorDisabled)
        // It's already disabled.  No more checking needed.
        return;

    OpCode eOp = r.GetOpCode();

    if (SC_OPCODE_START_FUNCTION <= eOp && eOp < SC_OPCODE_STOP_FUNCTION)
    {
        // This is a function opcode. For now, we only support vectorization
        // for min, max, sum and average.
        switch (eOp)
        {
            case ocAverage:
            case ocMin:
            case ocMinA:
            case ocMax:
            case ocMaxA:
            case ocSum:
            case ocSumProduct:
            case ocMatInv:
            case ocCount:
            case ocCount2:
                // Don't change the state.
            break;
            default:
                meVectorState = FormulaVectorDisabled;
        }
        return;
    }

    if (eOp == ocPush)
    {
        // This is a stack variable.  See if this is a reference.

        switch (r.GetType())
        {
            case svByte:
            case svDouble:
            case svString:
                // Don't change the state.
            break;
            case svSingleRef:
            case svDoubleRef:
                // Depends on the reference state.
                meVectorState = FormulaVectorCheckReference;
            break;
            case svError:
            case svEmptyCell:
            case svExternal:
            case svExternalDoubleRef:
            case svExternalName:
            case svExternalSingleRef:
            case svFAP:
            case svHybridCell:
            case svHybridValueCell:
            case svIndex:
            case svJump:
            case svJumpMatrix:
            case svMatrix:
            case svMatrixCell:
            case svMissing:
            case svRefList:
            case svSep:
            case svSubroutine:
            case svUnknown:
                // We don't support vectorization on these.
                meVectorState = FormulaVectorDisabled;
            default:
                ;
        }
    }
}

bool ScTokenArray::ImplGetReference( ScRange& rRange, const ScAddress& rPos, bool bValidOnly ) const
{
    bool bIs = false;
    if ( pCode && nLen == 1 )
    {
        const FormulaToken* pToken = pCode[0];
        if ( pToken )
        {
            if ( pToken->GetType() == svSingleRef )
            {
                const ScSingleRefData& rRef = ((const ScSingleRefToken*)pToken)->GetSingleRef();
                rRange.aStart = rRange.aEnd = rRef.toAbs(rPos);
                bIs = !bValidOnly || ValidAddress(rRange.aStart);
            }
            else if ( pToken->GetType() == svDoubleRef )
            {
                const ScComplexRefData& rCompl = ((const ScDoubleRefToken*)pToken)->GetDoubleRef();
                const ScSingleRefData& rRef1 = rCompl.Ref1;
                const ScSingleRefData& rRef2 = rCompl.Ref2;
                rRange.aStart = rRef1.toAbs(rPos);
                rRange.aEnd   = rRef2.toAbs(rPos);
                bIs = !bValidOnly || ValidRange(rRange);
            }
        }
    }
    return bIs;
}

namespace {

// we want to compare for similar not identical formulae
// so we can't use actual row & column indices.
size_t HashSingleRef( const ScSingleRefData& rRef )
{
    size_t nVal = 0;

    nVal += rRef.Flags.bColRel;
    nVal += (rRef.Flags.bRowRel << 1);
    nVal += (rRef.Flags.bTabRel << 2);

    return nVal;
}

}

void ScTokenArray::GenHash()
{
    static OUStringHash aHasher;

    size_t nHash = 1;
    OpCode eOp;
    StackVar eType;
    const ScToken* p;
    sal_uInt16 n = std::min<sal_uInt16>(nLen, 20);
    for (sal_uInt16 i = 0; i < n; ++i)
    {
        p = static_cast<const ScToken*>(pCode[i]);
        eOp = p->GetOpCode();
        if (eOp == ocPush)
        {
            // This is stack variable. Do additional differentiation.
            eType = p->GetType();
            switch (eType)
            {
                case svByte:
                {
                    // Constant value.
                    sal_uInt8 nVal = p->GetByte();
                    nHash += static_cast<size_t>(nVal);
                }
                break;
                case svDouble:
                {
                    // Constant value.
                    double fVal = p->GetDouble();
                    nHash += static_cast<size_t>(fVal);
                }
                break;
                case svString:
                {
                    // Constant string.
                    const String& rStr = p->GetString();
                    nHash += aHasher(rStr);
                }
                break;
                case svSingleRef:
                {
                    size_t nVal = HashSingleRef(p->GetSingleRef());
                    nHash += nVal;
                }
                break;
                case svDoubleRef:
                {
                    const ScComplexRefData& rRef = p->GetDoubleRef();
                    size_t nVal1 = HashSingleRef(rRef.Ref1);
                    size_t nVal2 = HashSingleRef(rRef.Ref2);
                    nHash += nVal1;
                    nHash += nVal2;
                }
                break;
                default:
                    // Use the opcode value in all the other cases.
                    nHash += static_cast<size_t>(eOp);
            }
        }
        else
            // Use the opcode value in all the other cases.
            nHash += static_cast<size_t>(eOp);

        nHash = (nHash << 4) - nHash;
    }

    mnHashValue = nHash;
}

size_t ScTokenArray::GetHash() const
{
    return mnHashValue;
}

ScFormulaVectorState ScTokenArray::GetVectorState() const
{
    return meVectorState;
}

bool ScTokenArray::IsReference( ScRange& rRange, const ScAddress& rPos ) const
{
    return ImplGetReference(rRange, rPos, false);
}

bool ScTokenArray::IsValidReference( ScRange& rRange, const ScAddress& rPos ) const
{
    return ImplGetReference(rRange, rPos, true);
}

////////////////////////////////////////////////////////////////////////////

ScTokenArray::ScTokenArray() :
    FormulaTokenArray(),
    mnHashValue(0),
    meVectorState(FormulaVectorEnabled)
{
}

ScTokenArray::ScTokenArray( const ScTokenArray& rArr ) :
    FormulaTokenArray(rArr),
    mnHashValue(rArr.mnHashValue),
    meVectorState(rArr.meVectorState)
{
}

ScTokenArray::~ScTokenArray()
{
}

ScTokenArray& ScTokenArray::operator=( const ScTokenArray& rArr )
{
    Clear();
    Assign( rArr );
    return *this;
}

ScTokenArray* ScTokenArray::Clone() const
{
    ScTokenArray* p = new ScTokenArray();
    p->nLen = nLen;
    p->nRPN = nRPN;
    p->nRefs = nRefs;
    p->nMode = nMode;
    p->nError = nError;
    p->bHyperLink = bHyperLink;
    p->mnHashValue = mnHashValue;
    p->meVectorState = meVectorState;

    FormulaToken** pp;
    if( nLen )
    {
        pp = p->pCode = new FormulaToken*[ nLen ];
        memcpy( pp, pCode, nLen * sizeof( ScToken* ) );
        for( sal_uInt16 i = 0; i < nLen; i++, pp++ )
        {
            *pp = (*pp)->Clone();
            (*pp)->IncRef();
        }
    }
    if( nRPN )
    {
        pp = p->pRPN = new FormulaToken*[ nRPN ];
        memcpy( pp, pRPN, nRPN * sizeof( ScToken* ) );
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

FormulaToken* ScTokenArray::AddRawToken( const ScRawToken& r )
{
    return Add( r.CreateToken() );
}

// Utility function to ensure that there is strict alternation of values and
// separators.
static bool
checkArraySep( bool & bPrevWasSep, bool bNewVal )
{
    bool bResult = (bPrevWasSep == bNewVal);
    bPrevWasSep = bNewVal;
    return bResult;
}

FormulaToken* ScTokenArray::MergeArray( )
{
    int nCol = -1, nRow = 0;
    int i, nPrevRowSep = -1, nStart = 0;
    bool bPrevWasSep = false; // top of stack is ocArrayClose
    FormulaToken* t;
    bool bNumeric = false;  // numeric value encountered in current element

    // (1) Iterate from the end to the start to find matrix dims
    // and do basic validation.
    for ( i = nLen ; i-- > nStart ; )
    {
        t = pCode[i];
        switch ( t->GetOpCode() )
        {
            case ocPush :
                if( checkArraySep( bPrevWasSep, false ) )
                {
                    return NULL;
                }

                // no references or nested arrays
                if ( t->GetType() != svDouble  && t->GetType() != svString )
                {
                    return NULL;
                }
                bNumeric = (t->GetType() == svDouble);
            break;

            case ocMissing :
            case ocTrue :
            case ocFalse :
                if( checkArraySep( bPrevWasSep, false ) )
                {
                    return NULL;
                }
                bNumeric = false;
            break;

            case ocArrayColSep :
            case ocSep :
                if( checkArraySep( bPrevWasSep, true ) )
                {
                    return NULL;
                }
                bNumeric = false;
            break;

            case ocArrayClose :
                // not possible with the , but check just in case
                // something changes in the future
                if( i != (nLen-1))
                {
                    return NULL;
                }

                if( checkArraySep( bPrevWasSep, true ) )
                {
                    return NULL;
                }

                nPrevRowSep = i;
                bNumeric = false;
            break;

            case ocArrayOpen :
                nStart = i; // stop iteration
                // fall through to ArrayRowSep

            case ocArrayRowSep :
                if( checkArraySep( bPrevWasSep, true ) )
                {
                    return NULL;
                }

                if( nPrevRowSep < 0 ||              // missing ocArrayClose
                    ((nPrevRowSep - i) % 2) == 1)   // no complex elements
                {
                    return NULL;
                }

                if( nCol < 0 )
                {
                    nCol = (nPrevRowSep - i) / 2;
                }
                else if( (nPrevRowSep - i)/2 != nCol)   // irregular array
                {
                    return NULL;
                }

                nPrevRowSep = i;
                nRow++;
                bNumeric = false;
            break;

            case ocNegSub :
            case ocAdd :
                // negation or unary plus must precede numeric value
                if( !bNumeric )
                {
                    return NULL;
                }
                --nPrevRowSep;      // shorten this row by 1
                bNumeric = false;   // one level only, no --42
            break;

            case ocSpaces :
                // ignore spaces
                --nPrevRowSep;      // shorten this row by 1
            break;

            default :
                // no functions or operators
                return NULL;
        }
    }
    if( nCol <= 0 || nRow <= 0 )
        return NULL;

    int nSign = 1;
    ScMatrix* pArray = new ScMatrix(nCol, nRow, 0.0);
    for ( i = nStart, nCol = 0, nRow = 0 ; i < nLen ; i++ )
    {
        t = pCode[i];

        switch ( t->GetOpCode() )
        {
            case ocPush :
                if ( t->GetType() == svDouble )
                {
                    pArray->PutDouble( t->GetDouble() * nSign, nCol, nRow );
                    nSign = 1;
                }
                else if ( t->GetType() == svString )
                {
                    pArray->PutString( t->GetString(), nCol, nRow );
                }
            break;

            case ocMissing :
                pArray->PutEmpty( nCol, nRow );
            break;

            case ocTrue :
                pArray->PutBoolean( true, nCol, nRow );
            break;

            case ocFalse :
                pArray->PutBoolean( false, nCol, nRow );
            break;

            case ocArrayColSep :
            case ocSep :
                nCol++;
            break;

            case ocArrayRowSep :
                nRow++; nCol = 0;
            break;

            case ocNegSub :
                nSign = -nSign;
            break;

            default :
                break;
        }
        pCode[i] = NULL;
        t->DecRef();
    }
    nLen = sal_uInt16( nStart );
    return AddMatrix( pArray );
}


FormulaToken* ScTokenArray::MergeRangeReference( const ScAddress & rPos )
{
    if (!pCode || !nLen)
        return NULL;
    sal_uInt16 nIdx = nLen;
    FormulaToken *p1, *p2, *p3;      // ref, ocRange, ref
    // The actual types are checked in ExtendRangeReference().
    if (((p3 = PeekPrev(nIdx)) != 0) &&
            (((p2 = PeekPrev(nIdx)) != 0) && p2->GetOpCode() == ocRange) &&
            ((p1 = PeekPrev(nIdx)) != 0))
    {
        FormulaTokenRef p = ScToken::ExtendRangeReference( *p1, *p3, rPos, true);
        if (p)
        {
            p->IncRef();
            p1->DecRef();
            p2->DecRef();
            p3->DecRef();
            nLen -= 2;
            pCode[ nLen-1 ] = p.get();
            nRefs--;
        }
    }
    return pCode[ nLen-1 ];
}

FormulaToken* ScTokenArray::AddOpCode( OpCode e )
{
    ScRawToken t;
    t.SetOpCode( e );
    return AddRawToken( t );
}

FormulaToken* ScTokenArray::AddSingleReference( const ScSingleRefData& rRef )
{
    return Add( new ScSingleRefToken( rRef ) );
}

FormulaToken* ScTokenArray::AddMatrixSingleReference( const ScSingleRefData& rRef )
{
    return Add( new ScSingleRefToken( rRef, ocMatRef ) );
}

FormulaToken* ScTokenArray::AddDoubleReference( const ScComplexRefData& rRef )
{
    return Add( new ScDoubleRefToken( rRef ) );
}

FormulaToken* ScTokenArray::AddMatrix( const ScMatrixRef& p )
{
    return Add( new ScMatrixToken( p ) );
}

FormulaToken* ScTokenArray::AddRangeName( sal_uInt16 n, bool bGlobal )
{
    return Add( new FormulaIndexToken( ocName, n, bGlobal));
}

FormulaToken* ScTokenArray::AddDBRange( sal_uInt16 n )
{
    return Add( new FormulaIndexToken( ocDBArea, n));
}

FormulaToken* ScTokenArray::AddExternalName( sal_uInt16 nFileId, const String& rName )
{
    return Add( new ScExternalNameToken(nFileId, rName) );
}

FormulaToken* ScTokenArray::AddExternalSingleReference( sal_uInt16 nFileId, const String& rTabName, const ScSingleRefData& rRef )
{
    return Add( new ScExternalSingleRefToken(nFileId, rTabName, rRef) );
}

FormulaToken* ScTokenArray::AddExternalDoubleReference( sal_uInt16 nFileId, const String& rTabName, const ScComplexRefData& rRef )
{
    return Add( new ScExternalDoubleRefToken(nFileId, rTabName, rRef) );
}

FormulaToken* ScTokenArray::AddColRowName( const ScSingleRefData& rRef )
{
    return Add( new ScSingleRefToken( rRef, ocColRowName ) );
}

bool ScTokenArray::GetAdjacentExtendOfOuterFuncRefs( SCCOLROW& nExtend,
        const ScAddress& rPos, ScDirection eDir )
{
    SCCOL nCol = 0;
    SCROW nRow = 0;
    switch ( eDir )
    {
        case DIR_BOTTOM :
            if ( rPos.Row() < MAXROW )
                nRow = (nExtend = rPos.Row()) + 1;
            else
                return false;
        break;
        case DIR_RIGHT :
            if ( rPos.Col() < MAXCOL )
                nCol = static_cast<SCCOL>(nExtend = rPos.Col()) + 1;
            else
                return false;
        break;
        case DIR_TOP :
            if ( rPos.Row() > 0 )
                nRow = (nExtend = rPos.Row()) - 1;
            else
                return false;
        break;
        case DIR_LEFT :
            if ( rPos.Col() > 0 )
                nCol = static_cast<SCCOL>(nExtend = rPos.Col()) - 1;
            else
                return false;
        break;
        default:
            OSL_FAIL( "unknown Direction" );
            return false;
    }
    if ( pRPN && nRPN )
    {
        FormulaToken* t = pRPN[nRPN-1];
        if ( t->GetType() == svByte )
        {
            sal_uInt8 nParamCount = t->GetByte();
            if ( nParamCount && nRPN > nParamCount )
            {
                bool bRet = false;
                sal_uInt16 nParam = nRPN - nParamCount - 1;
                for ( ; nParam < nRPN-1; nParam++ )
                {
                    FormulaToken* p = pRPN[nParam];
                    switch ( p->GetType() )
                    {
                        case svSingleRef :
                        {
                            ScSingleRefData& rRef = static_cast<ScToken*>(p)->GetSingleRef();
                            ScAddress aAbs = rRef.toAbs(rPos);
                            switch ( eDir )
                            {
                                case DIR_BOTTOM :
                                    if (aAbs.Row() == nRow && aAbs.Row() > nExtend)
                                    {
                                        nExtend = aAbs.Row();
                                        bRet = true;
                                    }
                                break;
                                case DIR_RIGHT :
                                    if (aAbs.Col() == nCol && static_cast<SCCOLROW>(aAbs.Col()) > nExtend)
                                    {
                                        nExtend = aAbs.Col();
                                        bRet = true;
                                    }
                                break;
                                case DIR_TOP :
                                    if (aAbs.Row() == nRow && aAbs.Row() < nExtend)
                                    {
                                        nExtend = aAbs.Row();
                                        bRet = true;
                                    }
                                break;
                                case DIR_LEFT :
                                    if (aAbs.Col() == nCol && static_cast<SCCOLROW>(aAbs.Col()) < nExtend)
                                    {
                                        nExtend = aAbs.Col();
                                        bRet = true;
                                    }
                                break;
                            }
                        }
                        break;
                        case svDoubleRef :
                        {
                            ScComplexRefData& rRef = static_cast<ScToken*>(p)->GetDoubleRef();
                            ScRange aAbs = rRef.toAbs(rPos);
                            switch ( eDir )
                            {
                                case DIR_BOTTOM :
                                    if (aAbs.aStart.Row() == nRow && aAbs.aEnd.Row() > nExtend)
                                    {
                                        nExtend = aAbs.aEnd.Row();
                                        bRet = true;
                                    }
                                break;
                                case DIR_RIGHT :
                                    if (aAbs.aStart.Col() == nCol && static_cast<SCCOLROW>(aAbs.aEnd.Col()) > nExtend)
                                    {
                                        nExtend = aAbs.aEnd.Col();
                                        bRet = true;
                                    }
                                break;
                                case DIR_TOP :
                                    if (aAbs.aEnd.Row() == nRow && aAbs.aStart.Row() < nExtend)
                                    {
                                        nExtend = aAbs.aStart.Row();
                                        bRet = true;
                                    }
                                break;
                                case DIR_LEFT :
                                    if (aAbs.aEnd.Col() == nCol && static_cast<SCCOLROW>(aAbs.aStart.Col()) < nExtend)
                                    {
                                        nExtend = aAbs.aStart.Col();
                                        bRet = true;
                                    }
                                break;
                            }
                        }
                        break;
                        default:
                        {
                            // added to avoid warnings
                        }
                    } // switch
                } // for
                return bRet;
            }
        }
    }
    return false;
}


void ScTokenArray::ReadjustRelative3DReferences( const ScAddress& rOldPos,
        const ScAddress& rNewPos )
{
    for ( sal_uInt16 j=0; j<nLen; ++j )
    {
        switch ( pCode[j]->GetType() )
        {
            case svDoubleRef :
            {
                ScSingleRefData& rRef2 = static_cast<ScToken*>(pCode[j])->GetSingleRef2();
                // Also adjust if the reference is of the form Sheet1.A2:A3
                if ( rRef2.IsFlag3D() || static_cast<ScToken*>(pCode[j])->GetSingleRef().IsFlag3D() )
                {
                    ScAddress aAbs = rRef2.toAbs(rOldPos);
                    rRef2.SetAddress(aAbs, rNewPos);
                }
            }
            //! fallthru
            case svSingleRef :
            {
                ScSingleRefData& rRef1 = static_cast<ScToken*>(pCode[j])->GetSingleRef();
                if ( rRef1.IsFlag3D() )
                {
                    ScAddress aAbs = rRef1.toAbs(rOldPos);
                    rRef1.SetAddress(aAbs, rNewPos);
                }
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
}

namespace {

void GetExternalTableData(const ScDocument* pOldDoc, const ScDocument* pNewDoc, const SCTAB nTab, OUString& rTabName, sal_uInt16& rFileId)
{
    OUString aFileName = pOldDoc->GetFileURL();;
    rFileId = pNewDoc->GetExternalRefManager()->getExternalFileId(aFileName);
    rTabName = pOldDoc->GetCopyTabName(nTab);
    if (rTabName.isEmpty())
        pOldDoc->GetName(nTab, rTabName);
}

bool IsInCopyRange( const ScRange& rRange, const ScDocument* pClipDoc )
{
    ScClipParam& rClipParam = const_cast<ScDocument*>(pClipDoc)->GetClipParam();
    return rClipParam.maRanges.In(rRange);
}

bool SkipReference(ScToken* pToken, const ScAddress& rPos, const ScDocument* pOldDoc, bool bRangeName, bool bCheckCopyArea)
{
    ScRange aRange;

    if (!ScRefTokenHelper::getRangeFromToken(aRange, pToken, rPos))
        return true;

    if (bRangeName && aRange.aStart.Tab() == rPos.Tab())
    {
        switch (pToken->GetType())
        {
            case svDoubleRef:
                {
                    ScSingleRefData& rRef = pToken->GetSingleRef2();
                    if (rRef.IsColRel() || rRef.IsRowRel())
                        return true;
                } // fall through
            case svSingleRef:
                {
                    ScSingleRefData& rRef = pToken->GetSingleRef();
                    if (rRef.IsColRel() || rRef.IsRowRel())
                        return true;
                }
                break;
            default:
                break;
        }
    }

    if (bCheckCopyArea && IsInCopyRange(aRange, pOldDoc))
        return true;

    return false;
}

void AdjustSingleRefData( ScSingleRefData& rRef, const ScAddress& rOldPos, const ScAddress& rNewPos)
{
    SCsCOL nCols = rNewPos.Col() - rOldPos.Col();
    SCsROW nRows = rNewPos.Row() - rOldPos.Row();
    SCsTAB nTabs = rNewPos.Tab() - rOldPos.Tab();

    if (!rRef.IsColRel())
        rRef.IncCol(nCols);

    if (!rRef.IsRowRel())
        rRef.IncRow(nRows);

    if (!rRef.IsTabRel())
        rRef.IncTab(nTabs);
}

}

void ScTokenArray::ReadjustAbsolute3DReferences( const ScDocument* pOldDoc, const ScDocument* pNewDoc, const ScAddress& rPos, bool bRangeName )
{
    for ( sal_uInt16 j=0; j<nLen; ++j )
    {
        switch ( pCode[j]->GetType() )
        {
            case svDoubleRef :
            {
                if (SkipReference(static_cast<ScToken*>(pCode[j]), rPos, pOldDoc, bRangeName, true))
                    continue;

                ScComplexRefData& rRef = static_cast<ScToken*>(pCode[j])->GetDoubleRef();
                ScSingleRefData& rRef2 = rRef.Ref2;
                ScSingleRefData& rRef1 = rRef.Ref1;

                if ( (rRef2.IsFlag3D() && !rRef2.IsTabRel()) || (rRef1.IsFlag3D() && !rRef1.IsTabRel()) )
                {
                    OUString aTabName;
                    sal_uInt16 nFileId;
                    GetExternalTableData(pOldDoc, pNewDoc, rRef1.nTab, aTabName, nFileId);
                    pCode[j]->DecRef();
                    ScExternalDoubleRefToken* pToken = new ScExternalDoubleRefToken(nFileId, aTabName, rRef);
                    pToken->IncRef();
                    pCode[j] = pToken;
                }
            }
            break;
            case svSingleRef :
            {
                if (SkipReference(static_cast<ScToken*>(pCode[j]), rPos, pOldDoc, bRangeName, true))
                    continue;

                ScSingleRefData& rRef = static_cast<ScToken*>(pCode[j])->GetSingleRef();

                if ( rRef.IsFlag3D() && !rRef.IsTabRel() )
                {
                    OUString aTabName;
                    sal_uInt16 nFileId;
                    GetExternalTableData(pOldDoc, pNewDoc, rRef.nTab, aTabName, nFileId);
                    //replace with ScExternalSingleRefToken and adjust references
                    pCode[j]->DecRef();
                    ScExternalSingleRefToken* pToken = new ScExternalSingleRefToken(nFileId, aTabName, rRef);
                    pToken->IncRef();
                    pCode[j] = pToken;
                }
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
}

void ScTokenArray::AdjustAbsoluteRefs( const ScDocument* pOldDoc, const ScAddress& rOldPos, const ScAddress& rNewPos, bool bRangeName, bool bCheckCopyRange)
{
    for ( sal_uInt16 j=0; j<nLen; ++j )
    {
        switch ( pCode[j]->GetType() )
        {
            case svDoubleRef :
            {
                if (!SkipReference(static_cast<ScToken*>(pCode[j]), rOldPos, pOldDoc, false, bCheckCopyRange))
                    continue;

                ScComplexRefData& rRef = static_cast<ScToken*>(pCode[j])->GetDoubleRef();
                ScSingleRefData& rRef2 = rRef.Ref2;
                ScSingleRefData& rRef1 = rRef.Ref1;

                // for range names only adjust if all parts are absolute
                if (!bRangeName || !(rRef1.IsColRel() || rRef1.IsRowRel() || rRef1.IsTabRel()))
                    AdjustSingleRefData( rRef1, rOldPos, rNewPos );
                if (!bRangeName || !(rRef2.IsColRel() || rRef2.IsRowRel() || rRef2.IsTabRel()))
                    AdjustSingleRefData( rRef2, rOldPos, rNewPos );

            }
            break;
            case svSingleRef :
            {
                if (!SkipReference(static_cast<ScToken*>(pCode[j]), rOldPos, pOldDoc, false, bCheckCopyRange))
                    continue;

                ScSingleRefData& rRef = static_cast<ScToken*>(pCode[j])->GetSingleRef();

                // for range names only adjust if all parts are absolute
                if (!bRangeName || !(rRef.IsColRel() || rRef.IsRowRel() || rRef.IsTabRel()))
                    AdjustSingleRefData( rRef, rOldPos, rNewPos );


            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
}

namespace {

ScRange getSelectedRange( const sc::RefUpdateContext& rCxt )
{
    ScRange aSelectedRange(ScAddress::INITIALIZE_INVALID);
    if (rCxt.mnColDelta < 0)
    {
        // Delete and shift to left.
        aSelectedRange.aStart = ScAddress(rCxt.maRange.aStart.Col()+rCxt.mnColDelta, rCxt.maRange.aStart.Row(), rCxt.maRange.aStart.Tab());
        aSelectedRange.aEnd = ScAddress(rCxt.maRange.aStart.Col()-1, rCxt.maRange.aEnd.Row(), rCxt.maRange.aEnd.Tab());
    }
    else if (rCxt.mnRowDelta < 0)
    {
        // Delete and shift up.
        aSelectedRange.aStart = ScAddress(rCxt.maRange.aStart.Col(), rCxt.maRange.aStart.Row()+rCxt.mnRowDelta, rCxt.maRange.aStart.Tab());
        aSelectedRange.aEnd = ScAddress(rCxt.maRange.aEnd.Col(), rCxt.maRange.aStart.Row()-1, rCxt.maRange.aEnd.Tab());
    }
    else if (rCxt.mnTabDelta < 0)
    {
        // Deleting sheets.
        // TODO : Figure out what to do here.
    }
    else if (rCxt.mnColDelta > 0)
    {
        // Insert and shift to the right.
        aSelectedRange.aStart = rCxt.maRange.aStart;
        aSelectedRange.aEnd = ScAddress(rCxt.maRange.aStart.Col()+rCxt.mnColDelta-1, rCxt.maRange.aEnd.Row(), rCxt.maRange.aEnd.Tab());
    }
    else if (rCxt.mnRowDelta > 0)
    {
        // Insert and shift down.
        aSelectedRange.aStart = rCxt.maRange.aStart;
        aSelectedRange.aEnd = ScAddress(rCxt.maRange.aEnd.Col(), rCxt.maRange.aStart.Row()+rCxt.mnRowDelta-1, rCxt.maRange.aEnd.Tab());
    }
    else if (rCxt.mnTabDelta > 0)
    {
        // Inserting sheets.
        // TODO : Figure out what to do here.
    }

    return aSelectedRange;
}

void setRefDeleted( ScSingleRefData& rRef, const sc::RefUpdateContext& rCxt )
{
    if (rCxt.mnColDelta < 0)
        rRef.SetColDeleted(true);
    else if (rCxt.mnRowDelta < 0)
        rRef.SetRowDeleted(true);
    else if (rCxt.mnTabDelta < 0)
        rRef.SetTabDeleted(true);
}

void setRefDeleted( ScComplexRefData& rRef, const sc::RefUpdateContext& rCxt )
{
    if (rCxt.mnColDelta < 0)
    {
        rRef.Ref1.SetColDeleted(true);
        rRef.Ref2.SetColDeleted(true);
    }
    else if (rCxt.mnRowDelta < 0)
    {
        rRef.Ref1.SetRowDeleted(true);
        rRef.Ref2.SetRowDeleted(true);
    }
    else if (rCxt.mnTabDelta < 0)
    {
        rRef.Ref1.SetTabDeleted(true);
        rRef.Ref2.SetTabDeleted(true);
    }
}

bool shrinkRange( const sc::RefUpdateContext& rCxt, ScRange& rRefRange, const ScRange& rDeletedRange )
{
    if (rCxt.mnColDelta < 0)
    {
        // Shifting left.
        if (rRefRange.aStart.Row() < rDeletedRange.aStart.Row() || rDeletedRange.aEnd.Row() < rRefRange.aEnd.Row())
            // Deleted range is only partially overlapping in vertical direction. Bail out.
            return false;

        // Move the last column position to the left.
        SCCOL nDelta = rDeletedRange.aStart.Col() - rDeletedRange.aEnd.Col() - 1;
        rRefRange.aEnd.IncCol(nDelta);
        return true;
    }
    else if (rCxt.mnRowDelta < 0)
    {
        // Shifting up.

        if (rRefRange.aStart.Col() < rDeletedRange.aStart.Col() || rDeletedRange.aEnd.Col() < rRefRange.aEnd.Col())
            // Deleted range is only partially overlapping in horizontal direction. Bail out.
            return false;

        // Move the last row position up.
        SCROW nDelta = rDeletedRange.aStart.Row() - rDeletedRange.aEnd.Row() - 1;
        rRefRange.aEnd.IncRow(nDelta);
        return true;
    }

    return false;
}

bool expandRange( const sc::RefUpdateContext& rCxt, ScRange& rRefRange, const ScRange& rSelectedRange )
{
    if (!rSelectedRange.Intersects(rRefRange))
        return false;

    if (rCxt.mnColDelta > 0)
    {
        // Insert and shifting right.
        if (rRefRange.aStart.Row() < rSelectedRange.aStart.Row() || rSelectedRange.aEnd.Row() < rRefRange.aEnd.Row())
            // Selected range is only partially overlapping in vertical direction. Bail out.
            return false;

        if (!rCxt.mrDoc.IsExpandRefs() && rSelectedRange.aStart.Col() == rRefRange.aStart.Col())
            // Selected range is at the left end and the edge expansion is turned off.  No expansion.
            return false;

        // Move the last column position to the right.
        SCCOL nDelta = rSelectedRange.aEnd.Col() - rSelectedRange.aStart.Col() + 1;
        rRefRange.aEnd.IncCol(nDelta);
        return true;
    }
    else if (rCxt.mnRowDelta > 0)
    {
        // Insert and shifting down.
        if (rRefRange.aStart.Col() < rSelectedRange.aStart.Col() || rSelectedRange.aEnd.Col() < rRefRange.aEnd.Col())
            // Selected range is only partially overlapping in horizontal direction. Bail out.
            return false;

        if (!rCxt.mrDoc.IsExpandRefs() && rSelectedRange.aStart.Row() == rRefRange.aStart.Row())
            // Selected range is at the top end and the edge expansion is turned off.  No expansion.
            return false;

        // Move the last row position down.
        SCROW nDelta = rSelectedRange.aEnd.Row() - rSelectedRange.aStart.Row() + 1;
        rRefRange.aEnd.IncRow(nDelta);
        return true;
    }
    return false;
}

/**
 * Check if the referenced range is expandable when the selected range is
 * not overlapping the referenced range.
 */
bool expandRangeByEdge( const sc::RefUpdateContext& rCxt, ScRange& rRefRange, const ScRange& rSelectedRange )
{
    if (!rCxt.mrDoc.IsExpandRefs())
        // Edge-expansion is turned off.
        return false;

    if (rCxt.mnColDelta > 0)
    {
        // Insert and shift right.
        if (rRefRange.aStart.Row() < rSelectedRange.aStart.Row() || rSelectedRange.aEnd.Row() < rRefRange.aEnd.Row())
            // Selected range is only partially overlapping in vertical direction. Bail out.
            return false;

        if (rSelectedRange.aStart.Col() - rRefRange.aEnd.Col() != 1)
            // Selected range is not immediately adjacent. Bail out.
            return false;

        // Move the last column position to the right.
        SCCOL nDelta = rSelectedRange.aEnd.Col() - rSelectedRange.aStart.Col() + 1;
        rRefRange.aEnd.IncCol(nDelta);
        return true;
    }
    else if (rCxt.mnRowDelta > 0)
    {
        if (rRefRange.aStart.Col() < rSelectedRange.aStart.Col() || rSelectedRange.aEnd.Col() < rRefRange.aEnd.Col())
            // Selected range is only partially overlapping in horizontal direction. Bail out.
            return false;

        if (rSelectedRange.aStart.Row() - rRefRange.aEnd.Row() != 1)
            // Selected range is not immediately adjacent. Bail out.
            return false;

        // Move the last row position down.
        SCROW nDelta = rSelectedRange.aEnd.Row() - rSelectedRange.aStart.Row() + 1;
        rRefRange.aEnd.IncRow(nDelta);
        return true;
    }

    return false;
}

}

sc::RefUpdateResult ScTokenArray::AdjustReferenceOnShift( const sc::RefUpdateContext& rCxt, const ScAddress& rOldPos )
{
    ScRange aSelectedRange = getSelectedRange(rCxt);

    sc::RefUpdateResult aRes;
    ScAddress aNewPos = rOldPos;
    bool bCellShifted = rCxt.maRange.In(rOldPos);
    if (bCellShifted)
        aNewPos.Move(rCxt.mnColDelta, rCxt.mnRowDelta, rCxt.mnTabDelta);

    FormulaToken** p = pCode;
    FormulaToken** pEnd = p + static_cast<size_t>(nLen);
    for (; p != pEnd; ++p)
    {
        switch ((*p)->GetType())
        {
            case svSingleRef:
            {
                ScToken* pToken = static_cast<ScToken*>(*p);
                ScSingleRefData& rRef = pToken->GetSingleRef();
                ScAddress aAbs = rRef.toAbs(rOldPos);

                if (rCxt.isDeleted() && aSelectedRange.In(aAbs))
                {
                    // This reference is in the deleted region.
                    setRefDeleted(rRef, rCxt);
                    aRes.mbValueChanged = true;
                    break;
                }

                if (rCxt.maRange.In(aAbs))
                {
                    aAbs.Move(rCxt.mnColDelta, rCxt.mnRowDelta, rCxt.mnTabDelta);
                    aRes.mbReferenceModified = true;
                }

                rRef.SetAddress(aAbs, aNewPos);
            }
            break;
            case svDoubleRef:
            {
                ScToken* pToken = static_cast<ScToken*>(*p);
                ScComplexRefData& rRef = pToken->GetDoubleRef();
                ScRange aAbs = rRef.toAbs(rOldPos);

                if (rCxt.isDeleted())
                {
                    if (aSelectedRange.In(aAbs))
                    {
                        // This reference is in the deleted region.
                        setRefDeleted(rRef, rCxt);
                        aRes.mbValueChanged = true;
                        break;
                    }
                    else if (aSelectedRange.Intersects(aAbs))
                    {
                        if (shrinkRange(rCxt, aAbs, aSelectedRange))
                        {
                            // The reference range has been shrunk.
                            rRef.SetRange(aAbs, aNewPos);
                            aRes.mbValueChanged = true;
                            aRes.mbReferenceModified = true;
                            break;
                        }
                    }
                }

                if (rCxt.isInserted())
                {
                    if (expandRange(rCxt, aAbs, aSelectedRange))
                    {
                        // The reference range has been expanded.
                        rRef.SetRange(aAbs, aNewPos);
                        aRes.mbValueChanged = true;
                        aRes.mbReferenceModified = true;
                        break;
                    }

                    if (expandRangeByEdge(rCxt, aAbs, aSelectedRange))
                    {
                        // The reference range has been expanded on the edge.
                        rRef.SetRange(aAbs, aNewPos);
                        aRes.mbValueChanged = true;
                        aRes.mbReferenceModified = true;
                        break;
                    }
                }

                if (rCxt.maRange.In(aAbs))
                {
                    aAbs.Move(rCxt.mnColDelta, rCxt.mnRowDelta, rCxt.mnTabDelta);
                    aRes.mbReferenceModified = true;
                }
                else if (rCxt.maRange.Intersects(aAbs))
                {
                    // Part of the referenced range is being shifted. This
                    // will change the values of the range.
                    aRes.mbValueChanged = true;
                }

                rRef.SetRange(aAbs, aNewPos);
            }
            break;
            default:
                ;
        }
    }

    return aRes;
}

sc::RefUpdateResult ScTokenArray::AdjustReferenceOnMove(
    const sc::RefUpdateContext& rCxt, const ScAddress& rOldPos, const ScAddress& rNewPos )
{
    // When moving, the range is the destination range. We need to use the old
    // range prior to the move for hit analysis.
    ScRange aOldRange = rCxt.maRange;
    aOldRange.Move(-rCxt.mnColDelta, -rCxt.mnRowDelta, -rCxt.mnTabDelta);

    sc::RefUpdateResult aRes;

    FormulaToken** p = pCode;
    FormulaToken** pEnd = p + static_cast<size_t>(nLen);
    for (; p != pEnd; ++p)
    {
        switch ((*p)->GetType())
        {
            case svSingleRef:
            {
                ScToken* pToken = static_cast<ScToken*>(*p);
                ScSingleRefData& rRef = pToken->GetSingleRef();
                ScAddress aAbs = rRef.toAbs(rOldPos);
                if (aOldRange.In(aAbs))
                {
                    aAbs.Move(rCxt.mnColDelta, rCxt.mnRowDelta, rCxt.mnTabDelta);
                    aRes.mbReferenceModified = true;
                }

                rRef.SetAddress(aAbs, rNewPos);
            }
            break;
            case svDoubleRef:
            {
                ScToken* pToken = static_cast<ScToken*>(*p);
                ScComplexRefData& rRef = pToken->GetDoubleRef();
                ScRange aAbs = rRef.toAbs(rOldPos);
                if (aOldRange.In(aAbs))
                {
                    aAbs.Move(rCxt.mnColDelta, rCxt.mnRowDelta, rCxt.mnTabDelta);
                    aRes.mbReferenceModified = true;
                }

                rRef.SetRange(aAbs, rNewPos);
            }
            break;
            default:
                ;
        }
    }

    return aRes;
}

namespace {

bool adjustSingleRefOnDeletedTab( ScSingleRefData& rRef, SCTAB nDelPos, SCTAB nSheets, const ScAddress& rOldPos, const ScAddress& rNewPos )
{
    ScAddress aAbs = rRef.toAbs(rOldPos);
    if (nDelPos <= aAbs.Tab() && aAbs.Tab() < nDelPos + nSheets)
    {
        rRef.SetTabDeleted(true);
        return true;
    }

    if (nDelPos < aAbs.Tab())
    {
        // Reference sheet needs to be adjusted.
        aAbs.IncTab(-1*nSheets);
        rRef.SetAddress(aAbs, rNewPos);
        return true;
    }
    else if (rOldPos.Tab() != rNewPos.Tab())
    {
        // Cell itself has moved.
        rRef.SetAddress(aAbs, rNewPos);
        return true;
    }

    return false;
}

bool adjustSingleRefOnInsertedTab( ScSingleRefData& rRef, SCTAB nInsPos, SCTAB nSheets, const ScAddress& rOldPos, const ScAddress& rNewPos )
{
    ScAddress aAbs = rRef.toAbs(rOldPos);
    if (nInsPos <= aAbs.Tab())
    {
        // Reference sheet needs to be adjusted.
        aAbs.IncTab(nSheets);
        rRef.SetAddress(aAbs, rNewPos);
        return true;
    }
    else if (rOldPos.Tab() != rNewPos.Tab())
    {
        // Cell itself has moved.
        rRef.SetAddress(aAbs, rNewPos);
        return true;
    }

    return false;
}

}

bool ScTokenArray::AdjustReferenceOnDeletedTab( SCTAB nDelPos, SCTAB nSheets, const ScAddress& rOldPos )
{
    bool bRefChanged = false;
    ScAddress aNewPos = rOldPos;
    if (nDelPos < rOldPos.Tab())
        aNewPos.IncTab(-1*nSheets);

    FormulaToken** p = pCode;
    FormulaToken** pEnd = p + static_cast<size_t>(nLen);
    for (; p != pEnd; ++p)
    {
        switch ((*p)->GetType())
        {
            case svSingleRef:
            {
                ScToken* pToken = static_cast<ScToken*>(*p);
                ScSingleRefData& rRef = pToken->GetSingleRef();
                if (adjustSingleRefOnDeletedTab(rRef, nDelPos, nSheets, rOldPos, aNewPos))
                    bRefChanged = true;
            }
            break;
            case svDoubleRef:
            {
                ScToken* pToken = static_cast<ScToken*>(*p);
                ScComplexRefData& rRef = pToken->GetDoubleRef();
                if (adjustSingleRefOnDeletedTab(rRef.Ref1, nDelPos, nSheets, rOldPos, aNewPos))
                    bRefChanged = true;
                if (adjustSingleRefOnDeletedTab(rRef.Ref2, nDelPos, nSheets, rOldPos, aNewPos))
                    bRefChanged = true;
            }
            break;
            default:
                ;
        }
    }
    return bRefChanged;
}

bool ScTokenArray::AdjustReferenceOnInsertedTab( SCTAB nInsPos, SCTAB nSheets, const ScAddress& rOldPos )
{
    bool bRefChanged = false;
    ScAddress aNewPos = rOldPos;
    if (nInsPos <= rOldPos.Tab())
        aNewPos.IncTab(nSheets);

    FormulaToken** p = pCode;
    FormulaToken** pEnd = p + static_cast<size_t>(nLen);
    for (; p != pEnd; ++p)
    {
        switch ((*p)->GetType())
        {
            case svSingleRef:
            {
                ScToken* pToken = static_cast<ScToken*>(*p);
                ScSingleRefData& rRef = pToken->GetSingleRef();
                if (adjustSingleRefOnInsertedTab(rRef, nInsPos, nSheets, rOldPos, aNewPos))
                    bRefChanged = true;
            }
            break;
            case svDoubleRef:
            {
                ScToken* pToken = static_cast<ScToken*>(*p);
                ScComplexRefData& rRef = pToken->GetDoubleRef();
                if (adjustSingleRefOnInsertedTab(rRef.Ref1, nInsPos, nSheets, rOldPos, aNewPos))
                    bRefChanged = true;
                if (adjustSingleRefOnInsertedTab(rRef.Ref2, nInsPos, nSheets, rOldPos, aNewPos))
                    bRefChanged = true;
            }
            break;
            default:
                ;
        }
    }
    return bRefChanged;
}

namespace {

void adjustTabOnMove( ScAddress& rPos, SCTAB nOldPos, SCTAB nNewPos )
{
    // Sheets below the lower bound or above the uppper bound will not change.
    SCTAB nLowerBound = std::min(nOldPos, nNewPos);
    SCTAB nUpperBound = std::max(nOldPos, nNewPos);

    if (rPos.Tab() < nLowerBound || nUpperBound < rPos.Tab())
        // Outside the boundary. Nothing to adjust.
        return;

    if (rPos.Tab() == nOldPos)
    {
        rPos.SetTab(nNewPos);
        return;
    }

    // It's somewhere in between.
    if (nOldPos < nNewPos)
    {
        // Moving a sheet to the right. The rest of the sheets shifts to the left.
        rPos.IncTab(-1);
    }
    else
    {
        // Moving a sheet to the left. The rest of the sheets shifts to the right.
        rPos.IncTab();
    }
}

}

void ScTokenArray::AdjustReferenceOnMovedTab( SCTAB nOldPos, SCTAB nNewPos, const ScAddress& rOldPos )
{
    if (nOldPos == nNewPos)
        return;

    ScAddress aNewPos = rOldPos;
    adjustTabOnMove(aNewPos, nOldPos, nNewPos);

    FormulaToken** p = pCode;
    FormulaToken** pEnd = p + static_cast<size_t>(nLen);
    for (; p != pEnd; ++p)
    {
        switch ((*p)->GetType())
        {
            case svSingleRef:
            {
                ScToken* pToken = static_cast<ScToken*>(*p);
                ScSingleRefData& rRef = pToken->GetSingleRef();
                ScAddress aAbs = rRef.toAbs(rOldPos);
                adjustTabOnMove(aAbs, nOldPos, nNewPos);
                rRef.SetAddress(aAbs, aNewPos);

            }
            break;
            case svDoubleRef:
            {
                ScToken* pToken = static_cast<ScToken*>(*p);
                ScComplexRefData& rRef = pToken->GetDoubleRef();
                ScRange aAbs = rRef.toAbs(rOldPos);
                adjustTabOnMove(aAbs.aStart, nOldPos, nNewPos);
                adjustTabOnMove(aAbs.aEnd, nOldPos, nNewPos);
                rRef.SetRange(aAbs, aNewPos);
            }
            break;
            default:
                ;
        }
    }
}

#if DEBUG_FORMULA_COMPILER
void ScTokenArray::Dump() const
{
    for (sal_uInt16 i = 0; i < nLen; ++i)
    {
        const ScToken* p = dynamic_cast<const ScToken*>(pCode[i]);
        if (!p)
        {
            cout << "-- (non ScToken)" << endl;
            continue;
        }

        p->Dump();
    }
}
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
