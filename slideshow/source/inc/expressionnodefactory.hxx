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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
