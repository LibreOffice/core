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

#include <basic/sbx.hxx>
#include "sbcomp.hxx"
#include "sbunoobj.hxx"
#include <svtools/miscopt.hxx>
#include "com/sun/star/reflection/XIdlReflection.hpp"
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/configurationhelper.hxx>
#include "com/sun/star/reflection/XInterfaceMemberTypeDescription.hpp"
#include "com/sun/star/reflection/XIdlMethod.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include <basic/codecompletecache.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SbxObject* cloneTypeObjectImpl( const SbxObject& rTypeObj );

// Declaration of a variable
// If there are errors it will be parsed up to the comma or the newline.
// Return-value: a new instance, which were inserted and then deleted.
// Array-Indexex were returned as SbiDimList

SbiSymDef* SbiParser::VarDecl( SbiDimList** ppDim, bool bStatic, bool bConst )
{
    bool bWithEvents = false;
    if( Peek() == WITHEVENTS )
    {
        Next();
        bWithEvents = true;
    }
    if( !TestSymbol() ) return NULL;
    SbxDataType t = eScanType;
    SbiSymDef* pDef = bConst ? new SbiConstDef( aSym ) : new SbiSymDef( aSym );
    SbiDimList* pDim = NULL;
    // Brackets?
    if( Peek() == LPAREN )
    {
        pDim = new SbiDimList( this );
        if( !pDim->GetDims() )
            pDef->SetWithBrackets();
    }
    pDef->SetType( t );
    if( bStatic )
        pDef->SetStatic();
    if( bWithEvents )
        pDef->SetWithEvents();
    TypeDecl( *pDef );
    if( !ppDim && pDim )
    {
        if(pDim->GetDims() )
            Error( SbERR_EXPECTED, "()" );
        delete pDim;
    }
    else if( ppDim )
        *ppDim = pDim;
    return pDef;
}

// Resolving of a AS-Type-Declaration
// The data type were inserted into the handed over variable

void SbiParser::TypeDecl( SbiSymDef& rDef, bool bAsNewAlreadyParsed )
{
    SbxDataType eType = rDef.GetType();
    if( bAsNewAlreadyParsed || Peek() == AS )
    {
        short nSize = 0;
        if( !bAsNewAlreadyParsed )
            Next();
        rDef.SetDefinedAs();
        SbiToken eTok = Next();
        if( !bAsNewAlreadyParsed && eTok == NEW )
        {
            rDef.SetNew();
            eTok = Next();
        }
        switch( eTok )
        {
            case ANY:
                if( rDef.IsNew() )
                    Error( SbERR_SYNTAX );
                eType = SbxVARIANT; break;
            case TINTEGER:
            case TLONG:
            case TSINGLE:
            case TDOUBLE:
            case TCURRENCY:
            case TDATE:
            case TSTRING:
            case TOBJECT:
            case _ERROR_:
            case TBOOLEAN:
            case TVARIANT:
            case TBYTE:
                if( rDef.IsNew() )
                    Error( SbERR_SYNTAX );
                eType = (eTok==TBYTE) ? SbxBYTE : SbxDataType( eTok - TINTEGER + SbxINTEGER );
                if( eType == SbxSTRING )
                {
                    // STRING*n ?
                    if( Peek() == MUL )
                    {       // fixed size!
                        Next();
                        SbiConstExpression aSize( this );
                        nSize = aSize.GetShortValue();
                        if( nSize < 0 || (bVBASupportOn && nSize <= 0) )
                            Error( SbERR_OUT_OF_RANGE );
                        else
                            rDef.SetFixedStringLength( nSize );
                    }
                }
                break;
            case SYMBOL: // can only be a TYPE or a object class!
                if( eScanType != SbxVARIANT )
                    Error( SbERR_SYNTAX );
                else
                {
                    OUString aCompleteName = aSym;

                    // #52709 DIM AS NEW for Uno with full-qualified name
                    if( Peek() == DOT )
                    {
                        OUString aDotStr( '.' );
                        while( Peek() == DOT )
                        {
                            aCompleteName += aDotStr;
                            Next();
                            SbiToken ePeekTok = Peek();
                            if( ePeekTok == SYMBOL || IsKwd( ePeekTok ) )
                            {
                                Next();
                                aCompleteName += aSym;
                            }
                            else
                            {
                                Next();
                                Error( SbERR_UNEXPECTED, SYMBOL );
                                break;
                            }
                        }
                    }
                    else if( rEnumArray->Find( aCompleteName, SbxCLASS_OBJECT ) || ( IsVBASupportOn() && VBAConstantHelper::instance().isVBAConstantType( aCompleteName ) ) )
                    {
                        eType = SbxLONG;
                        break;
                    }

                    // Take over in the string pool
                    rDef.SetTypeId( aGblStrings.Add( aCompleteName ) );

                    if( rDef.IsNew() && pProc == NULL )
                        aRequiredTypes.push_back( aCompleteName );
                }
                eType = SbxOBJECT;
                break;
            case FIXSTRING: // new syntax for complex UNO types
                rDef.SetTypeId( aGblStrings.Add( aSym ) );
                eType = SbxOBJECT;
                break;
            default:
                Error( SbERR_UNEXPECTED, eTok );
                Next();
        }
        // The variable could have been declared with a suffix
        if( rDef.GetType() != SbxVARIANT )
        {
            if( rDef.GetType() != eType )
                Error( SbERR_VAR_DEFINED, rDef.GetName() );
            else if( eType == SbxSTRING && rDef.GetLen() != nSize )
                Error( SbERR_VAR_DEFINED, rDef.GetName() );
        }
        rDef.SetType( eType );
        rDef.SetLen( nSize );
    }
}

