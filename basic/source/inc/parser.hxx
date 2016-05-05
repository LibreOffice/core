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

#ifndef INCLUDED_BASIC_SOURCE_INC_PARSER_HXX
#define INCLUDED_BASIC_SOURCE_INC_PARSER_HXX

#include "expr.hxx"
#include "codegen.hxx"
#include "symtbl.hxx"


#include <vector>
typedef ::std::vector< OUString > StringVector;

struct SbiParseStack;

class SbiParser : public SbiTokenizer
{
    friend class SbiExpression;

    SbiParseStack* pStack;
    SbiProcDef* pProc;
    SbiExprNode*  pWithVar;
    SbiToken    eEndTok;
    sal_uInt32      nGblChain;          // for global DIMs
    bool        bGblDefs;           // true: global definitions general
    bool        bNewGblDefs;        // true: globale definitions before sub
    bool        bSingleLineIf;
    bool        bCodeCompleting;

    SbiSymDef*  VarDecl( SbiExprListPtr*, bool, bool );
    SbiProcDef* ProcDecl(bool bDecl);
    void DefStatic( bool bPrivate );
    void DefProc( bool bStatic, bool bPrivate ); // read in procedure
    void DefVar( SbiOpcode eOp, bool bStatic ); // read in DIM/REDIM
    void TypeDecl( SbiSymDef&, bool bAsNewAlreadyParsed=false );    // AS-declaration
    void OpenBlock( SbiToken, SbiExprNode* = nullptr );
    void CloseBlock();
    bool Channel( bool bAlways=false );     // parse channel number
    void StmntBlock( SbiToken );
    void DefType( bool bPrivate );  // Parse type declaration
    void DefEnum( bool bPrivate );  // Parse enum declaration
    void DefDeclare( bool bPrivate );
    void EnableCompatibility();
    static bool IsUnoInterface( const OUString& sTypeName );
public:
    SbxArrayRef   rTypeArray;
    SbxArrayRef   rEnumArray;
    SbiStringPool aGblStrings;      // string-pool
    SbiStringPool aLclStrings;      // string-pool
    SbiSymPool    aGlobals;
    SbiSymPool    aPublics;         // module global
    SbiSymPool    aRtlSyms;         // Runtime-Library
    SbiCodeGen    aGen;             // Code-Generator
    SbiSymPool*   pPool;
    SbiExprType   eCurExpr;
    short         nBase;            // OPTION BASE-value
    bool          bText;            // OPTION COMPARE TEXT
    bool          bExplicit;        // true: OPTION EXPLICIT
    bool          bClassModule;     // true: OPTION ClassModule
    StringVector  aIfaceVector;     // Holds all interfaces implemented by a class module
    StringVector  aRequiredTypes;   // Types used in Dim As New <type> outside subs
#   define N_DEF_TYPES 26
    SbxDataType   eDefTypes[N_DEF_TYPES];    // DEFxxx data types

    SbiParser( StarBASIC*, SbModule* );
    bool Parse();
    void SetCodeCompleting( bool b );
    bool IsCodeCompleting() const { return bCodeCompleting;}
    SbiExprNode* GetWithVar();

    // from 31.3.1996, search symbol in the runtime-library
    SbiSymDef* CheckRTLForSym( const OUString& rSym, SbxDataType eType );
    void AddConstants();

    bool HasGlobalCode();

    bool TestToken( SbiToken );
    bool TestSymbol();
    bool TestComma();
    void TestEoln();

    void Symbol( const KeywordSymbolInfo* pKeywordSymbolInfo = nullptr );  // let or call
    void ErrorStmnt();              // ERROR n
    void BadBlock();                // LOOP/WEND/NEXT
    void NoIf();                    // ELSE/ELSE IF without IF
    void Assign();                  // LET
    void Attribute();
    void Call();                    // CALL
    void Close();                   // CLOSE
    void Declare();                 // DECLARE
    void DefXXX();                  // DEFxxx
    void Dim();                     // DIM
    void ReDim();                   // ReDim();
    void Erase();                   // ERASE
    void Exit();                    // EXIT
    void For();                     // FOR...NEXT
    void Goto();                    // GOTO / GOSUB
    void If();                      // IF
    void Implements();              // IMPLEMENTS
    void Input();                   // INPUT, INPUT #
    void Line();                    // LINE -> LINE INPUT [#] (#i92642)
    void LineInput();               // LINE INPUT, LINE INPUT #
    void LSet();                    // LSET
    void Name();                    // NAME .. AS ..
    void On();                      // ON ERROR/variable
    void OnGoto();                  // ON...GOTO / GOSUB
    void Open();                    // OPEN
    void Option();                  // OPTION
    void Print();                   // PRINT, PRINT #
    void SubFunc();                 // SUB / FUNCTION
    void Resume();                  // RESUME
    void Return();                  // RETURN
    void RSet();                    // RSET
    void DoLoop();                  // DO...LOOP
    void Select();                  // SELECT ... CASE
    void Set();                     // SET
    void Static();                  // STATIC
    void Stop();                    // STOP/SYSTEM
    void Type();                    // TYPE...AS...END TYPE
    void Enum();                    // TYPE...END ENUM
    void While();                   // WHILE/WEND
    void With();                    // WITH
    void Write();                   // WRITE
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
