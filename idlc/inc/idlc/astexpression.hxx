/*************************************************************************
 *
 *  $RCSfile: astexpression.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:23:01 $
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
#define _IDLC_ASTEXPRESSION_HXX_

#ifndef _IDLC_IDLC_HXX_
#include <idlc/idlc.hxx>
#endif

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
    ET_none,        // Expression value is missing
    ET_pseudo       // Expression value is pseudo (for keywords)
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
        sal_Bool        bval;       // Contains boolean expression value
        float           fval;       // Contains 32-bit float expr value
        double          dval;       // Contains 64-bit float expr value
        sal_Char        cval;       // Contains char expression value
        ::rtl::OString* strval;     // Contains String * expr value
        sal_uInt32      eval;       // Contains enumeration value
    } u;
    ExprType et;
};

sal_Char* SAL_CALL exprTypeToString(ExprType t);

class AstExpression
{
public:
    // Constructor(s)
    AstExpression(AstExpression *pExpr, ExprType et);
    AstExpression(ExprComb c, AstExpression *pExpr1, AstExpression *pExpr2);

    AstExpression(sal_Int16         s);
    AstExpression(sal_uInt16        us);
    AstExpression(sal_Int32         l);
    AstExpression(sal_Int32         l, ExprType et);
    AstExpression(sal_uInt32        ul);
    AstExpression(sal_Int64         h);
    AstExpression(sal_uInt64        uh);
    AstExpression(float             f);
    AstExpression(double            d);
    AstExpression(sal_Char          c);
    AstExpression(::rtl::OString* s, sal_Bool bIsScopedName = sal_False);

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
    AstExprValue* eval(EvalKind ek);
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

