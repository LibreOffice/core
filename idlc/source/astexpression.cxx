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


#include <idlc/astexpression.hxx>
#include <idlc/astconstant.hxx>
#include <idlc/astscope.hxx>
#include <idlc/errorhandler.hxx>

#include <limits.h>
#include <float.h>
#include <memory> // auto_ptr<>

#undef  MAXCHAR
#define MAXCHAR         127
#undef  MINCHAR
#define MINCHAR         -128

using namespace ::rtl;

AstExpression::AstExpression(ExprComb c, AstExpression *pExpr1, AstExpression *pExpr2)
    : m_combOperator(c)
    , m_subExpr1(pExpr1)
    , m_subExpr2(pExpr2)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

}

AstExpression::AstExpression(sal_Int32 l)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    m_exprValue = new AstExprValue();
    m_exprValue->et = ET_long;
    m_exprValue->u.lval = l;
}

AstExpression::AstExpression(sal_Int32  l, ExprType et)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    m_exprValue = new AstExprValue();
    m_exprValue->et = et;
    m_exprValue->u.lval = l;
}

AstExpression::AstExpression(sal_Int64  h)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    m_exprValue = new AstExprValue();
    m_exprValue->et = ET_hyper;
    m_exprValue->u.hval = h;
}

AstExpression::AstExpression(sal_uInt64 uh)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    m_exprValue = new AstExprValue();
    m_exprValue->et = ET_uhyper;
    m_exprValue->u.uhval = uh;
}

AstExpression::AstExpression(double d)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    m_exprValue = new AstExprValue();
    m_exprValue->et = ET_double;
    m_exprValue->u.dval = d;
}

AstExpression::AstExpression(::rtl::OString* scopedName)
    : m_combOperator(EC_symbol)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(scopedName)
{
    fillDefinitionDetails();
}

AstExpression::~AstExpression()
{
    if ( m_exprValue )
        delete m_exprValue;
    if ( m_subExpr1 )
        delete m_subExpr1;
    if ( m_subExpr2 )
        delete m_subExpr2;
    if ( m_pSymbolicName )
        delete m_pSymbolicName;
}

/*
 * Perform the coercion from the given AstExprValue to the requested
 * ExprType. Return an AstExprValue if successful, NULL if failed.
 * must be done for hyper, uhyper
 */
