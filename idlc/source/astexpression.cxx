/*************************************************************************
 *
 *  $RCSfile: astexpression.cxx,v $
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
#ifndef _IDLC_ASTEXPRESSION_HXX_
#include <idlc/astexpression.hxx>
#endif
#ifndef _IDLC_ASTCONSTANT_HXX_
#include <idlc/astconstant.hxx>
#endif
#ifndef _IDLC_ASTSCOPE_HXX_
#include <idlc/astscope.hxx>
#endif
#ifndef _IDLC_ERRORHANDLER_HXX_
#include <idlc/errorhandler.hxx>
#endif

#include <limits.h>
#include <float.h>

#undef  MAXCHAR
#define MAXCHAR 127
#undef  MINCHAR
#define MINCHAR -128

#define MAXINT64 9223372036854775807
#define MININT64 -9223372036854775807-1
#define MAXUINT64 18446744073709551615

using namespace ::rtl;

AstExpression::AstExpression(AstExpression *pExpr, ExprType et)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    m_exprValue = pExpr->coerce(et);
    if ( !m_exprValue)
    {
        idlc()->error()->coercionError(pExpr, et);
    }
}

AstExpression::AstExpression(ExprComb c, AstExpression *pExpr1, AstExpression *pExpr2)
    : m_combOperator(c)
    , m_subExpr1(pExpr1)
    , m_subExpr2(pExpr2)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

}

AstExpression::AstExpression(sal_Int16 s)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    m_exprValue = new AstExprValue();
    m_exprValue->et = ET_short;
    m_exprValue->u.sval = s;
}

AstExpression::AstExpression(sal_uInt16 us)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    m_exprValue = new AstExprValue();
    m_exprValue->et = ET_ushort;
    m_exprValue->u.usval = us;
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

AstExpression::AstExpression(sal_uInt32 ul)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    m_exprValue = new AstExprValue();
    m_exprValue->et = ET_ulong;
    m_exprValue->u.ulval = ul;
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

AstExpression::AstExpression(float f)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    m_exprValue = new AstExprValue();
    m_exprValue->et = ET_float;
    m_exprValue->u.fval = f;
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

AstExpression::AstExpression(sal_Char c)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    m_exprValue = new AstExprValue();
    m_exprValue->et = ET_char;
    m_exprValue->u.cval = c;
}

AstExpression::AstExpression(::rtl::OString* s, sal_Bool bIsScopedName)
    : m_combOperator(EC_none)
    , m_subExpr1(NULL)
    , m_subExpr2(NULL)
    , m_exprValue(NULL)
    , m_pSymbolicName(NULL)
{
    fillDefinitionDetails();

    if ( bIsScopedName )
    {
        m_pSymbolicName = s;
        m_combOperator = EC_symbol;
    } else
    {
        m_exprValue = new AstExprValue();
        m_exprValue->et = ET_string;
        m_exprValue->u.strval = s;
    }
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
                    if ((sal_uInt16)ev->u.usval > SHRT_MAX)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.usval;
                    ev->et = ET_short;
                    return ev;
                case ET_long:
                    if ((sal_Int16)ev->u.lval > SHRT_MAX || (sal_Int16)ev->u.lval < SHRT_MIN)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.lval;
                    ev->et = ET_short;
                    return ev;
                case ET_ulong:
                    if ((sal_Int16)ev->u.ulval > SHRT_MAX)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.ulval;
                    ev->et = ET_short;
                    return ev;
                case ET_hyper:
                    if ((sal_Int16)ev->u.hval > SHRT_MAX || (sal_Int16)ev->u.hval < SHRT_MIN)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.hval;
                    ev->et = ET_short;
                    return ev;
                case ET_uhyper:
                    if ((sal_Int16)ev->u.uhval > SHRT_MAX)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.uhval;
                    ev->et = ET_short;
                    return ev;
                case ET_boolean:
                    ev->u.sval = (sal_Int16)ev->u.bval;
                    ev->et = ET_short;
                    return ev;
                case ET_float:
                    if ((sal_Int16)ev->u.fval > SHRT_MAX || (sal_Int16)ev->u.fval < SHRT_MIN)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.fval;
                    ev->et = ET_short;
                    return ev;
                case ET_double:
                    if ((sal_Int16)ev->u.dval > SHRT_MAX || (sal_Int16)ev->u.dval < SHRT_MIN)
                        return NULL;
                    ev->u.sval = (sal_Int16)ev->u.dval;
                    ev->et = ET_short;
                    return ev;
                case ET_char:
                    ev->u.sval = (sal_Int16)ev->u.cval;
                    ev->et = ET_short;
                    return ev;
                case ET_byte:
                    ev->u.sval = (sal_Int16)ev->u.byval;
                    ev->et = ET_short;
                    return ev;
                case ET_string:
                case ET_any:
                case ET_void:
                case ET_type:
                case ET_none:
                    return NULL;
            }
        case ET_ushort:
            switch (ev->et)
            {
                case ET_short:
                    if (ev->u.sval < 0 && ev->u.sval != USHRT_MAX)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.sval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_ushort:
                    return ev;
                case ET_long:
                    if ((sal_uInt16)ev->u.lval > USHRT_MAX || ev->u.lval < 0)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.lval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_ulong:
                    if ((sal_uInt16)ev->u.ulval > USHRT_MAX)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.ulval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_hyper:
                    if ((sal_uInt16)ev->u.hval > USHRT_MAX || ev->u.hval < 0)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.hval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_uhyper:
                    if ((sal_uInt16)ev->u.uhval > USHRT_MAX)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.uhval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_boolean:
                    ev->u.usval = (sal_uInt16)ev->u.bval;
                    ev->et = ET_short;
                    return ev;
                case ET_float:
                    if (ev->u.fval < 0.0 || (sal_uInt16)ev->u.fval > USHRT_MAX)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.fval;
                    ev->et = ET_short;
                    return ev;
                case ET_double:
                    if (ev->u.dval < 0.0 || (sal_uInt16)ev->u.dval > USHRT_MAX)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.dval;
                    ev->et = ET_short;
                    return ev;
                case ET_char:
                    if (ev->u.cval < 0)
                        return NULL;
                    ev->u.usval = (sal_uInt16)ev->u.cval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_byte:
                    ev->u.usval = (sal_uInt16)ev->u.byval;
                    ev->et = ET_ushort;
                    return ev;
                case ET_string:
                case ET_any:
                case ET_void:
                case ET_type:
                case ET_none:
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
                    if ((sal_Int32)ev->u.ulval > LONG_MAX)
                        return NULL;
                    ev->u.lval = (sal_Int32)ev->u.ulval;
                    ev->et = ET_long;
                    return ev;
                case ET_hyper:
                    if ((sal_Int32)ev->u.hval > LONG_MAX || (sal_Int32)ev->u.hval < LONG_MIN)
                        return NULL;
                    ev->u.lval = (sal_Int32)ev->u.hval;
                    ev->et = ET_long;
                    return ev;
                case ET_uhyper:
                    if ((sal_Int32)ev->u.uhval > LONG_MAX)
                        return NULL;
                    ev->u.lval = (sal_Int32)ev->u.uhval;
                    ev->et = ET_long;
                    return ev;
                case ET_boolean:
                    ev->u.lval = (sal_Int32)ev->u.bval;
                    ev->et = ET_long;
                    return ev;
                case ET_float:
                    if ((sal_Int32)ev->u.fval > LONG_MAX || (sal_Int32)ev->u.fval < LONG_MIN)
                        return NULL;
                    ev->u.lval = (sal_Int32)ev->u.fval;
                    ev->et = ET_long;
                    return ev;
                case ET_double:
                    if ((sal_Int32)ev->u.dval > LONG_MAX || (sal_Int32)ev->u.dval < LONG_MIN)
                        return NULL;
                    ev->u.lval = (sal_Int32)ev->u.dval;
                    ev->et = ET_long;
                    return ev;
                case ET_char:
                    ev->u.lval = (sal_Int32) ev->u.cval;
                    ev->et = ET_long;
                    return ev;
                case ET_byte:
                    ev->u.lval = (sal_Int32) ev->u.byval;
                    ev->et = ET_long;
                    return ev;
                case ET_string:
                case ET_any:
                case ET_void:
                case ET_type:
                case ET_none:
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
                    if ((sal_uInt32)ev->u.lval < 0 && (sal_uInt32)ev->u.lval != ULONG_MAX)
                        return NULL;
                    ev->u.ulval = (sal_uInt32)ev->u.lval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_ulong:
                    return ev;
                case ET_hyper:
                    if (ev->u.hval > (sal_Int64)ULONG_MAX || ev->u.hval < 0)
                        return NULL;
                    ev->u.lval = (sal_uInt32)ev->u.hval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_uhyper:
                    if (ev->u.uhval > (sal_uInt64)ULONG_MAX)
                        return NULL;
                    ev->u.ulval = (sal_uInt32)ev->u.uhval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_boolean:
                    ev->u.ulval = (sal_uInt32)ev->u.bval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_float:
                    if (ev->u.fval < 0.0 || (sal_uInt32)ev->u.fval > ULONG_MAX)
                        return NULL;
                    ev->u.ulval = (sal_uInt32)ev->u.fval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_double:
                    if (ev->u.dval < 0.0 || (sal_uInt32)ev->u.dval > ULONG_MAX)
                        return NULL;
                    ev->u.ulval = (sal_uInt32)ev->u.dval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_char:
                    if (ev->u.cval < 0)
                        return NULL;
                    ev->u.ulval = (sal_uInt32)ev->u.cval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_byte:
                    ev->u.ulval = (sal_uInt32)ev->u.byval;
                    ev->et = ET_ulong;
                    return ev;
                case ET_string:
                case ET_any:
                case ET_void:
                case ET_type:
                case ET_none:
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
                    if (ev->u.uhval > (sal_uInt64)MAXINT64)
                        return NULL;
                    ev->u.hval = (sal_Int64)ev->u.uhval;
                    ev->et = ET_long;
                    return ev;
                case ET_boolean:
                    ev->u.hval = (sal_Int64)ev->u.bval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_float:
                    if ((sal_Int64)ev->u.fval > MAXINT64 || (sal_Int64)ev->u.fval < MININT64)
                        return NULL;
                    ev->u.hval = (sal_Int64)ev->u.fval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_double:
                    if ((sal_Int64)ev->u.dval > MAXINT64 || (sal_Int64)ev->u.dval < MININT64)
                        return NULL;
                    ev->u.hval = (sal_Int64)ev->u.dval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_char:
                    ev->u.hval = (sal_Int64)ev->u.cval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_byte:
                    ev->u.hval = (sal_Int64)ev->u.byval;
                    ev->et = ET_hyper;
                    return ev;
                case ET_string:
                case ET_any:
                case ET_void:
                case ET_type:
                case ET_none:
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
                    if ((sal_uInt64)ev->u.lval < 0 && ev->u.lval != MAXUINT64)
                        return NULL;
                    ev->u.uhval = (sal_uInt64)ev->u.lval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_ulong:
                    ev->u.uhval = (sal_uInt64)ev->u.ulval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_hyper:
                    if ((sal_uInt64)ev->u.hval < 0 && ev->u.hval != MAXUINT64)
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
                    if (ev->u.fval < 0.0 || (sal_uInt64)ev->u.fval > MAXUINT64)
                        return NULL;
                    ev->u.uhval = (sal_uInt64)ev->u.fval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_double:
                    if (ev->u.dval < 0.0 || (sal_uInt64)ev->u.dval > MAXUINT64)
                        return NULL;
                    ev->u.uhval = (sal_uInt64)ev->u.dval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_char:
                    if (ev->u.cval < 0)
                        return NULL;
                    ev->u.uhval = (sal_uInt64)ev->u.cval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_byte:
                    ev->u.uhval = (sal_uInt64)ev->u.byval;
                    ev->et = ET_uhyper;
                    return ev;
                case ET_string:
                case ET_any:
                case ET_void:
                case ET_type:
                case ET_none:
                    return NULL;
            }
        case ET_boolean:
            switch (ev->et)
            {
                case ET_short:
                    ev->u.bval = (ev->u.sval == 0) ? sal_False : sal_True;
                    ev->et = ET_boolean;
                    return ev;
                case ET_ushort:
                    ev->u.bval = (ev->u.usval == 0) ? sal_False : sal_True;
                    ev->et = ET_boolean;
                    return ev;
                case ET_long:
                    ev->u.bval = (ev->u.lval == 0) ? sal_False : sal_True;
                    ev->et = ET_boolean;
                    return ev;
                case ET_ulong:
                    ev->u.bval = (ev->u.ulval == 0) ? sal_False : sal_True;
                    ev->et = ET_boolean;
                    return ev;
                case ET_hyper:
                    ev->u.bval = (ev->u.hval == 0) ? sal_False : sal_True;
                    ev->et = ET_boolean;
                    return ev;
                case ET_uhyper:
                    ev->u.bval = (ev->u.uhval == 0) ? sal_False : sal_True;
                    ev->et = ET_boolean;
                    return ev;
                case ET_boolean:
                    return ev;
                case ET_float:
                    ev->u.bval = (ev->u.fval == 0.0) ? sal_False : sal_True;
                    ev->et = ET_boolean;
                    return ev;
                case ET_double:
                    ev->u.bval = (ev->u.dval == 0.0) ? sal_False : sal_True;
                    ev->et = ET_boolean;
                    return ev;
                case ET_char:
                    ev->u.bval = (ev->u.cval == 0) ? sal_False : sal_True;
                    ev->et = ET_boolean;
                    return ev;
                case ET_byte:
                    ev->u.bval = (ev->u.byval == 0) ? sal_False : sal_True;
                    ev->et = ET_boolean;
                    return ev;
                case ET_string:
                case ET_any:
                case ET_void:
                case ET_type:
                case ET_none:
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
                    return NULL;
                case ET_boolean:
                    ev->u.fval = (ev->u.bval == sal_True) ? 1.0f : 0.0f;
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
                case ET_char:
                    ev->u.fval = (float)ev->u.cval;
                    ev->et = ET_float;
                    return ev;
                case ET_byte:
                    ev->u.fval = (float)ev->u.byval;
                    ev->et = ET_float;
                    return ev;
                case ET_string:
                case ET_any:
                case ET_void:
                case ET_type:
                case ET_none:
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
                    ev->u.dval = (ev->u.bval == sal_True) ? 1.0 : 0.0;
                    ev->et = ET_double;
                    return ev;
                case ET_float:
                    ev->u.dval = (double)ev->u.fval;
                    ev->et = ET_double;
                    return ev;
                case ET_double:
                    return ev;
                case ET_char:
                    ev->u.dval = (double)ev->u.cval;
                    ev->et = ET_double;
                    return ev;
                case ET_byte:
                    ev->u.dval = (double)ev->u.byval;
                    ev->et = ET_double;
                    return ev;
                case ET_string:
                case ET_any:
                case ET_void:
                case ET_type:
                case ET_none:
                    return NULL;
            }
        case ET_char:
            switch (ev->et)
            {
                case ET_short:
                    if ((sal_Char)ev->u.sval > MAXCHAR || (sal_Char)ev->u.sval < MINCHAR)
                        return NULL;
                    ev->u.cval = (sal_Char)ev->u.sval;
                    ev->et = ET_char;
                    return ev;
                case ET_ushort:
                    if ((sal_Char)ev->u.usval > MAXCHAR)
                        return NULL;
                    ev->u.cval = (sal_Char)ev->u.usval;
                    ev->et = ET_char;
                    return ev;
                case ET_long:
                    if ((sal_Char)ev->u.lval > MAXCHAR || (sal_Char)ev->u.lval < MINCHAR)
                        return NULL;
                    ev->u.cval = (sal_Char)ev->u.lval;
                    ev->et = ET_char;
                    return ev;
                case ET_ulong:
                    if ((sal_Char)ev->u.ulval > MAXCHAR)
                        return NULL;
                    ev->u.cval = (sal_Char)ev->u.ulval;
                    ev->et = ET_char;
                    return ev;
                case ET_hyper:
                    if ((sal_Char)ev->u.hval > MAXCHAR || (sal_Char)ev->u.hval < MINCHAR)
                        return NULL;
                    ev->u.cval = (sal_Char)ev->u.hval;
                    ev->et = ET_char;
                    return ev;
                case ET_uhyper:
                    if ((sal_Char)ev->u.ulval > MAXCHAR)
                        return NULL;
                    ev->u.cval = (sal_Char)ev->u.uhval;
                    ev->et = ET_char;
                    return ev;
                case ET_boolean:
                    ev->u.cval = (sal_Char)ev->u.bval;
                    ev->et = ET_char;
                    return ev;
                case ET_float:
                    if ((sal_Char)ev->u.fval > MAXCHAR || (sal_Char)ev->u.fval < MINCHAR)
                        return NULL;
                    ev->u.cval = (sal_Char)ev->u.fval;
                    ev->et = ET_char;
                    return ev;
                case ET_double:
                    if ((sal_Char)ev->u.dval > MAXCHAR || (sal_Char)ev->u.dval < MINCHAR)
                        return NULL;
                    ev->u.cval = (sal_Char)ev->u.dval;
                    ev->et = ET_char;
                    return ev;
                case ET_char:
                    return ev;
                case ET_byte:
                    if ((sal_Char)ev->u.byval > MAXCHAR)
                        return NULL;
                    ev->u.cval = (sal_Char)ev->u.byval;
                    ev->et = ET_char;
                    return ev;
                case ET_string:
                case ET_any:
                case ET_void:
                case ET_type:
                case ET_none:
                    return NULL;
            }
        case ET_byte:
            switch (ev->et)
            {
                case ET_short:
                    if (ev->u.sval < 0 || (sal_uChar)ev->u.sval > (MAXCHAR << 1))
                        return NULL;
                    ev->u.byval = (sal_uChar)ev->u.sval;
                    ev->et = ET_byte;
                    return ev;
                case ET_ushort:
                    if ((sal_uChar)ev->u.usval > (MAXCHAR << 1))
                        return NULL;
                    ev->u.byval = (sal_uChar)ev->u.usval;
                    ev->et = ET_byte;
                    return ev;
                case ET_long:
                    if (ev->u.lval < 0 || (sal_uChar)ev->u.lval > (MAXCHAR << 1))
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.lval;
                    ev->et = ET_byte;
                    return ev;
                case ET_ulong:
                    if ((sal_uChar)ev->u.ulval > (MAXCHAR << 1))
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.ulval;
                    ev->et = ET_byte;
                    return ev;
                case ET_hyper:
                    if (ev->u.hval < 0 || (sal_uChar)ev->u.hval > (MAXCHAR << 1))
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.hval;
                    ev->et = ET_byte;
                    return ev;
                case ET_uhyper:
                    if ((sal_uChar)ev->u.uhval > (MAXCHAR << 1))
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.uhval;
                    ev->et = ET_byte;
                    return ev;
                case ET_boolean:
                    ev->u.byval = (ev->u.bval == sal_False) ? 1 : 0;
                    ev->et = ET_byte;
                    return ev;
                case ET_float:
                    if (ev->u.fval < 0.0 || (sal_uChar)ev->u.fval > (MAXCHAR << 1))
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.fval;
                    ev->et = ET_byte;
                    return ev;
                case ET_double:
                    if (ev->u.dval < 0.0 || (sal_uChar)ev->u.dval > (MAXCHAR << 1))
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.dval;
                    ev->et = ET_byte;
                    return ev;
                case ET_char:
                    if (ev->u.cval < 0)
                        return NULL;
                    ev->u.byval = (sal_uChar) ev->u.cval;
                    ev->et = ET_byte;
                    return ev;
                case ET_byte:
                    return ev;
                case ET_string:
                case ET_any:
                case ET_void:
                case ET_type:
                case ET_none:
                    return NULL;
            }
        case ET_any:
            switch (ev->et)
            {
                case ET_any:
                    return ev;
                default:
                    return NULL;
            }
        case ET_void:
            switch (ev->et)
            {
                case ET_void:
                    return ev;
                default:
                    return NULL;
            }
        case ET_type:
            switch (ev->et)
            {
                case ET_type:
                    return ev;
                default:
                    return NULL;
            }
        case ET_none:
            return NULL;
        case ET_string:
            switch (ev->et)
            {
                case ET_string:
                    return ev;
                default:
                    return NULL;
            }
      }

    return NULL;
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



AstExprValue* AstExpression::eval(EvalKind ek)
{
    AstExprValue *v = NULL;

    /*
     * Call internal evaluator which does not coerce value to
     * EvalKind-expected format
     */
    v = eval_internal(ek);
    /*
     * Then coerce according to EvalKind-expected format
     */
    return eval_kind(v, ek);
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
        case ET_void:
        case ET_none:
        case ET_any:
            return NULL;
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
        case ET_char:
            copy->u.cval = m_exprValue->u.cval;
            break;
          case ET_byte:
            copy->u.byval = m_exprValue->u.byval;
            break;
          case ET_string:
            copy->u.strval = m_exprValue->u.strval;
            break;
    }

    if (bAssign)
    {
        m_exprValue = coerce_value(copy, t);
        return m_exprValue;
    }

    return coerce_value(copy, t);
}

