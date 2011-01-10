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
    sal_uInt32      nGblChain;          // Chainkette fuer globale DIMs
    sal_Bool        bGblDefs;           // sal_True globale Definitionen allgemein
    sal_Bool        bNewGblDefs;        // sal_True globale Definitionen vor Sub
    sal_Bool        bSingleLineIf;      // sal_True einzeiliges if-Statement

    SbiSymDef*  VarDecl( SbiDimList**,sal_Bool,sal_Bool );// Variablen-Deklaration
    SbiProcDef* ProcDecl(sal_Bool bDecl);// Prozedur-Deklaration
    void DefStatic( sal_Bool bPrivate );
    void DefProc( sal_Bool bStatic, sal_Bool bPrivate ); // Prozedur einlesen
    void DefVar( SbiOpcode eOp, sal_Bool bStatic ); // DIM/REDIM einlesen
    void TypeDecl( SbiSymDef&, sal_Bool bAsNewAlreadyParsed=sal_False );    // AS-Deklaration
    void OpenBlock( SbiToken, SbiExprNode* = NULL );    // Block oeffnen
    void CloseBlock();              // Block aufloesen
    sal_Bool Channel( sal_Bool=sal_False );     // Kanalnummer parsen
    void StmntBlock( SbiToken );    // Statement-Block abarbeiten
    void DefType( sal_Bool bPrivate );  // Parse type declaration
    void DefEnum( sal_Bool bPrivate );  // Parse enum declaration
    void DefDeclare( sal_Bool bPrivate );
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
    sal_Bool          bText;            // OPTION COMPARE TEXT
    sal_Bool          bExplicit;        // sal_True: OPTION EXPLICIT
    sal_Bool          bClassModule;     // sal_True: OPTION ClassModule
    StringVector  aIfaceVector;     // Holds all interfaces implemented by a class module
    StringVector  aRequiredTypes;   // Types used in Dim As New <type> outside subs
    SbxDataType   eDefTypes[26];    // DEFxxx-Datentypen

    SbiParser( StarBASIC*, SbModule* );
    sal_Bool Parse();                   // die Aktion
    SbiExprNode* GetWithVar();      // Innerste With-Variable liefern

    // AB 31.3.1996, Symbol in Runtime-Library suchen
    SbiSymDef* CheckRTLForSym( const String& rSym, SbxDataType eType );
    void AddConstants( void );

    sal_Bool HasGlobalCode();           // Globaler Code definiert?

    sal_Bool TestToken( SbiToken );     // bestimmtes TOken?
    sal_Bool TestSymbol( sal_Bool=sal_False );  // Symbol?
    sal_Bool TestComma();               // Komma oder EOLN?
    void TestEoln();                // EOLN?

    void Symbol( const KeywordSymbolInfo* pKeywordSymbolInfo = NULL );  // Let oder Call
    void ErrorStmnt();              // ERROR n
    void NotImp();                  // nicht implementiert
    void BadBlock();                // LOOP/WEND/NEXT
    void BadSyntax();               // Falsches SbiToken
    void NoIf();                    // ELSE/ELSE IF ohne IF
    void Assign();                  // LET
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
