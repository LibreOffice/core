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

#ifndef INCLUDED_BASIC_SOURCE_INC_EXPR_HXX
#define INCLUDED_BASIC_SOURCE_INC_EXPR_HXX

#include <memory>

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
    OUString m_aKeywordSymbol;
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
    OUString aStrVal;               // #i59791/#i45570 Store string directly
    std::unique_ptr<SbiExprNode> pLeft; // left branch
    std::unique_ptr<SbiExprNode> pRight; // right branch (NULL for unary ops)
    SbiExprNode* pWithParent;       // node, whose member is "this per with"
    SbiNodeType  eNodeType;
    SbxDataType eType;
    SbiToken     eTok;
    bool  bError;                   // true: error
    void  FoldConstants(SbiParser*);
    void  CollectBits();            // converting numbers to strings
    bool  IsOperand()
        { return eNodeType != SbxNODE && eNodeType != SbxTYPEOF && eNodeType != SbxNEW; }
    bool  IsTypeOf()
        { return eNodeType == SbxTYPEOF; }
    bool  IsNew()
        { return eNodeType == SbxNEW; }
    bool  IsNumber();
    bool  IsLvalue();               // true, if usable as Lvalue
    void  GenElement( SbiCodeGen&, SbiOpcode );

public:
    SbiExprNode();
    SbiExprNode( double, SbxDataType );
    SbiExprNode( const OUString& );
    SbiExprNode( const SbiSymDef&, SbxDataType, SbiExprList* = nullptr );
    SbiExprNode( SbiExprNode*, SbiToken, SbiExprNode* );
    SbiExprNode( SbiExprNode*, sal_uInt16 );    // #120061 TypeOf
    SbiExprNode( sal_uInt16 );                  // new <type>
    virtual ~SbiExprNode();

    bool IsValid()                  { return !bError; }
    bool IsConstant()               // true: constant operand
        { return eNodeType == SbxSTRVAL || eNodeType == SbxNUMVAL; }
    bool IsIntConst();
    bool IsVariable();

    SbiExprNode* GetWithParent()            { return pWithParent; }
    void SetWithParent( SbiExprNode* p )    { pWithParent = p; }

    SbxDataType GetType()           { return eType; }
    void SetType( SbxDataType eTp ) { eType = eTp; }
    SbiNodeType GetNodeType()       { return eNodeType; }
    SbiSymDef* GetVar();
    SbiSymDef* GetRealVar();        // last variable in x.y.z
    SbiExprNode* GetRealNode();     // last node in x.y.z
    short GetDepth();               // compute a tree's depth
    const OUString& GetString()     { return aStrVal; }
    short GetNumber()               { return (short)nVal; }
    SbiExprList* GetParameters()    { return aVar.pPar; }

    void Optimize(SbiParser*);                // tree matching

    void Gen( SbiCodeGen& rGen, RecursiveMode eRecMode = UNDEFINED ); // giving out a node
};

class SbiExpression {
    friend class SbiExprList;
    friend class SbiParameters;
    friend class SbiDimList;
protected:
    OUString      aArgName;
    SbiParser*    pParser;
    SbiExpression* pNext;            // link at parameter lists
    SbiExprNode*   pExpr;            // expression tree
    SbiExprType   eCurExpr;         // type of expression
    SbiExprMode   m_eMode;          // expression context
    bool          bBased;           // true: easy DIM-part (+BASE)
    bool          bError;
    bool          bByVal;           // true: ByVal-Parameter
    bool          bBracket;         // true: Parameter list with brackets
    sal_uInt16        nParenLevel;
    SbiExprNode* Term( const KeywordSymbolInfo* pKeywordSymbolInfo = nullptr );
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
    SbiExprNode* Comp();
    SbiExprNode* Boolean();
public:
    SbiExpression( SbiParser*, SbiExprType = SbSTDEXPR,
        SbiExprMode eMode = EXPRMODE_STANDARD, const KeywordSymbolInfo* pKeywordSymbolInfo = nullptr ); // parsing Ctor
    SbiExpression( SbiParser*, double, SbxDataType = SbxDOUBLE );
    SbiExpression( SbiParser*, const SbiSymDef&, SbiExprList* = nullptr );
   ~SbiExpression();
    OUString& GetName()             { return aArgName;            }
    void SetBased()                 { bBased = true;              }
    bool IsBased()                  { return bBased;              }
    void SetByVal()                 { bByVal = true;              }
    bool IsBracket()                { return bBracket;            }
    bool IsValid()                  { return pExpr->IsValid();    }
    bool IsVariable()               { return pExpr->IsVariable(); }
    bool IsLvalue()                 { return pExpr->IsLvalue();   }
    bool IsIntConstant()            { return pExpr->IsIntConst(); }
    const OUString& GetString()     { return pExpr->GetString();  }
    SbiSymDef* GetRealVar()         { return pExpr->GetRealVar(); }
    SbiExprNode* GetExprNode()      { return pExpr; }
    SbxDataType GetType()           { return pExpr->GetType();    }
    void Gen( RecursiveMode eRecMode = UNDEFINED );
};

class SbiConstExpression : public SbiExpression {
    double nVal;
    OUString aVal;
    SbxDataType eType;
public:                             // numeric constant
    SbiConstExpression( SbiParser* );
    SbxDataType GetType() { return eType; }
    const OUString& GetString() { return aVal; }
    double GetValue() { return nVal; }
    short GetShortValue();
};

class SbiExprList {                  // base class for parameters and dims
protected:
    SbiParser* pParser;
    SbiExpression* pFirst;
    short nExpr;
    short nDim;
    bool  bError;
    bool  bBracket;
public:
    SbiExprList( SbiParser* );
    virtual ~SbiExprList();
    bool  IsBracket()               { return bBracket;        }
    bool  IsValid()                 { return !bError; }
    short GetSize()                 { return nExpr;           }
    short GetDims()                 { return nDim;            }
    SbiExpression* Get( short );
    void  Gen();                    // code generation
    void addExpression( SbiExpression* pExpr );
};

class SbiParameters : public SbiExprList {
public:
    SbiParameters( SbiParser*, bool bStandaloneExpression = false, bool bPar = true);// parsing Ctor
};

class SbiDimList : public SbiExprList {
    bool  bConst;                   // true: everything integer constants
public:
    SbiDimList( SbiParser* );         // parsing Ctor
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
