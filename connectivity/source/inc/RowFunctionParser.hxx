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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ROWFUNCTIONPARSER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ROWFUNCTIONPARSER_HXX

#include <sal/config.h>
#include "FDatabaseMetaDataResultSet.hxx"
#include <connectivity/dbtoolsdllapi.hxx>
#include <memory>
#include <vector>

namespace connectivity
{

enum class ExpressionFunct
{
    Equation,
    And,
    Or
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
};

/** This exception is thrown, when the arithmetic expression
    parser failed to parse a string.
    */
struct OOO_DLLPUBLIC_DBTOOLS ParseError
{
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

    static std::shared_ptr<ExpressionNode> parseFunction( const OUString& _sFunction);

private:
    // disabled constructor/destructor, since this is
    // supposed to be a singleton
    FunctionParser() = delete;
    FunctionParser(const FunctionParser&) = delete;
    FunctionParser& operator=( const FunctionParser& ) = delete;
};


} // namespace connectivity

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ROWFUNCTIONPARSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
