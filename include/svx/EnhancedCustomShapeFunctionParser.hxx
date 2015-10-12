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

#ifndef INCLUDED_SVX_ENHANCEDCUSTOMSHAPEFUNCTIONPARSER_HXX
#define INCLUDED_SVX_ENHANCEDCUSTOMSHAPEFUNCTIONPARSER_HXX

#include <sal/config.h>
#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <memory>
#include <vector>

#include <svx/svxdllapi.h>

struct EnhancedCustomShapeEquation
{
    sal_Int32   nOperation;
    sal_Int32   nPara[ 3 ];

    EnhancedCustomShapeEquation() :
        nOperation  ( 0 )
        {
            nPara[ 0 ] = nPara[ 1 ] = nPara[ 2 ] = 0;
        }
};

class EnhancedCustomShape2d;

namespace EnhancedCustomShape {

enum ExpressionFunct
{
    FUNC_CONST,

    ENUM_FUNC_PI,
    ENUM_FUNC_LEFT,
    ENUM_FUNC_TOP,
    ENUM_FUNC_RIGHT,
    ENUM_FUNC_BOTTOM,
    ENUM_FUNC_XSTRETCH,
    ENUM_FUNC_YSTRETCH,
    ENUM_FUNC_HASSTROKE,
    ENUM_FUNC_HASFILL,
    ENUM_FUNC_WIDTH,
    ENUM_FUNC_HEIGHT,
    ENUM_FUNC_LOGWIDTH,
    ENUM_FUNC_LOGHEIGHT,
    ENUM_FUNC_ADJUSTMENT,
    ENUM_FUNC_EQUATION,

    UNARY_FUNC_ABS,
    UNARY_FUNC_SQRT,
    UNARY_FUNC_SIN,
    UNARY_FUNC_COS,
    UNARY_FUNC_TAN,
    UNARY_FUNC_ATAN,
    UNARY_FUNC_NEG,

    BINARY_FUNC_PLUS,
    BINARY_FUNC_MINUS,
    BINARY_FUNC_MUL,
    BINARY_FUNC_DIV,
    BINARY_FUNC_MIN,
    BINARY_FUNC_MAX,
    BINARY_FUNC_ATAN2,

    TERNARY_FUNC_IF
};

#define EXPRESSION_FLAG_SUMANGLE_MODE 1

SVX_DLLPUBLIC void FillEquationParameter( const com::sun::star::drawing::EnhancedCustomShapeParameter&, const sal_Int32, EnhancedCustomShapeEquation& );

class ExpressionNode
{
public:
    virtual ~ExpressionNode();

    /** Predicate whether this node is constant.

        This predicate returns true, if this node is
        neither time- nor ViewInfo dependent. This allows
        for certain obtimizations, i.e. not the full
        expression tree needs be represented by
        ExpressionNodes.

        @returns true, if the note is constant
    */
    virtual bool isConstant() const = 0;

    /** Operator to calculate function value.

        This method calculates the function value.
    */
    virtual double operator()() const = 0;

    /** Operator to retrieve the type of expression node
    */
    virtual ExpressionFunct getType() const = 0;

    /** Operator to retrieve the ms version of expression
    */
    virtual com::sun::star::drawing::EnhancedCustomShapeParameter fillNode(
        std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* pOptionalArg, sal_uInt32 nFlags ) = 0;
};
typedef std::shared_ptr< ExpressionNode > ExpressionNodeSharedPtr;

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

        identifier = 'pi'|'left'|'top'|'right'|'bottom'|'xstretch'|'ystretch'|
                 'hasstroke'|'hasfill'|'width'|'height'|'logwidth'|'logheight'

        unary_function = 'abs'|'sqrt'|'sin'|'cos'|'tan'|'atan'
        binary_function = 'min'|'max'|'atan2'
        ternary_function = 'if'

        function_reference = '?' 'a-z,A-Z,0-9' ' '
        modifier_reference = '$' '0-9' ' '

        basic_expression =
            number |
            identifier |
            function_reference |
            unary_function '(' additive_expression ')' |
            binary_function '(' additive_expression ',' additive_expression ')' |
            ternary_function '(' additive_expression ',' additive_expression ',
                               ' additive_expression ')' | '(' additive_expression ')'

        unary_expression = '-' basic_expression

        multiplicative_expression =
                          basic_expression |
                          multiplicative_expression '*' basic_expression |
                          multiplicative_expression '/' basic_expression

        additive_expression =
                        multiplicative_expression |
                        additive_expression '+' multiplicative_expression |
                        additive_expression '-' multiplicative_expression

        </code>

        @param rFunction
        The string to parse

        @param rCustoShape
        The CustomShape is required for calculation of dynamic values such
        "hasstroke", function references and or modifier references ...

        @throws ParseError if an invalid expression is given.

        @return the generated function object.
       */

    SVX_DLLPUBLIC static ExpressionNodeSharedPtr parseFunction( const OUString& rFunction, const EnhancedCustomShape2d& rCustoShape );

private:
    // disabled constructor/destructor, since this is
    // supposed to be a singleton
    FunctionParser();

    FunctionParser(const FunctionParser&) = delete;
    FunctionParser& operator=( const FunctionParser& ) = delete;
};

}

#endif // INCLUDED_SVX_ENHANCEDCUSTOMSHAPEFUNCTIONPARSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