static AstExprValue *
coerce_value(AstExprValue *ev, ExprType t)
{
    if (ev == NULL)
        return NULL;

    switch (t)
    {
        case ET_short:
            switch (ev->et)
            {
                case ET_short:
                    return ev;
                case ET_ushort:
                    if (ev->u.usval > SAL_MAX_INT16)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.usval;
                    ev->et = ET_short;
                    return ev;
                case ET_long:
                    if (ev->u.lval < SAL_MIN_INT16 || ev->u.lval > SAL_MAX_INT16)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.lval;
                    ev->et = ET_short;
                    return ev;
                case ET_ulong:
                    if (ev->u.ulval > SAL_MAX_INT16)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.ulval;
                    ev->et = ET_short;
                    return ev;
                case ET_hyper:
                    if (ev->u.hval < SAL_MIN_INT16 || ev->u.hval > SAL_MAX_INT16)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.hval;
                    ev->et = ET_short;
                    return ev;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_INT16)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.uhval;
                    ev->et = ET_short;
                    return ev;
                case ET_boolean:
                    ev->u.sval = (sal_Int16)ev->u.bval;
                    ev->et = ET_short;
                    return ev;
                case ET_float:
                    if (ev->u.fval < SAL_MIN_INT16 || ev->u.fval > SAL_MAX_INT16)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.fval;
                    ev->et = ET_short;
                    return ev;
                case ET_double:
                    if (ev->u.dval < SAL_MIN_INT16 || ev->u.dval > SAL_MAX_INT16)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.dval;
                    ev->et = ET_short;
                    return ev;
                case ET_byte:
                    ev->u.sval = (sal_Int16)ev->u.byval;
                    ev->et = ET_short;
                    return ev;
                default:
                    OSL_ASSERT(false);
                    return NULL;
            }
        case ET_ushort:
            switch (ev->et)
            {
                case ET_short:
                    if (ev->u.sval < 0)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.sval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_ushort:
                    return ev;
                case ET_long:
                    if (ev->u.lval < 0 || ev->u.lval > SAL_MAX_UINT16)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.lval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_ulong:
                    if (ev->u.ulval > SAL_MAX_UINT16)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.ulval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_hyper:
                    if (ev->u.hval < 0 || ev->u.hval > SAL_MAX_UINT16)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.hval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_UINT16)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.uhval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_boolean:
                    ev->u.usval = (sal_uInt16)ev->u.bval;
                    ev->et = ET_short;
                    return ev;
                case ET_float:
                    if (ev->u.fval < 0.0 || ev->u.fval > SAL_MAX_UINT16)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.fval;
                    ev->et = ET_short;
                    return ev;
                case ET_double:
                    if (ev->u.dval < 0.0 || ev->u.dval > SAL_MAX_UINT16)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.dval;
                    ev->et = ET_short;
                    return ev;
                case ET_byte:
                    ev->u.usval = (sal_uInt16)ev->u.byval;
                    ev->et = ET_ushort;
                    return ev;
                default:
                    OSL_ASSERT(false);
                    return NULL;
            }
        case ET_long:
            switch (ev->et)
            {
                case ET_short:
                    ev->u.lval = (sal_Int32)ev->u.sval;
                    ev->et = ET_long;
                    return ev;
                case ET_ushort:
                    ev->u.lval = (sal_Int32)ev->u.usval;
                    ev->et = ET_long;
                    return ev;
                case ET_long:
                    return ev;
                case ET_ulong:
                    if (ev->u.ulval > SAL_MAX_INT32)
                        return NULL;
                    ev->u.lval = (sal_Int32)ev->u.ulval;
                    ev->et = ET_long;
                    return ev;
                case ET_hyper:
                    if (ev->u.hval < SAL_MIN_INT32 || ev->u.hval > SAL_MAX_INT32)
                        return NULL;
                    ev->u.lval = (sal_Int32)ev->u.hval;
                    ev->et = ET_long;
                    return ev;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_INT32)
                        return NULL;
                    ev->u.lval = (sal_Int32)ev->u.uhval;
                    ev->et = ET_long;
                    return ev;
                case ET_boolean:
                    ev->u.lval = (sal_Int32)ev->u.bval;
                    ev->et = ET_long;
                    return ev;
                case ET_float:
                    if (ev->u.fval < SAL_MIN_INT32 || ev->u.fval > SAL_MAX_INT32)
                        return NULL;
                    ev->u.lval = (sal_Int32)ev->u.fval;
                    ev->et = ET_long;
                    return ev;
                case ET_double:
                    if (ev->u.dval < SAL_MIN_INT32 || ev->u.dval > SAL_MAX_INT32)
                        return NULL;
                    ev->u.lval = (sal_Int32)ev->u.dval;
                    ev->et = ET_long;
                    return ev;
                case ET_byte:
                    ev->u.lval = (sal_Int32) ev->u.byval;
                    ev->et = ET_long;
                    return ev;
                default:
                    OSL_ASSERT(false);
                    return NULL;
            }
        case ET_ulong:
            switch (ev->et)
            {
                case ET_short:
                    if (ev->u.sval < 0)
                        return NULL;
                    ev->u.ulval = (sal_uInt32)ev->u.sval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_ushort:
                    ev->u.ulval = (sal_uInt32)ev->u.usval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_long:
                    if (ev->u.lval < 0)
                        return NULL;
                    ev->u.ulval = (sal_uInt32)ev->u.lval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_ulong:
                    return ev;
                case ET_hyper:
                    if (ev->u.hval < 0 || ev->u.hval > SAL_MAX_UINT32)
                        return NULL;
                    ev->u.lval = (sal_uInt32)ev->u.hval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_UINT32)
                        return NULL;
                    ev->u.ulval = (sal_uInt32)ev->u.uhval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_boolean:
                    ev->u.ulval = (sal_uInt32)ev->u.bval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_float:
                    if (ev->u.fval < 0.0 || ev->u.fval > SAL_MAX_UINT32)
                        return NULL;
                    ev->u.ulval = (sal_uInt32)ev->u.fval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_double:
                    if (ev->u.dval < 0.0 || ev->u.dval > SAL_MAX_UINT32)
                        return NULL;
                    ev->u.ulval = (sal_uInt32)ev->u.dval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_byte:
                    ev->u.ulval = (sal_uInt32)ev->u.byval;
                    ev->et = ET_ulong;
                    return ev;
                default:
                    OSL_ASSERT(false);
                    return NULL;
            }
        case ET_hyper:
            switch (ev->et)
            {
                case ET_short:
                    ev->u.hval = (sal_Int64)ev->u.sval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_ushort:
                    ev->u.hval = (sal_Int64)ev->u.usval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_long:
                    ev->u.hval = (sal_Int64)ev->u.lval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_ulong:
                    ev->u.hval = (sal_Int64)ev->u.ulval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_hyper:
                    return ev;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_INT64)
                        return NULL;
                    ev->u.hval = (sal_Int64)ev->u.uhval;
                    ev->et = ET_long;
                    return ev;
                case ET_boolean:
                    ev->u.hval = (sal_Int64)ev->u.bval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_float:
                    if (ev->u.fval < SAL_MIN_INT64 || ev->u.fval > SAL_MAX_INT64)
                        return NULL;
                    ev->u.hval = (sal_Int64)ev->u.fval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_double:
                    if (ev->u.dval < SAL_MIN_INT64 || ev->u.dval > SAL_MAX_INT64)
                        return NULL;
                    ev->u.hval = (sal_Int64)ev->u.dval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_byte:
                    ev->u.hval = (sal_Int64)ev->u.byval;
                    ev->et = ET_hyper;
                    return ev;
                default:
                    OSL_ASSERT(false);
                    return NULL;
            }
        case ET_uhyper:
            switch (ev->et)
            {
                case ET_short:
                    if (ev->u.sval < 0)
                        return NULL;
                    ev->u.uhval = (sal_uInt64)ev->u.sval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_ushort:
                    ev->u.uhval = (sal_uInt64)ev->u.usval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_long:
                    if (ev->u.lval < 0)
                        return NULL;
                    ev->u.uhval = (sal_uInt64)ev->u.lval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_ulong:
                    ev->u.uhval = (sal_uInt64)ev->u.ulval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_hyper:
                    if (ev->u.hval < 0)
                        return NULL;
                    ev->u.uhval = (sal_uInt64)ev->u.hval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_uhyper:
                    return ev;
                case ET_boolean:
                    ev->u.uhval = (sal_uInt64)ev->u.bval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_float:
                    if (ev->u.fval < 0.0 || ev->u.fval > SAL_MAX_UINT64)
                        return NULL;
                    ev->u.uhval = (sal_uInt64)ev->u.fval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_double:
                    if (ev->u.dval < 0.0 || ev->u.dval > SAL_MAX_UINT64)
                        return NULL;
                    ev->u.uhval = (sal_uInt64)ev->u.dval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_byte:
                    ev->u.uhval = (sal_uInt64)ev->u.byval;
                    ev->et = ET_uhyper;
                    return ev;
                default:
                    OSL_ASSERT(false);
                    return NULL;
            }
        case ET_boolean:
            switch (ev->et)
            {
                case ET_short:
                    ev->u.bval = ev->u.sval != 0;
                    ev->et = ET_boolean;
                    return ev;
                case ET_ushort:
                    ev->u.bval = ev->u.usval != 0;
                    ev->et = ET_boolean;
                    return ev;
                case ET_long:
                    ev->u.bval = ev->u.lval != 0;
                    ev->et = ET_boolean;
                    return ev;
                case ET_ulong:
                    ev->u.bval = ev->u.ulval != 0;
                    ev->et = ET_boolean;
                    return ev;
                case ET_hyper:
                    ev->u.bval = ev->u.hval != 0;
                    ev->et = ET_boolean;
                    return ev;
                case ET_uhyper:
                    ev->u.bval = ev->u.uhval != 0;
                    ev->et = ET_boolean;
                    return ev;
                case ET_boolean:
                    return ev;
                case ET_float:
                    ev->u.bval = ev->u.fval != 0.0;
                    ev->et = ET_boolean;
                    return ev;
                case ET_double:
                    ev->u.bval = ev->u.dval != 0.0;
                    ev->et = ET_boolean;
                    return ev;
                case ET_byte:
                    ev->u.bval = ev->u.byval != 0;
                    ev->et = ET_boolean;
                    return ev;
                default:
                    OSL_ASSERT(false);
                    return NULL;
            }
        case ET_float:
            switch (ev->et)
            {
                case ET_short:
                    ev->u.fval = (float)ev->u.sval;
                    ev->et = ET_float;
                    return ev;
                case ET_ushort:
                    ev->u.fval = (float)ev->u.usval;
                    ev->et = ET_float;
                    return ev;
                case ET_long:
                    ev->u.fval = (float)ev->u.lval;
                    ev->et = ET_float;
                    return ev;
                case ET_ulong:
                    ev->u.fval = (float)ev->u.ulval;
                    ev->et = ET_float;
                    return ev;
                case ET_hyper:
                    ev->u.fval = (float)ev->u.hval;
                    ev->et = ET_float;
                    return ev;
                case ET_uhyper:
                    if ((float)ev->u.ulval > FLT_MAX)
                        return NULL;
                    ev->u.fval = (float)ev->u.ulval;
                    ev->et = ET_float;
                    return ev;
                case ET_boolean:
                    ev->u.fval = ev->u.bval ? 1.0f : 0.0f;
                    ev->et = ET_float;
                    return ev;
                case ET_float:
                    return ev;
                case ET_double:
                    if ((float)ev->u.dval > FLT_MAX || (float)ev->u.dval < -FLT_MAX)
                        return NULL;
                    ev->u.fval = (float)ev->u.dval;
                    ev->et = ET_float;
                    return ev;
                case ET_byte:
                    ev->u.fval = (float)ev->u.byval;
                    ev->et = ET_float;
                    return ev;
                default:
                    OSL_ASSERT(false);
                    return NULL;
            }
        case ET_double:
            switch (ev->et)
            {
                case ET_short:
                    ev->u.dval = (double)ev->u.sval;
                    ev->et = ET_double;
                    return ev;
                case ET_ushort:
                    ev->u.dval = (double)ev->u.usval;
                    ev->et = ET_double;
                    return ev;
                case ET_long:
                    ev->u.dval = (double)ev->u.lval;
                    ev->et = ET_double;
                    return ev;
                case ET_ulong:
                    ev->u.dval = (double)ev->u.ulval;
                    ev->et = ET_double;
                    return ev;
                case ET_hyper:
                    ev->u.dval = (double)ev->u.hval;
                    ev->et = ET_double;
                    return ev;
                case ET_uhyper:
                    if ((double)ev->u.dval > FLT_MAX || (double)ev->u.dval < -FLT_MAX)
                        return NULL;
                    ev->u.dval = (double)ev->u.ulval;
                    ev->et = ET_double;
                    return ev;
                case ET_boolean:
                    ev->u.dval = ev->u.bval ? 1.0 : 0.0;
                    ev->et = ET_double;
                    return ev;
                case ET_float:
                    ev->u.dval = (double)ev->u.fval;
                    ev->et = ET_double;
                    return ev;
                case ET_double:
                    return ev;
                case ET_byte:
                    ev->u.dval = (double)ev->u.byval;
                    ev->et = ET_double;
                    return ev;
                default:
                    OSL_ASSERT(false);
                    return NULL;
            }
        case ET_byte:
            switch (ev->et)
            {
                case ET_short:
                    if (ev->u.sval < SAL_MIN_INT8 || ev->u.sval > SAL_MAX_UINT8)
                        return NULL;
                    ev->u.byval = (sal_uChar)ev->u.sval;
                    ev->et = ET_byte;
                    return ev;
                case ET_ushort:
                    if (ev->u.usval > SAL_MAX_UINT8)
                        return NULL;
                    ev->u.byval = (sal_uChar)ev->u.usval;
                    ev->et = ET_byte;
                    return ev;
                case ET_long:
                    if (ev->u.lval < SAL_MIN_INT8 || ev->u.lval > SAL_MAX_UINT8)
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.lval;
                    ev->et = ET_byte;
                    return ev;
                case ET_ulong:
                    if (ev->u.ulval > SAL_MAX_UINT8)
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.ulval;
                    ev->et = ET_byte;
                    return ev;
                case ET_hyper:
                    if (ev->u.hval < SAL_MIN_INT8 || ev->u.hval > SAL_MAX_UINT8)
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.hval;
                    ev->et = ET_byte;
                    return ev;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_UINT8)
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.uhval;
                    ev->et = ET_byte;
                    return ev;
                case ET_boolean:
                    ev->u.byval = (ev->u.bval == false) ? 1 : 0;
                    ev->et = ET_byte;
                    return ev;
                case ET_float:
                    if (ev->u.fval < SAL_MIN_INT8 || ev->u.fval > SAL_MAX_UINT8)
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.fval;
                    ev->et = ET_byte;
                    return ev;
                case ET_double:
                    if (ev->u.dval < SAL_MIN_INT8 || ev->u.dval > SAL_MAX_UINT8)
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.dval;
                    ev->et = ET_byte;
                    return ev;
                case ET_byte:
                    return ev;
                default:
                    OSL_ASSERT(false);
                    return NULL;
            }
        default:
            OSL_ASSERT(false);
            return NULL;
    }
}