void AstExpression::evaluate(EvalKind ek)
{
    m_exprValue = eval_internal(ek);
    m_exprValue = eval_kind(m_exprValue, ek);
}

sal_Bool AstExpression::operator==(AstExpression *pExpr)
{
    if (m_combOperator != pExpr->getCombOperator())
        return sal_False;
    evaluate(EK_const);
    pExpr->evaluate(EK_const);
    if (m_exprValue == NULL || pExpr->getExprValue() == NULL)
        return sal_False;
    if (m_exprValue->et != pExpr->getExprValue()->et)
        return sal_False;
    switch (m_exprValue->et)
    {
        case ET_short:
            return (m_exprValue->u.sval == pExpr->getExprValue()->u.sval) ? sal_True : sal_False;
        case ET_ushort:
            return (m_exprValue->u.usval == pExpr->getExprValue()->u.usval) ? sal_True : sal_False;
        case ET_long:
            return (m_exprValue->u.lval == pExpr->getExprValue()->u.lval) ? sal_True : sal_False;
        case ET_ulong:
            return (m_exprValue->u.ulval == pExpr->getExprValue()->u.ulval) ? sal_True : sal_False;
        case ET_hyper:
            return (m_exprValue->u.hval == pExpr->getExprValue()->u.hval) ? sal_True : sal_False;
        case ET_uhyper:
            return (m_exprValue->u.uhval == pExpr->getExprValue()->u.uhval) ? sal_True : sal_False;
        case ET_float:
            return (m_exprValue->u.fval == pExpr->getExprValue()->u.fval) ? sal_True : sal_False;
        case ET_double:
            return (m_exprValue->u.dval == pExpr->getExprValue()->u.dval) ? sal_True : sal_False;
        case ET_char:
            return (m_exprValue->u.cval == pExpr->getExprValue()->u.cval) ? sal_True : sal_False;
        case ET_byte:
            return (m_exprValue->u.byval == pExpr->getExprValue()->u.byval) ? sal_True : sal_False;
        case ET_boolean:
            return (m_exprValue->u.lval == pExpr->getExprValue()->u.lval) ? sal_True : sal_False;
        case ET_string:
            if (m_exprValue->u.strval == NULL)
            {
                if (pExpr->getExprValue()->u.strval == NULL)
                    return sal_True;
                else
                    return sal_False;
            } else
                if (pExpr->getExprValue()->u.strval == NULL)
                    return sal_False;
                else
                    return (m_exprValue->u.strval == pExpr->getExprValue()->u.strval) ? sal_True : sal_False;
        case ET_any:
        case ET_void:
        case ET_none:
            return sal_False;
    }

    return sal_False;
}

