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

struct SbVar {
    SbiExprNode*        pNext;      // next element (for structures)
    SbiSymDef*          pDef;       // symbol definition
    SbiExprList*        pPar;       // optional parameters (is deleted)
    SbiExprListVector*  pvMorePar;  // Array of arrays foo(pPar)(avMorePar[0])(avMorePar[1])...
};

struct KeywordSymbolInfo
{
    String          m_aKeywordSymbol;
    SbxDataType     m_eSbxDataType;
    SbiToken        m_eTok;
};

enum SbiExprType {                  // expression types:
    SbSTDEXPR,                      // normal expression
    SbLVALUE,                       // any lValue
    SbSYMBOL,                       // any composite symbol
    SbOPERAND                       // variable/function
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
    SbxNUMVAL,                      // nVal = value
    SbxSTRVAL,                      // aStrVal = value, before #i59791/#i45570: nStringId = value
    SbxVARVAL,                      // aVar = value
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

class SbiExprNode {                  // operators (and operands)
    friend class SbiExpression;
    friend class SbiConstExpression;
    union {
        sal_uInt16 nTypeStrId;          // pooled String-ID, #i59791/#i45570 Now only for TypeOf
        double nVal;                // numeric value
        SbVar  aVar;                // or variable
    };
    String aStrVal;                 // #i59791/#i45570 Store string directly
    SbiExprNode* pLeft;             // right branch
    SbiExprNode* pRight;            // right branch (NULL for unary ops)
    SbiExprNode* pWithParent;       // node, whose member is "this per with"
    SbiCodeGen*  pGen;              // code-generator
    SbiNodeType  eNodeType;
    SbxDataType eType;
    SbiToken     eTok;
    sal_Bool  bComposite;               // sal_True: composite expression
    sal_Bool  bError;                   // sal_True: error
    void  FoldConstants();
    void  CollectBits();            // converting numbers to strings
    sal_Bool  IsOperand()
        { return sal_Bool( eNodeType != SbxNODE && eNodeType != SbxTYPEOF && eNodeType != SbxNEW ); }
    sal_Bool  IsTypeOf()
        { return sal_Bool( eNodeType == SbxTYPEOF ); }
    sal_Bool  IsNew()
        { return sal_Bool( eNodeType == SbxNEW ); }
    sal_Bool  IsNumber();
    sal_Bool  IsString();
    sal_Bool  IsLvalue();               // sal_True, if usable as Lvalue
    void  GenElement( SbiOpcode );
    void  BaseInit( SbiParser* p ); // help function for Ctor, from 17.12.95
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
    sal_Bool IsConstant()               // sal_True constant operand
        { return sal_Bool( eNodeType == SbxSTRVAL || eNodeType == SbxNUMVAL ); }
    sal_Bool IsIntConst();
    sal_Bool IsVariable();

    SbiExprNode* GetWithParent()            { return pWithParent; }
    void SetWithParent( SbiExprNode* p )    { pWithParent = p; }

    SbxDataType GetType()           { return eType; }
    void SetType( SbxDataType eTp ) { eType = eTp; }
    SbiNodeType GetNodeType()       { return eNodeType; }
    SbiSymDef* GetVar();
    SbiSymDef* GetRealVar();        // last variable in x.y.z
    SbiExprNode* GetRealNode();     // last node in x.y.z
    short GetDepth();               // compute a tree's depth
    const String& GetString()       { return aStrVal; }
    short GetNumber()               { return (short)nVal; }
    SbiExprList* GetParameters()    { return aVar.pPar; }
    SbiExprListVector* GetMoreParameters()  { return aVar.pvMorePar; }

    void Optimize();                // tree matching

    void Gen( RecursiveMode eRecMode = UNDEFINED ); // giving out a node
};

class SbiExpression {
    friend class SbiExprList;
    friend class SbiParameters;
    friend class SbiDimList;
protected:
    String        aArgName;
    SbiParser*    pParser;
    SbiExpression* pNext;            // link at parameter lists
    SbiExprNode*   pExpr;            // expression tree
    SbiExprType   eCurExpr;         // type of expression
    SbiExprMode   m_eMode;          // expression context
    sal_Bool          bBased;           // sal_True: easy DIM-part (+BASE)
    sal_Bool          bError;
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
        SbiExprMode eMode = EXPRMODE_STANDARD, const KeywordSymbolInfo* pKeywordSymbolInfo = NULL ); // parsing Ctor
    SbiExpression( SbiParser*, const String& );
    SbiExpression( SbiParser*, double, SbxDataType = SbxDOUBLE );
    SbiExpression( SbiParser*, const SbiSymDef&, SbiExprList* = NULL );
    SbiExpression( SbiParser*, SbiToken );        // special expr with special tokens
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
    void Gen( RecursiveMode eRecMode = UNDEFINED );
};

class SbiConstExpression : public SbiExpression {
    double nVal;
    String aVal;
    SbxDataType eType;
public:                             // numeric constant
    SbiConstExpression( SbiParser* );
    SbxDataType GetType() { return eType; }
    const String& GetString() { return aVal; }
    double GetValue() { return nVal; }
    short GetShortValue();
};

class SbiExprList {                  // base class for parameters and dims
protected:
    SbiParser* pParser;
    SbiExpression* pFirst;
    short nExpr;
    short nDim;
    sal_Bool  bError;
    sal_Bool  bBracket;
public:
    SbiExprList( SbiParser* );
    virtual ~SbiExprList();
    sal_Bool  IsBracket()               { return bBracket;        }
    sal_Bool  IsValid()                 { return sal_Bool( !bError ); }
    short GetSize()                 { return nExpr;           }
    short GetDims()                 { return nDim;            }
    SbiExpression* Get( short );
    sal_Bool  Test( const SbiProcDef& );    // parameter checks
    void  Gen();                    // code generation
    void addExpression( SbiExpression* pExpr );
};

class SbiParameters : public SbiExprList {
public:
    SbiParameters( SbiParser*, sal_Bool bConst = sal_False, sal_Bool bPar = sal_True);// parsing Ctor
};

class SbiDimList : public SbiExprList {
    sal_Bool  bConst;                   // sal_True: everything integer constants
public:
    SbiDimList( SbiParser* );         // parsing Ctor
    sal_Bool  IsConstant()              { return bConst; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