/*
 * Evaluate the expression with the evaluation kind requested. Supported
 * evaluation kinds are
 * - EK_const:      The expression must evaluate to a constant
 * - EK_positive_int:   The expression must further evaluate to a
 *          positive integer
 */
static AstExprValue *
eval_kind(AstExprValue *ev, EvalKind ek)
{
    if (ek == EK_const)
        return ev;
    if (ek == EK_positive_int)
        return coerce_value(ev, ET_ulong);

    return NULL;
}

AstExprValue* AstExpression::coerce(ExprType t, sal_Bool bAssign)
{
    AstExprValue *copy;

    /*
     * Is it already of the right type?
     */
    if (m_exprValue != NULL && m_exprValue->et == t)
        return m_exprValue;
    /*
     * OK, must coerce
     *
     * First, evaluate it, then try to coerce result type
     * If already evaluated, return the result
     */
    m_exprValue = eval_internal(EK_const);
    if (m_exprValue == NULL)
        return NULL;

    /*
     * Create a copy to contain coercion result
     */
    copy = new AstExprValue;

    copy->et = m_exprValue->et;
    switch (m_exprValue->et)
    {
        case ET_short:
            copy->u.sval = m_exprValue->u.sval;
            break;
        case ET_ushort:
            copy->u.usval = m_exprValue->u.usval;
            break;
        case ET_long:
            copy->u.lval = m_exprValue->u.lval;
            break;
        case ET_ulong:
            copy->u.ulval = m_exprValue->u.ulval;
            break;
        case ET_hyper:
            copy->u.hval = m_exprValue->u.hval;
            break;
        case ET_uhyper:
            copy->u.uhval = m_exprValue->u.uhval;
            break;
        case ET_boolean:
            copy->u.bval = m_exprValue->u.bval;
            break;
        case ET_float:
            copy->u.fval = m_exprValue->u.fval;
            break;
        case ET_double:
            copy->u.dval = m_exprValue->u.dval;
            break;
          case ET_byte:
            copy->u.byval = m_exprValue->u.byval;
            break;
        default:
            OSL_ASSERT(false);
            break;
    }

    AstExprValue* const coerced(coerce_value(copy, t));
    if (!coerced)
        delete copy;

    if (bAssign)
        m_exprValue = coerced;

    return coerced;
}

