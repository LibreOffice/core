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


#include <tools/diagnose_ex.h>

#include <rtl/math.hxx>

#include <smilfunctionparser.hxx>
#include <expressionnodefactory.hxx>

#include <rtl/ustring.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>

// Makes parser a static resource,
// we're synchronized externally.
// But watch out, the parser might have
// state not visible to this code!
#define BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
#if defined(DBG_UTIL)
#include <typeinfo>
#define BOOST_SPIRIT_DEBUG
#endif
#include <boost/spirit/include/classic_core.hpp>

#include <iostream>
#include <functional>
#include <algorithm>
#include <stack>



/* Implementation of SmilFunctionParser class */

namespace slideshow
{
    namespace internal
    {
        namespace
        {
            typedef const sal_Char*                   StringIteratorT;

            struct ParserContext
            {
                typedef ::std::stack< ExpressionNodeSharedPtr > OperandStack;

                // stores a stack of not-yet-evaluated operands. This is used
                // by the operators (i.e. '+', '*', 'sin' etc.) to pop their
                // arguments from. If all arguments to an operator are constant,
                // the operator pushes a precalculated result on the stack, and
                // a composite ExpressionNode otherwise.
                OperandStack                maOperandStack;

                // bounds of the shape this expression is associated with
                ::basegfx::B2DRectangle     maShapeBounds;

                // when true, enable usage of time-dependent variable '$'
                // in expressions
                bool                        mbParseAnimationFunction;
            };

            typedef ::boost::shared_ptr< ParserContext > ParserContextSharedPtr;


            template< typename Generator > class ShapeBoundsFunctor
            {
            public:
                ShapeBoundsFunctor( Generator                       aGenerator,
                                    const ParserContextSharedPtr&   rContext ) :
                    maGenerator( aGenerator ),
                    mpContext( rContext )
                {
                    ENSURE_OR_THROW( mpContext,
                                      "ShapeBoundsFunctor::ShapeBoundsFunctor(): Invalid context" );
                }

                void operator()( StringIteratorT, StringIteratorT ) const
                {
                    mpContext->maOperandStack.push(
                        ExpressionNodeFactory::createConstantValueExpression(
                            maGenerator( mpContext->maShapeBounds ) ) );
                }

            private:
                Generator               maGenerator;
                ParserContextSharedPtr  mpContext;
            };

            template< typename Generator > ShapeBoundsFunctor< Generator >
                makeShapeBoundsFunctor( const Generator&                rGenerator,
                                        const ParserContextSharedPtr&   rContext )
            {
                return ShapeBoundsFunctor<Generator>(rGenerator, rContext);
            }

            /** Generate apriori constant value
             */
            class ConstantFunctor
            {
            public:
                ConstantFunctor( double                         rValue,
                                 const ParserContextSharedPtr&  rContext ) :
                    mnValue( rValue ),
                    mpContext( rContext )
                {
                    ENSURE_OR_THROW( mpContext,
                                      "ConstantFunctor::ConstantFunctor(): Invalid context" );
                }

                void operator()( StringIteratorT, StringIteratorT ) const
                {
                    mpContext->maOperandStack.push(
                        ExpressionNodeFactory::createConstantValueExpression( mnValue ) );
                }

            private:
                const double            mnValue;
                ParserContextSharedPtr  mpContext;
            };

            /** Generate parse-dependent-but-then-constant value
             */
            class DoubleConstantFunctor
            {
            public:
                explicit DoubleConstantFunctor( const ParserContextSharedPtr& rContext ) :
                    mpContext( rContext )
                {
                    ENSURE_OR_THROW( mpContext,
                                      "DoubleConstantFunctor::DoubleConstantFunctor(): Invalid context" );
                }

                void operator()( double n ) const
                {
                    // push constant value expression to the stack
                    mpContext->maOperandStack.push(
                        ExpressionNodeFactory::createConstantValueExpression( n ) );
                }