// Here variables, arrays and structures were definied.
// DIM/PRIVATE/PUBLIC/GLOBAL

void SbiParser::Dim()
{
    DefVar( _DIM, ( pProc && bVBASupportOn ) ? pProc->IsStatic() : false );
}

void SbiParser::DefVar( SbiOpcode eOp, bool bStatic )
{
    SbiSymPool* pOldPool = pPool;
    bool bSwitchPool = false;
    bool bPersistantGlobal = false;
    SbiToken eFirstTok = eCurTok;

    if( pProc && ( eCurTok == GLOBAL || eCurTok == PUBLIC || eCurTok == PRIVATE ) )
        Error( SbERR_NOT_IN_SUBR, eCurTok );
    if( eCurTok == PUBLIC || eCurTok == GLOBAL )
    {
        bSwitchPool = true;     // at the right moment switch to the global pool
        if( eCurTok == GLOBAL )
            bPersistantGlobal = true;
    }
    // behavior in VBA is that a module scope variable's lifetime is
    // tied to the document. e.g. a module scope variable is global
       if(  GetBasic()->IsDocBasic() && bVBASupportOn && !pProc )
        bPersistantGlobal = true;
    // PRIVATE is a synonymous for DIM
    // _CONST_?
    bool bConst = false;
    if( eCurTok == _CONST_ )
        bConst = true;
    else if( Peek() == _CONST_ )
        Next(), bConst = true;

    // #110004 It can also be a sub/function
    if( !bConst && (eCurTok == SUB || eCurTok == FUNCTION || eCurTok == PROPERTY ||
                    eCurTok == STATIC || eCurTok == ENUM || eCurTok == DECLARE || eCurTok == TYPE) )
    {
        // Next token is read here, because !bConst
        bool bPrivate = ( eFirstTok == PRIVATE );

        if( eCurTok == STATIC )
        {
            Next();
            DefStatic( bPrivate );
        }
        else if( eCurTok == SUB || eCurTok == FUNCTION || eCurTok == PROPERTY )
        {
            // End global chain if necessary (not done in
            // SbiParser::Parse() under these conditions
            if( bNewGblDefs && nGblChain == 0 )
            {
                nGblChain = aGen.Gen( _JUMP, 0 );
                bNewGblDefs = false;
            }
            Next();
            DefProc( false, bPrivate );
            return;
        }
        else if( eCurTok == ENUM )
        {
            Next();
            DefEnum( bPrivate );
            return;
        }
        else if( eCurTok == DECLARE )
        {
            Next();
            DefDeclare( bPrivate );
            return;
        }
        // #i109049
        else if( eCurTok == TYPE )
        {
            Next();
            DefType( bPrivate );
            return;
        }
    }

#ifdef SHARED
#define tmpSHARED
#undef SHARED
#endif
    // SHARED were ignored
    if( Peek() == SHARED ) Next();
#ifdef tmpSHARED
#define SHARED
#undef tmpSHARED
#endif
    // PRESERVE only at REDIM
    if( Peek() == PRESERVE )
    {
        Next();
        if( eOp == _REDIM )
            eOp = _REDIMP;
        else
            Error( SbERR_UNEXPECTED, eCurTok );
    }
    SbiSymDef* pDef;
    SbiDimList* pDim;

    // #40689, Statics -> Modul-Initialising, skip in Sub
    sal_uInt32 nEndOfStaticLbl = 0;
    if( !bVBASupportOn && bStatic )
    {
        nEndOfStaticLbl = aGen.Gen( _JUMP, 0 );
        aGen.Statement();   // catch up on static here
    }

    bool bDefined = false;
    while( ( pDef = VarDecl( &pDim, bStatic, bConst ) ) != NULL )
    {
        /*fprintf(stderr, "Actual sub: \n");
        fprintf(stderr, "Symbol name: %s\n",OUStringToOString(pDef->GetName(),RTL_TEXTENCODING_UTF8).getStr());*/
        EnableErrors();
        // search variable:
        if( bSwitchPool )
            pPool = &aGlobals;
        SbiSymDef* pOld = pPool->Find( pDef->GetName() );
        // search also in the Runtime-Library
        bool bRtlSym = false;
        if( !pOld )
        {
            pOld = CheckRTLForSym( pDef->GetName(), SbxVARIANT );
            if( pOld )
                bRtlSym = true;
        }
        if( pOld && !(eOp == _REDIM || eOp == _REDIMP) )
        {
            if( pDef->GetScope() == SbLOCAL && pOld->GetScope() != SbLOCAL )
                pOld = NULL;
        }
        if( pOld )
        {
            bDefined = true;
            // always an error at a RTL-S
            if( !bRtlSym && (eOp == _REDIM || eOp == _REDIMP) )
            {
                // compare the attributes at a REDIM
                SbxDataType eDefType;
                bool bError_ = false;
                if( pOld->IsStatic() )
                {
                    bError_ = true;
                }
                else if( pOld->GetType() != ( eDefType = pDef->GetType() ) )
                {
                    if( !( eDefType == SbxVARIANT && !pDef->IsDefinedAs() ) )
                        bError_ = true;
                }
                if( bError_ )
                    Error( SbERR_VAR_DEFINED, pDef->GetName() );
            }
            else
                Error( SbERR_VAR_DEFINED, pDef->GetName() );
            delete pDef; pDef = pOld;
        }
        else
            pPool->Add( pDef );

        // #36374: Create the variable in front of the distinction IsNew()
        // Otherwise error at Dim Identifier As New Type and option explicit
        if( !bDefined && !(eOp == _REDIM || eOp == _REDIMP)
                      && ( !bConst || pDef->GetScope() == SbGLOBAL ) )
        {
            // Declare variable or global constant
            SbiOpcode eOp2;
            switch ( pDef->GetScope() )
            {
                case SbGLOBAL:  eOp2 = bPersistantGlobal ? _GLOBAL_P : _GLOBAL;
                                goto global;
                case SbPUBLIC:  eOp2 = bPersistantGlobal ? _PUBLIC_P : _PUBLIC;
                                // #40689, no own Opcode anymore
                                if( bVBASupportOn && bStatic )
                                {
                                    eOp2 = _STATIC;
                                    break;
                                }
                global:         aGen.BackChain( nGblChain );
                                nGblChain = 0;
                                bGblDefs = bNewGblDefs = true;
                                break;
                default:        eOp2 = _LOCAL;
            }
            sal_uInt32 nOpnd2 = sal::static_int_cast< sal_uInt16 >( pDef->GetType() );
            if( pDef->IsWithEvents() )
                nOpnd2 |= SBX_TYPE_WITH_EVENTS_FLAG;

            if( bCompatible && pDef->IsNew() )
                nOpnd2 |= SBX_TYPE_DIM_AS_NEW_FLAG;

            short nFixedStringLength = pDef->GetFixedStringLength();
            if( nFixedStringLength >= 0 )
                nOpnd2 |= (SBX_FIXED_LEN_STRING_FLAG + (sal_uInt32(nFixedStringLength) << 17));     // len = all bits above 0x10000

            if( pDim != NULL && pDim->GetDims() > 0 )
                nOpnd2 |= SBX_TYPE_VAR_TO_DIM_FLAG;

            aGen.Gen( eOp2, pDef->GetId(), nOpnd2 );
        }

        // Initialising for self-defined daty types
        // and per NEW created variable
        if( pDef->GetType() == SbxOBJECT
         && pDef->GetTypeId() )
        {
            if( !bCompatible && !pDef->IsNew() )
            {
                OUString aTypeName( aGblStrings.Find( pDef->GetTypeId() ) );
                if( rTypeArray->Find( aTypeName, SbxCLASS_OBJECT ) == NULL )
                {
                    if( CodeCompleteOptions::IsExtendedTypeDeclaration() )
                    {
                        if(!IsUnoInterface(aTypeName))
                            Error( SbERR_UNDEF_TYPE, aTypeName );
                    }
                    else
                        Error( SbERR_UNDEF_TYPE, aTypeName );
                }
            }

            if( bConst )
            {
                Error( SbERR_SYNTAX );
            }

            if( pDim )
            {
                if( eOp == _REDIMP )
                {
                    SbiExpression aExpr( this, *pDef, NULL );
                    aExpr.Gen();
                    aGen.Gen( _REDIMP_ERASE );

                    pDef->SetDims( pDim->GetDims() );
                    SbiExpression aExpr2( this, *pDef, pDim );
                    aExpr2.Gen();
                    aGen.Gen( _DCREATE_REDIMP, pDef->GetId(), pDef->GetTypeId() );
                }
                else
                {
                    pDef->SetDims( pDim->GetDims() );
                    SbiExpression aExpr( this, *pDef, pDim );
                    aExpr.Gen();
                    aGen.Gen( _DCREATE, pDef->GetId(), pDef->GetTypeId() );
                }
            }
            else
            {
                SbiExpression aExpr( this, *pDef );
                aExpr.Gen();
                SbiOpcode eOp_ = pDef->IsNew() ? _CREATE : _TCREATE;
                aGen.Gen( eOp_, pDef->GetId(), pDef->GetTypeId() );
                if ( bVBASupportOn )
                    aGen.Gen( _VBASET );
                else
                    aGen.Gen( _SET );
            }
        }
        else
        {
            if( bConst )
            {
                // Definition of the constants
                if( pDim )
                {
                    Error( SbERR_SYNTAX );
                    delete pDim;
                }
                SbiExpression aVar( this, *pDef );
                if( !TestToken( EQ ) )
                    goto MyBreak;   // (see below)
                SbiConstExpression aExpr( this );
                if( !bDefined && aExpr.IsValid() )
                {
                    if( pDef->GetScope() == SbGLOBAL )
                    {
                        // Create code only for the global constant!
                        aVar.Gen();
                        aExpr.Gen();
                        aGen.Gen( _PUTC );
                    }
                    SbiConstDef* pConst = pDef->GetConstDef();
                    if( aExpr.GetType() == SbxSTRING )
                        pConst->Set( aExpr.GetString() );
                    else
                        pConst->Set( aExpr.GetValue(), aExpr.GetType() );
                }
            }
            else if( pDim )
            {
                // Dimension the variable
                // Delete the var at REDIM beforehand
                if( eOp == _REDIM )
                {
                    SbiExpression aExpr( this, *pDef, NULL );
                    aExpr.Gen();
                    if ( bVBASupportOn )
                        // delete the array but
                        // clear the variable ( this
                        // allows the processing of
                        // the param to happen as normal without errors ( ordinary ERASE just clears the array )
                        aGen.Gen( _ERASE_CLEAR );
                    else
                        aGen.Gen( _ERASE );
                }
                else if( eOp == _REDIMP )
                {
                    SbiExpression aExpr( this, *pDef, NULL );
                    aExpr.Gen();
                    aGen.Gen( _REDIMP_ERASE );
                }
                pDef->SetDims( pDim->GetDims() );
                if( bPersistantGlobal )
                    pDef->SetGlobal( sal_True );
                SbiExpression aExpr( this, *pDef, pDim );
                aExpr.Gen();
                pDef->SetGlobal( sal_False );
                aGen.Gen( (eOp == _STATIC) ? _DIM : eOp );
            }
        }
        if( !TestComma() )
            goto MyBreak;

        // Implementation of bSwitchPool (see above): pPool must not be set to &aGlobals
        // at the VarDecl-Call.
        // Apart from that the behavior should be absolutely identical,
        // i.e., pPool had to be reset always at the end of the loop.
        // also at a break
        pPool = pOldPool;
        continue;       // Skip MyBreak
    MyBreak:
        pPool = pOldPool;
        break;
    }

    // #40689, finalize the jump over statics declarations
    if( !bVBASupportOn && bStatic )
    {
        // maintain the global chain
        nGblChain = aGen.Gen( _JUMP, 0 );
        bGblDefs = bNewGblDefs = true;

        // Register for Sub a jump to the end of statics
        aGen.BackChain( nEndOfStaticLbl );
    }

}

