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

#ifndef _EXPR_HXX
#define _EXPR_HXX

#include "opcodes.hxx"
#include "token.hxx"

class SbiExprNode;
class SbiExpression;
class SbiExprList;
class SbiDimList;
class SbiParameters;
class SbiParser;
class SbiCodeGen;
class SbiSymDef;
class SbiProcDef;


#include <vector>
typedef ::std::vector<SbiExprList*> SbiExprListVector;

struct SbVar {                      // Variablen-Element:
    SbiExprNode*        pNext;      // Weiteres Element (bei Strukturen)
    SbiSymDef*          pDef;       // Symboldefinition
    SbiExprList*        pPar;       // optionale Parameter (wird geloescht)
    SbiExprListVector*  pvMorePar;  // Array of arrays foo(pPar)(avMorePar[0])(avMorePar[1])...
};

struct KeywordSymbolInfo
{
    String          m_aKeywordSymbol;
    SbxDataType     m_eSbxDataType;
    SbiToken        m_eTok;
};

enum SbiExprType {                  // Expression-Typen:
    SbSTDEXPR,                      // normaler Ausdruck
    SbLVALUE,                       // beliebiger lValue
    SbSYMBOL,                       // beliebiges zusammengesetztes Symbol
    SbOPERAND                       // Variable/Funktion
};

enum SbiExprMode {                  // Expression context:
    EXPRMODE_STANDARD,              // default
    EXPRMODE_STANDALONE,            // a param1, param2 OR a( param1, param2 ) = 42
    EXPRMODE_LPAREN_PENDING,        // start of parameter list with bracket, special handling
    EXPRMODE_LPAREN_NOT_NEEDED,     // pending LPAREN has not been used
    EXPRMODE_ARRAY_OR_OBJECT,       // '=' or '(' or '.' found after ')' on ParenLevel 0, stopping
                                    // expression, assuming array syntax a(...)[(...)] = ?
                                    // or a(...).b(...)
    EXPRMODE_EMPTY_PAREN            // It turned out that the paren don't contain anything: a()
};

enum SbiNodeType {
    SbxNUMVAL,                      // nVal = Wert
    SbxSTRVAL,                      // aStrVal = Wert, before #i59791/#i45570: nStringId = Wert
    SbxVARVAL,                      // aVar = Wert
    SbxTYPEOF,                      // TypeOf ObjExpr Is Type
    SbxNODE,                        // Node
    SbxNEW,                         // new <type> expression
    SbxDUMMY
};

enum RecursiveMode
{
    UNDEFINED,
    FORCE_CALL,
    PREVENT_CALL
};

class SbiExprNode {                  // Operatoren (und Operanden)
    friend class SbiExpression;
    friend class SbiConstExpression;
    union {
        sal_uInt16 nTypeStrId;          // gepoolter String-ID, #i59791/#i45570 Now only for TypeOf
        double nVal;                // numerischer Wert
        SbVar  aVar;                // oder Variable
    };
    String aStrVal;                 // #i59791/#i45570 Store string directly
    SbiExprNode* pLeft;             // linker Zweig
    SbiExprNode* pRight;            // rechter Zweig (NULL bei unaeren Ops)
    SbiExprNode* pWithParent;       // Knoten, dessen Member this per with ist
    SbiCodeGen*  pGen;              // Code-Generator
    SbiNodeType  eNodeType;         // Art des Nodes
    SbxDataType eType;              // aktueller Datentyp
    SbiToken     eTok;              // Token des Operators
    sal_Bool  bComposite;               // sal_True: Zusammengesetzter Ausdruck
    sal_Bool  bError;                   // sal_True: Fehlerhaft
    void  FoldConstants();          // Constant Folding durchfuehren
    void  CollectBits();            // Umwandeln von Zahlen in Strings
    sal_Bool  IsOperand()               // sal_True, wenn Operand
        { return sal_Bool( eNodeType != SbxNODE && eNodeType != SbxTYPEOF && eNodeType != SbxNEW ); }
    sal_Bool  IsTypeOf()
        { return sal_Bool( eNodeType == SbxTYPEOF ); }
    sal_Bool  IsNew()
        { return sal_Bool( eNodeType == SbxNEW ); }
    sal_Bool  IsNumber();               // sal_True bei Zahlen
    sal_Bool  IsString();               // sal_True bei Strings
    sal_Bool  IsLvalue();               // sal_True, falls als Lvalue verwendbar
    void  GenElement( SbiOpcode );  // Element
    void  BaseInit( SbiParser* p ); // Hilfsfunktion fuer Ctor, AB 17.12.95
public:
    SbiExprNode( void );
    SbiExprNode( SbiParser*, double, SbxDataType );
    SbiExprNode( SbiParser*, const String& );
    SbiExprNode( SbiParser*, const SbiSymDef&, SbxDataType, SbiExprList* = NULL );
    SbiExprNode( SbiParser*, SbiExprNode*, SbiToken, SbiExprNode* );
    SbiExprNode( SbiParser*, SbiExprNode*, sal_uInt16 );    // #120061 TypeOf
    SbiExprNode( SbiParser*, sal_uInt16 );                  // new <type>
    virtual ~SbiExprNode();