            private:
                ParserContextSharedPtr  mpContext;
            };

            /** Generate special t value expression node
             */
            class ValueTFunctor
            {
            public:
                explicit ValueTFunctor( const ParserContextSharedPtr& rContext ) :
                    mpContext( rContext )
                {
                    ENSURE_OR_THROW( mpContext,
                                      "ValueTFunctor::ValueTFunctor(): Invalid context" );
                }

                void operator()( StringIteratorT, StringIteratorT ) const
                {
                    if( !mpContext->mbParseAnimationFunction )
                    {
                        SAL_WARN("slideshow", "ValueTFunctor::operator(): variable encountered, but we're not parsing a function here" );
                        throw ParseError();
                    }

                    // push special t value expression to the stack
                    mpContext->maOperandStack.push(
                        ExpressionNodeFactory::createValueTExpression() );
                }

            private:
                ParserContextSharedPtr  mpContext;
            };

            template< typename Functor > class UnaryFunctionFunctor
            {
            private:
                /** ExpressionNode implementation for unary
                    function over one ExpressionNode
                 */
                class UnaryFunctionExpression : public ExpressionNode
                {
                public:
                    UnaryFunctionExpression( const Functor&                 rFunctor,
                                             const ExpressionNodeSharedPtr& rArg ) :
                        maFunctor( rFunctor ),
                        mpArg( rArg )
                    {
                    }

                    virtual double operator()( double t ) const override
                    {
                        return maFunctor( (*mpArg)(t) );
                    }

                    virtual bool isConstant() const override
                    {
                        return mpArg->isConstant();
                    }

                private:
                    Functor                 maFunctor;
                    ExpressionNodeSharedPtr mpArg;
                };

            public:
                UnaryFunctionFunctor( const Functor&                rFunctor,
                                      const ParserContextSharedPtr& rContext ) :
                    maFunctor( rFunctor ),
                    mpContext( rContext )
                {
                    ENSURE_OR_THROW( mpContext,
                                      "UnaryFunctionFunctor::UnaryFunctionFunctor(): Invalid context" );
                }

                void operator()( StringIteratorT, StringIteratorT ) const
                {
                    ParserContext::OperandStack& rNodeStack( mpContext->maOperandStack );

                    if( rNodeStack.size() < 1 )
                        throw ParseError( "Not enough arguments for unary operator" );

                    // retrieve arguments
                    ExpressionNodeSharedPtr pArg( rNodeStack.top() );
                    rNodeStack.pop();

                    // check for constness
                    if( pArg->isConstant() )
                    {
                        rNodeStack.push(
                            ExpressionNodeFactory::createConstantValueExpression(
                                maFunctor( (*pArg)(0.0) ) ) );
                    }
                    else
                    {
                        // push complex node, that calcs the value on demand
                        rNodeStack.push(
                            ExpressionNodeSharedPtr(
                                new UnaryFunctionExpression(
                                    maFunctor,
                                    pArg ) ) );
                    }
                }

            private:
                Functor                 maFunctor;
                ParserContextSharedPtr  mpContext;
            };

            // TODO(Q2): Refactor makeUnaryFunctionFunctor,
            // makeBinaryFunctionFunctor and the whole
            // ExpressionNodeFactory, to use a generic
            // makeFunctionFunctor template, which is overloaded for
            // unary, binary, ternary, etc. function pointers.
            template< typename Functor > UnaryFunctionFunctor<Functor>
                makeUnaryFunctionFunctor( const Functor&                rFunctor,
                                          const ParserContextSharedPtr& rContext )
            {
                return UnaryFunctionFunctor<Functor>( rFunctor, rContext );
            }

            // MSVC has problems instantiating above template function with plain function
            // pointers (doesn't like the const reference there). Thus, provide it with
            // a dedicated overload here.
            UnaryFunctionFunctor< double (*)(double) >
                makeUnaryFunctionFunctor( double (*pFunc)(double),
                                          const ParserContextSharedPtr& rContext )
            {
                return UnaryFunctionFunctor< double (*)(double) >( pFunc, rContext );
            }

