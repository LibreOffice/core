/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RowFunctionParser.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 07:59:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONNECTIVITY_ROWFUNCTIONPARSER_HXX_INCLUDED
#define CONNECTIVITY_ROWFUNCTIONPARSER_HXX_INCLUDED

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif
#include "FDatabaseMetaDataResultSet.hxx"
#include <vector>

// -------------------------------------------------------------------------
namespace connectivity
{
// -------------------------------------------------------------------------
struct RowEquation
{
    sal_Int32   nOperation;
    ORowSetValueDecoratorRef    nPara[ 3 ];

    RowEquation() :
        nOperation  ( 0 )
        {
        }
};

enum ExpressionFunct
{
    FUNC_CONST,

    ENUM_FUNC_EQUATION,

    UNARY_FUNC_COLUMN,
    ENUM_FUNC_AND,
    ENUM_FUNC_OR
};

#define EXPRESSION_FLAG_SUMANGLE_MODE 1

class SAL_NO_VTABLE ExpressionNode
{
public:
    virtual ~ExpressionNode(){}

    /** Operator to calculate function value.

        This method calculates the function value.
    */
    virtual ORowSetValueDecoratorRef evaluate(const ODatabaseMetaDataResultSet::ORow& _aRow ) const = 0;

    virtual void fill(const ODatabaseMetaDataResultSet::ORow& _aRow ) const = 0;

    /** Operator to retrieve the type of expression node
    */
    virtual ExpressionFunct getType() const = 0;

    /** Operator to retrieve the ms version of expression
    */
    virtual ODatabaseMetaDataResultSet::ORow fillNode(
        std::vector< RowEquation >& rEquations, ExpressionNode* pOptionalArg, sal_uInt32 nFlags ) = 0;
};
typedef ::boost::shared_ptr< ExpressionNode > ExpressionNodeSharedPtr;

/** This exception is thrown, when the arithmetic expression
    parser failed to parse a string.
    */
struct ParseError
{
    ParseError() {}
    ParseError( const char* ) {}
};

class FunctionParser
{
public:

    /** Parse a string

        The following grammar is accepted by this method:
        <code>

        number_digit = '0'|'1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'

        number = number number_digit | number_digit

        equal_function = '='
        ternary_function = 'if'

        string_reference = 'a-z,A-Z,0-9' ' '
        modifier_reference = '$' '0-9' ' '

        basic_expression =
            number |
            string_reference |
            additive_expression equal_function additive_expression |
            unary_function '(' number_digit ')'
            ternary_function '(' additive_expression ',' additive_expression ',
                               ' additive_expression ')' | '(' additive_expression ')'

        </code>

        @param rFunction
        The string to parse

        @throws ParseError if an invalid expression is given.

        @return the generated function object.
       */

    static ExpressionNodeSharedPtr parseFunction( const ::rtl::OUString& _sFunction);

private:
    // disabled constructor/destructor, since this is
    // supposed to be a singleton
    FunctionParser();

    // default: disabled copy/assignment
    FunctionParser(const FunctionParser&);
    FunctionParser& operator=( const FunctionParser& );
};

// -------------------------------------------------------------------------
} // namespace connectivity
// -------------------------------------------------------------------------
#endif /* CONNECTIVITY_ROWFUNCTIONPARSER_HXX_INCLUDED */
