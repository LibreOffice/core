/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnhancedCustomShapeFunctionParser.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 11:26:05 $
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

#ifndef _ENHANCEDCUSTOMSHAPEFUNCTIONPARSER_HXX
#define _ENHANCEDCUSTOMSHAPEFUNCTIONPARSER_HXX

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif
#ifndef _ENHANCEDCUSTOMSHAPEFUNCTIONPARSER_HXX
#include "EnhancedCustomShapeFunctionParser.hxx"
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETER_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETERTYPE_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#endif
#include <vector>

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

void FillEquationParameter( const com::sun::star::drawing::EnhancedCustomShapeParameter&, const sal_Int32, EnhancedCustomShapeEquation& );

class ExpressionNode
{
public:
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

    static ExpressionNodeSharedPtr parseFunction( const ::rtl::OUString& rFunction, const EnhancedCustomShape2d& rCustoShape );

private:
    // disabled constructor/destructor, since this is
    // supposed to be a singleton
    FunctionParser();

    // default: disabled copy/assignment
    FunctionParser(const FunctionParser&);
    FunctionParser& operator=( const FunctionParser& );
};

}

#endif /* _ENHANCEDCUSTOMSHAPEFUNCTIONPARSER_HXX */