void AstExpression::evaluate(EvalKind ek)
{
    m_exprValue = eval_internal(ek);
    m_exprValue = eval_kind(m_exprValue, ek);
}

sal_Bool AstExpression::operator==(AstExpression *pExpr)
{
    sal_Bool bRet = sal_False;
    if (m_combOperator != pExpr->getCombOperator())
        return bRet;
    evaluate(EK_const);
    pExpr->evaluate(EK_const);
    if (m_exprValue == NULL || pExpr->getExprValue() == NULL)
        return bRet;
    if (m_exprValue->et != pExpr->getExprValue()->et)
        return bRet;
    switch (m_exprValue->et)
    {
        case ET_short:
            bRet = (m_exprValue->u.sval == pExpr->getExprValue()->u.sval) ? sal_True : sal_False;
            break;
        case ET_ushort:
            bRet = (m_exprValue->u.usval == pExpr->getExprValue()->u.usval) ? sal_True : sal_False;
            break;
        case ET_long:
            bRet = (m_exprValue->u.lval == pExpr->getExprValue()->u.lval) ? sal_True : sal_False;
            break;
        case ET_ulong:
            bRet = (m_exprValue->u.ulval == pExpr->getExprValue()->u.ulval) ? sal_True : sal_False;
            break;
        case ET_hyper:
            bRet = (m_exprValue->u.hval == pExpr->getExprValue()->u.hval) ? sal_True : sal_False;
            break;
        case ET_uhyper:
            bRet = (m_exprValue->u.uhval == pExpr->getExprValue()->u.uhval) ? sal_True : sal_False;
            break;
        case ET_float:
            bRet = (m_exprValue->u.fval == pExpr->getExprValue()->u.fval) ? sal_True : sal_False;
            break;
        case ET_double:
            bRet = (m_exprValue->u.dval == pExpr->getExprValue()->u.dval) ? sal_True : sal_False;
            break;
        case ET_byte:
            bRet = (m_exprValue->u.byval == pExpr->getExprValue()->u.byval) ? sal_True : sal_False;
            break;
        case ET_boolean:
            bRet = (m_exprValue->u.lval == pExpr->getExprValue()->u.lval) ? sal_True : sal_False;
            break;
        default:
            OSL_ASSERT(false);
            bRet = sal_False;
            break;
    }

    return bRet;
}