// Here were Arrays redimensioned.

void SbiParser::ReDim()
{
    DefVar( _REDIM, (  pProc && bVBASupportOn ) ? pProc->IsStatic() : false );
}

// ERASE array, ...

void SbiParser::Erase()
{
    while( !bAbort )
    {
        SbiExpression aExpr( this, SbLVALUE );
        aExpr.Gen();
        aGen.Gen( _ERASE );
        if( !TestComma() ) break;
    }
}

// Declaration of a data type

void SbiParser::Type()
{
    DefType( false );
}

void SbiParser::DefType( bool bPrivate )
{
    // TODO: Use bPrivate
    (void)bPrivate;

    // Read the new Token lesen. It had to be a symbol
    if (!TestSymbol())
        return;

    if (rTypeArray->Find(aSym,SbxCLASS_OBJECT))
    {
        Error( SbERR_VAR_DEFINED, aSym );
        return;
    }

    SbxObject *pType = new SbxObject(aSym);

    SbiSymDef* pElem;
    SbiDimList* pDim = NULL;
    bool bDone = false;

    while( !bDone && !IsEof() )
    {
        switch( Peek() )
        {
            case ENDTYPE :
                pElem = NULL;
                bDone = true;
                Next();
            break;

            case EOLN :
            case REM :
                pElem = NULL;
                Next();
            break;

            default:
                pElem = VarDecl(&pDim, false, false);
                if( !pElem )
                    bDone = true;   // Error occurred
        }
        if( pElem )
        {
            SbxArray *pTypeMembers = pType->GetProperties();
            OUString aElemName = pElem->GetName();
            if( pTypeMembers->Find( aElemName, SbxCLASS_DONTCARE) )
            {
                Error (SbERR_VAR_DEFINED);
            }
            else
            {
                SbxDataType eElemType = pElem->GetType();
                SbxProperty *pTypeElem = new SbxProperty( aElemName, eElemType );
                if( pDim )
                {
                    SbxDimArray* pArray = new SbxDimArray( pElem->GetType() );
                    if ( pDim->GetSize() )
                    {
                        // Dimension the target array

                        for ( short i=0; i<pDim->GetSize();++i )
                        {
                            sal_Int32 lb = nBase;
                            SbiExprNode* pNode =  pDim->Get(i)->GetExprNode();
                            sal_Int32 ub = pNode->GetNumber();
                            if ( !pDim->Get( i )->IsBased() ) // each dim is low/up
                            {
                                if (  ++i >= pDim->GetSize() ) // trouble
                                    StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
                                pNode =  pDim->Get(i)->GetExprNode();
                                lb = ub;
                                ub = pNode->GetNumber();
                            }
                            else if ( !bCompatible )
                                ub += nBase;
                            pArray->AddDim32( lb, ub );
                        }
                        pArray->setHasFixedSize( true );
                    }
                    else
                        pArray->unoAddDim( 0, -1 ); // variant array
                    sal_uInt16 nSavFlags = pTypeElem->GetFlags();
                    // need to reset the FIXED flag
                    // when calling PutObject ( because the type will not match Object )
                    pTypeElem->ResetFlag( SBX_FIXED );
                    pTypeElem->PutObject( pArray );
                    pTypeElem->SetFlags( nSavFlags );
                }
                // Nested user type?
                if( eElemType == SbxOBJECT )
                {
                    sal_uInt16 nElemTypeId = pElem->GetTypeId();
                    if( nElemTypeId != 0 )
                    {
                        OUString aTypeName( aGblStrings.Find( nElemTypeId ) );
                        SbxObject* pTypeObj = static_cast< SbxObject* >( rTypeArray->Find( aTypeName, SbxCLASS_OBJECT ) );
                        if( pTypeObj != NULL )
                        {
                            SbxObject* pCloneObj = cloneTypeObjectImpl( *pTypeObj );
                            pTypeElem->PutObject( pCloneObj );
                        }
                    }
                }
                pTypeMembers->Insert( pTypeElem, pTypeMembers->Count() );
            }
            delete pDim, pDim = NULL;
            delete pElem;
        }
    }

    pType->Remove( OUString("Name"), SbxCLASS_DONTCARE );
    pType->Remove( OUString("Parent"), SbxCLASS_DONTCARE );

    rTypeArray->Insert (pType,rTypeArray->Count());
}


