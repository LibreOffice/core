/*************************************************************************
 *
 *  $RCSfile: astconstant.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:30:43 $
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
#ifndef _IDLC_ASTCONSTANT_HXX_
#include <idlc/astconstant.hxx>
#endif
#ifndef _IDLC_ASTSCOPE_HXX_
#include <idlc/astscope.hxx>
#endif

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

sal_Bool AstConstant::dumpBlob(RegistryTypeWriter& rBlob, sal_uInt16 index)
{
    RTConstValue    aConst;
    sal_Unicode*    str = NULL;
    sal_Bool        bRelativ = sal_False;
    if ( getScope()->getScopeNodeType() == NT_constants )
        bRelativ = sal_True;

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
        case ET_char:
            break;
        case ET_byte:
            aConst.m_type = RT_TYPE_BYTE;
            aConst.m_value.aByte = exprVal->u.byval;
            break;
        case ET_boolean:
            aConst.m_type = RT_TYPE_BOOL;
            aConst.m_value.aBool = exprVal->u.bval;
            break;
        case ET_string:
            {
                aConst.m_type = RT_TYPE_STRING;
                ::rtl::OUString aTempStr( OUString::createFromAscii(exprVal->u.strval->getStr()));
                str = new sal_Unicode[aTempStr.getLength()+1];
                rtl_copyMemory(str, aTempStr.getStr(), (aTempStr.getLength()+1) * sizeof(sal_Unicode));
                aConst.m_value.aString = str;
            }
            break;
        default:
            {
                fprintf(stderr, "%s: exprtype to const type: cannot convert ExprType\n",
                        idlc()->getOptions()->getProgramName().getStr());
                return sal_False;
            }
    }

    OString name(getRelativName());
    if ( getNodeType() == NT_enum_val || bRelativ )
        name = getLocalName();

    OUString type;
    OUString fileName;
    if ( getNodeType() != NT_enum_val )
    {
        type = OStringToOUString(exprTypeToString(getConstValueType()), RTL_TEXTENCODING_UTF8);
        fileName = OStringToOUString(getFileName(), RTL_TEXTENCODING_UTF8);
    }

    rBlob.setFieldData(index, OStringToOUString(name, RTL_TEXTENCODING_UTF8), type,
                       getDocumentation(), fileName, RT_ACCESS_CONST, aConst);
    if (str)
        delete[] str;

    return sal_True;
}
