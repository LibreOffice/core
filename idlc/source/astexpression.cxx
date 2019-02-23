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


#include <astexpression.hxx>
#include <astconstant.hxx>
#include <astscope.hxx>
#include <errorhandler.hxx>

#include <osl/diagnose.h>

#include <limits.h>
#include <float.h>
#include <memory>

AstExpression::AstExpression(ExprComb c, AstExpression *pExpr1, AstExpression *pExpr2)
    : m_combOperator(c)
    , m_subExpr1(pExpr1)
    , m_subExpr2(pExpr2)
{
}

AstExpression::AstExpression(sal_Int32 l)
    : m_combOperator(ExprComb::NONE)
{
    m_exprValue.reset( new AstExprValue );
    m_exprValue->et = ET_long;
    m_exprValue->u.lval = l;
}

AstExpression::AstExpression(sal_Int32  l, ExprType et)
    : m_combOperator(ExprComb::NONE)
{
    m_exprValue.reset( new AstExprValue );
    m_exprValue->et = et;
    m_exprValue->u.lval = l;
}

AstExpression::AstExpression(sal_Int64  h)
    : m_combOperator(ExprComb::NONE)
{
    m_exprValue.reset( new AstExprValue );
    m_exprValue->et = ET_hyper;
    m_exprValue->u.hval = h;
}

AstExpression::AstExpression(sal_uInt64 uh)
    : m_combOperator(ExprComb::NONE)
{
    m_exprValue.reset( new AstExprValue );
    m_exprValue->et = ET_uhyper;
    m_exprValue->u.uhval = uh;
}

AstExpression::AstExpression(double d)
    : m_combOperator(ExprComb::NONE)
{
    m_exprValue.reset( new AstExprValue );
    m_exprValue->et = ET_double;
    m_exprValue->u.dval = d;
}

AstExpression::AstExpression(OString* scopedName)
    : m_combOperator(ExprComb::Symbol)
{
    if (scopedName)
        m_xSymbolicName = *scopedName;
}

AstExpression::~AstExpression()
{
}

/*
 * Perform the coercion from the given AstExprValue to the requested
 * ExprType. Return an AstExprValue if successful, NULL if failed.
 * must be done for hyper, uhyper
 */
