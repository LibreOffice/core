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

#ifndef _PARSER_HXX
#define _PARSER_HXX

#include "expr.hxx"
#include "codegen.hxx"
#include "symtbl.hxx"


#include <vector>
typedef ::std::vector< String > StringVector;

struct SbiParseStack;

class SbiParser : public SbiTokenizer
{
    friend class SbiExpression;

    SbiParseStack* pStack;          // Block-Stack
    SbiProcDef* pProc;              // aktuelle Prozedur
    SbiExprNode*  pWithVar;         // aktuelle With-Variable
    SbiToken    eEndTok;            // das Ende-Token
    UINT32      nGblChain;          // Chainkette fuer globale DIMs
    BOOL        bGblDefs;           // TRUE globale Definitionen allgemein
    BOOL        bNewGblDefs;        // TRUE globale Definitionen vor Sub
    BOOL        bSingleLineIf;      // TRUE einzeiliges if-Statement

    SbiSymDef*  VarDecl( SbiDimList**,BOOL,BOOL );// Variablen-Deklaration
    SbiProcDef* ProcDecl(BOOL bDecl);// Prozedur-Deklaration
    void DefStatic( BOOL bPrivate );
    void DefProc( BOOL bStatic, BOOL bPrivate ); // Prozedur einlesen
    void DefVar( SbiOpcode eOp, BOOL bStatic ); // DIM/REDIM einlesen
    void TypeDecl( SbiSymDef&, BOOL bAsNewAlreadyParsed=FALSE );    // AS-Deklaration
    void OpenBlock( SbiToken, SbiExprNode* = NULL );    // Block oeffnen
    void CloseBlock();              // Block aufloesen
    BOOL Channel( BOOL=FALSE );     // Kanalnummer parsen
    void StmntBlock( SbiToken );    // Statement-Block abarbeiten
    void DefType( BOOL bPrivate );  // Parse type declaration
    void DefEnum( BOOL bPrivate );  // Parse enum declaration
    void DefDeclare( BOOL bPrivate );
    void EnableCompatibility();
public:
    SbxArrayRef   rTypeArray;       // das Type-Array
    SbxArrayRef   rEnumArray;       // Enum types
    SbiStringPool aGblStrings;      // der String-Pool
    SbiStringPool aLclStrings;      // der String-Pool
    SbiSymPool    aGlobals;         // globale Variable
    SbiSymPool    aPublics;         // modulglobale Variable
    SbiSymPool    aRtlSyms;         // Runtime-Library
    SbiCodeGen    aGen;             // Code-Generator
    StarBASIC*    pBasic;           // StarBASIC-Instanz
    SbiSymPool*   pPool;            // aktueller Pool
    SbiExprType   eCurExpr;         // aktueller Expr-Typ
    short         nBase;            // OPTION BASE-Wert
    BOOL          bText;            // OPTION COMPARE TEXT
    BOOL          bExplicit;        // TRUE: OPTION EXPLICIT
    BOOL          bClassModule;     // TRUE: OPTION ClassModule
    StringVector  aIfaceVector;     // Holds all interfaces implemented by a class module
    StringVector  aRequiredTypes;   // Types used in Dim As New <type> outside subs
    SbxDataType   eDefTypes[26];    // DEFxxx-Datentypen

    SbiParser( StarBASIC*, SbModule* );
    BOOL Parse();                   // die Aktion
    SbiExprNode* GetWithVar();      // Innerste With-Variable liefern

    // AB 31.3.1996, Symbol in Runtime-Library suchen
    SbiSymDef* CheckRTLForSym( const String& rSym, SbxDataType eType );
    void AddConstants( void );

    BOOL HasGlobalCode();           // Globaler Code definiert?

    BOOL TestToken( SbiToken );     // bestimmtes TOken?
    BOOL TestSymbol( BOOL=FALSE );  // Symbol?
    BOOL TestComma();               // Komma oder EOLN?
    void TestEoln();                // EOLN?

    void Symbol( const KeywordSymbolInfo* pKeywordSymbolInfo = NULL );  // Let oder Call
    void ErrorStmnt();              // ERROR n
    void NotImp();                  // nicht implementiert
    void BadBlock();                // LOOP/WEND/NEXT
    void BadSyntax();               // Falsches SbiToken
    void NoIf();                    // ELSE/ELSE IF ohne IF
    void Assign();                  // LET
    void Attribute();                                   // Attribute
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
