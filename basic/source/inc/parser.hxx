/*************************************************************************
 *
 *  $RCSfile: parser.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
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

#ifndef _PARSER_HXX
#define _PARSER_HXX

#ifndef _EXPR_HXX
#include "expr.hxx"
#endif
#ifndef _CODEGEN_HXX
#include "codegen.hxx"
#endif
#ifndef _SYMTBL_HXX
#include "symtbl.hxx"
#endif


struct SbiParseStack;

class SbiParser : public SbiTokenizer
{
    SbiParseStack* pStack;          // Block-Stack
    SbiProcDef* pProc;              // aktuelle Prozedur
    SbiExprNode*  pWithVar;         // aktuelle With-Variable
    SbiToken    eEndTok;            // das Ende-Token
    USHORT      nGblChain;          // Chainkette fuer globale DIMs
    BOOL        bGblDefs;           // TRUE globale Definitionen allgemein
    BOOL        bNewGblDefs;        // TRUE globale Definitionen vor Sub
    BOOL        bSingleLineIf;      // TRUE einzeiliges if-Statement

    SbiSymDef*  VarDecl( SbiDimList**,BOOL,BOOL );// Variablen-Deklaration
    SbiProcDef* ProcDecl(BOOL bDecl);// Prozedur-Deklaration
    void DefProc( BOOL bStatic );   // Prozedur einlesen
    void DefVar( SbiOpcode eOp, BOOL bStatic ); // DIM/REDIM einlesen
    void TypeDecl( SbiSymDef& );    // AS-Deklaration
    void OpenBlock( SbiToken, SbiExprNode* = NULL );    // Block oeffnen
    void CloseBlock();              // Block aufloesen
    BOOL Channel( BOOL=FALSE );     // Kanalnummer parsen
    void StmntBlock( SbiToken );    // Statement-Block abarbeiten

public:
    SbxArrayRef   rTypeArray;       // das Type-Array
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
    SbxDataType   eDefTypes[26];    // DEFxxx-Datentypen

    SbiParser( StarBASIC*, SbModule* );
    BOOL Parse();                   // die Aktion
    SbiExprNode* GetWithVar();      // Innerste With-Variable liefern

    // AB 31.3.1996, Symbol in Runtime-Library suchen
    SbiSymDef* CheckRTLForSym( const String& rSym, SbxDataType eType );

    BOOL HasGlobalCode();           // Globaler Code definiert?

    BOOL TestToken( SbiToken );     // bestimmtes TOken?
    BOOL TestSymbol( BOOL=FALSE );  // Symbol?
    BOOL TestComma();               // Komma oder EOLN?
    void TestEoln();                // EOLN?

    void Symbol();                  // Let oder Call
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
    void Input();                   // INPUT, INPUT #
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
    void While();                   // WHILE/WEND
    void With();                    // WITH
    void Write();                   // WRITE

    // JavaScript-Parsing
    void OpenJavaBlock( SbiToken, SbiExprNode* = NULL );    // Block oeffnen
    void CloseJavaBlock();                                  // Block aufloesen
    void JavaStmntBlock( SbiToken );    // Statement-Block abarbeiten
    void JavaBreak();
    void JavaContinue();
    void JavaFor();
    void JavaFunction();
    void JavaIf();
    void JavaNew();
    void JavaReturn();
    void JavaThis();
    void JavaVar();
    void JavaWhile();
    void JavaWith();
};






#endif
