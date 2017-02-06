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


// Makes parser a static resource,
// we're synchronized externally.
// But watch out, the parser might have
// state not visible to this code!
#define BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
#if OSL_DEBUG_LEVEL >= 2 && defined(DBG_UTIL)
#include <typeinfo>
#define BOOST_SPIRIT_DEBUG
#endif
#include <boost/spirit/include/classic_core.hpp>
#include "RowFunctionParser.hxx"
#include <rtl/ustring.hxx>
#include <tools/fract.hxx>


#if (OSL_DEBUG_LEVEL > 0)
#include <iostream>
#endif
#include <functional>
#include <algorithm>
#include <stack>

namespace connectivity
{
using namespace com::sun::star;

namespace
{


// EXPRESSION NODES


class ConstantValueExpression : public ExpressionNode
{
    ORowSetValueDecoratorRef maValue;

public:

    explicit ConstantValueExpression( ORowSetValueDecoratorRef const & rValue ) :
        maValue( rValue )
    {
    }
    virtual ORowSetValueDecoratorRef evaluate(const ODatabaseMetaDataResultSet::ORow& /*_aRow*/ ) const override
    {
        return maValue;
    }
    virtual void fill(const ODatabaseMetaDataResultSet::ORow& /*_aRow*/ ) const override
    {
    }
};


/** ExpressionNode implementation for unary
    function over two ExpressionNodes
    */
class BinaryFunctionExpression : public ExpressionNode
{
    const ExpressionFunct   meFunct;
    std::shared_ptr<ExpressionNode> mpFirstArg;
    std::shared_ptr<ExpressionNode> mpSecondArg;

public:

    BinaryFunctionExpression( const ExpressionFunct eFunct, const std::shared_ptr<ExpressionNode>& rFirstArg, const std::shared_ptr<ExpressionNode>& rSecondArg ) :
        meFunct( eFunct ),
        mpFirstArg( rFirstArg ),
        mpSecondArg( rSecondArg )
    {
    }
    virtual ORowSetValueDecoratorRef evaluate(const ODatabaseMetaDataResultSet::ORow& _aRow ) const override
    {
        ORowSetValueDecoratorRef aRet;
        switch(meFunct)
        {
            case ExpressionFunct::Equation:
                aRet = new ORowSetValueDecorator( mpFirstArg->evaluate(_aRow )->getValue() == mpSecondArg->evaluate(_aRow )->getValue() );
                break;
            case ExpressionFunct::And:
                aRet = new ORowSetValueDecorator( mpFirstArg->evaluate(_aRow )->getValue().getBool() && mpSecondArg->evaluate(_aRow )->getValue().getBool() );
                break;
            case ExpressionFunct::Or:
                aRet = new ORowSetValueDecorator( mpFirstArg->evaluate(_aRow )->getValue().getBool() || mpSecondArg->evaluate(_aRow )->getValue().getBool() );
                break;
            default:
                break;
        }
        return aRet;
    }
    virtual void fill(const ODatabaseMetaDataResultSet::ORow& _aRow ) const override
    {
        switch(meFunct)
        {
            case ExpressionFunct::Equation:
                (*mpFirstArg->evaluate(_aRow )) = mpSecondArg->evaluate(_aRow )->getValue();
                break;
            default:
                break;
        }
    }
};


// FUNCTION PARSER


typedef const sal_Char* StringIteratorT;

struct ParserContext
{
    typedef ::std::stack< std::shared_ptr<ExpressionNode> > OperandStack;

    // stores a stack of not-yet-evaluated operands. This is used
    // by the operators (i.e. '+', '*', 'sin' etc.) to pop their
    // arguments from. If all arguments to an operator are constant,
    // the operator pushes a precalculated result on the stack, and
    // a composite ExpressionNode otherwise.
    OperandStack                            maOperandStack;
};

typedef std::shared_ptr< ParserContext > ParserContextSharedPtr;

/** Generate apriori constant value
    */

class ConstantFunctor
{
    ParserContextSharedPtr          mpContext;

public:

    explicit ConstantFunctor( const ParserContextSharedPtr& rContext ) :
        mpContext( rContext )
    {
    }
    void operator()( StringIteratorT rFirst,StringIteratorT rSecond) const
    {
        OUString sVal( rFirst, rSecond - rFirst, RTL_TEXTENCODING_UTF8 );
        mpContext->maOperandStack.push( std::shared_ptr<ExpressionNode>( new ConstantValueExpression( new ORowSetValueDecorator( sVal ) ) ) );
    }
};

/** Generate parse-dependent-but-then-constant value
    */
class IntConstantFunctor
{
    ParserContextSharedPtr  mpContext;

public:
    explicit IntConstantFunctor( const ParserContextSharedPtr& rContext ) :
        mpContext( rContext )
    {
    }
    void operator()( sal_Int32 n ) const
    {
        mpContext->maOperandStack.push( std::shared_ptr<ExpressionNode>( new ConstantValueExpression( new ORowSetValueDecorator( n ) ) ) );
    }
};

/** Implements a binary function over two ExpressionNodes

    @tpl Generator
    Generator functor, to generate an ExpressionNode of
    appropriate type

    */
class BinaryFunctionFunctor
{
    const ExpressionFunct   meFunct;
    ParserContextSharedPtr  mpContext;

public:

    BinaryFunctionFunctor( const ExpressionFunct eFunct, const ParserContextSharedPtr& rContext ) :
        meFunct( eFunct ),
        mpContext( rContext )
    {
    }

    void operator()( StringIteratorT, StringIteratorT ) const
    {
        ParserContext::OperandStack& rNodeStack( mpContext->maOperandStack );

        if( rNodeStack.size() < 2 )
            throw ParseError( "Not enough arguments for binary operator" );

        // retrieve arguments
        std::shared_ptr<ExpressionNode> pSecondArg( rNodeStack.top() );
        rNodeStack.pop();
        std::shared_ptr<ExpressionNode> pFirstArg( rNodeStack.top() );
        rNodeStack.pop();

        // create combined ExpressionNode
        std::shared_ptr<ExpressionNode> pNode = std::shared_ptr<ExpressionNode>( new BinaryFunctionExpression( meFunct, pFirstArg, pSecondArg ) );
        // check for constness
        rNodeStack.push( pNode );
    }
};
/** ExpressionNode implementation for unary
    function over one ExpressionNode
    */
class UnaryFunctionExpression : public ExpressionNode
{
    std::shared_ptr<ExpressionNode> mpArg;

public:
    explicit UnaryFunctionExpression( const std::shared_ptr<ExpressionNode>& rArg ) :
        mpArg( rArg )
    {
    }
    virtual ORowSetValueDecoratorRef evaluate(const ODatabaseMetaDataResultSet::ORow& _aRow ) const override
    {
        return _aRow[mpArg->evaluate(_aRow )->getValue().getInt32()];
    }
    virtual void fill(const ODatabaseMetaDataResultSet::ORow& /*_aRow*/ ) const override
    {
    }
};

class UnaryFunctionFunctor
{
    ParserContextSharedPtr  mpContext;

public:

    explicit UnaryFunctionFunctor(const ParserContextSharedPtr& rContext)
        : mpContext(rContext)
    {
    }
    void operator()( StringIteratorT, StringIteratorT ) const
    {

        ParserContext::OperandStack& rNodeStack( mpContext->maOperandStack );

        if( rNodeStack.size() < 1 )
            throw ParseError( "Not enough arguments for unary operator" );

        // retrieve arguments
        std::shared_ptr<ExpressionNode> pArg( rNodeStack.top() );
        rNodeStack.pop();

        rNodeStack.push( std::shared_ptr<ExpressionNode>( new UnaryFunctionExpression( pArg ) ) );
    }
};

/* This class implements the following grammar (more or
    less literally written down below, only slightly
    obfuscated by the parser actions):

    basic_expression =
                       number |
                       '(' additive_expression ')'

    unary_expression =
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
            using ::boost::spirit::space_p;
            using ::boost::spirit::range_p;
            using ::boost::spirit::lexeme_d;
            using ::boost::spirit::ch_p;
            using ::boost::spirit::int_p;
            using ::boost::spirit::as_lower_d;
            using ::boost::spirit::strlit;
            using ::boost::spirit::inhibit_case;


            typedef inhibit_case<strlit<> > token_t;
            token_t COLUMN  = as_lower_d[ "column" ];
            token_t OR_     = as_lower_d[ "or" ];
            token_t AND_    = as_lower_d[ "and" ];

            integer =
                    int_p
                                [IntConstantFunctor(self.getContext())];

            argument =
                    integer
                |    lexeme_d[ +( range_p('a','z') | range_p('A','Z') | range_p('0','9') ) ]
                                [ ConstantFunctor(self.getContext()) ]
               ;

            unaryFunction =
                    (COLUMN >> '(' >> integer >> ')' )
                                [ UnaryFunctionFunctor( self.getContext()) ]
                ;

            assignment =
                    unaryFunction >> ch_p('=') >> argument
                                [ BinaryFunctionFunctor( ExpressionFunct::Equation,  self.getContext()) ]
               ;

            andExpression =
                    assignment
                |   ( '(' >> orExpression >> ')' )
                |   ( assignment >> AND_ >> assignment )  [ BinaryFunctionFunctor( ExpressionFunct::And,  self.getContext()) ]
                ;

            orExpression =
                    andExpression
                |   ( orExpression >> OR_ >> andExpression ) [ BinaryFunctionFunctor( ExpressionFunct::Or,  self.getContext()) ]
                ;

            basicExpression =
                    orExpression
                ;

            BOOST_SPIRIT_DEBUG_RULE(basicExpression);
            BOOST_SPIRIT_DEBUG_RULE(unaryFunction);
            BOOST_SPIRIT_DEBUG_RULE(assignment);
            BOOST_SPIRIT_DEBUG_RULE(argument);
            BOOST_SPIRIT_DEBUG_RULE(integer);
            BOOST_SPIRIT_DEBUG_RULE(orExpression);
            BOOST_SPIRIT_DEBUG_RULE(andExpression);
        }

        const ::boost::spirit::rule< ScannerT >& start() const
        {
            return basicExpression;
        }

    private:
        // the constituents of the Spirit arithmetic expression grammar.
        // For the sake of readability, without 'ma' prefix.
        ::boost::spirit::rule< ScannerT >   basicExpression;
        ::boost::spirit::rule< ScannerT >   unaryFunction;
        ::boost::spirit::rule< ScannerT >   assignment;
        ::boost::spirit::rule< ScannerT >   integer,argument;
        ::boost::spirit::rule< ScannerT >   orExpression,andExpression;
    };

    const ParserContextSharedPtr& getContext() const
    {
        return mpParserContext;
    }

private:
    ParserContextSharedPtr          mpParserContext; // might get modified during parsing
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

std::shared_ptr<ExpressionNode> FunctionParser::parseFunction( const OUString& _sFunction)
{
    // TODO(Q1): Check if a combination of the RTL_UNICODETOTEXT_FLAGS_*
    // gives better conversion robustness here (we might want to map space
    // etc. to ASCII space here)
    const OString& rAsciiFunction(
        OUStringToOString( _sFunction, RTL_TEXTENCODING_ASCII_US ) );

    StringIteratorT aStart( rAsciiFunction.getStr() );
    StringIteratorT aEnd( rAsciiFunction.getStr()+rAsciiFunction.getLength() );

    ParserContextSharedPtr pContext;

#ifdef BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
    // static parser context, because the actual
    // Spirit parser is also a static object
    pContext = getParserContext();
#else
    pContext.reset( new ParserContext() );
#endif

    ExpressionGrammar aExpressionGrammer( pContext );

    const ::boost::spirit::parse_info<StringIteratorT> aParseInfo(
            ::boost::spirit::parse( aStart,
                                    aEnd,
                                    aExpressionGrammer,
                                    ::boost::spirit::space_p ) );

#if (OSL_DEBUG_LEVEL > 0)
    ::std::cout.flush(); // needed to keep stdout and cout in sync
#endif

    // input fully congested by the parser?
    if( !aParseInfo.full )
        throw ParseError( "RowFunctionParser::parseFunction(): string not fully parseable" );

    // parser's state stack now must contain exactly _one_ ExpressionNode,
    // which represents our formula.
    if( pContext->maOperandStack.size() != 1 )
        throw ParseError( "RowFunctionParser::parseFunction(): incomplete or empty expression" );

    return pContext->maOperandStack.top();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