            /** Implements a binary function over two ExpressionNodes

                @tpl Generator
                Generator functor, to generate an ExpressionNode of
                appropriate type

             */
            template< class Generator > class BinaryFunctionFunctor
            {
            public:
                BinaryFunctionFunctor( const Generator&                 rGenerator,
                                       const ParserContextSharedPtr&    rContext ) :
                    maGenerator( rGenerator ),
                    mpContext( rContext )
                {
                    ENSURE_OR_THROW( mpContext,
                                      "BinaryFunctionFunctor::BinaryFunctionFunctor(): Invalid context" );
                }

                void operator()( StringIteratorT, StringIteratorT ) const
                {
                    ParserContext::OperandStack& rNodeStack( mpContext->maOperandStack );

                    if( rNodeStack.size() < 2 )
                        throw ParseError( "Not enough arguments for binary operator" );

                    // retrieve arguments
                    ExpressionNodeSharedPtr pSecondArg( rNodeStack.top() );
                    rNodeStack.pop();
                    ExpressionNodeSharedPtr pFirstArg( rNodeStack.top() );
                    rNodeStack.pop();

                    // create combined ExpressionNode
                    ExpressionNodeSharedPtr pNode( maGenerator( pFirstArg,
                                                                pSecondArg ) );
                    // check for constness
                    if( pFirstArg->isConstant() &&
                        pSecondArg->isConstant() )
                    {
                        // call the operator() at pNode, store result
                        // in constant value ExpressionNode.
                        rNodeStack.push(
                            ExpressionNodeFactory::createConstantValueExpression(
                                (*pNode)( 0.0 ) ) );
                    }
                    else
                    {
                        // push complex node, that calcs the value on demand
                        rNodeStack.push( pNode );
                    }
                }

            private:
                Generator               maGenerator;
                ParserContextSharedPtr  mpContext;
            };

            template< typename Generator > BinaryFunctionFunctor<Generator>
                makeBinaryFunctionFunctor( const Generator&                 rGenerator,
                                           const ParserContextSharedPtr&    rContext )
            {
                return BinaryFunctionFunctor<Generator>( rGenerator, rContext );
            }


            // Workaround for MSVC compiler anomaly (stack trashing)

            // The default ureal_parser_policies implementation of parse_exp
            // triggers a really weird error in MSVC7 (Version 13.00.9466), in
            // that the real_parser_impl::parse_main() call of parse_exp()
            // overwrites the frame pointer _on the stack_ (EBP of the calling
            // function gets overwritten while lying on the stack).

            // For the time being, our parser thus can only read the 1.0E10
            // notation, not the 1.0e10 one.

            // TODO(F1): Also handle the 1.0e10 case here.
            template< typename T > struct custom_real_parser_policies : public ::boost::spirit::ureal_parser_policies<T>
            {
                template< typename ScannerT >
                    static typename ::boost::spirit::parser_result< ::boost::spirit::chlit<>, ScannerT >::type
                parse_exp(ScannerT& scan)
                {
                    // as_lower_d somehow breaks MSVC7
                    return ::boost::spirit::ch_p('E').parse(scan);
                }
            };

