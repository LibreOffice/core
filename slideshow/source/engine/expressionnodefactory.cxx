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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <expressionnodefactory.hxx>

#include <canvas/verbosetrace.hxx>

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
                ConstantValueExpression( double rValue ) :
                    maValue( rValue )
                {
                }

                virtual double operator()( double /*t*/ ) const
                {
                    return maValue;
                }

                virtual bool isConstant() const
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

                virtual double operator()( double t ) const
                {
                    return t;
                }

                virtual bool isConstant() const
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

                virtual bool isConstant() const
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

                virtual double operator()( double t ) const
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

                virtual double operator()( double t ) const
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

                virtual double operator()( double t ) const
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

                virtual double operator()( double t ) const
                {
                    return (*mpFirstArg)(t) / (*mpSecondArg)(t);
                }
            };

            class ComposedExpression : public BinaryExpressionBase
            {
            public:
                ComposedExpression( const ExpressionNodeSharedPtr&  rFirstArg,
                                    const ExpressionNodeSharedPtr&  rSecondArg ) :
                    BinaryExpressionBase( rFirstArg, rSecondArg )
                {
                }

                virtual double operator()( double t ) const
                {
                    return (*mpFirstArg)( (*mpSecondArg)(t) );
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

                virtual double operator()( double t ) const
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

                virtual double operator()( double t ) const
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

        ExpressionNodeSharedPtr ExpressionNodeFactory::createComposedExpression  ( const ExpressionNodeSharedPtr&   rOuterFunction,
                                                                                   const ExpressionNodeSharedPtr&   rInnerFunction )
        {
            return ExpressionNodeSharedPtr( new ComposedExpression(rOuterFunction, rInnerFunction) );
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
