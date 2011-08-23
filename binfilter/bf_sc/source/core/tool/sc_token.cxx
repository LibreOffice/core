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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#if STLPORT_VERSION<321
#include <stddef.h>
#else
#include <cstddef>
#endif
#include <string.h>

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#include "compiler.hxx"
#include "rechead.hxx"
namespace binfilter {

/*N*/ struct ImpTokenIterator
/*N*/ {
/*N*/ 	ImpTokenIterator* pNext;
/*N*/ 	ScTokenArray* pArr;
/*N*/ 	short nPC;
/*N*/ 
/*N*/ 	DECL_FIXEDMEMPOOL_NEWDEL( ImpTokenIterator );
/*N*/ };

// ImpTokenIterator wird je Interpreter angelegt, mehrfache auch durch
// SubCode via ScTokenIterator Push/Pop moeglich
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ImpTokenIterator, 32, 16 )//STRIP008 ;

// Align MemPools on 4k boundaries - 64 bytes (4k is a MUST for OS/2)

// Since RawTokens are temporary for the compiler, don't align on 4k and waste memory.
// ScRawToken size is FixMembers + MAXSTRLEN ~= 264
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScRawToken, 8, 4 )//STRIP008 ;
// Some ScDoubleRawToken, FixMembers + sizeof(double) ~= 16
/*N*/ const USHORT nMemPoolDoubleRawToken = 0x0400 / sizeof(ScDoubleRawToken);
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScDoubleRawToken, nMemPoolDoubleRawToken, nMemPoolDoubleRawToken )//STRIP008 ;

// Need a whole bunch of ScSingleRefToken
/*N*/ const USHORT nMemPoolSingleRefToken = (0x4000 - 64) / sizeof(ScSingleRefToken);
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScSingleRefToken, nMemPoolSingleRefToken, nMemPoolSingleRefToken )//STRIP008 ;
// Need a lot of ScDoubleToken
/*N*/ const USHORT nMemPoolDoubleToken = (0x3000 - 64) / sizeof(ScDoubleToken);
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScDoubleToken, nMemPoolDoubleToken, nMemPoolDoubleToken )//STRIP008 ;
// Need a lot of ScByteToken
/*N*/ const USHORT nMemPoolByteToken = (0x3000 - 64) / sizeof(ScByteToken);
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScByteToken, nMemPoolByteToken, nMemPoolByteToken )//STRIP008 ;
// Need quite a lot of ScDoubleRefToken
/*N*/ const USHORT nMemPoolDoubleRefToken = (0x2000 - 64) / sizeof(ScDoubleRefToken);
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScDoubleRefToken, nMemPoolDoubleRefToken, nMemPoolDoubleRefToken )//STRIP008 ;
// Need several ScStringToken
/*N*/ const USHORT nMemPoolStringToken = (0x1000 - 64) / sizeof(ScStringToken);
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScStringToken, nMemPoolStringToken, nMemPoolStringToken )//STRIP008 ;


// --- helpers --------------------------------------------------------------

/*N*/ inline BOOL lcl_IsReference( OpCode eOp, StackVar eType )
/*N*/ {
/*N*/ 	return
/*N*/ 		(eOp == ocPush && (eType == svSingleRef || eType == svDoubleRef))
/*N*/ 		|| (eOp == ocColRowNameAuto && eType == svDoubleRef)
/*N*/ 		|| (eOp == ocColRowName && eType == svSingleRef)
/*N*/ 		|| (eOp == ocMatRef && eType == svSingleRef)
/*N*/ 		;
/*N*/ }


// --- class ScRawToken -----------------------------------------------------

/*N*/ xub_StrLen ScRawToken::GetStrLen( const sal_Unicode* pStr )
/*N*/ {
/*N*/ 	if ( !pStr )
/*N*/ 		return 0;
/*N*/ 	register const sal_Unicode* p = pStr;
/*N*/ 	while ( *p )
/*N*/ 		p++;
/*N*/ 	return p - pStr;
/*N*/ }


/*N*/ void ScRawToken::SetOpCode( OpCode e )
/*N*/ {
/*N*/ 	eOp   = e;
/*N*/ 	if( eOp == ocIf )
/*N*/ 	{
/*N*/ 		eType = svJump; nJump[ 0 ] = 3;	// If, Else, Behind
/*N*/ 	}
/*N*/ 	else if( eOp == ocChose )
/*N*/ 	{
/*N*/ 		eType = svJump; nJump[ 0 ] = MAXJUMPCOUNT+1;
/*N*/ 	}
/*N*/ 	else if( eOp == ocMissing )
/*N*/ 		eType = svMissing;
/*N*/ 	else
/*N*/ 		eType = svByte,	cByte = 0;
/*N*/ 	nRefCnt = 0;
/*N*/ }

