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
#include <sbunoobj.hxx>
#include <parser.hxx>
#include <svtools/miscopt.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/reflection/XInterfaceMemberTypeDescription.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <basic/codecompletecache.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

// Declaration of a variable
// If there are errors it will be parsed up to the comma or the newline.
// Return-value: a new instance, which were inserted and then deleted.
// Array-Index were returned as SbiExprList

SbiSymDef* SbiParser::VarDecl( SbiExprListPtr* ppDim, bool bStatic, bool bConst )
{
    bool bWithEvents = false;
    if( Peek() == WITHEVENTS )
    {
        Next();
        bWithEvents = true;
    }
    if( !TestSymbol() ) return nullptr;
    SbxDataType t = eScanType;
    SbiSymDef* pDef = bConst ? new SbiConstDef( aSym ) : new SbiSymDef( aSym );
    SbiExprListPtr pDim;
    // Brackets?
    if( Peek() == LPAREN )
    {
        pDim = SbiExprList::ParseDimList( this );
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
            Error( ERRCODE_BASIC_EXPECTED, "()" );
    }
    else if( ppDim )
        *ppDim = std::move(pDim);
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
                    Error( ERRCODE_BASIC_SYNTAX );
                eType = SbxVARIANT; break;
            case TINTEGER:
            case TLONG:
            case TSINGLE:
            case TDOUBLE:
            case TCURRENCY:
            case TDATE:
            case TSTRING:
            case TOBJECT:
            case ERROR_:
            case TBOOLEAN:
            case TVARIANT:
            case TBYTE:
                if( rDef.IsNew() )
                    Error( ERRCODE_BASIC_SYNTAX );
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
                            Error( ERRCODE_BASIC_OUT_OF_RANGE );
                        else
                            rDef.SetFixedStringLength( nSize );
                    }
                }
                break;
            case SYMBOL: // can only be a TYPE or a object class!
                if( eScanType != SbxVARIANT )
                    Error( ERRCODE_BASIC_SYNTAX );
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
                                Error( ERRCODE_BASIC_UNEXPECTED, SYMBOL );
                                break;
                            }
                        }
                    }
                    else if( rEnumArray->Find( aCompleteName, SbxClassType::Object ) || ( IsVBASupportOn() && VBAConstantHelper::instance().isVBAConstantType( aCompleteName ) ) )
                    {
                        eType = SbxLONG;
                        break;
                    }

                    // Take over in the string pool
                    rDef.SetTypeId( aGblStrings.Add( aCompleteName ) );

                    if( rDef.IsNew() && pProc == nullptr )
                        aRequiredTypes.push_back( aCompleteName );
                }
                eType = SbxOBJECT;
                break;
            case FIXSTRING: // new syntax for complex UNO types
                rDef.SetTypeId( aGblStrings.Add( aSym ) );
                eType = SbxOBJECT;
                break;
            default:
                Error( ERRCODE_BASIC_UNEXPECTED, eTok );
                Next();
        }
        // The variable could have been declared with a suffix
        if( rDef.GetType() != SbxVARIANT )
        {
            if( rDef.GetType() != eType )
                Error( ERRCODE_BASIC_VAR_DEFINED, rDef.GetName() );
            else if( eType == SbxSTRING && rDef.GetLen() != nSize )
                Error( ERRCODE_BASIC_VAR_DEFINED, rDef.GetName() );
        }
        rDef.SetType( eType );
        rDef.SetLen( nSize );
    }
}

// Here variables, arrays and structures were defined.
// DIM/PRIVATE/PUBLIC/GLOBAL

void SbiParser::Dim()
{
    DefVar( SbiOpcode::DIM_, pProc && bVBASupportOn && pProc->IsStatic() );
}