static bool
coerce_value(AstExprValue *ev, ExprType t)
{
    if (ev == nullptr)
        return false;

    switch (t)
    {
        case ET_short:
            switch (ev->et)
            {
                case ET_short:
                    return true;
                case ET_ushort:
                    if (ev->u.usval > SAL_MAX_INT16)
                        return false;
                    ev->u.sval = static_cast<sal_Int16>(ev->u.usval);
                    ev->et = ET_short;
                    return true;
                case ET_long:
                    if (ev->u.lval < SAL_MIN_INT16 || ev->u.lval > SAL_MAX_INT16)
                        return false;
                    ev->u.sval = static_cast<sal_Int16>(ev->u.lval);
                    ev->et = ET_short;
                    return true;
                case ET_ulong:
                    if (ev->u.ulval > SAL_MAX_INT16)
                        return false;
                    ev->u.sval = static_cast<sal_Int16>(ev->u.ulval);
                    ev->et = ET_short;
                    return true;
                case ET_hyper:
                    if (ev->u.hval < SAL_MIN_INT16 || ev->u.hval > SAL_MAX_INT16)
                        return false;
                    ev->u.sval = static_cast<sal_Int16>(ev->u.hval);
                    ev->et = ET_short;
                    return true;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_INT16)
                        return false;
                    ev->u.sval = static_cast<sal_Int16>(ev->u.uhval);
                    ev->et = ET_short;
                    return true;
                case ET_boolean:
                    ev->u.sval = static_cast<sal_Int16>(ev->u.bval);
                    ev->et = ET_short;
                    return true;
                case ET_float:
                    if (ev->u.fval < SAL_MIN_INT16 || ev->u.fval > SAL_MAX_INT16)
                        return false;
                    ev->u.sval = static_cast<sal_Int16>(ev->u.fval);
                    ev->et = ET_short;
                    return true;
                case ET_double:
                    if (ev->u.dval < SAL_MIN_INT16 || ev->u.dval > SAL_MAX_INT16)
                        return false;
                    ev->u.sval = static_cast<sal_Int16>(ev->u.dval);
                    ev->et = ET_short;
                    return true;
                case ET_byte:
                    ev->u.sval = static_cast<sal_Int16>(ev->u.byval);
                    ev->et = ET_short;
                    return true;
                default:
                    OSL_ASSERT(false);
                    return false;
            }
        case ET_ushort:
            switch (ev->et)
            {
                case ET_short:
                    if (ev->u.sval < 0)
                        return false;
                    ev->u.usval = static_cast<sal_uInt16>(ev->u.sval);
                    ev->et = ET_ushort;
                    return true;
                case ET_ushort:
                    return true;
                case ET_long:
                    if (ev->u.lval < 0 || ev->u.lval > SAL_MAX_UINT16)
                        return false;
                    ev->u.usval = static_cast<sal_uInt16>(ev->u.lval);
                    ev->et = ET_ushort;
                    return true;
                case ET_ulong:
                    if (ev->u.ulval > SAL_MAX_UINT16)
                        return false;
                    ev->u.usval = static_cast<sal_uInt16>(ev->u.ulval);
                    ev->et = ET_ushort;
                    return true;
                case ET_hyper:
                    if (ev->u.hval < 0 || ev->u.hval > SAL_MAX_UINT16)
                        return false;
                    ev->u.usval = static_cast<sal_uInt16>(ev->u.hval);
                    ev->et = ET_ushort;
                    return true;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_UINT16)
                        return false;
                    ev->u.usval = static_cast<sal_uInt16>(ev->u.uhval);
                    ev->et = ET_ushort;
                    return true;
                case ET_boolean:
                    ev->u.usval = static_cast<sal_uInt16>(ev->u.bval);
                    ev->et = ET_short;
                    return true;
                case ET_float:
                    if (ev->u.fval < 0.0 || ev->u.fval > SAL_MAX_UINT16)
                        return false;
                    ev->u.usval = static_cast<sal_uInt16>(ev->u.fval);
                    ev->et = ET_short;
                    return true;
                case ET_double:
                    if (ev->u.dval < 0.0 || ev->u.dval > SAL_MAX_UINT16)
                        return false;
                    ev->u.usval = static_cast<sal_uInt16>(ev->u.dval);
                    ev->et = ET_short;
                    return true;
                case ET_byte:
                    ev->u.usval = static_cast<sal_uInt16>(ev->u.byval);
                    ev->et = ET_ushort;
                    return true;
                default:
                    OSL_ASSERT(false);
                    return false;
            }
        case ET_long:
            switch (ev->et)
            {
                case ET_short:
                    ev->u.lval = static_cast<sal_Int32>(ev->u.sval);
                    ev->et = ET_long;
                    return true;
                case ET_ushort:
                    ev->u.lval = static_cast<sal_Int32>(ev->u.usval);
                    ev->et = ET_long;
                    return true;
                case ET_long:
                    return true;
                case ET_ulong:
                    if (ev->u.ulval > SAL_MAX_INT32)
                        return false;
                    ev->u.lval = static_cast<sal_Int32>(ev->u.ulval);
                    ev->et = ET_long;
                    return true;
                case ET_hyper:
                    if (ev->u.hval < SAL_MIN_INT32 || ev->u.hval > SAL_MAX_INT32)
                        return false;
                    ev->u.lval = static_cast<sal_Int32>(ev->u.hval);
                    ev->et = ET_long;
                    return true;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_INT32)
                        return false;
                    ev->u.lval = static_cast<sal_Int32>(ev->u.uhval);
                    ev->et = ET_long;
                    return true;
                case ET_boolean:
                    ev->u.lval = static_cast<sal_Int32>(ev->u.bval);
                    ev->et = ET_long;
                    return true;
                case ET_float:
                    if (ev->u.fval < SAL_MIN_INT32 || ev->u.fval > SAL_MAX_INT32)
                        return false;
                    ev->u.lval = static_cast<sal_Int32>(ev->u.fval);
                    ev->et = ET_long;
                    return true;
                case ET_double:
                    if (ev->u.dval < SAL_MIN_INT32 || ev->u.dval > SAL_MAX_INT32)
                        return false;
                    ev->u.lval = static_cast<sal_Int32>(ev->u.dval);
                    ev->et = ET_long;
                    return true;
                case ET_byte:
                    ev->u.lval = static_cast<sal_Int32>(ev->u.byval);
                    ev->et = ET_long;
                    return true;
                default:
                    OSL_ASSERT(false);
                    return false;
            }
        case ET_ulong:
            switch (ev->et)
            {
                case ET_short:
                    if (ev->u.sval < 0)
                        return false;
                    ev->u.ulval = static_cast<sal_uInt32>(ev->u.sval);
                    ev->et = ET_ulong;
                    return true;
                case ET_ushort:
                    ev->u.ulval = static_cast<sal_uInt32>(ev->u.usval);
                    ev->et = ET_ulong;
                    return true;
                case ET_long:
                    if (ev->u.lval < 0)
                        return false;
                    ev->u.ulval = static_cast<sal_uInt32>(ev->u.lval);
                    ev->et = ET_ulong;
                    return true;
                case ET_ulong:
                    return true;
                case ET_hyper:
                    if (ev->u.hval < 0 || ev->u.hval > SAL_MAX_UINT32)
                        return false;
                    ev->u.lval = static_cast<sal_uInt32>(ev->u.hval);
                    ev->et = ET_ulong;
                    return true;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_UINT32)
                        return false;
                    ev->u.ulval = static_cast<sal_uInt32>(ev->u.uhval);
                    ev->et = ET_ulong;
                    return true;
                case ET_boolean:
                    ev->u.ulval = static_cast<sal_uInt32>(ev->u.bval);
                    ev->et = ET_ulong;
                    return true;
                case ET_float:
                    if (ev->u.fval < 0.0 || ev->u.fval > SAL_MAX_UINT32)
                        return false;
                    ev->u.ulval = static_cast<sal_uInt32>(ev->u.fval);
                    ev->et = ET_ulong;
                    return true;
                case ET_double:
                    if (ev->u.dval < 0.0 || ev->u.dval > SAL_MAX_UINT32)
                        return false;
                    ev->u.ulval = static_cast<sal_uInt32>(ev->u.dval);
                    ev->et = ET_ulong;
                    return true;
                case ET_byte:
                    ev->u.ulval = static_cast<sal_uInt32>(ev->u.byval);
                    ev->et = ET_ulong;
                    return true;
                default:
                    OSL_ASSERT(false);
                    return false;
            }
        case ET_hyper:
            switch (ev->et)
            {
                case ET_short:
                    ev->u.hval = static_cast<sal_Int64>(ev->u.sval);
                    ev->et = ET_hyper;
                    return true;
                case ET_ushort:
                    ev->u.hval = static_cast<sal_Int64>(ev->u.usval);
                    ev->et = ET_hyper;
                    return true;
                case ET_long:
                    ev->u.hval = static_cast<sal_Int64>(ev->u.lval);
                    ev->et = ET_hyper;
                    return true;
                case ET_ulong:
                    ev->u.hval = static_cast<sal_Int64>(ev->u.ulval);
                    ev->et = ET_hyper;
                    return true;
                case ET_hyper:
                    return true;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_INT64)
                        return false;
                    ev->u.hval = static_cast<sal_Int64>(ev->u.uhval);
                    ev->et = ET_long;
                    return true;
                case ET_boolean:
                    ev->u.hval = static_cast<sal_Int64>(ev->u.bval);
                    ev->et = ET_hyper;
                    return true;
                case ET_float:
                    if (ev->u.fval < SAL_MIN_INT64 || ev->u.fval > SAL_MAX_INT64)
                        return false;
                    ev->u.hval = static_cast<sal_Int64>(ev->u.fval);
                    ev->et = ET_hyper;
                    return true;
                case ET_double:
                    if (ev->u.dval < SAL_MIN_INT64 || ev->u.dval > SAL_MAX_INT64)
                        return false;
                    ev->u.hval = static_cast<sal_Int64>(ev->u.dval);
                    ev->et = ET_hyper;
                    return true;
                case ET_byte:
                    ev->u.hval = static_cast<sal_Int64>(ev->u.byval);
                    ev->et = ET_hyper;
                    return true;
                default:
                    OSL_ASSERT(false);
                    return false;
            }
        case ET_uhyper:
            switch (ev->et)
            {
                case ET_short:
                    if (ev->u.sval < 0)
                        return false;
                    ev->u.uhval = static_cast<sal_uInt64>(ev->u.sval);
                    ev->et = ET_uhyper;
                    return true;
                case ET_ushort:
                    ev->u.uhval = static_cast<sal_uInt64>(ev->u.usval);
                    ev->et = ET_uhyper;
                    return true;
                case ET_long:
                    if (ev->u.lval < 0)
                        return false;
                    ev->u.uhval = static_cast<sal_uInt64>(ev->u.lval);
                    ev->et = ET_uhyper;
                    return true;
                case ET_ulong:
                    ev->u.uhval = static_cast<sal_uInt64>(ev->u.ulval);
                    ev->et = ET_uhyper;
                    return true;
                case ET_hyper:
                    if (ev->u.hval < 0)
                        return false;
                    ev->u.uhval = static_cast<sal_uInt64>(ev->u.hval);
                    ev->et = ET_uhyper;
                    return true;
                case ET_uhyper:
                    return true;
                case ET_boolean:
                    ev->u.uhval = static_cast<sal_uInt64>(ev->u.bval);
                    ev->et = ET_uhyper;
                    return true;
                case ET_float:
                    if (ev->u.fval < 0.0 || ev->u.fval > SAL_MAX_UINT64)
                        return false;
                    ev->u.uhval = static_cast<sal_uInt64>(ev->u.fval);
                    ev->et = ET_uhyper;
                    return true;
                case ET_double:
                    if (ev->u.dval < 0.0 || ev->u.dval > SAL_MAX_UINT64)
                        return false;
                    ev->u.uhval = static_cast<sal_uInt64>(ev->u.dval);
                    ev->et = ET_uhyper;
                    return true;
                case ET_byte:
                    ev->u.uhval = static_cast<sal_uInt64>(ev->u.byval);
                    ev->et = ET_uhyper;
                    return true;
                default:
                    OSL_ASSERT(false);
                    return false;
            }
        case ET_boolean:
            switch (ev->et)
            {
                case ET_short:
                    ev->u.bval = ev->u.sval != 0;
                    ev->et = ET_boolean;
                    return true;
                case ET_ushort:
                    ev->u.bval = ev->u.usval != 0;
                    ev->et = ET_boolean;
                    return true;
                case ET_long:
                    ev->u.bval = ev->u.lval != 0;
                    ev->et = ET_boolean;
                    return true;
                case ET_ulong:
                    ev->u.bval = ev->u.ulval != 0;
                    ev->et = ET_boolean;
                    return true;
                case ET_hyper:
                    ev->u.bval = ev->u.hval != 0;
                    ev->et = ET_boolean;
                    return true;
                case ET_uhyper:
                    ev->u.bval = ev->u.uhval != 0;
                    ev->et = ET_boolean;
                    return true;
                case ET_boolean:
                    return true;
                case ET_float:
                    ev->u.bval = ev->u.fval != 0.0;
                    ev->et = ET_boolean;
                    return true;
                case ET_double:
                    ev->u.bval = ev->u.dval != 0.0;
                    ev->et = ET_boolean;
                    return true;
                case ET_byte:
                    ev->u.bval = ev->u.byval != 0;
                    ev->et = ET_boolean;
                    return true;
                default:
                    OSL_ASSERT(false);
                    return false;
            }
        case ET_float:
            switch (ev->et)
            {
                case ET_short:
                    ev->u.fval = static_cast<float>(ev->u.sval);
                    ev->et = ET_float;
                    return true;
                case ET_ushort:
                    ev->u.fval = static_cast<float>(ev->u.usval);
                    ev->et = ET_float;
                    return true;
                case ET_long:
                    ev->u.fval = static_cast<float>(ev->u.lval);
                    ev->et = ET_float;
                    return true;
                case ET_ulong:
                    ev->u.fval = static_cast<float>(ev->u.ulval);
                    ev->et = ET_float;
                    return true;
                case ET_hyper:
                    ev->u.fval = static_cast<float>(ev->u.hval);
                    ev->et = ET_float;
                    return true;
                case ET_uhyper:
                    if (static_cast<float>(ev->u.ulval) > FLT_MAX)
                        return false;
                    ev->u.fval = static_cast<float>(ev->u.ulval);
                    ev->et = ET_float;
                    return true;
                case ET_boolean:
                    ev->u.fval = ev->u.bval ? 1.0f : 0.0f;
                    ev->et = ET_float;
                    return true;
                case ET_float:
                    return true;
                case ET_double:
                    if (static_cast<float>(ev->u.dval) > FLT_MAX || static_cast<float>(ev->u.dval) < -FLT_MAX)
                        return false;
                    ev->u.fval = static_cast<float>(ev->u.dval);
                    ev->et = ET_float;
                    return true;
                case ET_byte:
                    ev->u.fval = static_cast<float>(ev->u.byval);
                    ev->et = ET_float;
                    return true;
                default:
                    OSL_ASSERT(false);
                    return false;
            }
        case ET_double:
            switch (ev->et)
            {
                case ET_short:
                    ev->u.dval = static_cast<double>(ev->u.sval);
                    ev->et = ET_double;
                    return true;
                case ET_ushort:
                    ev->u.dval = static_cast<double>(ev->u.usval);
                    ev->et = ET_double;
                    return true;
                case ET_long:
                    ev->u.dval = static_cast<double>(ev->u.lval);
                    ev->et = ET_double;
                    return true;
                case ET_ulong:
                    ev->u.dval = static_cast<double>(ev->u.ulval);
                    ev->et = ET_double;
                    return true;
                case ET_hyper:
                    ev->u.dval = static_cast<double>(ev->u.hval);
                    ev->et = ET_double;
                    return true;
                case ET_uhyper:
                    if (ev->u.dval > FLT_MAX || ev->u.dval < -FLT_MAX)
                        return false;
                    ev->u.dval = static_cast<double>(ev->u.ulval);
                    ev->et = ET_double;
                    return true;
                case ET_boolean:
                    ev->u.dval = ev->u.bval ? 1.0 : 0.0;
                    ev->et = ET_double;
                    return true;
                case ET_float:
                    ev->u.dval = static_cast<double>(ev->u.fval);
                    ev->et = ET_double;
                    return true;
                case ET_double:
                    return true;
                case ET_byte:
                    ev->u.dval = static_cast<double>(ev->u.byval);
                    ev->et = ET_double;
                    return true;
                default:
                    OSL_ASSERT(false);
                    return false;
            }
        case ET_byte:
            switch (ev->et)
            {
                case ET_short:
                    if (ev->u.sval < SAL_MIN_INT8 || ev->u.sval > SAL_MAX_UINT8)
                        return false;
                    ev->u.byval = static_cast<unsigned char>(ev->u.sval);
                    ev->et = ET_byte;
                    return true;
                case ET_ushort:
                    if (ev->u.usval > SAL_MAX_UINT8)
                        return false;
                    ev->u.byval = static_cast<unsigned char>(ev->u.usval);
                    ev->et = ET_byte;
                    return true;
                case ET_long:
                    if (ev->u.lval < SAL_MIN_INT8 || ev->u.lval > SAL_MAX_UINT8)
                        return false;
                    ev->u.byval = static_cast<unsigned char>(ev->u.lval);
                    ev->et = ET_byte;
                    return true;
                case ET_ulong:
                    if (ev->u.ulval > SAL_MAX_UINT8)
                        return false;
                    ev->u.byval = static_cast<unsigned char>(ev->u.ulval);
                    ev->et = ET_byte;
                    return true;
                case ET_hyper:
                    if (ev->u.hval < SAL_MIN_INT8 || ev->u.hval > SAL_MAX_UINT8)
                        return false;
                    ev->u.byval = static_cast<unsigned char>(ev->u.hval);
                    ev->et = ET_byte;
                    return true;
                case ET_uhyper:
                    if (ev->u.uhval > SAL_MAX_UINT8)
                        return false;
                    ev->u.byval = static_cast<unsigned char>(ev->u.uhval);
                    ev->et = ET_byte;
                    return true;
                case ET_boolean:
                    ev->u.byval = ev->u.bval ? 1 : 0;
                    ev->et = ET_byte;
                    return true;
                case ET_float:
                    if (ev->u.fval < SAL_MIN_INT8 || ev->u.fval > SAL_MAX_UINT8)
                        return false;
                    ev->u.byval = static_cast<unsigned char>(ev->u.fval);
                    ev->et = ET_byte;
                    return true;
                case ET_double:
                    if (ev->u.dval < SAL_MIN_INT8 || ev->u.dval > SAL_MAX_UINT8)
                        return false;
                    ev->u.byval = static_cast<unsigned char>(ev->u.dval);
                    ev->et = ET_byte;
                    return true;
                case ET_byte:
                    return true;
                default:
                    OSL_ASSERT(false);
                    return false;
            }
        default:
            OSL_ASSERT(false);
            return false;
    }
}