// Declaration of Enum type

void SbiParser::Enum()
{
    DefEnum( false );
}

void SbiParser::DefEnum( bool bPrivate )
{
    // Read a the new Token. It had to be a symbol
    if (!TestSymbol())
        return;

    OUString aEnumName = aSym;
    if( rEnumArray->Find(aEnumName,SbxCLASS_OBJECT) )
    {
        Error( SbERR_VAR_DEFINED, aSym );
        return;
    }

    SbxObject *pEnum = new SbxObject( aEnumName );
    if( bPrivate )
    {
        pEnum->SetFlag( SBX_PRIVATE );
    }
    SbiSymDef* pElem;
    SbiDimList* pDim;
    bool bDone = false;

    // Starting with -1 to make first default value 0 after ++
    sal_Int32 nCurrentEnumValue = -1;
    while( !bDone && !IsEof() )
    {
        switch( Peek() )
        {
            case ENDENUM :
                pElem = NULL;
                bDone = true;
                Next();
            break;

            case EOLN :
            case REM :
                pElem = NULL;
                Next();
            break;

            default:
            {
                // TODO: Check existing!
                bool bDefined = false;

                pDim = NULL;
                pElem = VarDecl( &pDim, false, true );
                if( !pElem )
                {
                    bDone = true;   // Error occurred
                    break;
                }
                else if( pDim )
                {
                    delete pDim;
                    Error( SbERR_SYNTAX );
                    bDone = true;   // Error occurred
                    break;
                }

                SbiExpression aVar( this, *pElem );
                if( Peek() == EQ )
                {
                    Next();

                    SbiConstExpression aExpr( this );
                    if( !bDefined && aExpr.IsValid() )
                    {
                        SbxVariableRef xConvertVar = new SbxVariable();
                        if( aExpr.GetType() == SbxSTRING )
                            xConvertVar->PutString( aExpr.GetString() );
                        else
                            xConvertVar->PutDouble( aExpr.GetValue() );

                        nCurrentEnumValue = xConvertVar->GetLong();
                    }
                }
                else
                    nCurrentEnumValue++;

                SbiSymPool* pPoolToUse = bPrivate ? pPool : &aGlobals;

                SbiSymDef* pOld = pPoolToUse->Find( pElem->GetName() );
                if( pOld )
                {
                    Error( SbERR_VAR_DEFINED, pElem->GetName() );
                    bDone = true;   // Error occurred
                    break;
                }

                pPool->Add( pElem );

                if( !bPrivate )
                {
                    SbiOpcode eOp = _GLOBAL;
                    aGen.BackChain( nGblChain );
                    nGblChain = 0;
                    bGblDefs = bNewGblDefs = true;
                    aGen.Gen(
                        eOp, pElem->GetId(),
                        sal::static_int_cast< sal_uInt16 >( pElem->GetType() ) );

                    aVar.Gen();
                    sal_uInt16 nStringId = aGen.GetParser()->aGblStrings.Add( nCurrentEnumValue, SbxLONG );
                    aGen.Gen( _NUMBER, nStringId );
                    aGen.Gen( _PUTC );
                }

                SbiConstDef* pConst = pElem->GetConstDef();
                pConst->Set( nCurrentEnumValue, SbxLONG );
            }
        }
        if( pElem )
        {
            SbxArray *pEnumMembers = pEnum->GetProperties();
            SbxProperty *pEnumElem = new SbxProperty( pElem->GetName(), SbxLONG );
            pEnumElem->PutLong( nCurrentEnumValue );
            pEnumElem->ResetFlag( SBX_WRITE );
            pEnumElem->SetFlag( SBX_CONST );
            pEnumMembers->Insert( pEnumElem, pEnumMembers->Count() );
        }
    }

    pEnum->Remove( OUString("Name"), SbxCLASS_DONTCARE );
    pEnum->Remove( OUString("Parent"), SbxCLASS_DONTCARE );

    rEnumArray->Insert( pEnum, rEnumArray->Count() );
}


