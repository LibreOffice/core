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
#ifndef _IDLC_ASTEXPRESSION_HXX_
#define _IDLC_ASTEXPRESSION_HXX_

#include <idlc/idlc.hxx>

// Enum to define all the different operators to combine expressions
enum ExprComb
{
    EC_add,     // '+'
    EC_minus,   // '-'
    EC_mul,     // '*'
    EC_div,     // '/'
    EC_mod,     // '%'
    EC_or,      // '|'
    EC_xor,     // '^'
    EC_and,     // '&'
    EC_left,    // '<<'
    EC_right,   // '>>'
    EC_u_plus,  // unary '+'
    EC_u_minus, // unary '-'
    EC_bit_neg, // '~'
    EC_none,    // No operator (missing)
    EC_symbol   // a symbol (function or constant name)
};

// Enum to define the different kinds of evaluation possible
enum EvalKind
{
    EK_const,           // Must evaluate to constant
    EK_positive_int     // Must evaluate to positive integer
};

// Enum to define expression type
enum ExprType
{
    ET_short,       // Expression value is short
    ET_ushort,      // Expression value is unsigned short
    ET_long,        // Expression value is long
    ET_ulong,       // Expression value is unsigned long
    ET_hyper,       // Expression value is hyper (64 bit)
    ET_uhyper,      // Expression value is unsigned hyper
    ET_float,       // Expression value is 32-bit float
    ET_double,      // Expression value is 64-bit float
    ET_char,        // Expression value is char
    ET_byte,        // Expression value is byte
    ET_boolean,     // Expression value is boolean
    ET_string,      // Expression value is char *
    ET_any,         // Expression value is any of above
    ET_void,        // Expression value is void (absent)
    ET_type,        // Expression value is type
    ET_none         // Expression value is missing
};

// Structure to describe value of constant expression and its type
struct AstExprValue
{
    union
    {
        sal_uInt8       byval;      // Contains byte expression value
        sal_Int16       sval;       // Contains short expression value
        sal_uInt16      usval;      // Contains unsigned short expr value
        sal_Int32       lval;       // Contains long expression value
        sal_uInt32      ulval;      // Contains unsigned long expr value
        sal_Int64       hval;       // Contains hyper expression value
        sal_uInt64      uhval;      // Contains unsigned hyper expr value
        bool            bval;       // Contains boolean expression value
        float           fval;       // Contains 32-bit float expr value
        double          dval;       // Contains 64-bit float expr value
        sal_uInt32      eval;       // Contains enumeration value
    } u;
    ExprType et;
};

const sal_Char* SAL_CALL exprTypeToString(ExprType t);

class AstExpression
{
public:
    // Constructor(s)
    AstExpression(ExprComb c, AstExpression *pExpr1, AstExpression *pExpr2);

    AstExpression(sal_Int32         l);
    AstExpression(sal_Int32         l, ExprType et);
    AstExpression(sal_Int64         h);
    AstExpression(sal_uInt64        uh);
    AstExpression(double            d);
    AstExpression(::rtl::OString* scopedName);

    virtual ~AstExpression();

    // Data Accessors
    AstScope* getScope()
        { return m_pScope; }
    void setScope(AstScope* pScope)
        { m_pScope = pScope; }
    sal_Int32 getLine()
        { return m_lineNo; }
    void setLine(sal_Int32 l)
        { m_lineNo = l; }
    const ::rtl::OString& getFileName()
        { return m_fileName; }
    void setFileName(const ::rtl::OString& fileName)
        { m_fileName = fileName; }
    ExprComb getCombOperator()
        { return m_combOperator; }
    void setCombOperator(ExprComb new_ec)
        { m_combOperator = new_ec; }
    AstExprValue* getExprValue()
        { return m_exprValue; }
    void setExprValue(AstExprValue *pEv)
        { m_exprValue = pEv; }
    AstExpression* getExpr1()
        { return m_subExpr1; }
    void setExpr1(AstExpression *pExpr)
        { m_subExpr1 = pExpr; }
    AstExpression* getExpr2()
        { return m_subExpr2; }
    void setExpr2(AstExpression *pExpr)
        { m_subExpr2 = pExpr; }
    ::rtl::OString* getSymbolicName()
        { return m_pSymbolicName; }
    void setSymbolicName(::rtl::OString* pSymbolicName)
        { m_pSymbolicName = pSymbolicName; }

    // Evaluation and value coercion
    AstExprValue* coerce(ExprType type, sal_Bool bAssign=sal_True);

    // Evaluate then store value inside this AstExpression
    void evaluate(EvalKind ek);

    // Compare to AstExpressions
    sal_Bool operator==(AstExpression *pExpr);
    sal_Bool compare(AstExpression *pExpr);

    ::rtl::OString toString();
    void dump() {}
private:
    // Fill out the lineno, filename and definition scope details
    void    fillDefinitionDetails();
    // Internal evaluation
    AstExprValue* eval_internal(EvalKind ek);
    // Evaluate different sets of operators
    AstExprValue* eval_bin_op(EvalKind ek);
    AstExprValue* eval_bit_op(EvalKind ek);
    AstExprValue* eval_un_op(EvalKind ek);
    AstExprValue* eval_symbol(EvalKind ek);

    AstScope*       m_pScope;       // scope defined in
    sal_Int32       m_lineNo;       // line number defined in
    ::rtl::OString  m_fileName;     // fileName defined in

    ExprComb        m_combOperator;
    AstExpression*  m_subExpr1;
    AstExpression*  m_subExpr2;
    AstExprValue*   m_exprValue;
    ::rtl::OString* m_pSymbolicName;
};

#endif // _IDLC_ASTEXPRESSION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