sal_Bool AstExpression::compare(AstExpression *pExpr)
{
    bool bRet = sal_False;
    if (m_combOperator != pExpr->getCombOperator())
        return bRet;
    evaluate(EK_const);
    pExpr->evaluate(EK_const);
    if (m_exprValue == NULL || pExpr->getExprValue() == NULL)
        return bRet;
    if (m_exprValue->et != pExpr->getExprValue()->et)
        return bRet;
    switch (m_exprValue->et)
    {
        case ET_short:
            bRet = (m_exprValue->u.sval == pExpr->getExprValue()->u.sval) ? sal_True : sal_False;
            break;
        case ET_ushort:
            bRet = (m_exprValue->u.usval == pExpr->getExprValue()->u.usval) ? sal_True : sal_False;
            break;
        case ET_long:
            bRet = (m_exprValue->u.lval == pExpr->getExprValue()->u.lval) ? sal_True : sal_False;
            break;
        case ET_ulong:
            bRet = (m_exprValue->u.ulval == pExpr->getExprValue()->u.ulval) ? sal_True : sal_False;
            break;
        case ET_hyper:
            bRet = (m_exprValue->u.hval == pExpr->getExprValue()->u.hval) ? sal_True : sal_False;
            break;
        case ET_uhyper:
            bRet = (m_exprValue->u.uhval == pExpr->getExprValue()->u.uhval) ? sal_True : sal_False;
            break;
        case ET_float:
            bRet = (m_exprValue->u.fval == pExpr->getExprValue()->u.fval) ? sal_True : sal_False;
            break;
        case ET_double:
            bRet = (m_exprValue->u.dval == pExpr->getExprValue()->u.dval) ? sal_True : sal_False;
            break;
        case ET_byte:
            bRet = (m_exprValue->u.byval == pExpr->getExprValue()->u.byval) ? sal_True : sal_False;
            break;
        case ET_boolean:
            bRet = (m_exprValue->u.lval == pExpr->getExprValue()->u.lval) ? sal_True : sal_False;
            break;
        default:
            OSL_ASSERT(false);
            bRet = sal_False;
            break;
    }
    return bRet;
}

