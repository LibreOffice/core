/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_SLIDESHOW_EXPRESSIONNODEFACTORY_HXX
#define INCLUDED_SLIDESHOW_EXPRESSIONNODEFACTORY_HXX

#include "expressionnode.hxx"

#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/range/b2drectangle.hxx>


/* Definition of ExpressionNodeFactory class */

namespace slideshow
{
    namespace internal
    {
        /** ExpressionNode factory

            This class can be used to generate a wide variety of
            ExpressionNode objects, e.g. when parsing SMIL function
            expressions.
        */
        class ExpressionNodeFactory
        {
        public:
            static ExpressionNodeSharedPtr createConstantValueExpression( double rConstantValue );

            static ExpressionNodeSharedPtr createValueTExpression    ();

            static ExpressionNodeSharedPtr createPlusExpression      ( const ExpressionNodeSharedPtr&   rLHS,
                                                                       const ExpressionNodeSharedPtr&   rRHS );
            static ExpressionNodeSharedPtr createMinusExpression     ( const ExpressionNodeSharedPtr&   rLHS,
                                                                       const ExpressionNodeSharedPtr&   rRHS );
            static ExpressionNodeSharedPtr createMultipliesExpression( const ExpressionNodeSharedPtr&   rLHS,
                                                                       const ExpressionNodeSharedPtr&   rRHS );
            static ExpressionNodeSharedPtr createDividesExpression   ( const ExpressionNodeSharedPtr&   rLHS,
                                                                       const ExpressionNodeSharedPtr&   rRHS );

            /** Composes two ExpressionNode function.

                The resulting expression will calculate
                rOuterFunction( rInnerFunction(t) ).
             */
            static ExpressionNodeSharedPtr createComposedExpression  ( const ExpressionNodeSharedPtr&   rOuterFunction,
                                                                       const ExpressionNodeSharedPtr&   rInnerFunction );

            static ExpressionNodeSharedPtr createMinExpression      ( const ExpressionNodeSharedPtr&    rOuterFunction,
                                                                      const ExpressionNodeSharedPtr&    rInnerFunction );

            static ExpressionNodeSharedPtr createMaxExpression      ( const ExpressionNodeSharedPtr&    rOuterFunction,
                                                                      const ExpressionNodeSharedPtr&    rInnerFunction );
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_EXPRESSIONNODEFACTORY_HXX */
