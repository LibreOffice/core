/*************************************************************************
 *
 *  $RCSfile: expr.hxx,v $
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
    SbxSTRVAL,                      // nStringId = Wert
    SbxVARVAL,                      // aVar = Wert
    SbxNODE                         // Node
};

class SbiExprNode {                  // Operatoren (und Operanden)
    friend class SbiExpression;
    friend class SbiConstExpression;
    union {
        USHORT nStringId;           // gepoolter String-ID
        double nVal;                // numerischer Wert
        SbVar  aVar;                // oder Variable
    };
    SbiExprNode* pLeft;             // linker Zweig
    SbiExprNode* pRight;            // rechter Zweig (NULL bei unaeren Ops)
    SbiCodeGen*  pGen;              // Code-Generator
    SbiNodeType  eNodeType;         // Art des Nodes
    SbxDataType eType;              // aktueller Datentyp
    SbiToken     eTok;              // Token des Operators
    BOOL  bComposite;               // TRUE: Zusammengesetzter Ausdruck
    BOOL  bError;                   // TRUE: Fehlerhaft
    BOOL  bPartOfWith;              // TRUE: .-Anweisung in with-Block
    void  FoldConstants();          // Constant Folding durchfuehren
    void  CollectBits();            // Umwandeln von Zahlen in Strings
    BOOL  IsOperand();              // TRUE, wenn Operand
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
    virtual ~SbiExprNode();

    BOOL IsValid()                  { return BOOL( !bError ); }
    BOOL IsConstant();              // TRUE bei konstantem Operanden
    BOOL IsIntConst();              // TRUE bei Integer-Konstanten
    BOOL IsVariable();              // TRUE, wenn Variable

    BOOL IsPartOfWith()             { return bPartOfWith; }
    void SetPartOfWith( BOOL b )    { bPartOfWith = b; }

    SbxDataType GetType()           { return eType; }
    void SetType( SbxDataType eTp ) { eType = eTp; }
    SbiNodeType GetNodeType()       { return eNodeType; }
    SbiSymDef* GetVar();            // Variable (falls vorhanden)
    SbiSymDef* GetRealVar();        // letzte Variable in x.y.z
    SbiExprNode* GetRealNode();     // letzter Knoten in x.y.z
    short GetDepth();               // Tiefe eines Baumes berechnen
    const String& GetString();      // String liefern

    void Optimize();                // Baumabgleich

    void Gen();                     // Ausgabe eines Nodes
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
    void Gen();
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