void AstExpression::fillDefinitionDetails()
{
    m_pScope = idlc()->scopes()->depth() > 0 ? idlc()->scopes()->top() : NULL;
    m_lineNo = idlc()->getLineNumber();
    m_fileName = idlc()->getFileName();
}

AstExprValue* AstExpression::eval_internal(EvalKind ek)
{
    /*
     * Already evaluated?
     */
    if ( m_exprValue != NULL )
        return eval_kind(m_exprValue, ek);
    /*
     * OK, must evaluate operator
     */
    switch (m_combOperator)
    {
        case EC_add:
        case EC_minus:
        case EC_mul:
        case EC_div:
        case EC_mod:
            m_exprValue = eval_bin_op(ek);
            return eval_kind(m_exprValue, ek);
        case EC_or:
        case EC_xor:
        case EC_and:
        case EC_left:
        case EC_right:
            m_exprValue = eval_bit_op(ek);
            return eval_kind(m_exprValue, ek);
        case EC_u_plus:
        case EC_u_minus:
        case EC_bit_neg:
            m_exprValue = eval_un_op(ek);
            return eval_kind(m_exprValue, ek);
        case EC_symbol:
            m_exprValue = eval_symbol(ek);
            return eval_kind(m_exprValue, ek);
        case EC_none:
            return NULL;
    }

    return NULL;
}