// Procedure-Declaration
// the first Token is already read in (SUB/FUNCTION)
// xxx Name [LIB "name"[ALIAS "name"]][(Parameter)][AS TYPE]

SbiProcDef* SbiParser::ProcDecl( bool bDecl )
{
    bool bFunc = ( eCurTok == FUNCTION );
    bool bProp = ( eCurTok == GET || eCurTok == SET || eCurTok == LET );
    if( !TestSymbol() ) return NULL;
    OUString aName( aSym );
    SbxDataType eType = eScanType;
    SbiProcDef* pDef = new SbiProcDef( this, aName, true );
    pDef->SetType( eType );
    if( Peek() == _CDECL_ )
    {
        Next(); pDef->SetCdecl();
    }
    if( Peek() == LIB )
    {
        Next();
        if( Next() == FIXSTRING )
        {
            pDef->GetLib() = aSym;
        }
        else
        {
            Error( SbERR_SYNTAX );
        }
    }
    if( Peek() == ALIAS )
    {
        Next();
        if( Next() == FIXSTRING )
        {
            pDef->GetAlias() = aSym;
        }
        else
        {
            Error( SbERR_SYNTAX );
        }
    }
    if( !bDecl )
    {
        // CDECL, LIB and ALIAS are invalid
        if( !pDef->GetLib().isEmpty() )
        {
            Error( SbERR_UNEXPECTED, LIB );
        }
        if( !pDef->GetAlias().isEmpty() )
        {
            Error( SbERR_UNEXPECTED, ALIAS );
        }
        if( pDef->IsCdecl() )
        {
            Error( SbERR_UNEXPECTED, _CDECL_ );
        }
        pDef->SetCdecl( false );
        pDef->GetLib() = "";
        pDef->GetAlias() = "";
    }
    else if( pDef->GetLib().isEmpty() )
    {
        // ALIAS and CDECL only together with LIB
        if( !pDef->GetAlias().isEmpty() )
        {
            Error( SbERR_UNEXPECTED, ALIAS );
        }
        if( pDef->IsCdecl() )
        {
            Error( SbERR_UNEXPECTED, _CDECL_ );
        }
        pDef->SetCdecl( false );
        pDef->GetAlias() = "";
    }
    // Brackets?
    if( Peek() == LPAREN )
    {
        Next();
        if( Peek() == RPAREN )
        {
            Next();
        }
        else
        {
            for(;;)
            {
                bool bByVal = false;
                bool bOptional = false;
                bool bParamArray = false;
                while( Peek() == BYVAL || Peek() == BYREF || Peek() == _OPTIONAL_ )
                {
                    if( Peek() == BYVAL )
                    {
                        bByVal = true;
                    }
                    else if ( Peek() == BYREF )
                    {
                        bByVal = false;
                    }
                    else if ( Peek() == _OPTIONAL_ )
                    {
                        bOptional = true;
                    }
                    Next();
                }
                if( bCompatible && Peek() == PARAMARRAY )
                {
                    if( bByVal || bOptional )
                    {
                        Error( SbERR_UNEXPECTED, PARAMARRAY );
                    }
                    Next();
                    bParamArray = true;
                }
                SbiSymDef* pPar = VarDecl( NULL, false, false );
                if( !pPar )
                {
                    break;
                }
                if( bByVal )
                {
                    pPar->SetByVal();
                }
                if( bOptional )
                {
                    pPar->SetOptional();
                }
                if( bParamArray )
                {
                    pPar->SetParamArray();
                }
                pDef->GetParams().Add( pPar );
                SbiToken eTok = Next();
                if( eTok != COMMA && eTok != RPAREN )
                {
                    bool bError2 = true;
                    if( bOptional && bCompatible && eTok == EQ )
                    {
                        SbiConstExpression* pDefaultExpr = new SbiConstExpression( this );
                        SbxDataType eType2 = pDefaultExpr->GetType();

                        sal_uInt16 nStringId;
                        if( eType2 == SbxSTRING )
                        {
                            nStringId = aGblStrings.Add( pDefaultExpr->GetString() );
                        }
                        else
                        {
                            nStringId = aGblStrings.Add( pDefaultExpr->GetValue(), eType2 );
                        }
                        pPar->SetDefaultId( nStringId );
                        delete pDefaultExpr;

                        eTok = Next();
                        if( eTok == COMMA || eTok == RPAREN )
                        {
                            bError2 = false;
                        }
                    }
                    if( bError2 )
                    {
                        Error( SbERR_EXPECTED, RPAREN );
                        break;
                    }
                }
                if( eTok == RPAREN )
                {
                    break;
                }
            }
        }
    }
    TypeDecl( *pDef );
    if( eType != SbxVARIANT && pDef->GetType() != eType )
    {
        Error( SbERR_BAD_DECLARATION, aName );
    }
    if( pDef->GetType() == SbxVARIANT && !( bFunc || bProp ) )
    {
        pDef->SetType( SbxEMPTY );
    }
    return pDef;
}