bool AstExpression::coerce(ExprType t)
{
    /*
     * Is it already of the right type?
     */
    if (m_exprValue != nullptr && m_exprValue->et == t)
        return true;
    /*
     * OK, must coerce
     *
     * First, evaluate it, then try to coerce result type
     * If already evaluated, return the result
     */
    evaluate();
    if (m_exprValue == nullptr)
        return false;

    if (!coerce_value(m_exprValue.get(), t))
        m_exprValue.reset();

    return m_exprValue != nullptr;
}

bool AstExpression::compareLong(AstExpression *pExpr)
{
    bool bRet = false;
    if (m_combOperator != pExpr->m_combOperator)
        return bRet;
    evaluate();
    pExpr->evaluate();
    if (m_exprValue == nullptr || pExpr->getExprValue() == nullptr)
        return bRet;
    if (m_exprValue->et != pExpr->getExprValue()->et)
        return bRet;
    switch (m_exprValue->et)
    {
        case ET_long:
            bRet = m_exprValue->u.lval == pExpr->getExprValue()->u.lval;
            break;
        default:
            OSL_ASSERT(false);
            bRet = false;
            break;
    }
    return bRet;
}

void AstExpression::evaluate()
{
    /*
     * Already evaluated?
     */
    if ( m_exprValue != nullptr )
        return;
    /*
     * OK, must evaluate operator
     */
    switch (m_combOperator)
    {
        case ExprComb::Add:
        case ExprComb::Minus:
        case ExprComb::Mul:
        case ExprComb::Div:
        case ExprComb::Mod:
            m_exprValue = eval_bin_op();
            break;
        case ExprComb::Or:
        case ExprComb::Xor:
        case ExprComb::And:
        case ExprComb::Left:
        case ExprComb::Right:
            m_exprValue = eval_bit_op();
            break;
        case ExprComb::UPlus:
        case ExprComb::UMinus:
            m_exprValue = eval_un_op();
            break;
        case ExprComb::Symbol:
            m_exprValue = eval_symbol();
            break;
        case ExprComb::NONE:
            break;
    }
}

