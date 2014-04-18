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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_EXPRESSIONNODEFACTORY_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_EXPRESSIONNODEFACTORY_HXX

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
            static ExpressionNodeSharedPtr createMinExpression      ( const ExpressionNodeSharedPtr&    rOuterFunction,
                                                                      const ExpressionNodeSharedPtr&    rInnerFunction );

            static ExpressionNodeSharedPtr createMaxExpression      ( const ExpressionNodeSharedPtr&    rOuterFunction,
                                                                      const ExpressionNodeSharedPtr&    rInnerFunction );
        };
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_EXPRESSIONNODEFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