sal_Bool AstExpression::compare(AstExpression *pExpr)
{
    if (m_combOperator != pExpr->getCombOperator())
        return sal_False;
    evaluate(EK_const);
    pExpr->evaluate(EK_const);
    if (m_exprValue == NULL || pExpr->getExprValue() == NULL)
        return sal_False;
    if (m_exprValue->et != pExpr->getExprValue()->et)
        return sal_False;
    switch (m_exprValue->et)
    {
        case ET_short:
            return (m_exprValue->u.sval == pExpr->getExprValue()->u.sval) ? sal_True : sal_False;
        case ET_ushort:
            return (m_exprValue->u.usval == pExpr->getExprValue()->u.usval) ? sal_True : sal_False;
        case ET_long:
            return (m_exprValue->u.lval == pExpr->getExprValue()->u.lval) ? sal_True : sal_False;
        case ET_ulong:
            return (m_exprValue->u.ulval == pExpr->getExprValue()->u.ulval) ? sal_True : sal_False;
        case ET_hyper:
            return (m_exprValue->u.hval == pExpr->getExprValue()->u.hval) ? sal_True : sal_False;
        case ET_uhyper:
            return (m_exprValue->u.uhval == pExpr->getExprValue()->u.uhval) ? sal_True : sal_False;
        case ET_float:
            return (m_exprValue->u.fval == pExpr->getExprValue()->u.fval) ? sal_True : sal_False;
        case ET_double:
            return (m_exprValue->u.dval == pExpr->getExprValue()->u.dval) ? sal_True : sal_False;
        case ET_char:
            return (m_exprValue->u.cval == pExpr->getExprValue()->u.cval) ? sal_True : sal_False;
        case ET_byte:
            return (m_exprValue->u.byval == pExpr->getExprValue()->u.byval) ? sal_True : sal_False;
        case ET_boolean:
            return (m_exprValue->u.lval == pExpr->getExprValue()->u.lval) ? sal_True : sal_False;
        case ET_string:
            if (m_exprValue->u.strval == NULL)
            {
                if (pExpr->getExprValue()->u.strval == NULL)
                    return sal_True;
                else
                    return sal_False;
            } else
                if (pExpr->getExprValue()->u.strval == NULL)
                    return sal_False;
                else
                    return (m_exprValue->u.strval == pExpr->getExprValue()->u.strval) ? sal_True : sal_False;
        case ET_any:
        case ET_void:
        case ET_none:
            return sal_False;
    }

    return sal_False;
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
    AstExprValue *retval = NULL;
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

    retval = new AstExprValue();
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

    return retval;
}