std::unique_ptr<AstExprValue> AstExpression::eval_bin_op()
{
    ExprType eType = ET_double;

    if ( m_combOperator == ExprComb::Mod )
        eType = ET_hyper;

    if (m_subExpr1 == nullptr || m_subExpr2 == nullptr)
        return nullptr;
    m_subExpr1->evaluate();
    if (m_subExpr1->getExprValue() == nullptr)
        return nullptr;
    if (!m_subExpr1->coerce(eType))
        return nullptr;
    m_subExpr2->evaluate();
    if (m_subExpr2->getExprValue() == nullptr)
        return nullptr;
    if (!m_subExpr2->coerce(eType))
        return nullptr;

    std::unique_ptr< AstExprValue > retval(new AstExprValue);
    retval->et = eType;

    switch (m_combOperator)
    {
        case ExprComb::Mod:
            if (m_subExpr2->getExprValue()->u.hval == 0)
                return nullptr;
            retval->u.hval = m_subExpr1->getExprValue()->u.hval % m_subExpr2->getExprValue()->u.hval;
            break;
        case ExprComb::Add:
            retval->u.dval = m_subExpr1->getExprValue()->u.dval + m_subExpr2->getExprValue()->u.dval;
            break;
        case ExprComb::Minus:
            retval->u.dval = m_subExpr1->getExprValue()->u.dval - m_subExpr2->getExprValue()->u.dval;
            break;
        case ExprComb::Mul:
            retval->u.dval = m_subExpr1->getExprValue()->u.dval * m_subExpr2->getExprValue()->u.dval;
            break;
        case ExprComb::Div:
            if (m_subExpr2->getExprValue()->u.dval == 0.0)
                return nullptr;
            retval->u.dval = m_subExpr1->getExprValue()->u.dval / m_subExpr2->getExprValue()->u.dval;
            break;
        default:
            return nullptr;
    }

    return retval;
}