// DECLARE

void SbiParser::Declare()
{
    DefDeclare( false );
}

void SbiParser::DefDeclare( bool bPrivate )
{
    Next();
    if( eCurTok != SUB && eCurTok != FUNCTION )
    {
      Error( SbERR_UNEXPECTED, eCurTok );
    }
    else
    {
        bool bFunction = (eCurTok == FUNCTION);

        SbiProcDef* pDef = ProcDecl( true );
        if( pDef )
        {
            if( pDef->GetLib().isEmpty() )
            {
                Error( SbERR_EXPECTED, LIB );
            }
            // Is it already there?
            SbiSymDef* pOld = aPublics.Find( pDef->GetName() );
            if( pOld )
            {
                SbiProcDef* p = pOld->GetProcDef();
                if( !p )
                {
                    // Declared as a variable
                    Error( SbERR_BAD_DECLARATION, pDef->GetName() );
                    delete pDef;
                    pDef = NULL;
                }
                else
                {
                    pDef->Match( p );
                }
            }
            else
            {
                aPublics.Add( pDef );
            }
            if ( pDef )
            {
                pDef->SetPublic( !bPrivate );

                // New declare handling
                if( !pDef->GetLib().isEmpty())
                {
                    if( bNewGblDefs && nGblChain == 0 )
                    {
                        nGblChain = aGen.Gen( _JUMP, 0 );
                        bNewGblDefs = false;
                    }

                    sal_uInt16 nSavLine = nLine;
                    aGen.Statement();
                    pDef->Define();
                    pDef->SetLine1( nSavLine );
                    pDef->SetLine2( nSavLine );

                    SbiSymPool& rPool = pDef->GetParams();
                    sal_uInt16 nParCount = rPool.GetSize();

                    SbxDataType eType = pDef->GetType();
                    if( bFunction )
                    {
                        aGen.Gen( _PARAM, 0, sal::static_int_cast< sal_uInt16 >( eType ) );
                    }
                    if( nParCount > 1 )
                    {
                        aGen.Gen( _ARGC );

                        for( sal_uInt16 i = 1 ; i < nParCount ; ++i )
                        {
                            SbiSymDef* pParDef = rPool.Get( i );
                            SbxDataType eParType = pParDef->GetType();

                            aGen.Gen( _PARAM, i, sal::static_int_cast< sal_uInt16 >( eParType ) );
                            aGen.Gen( _ARGV );

                            sal_uInt16 nTyp = sal::static_int_cast< sal_uInt16 >( pParDef->GetType() );
                            if( pParDef->IsByVal() )
                            {
                                // Reset to avoid additional byval in call to wrapper function
                                pParDef->SetByVal( sal_False );
                                nTyp |= 0x8000;
                            }
                            aGen.Gen( _ARGTYP, nTyp );
                        }
                    }

                    aGen.Gen( _LIB, aGblStrings.Add( pDef->GetLib() ) );

                    SbiOpcode eOp = pDef->IsCdecl() ? _CALLC : _CALL;
                    sal_uInt16 nId = pDef->GetId();
                    if( !pDef->GetAlias().isEmpty() )
                    {
                        nId = ( nId & 0x8000 ) | aGblStrings.Add( pDef->GetAlias() );
                    }
                    if( nParCount > 1 )
                    {
                        nId |= 0x8000;
                    }
                    aGen.Gen( eOp, nId, sal::static_int_cast< sal_uInt16 >( eType ) );

                    if( bFunction )
                    {
                        aGen.Gen( _PUT );
                    }
                    aGen.Gen( _LEAVE );
                }
            }
        }
    }
}