AstExprValue* AstExpression::eval_bin_op(EvalKind ek)
{
    ExprType eType = ET_double;

    if ( m_combOperator == EC_mod )
        eType = ET_hyper;

    if (ek != EK_const && ek != EK_positive_int)
        return NULL;
    if (m_subExpr1 == NULL || m_subExpr2 == NULL)
        return NULL;
    m_subExpr1->setExprValue(m_subExpr1->eval_internal(ek));
    if (m_subExpr1->getExprValue() == NULL)
        return NULL;
    m_subExpr1->setExprValue(m_subExpr1->coerce(eType));
    if (m_subExpr1->getExprValue() == NULL)
        return NULL;
    m_subExpr2->setExprValue(m_subExpr2->eval_internal(ek));
    if (m_subExpr2->getExprValue() == NULL)
        return NULL;
    m_subExpr2->setExprValue(m_subExpr2->coerce(eType));
    if (m_subExpr2->getExprValue() == NULL)
        return NULL;

    std::auto_ptr< AstExprValue > retval(new AstExprValue());
    retval->et = eType;

    switch (m_combOperator)
    {
        case EC_mod:
            if (m_subExpr2->getExprValue()->u.hval == 0)
                return NULL;
            retval->u.hval = m_subExpr1->getExprValue()->u.hval % m_subExpr2->getExprValue()->u.hval;
            break;
        case EC_add:
            retval->u.dval = m_subExpr1->getExprValue()->u.dval + m_subExpr2->getExprValue()->u.dval;
            break;
        case EC_minus:
            retval->u.dval = m_subExpr1->getExprValue()->u.dval - m_subExpr2->getExprValue()->u.dval;
            break;
        case EC_mul:
            retval->u.dval = m_subExpr1->getExprValue()->u.dval * m_subExpr2->getExprValue()->u.dval;
            break;
        case EC_div:
            if (m_subExpr2->getExprValue()->u.dval == 0.0)
                return NULL;
            retval->u.dval = m_subExpr1->getExprValue()->u.dval / m_subExpr2->getExprValue()->u.dval;
            break;
        default:
            return NULL;
    }

    return retval.release();
}

AstExprValue* AstExpression::eval_bit_op(EvalKind ek)
{
    if (ek != EK_const && ek != EK_positive_int)
        return NULL;
    if (m_subExpr1 == NULL || m_subExpr2 == NULL)
        return NULL;
    m_subExpr1->setExprValue(m_subExpr1->eval_internal(ek));
    if (m_subExpr1->getExprValue() == NULL)
        return NULL;
    m_subExpr1->setExprValue(m_subExpr1->coerce(ET_long));
    if (m_subExpr1->getExprValue() == NULL)
        return NULL;
    m_subExpr2->setExprValue(m_subExpr2->eval_internal(ek));
    if (m_subExpr2->getExprValue() == NULL)
        return NULL;
    m_subExpr2->setExprValue(m_subExpr2->coerce(ET_long));
    if (m_subExpr2->getExprValue() == NULL)
        return NULL;

SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< AstExprValue > retval(new AstExprValue());
SAL_WNODEPRECATED_DECLARATIONS_POP
    retval->et = ET_long;

    switch (m_combOperator)
    {
        case EC_or:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval | m_subExpr2->getExprValue()->u.lval;
            break;
        case EC_xor:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval ^ m_subExpr2->getExprValue()->u.lval;
            break;
        case EC_and:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval & m_subExpr2->getExprValue()->u.lval;
            break;
        case EC_left:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval << m_subExpr2->getExprValue()->u.lval;
            break;
        case EC_right:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval >> m_subExpr2->getExprValue()->u.lval;
            break;
        default:
            return NULL;
    }

    return retval.release();
}

AstExprValue* AstExpression::eval_un_op(EvalKind ek)
{
    if (m_exprValue != NULL)
        return m_exprValue;

    if (ek != EK_const && ek != EK_positive_int)
        return NULL;
    if (m_subExpr1 == NULL)
        return NULL;
    m_subExpr1->setExprValue(m_subExpr1->eval_internal(ek));
    if (m_subExpr1->getExprValue() == NULL)
        return NULL;
    m_subExpr1->setExprValue(m_subExpr1->coerce(ET_double));
    if (m_subExpr1->getExprValue() == NULL)
        return NULL;

SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< AstExprValue > retval(new AstExprValue());
SAL_WNODEPRECATED_DECLARATIONS_POP
    retval->et = ET_double;

    switch (m_combOperator)
    {
        case EC_u_plus:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval;
            break;
        case EC_u_minus:
            retval->u.lval = -(m_subExpr1->getExprValue()->u.lval);
            break;
        case EC_bit_neg:
            m_subExpr1->setExprValue(m_subExpr1->coerce(ET_long));
            if (m_subExpr1->getExprValue() == NULL)
                return NULL;
            retval->u.lval = ~m_subExpr1->getExprValue()->u.lval;
            break;
        default:
            return NULL;
    }

    return retval.release();
}

