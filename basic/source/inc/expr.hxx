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
        USHORT nTypeStrId;          // gepoolter String-ID, #i59791/#i45570 Now only for TypeOf
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
    BOOL  bComposite;               // TRUE: Zusammengesetzter Ausdruck
    BOOL  bError;                   // TRUE: Fehlerhaft
    void  FoldConstants();          // Constant Folding durchfuehren
    void  CollectBits();            // Umwandeln von Zahlen in Strings
    BOOL  IsOperand()               // TRUE, wenn Operand
        { return BOOL( eNodeType != SbxNODE && eNodeType != SbxTYPEOF && eNodeType != SbxNEW ); }
    BOOL  IsTypeOf()
        { return BOOL( eNodeType == SbxTYPEOF ); }
    BOOL  IsNew()
        { return BOOL( eNodeType == SbxNEW ); }
    BOOL  IsNumber();               // TRUE bei Zahlen
    BOOL  IsString();               // TRUE bei Strings
    BOOL  IsLvalue();               // TRUE, falls als Lvalue verwendbar
    void  GenElement( SbiOpcode );  // Element
    void  BaseInit( SbiParser* p ); // Hilfsfunktion fuer Ctor, AB 17.12.95
public:
    SbiExprNode( void );
    SbiExprNode( SbiParser*, double, SbxDataType );
    SbiExprNode( SbiParser*, const String& );
    SbiExprNode( SbiParser*, const SbiSymDef&, SbxDataType, SbiExprList* = NULL );
    SbiExprNode( SbiParser*, SbiExprNode*, SbiToken, SbiExprNode* );
    SbiExprNode( SbiParser*, SbiExprNode*, USHORT );    // #120061 TypeOf
    SbiExprNode( SbiParser*, USHORT );                  // new <type>
    virtual ~SbiExprNode();

    BOOL IsValid()                  { return BOOL( !bError ); }
    BOOL IsConstant()               // TRUE bei konstantem Operanden
        { return BOOL( eNodeType == SbxSTRVAL || eNodeType == SbxNUMVAL ); }
    BOOL IsIntConst();              // TRUE bei Integer-Konstanten
    BOOL IsVariable();              // TRUE, wenn Variable

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
    BOOL          bBased;           // TRUE: einfacher DIM-Teil (+BASE)
    BOOL          bError;           // TRUE: Fehler
    BOOL          bByVal;           // TRUE: ByVal-Parameter
    BOOL          bBracket;         // TRUE: Parameter list with brackets
    USHORT        nParenLevel;
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
    void SetBased()                 { bBased = TRUE;              }
    BOOL IsBased()                  { return bBased;              }
    void SetByVal()                 { bByVal = TRUE;              }
    BOOL IsByVal()                  { return bByVal;              }
    BOOL IsBracket()                { return bBracket;            }
    BOOL IsValid()                  { return pExpr->IsValid();    }
    BOOL IsConstant()               { return pExpr->IsConstant(); }
    BOOL IsVariable()               { return pExpr->IsVariable(); }
    BOOL IsLvalue()                 { return pExpr->IsLvalue();   }
    BOOL IsIntConstant()            { return pExpr->IsIntConst(); }
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
    BOOL  bError;                   // TRUE: Fehler
    BOOL  bBracket;                 // TRUE: Klammern
public:
    SbiExprList( SbiParser* );
    virtual ~SbiExprList();
    BOOL  IsBracket()               { return bBracket;        }
    BOOL  IsValid()                 { return BOOL( !bError ); }
    short GetSize()                 { return nExpr;           }
    short GetDims()                 { return nDim;            }
    SbiExpression* Get( short );
    BOOL  Test( const SbiProcDef& );    // Parameter-Checks
    void  Gen();                    // Code-Erzeugung
    void addExpression( SbiExpression* pExpr );
};

class SbiParameters : public SbiExprList {
public:
    SbiParameters( SbiParser*, BOOL bConst = FALSE, BOOL bPar = TRUE);// parsender Ctor
};

class SbiDimList : public SbiExprList {
    BOOL  bConst;                   // TRUE: Alles sind Integer-Konstanten
public:
    SbiDimList( SbiParser* );         // Parsender Ctor
    BOOL  IsConstant()              { return bConst; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