std::unique_ptr<AstExprValue> AstExpression::eval_bit_op()
{
    if (m_subExpr1 == nullptr || m_subExpr2 == nullptr)
        return nullptr;
    m_subExpr1->evaluate();
    if (m_subExpr1->getExprValue() == nullptr)
        return nullptr;
    if (!m_subExpr1->coerce(ET_long))
        return nullptr;
    m_subExpr2->evaluate();
    if (m_subExpr2->getExprValue() == nullptr)
        return nullptr;
    if (!m_subExpr2->coerce(ET_long))
        return nullptr;

    std::unique_ptr< AstExprValue > retval(new AstExprValue);
    retval->et = ET_long;

    switch (m_combOperator)
    {
        case ExprComb::Or:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval | m_subExpr2->getExprValue()->u.lval;
            break;
        case ExprComb::Xor:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval ^ m_subExpr2->getExprValue()->u.lval;
            break;
        case ExprComb::And:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval & m_subExpr2->getExprValue()->u.lval;
            break;
        case ExprComb::Left:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval << m_subExpr2->getExprValue()->u.lval;
            break;
        case ExprComb::Right:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval >> m_subExpr2->getExprValue()->u.lval;
            break;
        default:
            return nullptr;
    }

    return retval;
}

std::unique_ptr<AstExprValue> AstExpression::eval_un_op()
{
    if (m_subExpr1 == nullptr)
        return nullptr;
    m_subExpr1->evaluate();
    if (m_subExpr1->getExprValue() == nullptr)
        return nullptr;
    if (!m_subExpr1->coerce(ET_double))
        return nullptr;

    std::unique_ptr< AstExprValue > retval(new AstExprValue);
    retval->et = ET_double;

    switch (m_combOperator)
    {
        case ExprComb::UPlus:
            retval->u.lval = m_subExpr1->getExprValue()->u.lval;
            break;
        case ExprComb::UMinus:
            retval->u.lval = -(m_subExpr1->getExprValue()->u.lval);
            break;
        default:
            return nullptr;
    }

    return retval;
}