    sal_Bool IsValid()                  { return sal_Bool( !bError ); }
    sal_Bool IsConstant()               // sal_True bei konstantem Operanden
        { return sal_Bool( eNodeType == SbxSTRVAL || eNodeType == SbxNUMVAL ); }
    sal_Bool IsIntConst();              // sal_True bei Integer-Konstanten
    sal_Bool IsVariable();              // sal_True, wenn Variable

    SbiExprNode* GetWithParent()            { return pWithParent; }
    void SetWithParent( SbiExprNode* p )    { pWithParent = p; }

    SbxDataType GetType()           { return eType; }
    void SetType( SbxDataType eTp ) { eType = eTp; }
    SbiNodeType GetNodeType()       { return eNodeType; }
    SbiSymDef* GetVar();            // Variable (falls vorhanden)
    SbiSymDef* GetRealVar();        // letzte Variable in x.y.z
    SbiExprNode* GetRealNode();     // letzter Knoten in x.y.z
    short GetDepth();               // Tiefe eines Baumes berechnen
    const String& GetString()       { return aStrVal; }
    short GetNumber()               { return (short)nVal; }
    SbiExprList* GetParameters()    { return aVar.pPar; }
    SbiExprListVector* GetMoreParameters()  { return aVar.pvMorePar; }

    void Optimize();                // Baumabgleich

    void Gen( RecursiveMode eRecMode = UNDEFINED ); // Ausgabe eines Nodes
};

