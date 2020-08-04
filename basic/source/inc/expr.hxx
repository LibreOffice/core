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

#pragma once

#include <memory>

#include "opcodes.hxx"
#include "token.hxx"
#include <vector>

class SbiExprNode;
class SbiExpression;
class SbiExprList;
class SbiParser;
class SbiCodeGen;
class SbiSymDef;
class SbiProcDef;


typedef std::unique_ptr<SbiExprList> SbiExprListPtr;
typedef std::vector<SbiExprListPtr> SbiExprListVector;

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

class SbiExprNode final {           // operators (and operands)
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
    void  FoldConstantsBinaryNode(SbiParser*);
    void  FoldConstantsUnaryNode(SbiParser*);
    void  CollectBits();            // converting numbers to strings
    bool  IsOperand() const
        { return eNodeType != SbxNODE && eNodeType != SbxTYPEOF && eNodeType != SbxNEW; }
    bool  IsNumber() const;
    bool  IsLvalue() const;        // true, if usable as Lvalue
    void  GenElement( SbiCodeGen&, SbiOpcode );

public:
    SbiExprNode();
    SbiExprNode( double, SbxDataType );
    SbiExprNode( const OUString& );
    SbiExprNode( const SbiSymDef&, SbxDataType, SbiExprListPtr = nullptr );
    SbiExprNode( std::unique_ptr<SbiExprNode>, SbiToken, std::unique_ptr<SbiExprNode> );
    SbiExprNode( std::unique_ptr<SbiExprNode>, sal_uInt16 );    // #120061 TypeOf
    SbiExprNode( sal_uInt16 );                  // new <type>
    ~SbiExprNode();

    bool IsValid() const               { return !bError; }
    bool IsConstant() const           // true: constant operand
        { return eNodeType == SbxSTRVAL || eNodeType == SbxNUMVAL; }
    void ConvertToIntConstIfPossible();
    bool IsVariable() const;

    void SetWithParent( SbiExprNode* p )    { pWithParent = p; }

    SbxDataType GetType() const     { return eType; }
    void SetType( SbxDataType eTp ) { eType = eTp; }
    SbiNodeType GetNodeType() const { return eNodeType; }
    SbiSymDef* GetVar();
    SbiSymDef* GetRealVar();        // last variable in x.y.z
    SbiExprNode* GetRealNode();     // last node in x.y.z
    const OUString& GetString() const { return aStrVal; }
    short GetNumber() const         { return static_cast<short>(nVal); }
    SbiExprList* GetParameters()    { return aVar.pPar; }

    void Optimize(SbiParser*);                // tree matching

    void Gen( SbiCodeGen& rGen, RecursiveMode eRecMode = UNDEFINED ); // giving out a node
};

class SbiExpression {
    friend class SbiExprList;
protected:
    OUString      aArgName;
    SbiParser*    pParser;
    std::unique_ptr<SbiExprNode>   pExpr; // expression tree
    SbiExprType   eCurExpr;         // type of expression
    SbiExprMode   m_eMode;          // expression context
    bool          bBased;           // true: easy DIM-part (+BASE)
    bool          bError;
    bool          bByVal;           // true: ByVal-Parameter
    bool          bBracket;         // true: Parameter list with brackets
    sal_uInt16        nParenLevel;
    std::unique_ptr<SbiExprNode> Term( const KeywordSymbolInfo* pKeywordSymbolInfo = nullptr );
    std::unique_ptr<SbiExprNode> ObjTerm( SbiSymDef& );
    std::unique_ptr<SbiExprNode> Operand( bool bUsedForTypeOf = false );
    std::unique_ptr<SbiExprNode> Unary();
    std::unique_ptr<SbiExprNode> Exp();
    std::unique_ptr<SbiExprNode> MulDiv();
    std::unique_ptr<SbiExprNode> IntDiv();
    std::unique_ptr<SbiExprNode> Mod();
    std::unique_ptr<SbiExprNode> AddSub();
    std::unique_ptr<SbiExprNode> Cat();
    std::unique_ptr<SbiExprNode> Like();
    std::unique_ptr<SbiExprNode> VBA_Not();
    std::unique_ptr<SbiExprNode> Comp();
    std::unique_ptr<SbiExprNode> Boolean();
public:
    SbiExpression( SbiParser*, SbiExprType = SbSTDEXPR,
        SbiExprMode eMode = EXPRMODE_STANDARD, const KeywordSymbolInfo* pKeywordSymbolInfo = nullptr ); // parsing Ctor
    SbiExpression( SbiParser*, double, SbxDataType );
    SbiExpression( SbiParser*, const SbiSymDef&, SbiExprListPtr = nullptr );
   ~SbiExpression();
    OUString& GetName()             { return aArgName;            }
    void SetBased()                 { bBased = true;              }
    bool IsBased() const            { return bBased;              }
    void SetByVal()                 { bByVal = true;              }
    bool IsBracket() const          { return bBracket;            }
    bool IsValid() const            { return pExpr->IsValid();    }
    bool IsVariable() const         { return pExpr->IsVariable(); }
    bool IsLvalue() const           { return pExpr->IsLvalue();   }
    void ConvertToIntConstIfPossible() { pExpr->ConvertToIntConstIfPossible();     }
    const OUString& GetString() const { return pExpr->GetString();  }
    SbiSymDef* GetRealVar()         { return pExpr->GetRealVar(); }
    SbiExprNode* GetExprNode()      { return pExpr.get();         }
    SbxDataType GetType() const     { return pExpr->GetType();    }
    void Gen( RecursiveMode eRecMode = UNDEFINED );
};

class SbiConstExpression : public SbiExpression {
    double nVal;
    OUString aVal;
    SbxDataType eType;
public:                             // numeric constant
    SbiConstExpression( SbiParser* );
    SbxDataType GetType() const { return eType; }
    const OUString& GetString() const { return aVal; }
    double GetValue() const { return nVal; }
    short GetShortValue();
};

class SbiExprList final {            // class for parameters and dims
    std::vector<std::unique_ptr<SbiExpression>> aData;
    short nDim;
    bool  bError;
    bool  bBracket;
public:
    SbiExprList();
    ~SbiExprList();
    static SbiExprListPtr ParseParameters(SbiParser*, bool bStandaloneExpression = false, bool bPar = true);
    static SbiExprListPtr ParseDimList( SbiParser* );
    bool  IsBracket() const         { return bBracket;        }
    bool  IsValid() const           { return !bError; }
    short GetSize() const           { return aData.size();    }
    short GetDims() const           { return nDim;            }
    SbiExpression* Get( size_t );
    void  Gen( SbiCodeGen& rGen);                    // code generation
    void addExpression( std::unique_ptr<SbiExpression>&& pExpr  );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
