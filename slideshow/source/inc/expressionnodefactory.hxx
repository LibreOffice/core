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

#include <memory>


/* Definition of ExpressionNodeFactory class */

namespace slideshow::internal
    {
        /** ExpressionNode factory

            This class can be used to generate a wide variety of
            ExpressionNode objects, e.g. when parsing SMIL function
            expressions.
        */
        class ExpressionNodeFactory
        {
        public:
            static std::shared_ptr<ExpressionNode> createConstantValueExpression( double rConstantValue );

            static std::shared_ptr<ExpressionNode> createValueTExpression    ();

            static std::shared_ptr<ExpressionNode> createPlusExpression      ( const std::shared_ptr<ExpressionNode>&   rLHS,
                                                                       const std::shared_ptr<ExpressionNode>&   rRHS );
            static std::shared_ptr<ExpressionNode> createMinusExpression     ( const std::shared_ptr<ExpressionNode>&   rLHS,
                                                                       const std::shared_ptr<ExpressionNode>&   rRHS );
            static std::shared_ptr<ExpressionNode> createMultipliesExpression( const std::shared_ptr<ExpressionNode>&   rLHS,
                                                                       const std::shared_ptr<ExpressionNode>&   rRHS );
            static std::shared_ptr<ExpressionNode> createDividesExpression   ( const std::shared_ptr<ExpressionNode>&   rLHS,
                                                                       const std::shared_ptr<ExpressionNode>&   rRHS );

            /** Composes two ExpressionNode function.

                The resulting expression will calculate
                rOuterFunction( rInnerFunction(t) ).
             */
            static std::shared_ptr<ExpressionNode> createMinExpression      ( const std::shared_ptr<ExpressionNode>&    rOuterFunction,
                                                                      const std::shared_ptr<ExpressionNode>&    rInnerFunction );

            static std::shared_ptr<ExpressionNode> createMaxExpression      ( const std::shared_ptr<ExpressionNode>&    rOuterFunction,
                                                                      const std::shared_ptr<ExpressionNode>&    rInnerFunction );
        };

}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_EXPRESSIONNODEFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