AstExprValue* AstExpression::eval_bit_op(EvalKind ek)
{
    AstExprValue    *retval = NULL;

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

    retval = new AstExprValue;
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

    return retval;
}

AstExprValue* AstExpression::eval_un_op(EvalKind ek)
{
    AstExprValue    *retval = NULL;

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

    retval = new AstExprValue();
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

    return retval;
}

AstExprValue* AstExpression::eval_symbol(EvalKind ek)
{
    AstScope        *pScope;
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
    /*
     * OK, now evaluate the constant we just got, to produce its value
     */
    pConst = reinterpret_cast< AstConstant* >(pDecl);
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
                return OString::valueOf((sal_Int64)m_exprValue->u.ulval);
            case ET_hyper:
                return OString::valueOf(m_exprValue->u.hval);
            case ET_uhyper:
                return OString::valueOf((sal_Int64)m_exprValue->u.uhval);
            case ET_float:
                return OString::valueOf(m_exprValue->u.fval);
            case ET_double:
                return OString::valueOf(m_exprValue->u.dval);
            case ET_char:
                return OString::valueOf(m_exprValue->u.cval);
            case ET_byte:
                return OString::valueOf((sal_Int32)m_exprValue->u.byval);
            case ET_boolean:
                if ( m_exprValue->u.lval == 0)
                    return OString("FALSE");
                else
                    return OString("TRUE");
            case ET_string:
                return *(m_exprValue->u.strval);
            case ET_any:
            case ET_void:
            case ET_none:
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
    }

    if ( m_subExpr2 )
        exprStr += m_subExpr2->toString();

    return exprStr;
}

// Convert the type of an AST_Expression to a char *
sal_Char* SAL_CALL exprTypeToString(ExprType t)
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