std::unique_ptr<AstExprValue> AstExpression::eval_symbol()
{
    AstScope        *pScope = nullptr;
    AstDeclaration  *pDecl;
    AstConstant     *pConst;

    /*
     * Is there a symbol stored?
     */
    if (!m_xSymbolicName)
    {
        ErrorHandler::evalError(this);
        return nullptr;
    }
    /*
     * Get current scope for lookup
     */
    if (idlc()->scopes()->depth() > 0)
        pScope = idlc()->scopes()->topNonNull();
    if ( !pScope )
    {
        ErrorHandler::lookupError(*m_xSymbolicName);
        return nullptr;
    }
    /*
     * Do lookup
     */
    pDecl = pScope->lookupByName(*m_xSymbolicName);
    if (pDecl == nullptr)
    {
        ErrorHandler::lookupError(*m_xSymbolicName);
        return nullptr;
    }
    /*
     * Is it a constant?
     */
    if (pDecl->getNodeType() != NT_const &&
        pDecl->getNodeType() != NT_enum_val)
    {
        ErrorHandler::constantExpected(pDecl, *m_xSymbolicName);
        return nullptr;
    }
    if (!ErrorHandler::checkPublished(pDecl))
    {
        return nullptr;
    }
    /*
     * OK, now evaluate the constant we just got, to produce its value
     */
    pConst = static_cast< AstConstant* >(pDecl);
    pConst->getConstValue()->evaluate();
    auto const val = pConst->getConstValue()->getExprValue();
    return val == nullptr ? nullptr : std::make_unique<AstExprValue>(*val);
}

