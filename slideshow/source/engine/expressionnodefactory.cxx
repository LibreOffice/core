/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: expressionnodefactory.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:25:56 $
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
#include <expressionnodefactory.hxx>

#ifndef _CANVAS_VERBOSETRACE_HXX
#include <canvas/verbosetrace.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#include <functional>
#include <algorithm>


/* Implementation of ExpressionNodeFactory class */

namespace presentation
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

                virtual double operator()( double t ) const
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
