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
#include <ostream>
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

enum class ExpressionFunct
{
    Const,

    EnumPi,
    EnumLeft,
    EnumTop,
    EnumRight,
    EnumBottom,
    EnumXStretch,
    EnumYStretch,
    EnumHasStroke,
    EnumHasFill,
    EnumWidth,
    EnumHeight,
    EnumLogWidth,
    EnumLogHeight,
    EnumAdjustment,
    EnumEquation,

    UnaryAbs,
    UnarySqrt,
    UnarySin,
    UnaryCos,
    UnaryTan,
    UnaryAtan,
    UnaryNeg,

    BinaryPlus,
    BinaryMinus,
    BinaryMul,
    BinaryDiv,
    BinaryMin,
    BinaryMax,
    BinaryAtan2,

    TernaryIf
};

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const ExpressionFunct& eFunc )
{
    switch (eFunc)
    {
    case ExpressionFunct::Const : return stream << "const";

    case ExpressionFunct::EnumPi : return stream << "pi";
    case ExpressionFunct::EnumLeft : return stream << "left";
    case ExpressionFunct::EnumTop : return stream << "top";
    case ExpressionFunct::EnumRight : return stream << "right";
    case ExpressionFunct::EnumBottom : return stream << "bottom";
    case ExpressionFunct::EnumXStretch : return stream << "xstretch";
    case ExpressionFunct::EnumYStretch : return stream << "ystretch";
    case ExpressionFunct::EnumHasStroke : return stream << "hasstroke";
    case ExpressionFunct::EnumHasFill : return stream << "hasfill";
    case ExpressionFunct::EnumWidth : return stream << "width";
    case ExpressionFunct::EnumHeight : return stream << "height";
    case ExpressionFunct::EnumLogWidth : return stream << "logwidth";
    case ExpressionFunct::EnumLogHeight : return stream << "logheight";
    case ExpressionFunct::EnumAdjustment : return stream << "adjustment";
    case ExpressionFunct::EnumEquation : return stream << "equation";

    case ExpressionFunct::UnaryAbs : return stream << "abs";
    case ExpressionFunct::UnarySqrt : return stream << "sqrt";
    case ExpressionFunct::UnarySin : return stream << "sin";
    case ExpressionFunct::UnaryCos : return stream << "cos";
    case ExpressionFunct::UnaryTan : return stream << "tan";
    case ExpressionFunct::UnaryAtan : return stream << "atan";
    case ExpressionFunct::UnaryNeg : return stream << "neg";

    case ExpressionFunct::BinaryPlus : return stream << "plus";
    case ExpressionFunct::BinaryMinus : return stream << "minus";
    case ExpressionFunct::BinaryMul : return stream << "mul";
    case ExpressionFunct::BinaryDiv : return stream << "div";
    case ExpressionFunct::BinaryMin : return stream << "min";
    case ExpressionFunct::BinaryMax : return stream << "max";
    case ExpressionFunct::BinaryAtan2 : return stream << "atan2";

    case ExpressionFunct::TernaryIf : return stream << "if";

    default: return stream << "?(" << static_cast<int>(eFunc) << ")";
    }
}

#define EXPRESSION_FLAG_SUMANGLE_MODE 1

SVX_DLLPUBLIC void FillEquationParameter( const css::drawing::EnhancedCustomShapeParameter&, const sal_Int32, EnhancedCustomShapeEquation& );

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
    virtual css::drawing::EnhancedCustomShapeParameter fillNode(
        std::vector< EnhancedCustomShapeEquation >& rEquations, ExpressionNode* pOptionalArg, sal_uInt32 nFlags ) = 0;
};

/** This exception is thrown, when the arithmetic expression
    parser failed to parse a string.
    */
struct ParseError
{
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

    SVX_DLLPUBLIC static std::shared_ptr<ExpressionNode> const & parseFunction( const OUString& rFunction, const EnhancedCustomShape2d& rCustoShape );

    // this is a singleton
    FunctionParser() = delete;
    FunctionParser(const FunctionParser&) = delete;
    FunctionParser& operator=( const FunctionParser& ) = delete;
};

}

#endif // INCLUDED_SVX_ENHANCEDCUSTOMSHAPEFUNCTIONPARSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