OString AstExpression::toString()
{
    OString exprStr;
    if ( m_combOperator == ExprComb::Symbol )
        return m_xSymbolicName ? *m_xSymbolicName : OString("<Undefined Name>");

    if ( m_exprValue )
    {
        switch (m_exprValue->et)
        {
            case ET_short:
                return OString::number(m_exprValue->u.sval);
            case ET_ushort:
                return OString::number(m_exprValue->u.usval);
            case ET_long:
                return OString::number(m_exprValue->u.lval);
            case ET_ulong:
                return OString::number(m_exprValue->u.ulval);
            case ET_hyper:
                return OString::number(m_exprValue->u.hval);
            case ET_uhyper:
                return OString::number(m_exprValue->u.uhval);
            case ET_float:
                return OString::number(m_exprValue->u.fval);
            case ET_double:
                return OString::number(m_exprValue->u.dval);
            case ET_byte:
                return OString::number(m_exprValue->u.byval);
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
        case ExprComb::UPlus:
            exprStr += "+";
            break;
        case ExprComb::UMinus:
            exprStr += "-";
            break;
        default:
            break;
    }
    if ( m_subExpr1 )
        exprStr += m_subExpr1->toString();
    switch (m_combOperator)
    {
        case ExprComb::Add:
            exprStr += " + ";
            break;
        case ExprComb::Minus:
            exprStr += " - ";
            break;
        case ExprComb::Mul:
            exprStr += " * ";
            break;
        case ExprComb::Div:
            exprStr += " / ";
            break;
        case ExprComb::Mod:
            exprStr += " % ";
            break;
        case ExprComb::Or:
            exprStr += " | ";
            break;
        case ExprComb::Xor:
            exprStr += " ^ ";
            break;
        case ExprComb::And:
            exprStr += " & ";
            break;
        case ExprComb::Left:
            exprStr += " << ";
            break;
        case ExprComb::Right:
            exprStr += " >> ";
            break;
        default:
            break;
    }

    if ( m_subExpr2 )
        exprStr += m_subExpr2->toString();

    return exprStr;
}

// Convert the type of an AST_Expression to a char *
const sal_Char* exprTypeToString(ExprType t)
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

    return "unknown";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
