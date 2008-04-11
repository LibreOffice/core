/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astconstant.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idlc.hxx"
#include <idlc/astconstant.hxx>
#include <idlc/astscope.hxx>

#include "registry/writer.hxx"

using namespace ::rtl;

AstConstant::AstConstant(const ExprType type,
                         const NodeType nodeType,
                         AstExpression* pExpr,
                         const ::rtl::OString& name,
                         AstScope* pScope)
    : AstDeclaration(nodeType, name, pScope)
    , m_pConstValue(pExpr)
    , m_constValueType(type)
{
}

AstConstant::AstConstant(const ExprType type,
                         AstExpression* pExpr,
                         const ::rtl::OString& name,
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
    sal_Unicode*    str = NULL;

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
    if (str)
        delete[] str;

    return sal_True;
}