void SbiParser::DefVar( SbiOpcode eOp, bool bStatic )
{
    SbiSymPool* pOldPool = pPool;
    bool bSwitchPool = false;
    bool bPersistentGlobal = false;
    SbiToken eFirstTok = eCurTok;

    if( pProc && ( eCurTok == GLOBAL || eCurTok == PUBLIC || eCurTok == PRIVATE ) )
        Error( ERRCODE_BASIC_NOT_IN_SUBR, eCurTok );
    if( eCurTok == PUBLIC || eCurTok == GLOBAL )
    {
        bSwitchPool = true;     // at the right moment switch to the global pool
        if( eCurTok == GLOBAL )
            bPersistentGlobal = true;
    }
    // behavior in VBA is that a module scope variable's lifetime is
    // tied to the document. e.g. a module scope variable is global
    if(  GetBasic()->IsDocBasic() && bVBASupportOn && !pProc )
        bPersistentGlobal = true;
    // PRIVATE is a synonymous for DIM
    // _CONST_?
    bool bConst = false;
    if( eCurTok == CONST_ )
        bConst = true;
    else if( Peek() == CONST_ )
    {
        Next();
        bConst = true;
    }

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
                nGblChain = aGen.Gen( SbiOpcode::JUMP_, 0 );
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
            DefType(); // TODO: Use bPrivate in DefType()
            return;
        }
    }

    // SHARED were ignored
    if( Peek() == SHARED ) Next();

    // PRESERVE only at REDIM
    if( Peek() == PRESERVE )
    {
        Next();
        if( eOp == SbiOpcode::REDIM_ )
            eOp = SbiOpcode::REDIMP_;
        else
            Error( ERRCODE_BASIC_UNEXPECTED, eCurTok );
    }
    SbiSymDef* pDef;
    SbiExprListPtr pDim;

    // #40689, Statics -> Module-Initialising, skip in Sub
    sal_uInt32 nEndOfStaticLbl = 0;
    if( !bVBASupportOn && bStatic )
    {
        nEndOfStaticLbl = aGen.Gen( SbiOpcode::JUMP_, 0 );
        aGen.Statement();   // catch up on static here
    }

    bool bDefined = false;
    while( ( pDef = VarDecl( &pDim, bStatic, bConst ) ) != nullptr )
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
        if( pOld && !(eOp == SbiOpcode::REDIM_ || eOp == SbiOpcode::REDIMP_) )
        {
            if( pDef->GetScope() == SbLOCAL && pOld->GetScope() != SbLOCAL )
                pOld = nullptr;
        }
        if( pOld )
        {
            bDefined = true;
            // always an error at a RTL-S
            if( !bRtlSym && (eOp == SbiOpcode::REDIM_ || eOp == SbiOpcode::REDIMP_) )
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
                    Error( ERRCODE_BASIC_VAR_DEFINED, pDef->GetName() );
            }
            else
                Error( ERRCODE_BASIC_VAR_DEFINED, pDef->GetName() );
            delete pDef; pDef = pOld;
        }
        else
            pPool->Add( pDef );

        // #36374: Create the variable in front of the distinction IsNew()
        // Otherwise error at Dim Identifier As New Type and option explicit
        if( !bDefined && !(eOp == SbiOpcode::REDIM_ || eOp == SbiOpcode::REDIMP_)
                      && ( !bConst || pDef->GetScope() == SbGLOBAL ) )
        {
            // Declare variable or global constant
            SbiOpcode eOp2;
            switch ( pDef->GetScope() )
            {
                case SbGLOBAL:  eOp2 = bPersistentGlobal ? SbiOpcode::GLOBAL_P_ : SbiOpcode::GLOBAL_;
                                goto global;
                case SbPUBLIC:  eOp2 = bPersistentGlobal ? SbiOpcode::PUBLIC_P_ : SbiOpcode::PUBLIC_;
                                // #40689, no own Opcode anymore
                                if( bVBASupportOn && bStatic )
                                {
                                    eOp2 = SbiOpcode::STATIC_;
                                    break;
                                }
                global:         aGen.BackChain( nGblChain );
                                nGblChain = 0;
                                bGblDefs = bNewGblDefs = true;
                                break;
                default:        eOp2 = SbiOpcode::LOCAL_;
            }
            sal_uInt32 nOpnd2 = sal::static_int_cast< sal_uInt16 >( pDef->GetType() );
            if( pDef->IsWithEvents() )
                nOpnd2 |= SBX_TYPE_WITH_EVENTS_FLAG;

            if( bCompatible && pDef->IsNew() )
                nOpnd2 |= SBX_TYPE_DIM_AS_NEW_FLAG;

            short nFixedStringLength = pDef->GetFixedStringLength();
            if( nFixedStringLength >= 0 )
                nOpnd2 |= (SBX_FIXED_LEN_STRING_FLAG + (sal_uInt32(nFixedStringLength) << 17));     // len = all bits above 0x10000

            if( pDim != nullptr && pDim->GetDims() > 0 )
                nOpnd2 |= SBX_TYPE_VAR_TO_DIM_FLAG;

            aGen.Gen( eOp2, pDef->GetId(), nOpnd2 );
        }

        // Initialising for self-defined data types
        // and per NEW created variable
        if( pDef->GetType() == SbxOBJECT
         && pDef->GetTypeId() )
        {
            if( !bCompatible && !pDef->IsNew() )
            {
                OUString aTypeName( aGblStrings.Find( pDef->GetTypeId() ) );
                if( rTypeArray->Find( aTypeName, SbxClassType::Object ) == nullptr )
                {
                    if( CodeCompleteOptions::IsExtendedTypeDeclaration() )
                    {
                        if(!IsUnoInterface(aTypeName))
                            Error( ERRCODE_BASIC_UNDEF_TYPE, aTypeName );
                    }
                    else
                        Error( ERRCODE_BASIC_UNDEF_TYPE, aTypeName );
                }
            }

            if( bConst )
            {
                Error( ERRCODE_BASIC_SYNTAX );
            }

            if( pDim )
            {
                if( eOp == SbiOpcode::REDIMP_ )
                {
                    SbiExpression aExpr( this, *pDef, nullptr );
                    aExpr.Gen();
                    aGen.Gen( SbiOpcode::REDIMP_ERASE_ );

                    pDef->SetDims( pDim->GetDims() );
                    SbiExpression aExpr2( this, *pDef, std::move(pDim) );
                    aExpr2.Gen();
                    aGen.Gen( SbiOpcode::DCREATE_REDIMP_, pDef->GetId(), pDef->GetTypeId() );
                }
                else
                {
                    pDef->SetDims( pDim->GetDims() );
                    SbiExpression aExpr( this, *pDef, std::move(pDim) );
                    aExpr.Gen();
                    aGen.Gen( SbiOpcode::DCREATE_, pDef->GetId(), pDef->GetTypeId() );
                }
            }
            else
            {
                SbiExpression aExpr( this, *pDef );
                aExpr.Gen();
                SbiOpcode eOp_ = pDef->IsNew() ? SbiOpcode::CREATE_ : SbiOpcode::TCREATE_;
                aGen.Gen( eOp_, pDef->GetId(), pDef->GetTypeId() );
                if ( bVBASupportOn )
                    aGen.Gen( SbiOpcode::VBASET_ );
                else
                    aGen.Gen( SbiOpcode::SET_ );
            }
        }
        else
        {
            if( bConst )
            {
                // Definition of the constants
                if( pDim )
                {
                    Error( ERRCODE_BASIC_SYNTAX );
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
                        aGen.Gen( SbiOpcode::PUTC_ );
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
                if( eOp == SbiOpcode::REDIM_ )
                {
                    SbiExpression aExpr( this, *pDef, nullptr );
                    aExpr.Gen();
                    if ( bVBASupportOn )
                        // delete the array but
                        // clear the variable ( this
                        // allows the processing of
                        // the param to happen as normal without errors ( ordinary ERASE just clears the array )
                        aGen.Gen( SbiOpcode::ERASE_CLEAR_ );
                    else
                        aGen.Gen( SbiOpcode::ERASE_ );
                }
                else if( eOp == SbiOpcode::REDIMP_ )
                {
                    SbiExpression aExpr( this, *pDef, nullptr );
                    aExpr.Gen();
                    aGen.Gen( SbiOpcode::REDIMP_ERASE_ );
                }
                pDef->SetDims( pDim->GetDims() );
                if( bPersistentGlobal )
                    pDef->SetGlobal( true );
                SbiExpression aExpr( this, *pDef, std::move(pDim) );
                aExpr.Gen();
                pDef->SetGlobal( false );
                aGen.Gen( (eOp == SbiOpcode::STATIC_) ? SbiOpcode::DIM_ : eOp );
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
        nGblChain = aGen.Gen( SbiOpcode::JUMP_, 0 );
        bGblDefs = bNewGblDefs = true;

        // Register for Sub a jump to the end of statics
        aGen.BackChain( nEndOfStaticLbl );
    }

}

// Here were Arrays redimensioned.

void SbiParser::ReDim()
{
    DefVar( SbiOpcode::REDIM_, pProc && bVBASupportOn && pProc->IsStatic() );
}

// ERASE array, ...

void SbiParser::Erase()
{
    while( !bAbort )
    {
        SbiExpression aExpr( this, SbLVALUE );
        aExpr.Gen();
        aGen.Gen( SbiOpcode::ERASE_ );
        if( !TestComma() ) break;
    }
}

// Declaration of a data type

void SbiParser::Type()
{
    DefType();
}

void SbiParser::DefType()
{
    // Read the new Token lesen. It had to be a symbol
    if (!TestSymbol())
        return;

    if (rTypeArray->Find(aSym,SbxClassType::Object))
    {
        Error( ERRCODE_BASIC_VAR_DEFINED, aSym );
        return;
    }

    SbxObject *pType = new SbxObject(aSym);

    bool bDone = false;

    while( !bDone && !IsEof() )
    {
        std::unique_ptr<SbiSymDef> pElem;
        SbiExprListPtr pDim;
        switch( Peek() )
        {
            case ENDTYPE :
                bDone = true;
                Next();
            break;

            case EOLN :
            case REM :
                Next();
            break;

            default:
                pElem.reset(VarDecl(&pDim, false, false));
                if( !pElem )
                    bDone = true;   // Error occurred
        }
        if( pElem )
        {
            SbxArray *pTypeMembers = pType->GetProperties();
            OUString aElemName = pElem->GetName();
            if( pTypeMembers->Find( aElemName, SbxClassType::DontCare) )
            {
                Error (ERRCODE_BASIC_VAR_DEFINED);
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
                                    StarBASIC::FatalError( ERRCODE_BASIC_INTERNAL_ERROR );
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
                    SbxFlagBits nSavFlags = pTypeElem->GetFlags();
                    // need to reset the FIXED flag
                    // when calling PutObject ( because the type will not match Object )
                    pTypeElem->ResetFlag( SbxFlagBits::Fixed );
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
                        SbxObject* pTypeObj = static_cast< SbxObject* >( rTypeArray->Find( aTypeName, SbxClassType::Object ) );
                        if( pTypeObj != nullptr )
                        {
                            SbxObject* pCloneObj = cloneTypeObjectImpl( *pTypeObj );
                            pTypeElem->PutObject( pCloneObj );
                        }
                    }
                }
                pTypeMembers->Insert( pTypeElem, pTypeMembers->Count() );
            }
        }
    }

    pType->Remove( "Name", SbxClassType::DontCare );
    pType->Remove( "Parent", SbxClassType::DontCare );

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
    if( rEnumArray->Find(aEnumName,SbxClassType::Object) )
    {
        Error( ERRCODE_BASIC_VAR_DEFINED, aSym );
        return;
    }

    SbxObject *pEnum = new SbxObject( aEnumName );
    if( bPrivate )
    {
        pEnum->SetFlag( SbxFlagBits::Private );
    }
    SbiSymDef* pElem;
    bool bDone = false;

    // Starting with -1 to make first default value 0 after ++
    sal_Int32 nCurrentEnumValue = -1;
    while( !bDone && !IsEof() )
    {
        switch( Peek() )
        {
            case ENDENUM :
                pElem = nullptr;
                bDone = true;
                Next();
            break;

            case EOLN :
            case REM :
                pElem = nullptr;
                Next();
            break;

            default:
            {
                SbiExprListPtr pDim;
                pElem = VarDecl( &pDim, false, true );
                if( !pElem )
                {
                    bDone = true;   // Error occurred
                    break;
                }
                else if( pDim )
                {
                    Error( ERRCODE_BASIC_SYNTAX );
                    bDone = true;   // Error occurred
                    break;
                }

                SbiExpression aVar( this, *pElem );
                if( Peek() == EQ )
                {
                    Next();

                    SbiConstExpression aExpr( this );
                    if( aExpr.IsValid() )
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
                    Error( ERRCODE_BASIC_VAR_DEFINED, pElem->GetName() );
                    bDone = true;   // Error occurred
                    break;
                }

                pPool->Add( pElem );

                if( !bPrivate )
                {
                    aGen.BackChain( nGblChain );
                    nGblChain = 0;
                    bGblDefs = bNewGblDefs = true;
                    aGen.Gen(
                        SbiOpcode::GLOBAL_, pElem->GetId(),
                        sal::static_int_cast< sal_uInt16 >( pElem->GetType() ) );

                    aVar.Gen();
                    sal_uInt16 nStringId = aGen.GetParser()->aGblStrings.Add( nCurrentEnumValue, SbxLONG );
                    aGen.Gen( SbiOpcode::NUMBER_, nStringId );
                    aGen.Gen( SbiOpcode::PUTC_ );
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
            pEnumElem->ResetFlag( SbxFlagBits::Write );
            pEnumElem->SetFlag( SbxFlagBits::Const );
            pEnumMembers->Insert( pEnumElem, pEnumMembers->Count() );
        }
    }

    pEnum->Remove( "Name", SbxClassType::DontCare );
    pEnum->Remove( "Parent", SbxClassType::DontCare );

    rEnumArray->Insert( pEnum, rEnumArray->Count() );
}


// Procedure-Declaration
// the first Token is already read in (SUB/FUNCTION)
// xxx Name [LIB "name"[ALIAS "name"]][(Parameter)][AS TYPE]

SbiProcDef* SbiParser::ProcDecl( bool bDecl )
{
    bool bFunc = ( eCurTok == FUNCTION );
    bool bProp = ( eCurTok == GET || eCurTok == SET || eCurTok == LET );
    if( !TestSymbol() ) return nullptr;
    OUString aName( aSym );
    SbxDataType eType = eScanType;
    SbiProcDef* pDef = new SbiProcDef( this, aName, true );
    pDef->SetType( eType );
    if( Peek() == CDECL_ )
    {
        Next(); pDef->SetCdecl(true);
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
            Error( ERRCODE_BASIC_SYNTAX );
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
            Error( ERRCODE_BASIC_SYNTAX );
        }
    }
    if( !bDecl )
    {
        // CDECL, LIB and ALIAS are invalid
        if( !pDef->GetLib().isEmpty() )
        {
            Error( ERRCODE_BASIC_UNEXPECTED, LIB );
        }
        if( !pDef->GetAlias().isEmpty() )
        {
            Error( ERRCODE_BASIC_UNEXPECTED, ALIAS );
        }
        if( pDef->IsCdecl() )
        {
            Error( ERRCODE_BASIC_UNEXPECTED, CDECL_ );
        }
        pDef->SetCdecl( false );
        pDef->GetLib().clear();
        pDef->GetAlias().clear();
    }
    else if( pDef->GetLib().isEmpty() )
    {
        // ALIAS and CDECL only together with LIB
        if( !pDef->GetAlias().isEmpty() )
        {
            Error( ERRCODE_BASIC_UNEXPECTED, ALIAS );
        }
        if( pDef->IsCdecl() )
        {
            Error( ERRCODE_BASIC_UNEXPECTED, CDECL_ );
        }
        pDef->SetCdecl( false );
        pDef->GetAlias().clear();
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
                while( Peek() == BYVAL || Peek() == BYREF || Peek() == OPTIONAL_ )
                {
                    if( Peek() == BYVAL )
                    {
                        bByVal = true;
                    }
                    else if ( Peek() == BYREF )
                    {
                        bByVal = false;
                    }
                    else if ( Peek() == OPTIONAL_ )
                    {
                        bOptional = true;
                    }
                    Next();
                }
                if( bCompatible && Peek() == PARAMARRAY )
                {
                    if( bByVal || bOptional )
                    {
                        Error( ERRCODE_BASIC_UNEXPECTED, PARAMARRAY );
                    }
                    Next();
                    bParamArray = true;
                }
                SbiSymDef* pPar = VarDecl( nullptr, false, false );
                if( !pPar )
                {
                    break;
                }
                if( bByVal )
                {
                    pPar->SetByVal(true);
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
                        std::unique_ptr<SbiConstExpression> pDefaultExpr(new SbiConstExpression( this ));
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
                        pDefaultExpr.reset();

                        eTok = Next();
                        if( eTok == COMMA || eTok == RPAREN )
                        {
                            bError2 = false;
                        }
                    }
                    if( bError2 )
                    {
                        Error( ERRCODE_BASIC_EXPECTED, RPAREN );
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
        Error( ERRCODE_BASIC_BAD_DECLARATION, aName );
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
    if( eCurTok == PTRSAFE )
        Next();

    if( eCurTok != SUB && eCurTok != FUNCTION )
    {
      Error( ERRCODE_BASIC_UNEXPECTED, eCurTok );
    }
    else
    {
        bool bFunction = (eCurTok == FUNCTION);

        SbiProcDef* pDef = ProcDecl( true );
        if( pDef )
        {
            if( pDef->GetLib().isEmpty() )
            {
                Error( ERRCODE_BASIC_EXPECTED, LIB );
            }
            // Is it already there?
            SbiSymDef* pOld = aPublics.Find( pDef->GetName() );
            if( pOld )
            {
                SbiProcDef* p = pOld->GetProcDef();
                if( !p )
                {
                    // Declared as a variable
                    Error( ERRCODE_BASIC_BAD_DECLARATION, pDef->GetName() );
                    delete pDef;
                    pDef = nullptr;
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
                        nGblChain = aGen.Gen( SbiOpcode::JUMP_, 0 );
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
                        aGen.Gen( SbiOpcode::PARAM_, 0, sal::static_int_cast< sal_uInt16 >( eType ) );
                    }
                    if( nParCount > 1 )
                    {
                        aGen.Gen( SbiOpcode::ARGC_ );

                        for( sal_uInt16 i = 1 ; i < nParCount ; ++i )
                        {
                            SbiSymDef* pParDef = rPool.Get( i );
                            SbxDataType eParType = pParDef->GetType();

                            aGen.Gen( SbiOpcode::PARAM_, i, sal::static_int_cast< sal_uInt16 >( eParType ) );
                            aGen.Gen( SbiOpcode::ARGV_ );

                            sal_uInt16 nTyp = sal::static_int_cast< sal_uInt16 >( pParDef->GetType() );
                            if( pParDef->IsByVal() )
                            {
                                // Reset to avoid additional byval in call to wrapper function
                                pParDef->SetByVal( false );
                                nTyp |= 0x8000;
                            }
                            aGen.Gen( SbiOpcode::ARGTYP_, nTyp );
                        }
                    }

                    aGen.Gen( SbiOpcode::LIB_, aGblStrings.Add( pDef->GetLib() ) );

                    SbiOpcode eOp = pDef->IsCdecl() ? SbiOpcode::CALLC_ : SbiOpcode::CALL_;
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
                        aGen.Gen( SbiOpcode::PUT_ );
                    }
                    aGen.Gen( SbiOpcode::LEAVE_ );
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
        Error( ERRCODE_BASIC_SYNTAX );
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
    aGen.Gen( SbiOpcode::GET_ );
}

// SUB/FUNCTION

void SbiParser::SubFunc()
{
    DefProc( false, false );
}

// Read in of a procedure

void SbiParser::DefProc( bool bStatic, bool bPrivate )
{
    sal_uInt16 l1 = nLine;
    bool bSub = ( eCurTok == SUB );
    bool bProperty = ( eCurTok == PROPERTY );
    PropertyMode ePropertyMode = PropertyMode::NONE;
    if( bProperty )
    {
        Next();
        if( eCurTok == GET )
        {
            ePropertyMode = PropertyMode::Get;
        }
        else if( eCurTok == LET )
        {
            ePropertyMode = PropertyMode::Let;
        }
        else if( eCurTok == SET )
        {
            ePropertyMode = PropertyMode::Set;
        }
        else
        {
            Error( ERRCODE_BASIC_EXPECTED, "Get or Let or Set" );
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
        pProc = pOld->GetProcDef();
        if( !pProc )
        {
            // Declared as a variable
            Error( ERRCODE_BASIC_BAD_DECLARATION, pDef->GetName() );
            delete pDef;
            return;
        }
        // #100027: Multiple declaration -> Error
        // #112787: Not for setup, REMOVE for 8
        else if( pProc->IsUsedForProcDecl() )
        {
            PropertyMode ePropMode = pDef->getPropertyMode();
            if( ePropMode == PropertyMode::NONE || ePropMode == pProc->getPropertyMode() )
            {
                Error( ERRCODE_BASIC_PROC_DEFINED, pDef->GetName() );
                delete pDef;
                return;
            }
        }

        pDef->Match( pProc );
    }
    else
    {
        aPublics.Add( pDef );
    }
    assert(pDef);
    pProc = pDef;
    pProc->SetPublic( !bPrivate );

    // Now we set the search hierarchy for symbols as well as the
    // current procedure.
    aPublics.SetProcId( pProc->GetId() );
    pProc->GetParams().SetParent( &aPublics );
    if( bStatic )
    {
        if ( bVBASupportOn )
        {
            pProc->SetStatic();
        }
        else
        {
            Error( ERRCODE_BASIC_NOT_IMPLEMENTED ); // STATIC SUB ...
        }
    }
    else
    {
        pProc->SetStatic( false );
    }
    // Normal case: Local variable->parameter->global variable
    pProc->GetLocals().SetParent( &pProc->GetParams() );
    pPool = &pProc->GetLocals();

    pProc->Define();
    OpenBlock( eExit );
    StmntBlock( bSub ? ENDSUB : (bProperty ? ENDPROPERTY : ENDFUNC) );
    sal_uInt16 l2 = nLine;
    pProc->SetLine1( l1 );
    pProc->SetLine2( l2 );
    pPool = &aPublics;
    aPublics.SetProcId( 0 );
    // Open labels?
    pProc->GetLabels().CheckRefs();
    CloseBlock();
    aGen.Gen( SbiOpcode::LEAVE_ );
    pProc = nullptr;
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
            nGblChain = aGen.Gen( SbiOpcode::JUMP_, 0 );
            bNewGblDefs = false;
        }
        Next();
        DefProc( true, bPrivate );
        break;
    default:
        if( !pProc )
        {
            Error( ERRCODE_BASIC_NOT_IN_SUBR );
        }
        // Reset the Pool, so that STATIC-Declarations go into the
        // global Pool
        p = pPool;
        pPool = &aPublics;
        DefVar( SbiOpcode::STATIC_, true );
        pPool = p;
        break;
    }
}

bool SbiParser::IsUnoInterface(const OUString& sTypeName)
{
    try
    {
        return css::reflection::theCoreReflection::get(
            comphelper::getProcessComponentContext())->forName(sTypeName).is();
    }
    catch(const Exception&)
    {
        OSL_FAIL("Could not create reflection.CoreReflection.");
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
