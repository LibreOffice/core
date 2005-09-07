/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: smilfunctionparser.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:30:46 $
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

// must be first
#include <canvas/debug.hxx>

#ifndef  _USE_MATH_DEFINES
#define  _USE_MATH_DEFINES  // needed by Visual C++ for math constants
#endif
#include <math.h>           // M_PI definition

#include <smilfunctionparser.hxx>
#include <expressionnodefactory.hxx>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CANVAS_VERBOSETRACE_HXX
#include <canvas/verbosetrace.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

// Makes parser a static resource,
// we're synchronized externally.
// But watch out, the parser might have
// state not visible to this code!
#define BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
#if defined(VERBOSE) && defined(DBG_UTIL)
#include <typeinfo>
#define BOOST_SPIRIT_DEBUG
#endif
#include <boost/spirit/core.hpp>

#include <iostream>
#include <functional>
#include <algorithm>
#include <stack>



/* Implementation of SmilFunctionParser class */

namespace presentation
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
                    ENSURE_AND_THROW( mpContext.get(),
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
                    ENSURE_AND_THROW( mpContext.get(),
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
                DoubleConstantFunctor( const ParserContextSharedPtr& rContext ) :
                    mpContext( rContext )
                {
                    ENSURE_AND_THROW( mpContext.get(),
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
                ValueTFunctor( const ParserContextSharedPtr& rContext ) :
                    mpContext( rContext )
                {
                    ENSURE_AND_THROW( mpContext.get(),
                                      "ValueTFunctor::ValueTFunctor(): Invalid context" );
                }

                void operator()( StringIteratorT, StringIteratorT ) const
                {
                    if( !mpContext->mbParseAnimationFunction )
                    {
                        OSL_ENSURE( false,
                                    "ValueTFunctor::operator(): variable encountered, but we're not parsing a function here" );
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

                    virtual double operator()( double t ) const
                    {
                        return maFunctor( (*mpArg)(t) );
                    }

                    virtual bool isConstant() const
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
                    ENSURE_AND_THROW( mpContext.get(),
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
                    ENSURE_AND_THROW( mpContext.get(),
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
            //
            // The default ureal_parser_policies implementation of parse_exp
            // triggers a really weird error in MSVC7 (Version 13.00.9466), in
            // that the real_parser_impl::parse_main() call of parse_exp()
            // overwrites the frame pointer _on the stack_ (EBP of the calling
            // function gets overwritten while lying on the stack).
            //
            // For the time being, our parser thus can only read the 1.0E10
            // notation, not the 1.0e10 one.
            //
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
                ExpressionGrammar( const ParserContextSharedPtr& rParserContext ) :
                    mpParserContext( rParserContext )
                {
                }

                template< typename ScannerT > class definition
                {
                public:
                    // grammar definition
                    definition( const ExpressionGrammar& self )
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

        ExpressionNodeSharedPtr SmilFunctionParser::parseSmilValue( const ::rtl::OUString&          rSmilValue,
                                                                    const ::basegfx::B2DRectangle&  rRelativeShapeBounds )
        {
            // TODO(Q1): Check if a combination of the RTL_UNICODETOTEXT_FLAGS_*
            // gives better conversion robustness here (we might want to map space
            // etc. to ASCII space here)
            const ::rtl::OString& rAsciiSmilValue(
                rtl::OUStringToOString( rSmilValue, RTL_TEXTENCODING_ASCII_US ) );

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
            OSL_DEBUG_ONLY(::std::cout.flush()); // needed to keep stdout and cout in sync

            // input fully congested by the parser?
            if( !aParseInfo.full )
                throw ParseError( "SmilFunctionParser::parseSmilValue(): string not fully parseable" );

            // parser's state stack now must contain exactly _one_ ExpressionNode,
            // which represents our formula.
            if( pContext->maOperandStack.size() != 1 )
                throw ParseError( "SmilFunctionParser::parseSmilValue(): incomplete or empty expression" );

            return pContext->maOperandStack.top();
        }

        ExpressionNodeSharedPtr SmilFunctionParser::parseSmilFunction( const ::rtl::OUString&           rSmilFunction,
                                                                       const ::basegfx::B2DRectangle&   rRelativeShapeBounds )
        {
            // TODO(Q1): Check if a combination of the RTL_UNICODETOTEXT_FLAGS_*
            // gives better conversion robustness here (we might want to map space
            // etc. to ASCII space here)
            const ::rtl::OString& rAsciiSmilFunction(
                rtl::OUStringToOString( rSmilFunction, RTL_TEXTENCODING_ASCII_US ) );

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
                                          aExpressionGrammer,
                                          ::boost::spirit::space_p ) );
            OSL_DEBUG_ONLY(::std::cout.flush()); // needed to keep stdout and cout in sync

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
