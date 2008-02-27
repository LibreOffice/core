/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: expr.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-27 10:33:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _EXPR_HXX
#define _EXPR_HXX

#ifndef _OPCODES_HXX
#include "opcodes.hxx"
#endif
#ifndef _TOKEN_HXX
#include "token.hxx"
#endif

class SbiExprNode;
class SbiExpression;
class SbiExprList;
class SbiDimList;
class SbiParameters;
class SbiParser;
class SbiCodeGen;
class SbiSymDef;
class SbiProcDef;

struct SbVar {                      // Variablen-Element:
    SbiExprNode*   pNext;           // Weiteres Element (bei Strukturen)
    SbiSymDef*     pDef;            // Symboldefinition
    SbiExprList*   pPar;            // optionale Parameter (wird geloescht)
};

enum SbiExprType {                  // Expression-Typen:
    SbSTDEXPR,                      // normaler Ausdruck
    SbLVALUE,                       // beliebiger lValue
    SbSYMBOL,                       // beliebiges zusammengesetztes Symbol
    SbOPERAND                       // Variable/Funktion
};

enum SbiNodeType {
    SbxNUMVAL,                      // nVal = Wert
    SbxSTRVAL,                      // aStrVal = Wert, before #i59791/#i45570: nStringId = Wert
    SbxVARVAL,                      // aVar = Wert
    SbxTYPEOF,                      // TypeOf ObjExpr Is Type
    SbxNODE                         // Node
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
        { return BOOL( eNodeType != SbxNODE && eNodeType != SbxTYPEOF ); }
    BOOL  IsTypeOf()
        { return BOOL( eNodeType == SbxTYPEOF ); }
    BOOL  IsNumber();               // TRUE bei Zahlen
    BOOL  IsString();               // TRUE bei Strings
    BOOL  IsLvalue();               // TRUE, falls als Lvalue verwendbar
    void  GenElement( SbiOpcode );  // Element
    void  BaseInit( SbiParser* p ); // Hilfsfunktion fuer Ctor, AB 17.12.95
public:
    SbiExprNode( SbiParser*, double, SbxDataType );
    SbiExprNode( SbiParser*, const String& );
    SbiExprNode( SbiParser*, const SbiSymDef&, SbxDataType, SbiExprList* = NULL );
    SbiExprNode( SbiParser*, SbiExprNode*, SbiToken, SbiExprNode* );
    SbiExprNode( SbiParser*, SbiExprNode*, USHORT );    // #120061 TypeOf
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
    SbiExprList* GetParameters()    { return aVar.pPar; }

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
    BOOL          bBased;           // TRUE: einfacher DIM-Teil (+BASE)
    BOOL          bError;           // TRUE: Fehler
    BOOL          bByVal;           // TRUE: ByVal-Parameter
    SbiExprNode* Term();
    SbiExprNode* ObjTerm( SbiSymDef& );
    SbiExprNode* Operand();
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
    SbiExpression( SbiParser*, SbiExprType = SbSTDEXPR ); // Parsender Ctor
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
    SbiProcDef* pProc;              // DECLARE-Funktion (Parameter-Anpassung)
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
    // Setzen einer Funktionsdefinition zum Abgleich der Parameter
    void SetProc( SbiProcDef* p )   { pProc = p; }
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