/*N*/ void ScRawToken::SetString( const sal_Unicode* pStr )
/*N*/ {
/*N*/ 	eOp   = ocPush;
/*N*/ 	eType = svString;
/*N*/ 	if ( pStr )
/*N*/ 	{
/*N*/ 		xub_StrLen nLen = GetStrLen( pStr ) + 1;
/*N*/ 		if( nLen > MAXSTRLEN )
/*N*/ 			nLen = MAXSTRLEN;
/*N*/ 		memcpy( cStr, pStr, GetStrLenBytes( nLen ) );
/*N*/ 		cStr[ nLen-1 ] = 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		cStr[0] = 0;
/*N*/ 	nRefCnt = 0;
/*N*/ }

/*N*/ void ScRawToken::SetSingleReference( const SingleRefData& rRef )
/*N*/ {
/*N*/ 	eOp       = ocPush;
/*N*/ 	eType     = svSingleRef;
/*N*/ 	aRef.Ref1 =
/*N*/ 	aRef.Ref2 = rRef;
/*N*/ 	nRefCnt   = 0;
/*N*/ }

/*N*/ void ScRawToken::SetDoubleReference( const ComplRefData& rRef )
/*N*/ {
/*N*/ 	eOp   = ocPush;
/*N*/ 	eType = svDoubleRef;
/*N*/ 	aRef  = rRef;
/*N*/ 	nRefCnt = 0;
/*N*/ }

/*N*/ void ScRawToken::SetDouble(double rVal)
/*N*/ {
/*N*/ 	eOp   = ocPush;
/*N*/ 	eType = svDouble;
/*N*/ 	nValue = rVal;
/*N*/ 	nRefCnt = 0;
/*N*/ }

/*N*/ void ScRawToken::SetName( USHORT n )
/*N*/ {
/*N*/ 	eOp    = ocName;
/*N*/ 	eType  = svIndex;
/*N*/ 	nIndex = n;
/*N*/ 	nRefCnt = 0;
/*N*/ }

/*N*/ void ScRawToken::SetExternal( const sal_Unicode* pStr )
/*N*/ {
/*N*/ 	eOp   = ocExternal;
/*N*/ 	eType = svExternal;
/*N*/ 	xub_StrLen nLen = GetStrLen( pStr ) + 1;
/*N*/ 	if( nLen >= MAXSTRLEN )
/*N*/ 		nLen = MAXSTRLEN-1;
/*N*/ 	// Platz fuer Byte-Parameter lassen!
/*N*/ 	memcpy( cStr+1, pStr, GetStrLenBytes( nLen ) );
/*N*/ 	cStr[ nLen+1 ] = 0;
/*N*/ 	nRefCnt = 0;
/*N*/ }

/*N*/ ScRawToken* ScRawToken::Clone() const
/*N*/ {
/*N*/ 	ScRawToken* p;
/*N*/ 	if ( eType == svDouble )
/*N*/ 	{
/*N*/ 		p = (ScRawToken*) new ScDoubleRawToken;
/*N*/ 		p->eOp = eOp;
/*N*/ 		p->eType = eType;
/*N*/ 		p->nValue = nValue;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT n = offsetof( ScRawToken, cByte );
/*N*/ 		switch( eType )
/*N*/ 		{
/*N*/ 			case svByte:		n++; break;
/*N*/ 			case svDouble:		n += sizeof(double); break;
/*N*/ 			case svString:		n += GetStrLenBytes( cStr ) + GetStrLenBytes( 1 ); break;
/*N*/ 			case svSingleRef:
/*N*/ 			case svDoubleRef:	n += sizeof(aRef); break;
/*N*/ 			case svMatrix:		n += sizeof(ScMatrix*); break;
/*N*/ 			case svIndex:		n += sizeof(USHORT); break;
/*N*/ 			case svJump:		n += nJump[ 0 ] * 2 + 2; break;
/*N*/ 			case svExternal:	n += GetStrLenBytes( cStr+1 ) + GetStrLenBytes( 2 ); break;
/*N*/ 			default:			n += *((BYTE*)cStr);    // read in unknown!
/*N*/ 		}
/*N*/ 		p = (ScRawToken*) new BYTE[ n ];
/*N*/ 		memcpy( p, this, n * sizeof(BYTE) );
/*N*/ 	}
/*N*/ 	p->nRefCnt = 0;
/*N*/ 	p->bRaw = FALSE;
/*N*/ 	return p;
/*N*/ }


/*N*/ ScToken* ScRawToken::CreateToken() const
/*N*/ {
/*N*/ 	switch ( GetType() )
/*N*/ 	{
/*N*/ 		case svByte :
/*N*/ 			return new ScByteToken( eOp, cByte );
/*N*/ 		break;
/*N*/ 		case svDouble :
/*N*/ 			return new ScDoubleToken( eOp, nValue );
/*N*/ 		break;
/*N*/ 		case svString :
/*N*/ 			return new ScStringToken( eOp, String( cStr ) );
/*N*/ 		break;
/*N*/ 		case svSingleRef :
/*N*/ 			return new ScSingleRefToken( eOp, aRef.Ref1 );
/*N*/ 		break;
/*N*/ 		case svDoubleRef :
/*N*/ 			return new ScDoubleRefToken( eOp, aRef );
/*N*/ 		break;
/*?*/ 		case svMatrix :
/*?*/ 			return new ScMatrixToken( eOp, pMat );
/*?*/ 		break;
/*N*/ 		case svIndex :
/*N*/ 			return new ScIndexToken( eOp, nIndex );
/*N*/ 		break;
/*N*/ 		case svJump :
/*N*/ 			return new ScJumpToken( eOp, (short*) nJump );
/*N*/ 		break;
/*N*/ 		case svExternal :
/*N*/ 			return new ScExternalToken( eOp, cByte, String( cStr+1 ) );
/*N*/ 		break;
/*N*/         case svFAP :
/*?*/             return new ScFAPToken( eOp, cByte, NULL );
/*?*/         break;
/*?*/ 		case svMissing :
/*?*/ 			return new ScMissingToken( eOp );
/*?*/ 		break;
/*?*/ 		case svErr :
/*?*/ 			return new ScErrToken( eOp );
/*?*/ 		break;
/*?*/ 		default:
/*?*/ 			// read in unknown!
/*?*/ 			return new ScUnknownToken( eOp, GetType(), (BYTE*) cStr );
/*N*/ 	}
/*N*/ }


/*N*/ void ScRawToken::Delete()
/*N*/ {
/*N*/ 	if ( bRaw )
/*?*/ 		delete this;							// FixedMemPool ScRawToken
/*N*/ 	else
/*N*/ 	{	// created per Clone
/*N*/ 		switch ( eType )
/*N*/ 		{
/*N*/ 			case svDouble :
/*N*/ 				delete (ScDoubleRawToken*) this;	// FixedMemPool ScDoubleRawToken
/*N*/ 			break;
/*N*/ 			default:
/*N*/ 				delete [] (BYTE*) this;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


// --- class ScToken --------------------------------------------------------

/*N*/ SingleRefData lcl_ScToken_InitSingleRef()
/*N*/ {
/*N*/ 	SingleRefData aRef;
/*N*/ 	aRef.InitAddress( ScAddress() );
/*N*/ 	return aRef;
/*N*/ }

/*N*/ ComplRefData lcl_ScToken_InitDoubleRef()
/*N*/ {
/*N*/ 	ComplRefData aRef;
/*N*/ 	aRef.Ref1 = lcl_ScToken_InitSingleRef();
/*N*/ 	aRef.Ref2 = aRef.Ref1;
/*N*/ 	return aRef;
/*N*/ }

/*N*/ SingleRefData	ScToken::aDummySingleRef = lcl_ScToken_InitSingleRef();
/*N*/ ComplRefData	ScToken::aDummyDoubleRef = lcl_ScToken_InitDoubleRef();
/*N*/ String			ScToken::aDummyString;


/*N*/ ScToken::~ScToken()
/*N*/ {
/*N*/ }


/*N*/ BYTE ScToken::GetParamCount() const
/*N*/ {
/*N*/ 	if ( eOp <= ocEndDiv && eOp != ocExternal && eOp != ocMacro &&
/*N*/             eOp != ocIf && eOp != ocChose && eOp != ocPercentSign )
/*N*/ 		return 0;		// parameters and specials
/*N*/ 						// ocIf and ocChose not for FAP, have cByte then
/*N*/ //2do: BOOL parameter whether FAP or not?
/*N*/ 	else if ( GetByte() )
/*N*/ 		return GetByte();	// all functions, also ocExternal and ocMacro
/*N*/ 	else if ( ocEndDiv < eOp && eOp <= ocEndBinOp )
/*N*/ 		return 2;			// binary
/*N*/     else if ( (ocEndBinOp < eOp && eOp <= ocEndUnOp) || eOp == ocPercentSign )
/*N*/ 		return 1;			// unary
/*N*/ 	else if ( ocEndUnOp < eOp && eOp <= ocEndNoPar )
/*N*/ 		return 0;			// no parameter
/*N*/ 	else if ( ocEndNoPar < eOp && eOp <= ocEnd1Par )
/*N*/ 		return 1;			// one parameter
/*N*/ 	else
/*N*/ 		return 0;			// all the rest, no Parameter, or
/*N*/ 							// if so then it should be in cByte
/*N*/ }


/*N*/ ScToken* ScToken::Clone() const
/*N*/ {
/*N*/ 	switch ( GetType() )
/*N*/ 	{
/*N*/ 		case svByte :
/*N*/ 			return new ScByteToken( *static_cast<const ScByteToken*>(this) );
/*N*/ 		break;
/*N*/ 		case svDouble :
/*N*/ 			return new ScDoubleToken( *static_cast<const ScDoubleToken*>(this) );
/*N*/ 		break;
/*N*/ 		case svString :
/*N*/ 			return new ScStringToken( *static_cast<const ScStringToken*>(this) );
/*N*/ 		break;
/*N*/ 		case svSingleRef :
/*N*/ 			return new ScSingleRefToken( *static_cast<const ScSingleRefToken*>(this) );
/*N*/ 		break;
/*N*/ 		case svDoubleRef :
/*N*/ 			return new ScDoubleRefToken( *static_cast<const ScDoubleRefToken*>(this) );
/*N*/ 		break;
/*?*/ 		case svMatrix :
/*?*/ 			return new ScMatrixToken( *static_cast<const ScMatrixToken*>(this) );
/*?*/ 		break;
/*?*/ 		case svIndex :
/*?*/ 			return new ScIndexToken( *static_cast<const ScIndexToken*>(this) );
/*?*/ 		break;
/*N*/ 		case svJump :
/*N*/ 			return new ScJumpToken( *static_cast<const ScJumpToken*>(this) );
/*N*/ 		break;
/*?*/ 		case svExternal :
/*?*/ 			return new ScExternalToken( *static_cast<const ScExternalToken*>(this) );
/*?*/ 		break;
/*?*/         case svFAP :
/*?*/             return new ScFAPToken( *static_cast<const ScFAPToken*>(this) );
/*?*/         break;
/*?*/ 		case svMissing :
/*?*/ 			return new ScMissingToken( *static_cast<const ScMissingToken*>(this) );
/*?*/ 		break;
/*?*/ 		case svErr :
/*?*/ 			return new ScErrToken( *static_cast<const ScErrToken*>(this) );
/*?*/ 		break;
/*?*/ 		default:
/*?*/ 			// read in unknown!
/*?*/ 			return new ScUnknownToken( *static_cast<const ScUnknownToken*>(this) );
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScToken::operator==( const ScToken& rToken ) const
/*N*/ {
/*N*/ 	// don't compare reference count!
/*N*/ 	return eOp == rToken.eOp && eType == rToken.eType;
/*N*/ }


//	TextEqual: if same formula entered (for optimization in sort)
/*N*/ BOOL ScToken::TextEqual( const ScToken& rToken ) const
/*N*/ {
/*N*/ 	if ( eType == svSingleRef || eType == svDoubleRef )
/*N*/ 	{
/*N*/ 		//	in relative Refs only compare relative parts
/*N*/ 
/*N*/ 		if ( eOp != rToken.eOp || eType != rToken.eType )
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 		ComplRefData aTemp1;
/*N*/ 		if ( eType == svSingleRef )
/*N*/ 		{
/*N*/ 			aTemp1.Ref1 = GetSingleRef();
/*N*/ 			aTemp1.Ref2 = aTemp1.Ref1;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			aTemp1 = GetDoubleRef();
/*N*/ 
/*N*/ 		ComplRefData aTemp2;
/*N*/ 		if ( rToken.eType == svSingleRef )
/*N*/ 		{
/*N*/ 			aTemp2.Ref1 = rToken.GetSingleRef();
/*N*/ 			aTemp2.Ref2 = aTemp2.Ref1;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			aTemp2 = rToken.GetDoubleRef();
/*N*/ 
/*N*/ 		ScAddress aPos;
/*N*/ 		aTemp1.SmartRelAbs(aPos);
/*N*/ 		aTemp2.SmartRelAbs(aPos);
/*N*/ 
/*N*/ 		//	memcmp doesn't work because of the alignment byte after bFlags.
/*N*/ 		//	After SmartRelAbs only absolute parts have to be compared.
/*N*/ 		return aTemp1.Ref1.nCol   == aTemp2.Ref1.nCol   &&
/*N*/ 			   aTemp1.Ref1.nRow   == aTemp2.Ref1.nRow   &&
/*N*/ 			   aTemp1.Ref1.nTab   == aTemp2.Ref1.nTab   &&
/*N*/ 			   aTemp1.Ref1.bFlags == aTemp2.Ref1.bFlags &&
/*N*/ 			   aTemp1.Ref2.nCol   == aTemp2.Ref2.nCol   &&
/*N*/ 			   aTemp1.Ref2.nRow   == aTemp2.Ref2.nRow   &&
/*N*/ 			   aTemp1.Ref2.nTab   == aTemp2.Ref2.nTab   &&
/*N*/ 			   aTemp1.Ref2.bFlags == aTemp2.Ref2.bFlags;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return *this == rToken;		// else normal operator==
/*N*/ }

// --- virtual dummy methods -------------------------------------------------

/*N*/ BYTE ScToken::GetByte() const
/*N*/ {
/*N*/ 	// ok to be called for any derived class
/*N*/ 	return 0;
/*N*/ }

/*N*/ void ScToken::SetByte( BYTE n )
/*N*/ {
/*N*/  DBG_ERRORFILE( "ScToken::SetByte: virtual dummy called" );
/*N*/ }

/*N*/ double ScToken::GetDouble() const
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::GetDouble: virtual dummy called" );
/*N*/ 	return 0.0;
/*N*/ }

/*N*/ const String& ScToken::GetString() const
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::GetString: virtual dummy called" );
/*N*/ 	return aDummyString;
/*N*/ }

/*N*/ const SingleRefData& ScToken::GetSingleRef() const
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::GetSingleRef: virtual dummy called" );
/*N*/ 	return aDummySingleRef;
/*N*/ }

/*N*/ SingleRefData& ScToken::GetSingleRef()
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::GetSingleRef: virtual dummy called" );
/*N*/ 	return aDummySingleRef;
/*N*/ }

/*N*/ const ComplRefData& ScToken::GetDoubleRef() const
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::GetDoubleRef: virtual dummy called" );
/*N*/ 	return aDummyDoubleRef;
/*N*/ }

/*N*/ ComplRefData& ScToken::GetDoubleRef()
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::GetDoubleRef: virtual dummy called" );
/*N*/ 	return aDummyDoubleRef;
/*N*/ }

/*N*/  const SingleRefData& ScToken::GetSingleRef2() const
/*N*/  {
/*N*/      DBG_ERRORFILE( "ScToken::GetSingleRef2: virtual dummy called" );
/*N*/  	return aDummySingleRef;
/*N*/  }

/*N*/  SingleRefData& ScToken::GetSingleRef2()
/*N*/  {
/*N*/      DBG_ERRORFILE( "ScToken::GetSingleRef2: virtual dummy called" );
/*N*/  	return aDummySingleRef;
/*N*/  }

/*N*/ void ScToken::CalcAbsIfRel( const ScAddress& rPos )
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::CalcAbsIfRel: virtual dummy called" );
/*N*/ }

/*N*/ void ScToken::CalcRelFromAbs( const ScAddress& rPos )
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::CalcRelFromAbs: virtual dummy called" );
/*N*/ }

/*N*/ ScMatrix* ScToken::GetMatrix() const
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::GetMatrix: virtual dummy called" );
/*N*/ 	return NULL;
/*N*/ }