            /* This class implements the following grammar (more or
               less literally written down below, only slightly
               obfuscated by the parser actions):

               identifier = '$'|'pi'|'e'|'X'|'Y'|'Width'|'Height'

               function = 'abs'|'sqrt'|'sin'|'cos'|'tan'|'atan'|'acos'|'asin'|'exp'|'log'

               basic_expression =
                                number |
                                identifier |
                                function '(' additive_expression ')' |
                                '(' additive_expression ')'

               unary_expression =
                                   '-' basic_expression |
                                basic_expression

               multiplicative_expression =
                                   unary_expression ( ( '*' unary_expression )* |
                                                   ( '/' unary_expression )* )

               additive_expression =
                                   multiplicative_expression ( ( '+' multiplicative_expression )* |
                                                               ( '-' multiplicative_expression )* )

             */
            class ExpressionGrammar : public ::boost::spirit::grammar< ExpressionGrammar >
            {
            public:
                /** Create an arithmetic expression grammar

                    @param rParserContext
                    Contains context info for the parser
                 */
                explicit ExpressionGrammar( const ParserContextSharedPtr& rParserContext ) :
                    mpParserContext( rParserContext )
                {
                }

                template< typename ScannerT > class definition
                {
                public:
                    // grammar definition
                    explicit definition( const ExpressionGrammar& self )
                    {
                        using ::boost::spirit::str_p;
                        using ::boost::spirit::real_parser;

                        identifier =
                                    str_p( "$"      )[ ValueTFunctor(                                                              self.getContext()) ]
                              |     str_p( "pi"     )[ ConstantFunctor(M_PI,                                                       self.getContext()) ]
                              |     str_p( "e"      )[ ConstantFunctor(M_E,                                                        self.getContext()) ]
                              |     str_p( "x"      )[ makeShapeBoundsFunctor(::std::mem_fun_ref(&::basegfx::B2DRange::getCenterX),self.getContext()) ]
                              |     str_p( "y"      )[ makeShapeBoundsFunctor(::std::mem_fun_ref(&::basegfx::B2DRange::getCenterY),self.getContext()) ]
                              |     str_p( "width"  )[ makeShapeBoundsFunctor(::std::mem_fun_ref(&::basegfx::B2DRange::getWidth),  self.getContext()) ]
                              |     str_p( "height" )[ makeShapeBoundsFunctor(::std::mem_fun_ref(&::basegfx::B2DRange::getHeight), self.getContext()) ]
                              ;

                        unaryFunction =
                                (str_p( "abs"  ) >> '(' >> additiveExpression >> ')' )[ makeUnaryFunctionFunctor(&fabs, self.getContext()) ]
                            |   (str_p( "sqrt" ) >> '(' >> additiveExpression >> ')' )[ makeUnaryFunctionFunctor(&sqrt, self.getContext()) ]
                            |   (str_p( "sin"  ) >> '(' >> additiveExpression >> ')' )[ makeUnaryFunctionFunctor(&sin,  self.getContext()) ]
                            |   (str_p( "cos"  ) >> '(' >> additiveExpression >> ')' )[ makeUnaryFunctionFunctor(&cos,  self.getContext()) ]
                            |   (str_p( "tan"  ) >> '(' >> additiveExpression >> ')' )[ makeUnaryFunctionFunctor(&tan,  self.getContext()) ]
                            |   (str_p( "atan" ) >> '(' >> additiveExpression >> ')' )[ makeUnaryFunctionFunctor(&atan, self.getContext()) ]
                            |   (str_p( "acos" ) >> '(' >> additiveExpression >> ')' )[ makeUnaryFunctionFunctor(&acos, self.getContext()) ]
                            |   (str_p( "asin" ) >> '(' >> additiveExpression >> ')' )[ makeUnaryFunctionFunctor(&asin, self.getContext()) ]
                            |   (str_p( "exp"  ) >> '(' >> additiveExpression >> ')' )[ makeUnaryFunctionFunctor(&exp,  self.getContext()) ]
                            |   (str_p( "log"  ) >> '(' >> additiveExpression >> ')' )[ makeUnaryFunctionFunctor(&log,  self.getContext()) ]
                            ;

                        binaryFunction =
                                (str_p( "min"  ) >> '(' >> additiveExpression >> ',' >> additiveExpression >> ')' )[ makeBinaryFunctionFunctor(&ExpressionNodeFactory::createMinExpression, self.getContext()) ]
                            |   (str_p( "max"  ) >> '(' >> additiveExpression >> ',' >> additiveExpression >> ')' )[ makeBinaryFunctionFunctor(&ExpressionNodeFactory::createMaxExpression, self.getContext()) ]
                            ;

                        basicExpression =
                                real_parser<double, custom_real_parser_policies<double> >()[ DoubleConstantFunctor(self.getContext()) ]
                            |   identifier
                            |   unaryFunction
                            |   binaryFunction
                            |   '(' >> additiveExpression >> ')'
                            ;

                        unaryExpression =
                                ('-' >> basicExpression)[ makeUnaryFunctionFunctor(::std::negate<double>(), self.getContext()) ]
                            |   basicExpression
                            ;

                        multiplicativeExpression =
                                unaryExpression
                            >> *( ('*' >> unaryExpression)[ makeBinaryFunctionFunctor(&ExpressionNodeFactory::createMultipliesExpression, self.getContext()) ]
                                | ('/' >> unaryExpression)[ makeBinaryFunctionFunctor(&ExpressionNodeFactory::createDividesExpression,    self.getContext()) ]
                                )
                            ;

                        additiveExpression =
                                multiplicativeExpression
                            >> *( ('+' >> multiplicativeExpression)[ makeBinaryFunctionFunctor(&ExpressionNodeFactory::createPlusExpression,  self.getContext()) ]
                                | ('-' >> multiplicativeExpression)[ makeBinaryFunctionFunctor(&ExpressionNodeFactory::createMinusExpression, self.getContext()) ]
                                )
                            ;

                        BOOST_SPIRIT_DEBUG_RULE(additiveExpression);
                        BOOST_SPIRIT_DEBUG_RULE(multiplicativeExpression);
                        BOOST_SPIRIT_DEBUG_RULE(unaryExpression);
                        BOOST_SPIRIT_DEBUG_RULE(basicExpression);
                        BOOST_SPIRIT_DEBUG_RULE(unaryFunction);
                        BOOST_SPIRIT_DEBUG_RULE(binaryFunction);
                        BOOST_SPIRIT_DEBUG_RULE(identifier);
                    }

                    const ::boost::spirit::rule< ScannerT >& start() const
                    {
                        return additiveExpression;
                    }

                private:
                    // the constituents of the Spirit arithmetic expression grammar.
                    // For the sake of readability, without 'ma' prefix.
                    ::boost::spirit::rule< ScannerT >   additiveExpression;
                    ::boost::spirit::rule< ScannerT >   multiplicativeExpression;
                    ::boost::spirit::rule< ScannerT >   unaryExpression;
                    ::boost::spirit::rule< ScannerT >   basicExpression;
                    ::boost::spirit::rule< ScannerT >   unaryFunction;
                    ::boost::spirit::rule< ScannerT >   binaryFunction;
                    ::boost::spirit::rule< ScannerT >   identifier;
                };