void SbiParser::Attribute()
{
    // TODO: Need to implement the method as an attributed object.
    while( Next() != EQ )
    {
        if( Next() != DOT)
        {
            break;
        }
    }

    if( eCurTok != EQ )
    {
        Error( SbERR_SYNTAX );
    }
    else
    {
        SbiExpression aValue( this );
    }
    // Don't generate any code - just discard it.
}

// Call of a SUB or a FUNCTION

void SbiParser::Call()
{
    SbiExpression aVar( this, SbSYMBOL );
    aVar.Gen( FORCE_CALL );
    aGen.Gen( _GET );
}

// SUB/FUNCTION

void SbiParser::SubFunc()
{
    DefProc( false, false );
}

// Read in of a procedure

void SbiParser::DefProc( bool bStatic, bool bPrivate )
{
    sal_uInt16 l1 = nLine, l2 = nLine;
    bool bSub = ( eCurTok == SUB );
    bool bProperty = ( eCurTok == PROPERTY );
    PropertyMode ePropertyMode = PROPERTY_MODE_NONE;
    if( bProperty )
    {
        Next();
        if( eCurTok == GET )
        {
            ePropertyMode = PROPERTY_MODE_GET;
        }
        else if( eCurTok == LET )
        {
            ePropertyMode = PROPERTY_MODE_LET;
        }
        else if( eCurTok == SET )
        {
            ePropertyMode = PROPERTY_MODE_SET;
        }
        else
        {
            Error( SbERR_EXPECTED, "Get or Let or Set" );
        }
    }

    SbiToken eExit = eCurTok;
    SbiProcDef* pDef = ProcDecl( false );
    if( !pDef )
    {
        return;
    }
    pDef->setPropertyMode( ePropertyMode );

    // Is the Proc already declared?
    SbiSymDef* pOld = aPublics.Find( pDef->GetName() );
    if( pOld )
    {
        bool bError_ = false;

        pProc = pOld->GetProcDef();
        if( !pProc )
        {
            // Declared as a variable
            Error( SbERR_BAD_DECLARATION, pDef->GetName() );
            delete pDef;
            pProc = NULL;
            bError_ = true;
        }
        // #100027: Multiple declaration -> Error
        // #112787: Not for setup, REMOVE for 8
        else if( pProc->IsUsedForProcDecl() )
        {
            PropertyMode ePropMode = pDef->getPropertyMode();
            if( ePropMode == PROPERTY_MODE_NONE || ePropMode == pProc->getPropertyMode() )
            {
                Error( SbERR_PROC_DEFINED, pDef->GetName() );
                delete pDef;
                pProc = NULL;
                bError_ = true;
            }
        }

        if( !bError_ )
        {
            pDef->Match( pProc );
            pProc = pDef;
        }
    }
    else
    {
        aPublics.Add( pDef ), pProc = pDef;
    }
    if( !pProc )
    {
        return;
    }
    pProc->SetPublic( !bPrivate );

    // Now we set the search hierarchy for symbols as well as the
    // current procedure.
    aPublics.SetProcId( pProc->GetId() );
    pProc->GetParams().SetParent( &aPublics );
    if( bStatic )
    {
        if ( bVBASupportOn )
        {
            pProc->SetStatic( sal_True );
        }
        else
        {
            Error( SbERR_NOT_IMPLEMENTED ); // STATIC SUB ...
        }
    }
    else
    {
        pProc->SetStatic( sal_False );
    }
    // Normal case: Local variable->parameter->global variable
    pProc->GetLocals().SetParent( &pProc->GetParams() );
    pPool = &pProc->GetLocals();

    pProc->Define();
    OpenBlock( eExit );
    StmntBlock( bSub ? ENDSUB : (bProperty ? ENDPROPERTY : ENDFUNC) );
    l2 = nLine;
    pProc->SetLine1( l1 );
    pProc->SetLine2( l2 );
    pPool = &aPublics;
    aPublics.SetProcId( 0 );
    // Open labels?
    pProc->GetLabels().CheckRefs();
    CloseBlock();
    aGen.Gen( _LEAVE );
    pProc = NULL;
}

