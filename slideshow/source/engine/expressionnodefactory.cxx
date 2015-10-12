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


#include <expressionnodefactory.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>

#include <functional>
#include <algorithm>


/* Implementation of ExpressionNodeFactory class */

namespace slideshow
{
    namespace internal
    {
        namespace
        {
            class ConstantValueExpression : public ExpressionNode
            {
            public:
                explicit ConstantValueExpression( double rValue ) :
                    maValue( rValue )
                {
                }

                virtual double operator()( double /*t*/ ) const override
                {
                    return maValue;
                }

                virtual bool isConstant() const override
                {
                    return true;
                }

            private:
                double  maValue;
            };

            class TValueExpression : public ExpressionNode
            {
            public:
                TValueExpression()
                {
                }

                virtual double operator()( double t ) const override
                {
                    return t;
                }

                virtual bool isConstant() const override
                {
                    return false;
                }
            };

            /** Base class for following binary functions (*+-/)

                Does not pay off to have all this as a template, since
                we'd have to hold the functor as a member (+33% object
                size).
             */
            class BinaryExpressionBase : public ExpressionNode
            {
            public:
                BinaryExpressionBase( const ExpressionNodeSharedPtr&    rFirstArg,
                                      const ExpressionNodeSharedPtr&    rSecondArg ) :
                    mpFirstArg( rFirstArg ),
                    mpSecondArg( rSecondArg )
                {
                }

                virtual bool isConstant() const override
                {
                    return
                        mpFirstArg->isConstant() &&
                        mpSecondArg->isConstant();
                }

            protected:
                ExpressionNodeSharedPtr mpFirstArg;
                ExpressionNodeSharedPtr mpSecondArg;
            };

            class PlusExpression : public BinaryExpressionBase
            {
            public:
                PlusExpression( const ExpressionNodeSharedPtr&  rFirstArg,
                                const ExpressionNodeSharedPtr&  rSecondArg ) :
                    BinaryExpressionBase( rFirstArg, rSecondArg )
                {
                }

                virtual double operator()( double t ) const override
                {
                    return (*mpFirstArg)(t) + (*mpSecondArg)(t);
                }
            };

            class MinusExpression : public BinaryExpressionBase
            {
            public:
                MinusExpression( const ExpressionNodeSharedPtr& rFirstArg,
                                 const ExpressionNodeSharedPtr& rSecondArg ) :
                    BinaryExpressionBase( rFirstArg, rSecondArg )
                {
                }

                virtual double operator()( double t ) const override
                {
                    return (*mpFirstArg)(t) - (*mpSecondArg)(t);
                }
            };

            class MultipliesExpression : public BinaryExpressionBase
            {
            public:
                MultipliesExpression( const ExpressionNodeSharedPtr&    rFirstArg,
                                      const ExpressionNodeSharedPtr&    rSecondArg ) :
                    BinaryExpressionBase( rFirstArg, rSecondArg )
                {
                }

                virtual double operator()( double t ) const override
                {
                    return (*mpFirstArg)(t) * (*mpSecondArg)(t);
                }
            };

            class DividesExpression : public BinaryExpressionBase
            {
            public:
                DividesExpression( const ExpressionNodeSharedPtr&   rFirstArg,
                                   const ExpressionNodeSharedPtr&   rSecondArg ) :
                    BinaryExpressionBase( rFirstArg, rSecondArg )
                {
                }

                virtual double operator()( double t ) const override
                {
                    return (*mpFirstArg)(t) / (*mpSecondArg)(t);
                }
            };

            class MinExpression : public BinaryExpressionBase
            {
            public:
                MinExpression( const ExpressionNodeSharedPtr&   rFirstArg,
                               const ExpressionNodeSharedPtr&   rSecondArg ) :
                    BinaryExpressionBase( rFirstArg, rSecondArg )
                {
                }

                virtual double operator()( double t ) const override
                {
                    return ::std::min( (*mpFirstArg)(t), (*mpSecondArg)(t) );
                }
            };

            class MaxExpression : public BinaryExpressionBase
            {
            public:
                MaxExpression( const ExpressionNodeSharedPtr&   rFirstArg,
                               const ExpressionNodeSharedPtr&   rSecondArg ) :
                    BinaryExpressionBase( rFirstArg, rSecondArg )
                {
                }

                virtual double operator()( double t ) const override
                {
                    return ::std::max( (*mpFirstArg)(t), (*mpSecondArg)(t) );
                }
            };
        }

        ExpressionNodeSharedPtr ExpressionNodeFactory::createConstantValueExpression( double rConstantValue )
        {
            return ExpressionNodeSharedPtr( new ConstantValueExpression(rConstantValue) );
        }

        ExpressionNodeSharedPtr ExpressionNodeFactory::createValueTExpression()
        {
            return ExpressionNodeSharedPtr( new TValueExpression() );
        }

        ExpressionNodeSharedPtr ExpressionNodeFactory::createPlusExpression( const ExpressionNodeSharedPtr& rLHS,
                                                                             const ExpressionNodeSharedPtr& rRHS )
        {
            return ExpressionNodeSharedPtr( new PlusExpression(rLHS, rRHS) );
        }

        ExpressionNodeSharedPtr ExpressionNodeFactory::createMinusExpression( const ExpressionNodeSharedPtr&    rLHS,
                                                                              const ExpressionNodeSharedPtr&    rRHS )
        {
            return ExpressionNodeSharedPtr( new MinusExpression(rLHS, rRHS) );
        }

        ExpressionNodeSharedPtr ExpressionNodeFactory::createMultipliesExpression( const ExpressionNodeSharedPtr&   rLHS,
                                                                                   const ExpressionNodeSharedPtr&   rRHS )
        {
            return ExpressionNodeSharedPtr( new MultipliesExpression(rLHS, rRHS) );
        }

        ExpressionNodeSharedPtr ExpressionNodeFactory::createDividesExpression( const ExpressionNodeSharedPtr&  rLHS,
                                                                                const ExpressionNodeSharedPtr&  rRHS )
        {
            return ExpressionNodeSharedPtr( new DividesExpression(rLHS, rRHS) );
        }

        ExpressionNodeSharedPtr ExpressionNodeFactory::createMinExpression   ( const ExpressionNodeSharedPtr&   rOuterFunction,
                                                                               const ExpressionNodeSharedPtr&   rInnerFunction )
        {
            return ExpressionNodeSharedPtr( new MinExpression(rOuterFunction, rInnerFunction) );
        }

        ExpressionNodeSharedPtr ExpressionNodeFactory::createMaxExpression   ( const ExpressionNodeSharedPtr&   rOuterFunction,
                                                                               const ExpressionNodeSharedPtr&   rInnerFunction )
        {
            return ExpressionNodeSharedPtr( new MaxExpression(rOuterFunction, rInnerFunction) );
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
