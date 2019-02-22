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
#ifndef INCLUDED_IDLC_INC_ASTEXPRESSION_HXX
#define INCLUDED_IDLC_INC_ASTEXPRESSION_HXX

#include <sal/config.h>

#include <memory>

#include "idlc.hxx"
#include <boost/optional.hpp>

// Enum to define all the different operators to combine expressions
enum class ExprComb
{
    Add,     // '+'
    Minus,   // '-'
    Mul,     // '*'
    Div,     // '/'
    Mod,     // '%'
    Or,      // '|'
    Xor,     // '^'
    And,     // '&'
    Left,    // '<<'
    Right,   // '>>'
    UPlus,   // unary '+'
    UMinus,  // unary '-'
    NONE,    // No operator (missing)
    Symbol   // a symbol (function or constant name)
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
    } u;
    ExprType et;
};

const sal_Char* exprTypeToString(ExprType t);

class AstExpression final
{
public:
    // Constructor(s)
    AstExpression(ExprComb c, AstExpression *pExpr1, AstExpression *pExpr2);

    AstExpression(sal_Int32         l);
    AstExpression(sal_Int32         l, ExprType et);
    AstExpression(sal_Int64         h);
    AstExpression(sal_uInt64        uh);
    AstExpression(double            d);
    AstExpression(OString* scopedName);

    ~AstExpression();

    // Data Accessors
    AstExprValue* getExprValue()
        { return m_exprValue.get(); }

    // Evaluation and value coercion
    bool coerce(ExprType type);

    // Evaluate then store value inside this AstExpression
    void evaluate();

    // Compare LONG AstExpression values
    bool compareLong(AstExpression *pExpr);

    OString toString();
private:
    // Evaluate different sets of operators
    std::unique_ptr<AstExprValue> eval_bin_op();
    std::unique_ptr<AstExprValue> eval_bit_op();
    std::unique_ptr<AstExprValue> eval_un_op();
    std::unique_ptr<AstExprValue> eval_symbol();

    ExprComb        m_combOperator;
    std::unique_ptr<AstExpression>
                    m_subExpr1;
    std::unique_ptr<AstExpression>
                    m_subExpr2;
    std::unique_ptr<AstExprValue>
                    m_exprValue;
    boost::optional<OString>
                    m_xSymbolicName;
};

#endif // INCLUDED_IDLC_INC_ASTEXPRESSION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