class SbiExpression {                // der Ausdruck:
    friend class SbiExprList;
    friend class SbiParameters;
    friend class SbiDimList;
protected:
    String        aArgName;         // Name fuer bananntes Argument
    SbiParser*    pParser;          // fuer Fehlermeldungen, Parsing
    SbiExpression* pNext;            // Link bei Parameterlisten
    SbiExprNode*   pExpr;            // Der Expression-Baum
    SbiExprType   eCurExpr;         // Art des Ausdrucks
    SbiExprMode   m_eMode;          // Expression context
    sal_Bool          bBased;           // sal_True: einfacher DIM-Teil (+BASE)
    sal_Bool          bError;           // sal_True: Fehler
    sal_Bool          bByVal;           // sal_True: ByVal-Parameter
    sal_Bool          bBracket;         // sal_True: Parameter list with brackets
    sal_uInt16        nParenLevel;
    SbiExprNode* Term( const KeywordSymbolInfo* pKeywordSymbolInfo = NULL );
    SbiExprNode* ObjTerm( SbiSymDef& );
    SbiExprNode* Operand( bool bUsedForTypeOf = false );
    SbiExprNode* Unary();
    SbiExprNode* Exp();
    SbiExprNode* MulDiv();
    SbiExprNode* IntDiv();
    SbiExprNode* Mod();
    SbiExprNode* AddSub();
    SbiExprNode* Cat();
    SbiExprNode* Like();
    SbiExprNode* VBA_Not();
    SbiExprNode* VBA_And();
    SbiExprNode* VBA_Or();
    SbiExprNode* VBA_Xor();
    SbiExprNode* VBA_Eqv();
    SbiExprNode* VBA_Imp();
    SbiExprNode* Comp();
    SbiExprNode* Boolean();
public:
    SbiExpression( SbiParser*, SbiExprType = SbSTDEXPR,
        SbiExprMode eMode = EXPRMODE_STANDARD, const KeywordSymbolInfo* pKeywordSymbolInfo = NULL ); // Parsender Ctor
    SbiExpression( SbiParser*, const String& );
    SbiExpression( SbiParser*, double, SbxDataType = SbxDOUBLE );
    SbiExpression( SbiParser*, const SbiSymDef&, SbiExprList* = NULL );
    SbiExpression( SbiParser*, SbiToken );        // Spezial-Expr mit Spezial-Tokens
   ~SbiExpression();
    String& GetName()               { return aArgName;            }
    void SetBased()                 { bBased = sal_True;              }
    sal_Bool IsBased()                  { return bBased;              }
    void SetByVal()                 { bByVal = sal_True;              }
    sal_Bool IsByVal()                  { return bByVal;              }
    sal_Bool IsBracket()                { return bBracket;            }
    sal_Bool IsValid()                  { return pExpr->IsValid();    }
    sal_Bool IsConstant()               { return pExpr->IsConstant(); }
    sal_Bool IsVariable()               { return pExpr->IsVariable(); }
    sal_Bool IsLvalue()                 { return pExpr->IsLvalue();   }
    sal_Bool IsIntConstant()            { return pExpr->IsIntConst(); }
    const String& GetString()       { return pExpr->GetString();  }
    SbiSymDef* GetVar()             { return pExpr->GetVar();     }
    SbiSymDef* GetRealVar()         { return pExpr->GetRealVar(); }
    SbiExprNode* GetExprNode()      { return pExpr; }
    SbxDataType GetType()           { return pExpr->GetType();    }
    void SetType( SbxDataType eType){ pExpr->eType = eType;       }
    void Gen( RecursiveMode eRecMode = UNDEFINED ); // Ausgabe eines Nodes
};

class SbiConstExpression : public SbiExpression {
    double nVal;
    String aVal;
    SbxDataType eType;
public:                             // numerische Konstante
    SbiConstExpression( SbiParser* );
    SbxDataType GetType() { return eType; }
    const String& GetString() { return aVal; }
    double GetValue() { return nVal; }
    short GetShortValue();
};

class SbiExprList {                  // Basisklasse fuer Parameter und Dims
protected:
    SbiParser* pParser;             // Parser
    SbiExpression* pFirst;          // Expressions
    short nExpr;                    // Anzahl Expressions
    short nDim;                     // Anzahl Dimensionen
    sal_Bool  bError;                   // sal_True: Fehler
    sal_Bool  bBracket;                 // sal_True: Klammern
public:
    SbiExprList( SbiParser* );
    virtual ~SbiExprList();
    sal_Bool  IsBracket()               { return bBracket;        }
    sal_Bool  IsValid()                 { return sal_Bool( !bError ); }
    short GetSize()                 { return nExpr;           }
    short GetDims()                 { return nDim;            }
    SbiExpression* Get( short );
    sal_Bool  Test( const SbiProcDef& );    // Parameter-Checks
    void  Gen();                    // Code-Erzeugung
    void addExpression( SbiExpression* pExpr );
};

class SbiParameters : public SbiExprList {
public:
    SbiParameters( SbiParser*, sal_Bool bConst = sal_False, sal_Bool bPar = sal_True);// parsender Ctor
};

class SbiDimList : public SbiExprList {
    sal_Bool  bConst;                   // sal_True: Alles sind Integer-Konstanten
public:
    SbiDimList( SbiParser* );         // Parsender Ctor
    sal_Bool  IsConstant()              { return bConst; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