/*N*/ USHORT ScToken::GetIndex() const
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::GetIndex: virtual dummy called" );
/*N*/ 	return 0;
/*N*/ }

/*N*/  void ScToken::SetIndex( USHORT n )
/*N*/  {
/*N*/  	DBG_ERRORFILE( "ScToken::SetIndex: virtual dummy called" );
/*N*/  }

/*N*/ short* ScToken::GetJump() const
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::GetJump: virtual dummy called" );
/*N*/ 	return NULL;
/*N*/ }

/*N*/ const String& ScToken::GetExternal() const
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::GetExternal: virtual dummy called" );
/*N*/ 	return aDummyString;
/*N*/ }

/*N*/ BYTE* ScToken::GetUnknown() const
/*N*/ {
/*N*/ 	DBG_ERRORFILE( "ScToken::GetUnknown: virtual dummy called" );
/*N*/ 	return NULL;
/*N*/ }

/*N*/  ScToken* ScToken::GetFAPOrigToken() const
/*N*/  {
/*N*/      DBG_ERRORFILE( "ScToken::GetFAPOrigToken: virtual dummy called" );
/*N*/      return NULL;
/*N*/  }


// real implementations of virtual functions

/*N*/ BYTE ScByteToken::GetByte() const						{ return nByte; }
/*N*/ void ScByteToken::SetByte( BYTE n )						{ nByte = n; }
/*N*/ BOOL ScByteToken::operator==( const ScToken& r ) const
/*N*/ {
/*N*/ 	return ScToken::operator==( r ) && nByte == r.GetByte();
/*N*/ }


/*N*/  ScToken* ScFAPToken::GetFAPOrigToken() const            { return pOrigToken; }
/*N*/ BOOL ScFAPToken::operator==( const ScToken& r ) const
/*N*/ {
/*?*/     DBG_BF_ASSERT(0, "STRIP"); return FALSE;/*N*/  return ScToken::operator==( r ) && pOrigToken == r.GetFAPOrigToken();
/*N*/ }


/*N*/ double ScDoubleToken::GetDouble() const					{ return fDouble; }
/*N*/ BOOL ScDoubleToken::operator==( const ScToken& r ) const
/*N*/ {
/*N*/ 	return ScToken::operator==( r ) && fDouble == r.GetDouble();
/*N*/ }


/*N*/ const String& ScStringToken::GetString() const			{ return aString; }
/*N*/  BOOL ScStringToken::operator==( const ScToken& r ) const
/*N*/  {
/*N*/  	return ScToken::operator==( r ) && aString == r.GetString();
/*N*/  }


/*N*/ const SingleRefData&	ScSingleRefToken::GetSingleRef() const	{ return aSingleRef; }
/*N*/ SingleRefData&			ScSingleRefToken::GetSingleRef()		{ return aSingleRef; }
/*N*/ void					ScSingleRefToken::CalcAbsIfRel( const ScAddress& rPos )
/*N*/ 							{ aSingleRef.CalcAbsIfRel( rPos ); }
/*N*/ void					ScSingleRefToken::CalcRelFromAbs( const ScAddress& rPos )
/*N*/ 							{ aSingleRef.CalcRelFromAbs( rPos ); }
/*N*/  BOOL ScSingleRefToken::operator==( const ScToken& r ) const
/*N*/  {
/*N*/  	return ScToken::operator==( r ) && aSingleRef == r.GetSingleRef();
/*N*/  }


/*N*/ const SingleRefData&	ScDoubleRefToken::GetSingleRef() const	{ return aDoubleRef.Ref1; }
/*N*/ SingleRefData&			ScDoubleRefToken::GetSingleRef()		{ return aDoubleRef.Ref1; }
/*N*/ const ComplRefData&		ScDoubleRefToken::GetDoubleRef() const	{ return aDoubleRef; }
/*N*/ ComplRefData&			ScDoubleRefToken::GetDoubleRef()		{ return aDoubleRef; }
/*N*/  const SingleRefData&    ScDoubleRefToken::GetSingleRef2() const { return aDoubleRef.Ref2; }
/*N*/  SingleRefData&          ScDoubleRefToken::GetSingleRef2()       { return aDoubleRef.Ref2; }
/*N*/ void					ScDoubleRefToken::CalcAbsIfRel( const ScAddress& rPos )
/*N*/ 							{ aDoubleRef.CalcAbsIfRel( rPos ); }
/*N*/  void					ScDoubleRefToken::CalcRelFromAbs( const ScAddress& rPos )
/*N*/  							{ aDoubleRef.CalcRelFromAbs( rPos ); }
/*N*/  BOOL ScDoubleRefToken::operator==( const ScToken& r ) const
/*N*/  {
/*N*/  	return ScToken::operator==( r ) && aDoubleRef == r.GetDoubleRef();
/*N*/  }


/*N*/ ScMatrix* ScMatrixToken::GetMatrix() const				{ return pMatrix; }
/*N*/  BOOL ScMatrixToken::operator==( const ScToken& r ) const
/*N*/  {
/*N*/  	return ScToken::operator==( r ) && pMatrix == r.GetMatrix();
/*N*/  }


/*N*/ USHORT	ScIndexToken::GetIndex() const					{ return nIndex; }
/*N*/  void	ScIndexToken::SetIndex( USHORT n )				{ nIndex = n; }
/*N*/  BOOL ScIndexToken::operator==( const ScToken& r ) const
/*N*/  {
/*N*/  	return ScToken::operator==( r ) && nIndex == r.GetIndex();
/*N*/  }


/*N*/ short* ScJumpToken::GetJump() const						{ return pJump; }
/*N*/  BOOL ScJumpToken::operator==( const ScToken& r ) const
/*N*/  {
/*N*/  	return ScToken::operator==( r ) && pJump[0] == r.GetJump()[0] &&
/*N*/  		memcmp( pJump+1, r.GetJump()+1, pJump[0] * sizeof(short) ) == 0;
/*N*/  }
/*N*/ ScJumpToken::~ScJumpToken()
/*N*/ {
/*N*/ 	delete [] pJump;
/*N*/ }


/*N*/ const String&	ScExternalToken::GetExternal() const	{ return aExternal; }
/*N*/ BYTE			ScExternalToken::GetByte() const		{ return nByte; }
/*N*/ void			ScExternalToken::SetByte( BYTE n )		{ nByte = n; }
/*N*/  BOOL ScExternalToken::operator==( const ScToken& r ) const
/*N*/  {
/*N*/  	return ScToken::operator==( r ) && nByte == r.GetByte() &&
/*N*/  		aExternal == r.GetExternal();
/*N*/  }


/*N*/  double			ScMissingToken::GetDouble() const		{ return 0.0; }
/*N*/  const String&	ScMissingToken::GetString() const		{ return aDummyString; }
/*N*/  BOOL ScMissingToken::operator==( const ScToken& r ) const
/*N*/  {
/*N*/  	return ScToken::operator==( r );
/*N*/  }


/*N*/  BOOL ScErrToken::operator==( const ScToken& r ) const
/*N*/  {
/*N*/  	return ScToken::operator==( r );
/*N*/  }


/*N*/  BYTE* ScUnknownToken::GetUnknown() const				{ return pUnknown; }
/*N*/  BOOL ScUnknownToken::operator==( const ScToken& r ) const
/*N*/  {
/*N*/  	return ScToken::operator==( r ) && pUnknown[0] == r.GetUnknown()[0] &&
/*N*/  		memcmp( pUnknown+1, r.GetUnknown()+1, pUnknown[0] * sizeof(BYTE) ) == 0;
/*N*/  }
/*N*/  ScUnknownToken::~ScUnknownToken()
/*N*/  {
/*N*/  	delete [] pUnknown;
/*N*/  }