AstExprValue* AstExpression::eval_symbol(EvalKind ek)
{
    AstScope        *pScope = 0;
    AstDeclaration  *pDecl;
    AstConstant     *pConst;

    /*
     * Is there a symbol stored?
     */
    if (m_pSymbolicName == NULL)
    {
        idlc()->error()->evalError(this);
        return NULL;
    }
    /*
     * Get current scope for lookup
     */
    if (idlc()->scopes()->depth() > 0)
        pScope = idlc()->scopes()->topNonNull();
    if ( !pScope )
    {
        idlc()->error()->lookupError(*m_pSymbolicName);
        return NULL;
    }
    /*
     * Do lookup
     */
    pDecl = pScope->lookupByName(*m_pSymbolicName);
    if (pDecl == NULL)
    {
        idlc()->error()->lookupError(*m_pSymbolicName);
        return NULL;
    }
    /*
     * Is it a constant?
     */
    if (pDecl->getNodeType() != NT_const &&
        pDecl->getNodeType() != NT_enum_val)
    {
        idlc()->error()->constantExpected(pDecl, *m_pSymbolicName);
        return NULL;
    }
    if (!idlc()->error()->checkPublished(pDecl))
    {
        return NULL;
    }
    /*
     * OK, now evaluate the constant we just got, to produce its value
     */
    pConst = static_cast< AstConstant* >(pDecl);
    if (pConst == NULL)
        return NULL;
    return pConst->getConstValue()->eval_internal(ek);
}

OString AstExpression::toString()
{
    OString exprStr;
    if ( m_combOperator == EC_symbol )
        return *m_pSymbolicName;

    if ( m_exprValue )
    {
        switch (m_exprValue->et)
        {
            case ET_short:
                return OString::valueOf((sal_Int32)m_exprValue->u.sval);
            case ET_ushort:
                return OString::valueOf((sal_Int32)m_exprValue->u.usval);
            case ET_long:
                return OString::valueOf(m_exprValue->u.lval);
            case ET_ulong:
                return OString::valueOf((sal_Int32)m_exprValue->u.ulval);
            case ET_hyper:
                return OString::valueOf(m_exprValue->u.hval);
            case ET_uhyper:
                return OString::valueOf((sal_Int64)m_exprValue->u.uhval);
            case ET_float:
                return OString::valueOf(m_exprValue->u.fval);
            case ET_double:
                return OString::valueOf(m_exprValue->u.dval);
            case ET_byte:
                return OString::valueOf((sal_Int32)m_exprValue->u.byval);
            case ET_boolean:
                if ( m_exprValue->u.lval == 0)
                    return OString("FALSE");
                else
                    return OString("TRUE");
            default:
                OSL_ASSERT(false);
                return OString();
        }
    }

    switch (m_combOperator)
    {
        case EC_u_plus:
            exprStr += OString("+");
            break;
        case EC_u_minus:
            exprStr += OString("-");
            break;
        case EC_bit_neg:
            exprStr += OString("~");
            break;
        default:
            break;
    }
    if ( m_subExpr1 )
        exprStr += m_subExpr1->toString();
    switch (m_combOperator)
    {
        case EC_add:
            exprStr += OString(" + ");
            break;
        case EC_minus:
            exprStr += OString(" - ");
            break;
        case EC_mul:
            exprStr += OString(" * ");
            break;
        case EC_div:
            exprStr += OString(" / ");
            break;
        case EC_mod:
            exprStr += OString(" % ");
            break;
        case EC_or:
            exprStr += OString(" | ");
            break;
        case EC_xor:
            exprStr += OString(" ^ ");
            break;
        case EC_and:
            exprStr += OString(" & ");
            break;
        case EC_left:
            exprStr += OString(" << ");
            break;
        case EC_right:
            exprStr += OString(" >> ");
            break;
        default:
            break;
    }

    if ( m_subExpr2 )
        exprStr += m_subExpr2->toString();

    return exprStr;
}

// Convert the type of an AST_Expression to a char *
const sal_Char* SAL_CALL exprTypeToString(ExprType t)
{
    switch (t)
    {
        case ET_short:
            return "short";
        case ET_ushort:
            return "unsigned short";
        case ET_long:
            return "long";
        case ET_ulong:
            return "unsigned long";
        case ET_hyper:
            return "hyper";
        case ET_uhyper:
            return "unsigned hyper";
        case ET_float:
            return "float";
        case ET_double:
            return "double";
        case ET_char:
            return "char";
        case ET_byte:
            return "byte";
        case ET_boolean:
            return "boolean";
        case ET_string:
            return "string";
        case ET_any:
            return "any";
        case ET_type:
            return "type";
        case ET_void:
            return "void";
        case ET_none:
            return "none";
    }

    return ("unkown");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