                const ParserContextSharedPtr& getContext() const
                {
                    return mpParserContext;
                }

            private:
                ParserContextSharedPtr  mpParserContext; // might get modified during parsing
            };

#ifdef BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
            const ParserContextSharedPtr& getParserContext()
            {
                static ParserContextSharedPtr lcl_parserContext( new ParserContext() );

                // clear node stack (since we reuse the static object, that's
                // the whole point here)
                while( !lcl_parserContext->maOperandStack.empty() )
                    lcl_parserContext->maOperandStack.pop();

                return lcl_parserContext;
            }
#endif
        }

        ExpressionNodeSharedPtr SmilFunctionParser::parseSmilValue( const OUString&          rSmilValue,
                                                                    const ::basegfx::B2DRectangle&  rRelativeShapeBounds )
        {
            // TODO(Q1): Check if a combination of the RTL_UNICODETOTEXT_FLAGS_*
            // gives better conversion robustness here (we might want to map space
            // etc. to ASCII space here)
            const OString& rAsciiSmilValue(
                OUStringToOString( rSmilValue, RTL_TEXTENCODING_ASCII_US ) );

            StringIteratorT aStart( rAsciiSmilValue.getStr() );
            StringIteratorT aEnd( rAsciiSmilValue.getStr()+rAsciiSmilValue.getLength() );

            ParserContextSharedPtr pContext;

#ifdef BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
            // static parser context, because the actual
            // Spirit parser is also a static object
            pContext = getParserContext();
#else
            pContext.reset( new ParserContext() );
#endif

            pContext->maShapeBounds = rRelativeShapeBounds;
            pContext->mbParseAnimationFunction = false; // parse with '$' disabled


            ExpressionGrammar aExpressionGrammer( pContext );
            const ::boost::spirit::parse_info<StringIteratorT> aParseInfo(
                  ::boost::spirit::parse( aStart,
                                          aEnd,
                                          aExpressionGrammer,
                                          ::boost::spirit::space_p ) );

#if OSL_DEBUG_LEVEL > 0
            ::std::cout.flush(); // needed to keep stdout and cout in sync
#endif

            // input fully congested by the parser?
            if( !aParseInfo.full )
                throw ParseError( "SmilFunctionParser::parseSmilValue(): string not fully parseable" );

            // parser's state stack now must contain exactly _one_ ExpressionNode,
            // which represents our formula.
            if( pContext->maOperandStack.size() != 1 )
                throw ParseError( "SmilFunctionParser::parseSmilValue(): incomplete or empty expression" );

            return pContext->maOperandStack.top();
        }