//////////////////////////////////////////////////////////////////////////

/*N*/ ScToken* ScTokenArray::GetNextReference()
/*N*/ {
/*N*/ 	while( nIndex < nLen )
/*N*/ 	{
/*N*/ 		ScToken* t = pCode[ nIndex++ ];
/*N*/ 		switch( t->GetType() )
/*N*/ 		{
/*N*/ 			case svSingleRef:
/*N*/ 			case svDoubleRef:
/*N*/ 				return t;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*N*/ ScToken* ScTokenArray::GetNextReferenceRPN()
/*N*/ {
/*N*/ 	while( nIndex < nRPN )
/*N*/ 	{
/*N*/ 		ScToken* t = pRPN[ nIndex++ ];
/*N*/ 		switch( t->GetType() )
/*N*/ 		{
/*N*/ 			case svSingleRef:
/*N*/ 			case svDoubleRef:
/*N*/ 				return t;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*N*/ ScToken* ScTokenArray::GetNextReferenceOrName()
/*N*/ {
/*N*/ 	for( ScToken* t = Next(); t; t = Next() )
/*N*/ 	{
/*N*/ 		switch( t->GetType() )
/*N*/ 		{
/*N*/ 			case svSingleRef:
/*N*/ 			case svDoubleRef:
/*N*/ 			case svIndex:
/*N*/ 				return t;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*N*/ ScToken* ScTokenArray::GetNextOpCodeRPN( OpCode eOp )
/*N*/ {
/*N*/ 	while( nIndex < nRPN )
/*N*/ 	{
/*?*/ 		ScToken* t = pRPN[ nIndex++ ];
/*?*/ 		if ( t->GetOpCode() == eOp )
/*?*/ 			return t;
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*N*/ ScToken* ScTokenArray::Next()
/*N*/ {
/*N*/ 	if( pCode && nIndex < nLen )
/*N*/ 		return pCode[ nIndex++ ];
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ ScToken* ScTokenArray::NextRPN()
/*N*/ {
/*N*/ 	if( pRPN && nIndex < nRPN )
/*N*/ 		return pRPN[ nIndex++ ];
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ void ScTokenArray::DelRPN()
/*N*/ {
/*N*/ 	if( nRPN )
/*N*/ 	{
/*N*/ 		ScToken** p = pRPN;
/*N*/ 		for( USHORT i = 0; i < nRPN; i++ )
/*N*/ 		{
/*N*/ 			(*p++)->DecRef();
/*N*/ 		}
/*N*/ 		delete [] pRPN;
/*N*/ 	}
/*N*/ 	pRPN = NULL;
/*N*/ 	nRPN = nIndex = 0;
/*N*/ }

/*N*/ ScToken* ScTokenArray::PeekNext()
/*N*/ {
/*N*/ 	if( pCode && nIndex < nLen )
/*N*/ 		return pCode[ nIndex ];
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ ScToken* ScTokenArray::PeekNextNoSpaces()
/*N*/ {
/*N*/ 	if( pCode && nIndex < nLen )
/*N*/ 	{
/*N*/ 		USHORT j = nIndex;
/*N*/ 		while ( pCode[j]->GetOpCode() == ocSpaces && j < nLen )
/*N*/ 			j++;
/*N*/ 		if ( j < nLen )
/*N*/ 			return pCode[ j ];
/*N*/ 		else
/*N*/ 			return NULL;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ ScToken* ScTokenArray::PeekPrevNoSpaces()
/*N*/ {
/*N*/ 	if( pCode && nIndex > 1 )
/*N*/ 	{
/*?*/ 		USHORT j = nIndex - 2;
/*?*/ 		while ( pCode[j]->GetOpCode() == ocSpaces && j > 0 )
/*?*/ 			j--;
/*?*/ 		if ( j > 0 || pCode[j]->GetOpCode() != ocSpaces )
/*?*/ 			return pCode[ j ];
/*?*/ 		else
/*?*/ 			return NULL;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ BOOL ScTokenArray::HasOpCodeRPN( OpCode eOp ) const
/*N*/ {
/*N*/ 	for ( USHORT j=0; j < nRPN; j++ )
/*N*/ 	{
/*N*/ 		if ( pRPN[j]->GetOpCode() == eOp )
/*N*/ 			return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/  BOOL ScTokenArray::HasNameOrColRowName() const
/*N*/  {
/*N*/  	for ( USHORT j=0; j < nLen; j++ )
/*N*/  	{
/*N*/          if( pCode[j]->GetType() == svIndex || pCode[j]->GetOpCode() == ocColRowName )
/*N*/  			return TRUE;
/*N*/  	}
/*N*/  	return FALSE;
/*N*/  }

/*N*/ BOOL ScTokenArray::ImplGetReference( ScRange& rRange, BOOL bValidOnly ) const
/*N*/ {
/*N*/     BOOL bIs = FALSE;
/*N*/     if ( pCode && nLen == 1 )
/*N*/     {
/*N*/         const ScToken* pToken = pCode[0];
/*N*/         if ( pToken )
/*N*/         {
/*N*/             if ( pToken->GetType() == svSingleRef )
/*N*/             {
/*N*/                 const SingleRefData& rRef = ((const ScSingleRefToken*)pToken)->GetSingleRef();
/*N*/                 rRange.aStart = rRange.aEnd = ScAddress( rRef.nCol, rRef.nRow, rRef.nTab );
/*N*/                 bIs = !bValidOnly || !rRef.IsDeleted();
/*N*/             }
/*N*/             else if ( pToken->GetType() == svDoubleRef )
/*N*/             {
/*N*/                 const ComplRefData& rCompl = ((const ScDoubleRefToken*)pToken)->GetDoubleRef();
/*N*/                 const SingleRefData& rRef1 = rCompl.Ref1;
/*N*/                 const SingleRefData& rRef2 = rCompl.Ref2;
/*N*/                 rRange.aStart = ScAddress( rRef1.nCol, rRef1.nRow, rRef1.nTab );
/*N*/                 rRange.aEnd   = ScAddress( rRef2.nCol, rRef2.nRow, rRef2.nTab );
/*N*/                 bIs = !bValidOnly || (!rRef1.IsDeleted() && !rRef2.IsDeleted());
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/     return bIs;
/*N*/ }

/*N*/ BOOL ScTokenArray::IsReference( ScRange& rRange ) const
/*N*/ {
/*N*/     return ImplGetReference( rRange, FALSE );
/*N*/ }

/*N*/  inline void lcl_GetAddress( ScAddress& rAddress, const ScToken& rToken )
/*N*/  {
/*N*/  	if ( rToken.GetType() == svSingleRef )
/*N*/  	{
/*N*/  		const SingleRefData& rRef = ((const ScSingleRefToken&)rToken).GetSingleRef();
/*N*/  		rAddress.Set( rRef.nCol, rRef.nRow, rRef.nTab );
/*N*/  	}
/*N*/  }

/*N*/ void ScTokenArray::Load30( SvStream& rStream, const ScAddress& rPos )
/*N*/ {
/*N*/ 	Clear();
/*N*/ 	ScToken* pToks[ MAXCODE ];
/*N*/ 	ScRawToken t;
/*N*/ 	for( nLen = 0; nLen < MAXCODE; nLen++ )
/*N*/ 	{
/*N*/ 		t.Load30( rStream );
/*N*/ 		if( t.GetOpCode() == ocStop )
/*N*/ 			break;
/*N*/ 		else if( t.GetOpCode() == ocPush
/*N*/ 		  && ( t.GetType() == svSingleRef || t.GetType() == svDoubleRef ) )
/*N*/ 		{
/*N*/ 			nRefs++;
/*N*/ 			t.aRef.CalcRelFromAbs( rPos );
/*N*/ 		}
/*N*/ 		ScToken* p = pToks[ nLen ] = t.CreateToken();
/*N*/ 		p->IncRef();
/*N*/ 	}
/*N*/ 	pCode = new ScToken*[ nLen ];
/*N*/ 	memcpy( pCode, pToks, nLen * sizeof( ScToken* ) );
/*N*/ }

/*N*/ void ScTokenArray::Load( SvStream& rStream, USHORT nVer, const ScAddress& rPos )
/*N*/ {
/*N*/ 	Clear();
/*N*/ 	// 0x10 - nRefs
/*N*/ 	// 0x20 - nError
/*N*/ 	// 0x40 - TokenArray
/*N*/ 	// 0x80 - CodeArray
/*N*/ 	BYTE cData;
/*N*/ 	rStream >> cData;
/*N*/ 	if( cData & 0x0F )
/*?*/ 		rStream.SeekRel( cData & 0x0F );
/*N*/ 	if ( nVer < SC_RECALC_MODE_BITS )
/*N*/ 	{
/*N*/ 			 BYTE cMode;
/*N*/ 			rStream >> cMode;
/*N*/ 			ImportRecalcMode40( (ScRecalcMode40) cMode );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rStream >> nMode;
/*N*/ 	if( cData & 0x10 )
/*N*/ 		rStream >> nRefs;
/*N*/ 	if( cData & 0x20 )
/*N*/ 		rStream >> nError;
/*N*/ 	ScToken* pToks[ MAXCODE ];
/*N*/ 	ScToken** pp = pToks;
/*N*/ 	ScRawToken t;
/*N*/ 	if( cData & 0x40 )
/*N*/ 	{
/*N*/ 		rStream >> nLen;
/*N*/ 		for( USHORT i = 0; i < nLen; i++ )
/*N*/ 		{
/*N*/ 			t.Load( rStream, nVer );
/*N*/ 			if ( t.GetType() == svSingleRef || t.GetType() == svDoubleRef )
/*N*/ 				t.aRef.CalcRelFromAbs( rPos );
/*N*/ 				// gespeichert wurde und wird immer absolut
/*N*/ 			*pp = t.CreateToken();
/*N*/ 			(*pp++)->IncRef();
/*N*/ 		}
/*N*/ 		pCode = new ScToken*[ nLen ];
/*N*/ 		memcpy( pCode, pToks, nLen * sizeof( ScToken* ) );
/*N*/ 	}
/*N*/ 	pp = pToks;
/*N*/ 	if( cData & 0x80 )
/*N*/ 	{
/*N*/ 		rStream >> nRPN;
/*N*/ 		for( USHORT i = 0; i < nRPN; i++, pp++ )
/*N*/ 		{
/*N*/ 			BYTE b1, b2 = 0;
/*N*/ 			UINT16 nIdx;
/*N*/ 			rStream >> b1;
/*N*/ 			// 0xFF 	 - Token folgt
/*N*/ 			// 0x40-0x7F - untere 6 Bits, 1 Byte mit 8 weiteren Bits
/*N*/ 			// 0x00-0x3F - Index
/*N*/ 			if( b1 == 0xFF )
/*N*/ 			{
/*N*/ 				t.Load( rStream, nVer );
/*N*/ 				if ( t.GetType() == svSingleRef || t.GetType() == svDoubleRef )
/*N*/ 					t.aRef.CalcRelFromAbs( rPos );
/*N*/ 					// gespeichert wurde und wird immer absolut
/*N*/ 				*pp = t.CreateToken();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if( b1 & 0x40 )
/*N*/ 				{
/*N*/ 					rStream >> b2;
/*N*/ 					nIdx = ( b1 & 0x3F ) | ( b2 << 6 );
/*N*/ 				}
/*N*/ 				else
/*N*/ 					nIdx = b1;
/*N*/ 				*pp = pCode[ nIdx ];
/*N*/ 			}
/*N*/ 			(*pp)->IncRef();
/*N*/ 			// #73616# CONVERT function recalculated on each load
/*N*/ 			if ( nVer < SC_CONVERT_RECALC_ON_LOAD && (*pp)->GetOpCode() == ocConvert )
/*N*/					AddRecalcMode( RECALCMODE_ONLOAD );
/*N*/ 		}
/*N*/ 		pRPN = new ScToken*[ nRPN ];
/*N*/ 		memcpy( pRPN, pToks, nRPN * sizeof( ScToken* ) );
/*N*/ 		// Aeltere Versionen: kein UPN-Array laden
/*N*/ 		if( nVer < SC_NEWIF )
/*?*/ 			DelRPN();
/*N*/ 	}
/*N*/ }

/*N*/ void ScTokenArray::Store( SvStream& rStream, const ScAddress& rPos ) const
/*N*/ {
/*N*/ 	// 0x10 - nRefs
/*N*/ 	// 0x20 - nError
/*N*/ 	// 0x40 - TokenArray
/*N*/ 	// 0x80 - CodeArray
/*N*/ 	BYTE cFlags = 0;
/*N*/ 	if( nRefs )
/*N*/ 		cFlags |= 0x10;
/*N*/ 	if( nError )
/*N*/ 		cFlags |= 0x20;
/*N*/ 	if( nLen )
/*N*/ 		cFlags |= 0x40;
/*N*/ 	if( nRPN )
/*N*/ 		cFlags |= 0x80;
/*N*/ 	rStream << cFlags;
/*N*/ 	// Hier ggf. Zusatzdaten!
/*N*/ 	if ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 )
/*N*/		rStream << (BYTE) ExportRecalcMode40();
/*N*/ 	else
/*N*/ 		rStream << (BYTE) nMode;
/*N*/ 	if( cFlags & 0x10 )
/*N*/ 		rStream << (INT16) nRefs;
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 		rStream << (UINT16) nError;
/*N*/ 	if( cFlags & 0x40 )
/*N*/ 	{
/*N*/ 		rStream << nLen;
/*N*/ 		ScToken** p = pCode;
/*N*/ 		for( USHORT i = 0; i < nLen; i++, p++ )
/*N*/ 		{
/*N*/ 			// gespeichert wurde und wird immer absolut
/*N*/ 			switch ( (*p)->GetType() )
/*N*/ 			{
/*N*/ 				case svSingleRef :
/*N*/ 						(*p)->GetSingleRef().CalcAbsIfRel( rPos );
/*N*/ 					break;
/*N*/ 				case svDoubleRef :
/*N*/ 						(*p)->GetDoubleRef().CalcAbsIfRel( rPos );
/*N*/ 					break;
/*N*/ 			}
/*N*/ 			(*p)->Store( rStream );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( cFlags & 0x80 )
/*N*/ 	{
/*N*/ 		rStream << nRPN;
/*N*/ 		ScToken** p = pRPN;
/*N*/ 		for( USHORT i = 0; i < nRPN; i++, p++ )
/*N*/ 		{
/*N*/ 			ScToken* t = *p;
/*N*/ 			USHORT nIdx = 0xFFFF;
/*N*/ 			if( t->GetRef() > 1 )
/*N*/ 			{
/*N*/ 				ScToken** p2 = pCode;
/*N*/ 				for( USHORT j = 0; j < nLen; j++, p2++ )
/*N*/ 				{
/*N*/ 					if( *p2 == t )
/*N*/ 					{
/*N*/ 						nIdx = j; break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			// 0xFF 	 - Token folgt
/*N*/ 			// 0x40-0x7F - untere 6 Bits, 1 Byte mit 8 weiteren Bits
/*N*/ 			// 0x00-0x3F - Index
/*N*/ 			if( nIdx == 0xFFFF )
/*N*/ 			{
/*N*/ 				// gespeichert wurde und wird immer absolut
/*N*/ 				switch ( t->GetType() )
/*N*/ 				{
/*N*/ 					case svSingleRef :
/*N*/ 							t->GetSingleRef().CalcAbsIfRel( rPos );
/*N*/ 						break;
/*?*/ 					case svDoubleRef :
/*?*/ 							t->GetDoubleRef().CalcAbsIfRel( rPos );
/*?*/ 						break;
/*N*/ 				}
/*N*/ 				rStream << (BYTE) 0xFF;
/*N*/ 				t->Store( rStream );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if( nIdx < 0x40 )
/*N*/ 					rStream << (BYTE) nIdx;
/*N*/ 				else
/*N*/ 					rStream << (BYTE) ( ( nIdx & 0x3F ) | 0x40 )
/*N*/ 							<< (BYTE) ( nIdx >> 6 );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////

/*N*/ ScTokenArray::ScTokenArray()
/*N*/ {
/*N*/ 	pCode = NULL; pRPN = NULL;
/*N*/ 	nError = nLen = nIndex = nRPN = nRefs = 0;
/*M*/     bReplacedSharedFormula = FALSE;
/*N*/ 	ClearRecalcMode();
/*N*/ }

/*N*/ ScTokenArray::ScTokenArray( const ScTokenArray& rArr )
/*N*/ {
/*N*/ 	Assign( rArr );
/*N*/ }

/*N*/ ScTokenArray::~ScTokenArray()
/*N*/ {
/*N*/ 	Clear();
/*N*/ }

/*N*/ void ScTokenArray::Assign( const ScTokenArray& r )
/*N*/ {
/*N*/ 	nLen   = r.nLen;
/*N*/ 	nRPN   = r.nRPN;
/*N*/ 	nIndex = r.nIndex;
/*N*/ 	nError = r.nError;
/*N*/ 	nRefs  = r.nRefs;
/*N*/ 	nMode  = r.nMode;
/*M*/     bReplacedSharedFormula = FALSE;
/*N*/ 	pCode  = NULL;
/*N*/ 	pRPN   = NULL;
/*N*/ 	ScToken** pp;
/*N*/ 	if( nLen )
/*N*/ 	{
/*N*/ 		pp = pCode = new ScToken*[ nLen ];
/*N*/ 		memcpy( pp, r.pCode, nLen * sizeof( ScToken* ) );
/*N*/ 		for( USHORT i = 0; i < nLen; i++ )
/*N*/ 			(*pp++)->IncRef();
/*N*/ 	}
/*N*/ 	if( nRPN )
/*N*/ 	{
/*?*/ 		pp = pRPN = new ScToken*[ nRPN ];
/*?*/ 		memcpy( pp, r.pRPN, nRPN * sizeof( ScToken* ) );
/*?*/ 		for( USHORT i = 0; i < nRPN; i++ )
/*?*/ 			(*pp++)->IncRef();
/*N*/ 	}
/*N*/ }

/*N*/  ScTokenArray& ScTokenArray::operator=( const ScTokenArray& rArr )
/*N*/  {
/*N*/  	Clear();
/*N*/  	Assign( rArr );
/*N*/  	return *this;
/*N*/  }

/*N*/ ScTokenArray* ScTokenArray::Clone() const
/*N*/ {
/*N*/ 	ScTokenArray* p = new ScTokenArray;
/*N*/ 	p->nLen = nLen;
/*N*/ 	p->nRPN = nRPN;
/*N*/ 	p->nRefs = nRefs;
/*N*/ 	p->nMode = nMode;
/*N*/ 	p->nError = nError;
/*N*/ 	ScToken** pp;
/*N*/ 	if( nLen )
/*N*/ 	{
/*N*/ 		pp = p->pCode = new ScToken*[ nLen ];
/*N*/ 		memcpy( pp, pCode, nLen * sizeof( ScToken* ) );
/*N*/ 		for( USHORT i = 0; i < nLen; i++, pp++ )
/*N*/ 		{
/*N*/ 			*pp = (*pp)->Clone();
/*N*/ 			(*pp)->IncRef();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( nRPN )
/*N*/ 	{
/*N*/ 		pp = p->pRPN = new ScToken*[ nRPN ];
/*N*/ 		memcpy( pp, pRPN, nRPN * sizeof( ScToken* ) );
/*N*/ 		for( USHORT i = 0; i < nRPN; i++, pp++ )
/*N*/ 		{
/*N*/ 			ScToken* t = *pp;
/*N*/ 			if( t->GetRef() > 1 )
/*N*/ 			{
/*N*/ 				ScToken** p2 = pCode;
/*N*/ 				USHORT nIdx = 0xFFFF;
/*N*/ 				for( USHORT j = 0; j < nLen; j++, p2++ )
/*N*/ 				{
/*N*/ 					if( *p2 == t )
/*N*/ 					{
/*N*/ 						nIdx = j; break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				if( nIdx == 0xFFFF )
/*?*/ 					*pp = t->Clone();
/*N*/ 				else
/*N*/ 					*pp = p->pCode[ nIdx ];
/*N*/ 			}
/*N*/ 			else
/*N*/ 				*pp = t->Clone();
/*N*/ 			(*pp)->IncRef();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return p;
/*N*/ }

/*N*/ void ScTokenArray::Clear()
/*N*/ {
/*N*/ 	if( nRPN ) DelRPN();
/*N*/ 	if( pCode )
/*N*/ 	{
/*N*/ 		ScToken** p = pCode;
/*N*/ 		for( USHORT i = 0; i < nLen; i++ )
/*N*/ 		{
/*N*/ 			(*p++)->DecRef();
/*N*/ 		}
/*N*/ 		delete [] pCode;
/*N*/ 	}
/*N*/ 	pCode = NULL; pRPN = NULL;
/*N*/ 	nError = nLen = nIndex = nRPN = nRefs = 0;
/*M*/     bReplacedSharedFormula = FALSE;
/*N*/ 	ClearRecalcMode();
/*N*/ }

/*N*/ ScToken* ScTokenArray::AddToken( const ScRawToken& r )
/*N*/ {
/*N*/ 	return Add( r.CreateToken() );
/*N*/ }

/*N*/  ScToken* ScTokenArray::AddToken( const ScToken& r )
/*N*/  {
/*N*/  	return Add( r.Clone() );
/*N*/  }

// Wird auch vom Compiler genutzt. Das Token ist per new angelegt!

/*N*/ ScToken* ScTokenArray::Add( ScToken* t )
/*N*/ {
/*N*/ 	if( !pCode )
/*N*/ 		pCode = new ScToken*[ MAXCODE ];
/*N*/ 	if( nLen < MAXCODE-1 )
/*N*/ 	{
/*N*/ 		pCode[ nLen++ ] = t;
/*N*/ 		if( t->GetOpCode() == ocPush
/*N*/ 			&& ( t->GetType() == svSingleRef || t->GetType() == svDoubleRef ) )
/*N*/ 			nRefs++;
/*N*/ 		t->IncRef();
/*N*/ 		return t;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		t->Delete();
/*?*/         if ( nLen == MAXCODE-1 )
/*?*/         {
/*?*/             t = new ScByteToken( ocStop );
/*?*/             pCode[ nLen++ ] = t;
/*?*/             t->IncRef();
/*?*/         }
/*?*/ 		return NULL;
/*N*/ 	}
/*N*/ }

/*N*/ ScToken* ScTokenArray::AddOpCode( OpCode e )
/*N*/ {
/*N*/ 	ScRawToken t;
/*N*/ 	t.SetOpCode( e );
/*N*/ 	return AddToken( t );
/*N*/ }

/*N*/ ScToken* ScTokenArray::AddString( const sal_Unicode* pStr )
/*N*/ {
/*N*/ 	return AddString( String( pStr ) );
/*N*/ }

/*N*/ ScToken* ScTokenArray::AddString( const String& rStr )
/*N*/ {
/*N*/ 	return Add( new ScStringToken( rStr ) );
/*N*/ }

/*N*/ ScToken* ScTokenArray::AddDouble( double fVal )
/*N*/ {
/*N*/ 	return Add( new ScDoubleToken( fVal ) );
/*N*/ }

/*N*/ ScToken* ScTokenArray::AddSingleReference( const SingleRefData& rRef )
/*N*/ {
/*N*/ 	return Add( new ScSingleRefToken( rRef ) );
/*N*/ }

/*N*/ ScToken* ScTokenArray::AddDoubleReference( const ComplRefData& rRef )
/*N*/ {
/*N*/ 	return Add( new ScDoubleRefToken( rRef ) );
/*N*/ }

/*N*/ ScToken* ScTokenArray::AddExternal( const sal_Unicode* pStr )
/*N*/ {
/*N*/ 	return AddExternal( String( pStr ) );
/*N*/ }

/*N*/ ScToken* ScTokenArray::AddExternal( const String& rStr )
/*N*/ {
/*N*/ 	return Add( new ScExternalToken( ocExternal, rStr ) );
/*N*/ }

/*N*/ ScToken* ScTokenArray::AddBad( const String& rStr )
/*N*/ {
/*N*/ 	return Add( new ScStringToken( ocBad, rStr ) );
/*N*/ }

/*N*/ void ScTokenArray::ImportRecalcMode40( ScRecalcMode40 eMode )
/*N*/ {
/*N*/ 	switch ( eMode )
/*N*/ 	{
/*N*/ 		case RC_NORMAL :
/*N*/ 			nMode = RECALCMODE_NORMAL;
/*N*/ 		break;
/*?*/ 		case RC_ALWAYS :
/*?*/ 			nMode = RECALCMODE_ALWAYS;
/*?*/ 		break;
/*?*/ 		case RC_ONLOAD :
/*?*/ 			nMode = RECALCMODE_ONLOAD;
/*?*/ 		break;
/*?*/ 		case RC_ONLOAD_ONCE :
/*?*/ 			nMode = RECALCMODE_ONLOAD_ONCE;
/*?*/ 		break;
/*?*/ 		case RC_FORCED :
/*?*/ 			nMode = RECALCMODE_NORMAL | RECALCMODE_FORCED;
/*?*/ 		break;
/*?*/ 		case RC_ONREFMOVE :
/*?*/ 			nMode = RECALCMODE_NORMAL | RECALCMODE_ONREFMOVE;
/*?*/ 		break;
/*?*/ 		default:
/*?*/ 			DBG_ERRORFILE( "ScTokenArray::ImportRecalcMode40: unknown ScRecalcMode40" );
/*?*/ 			nMode = RECALCMODE_NORMAL;
/*N*/ 	}
/*N*/ }


/*N*/ ScRecalcMode40 ScTokenArray::ExportRecalcMode40() const
/*N*/ {
/*N*/ 	//! Reihenfolge ist wichtig
/*N*/ 	if ( nMode & RECALCMODE_ALWAYS )
/*N*/ 		return RC_ALWAYS;
/*N*/ 	if ( nMode & RECALCMODE_ONLOAD )
/*N*/ 		return RC_ONLOAD;
/*N*/ 	if ( nMode & RECALCMODE_FORCED )
/*N*/ 		return RC_FORCED;
/*N*/ 	if ( nMode & RECALCMODE_ONREFMOVE )
/*N*/ 		return RC_ONREFMOVE;
/*N*/ 	// kommt eigentlich nicht vor weil in Calc bereits umgesetzt,
/*N*/ 	// und woanders gibt es keinen 4.0-Export, deswegen als letztes
/*N*/ 	if ( nMode & RECALCMODE_ONLOAD_ONCE )
/*N*/ 		return RC_ONLOAD_ONCE;
/*N*/ 	return RC_NORMAL;
/*N*/ }


/*N*/ void ScTokenArray::AddRecalcMode( ScRecalcMode nBits )
/*N*/ {
/*N*/ 	//! Reihenfolge ist wichtig
/*N*/ 	if ( nBits & RECALCMODE_ALWAYS )
/*?*/ 		SetRecalcModeAlways();
/*N*/ 	else if ( !IsRecalcModeAlways() )
/*N*/ 	{
/*N*/ 		if ( nBits & RECALCMODE_ONLOAD )
/*N*/ 			SetRecalcModeOnLoad();
/*N*/ 		else if ( nBits & RECALCMODE_ONLOAD_ONCE && !IsRecalcModeOnLoad() )
/*?*/ 			SetRecalcModeOnLoadOnce();
/*N*/ 	}
/*N*/ 	SetCombinedBitsRecalcMode( nBits );
/*N*/ }


/*N*/ BOOL ScTokenArray::HasMatrixDoubleRefOps()
/*N*/ {
/*N*/ 	if ( pRPN && nRPN )
/*N*/ 	{
/*N*/ 		// RPN-Interpreter Simulation
/*N*/ 		// als Ergebnis jeder Funktion wird einfach ein Double angenommen
/*N*/ 		ScToken** pStack = new ScToken* [nRPN];
/*N*/ 		ScToken* pResult = new ScDoubleToken( ocPush, 0.0 );
/*N*/ 		short sp = 0;
/*N*/ 		for ( USHORT j = 0; j < nRPN; j++ )
/*N*/ 		{
/*N*/ 			ScToken* t = pRPN[j];
/*N*/ 			OpCode eOp = t->GetOpCode();
/*N*/ 			BYTE nParams = t->GetParamCount();
/*N*/ 			switch ( eOp )
/*N*/ 			{
/*N*/ 				case ocAdd :
/*N*/ 				case ocSub :
/*N*/ 				case ocMul :
/*N*/ 				case ocDiv :
/*N*/ 				case ocPow :
/*N*/ 				case ocPower :
/*N*/ 				case ocAmpersand :
/*N*/ 				case ocEqual :
/*N*/ 				case ocNotEqual :
/*N*/ 				case ocLess :
/*N*/ 				case ocGreater :
/*N*/ 				case ocLessEqual :
/*N*/ 				case ocGreaterEqual :
/*N*/ 				{
/*N*/ 					for ( BYTE k = nParams; k; k-- )
/*N*/ 					{
/*N*/ 						if ( sp >= k && pStack[sp-k]->GetType() == svDoubleRef )
/*N*/ 						{
/*?*/ 							pResult->Delete();
/*?*/ 							delete [] pStack;
/*?*/ 							return TRUE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			if ( eOp == ocPush || lcl_IsReference( eOp, t->GetType() )  )
/*N*/ 				pStack[sp++] = t;
/*N*/ 			else if ( eOp == ocIf || eOp == ocChose )
/*N*/ 			{	// Jumps ignorieren, vorheriges Result (Condition) poppen
/*N*/ 				if ( sp )
/*N*/ 					--sp;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{	// pop parameters, push result
/*N*/ 				sp -= nParams;
/*N*/ 				if ( sp < 0 )
/*N*/ 				{
/*N*/ 					DBG_ERROR( "ScTokenArray::HasMatrixDoubleRefOps: sp < 0" );
/*N*/ 					sp = 0;
/*N*/ 				}
/*N*/ 				pStack[sp++] = pResult;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pResult->Delete();
/*N*/ 		delete [] pStack;
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

///////////////////////////////////////////////////////////////////////////

/*N*/ void ScRawToken::Load30( SvStream& rStream )
/*N*/ {
/*N*/ 	UINT16 nOp;
/*N*/ 	BYTE n;
/*N*/ 	nRefCnt = 0;
/*N*/ 	rStream >> nOp;
/*N*/ 	eOp = (OpCode) nOp;
/*N*/ 	switch( eOp )
/*N*/ 	{
/*N*/ 		case ocIf:
/*?*/ 			eType = svJump; nJump[ 0 ] = 3; break;	// then, else, behind
/*N*/ 		case ocChose:
/*?*/ 			eType = svJump; nJump[ 0 ] = MAXJUMPCOUNT+1; break;
/*N*/ 		case ocPush:
/*N*/ 			rStream >> n;
/*N*/ 			eType = (StackVar) n;
/*N*/ 			switch( eType )
/*N*/ 			{
/*N*/ 				case svByte:
/*?*/ 					rStream >> cByte;
/*?*/ 					break;
/*N*/ 				case svDouble:
/*N*/ 					rStream >> nValue;
/*N*/ 					break;
/*N*/ 				case svString:
/*N*/ 				{
/*?*/ 					sal_Char c[ MAXSTRLEN+1 ];
/*?*/ 					rStream >> nOp;
/*?*/ 					if( nOp > MAXSTRLEN-1 )
/*?*/ 					{
/*?*/ 						DBG_ERROR("Dokument huehnerich");
/*?*/ 						USHORT nDiff = nOp - (MAXSTRLEN-1);
/*?*/ 						nOp = MAXSTRLEN-1;
/*?*/ 						rStream.Read( c, nOp );
/*?*/ 						rStream.SeekRel( nDiff );
/*?*/ 					}
/*?*/ 					else
/*?*/ 						rStream.Read( c, nOp );
/*?*/ 					CharSet eSrc = rStream.GetStreamCharSet();
/*?*/ 					for ( BYTE j=0; j<nOp; j++ )
/*?*/ 						cStr[j] = ByteString::ConvertToUnicode( c[j], eSrc );
/*?*/ 					cStr[ nOp ] = 0;
/*?*/ 					break;
/*N*/ 				}
/*N*/ 				case svSingleRef:
/*N*/ 				{
/*N*/ 					OldSingleRefBools aBools;
/*N*/ 					rStream >> aRef.Ref1.nCol
/*N*/ 							>> aRef.Ref1.nRow
/*N*/ 							>> aRef.Ref1.nTab
/*N*/ 							>> aBools.bRelCol
/*N*/ 							>> aBools.bRelRow
/*N*/ 							>> aBools.bRelTab
/*N*/ 							>> aBools.bOldFlag3D;
/*N*/ 					aRef.Ref1.OldBoolsToNewFlags( aBools );
/*N*/ 					aRef.Ref2 = aRef.Ref1;
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				case svDoubleRef:
/*N*/ 				{
/*N*/ 					OldSingleRefBools aBools1;
/*N*/ 					OldSingleRefBools aBools2;
/*N*/ 					rStream >> aRef.Ref1.nCol
/*N*/ 							>> aRef.Ref1.nRow
/*N*/ 							>> aRef.Ref1.nTab
/*N*/ 							>> aRef.Ref2.nCol
/*N*/ 							>> aRef.Ref2.nRow
/*N*/ 							>> aRef.Ref2.nTab
/*N*/ 							>> aBools1.bRelCol
/*N*/ 							>> aBools1.bRelRow
/*N*/ 							>> aBools1.bRelTab
/*N*/ 							>> aBools2.bRelCol
/*N*/ 							>> aBools2.bRelRow
/*N*/ 							>> aBools2.bRelTab
/*N*/ 							>> aBools1.bOldFlag3D
/*N*/ 							>> aBools2.bOldFlag3D;
/*N*/ 					aRef.Ref1.OldBoolsToNewFlags( aBools1 );
/*N*/ 					aRef.Ref2.OldBoolsToNewFlags( aBools2 );
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				default: DBG_ERROR("Unknown Stack Variable");
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		case ocName:
/*?*/ 			eType = svIndex;
/*?*/ 			rStream >> nIndex;
/*?*/ 			break;
/*?*/ 		case ocExternal:
/*?*/ 		{
/*?*/ 			sal_Char c[ MAXSTRLEN+1 ];
/*?*/ 			eType = svExternal;
/*?*/ 			rStream >> nOp;
/*?*/ 			// lieber ein rottes Dokument als stack overwrite
/*?*/ 			if( nOp > MAXSTRLEN-2 )
/*?*/ 			{
/*?*/ 				DBG_ERROR("Dokument huehnerich");
/*?*/ 				USHORT nDiff = nOp - (MAXSTRLEN-2);
/*?*/ 				nOp = MAXSTRLEN-2;
/*?*/ 				rStream.Read( c, nOp );
/*?*/ 				rStream.SeekRel( nDiff );
/*?*/ 			}
/*?*/ 			else
/*?*/ 				rStream.Read( c, nOp );
/*?*/ 			CharSet eSrc = rStream.GetStreamCharSet();
/*?*/ 			for ( BYTE j=1; j<nOp; j++ )
/*?*/ 				cStr[j] = ByteString::ConvertToUnicode( c[j-1], eSrc );
/*?*/ 			cStr[ 0 ] = 0;		//! parameter count is what?!?
/*?*/ 			cStr[ nOp ] = 0;
/*?*/ 			break;
/*?*/ 		}
/*N*/ 		default:
/*N*/ 			eType = svByte;
/*N*/ 			cByte = 0;
/*N*/ 	}
/*N*/ }

// Bei unbekannten Tokens steht in cStr (k)ein Pascal-String (cStr[0] = Laenge),
// der nur gepuffert wird. cStr[0] = GESAMT-Laenge inkl. [0] !!!

/*N*/ void ScRawToken::Load( SvStream& rStream, USHORT nVer )
/*N*/ {
/*N*/ 	BYTE n;
/*N*/ 	UINT16 nOp;
/*N*/ 	USHORT i;
/*N*/ 	rStream >> nOp >> n;
/*N*/ 	eOp = (OpCode) nOp;
/*N*/ 	eType = (StackVar) n;
/*N*/ 	switch( eType )
/*N*/ 	{
/*N*/ 		case svByte:
/*N*/ 			rStream >> cByte;
/*N*/ 			break;
/*N*/ 		case svDouble:
/*N*/ 			rStream >> nValue;
/*N*/ 			break;
/*N*/ 		case svExternal:
/*N*/ 		{
/*N*/ 			sal_Char c[ MAXSTRLEN+1 ];
/*N*/ 			rStream >> cByte >> n;
/*N*/ 			if( n > MAXSTRLEN-2 )
/*N*/ 			{
/*?*/ 				DBG_ERRORFILE( "bad string array boundary" );
/*?*/ 				USHORT nDiff = n - (MAXSTRLEN-2);
/*?*/ 				n = MAXSTRLEN-2;
/*?*/ 				rStream.Read( c+1, n );
/*?*/ 				rStream.SeekRel( nDiff );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				rStream.Read( c+1, n );
/*N*/             //! parameter count is in cByte (cStr[0] little endian)
/*N*/ 			CharSet eSrc = rStream.GetStreamCharSet();
/*N*/ 			for ( BYTE j=1; j<n+1; j++ )
/*N*/ 				cStr[j] = ByteString::ConvertToUnicode( c[j], eSrc );
/*N*/ 			cStr[ n+1 ] = 0;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case svString:
/*N*/ 		{
/*N*/ 			sal_Char c[ MAXSTRLEN+1 ];
/*N*/ 			rStream >> n;
/*N*/ 			if( n > MAXSTRLEN-1 )
/*N*/ 			{
/*?*/ 				DBG_ERRORFILE( "bad string array boundary" );
/*?*/ 				USHORT nDiff = n - (MAXSTRLEN-1);
/*?*/ 				n = MAXSTRLEN-1;
/*?*/ 				rStream.Read( c, n );
/*?*/ 				rStream.SeekRel( nDiff );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				rStream.Read( c, n );
/*N*/ 			cStr[ n ] = 0;
/*N*/ 			CharSet eSrc = rStream.GetStreamCharSet();
/*N*/ 			for ( BYTE j=0; j<n; j++ )
/*N*/ 				cStr[j] = ByteString::ConvertToUnicode( c[j], eSrc );
/*N*/ 			cStr[ n ] = 0;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case svSingleRef:
/*N*/ 		case svDoubleRef:
/*N*/ 		{
/*N*/ 			SingleRefData& r = aRef.Ref1;
/*N*/ 			rStream >> r.nCol
/*N*/ 					>> r.nRow
/*N*/ 					>> r.nTab
/*N*/ 					>> n;
/*N*/ 			if ( nVer < SC_RELATIVE_REFS )
/*N*/ 			{
/*N*/				OldSingleRefBools aBools;
/*N*/ 				aBools.bRelCol = ( n & 0x03 );
/*N*/ 				aBools.bRelRow = ( ( n >> 2 ) & 0x03 );
/*N*/ 				aBools.bRelTab = ( ( n >> 4 ) & 0x03 );
/*N*/ 				aBools.bOldFlag3D = ( ( n >> 6 ) & 0x03 );
/*N*/ 				r.OldBoolsToNewFlags( aBools );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				r.CreateFlagsFromLoadByte( n );
/*N*/ 			if( eType == svSingleRef )
/*N*/ 				aRef.Ref2 = r;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SingleRefData& r = aRef.Ref2;
/*N*/ 				rStream >> r.nCol
/*N*/ 						>> r.nRow
/*N*/ 						>> r.nTab
/*N*/ 						>> n;
/*N*/ 				if ( nVer < SC_RELATIVE_REFS )
/*N*/ 				{
/*N*/					OldSingleRefBools aBools;
/*N*/ 					aBools.bRelCol = ( n & 0x03 );
/*N*/ 					aBools.bRelRow = ( ( n >> 2 ) & 0x03 );
/*N*/ 					aBools.bRelTab = ( ( n >> 4 ) & 0x03 );
/*N*/ 					aBools.bOldFlag3D = ( ( n >> 6 ) & 0x03 );
/*N*/ 					r.OldBoolsToNewFlags( aBools );
/*N*/ 				}
/*N*/ 				else
/*N*/ 					r.CreateFlagsFromLoadByte( n );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case svIndex:
/*N*/ 			rStream >> nIndex;
/*N*/ 			break;
/*N*/ 		case svJump:
/*N*/ 			rStream >> n;
/*N*/ 			nJump[ 0 ] = n;
/*N*/ 			for( i = 1; i <= n; i++ )
/*N*/ 				rStream >> nJump[ i ];
/*N*/ 			break;
/*?*/ 		case svMissing:
/*?*/ 		case svErr:
/*?*/ 			break;
/*?*/ 		default:
/*?*/ 		{
/*?*/ 			rStream >> n;
/*?*/ 			if( n > MAXSTRLEN-2 )
/*?*/ 			{
/*?*/ 				DBG_ERRORFILE( "bad unknown token type array boundary" );
/*?*/ 				USHORT nDiff = n - (MAXSTRLEN-2);
/*?*/ 				n = MAXSTRLEN-2;
/*?*/ 				rStream.Read( ((BYTE*)cStr)+1, n );
/*?*/ 				rStream.SeekRel( nDiff );
/*?*/                 ++n;
/*?*/ 			}
/*?*/             else if ( n > 1 )
/*?*/ 				rStream.Read( ((BYTE*)cStr)+1, n-1 );
/*?*/             else if ( n == 0 )
/*?*/             {
/*?*/ 				DBG_ERRORFILE( "unknown token type length==0" );
/*?*/                 n = 1;
/*?*/             }
/*?*/ 			*((BYTE*)cStr) = n;     // length including length byte
/*?*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScToken::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	short i;
/*N*/ 	rStream << (UINT16) eOp << (BYTE) eType;
/*N*/ 	switch( eType )
/*N*/ 	{
/*N*/ 		case svByte:
/*N*/ 			rStream << GetByte();
/*N*/ 			break;
/*N*/ 		case svDouble:
/*N*/ 			rStream << GetDouble();
/*N*/ 			break;
/*N*/ 		case svExternal:
/*N*/ 		{
/*N*/ 			ByteString aTmp( GetExternal(), rStream.GetStreamCharSet() );
/*N*/ 			aTmp.Erase( 255 );		// old SO5 can't handle more
/*N*/ 			rStream << GetByte()
/*N*/ 					<< (UINT8) aTmp.Len();
/*N*/ 			rStream.Write( aTmp.GetBuffer(), (UINT8) aTmp.Len() );
/*N*/ 		}
/*N*/ 			break;
/*N*/ 		case svString:
/*N*/ 		{
/*N*/ 			ByteString aTmp( GetString(), rStream.GetStreamCharSet() );
/*N*/ 			aTmp.Erase( 255 );		// old SO5 can't handle more
/*N*/ 			rStream << (UINT8) aTmp.Len();
/*N*/ 			rStream.Write( aTmp.GetBuffer(), (UINT8) aTmp.Len() );
/*N*/ 		}
/*N*/ 			break;
/*N*/ 		case svSingleRef:
/*N*/ 		{
/*N*/ 			const SingleRefData& r = GetSingleRef();
/*N*/ 			BYTE n = r.CreateStoreByteFromFlags();
/*N*/ 			rStream << (INT16) r.nCol
/*N*/ 					<< (INT16) r.nRow
/*N*/ 					<< (INT16) r.nTab
/*N*/ 					<< (BYTE) n;
/*N*/ 		}
/*N*/ 			break;
/*N*/ 		case svDoubleRef:
/*N*/ 		{
/*N*/ 			const ComplRefData& rRef = GetDoubleRef();
/*N*/ 			const SingleRefData& r1 = rRef.Ref1;
/*N*/ 			BYTE n = r1.CreateStoreByteFromFlags();
/*N*/ 			rStream << (INT16) r1.nCol
/*N*/ 					<< (INT16) r1.nRow
/*N*/ 					<< (INT16) r1.nTab
/*N*/ 					<< (BYTE) n;
/*N*/ 			const SingleRefData& r2 = rRef.Ref2;
/*N*/ 			n = r2.CreateStoreByteFromFlags();
/*N*/ 			rStream << (INT16) r2.nCol
/*N*/ 					<< (INT16) r2.nRow
/*N*/ 					<< (INT16) r2.nTab
/*N*/ 					<< (BYTE) n;
/*N*/ 		}
/*N*/ 			break;
/*N*/ 		case svIndex:
/*N*/ 			rStream << (UINT16) GetIndex();
/*N*/ 			break;
/*N*/ 		case svJump:
/*N*/ 		{
/*N*/ 			short* pJump = GetJump();
/*N*/ 			rStream << (BYTE) pJump[ 0 ];
/*N*/ 			for( i = 1; i <= pJump[ 0 ]; i++ )
/*N*/ 				rStream << (UINT16) pJump[ i ];
/*N*/ 		}
/*N*/ 			break;
/*?*/ 		case svMissing:
/*?*/ 		case svErr:
/*?*/ 			break;
/*?*/ 		default:
/*?*/ 		{
/*?*/ 			BYTE* pUnknown = GetUnknown();
/*?*/ 			if ( pUnknown )
/*?*/ 				rStream.Write( pUnknown, pUnknown[ 0 ] );
/*?*/ 		}
/*N*/ 	}
/*N*/ }

/*----------------------------------------------------------------------*/

/*N*/ ScTokenIterator::ScTokenIterator( const ScTokenArray& rArr )
/*N*/ {
/*N*/ 	pCur = NULL;
/*N*/ 	Push( (ScTokenArray*) &rArr );
/*N*/ }

/*N*/ ScTokenIterator::~ScTokenIterator()
/*N*/ {
/*N*/ 	while( pCur )
/*N*/ 		Pop();
/*N*/ }

/*N*/ void ScTokenIterator::Push( ScTokenArray* pArr )
/*N*/ {
/*N*/ 	ImpTokenIterator* p = new ImpTokenIterator;
/*N*/ 	p->pArr  = pArr;
/*N*/ 	p->nPC   = -1;
/*N*/ 	p->pNext = pCur;
/*N*/ 	pCur     = p;
/*N*/ }

/*N*/ void ScTokenIterator::Pop()
/*N*/ {
/*N*/ 	ImpTokenIterator* p = pCur;
/*N*/ 	if( p )
/*N*/ 	{
/*N*/ 		pCur = p->pNext;
/*N*/ 		delete p;
/*N*/ 	}
/*N*/ }

/*N*/ void ScTokenIterator::Reset()
/*N*/ {
/*N*/ 	while( pCur->pNext )
/*?*/ 		Pop();
/*N*/ 	pCur->nPC = -1;
/*N*/ }

/*N*/ const ScToken* ScTokenIterator::Next()
/*N*/ {
/*N*/ 	const ScToken* t = NULL;
/*N*/ 	if( ++pCur->nPC < pCur->pArr->nRPN )
/*N*/ 	{
/*N*/ 		t = pCur->pArr->pRPN[ pCur->nPC ];
/*N*/ 		// ein derartiger Opcode endet einen WENN- oder WAHL-Bereich
/*N*/ 		if( t->GetOpCode() == ocSep || t->GetOpCode() == ocClose )
/*N*/ 			t = NULL;
/*N*/ 	}
/*N*/ 	if( !t && pCur->pNext )
/*N*/ 	{
/*N*/ 		Pop(); t = Next();
/*N*/ 	}
/*N*/ 	return t;
/*N*/ }

// Die PC-Werte sind -1!

/*N*/ void ScTokenIterator::Jump( short nStart, short nNext )
/*N*/ {
/*N*/ 	pCur->nPC = nNext;
/*N*/ 	if( nStart != nNext )
/*N*/ 	{
/*N*/ 		Push( pCur->pArr );
/*N*/ 		pCur->nPC = nStart;
/*N*/ 	}
/*N*/ }


}
