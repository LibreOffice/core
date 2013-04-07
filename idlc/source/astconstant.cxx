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

#include <idlc/astconstant.hxx>
#include <idlc/astscope.hxx>

#include "registry/writer.hxx"

using namespace ::rtl;

AstConstant::AstConstant(const ExprType type,
                         const NodeType nodeType,
                         AstExpression* pExpr,
                         const OString& name,
                         AstScope* pScope)
    : AstDeclaration(nodeType, name, pScope)
    , m_pConstValue(pExpr)
    , m_constValueType(type)
{
}

AstConstant::AstConstant(const ExprType type,
                         AstExpression* pExpr,
                         const OString& name,
                         AstScope* pScope)
    : AstDeclaration(NT_const, name, pScope)
    , m_pConstValue(pExpr)
    , m_constValueType(type)
{
}

AstConstant::~AstConstant()
{

}

sal_Bool AstConstant::dumpBlob(
    typereg::Writer & rBlob, sal_uInt16 index, bool published)
{
    RTConstValue    aConst;

    AstExprValue *exprVal = getConstValue()->getExprValue();
    switch (getConstValueType())
    {
        case ET_short:
            aConst.m_type = RT_TYPE_INT16;
            aConst.m_value.aShort = exprVal->u.sval;
            break;
        case ET_ushort:
            aConst.m_type = RT_TYPE_UINT16;
            aConst.m_value.aUShort = exprVal->u.usval;
            break;
        case ET_long:
            aConst.m_type = RT_TYPE_INT32;
            aConst.m_value.aLong = exprVal->u.lval;
            break;
        case ET_ulong:
            aConst.m_type = RT_TYPE_UINT32;
            aConst.m_value.aULong = exprVal->u.ulval;
            break;
        case ET_hyper:
            aConst.m_type = RT_TYPE_INT64;
            aConst.m_value.aHyper = exprVal->u.hval;
            break;
        case ET_uhyper:
            aConst.m_type = RT_TYPE_UINT64;
            aConst.m_value.aUHyper = exprVal->u.uhval;
            break;
        case ET_float:
            aConst.m_type = RT_TYPE_FLOAT;
            aConst.m_value.aFloat = exprVal->u.fval;
            break;
        case ET_double:
            aConst.m_type = RT_TYPE_DOUBLE;
            aConst.m_value.aDouble = exprVal->u.dval;
            break;
        case ET_byte:
            aConst.m_type = RT_TYPE_BYTE;
            aConst.m_value.aByte = exprVal->u.byval;
            break;
        case ET_boolean:
            aConst.m_type = RT_TYPE_BOOL;
            aConst.m_value.aBool = exprVal->u.bval;
            break;
        default:
            {
                fprintf(stderr, "%s: exprtype to const type: cannot convert ExprType\n",
                        idlc()->getOptions()->getProgramName().getStr());
                return sal_False;
            }
    }

    OString name = getLocalName();

    OUString type;
    if ( getNodeType() != NT_enum_val )
    {
        type = OStringToOUString(exprTypeToString(getConstValueType()), RTL_TEXTENCODING_UTF8);
    }

    rBlob.setFieldData(
        index, getDocumentation(), OUString(),
        RT_ACCESS_CONST | (published ? RT_ACCESS_PUBLISHED : 0),
        OStringToOUString(name, RTL_TEXTENCODING_UTF8), type, aConst);

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