        ExpressionNodeSharedPtr SmilFunctionParser::parseSmilFunction( const OUString&           rSmilFunction,
                                                                       const ::basegfx::B2DRectangle&   rRelativeShapeBounds )
        {
            // TODO(Q1): Check if a combination of the RTL_UNICODETOTEXT_FLAGS_*
            // gives better conversion robustness here (we might want to map space
            // etc. to ASCII space here)
            const OString& rAsciiSmilFunction(
                OUStringToOString( rSmilFunction, RTL_TEXTENCODING_ASCII_US ) );

            StringIteratorT aStart( rAsciiSmilFunction.getStr() );
            StringIteratorT aEnd( rAsciiSmilFunction.getStr()+rAsciiSmilFunction.getLength() );

            ParserContextSharedPtr pContext;

#ifdef BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
            // static parser context, because the actual
            // Spirit parser is also a static object
            pContext = getParserContext();
#else
            pContext.reset( new ParserContext() );
#endif

            pContext->maShapeBounds = rRelativeShapeBounds;
            pContext->mbParseAnimationFunction = true; // parse with '$' enabled


            ExpressionGrammar aExpressionGrammer( pContext );
            const ::boost::spirit::parse_info<StringIteratorT> aParseInfo(
                  ::boost::spirit::parse( aStart,
                                          aEnd,
                                          aExpressionGrammer >> ::boost::spirit::end_p,
                                          ::boost::spirit::space_p ) );

#if OSL_DEBUG_LEVEL > 0
            ::std::cout.flush(); // needed to keep stdout and cout in sync
#endif
            // input fully congested by the parser?
            if( !aParseInfo.full )
                throw ParseError( "SmilFunctionParser::parseSmilFunction(): string not fully parseable" );

            // parser's state stack now must contain exactly _one_ ExpressionNode,
            // which represents our formula.
            if( pContext->maOperandStack.size() != 1 )
                throw ParseError( "SmilFunctionParser::parseSmilFunction(): incomplete or empty expression" );

            return pContext->maOperandStack.top();
        }
    }
}

#if defined(BOOST_SP_ENABLE_DEBUG_HOOKS)

// debug hooks

namespace boost
{

void sp_scalar_constructor_hook(void *)
{
}

void sp_scalar_destructor_hook(void *)
{
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
