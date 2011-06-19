/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef CONNECTIVITY_ROWFUNCTIONPARSER_HXX_INCLUDED
#define CONNECTIVITY_ROWFUNCTIONPARSER_HXX_INCLUDED

#include <sal/config.h>
#include <boost/shared_ptr.hpp>
#include "FDatabaseMetaDataResultSet.hxx"
#include <vector>
#include "connectivity/dbtoolsdllapi.hxx"

// -------------------------------------------------------------------------
namespace connectivity
{
// -------------------------------------------------------------------------
struct OOO_DLLPUBLIC_DBTOOLS RowEquation
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

class OOO_DLLPUBLIC_DBTOOLS SAL_NO_VTABLE ExpressionNode
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
struct OOO_DLLPUBLIC_DBTOOLS ParseError
{
    ParseError() {}
    ParseError( const char* ) {}
};

class OOO_DLLPUBLIC_DBTOOLS FunctionParser
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