// STATIC variable|procedure

void SbiParser::Static()
{
    DefStatic( false );
}

void SbiParser::DefStatic( bool bPrivate )
{
    SbiSymPool* p;

    switch( Peek() )
    {
    case SUB:
    case FUNCTION:
    case PROPERTY:
        // End global chain if necessary (not done in
        // SbiParser::Parse() under these conditions
        if( bNewGblDefs && nGblChain == 0 )
        {
            nGblChain = aGen.Gen( _JUMP, 0 );
            bNewGblDefs = false;
        }
        Next();
        DefProc( true, bPrivate );
        break;
    default:
        if( !pProc )
        {
            Error( SbERR_NOT_IN_SUBR );
        }
        // Reset the Pool, so that STATIC-Declarations go into the
        // global Pool
        p = pPool;
        pPool = &aPublics;
        DefVar( _STATIC, true );
        pPool = p;
        break;
    }
}

bool SbiParser::IsUnoInterface(const OUString& sTypeName)
{
    try
    {
        Reference< lang::XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory(), UNO_SET_THROW );
        Reference< reflection::XIdlReflection > xRefl( xFactory->createInstance("com.sun.star.reflection.CoreReflection"), UNO_QUERY_THROW );
        //DBG_ASSERT(xRefl.Is(), "No reflection class!"); ???
        if( !xRefl.is() )
        {
            return false;
        }
        Reference< reflection::XIdlClass > xClass = xRefl->forName(sTypeName);
        if( xClass != NULL )
        {
            return true;
        }
        return false;
    }
    catch(const Exception& ex)
    {
        OSL_FAIL("Could not create reflection.CoreReflection.");
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
